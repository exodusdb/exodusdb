#ifndef LIBEXODUS_VARB_H_
#define LIBEXODUS_VARB_H_

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

// gendoc: var - Base

// var_base for basic var-like functionality
// 1. int and fp arithmetic (+, -, *, /, %)
// 2. string concat (^)
// 3. iostream input/output

// clang-format off

#if EXO_MODULE > 1
	import std;
#else
#	include <cstdint> // for std::int64_t etc.
#	include <iostream>
#	include <fstream>
#	include <string>
#	include <string_view>
#	include <iomanip>
#	if __has_include(<concepts>)
#		include <concepts>
#	endif
#endif

// Verify __cpp_ macros are available from #include <version>
#ifndef  __cpp_constexpr
#	error "#include <version>" is missing
#endif

// EXO_CONCEPTS
//
#if __cpp_lib_concepts >= 201907L && ( __GNUC__ > 10 || __clang_major__ > 1 )
#	define EXO_CONCEPTS
#endif

namespace exo {
	using SV = std::string_view;
}

#include <var/vardefs.h>

// timebank profiling
//
// Capture function execution times and dump on program exit
// Use cmake -DEXO_TIMEBANK=1 to enable
#ifdef EXO_TIMEBANK
#	include "timebank.h"
#endif

// varerror
//
#define BACKTRACE_MAXADDRESSES 100
#include <var/varerr.h>

// vartype
//
#include <var/vartyp.h>

namespace exo {

#ifdef EXO_CONCEPTS
	template <typename T>
	concept std_string_or_convertible = std::is_convertible_v<T, std::string_view>;

	template <typename T>
	concept std_u32string_or_convertible = std::is_convertible_v<T, std::u32string_view>;
#endif

// Maximum number of post decimal place digits to consider when comparing
// floating point numbers or converting them to strings
#ifndef EXO_VARB_CPP
	extern thread_local int    EXO_PRECISION;
	extern thread_local double EXO_SMALLEST_NUMBER;
#else
	thread_local int    EXO_PRECISION = 4;// for pickos compatibility
	thread_local double EXO_SMALLEST_NUMBER = 0.0001;// for pickos compatibility
#endif

// Forward declarations

class var;
class var_stg;
class var_os;
class var_db;
//class var_stg; // forward declaration of a class template
class var_iter;

#define VB1         var_base
#define VBR1        var_base&
#define CBR1  const var_base&
#define TBR1        var_base&&

#define VBR         var_base&
#define VARBASEREF  var_base&
#define CBR   const var_base&
#define TBR         var_base&&

#define RETVAR      exo::var
#define RETVARREF   exo::var&

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

class PUBLIC var_base {

	/////////////////////////
	// PROTECTED DATA MEMBERS
	/////////////////////////

 protected:

//	using VAR    =       var_base;
//	using VARREF =       var_base&;
//	using VBR    =       var_base&;
//	using CBR    = const var_base&;
//	using TBR    =       var_base&&;

	using in     = const var_base&;
	using out    =       var_base&;
	using io     =       var_base&;

	// Understanding size of bytes, characters, integers and floating point
	// See https://en.cppreference.com/w/cpp/language/types

	// All mutable because asking a var for a string can cause it to create one internally from an integer or double and vice versa

	mutable std::string var_str;     // 32 bytes on g++. is default constructed to empty string
	mutable varint_t    var_int = 0; // 8 bytes/64 bits - currently defined as a long long
	mutable double      var_dbl = 0; // 8 bytes/64 buts - double
	mutable VARTYP      var_typ;     // Default initialised to VARTYP_UNA.
	                                 // Implemented as an unsigned int 1.e. 4 bytes

	friend class var;
	friend class var_stg;
	friend class var_os;
	friend class var_db;
	friend class var_iter;
	friend class dim;
//	friend class rex;

/////////////////////////////////////////////////////////////////////////////////////////////////
//                                SPECIAL MEMBER FUNCTIONS (SMF)
/////////////////////////////////////////////////////////////////////////////////////////////////

 public:

	// Define all ctor/assign/conversions inline to ensure optimisation can remove redundant assembler.
	// This is important for value types like var_base.

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
		// programming if (var_typ&VARTYP_MASK) throw VarUnconstructed("funcname()"); should really
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
	var_base(CBR rhs)
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
	var_base(TBR fromvar) noexcept = default;

#else
	CONSTEXPR
	var_base(TBR rhs) noexcept
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
	void operator=(CBR rhs) && = delete;

	// var_base& operator=(CBR rhs) & = default;
	// Cannot use default copy assignment because
	// a) it returns a value allowing accidental use of "=" instead of == in if statements
	// b) doesnt check if rhs is assigned

	CONSTEXPR // but new std::string?
	void operator=(CBR rhs) & {

		//assertVar(__PRETTY_FUNCTION__);  //could be skipped for speed?
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

	// var_base = var_base&&

	// Prevent assigning to temporaries
	// xyz.f(2) = "abc"; // Must not compile

	CONSTEXPR
	void operator=(TBR rhs) && noexcept = delete;

	// Cannot use the default move assignment because
	// a) It returns a value allowing accidental use of "=" in if statements instead of ==
	// b) It doesnt check if rhs is assigned although this is
	//    is less important for temporaries which are rarely unassigned.
	//var_base& operator=(TBR rhs) & noexcept = default;

	CONSTEXPR
	void operator=(TBR rhs) & noexcept {

		// Skipped for speed
		//assertVar(__PRETTY_FUNCTION__);

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

		// std::move on an lvalue to mark the lvalue as unassigned?
//		rhs.var_typ = VARTYP_UNA;

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
//		var_int(rhs),
		var_int(static_cast<std::make_signed_t<varint_t>>(rhs)),
		var_typ(VARTYP_INT) {

		// Prevent overlarge unsigned ints resulting in negative ints
		// Reject at compiletime or runtime
		// Only check if either a) the incoming int is same size and unsigned, or b) it is greater in size
		// Note: Since varint_t is long long int (8 bytes),
		// we are really only protecting against *unsigned* long long ints > varint_t max
		// unless int128 appears
		if ((std::is_unsigned<Integer>::value && sizeof(rhs) >= sizeof(varint_t)) || sizeof(rhs) > sizeof(varint_t)) {
			if (this->var_int < 0) {
				[[unlikely]]
				//throw VarNumOverflow(var_base(__PRETTY_FUNCTION__)/*.field(";", 1)*/);
				throw VarNumOverflow(
					" In "
					+ std::string(__PRETTY_FUNCTION__)
					+ " int '"
					+ std::to_string(rhs)
					+ "' but var range is " + std::to_string(std::numeric_limits<varint_t>::min())
					+ " to +" + std::to_string(std::numeric_limits<varint_t>::max())
				);
			}
		}

//		if ((!std::is_signed<Integer>() && sizeof(rhs) >= sizeof(varint_t)) || sizeof(rhs) > sizeof(varint_t)) {
//			// Check for high bit or any higher bit set
//			if (rhs & ~static_cast<Integer>(0x7FFFFFFFFFFFFFFFLL)) {
////			if (rhs & ~std::numeric_limits<varint_t>::max()) {
////			if (rhs & ~std::make_signed_t<varint_t>(std::numeric_limits<varint_t>::max())) {
////			if (rhs & ~static_cast<Integer>(std::numeric_limits<varint_t>::max())) {
//				UNLIKELY
//				throw VarNumOverflow(
//					" In "
//					+ std::string(__PRETTY_FUNCTION__)
//					+ " int '"
//					+ std::to_string(rhs)
//					+ "' but var range is " + std::to_string(std::numeric_limits<varint_t>::min())
//					+ " to +" + std::to_string(std::numeric_limits<varint_t>::max())
//				);
//			}
//		}
//
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

	auto toBool()       const   -> bool;

	auto toChar()       const   -> char;

	// Standard c/c++ int() truncate towards zero.
	// i.e. take the number to the left of the point.
	// Whereas pickos is floor i.e. -1.9 -> 2.
	auto toInt()        const   -> int;

	auto toInt64()      const   -> std::int64_t;
	auto toSize()       const   -> std::size_t;

	auto toDouble()     const   -> double;

	auto c_str()        const&  -> const char*;       // DONT retain beyond scope of the var
	auto toString()     const&  -> const std::string&;
	auto toString()          && -> std::string;       // Temporaries can move their inner string

	auto to_wstring()   const   -> std::wstring;
	auto to_u32string() const   -> std::u32string;

	//void from_u32string(std::u32string) const;

/////////////////////////////////////////////////////////////////////////////////////////////////
//                               IMPLICIT CONVERSION OPERATORS (CONVERT TO)
/////////////////////////////////////////////////////////////////////////////////////////////////

	// Someone recommends not to create more than one automatic converter
	// to avoid the compiler error "ambiguous conversion"

	// explicit keyword only works on conversion TO var_base not conversion FROM var_base

	// NB const possibly determines priority of choice between automatic conversion
	// and C++ uses bool and int conversion in certain cases to convert to int and bool
	// therefore have chosen to make both bool and int "const" since they dont make
	// any changes to the base object.

	// Implicitly convert var_base to var

//	// Not defined anywhere yet so this is obviously not used.
//	// Type conversion since reference will be optimised away by -O2
//	// Note that most var_base function directly return a var since var's have full functionality
//	// and can be implicitly used as var_base but not vice versa.
//	CONSTEXPR operator       var&() &;
//	CONSTEXPR operator const var()  const; //{ return *reinterpret_cast<const var*>(this); }
//	CONSTEXPR operator       var();       //{ return *reinterpret_cast<var*>(this); }
//
//	constexpr operator       var()  const { return *reinterpret_cast<const var*>(this); }
//	constexpr operator const var()  const { return *reinterpret_cast<const var*>(this); }
//	constexpr operator const var&() const { return *reinterpret_cast<const var*>(this); }
//	constexpr operator       var&()       { return *reinterpret_cast<var*>(this); }

    CONSTEXPR operator       var()  const &  ;//{ return *reinterpret_cast<const var*>(this); }
//    CONSTEXPR operator const var()  const    ;//{ return *reinterpret_cast<const var*>(this); }
    CONSTEXPR operator const var&() const &  ;//{ return *reinterpret_cast<const var*>(this); }
    CONSTEXPR operator       var&()       &  ;//{ return *reinterpret_cast<var*>(this); }
    CONSTEXPR operator       var()        && ;//{ return *reinterpret_cast<const var*>(this); }

//	CONSTEXPR
//	// move, like most var_base functions returns a var since var's are var_base's but not vice versa
//	operator var() &&;

//	// TODO Check if this provides any useful functionality
//	// but ensure that it is safe in case var acquires a different data layout from var_base
//	operator var&() & {
//		return static_cast<exo::var*>(this);
//	}

	// integer <- var_base
	//////////////////////

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
	ND CONSTEXPR
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

	// floating point <- var_base
	/////////////////////////////

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
	ND CONSTEXPR
	operator FloatingPoint() const {
		assertDecimal(__PRETTY_FUNCTION__);
		EXO_SNITCH("var_base >Fp ")
		return static_cast<FloatingPoint>(var_dbl);
	}

	// void* <- var_base
	////////////////////

	// Recommended to provide automatic conversion to VOID POINTER instead of direct to bool
	// since void* is auto converted to bool nicely for natural "if (xyz)" syntax
	// and doesnt suffer the "ambiguous bool promotion to int" issue that automatic conversion
	// to bool has why this is working here now is probably because of non-portable compiler
	// characteristics or lack of source examples Note: The boost::shared_ptr class support an
	// implicit conversion to bool which is free from unintended conversion to arithmetic types.
	// http://www.informit.com/guides/content.aspx?g=cplusplus&seqNum=297
	// necessary to allow var to be used standalone in "if (xxx)" but see mv.h for discussion of
	// using void* instead of bool
	ND CONSTEXPR
	operator void*() const {
		// result in an attempt to convert an uninitialised object to void* since there is a bool
		// conversion when does c++ use automatic conversion to void* note that exodus operator !
		// uses (void*) trial elimination of operator void* seems to cause no problems but without
		// full regression testing
		EXO_SNITCH("var_base >vd*")
		return reinterpret_cast<void*>(this->toBool());
	}

	// bool <- var_base
	///////////////////

	ND CONSTEXPR
	operator bool() const {
		EXO_SNITCH("var_base >boo")
		return this->toBool();
	}

	// Implemented in varnum after ::toBool to avoid
	// varnum.cpp:646:34: error: explicit specialization of 'toBool' after instantiation

	// NB && and || operators are NOT overloaded because var_base is set to convert to boolean
	// automatically this is fortunate because "shortcircuiting" of && and || doesnt happen when
	// overloaded

	// Perhaps should NOT allow automatic convertion to char* since it assumes a conversion to
	// utf8 and cannot hold char(0) perhaps and force people to use something like .utf8() or
	// .toString().c_char() This was added to allow the arguments of osread type functions which
	// need cstrings to be cstrings so that calls with fixed filenames etc dont require a
	// conversion to var_base and back again The other solution would be to declare a parallel set of
	// function with var_base arguments operator const char*() const;

	// convert to c_str may or may not be necessary in order to do wcout<<var_base
	// NB not needed because we overide the << operator itself
	// and causes a problem with unwanted conversions to strings
	// maybe should throw an error is 0x00 present in the string
	// allow the use of any cstring function
	// var operator const char*();

	// string - replicates toString()
	// would allow the usage of any std::string function but may result
	// in a lot of compilation failures due to "ambiguous overload"
	// unfortunately there is no "explicit" keyword as for constructors - coming in C++0X
	// for now prevent this to ensure efficient programming
	//explicit

	// string-like <- var_base
	//////////////////////////

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

	ND CONSTEXPR
	operator std::string() const {
		assertString(__PRETTY_FUNCTION__);
		EXO_SNITCH("var_base >Str")
		return var_str;
	}

	ND CONSTEXPR
	operator std::string_view() const {
		assertString(__PRETTY_FUNCTION__);
		EXO_SNITCH("var_base >SV ")
		return std::string_view(var_str);
	}

	ND CONSTEXPR
	operator const char*() const {
		assertString(__PRETTY_FUNCTION__);
		EXO_SNITCH("var_base >ch*")
		return var_str.c_str();
	}

	ND CONSTEXPR
	operator std::u32string() const {
		EXO_SNITCH("var_base >u32")
		return this->to_u32string();
	}

	// In case using a pointer to a var_base
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

	// Allow conversion to char (takes first char or char 0 if zero length string)
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

	// var_base = Integral
	//////////////////////

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

	// var_base = Floating Point
	////////////////////////////

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

	// var_base = char
	//////////////////
	CONSTEXPR
	void operator=(const char char2) & {

		//THISIS("RETVARREF operator= (const char char2) &")
		// protect against unlikely syntax as follows:
		// var undefinedassign=undefinedassign=L'X';
		// this causes crash due to bad memory access due to setting string that doesnt exist
		// slows down all string settings so consider NOT CHECKING in production code
		//THISISVAR()	 // ALN:TODO: this definition kind of misleading, try to find
		// ALN:TODO: or change name to something like: THISISNOTDEAD :)
		// ALN:TODO: argumentation: var with mvtyp=0 is NOT defined

		var_str = char2;

		// reset to one unique type
		var_typ = VARTYP_STR;

		EXO_SNITCH("var_base =chr")
		return;
	}

	// var_base = char*
	///////////////////

	// The assignment operator should always return a reference to *this.
	CONSTEXPR
	void operator=(const char* cstr) & {
		//THISIS("RETVARREF operator= (const char* cstr2) &")
		// protect against unlikely syntax as follows:
		// var undefinedassign=undefinedassign="xxx";
		// this causes crash due to bad memory access due to setting string that doesnt exist
		// slows down all string settings so consider NOT CHECKING in production code
		//THISISVAR()

		var_str = cstr;

		// reset to one unique type
		var_typ = VARTYP_STR;

		EXO_SNITCH("var_base =ch*")
		return;
	}

//	// var_base = std::string& - lvalue
//	///////////////////////////////////
//
//	void operator=(const std::string& string2) & {
//
//		//THISIS("RETVARREF operator= (const std::string& string2) &")
//		// protect against unlikely syntax as follows:
//		// var undefinedassign=undefinedassign=std::string("xxx"";
//		// this causes crash due to bad memory access due to setting string that doesnt exist
//		// slows down all string settings so consider NOT CHECKING in production code
//		//THISISVAR()
//		var_str = string2;
//		var_typ = VARTYP_STR;  // reset to one unique type
//
//		return;
//	}

	CONSTEXPR void operator=(const std::string& rhs) & {
		var_str = rhs;
		var_typ = VARTYP_STR;
		EXO_SNITCH("var_base =st&")
	}

	CONSTEXPR void operator=(std::string_view rhs) & {
		var_str = std::string(rhs);
		var_typ = VARTYP_STR;
		EXO_SNITCH("var_base =sv ")
	}

	// var_base = std::string&& - lvalue/rvalue perfect forwarding
	//////////////////////////////////////////////////////////////

	// The assignment operator should always return a reference to *this.
	// but we do not in order to prevent misuse when == intended
	CONSTEXPR
	void operator=(std::string&& string2) & {

		//THISIS("RETVARREF operator= (const std::string&& string2) &")
		// protect against unlikely syntax as follows:
		// var undefinedassign=undefinedassign=std::string("xxx"";
		// this causes crash due to bad memory access due to setting string that doesnt exist
		// slows down all string settings so consider NOT CHECKING in production code
		//THISISVAR()

		var_str = std::forward<std::string>(string2);

		// reset to one unique type
		var_typ = VARTYP_STR;

		EXO_SNITCH("var_base =s&&")
		return;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////
//                                   SELF ASSIGN OPERATOR ON LVALUES
/////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SELF_OP_ARE_CHAINABLE
#	define VBR_OR_VOID VBR
#	define VBR1_OR_VOID VBR1
#	define THIS_OR_NOTHING *this
#else
#	define VBR_OR_VOID void
#	define VBR1_OR_VOID void
#	define THIS_OR_NOTHING
#endif
	VBR_OR_VOID operator+=(CBR) &;
	VBR_OR_VOID operator*=(CBR) &;
	VBR_OR_VOID operator-=(CBR) &;
	VBR_OR_VOID operator/=(CBR) &;
	VBR_OR_VOID operator%=(CBR) &;

	// Specialisations for speed to avoid a) unnecessary converting to a var and then b) having to decide what type of var we have

	// Addvar_base(
	VBR_OR_VOID operator+=(const int) &;
	VBR_OR_VOID operator+=(const double) &;
	VBR_OR_VOID operator+=(const char  char1) & {(*this) += var_base(char1); return THIS_OR_NOTHING;}
	VBR_OR_VOID operator+=(const char* chars) & {(*this) += var_base(chars); return THIS_OR_NOTHING;}
	VBR_OR_VOID operator+=(const bool) &;

	// Multiply
	VBR_OR_VOID operator*=(const int) &;
	VBR_OR_VOID operator*=(const double) &;
	VBR_OR_VOID operator*=(const char  char1) & {(*this) *= var_base(char1); return THIS_OR_NOTHING;}
	VBR_OR_VOID operator*=(const char* chars) & {(*this) *= var_base(chars); return THIS_OR_NOTHING;}
	VBR_OR_VOID operator*=(const bool) &;

	// Subtract
	VBR_OR_VOID operator-=(const int) &;
	VBR_OR_VOID operator-=(const double) &;
	VBR_OR_VOID operator-=(const char  char1) & {(*this) -= var_base(char1); return THIS_OR_NOTHING;}
	VBR_OR_VOID operator-=(const char* chars) & {(*this) -= var_base(chars); return THIS_OR_NOTHING;}
	VBR_OR_VOID operator-=(const bool) &;

	// Divide
	VBR_OR_VOID operator/=(const int) &;
	VBR_OR_VOID operator/=(const double) &;
	VBR_OR_VOID operator/=(const char  char1) & {(*this) /= var_base(char1); return THIS_OR_NOTHING;}
	VBR_OR_VOID operator/=(const char* chars) & {(*this) /= var_base(chars); return THIS_OR_NOTHING;}
	VBR_OR_VOID operator/=(const bool) &;

	// Modulo
	VBR_OR_VOID operator%=(const int) &;
	VBR_OR_VOID operator%=(const double dbl1) &;
	VBR_OR_VOID operator%=(const char  char1) & {(*this) %= var_base(char1); return THIS_OR_NOTHING;}
	VBR_OR_VOID operator%=(const char* chars) & {(*this) %= var_base(chars); return THIS_OR_NOTHING;}
	VBR_OR_VOID operator%=(const bool  bool1) & {(*this) %= var_base(bool1); return THIS_OR_NOTHING;}

	// Concat self assign is different.
	// It does return *this in order for chainable efficient multiple concatentation.

	VBR operator^=(CBR) &;
	VBR operator^=(TBR) &;
	VBR operator^=(const int) &;
	VBR operator^=(const double) &;
	VBR operator^=(const char) &;

#define NOT_TEMPLATED_APPEND
#ifdef NOT_TEMPLATED_APPEND
	VBR operator^=(const char*) &;
	VBR operator^=(const std::string&) &;
	VBR operator^=(SV) &;
#else
	// var_base = string-like
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
	VBR operator^=(Appendable str2) & {
		assertString(__PRETTY_FUNCTION__);
		var_str += str2;
		var_typ = VARTYP_STR;  // Must reset to one unique type
		return *this;
	}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////
//                          SELF ASSIGN OPERATOR ON TEMPORARIES - DEPRECATED
/////////////////////////////////////////////////////////////////////////////////////////////////

	// Prevent accidental, unusual and unnecessary coding
/*
	#define DEPRECATE [[deprecated("Using self assign operators on temporaries is pointless. Use the operator by itself, without the = sign, to achieve the same.")]]

	DEPRECATE VBR operator+=(CBR rhs) && {(*this) += rhs; return *this;}// = delete;
	DEPRECATE VBR operator*=(CBR rhs) && {(*this) *= rhs; return *this;}// = delete;
	DEPRECATE VBR operator-=(CBR rhs) && {(*this) -= rhs; return *this;}// = delete;
	DEPRECATE VBR operator/=(CBR rhs) && {(*this) /= rhs; return *this;}// = delete;
	DEPRECATE VBR operator%=(CBR rhs) && {(*this) %= rhs; return *this;}// = delete;
	DEPRECATE VBR operator^=(CBR rhs) && {(*this) ^= rhs; return *this;}// = delete;

	// Specialisations are deprecated too

	// Add
	DEPRECATE VBR operator+=(const int    rhs) && {(*this) += rhs; return *this;}// = delete;
	DEPRECATE VBR operator+=(const double rhs) && {(*this) += rhs; return *this;}// = delete;
	DEPRECATE VBR operator+=(const char   rhs) && {(*this) += rhs; return *this;}// = delete;
	DEPRECATE VBR operator+=(const char*  rhs) && {(*this) += rhs; return *this;}// = delete;
	DEPRECATE VBR operator+=(const bool   rhs) && {(*this) += rhs; return *this;}// = delete;

	// Multiply
	DEPRECATE VBR operator*=(const int    rhs) && {(*this) *= rhs; return *this;}// = delete;
	DEPRECATE VBR operator*=(const double rhs) && {(*this) *= rhs; return *this;}// = delete;
	DEPRECATE VBR operator*=(const char   rhs) && {(*this) *= rhs; return *this;}// = delete;
	DEPRECATE VBR operator*=(const char*  rhs) && {(*this) *= rhs; return *this;}// = delete;
	DEPRECATE VBR operator*=(const bool   rhs) && {(*this) *= rhs; return *this;}// = delete;

	// Subtract
	DEPRECATE VBR operator-=(const int    rhs) && {(*this) -= rhs; return *this;}// = delete;
	DEPRECATE VBR operator-=(const double rhs) && {(*this) -= rhs; return *this;}// = delete;
	DEPRECATE VBR operator-=(const char   rhs) && {(*this) -= rhs; return *this;}// = delete;
	DEPRECATE VBR operator-=(const char*  rhs) && {(*this) -= rhs; return *this;}// = delete;
	DEPRECATE VBR operator-=(const bool   rhs) && {(*this) -= rhs; return *this;}// = delete;

	// Divide
	DEPRECATE VBR operator/=(const int    rhs) && {(*this) /= rhs; return *this;}// = delete;
	DEPRECATE VBR operator/=(const double rhs) && {(*this) /= rhs; return *this;}// = delete;
	DEPRECATE VBR operator/=(const char   rhs) && {(*this) /= rhs; return *this;}// = delete;
	DEPRECATE VBR operator/=(const char*  rhs) && {(*this) /= rhs; return *this;}// = delete;
	DEPRECATE VBR operator/=(const bool   rhs) && {(*this) /= rhs; return *this;}// = delete;

	// Modulo
	DEPRECATE VBR operator%=(const int    rhs) && {(*this) %= rhs; return *this;}// = delete;
	DEPRECATE VBR operator%=(const double rhs) && {(*this) %= rhs; return *this;}// = delete;
	DEPRECATE VBR operator%=(const char   rhs) && {(*this) %= rhs; return *this;}// = delete;
	DEPRECATE VBR operator%=(const char*  rhs) && {(*this) %= rhs; return *this;}// = delete;
	DEPRECATE VBR operator%=(const bool   rhs) && {(*this) %= rhs; return *this;}// = delete;

	// Concat
	DEPRECATE VBR operator^=(const int          rhs) && {(*this) ^= rhs; return *this;}// = delete;
	DEPRECATE VBR operator^=(const double       rhs) && {(*this) ^= rhs; return *this;}// = delete;
	DEPRECATE VBR operator^=(const char         rhs) && {(*this) ^= rhs; return *this;}// = delete;
	DEPRECATE VBR operator^=(const char*        rhs) && {(*this) ^= rhs; return *this;}// = delete;
	DEPRECATE VBR operator^=(const std::string& rhs) && {(*this) ^= rhs; return *this;}// = delete;
	DEPRECATE VBR operator^=(const std::string_view rhs) && {(*this) ^= rhs; return *this;}// = delete;

	#undef DEPRECATE
*/

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
	///  :
	///////////////

	// Logical

	friend bool var_eq_var (      CBR1   lhs,       CBR1   rhs  );
	friend bool var_lt_var (      CBR1   lhs,       CBR1   rhs  );

	// Specialisations for speed

	friend bool var_eq_dbl (      CBR1   lhs, const double dbl1 );
	friend bool var_eq_int (      CBR1   lhs, const int    int1 );
	friend bool var_eq_bool(      CBR1   lhs, const bool   bool1);

	friend bool var_lt_int (      CBR1   lhs, const int    int1 );
	friend bool int_lt_var (const int    int1,      CBR1   rhs  );
	friend bool var_lt_bool (      CBR1  lhs, const bool   bool1);

	friend bool var_lt_dbl (      CBR1   lhs, const double dbl1 );
	friend bool dbl_lt_var (const double dbl1,      CBR1   rhs  );
	friend bool bool_lt_var (const bool  bool1,     CBR1   rhs  );

//	friend var operator""_var(const char* cstr, std::size_t size);

	//////////////////////////////////////
	// Declare friendship within the class
	//////////////////////////////////////

	// and an edited version "varb_friend_impl.h" which contains implementations is included in var.cpp
	// ENSURE all changes in varb_friends.h and varb_friends_impl.h are replicated using sed commands listed in the files
	// Implementations are in varb_friends_impl.h included in var.cpp

// Also included *without* friend definition after class var_base definition below.
#define VAR_FRIEND friend
#include "varb_friends.h"
// Implementation in varb.cpp
//#include "varb_friends_impl.h"

#pragma GCC diagnostic pop // "-Wnon-template-friend"

/////////////////////////////////////////////////////////////////////////////////////////////////
//                                         IOSTREAM FRIENDS
/////////////////////////////////////////////////////////////////////////////////////////////////

	// OSTREAM
	//////////

//	// Note replicated for var but that one converts FM, VM etc. to visible chars before output
	friend std::ostream& operator<<(std::ostream& ostream1, CBR1 outvar)
	{
//		THISIS("ostream << var")
//		outvar.assertString(function_sig);
		outvar.assertString("ostream << var");
		ostream1 << outvar.var_str;
		return ostream1;
	}

	// ISTREAM
	//////////

	friend std::istream& operator>>(std::istream& istream1, VBR invar) {

//		THISIS("istream >> var")
//		invar.assertVar(function_sig);
		invar.assertVar("istream >> var");

		invar.var_str.clear();
		invar.var_typ = VARTYP_STR;

		//std::string tempstr;
		istream1 >> std::noskipws >> invar.var_str;

		// this would verify all input is valid utf8
		// in_str1.var_str=boost::locale::conv::utf_to_utf<char>(in_str1)

		return istream1;
	}

	/// UTILITY:
	///////////

	// obj is var

	// Get the length in number of chars/bytes.
	// return: A number
	//
	// `let v1 = "abc"_var.len(); // 3
	//  // or
	//  let v2 = len("abc");`
	//
	ND RETVAR len() const;

	// Test if the var is an empty string.
	// return: True if it is empty amd false if not.
	// This is a shorthand and more expressive way of writing 'if (var == "")' or 'if (var.len() == 0)' or 'if (not var.len())'
	// Note that 'if (var.empty())' is not exactly the same as 'if (not var)' because 'if (var("0.0")' is also defined as false. If a string can be converted to 0 then it is considered to be false. Contrast this with common scripting languages where 'if (var("0"))' is defined to be true.
	//
	// `let v1 = "0";
	//  if (not v1.empty()) ... ok // true
	//  // or
	//  if (not empty(v1)) ... ok // true`
	//
	ND bool empty() const;

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
	// undocumented
	[[deprecated ("EXODUS: Replace single character accessors like xxx[n] with xxx.at(n)")]]
	ND RETVAR operator[](int pos1) const;

	// Named member function identical to operator[]
	// undocumented
	ND RETVAR at(const int pos1) const;

	// as of now, sadly the following all works EXCEPT that var[99].anymethod() doesnt work
	// so would have to implement all var methods and free functions on the proxy object
	//ND var_brackets_proxy operator[](int pos1);
	//ND var operator[](int pos1) &&;


/////////////////////////////////////////////////////////////////////////////////////////////////
//                                         PUBLIC MEMBER FUNCTIONS
/////////////////////////////////////////////////////////////////////////////////////////////////


	// Check if a var has been assigned a value.
	// return: True if the var is assigned, otherwise false
	ND bool assigned() const;

	// Check if a var has not been assigned a value;
	// return: True if the var is unassigned, otherwise false
	ND bool unassigned() const;

	// If a var is unassigned, assign a default value.
	// If the var is unassigned then assign the default value to it, otherwise do nothing.
	// defaultvalue: Cannot be unassigned.
	//
	// `var v1; // Unassigned
	//  v1.defaulter("abc"); // v1 -> "abc"
	//  // or
	//  defaulter(v1, "abc");`
	//
	void defaulter(CBR defaultvalue);

	// Copy a var or, if it is unassigned, copy a default value.
	// return: A copy of the var if it is assigned or the default value if it is not.
	// Can be used to handle optional arguments in functions.
	// defaultvalue: Cannot be unassigned.
	// obj is v2
	//
	// `var v1; // Unassigned
	//  var v2 = v1.or_default("abc"); // v2 -> "abc"
	//  // or
	//  var v3 = or_default(v1, "abc");`
	//
	// Mutator: defaulter()
	//
	ND RETVAR or_default(CBR defaultvalue) const;

	// "move" the var to the destination var leaving the source var unassigned.
	// This is useful for performance with vars which own strings larger than the internal buffer of a std::string object. (15 chars/bytes on linux).
	// Move a var into another var.
	// Performs a shallow copy of the var's data and transfers ownership of its string, if any. The moved var is set to an empty string.
	// Enables efficient handling of large strings by moving pointers without copying or allocating memory.
	// throw: VarUnassigned if the moved var is unassigned before the move.
	// obj is v2
	//
	// `var v1 = space(65'535);
	//  var v2;
	//  v1.move(v2); // v2.len() -> 65'536 // v1 -> ""`
	//
	void move(VBR destinationvar);

	// "move" a var into a temporary var.
	// See move(destinationvar).
	//
	// `var v1 = space(65'535);
	//  var v2 = v1.move(); // v2.len() -> 65'536 // v1 -> ""
	//  // or
	//  let v3 = move(v2);`
	//
	ND RETVAR move();

	// Swap the contents of one var with another.
    // Useful for stashing large strings quickly. They are moved using pointers without making copies or allocating memory.
	// Either or both variables may be unassigned.
	//
	// `var v1 = space(65'536);
	//  var v2 = "";
	//  v1.swap(v2); // v1 -> "" // v2.len() -> 65'536
	//  // or
	//  swap(v1, v2);`
	//
	void swap(VBR var2);

	// Swaps the contents of two variables.
	// This version works on const vars
	// This version of swap is marked as const despite it replacing the var with var2
	// Currently this is required in rare cases where functions like exoprog::calculate
	// temporarily require member variables to be something else but switch back before exiting
	// If such functions throw an error without catching then it would leave the member variables in a changed state.
	// Either or both vars may be unassigned without triggering a VarUnnassigned error.
	void swap(CBR var2) const;

    // Return a copy of the var.
    // The cloned var may be unassigned. No VarUnassigned error will not be thrown.
	// obj is v2
	// `var v1 = "abc";
	//  var v2 = v1.clone(); // "abc"
	//  // or
	//  var v3 = clone(v2);`
	//
	ND RETVAR clone() const;

	// Return a string describing internal data of a var.
	// If the var holds an internal std::string using heap storage then its its heap storage address is given.
	// typ: Multiple typs may exist simultaneously.
    // * 0x01 * str is available.
    // * 0x02 * int is available.
    // * 0x04 * dbl is available.
    // * 0x08 * nan: str is not a number.
    // * 0x16 * osfile (nan is true, str, int and dbl have special meaning).
	//
	// `var v1 = str("x", 32);
	//  v1.dump().outputl(); /// e.g. var:0x7ffea7462cd0 typ:1 str:0x584d9e9f6e70 "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
	//  // or
	//  outputl(dump(v1));`
	//
	ND RETVAR dump() const;

	////////////
	/// NUMERIC:
	////////////

	// obj is var

	// Check if a var is numeric.
	// return: True if a var holds a double, integer, or a string representing a numeric value.
	// A string is considered numeric if it is:
	// * Empty (treated as zero), or
	// * Composed of one or more digits (0-9), an optional leading '+' or '-' sign, and an optional single decimal point ('.') placed before, within, or after the digits.
	// * Optionally includes an exponential suffix ('e' or 'E', optionally followed by '+' or '-', and 1-3 digits).
	//
	// `if (   "+123.45"_var.isnum()) ... ok
	//  if ("+1.2345e+2"_var.isnum()) ... ok
	//  if (          ""_var.isnum()) ... ok
	//  if (not      "."_var.isnum()) ... ok
	//  // or
	//  if (isnum("123.")) ... ok`
	//
	ND bool isnum() const;

    // Copy a var if it is numeric or 0 if not.
	// This allows working numerically with data that may be non-numeric.
	// return: A guaranteed numeric var
	//
	// `var v1 = "123.45"_var.num();    // 123.45
	//  var v2 = "abc"_var.num() + 100; // 100`
	//
	ND RETVAR num() const;

	// Unary plus.
	// return: The numeric value of a variable or expression, or throws a VarNonNumeric error.
	//
	// `let v1 = 3;
	//  let v2 = +v1; // 3`
	//
	ND RETVAR operator+() const;

	// Unary minus.
	// return: The negative of a numeric variable or expression, or throws a VarNonNumeric error. Negative of a negative is a positive.
	//
	// `let v1 = 3;
	//  let v2 = -v1; // -3
	//  let v3 = -v2; // 3`
	//
	ND RETVAR operator-() const;

	// Boolean negate.
	// Alias: "not" as in "if (not ...)".
	// return: True if a variable or expression is false, or False if it is true.
	// Negate the result of the following:
	// * Empty string * false
	// * Non-numeric string * true
	// * Zero (string or number) * false
	// * Non-zero (string or number) * true
	//
	// `if (!var("") eq true) ... ok
	//  if (!var("x") eq false) ... ok
	//  if (!var("0") eq true) ... ok
	//  if (!var("123") eq false) ... ok
	//  // or
	//  if (not var("") eq true) ... ok`
	//
	ND bool operator!() const {return !this->toBool();}

	// Post-increment
	// ++ and -- are only allowed on named variables, not expressions, to prevent writing cryptic and buggy code.
	//
	// `var v1 = 3;
	//  let v2 = v1++; // v2 -> 3 // v1 -> 4`
	//
	RETVAR operator++(int) &;

	// Post-decrement
	//
	// `var v1 = 3;
	//  let v2 = v1--; // v2 -> 3 // v1 -> 2`
	//
	RETVAR operator--(int) &;

	// Pre-increment
	//
	// `var v1 = 3;
	//  let v2 = ++v1; // v2 -> 4 // v1 -> 4`
	//
	RETVARREF operator++() &;

	// Pre-decrement
	//
	// `var v1 = 3;
	//  let v2 = --v1; // v2 -> 2 // v1 -> 2`
	//
	RETVARREF operator--() &;

/* for gendoc
	// Addition
	// Any attempt to perform numeric operations on non-numeric strings will throw a runtime error VarNonNumeric.
	// Floating point numbers are implicitly converted to strings with no more than 12 significant digits of precision. This practically eliminates all floatng point rounding errors.
	// Internally, 0.1 + 0.2 looks like this using doubles.
	// 0.10000000000000003 + 0.20000000000000004 -> 0.30000000000000004
	//
	// `var v1 = 0.1;
	//  var v2 = v1 + 0.2; // 0.3`
	//
	ND var operator+(var);

	// Subtraction
	ND var operator-(var);

	// Multiplication
	ND var operator*(var);

	// Division
	ND var operator/(var);

	// Modulus
	ND var operator%(var);

	// Self addition
	//
	// `var v1 = 0.1;
	//  v1 += 0.2; // 0.3`
	//
	ND var operator+=(var);

	// Self subtraction
	ND var operator-=(var);

	// Self multiplication
	ND var operator*=(var);

	// Self division
	ND var operator/=(var);

	// Self modulus
	ND var operator%=(var);

*/
	// Absolute value.
	// `let v1 = -12.34;
	//  let v2 = v1.abs(); // 12.34
	//  // or
	//  let v3 = abs(v1);`
	//
	ND RETVAR abs() const;

	// Power.
	// `let v1 = var(2).pwr(8); // 256
	//  // or
	//  let v2 = pwr(2, 8);`
	//
	ND RETVAR pwr(in exponent) const;

	// Initialise the seed for rnd().
	// Allow the stream of pseudo random numbers generated by rnd() to be reproduced.
	// Seeded from std::chrono::high_resolution_clock::now() if the argument is 0;
	//
	// `var(123).initrnd(); /// Set seed to 123
	//  // or
	//  initrnd(123);`
	//
	   void initrnd() const;

	// Pseudo random number generator.
	// return: A pseudo random integer between 0 and the provided maximum minus 1.
	// Uses std::mt19937 and std::uniform_int_distribution<int>
	//
	// `let v1 = var(100).rnd(); /// Random 0 to 99
	//  // or
	//  let v2 = rnd(100);`
	//
	ND RETVAR rnd() const;

	// Power of e.
	// `let v1 = var(1).exp(); // 2.718281828459045
	//  // or
	//  let v2 = exp(1);`
	//
	ND RETVAR exp() const;

	// Square root.
	// `let v1 = var(100).sqrt(); // 10
	//  // or
	//  let v2 = sqrt(100);`
	//
	ND RETVAR sqrt() const;

	// Sine of degrees.
	// `let v1 = var(30).sin(); // 0.5
	//  // or
	//  let v2 = sin(30);`
	//
	ND RETVAR sin() const;

	// Cosine of degrees.
	// `let v1 = var(60).cos(); // 0.5
	//  // or
	//  let v2 = cos(60);`
	//
	ND RETVAR cos() const;

	// Tangent of degrees.
	// `let v1 = var(45).tan(); // 1
	//  // or
	//  let v2 = tan(45);`
	//
	ND RETVAR tan() const;

	// Arctangent of degrees.
	// `let v1 = var(1).atan(); // 45
	//  // or
	//  let v2 = atan(1);`
	//
	ND RETVAR atan() const;

	// Natural logarithm.
	// return: Floating point var (double)
	// `let v1 = var(2.718281828459045).loge(); // 1
	//  // or
	//  let v2 = loge(2.718281828459045);`
	//
	ND RETVAR loge() const;

	//  ND RETVAR int() const;//reserved word

	// Truncate decimals.
	// Convert decimal to nearest integer towards zero.
	// Remove decimal fraction.
	// return: An integer var
	// `let v1 = var(2.9).integer(); // 2
	//  // or
	//  let v2 = integer(2.9);
	//
	//  var v3 = var(-2.9).integer(); // -2
	//  // or
	//  var v4 = integer(-2.9);`
	//
	ND RETVAR integer() const;

	// Floor decimals.
	// Convert decimal to nearest integer towards negative infinity.
	// return: An integer var
	// `let v1 = var(2.9).floor(); // 2
	//  // or
	//  let v2 = floor(2.9);
	//
	//  var v3 = var(-2.9).floor(); // -3
	//  // or
	//  var v4 = floor(-2.9);`
	//
	ND RETVAR floor() const;

	// Get the modulo of two numbers.
	// return: "floored division modulo" also known as Euclidean modulo.
	// limit:
	// * Positive * return:  range [0, limit)                    0 ≤ result < limit
	// * Negative * return:  range (limit, 0]   limit < result ≤ 0
	//
	// Exodus' modulo limits the value instead of doing a kind of remainder as per C/C++ % operator which performs "truncated division modulo".
	// Exodus' is symmetrical about 0. The symmetry property is a hallmark of floored division modulo, distinguishing it from truncated modulo.
	// * mod(x, y) == -(mod(-x, -y))
	//
	// `let v01 = var(3.0).mod(3.0);   // 0
	//  let v02 = var(2.0).mod(3.0);   // 2
	//  let v03 = var(0.0).mod(3.0);   // 0
	//  let v04 = var(-2.0).mod(3.0);  // 1
	//  let v05 = var(-3.0).mod(3.0);  // 0
	//  let v06 = var(3.0).mod(-3.0);  // 0
	//  let v07 = var(2.0).mod(-3.0);  // -1
	//  let v08 = var(0.0).mod(-3.0);  // 0
	//  let v09 = var(-2.0).mod(-3.0); // -2
	//  let v10 = var(-3.0).mod(-3.0); // 0
	//  // or
	//  let v11 = mod(4, 3);  // 1
	//  // or
	//  let v12 = var(4) % 3; // 1`
	//
	ND RETVAR mod(CBR limit) const;

	// undocumented overload for double
	ND RETVAR mod(double limit) const;

	// undocumented overload for int
	ND RETVAR mod(const int limit) const;

	// Get the maximum of two values
	// Can be numeric or alphanumeric
	//
	// `let v1 = 100;
	//  let v2 = 200;
	//  //
	//  let v3 = v1.max(v2);  // 200
	//  // or
	//  let v4 = max(v1, v2); // 200
	//  // also
	//  let v5 = v1.min(v2);  // 100
	//  let v6 = min(v1, v2); // 100`
	//
	ND RETVAR max(in arg2) const;

	// Get the min of two values
	// Similar to max()
	ND RETVAR min(in arg2) const;

	// Fix the number of decimal places.
	//
	// `let v1 = var(123.4).round(2);   // "123.40"
	//  let v2 = var(123456).round(-3); // "123000"
	//  // or
	//  let v3 = round(123.4, 2);`
	//
	ND RETVAR round(const int ndecimals = 0) const;

	// Get the precision used for floating point comparisons. The default is 4.
	// Precision is used when comparing floating point numbers. Practically speaking, standard computer floating point numbers only have about 12 significant decimal digits. Exodus chooses to use these 12 digits to handle numbers in the range +/- 99'999'999.9999 by default.
	// Exodus precision means the number of significant decimal digits after the decimal place. This is different from standard scientific precision which is defined as the total number of decimal digits.
	// If precision is 4 then the smallest significant difference is 0.0001. Differences smaller than this are treated as zero.
	// Precision can be set per thread. Exodus programs started with run() start with default precision 4.
	// Performed or executed programs get the precision of the caller. They can change the precision without affecting the precision of the parent program.
	//
	// `let v1 = var(0.00001) eq 0; // true
	//  var::setprecision(6);
	//  let v2 = var(0.00001) eq 0; // false;
	//  // or
	//  setprecision(4);`
	//
	ND static var  getprecision();

	// Set the precision to be used for floating point comparisons.
	// See getprecision() for more info.
	   static var  setprecision(int);

	////////////////////
	/// Stop documenting
	/// :
	////////////////////

	// basic string function on var_base for throwing errors
	ND RETVAR first_(int nchars) const;

	// Static member for speed on std strings because of underlying boost implementation
	// Unprotected currently because it is used in free function locateat
	// TODO verify if static function is thread safe
	static int localeAwareCompare(const std::string& str1, const std::string& str2);
//	static int localeAwareCompare(const std::string_view str1, const std::string_view str2);
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

	// ::assertVar

	///////////
	// WARNING: MUST NOT use any var while in assertVar
	// OTHERWISE *WILL* get recursion/segfault
	///////////
	CONSTEXPR
	void assertVar(const char* message, const char* varname = "") const {
////#if __GNUC__ >= 99 || __clang_major__ > 18
////#else
//#if defined(__OPTIMIZE__) && !defined(__NO_INLINE__)
//    // Code for when optimization is on, but not -O0
//#else
		if (var_typ & VARTYP_MASK)
			[[unlikely]]
			throw VarUnconstructed(std::string(varname) + " in " + message);
//#endif
	}

	// ::assertAssigned

	CONSTEXPR
	void assertAssigned(const char* message, const char* varname = "") const {
		assertVar(message, varname);
		if (!var_typ)
			[[unlikely]]
			throw VarUnassigned(std::string(varname) + " in " + message);
	}

	// assertNumeric

	//CONSTEXPR
	void assertNumeric(const char* message, const char* varname = "") const;

	// ::assertDecimal

	CONSTEXPR
	void assertDecimal(const char* message, const char* varname = "") const {
		assertNumeric(message, varname);
		if (!(var_typ & VARTYP_DBL)) {
			var_dbl = static_cast<double>(var_int);
			// Add double flag
			var_typ |= VARTYP_DBL;
		}
	}

	// ::assertInteger

	//CONSTEXPR
	void assertInteger(const char* message, const char* varname = "") const;
// {
//		assertNumeric(message, varname);
//		if (!(var_typ & VARTYP_INT)) {
//
//			//var_int = std::floor(var_dbl);
//
//			// Truncate double to int
//			//var_int = std::trunc(var_dbl);
//			if (var_dbl >= 0) {
//				// 2.9 -> 2
//				// 2.9999 -> 2
//				// 2.99999 -> 3
//				var_int = static_cast<varint_t>(var_dbl + EXO_SMALLEST_NUMBER / 10);
//			} else {
//				// -2.9 -> -2
//				// -2.9999 -> -2.9
//				// -2.99999 -> -3
//				var_int = static_cast<varint_t>(var_dbl - EXO_SMALLEST_NUMBER / 10);
//			}
//
//			// Add int flag
//			var_typ |= VARTYP_INT;
//		}
//	}
//
	// ::assertString

	CONSTEXPR
	void assertString(const char* message, const char* varname = "") const {
		assertVar(message, varname);
		if (!(var_typ & VARTYP_STR)) {
			if (!var_typ)
				[[unlikely]]
				throw VarUnassigned(std::string(varname) + " in " + message);
			this->createString();
		}
	}

	// ::assertStringMutator

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

// varb_friends again to add [[nodicard]] attribute
#undef VAR_FRIEND
#define VAR_FRIEND
#undef PUBLIC
#define PUBLIC [[nodiscard]] __attribute__((visibility("default")))
#include "varb_friends.h"
#undef PUBLIC
#define PUBLIC __attribute__((visibility("default")))
////#include "varb_friends_impl.h"

} // namespace exo

#endif // LIBEXODUS_VARB_H_
