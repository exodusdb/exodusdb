module;

#include <unistd.h>
#include "std-gmf.hpp"

#include "allstd.hpp"

export module std;

// Export POSIX functions into the global namespace
export using ::size_t;
export using ::getpid;
export using ::isatty;

export namespace std {

	using std::getline;
	using std::stack;
	using std::all_of;

//	using std::concepts;

//	using ::assert;

	using std::operator<<;
	using std::operator>>;

	using std::operator+;
	using std::operator-;
	using std::operator>=;
	using std::operator<=;
	using std::operator==;
	using std::operator!=;
	using std::operator<;
	using std::operator>;
	using std::operator<=>;

	using std::string;
	using std::u32string;
	using std::wstring;
	using std::string_view;
	using std::operator""sv;
	using std::u32string_view;
	using std::stoi;
	using std::stod;

	using std::remove_if;
	using std::find;
	using std::unique;
	using std::distance;
	using std::remainder;

	// <<functional>
//	using std::greater;
//	using std::greater_equal;
//	using std::less;
//	using std::less_equal;
//	using std::__equal_to;
//	using std::__not_equal_to;
//	using std::equal_to;
//	using std::not_equal_to;

	using std::plus;
	using std::minus;
	using std::multiplies;
	using std::divides;
	using std::modulus;
	using std::negate;

	using std::bad_alloc;

	// futile attempt at removing need for include iomanip in varb.h
	using std::normal_distribution;
	using std::allocator;
	using std::iterator;
	using std::basic_string;
	using std::char_traits;
	using std::system_error;
	using std::resetiosflags;
	using std::setiosflags;

	using std::setbase;
	using std::setfill;
	using std::dec;
	using std::hex;
	using std::oct;
	using std::fixed;
	using std::scientific;
	using std::hexfloat;
	using std::defaultfloat;
	using std::setprecision;
	using std::setw;

	using std::boolalpha;
	using std::noboolalpha;
	using std::showpoint;
	using std::noshowpoint;
	using std::showpos;
	using std::noshowpos;
	using std::showbase;
	using std::noshowbase;
	using std::uppercase;
	using std::nouppercase;
	using std::skipws;
	using std::noskipws;
	using std::unitbuf;
	using std::nounitbuf;
	using std::left;
	using std::right;
	using std::internal;
	using std::noskipws;

	using std::nextafter;
	using std::get_money;
	using std::put_money;
	using std::get_time;
	using std::put_time;
	using std::quoted;

	using std::ios;
	using std::ios_base;
	using std::basic_ios;

	using std::to_string;
	using std::to_chars;
	using std::from_chars;
	using std::chars_format;

	// really? since we are providing fmt::format
	// Causes "error: call to 'format' is ambiguous"
	// Should not cause problems since we are using in the std namespace ... but it does.
	//using std::format;

	using std::pair;
	using std::initializer_list;

	using std::min;
	using std::max;

	using std::iterator;
	using std::begin;
	using std::end;

	using std::cbegin;
	using std::cend;
	using std::rbegin;
	using std::rend;
	using std::crbegin;
	using std::crend;
	using std::size;
	using std::ssize;
	using std::empty;
	using std::data;

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

	using std::make_signed_t;

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
// Lots more - see https://en.cppreference.com/w/cpp/filesystem
	namespace filesystem {
		using std::filesystem::temp_directory_path;
		using std::filesystem::is_directory;
		using std::filesystem::path;
		using std::filesystem::current_path;
		using std::filesystem::absolute;
		using std::filesystem::create_directories;
		using std::filesystem::directory_iterator;
		using std::filesystem::exists;
		using std::filesystem::rename;
		using std::filesystem::copy;
		using std::filesystem::copy_options;
		using std::filesystem::copy_options::overwrite_existing;
		using std::filesystem::copy_options::recursive;
		using std::filesystem::copy_options::copy_symlinks;
		using std::filesystem::remove_all;
		using std::filesystem::remove;
		using std::filesystem::read_symlink;
		using std::filesystem::is_regular_file;
	} // namespace filesystem

	using std::ratio;
	using std::ratio_multiply;
	using std::common_type;
	using std::hash;

	namespace chrono {
		using std::chrono::clock_time_conversion;
		using std::chrono::sys_time;
		using std::chrono::high_resolution_clock;
		using std::chrono::system_clock;
		using std::chrono::time_point;
		using std::chrono::time_point_cast;
		using std::chrono::clock_cast;
		using std::chrono::duration;
		using std::chrono::treat_as_floating_point;
		using std::chrono::duration_values;
		using std::chrono::duration_cast;
//		using std::chrono::years;
//		using std::chrono::months;
//		using std::chrono::weeks;
//		using std::chrono::days;
		using std::chrono::hours;
		using std::chrono::minutes;
		using std::chrono::seconds;
		using std::chrono::milliseconds;
		using std::chrono::microseconds;
		using std::chrono::nanoseconds;

		using std::chrono::operator""h;   // hours
		using std::chrono::operator""min; // minutes
		using std::chrono::operator""s;   // sec
		using std::chrono::operator""ms;  // millisecs
		using std::chrono::operator""us;  // microsecs
		using std::chrono::operator""ns;  // nanosecs
		using std::chrono::operator""d;   // day of month
		using std::chrono::operator""y;   // year

	} // namespace chrono

	namespace this_thread {
		using std::this_thread::sleep_for;
	}

	// from <cstdint>
	using ::uint;
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
	using std::floor;
	using std::ceil;
	using std::round;

	using std::strcmp;
	using std::strlen;
	using std::strstr;

	using std::perror;
	using std::exit;
	using std::free;

	// <ctime>
	using std::time_t;

	// <cctype>
// Should not be using these since they are not fully threadsafe even for narrow chars
// Replaced by ASCII_isdigit ASCII_isalpha ASCII_toupper in exodus/ASCIIutil.h
//	using std::isdigit;
//	using std::isalpha;
//	using std::toupper;
//	using std::tolower;

	// <stdexcept>
	using std::invalid_argument;

	using std::getenv;

	using std::regex;
	using std::smatch;
	using std::regex_search;

//	using std::binary_function;
//	using std::_Select1st;
	
	// Allow three way access to string_literals
	// https://en.cppreference.com/w/cpp/string/basic_string/operator%22%22s
	inline namespace literals {
		using literals::operator""s;
		inline namespace string_literals {
			using string_literals::operator""s;
		}
	}
	inline namespace string_literals {
		using string_literals::operator""s;
	}

//using std::ExoEnv;
//using std::ExodusProgram;
//using std::ExoProgram;
//using std::Init;
using std::align_val_t;
using std::allocator;
using std::back_insert_iterator;
using std::bad_alloc;
using std::bad_array_new_length;
using std::bad_cast;
using std::bad_exception;
using std::bad_typeid;
using std::basic_const_iterator;
using std::basic_filebuf;
using std::basic_fstream;
using std::basic_ifstream;
using std::basic_ios;
using std::basic_iostream;
using std::basic_ispanstream;
using std::basic_istream;
using std::basic_istringstream;
using std::basic_ofstream;
using std::basic_ospanstream;
using std::basic_ostream;
using std::basic_ostringstream;
using std::basic_osyncstream;
using std::basic_spanbuf;
using std::basic_spanstream;
using std::basic_streambuf;
using std::basic_string;
using std::basic_string_view;
using std::basic_stringbuf;
using std::basic_stringstream;
using std::basic_syncbuf;
using std::binder1st;
using std::binder2nd;
using std::byte;
using std::codecvt;
using std::codecvt_base;
using std::codecvt_byname;
using std::collate;
using std::collate_byname;
using std::common_iterator;
using std::const_mem_fun1_ref_t;
using std::const_mem_fun1_t;
using std::const_mem_fun_ref_t;
using std::const_mem_fun_t;
using std::counted_iterator;
using std::ctype;
using std::ctype_byname;
using std::domain_error;
using std::endian;
using std::errc;
using std::error_category;
using std::error_code;
using std::error_condition;
using std::exception;
using std::exception_ptr;
//using std::facet;
using std::fpos;
using std::front_insert_iterator;
//using std::id;
using std::initializer_list;
using std::insert_iterator;
using std::invalid_argument;
using std::io_errc;
using std::ios_base;
using std::istream_iterator;
using std::istreambuf_iterator;
using std::length_error;
using std::locale;
using std::logic_error;
using std::mem_fun1_ref_t;
using std::mem_fun1_t;
using std::mem_fun_ref_t;
using std::mem_fun_t;
//using std::memory_resource;
using std::messages;
using std::messages_byname;
using std::money_base;
using std::money_get;
using std::money_put;
using std::moneypunct;
using std::moneypunct_byname;
using std::move_iterator;
using std::move_sentinel;
using std::nested_exception;
using std::num_get;
using std::num_put;
using std::numpunct;
using std::numpunct_byname;
//using std::or;
using std::ostream_iterator;
using std::ostreambuf_iterator;
using std::out_of_range;
using std::overflow_error;
using std::partial_ordering;
using std::pointer_to_binary_function;
using std::pointer_to_unary_function;
//using std::polymorphic_allocator;
using std::range_error;
using std::reference_wrapper;
using std::reverse_iterator;
using std::runtime_error;
//using std::std;
using std::strong_ordering;
//using std::subrange;
//using std::subrange_kind;
using std::system_error;
using std::time_base;
using std::time_get;
using std::time_get_byname;
using std::time_put;
using std::time_put_byname;
using std::tuple;
using std::type_info;
using std::underflow_error;
using std::valarray;
//using std::view_interface;
using std::weak_ordering;
using std::add_const;
using std::add_cv;
using std::add_lvalue_reference;
using std::add_pointer;
using std::add_rvalue_reference;
using std::add_volatile;
using std::alignment_of;
using std::allocator_arg_t;
using std::allocator_traits;
using std::array;
using std::basic_common_reference;
using std::bidirectional_iterator_tag;
using std::binary_function;
using std::bit_and;
using std::bit_not;
using std::bit_or;
using std::bit_xor;
using std::char_traits;
using std::common_comparison_category;
using std::common_reference;
using std::common_type;
using std::compare_three_way;
using std::compare_three_way_result;
using std::conditional;
using std::conjunction;
using std::contiguous_iterator_tag;
using std::ctype_base;
//using std::dangling;
using std::decay;
using std::default_sentinel_t;
using std::destroying_delete_t;
using std::disjunction;
using std::divides;
//using std::drand48_data;
using std::enable_if;
using std::equal_to;
using std::extent;
using std::forward_iterator_tag;
using std::greater;
using std::greater_equal;
using std::has_unique_object_representations;
using std::has_virtual_destructor;
using std::hash;
using std::identity;
//using std::in_in_result;
using std::in_place_index_t;
using std::in_place_t;
using std::in_place_type_t;
using std::incrementable_traits;
using std::indirectly_readable_traits;
using std::input_iterator_tag;
using std::integer_sequence;
using std::integral_constant;
using std::invoke_result;
using std::is_abstract;
using std::is_aggregate;
using std::is_arithmetic;
using std::is_array;
using std::is_assignable;
using std::is_base_of;
using std::is_bounded_array;
using std::is_class;
using std::is_compound;
using std::is_const;
using std::is_constructible;
using std::is_convertible;
using std::is_copy_assignable;
using std::is_copy_constructible;
using std::is_default_constructible;
using std::is_destructible;
using std::is_empty;
using std::is_enum;
using std::is_error_code_enum;
using std::is_error_condition_enum;
using std::is_final;
using std::is_floating_point;
using std::is_function;
using std::is_fundamental;
using std::is_integral;
using std::is_invocable;
using std::is_invocable_r;
using std::is_lvalue_reference;
using std::is_member_function_pointer;
using std::is_member_object_pointer;
using std::is_member_pointer;
using std::is_move_assignable;
using std::is_move_constructible;
using std::is_nothrow_assignable;
using std::is_nothrow_constructible;
using std::is_nothrow_convertible;
using std::is_nothrow_copy_assignable;
using std::is_nothrow_copy_constructible;
using std::is_nothrow_default_constructible;
using std::is_nothrow_destructible;
using std::is_nothrow_invocable;
using std::is_nothrow_invocable_r;
using std::is_nothrow_move_assignable;
using std::is_nothrow_move_constructible;
using std::is_nothrow_swappable;
using std::is_nothrow_swappable_with;
using std::is_null_pointer;
using std::is_object;
using std::is_pointer;
using std::is_polymorphic;
using std::is_reference;
using std::is_rvalue_reference;
using std::is_same;
using std::is_scalar;
using std::is_scoped_enum;
using std::is_signed;
using std::is_standard_layout;
using std::is_swappable;
using std::is_swappable_with;
using std::is_trivial;
using std::is_trivially_assignable;
using std::is_trivially_constructible;
using std::is_trivially_copy_assignable;
using std::is_trivially_copy_constructible;
using std::is_trivially_copyable;
using std::is_trivially_default_constructible;
using std::is_trivially_destructible;
using std::is_trivially_move_assignable;
using std::is_trivially_move_constructible;
using std::is_unbounded_array;
using std::is_union;
using std::is_unsigned;
using std::is_void;
using std::is_volatile;
using std::iterator_traits;
//using std::itimerspec;
using std::lconv;
using std::less;
using std::less_equal;
using std::logical_and;
using std::logical_not;
using std::logical_or;
using std::make_signed;
using std::make_unsigned;
using std::messages_base;
using std::minus;
using std::modulus;
using std::multiplies;
using std::negate;
using std::negation;
using std::not_equal_to;
using std::nothrow_t;
using std::numeric_limits;
//using std::obstack;
using std::output_iterator_tag;
using std::pair;
using std::piecewise_construct_t;
using std::plus;
using std::pointer_traits;
using std::random_access_iterator_tag;
//using std::random_data;
using std::rank;
//using std::rebind;
using std::remove_all_extents;
using std::remove_const;
using std::remove_cv;
using std::remove_cvref;
using std::remove_extent;
using std::remove_pointer;
using std::remove_reference;
using std::remove_volatile;
using std::result_of;
//using std::sched_param;
//using std::sigevent;
using std::timespec;
//using std::timeval;
//using std::timex;
using std::tm;
using std::tuple_element;
using std::tuple_size;
using std::type_identity;
using std::unary_function;
using std::underlying_type;
using std::unreachable_sentinel_t;
using std::unwrap_ref_decay;
using std::unwrap_reference;
using std::uses_allocator;
//using std::view_base;
//using std::views;

} // namespace std


