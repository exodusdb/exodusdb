#ifndef EXODUS_LIBEXODUS_EXODUS_VARB_H_
#define EXODUS_LIBEXODUS_EXODUS_VARB_H_

/*
Copyright (c) 2009 steve.bush@neosys.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// var_base for basic var-like functionality
// 1. int and fp arithmetic (+, -, *, /, %)
// 2. string concat (^)
// 3. iostream input/output

// clang-format off

import std;
//#include <iostream>
//#include <fstream>
#include <iomanip>
//#include <string>
//#include <string_view>

namespace exo {
	using SV = std::string_view;
}

//#if __has_include(<concepts>)
//#	include <concepts>
//#endif

#if __cpp_lib_concepts >= 201907L && ( __GNUC__ > 10 || __clang_major__ > 1 )
#	define EXO_CONCEPTS
#endif

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

// timebank profiling
//
// Capture function execution times and dump on program exit
// Use cmake -DEXODUS_TIMEBANK=1 to enable
#ifdef EXODUS_TIMEBANK
#	include "timebank.h"
#endif

// varerror
//
#define BACKTRACE_MAXADDRESSES 100
#include <exodus/varerr.h>

// vartype
//
#define xEXO_IMPORT import
#ifdef EXO_IMPORT
//	EXO_IMPORT vartyp;
	import vartyp;
#else
	#include <exodus/vartyp.h>
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"

#ifdef EXODUS_TIMEBANK
#	define THISIS(FUNC_DESC) [[maybe_unused]] static const char* function_sig = FUNC_DESC;Timer thisistimer(get_timeacno(FUNC_DESC));
#else
#	define THISIS(FUNC_DESC) [[maybe_unused]] static const char* function_sig = FUNC_DESC;
#endif

#define ISDEFINED(VARNAME) (VARNAME).assertDefined(function_sig, #VARNAME);
#define ISASSIGNED(VARNAME) (VARNAME).assertAssigned(function_sig, #VARNAME);
#define ISSTRING(VARNAME) (VARNAME).assertString(function_sig, #VARNAME);
#define ISNUMERIC(VARNAME) (VARNAME).assertNumeric(function_sig, #VARNAME);

#pragma clang diagnostic pop

// tracing ctor/dtor/assign/conversions
// Use cmake -DEXO_SNITCH=1 to enable
//
#ifdef EXO_SNITCH
#	undef EXO_SNITCH
//#include <iomanip>
#	define EXO_SNITCH(FUNC) std::clog << this << " " << FUNC << " " << var_typ << " " << std::setw(10) << var_int << " " << std::setw(10) << var_dbl << " '" << var_str << "' " << std::endl;
#	define EXO_SNITCHING
#else
#	define EXO_SNITCH(FUNC)
#endif

namespace exo {

#ifdef EXO_CONCEPTS
	template <typename T>
	concept std_string_or_convertible = std::is_convertible_v<T, std::string_view>;

	template <typename T>
	concept std_u32string_or_convertible = std::is_convertible_v<T, std::u32string_view>;
#endif

// floating point comparison
//
//#define SMALLEST_NUMBER 1e-13
//sum(1287.89,-1226.54,-61.35,1226.54,-1226.54) -> 0.000'000'000'000'23
//#define SMALLEST_NUMBER 1e-10
//#define SMALLEST_NUMBER 2.91E-11 (2^-35) or 0.000'000'000'029'1
//#define SMALLEST_NUMBER 1e-4d//0.0001 for pickos compatibility
//PUBLIC CONSTINIT_OR_CONSTEXPR double SMALLEST_NUMBER = 0.0001;// for pickos compatibility
inline const double SMALLEST_NUMBER = 0.0001;// for pickos compatibility

// Forward declarations

class var;
template<typename var> class var_mid; // forward declaration of a class template

// grep -P "VBR|CBR|TBR|RETVAR" -rl
#define VARBASE1   var_base<var_mid<exo::var>>

#define VBR1      var_base<var_mid<var>>&
#define CBR const var_base<var_mid<exo::var>>&
#define TBR       var_base<var_mid<exo::var>>&&

#define VBX       var_base&
#define CBX const var_base&
#define TBX       var_base&&

#define RETVAR    exo::var
#define RETVARREF exo::var&

// original help from Thinking in C++ Volume 1 Chapter 12
// http://www.camtp.uni-mb.si/books/Thinking-in-C++/TIC2Vone-distribution/html/Chapter12.html

// could also use http://www.informit.com/articles/article.asp?p=25264&seqNum=1
// which is effectively about makeing objects behave like ordinary variable syntactically
// implementing smartpointers

// the destructor is public non-virtual (supposedly to save space)
// since this class has no virtual functions
// IT SHOULD BE ABLE TO DERIVE FROM IT AND DO DELETE()
// http://www.parashift.com/c++-faq-lite/virtual-functions.html#faq-20.7

// on gcc and clang, size of var_base is as follows:
//
// var_str std::string: 32
// var_int int64_t:      8
// var_dbl double:       8
// var_typ uint:         4 (Only 5 bits are used. Presence of any other bits indicates use of an improperly constructed var_base)
//                     ---
// var_base:            52

using varint_t = std::int64_t;

// Hard coding to avoid including <limits>
// #define VAR_MAX_DOUBLE std::numeric_limits<varint_t>::max()
// #define VAR_LOW_DOUBLE std::numeric_limits<varint_t>::lowest()
//#define VAR_MAX_DOUBLE 1.797693134862315708145274237317043567981e+308
//#define VAR_LOW_DOUBLE -1.797693134862315708145274237317043567981e+308
inline const long double VAR_MAX_DOUBLE = static_cast<long double>( 1.797693134862315708145274237317043567981e+308);
inline const long double VAR_LOW_DOUBLE = static_cast<long double>(-1.797693134862315708145274237317043567981e+308);

/////////////////
// class var_base
/////////////////

// Converting a base object to a derived object (static cast of pointers/addresses)
//
//	BaseCRTP<DerivedCRTP> base;
//	DerivedCRTP* derived = static_cast<DerivedCRTP*>(&base);
//
// Note that this example assumes that the derived class has a compatible layout with the base class.
// If the derived class has additional members or a different layout, this approach may not work as expected.
// Also, be cautious when using static_cast like this, as it can lead to undefined behavior if the cast
// is not valid. In this case, we know that the cast is valid because BaseCRTP is the base class of DerivedCRTP.

// Note about var_base or var arguments and return values:
// var_base function arguments are always var_base  where possible since var IS A var_base so readily convertible
// but always return a var where possible so that var member functionality is readily available to function results

template<typename var>
class PUBLIC var_base {

	/////////////////////////
	// PROTECTED DATA MEMBERS
	/////////////////////////

 protected:

	using VAR    =       var_base;
	using VARREF =       var_base&;
	using CVR    = const var_base&;
	using TVR    =       var_base&&;

	// Understanding size of bytes, characters, integers and floating point
	// See https://en.cppreference.com/w/cpp/language/types

	// All mutable because asking a var for a string can cause it to create one internally from an integer or double and vice versa

	mutable std::string var_str;     // 32 bytes on g++. is default constructed to empty string
	mutable varint_t    var_int = 0; // 8 bytes/64 bits - currently defined as a long long
	mutable double      var_dbl = 0; // 8 bytes/64 buts - double
	mutable VARTYP      var_typ;     // Default initialised to VARTYP_UNA.
	                                 // Implemented as an unsigned int 1.e. 4 bytes

/////////////////////////////////////////////////////////////////////////////////////////////////
//                                SPECIAL MEMBER FUNCTIONS (SMF)
/////////////////////////////////////////////////////////////////////////////////////////////////

 public:

	//friend class var; // Does this actually do anything useful? var already inherits access to all var_base protected members
	friend class var_mid<var>; // Does this actually do anything useful? var already inherits access to all var_base protected members

	/////////////////////////
	// 1. Default constructor
	/////////////////////////
	//
	// 1. Allows syntax "var v;" to create an "unassigned" var (var_typ is 0)
	// 2. Constructors with a constexpr specifier make their type a LiteralType.
	// 3. Using default since members std::string and var_typ have constructors

#ifndef EXO_SNITCHING
	CONSTEXPR
	var_base() noexcept = default;

#else
	CONSTEXPR
	var_base()
		: var_typ(VARTYP_UNA) {
		//std::cout << "ctor()" << std::endl;

		// int xyz=3;
		// WARNING neither initialisers nor constructors are called in the following case !!!
		// var xxx=xxx.somefunction()
		// known as "undefined usage of uninitialised variable";
		// and not even a compiler warning in msvc8 or g++4.1.2

		// so the following test is put everywhere to protect against this type of accidental
		// programming if (var_typ&VARTYP_MASK) throw VarUndefined("funcname()"); should really
		// ensure a magic number and not just HOPE for some binary digits above bottom four 0-15
		// decimal 1111binary this could be removed in production code perhaps

		EXO_SNITCH("var_base +   ")
	}
#endif

	////////////////
	// 2. Destructor - We need to set var_typ to undefined for safety.
	////////////////
	//
	// 1. Merely sets var_typ to undefined for safety. So not very important. Could be omitted.
	// 2. Non-virtual destructor since we have no virtual functions
	// 3. Inline for speed but slows compilation unless optimising switched off

#define VAR_SAFE_DESTRUCTOR
#ifdef VAR_SAFE_DESTRUCTOR
	CONSTEXPR
	~var_base() {
		//std::cout << "dtor:" << var_str << std::endl;

		EXO_SNITCH("var_base -   ")

		// Try to ensure any memory is not later recognises as initialised memory
		//(exodus tries to detect undefined use of uninitialised objects at runtime - that dumb
		// compilers allow without warning) this could be removed in production code perhaps set all
		// unused bits to 1 to ease detection of usage of uninitialised variables (bad c++ syntax
		// like var x=x+1; set all used bits to 0 to increase chance of detecting unassigned
		// variables var_typ=(char)0xFFFFFFF0;
		var_typ = VARTYP_MASK;

	}

#else
	CONSTEXPR
	~var_base() = default;
#endif

	//////////////////////
	// 3. Copy constructor - Cant use default because we need to throw if rhs is unassigned
	//////////////////////
	//
	CONSTEXPR // but new std::string?
	var_base(CVR rhs)
		:
		var_str(rhs.var_str),
		var_int(rhs.var_int),
		var_dbl(rhs.var_dbl),
		var_typ(rhs.var_typ) {

		// use initializers for speed? and only check afterwards if rhs was assigned
		rhs.assertAssigned(__PRETTY_FUNCTION__);

		EXO_SNITCH("var_base COPY")
	}

	//////////////////////
	// 4. move constructor
	//////////////////////
	//
	// If noexcept then STL containers will use move during various operations otherwise they will use copy
	// We will use default since temporaries are unlikely to be undefined or unassigned and we will skip the usual checks

#ifndef SNITCHING
	CONSTEXPR
	var_base(TVR fromvar) noexcept = default;

#else
	CONSTEXPR
	var_base(TVR rhs) noexcept
		:
		var_str(std::move(rhs.var_str)),
		var_int(rhs.var_int),
		var_dbl(rhs.var_dbl),
		var_typ(rhs.var_typ) {

		// use initializers for speed? and only check afterwards if rhs was assigned
		//rhs.assertAssigned(__PRETTY_FUNCTION__);

		EXO_SNITCH("var_base MOVE")
	}
#endif

	/////////////////////
	// 5. copy assignment - from lvalue
	/////////////////////

	//var_base = var_base

	// Not using copy and replace idiom (copy assignment by value)
	// because Howard Hinnant recommends against in our case
	// We avoids a copy and make sure that we do the

	// Prevent assigning to temporaries with a clearer error message
	// e.g. printl(date() = 12345);
	//[[deprecated("Deprecated is a great way to highlight all uses of something which can otherwise be hard or slow to find!")]
	CONSTEXPR
	void operator=(CVR rhs) && = delete;

	// var_base& operator=(CVR rhs) & = default;
	// Cannot use default copy assignment because
	// a) it returns a value allowing accidental use of "=" instead of == in if statements
	// b) doesnt check if rhs is assigned

	CONSTEXPR // but new std::string?
	void operator=(CVR rhs) & {

		//assertDefined(__PRETTY_FUNCTION__);  //could be skipped for speed?
		rhs.assertAssigned(__PRETTY_FUNCTION__);

		// Prevent self assign
		// Removed for speed since we assume std::string handles it ok
		//if (this == &rhs)
		//	return;

		// copy everything across
		var_str = rhs.var_str;
		var_dbl = rhs.var_dbl;
		var_int = rhs.var_int;
		var_typ = rhs.var_typ;

		EXO_SNITCH("var_base =cop")
	}

	/////////////////////
	// 6. move assignment - from rvalue/temporary
	/////////////////////

	// var_base = temp var_base

	// Prevent assigning to temporaries
	// xyz.f(2) = "abc"; // Must not compile

	CONSTEXPR
	void operator=(TVR rhs) && noexcept = delete;

	// Cannot use the default move assignment because
	// a) It returns a value allowing accidental use of "=" in if statements instead of ==
	// b) It doesnt check if rhs is assigned although this is
	//    is less important for temporaries which are rarely unassigned.
	//var_base& operator=(TVR rhs) & noexcept = default;

	CONSTEXPR
	void operator=(TVR rhs) & noexcept {

		// Skipped for speed
		//assertDefined(__PRETTY_FUNCTION__);

		// Skipped for speed since temporaries are unlikely to be unassigned
		//rhs.assertAssigned(__PRETTY_FUNCTION__);

		//std::clog << "move assignment" <<std::endl;

		// Prevent self assign
		// Skipped for speed since we assume std::string handles it
		//if (this == &rhs)
		//	return;

		// *move* everything over
		var_str = std::move(rhs.var_str);
		var_dbl = rhs.var_dbl;
		var_int = rhs.var_int;
		var_typ = rhs.var_typ;

		EXO_SNITCH("var_base =mov")
	}

/////////////////////////////////////////////////////////////////////////////////////////////////
//                                CONSTRUCTORS
/////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////
	// INT CONSTRUCTORS
	///////////////////

	// var_base(integer)

#ifdef EXO_CONCEPTS
	template <std::integral Integer>
#else
	template <typename Integer, std::enable_if_t<std::is_integral<Integer>::value, bool> = true>
#endif
	/*
		bool

		char
		signed char
		unsigned char

		wchar_t
		char8_t (C++20)
		char16_t (C++11)
		char32_t (C++11)

		signed short
		signed int
		signed long
		signed long long (C++11)

		unsigned short
		unsigned int
		unsigned long
		unsigned long long (C++11)
	*/
	CONSTEXPR
	var_base(Integer rhs)
		:
		var_int(rhs),
		var_typ(VARTYP_INT) {

		// Prevent overlarge unsigned ints resulting in negative ints
		if (std::is_unsigned<Integer>::value) {
			if (this->var_int < 0)
				[[unlikely]]
				throw VarNumOverflow(var_base(__PRETTY_FUNCTION__)/*.field(";", 1)*/);
		}
		EXO_SNITCH("var_base Int ")
	}


	//////////////////////////////
	// FLOATING POINT CONSTRUCTORS
	//////////////////////////////

	// var_base = floating point

#ifdef EXO_CONCEPTS
	template <std::floating_point FloatingPoint>
#else
	template <typename FloatingPoint, std::enable_if_t<std::is_floating_point<FloatingPoint>::value, bool> = true>
#endif
	/*
		float,
		double,
		long double
	*/
	CONSTEXPR
	var_base(FloatingPoint rhs)
		:
		var_dbl(static_cast<double>(rhs)),
		var_typ(VARTYP_DBL) {

		// Prevent overlarge or overnegative long doubles entering var_base's double
		if (std::is_same<FloatingPoint, long double>::value) {
			if (rhs > VAR_MAX_DOUBLE)
				[[unlikely]]
				throw VarNumOverflow(var_base(__PRETTY_FUNCTION__)/*.field(";", 1)*/);
			if (rhs < VAR_LOW_DOUBLE)
				[[unlikely]]
				throw VarNumUnderflow(var_base(__PRETTY_FUNCTION__)/*.field(";", 1)*/);
		}
		EXO_SNITCH("var_base Fp  ")
	}

	//////////////////////
	// STRING CONSTRUCTORS
	//////////////////////

	// var_base = string-like

#ifdef EXO_CONCEPTS
	template <std_string_or_convertible StringLike>
#else
	template <typename StringLike, std::enable_if_t<std::is_convertible<StringLike, std::string_view>::value, bool> = true>
#endif
	/*
		// Accepts l and r values efficiently hopefully
		std::string,
		std::string_view,
		char*,
	*/
	CONSTEXPR // but new std::string?
	var_base(StringLike&& fromstr)
		:
		var_str(std::forward<StringLike>(fromstr)),
		var_typ(VARTYP_STR) {

		//std::cerr << "var_base(str)" << std::endl;
		EXO_SNITCH("var_base Str ")
	}

	// only for g++12 which cant constexpr from cstr
	// char*
	/////////
	CONSTEXPR
	var_base(const char* cstr1)
		:
		var_str(cstr1),
		var_typ(VARTYP_STR){
	}

	// memory block
	///////////////
	CONSTEXPR // but new std::string?
	var_base(const char* charstart, const std::size_t nchars)
		:
		var_typ(VARTYP_STR) {

		if (charstart)
			var_str = std::string_view(charstart, nchars);
		EXO_SNITCH("var_base c*+n")
	}

	// char
	///////
	//must be separate from unsigned int contructor to get a string/char not an int/ character number
	CONSTEXPR
	var_base(const char char1) noexcept
		:
		//var_str(1, char1), // not liked by what?
		var_typ(VARTYP_STR) {
		var_str = char1;
		//std::cerr << "var_base(char)" << std::endl;
		EXO_SNITCH("var_base char")
	}


	////////////////////////////////
	// STRING CONSTRUCTORS FROM WIDE
	////////////////////////////////

	// var_base(wide-string-like)

#ifdef EXO_CONCEPTS
	template <std_u32string_or_convertible U32S>
#else
	template <typename U32S, std::enable_if_t<std::is_convertible<U32S, std::u32string_view>::value, bool> = true>
#endif
	/*
		std::u32string,
		std::u32string_view,
		u32char*,
		u32char
	*/
	//CONSTEXPR
	var_base(U32S& from_wstr)
		:
		var_typ(VARTYP_STR)	{

		this->from_u32string(from_wstr);
		EXO_SNITCH("var_base U32S")
	}

	// const std::wstring&
	//////////////////////
	var_base(const std::wstring& wstr1);
//		EXO_SNITCH("var_base = mv")

	// wchar*
	/////////
	//CONSTEXPR
	var_base(const wchar_t* wcstr1)
		:
		var_str(var_base(std::wstring(wcstr1)).var_str),
		var_typ(VARTYP_STR){
		EXO_SNITCH("var_base wch*")
	}

// Cant provide these and support L'x' since it they are ambiguous with char when creating var_base from ints
//
//	// wchar
//  ////////
//	explicit var_base(wchar_t wchar1)
//		: var_typ(VARTYP_STR) {
//
//		(*this) = var_base(std::wstring(1, wchar1));
//		EXO_SNITCH("var_base wcha")
//	}
//
//	// char32_t char
//  ////////////////
//	var_base(char32_t u32char)
//		: var_typ(VARTYP_STR) {
//
//		this->from_u32string(std::u32string(1, u32char));
//		EXO_SNITCH("var_base u32c")
//	}
//
//	// char8_t char
//  ///////////////
//	var_base(char8_t u8char)
//		: var_typ(VARTYP_STR) {
//
//		this->var_str = std::string(1, u8char);
//		EXO_SNITCH("var_base u8ch")
//	}

//	// const std::u32string&
//  ////////////////////////
//	var_base(const std::u32string&& u32str1);
//	//	this->from_u32string(str1);

/////////////////////////////////////////////////////////////////////////////////////////////////
//                                  NAMED CONVERSION FUNCTIONS (CONVERT TO)
/////////////////////////////////////////////////////////////////////////////////////////////////

	bool toBool() const;

	char toChar() const;

	// standard c/c++ int() truncate towards zero i.e. take the number to the left of the point.
	// (whereas pickos is floor i.e. -1.9 -> 2)
	//int64_t toInt() const;
	int toInt() const;

	std::int64_t toInt64() const;

	double toDouble() const;

	// Be careful not to retain the pointer or reference beyond the scope of the var
	const char* c_str() const&;
	const std::string& toString() const&;

	// Temporaries can move their inner std::string
	std::string toString() &&;

	std::wstring to_wstring() const;
	std::u32string to_u32string() const;

	//void from_u32string(std::u32string) const;

/////////////////////////////////////////////////////////////////////////////////////////////////
//                               IMPLICIT CONVERSION OPERATORS (CONVERT TO)
/////////////////////////////////////////////////////////////////////////////////////////////////

	// someone recommends not to create more than one automatic converter
	// to avoid the compiler error "ambiguous conversion"

	// explicit keyword only works on conversion TO var not conversion FROM var

	// NB const possibly determines priority of choice between automatic conversion
	// and C++ uses bool and int conversion in certain cases to convert to int and bool
	// therefore have chosen to make both bool and int "const" since they dont make
	// any changes to the base object.

	// Implicitly convert var_base to var

//	// clone, like most var_base functions returns a var since var's are var_base's but not vice versa
//	CONSTEXPR
//	operator var() &;
//
//	CONSTEXPR
//	// move, like most var_base functions returns a var since var's are var_base's but not vice versa
//	operator var() &&;

//	// TODO Check if this provides any useful functionality
//	// but ensure that it is safe in case var acquires a different data layout from var_base
//	operator var&() & {
//		return static_cast<exo::var*>(this);
//	}

	// integer <- var
	/////////////////

	// necessary to allow conversion to int in many functions like extract(x,y,z)

#ifdef EXO_CONCEPTS
	template <std::integral Integer>
#else
	template <typename Integer, std::enable_if_t<std::is_integral<Integer>::value, bool> = false>
#endif
	/*
		bool

		char
		signed char
		unsigned char

		wchar_t
		char8_t (C++20)
		char16_t (C++11)
		char32_t (C++11)

		signed short
		signed int
		signed long
		signed long long (C++11)

		unsigned short
		unsigned int
		unsigned long
		unsigned long long (C++11)

	*/
	CONSTEXPR
	operator Integer() const {
		assertInteger(__PRETTY_FUNCTION__);

		// Prevent conversion of negative numbers to unsigned integers
		if (std::is_unsigned<Integer>::value) {
			if (this->var_int < 0)
				[[unlikely]]
				throw VarNonPositive(__PRETTY_FUNCTION__);
		}
		// Similar code in constructor(int) operator=(int) and int()

		EXO_SNITCH("var_base >Int")
		return static_cast<Integer>(var_int);
	}

	// floating point <- var
	////////////////////////

#ifdef EXO_CONCEPTS
	template <std::floating_point FloatingPoint>
#else
	template <typename FloatingPoint, std::enable_if_t<std::is_floating_point<FloatingPoint>::value, bool> = true>
#endif
	/*
		float,
		double,
		long double
	*/
	CONSTEXPR
	operator FloatingPoint() const {
		assertDecimal(__PRETTY_FUNCTION__);
		EXO_SNITCH("var_base >Fp ")
		return static_cast<FloatingPoint>(var_dbl);
	}

	// void* <- var
	///////////////

	// Recommended to provide automatic conversion to VOID POINTER instead of direct to bool
	// since void* is auto converted to bool nicely for natural "if (xyz)" syntax
	// and doesnt suffer the "ambiguous bool promotion to int" issue that automatic conversion
	// to bool has why this is working here now is probably because of non-portable compiler
	// characteristics or lack of source examples Note: The boost::shared_ptr class support an
	// implicit conversion to bool which is free from unintended conversion to arithmetic types.
	// http://www.informit.com/guides/content.aspx?g=cplusplus&seqNum=297
	// necessary to allow var to be used standalone in "if (xxx)" but see mv.h for discussion of
	// using void* instead of bool
	CONSTEXPR
	operator void*() const {
		// result in an attempt to convert an uninitialised object to void* since there is a bool
		// conversion when does c++ use automatic conversion to void* note that exodus operator !
		// uses (void*) trial elimination of operator void* seems to cause no problems but without
		// full regression testing
		EXO_SNITCH("var_base >vd*")
		return reinterpret_cast<void*>(this->toBool());
	}

	// bool <- var
	//////////////

	CONSTEXPR
	operator bool() const {
		EXO_SNITCH("var_base >boo")
		return this->toBool();
	}

	// Implemented in varnum after ::toBool to avoid
	// varnum.cpp:646:34: error: explicit specialization of 'toBool' after instantiation

	// NB && and || operators are NOT overloaded because var is set to convert to boolean
	// automatically this is fortunate because "shortcircuiting" of && and || doesnt happen when
	// overloaded

	// Perhaps should NOT allow automatic convertion to char* since it assumes a conversion to
	// utf8 and cannot hold char(0) perhaps and force people to use something like .utf8() or
	// .toString().c_char() This was added to allow the arguments of osread type functions which
	// need cstrings to be cstrings so that calls with fixed filenames etc dont require a
	// conversion to var and back again The other solution would be to declare a parallel set of
	// function with var arguments operator const char*() const;

	// convert to c_str may or may not be necessary in order to do wcout<<var
	// NB not needed because we overide the << operator itself
	// and causes a problem with unwanted conversions to strings
	// maybe should throw an error is 0x00 present in the string
	// allow the use of any cstring function
	// var::operator const char*();

	// string - replicates toString()
	// would allow the usage of any std::string function but may result
	// in a lot of compilation failures due to "ambiguous overload"
	// unfortunately there is no "explicit" keyword as for constructors - coming in C++0X
	// for now prevent this to ensure efficient programming
	//explicit

	// string-like <- var
	/////////////////////

//	template <typename StringLike, std::enable_if_t<std::is_convertible<StringLike, std::string_view>::value, bool> = true>
//	//enable_if can be replaced by a concept when available in g++ compiler (gcc v11?)
//	/*
//	// Accepts l and r values efficiently hopefully
//	std::string,
//	std::string_view,
//	char*,
//	*/
//	operator StringLike() {
//		assertString(__PRETTY_FUNCTION__);
//		return std::string_view(var_str);
//		//std::cerr << "var(str)" << std::endl;
//	}

	CONSTEXPR
	operator std::string() const {
		assertString(__PRETTY_FUNCTION__);
		EXO_SNITCH("var_base >Str")
		return var_str;
	}

	CONSTEXPR
	operator std::string_view() const {
		assertString(__PRETTY_FUNCTION__);
		EXO_SNITCH("var_base >SV ")
		return std::string_view(var_str);
	}

	CONSTEXPR
	operator const char*() const {
		assertString(__PRETTY_FUNCTION__);
		EXO_SNITCH("var_base >ch*")
		return var_str.c_str();
	}

	CONSTEXPR
	operator std::u32string() const {
		EXO_SNITCH("var_base >u32")
		return this->to_u32string();
	}


	// In case using a pointer to a var
	//
	// e.g. when passing io or out vars to functions as pointers instead of const ref
	//
	// var funcxyz(var* outvar)
	//
	// Prevent compilation of expressions containing
	//
	//  *outvar[4]
	//  *outvar.b(99)
	//
	// but allow
	//
	//  (*outvar)[4]
	//  outvar->f(99)
	//
	// Unfortunately there is no way to prevent
	//
	//  outvar[4] //compiles with undefined behaviour at runtime since there is only one var!
	//
	// Since c++ always allows treating any pointer as an array of pointers!
	//
	// but var objects use [] for substr access and super easy to get false array access instead
	//
	auto operator*() = delete;

	// allow conversion to char (takes first char or char 0 if zero length string)
	// would allow the usage of any char function but may result
	// in a lot of compilation failures due to "ambiguous overload"
	// unfortunately there is no "explicit" keyword as for constructors - coming in C++0X
	// operator char() const;

	// non-const xxxx(fn,vn,sn) returns a proxy that can be aasigned to or implicitly converted to a var
	//

/////////////////////////////////////////////////////////////////////////////////////////////////
//                                        ASSIGNMENT OPERATOR
/////////////////////////////////////////////////////////////////////////////////////////////////

	// The assignment operators return void to prevent accidental misuse where == was intended.

	// var = Integral
	/////////////////

#ifdef EXO_CONCEPTS
	template <std::integral Integer>
#else
	template <typename Integer, std::enable_if_t<std::is_integral<Integer>::value, bool> = true>
#endif
	/*
		bool

		char
		signed char
		unsigned char
		wchar_t
		char8_t (C++20)
		char16_t (C++11)
		char32_t (C++11)

		short
		unsigned short
		int
		unsigned int
		long
		unsigned long
		long long (C++11)
		unsigned long long (C++11)
		etc.
	*/
	CONSTEXPR
	void operator=(Integer rhs) & {

		// Similar code in constructor(int) operator=(int) and int()

		var_int = rhs;
		var_typ = VARTYP_INT;

		// Prevent overlarge unsigned ints resulting in negative ints
		if (std::is_unsigned<Integer>::value) {
			if (this->var_int < 0)
				[[unlikely]]
				throw VarNumOverflow(var_base(__PRETTY_FUNCTION__)/*.field(";", 1)*/);
		}

		EXO_SNITCH("var_base =Int")
	}

//	// Specialisation for bool to avoid a compiler warning
//	void operator=(bool rhs) & {
//TRACE2(__PRETTY_FUNCTION__)
//TRACE2(rhs)
//		var_int = rhs;
//		var_typ = VARTYP_INT;
//	}

	// var = Floating Point
	///////////////////////

#ifdef EXO_CONCEPTS
	template <std::floating_point FloatingPoint>
#else
	template <typename FloatingPoint, std::enable_if_t<std::is_floating_point<FloatingPoint>::value, bool> = true>
#endif
	/*
		float,
		double,
		long double
	*/
	CONSTEXPR
	void operator=(FloatingPoint rhs) & {
		var_dbl = rhs;
		var_typ = VARTYP_DBL;
		EXO_SNITCH("var_base =Fp ")
	}

	// var = char
	/////////////
	CONSTEXPR
	void operator=(const char char2) & {

		//THISIS("RETVARREF operator= (const char char2) &")
		// protect against unlikely syntax as follows:
		// var undefinedassign=undefinedassign=L'X';
		// this causes crash due to bad memory access due to setting string that doesnt exist
		// slows down all string settings so consider NOT CHECKING in production code
		//THISISDEFINED()	 // ALN:TODO: this definition kind of misleading, try to find
		// ALN:TODO: or change name to something like: THISISNOTDEAD :)
		// ALN:TODO: argumentation: var with mvtyp=0 is NOT defined

		var_str = char2;

		// reset to one unique type
		var_typ = VARTYP_STR;

		EXO_SNITCH("var_base =chr")
		return;
	}

	// var = char*
	//////////////

	// The assignment operator should always return a reference to *this.
	CONSTEXPR
	void operator=(const char* cstr) & {
		//THISIS("RETVARREF operator= (const char* cstr2) &")
		// protect against unlikely syntax as follows:
		// var undefinedassign=undefinedassign="xxx";
		// this causes crash due to bad memory access due to setting string that doesnt exist
		// slows down all string settings so consider NOT CHECKING in production code
		//THISISDEFINED()

		var_str = cstr;

		// reset to one unique type
		var_typ = VARTYP_STR;

		EXO_SNITCH("var_base =ch*")
		return;
	}

//	// var = std::string& - lvalue
//	//////////////////////////////
//
//	void operator=(const std::string& string2) & {
//
//		//THISIS("RETVARREF operator= (const std::string& string2) &")
//		// protect against unlikely syntax as follows:
//		// var undefinedassign=undefinedassign=std::string("xxx"";
//		// this causes crash due to bad memory access due to setting string that doesnt exist
//		// slows down all string settings so consider NOT CHECKING in production code
//		//THISISDEFINED()
//		var_str = string2;
//		var_typ = VARTYP_STR;  // reset to one unique type
//
//		return;
//	}

	// var = std::string&& - lvalue/rvalue perfect forwarding
	/////////////////////////////////////////////////////////

	// The assignment operator should always return a reference to *this.
	// but we do not in order to prevent misuse when == intended
	CONSTEXPR
	void operator=(std::string&& string2) & {

		//THISIS("RETVARREF operator= (const std::string&& string2) &")
		// protect against unlikely syntax as follows:
		// var undefinedassign=undefinedassign=std::string("xxx"";
		// this causes crash due to bad memory access due to setting string that doesnt exist
		// slows down all string settings so consider NOT CHECKING in production code
		//THISISDEFINED()

		var_str = std::forward<std::string>(string2);

		// reset to one unique type
		var_typ = VARTYP_STR;

		EXO_SNITCH("var_base =str")
		return;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////
//                                   SELF ASSIGN OPERATOR ON LVALUES
/////////////////////////////////////////////////////////////////////////////////////////////////

	VARREF operator+=(CVR) &;
	VARREF operator*=(CVR) &;
	VARREF operator-=(CVR) &;
	VARREF operator/=(CVR) &;
	VARREF operator%=(CVR) &;
	VARREF operator^=(CVR) &;

	// Specialisations for speed to avoid a) unnecessary converting to a var and then b) having to decide what type of var we have

	// Addvar_base(
	VARREF operator+=(const int) &;
	VARREF operator+=(const double) &;
	VARREF operator+=(const char  char1) & {(*this) += var_base(char1); return *this;}
	VARREF operator+=(const char* chars) & {(*this) += var_base(chars); return *this;}
	VARREF operator+=(const bool) &;

	// Multiply
	VARREF operator*=(const int) &;
	VARREF operator*=(const double) &;
	VARREF operator*=(const char  char1) & {(*this) *= var_base(char1); return *this;}
	VARREF operator*=(const char* chars) & {(*this) *= var_base(chars); return *this;}
	VARREF operator*=(const bool) &;

	// Subtract
	VARREF operator-=(const int) &;
	VARREF operator-=(const double) &;
	VARREF operator-=(const char  char1) & {(*this) -= var_base(char1); return *this;}
	VARREF operator-=(const char* chars) & {(*this) -= var_base(chars); return *this;}
	VARREF operator-=(const bool) &;

	// Divide
	VARREF operator/=(const int) &;
	VARREF operator/=(const double) &;
	VARREF operator/=(const char  char1) & {(*this) /= var_base(char1); return *this;}
	VARREF operator/=(const char* chars) & {(*this) /= var_base(chars); return *this;}
	VARREF operator/=(const bool) &;

	// Modulo
	VARREF operator%=(const int) &;
	VARREF operator%=(const double dbl1) &;
	VARREF operator%=(const char  char1) & {(*this) %= var_base(char1); return *this;}
	VARREF operator%=(const char* chars) & {(*this) %= var_base(chars); return *this;}
	VARREF operator%=(const bool  bool1) & {(*this) %= var_base(bool1); return *this;}

	// Concat
	VARREF operator^=(const int) &;
	VARREF operator^=(const double) &;
	VARREF operator^=(const char) &;

#define NOT_TEMPLATED_APPEND
#ifdef NOT_TEMPLATED_APPEND
	VARREF operator^=(const char*) &;
	VARREF operator^=(const std::string&) &;
	VARREF operator^=(SV) &;
#else
	// var = string-like
	template <typename Appendable, std::enable_if_t<
		std::is_same<Appendable, const char*>::value
		||
		std::is_same<Appendable, const std::string>::value
		||
		std::is_same<Appendable, const std::string&>::value
		||
		std::is_same<Appendable, const std::string_view>::value
	>>
	/*
		char*,
		std::string,
		std::string_view,
	*/
	VARREF operator^=(Appendable str2) & {
		assertString(__PRETTY_FUNCTION__);
		var_str += str2;
		var_typ = VARTYP_STR;  // must reset to one unique type
		return *this;
	}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////
//                          SELF ASSIGN OPERATOR ON TEMPORARIES - DEPRECATED
/////////////////////////////////////////////////////////////////////////////////////////////////

	// Prevent accidental, unusual and unnecessary coding

	#define DEPRECATE [[deprecated("Using self assign operators on temporaries is pointless. Use the operator by itself, without the = sign, to achieve the same.")]]

	DEPRECATE VARREF operator+=(CVR rhs) && {(*this) += rhs; return *this;}// = delete;
	DEPRECATE VARREF operator*=(CVR rhs) && {(*this) *= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator-=(CVR rhs) && {(*this) -= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator/=(CVR rhs) && {(*this) /= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator%=(CVR rhs) && {(*this) %= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator^=(CVR rhs) && {(*this) ^= rhs; return *this;}// = delete;

	// Specialisations are deprecated too

	// Add
	DEPRECATE VARREF operator+=(const int    rhs) && {(*this) += rhs; return *this;}// = delete;
	DEPRECATE VARREF operator+=(const double rhs) && {(*this) += rhs; return *this;}// = delete;
	DEPRECATE VARREF operator+=(const char   rhs) && {(*this) += rhs; return *this;}// = delete;
	DEPRECATE VARREF operator+=(const char*  rhs) && {(*this) += rhs; return *this;}// = delete;
	DEPRECATE VARREF operator+=(const bool   rhs) && {(*this) += rhs; return *this;}// = delete;

	// Multiply
	DEPRECATE VARREF operator*=(const int    rhs) && {(*this) *= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator*=(const double rhs) && {(*this) *= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator*=(const char   rhs) && {(*this) *= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator*=(const char*  rhs) && {(*this) *= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator*=(const bool   rhs) && {(*this) *= rhs; return *this;}// = delete;

	// Subtract
	DEPRECATE VARREF operator-=(const int    rhs) && {(*this) -= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator-=(const double rhs) && {(*this) -= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator-=(const char   rhs) && {(*this) -= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator-=(const char*  rhs) && {(*this) -= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator-=(const bool   rhs) && {(*this) -= rhs; return *this;}// = delete;

	// Divide
	DEPRECATE VARREF operator/=(const int    rhs) && {(*this) /= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator/=(const double rhs) && {(*this) /= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator/=(const char   rhs) && {(*this) /= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator/=(const char*  rhs) && {(*this) /= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator/=(const bool   rhs) && {(*this) /= rhs; return *this;}// = delete;

	// Modulo
	DEPRECATE VARREF operator%=(const int    rhs) && {(*this) %= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator%=(const double rhs) && {(*this) %= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator%=(const char   rhs) && {(*this) %= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator%=(const char*  rhs) && {(*this) %= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator%=(const bool   rhs) && {(*this) %= rhs; return *this;}// = delete;

	// Concat
	DEPRECATE VARREF operator^=(const int          rhs) && {(*this) ^= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator^=(const double       rhs) && {(*this) ^= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator^=(const char         rhs) && {(*this) ^= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator^=(const char*        rhs) && {(*this) ^= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator^=(const std::string& rhs) && {(*this) ^= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator^=(const std::string_view rhs) && {(*this) ^= rhs; return *this;}// = delete;

	#undef DEPRECATE

/////////////////////////////////////////////////////////////////////////////////////////////////
//                                      UNARY OPERATORS
/////////////////////////////////////////////////////////////////////////////////////////////////

	// Unary
	RETVAR operator+() const;
	RETVAR operator-() const;
	bool operator!() const {return !this->toBool();}

	// Postfix increment/decrement - only on lvalue because can use + 1 on temporaries and not risk wrong code e.g. x.f(1)++;
	RETVAR operator++(int) &;
	RETVAR operator--(int) &;

	// Prefix increment/decrement
	VARREF operator++() &;
	VARREF operator--() &;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"

/////////////////////////////////////////////////////////////////////////////////////////////////
//                                      BINARY OP FRIENDS
/////////////////////////////////////////////////////////////////////////////////////////////////

	// For detailed info on making templated value types operators work with free and templated function:
	// See CppCon 2018: Dan Saks “Making New Friends”
	//  https://www.youtube.com/watch?v=POa_V15je8Y
	// and
	// Back to Basics: Function and Class Templates - Dan Saks - CppCon 2019
	//  https://www.youtube.com/watch?v=LMP_sxOaz6g
	//
	// In var_base we use non-templated binary free functions as far as possible in order to exploit
	// C++ implicit automatic conversions which are more limited for template arguments.
	// although it does require a lot of overloads to be created to avoid ambiguous overloads
	//
	// Free tunction arguments allow three step conversions 1. Built in 2. Class defined 3. Built in
	// Templates dont support three step conversion.

	// Declared and defined free friend functions below
	//
	// Note that all or most of these do not actually need to be friends since they either utilise friended helpers
	// or are implemented in terms of self assign operators eg (a+b) is implemented as var(a)+=b
	//
	// 1. They could be defined outide of var class but we leave them in for clarity
	//
	// 2. The FRIEND word inside a class can be used to DEFINE free functions EVEN IF THEY DO NOT NEED FRIENDSHIP
	//
	// 3. FREE functions DEFINED in class scope can be found via ARGUMENT-DEPENDENT-LOOKUP
	//
	// 4. TEMPLATED functions do not benefit from 2, and 3. above
	//
	// 5. TEMPLATED functions do not benefit from automatic conversion to and from other types

	///////////////
	// FRIEND UTILS
	///////////////

	// Logical

	friend bool var_eq_var (      CBR     lhs,         CBR     rhs);
	friend bool var_lt_var (      CBR     lhs,         CBR     rhs);

	// Specialisations for speed

	friend bool var_eq_dbl (      CBR     lhs,   const double dbl1 );
	friend bool var_eq_int (      CBR     lhs,   const int    int1 );
	friend bool var_eq_bool(      CBR     lhs,   const bool   bool1);

	friend bool var_lt_int (      CBR     lhs,   const int    int1 );
	friend bool int_lt_var (const int    int1,        CBR     rhs  );

	friend bool var_lt_dbl (      CBR     lhs,   const double dbl1 );
	friend bool dbl_lt_var (const double dbl1,        CBR     rhs  );

//	friend bool var_lt_bool (      CBR    lhs,   const bool   bool1) = delete;
//	friend bool bool_lt_var (const bool  bool1,       CBR     rhs  ) = delete;

	// Concatenation

	friend RETVAR var_cat_var(       CBR   lhs,       CBR    rhs);

	// Specialisations for speed

	friend RETVAR var_cat_cstr(      CBR   lhs, const char* rhs);
	friend RETVAR var_cat_char(      CBR   lhs, const char  rhs);
	friend RETVAR cstr_cat_var(const char* lhs,       CBR   rhs);

	//friend var operator""_var(const char* cstr, std::size_t size);

	//////////////////////////////////////
	// Declare friendship within the class
	//////////////////////////////////////

	// and an edited version "varfriend_impl.h" which contains implementations is included in var.cpp
	// ENSURE all changes in varfriends.h and varfriends_impl.h are replicated using sed commands listed in the files
	// Implementations are in varfriends_impl.h included in var.cpp

#define VAR_FRIEND friend
#undef TBR
#define TBR var_base&&
#include "varfriends.h"
#undef TBR
#define TBR var_base<var_mid<var>>&&
//#include "varfriends_impl.h"

#pragma GCC diagnostic pop

	// UTILITY
	//////////

	// integer or floating point. optional prefix -, + disallowed, solitary . and - not allowed. Empty string is numeric 0
	//CONSTEXPR
	bool isnum() const;

/////////////////////////////////////////////////////////////////////////////////////////////////
//                                   [] or at
/////////////////////////////////////////////////////////////////////////////////////////////////

	// Coming in C++23 (already in gcc trunk) - multi-dimensional arrays with more than one index eg [x,y], [x,y,z] etc.
	// and left/right distinction for assign/extract? SADLY NO
	//
	// 2024 decided to deprecate usage of pickos-like brackets [] for text extraction and
	// not limit [] to accessing array like objects
	// e.g. dim which can be used on either side of the = (lvalue and rvalue)

	// Extract a character from a constant var
	// first=1 last=-1 etc.
	// DONT change deprecation wordng without also changing it in cli/fixdeprecated
	[[deprecated ("EXODUS: Replace single character accessors like xxx[n] with xxx.at(n)")]]
	ND RETVAR operator[](int pos1) const; //{return this->at(pos1);}

	// Named member function identical to operator[]
	//ND RETVAR at(const int pos1) const;
	ND RETVAR at(const int pos1) const;

	// as of now, sadly the following all works EXCEPT that var[99].anymethod() doesnt work
	// so would have to implement all var methods and free functions on the proxy object
	//ND var_brackets_proxy operator[](int pos1);
	//ND var operator[](int pos1) &&;


/////////////////////////////////////////////////////////////////////////////////////////////////
//                                         IOSTREAM FRIENDS
/////////////////////////////////////////////////////////////////////////////////////////////////

	// OSTREAM
	//////////

	// Causes ambiguous overload for some unknown reason despite being a hidden friend
	// friend std::ostream& operator<<(std::ostream& ostream1, TVR var1);

	// Note replicated for var but that one converts FM, VM etc. to visible chars before output
	PUBLIC friend std::ostream& operator<<(std::ostream& ostream1, CBR outvar) {
		outvar.assertString(__PRETTY_FUNCTION__);
		ostream1 << outvar.var_str;
		return ostream1;
	}

	// ISTREAM
	//////////

	PUBLIC friend std::istream& operator>>(std::istream& istream1, VARREF invar) {

		invar.assertDefined(__PRETTY_FUNCTION__);

		invar.var_str.clear();
		invar.var_typ = VARTYP_STR;

		//std::string tempstr;
		istream1 >> std::noskipws >> invar.var_str;

		// this would verify all input is valid utf8
		// in_str1.var_str=boost::locale::conv::utf_to_utf<char>(in_str1)

		return istream1;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////
//                                         PUBLIC MEMBER FUNCTIONS
/////////////////////////////////////////////////////////////////////////////////////////////////

	// Housekeeping functions
	/////////////////////////

	ND bool assigned() const;
	ND bool unassigned() const;
	void default_to(CVR defaultvalue);
	ND RETVAR default_from(CVR defaultvalue) const;

	VARREF move(VARREF destinationvar);

	// swap is marked as const despite it replaceping the var with var2
	// Currently this is required in rare cases where functions like exoprog::calculate
	// temporarily require member variables to be something else but switch back before exiting
	// if such function throws then it would leave the member variables in a changed state.
	CBR swap(CVR var2) const;//version that works on const vars
	VARREF swap(VARREF var2);//version that works on non-const vars

	// Create a var
	RETVAR clone() const;
	RETVAR move();

	// Text representation of var_base internals
	ND RETVAR dump() const;

	// basic string function on var_base for throwing errors
	ND RETVAR first_(int nchars) const;

	// Needed in operator%
	ND RETVAR mod(CVR divisor) const;
	ND RETVAR mod(double divisor) const;
	ND RETVAR mod(const int divisor) const;

	// Static member for speed on std strings because of underlying boost implementation
	// Unprotected currently because it is used in free function locateat
	// TODO verify if static function is thread safe
	static int localeAwareCompare(const std::string& str1, const std::string& str2);
	//int localeAwareCompare(const std::string& str2) const;


/////////////////////////////////////////////////////////////////////////////////////////////////
//                                       PROTECTED MEMBER FUNCTIONS
/////////////////////////////////////////////////////////////////////////////////////////////////
protected:

	// u32string utility

	void from_u32string(std::u32string u32str) const;

	// force var_str available or VNA

	//CONSTEXPR
	void createString() const;

	// assertDefined

	// WARNING: MUST NOT use any var when checking Undefined
	// OTHERWISE *WILL* get recursion/segfault
	CONSTEXPR
	void assertDefined(const char* message, const char* varname = "") const {
		if (var_typ & VARTYP_MASK)
			[[unlikely]]
			throw VarUndefined(std::string(varname) + " in " + message);
	}

	// assertAssigned

	CONSTEXPR
	void assertAssigned(const char* message, const char* varname = "") const {
		assertDefined(message, varname);
		if (!var_typ)
			[[unlikely]]
			throw VarUnassigned(var_base(varname) ^ " in " ^ message);
	}

	// assertNumeric

	//CONSTEXPR
	void assertNumeric(const char* message, const char* varname = "") const;

	// assertDecimal

	CONSTEXPR
	void assertDecimal(const char* message, const char* varname = "") const {
		assertNumeric(message, varname);
		if (!(var_typ & VARTYP_DBL)) {
			var_dbl = static_cast<double>(var_int);
			// Add double flag
			var_typ |= VARTYP_DBL;
		}
	}

	// assertInteger

	CONSTEXPR
	void assertInteger(const char* message, const char* varname = "") const {
		assertNumeric(message, varname);
		if (!(var_typ & VARTYP_INT)) {

			//var_int = std::floor(var_dbl);

			// Truncate double to int
			//var_int = std::trunc(var_dbl);
			if (var_dbl >= 0) {
				// 2.9 -> 2
				// 2.9999 -> 2
				// 2.99999 -> 3
				var_int = static_cast<varint_t>(var_dbl + SMALLEST_NUMBER / 10);
			} else {
				// -2.9 -> -2
				// -2.9999 -> -2.9
				// -2.99999 -> -3
				var_int = static_cast<varint_t>(var_dbl - SMALLEST_NUMBER / 10);
			}

			// Add int flag
			var_typ |= VARTYP_INT;
		}
	}

	// assertString

	CONSTEXPR
	void assertString(const char* message, const char* varname = "") const {
		assertDefined(message, varname);
		if (!(var_typ & VARTYP_STR)) {
			if (!var_typ)
				[[unlikely]]
				throw VarUnassigned(var_base(varname) ^ " in " ^ message);
			this->createString();
		}
	}

	// assertStringMutator

	CONSTEXPR
	void assertStringMutator(const char* message, const char* varname = "") const {
		assertString(message, varname);

		// VERY IMPORTANT:
		// If var_str is mutated then we MUST
		// reset all flags to ensure that the int/dbl, if needed,
		// are again lazily derived from the new string when required.
		var_typ = VARTYP_STR;
	}

}; // class var_base

////////////////////////////////
// var_base forward declarations
////////////////////////////////

// Forward declaration of some member functions to avoid errors like
// error: explicit specialization of 'toBool' after instantiation
template<> PUBLIC bool VARBASE1::toBool() const;
template<> PUBLIC void VARBASE1::createString() const;
template<> PUBLIC void VARBASE1::assertNumeric(const char* message, const char* varname/* = ""*/) const;

} // namespace exo

#endif // EXODUS_LIBEXODUS_EXODUS_VARB_H_
