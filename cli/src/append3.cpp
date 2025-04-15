#include <iostream>
#include <filesystem>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/statvfs.h> // For statvfs
#include <cerrno>
#include <cstring>

// Btrfs support
#ifdef __linux__
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <linux/btrfs.h>
#ifdef BTRFS_IOC_CLONE_RANGE
#define HAVE_BTRFS 1
#ifndef BTRFS_SUPER_MAGIC
#define BTRFS_SUPER_MAGIC 0x9123683E
#endif
#endif
#endif

// ZFS support
#ifdef __linux__
#ifndef ZFS_SUPER_MAGIC
#define ZFS_SUPER_MAGIC 0x2fc12fc1
#endif
#define HAVE_ZFS 1
#endif
#ifdef __FreeBSD__
#include <sys/capsicum.h>
#define HAVE_ZFS 1
#ifndef ZFS_SUPER_MAGIC
#define ZFS_SUPER_MAGIC 0x2fc12fc1
#endif
#endif

namespace fs = std::filesystem;

// Get the filesystem block size for a given file
off_t get_block_size(const fs::path& file) {
    struct statvfs stat;
    if (statvfs(file.c_str(), &stat) == -1) {
        std::cerr << "Failed to get block size for " << file << ": " << strerror(errno) << ", defaulting to 4096\n";
        return 4096; // Default to 4KB if query fails
    }
    return static_cast<off_t>(stat.f_bsize); // f_bsize is the optimal block size
}

// Pad a file to the next block size multiple with spaces
bool pad_to_block_size(const fs::path& file) {
    int fd = open(file.c_str(), O_RDWR);
    if (fd == -1) {
        std::cerr << "Failed to open " << file << " for padding: " << strerror(errno) << "\n";
        return false;
    }

    off_t size = lseek(fd, 0, SEEK_END);
    if (size == -1) {
        std::cerr << "Failed to seek " << file << " for padding: " << strerror(errno) << "\n";
        close(fd);
        return false;
    }

    off_t block_size = get_block_size(file);
    off_t remainder = size % block_size;
    if (remainder == 0) {
        close(fd);
        return true; // Already a multiple of block size
    }

    off_t padding = block_size - remainder;
    char buffer[4096];
    memset(buffer, ' ', sizeof(buffer)); // Fill with spaces

    // Write padding in chunks if needed
    while (padding > 0) {
        ssize_t to_write = std::min(static_cast<off_t>(sizeof(buffer)), padding);
        if (write(fd, buffer, static_cast<size_t>(to_write)) != to_write) {
            std::cerr << "Failed to pad " << file << ": " << strerror(errno) << "\n";
            close(fd);
            return false;
        }
        padding -= to_write;
    }

    close(fd);
    return true;
}

bool is_btrfs(const fs::path& path) {
#ifdef HAVE_BTRFS
    struct statfs statfs_buf;
    if (statfs(path.c_str(), &statfs_buf) == -1) {
        std::cerr << "statfs failed: " << strerror(errno) << "\n";
        return false;
    }
    std::cerr << "Filesystem type: 0x" << std::hex << std::showbase << statfs_buf.f_type << std::dec << "\n";
    return statfs_buf.f_type == BTRFS_SUPER_MAGIC;
#else
    // std::cerr << "Btrfs support not compiled in\n";
    return false;
#endif
}

bool is_zfs(const fs::path& path) {
#ifdef HAVE_ZFS
    struct statfs statfs_buf;
    if (statfs(path.c_str(), &statfs_buf) == -1) {
        std::cerr << "statfs failed: " << strerror(errno) << "\n";
        return false;
    }
    std::cerr << "Filesystem type: 0x" << std::hex << std::showbase << statfs_buf.f_type << std::dec << "\n";
    return statfs_buf.f_type == ZFS_SUPER_MAGIC;
#else
    // std::cerr << "ZFS support not compiled in\n";
    return false;
#endif
}

bool append_btrfs_clone(const fs::path& target, const fs::path& source) {
#ifdef HAVE_BTRFS
    // Pad both files to block size multiples
    if (!pad_to_block_size(target) || !pad_to_block_size(source)) {
        std::cerr << "Padding failed, falling back to copy\n";
        return false;
    }

    int fd_target = open(target.c_str(), O_RDWR);
    if (fd_target == -1) {
        std::cerr << "Failed to open target: " << strerror(errno) << "\n";
        return false;
    }
    int fd_source = open(source.c_str(), O_RDONLY);
    if (fd_source == -1) {
        std::cerr << "Failed to open source: " << strerror(errno) << "\n";
        close(fd_target);
        return false;
    }

    // struct stat stat_target, stat_source;
    // if (fstat(fd_target, &stat_target) == -1 || fstat(fd_source, &stat_source) == -1) {
    //     std::cerr << "fstat failed: " << strerror(errno) << "\n";
    //     close(fd_source);
    //     close(fd_target);
    //     return false;
    // }
    // std::cerr << "Target inode: " << stat_target.st_ino << ", Source inode: " << stat_source.st_ino << "\n";

    off_t target_size = lseek(fd_target, 0, SEEK_END);
    if (target_size == -1) {
        std::cerr << "Failed to seek target: " << strerror(errno) << "\n";
        close(fd_source);
        close(fd_target);
        return false;
    }
    off_t source_size = lseek(fd_source, 0, SEEK_END);
    if (source_size == -1) {
        std::cerr << "Failed to seek source: " << strerror(errno) << "\n";
        close(fd_source);
        close(fd_target);
        return false;
    }
    if (lseek(fd_source, 0, SEEK_SET) == -1) {
        std::cerr << "Failed to reset source offset: " << strerror(errno) << "\n";
        close(fd_source);
        close(fd_target);
        return false;
    }

    std::cerr << "Cloning: src_fd=" << fd_source << ", src_size=" << source_size
              << ", dest_fd=" << fd_target << ", dest_offset=" << target_size << "\n";

    struct btrfs_ioctl_clone_range_args args = {
        .src_fd = fd_source,
        .src_offset = 0,
        .src_length = static_cast<__u64>(source_size),
        .dest_offset = static_cast<__u64>(target_size)
    };

    bool success = (ioctl(fd_target, BTRFS_IOC_CLONE_RANGE, &args) != -1);
    if (!success) {
        std::cerr << "Clone failed: " << strerror(errno) << " (falling back)\n";
    }
    close(fd_source);
    close(fd_target);
    return success;
#else
    // std::cerr << "Btrfs cloning not available, falling back to copy\n";
    return false;
#endif
}

bool append_zfs_clone(const fs::path& target, const fs::path& source) {
#ifdef HAVE_ZFS
    // Pad both files to block size multiples
    if (!pad_to_block_size(target) || !pad_to_block_size(source)) {
        std::cerr << "Padding failed, falling back to copy\n";
        return false;
    }

    int fd_target = open(target.c_str(), O_RDWR);
    if (fd_target == -1) {
        std::cerr << "Failed to open target: " << strerror(errno) << "\n";
        return false;
    }
    int fd_source = open(source.c_str(), O_RDONLY);
    if (fd_source == -1) {
        std::cerr << "Failed to open source: " << strerror(errno) << "\n";
        close(fd_target);
        return false;
    }

    // struct stat stat_target, stat_source;
    // if (fstat(fd_target, &stat_target) == -1 || fstat(fd_source, &stat_source) == -1) {
    //     std::cerr << "fstat failed: " << strerror(errno) << "\n";
    //     close(fd_source);
    //     close(fd_target);
    //     return false;
    // }
    // std::cerr << "Target inode: " << stat_target.st_ino << ", Source inode: " << stat_source.st_ino << "\n";

    off_t target_size = lseek(fd_target, 0, SEEK_END);
    if (target_size == -1) {
        std::cerr << "Failed to seek target: " << strerror(errno) << "\n";
        close(fd_source);
        close(fd_target);
        return false;
    }
    off_t source_size = lseek(fd_source, 0, SEEK_END);
    if (source_size == -1) {
        std::cerr << "Failed to seek source: " << strerror(errno) << "\n";
        close(fd_source);
        close(fd_target);
        return false;
    }
    if (lseek(fd_source, 0, SEEK_SET) == -1) {
        std::cerr << "Failed to reset source offset: " << strerror(errno) << "\n";
        close(fd_source);
        close(fd_target);
        return false;
    }

    std::cerr << "ZFS Cloning attempt: src_fd=" << fd_source << ", src_size=" << source_size
              << ", dest_fd=" << fd_target << ", dest_offset=" << target_size << "\n";

    fs::path temp = target.string() + ".tmp";
    int fd_temp = open(temp.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_temp == -1) {
        std::cerr << "Failed to create temp file: " << strerror(errno) << "\n";
        close(fd_source);
        close(fd_target);
        return false;
    }

    ssize_t cloned = copy_file_range(fd_source, nullptr, fd_temp, nullptr, static_cast<size_t>(source_size), 0);
    if (cloned != source_size) {
        std::cerr << "ZFS clone via copy_file_range failed: " << strerror(errno) << " (falling back)\n";
        close(fd_temp);
        close(fd_source);
        close(fd_target);
        fs::remove(temp);
        return false;
    }

    lseek(fd_temp, 0, SEEK_SET);
    char buffer[4096];
    ssize_t bytes;
    while ((bytes = read(fd_temp, buffer, sizeof(buffer))) > 0) {
        if (write(fd_target, buffer, static_cast<size_t>(bytes)) != bytes) {
            std::cerr << "Failed to append cloned data: " << strerror(errno) << "\n";
            close(fd_temp);
            close(fd_source);
            close(fd_target);
            fs::remove(temp);
            return false;
        }
    }
    if (bytes == -1) {
        std::cerr << "Read from temp failed: " << strerror(errno) << "\n";
        close(fd_temp);
        close(fd_source);
        close(fd_target);
        fs::remove(temp);
        return false;
    }

    close(fd_temp);
    close(fd_source);
    close(fd_target);
    fs::remove(temp);
    return true;
#else
    // std::cerr << "ZFS cloning not available, falling back to copy\n";
    return false;
#endif
}

void append_copy_remove(const fs::path& target, const fs::path& source) {
    FILE* f_target = fopen(target.c_str(), "ab");
    if (!f_target) {
        throw std::runtime_error("Failed to open target file: " + std::string(strerror(errno)));
    }
    FILE* f_source = fopen(source.c_str(), "rb");
    if (!f_source) {
        fclose(f_target);
        throw std::runtime_error("Failed to open source file: " + std::string(strerror(errno)));
    }

    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), f_source)) > 0) {
        if (fwrite(buffer, 1, bytes, f_target) != bytes) {
            fclose(f_source);
            fclose(f_target);
            throw std::runtime_error("Failed to write to target: " + std::string(strerror(errno)));
        }
    }
    if (ferror(f_source)) {
        fclose(f_source);
        fclose(f_target);
        throw std::runtime_error("Failed to read source: " + std::string(strerror(errno)));
    }

    fclose(f_source);
    fclose(f_target);

    if (!fs::remove(source)) {
        throw std::runtime_error("Failed to remove source file: " + std::string(strerror(errno)));
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " target_file source_file\nFiles will be padded to file system block size with spaces first.\n";
        return 1;
    }

    fs::path target = argv[1];
    fs::path source = argv[2];

    try {
        if (!fs::exists(target) || !fs::exists(source)) {
            std::cerr << "Error: One or both files do not exist\n";
            return 1;
        }

        bool cloned = false;
        if (is_btrfs(target)) {
            cloned = append_btrfs_clone(target, source);
        } else if (is_zfs(target)) {
            cloned = append_zfs_clone(target, source);
        }
        if (cloned) {
            std::cout << "Appended " << source << " to " << target << " using filesystem clone\n";
            if (!fs::remove(source)) {
                std::cerr << "Warning: Failed to remove " << source << ": " << strerror(errno) << "\n";
                return 1;
            }
            std::cout << "Removed " << source << "\n";
        } else {
            append_copy_remove(target, source);
            std::cout << "Appended " << source << " to " << target << " via copy and removed " << source << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
