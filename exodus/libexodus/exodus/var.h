#ifndef EXODUS_LIBEXODUS_EXODUS_VAR_H_
#define EXODUS_LIBEXODUS_EXODUS_VAR_H_
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

// clang-format off

#define EXODUS_RELEASE "23.09"
#define EXODUS_PATCH "23.09.0"

#include <cstdint> // for uint64_t
#if __has_include(<concepts>)
#	include <concepts>
#endif
#include <iostream>
#include <string>
#include <string_view>
#ifdef EXODUS_TIMEBANK
#	include "timebank.h"
#endif

// http://stackoverflow.com/questions/538134/exporting-functions-from-a-dll-with-dllexport
// Using dllimport and dllexport in C++ Classes
// http://msdn.microsoft.com/en-us/library/81h27t8c(VS.80).aspx

//https://quuxplusone.github.io/blog/2021/11/09/pass-string-view-by-value/

#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__

	// BUILDING_LIBRARY is defined set in exodus/library.h and exodus/common.h
#	ifdef BUILDING_LIBRARY
#		define PUBLIC __declspec(dllexport)
#	else
#		define PUBLIC __declspec(dllimport)
#	endif

#else

	// use g++ -fvisibility=hidden to make all hidden except those marked PUBLIC ie "default"
	// see nm -D libexodus.so --demangle |grep T -w

#	define PUBLIC __attribute__((visibility("default")))

#endif // not windows

//#ifndef EXO_VAR_CPP
#	define EXTERN extern
//#else
//#	define EXTERN
//#endif

#define ND [[nodiscard]]

#include <exodus/vartyp.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"

// Use ASCII 0x1A-0x1F for PickOS separator chars instead
// of PickOS 0xFA-0xFF which are illegal utf-8 bytes

// also defined in pgexodus extract.c

// the var versions of the following (without leading or trailing _)
// are defined AFTER the class declaration of "var"

// leading _ char* versions of classic pick delimiters
// Using macros to allow use of space as compile time concatenation operator
#define _RM "\x1F"      // Record Mark
#define _FM "\x1E"      // Field Mark
#define _VM "\x1D"      // Value Mark
#define _SM "\x1C"      // Subvalue Mark
#define _TM "\x1B"      // Text Mark
#define _ST "\x1A"  // Subtext Mark

#define _BS "\\"
#define _DQ "\""
#define _SQ "\'"

// trailing _ char versions of classic pick delimiters
inline const char RM_ = '\x1F';     // Record Mark
inline const char FM_ = '\x1E';     // Field Mark
inline const char VM_ = '\x1D';     // Value Mark
inline const char SM_ = '\x1C';     // Subvalue Mark
inline const char TM_ = '\x1B';     // Text Mark
inline const char ST_ = '\x1A';     // Subtext Mark

inline const char BS_ = '\\';
inline const char DQ_ = '\"';
inline const char SQ_ = '\'';

// printx() converts FM etc to these characters. user literal conversion _var also has them but hard coded in fmiconverter()
//#define _VISIBLE_FMS "_^]\[Z"  //PickOS standard. Backslash not good since it is often used for escaping chars. Z is normal letter.
//#define _VISIBLE_FMS "<[{}]>" //logical but hard to read direction of brackets quickly
//#define _VISIBLE_FMS "_^]}`~" //all uncommon in natural language. first 3 _^] are identical to pickos
//#define _VISIBLE_FMS "_^]}|~"   //all uncommon in natural language. first 3 _^] are identical to pickos
#define _VISIBLE_FMS "`^]}|~"   //all uncommon in natural language. ^] are identical to pickos. Using ` for RM since _ common in IT
inline const char VISIBLE_RM_ = '`';
inline const char VISIBLE_FM_ = '^';
inline const char VISIBLE_VM_ = ']';
inline const char VISIBLE_SM_ = '}';
inline const char VISIBLE_TM_ = '|';
inline const char VISIBLE_ST_ = '~';
#define _ALL_FMS _RM _FM _VM _SM _TM _ST

// Useful TRACE() function for debugging
#define TRACE(EXPRESSION) \
	var(EXPRESSION).convert(_ALL_FMS, _VISIBLE_FMS).quote().logputl("TRACE: " #EXPRESSION "=");
#define TRACE2(EXPRESSION) \
	std::cerr << (EXPRESSION) << std::endl;

#define DEFAULT_UNASSIGNED = var()
#define DEFAULT_EMPTY = ""
#define DEFAULT_DOT = "."
#define DEFAULT_SPACE = " "
#define DEFAULT_VM = VM_
#define DEFAULT_NULL = nullptr

#ifdef EXODUS_TIMEBANK
#	define THISIS(FUNC_DESC) [[maybe_unused]] static const char* function_sig = FUNC_DESC;Timer thisistimer(get_timeacno(FUNC_DESC));
#else
#	define THISIS(FUNC_DESC) [[maybe_unused]] static const char* function_sig = FUNC_DESC;
#endif

#define ISDEFINED(VARNAME) (VARNAME).assertDefined(function_sig, #VARNAME);
#define ISASSIGNED(VARNAME) (VARNAME).assertAssigned(function_sig, #VARNAME);
#define ISSTRING(VARNAME) (VARNAME).assertString(function_sig, #VARNAME);
#define ISNUMERIC(VARNAME) (VARNAME).assertNumeric(function_sig, #VARNAME);

#define BACKTRACE_MAXADDRESSES 100

#pragma clang diagnostic push

namespace exodus {

#if __cpp_lib_concepts >= 201907L
	template <typename T>
	concept std_string_or_convertible = std::is_convertible_v<T, std::string_view>;

	template <typename T>
	concept std_u32string_or_convertible = std::is_convertible_v<T, std::u32string_view>;
#endif

//#define SMALLEST_NUMBER 1e-13
//sum(1287.89,-1226.54,-61.35,1226.54,-1226.54) -> 0.000'000'000'000'23
//#define SMALLEST_NUMBER 1e-10
//#define SMALLEST_NUMBER 2.91E-11 (2^-35) or 0.000'000'000'029'1
//#define SMALLEST_NUMBER 1e-4d//0.0001 for pickos compatibility
constexpr double SMALLEST_NUMBER = 0.0001;// for pickos compatibility

class var;
class dim;
class var_iter;
class var_extractreplace;

class var_proxy1;
class var_proxy2;
class var_proxy3;
//class var_brackets_proxy;

using VARREF = var&;
using CVR = const var&;
using TVR = var&&;
using SV = std::string_view;

// original help from Thinking in C++ Volume 1 Chapter 12
// http://www.camtp.uni-mb.si/books/Thinking-in-C++/TIC2Vone-distribution/html/Chapter12.html

// could also use http://www.informit.com/articles/article.asp?p=25264&seqNum=1
// which is effectively about makeing objects behave like ordinary variable syntactically
// implementing smartpointers

// the destructor is public non-virtual (supposedly to save space)
// since this class has no virtual functions
// IT SHOULD BE ABLE TO DERIVE FROM IT AND DO DELETE()
// http://www.parashift.com/c++-faq-lite/virtual-functions.html#faq-20.7

// on gcc, size of var is 48 bytes
//
// string:    32
// int:        8
// double:     8
// type:       4
//           ---
// var:       52

using varint_t = int64_t;

// Hard coding to avoid including <limits>
// #define VAR_MAX_DOUBLE std::numeric_limits<varint_t>::max()
// #define VAR_LOW_DOUBLE std::numeric_limits<varint_t>::lowest()
//#define VAR_MAX_DOUBLE 1.797693134862315708145274237317043567981e+308
//#define VAR_LOW_DOUBLE -1.797693134862315708145274237317043567981e+308
inline const long double VAR_MAX_DOUBLE = static_cast<long double>(1.797693134862315708145274237317043567981e+30);
inline const long double VAR_LOW_DOUBLE = static_cast<long double>(-1.797693134862315708145274237317043567981e+30);

//template<class T, class... P0toN>
//struct is_one_of;
//
//template<class T>
//struct is_one_of<T> : std::false_type {static const bool value{false};};
//
//template<class T, class... P1toN>
//struct is_one_of<T, T, P1toN...> : std::true_type {static const bool value{true};};
//
//template<class T, class P0, class... P1toN>
//struct is_one_of<T, P0, P1toN...> : is_one_of<T, P1toN...> {};

	////////////
	// class var
	////////////

//"final" to prevent inheritance because var has a destructor which is non-virtual to save space and time
class PUBLIC var final {

	///////////////////////
	// PRIVATE DATA MEMBERS
	///////////////////////

 private:

	// Understanding size of bytes, characters, integers and floating point
	// See https://en.cppreference.com/w/cpp/language/types

	// All mutable because asking for a string can create it from an integer and vice versa
	mutable std::string var_str; // 32 bytes on g++. is default constructed to empty string
	mutable varint_t    var_int; // 8 bytes/64 bits - currently defined as a long long
	mutable double      var_dbl; // 8 bytes/64 buts - double
	mutable VARTYP      var_typ; // Default initialised to VARTYP_UNA
	                             // Actually a unsigned int which will be 4 bytes


	/////////////////////////////////
	// SPECIAL MEMBER FUNCTIONS (SMF)
	/////////////////////////////////

 public:

	/////////////////////////
	// 1. Default constructor - Can default as string and var_typ have constructors
	/////////////////////////
	//
	// allow syntax "var v;" to create an "unassigned" var (var_typ is 0)
	//
	var() noexcept = default;

//	var()
//		: var_typ(VARTYP_UNA) {
//		//std::cout << "ctor()" << std::endl;
//
//		// int xyz=3;
//		// WARNING neither initialisers nor constructors are called in the following case !!!
//		// var xxx=xxx.somefunction()
//		// known as "undefined usage of uninitialised variable";
//		// and not even a compiler warning in msvc8 or g++4.1.2
//
//		// so the following test is put everywhere to protect against this type of accidental
//		// programming if (var_typ&VARTYP_MASK) throw VarUndefined("funcname()"); should really
//		// ensure a magic number and not just HOPE for some binary digits above bottom four 0-15
//		// decimal 1111binary this could be removed in production code perhaps
//
//	}

	////////////////
	// 2. Destructor - We need to set var_typ to undefined for safety.
	////////////////
	//
	// sets var_typ undefined
	//
	// WARNING: non-virtual destructor - so cannot create derived classes
	// Inline for speed but slows compilation unless optimising switched off
#define VAR_SAFE_DESTRUCTOR
#ifdef VAR_SAFE_DESTRUCTOR
	~var() {
		//std::cout << "dtor:" << var_str << std::endl;

		// try to ensure any memory is not later recognises as initialised memory
		//(exodus tries to detect undefined use of uninitialised objects at runtime - that dumb
		// compilers allow without warning) this could be removed in production code perhaps set all
		// unused bits to 1 to ease detection of usage of uninitialised variables (bad c++ syntax
		// like var x=x+1; set all used bits to 0 to increase chance of detecting unassigned
		// variables var_typ=(char)0xFFFFFFF0;
		var_typ = VARTYP_MASK;
#ifdef EXO_SNITCH
		std::clog << this << " var dtor" <<std::endl;
#endif
	}
#else
	~var() = default;
#endif

	//////////////////////
	// 3. Copy constructor - Cant use default because we need to throw if rhs is unassigned
	//////////////////////
	//
	var(CVR rhs)
		:
		var_str(rhs.var_str),
		var_int(rhs.var_int),
		var_dbl(rhs.var_dbl),
		var_typ(rhs.var_typ) {

		// use initializers for speed? and only check afterwards if rhs was assigned
		rhs.assertAssigned(__PRETTY_FUNCTION__);

#ifdef EXO_SNITCH
		std::clog << this << " var copy ctor" <<std::endl;
#endif
	}

	//////////////////////
	// 4. move constructor
	//////////////////////
	//
	// If noexcept then STL containers will use move during various operations otherwise they will use copy
	// We will use default since temporaries are unlikely to be undefined or unassigned and we will skip the usual checks
	var(TVR fromvar) noexcept = default;

	/////////////////////
	// 5. copy assignment - from lvalue
	/////////////////////

	//var = var

	// Not using copy and replace idiom (copy assignment by value)
	// because Howard Hinnant recommends against in our case
	// We avoids a copy and make sure that we do the

	// Prevent assigning to temporaries with a clearer error message
	// e.g. printl(date() = 12345);
	//[[deprecated("Deprecated is a great way to highlight all uses of something which can otherwise be hard or slow to find!")]
	void operator=(CVR rhs) && = delete;

	// var& operator=(CVR rhs) & = default;
	// Cannot use default copy assignment because
	// a) it returns a value allowing accidental use of "=" instead of == in if statements
	// b) doesnt check if rhs is assigned
	void operator=(CVR rhs) & {

		//assertDefined(__PRETTY_FUNCTION__);  //could be skipped for speed?
		rhs.assertAssigned(__PRETTY_FUNCTION__);

#ifdef EXO_SNITCH
		std::clog << this << " var copy assign" <<std::endl;
#endif
		// Prevent self assign
		// Removed for speed since we assume std::string handles it ok
		//if (this == &rhs)
		//	return;

		// copy everything across
		var_str = rhs.var_str;
		var_dbl = rhs.var_dbl;
		var_int = rhs.var_int;
		var_typ = rhs.var_typ;

		return;
	}

	/////////////////////
	// 6. move assignment - from rvalue/temporary
	/////////////////////

	// var = temp var

	// Prevent assigning to temporaries
	// xyz.f(2) = "abc"; // Must not compile
	void operator=(TVR rhs) && noexcept = delete;

	// Cannot use the default move assignment because
	// a) It returns a value allowing accidental use of "=" in if statements instead of ==
	// b) It doesnt check if rhs is assigned although this is
	//    is less important for temporaries which are rarely unassigned.
	//var& operator=(TVR rhs) & noexcept = default;
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

		return;
	}


	///////////////////
	// INT CONSTRUCTORS
	///////////////////

	// var(integer)

#if __cpp_lib_concepts >= 201907L
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
	var(Integer rhs)
		:
		var_int(rhs),
		var_typ(VARTYP_INT) {

		// Prevent overlarge unsigned ints resulting in negative ints
		if (std::is_unsigned<Integer>::value) {
			if (this->var_int < 0)
				[[unlikely]]
				throwNumOverflow(var(__PRETTY_FUNCTION__).field(";", 1));
		}
	}


	//////////////////////////////
	// FLOATING POINT CONSTRUCTORS
	//////////////////////////////

	// var = floating point

#if __cpp_lib_concepts >= 201907L
	template <std::floating_point FloatingPoint>
#else
	template <typename FloatingPoint, std::enable_if_t<std::is_floating_point<FloatingPoint>::value, bool> = true>
#endif
	/*
		float,
		double,
		long double
	*/
	var(FloatingPoint rhs)
		:
		var_dbl(static_cast<double>(rhs)),
		var_typ(VARTYP_DBL) {

		// Prevent overlarge or overnegative long doubles entering var's double
		if (std::is_same<FloatingPoint, long double>::value) {
			if (rhs > VAR_MAX_DOUBLE)
				[[unlikely]]
				throwNumOverflow(var(__PRETTY_FUNCTION__).field(";", 1));
			if (rhs < VAR_LOW_DOUBLE)
				[[unlikely]]
				throwNumUnderflow(var(__PRETTY_FUNCTION__).field(";", 1));
		}
	}


	//////////////////////
	// STRING CONSTRUCTORS
	//////////////////////

	// var = string-like

#if __cpp_lib_concepts >= 201907L
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
	var(StringLike&& fromstr)
		:
		var_str(std::forward<StringLike>(fromstr)),
		var_typ(VARTYP_STR) {

		//std::cerr << "var(str)" << std::endl;
	}

	// memory block
	///////////////
	var(const char* charstart, const size_t nchars)
		:
		var_typ(VARTYP_STR) {

		if (charstart)
			var_str = std::string_view(charstart, nchars);
	}

	// char
	///////
	//must be separate from unsigned int contructor to get a string/char not an int/ character number
	var(const char char1) noexcept
		:
		var_str(1, char1),
		var_typ(VARTYP_STR) {

			//std::cerr << "var(char)" << std::endl;
	}


	////////////////////////////////
	// STRING CONSTRUCTORS FROM WIDE
	////////////////////////////////

	// var(wide-string-like)

#if __cpp_lib_concepts >= 201907L
	template <std_u32string_or_convertible W>
#else
	template <typename W, std::enable_if_t<std::is_convertible<W, std::u32string_view>::value, bool> = true>
#endif
	/*
		std::u32string,
		std::u32string_view,
		u32char*,
		u32char
	*/
	var(W& from_wstr)
		:
		var_typ(VARTYP_STR)	{

		this->from_u32string(from_wstr);
	}

	// const std::wstring&
	//////////////////////
	var(const std::wstring& wstr1);

	// wchar*
	/////////
	var(const wchar_t* wcstr1)
		:
		var_str(var(std::wstring(wcstr1)).var_str),
		var_typ(VARTYP_STR){
	}

// Cant provide these and support L'x' since it they are ambiguous with char when creating var from ints
//
//	// wchar
//  ////////
//	explicit var(wchar_t wchar1)
//		: var_typ(VARTYP_STR) {
//
//		(*this) = var(std::wstring(1, wchar1));
//	}
//
//	// char32_t char
//  ////////////////
//	var(char32_t u32char)
//		: var_typ(VARTYP_STR) {
//
//		this->from_u32string(std::u32string(1, u32char));
//	}
//
//	// char8_t char
//  ///////////////
//	var(char8_t u8char)
//		: var_typ(VARTYP_STR) {
//
//		this->var_str = std::string(1, u8char);
//	}

//	// const std::u32string&
//  ////////////////////////
//	var(const std::u32string&& u32str1);
//	//	this->from_u32string(str1);


	////////////////////////////////////////////
	// GENERAL CONSTRUCTOR FROM INITIALIZER LIST
	////////////////////////////////////////////

	// var{...}

	template <class T>
	// int, double, cstr etc.
	/////////////////////////
	var(std::initializer_list<T> list)
		:
		var_typ(VARTYP_STR) {

		for (auto item : list) {
			(*this) ^= item;
			var_str.push_back(FM_);
		}
		if (!var_str.empty())
			var_str.pop_back();
	}


	///////////////////////
	// NAMED CONVERSIONS TO
	///////////////////////

	bool toBool() const;

	char toChar() const;

	// standard c/c++ int() truncate towards zero i.e. take the number to the left of the point.
	// (whereas pickos is floor i.e. -1.9 -> 2)
	//int64_t toInt() const;
	int toInt() const;

	//long long toLong() const;
	int64_t toInt64() const;

	double toDouble() const;

	const char* c_str() const {
		assertString(__PRETTY_FUNCTION__);
		return var_str.c_str();
	}

	std::wstring to_wstring() const;
	std::u32string to_u32string() const;

	//void from_u32string(std::u32string) const;

	std::string toString() &&;               //for temporary vars

	//Be careful not to retain the reference beyond the scope of the var
	const std::string& toString() const&;  //for non-temporaries.


	//////////////////////////
	// IMPLICIT CONVERSIONS TO
	//////////////////////////

	// someone recommends not to create more than one automatic converter
	// to avoid the compiler error "ambiguous conversion"

	// explicit keyword only works on conversion TO var not conversion FROM var

	// NB const possibly determines priority of choice between automatic conversion
	// and C++ uses bool and int conversion in certain cases to convert to int and bool
	// therefore have chosen to make both bool and int "const" since they dont make
	// any changes to the base object.

	// integer <- var
	/////////////////

	// necessary to allow conversion to int in many functions like extract(x,y,z)

#if __cpp_lib_concepts >= 201907L
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
	operator Integer() const {
		assertInteger(__PRETTY_FUNCTION__);

		// Prevent conversion of negative numbers to unsigned integers
		if (std::is_unsigned<Integer>::value) {
			if (this->var_int < 0)
				[[unlikely]]
				throwNonPositive(__PRETTY_FUNCTION__);
		}
		// Similar code in constructor(int) operator=(int) and int()

		return static_cast<Integer>(var_int);
	}

	// floating point <- var
	////////////////////////

#if __cpp_lib_concepts >= 201907L
	template <std::floating_point FloatingPoint>
#else
	template <typename FloatingPoint, std::enable_if_t<std::is_floating_point<FloatingPoint>::value, bool> = true>
#endif
	/*
		float,
		double,
		long double
	*/
	operator FloatingPoint() const {
		assertDecimal(__PRETTY_FUNCTION__);
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
	operator void*() const {
		// result in an attempt to convert an uninitialised object to void* since there is a bool
		// conversion when does c++ use automatic conversion to void* note that exodus operator !
		// uses (void*) trial elimination of operator void* seems to cause no problems but without
		// full regression testing
		return reinterpret_cast<void*>(this->toBool());
	}

	// bool <- var
	//////////////

	operator bool() const {
		return this->toBool();
	}

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

	operator std::string() const {
		assertString(__PRETTY_FUNCTION__);
		return var_str;
	}

	operator std::string_view() const {
		assertString(__PRETTY_FUNCTION__);
		return std::string_view(var_str);
	}

	operator const char*() const {
		assertString(__PRETTY_FUNCTION__);
		return var_str.c_str();
	}

	operator std::u32string() const {
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

	/////////////////
	// PARENTHESIS ()
	/////////////////

	// extract using () int int int (alternative to .f() and extract())
	// instead of xyz=abc.extract(1,2,3);
	// sadly there is no way to use pick/mv angle brackets like "abc<1,2,3>"
	// and [] brackets would only allow one dimension eg abc[1] (c++23 allows more than one)

	//SADLY no way to get a different operator() function called when on the left hand side of assign
	//http://codepad.org/MzzhlRkb

	//subscript operators often come in pairs
	//
	// 1. const returning a value
	// 2. non-const returning a reference or a proxy object

	// 1. () on const vars will extract the desired field/value/subvalue as a proper var
	// Note that all  function "in" arguments are const vars
	// so will work perfectly with () extraction
	ND var operator()(int fieldno, int valueno = 0, int subvalueno = 0) const {return this->f(fieldno, valueno, subvalueno);}
	//ND var operator()(int fieldno, int valueno = 0, int subvalueno = 0) &&      {return a(fieldno, valueno, subvalueno);}

	// DONT declare this so we force use of the above const version that produces a temporary
	//VARREF operator()(int fieldno, int valueno = 0, int subvalueno = 0);

	// 2. () on non-const vars produces a proxy which can be assigned to or converted to a var implicitly
	// sadly the implicit conversion does not allow OO orientated syntax eg xyz(1,2).oconv("D")
	//
	//  var x = y(3);              will compile and work regardless of if y is const or not
	//  var x = oconv(y(3), "D");  will also compile and work perfectly
	//
	//  var x = y(3).oconv("D");"  will not compile UNLESS y is const e.g function arg of type "in"
	//
	// The proxy object created by (1,2,3) on a non-const var does not have the member function "oconv"
	//
	//
	ND var_proxy1 operator()(int fieldno);
	ND var_proxy2 operator()(int fieldno, int valueno);
	ND var_proxy3 operator()(int fieldno, int valueno, int subvalueno);

	// non-const xxxx(fn,vn,sn) returns a proxy that can be aasigned to or implicitly converted to a var
	//

	/////////////
	// ASSIGNMENT
	/////////////

	// The assignment operators return void to prevent accidental misuse where == was intended.

	// var = Integral
	/////////////////

#if __cpp_lib_concepts >= 201907L
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
	void operator=(Integer rhs) & {

		// Similar code in constructor(int) operator=(int) and int()

		var_int = rhs;
		var_typ = VARTYP_INT;

		// Prevent overlarge unsigned ints resulting in negative ints
		if (std::is_unsigned<Integer>::value) {
			if (this->var_int < 0)
				[[unlikely]]
				throwNumOverflow(var(__PRETTY_FUNCTION__).field(";", 1));
		}

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

#if __cpp_lib_concepts >= 201907L
	template <std::floating_point FloatingPoint>
#else
	template <typename FloatingPoint, std::enable_if_t<std::is_floating_point<FloatingPoint>::value, bool> = true>
#endif
	/*
		float,
		double,
		long double
	*/
	void operator=(FloatingPoint rhs) & {
		var_dbl = rhs;
		var_typ = VARTYP_DBL;
	}

	// var = char
	/////////////

	void operator=(const char char2) & {

		//THISIS("VARREF operator= (const char char2) &")
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

		return;
	}

	// var = char*
	//////////////

	// The assignment operator should always return a reference to *this.
	void operator=(const char* cstr) & {
		//THISIS("VARREF operator= (const char* cstr2) &")
		// protect against unlikely syntax as follows:
		// var undefinedassign=undefinedassign="xxx";
		// this causes crash due to bad memory access due to setting string that doesnt exist
		// slows down all string settings so consider NOT CHECKING in production code
		//THISISDEFINED()

		var_str = cstr;

		// reset to one unique type
		var_typ = VARTYP_STR;

		return;
	}

//	// var = std::string& - lvalue
//	//////////////////////////////
//
//	void operator=(const std::string& string2) & {
//
//		//THISIS("VARREF operator= (const std::string& string2) &")
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
	void operator=(std::string&& string2) & {

		//THISIS("VARREF operator= (const std::string&& string2) &")
		// protect against unlikely syntax as follows:
		// var undefinedassign=undefinedassign=std::string("xxx"";
		// this causes crash due to bad memory access due to setting string that doesnt exist
		// slows down all string settings so consider NOT CHECKING in production code
		//THISISDEFINED()

		var_str = std::forward<std::string>(string2);

		// reset to one unique type
		var_typ = VARTYP_STR;

		return;
	}


	//////////////
	// BRACKETS []
	//////////////

	// Coming in C++23 (already in gcc trunk) - multi-dimensional arrays with more than one index eg [x,y], [x,y,z] etc.
	// and left/right distinction for assign/extract?

	// Extract a character from a constant var
	// first=1 last=-1 etc.
	ND var operator[](int pos1) const {return this->at(pos1);}

	// Named member function identical to operator[]
	ND var at(const int pos1) const;

	// as of now, sadly the following all works EXCEPT that var[99].anymethod() doesnt work
	// so would have to implement all var methods and free functions on the proxy object
	//ND var_brackets_proxy operator[](int pos1);
	//ND var operator[](int pos1) &&;


	////////////////////////
	// SELF ASSIGN OPERATORS
	////////////////////////

	VARREF operator+=(CVR) &;
	VARREF operator*=(CVR) &;
	VARREF operator-=(CVR) &;
	VARREF operator/=(CVR) &;
	VARREF operator%=(CVR) &;
	VARREF operator^=(CVR) &;

	// Specialisations

	// Add
	VARREF operator+=(const int) &;
	VARREF operator+=(const double) &;
	VARREF operator+=(const char  char1) & {(*this) += var(char1); return *this;}
	VARREF operator+=(const char* chars) & {(*this) += var(chars); return *this;}
	VARREF operator+=(const bool) &;

	// Multiply
	VARREF operator*=(const int) &;
	VARREF operator*=(const double) &;
	VARREF operator*=(const char  char1) & {(*this) *= var(char1); return *this;}
	VARREF operator*=(const char* chars) & {(*this) *= var(chars); return *this;}
	VARREF operator*=(const bool) &;

	// Subtract
	VARREF operator-=(const int) &;
	VARREF operator-=(const double) &;
	VARREF operator-=(const char  char1) & {(*this) -= var(char1); return *this;}
	VARREF operator-=(const char* chars) & {(*this) -= var(chars); return *this;}
	VARREF operator-=(const bool) &;

	// Divide
	VARREF operator/=(const int) &;
	VARREF operator/=(const double) &;
	VARREF operator/=(const char  char1) & {(*this) /= var(char1); return *this;}
	VARREF operator/=(const char* chars) & {(*this) /= var(chars); return *this;}
	VARREF operator/=(const bool) &;

	// Modulo
	VARREF operator%=(const int) &;
	VARREF operator%=(const double dbl1) &;
	VARREF operator%=(const char  char1) & {(*this) %= var(char1); return *this;}
	VARREF operator%=(const char* chars) & {(*this) %= var(chars); return *this;}
	VARREF operator%=(const bool  bool1) & {(*this) %= var(bool1); return *this;}

	// Concat
	VARREF operator^=(const int) &;
	VARREF operator^=(const double) &;
	VARREF operator^=(const char) &;
////#define NOT_TEMPLATED_APPEND
//#ifdef NOT_TEMPLATED_APPEND
//	VARREF operator^=(const char*) &;
//	VARREF operator^=(const std::string&) &;
//	VARREF operator^=(SV) &;
//#else
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
//#endif

	///////////////////////////////////////
	// SELF ASSIGN OPERATORS ON TEMPORARIES - all deprecated or deleted to prevent unusual and unnecessary coding
	///////////////////////////////////////

	#define DEPRECATE [[deprecated("Using self assign operators on temporaries is pointless. Use the operator by itself, without the = sign, to achieve the same.")]]

	DEPRECATE VARREF operator+=(CVR rhs) && {(*this) += rhs; return *this;}// = delete;
	DEPRECATE VARREF operator*=(CVR rhs) && {(*this) *= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator-=(CVR rhs) && {(*this) -= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator/=(CVR rhs) && {(*this) /= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator%=(CVR rhs) && {(*this) %= rhs; return *this;}// = delete;
	DEPRECATE VARREF operator^=(CVR rhs) && {(*this) ^= rhs; return *this;}// = delete;

	// Specialisations

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

	//////////////////////
	// INCREMENT/DECREMENT
	//////////////////////

	// increment/decrement as postfix - only allow lvalue
	var operator++(int) &;
	var operator--(int) &;

	// increment/decrement as prefix - only allow lvalue
	VARREF operator++() &;
	VARREF operator--() &;

	//////////////////
	// UNARY OPERATORS
	//////////////////

	var operator+() const;
	var operator-() const;
	bool operator!() const {return !this->toBool();}

	/////////////
	// BINARY OPS
	/////////////

	// declared and defined free friend functions below

	// Note that all or most of these do not actually need to be friends since they either utilise friended helpers
	// or are implemented in terms of self assign operators eg (a+b) is implemented as var(a)+=b
	//
	// they could be defined outide of var class but we leave them in for clarity
	//
	// note that the friend word is required to define free functions inside a class even if they do not need to be friends
	//
	// friend function defined in class scope can be found via argument-dependent lookup

	//////////////////
	// ITERATOR FRIEND
	//////////////////

	friend class var_iter;

	//BEGIN/END - free functions to create iterators over a var
	PUBLIC friend var_iter begin(CVR v);
	PUBLIC friend var_iter end(CVR v);

	friend class dim;
	//friend class var_brackets_proxy;

	///////////////
	// FRIEND UTILS
	///////////////

	// Logical
	PUBLIC friend bool var_eq_var(CVR lhs, CVR rhs);
	PUBLIC friend bool var_lt_var(CVR lhs, CVR rhs);

	// Specialisations for speed

	PUBLIC friend bool var_eq_dbl(CVR           lhs,   const double dbl1 );
	PUBLIC friend bool var_eq_int(CVR           lhs,   const int    int1 );
	PUBLIC friend bool var_eq_bool(CVR          lhs,   const bool   bool1);

	PUBLIC friend bool var_lt_int(CVR           lhs,   const int    int1 );
	PUBLIC friend bool int_lt_var(const int     int1,  CVR          rhs  );

	PUBLIC friend bool var_lt_dbl(CVR           lhs,   const double dbl1 );
	PUBLIC friend bool dbl_lt_var(const double  dbl1,  CVR          rhs  );

	PUBLIC friend bool var_lt_bool(CVR          lhs,   const bool   bool1) = delete;
	PUBLIC friend bool bool_lt_var(const bool   bool1, CVR          rhs  ) = delete;

	// Concatenation

	PUBLIC friend var var_cat_var(CVR         lhs,  CVR         rhs);

	// Specialisations for speed

	PUBLIC friend var var_cat_cstr(CVR         lhs,  const char* cstr);
	PUBLIC friend var var_cat_char(CVR         lhs,  const char  char2);
	PUBLIC friend var cstr_cat_var(const char* cstr, CVR         rhs);

	friend var operator""_var(const char* cstr, std::size_t size);

	//////////////////////////////
	// OPERATOR FRIENDS - pure var
	//////////////////////////////

	// Logical friends for var and var

	PUBLIC friend bool operator==(CVR lhs, CVR rhs) {return  var_eq_var(lhs, rhs );}
#if !(__GNUG__ >= 11 || __clang_major__ >= 14)
	PUBLIC friend bool operator!=(CVR lhs, CVR rhs) {return !var_eq_var(lhs, rhs );}
#endif
	PUBLIC friend bool operator< (CVR lhs, CVR rhs) {return  var_lt_var(lhs, rhs );}
	PUBLIC friend bool operator>=(CVR lhs, CVR rhs) {return !var_lt_var(lhs, rhs );}
	PUBLIC friend bool operator> (CVR lhs, CVR rhs) {return  var_lt_var(rhs, lhs );}
	PUBLIC friend bool operator<=(CVR lhs, CVR rhs) {return !var_lt_var(rhs, lhs );}

	// Arithmetic friends for var and var (not templated, to prevent "error: ambiguous overload"

	PUBLIC friend var  operator+(CVR  lhs, CVR rhs) {var nrvo = lhs.clone(); nrvo += rhs; return nrvo;}
	PUBLIC friend var  operator*(CVR  lhs, CVR rhs) {var nrvo = lhs.clone(); nrvo *= rhs; return nrvo;}
	PUBLIC friend var  operator-(CVR  lhs, CVR rhs) {var nrvo = lhs.clone(); nrvo -= rhs; return nrvo;}
	PUBLIC friend var  operator/(CVR  lhs, CVR rhs) {var nrvo = lhs.clone(); nrvo /= rhs; return nrvo;}
	PUBLIC friend var  operator%(CVR  lhs, CVR rhs) {var nrvo = lhs.clone(); nrvo %= rhs; return nrvo;}

	// Concatenation friends for var and var

	PUBLIC friend var operator^(CVR lhs, CVR rhs) {return var_cat_var(lhs, rhs ); }
	PUBLIC friend var operator^(TVR lhs, CVR rhs) {return lhs ^= rhs ; }


	/////////////////////////////////
	// OPERATOR FRIENDS - mixed types
	/////////////////////////////////

	// LOGICAL friends v. main types

	//== and !=
	//TODO consider replacing by operator<=> and replacing var_eq_var and var_lt by MVcmp
	//or provide more specialisations of var_eq_var and var_lt esp. for numeric types

	// == EQ friends v. main types

	template<class RHS>
	PUBLIC friend bool operator==(CVR          lhs,   RHS          rhs   ) {return  var_eq_var(  lhs, rhs    ); }
	//PUBLIC friend bool operator==(CVR        lhs,   const char*  cstr2 ) {return  var_eq_var(  lhs, cstr2  ); }
	//PUBLIC friend bool operator==(CVR        lhs,   const char   char2 ) {return  var_eq_var(  lhs, char2  ); }
	PUBLIC friend bool operator==(CVR          lhs,   const int    int2  ) {return  var_eq_int(  lhs, int2   ); }
	PUBLIC friend bool operator==(CVR          lhs,   const double dbl2  ) {return  var_eq_dbl(  lhs, dbl2   ); }
	PUBLIC friend bool operator==(CVR          lhs,   const bool   bool2 ) {return  var_eq_bool( lhs, bool2  ); }

	template<class LHS>
	PUBLIC friend bool operator==(LHS lhs,            CVR          rhs   ) {return  var_eq_var(  rhs, lhs    ); }
	//PUBLIC friend bool operator==(const char*cstr1, CVR          rhs   ) {return  var_eq_var(  rhs, cstr1  ); }
	//PUBLIC friend bool operator==(const char char1, CVR          rhs   ) {return  var_eq_var(  rhs, char1  ); }
	PUBLIC friend bool operator==(const int    int1,  CVR          rhs   ) {return  var_eq_int(  rhs, int1   ); }
	PUBLIC friend bool operator==(const double dbl1,  CVR          rhs   ) {return  var_eq_dbl(  rhs, dbl1   ); }
	PUBLIC friend bool operator==(const bool   bool1, CVR          rhs   ) {return  var_eq_bool( rhs, bool1  ); }

	// != NE friends v. main types
#if !(__GNUG__ >= 11 || __clang_major__ >= 14)
	template<class RHS>
	PUBLIC friend bool operator!=(CVR          lhs,   RHS          rhs   ) {return !var_eq_var( lhs, rhs     ); }
	//PUBLIC friend bool operator!=(CVR        lhs,   const char*  cstr2 ) {return !var_eq_var(  lhs, cstr2  ); }
	//PUBLIC friend bool operator!=(CVR        lhs,   const char   char2 ) {return !var_eq_var(  lhs, char2  ); }
	PUBLIC friend bool operator!=(CVR          lhs,   const int    int2  ) {return !var_eq_int(  lhs, int2   ); }
	PUBLIC friend bool operator!=(CVR          lhs,   const double dbl2  ) {return !var_eq_dbl(  lhs, dbl2   ); }
	PUBLIC friend bool operator!=(CVR          lhs,   const bool   bool2 ) {return !var_eq_bool( lhs, bool2  ); }

	template<class LHS>
	PUBLIC friend bool operator!=(LHS lhs,            CVR          rhs   ) {return !var_eq_var(  rhs, lhs    ); }
	//PUBLIC friend bool operator!=(const char*cstr1, CVR          rhs   ) {return !var_eq_var(  rhs, cstr1  ); }
	//PUBLIC friend bool operator!=(const char char1, CVR          rhs   ) {return !var_eq_var(  rhs, char1  ); }
	PUBLIC friend bool operator!=(const int    int1,  CVR          rhs   ) {return !var_eq_int(  rhs, int1   ); }
	PUBLIC friend bool operator!=(const double dbl1,  CVR          rhs   ) {return !var_eq_dbl(  rhs, dbl1   ); }
	PUBLIC friend bool operator!=(const bool   bool1, CVR          rhs   ) {return !var_eq_bool( rhs, bool1  ); }
#endif
	// < LT friends v. main types

	template<class RHS>
	PUBLIC friend bool operator<(CVR           lhs,   RHS          rhs   ) {return  var_lt_var(lhs,   rhs   ); }
	//PUBLIC friend bool operator<(CVR         lhs,   const char*  cstr2 ) {return  var_lt_var(lhs,   cstr2 ); }
	//PUBLIC friend bool operator<(CVR         lhs,   const char   char2 ) {return  var_lt_var(lhs,   char2 ); }
	PUBLIC friend bool operator<(CVR           lhs,   const int    int2  ) {return  var_lt_int(lhs,   int2  ); }
	PUBLIC friend bool operator<(CVR           lhs,   const double dbl2  ) {return  var_lt_dbl(lhs,   dbl2  ); }
	//PUBLIC friend bool operator<(CVR         lhs,   const bool   bool1 ) {return  bool_lt_bool(lhs, bool1 ); }

	template<class LHS>
	PUBLIC friend bool operator<(LHS lhs,             CVR          rhs   ) {return  var_lt_var(lhs,   rhs   ); }
	//PUBLIC friend bool operator<(const char* cstr1, CVR          rhs   ) {return  var_lt_var(cstr1, rhs   ); }
	//PUBLIC friend bool operator<(const char  char1, CVR          rhs   ) {return  var_lt_var(char1, rhs   ); }
	PUBLIC friend bool operator<(const int     int1,  CVR          rhs   ) {return  int_lt_var(int1,  rhs   ); }
	PUBLIC friend bool operator<(const double  dbl1,  CVR          rhs   ) {return  dbl_lt_var(dbl1,  rhs   ); }
	//PUBLIC friend bool operator<(const bool  bool1, CVR          rhs   ) {return  bool_lt_bool(bool1, rhs ); }

	// >= GE friends v. main types

	template<class RHS>
	PUBLIC friend bool operator>=(CVR          lhs,   RHS          rhs   ) {return !var_lt_var(lhs,   rhs   ); }
	//PUBLIC friend bool operator>=(CVR        lhs,   const char*  cstr2 ) {return !var_lt_var(lhs,   cstr2 ); }
	//PUBLIC friend bool operator>=(CVR        lhs,   const char   char2 ) {return !var_lt_var(lhs,   char2 ); }
	PUBLIC friend bool operator>=(CVR          lhs,   const int    int2  ) {return !var_lt_int(lhs,   int2  ); }
	PUBLIC friend bool operator>=(CVR          lhs,   const double dbl2  ) {return !var_lt_dbl(lhs,   dbl2  ); }
	//PUBLIC friend bool operator>=(CVR        lhs,   const bool   bool2 ) {return !bool_lt_bool(lhs, bool2 ); }

	template<class LHS>
	PUBLIC friend bool operator>=(LHS lhs,            CVR          rhs   ) {return !var_lt_var(lhs,     rhs   ); }
	//PUBLIC friend bool operator>=(const char*cstr1, CVR          rhs   ) {return !var_lt_var(cstr1,   rhs   ); }
	//PUBLIC friend bool operator>=(const char char1, CVR          rhs   ) {return !var_lt_var(char1,   rhs   ); }
	PUBLIC friend bool operator>=(const int    int1,  CVR          rhs   ) {return !int_lt_var(int1,    rhs   ); }
	PUBLIC friend bool operator>=(const double dbl1,  CVR          rhs   ) {return !dbl_lt_var(dbl1,    rhs   ); }
	//PUBLIC friend bool operator>=(const bool bool1, CVR          rhs   ) {return !bool_lt_bool(bool1, rhs   ); }

	// > GT friends v. main types

	template<class RHS>
	PUBLIC friend bool operator>(CVR           lhs,   RHS          rhs   ) {return  var_lt_var(rhs,     lhs   ); }
	//PUBLIC friend bool operator>(CVR         lhs,   const char*  cstr2 ) {return  var_lt_var(cstr2,   lhs   ); }
	//PUBLIC friend bool operator>(CVR         lhs,   const char   char2 ) {return  var_lt_var(char2,   lhs   ); }
	PUBLIC friend bool operator>(CVR           lhs,   const int    int2  ) {return  int_lt_var(int2,    lhs   ); }
	PUBLIC friend bool operator>(CVR           lhs,   const double dbl2  ) {return  dbl_lt_var(dbl2,    lhs   ); }
	//PUBLIC friend bool operator>(CVR         lhs,   const bool   bool2 ) {return  bool_lt_bool(bool2, lhs   ); }

	template<class LHS>
	PUBLIC friend bool operator>(LHS lhs,             CVR          rhs   ) {return  var_lt_var(rhs,   lhs   ); }
	//PUBLIC friend bool operator>(const char* cstr1, CVR          rhs   ) {return  var_lt_var(rhs,   cstr1 ); }
	//PUBLIC friend bool operator>(const char  char1, CVR          rhs   ) {return  var_lt_var(rhs,   char1 ); }
	PUBLIC friend bool operator>(const int     int1,  CVR          rhs   ) {return  var_lt_int(rhs,   int1  ); }
	PUBLIC friend bool operator>(const double  dbl1,  CVR          rhs   ) {return  var_lt_dbl(rhs,   dbl1  ); }
	//PUBLIC friend bool operator>(const bool  bool1, CVR          rhs   ) {return  bool_lt_bool(rhs, bool1 ); }

	// <= LE friends v. main types

	template<class RHS>
	PUBLIC friend bool operator<=(CVR          lhs,   RHS          rhs   ) {return !var_lt_var(rhs,     lhs   ); }
	//PUBLIC friend bool operator<=(CVR        lhs,   const char*  cstr2 ) {return !var_lt_var(cstr2,   lhs   ); }
	//PUBLIC friend bool operator<=(CVR        lhs,   const char   char2 ) {return !var_lt_var(char2,   lhs   ); }
	PUBLIC friend bool operator<=(CVR          lhs,   const int    int2  ) {return !int_lt_var(int2,    lhs   ); }
	PUBLIC friend bool operator<=(CVR          lhs,   const double dbl2  ) {return !dbl_lt_var(dbl2,    lhs   ); }
	//PUBLIC friend bool operator<=(CVR        lhs,   const bool   bool2 ) {return !bool_lt_bool(bool2, lhs   ); }

	template<class LHS>
	PUBLIC friend bool operator<=(LHS           lhs,   CVR          rhs   ) {return !var_lt_var(rhs,   lhs   ); }
	//PUBLIC friend bool operator<=(const char* cstr1, CVR          rhs   ) {return !var_lt_var(rhs,   cstr1 ); }
	//PUBLIC friend bool operator<=(const char  char1, CVR          rhs   ) {return !var_lt_var(rhs,   char1 ); }
	PUBLIC friend bool operator<=(const int     int1,  CVR          rhs   ) {return !var_lt_int(rhs,   int1  ); }
	PUBLIC friend bool operator<=(const double  dbl1,  CVR          rhs   ) {return !var_lt_dbl(rhs,   dbl1  ); }
	//PUBLIC friend bool operator<=(const bool  bool1, CVR          rhs   ) {return !bool_lt_bool(rhs, bool1 ); }

	// ARITHMETIC friends v. main types

	// PLUS  friends v. main types

#if 1
	template<class T> PUBLIC friend var operator+(CVR lhs, const T rhs) {var nrvo = lhs.clone(); nrvo += rhs; return nrvo;}
	template<class T> PUBLIC friend var operator-(CVR lhs, const T rhs) {var nrvo = lhs.clone(); nrvo -= rhs; return nrvo;}
	template<class T> PUBLIC friend var operator*(CVR lhs, const T rhs) {var nrvo = lhs.clone(); nrvo *= rhs; return nrvo;}
	template<class T> PUBLIC friend var operator/(CVR lhs, const T rhs) {var nrvo = lhs.clone(); nrvo /= rhs; return nrvo;}
	template<class T> PUBLIC friend var operator%(CVR lhs, const T rhs) {var nrvo = lhs.clone(); nrvo %= rhs; return nrvo;}

	template<class T> PUBLIC friend var operator+(const T lhs, CVR rhs) {var nrvo = rhs.clone(); nrvo += lhs; return nrvo;}
	template<class T> PUBLIC friend var operator*(const T lhs, CVR rhs) {var nrvo = rhs.clone(); nrvo *= lhs; return nrvo;}
	template<class T> PUBLIC friend var operator-(const T lhs, CVR rhs) {var nrvo = -rhs;        nrvo += lhs; return nrvo;}
	template<class T> PUBLIC friend var operator/(const T lhs, CVR rhs) {var nrvo =  lhs;        nrvo /= rhs; return nrvo;}
	template<class T> PUBLIC friend var operator%(const T lhs, CVR rhs) {var nrvo =  lhs;        nrvo %= rhs; return nrvo;}
#else
//	PUBLIC friend var operator+(CVR            lhs,   const char*  cstr2 ) {var nrvo = lhs.clone(); nrvo    += cstr2; return nrvo;}
//	PUBLIC friend var operator+(CVR            lhs,   const char   char2 ) {var nrvo = lhs.clone(); nrvo    += char2; return nrvo;}
//	PUBLIC friend var operator+(CVR            lhs,   const int    int2  ) {var nrvo = lhs.clone(); nrvo    += int2;  return nrvo;}
//	PUBLIC friend var operator+(CVR            lhs,   const double dbl2  ) {var nrvo = lhs.clone(); nrvo    += dbl2;  return nrvo;}
//	//PUBLIC friend var operator+(const bool     bool1, CVR          rhs   ) {var nrvo = rhs.clone(); nrvo    += bool1; return nrvo;}
//	PUBLIC friend var operator+(CVR            lhs,   const bool   bool2 ) {if (bool2) return lhs + 1; else return lhs;}
//
//	PUBLIC friend var operator+(const char*    cstr1, CVR          rhs   ) {var nrvo = rhs.clone(); nrvo    += cstr1; return nrvo;}
//	PUBLIC friend var operator+(const char     char1, CVR          rhs   ) {var nrvo = rhs.clone(); nrvo    += char1; return nrvo;}
//	PUBLIC friend var operator+(const int      int1,  CVR          rhs   ) {var nrvo = rhs.clone(); nrvo    += int1;  return nrvo;}
//	PUBLIC friend var operator+(const double   dbl1,  CVR          rhs   ) {var nrvo = rhs.clone(); nrvo    += dbl1;  return nrvo;}
//	//PUBLIC friend var operator+(const bool     bool1, CVR          rhs   ) {var nrvo = rhs.clone(); nrvo    += bool1; return nrvo;}
//	PUBLIC friend var operator+(const bool     bool1, CVR          rhs   ) {if (bool1) return rhs + 1; else return rhs;}
//
//	// MULTIPLY  friends v. main types
//
//	PUBLIC friend var operator*(CVR            lhs,   const char*  cstr2 ) {var nrvo = lhs.clone(); nrvo    *= cstr2; return nrvo;}
//	PUBLIC friend var operator*(CVR            lhs,   const char   char2 ) {var nrvo = lhs.clone(); nrvo    *= char2; return nrvo;}
//	PUBLIC friend var operator*(CVR            lhs,   const int    int2  ) {var nrvo = lhs.clone(); nrvo    *= int2;  return nrvo;}
//	PUBLIC friend var operator*(CVR            lhs,   const double dbl2  ) {var nrvo = lhs.clone(); nrvo    *= dbl2;  return nrvo;}
//	//PUBLIC friend var operator*(CVR            lhs,   const bool   bool2 ) {var nrvo = lhs.clone(); nrvo    *= bool2; return nrvo;}
//	PUBLIC friend var operator*(CVR            lhs,   const bool   bool2 ) {if (bool2) return lhs; else return 0;}
//
//	PUBLIC friend var operator*(const char*    cstr1, CVR          rhs   ) {var nrvo = rhs.clone(); nrvo    *= cstr1; return nrvo;}
//	PUBLIC friend var operator*(const char     char1, CVR          rhs   ) {var nrvo = rhs.clone(); nrvo    *= char1; return nrvo;}
//	PUBLIC friend var operator*(const int      int1,  CVR          rhs   ) {var nrvo = rhs.clone(); nrvo    *= int1;  return nrvo;}
//	PUBLIC friend var operator*(const double   dbl1,  CVR          rhs   ) {var nrvo = rhs.clone(); nrvo    *= dbl1;  return nrvo;}
//	//PUBLIC friend var operator*(const bool     bool1, CVR          rhs   ) {var nrvo = rhs.clone(); nrvo    *= bool1; return nrvo;}
//	PUBLIC friend var operator*(const bool     bool1, CVR          rhs   ) {if (bool1) return rhs; else return 0;}
//
//	// MINUS  friends v. main types
//
//	PUBLIC friend var operator-(CVR            lhs,   const char*  cstr2 ) {var nrvo = lhs.clone();   nrvo  -= cstr2; return nrvo;}
//	PUBLIC friend var operator-(CVR            lhs,   const char   char2 ) {var nrvo = lhs.clone();   nrvo  -= char2; return nrvo;}
//	PUBLIC friend var operator-(CVR            lhs,   const int    int2  ) {var nrvo = lhs.clone();   nrvo  -= int2;  return nrvo;}
//	PUBLIC friend var operator-(CVR            lhs,   const double dbl2  ) {var nrvo = lhs.clone();   nrvo  -= dbl2;  return nrvo;}
//	//PUBLIC friend var operator-(CVR            lhs,   const bool   bool2 ) {var nrvo = lhs.clone();   nrvo  -= bool2; return nrvo;}
//	//PUBLIC friend var operator-(CVR            lhs,   const bool   bool2 ) {var nrvo = lhs.clone();   if (bool2) nrvo  -= 1; return nrvo;}
//	PUBLIC friend var operator-(CVR            lhs,   const bool   bool2 ) {if (bool2) return lhs - 1; else return lhs;}
//
////	PUBLIC friend var operator-(const char*    cstr1, CVR          rhs   ) {var nrvo = cstr1; nrvo  -= rhs;   return nrvo;}
////	PUBLIC friend var operator-(const char     char1, CVR          rhs   ) {var nrvo = char1; nrvo  -= rhs;   return nrvo;}
////	PUBLIC friend var operator-(const int      int1,  CVR          rhs   ) {var nrvo = int1;  nrvo  -= rhs;   return nrvo;}
////	PUBLIC friend var operator-(const double   dbl1,  CVR          rhs   ) {var nrvo = dbl1;  nrvo  -= rhs;   return nrvo;}
////	PUBLIC friend var operator-(const bool     bool1, CVR          rhs   ) {var nrvo = bool1; nrvo  -= rhs;   return nrvo;}
//
////	PUBLIC friend var operator-(const char*    cstr1, CVR          rhs   ) {return -rhs + cstr1;}
////	PUBLIC friend var operator-(const char     char1, CVR          rhs   ) {return -rhs + char1;}
////	PUBLIC friend var operator-(const int      int1,  CVR          rhs   ) {return -rhs + int1;}
////	PUBLIC friend var operator-(const double   dbl1,  CVR          rhs   ) {return -rhs + dbl1;}
////	PUBLIC friend var operator-(const bool     bool1, CVR          rhs   ) {return -rhs + bool;}
//
//	PUBLIC friend var operator-(const char*    cstr1, CVR          rhs   ) {var nrvo = -rhs; nrvo += cstr1; return nrvo;}
//	PUBLIC friend var operator-(const char     char1, CVR          rhs   ) {var nrvo = -rhs; nrvo += char1; return nrvo;}
//	PUBLIC friend var operator-(const int      int1,  CVR          rhs   ) {var nrvo = -rhs; nrvo += int1;  return nrvo;}
//	PUBLIC friend var operator-(const double   dbl1,  CVR          rhs   ) {var nrvo = -rhs; nrvo += dbl1;  return nrvo;}
//	//PUBLIC friend var operator-(const bool     bool1, CVR          rhs   ) {var nrvo = -rhs; nrvo += bool1; return nrvo;}
//	PUBLIC friend var operator-(const bool     bool1, CVR          rhs   ) {if (bool1) return (-rhs) + 1; else return -rhs;}
//
//	// DIVIDE  friends v. main types
//
//	PUBLIC friend var operator/(CVR            lhs,   const char*  cstr2 ) {var nrvo = lhs.clone();   nrvo  /= cstr2; return nrvo;}
//	PUBLIC friend var operator/(CVR            lhs,   const char   char2 ) {var nrvo = lhs.clone();   nrvo  /= char2; return nrvo;}
//	PUBLIC friend var operator/(CVR            lhs,   const int    int2  ) {var nrvo = lhs.clone();   nrvo  /= int2;  return nrvo;}
//	PUBLIC friend var operator/(CVR            lhs,   const double dbl2  ) {var nrvo = lhs.clone();   nrvo  /= dbl2;  return nrvo;}
////	PUBLIC friend var operator/(CVR            lhs,   const bool   bool2 ) {var nrvo = lhs.clone();   nrvo  /= bool2; return nrvo;} // Either does nothing or throws divide by zero
//
//	PUBLIC friend var operator/(const char*    cstr1, CVR          rhs   ) {var nrvo = cstr1; nrvo  /= rhs;   return nrvo;}
//	PUBLIC friend var operator/(const char     char1, CVR          rhs   ) {var nrvo = char1; nrvo  /= rhs;   return nrvo;}
//	PUBLIC friend var operator/(const int      int1,  CVR          rhs   ) {var nrvo = int1;  nrvo  /= rhs;   return nrvo;}
//	PUBLIC friend var operator/(const double   dbl1,  CVR          rhs   ) {var nrvo = dbl1;  nrvo  /= rhs;   return nrvo;}
////	PUBLIC friend var operator/(const bool     bool1, CVR          rhs   ) {var nrvo = bool1 ;nrvo /= rhs;   return nrvo;} // Almost meaningless
//
//	// MODULO  friends v. main types
//
//	PUBLIC friend var operator%(CVR            lhs,   const char*  cstr2 ) {var nrvo = lhs.clone(); nrvo  %= cstr2; return nrvo;}
//	PUBLIC friend var operator%(CVR            lhs,   const char   char2 ) {var nrvo = lhs.clone(); nrvo  %= char2; return nrvo;}
//	PUBLIC friend var operator%(CVR            lhs,   const int    int2  ) {var nrvo = lhs.clone(); nrvo  %= int2;  return nrvo;}
//	PUBLIC friend var operator%(CVR            lhs,   const double dbl2  ) {var nrvo = lhs.clone(); nrvo  %= dbl2;  return nrvo;}
////	PUBLIC friend var operator%(CVR            lhs,   const bool   bool2 ) {var nrvo = lhs.clone();   nrvo  %= bool2; return nrvo;} // Rather useless or throws divide by zero
//
//	PUBLIC friend var operator%(const char*    cstr1, CVR          rhs   ) {var nrvo = cstr1; nrvo  %= rhs;   return nrvo;}
//	PUBLIC friend var operator%(const char     char1, CVR          rhs   ) {var nrvo = char1; nrvo  %= rhs;   return nrvo;}
//	PUBLIC friend var operator%(const int      int1,  CVR          rhs   ) {var nrvo = int1;  nrvo  %= rhs;   return nrvo;}
//	PUBLIC friend var operator%(const double   dbl1,  CVR          rhs   ) {var nrvo = dbl1;  nrvo  %= rhs;   return nrvo;}
//	//PUBLIC friend var operator%(const bool   bool1, CVR          rhs   ) {var nrvo = bool1 ;nrvo  %= rhs;   return nrvo;} // Almost meaningless
//
#endif

	// STRING CONCATENATE  friends v. main types

	// NB do *NOT* support concatenate with bool or vice versa!
	// to avoid compiler doing wrong precendence issue between ^ and logical operators
	//remove this to avoid some gcc ambiguous warnings although it means concat std::string will create a temp var
	//PUBLIC friend var operator^(CVR          lhs,   const std::string str2 ) {return var_cat_var(lhs,    var(str2) ); }
	PUBLIC friend var operator^(CVR          lhs,   const char*  cstr ) {return var_cat_cstr(lhs,   cstr );}
	PUBLIC friend var operator^(CVR          lhs,   const char   char2) {return var_cat_char(lhs,   char2);}
	PUBLIC friend var operator^(CVR          lhs,   const int    int2 ) {return var_cat_var( lhs,   int2 );}
	PUBLIC friend var operator^(CVR          lhs,   const double dbl2 ) {return var_cat_var( lhs,   dbl2 );}
	PUBLIC friend var operator^(const char*  cstr,  CVR          rhs  ) {return cstr_cat_var(cstr,  rhs  );}
	PUBLIC friend var operator^(const char   char1, CVR          rhs  ) {return var_cat_var( char1, rhs  );}
	PUBLIC friend var operator^(const int    int1,  CVR          rhs  ) {return var_cat_var( int1,  rhs  );}
	PUBLIC friend var operator^(const double dbl1,  CVR          rhs  ) {return var_cat_var( dbl1,  rhs  );}

	//temporaries (rvalues) - mutate the temporary string to save a copy
	PUBLIC friend var operator^(TVR          lhs,   const char*  cstr2 ) {return lhs ^= cstr2;}
	PUBLIC friend var operator^(TVR          lhs,   const char   char2 ) {return lhs ^= char2;}
	PUBLIC friend var operator^(TVR          lhs,   const int    int2  ) {return lhs ^= int2 ;}
	PUBLIC friend var operator^(TVR          lhs,   const double dbl2  ) {return lhs ^= dbl2 ;}

	// OUTPUT
	/////////

	// To stdout/cout buffered
	CVR output() const;      // stdout no new line, buffered
	CVR outputl() const;     // stdout starts a new line, flushed
	CVR outputt() const;     // stdout adds a tab, buffered

	// As above but with a prefix
	CVR output(CVR var1) const;  // stdout with a prefix, no new line, buffered
	CVR outputl(CVR var1) const; // stdout with a prefix, starts a new line, flushed
	CVR outputt(CVR var1) const; // stdout with a prefix, adds a tab, buffered

	// To stdlog/clog buffered
	CVR logput() const;  // stdlog no new line, buffered
	CVR logputl() const; // stdlog starts a new line, flushed

	// As above but with a prefix
	CVR logput(CVR var1) const;  // stdlog with a prefix, no new line, buffered
	CVR logputl(CVR var1) const; // stdlog with a prefix, starts a new line, flushed

	// To stderr/cerr usually unbuffered
	CVR errput() const;  // stderr no new line, flushed
	CVR errputl() const; // stderr starts a new line, flushed

	// As above but with a prefix
	CVR errput(CVR var1) const;  // stderr with a prefix, no new line, flushed
	CVR errputl(CVR var1) const; // stderr with a prefix, starts a new line, flushed

	// Output to a given stream
	CVR put(std::ostream& ostream1) const;

	// STANDARD INPUT
	/////////////////

	VARREF input();
	VARREF input(CVR prompt);
	VARREF inputn(const int nchars);

	ND bool isterminal() const;
	ND bool hasinput(int milliseconds = 0) const;
	ND bool eof() const;
	bool echo(const int on_off) const;

	void breakon() const;
	void breakoff() const;

	// IO STREAM FRIENDS
	////////////////////

	PUBLIC friend std::istream& operator>>(std::istream& istream1, VARREF var1);

	PUBLIC friend std::ostream& operator<<(std::ostream& ostream1, var var1);
	//causes ambiguous overload for some unknown reason despite being a hidden friend
	//friend std::ostream& operator<<(std::ostream& ostream1, TVR var1);

	// friend bool operator<<(CVR);

	// VARIABLE CONTROL
	///////////////////

	ND bool assigned() const;
	ND bool unassigned() const;
	void default_to(CVR defaultvalue);
	var default_from(CVR defaultvalue) const;

	VARREF move(VARREF destinationvar);
	// swap is marked as const despite it replaceping the var with var2
	// Currently this is required in rare cases where functions like exoprog::calculate
	// temporarily require member variables to be something else but switch back before exiting
	// if such function throws then it would leave the member variables in a changed state.
	CVR swap(CVR var2) const;//version that works on const vars
	VARREF swap(VARREF var2);//version that works on non-const vars
	var clone() const {
		var rvo;
		rvo.var_typ = var_typ;
		rvo.var_str = var_str;

		// Avoid copying int and dbl in case cloning an unassigned var
		// since default ctor var() = default and int/dbl are not initialised to zero.
		//
		// Strategy is as follows:
		//
		// Cloning uninitialised data should leave the target uninitialised as well
		//
		// If var implementation ever changes so that int/dbl must be cloned
		// even if vartype is unassigned then int/dbl will have to be default initialised
		// to something, probably zero.
		//
		// Avoid triggering a compiler warning
		// warning: ‘<anonymous>.exodus::var::var_dbl’ may be used uninitialized in this function [-Wmaybe-uninitialized]
		if (var_typ) {
			rvo.var_int = var_int;
			rvo.var_dbl = var_dbl;
		}

		return rvo;
	}

	ND var dump() const;

	// MATH/BOOLEAN
	///////////////

	ND var abs() const;
	ND var mod(CVR divisor) const;
	ND var mod(double divisor) const;
	ND var mod(const int divisor) const;
	ND var pwr(CVR exponent) const;
	ND var rnd() const;
	void initrnd() const;
	ND var exp() const;
	ND var sqrt() const;
	ND var sin() const;
	ND var cos() const;
	ND var tan() const;
	ND var atan() const;
	ND var loge() const;
	//ND var int() const;//reserved word
	ND var integer() const;//returns a var in case you need a var and not an int  which the c++ built-in int(varx) produces
	ND var floor() const;
	ND var round(const int ndecimals = 0) const;

	// LOCALE
	/////////

	bool setxlocale() const;
	ND VARREF getxlocale();

	// STRING CREATION
	//////////////////

	// var chr() const;
	// version 1 chr - only char 0 - 255 returned in a single byte
	// bytes 128-255 are not valid utf-8 so cannot be written to database/postgres
	ND var chr(const int num) const;  // ASCII

	// version 2 textchr - returns utf8 byte sequences for all unicode code points
	// not unsigned int so to get utf codepoints > 2^63 must provide negative ints
	// not providing implicit constructor from var to unsigned int due to getting ambigious conversions
	// since int and unsigned int are parallel priority in c++ implicit conversions
	ND var textchr(const int num) const;  // UTF8

	ND var str(const int num) const;
	ND var space() const;

	ND var numberinwords(CVR languagename_or_locale_id DEFAULT_EMPTY);

	////////////
	// STRING // All utf8 unless mentioned
	////////////

	// STRING INFO
	//////////////

	ND var seq() const;     // byte
	ND var textseq() const;
	ND var len() const;     // bytes
	ND var textwidth() const;     // number of output columns. Allows multi column unicode and reduces combining characters etc.
	bool isnum() const;     // integer or floating point. optional prefix -, + disallowed, solitary . and - not allowed. Empty string is numeric 0

	// STRING SCANNING
	//////////////////

	ND var textlen() const;
	ND var fcount(SV str) const;
	ND var count(SV str) const;
	ND var match(SV str, SV options DEFAULT_EMPTY) const;

	//                                  Javascript   PHP             Python       Go          Rust          C++
	ND bool starts(SV str) const;    // startsWith() str_starts_with startswith() HasPrefix() starts_with() starts_with
	ND bool ends(SV str) const;      // endsWith     str_ends_with   endswith     HasSuffix() ends_with()   ends_with
	ND bool contains(SV str) const;  // includes()   str_contains    contains()   Contains()  contains()    contains

	//https://en.wikipedia.org/wiki/Comparison_of_programming_languages_(string_functions)#Find
	//ND var index(SV str) const;
	ND var index(SV str, const int startchar1 = 1) const;   // byte returned and startchar1
	ND var indexn(SV str, const int occurrence) const;      // byte returned
	ND var indexr(SV str, const int startchar1 = -1) const; // byte returned

	//static member for speed on std strings because of underlying boost implementation
	static int localeAwareCompare(const std::string& str1, const std::string& str2);
	//int localeAwareCompare(const std::string& str2) const;

	// STRING CONVERSION - Non-mutating
	////////////////////

	ND var ucase() const&;
	ND var lcase() const&;
	ND var tcase() const&;
	ND var fcase() const&;
	ND var normalize() const&;
	ND var invert() const&;

	ND var lower() const&;
	ND var raise() const&;
	ND var crop() const&;

	ND var quote() const&;
	ND var squote() const&;
	ND var unquote() const&;

	ND var trim(SV trimchars  DEFAULT_SPACE) const&;     // byte trimchars
	ND var trimfirst(SV trimchars DEFAULT_SPACE) const&; // byte trimchars
	ND var trimlast(SV trimchars DEFAULT_SPACE) const&;  // byte trimchars
	ND var trimboth(SV trimchars DEFAULT_SPACE) const&;  // byte trimchars

	ND var first(const size_t length) const&; // byte length
	ND var last(const size_t length) const&;  // byte length
	ND var cut(const int length) const&;      // byte length
	ND var paste(const int pos1, const int length, SV insertstr) const&; // byte pos1, length
	ND var paste(const int pos1, SV insertstr) const&; // byte pos1
	ND var prefix(SV insertstr) const&;
	//ND var append(SV appendstr) const&;
	ND var pop() const&;                      // byte removed

	// var.fieldstore(separator,fieldno,nfields,replacement)
	ND var fieldstore(SV separator, const int fieldno, const int nfields, CVR replacement) const&;

	//ND var substr(const int pos1, const int length) const&; // byte pos1, length
	ND var substr(const int startindex1, const int length) const&;
	//ND var substr(const int pos1) const&;                   // byte pos1
	ND var substr(const int startindex1) const&;

	ND var b(const int pos1, const int length) const&; // byte pos1, length
	ND var b(const int pos1) const&; // byte pos1

	ND var convert(SV fromchars, SV tochars) const&;        // byte fromchars, tochars
	ND var textconvert(SV fromchars, SV tochars) const&;

	ND var replace(SV fromstr, SV tostr) const&;
	ND var regex_replace(SV regex, SV replacement, SV options DEFAULT_EMPTY) const&;

	ND var unique() const&;
	ND var sort(SV sepchar = _FM) const&;
	ND var reverse(SV sepchar = _FM) const&;
	ND var shuffle(SV sepchar = _FM) const&;
	ND var parse(char sepchar = ' ') const&;

	// SAME ON TEMPORARIES - MUTATE FOR SPEED
	/////////////////////////////////////////

	// utf8/byte as for accessors

	ND VARREF ucase() &&;
	ND VARREF lcase() &&;
	ND VARREF tcase() &&;
	ND VARREF fcase() &&;
	ND VARREF normalize() &&;
	ND VARREF invert() &&;

	ND VARREF lower() &&;
	ND VARREF raise() &&;
	ND VARREF crop() &&;

	ND VARREF quote() &&;
	ND VARREF squote() &&;
	ND VARREF unquote() &&;

	ND VARREF trim(SV trimchars DEFAULT_SPACE) &&;
	ND VARREF trimfirst(SV trimchars DEFAULT_SPACE) &&;
	ND VARREF trimlast(SV trimchars DEFAULT_SPACE) &&;
	ND VARREF trimboth(SV trimchars DEFAULT_SPACE) &&;

	ND VARREF first(const size_t length) &&;
	ND VARREF last(const size_t length) &&;
	ND VARREF cut(const int length) &&;
	ND VARREF paste(const int pos1, const int length, SV insertstr) &&;
	ND VARREF paste(const int pos1, SV insertstr) &&;
	ND VARREF prefix(SV insertstr) &&;
	//ND VARREF append(SV appendstr) &&;
	ND VARREF pop() &&;

	ND VARREF fieldstore(SV sepchar, const int fieldno, const int nfields, CVR replacement) &&;
	ND VARREF substr(const int pos1, const int length) &&;
	ND VARREF substr(const int pos1) &&;

	ND VARREF convert(SV fromchars, SV tochars) &&;
	ND VARREF textconvert(SV fromchars, SV tochars) &&;
	ND VARREF replace(SV fromstr, SV tostr) &&;
	ND VARREF regex_replace(SV regex, SV replacement, SV options DEFAULT_EMPTY) &&;

	ND VARREF unique() &&;
	ND VARREF sort(SV sepchar = _FM) &&;
	ND VARREF reverse(SV sepchar = _FM) &&;
	ND VARREF shuffle(SV sepchar = _FM) &&;
    ND VARREF parse(char sepchar = ' ') &&;

//	ND VARREF ucase() && {return ucaser();}
//	ND VARREF lcase() && {return lcaser();}
//	ND VARREF tcase() && {return tcaser();}
//	ND VARREF fcase() && {return fcaser();}
//	ND VARREF normalize() && {return normalizer();}
//	ND VARREF invert() && {return inverter();}
//
//	ND VARREF lower() && {return lowerer();}
//	ND VARREF raise() && {return raiser();}
//	ND VARREF crop() && {return cropper();}
//
//	ND VARREF quote() && {return quoter();}
//	ND VARREF squote() && {return squoter();}
//	ND VARREF unquote() && {return unquoter();}
//
//	ND VARREF trim(SV trimchars DEFAULT_SPACE) && {return trimmer(trimchars);}
//	ND VARREF trimfirst(SV trimchars DEFAULT_SPACE) && {return trimmerfirst(trimchars);}
//	ND VARREF trimlast(SV trimchars DEFAULT_SPACE) && {return trimmerlast(trimchars);}
//	ND VARREF trimboth(SV trimchars DEFAULT_SPACE) && {return trimmerboth(trimchars);}
//
//	ND VARREF first(const size_t length) && {return firster(length);}
//	ND VARREF last(const size_t length) && {return laster(length);}
//	ND VARREF cut(const int length) && {return cutter(length);}
//	ND VARREF paste(const int pos1, const int length, SV insertstr) && {return paster(pos1, length, insertstr);}
//	ND VARREF paste(const int pos1, SV insertstr) && {return paster(pos1, insertstr);}
//	ND VARREF prefix(SV insertstr) && {return prefixer(insertstr);}
//	//ND VARREF append(SV appendstr) && {return appender(appendstr);}
//	ND VARREF pop() && {return popper();}
//
//	ND VARREF fieldstore(SV sepchar, const int fieldno, const int nfields, CVR replacement) && {return fieldstorer(sepchar, fieldno, nfields, replacement);}
//	ND VARREF substr(const int pos1, const int length) && {return substrer(pos1, length);}
//	ND VARREF substr(const int pos1) && {return substrer(pos1);}
//
//	ND VARREF convert(SV fromchars, SV tochars) && {return converter(fromchars, tochars);}
//	ND VARREF textconvert(SV fromchars, SV tochars) && {return textconverter(fromchars, tochars);}
//	ND VARREF replace(SV fromstr, SV tostr) && {return replacer(fromstr, tostr);}
//	ND VARREF regex_replace(SV regex, SV replacement, SV options DEFAULT_EMPTY) && {return regex_replacer(regex, replacement, options);}
//
//	ND VARREF unique() && {return uniquer();}
//	ND VARREF sort(SV sepchar = _FM) && {return sorter(sepchar);}
//	ND VARREF reverse(SV sepchar = _FM) && {return reverser(sepchar);}
//	ND VARREF shuffle(SV sepchar = _FM) && {return shuffler(sepchar);}
//	ND VARREF parse(char sepchar = ' ') && {return parser(sepchar);}

	// STRING MUTATORS
	//////////////////

	// utf8/byte as for accessors

	VARREF ucaser();
	VARREF lcaser();
	VARREF tcaser();
	VARREF fcaser();
	VARREF normalizer();
	VARREF inverter();

	VARREF quoter();
	VARREF squoter();
	VARREF unquoter();

	VARREF lowerer();
	VARREF raiser();
	VARREF cropper();

	VARREF trimmer(SV trimchars DEFAULT_SPACE);
	VARREF trimmerfirst(SV trimchars DEFAULT_SPACE);
	VARREF trimmerlast(SV trimchars DEFAULT_SPACE);
	VARREF trimmerboth(SV trimchars DEFAULT_SPACE);

	VARREF firster(const size_t length);
	VARREF laster(const size_t length);
	VARREF cutter(const int length);
	VARREF paster(const int pos1, const int length, SV insertstr);
	VARREF paster(const int pos1, SV insertstr);
	VARREF prefixer(SV insertstr);
	//VARREF appender(SV appendstr);
	VARREF popper();

	VARREF fieldstorer(SV sepchar, const int fieldno, const int nfields, CVR replacement);
	VARREF substrer(const int pos1, const int length);
	//VARREF substrer(const int pos1);
	// TODO look at using erase to speed up
	//VARREF substrer(const int startindex1) {this->toString();return this->substrer(startindex1, static_cast<int>(var_str.size()));}
	//VARREF substrer(const int startindex1) {return this->substrer(startindex1, static_cast<int>(var_str.size()));}
	VARREF substrer(const int startindex1) {this->assertString(__PRETTY_FUNCTION__);return this->substrer(startindex1, static_cast<int>(var_str.size()));}

	VARREF converter(SV fromchars, SV tochars);
	VARREF textconverter(SV fromchars, SV tochars);
	VARREF replacer(SV fromstr, SV tostr);
	VARREF regex_replacer(SV regex, SV replacement, SV options DEFAULT_EMPTY);

	VARREF uniquer();
	VARREF sorter(SV sepchar = _FM);
	VARREF reverser(SV sepchar = _FM);
	VARREF shuffler(SV sepchar = _FM);
	VARREF parser(char sepchar = ' ');

	// OTHER STRING ACCESS
	//////////////////////

	ND var hash(const uint64_t modulus = 0) const;

	ND dim split(SV sepchar = _FM) const;

	// v3 - returns bytes from some byte number upto the first of a given list of bytes
	// this is something like std::string::find_first_of but doesnt return the delimiter found
	var substr(const int pos1, CVR delimiterchars, int& endindex) const;
	var b(const int pos1, CVR delimiterchars, int& endindex) const {return substr(pos1, delimiterchars, endindex);}

	// v4 - like v3. was named "remove" in pick. notably used in nlist to print parallel columns
	// of mixed combinations of multivalues/subvalues and text marks
	// correctly lined up mv to mv, sv to sv, tm to tm even when particular columns were missing
	// some vm/sm/tm
	// it is like substr(pos1,delimiterbytes,endindex) except that the delimiter bytes are
	// hard coded as the usual RM/FM/VM/SM/TM/ST
	// except that it updates the startstopindex to point one after found delimiter byte and
	// returns the delimiter no (1-6)
	// if no delimiter byte is found then it returns bytes up to the end of the string, sets
	// startstopindex to after tne end of the string and returns delimiter no 0 NOTE that it
	// does NOT remove anything from the source string var remove(VARREF pos1, VARREF
	// delimiterno) const;
	var substr2(VARREF startstopindex, VARREF delimiterno) const;
	var b2(VARREF startstopindex, VARREF delimiterno) const {return substr2(startstopindex, delimiterno);}

	ND var field(SV strx, const int fieldnx = 1, const int nfieldsx = 1) const;
	// version that treats fieldn -1 as the last field, -2 the penultimate field etc. - TODO
	// should probably make field() do this
	//ND var field2(SV strx, const int fieldnx, const int nfieldsx = 1) const;
	ND var field2(SV separator, const int fieldno, const int nfields = 1) const {if (fieldno >= 0) return field(separator, fieldno, nfields); else return field(separator, count(separator) + 1 + fieldno + 1, nfields);}

	// I/O CONVERSION
	/////////////////

	ND var oconv(const char* convstr) const;
	ND var iconv(const char* convstr) const;

	ND var from_codepage(const char* codepage) const;
	ND var to_codepage(const char* codepage) const;

	// CLASSIC MV STRING FUNCTIONS
	//////////////////////////////

	// this function hardly occurs anywhere in exodus code and should probably be renamed to
	// something better it was called replace() in pickos but we are now using "replace()" to
	// change substrings using regex (similar to the old pickos replace function) its mutator function
	// is .r()
//	ND var pickreplace(const int fieldno, const int valueno, const int subvalueno, CVR replacement) const;
//	ND var pickreplace(const int fieldno, const int valueno, CVR replacement) const;
//	ND var pickreplace(const int fieldno, CVR replacement) const;
	ND var pickreplace(const int fieldno, const int valueno, const int subvalueno, CVR replacement) const {return var(*this).r(fieldno, valueno, subvalueno, replacement);}
	ND var pickreplace(const int fieldno, const int valueno, CVR replacement) const {return var(*this).r(fieldno, valueno, 0, replacement);}
	ND var pickreplace(const int fieldno, CVR replacement) const {return var(*this).r(fieldno, 0, 0, replacement);}

	// cf mutator inserter()
	ND var insert(const int fieldno, const int valueno, const int subvalueno, CVR insertion) const& {return var(*this).inserter(fieldno, valueno, subvalueno, insertion);}
	ND var insert(const int fieldno, const int valueno, CVR insertion) const& {return this->insert(fieldno, valueno, 0, insertion);}
	ND var insert(const int fieldno, CVR insertion) const& {return this->insert(fieldno, 0, 0, insertion);}

	/// remove() was delete() in pickos
	// var erase(const int fieldno, const int valueno=0, const int subvalueno=0) const;
	//ND var remove(const int fieldno, const int valueno = 0, const int subvalueno = 0) const;
	ND var remove(const int fieldno, const int valueno = 0, const int subvalueno = 0) const {return var(*this).remover(fieldno, valueno, subvalueno);}

	//.f(...) stands for .attribute(...) or extract(...)
	// pickos
	// xxx=yyy<10>";
	// becomes c++
	// xxx=yyy.f(10);
	ND var f(const int fieldno, const int valueno = 0, const int subvalueno = 0) const;
	ND var extract(const int fieldno, const int valueno = 0, const int subvalueno = 0) const {return this->f(fieldno, valueno, subvalueno);}

	// SAME AS ABOVE ON TEMPORARIES
	///////////////////////////////

	ND VARREF insert(const int fieldno, const int valueno, const int subvalueno, CVR insertion) && {return this->inserter(fieldno, valueno, subvalueno, insertion);}
	ND VARREF insert(const int fieldno, const int valueno, CVR insertion) && {return this->inserter(fieldno, valueno, 0, insertion);}
	ND VARREF insert(const int fieldno, CVR insertion) && {return this->inserter(fieldno, 0, 0, insertion);}

	// MV STRING FILTERS
	////////////////////

	ND var sum() const;
	ND var sumall() const;
	ND var sum(SV sepchar) const;

	// binary ops + - * / : on mv strings 10]20]30
	// e.g. var("10]20]30").mv("+","2]3]4")
	// result is "12]23]34"
	ND var mv(const char* opcode, CVR var2) const;

	// MV STRING MUTATORS
	/////////////////////

	// mutable versions update and return source
	// r stands for "replacer" abbreviated due to high incidience in code
	// pickos
	// xyz<10>="abc";
	// becomes c++
	//  xyz.r(10,"abc");

	// r() is short for replacer() since it is probably the most common var function after a()
	VARREF r(const int fieldno, const int valueno, const int subvalueno, CVR replacement);
	//VARREF r(const int fieldno, const int valueno, CVR replacement);
	//VARREF r(const int fieldno, CVR replacement);
	VARREF r(const int fieldno, const int valueno, CVR replacement) {return r(fieldno, valueno, 0, replacement);}
	VARREF r(const int fieldno, CVR replacement) {	return r(fieldno, 0, 0, replacement);}

	VARREF inserter(const int fieldno, const int valueno, const int subvalueno, CVR insertion);
	VARREF inserter(const int fieldno, const int valueno, CVR insertion) {return this->inserter(fieldno, valueno, 0, insertion);}
	VARREF inserter(const int fieldno, CVR insertion) {return this->inserter(fieldno, 0, 0, insertion);}

	// VARREF eraser(const int fieldno, const int valueno=0, const int subvalueno=0);
	VARREF remover(const int fieldno, const int valueno = 0, const int subvalueno = 0);
	//-er version could be extract and erase in one go
	// VARREF extracter(int fieldno,int valueno=0,int subvalueno=0) const;

	// MV STRING LOCATORS
	/////////////////////

	// should these be like extract, replace, insert, delete
	// locate(fieldno, valueno, subvalueno,target,setting,by DEFAULT_EMPTY)
	ND bool locate(CVR target) const;
	ND bool locate(CVR target, VARREF setting) const;
	ND bool locate(CVR target, VARREF setting, const int fieldno, const int valueno = 0) const;

	ND bool locateusing(const char* usingchar, CVR target, VARREF setting, const int fieldno = 0, const int valueno = 0, const int subvalueno = 0) const;
	ND bool locateusing(const char* usingchar, CVR target) const;

	// locateby without fieldno or valueno arguments uses character VM
	ND bool locateby(const char* ordercode, CVR target, VARREF setting) const;

	// locateby with fieldno=0 uses character FM
	ND bool locateby(const char* ordercode, CVR target, VARREF setting, const int fieldno, const int valueno = 0) const;

	// locatebyusing
	ND bool locatebyusing(const char* ordercode, const char* usingchar, CVR target, VARREF setting, const int fieldno = 0, const int valueno = 0, const int subvalueno = 0) const;

	// DATABASE ACCESS
	//////////////////

	ND bool connect(CVR conninfo DEFAULT_EMPTY);
	void disconnect();
	void disconnectall();

	ND bool attach(CVR filenames);
	void detach(CVR filenames);

	// var() is a db connection or default connection
	ND bool begintrans() const;
	ND bool rollbacktrans() const;
	ND bool committrans() const;
	ND bool statustrans() const;
	   void cleardbcache() const;

	ND bool sqlexec(CVR sqlcmd) const;
	ND bool sqlexec(CVR sqlcmd, VARREF response) const;

	ND var  lasterror() const;
	   var  loglasterror(CVR source DEFAULT_EMPTY) const;

	// DATABASE MANAGEMENT
	//////////////////////

	ND bool dbcreate(CVR dbname) const;
	ND var  dblist() const;
	ND bool dbcopy(CVR from_dbname, CVR to_dbname) const;
	ND bool dbdelete(CVR dbname) const;

	ND bool createfile(CVR filename) const;
	ND bool renamefile(CVR filename, CVR newfilename) const;
	ND bool deletefile(CVR filename) const;
	ND bool clearfile(CVR filename) const;
	ND var  listfiles() const;

	ND bool createindex(CVR fieldname, CVR dictfile DEFAULT_EMPTY) const;
	ND bool deleteindex(CVR fieldname) const;
	ND var  listindex(CVR filename DEFAULT_EMPTY, CVR fieldname DEFAULT_EMPTY) const;

	// bool selftest() const;
	ND var  version() const;

	ND var  reccount(CVR filename DEFAULT_EMPTY) const;
	   var  flushindex(CVR filename DEFAULT_EMPTY) const;

	ND bool open(CVR dbfilename, CVR connection DEFAULT_EMPTY);
	   void close();

	// 1=ok, 0=failed, ""=already locked
	ND var  lock(CVR key) const;
	// void unlock(CVR key) const;
	// void unlockall() const;
	   bool unlock(CVR key) const;
	   bool unlockall() const;

	// db file i/o
	ND bool read(CVR filehandle, CVR key);
	   bool write(CVR filehandle, CVR key) const;
	   bool deleterecord(CVR key) const;
	ND bool updaterecord(CVR filehandle, CVR key) const;
	ND bool insertrecord(CVR filehandle, CVR key) const;

	// specific db field i/o
	ND bool readf(CVR filehandle, CVR key, const int fieldno);
	   bool writef(CVR filehandle, CVR key, const int fieldno) const;

	// cached db file i/o
	ND bool readc(CVR filehandle, CVR key);
	   bool writec(CVR filehandle, CVR key) const;
	   bool deletec(CVR key) const;

	// ExoEnv function now to allow access to RECORD ID DICT etc. and call external
	// functions
	// var calculate() const;

	ND var  xlate(CVR filename, CVR fieldno, const char* mode) const;

	// DATABASE SORT/SELECT
	///////////////////////

	ND bool select(CVR sortselectclause DEFAULT_EMPTY);
	   void clearselect();

	//ND bool hasnext() const;
	ND bool hasnext();
	ND bool readnext(VARREF key);
	ND bool readnext(VARREF key, VARREF valueno);
	ND bool readnext(VARREF record, VARREF key, VARREF valueno);

	   bool savelist(CVR listname);
	ND bool getlist(CVR listname);
	ND bool makelist(CVR listname, CVR keys);
	   bool deletelist(CVR listname) const;
	ND bool formlist(CVR keys, CVR fieldno = 0);

	//bool saveselect(CVR filename);

	// OS TIME/DATE
	///////////////

	ND var  date() const;//int days since pick epoch 1967-12-31
	ND var  time() const;//int seconds since last midnight
	ND var  ostime() const;
	ND var  timestamp() const; // floating point fractional days since pick epoch 1967-12-31 00:00:00
	ND var  timestamp(CVR ostime) const; // construct a timestamp from a date and time

	   void ossleep(const int milliseconds) const;
	   var  oswait(const int milliseconds, SV directory) const;

	// OS FILE SYSTEM
	/////////////////

	ND bool osopen(CVR filename, const char* locale DEFAULT_EMPTY) const;
	ND bool osbread(CVR osfilevar, VARREF offset, const int length);
	ND bool osbwrite(CVR osfilevar, VARREF offset) const;
	   void osclose() const;

	ND bool osread(const char* osfilename, const char* codepage DEFAULT_EMPTY);
	ND bool oswrite(CVR osfilename, const char* codepage DEFAULT_EMPTY) const;
	ND bool osremove() const;
	ND bool osrename(CVR new_dirpath_or_filepath) const;
	ND bool oscopy(CVR to_osfilename) const;
	ND bool osmove(CVR to_osfilename) const;

	ND var  oslist(SV globpattern DEFAULT_EMPTY, const int mode = 0) const;
	ND var  oslistf(SV globpattern DEFAULT_EMPTY) const;
	ND var  oslistd(SV globpattern DEFAULT_EMPTY) const;
	ND var  osinfo(const int mode) const;
	ND var  osfile() const;
	ND var  osdir() const;
	ND var  osinfo() const;
	ND bool osmkdir() const;
	ND bool osrmdir(bool evenifnotempty = false) const;

	// TODO check for threadsafe
	ND var ospid() const;
	ND var ostid() const;
	ND var oscwd() const;
	ND bool oscwd(CVR newpath) const;
	void osflush() const;

	// OS SHELL/ENVIRONMENT
	///////////////////////

	ND bool osshell() const;
	ND bool osshellread(CVR oscmd);
	ND bool osshellwrite(CVR oscmd) const;
	ND var  ostempdirpath() const;
	ND var  ostempfilename() const;

	ND bool osgetenv(const char* code);
	   void ossetenv(const char* code) const;


	///////////////////////////
	// PRIVATE MEMBER FUNCTIONS
	///////////////////////////

 private:

	[[noreturn]] void throwUndefined(CVR message) const;
	[[noreturn]] void throwUnassigned(CVR message) const;
	[[noreturn]] void throwNonNumeric(CVR message) const;
	[[noreturn]] void throwNonPositive(CVR message) const;
	[[noreturn]] void throwNumOverflow(CVR message) const;
	[[noreturn]] void throwNumUnderflow(CVR message) const;

	// WARNING: MUST NOT use any var when checking Undefined
	// OTHERWISE *WILL* get recursion/segfault
	void assertDefined(const char* message, const char* varname = "") const {
		if (var_typ & VARTYP_MASK)
			[[unlikely]]
			throwUndefined(var(varname) ^ " in " ^ message);
	}

	void assertAssigned(const char* message, const char* varname = "") const {
		assertDefined(message, varname);
		if (!var_typ)
			[[unlikely]]
			throwUnassigned(var(varname) ^ " in " ^ message);
	}

	void assertNumeric(const char* message, const char* varname = "") const {
		if (!this->isnum())
			[[unlikely]]
			throwNonNumeric(var(varname) ^ " in " ^ var(message) ^ " data: " ^ this->first(128).quote());
	}

	void assertDecimal(const char* message, const char* varname = "") const {
		assertNumeric(message, varname);
		if (!(var_typ & VARTYP_DBL)) {
			var_dbl = static_cast<double>(var_int);
			// Add double flag
			var_typ |= VARTYP_DBL;
		}
	}

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

	void assertString(const char* message, const char* varname = "") const {
		assertDefined(message, varname);
		if (!(var_typ & VARTYP_STR)) {
			if (!var_typ)
				[[unlikely]]
				throwUnassigned(var(varname) ^ " in " ^ message);
			this->createString();
		}
	}

	void assertStringMutator(const char* message, const char* varname = "") const {
		assertString(message, varname);
		// VERY IMPORTANT:
		// If var_str is mutated then we MUST
		// reset all flags to ensure that the int/dbl, if needed,
		// are again lazily derived from the new string when required.
		var_typ = VARTYP_STR;
	}

	// Constructor
	void from_u32string(std::u32string u32str) const;

	void createString() const;

	ND bool cursorexists(); //database, not terminal
	ND bool selectx(CVR fieldnames, CVR sortselectclause);

	   var  setlasterror(CVR msg) const;

	// TODO check if can speed up by returning reference to converted self like MC

	// Faster primitive oconv
	// L/R/C: Text -> left/right/center padded and truncated
	ND std::string oconv_LRC(CVR format) const;
	// T: Text -> justified and folded
	ND std::string oconv_T(CVR format) const;
	// D: Int -> Date
	ND std::string oconv_D(const char* conversion) const;
	// MT: Int -> Time
	ND std::string oconv_MT(const char* conversion) const;
	// MD: Decimal -> Decimal
	ND std::string oconv_MD(const char* conversion) const;
	// MR: Character replacement
	ND VARREF oconv_MR(const char* conversion);
	// HEX: Chars -> Hex
	ND std::string oconv_HEX(const int ioratio) const;
	// TX: Record (FM) -> text (\n) and \ line endings
	ND std::string oconv_TX(const char* conversion) const;

	// Faster primitive iconv
	// D: Int <- Date
	ND var iconv_D(const char* conversion) const;
	// MT: Int <- Time
	ND var iconv_MT() const;
	// MD_ Decimal <- Decimal
	ND var iconv_MD(const char* conversion) const;
	// Chars <- Hex
	ND var iconv_HEX(const int ioratio) const;
	// TX: Record (FM) <- text (\n) and \ line endings
	ND var iconv_TX(const char* conversion) const;

	ND std::fstream* osopenx(CVR osfilename, const char* locale) const;

	bool THIS_IS_OSFILE() const { return ((var_typ & VARTYP_OSFILE) != VARTYP_UNA); }

	// Convert _VISIBLE_FMS to _ALL_FMS
	// In header to perhaps aid runtime string literal conversion for operator""_var
	// since currently it cannot be constexpr due to var containing a std::string
	var& fmiconverter() {
		for (char& c : this->var_str) {
			switch (c) {
				// Most common first to perhaps aid optimisation
				case VISIBLE_FM_: c = FM_; break;
				case VISIBLE_VM_: c = VM_; break;
				case VISIBLE_SM_: c = SM_; break;
				case VISIBLE_TM_: c = TM_; break;
				case VISIBLE_ST_: c = ST_; break;
				case VISIBLE_RM_: c = RM_; break;
				// All other chars left unconverted
				default:;
			}
		}
		return *this;
	}

	// Convert _ALL_FMS to _VISIBLE_FMS
	var& fmoconverter() {
		for (char& c : this->var_str) {
			if (c > RM_ || c > RM_) {
				switch (c) {
					// In order to perhaps aid optimisation
					case RM_: c = VISIBLE_RM_; break;
					case FM_: c = VISIBLE_FM_; break;
					case VM_: c = VISIBLE_VM_; break;
					case SM_: c = VISIBLE_SM_; break;
					case TM_: c = VISIBLE_TM_; break;
					case ST_: c = VISIBLE_ST_; break;
					// All other chars left unconverted;
					default:;
				}
			}
		}
		return *this;
	}

};  // class "var"

// NB we should probably NEVER add operator^(VARREF var1, bool)
// this is a trick to avoid a problem that exodus concat operator
// has the wrong precedence versus the logical operators
// a ^ b > c //we should prevent this from compiling because
// exodus will treat this like a^(b>c) which is WRONG
// multivalue basic requires it to be (a^b)>c
// however c++ ^ operator has lower priority than >
// whereas multivalue concatenate operator should have HIGHER priority than >
// and no way to change priority of operators in c++
// therefore to prevent silent non-standard behaviour from causing
// insidious bugs it is better to refuse to compile a request to
// concatenate boolean as "1" or "0" in Exodus
// a refusal to compile can be solved explicitly by putting brackets
// to force compiler to do the right thing multivalue wise
//(a^b)>c
// or, if you do want to concatenate the result of a comparison do this
// a^var(b>c)

/////////////////////////////////
// var_iter - iterate over fields
/////////////////////////////////
class PUBLIC var_iter {

	const var* pvar_;
	mutable std::string::size_type startpos_ = 0;
	mutable std::string::size_type endpos_ = std::string::npos;

 public:
	// Default constructor
	var_iter() = default;

	// Construct from var
	var_iter(CVR v);

	// Check iter != iter (i.e. iter != string::npos)
	bool operator!=(const var_iter& vi);

	// Convert to var
	var operator*() const;

	// Iter++
	var_iter operator++();

	// Iter-- prefix
	var_iter operator--();

	// --iter postfix
	var_iter operator--(int);

};

///////////////////////////////////////////////
// var_proxy1 - replace or extract fields by fn
///////////////////////////////////////////////
class PUBLIC var_proxy1 {

 private:

	var& var_;
	mutable int fn_;

	var_proxy1() = delete;

 public:

	// Constructor for var + fn
	var_proxy1(var& var1, int fn) : var_(var1), fn_(fn) {}

	// Implicit conversion to var if on the right hand side
	// Equivalent to PickOS expression 'xxx<fn>' but with round instead of angle brackets
	operator var() const {
		return var_.f(fn_);
	}

	// Operator assign using =
	// Equivalent to PickOS statement 'xxx<fn> = yyy' but with round instead of angle brackets
	void operator=(CVR replacement) {
		var_.r(fn_, replacement);
	}

	// Operator bool. Allow usage in if statements etc.
	// if (xxxx<fn>)
	explicit operator bool() const {
		return var_.f(fn_);
	}

	// Operator []. Allow character extraction.
	// xxx(fn)[cn]
	ND var operator[](const int pos1) const {
		return var_.f(fn_)[pos1];
	}

};

// class var_proxy2 - replace or extract fn, sn
class PUBLIC var_proxy2 {

 private:

	var& var_;
	mutable int fn_;
	mutable int vn_;

	var_proxy2() = delete;

 public:

	var_proxy2(var& var1, int fn, int vn) : var_(var1), fn_(fn), vn_(vn) {}

	operator var() const {
		return var_.f(fn_, vn_);
	}

	void operator=(CVR replacement) {
		var_.r(fn_, vn_, replacement);
	}

	explicit operator bool() const {
		return var_.f(fn_, vn_);
	}

	ND var operator[](const int pos1) const {
		return var_.f(fn_, vn_)[pos1];
	}

};

// class var_proxy3 - replace or extract fn, vn, sn
class PUBLIC var_proxy3 {

 private:

	var& var_;
	mutable int fn_;
	mutable int vn_;
	mutable int sn_;

	var_proxy3() = delete;

 public:

	var_proxy3(var& var1, int fn, int vn = 0, int sn = 0) : var_(var1), fn_(fn), vn_(vn), sn_(sn) {}

	operator var() const {
		return var_.f(fn_, vn_, sn_);
	}

	void operator=(CVR replacement) {
		var_.r(fn_, vn_, sn_, replacement);
	}

	explicit operator bool() const {
		return var_.f(fn_, vn_, sn_);
	}

	ND var operator[](const int pos1) const {
		return var_.f(fn_, vn_, sn_)[pos1];
	}

};

ND inline var_proxy1 var::operator()(int fieldno) {return var_proxy1(*this, fieldno);}
ND inline var_proxy2 var::operator()(int fieldno, int valueno) {return var_proxy2(*this, fieldno, valueno);}
ND inline var_proxy3 var::operator()(int fieldno, int valueno, int subvalueno) {return var_proxy3(*this, fieldno, valueno, subvalueno);}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"

////////////////////////////////////////////////
// var versions of various field mark characters
////////////////////////////////////////////////
	PUBLIC extern const var RM;
	PUBLIC extern const var FM;
	PUBLIC extern const var VM;
	PUBLIC extern const var SM;
	PUBLIC extern const var TM;
	PUBLIC extern const var ST;

	PUBLIC extern const var BS;
	PUBLIC extern const var DQ;
	PUBLIC extern const var SQ;
#ifdef EXO_VAR_CPP
	const var RM = RM_;
	const var FM = FM_;
	const var VM = VM_;
	const var SM = SM_;
	const var TM = TM_;
	const var ST = ST_;

	const var BS = BS_;
	const var DQ = DQ_;
	const var SQ = SQ_;
#endif

/////////////////////////////////////
// A global flag used in mvdbpostgres
/////////////////////////////////////
[[maybe_unused]] static inline int DBTRACE = var().osgetenv("EXO_DBTRACE");

#pragma clang diagnostic pop

//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Winline"

/////////////////////////
// A base exception class - Provide stack tracing
/////////////////////////
class PUBLIC VarError {
 public:

	//VarError(CVR description) = delete;
	explicit VarError(CVR description);

	// Note: "description" is not const so that an exception handler (catch block)
	// can add any context (additional info only known in the handler)
	// to the error description and rethrow the exception up to
	// a higher exception handler.
	// (using plain "throw;")
	// Otherwise the only the stack track captured by the exception site will be available. 
	var description;

	// Convert stack addresses to source code if available
	var stack(const size_t limit = 0) const;

 private:

	mutable void* stack_addresses_[BACKTRACE_MAXADDRESSES];
	mutable size_t stack_size_ = 0;

};
//#pragma GCC diagnostic pop

////////////////////
// _var user literal
////////////////////

// "abc^def"_var
PUBLIC var operator""_var(const char* cstr, std::size_t size);

// 123456_var
PUBLIC var operator""_var(unsigned long long int i);

// 123.456_var
PUBLIC var operator""_var(long double d);

}  // namespace exodus

#endif //EXODUS_LIBEXODUS_EXODUS_VAR_H_
