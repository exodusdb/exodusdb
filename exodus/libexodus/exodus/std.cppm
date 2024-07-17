module;

#include <unistd.h>
#include "std-gmf.hpp"

#include "allstd.hpp"

//#include <concepts>
//#include <string>
//#include <vector>
//#include <algorithm>
//#include <mutex>
//#include <memory>
//#include <list>
//#include <codecvt>
//#include <chrono>
//#include <thread>
//
//#include <map>
//
//#include <fstream>
//#include <istream>
//#include <ostream>
//
//#include <random>
//
//#include <bitset>
//
//#include <filesystem>

export module std;

export namespace std {

//	using std::concepts;

	using std::string;
	using std::string_view;
	using ::random;
	using std::vector;
	using std::map;
	using std::erase_if;

	using std::mutex;
	using std::lock_guard;

	using std::unique_ptr;
	using std::make_unique;
	using std::shuffle;
	using std::reverse;
	using std::sort;
	using ::getpid;
	using std::getpid;
	using std::list;
	using std::bitset;
	using std::codecvt_base;

	using std::mt19937;
	using std::replace;

//	using std::codecvt_utf8;
	using std::codecvt;
	using std::codecvt_base;

	using std::thread;

	using std::uniform_int_distribution;

	using std::basic_fstream;
	using std::fstream;
	using std::ifstream;
	using std::ofstream;
	using std::istream;
	using std::ostream;

//	using std::filesystem;
	namespace filesystem {
		using std::filesystem::temp_directory_path;
		using std::filesystem::is_directory;
		using std::filesystem::path;
		using std::filesystem::current_path;
		using std::filesystem::absolute;
		using std::filesystem::create_directories;
		using std::filesystem::directory_iterator;
		using std::filesystem::exists;
		using std::filesystem::copy;
		using std::filesystem::copy_options;
		using std::filesystem::copy_options::overwrite_existing;
		using std::filesystem::copy_options::recursive;
		using std::filesystem::copy_options::copy_symlinks;
		using std::filesystem::remove_all;
		using std::filesystem::remove;
		using std::filesystem::read_symlink;
	}
	using std::ratio;
	using std::ratio_multiply;
	namespace chrono {
		using std::chrono::high_resolution_clock;
		using std::chrono::system_clock;
		using std::chrono::duration;
		using std::chrono::duration_cast;
		using std::chrono::hours;
		using std::chrono::minutes;
		using std::chrono::seconds;
		using std::chrono::milliseconds;
		using std::chrono::microseconds;
		using std::chrono::nanoseconds;
	}
	namespace this_thread {
		using std::this_thread::sleep_for;
	}
}
//export namespace std::filesystem;