module;

#include <unistd.h>
#include "std-gmf.hpp"

#include "allstd.hpp"

//#include <concepts>
//#include <string>
//#include <vector>
//#include <array>
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

	using std::operator+;
//	using std::operator-;
	using std::operator<<;
	using std::operator>>;
//	using std::operator>=;
//	using std::operator<=;
	using std::operator==;
//	using std::operator!=;
//	using std::operator<;
//	using std::operator>;

	using std::string;
	using std::u32string;
	using std::wstring;
	using std::string_view;
	using std::u32string_view;

	// <<functional>
	using std::greater;
	using std::greater_equal;
	using std::less;
	using std::less_equal;
	using std::__equal_to;
	using std::__not_equal_to;

	using std::bad_alloc;

	// futile attempt at removing need for include iomanip in varb.h
	using std::normal_distribution;
	using std::allocator;
	using std::iterator;
	using std::__cxx11::basic_string;
	using std::basic_string;
	using std::char_traits;
	using std::system_error;
	using std::resetiosflags;
	using std::setiosflags;
	using std::setbase;
	using std::setfill;
	using std::setprecision;
	using std::setw;
	using std::get_money;
	using std::put_money;
	using std::get_time;
	using std::put_time;
	using std::quoted;

	using std::to_string;
	using std::to_chars;
	using std::from_chars;
	using std::chars_format;

	using std::pair;
	using std::initializer_list;

	using std::min;
	using std::max;

	using std::iterator;
	using std::begin;
	using std::end;

	using std::swap;
	using std::move;
	using std::forward;
	using std::remove_reference;

	using ::random;

	using std::vector;
	using std::array;
	using std::map;
	using std::set;
//	using std::unordered_map;

	using std::erase_if;

	using std::enable_if_t;
	using std::integral;
	using std::floating_point;
	using std::is_integral;
	using std::is_floating_point;
	using std::is_convertible;
	using std::is_convertible_v;
	using std::is_signed;
	using std::is_unsigned;
	using std::is_same;

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

	using std::cin;
	using std::cout;
	using std::cerr;
	using std::clog;

	using std::basic_fstream;
	using std::fstream;
	using std::ifstream;
	using std::ofstream;
	using std::istream;
	using std::ostream;

	using std::istringstream;
	using std::ostringstream;
	using std::stringstream;

	using std::noskipws;
	using std::hex;
	using std::uppercase;
	using std::endl;
	using std::flush;

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

	// from <cstdint>
	using std::size_t;
	using std::int8_t;
	using std::int16_t;
	using std::int32_t;
	using std::int64_t;
	using std::uint8_t;
	using std::uint16_t;
	using std::uint32_t;
	using std::uint64_t;

	// from <stdlib>?
	using ::isatty;
	using std::isatty;

	using std::numeric_limits;

	//cmath
	using std::abs;
	using std::fmod;
	using std::pow;
	using std::exp;
	using std::sqrt;
	using std::log;
	using std::sin;
	using std::cos;
	using std::tan;
	using std::atan;

	using std::strcmp;
	using std::strlen;
	using std::strstr;

	using std::perror;
	using std::exit;
	using std::free;

	// <ctime>
	using std::time_t;

	// <cctype>
	using std::isdigit;
	using std::isalpha;
	using std::toupper;
	using std::tolower;

	// <stdexcept>
	using std::invalid_argument;

	using std::getenv;

//	using std::binary_function;
//	using std::_Select1st;

} //export namespace std::filesystem;