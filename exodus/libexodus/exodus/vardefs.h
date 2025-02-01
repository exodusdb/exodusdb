#ifndef EXO_VARDEFS_H
#define EXO_VARDEFS_H

//#include <version> // for __cpp_consteval

#define BACKTRACE_MAXADDRESSES 100

#define DUMPDEFINE(x) DUMPDEFINE0(x)
#define DUMPDEFINE0(x) #x

// Info used in cli/compile
//
#define EXODUS_RELEASE "24.07"
#define EXODUS_PATCH "24.07.0"

// Used in patch of /usr/include/c++/14/ostream to exclude std::format
// which is ambiguous with exo::format despite different namespace due to argument dependent lookup
// https://en.wikipedia.org/wiki/Argument-dependent_name_lookup
// Patched in CMakeLists.txt
//    execute_process(
//        COMMAND
//            sed -i "s|# include <format>|# ifndef EXO_FORMAT\\n#  include <format>\\n# endif|" /usr/include/c++/14/ostream
//        COMMAND
//            sed -i "s|#if __cpp_lib_print|#if !defined(EXO_FORMAT) \\&\\& __cpp_lib_print|" /usr/include/c++/14/ostream
//    )
// Probably converted to EXO_FORMAT 2 after inclusion of exodus/format.h
#ifndef EXO_FORMAT
#	define EXO_FORMAT 1
#endif

// Use ASCII 0x1A-0x1F for PickOS separator chars instead
// of PickOS 0xFA-0xFF which are illegal utf-8 bytes

// Also defined in pgexodus in extract.c etc.

// The var versions of the following (without leading or trailing _)
// are defined AFTER the class declaration of "var"

// Leading _ char* versions of classic pick delimiters
// Using macros to allow use of space as compile time concatenation operator
// e.g. _FM _VM will compile directly to "\x1F\x1E"

#define _RM "\x1F"  // Record Mark
#define _FM "\x1E"  // Field Mark
#define _VM "\x1D"  // Value Mark
#define _SM "\x1C"  // Subvalue Mark
#define _TM "\x1B"  // Text Mark
#define _ST "\x1A"  // Subtext Mark

#define _BS "\\"
#define _DQ "\""
#define _SQ "\'"

// trailing _ char versions of classic pick delimiters
//

#define RM_ '\x1F'     // Record Mark
#define FM_ '\x1E'     // Field Mark
#define VM_ '\x1D'     // Value Mark
#define SM_ '\x1C'     // Subvalue Mark
#define TM_ '\x1B'     // Text Mark
#define ST_ '\x1A'     // Subtext Mark

#define BS_ '\\'
#define DQ_ '\"'
#define SQ_ '\''

// Visible versions of FM, VM etc.
//
// printx() converts FM etc to these characters. user literal conversion _var also has them but hard coded in fmiconverter()
//#define _VISIBLE_FMS "_^]\[Z"  //PickOS standard. Backslash not good since it is often used for escaping chars. Z is normal letter.
//#define _VISIBLE_FMS "<[{}]>" //logical but hard to read direction of brackets quickly
//#define _VISIBLE_FMS "_^]}`~" //all uncommon in natural language. first 3 _^] are identical to pickos
//#define _VISIBLE_FMS "_^]}|~"   //all uncommon in natural language. first 3 _^] are identical to pickos
#define _VISIBLE_FMS "`^]}|~"   //all uncommon in natural language. ^] are identical to pickos. Using ` for RM since _ common in IT
#define VISIBLE_RM_ '`'
#define VISIBLE_FM_ '^'
#define VISIBLE_VM_ ']'
#define VISIBLE_SM_ '}'
#define VISIBLE_TM_ '|'
#define VISIBLE_ST_ '~'

#define _ALL_FMS _RM _FM _VM _SM _TM _ST

// Useful TRACE() function for debugging
#define TRACE(EXPRESSION) \
	try{ \
		var(EXPRESSION).convert(_ALL_FMS, _VISIBLE_FMS).quote().errputl("TRACE: " #EXPRESSION "; /" "/ "); \
	} catch (VarError e) { \
		var(e.stack()).errputl(e.description); \
	};

#define TRACE2(EXPRESSION) \
	std::cerr << (EXPRESSION) << std::endl;

// Readability for defaults
//
#define DEFAULT_UNASSIGNED = var()
#define DEFAULT_EMPTY = ""
#define DEFAULT_DOT = "."
#define DEFAULT_SPACE = " "
#define DEFAULT_VM = VM_
#define DEFAULT_NULL = nullptr

//// string::view
////
//using SV = std::string_view;

// [[nodiscard]]
//
#define ND [[nodiscard]]

// EXO_REGEX_BOOST
//
#define EXO_REGEX_BOOST

#pragma GCC diagnostic ignored "-Winline"

// Visibility
//
// If using g++ -fvisibility=hidden to make all hidden except those marked PUBLIC ie "default"
// "Weak" template functions seem to get excluded if visiblity is hidden, despite being marked as PUBLIC
// so we explictly instantiate them as non-template functions with "template<> ..." syntax.
// nm -C *so |&grep -F "exo::var_base<exo::var_mid<exo::var> >::"
// nm -D libexodus.so --demangle |grep T -w
#define PUBLIC __attribute__((visibility("default")))

// [[likely]] [[unlikely]]
//
#if __has_cpp_attribute(likely)
#	define LIKELY [[likely]]
#	define UNLIKELY [[unlikely]]
#else
#	define LIKELY
#	define UNLIKELY
#endif

// [[nodiscard]]
//
#define ND [[nodiscard]]

// constinit/consteval where possible otherwise constexpt
//
// constinit https://en.cppreference.com/w/cpp/language/constinit
//
// constinit - asserts that a variable has static initialization,
// i.e. zero initialization and constant initialization, otherwise the program is ill-formed.
//
// The constinit specifier declares a variable with static or thread storage duration.
// If a variable is declared with constinit, its initializing declaration must be applied with constinit.
// If a variable declared with constinit has dynamic initialization
// (even if it is performed as static initialization), the program is ill-formed.
// If no constinit declaration is reachable at the point of the initializing declaration,
// the program is ill-formed, no diagnostic required.
//
// constinit cannot be used together with constexpr.
// When the declared variable is a reference, constinit is equivalent to constexpr.
// When the declared variable is an object, constexpr mandates that the object must
// have static initialization and constant destruction and makes the object const-qualified,
// however, constinit does not mandate constant destruction and const-qualification.
// As a result, an object of a type which has constexpr constructors and no constexpr destructor
// (e.g. std::shared_ptr<T>) might be declared with constinit but not constexpr.

// Make var constinit/constexpr if std::string is constexpr (c++20 but g++-12 has some limitation)
//
#if __cpp_lib_constexpr_string >= 201907L
#	define CONSTEXPR constexpr
#	define CONSTINIT_OR_CONSTEXPR constinit const // const because constexpr implies const

#if ( __GNUC__  >= 13 ) || ( __clang_major__ > 1)
#		define CONSTINIT_VAR constinit
#	else
		// Ubuntu 22.04 g++12 doesnt support constinit var
#		define CONSTINIT_VAR
#	endif

#else
#	define CONSTEXPR
#	define CONSTINIT_OR_CONSTEXPR constexpr
#	define CONSTINIT_VAR
#endif

#if __cpp_consteval >= 201811L
#	define CONSTEVAL_OR_CONSTEXPR consteval
#else
#	define CONSTEVAL_OR_CONSTEXPR constexpr
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"

#ifdef EXO_TIMEBANK
#	define THISIS(FUNC_DESC) [[maybe_unused]] static const char* function_sig = FUNC_DESC;exo::Timer thisistimer(exo::get_timebank_acno(FUNC_DESC));
#else
#	define THISIS(FUNC_DESC) [[maybe_unused]] static const char* function_sig = FUNC_DESC;
#endif

#define ISDEFINED(VARNAME) (VARNAME).assertDefined(function_sig, #VARNAME);
#define ISASSIGNED(VARNAME) (VARNAME).assertAssigned(function_sig, #VARNAME);
#define ISSTRING(VARNAME) (VARNAME).assertString(function_sig, #VARNAME);
#define ISNUMERIC(VARNAME) (VARNAME).assertNumeric(function_sig, #VARNAME);

#pragma clang diagnostic pop

// Tracing ctor/dtor/assign/conversions
// Use cmake -DEXO_SNITCH=1 to enable
//
#ifdef EXO_SNITCH
#	undef EXO_SNITCH
#	define EXO_SNITCH(FUNC) std::clog << this << " " << FUNC << " " << var_typ << " " << std::setw(10) << var_int << " " << std::setw(10) << var_dbl << " '" << var_str << "' " << std::endl;
#	define EXO_SNITCHING
#else
#	define EXO_SNITCH(FUNC)
#endif

// Allow x.cutter().x.popper() ?
//#define EXO_ALLOW_CHAINING
#ifdef EXO_ALLOW_CHAINING
#	define IO io
#	define THIS *this
#	define IOSTRING iostring
#	define REF
#else
#	define IO void
#	define THIS
#	define IOSTRING
#	define REF &
#endif

#endif //EXO_VARDEFS_H
