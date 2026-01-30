// fixdirtime.cpp
// Compile: g++ -std=c++17 -O2 -Wall -Wextra -o dir-mtime-max dir-mtime-max.cpp

#define _XOPEN_SOURCE 700
#include <ftw.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#if EXO_MODULE
	import std;
	import var;
#else
#	include <vector>
#	include <algorithm>
#	include <cstdio>
#	include <ctime>
#	include <cstring>
#	include <string>
#endif

static thread_local std::vector<struct timespec> ftw_stack;
static thread_local size_t                       ftw_recursive_size {0};
static thread_local timespec                     ftw_max_mtime      {0, 0};

static thread_local bool ftw_update  {false};
static thread_local int  ftw_verbose {0};
static thread_local bool ftw_ignore  {false};

#pragma clang diagnostic ignored "-Wsign-conversion"
static void log(int level, const char* mode, const char* fpath, struct timespec mtime1)
{
	char timestr[64];
	{
		std::strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", std::localtime(&mtime1.tv_sec));
		size_t len = std::strlen(timestr);
		std::snprintf(timestr + len, sizeof(timestr) - len, ".%09ld", mtime1.tv_nsec);
		if (ftw_verbose)
			std::printf("[%-6s] %d: %-40s → %s\n", mode, level, fpath, timestr);
	}

	// Validate if not updating (slow - calls find)
	if (strcmp(mode, "UPDREQ") == 0) {
		using namespace exo;

//		// No validation if cannot update
//		if (access(fpath, W_OK | X_OK) == 0)
//			return;
//
		var result;
		const var cmd = "find " ^ var(fpath).squote() ^ " -exec stat -c '%y %n' {} + | sort | tail -1";
		if (not result.osshellread(cmd)) {
			var::lasterror().errputl(cmd);
			if (!ftw_ignore)
				std::exit(1);
		}
		else {
			// result = 2026-01-29 19:36:42.903096367 +0000 /var/snap/core22/current
			bool ok = result.starts(timestr);
			if (ftw_verbose or not ok)
				std::printf("[%-6s] %d: %-40s → %s\n", ok ? "find" : "DIFFER", level, fpath, result.field("\n", 1).c_str());
			if (!ftw_ignore)
				std::exit(1);
		}
	}
}

bool timespec_gt_timespec(timespec lhs, timespec rhs) {
	return lhs.tv_sec > rhs.tv_sec || (lhs.tv_sec == rhs.tv_sec && lhs.tv_nsec > rhs.tv_nsec);
}

static int ftw_callback(const char* fpath,
					const struct stat* sb,
					int typeflag,
					struct FTW* ftwbuf)
{

//	// Skip /proc
//	if (strncmp(fpath, "/proc/", 6) == 0)
//		return 0;
//
//	// Skip /boot
//	if (strncmp(fpath, "/boot/", 6) == 0)
//		return 0;
//
//	// Skip /sys
//	if (strncmp(fpath, "/sys/", 5) == 0)
//		return 0;
//
//	// Skip /mnt
//	if (strncmp(fpath, "/mnt/", 5) == 0)
//		return 0;
//
//	// Skip /run
//	if (strncmp(fpath, "/run/", 5) == 0)
//		return 0;
//
	const int level = ftwbuf->level;
	if (level >= static_cast<int>(ftw_stack.size())) {
		ftw_stack.resize(level + 1, {0, 0});
	}

	// File updates its own stack level (content level)
	if (typeflag == FTW_F || typeflag == FTW_SL) {

		ftw_recursive_size += sb->st_size;

		const struct timespec file_mtime = sb->st_mtim;
		if (ftw_verbose > 2)
			log(level, typeflag == FTW_F ? "FTW_F" : "FTW_SL", fpath, file_mtime);

		if (timespec_gt_timespec(file_mtime, ftw_stack[level])) {
			ftw_stack[level] = file_mtime;
			if (ftw_verbose > 1)
				log(level, typeflag == FTW_F ? "BUMP F" : "BUMP S", fpath, file_mtime);
		}

		return 0;
	}

	// Dir at end of all its files (dir level = files level-1)
	// updates whole stack and maybe its own dir entry
	// It also zeros the files level read for the next subdir
	if (typeflag == FTW_DP) {

		const struct timespec file_mtime = sb->st_mtim;
		if (ftw_verbose > 1) {
			log(level, "FTW_DP", fpath, file_mtime);
		}

		timespec candidate = (size_t(level + 1) >= ftw_stack.size()) ? file_mtime : ftw_stack[level + 1];
		if (ftw_verbose > 1) {
			if (size_t(level + 1) >= ftw_stack.size()) {
				log(level, "STACK1", fpath, candidate);
			} else {
				log(level, "STACK", fpath, candidate);
			}
		}

		// Cheap pre-check with nftw's pre-order sb
		if (timespec_gt_timespec(candidate, file_mtime)) {
			struct stat st{};
			if (::stat(fpath, &st) == 0 &&
				timespec_gt_timespec(candidate, file_mtime)) {
				struct timespec times[2] = {
					{ st.st_atim.tv_sec, st.st_atim.tv_nsec },
					{ candidate.tv_sec, candidate.tv_nsec }
				};
				if (ftw_update) {
					// Updzte dir fs
					bool success = (utimensat(AT_FDCWD, fpath, times, 0) == 0);
					if (ftw_verbose)
						log(level, success ? "UPDATE" : "ERROR", fpath, candidate);
				}
				else {
//					if (ftw_verbose)
						log(level, "UPDREQ", fpath, candidate);

				}
			}
		} else {
				candidate = file_mtime;
		}
		// Propagate up one level in the stack
		// Actually all levels to be safe since
		// FTW_DP could conceivably be not called on the way up
		// through empty intermediary subdirs
		// e.g. Will FTW_DP be called for l1 here?
		// If not, then best to propagate l2a/b info up to l0
		//
		// l0
		// └── l1
		// ├── l2a
		// │   └── f1.txt
		// └── l2b
		// └── f2.txt
		for (int lev = 0; size_t(lev) < ftw_stack.size(); ++lev) {
			if (timespec_gt_timespec(candidate, ftw_stack[lev]))
				ftw_stack[lev] = candidate;
		}
		// Clear dir content level (+ everything below it to be safe)
		// so that entering a new subdir starts from scratch
		ftw_stack.resize(level + 1, {0, 0});
		return 0;
	}
	return 0;
}

