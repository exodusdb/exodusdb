#ifndef MV_H
#define MV_H 1
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

//#include <iostream>
#include <string>
#include <cmath>  //for floor

#define EXODUS_RELEASE "21.03"
#define EXODUS_PATCH "21.03.0"

// http://stackoverflow.com/questions/538134/exporting-functions-from-a-dll-with-dllexport
// Using dllimport and dllexport in C++ Classes
// http://msdn.microsoft.com/en-us/library/81h27t8c(VS.80).aspx

//https://quuxplusone.github.io/blog/2021/11/09/pass-string-view-by-value/

#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__

#ifdef BUILDING_LIBRARY

#ifdef __GNUC__
#define PUBLIC __attribute__((dllexport))
#else
#define PUBLIC __declspec(dllexport)  // Note: actually gcc seems to also support this syntax.
#endif

#else //not BUILDING LIBRARY

#ifdef __GNUC__
#define PUBLIC __attribute__((dllimport))
#else
#define PUBLIC __declspec(dllimport)  // Note: actually gcc seems to also support this syntax.
#endif

#endif

#define DLL_LOCAL

#else

#if __GNUC__ >= 4
// use g++ -fvisibility=hidden to make all hidden except those marked PUBLIC ie "default"
#	define PUBLIC __attribute__((visibility("default")))
#	define DLL_LOCAL __attribute__((visibility("hidden")))
#else
#	define PUBLIC
#	define DLL_LOCAL
#endif

#endif

// ND macro [[no_discard]]
#if __clang_major__ != 10
#define ND [[nodiscard]]
#else
#define ND
#endif

#include <exodus/vartyp.h>

#define VARREF var&
#define CVR const var&
#define TVR var&&
#define SV std::string_view

// Decided to use ASCII characters 0x1A-0x1F for PickOS separator chars
// instead of PickOS 0xFA-0xFF which are illegal utf-8 bytes

// also defined in extract.c and exodusmacros.h

// the var versions of the following (without leading or trailing _)
// are defined AFTER the class declaration of "var"

// leading and trailing _ char* versions of classic pick delimiters
// also in ADECOM
#define _RM_ "\x1F"      // Record Mark
#define _FM_ "\x1E"      // Field Mark
#define _VM_ "\x1D"      // Value Mark
#define _SM_ "\x1C"      // Subvalue Mark
#define _TM_ "\x1B"      // Text Mark
#define _STM_ "\x1A"  // Subtext Mark

// aliases for different implementations of multivalue
#define _IM_ _RM_
#define _AM_ _FM_
#define _SVM_ _SM_

#define _BS_ "\\"
#define _DQ_ "\""
#define _SQ_ "\'"

// trailing _ char versions of classic pick delimiters
#define RM_ '\x1F'     // Record Mark
#define FM_ '\x1E'     // Field Mark
#define VM_ '\x1D'     // Value Mark
#define SM_ '\x1C'     // Subvalue Mark
#define TM_ '\x1B'     // Text Mark
#define STM_ '\x1A'     // Subtext Mark

// aliases for different implementations of multivalue
#define IM_ RM_
#define AM_ FM_
#define SVM_ SM_

#define BS_ '\\'
#define DQ_ '\"'
#define SQ_ '\''

// Would be 256 if RM was character number 255.
// Last delimiter character is 0x1F (RM)
// Used in var::remove()
#define LASTDELIMITERCHARNOPLUS1 0x20

//#define VISIBLE_FMS "_]\[Z"  //PickOS standard. Backslash not good since it is often used for escaping chars. Z is normal letter.
//#define VISIBLE_FMS "<[{}]>" //logical but hard to read direction of brackets quickly
//#define VISIBLE_FMS "_^]}`~" //all uncommon in natural language. first 3 _^] are identical to pickos
#define VISIBLE_FMS "_^]}|~"   //all uncommon in natural language. first 3 _^] are identical to pickos

// Useful TRACE() function for debugging
#define TRACE(EXPRESSION) \
	var(EXPRESSION).convert(_RM_ _FM_ _VM_ _SM_ _TM_ _STM_, VISIBLE_FMS).quote().logputl("TRACE: " #EXPRESSION "=");

namespace exodus {

//#define SMALLEST_NUMBER 1e-13
//0.00000000000023 = sum(1287.89,-1226.54,-61.35,1226.54,-1226.54)
//#define SMALLEST_NUMBER 1e-10
//#define SMALLEST_NUMBER 1e-4d//0.0001 for pickos compatibility
constexpr double SMALLEST_NUMBER = 0.0001;// for pickos compatibility

class dim;
class var_iter;
class var__extractreplace;

// original help from Thinking in C++ Volume 1 Chapter 12
// http://www.camtp.uni-mb.si/books/Thinking-in-C++/TIC2Vone-distribution/html/Chapter12.html

// could also use http://www.informit.com/articles/article.asp?p=25264&seqNum=1
// which is effectively about makeing objects behave like ordinary variable syntactically
// implementing smartpointers

// the destructor is public non-virtual (supposedly to save space)
// since this class has no virtual functions
// IT SHOULD BE ABLE TO DERIVE FROM IT AND DO DELETE()
// http://www.parashift.com/c++-faq-lite/virtual-functions.html#faq-20.7

// on linux, size of var is 56 bytes (8 bytes "free" to round up to 64 bytes
//
// string:    32
// int:        8
// double:     8
// char:       4
//           ---
// var:       56

class var_proxy1;
class var_proxy2;
class var_proxy3;
//class var_brackets_proxy;

#define DEFAULT_UNASSIGNED = var()
#define DEFAULT_EMPTY = ""
#define DEFAULT_DOT = "."
#define DEFAULT_SPACE = " "
#define DEFAULT_VM = VM_
#define DEFAULT_NULL = nullptr

//prevent or allow assignment to var to return a reference to the var
//preventing will stop accidental usage of = instead of == in if() clauses
//if you really want to assign in an if clause then use something like  if (x=123;x)
#define PREVENT_ASSIGN_IN_IF
#ifdef PREVENT_ASSIGN_IN_IF
#define VOID_OR_VARREF void
#define VOID_OR_DIMREF void
#define VOID_OR_THIS
#else
#define VOID_OR_VARREF "VARREF"
#define VOID_OR_DIMREF "dim&"
#define VOID_OR_THIS "*this"
#endif

#define THISIS(FUNC_DESC) [[maybe_unused]] static const char* function_sig = FUNC_DESC;

#define ISDEFINED(VARNAME) VARNAME.assertDefined(function_sig, #VARNAME);
#define ISASSIGNED(VARNAME) VARNAME.assertAssigned(function_sig, #VARNAME);
#define ISSTRING(VARNAME) VARNAME.assertString(function_sig, #VARNAME);
#define ISNUMERIC(VARNAME) VARNAME.assertNumeric(function_sig, #VARNAME);

#define INLINE inline //this is the default anyway
//#define INLINE __attribute__ ((noinline)) //use this to reduce compllation speed and object size

// class var final
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
	mutable int64_t     var_int; // 8 bytes/64 bits - currently defined as a long long
	mutable double      var_dbl; // 8 bytes/64 buts - double
	mutable VARTYP      var_typ; // Default initialised to VARTYP_UNA
	                             // Actually a unsigned int which will be 4 bytes


	///////////////////////////
	// SPECIAL MEMBER FUNCTIONS
	///////////////////////////

   public:

	/////////////////////////
	// 1. Default constructor - Can default as string and var_typ have constructors
	/////////////////////////
	//
	// allow syntax "var v;" to create an "unassigned" var (var_typ is 0)
	//
	var() = default;
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
//		// programming if (var_typ&VARTYP_MASK) throw MVUndefined("funcname()"); should really
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

		// not a pimpl style pointer anymore for speed
		// delete priv;

		// try to ensure any memory is not later recognises as initialised memory
		//(exodus tries to detect undefined use of uninitialised objects at runtime - that dumb
		// compilers allow without warning) this could be removed in production code perhaps set all
		// unused bits to 1 to ease detection of usage of uninitialised variables (bad c++ syntax
		// like var x=x+1; set all used bits to 0 to increase chance of detecting unassigned
		// variables var_typ=(char)0xFFFFFFF0;
		var_typ = VARTYP_MASK;
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

		//std::clog << "copy ctor CVR " << rhs.var_str << std::endl;
	}

	//////////////////////
	// 4. move constructor - Can default assuming that temporaries are unlikely to be undefined or unassigned
	//////////////////////
	//
	var(TVR fromvar) noexcept = default;
	//	// var(TVR fromvar) noexcept;  // = default;
	//	// defined in class for inline/optimisation
	//	// move constructor
	//	var(TVR rhs) noexcept
	//		: var_str(std::move(rhs.var_str)),
	//		var_int(rhs.var_int),
	//		var_dbl(rhs.var_dbl),
	//		var_typ(rhs.var_typ) {
	//
	//		//std::clog << "move ctor TVR noexcept " << rhs.var_str << std::endl;
	//
	//		// skip this for speed since temporararies are unlikely to be unassigned
	//		// THISIS("var::var(TVR rhs) noexcept")
	//		// ISASSIGNED(rhs)
	//	}

	/////////////////////
	// 5. copy assignment - from lvalue
	/////////////////////

	// Not using copy assignment by value (copy-and-swap idiom)
	// because Howard Hinnant recommends against in our case

	// Prevent assigning to temporaries
	INLINE VOID_OR_VARREF operator=(CVR rhs) && = delete;

	// var& operator=(CVR rhs) & = default;
	// Cannot use default copy assignment because
	// a) it returns a value allowing accidental use of "=" instead of == in if statements
	// b) doesnt check if rhs is assigned
	INLINE VOID_OR_VARREF operator=(CVR rhs) & {

		//assertDefined(__PRETTY_FUNCTION__);  //could be skipped for speed?
		rhs.assertAssigned(__PRETTY_FUNCTION__);

		//std::clog << "copy assignment by reference" <<std::endl;

		// Prevent self assign
		// Removed for speed since we assume std::string handles it ok
		//if (this == &rhs)
		//	return VOID_OR_THIS;

		// copy everything across
		var_str = rhs.var_str;
		var_dbl = rhs.var_dbl;
		var_int = rhs.var_int;
		var_typ = rhs.var_typ;

		return VOID_OR_THIS;
	}

	/////////////////////
	// 6. move assignment - from rvalue/temporary
	/////////////////////

	// defined in class for inline/optimisation

	// Prevent assigning to temporaries
	INLINE VOID_OR_VARREF operator=(TVR rhs) && noexcept = delete;

	// Cannot use default move assignment because
	// a) it returns a value allowing accidental use of "=" in if statements instead of ==
	// b) doesnt check if rhs is assigned (less important for temporaries which are rarely unassigned)
	//var& operator=(TVR rhs) & noexcept = default;
	INLINE VOID_OR_VARREF operator=(TVR rhs) & {

		//assertDefined(__PRETTY_FUNCTION__);  //could be skipped for speed?

		// Removed for speed like move constructor since temporaries are unlikely to be unassigned
		//rhs.assertAssigned(__PRETTY_FUNCTION__);

		//std::clog << "move assignment" <<std::endl;

		// Prevent self assign
		// Removed for speed since we assume std::string handles it ok
		//if (this == &rhs)
		//	return VOID_OR_THIS;

		// move everything over
		var_str = std::move(rhs.var_str);
		var_dbl = rhs.var_dbl;
		var_int = rhs.var_int;
		var_typ = rhs.var_typ;

		return VOID_OR_THIS;
	}


	///////////////////
	// INT CONSTRUCTORS
	///////////////////

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
	*/
	template <typename I, std::enable_if_t<std::is_integral<I>::value, bool> = true>
	//enable_if can be replaced by a concept when available in g++ compiler (gcc v11?)
	INLINE var(I rhs)
		:
		var_int(rhs),
		var_typ(VARTYP_INT) {
		//std::cerr << "var(int)" << std::endl;
	}


	//////////////////////
	// DOUBLE CONSTRUCTORS
	//////////////////////

	/*
		float,
		double,
		long double
	*/
	template <typename F, std::enable_if_t<std::is_floating_point<F>::value, bool> = true>
	//enable_if can be replaced by a concept when available in g++ compiler (gcc v11?)
	INLINE var(F rhs)
		:
		var_dbl(rhs),
		var_typ(VARTYP_DBL){
			//std::cerr << "var(dbl)" << std::endl;
	}


	//////////////////////
	// STRING CONSTRUCTORS
	//////////////////////

	/*
		// Accepts l and r values efficiently hopefully
		std::string,
		std::string_view,
		char*,
	*/
	template <typename S, std::enable_if_t<std::is_convertible<S, std::string_view>::value, bool> = true>
	//enable_if can be replaced by a concept when available in g++ compiler (gcc v11?)
	INLINE var(S&& fromstr)
	    :
		var_str(std::forward<S>(fromstr)),
		var_typ(VARTYP_STR) {
		//std::cerr << "var(str)" << std::endl;
	}

	// memory block
	///////////////
	INLINE var(const char* charstart, const size_t nchars)
		:
		var_str(charstart, nchars),
		var_typ(VARTYP_STR) {
		//std::cerr << "var(mem)" << std::endl;
	}

	// char
	///////
	//must be separate from unsigned int contructor to get a string/char not an int/ character number
	INLINE var(const char char1) noexcept
		:
		var_str(1, char1),
		var_typ(VARTYP_STR) {
			//std::cerr << "var(char)" << std::endl;
	}


	////////////////////////////////
	// STRING CONSTRUCTORS FROM WIDE
	////////////////////////////////

	/*
		// Accepts l and r values efficiently hopefully
		std::u32string,
		std::u32string_view,
		u32char*,
		u32char
	*/

	template <typename W, std::enable_if_t<std::is_convertible<W, std::u32string_view>::value, bool> = true>
	//enable_if can be replaced by a concept when available in g++ compiler (gcc v11?)
	INLINE var(W&& fromstr)
	    :
		var_typ(VARTYP_STR) {
		//std::cerr << "TEMPLATE " << __PRETTY_FUNCTION__ << std::endl;
		this->from_u32string(fromstr);
	}

	// const std::wstring&
	//////////////////////
	var(const std::wstring& wstr1);

	// wchar*
	/////////
	INLINE var(const wchar_t* wcstr1)
		: var_typ(VARTYP_STR) {

		// protect against null pointer
		if (wcstr1 == 0)
			throw ("Null pointer in var::var(const wchar_t*)");

		(*this) = var(std::wstring(wcstr1));
	}

// Cant provide these and support L'x' since it they are ambiguous with char when creating var from ints
//
//	// wchar
//  ////////
//	INLINE explicit var(wchar_t wchar1)
//		: var_typ(VARTYP_STR) {
//
//		(*this) = var(std::wstring(1, wchar1));
//	}
//
//	// char32_t char
//  ////////////////
//	INLINE var(char32_t u32char)
//		: var_typ(VARTYP_STR) {
//
//		this->from_u32string(std::u32string(1, u32char));
//	}
//
//	// char8_t char
//  ///////////////
//	INLINE var(char8_t u8char)
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

	// int, double, cstr etc.
	/////////////////////////
	template <class T>
	var(std::initializer_list<T> list)
		: var_typ(VARTYP_STR) {
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

	// standard c/c++ int() in other words simply take the number to the left of the point. -1.5
	// becomes -1 and 1.5 becomes 1
	int64_t toInt() const;

	//long long toLong() const;

	double toDouble() const;

	std::u32string to_u32string() const;

	//void from_u32string(std::u32string) const;

	std::string toString() &&;               //for temporary vars

	//Be careful not to retain the reference beyond the scope of the var
	const std::string& toString() const&;  //for non-temporaries.

	// weird version for perl that outputs "" if undefined
	std::string toString2() const;


	///////////////////////////
	// AUTOMATIC CONVERSIONS TO
	///////////////////////////

	// someone recommends not to create more than one automatic converter
	// to avoid the compiler error "ambiguous conversion"
	// explicit keyword only works on conversion TO var not conversion FROM var

	// commented to allow conversion to int
	// if you uncomment this then also uncomment the definition in var.cpp
	// unfortunately int takes precedence over bool somehow (unless both const/not const?)
	// so if x is not numeric "if (x)" gives a non-numeric error.
	// automatic conversion to int would be nice to avoid large numbers of specialised function
	// calls I think like var::extract(int, int, int);

	// automatic conversion to int
	// necessary to allow conversion to int in many functions like extract(x,y,z)

	// if you uncomment this then also uncomment the definition in var.cpp
	// uncommented to allow simpler functions
	// NB const probably determines priority of choice between automatic conversion
	// and C++ uses bool and int conversion in certain cases to convert to int and bool
	// therefore have chosen to make both bool and int "const" since they dont make
	// and changes to the base object.
	// provide a reference to the (long long) int inside a var
	// This allows passing by reference to the guts of var for speed
	// it is safe because changing the int has no side effects
//#define HASINTREFOP
#ifdef HASINTREFOP
	operator int&() const;
	explicit operator long long&() const;
	operator double&() const;
#else

	template <typename I, std::enable_if_t<std::is_integral<I>::value, bool> = true>
	//enable_if can be replaced by a concept when available in g++ compiler (gcc v11?)
	operator I() const {
		//converts string or double to int using pickos int() which is floor()
		//unlike c/c++ int() function which rounds to nearest even number (negtive or positive)
		assertInteger(__PRETTY_FUNCTION__);
		return static_cast<I>(var_int);
	}

	template <typename F, std::enable_if_t<std::is_floating_point<F>::value, bool> = true>
	//enable_if can be replaced by a concept when available in g++ compiler (gcc v11?)
	operator F() const {
		assertDecimal(__PRETTY_FUNCTION__);
		return static_cast<F>(var_dbl);
	}

#endif

	// remove because causes "ambiguous" with -short_wchar on linux
	// operator unsigned int() const;

	// recommended to provide automatic conversion to VOID POINTER instead of direct to bool
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
		return (void*)this->toBool();
	}

	// this stops using var as indexes of arrays on msvc since msvc converts bool and int
	// ambiguous to index number g++ seems happy to make int higher and unambigous with bool but
	// probably non-standard c++ msvc (at least) cant do without this since it seems unwilling
	// to convert void* as a bool therefore we include it and restrict indexing on ints and not
	// var eg for (int ii=0 .... instead of for (var ii=0 ... #ifndef _MSVC explicit new in
	// c++11 still allows implicit conversion in places where bool is definitely required ie in
	// if (xx) or aa && bb but we do not use is because allowing implicit conversion to bool in
	// argument lists is convenient explicit
	//explicit operator bool() const;
	// supposed to be replaced with automatic void() and made explicit but just seems to force int
	// conversion during "if (var)" necessary to allow var to be used standalone in "if (xxx)" but see
	// mv.h for discussion of using void* instead of bool #ifndef _MSC_VER
	operator bool() const {
		return this->toBool();
	}

	//#endif

	// NB && and || operators are NOT overloaded because var is set to convert to boolean
	// automatically this is fortunate because "shortcircuiting" of && and || doesnt happen when
	// overloaded

	// operator size_t() const;

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
	operator std::string() const {
		assertString(__PRETTY_FUNCTION__);
		return var_str;
	}

	operator std::string_view() const {
		assertString(__PRETTY_FUNCTION__);
		return std::string_view(var_str);
	}

	operator std::u32string() const {
		return this->to_u32string();
	}

	operator const char*() const {
		assertString(__PRETTY_FUNCTION__);
		return var_str.c_str();
	}

	// allow conversion to char (takes first char or char 0 if zero length string)
	// would allow the usage of any char function but may result
	// in a lot of compilation failures due to "ambiguous overload"
	// unfortunately there is no "explicit" keyword as for constructors - coming in C++0X
	// operator char() const;

	// ROUND BRACKETS ()
	////////////////////

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
//	ND var_proxy1 operator()(int fieldno)                              & {return var_proxy1(this, fieldno);}
//	ND var_proxy2 operator()(int fieldno, int valueno)                 & {return var_proxy2(this, fieldno, valueno);}
//	ND var_proxy3 operator()(int fieldno, int valueno, int subvalueno) & {return var_proxy3(this, fieldno, valueno, subvalueno);}


	/////////////
	// ASSIGNMENT
	/////////////

	// The assignment operator should always return a reference to *this.

	// int assignment
	// The assignment operator should always return a reference to *this.
	INLINE VOID_OR_VARREF operator=(const int int1) & {
		// THISIS("VARREF operator= (const int int1)")
		// protect against unlikely syntax as follows:
		// var undefinedassign=undefinedassign=123';
		// !!RISK NOT CHECKING TO SPEED THINGS UP SINCE SEEMS TO WORK IN MSVC AND GCC
		// THISISDEFINED()

		var_int = int1;
		var_typ = VARTYP_INT;  // reset to one unique type

		return VOID_OR_THIS;
	}

	// double assignment
	// The assignment operator should always return a reference to *this.
	INLINE VOID_OR_VARREF operator=(const double dbl1) & {
		// THISIS("VARREF operator= (const double dbl1)")
		// protect against unlikely syntax as follows:
		// var undefinedassign=undefinedassign=9.9';
		// !!RISK NOT CHECKING TO SPEED THINGS UP SINCE SEEMS TO WORK IN MSVC AND GCC
		// THISISDEFINED()

		var_dbl = dbl1;
		var_typ = VARTYP_DBL;  // reset to one unique type

		return VOID_OR_THIS;
	}

	// char assignment
	// The assignment operator should always return a reference to *this.
	INLINE VOID_OR_VARREF operator=(const char char2) & {

		//THISIS("VARREF operator= (const char char2) &")
		// protect against unlikely syntax as follows:
		// var undefinedassign=undefinedassign=L'X';
		// this causes crash due to bad memory access due to setting string that doesnt exist
		// slows down all string settings so consider NOT CHECKING in production code
		//THISISDEFINED()	 // ALN:TODO: this definition kind of misleading, try to find
		// ALN:TODO: or change name to something like: THISISNOTDEAD :)
		// ALN:TODO: argumentation: var with mvtyp=0 is NOT defined

		var_str = char2;
		var_typ = VARTYP_STR;  // reset to one unique type

		return VOID_OR_THIS;
	}

	// char* assignment
	// The assignment operator should always return a reference to *this.
	INLINE VOID_OR_VARREF operator=(const char* cstr) & {
		//THISIS("VARREF operator= (const char* cstr2) &")
		// protect against unlikely syntax as follows:
		// var undefinedassign=undefinedassign="xxx";
		// this causes crash due to bad memory access due to setting string that doesnt exist
		// slows down all string settings so consider NOT CHECKING in production code
		//THISISDEFINED()

		var_str = cstr;
		var_typ = VARTYP_STR;  // reset to one unique type

		return VOID_OR_THIS;
	}

	// std::string assignment from variable (lvalue)
	// The assignment operator should always return a reference to *this.
	INLINE VOID_OR_VARREF operator=(const std::string& string2) & {

		//THISIS("VARREF operator= (const std::string& string2) &")
		// protect against unlikely syntax as follows:
		// var undefinedassign=undefinedassign=std::string("xxx"";
		// this causes crash due to bad memory access due to setting string that doesnt exist
		// slows down all string settings so consider NOT CHECKING in production code
		//THISISDEFINED()
		var_str = string2;
		var_typ = VARTYP_STR;  // reset to one unique type

		return VOID_OR_THIS;
	}

	// std::string assignment from temporary (rvalue)
	// The assignment operator should always return a reference to *this.
	INLINE VOID_OR_VARREF operator=(const std::string&& string2) & {

		//THISIS("VARREF operator= (const std::string&& string2) &")
		// protect against unlikely syntax as follows:
		// var undefinedassign=undefinedassign=std::string("xxx"";
		// this causes crash due to bad memory access due to setting string that doesnt exist
		// slows down all string settings so consider NOT CHECKING in production code
		//THISISDEFINED()

		var_str = std::move(string2);
		var_typ = VARTYP_STR;  // reset to one unique type

		return VOID_OR_THIS;
	}


	//////////////
	// BRACKETS []
	//////////////

	// Coming in C++23 (already in gcc trunk) - multi-dimensional arrays with more than one index eg [x,y], [x,y,z] etc.
	// and left/right distinction for assign/extract?

	// Extract a character from a constant var
	// first=1 last=-1 etc.
	ND var operator[](const int charno) const;

	// Named member function identical to operator[]
	ND var at(const int charno) const;

	// as of now, sadly the following all works EXCEPT that var[99].anymethod() doesnt work
	// so would have to implement all var methods and free functions on the proxy object
	//ND var_brackets_proxy operator[](int charno);
	//ND var operator[](int charno) &&;


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
	VARREF operator+=(const char  char1) &{(*this) += var(char1);return *this;}
	VARREF operator+=(const char* chars) &{(*this) += var(chars);return *this;}
	VARREF operator+=(const bool) &;

	// Multiply
	VARREF operator*=(const int) &;
	VARREF operator*=(const double) &;
	VARREF operator*=(const char  char1) &{(*this) *= var(char1);return *this;}
	VARREF operator*=(const char* chars) &{(*this) *= var(chars);return *this;}
	VARREF operator*=(const bool) &;

	// Subtract
	VARREF operator-=(const int) &;
	VARREF operator-=(const double) &;
	VARREF operator-=(const char  char1) &{(*this) -= var(char1);return *this;}
	VARREF operator-=(const char* chars) &{(*this) -= var(chars);return *this;}
	VARREF operator-=(const bool) &;

	// Divide
	VARREF operator/=(const int) &;
	VARREF operator/=(const double) &;
	VARREF operator/=(const char  char1) &{(*this) /= var(char1);return *this;}
	VARREF operator/=(const char* chars) &{(*this) /= var(chars);return *this;}
	VARREF operator/=(const bool) &;

	// Modulo
	VARREF operator%=(const int) &;
	VARREF operator%=(const double dbl1) &;
	VARREF operator%=(const char  char1) &{(*this) %= var(char1);return *this;}
	VARREF operator%=(const char* chars) &{(*this) %= var(chars);return *this;}
	VARREF operator%=(const bool  bool1) &{(*this) %= var(bool1);return *this;}

	// Concat
	VARREF operator^=(const int) &;
	VARREF operator^=(const double) &;
	VARREF operator^=(const char) &;
	VARREF operator^=(const char*) &;
	VARREF operator^=(const std::string&) &;


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

	//////////////////
	// ITERATOR FRIEND
	//////////////////

	friend class var_iter;

	//BEGIN/END - free functions to create iterators over a var
	PUBLIC ND friend var_iter begin(CVR v);
	PUBLIC ND friend var_iter end(CVR v);


	friend class dim;
	//friend class var_brackets_proxy;

	// clang-format off

	///////////////
	// FRIEND UTILS
	///////////////

	// Logical
	PUBLIC ND friend bool MVeq(CVR lhs, CVR rhs);
	PUBLIC ND friend bool MVlt(CVR lhs, CVR rhs);

	// Specialisations for speed

	PUBLIC ND friend bool MVeq_dbl( CVR          lhs,  const double dbl1 );
	PUBLIC ND friend bool MVeq_int( CVR          lhs,  const int    int1 );

	PUBLIC ND friend bool MVlt_int( CVR          lhs,  const int    int1 );
	PUBLIC ND friend bool MVlt_int( const int    int1, CVR          rhs  );

	PUBLIC ND friend bool MVlt_dbl( CVR          lhs,  const double dbl1 );
	PUBLIC ND friend bool MVlt_dbl( const double dbl1, CVR          rhs  );

	PUBLIC ND friend bool MVlt_bool(const bool   lhs,  const bool   int1 );

	// Concatenation

	PUBLIC ND friend var MVcat_var_var(CVR         lhs,  CVR         rhs);

	// Specialisations for speed

	PUBLIC ND friend var MVcat_var_cstr(CVR         lhs,  const char* cstr);
	PUBLIC ND friend var MVcat_var_char(CVR         lhs,  const char  char2);
	PUBLIC ND friend var MVcat_cstr_var(const char* cstr, CVR         rhs);

	//////////////////////////////
	// OPERATOR FRIENDS - pure var
	//////////////////////////////

	// Logical friends for var and var

	PUBLIC ND friend bool operator==(CVR lhs, CVR rhs){ return  MVeq(lhs, rhs); }
	PUBLIC ND friend bool operator!=(CVR lhs, CVR rhs){ return !MVeq(lhs, rhs); }
	PUBLIC ND friend bool operator< (CVR lhs, CVR rhs){ return  MVlt(lhs, rhs); }
	PUBLIC ND friend bool operator>=(CVR lhs, CVR rhs){ return !MVlt(lhs, rhs); }
	PUBLIC ND friend bool operator> (CVR lhs, CVR rhs){ return  MVlt(rhs, lhs); }
	PUBLIC ND friend bool operator<=(CVR lhs, CVR rhs){ return !MVlt(rhs, lhs); }

	// Arithmetic friends for var and var

	PUBLIC ND friend var  operator+(CVR  lhs, CVR rhs){ var rvo = lhs.clone(); rvo += rhs; return rvo;}
	PUBLIC ND friend var  operator*(CVR  lhs, CVR rhs){ var rvo = lhs.clone(); rvo *= rhs; return rvo;}
	PUBLIC ND friend var  operator-(CVR  lhs, CVR rhs){ var rvo = lhs.clone(); rvo -= rhs; return rvo;}
	PUBLIC ND friend var  operator/(CVR  lhs, CVR rhs){ var rvo = lhs.clone(); rvo /= rhs; return rvo;}
	PUBLIC ND friend var  operator%(CVR  lhs, CVR rhs){ var rvo = lhs.clone(); rvo %= rhs; return rvo;}

	// Concatenation friends for var and var

	PUBLIC ND friend var operator^(CVR lhs, CVR rhs){ return MVcat_var_var(lhs, rhs ); }
	PUBLIC ND friend var operator^(TVR lhs, CVR rhs){ return lhs ^= rhs ; }


	/////////////////////////////////
	// OPERATOR FRIENDS - mixed types
	/////////////////////////////////

	// LOGICAL friends v. main types

	//== and !=
	//TODO consider replacing by operator<=> and replacing MVeq and MVlt by MVcmp
	//or provide more specialisations of MVeq and MVlt esp. for numeric types

	// == EQ friends v. main types

	PUBLIC ND friend bool operator==(CVR          lhs,   const char*  cstr2 ){ return  MVeq(      lhs, cstr2  ); }
	PUBLIC ND friend bool operator==(CVR          lhs,   const char   char2 ){ return  MVeq(      lhs, char2  ); }
	PUBLIC ND friend bool operator==(CVR          lhs,   const int    int2  ){ return  MVeq_int(  lhs, int2   ); }
	PUBLIC ND friend bool operator==(CVR          lhs,   const double dbl2  ){ return  MVeq_dbl(  lhs, dbl2   ); }
	PUBLIC ND friend bool operator==(CVR          lhs,   const bool   bool2 ){ return  lhs.toBool() == bool2   ; }

	PUBLIC ND friend bool operator==(const char*  cstr1, CVR          rhs   ){ return  MVeq(      rhs, cstr1  ); }
	PUBLIC ND friend bool operator==(const char   char1, CVR          rhs   ){ return  MVeq(      rhs, char1  ); }
	PUBLIC ND friend bool operator==(const int    int1,  CVR          rhs   ){ return  MVeq_int(  rhs, int1   ); }
	PUBLIC ND friend bool operator==(const double dbl1,  CVR          rhs   ){ return  MVeq_dbl(  rhs, dbl1   ); }
	PUBLIC ND friend bool operator==(const bool   bool1, CVR          rhs   ){ return  rhs.toBool() == bool1   ; }

	// != NE friends v. main types

	PUBLIC ND friend bool operator!=(CVR          lhs,   const char*  cstr2 ){ return !MVeq(      lhs, cstr2  ); }
	PUBLIC ND friend bool operator!=(CVR          lhs,   const char   char2 ){ return !MVeq(      lhs, char2  ); }
	PUBLIC ND friend bool operator!=(CVR          lhs,   const int    int2  ){ return !MVeq_int(  lhs, int2   ); }
	PUBLIC ND friend bool operator!=(CVR          lhs,   const double dbl2  ){ return !MVeq_dbl(  lhs, dbl2   ); }
	PUBLIC ND friend bool operator!=(CVR          lhs,   const bool   bool2 ){ return lhs.toBool() != bool2    ; }

	PUBLIC ND friend bool operator!=(const char*  cstr1, CVR          rhs   ){ return !MVeq(      rhs, cstr1  ); }
	PUBLIC ND friend bool operator!=(const char   char1, CVR          rhs   ){ return !MVeq(      rhs, char1  ); }
	PUBLIC ND friend bool operator!=(const int    int1,  CVR          rhs   ){ return !MVeq_int(  rhs, int1   ); }
	PUBLIC ND friend bool operator!=(const double dbl1,  CVR          rhs   ){ return !MVeq_dbl(  rhs, dbl1   ); }
	PUBLIC ND friend bool operator!=(const bool   bool1, CVR          rhs   ){ return rhs.toBool() !=  bool1   ; }

	// < LT friends v. main types

	PUBLIC ND friend bool operator<(CVR           lhs,   const char*  cstr2 ){ return  MVlt(      lhs,   cstr2 ); }
	PUBLIC ND friend bool operator<(CVR           lhs,   const char   char2 ){ return  MVlt(      lhs,   char2 ); }
	PUBLIC ND friend bool operator<(CVR           lhs,   const int    int2  ){ return  MVlt_int(  lhs,   int2  ); }
	PUBLIC ND friend bool operator<(CVR           lhs,   const double dbl2  ){ return  MVlt_dbl(  lhs,   dbl2  ); }
	PUBLIC ND friend bool operator<(CVR           lhs,   const bool   bool1 ){ return  MVlt_bool( lhs,   bool1 ); }

	PUBLIC ND friend bool operator<(const char*   cstr1, CVR          rhs   ){ return  MVlt(      cstr1, rhs   ); }
	PUBLIC ND friend bool operator<(const char    char1, CVR          rhs   ){ return  MVlt(      char1, rhs   ); }
	PUBLIC ND friend bool operator<(const int     int1,  CVR          rhs   ){ return  MVlt_int(  int1,  rhs   ); }
	PUBLIC ND friend bool operator<(const double  dbl1,  CVR          rhs   ){ return  MVlt_dbl(  dbl1,  rhs   ); }
	PUBLIC ND friend bool operator<(const bool    bool1, CVR          rhs   ){ return  MVlt_bool( bool1, rhs   ); }

	// >= GE friends v. main types

	PUBLIC ND friend bool operator>=(CVR          lhs,   const char*  cstr2 ){ return !MVlt(      lhs,   cstr2 ); }
	PUBLIC ND friend bool operator>=(CVR          lhs,   const char   char2 ){ return !MVlt(      lhs,   char2 ); }
	PUBLIC ND friend bool operator>=(CVR          lhs,   const int    int2  ){ return !MVlt_int(  lhs,   int2  ); }
	PUBLIC ND friend bool operator>=(CVR          lhs,   const double dbl2  ){ return !MVlt_dbl(  lhs,   dbl2  ); }
	PUBLIC ND friend bool operator>=(CVR          lhs,   const bool   bool2 ){ return !MVlt_bool( lhs,   bool2 ); }

	PUBLIC ND friend bool operator>=(const char*  cstr1, CVR          rhs   ){ return !MVlt(      cstr1, rhs   ); }
	PUBLIC ND friend bool operator>=(const char   char1, CVR          rhs   ){ return !MVlt(      char1, rhs   ); }
	PUBLIC ND friend bool operator>=(const int    int1,  CVR          rhs   ){ return !MVlt_int(  int1,  rhs   ); }
	PUBLIC ND friend bool operator>=(const double dbl1,  CVR          rhs   ){ return !MVlt_dbl(  dbl1,  rhs   ); }
	PUBLIC ND friend bool operator>=(const bool   bool1, CVR          rhs   ){ return !MVlt_bool( bool1, rhs   ); }

	// > GT friends v. main types

	PUBLIC ND friend bool operator>(CVR           lhs,   const char*  cstr2 ){ return  MVlt(      cstr2, lhs   ); }
	PUBLIC ND friend bool operator>(CVR           lhs,   const char   char2 ){ return  MVlt(      char2, lhs   ); }
	PUBLIC ND friend bool operator>(CVR           lhs,   const int    int2  ){ return  MVlt_int(  int2,  lhs   ); }
	PUBLIC ND friend bool operator>(CVR           lhs,   const double dbl2  ){ return  MVlt_dbl(  dbl2,  lhs   ); }
	PUBLIC ND friend bool operator>(CVR           lhs,   const bool   bool2 ){ return  MVlt_bool( bool2, lhs   ); }

	PUBLIC ND friend bool operator>(const char*   cstr1, CVR          rhs   ){ return  MVlt(      rhs,   cstr1 ); }
	PUBLIC ND friend bool operator>(const char    char1, CVR          rhs   ){ return  MVlt(      rhs,   char1 ); }
	PUBLIC ND friend bool operator>(const int     int1,  CVR          rhs   ){ return  MVlt_int(  rhs,   int1  ); }
	PUBLIC ND friend bool operator>(const double  dbl1,  CVR          rhs   ){ return  MVlt_dbl(  rhs,   dbl1  ); }
	PUBLIC ND friend bool operator>(const bool    bool1, CVR          rhs   ){ return  MVlt_bool( rhs,   bool1 ); }

	// <= LE friends v. main types

	PUBLIC ND friend bool operator<=(CVR          lhs,   const char*  cstr2 ){ return !MVlt(      cstr2, lhs   ); }
	PUBLIC ND friend bool operator<=(CVR          lhs,   const char   char2 ){ return !MVlt(      char2, lhs   ); }
	PUBLIC ND friend bool operator<=(CVR          lhs,   const int    int2  ){ return !MVlt_int(  int2,  lhs   ); }
	PUBLIC ND friend bool operator<=(CVR          lhs,   const double dbl2  ){ return !MVlt_dbl(  dbl2,  lhs   ); }
	PUBLIC ND friend bool operator<=(CVR          lhs,   const bool   bool2 ){ return !MVlt_bool( bool2, lhs   ); }

	PUBLIC ND friend bool operator<=(const char*  cstr1, CVR          rhs   ){ return !MVlt(      rhs,   cstr1 ); }
	PUBLIC ND friend bool operator<=(const char   char1, CVR          rhs   ){ return !MVlt(      rhs,   char1 ); }
	PUBLIC ND friend bool operator<=(const int    int1,  CVR          rhs   ){ return !MVlt_int(  rhs,   int1  ); }
	PUBLIC ND friend bool operator<=(const double dbl1,  CVR          rhs   ){ return !MVlt_dbl(  rhs,   dbl1  ); }
	PUBLIC ND friend bool operator<=(const bool   bool1, CVR          rhs   ){ return !MVlt_bool( rhs,   bool1 ); }

	// ARITHMETIC friends v. main types

	// PLUS  friends v. main types

	PUBLIC ND friend var operator+(CVR            lhs,   const char*  cstr2 ){ var rvo = lhs.clone(); rvo    += cstr2; return rvo;}
	PUBLIC ND friend var operator+(CVR            lhs,   const char   char2 ){ var rvo = lhs.clone(); rvo    += char2; return rvo;}
	PUBLIC ND friend var operator+(CVR            lhs,   const int    int2  ){ var rvo = lhs.clone(); rvo    += int2;  return rvo;}
	PUBLIC ND friend var operator+(CVR            lhs,   const double dbl2  ){ var rvo = lhs.clone(); rvo    += dbl2;  return rvo;}
	PUBLIC ND friend var operator+(CVR            lhs,   const bool   bool2 ){ var rvo = lhs.clone(); rvo    += bool2; return rvo;}

	PUBLIC ND friend var operator+(const char*    cstr1, CVR          rhs   ){ var rvo = rhs.clone(); rvo    += cstr1; return rvo;}
	PUBLIC ND friend var operator+(const char     char1, CVR          rhs   ){ var rvo = rhs.clone(); rvo    += char1; return rvo;}
	PUBLIC ND friend var operator+(const int      int1,  CVR          rhs   ){ var rvo = rhs.clone(); rvo    += int1;  return rvo;}
	PUBLIC ND friend var operator+(const double   dbl1,  CVR          rhs   ){ var rvo = rhs.clone(); rvo    += dbl1;  return rvo;}
	PUBLIC ND friend var operator+(const bool     bool1, CVR          rhs   ){ var rvo = rhs.clone(); rvo    += bool1; return rvo;}

	// MULTIPLY  friends v. main types

	PUBLIC ND friend var operator*(CVR            lhs,   const char*  cstr2 ){ var rvo = lhs.clone(); rvo    *= cstr2; return rvo;}
	PUBLIC ND friend var operator*(CVR            lhs,   const char   char2 ){ var rvo = lhs.clone(); rvo    *= char2; return rvo;}
	PUBLIC ND friend var operator*(CVR            lhs,   const int    int2  ){ var rvo = lhs.clone(); rvo    *= int2;  return rvo;}
	PUBLIC ND friend var operator*(CVR            lhs,   const double dbl2  ){ var rvo = lhs.clone(); rvo    *= dbl2;  return rvo;}
	PUBLIC ND friend var operator*(CVR            lhs,   const bool   bool2 ){ var rvo = lhs.clone(); rvo    *= bool2; return rvo;}

	PUBLIC ND friend var operator*(const char*    cstr1, CVR          rhs   ){ var rvo = rhs.clone(); rvo    *= cstr1; return rvo;}
	PUBLIC ND friend var operator*(const char     char1, CVR          rhs   ){ var rvo = rhs.clone(); rvo    *= char1; return rvo;}
	PUBLIC ND friend var operator*(const int      int1,  CVR          rhs   ){ var rvo = rhs.clone(); rvo    *= int1;  return rvo;}
	PUBLIC ND friend var operator*(const double   dbl1,  CVR          rhs   ){ var rvo = rhs.clone(); rvo    *= dbl1;  return rvo;}
	PUBLIC ND friend var operator*(const bool     bool1, CVR          rhs   ){ var rvo = rhs.clone(); rvo    *= bool1; return rvo;}

	// MINUS  friends v. main types

	PUBLIC ND friend var operator-(CVR            lhs,   const char*  cstr2 ){ var rvo = lhs.clone();   rvo  -= cstr2; return rvo;}
	PUBLIC ND friend var operator-(CVR            lhs,   const char   char2 ){ var rvo = lhs.clone();   rvo  -= char2; return rvo;}
	PUBLIC ND friend var operator-(CVR            lhs,   const int    int2  ){ var rvo = lhs.clone();   rvo  -= int2;  return rvo;}
	PUBLIC ND friend var operator-(CVR            lhs,   const double dbl2  ){ var rvo = lhs.clone();   rvo  -= dbl2;  return rvo;}
	PUBLIC ND friend var operator-(CVR            lhs,   const bool   bool2 ){ var rvo = lhs.clone();   rvo  -= bool2; return rvo;}

	PUBLIC ND friend var operator-(const char*    cstr1, CVR          rhs   ){ var rvo = cstr1; rvo  -= rhs;   return rvo;}
	PUBLIC ND friend var operator-(const char     char1, CVR          rhs   ){ var rvo = char1; rvo  -= rhs;   return rvo;}
	PUBLIC ND friend var operator-(const int      int1,  CVR          rhs   ){ var rvo = int1;  rvo  -= rhs;   return rvo;}
	PUBLIC ND friend var operator-(const double   dbl1,  CVR          rhs   ){ var rvo = dbl1;  rvo  -= rhs;   return rvo;}
	PUBLIC ND friend var operator-(const bool     bool1, CVR          rhs   ){ var rvo = bool1; rvo  -= rhs;   return rvo;}

	// DIVIDE  friends v. main types

	PUBLIC ND friend var operator/(CVR            lhs,   const char*  cstr2 ){ var rvo = lhs.clone();   rvo  /= cstr2; return rvo;}
	PUBLIC ND friend var operator/(CVR            lhs,   const char   char2 ){ var rvo = lhs.clone();   rvo  /= char2; return rvo;}
	PUBLIC ND friend var operator/(CVR            lhs,   const int    int2  ){ var rvo = lhs.clone();   rvo  /= int2;  return rvo;}
	PUBLIC ND friend var operator/(CVR            lhs,   const double dbl2  ){ var rvo = lhs.clone();   rvo  /= dbl2;  return rvo;}
//	PUBLIC ND friend var operator/(CVR            lhs,   const bool   bool2 ){ var rvo = lhs.clone();   rvo  /= bool2; return rvo;} // Either does nothing or throws divide by zero

	PUBLIC ND friend var operator/(const char*    cstr1, CVR          rhs   ){ var rvo = cstr1; rvo  /= rhs;   return rvo;}
	PUBLIC ND friend var operator/(const char     char1, CVR          rhs   ){ var rvo = char1; rvo  /= rhs;   return rvo;}
	PUBLIC ND friend var operator/(const int      int1,  CVR          rhs   ){ var rvo = int1;  rvo  /= rhs;   return rvo;}
	PUBLIC ND friend var operator/(const double   dbl1,  CVR          rhs   ){ var rvo = dbl1;  rvo  /= rhs;   return rvo;}
//	PUBLIC ND friend var operator/(const bool     bool1, CVR          rhs   ){ var rvo = bool1 ;rvo /= rhs;   return rvo;} // Almost meaningless

	// MODULO  friends v. main types

	PUBLIC ND friend var operator%(CVR            lhs,   const char*  cstr2 ){ var rvo = lhs.clone();   rvo  %= cstr2; return rvo;}
	PUBLIC ND friend var operator%(CVR            lhs,   const char   char2 ){ var rvo = lhs.clone();   rvo  %= char2; return rvo;}
	PUBLIC ND friend var operator%(CVR            lhs,   const int    int2  ){ var rvo = lhs.clone();   rvo  %= int2;  return rvo;}
	PUBLIC ND friend var operator%(CVR            lhs,   const double dbl2  ){ var rvo = lhs.clone();   rvo  %= dbl2;  return rvo;}
//	PUBLIC ND friend var operator%(CVR            lhs,   const bool   bool2 ){ var rvo = lhs.clone();   rvo  %= bool2; return rvo;} // Rather useless or throws divide by zero

	PUBLIC ND friend var operator%(const char*    cstr1, CVR          rhs   ){ var rvo = cstr1; rvo  %= rhs;   return rvo;}
	PUBLIC ND friend var operator%(const char     char1, CVR          rhs   ){ var rvo = char1; rvo  %= rhs;   return rvo;}
	PUBLIC ND friend var operator%(const int      int1,  CVR          rhs   ){ var rvo = int1;  rvo  %= rhs;   return rvo;}
	PUBLIC ND friend var operator%(const double   dbl1,  CVR          rhs   ){ var rvo = dbl1;  rvo  %= rhs;   return rvo;}
	//PUBLIC ND friend var operator%(const bool   bool1, CVR          rhs   ){ var rvo = bool1 ;rvo  %= rhs;   return rvo;} // Almost meaningless

	// STRING CONCATENATE  friends v. main types

	// NB do *NOT* support concatenate with bool or vice versa!
	// to avoid compiler doing wrong precendence issue between ^ and logical operators
	//remove this to avoid some gcc ambiguous warnings although it means concat std:string will create a temp var
	//PUBLIC ND friend var operator^(CVR          lhs,   const std::string str2 ){ return MVcat(lhs,    var(str2) ); }
	PUBLIC ND friend var operator^(CVR            lhs,   const char*  cstr  ){ return MVcat_var_cstr( lhs,        cstr  ); }
	PUBLIC ND friend var operator^(CVR            lhs,   const char   char2 ){ return MVcat_var_char( lhs,        char2 ); }
	PUBLIC ND friend var operator^(CVR            lhs,   const int    int2  ){ return MVcat_var_var(  lhs,        int2  ); }
	PUBLIC ND friend var operator^(CVR            lhs,   const double dbl2  ){ return MVcat_var_var(  lhs,        dbl2  ); }
	PUBLIC ND friend var operator^(const char*    cstr,  CVR          rhs   ){ return MVcat_cstr_var( cstr,       rhs   ); }
	PUBLIC ND friend var operator^(const char     char1, CVR          rhs   ){ return MVcat_var_var(  char1,      rhs   ); }
	PUBLIC ND friend var operator^(const int      int1,  CVR          rhs   ){ return MVcat_var_var(  int1,       rhs   ); }
	PUBLIC ND friend var operator^(const double   dbl1,  CVR          rhs   ){ return MVcat_var_var(  dbl1,       rhs   ); }

	//temporaries (rvalues) - mutate the temporary string to save a copy
	PUBLIC ND friend var operator^(TVR            lhs,   const char*  cstr2 ){ return lhs ^= cstr2 ; }
	PUBLIC ND friend var operator^(TVR            lhs,   const char   char2 ){ return lhs ^= char2 ; }
	PUBLIC ND friend var operator^(TVR            lhs,   const int    int2  ){ return lhs ^= int2  ; }
	PUBLIC ND friend var operator^(TVR            lhs,   const double dbl2  ){ return lhs ^= dbl2  ; }

	// clang-format on

	// OS TIME/DATE
	///////////////

	ND var date() const;
	ND var time() const;
	ND var timedate() const;
	void ossleep(const int milliseconds) const;
	var oswait(const int milliseconds, CVR directory) const;
	ND var ostime() const;

	void breakon() const;
	void breakoff() const;

	// OUTPUT
	/////////

	//to stdout/cout
	CVR output() const;      //output without line ending, without flush
	CVR outputl() const;     //output without a line ending and flush
	CVR outputt() const;     //output with a tab and flush

	//as above but with var1 prefixed
	CVR output(CVR var1) const;
	CVR outputl(CVR var1) const;
	CVR outputt(CVR var1) const;

	//to stdlog/clog
	CVR logput() const;
	CVR logputl() const;

	//as above but with var1 prefixed
	CVR logput(CVR var1) const;
	CVR logputl(CVR var1) const;

	//to stderr/cerr
	CVR errput() const;
	CVR errputl() const;

	//as above but with var1 prefixed
	CVR errput(CVR var1) const;
	CVR errputl(CVR var1) const;

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
	VARREF unassigned(CVR defaultvalue);

	VARREF transfer(VARREF destinationvar);
	// exchange is marked as const despite it swapping the var with var2
	// Currently this is required in rare cases where functions like mvprogram::calculate
	// temporarily require member variables to be something else but switch back before exiting
	// if such function throws then it would leave the member variables in a changed state.
	CVR exchange(CVR var2) const;//version that works on const vars
	VARREF exchange(VARREF var2);//version that works on non-const vars
	var clone() const {
		var clone;
		clone.var_typ = var_typ;
		clone.var_str = var_str;
		clone.var_int = var_int;
		clone.var_dbl = var_dbl;
		return clone;
	}

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
	// integer() represents pick int() because int() is reserved word in c/c++
	// Note that integer like pick int() is the same as floor()
	// whereas the usual c/c++ int() simply take the next integer nearest 0 (ie cuts of any
	// fractional decimal places) to get the usual c/c++ effect use toInt() (although toInt()
	// returns an int instead of a var like normal exodus functions)
	ND var integer() const;
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

	// STRING INFO
	//////////////

	// bool match(CVR matchstr,CVR options DEFAULT_EMPTY) const;
	ND var match(CVR matchstr, SV options DEFAULT_EMPTY) const;
	ND var seq() const;     // ASCII
	ND var textseq() const; // TEXT
	ND var dcount(SV str) const;
	ND var count(SV str) const;
	//ND var count(const char charx) const;
	ND var length() const;
	ND var len() const;
	//const char* data() const;
	bool isnum() const;

	ND bool starts(SV str) const;
	ND bool ends(SV str) const;
	ND bool contains(SV str) const;

	//static member for speed on std strings
	static int localeAwareCompare(const std::string& str1, const std::string& str2);
	//int localeAwareCompare(const std::string& str2) const;

	// STRING MUTATORS
	//////////////////

	// all return VARREF and are not const)

	//var& converter(std::string_view oldchars, std::string_view newchars);
	VARREF converter(SV oldchars, SV newchars);
	//VARREF converter(const char* oldchars, const char* newchars);
	VARREF textconverter(SV oldchars, SV newchars);
	VARREF swapper(SV whatstr, SV withstr);
	VARREF regex_replacer(CVR regexstr, CVR replacementstr, SV options DEFAULT_EMPTY);
	VARREF splicer(const int start1, const int length, SV insertstr);
	VARREF splicer(const int start1, SV insertstr);
	VARREF popper();
	VARREF quoter();
	VARREF squoter();
	VARREF unquoter();
	VARREF ucaser();        // utf8
	VARREF lcaser();        // utf8
	VARREF tcaser();        // utf8
	VARREF fcaser();        // utf8
	VARREF normalizer();    // utf8
	VARREF inverter();    // utf8
	VARREF trimmer(SV trimchars DEFAULT_SPACE);
	VARREF trimmerf(SV trimchars DEFAULT_SPACE);
	VARREF trimmerb(SV trimchars DEFAULT_SPACE);
	VARREF trimmer(SV trimchars, SV options);
	VARREF fieldstorer(CVR sepchar, const int fieldno, const int nfields, CVR replacement);
	VARREF substrer(const int startindex);
	VARREF substrer(const int startindex, const int length);

	VARREF sorter(SV sepchar = _FM_);

	VARREF lowerer();
	VARREF raiser();
	VARREF cropper();
	VARREF uniquer();

	// SAME ON TEMPORARIES
	//////////////////////

	ND VARREF convert(SV oldchars, SV newchars) &&;
	ND VARREF textconvert(SV oldchars, SV newchars) &&;
	ND VARREF swap(SV whatstr, SV withstr) &&;
	ND VARREF regex_replace(CVR regexstr, CVR replacementstr, SV options DEFAULT_EMPTY) &&;
	ND VARREF splice(const int start1, const int length, SV insertstr) &&;
	ND VARREF splice(const int start1, SV insertstr) &&;
	ND VARREF pop() &&;
	ND VARREF quote() &&;
	ND VARREF squote() &&;
	ND VARREF unquote() &&;
	ND VARREF ucase() &&;     // utf8
	ND VARREF lcase() &&;     // utf8
	ND VARREF tcase() &&;     // utf8
	ND VARREF fcase() &&;     // utf8
	ND VARREF normalize() &&; // utf8
	ND VARREF invert() &&;    // utf8
	ND VARREF trim(SV trimchars DEFAULT_SPACE) && {return this->trimmer(trimchars);};
	ND VARREF trimf(SV trimchars DEFAULT_SPACE) && {return this->trimmerf(trimchars);};
	ND VARREF trimb(SV trimchars DEFAULT_SPACE) && {return this->trimmerb(trimchars);};
	ND VARREF trim(SV trimchars, SV options) && {return this->trimmer(trimchars, options);};
	ND VARREF fieldstore(CVR sepchar, const int fieldno, const int nfields, CVR replacement) &&;
	ND VARREF substr(const int startindex) &&;
	ND VARREF substr(const int startindex, const int length) &&;

	ND VARREF lower() &&;
	ND VARREF raise() &&;
	ND VARREF crop() &&;

	// SAME BUT NON-MUTATING
	////////////////////////

	// all are const

	ND var convert(SV oldchars, SV newchars) const&;
	ND var textconvert(SV oldchars, SV newchars) const&;
	ND var swap(SV whatstr, SV withstr) const&;
	ND var regex_replace(CVR regexstr, CVR replacementstr, SV options DEFAULT_EMPTY) const&;
	ND var splice(const int start1, const int length, SV insertstr) const&;
	ND var splice(const int start1, SV insertstr) const&;
	ND var pop() const&;
	ND var quote() const&;
	ND var squote() const&;
	ND var unquote() const&;
	ND var ucase() const&;        // utf8
	ND var lcase() const&;        // utf8
	ND var tcase() const&;        // utf8
	ND var fcase() const&;        // utf8
	ND var normalize() const&;    // utf8
	ND var invert() const&;        // utf8
	ND var trim(SV trimchars DEFAULT_SPACE) const&;
	ND var trimf(SV trimchars DEFAULT_SPACE) const&;
	ND var trimb(SV trimchars DEFAULT_SPACE) const&;
	ND var trim(SV trimchars, SV options) const&;
	ND var fieldstore(CVR sepchar, const int fieldno, const int nfields, CVR replacement) const&;

	ND var lower() const&;
	ND var raise() const&;
	ND var crop() const&;

	// OTHER STRING ACCESS
	//////////////////////

	ND var hash(const uint64_t modulus = 0) const;
	ND var unique() const;

	// CONVERT TO DIM (returns a dim)
	// see also dim.split()
	ND dim split(SV sepchar = _FM_) const;

	ND var sort(SV sepchar = _FM_) const;

	// STRING EXTRACTION varx[x,y] -> varx.substr(start,length)

	// NOTE char=byte ... NOT utf-8 code point
	// NOTE 1 based indexing. byte 1 = first byte as per mv conventions for all indexing (except
	// offset in osbread) NOTE start byte may be negative to count backwards -1=last byte

	// v1 - returns bytes from some char number up to the end of the string
	// equivalent to substr(x) from javascript except it is 1 based
	ND var substr(const int startindex) const&;

	// v2 - returns a given number of bytes starting from some byte
	// both start and length can be negative
	// negative length extracts characters up to the starting byte IN REVERSE
	// 'abcde'.substr(4,-3) -> 'dcb'
	ND var substr(const int startindex, const int length) const&;

	// v3 - returns bytes from some byte number upto the first of a given list of bytes
	// this is something like std::string::find_first_of but doesnt return the delimiter found
	var substr(const int startindex, CVR delimiterchars, int& endindex) const;

	// v4 - like v3. was named "remove" in pick. notably used in nlist to print parallel columns
	// of mixed combinations of multivalues/subvalues and text marks
	// correctly lined up mv to mv, sv to sv, tm to tm even when particular columns were missing
	// some vm/sm/tm
	// it is like substr(startindex,delimiterbytes,endindex) except that the delimiter bytes are
	// hard coded as the usual RM/FM/VM/SM/TM/STM
	// except that it updates the startstopindex to point one after found delimiter byte and
	// returns the delimiter no (1-6)
	// if no delimiter byte is found then it returns bytes up to the end of the string, sets
	// startstopindex to after tne end of the string and returns delimiter no 0 NOTE that it
	// does NOT remove anything from the source string var remove(VARREF startindex, VARREF
	// delimiterno) const;
	var substr2(VARREF startstopindex, VARREF delimiterno) const;

	ND var index(CVR substr, const int occurrenceno = 1) const;
	ND var index2(CVR substr, const int startchar1 = 1) const;
	ND var field(CVR substrx, const int fieldnx = 1, const int nfieldsx = 1) const;
	// version that treats fieldn -1 as the last field, -2 the penultimate field etc. - TODO
	// should probably make field() do this
	ND var field2(CVR substrx, const int fieldnx, const int nfieldsx = 1) const;

	// I/O CONVERSION
	/////////////////

	ND var oconv(const char* convstr) const;
	ND var iconv(const char* convstr) const;

	ND var from_codepage(const char* codepage) const;
	ND var to_codepage(const char* codepage) const;

	// CLASSIC MV STRING FUNCTIONS
	/////////////////////////////

	// this function hardly occurs anywhere in exodus code and should probably be renamed to
	// something better it was called replace() in pickos but we are now using "replace()" to
	// change substrings using regex (similar to the old pickos swap function) its mutator function
	// is .r()
	ND var pickreplace(const int fieldno, const int valueno, const int subvalueno, CVR replacement) const;
	ND var pickreplace(const int fieldno, const int valueno, CVR replacement) const;
	ND var pickreplace(const int fieldno, CVR replacement) const;

	// cf mutator inserter()
	ND var insert(const int fieldno, const int valueno, const int subvalueno, CVR insertion) const&;
	ND var insert(const int fieldno, const int valueno, CVR insertion) const&;
	ND var insert(const int fieldno, CVR insertion) const&;

	/// remove() was delete() in pickos
	// var erase(const int fieldno, const int valueno=0, const int subvalueno=0) const;
	ND var remove(const int fieldno, const int valueno = 0, const int subvalueno = 0) const;

	//.f(...) stands for .attribute(...) or extract(...)
	// pickos
	// xxx=yyy<10>";
	// becomes c++
	// xxx=yyy.f(10);
	ND var f(const int fieldno, const int valueno = 0, const int subvalueno = 0) const;
	ND var extract(const int fieldno, const int valueno = 0, const int subvalueno = 0) const;

	// SAME AS ABOVE ON TEMPORARIES
	///////////////////////////////

	ND VARREF insert(const int fieldno, const int valueno, const int subvalueno, CVR insertion) &&;
	ND VARREF insert(const int fieldno, const int valueno, CVR insertion) &&;
	ND VARREF insert(const int fieldno, CVR insertion) &&;

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
	////////////////////

	// mutable versions update and return source
	// r stands for "replacer" abbreviated due to high incidience in code
	// pickos
	// xyz<10>="abc";
	// becomes c++
	//  xyz.r(10,"abc");

	// r() is short for replacer() since it is probably the most common var function after a()
	VARREF r(const int fieldno, const int valueno, const int subvalueno, CVR replacement);
	VARREF r(const int fieldno, const int valueno, CVR replacement);
	VARREF r(const int fieldno, CVR replacement);

	VARREF inserter(const int fieldno, const int valueno, const int subvalueno, CVR insertion);
	VARREF inserter(const int fieldno, const int valueno, CVR insertion);
	VARREF inserter(const int fieldno, CVR insertion);

	// VARREF eraser(const int fieldno, const int valueno=0, const int subvalueno=0);
	VARREF remover(const int fieldno, const int valueno = 0, const int subvalueno = 0);
	//-er version could be extract and erase in one go
	// VARREF extracter(int fieldno,int valueno=0,int subvalueno=0) const;

	// MV STRING LOCATORS
	////////////////////

	// should these be like extract, replace, insert, delete
	// locate(fieldno, valueno, subvalueno,target,setting,by DEFAULT_EMPTY)
	ND bool locate(CVR target) const;
	bool locate(CVR target, VARREF setting) const;
	bool locate(CVR target, VARREF setting, const int fieldno, const int valueno = 0) const;

	bool locateusing(const char* usingchar, CVR target, VARREF setting, const int fieldno = 0, const int valueno = 0, const int subvalueno = 0) const;
	bool locateusing(const char* usingchar, CVR target) const;

	// locateby without fieldno or valueno arguments uses character VM
	bool locateby(const char* ordercode, CVR target, VARREF setting) const;

	// locateby with fieldno=0 uses character FM
	bool locateby(const char* ordercode, CVR target, VARREF setting, const int fieldno, const int valueno = 0) const;

	// locatebyusing
	bool locatebyusing(const char* ordercode, const char* usingchar, CVR target, VARREF setting, const int fieldno = 0, const int valueno = 0, const int subvalueno = 0) const;

	// DATABASE MANAGEMENT
	/////////////////////

	bool dbcreate(CVR dbname) const;
	ND var dblist() const;
	bool dbcopy(CVR from_dbname, CVR to_dbname) const;
	bool dbdelete(CVR dbname) const;

	bool createfile(CVR filename) const;
	bool renamefile(CVR filename, CVR newfilename) const;
	bool deletefile(CVR filename) const;
	bool clearfile(CVR filename) const;
	ND var listfiles() const;

	bool createindex(CVR fieldname, CVR dictfile DEFAULT_EMPTY) const;
	bool deleteindex(CVR fieldname) const;
	ND var listindexes(CVR filename DEFAULT_EMPTY, CVR fieldname DEFAULT_EMPTY) const;

	bool sqlexec(CVR sqlcmd) const;
	bool sqlexec(CVR sqlcmd, VARREF response) const;

	// bool selftest() const;
	ND var version() const;

	ND var lasterror() const;
	void lasterror(CVR msg) const;


	// DATABASE ACCESS
	/////////////////

	bool connect(CVR conninfo DEFAULT_EMPTY);
	void disconnect();
	void disconnectall();

	bool attach(CVR filenames);
	void detach(CVR filenames);

	// var() is a db connection or default connection
	bool begintrans() const;
	bool rollbacktrans() const;
	bool committrans() const;
	bool statustrans() const;
	void clearcache() const;

	ND var reccount(CVR filename DEFAULT_EMPTY) const;
	var flushindex(CVR filename DEFAULT_EMPTY) const;

	bool open(CVR dbfilename, CVR connection DEFAULT_EMPTY);
	void close();

	// 1=ok, 0=failed, ""=already locked
	var lock(CVR key) const;
	// void unlock(CVR key) const;
	// void unlockall() const;
	bool unlock(CVR key) const;
	bool unlockall() const;

	// db file i/o
	bool read(CVR filehandle, CVR key);
	bool write(CVR filehandle, CVR key) const;
	bool deleterecord(CVR key) const;
	bool updaterecord(CVR filehandle, CVR key) const;
	bool insertrecord(CVR filehandle, CVR key) const;

	// specific db field i/o
	bool readv(CVR filehandle, CVR key, const int fieldno);
	bool writev(CVR filehandle, CVR key, const int fieldno) const;

	// cached db file i/o
	bool reado(CVR filehandle, CVR key);
	bool writeo(CVR filehandle, CVR key) const;
	bool deleteo(CVR key) const;

	// MvEnvironment function now to allow access to RECORD ID DICT etc. and call external
	// functions
	// var calculate() const;

	ND var xlate(CVR filename, CVR fieldno, const char* mode) const;

	// DATABASE SORT/SELECT
	//////////////////////

	bool select(CVR sortselectclause DEFAULT_EMPTY);
	void clearselect();

	//ND bool hasnext() const;
	ND bool hasnext();
	bool readnext(VARREF key);
	bool readnext(VARREF key, VARREF valueno);
	bool readnext(VARREF record, VARREF key, VARREF valueno);

	bool savelist(CVR listname);
	bool getlist(CVR listname);
	bool makelist(CVR listname, CVR keys);
	bool deletelist(CVR listname) const;
	bool formlist(CVR keys, CVR fieldno = 0);

	//bool saveselect(CVR filename);

	// OS FILE SYSTEM
	/////////////////

	bool osopen(CVR filename, const char* locale DEFAULT_EMPTY) const;
	bool osbread(CVR osfilevar, VARREF offset, const int length);
	bool osbwrite(CVR osfilevar, VARREF offset) const;
//#define VAR_OSBREADWRITE_CONST_OFFSET
#ifdef VAR_OSBREADWRITE_CONST_OFFSET
	bool osbread(CVR osfilevar, CVR offset, const int length);
	bool osbwrite(CVR osfilevar, CVR offset) const;
#endif
	void osclose() const;

	bool osread(CVR osfilename, const char* codepage DEFAULT_EMPTY);
	bool oswrite(CVR osfilename, const char* codepage DEFAULT_EMPTY) const;
	bool osremove() const;
	bool osremove(CVR osfilename) const;
	bool osrename(CVR newosdir_or_filename) const;
	bool oscopy(CVR to_osfilename) const;
	bool osmove(CVR to_osfilename) const;

	ND var oslist(CVR path DEFAULT_DOT, CVR globpattern DEFAULT_EMPTY, const int mode = 0) const;
	ND var oslistf(CVR path DEFAULT_DOT, CVR globpattern DEFAULT_EMPTY) const;
	ND var oslistd(CVR path DEFAULT_DOT, CVR globpattern DEFAULT_EMPTY) const;
	ND var osfile() const;
	ND var osdir() const;
	bool osmkdir() const;
	bool osrmdir(bool evenifnotempty = false) const;

	// TODO check for threadsafe
	ND var ospid() const;
	ND var oscwd() const;
	bool oscwd(CVR newpath) const;
	void osflush() const;

	// TODO add performance enhancing char* argumented versions of many os functions
	// to avoid unnecessary conversion to and from var format
	// same again but this time allowing native strings without needing automatic conversion of
	// var->char* this is to only to avoid convertion too and from var but will usage of hard
	// coded filenames etc really be in fast loops and performance related? perhaps only provide
	bool osread(const char* osfilename, const char* codepage DEFAULT_EMPTY);

	// OS SHELL/ENVIRONMENT
	///////////////////////

	bool osshell() const;
	bool osshellread(CVR oscmd);
	bool osshellwrite(CVR oscmd) const;
	ND var ostempdirpath() const;
	ND var ostempfilename() const;

	bool osgetenv(const char* name);
	bool ossetenv(const char* name) const;

	// EXECUTE/PERFORM CONTROL
	/////////////////////////

	// done in mvprogram now since they need to pass mvenvironment
	// var perform() const;
	// var execute() const;
	// chain should be similar to one of the above?
	// var chain() const;

	void stop(CVR text DEFAULT_EMPTY) const;
	void abort(CVR text DEFAULT_EMPTY) const;
	void abortall(CVR text DEFAULT_EMPTY) const;

	var logoff() const;

//	void debug(CVR DEFAULT_EMPTY) const;
//	ND var backtrace() const;

	CVR dump(SV text DEFAULT_EMPTY) const;
	VARREF dump(SV text DEFAULT_EMPTY);

	///////////////////////////
	// PRIVATE MEMBER FUNCTIONS
	///////////////////////////

   private:

	void throwUndefined(CVR message) const;
	void throwUnassigned(CVR message) const;
	void throwNonNumeric(CVR message) const;

	// WARNING: MUST not use any var when checking Undefined
	// otherwise will get recursion/segfault
	INLINE void assertDefined(const char* message, const char* varname = "") const {
		if (var_typ & VARTYP_MASK)
			throwUndefined(var(varname) ^ " in " ^ message);
	}

	INLINE void assertAssigned(const char* message, const char* varname = "") const {
		assertDefined(message, varname);
		if (!var_typ)
			throwUnassigned(var(varname) ^ " in " ^ message);
	}

	INLINE void assertNumeric(const char* message, const char* varname = "") const {
		if (!this->isnum())
			throwNonNumeric(var(varname) ^ " in " ^ var(message) ^ " data: " ^ this->substr(1, 128).quote());
	}

	INLINE void assertDecimal(const char* message, const char* varname = "") const {
		assertNumeric(message, varname);
		if (!(var_typ & VARTYP_DBL)) {
			var_dbl = double(var_int);
			// Add double flag
			var_typ |= VARTYP_DBL;
		}
	}

	INLINE void assertInteger(const char* message, const char* varname = "") const {
		assertNumeric(message, varname);
		if (!(var_typ & VARTYP_INT)) {
			var_int = std::floor(var_dbl);
			// Add int flag
			var_typ |= VARTYP_INT;
		}
	}

	INLINE void assertString(const char* message, const char* varname = "") const {
		assertDefined(message, varname);
		if (!(var_typ & VARTYP_STR)) {
			if (!var_typ)
				throwUnassigned(var(varname) ^ " in " ^ message);
			this->createString();
		};
	}

	INLINE void assertStringMutator(const char* message, const char* varname = "") const {
		assertString(message, varname);
		// Very important:
		// Reset all flags to ensure they are evaluated again
		// after string mutation
		var_typ = VARTYP_STR;
	}

	// Constructor
	void from_u32string(std::u32string u32str) const;

	void createString() const;

	ND bool cursorexists();//database, not terminal
	bool selectx(CVR fieldnames, CVR sortselectclause);

	// retrieves cid from *this, or uses default connection, or autoconnect with default
	// connection string On return *this contains connection ID and type VARTYP_NANSTR_DBCONN
	//int getconnectionid_ordefault() const;
	//int getconnectionid() const;

	// finds connection of this variable:
	// if this is not filename SQLOPENED variable, returns thread default connection or attempts
	// a default connect()
	// returning a void pointer in order to not have to include postgres headers in mv.h
	// will have to be cast to (PGconn *)
	//void* connection() const;

	// gets lock_table, associated with connection, associated with this object
	//void* get_lock_table() const;
	//void* get_mvconnection() const;

	ND var build_conn_info(CVR conninfo) const;

	//var getdictexpression(CVR mainfilename, CVR filename, CVR dictfilename, CVR dictfile, CVR fieldname, VARREF joins, VARREF froms, VARREF selects, VARREF ismv, bool forsort_or_select_or_index = false) const;

	// TODO check if can speed up by returning reference to converted self like MC
	// left/right justification
	ND var oconv_LRC(CVR format) const;
	// text justification
	ND var oconv_T(CVR format) const;
	// date
	ND var oconv_D(const char* conversion) const;
	// time
	ND var oconv_MT(const char* conversion) const;
	// decimal
	ND var oconv_MD(const char* conversion) const;
	// character replacement
	ND VARREF oconv_MR(const char* conversion);
	// hex
	ND var oconv_HEX(const int ioratio) const;
	ND var oconv_TX(const int raw = 0) const;

	// faster primitive arguments
	ND var iconv_D(const char* conversion) const;
	// var iconv_MT(const char* conversion) const;
	ND var iconv_MT() const;
	ND var iconv_MD(const char* conversion) const;
	ND var iconv_HEX(const int ioratio) const;
	ND var iconv_TX(const int raw = 0) const;

	//ND const std::string to_path_string() const;

	//VARREF localeAwareChangeCase(const int lowerupper);

	ND std::fstream* osopenx(CVR osfilename, const char* locale) const;

	//bool THIS_IS_DBCONN() const { return ((var_typ & VARTYP_DBCONN) != VARTYP_UNA); }
	bool THIS_IS_OSFILE() const { return ((var_typ & VARTYP_OSFILE) != VARTYP_UNA); }
	// bool THIS_IS_DBCONN() const    { return var_typ & VARTYP_DBCONN; }
	// bool THIS_IS_OSFILE() const    { return var_typ & VARTYP_OSFILE; }

};    // class "var"

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

//class var_iter
class PUBLIC var_iter {

	const var* pvar_;
	mutable std::string::size_type startpos_ = 0;
	mutable std::string::size_type endpos_ = std::string::npos;

   public:
	//default constructor
	var_iter() = default;

	//construct from var
	var_iter(CVR v);

	//check iter != iter (i.e. iter != string::npos)
	bool operator!=(var_iter& vi);

	//convert to var
	var operator*() const;

	//iter++
	var_iter operator++();

}; // class var_iter

//class var_proxy1 - replace or extract fn
class PUBLIC var_proxy1 {

   private:

	var* var_;
	mutable int fn_;

	var_proxy1() = delete;

   public:

	//constructor
	var_proxy1(var* var1, int fn) : var_(var1), fn_(fn) {};

	//implicit conversion to var if on the right hand side
	operator var() const {
		return var_->f(fn_);
	}

	//operator assign = old pick replace but with round instead of angle brackets
	void operator=(CVR replacement) {
		var_->r(fn_, replacement);
	}

	//operator bool
	explicit operator bool() const {
		return var_->f(fn_);
	}

}; // class var_proxy1

//class var_proxy2 - replace or extract fn, sn
class PUBLIC var_proxy2 {

   private:

	var* var_;
	mutable int fn_;
	mutable int vn_;

	var_proxy2() = delete;

   public:

	//constructor
	var_proxy2(var* var1, int fn, int vn) : var_(var1), fn_(fn), vn_(vn) {};

	//implicit conversion to var if on the right hand side
	operator var() const {
		return var_->f(fn_, vn_);
	}

	//operator assign = old pick replace but with round instead of angle brackets
	void operator=(CVR replacement) {
		var_->r(fn_, vn_, replacement);
	}

	//operator bool
	explicit operator bool() const {
		return var_->f(fn_, vn_);
	}

}; // class var_proxy2

//class var_proxy3 - replace or extract fn, vn, sn
class PUBLIC var_proxy3 {

   private:

	var* var_;
	mutable int fn_;
	mutable int vn_;
	mutable int sn_;

	var_proxy3() = delete;

   public:

	//constructor
	var_proxy3(var* var1, int fn, int vn = 0, int sn = 0) : var_(var1), fn_(fn), vn_(vn), sn_(sn) {};

	//implicit conversion to var if on the right hand side
	operator var() const {
		return var_->f(fn_, vn_, sn_);
	}

	//operator assign = old pick replace but with round instead of angle brackets
	void operator=(CVR replacement) {
		var_->r(fn_, vn_, sn_, replacement);
	}

	//operator bool
	explicit operator bool() const {
		return var_->f(fn_, vn_, sn_);
	}

}; // class var_proxy3

// var versions of separator characters. Must be after class declaration
inline const var FM = FM_;
inline const var VM = VM_;
inline const var SM = SM_;
inline const var SVM = SVM_;
inline const var TM = TM_;
inline const var STM = STM_;

inline const var IM = IM_;
inline const var RM = RM_;
inline const var AM = AM_;

inline const var BS = BS_;
inline const var DQ = DQ_;
inline const var SQ = SQ_;

#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__
inline const var OSSLASH = "\\";
constexpr char OSSLASH_ = '\\';
constexpr bool SLASH_IS_BACKSLASH = true;
#else
inline const var OSSLASH = "/";
constexpr char OSSLASH_ = '/';
constexpr bool SLASH_IS_BACKSLASH = false;
#endif

// ASCII definition. Not used in ucase()/lcase() which handle unicode.
inline const var LOWERCASE_ = "abcdefghijklmnopqrstuvwxyz";
inline const var UPPERCASE_ = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

#if defined(_WIN64) or defined(_LP64)
inline const var PLATFORM_ = "x64";
#else
inline const var PLATFORM_ = "x86";
#endif

// A global flag used in mvdbpostgres
[[maybe_unused]] static inline int DBTRACE=var().osgetenv("EXO_DBTRACE");

// This is left a global copy for backtrace to get at it
PUBLIC inline exodus::var EXECPATH2 = "";

#ifndef SWIG
PUBLIC ND std::string naturalorder(const std::string& string1);
PUBLIC ND const std::string to_oscmd_string(CVR cmd);

#endif

PUBLIC ND int getenvironmentn();
PUBLIC void setenvironmentn(const int environmentn);

PUBLIC ND int getenvironmentn();
PUBLIC ND var getprocessn();
PUBLIC ND var getexecpath();

// MV Exceptions

// MVStop is similar to MVError
// but doesnt get stack since stop() is called normally
class PUBLIC MVStop {
   public:
	explicit MVStop(CVR var1 DEFAULT_EMPTY);
	var description;
};

// MVAbort is similar to MVError
// but doesnt get stack since abort() is called normally
class PUBLIC MVAbort {
   public:
	explicit MVAbort(CVR var1 DEFAULT_EMPTY);
	var description;
};

// MVAbortAll is similar to MVError
// but doesnt get stack since abortall() is called normally
class PUBLIC MVAbortAll {
   public:
	explicit MVAbortAll(CVR var1 DEFAULT_EMPTY);
	var description;
};

// MVLogoff is similar to MVError
// but doesnt get stack since abortall() is called normally
class PUBLIC MVLogoff {
   public:
	explicit MVLogoff(CVR var1 DEFAULT_EMPTY);
	var description;
};

// provide a public base exception for all other exceptions so exodus programmers can catch mv
// exceptions generally
class PUBLIC MVError {
   public:
	explicit MVError(CVR description);
	var description;
};

//inline avoids hitting ODR rule
ND inline var operator""_var(const char* cstr, std::size_t size) {
	return var(cstr, size).convert(VISIBLE_FMS, _RM_ _FM_ _VM_ _SM_ _TM_ _STM_);
}

ND inline var operator""_var(unsigned long long int i) {
	return var(int64_t(i));
}

ND inline var operator""_var(long double d) {
	return var(static_cast<double>(d));
}

//mv_iter class enables c++ range based programming over a dynamic array uses FM as field separator at the moment Example: for (CVR v : dynstr) 
//{...} Warning: updating the string probably invalidates the iterator! It is a byte pointer into the string. TODO add option a) to specify field 
//separator eg VM, SM and/or b) acquire field number as well as provided by SRP precompiler 
//https://wiki.srpcs.com/display/SRPUtilities/SRP_PreCompiler#SRP_PreCompiler-ForEachLoops For Each Value in MyValues using @STM setting Pos 
//NewValues<Pos> = Value Next Value / var_iter / CONSTRUCTOR from a var (ie begin()) var_iter::var_iter(CVR var1) pvar_(&var1){ if 
//(!var1.length()) startpos_ = std::string::npos; std::cerr << __PRETTY_FUNCTION__ << std::endl; check iter != iter (i.e. iter != end() bool 
//var_iter::operator!=([[maybe_unused]] var_iter& var_iter1) { std::cerr << __PRETTY_FUNCTION__ << std::endl; no need to use var_iter1 since the 
//end is always string::npos; return this->startpos_ != std::string::npos; CONVERSION - conversion to var var var_iter::operator*() const { 
//std::cerr << __PRETTY_FUNCTION__ << std::endl; find the end of the field if not already known if (endpos_ == std::string::npos) { endpos_ = 
//pvar_->var_str.find(FM_, startpos_); extract the field if (endpos_ == std::string::npos) return pvar_->var_str.substr(startpos_); else return 
//pvar_->var_str.substr(startpos_, endpos_ - startpos_); INCREMENT var_iter var_iter::operator++() { std::cerr << __PRETTY_FUNCTION__ << 
//std::endl; find the end of the field if not already found from a call to above CONVERSION if (endpos_ == std::string::npos) endpos_ = 
//pvar_->var_str.find(FM_, startpos_); move up to the next field startpos_ = endpos_; skip over any FM character if (startpos_ != 
//std::string::npos) { max str size = 9223372036854775807 string npos = 18446744073709551615 we will ignore the fact that we could be processing a 
//string of maximum size with a terminating FM and incrementing would take us past maximum string size but not beyond "no position" npos. 
//startpos_++; indicate that the end of the next field is not yet known endpos_ = std::string::npos; return *this; BEGIN - free function to create 
//an iterator -> begin PUBLIC var_iter begin(CVR var1) { std::cerr << __PRETTY_FUNCTION__ << std::endl; return var_iter(var1); END - free function 
//to create an interator -> end PUBLIC var_iter end([[maybe_unused]] CVR var1) { std::cerr << __PRETTY_FUNCTION__ << std::endl; No need to use 
//var1 since the end is always string::npos so var_iter!=var_iter is implemented in terms of startpos_ != string::npos; return var_iter();

}  // namespace exodus

#endif    // MV_H
