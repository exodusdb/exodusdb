#ifndef EXO_VARDEFS_H
#define EXO_VARDEFS_H

#if EXO_MODULE > 1
	import std;
#else
#	include <iostream>
#endif

//#include <version> // for __cpp_consteval

/////////////
// EXO_FORMAT
/////////////

#ifndef EXO_FORMAT
#	if __has_include(<format>)
#		define EXO_FORMAT 1
#	elif __has_include(<fmt/args.h>)
#		define EXO_FORMAT 2
#	endif
#endif

/////////////
// EXO_SNITCH
/////////////

// Tracing ctor/dtor/assign/conversions
// Use cmake -DEXO_SNITCH=1 to enable or enable it here
//
//#define EXO_SNITCH
#ifdef EXO_SNITCH
#	undef EXO_SNITCH
#	define EXO_SNITCH(FUNC) std::clog << this << " " << FUNC << " " << var_typ << " " << std::setw(10) << var_int << " " << std::setw(10) << var_dbl << " '" << var_str << "' " << std::endl;
#	define EXO_SNITCHING
#else
#	define EXO_SNITCH(FUNC)
#endif

/////////////////
// EXODUS_RELEASE
/////////////////

// Info used in cli/compile
//
#define EXODUS_RELEASE "25.07"
#define EXODUS_PATCH "25.07.0"

///////////
// TO_CHARS
///////////

// 1. TO_CHARS from Ubuntu 22.04
// Used varnum.cpp, test_main3.cpp, test_precision2.cpp.
#if __GNUC__ >= 11 || __clang_major__ >=  14
#	define EXO_USE_TO_CHARS
// 2. RYU
#elif __has_include(<ryu/ryu.h>)
#	define EXO_USE_RYU
#	include <ryu/ryu.h>
// 3. STRINGSTREAM
#else
#endif

// Use ASCII 0x1A-0x1F for PickOS separator chars instead
// of PickOS 0xFA-0xFF which are illegal utf-8 bytes

// Also defined in pgexodus in extract.c etc.

// The var versions of the following (without leading or trailing _)
// are defined AFTER the class declaration of "var"

// Leading _ char* versions of classic pick delimiters
// Using macros to allow use of space as compile time concatenation operator
// e.g. _FM _VM will compile directly to "\x1F\x1E"

////////////
// BACKTRACE
////////////

#define BACKTRACE_MAXADDRESSES 100

/////////////
// DUMPDEFINE
/////////////

#define DUMPDEFINE(x) DUMPDEFINE0(x)
#define DUMPDEFINE0(x) #x

////////////////
// _RM, _NL etc.
////////////////

#define _RM "\x1F"  // Record Mark
#define _FM "\x1E"  // Field Mark
#define _VM "\x1D"  // Value Mark
#define _SM "\x1C"  // Subvalue Mark
#define _TM "\x1B"  // Text Mark
#define _STM "\x1A"  // Subtext Mark

#define _NL "\n"
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
#define STM_ '\x1A'     // Subtext Mark

#define NL_ '\n'
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
#define VISIBLE_STM_ '~'

#define _VISIBLE_RM "`"
#define _VISIBLE_FM "^"
#define _VISIBLE_VM "]"
#define _VISIBLE_SM "}"
#define _VISIBLE_TM "|"
#define _VISIBLE_STM "~"

#define _ALL_FMS _RM _FM _VM _SM _TM _STM

////////
// TRACE
////////

// Useful TRACE() function for debugging
#define TRACE(EXPRESSION) \
	try{ \
		var TRACE_expressionx {EXPRESSION}; \
		/* Must use errputl to convert all fms to visible fms*/ \
		TRACE_expressionx.quote().errputl("TRACE(" #EXPRESSION ")\t/" "/ "); \
	} catch (VarError e) { \
		/*var(e.stack()).errputl(e.message);*/ \
		std::cerr << var(e.message).field(":", 1) << ": " << var(e.stack()) << std::endl; \
	};

#define TRACE2(EXPRESSION) \
	std::cerr << (EXPRESSION) << std::endl;

//// string::view
////
//using SV = std::string_view;

//////////////////
// EXO_REGEX_BOOST
//////////////////

#define EXO_REGEX_BOOST

#pragma GCC diagnostic ignored "-Winline"

// Visibility
//
// If using g++ -fvisibility=hidden to make all hidden except those marked PUBLIC ie "default"
// "Weak" template functions seem to get excluded if visiblity is hidden, despite being marked as PUBLIC
// so we explictly instantiate them as non-template functions with "template<> ..." syntax.
// nm -C *so |&grep -F "exo::var_base<exo::var_stg<exo::var> >::"
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
#if __cpp_lib_constexpr_string >= 201907L && !defined(EXO_SNITCH)
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
//#	define THISIS(FUNC_DESC) [[maybe_unused]] static const char* function_sig = FUNC_DESC; \
#	define THISIS(FUNC_DESC) [[maybe_unused]] constexpr char function_sig[] = FUNC_DESC; \
exo::Timer thisistimer(exo::get_timebank_acno(FUNC_DESC));
#else
//#	define THISIS(FUNC_DESC) [[maybe_unused]] static const char* function_sig = FUNC_DESC;
#	define THISIS(FUNC_DESC) [[maybe_unused]] constexpr char function_sig[] = FUNC_DESC;
#endif

#define ISVAR(VARNAME)      (VARNAME).assertVar(function_sig, #VARNAME);
#define ISASSIGNED(VARNAME) (VARNAME).assertAssigned(function_sig, #VARNAME);
#define ISSTRING(VARNAME)   (VARNAME).assertString(function_sig, #VARNAME);
#define ISNUMERIC(VARNAME)  (VARNAME).assertNumeric(function_sig, #VARNAME);

//#define ISVAR(VARNAME)      (static_cast<const var_base&>(VARNAME)).assertVar(function_sig, #VARNAME);
//#define ISASSIGNED(VARNAME) (static_cast<const var_base&>(VARNAME)).assertAssigned(function_sig, #VARNAME);
//#define ISSTRING(VARNAME)   (static_cast<const var_base&>(VARNAME)).assertString(function_sig, #VARNAME);
//#define ISNUMERIC(VARNAME)  (static_cast<const var_base&>(VARNAME)).assertNumeric(function_sig, #VARNAME);

#pragma clang diagnostic pop

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

// Define a function pointer type for the replacement lambda
// using ReplaceFunction = std::string (*)(in matchinfo);
// using ReplaceFunction = var(*)(in matchinfo);
//
// Better to show the arguments to the function at point of use
// "SomeFunction_that_returns_a_var"
#define SomeFunction var(*sf)

// Standard utilities to double quote and escape especially c++ macros
#define EXO_STRINGIFY_IMPL(x)   #x
#define EXO_STRINGIFY(x)        EXO_STRINGIFY_IMPL(x)
#define EXO_CONCAT_IMPL(a,b)    a##b
#define EXO_CONCAT(a,b)         EXO_CONCAT_IMPL(a,b)

#endif //EXO_VARDEFS_H
