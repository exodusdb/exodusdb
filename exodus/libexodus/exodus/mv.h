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

#define VARREF var&
#define CVR const var&
#define TVR var&&

#define EXODUS_RELEASE "21.03"
#define EXODUS_PATCH "21.03.0"

// http://stackoverflow.com/questions/538134/exporting-functions-from-a-dll-with-dllexport
// Using dllimport and dllexport in C++ Classes
// http://msdn.microsoft.com/en-us/library/81h27t8c(VS.80).aspx

#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__
#ifdef BUILDING_LIBRARY
#ifdef __GNUC__
#define PUBLIC __attribute__((dllexport))
#else
#define PUBLIC __declspec(dllexport)  // Note: actually gcc seems to also support this syntax.
#endif
#else
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
#define PUBLIC __attribute__((visibility("default")))
#define DLL_LOCAL __attribute__((visibility("hidden")))
#else
#define PUBLIC
#define DLL_LOCAL
#endif
#endif

#include <string>

// ND macro [[no_discard]]
#if __clang_major__ != 10
#define ND [[nodiscard]]
#else
#define ND
#endif

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

#define DQ_ '\"'
#define SQ_ '\''

// Would be 256 if RM was character number 255.
// Last delimiter character is 0x1F (RM)
// Used in var::remove()
#define LASTDELIMITERCHARNOPLUS1 0x20

//#define VISIBLE_FMS "_]\[Z"  //PickOS standard
//#define VISIBLE_FMS "<[{}]>" //logical but hard to read direction of brackets quickly
//#define VISIBLE_FMS "_^]}`~"   //all uncommon in data. first 3 (_ ^ ]) are identical to pickos
#define VISIBLE_FMS "_^]}|~"  //all uncommon in data. _ ^ ] are identical to pickos

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

// most help from Thinking in C++ Volume 1 Chapter 12
// http://www.camtp.uni-mb.si/books/Thinking-in-C++/TIC2Vone-distribution/html/Chapter12.html

// could also use http://www.informit.com/articles/article.asp?p=25264&seqNum=1
// which is effectively about makeing objects behave like ordinary variable syntactically
// implementing smartpointers

// explanation 1.
// we need a thread specific environment number so that functions written in C++
// and used in database calculations (via a callback to a different thread).
// Database calculation functions written in c++ should use this environment number
// as an index into arrays or vectors of global data and not use simple thread specific storage
// explanation 2.
// the SELECT statement often needs to sort/select on calculated fields and we provide a way for the
// database to call procedures in the main program using pipes and a separate thread to serve the
// pipe. to enhance programmability the procedures can have the environment of the thread that
// issued the SELECT statement so we have two threads essentially using the same environment but
// because one is waiting for the database to complete the select statement, the other thread can
// have free access to the same environment each main thread gets a supporting thread the first time
// it does a SELECT explanation 3.

/*TODO check if the following guidance is followed in class var
Conversion never applies to this for member functions, and this extends to operators. If the
operator was instead example operator+ (example, example) in the global namespace, it would compile
(or if pass-by-const-ref was used).

As a result, symmetric operators like + and - are generally implemented as non-members, whereas the
compound assignment operators like += and -= are implemented as members (they also change data,
meaning they should be members). And, since you want to avoid code duplication, the symmetric
operators can be implemented in terms of the compound assignment ones (as in your code example,
although convention recommends making the temporary inside the function).
*/

// the destructor is public non-virtual (supposedly to save space)
// since this class has no virtual functions
// IT SHOULD BE ABLE TO DERIVE FROM IT AND DO DELETE()
// http://www.parashift.com/c++-faq-lite/virtual-functions.html#faq-20.7

// on linux, size is 56 bytes
// string:    32
// int:        8
// double:    8
// char:        4
// var:        56

class var_brackets_proxy;
class var_proxy1;
class var_proxy2;
class var_proxy3;

class VARTYP {
   public:
	// constructors from uint and it
	// VARTYP(uint rhs) {flags=rhs;std::cout<<"uint32 ctor "<< rhs << std::endl;};
	// VARTYP(int rhs) {flags=rhs;std::cout<<"int ctor "<< rhs << std::endl;if (rhs==9999) throw
	// "kjhkjhkkjh";};
	VARTYP(uint rhs)
		: flags_(rhs){};

	// copy constructor
	// VARTYP(const VARTYP& rhs) {flags=rhs.flags;std::cout<<"copy ctor "<< rhs.flags <<
	// std::endl;};
	// will be defaulted so no need to define
	//(if defined then also need to define other constructors)
	//VARTYP(const VARTYP& rhs) : flags_(rhs.flags_){};

	// default constructor
	// VARTYP() : flags(0) {};
	// VARTYP() {flags=0;std::cout<<"def ctor1"<<std::endl;};
	// VARTYP() : flags{} {std::cout<<"def ctor2"<<std::endl;};
	// VARTYP() flags(0) {};

	// assign
	VARTYP& operator=(const uint newflags) {
		flags_ = newflags;
		return *this;
	};

	// bitwise mutators
	VARTYP& operator^=(const uint rhs) {
		flags_ ^= rhs;
		return *this;
	};
	VARTYP& operator|=(const uint rhs) {
		flags_ |= rhs;
		return *this;
	};
	VARTYP& operator&=(const uint rhs) {
		flags_ &= rhs;
		return *this;
	};

	// logical comparison
	ND bool operator==(const uint rhs) const { return flags_ == rhs; };
	ND bool operator!=(const uint rhs) const { return flags_ != rhs; };
	ND bool operator==(const VARTYP rhs) const { return flags_ == rhs.flags_; };
	ND bool operator!=(const VARTYP rhs) const { return flags_ != rhs.flags_; };

	// bitwise accessors
	VARTYP operator&(const uint rhs) const { return uint(flags_ & rhs); };
	VARTYP operator|(const uint rhs) const { return uint(flags_ | rhs); };
	VARTYP operator~() const { return VARTYP(~flags_); };

	// boolean - not explicit so we can do "if (var_type==something)"
	operator bool() const { return flags_ != 0; };

	// private:
	// initialisation
	// mutable
	//TODO check if initialisation required here or can speed up by removing and assuming set everywhere required
	uint flags_{0};

}; // class VARTYP

// WARNING these VARTYP constants must be initialised before any var variables are
// NOT they are declared inline which presumably makes them the same in all compilation units
// but it also seems to ensure that they are initialised BEFORE any var variables
// theoretically they should be because C++ says
// this is mandatory because initialising var variables REQUIRES these constants to be available
// with their correct values otherwise any such var, when used later on, will throw Unassigned
// Variable Used since its var_typ will be zero

// throw an exception if used an unassigned variable
inline const uint VARTYP_UNA {0x0};

// assigned string - unknown if numeric or not
inline const uint VARTYP_STR {0x1};

// following indicate that the var is numeric
inline const uint VARTYP_INT {0x2};
inline const uint VARTYP_DBL {0x4};

// indicates known non-numeric string
inline const uint VARTYP_NAN {0x8};

// following indicates that the int is an os file handle
inline const uint VARTYP_OSFILE {0x10};
//inline const uint VARTYP_DBCONN {0x20};

// various useful flag combinations
inline const uint VARTYP_INTDBL {VARTYP_INT | VARTYP_DBL};
inline const uint VARTYP_INTSTR {VARTYP_INT | VARTYP_STR};
inline const uint VARTYP_DBLSTR {VARTYP_DBL | VARTYP_STR};
inline const uint VARTYP_NANSTR {VARTYP_NAN | VARTYP_STR};
inline const uint VARTYP_NOTNUMFLAGS {~(VARTYP_INT | VARTYP_DBL | VARTYP_NAN)};

inline const uint VARTYP_NANSTR_OSFILE {VARTYP_NANSTR | VARTYP_OSFILE};
//inline const uint VARTYP_NANSTR_DBCONN {VARTYP_NANSTR | VARTYP_DBCONN};

//inline const uint VARTYP_DESTRUCTED {0xFFFFF0};

inline const uint VARTYP_MASK {~(VARTYP_STR | VARTYP_NAN | VARTYP_INT | VARTYP_DBL | VARTYP_OSFILE | VARTYP_OSFILE)};

//prevent or allow assignment to var to return a reference to the var
//preventing will stop accidental usage of = instead of == in if() clauses
//if you really want to assign in an if clause then use something like  if (x=123;x)
#define PREVENT_ASSIGN_IN_IF
#ifdef PREVENT_ASSIGN_IN_IF
#define VOID_OR_VARREF void
#define VOID_OR_THIS
#else
#define VOID_OR_VARREF "VARREF"
#define VOID_OR_THIS "*this"
#endif

// class var
//"final" to prevent inheritance because var has a destructor which is non-virtual to save space and time
class PUBLIC var final {

	///////////////////////
	// PRIVATE DATA MEMBERS
	///////////////////////

   private:
	// 1. not using pimpl idiom in order to maximise performance
	// 2. all mutable because asking for a string can create it from an integer and vice versa
	mutable std::string var_str; //32 bytes on g++
	mutable long long   var_int; //8 bytes/64 bits - currently defined as a long long
	mutable double      var_dbl; //8 bytes/64 buts - double
	mutable VARTYP      var_typ; //actually a uint which will be 4 bytes
	                             //mutable uint padding1;
	                             //mutable long int padding2;

	///////////////
	// CONSTRUCTORS
	///////////////

   public:
	// default constructor
	// allow syntax "var v;" to create an "unassigned" var (var_typ is 0)
	var();

	// copy constructor
	var(CVR fromvar);  // = default;

	// move constructor
	var(TVR fromvar) noexcept;  // = default;

	// copy assignment constructor
	VOID_OR_VARREF operator=(CVR fromvar) &;     // = default;
	//VARREF operator=(const var &) && = delete; //disable assign to temporaries

	// move assignment
	VOID_OR_VARREF operator=(TVR fromvar) & noexcept;     // = default;
	//VARREF operator=(TVR fromvar) && noexcept = delete;     // disable move to temporaries

	// destructor - sets var_typ undefined
	//WARNING: non-virtual destructor - so cannot create derived classes
	~var();

	////////////////////
	// CONSTRUCTORS FROM
	////////////////////

//#define ALL_IN_ONE_STRING_CONSTRUCTOR
#ifdef ALL_IN_ONE_STRING_CONSTRUCTOR
	//accepts l and r values
	template <typename S, typename = std::enable_if_t<std::is_convertible_v<S, std::string>>>
	//enable_if can be replaced by a concept when available in g++ compiler (gcc v11?)
	var(S&& fromstr)
	    : var_str(std::forward<S>(fromstr)), var_typ(VARTYP_STR){};

#else

	// from std::string
	//
	// swig java duplicates this with var(std::string&) above
#if !defined(SWIGJAVA) && !defined(SWIGCSHARP)
	var(const std::string& str1);
	var(std::string&& str1) noexcept;
	//in c++20 but not g++ v9.3
	//constexpr var(const std::string& str1);
#endif

#endif

#ifndef SWIGJAVA
	// constructor for char*
	// place first before char so SWIG isnt tempted to use char to acquire strings resulting in
	// ONE character strings)
	//	MV_CONSTRUCTION_FROM_CHAR_EXPLICIT
	var(const char*);
#endif

	// from char
#ifndef SWIG
	//    MV_CONSTRUCTION_FROM_CHAR_EXPLICIT
	var(const char) noexcept;
#endif

	// from char memory block
	//
	//    MV_CONSTRUCTION_FROM_CHAR_EXPLICIT
	var(const char* cstr, const size_t nchars);

	// from bool
	//
	var(const bool bool1) noexcept;

	// from int
	//
	var(const int) noexcept;

	// from long long
	//
	// suppressing for now since ambiguous with int
	//#if 0
	var(const long long) noexcept;
	//#endif

	// from double
	//
	var(const double) noexcept;

	///////////////////////
	// NAMED CONVERSIONS TO
	///////////////////////

	bool toBool() const;

	char toChar() const;

	// standard c/c++ int() in other words simply take the number to the left of the point. -1.5
	// becomes -1 and 1.5 becomes 1
	int toInt() const;
	long long toLong() const;

	double toDouble() const;

	std::u32string to_u32string() const;
	void from_u32string(std::u32string) const;

	std::string toString() &&;               //for temporary vars
	const std::string& toString() const&;  //for non-temporaries

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
	operator double&() const;
#else
	operator int() const;
	operator double() const;
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
	operator void*() const;

	// this stops using var as indexes of arrays on msvc since msvc converts bool and int
	// ambiguous to index number g++ seems happy to make int higher and unambigous with bool but
	// probably non-standard c++ msvc (at least) cant do without this since it seems unwilling
	// to convert void* as a bool therefore we include it and restrict indexing on ints and not
	// var eg for (int ii=0 .... instead of for (var ii=0 ... #ifndef _MSVC explicit new in
	// c++11 still allows implicit conversion in places where bool is definitely required ie in
	// if (xx) or aa && bb but we do not use is because allowing implicit conversion to bool in
	// argument lists is convenient explicit
	//explicit operator bool() const;
	operator bool() const;
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
	operator std::string() const;

	// allow conversion to char (takes first char or char 0 if zero length string)
	// would allow the usage of any char function but may result
	// in a lot of compilation failures due to "ambiguous overload"
	// unfortunately there is no "explicit" keyword as for constructors - coming in C++0X
	// operator char() const;

	// BRACKETS ()
	/////////////

	// extract using () int int int (alternative to .a() and extract())
	// instead of xyz=abc.extract(1,2,3);
	// sadly there is no way to use pick/mv angle brackets like "abc<1,2,3>"
	// and [] brackets would only allow one dimension eg abc[1]

	// TODO implement abc(1,2,3)="xyz"; or abc.a(1,2,3)="xyz";
	// An idea is that var::operator() will produce a "specialvar" that will
	// perform extract if a var is wanted or a replace if assignment is done
	// but it hasnt been established if it is possible to make the specialvar behave like a
	// normal var in all situation eg would the following work? var1(1,2,3).outputl(); also the
	// creation and deletion of the temp object may hit performance. One alternative would be to
	// use () only for extraction (and [] only for replacement of fields?)

	// dynamic array extract/replace eg: abc=xyz(1,2,3) and xyz(1,2,3)="abc";
	// var__extractreplace operator() (int fieldno, int valueno=0, int subvalueno=0) const;
	// var__extractreplace operator() (int fieldno, int valueno=0, int subvalueno=0);

	//the following produces a temporary on the rhs
//#define VAR_FUNCTOR_EXTRACTS
//#ifdef VAR_FUNCTOR_ONLY_EXTRACTS
	ND var operator()(int fieldno, int valueno = 0, int subvalueno = 0) const;
//#else
	ND var_proxy1 operator()(int fieldno);
	ND var_proxy2 operator()(int fieldno, int valueno);
	ND var_proxy3 operator()(int fieldno, int valueno, int subvalueno);
//endif

	// DONT declare this so we force use of the above const version that produces a temporary
	//VARREF operator()(int fieldno, int valueno = 0, int subvalueno = 0);
	/* SADLY no way to get a different operator() function called when on the left hand side of assign
	therefore not possible to create syntax like "xyz(1,2)="xx";" to REPLACE fields, values and
	subvalues so operator() is defined only for the more common xyz=extract(1,2,3); i.e. on the right
	hand side. http://codepad.org/MzzhlRkb
	*/

	// BRACKETS []
	//////////////

	// extract a character first=1 last=-1 etc
	ND var operator[](const int charno) const;

	/* sadly this all works EXCEPT that var[99].anymethod doesnt work
	so would have to implement all var methods on the proxy object
	ND var_brackets_proxy operator[](int charno) const&;
	ND var operator[](int charno) &&;
	*/

	///////////////////
	// ASSIGN OPERATORS
	///////////////////

	//=int|double|char|char*|std::string&|std::string&& - only allow lvalue
	VOID_OR_VARREF operator=(const int) &;
	VOID_OR_VARREF operator=(const double) &;
	VOID_OR_VARREF operator=(const char) &;
	VOID_OR_VARREF operator=(const char*) &;
	VOID_OR_VARREF operator=(const std::string&) &;
	VOID_OR_VARREF operator=(const std::string&&) &;

	// ^=var|int|double|char|char*|std::string& - only allow lvalue
	VARREF operator^=(CVR) &;
	VARREF operator^=(const int) &;
	VARREF operator^=(const double) &;
	VARREF operator^=(const char) &;
	VARREF operator^=(const char*) &;
	VARREF operator^=(const std::string&) &;

	// increment/decrement as postfix - only allow lvalue
	var operator++(int) &;
	var operator--(int) &;

	// increment/decrement as prefix - only allow lvalue
	VARREF operator++() &;
	VARREF operator--() &;

	//+=var|int|double - only allow lvalue
	VARREF operator+=(CVR) &;
	VARREF operator+=(int) &;
	VARREF operator+=(double) &;

	//-=var|int|double - only allow lvalue
	VARREF operator-=(CVR) &;
	VARREF operator-=(int) &;
	VARREF operator-=(double) &;

	/////////////
	// BINARY OPS
	/////////////

	//VARREF operator^(CVR);
	//VARREF operator^(const char*);
	//VARREF operator^(const std::string&);

	//////////
	// FRIENDS
	//////////

	PUBLIC ND friend var MVplus(CVR);
	PUBLIC ND friend var MVminus(CVR);
	PUBLIC ND friend bool MVnot(CVR);

	PUBLIC ND friend var MVadd(CVR lhs, CVR rhs);
	PUBLIC ND friend var MVsub(CVR lhs, CVR rhs);
	PUBLIC ND friend var MVmul(CVR lhs, CVR rhs);
	PUBLIC ND friend var MVdiv(CVR lhs, CVR rhs);
	PUBLIC ND friend var MVmod(CVR lhs, CVR rhs);

	PUBLIC ND friend var MVcat(CVR lhs, CVR rhs);
	PUBLIC ND friend var MVcat(CVR lhs, const char* cstr);
	PUBLIC ND friend var MVcat(CVR lhs, const char char2);
	PUBLIC ND friend var MVcat(const char* cstr, CVR rhs);

	PUBLIC ND friend bool MVeq(CVR lhs, CVR rhs);
	PUBLIC ND friend bool MVlt(CVR lhs, CVR rhs);
	// following specialisations done for speed of var in for loops
	PUBLIC ND friend bool MVlt(CVR lhs, const int);
	PUBLIC ND friend bool MVlt(const int int1, CVR rhs);

	//== and !=
	//TODO consider replacing by operator<=> and replacing MVeq and MVlt by MVcmp
	//or provide more specialisations of MVeq and MVlt

	PUBLIC friend bool operator==(CVR lhs, CVR rhs) { return MVeq(lhs, rhs); }
	PUBLIC friend bool operator==(CVR lhs, const char* cstr2) { return MVeq(lhs, var(cstr2)); }
	PUBLIC friend bool operator==(CVR lhs, const char char2) { return MVeq(lhs, var(char2)); }
	PUBLIC friend bool operator==(CVR lhs, const int int2) { return MVeq(lhs, var(int2)); }
	PUBLIC friend bool operator==(CVR lhs, const double double2) { return MVeq(lhs, var(double2)); }
	PUBLIC friend bool operator==(CVR lhs, const bool bool2) { return MVeq(lhs, var(bool2)); }
	PUBLIC friend bool operator==(const char* cstr1, CVR rhs) { return MVeq(rhs, var(cstr1)); }
	PUBLIC friend bool operator==(const char char1, CVR rhs) { return MVeq(rhs, var(char1)); }
	PUBLIC friend bool operator==(const int int1, CVR rhs) { return MVeq(rhs, var(int1)); }
	PUBLIC friend bool operator==(const double double1, CVR rhs) { return MVeq(rhs, var(double1)); }
	PUBLIC friend bool operator==(const bool bool1, CVR rhs) { return MVeq(rhs, var(bool1)); }

	PUBLIC friend bool operator!=(CVR lhs, CVR rhs) { return !MVeq(lhs, rhs); }
	PUBLIC friend bool operator!=(CVR lhs, const char* cstr2) { return !MVeq(lhs, var(cstr2)); }
	PUBLIC friend bool operator!=(CVR lhs, const char char2) { return !MVeq(lhs, var(char2)); }
	PUBLIC friend bool operator!=(CVR lhs, const int int2) { return !MVeq(lhs, var(int2)); }
	PUBLIC friend bool operator!=(CVR lhs, const double double2) { return !MVeq(lhs, var(double2)); }
	PUBLIC friend bool operator!=(CVR lhs, const bool bool2) { return !MVeq(lhs, var(bool2)); }
	PUBLIC friend bool operator!=(const char* cstr1, CVR rhs) { return !MVeq(rhs, var(cstr1)); }
	PUBLIC friend bool operator!=(const char char1, CVR rhs) { return !MVeq(rhs, var(char1)); }
	PUBLIC friend bool operator!=(const int int1, CVR rhs) { return !MVeq(rhs, var(int1)); }
	PUBLIC friend bool operator!=(const double double1, CVR rhs) { return !MVeq(rhs, var(double1)); }
	PUBLIC friend bool operator!=(const bool bool1, CVR rhs) { return !MVeq(rhs, var(bool1)); }

	//< V<= > >=
	PUBLIC friend bool operator<(CVR lhs, CVR rhs) { return MVlt(lhs, rhs); }
	PUBLIC friend bool operator<(CVR lhs, const char* cstr2) { return MVlt(lhs, var(cstr2)); }
	PUBLIC friend bool operator<(CVR lhs, const char char2) { return MVlt(lhs, var(char2)); }
	PUBLIC friend bool operator<(CVR lhs, const int int2) { return MVlt(lhs, int2); }
	PUBLIC friend bool operator<(CVR lhs, const double double2) { return MVlt(lhs, var(double2)); }
	PUBLIC friend bool operator<(const char* cstr1, CVR rhs) { return MVlt(var(cstr1), rhs); }
	PUBLIC friend bool operator<(const char char1, CVR rhs) { return MVlt(var(char1), rhs); }
	PUBLIC friend bool operator<(const int int1, CVR rhs) { return MVlt(int1, rhs); }
	PUBLIC friend bool operator<(const double double1, CVR rhs) { return MVlt(var(double1), rhs); }
	PUBLIC friend bool operator<(const bool bool1, CVR rhs) { return MVlt(var(bool1), rhs); }

	PUBLIC friend bool operator>=(CVR lhs, CVR rhs) { return !MVlt(lhs, rhs); }
	PUBLIC friend bool operator>=(CVR lhs, const char* cstr2) { return !MVlt(lhs, var(cstr2)); }
	PUBLIC friend bool operator>=(CVR lhs, const char char2) { return !MVlt(lhs, var(char2)); }
	PUBLIC friend bool operator>=(CVR lhs, const int int2) { return !MVlt(lhs, int2); }
	PUBLIC friend bool operator>=(CVR lhs, const double double2) { return !MVlt(lhs, var(double2)); }
	PUBLIC friend bool operator>=(const char* cstr1, CVR rhs) { return !MVlt(var(cstr1), rhs); }
	PUBLIC friend bool operator>=(const char char1, CVR rhs) { return !MVlt(var(char1), rhs); }
	PUBLIC friend bool operator>=(const int int1, CVR rhs) { return !MVlt(int1, rhs); }
	PUBLIC friend bool operator>=(const double double1, CVR rhs) { return !MVlt(var(double1), rhs); }
	PUBLIC friend bool operator>=(const bool bool1, CVR rhs) { return !MVlt(var(bool1), rhs); }

	PUBLIC friend bool operator>(CVR lhs, CVR rhs) { return MVlt(rhs, lhs); }
	PUBLIC friend bool operator>(CVR lhs, const char* cstr2) { return MVlt(var(cstr2), lhs); }
	PUBLIC friend bool operator>(CVR lhs, const char char2) { return MVlt(var(char2), lhs); }
	PUBLIC friend bool operator>(CVR lhs, const int int2) { return MVlt(int2, lhs); }
	PUBLIC friend bool operator>(CVR lhs, const double double2) { return MVlt(var(double2), lhs); }
	PUBLIC friend bool operator>(const char* cstr1, CVR rhs) { return MVlt(rhs, var(cstr1)); }
	PUBLIC friend bool operator>(const char char1, CVR rhs) { return MVlt(rhs, var(char1)); }
	PUBLIC friend bool operator>(const int int1, CVR rhs) { return MVlt(rhs, int1); }
	PUBLIC friend bool operator>(const double double1, CVR rhs) { return MVlt(rhs, var(double1)); }
	PUBLIC friend bool operator>(const bool bool1, CVR rhs) { return MVlt(rhs, var(bool1)); }

	PUBLIC friend bool operator<=(CVR lhs, CVR rhs) { return !MVlt(rhs, lhs); }
	PUBLIC friend bool operator<=(CVR lhs, const char* cstr2) { return !MVlt(var(cstr2), lhs); }
	PUBLIC friend bool operator<=(CVR lhs, const char char2) { return !MVlt(var(char2), lhs); }
	PUBLIC friend bool operator<=(CVR lhs, const int int2) { return !MVlt(int2, lhs); }
	PUBLIC friend bool operator<=(CVR lhs, const double double2) { return !MVlt(var(double2), lhs); }
	PUBLIC friend bool operator<=(const char* cstr1, CVR rhs) { return !MVlt(rhs, var(cstr1)); }
	PUBLIC friend bool operator<=(const char char1, CVR rhs) { return !MVlt(rhs, var(char1)); }
	PUBLIC friend bool operator<=(const int int1, CVR rhs) { return !MVlt(rhs, int1); }
	PUBLIC friend bool operator<=(const double double1, CVR rhs) { return !MVlt(rhs, var(double1)); }
	PUBLIC friend bool operator<=(const bool bool1, CVR rhs) { return !MVlt(rhs, var(bool1)); }

	//PLUS
	PUBLIC friend var operator+(CVR lhs, CVR rhs) { return MVadd(lhs, rhs); }
	PUBLIC friend var operator+(CVR lhs, const char* cstr2) { return MVadd(lhs, var(cstr2)); }
	PUBLIC friend var operator+(CVR lhs, const char char2) { return MVadd(lhs, var(char2)); }
	PUBLIC friend var operator+(CVR lhs, const int int2) { return MVadd(lhs, var(int2)); }
	PUBLIC friend var operator+(CVR lhs, const double double2) { return MVadd(lhs, var(double2)); }
	//PUBLIC friend var operator+(CVR lhs, const bool bool2) { return MVadd(lhs, var(bool2)); }
	PUBLIC friend var operator+(const char* cstr1, CVR rhs) { return MVadd(var(cstr1), rhs); }
	PUBLIC friend var operator+(const char char1, CVR rhs) { return MVadd(var(char1), rhs); }
	PUBLIC friend var operator+(const int int1, CVR rhs) { return MVadd(var(int1), rhs); }
	PUBLIC friend var operator+(const double double1, CVR rhs) { return MVadd(var(double1), rhs); }
	//PUBLIC friend var operator+(const bool bool1, CVR rhs) { return MVadd(var(bool1), rhs); }
	//rvalues
	//PUBLIC friend var operator+(TVR lhs, CVR rhs) { return lhs+=rhs; }
	//PUBLIC friend var operator+(TVR lhs, const char* cstr2) { return lhs+=char2; }
	//PUBLIC friend var operator+(TVR lhs, const int int2) { return lhs+=int2; }
	//PUBLIC friend var operator+(TVR lhs, const double double2) { return lhs+=double2; }
	//PUBLIC friend var operator+(TVR lhs, const bool bool2) { return lhs+=bool2; }
	//warning: ISO C++ says that these are ambiguous, even though the worst conversion for the first is better than the worst conversion for the second:
	//the following 3 (not 4) are excluded to avoid the above wrning from gcc 7
	//PUBLIC friend var operator+(const char* cstr1, TVR rhs) { return rhs+=char1; }
	//PUBLIC friend var operator+(const int int1, TVR rhs) { return rhs+=int1; }
	//PUBLIC friend var operator+(const double double1, TVR rhs) { return rhs+=double1; }
	//PUBLIC friend var operator+(const bool bool1, TVR rhs) { return rhs+=bool1; }

	//MINUS
	PUBLIC friend var operator-(CVR lhs, CVR rhs) { return MVsub(lhs, rhs); }
	PUBLIC friend var operator-(CVR lhs, const char* cstr2) { return MVsub(lhs, var(cstr2)); }
	PUBLIC friend var operator-(CVR lhs, const char char2) { return MVsub(lhs, var(char2)); }
	PUBLIC friend var operator-(CVR lhs, const int int2) { return MVsub(lhs, var(int2)); }
	PUBLIC friend var operator-(CVR lhs, const double double2) { return MVsub(lhs, var(double2)); }
	//PUBLIC friend var operator-(CVR lhs, const bool bool2) { return MVsub(lhs, var(bool2)); }
	PUBLIC friend var operator-(const char* cstr1, CVR rhs) { return MVsub(var(cstr1), rhs); }
	PUBLIC friend var operator-(const char char1, CVR rhs) { return MVsub(var(char1), rhs); }
	PUBLIC friend var operator-(const int int1, CVR rhs) { return MVsub(var(int1), rhs); }
	PUBLIC friend var operator-(const double double1, CVR rhs) { return MVsub(var(double1), rhs); }
	//PUBLIC friend var operator-(const bool bool1, CVR rhs) { return MVsub(var(bool1), rhs); }
	//rvalues
	PUBLIC friend var operator-(TVR lhs, CVR rhs) { return lhs -= rhs; }
	PUBLIC friend var operator-(TVR lhs, const char* cstr2) { return lhs -= var(cstr2); }
	PUBLIC friend var operator-(TVR lhs, const char char2) { return lhs -= var(char2); }
	PUBLIC friend var operator-(TVR lhs, const int int2) { return lhs -= int2; }
	PUBLIC friend var operator-(TVR lhs, const double double2) { return lhs -= double2; }

	//MULTIPLY
	PUBLIC friend var operator*(CVR lhs, CVR rhs) { return MVmul(lhs, rhs); }
	PUBLIC friend var operator*(CVR lhs, const char* cstr2) { return MVmul(lhs, var(cstr2)); }
	PUBLIC friend var operator*(CVR lhs, const char char2) { return MVmul(lhs, var(char2)); }
	PUBLIC friend var operator*(CVR lhs, const int int2) { return MVmul(lhs, var(int2)); }
	PUBLIC friend var operator*(CVR lhs, const double double2) { return MVmul(lhs, var(double2)); }
	//PUBLIC friend var operator*(CVR lhs, const bool bool2) { return MVmul(lhs, var(bool2)); }
	PUBLIC friend var operator*(const char* cstr1, CVR rhs) { return MVmul(var(cstr1), rhs); }
	PUBLIC friend var operator*(const char char1, CVR rhs) { return MVmul(var(char1), rhs); }
	PUBLIC friend var operator*(const int int1, CVR rhs) { return MVmul(var(int1), rhs); }
	PUBLIC friend var operator*(const double double1, CVR rhs) { return MVmul(var(double1), rhs); }
	//PUBLIC friend var operator*(const bool bool1, CVR rhs) { return MVmul(var(bool1), rhs); }
	//rvalues - pending implementation of var*=
	//PUBLIC friend var operator*(TVR lhs, CVR rhs) { return lhs*=rhs; }
	//PUBLIC friend var operator*(TVR lhs, const char* cstr2) { return lhs*=var(cstr2); }
	//PUBLIC friend var operator*(TVR lhs, const char char2) { return lhs*=var(char2); }
	//PUBLIC friend var operator*(TVR lhs, const int int2) { return lhs*=int2; }
	//PUBLIC friend var operator*(TVR lhs, const double double2) { return lhs*=double2; }

	//DIVIDE
	PUBLIC friend var operator/(CVR lhs, CVR rhs) { return MVdiv(lhs, rhs); }
	PUBLIC friend var operator/(CVR lhs, const char* cstr2) { return MVdiv(lhs, var(cstr2)); }
	PUBLIC friend var operator/(CVR lhs, const char char2) { return MVdiv(lhs, var(char2)); }
	PUBLIC friend var operator/(CVR lhs, const int int2) { return MVdiv(lhs, var(int2)); }
	PUBLIC friend var operator/(CVR lhs, const double double2) { return MVdiv(lhs, var(double2)); }
	// disallow divide by boolean to prevent possible runtime divide by zero
	// PUBLIC friend var operator/ (CVR     lhs    ,const bool     bool2   ){return
	// MVdiv(lhs,var(bool2)  );}
	PUBLIC friend var operator/(const char* cstr1, CVR rhs) { return MVdiv(var(cstr1), rhs); }
	PUBLIC friend var operator/(const char char1, CVR rhs) { return MVdiv(var(char1), rhs); }
	PUBLIC friend var operator/(const int int1, CVR rhs) { return MVdiv(var(int1), rhs); }
	PUBLIC friend var operator/(const double double1, CVR rhs) { return MVdiv(var(double1), rhs); }
	//PUBLIC friend var operator/(const bool bool1, CVR rhs) { return MVdiv(var(bool1), rhs); }

	//MODULO
	PUBLIC friend var operator%(CVR lhs, CVR rhs) { return MVmod(lhs, rhs); }
	PUBLIC friend var operator%(CVR lhs, const char* cstr2) { return MVmod(lhs, var(cstr2)); }
	PUBLIC friend var operator%(CVR lhs, const char char2) { return MVmod(lhs, var(char2)); }
	PUBLIC friend var operator%(CVR lhs, const int int2) { return MVmod(lhs, var(int2)); }
	PUBLIC friend var operator%(CVR lhs, const double double2) { return MVmod(lhs, var(double2)); }
	// disallow divide by boolean to prevent possible runtime divide by zero
	// PUBLIC friend var operator% (CVR     lhs    ,const bool    bool2   ){return
	// MVmod(lhs,var(bool2)  );}
	PUBLIC friend var operator%(const char* cstr1, CVR rhs) { return MVmod(var(cstr1), rhs); }
	PUBLIC friend var operator%(const char char1, CVR rhs) { return MVmod(var(char1), rhs); }
	PUBLIC friend var operator%(const int int1, CVR rhs) { return MVmod(var(int1), rhs); }
	PUBLIC friend var operator%(const double double1, CVR rhs) { return MVmod(var(double1), rhs); }
	//PUBLIC friend var operator%(const bool bool1, CVR rhs) { return MVmod(var(bool1), rhs); }

	//CONCATENATE
	// NB do *NOT* support concatenate with bool or vice versa!
	// to avoid compiler doing wrong precendence issue between ^ and logical operators
	PUBLIC friend var operator^(CVR lhs, CVR rhs) {
		//std::clog << "PUBLIC friend var operator^(CVR lhs, CVR rhs)" << std::endl;
		return MVcat(lhs, rhs);
	}
	//remove this to avoid some gcc ambiguous warnings although it means concat std:string will create a temp var
	//PUBLIC friend var operator^(CVR lhs, const std::string str2) {
	//    std::clog << "PUBLIC friend var operator^(CVR lhs, const std::string str2)" << std::endl;
	//    return MVcat(lhs, var(str2));
	//}
	PUBLIC friend var operator^(CVR lhs, const char* cstr) {
		//std::clog << "PUBLIC friend var operator^(CVR lhs, const char* cstr)" << std::endl;
		//return MVcat(lhs, var(cstr));
		return MVcat(lhs, cstr);
	}
	PUBLIC friend var operator^(CVR lhs, const char char2) {
		//std::clog << "PUBLIC friend var operator^(CVR lhs, const char char2)" << std::endl;
		return MVcat(lhs, char2);
	}
	PUBLIC friend var operator^(CVR lhs, const int int2) {
		//std::clog << "PUBLIC friend var operator^(CVR lhs, const int int2)" << std::endl;
		return MVcat(lhs, var(int2));
	}
	PUBLIC friend var operator^(CVR lhs, const double double2) {
		//std::clog << "PUBLIC friend var operator^(CVR lhs, const double double2)" << std::endl;
		return MVcat(lhs, var(double2));
	}
	PUBLIC friend var operator^(const char* cstr, CVR rhs) {
		//std::clog << "PUBLIC friend var operator^(const char* cstr, CVR rhs)" << std::endl;
		return MVcat(cstr, rhs);
	}
	PUBLIC friend var operator^(const char char1, CVR rhs) {
		//std::clog << "PUBLIC friend var operator^(const char char1, CVR rhs)" << std::endl;
		return MVcat(var(char1), rhs);
	}
	PUBLIC friend var operator^(const int int1, CVR rhs) {
		//std::clog << "PUBLIC friend var operator^(const int int1, CVR rhs)" << std::endl;
		return MVcat(var(int1), rhs);
	}
	PUBLIC friend var operator^(const double double1, CVR rhs) {
		//std::clog << "PUBLIC friend var operator^(const double double1, CVR rhs)" << std::endl;
		return MVcat(var(double1), rhs);
	}
	//temporaries (rvalues)
	PUBLIC friend var operator^(TVR lhs, CVR rhs) {
		//std::clog << "PUBLIC friend var operator^(TVR lhs, CVR rhs)" << std::endl;
		return lhs ^= rhs;
		//return std::move(lhs) ^= rhs;
	}
	PUBLIC friend var operator^(TVR lhs, const char* cstr2) {
		//std::clog << "PUBLIC friend var operator^(TVR lhs, const char* cstr2)" << std::endl;
		/*
    	var x="abcdefghijklmnop";//59ns
    	//var y(x);// (114ns)
    	var(x)^"a";// +126ns (240ns) surprisingly high looks like two additional heap allocs
    	*/
		//no difference in speed between the following:
		return lhs ^= cstr2;
		//return std::move(lhs) ^= cstr2;
	}
	PUBLIC friend var operator^(TVR lhs, const char char2) {
		//std::clog << "PUBLIC friend var operator^(TVR lhs, const char char2)" << std::endl;
		return lhs ^= char2;
	}
	PUBLIC friend var operator^(TVR lhs, const int int2) {
		//std::clog << "PUBLIC friend var operator^(TVR lhs, const int int2)" << std::endl;
		return lhs ^= int2;
	}
	PUBLIC friend var operator^(TVR lhs, const double double2) {
		//std::clog << "PUBLIC friend var operator^(TVR lhs, const double double2)" <<std::endl;
		return lhs ^= double2;
	}

	// unary operators +var -var !var
	PUBLIC ND friend var operator+(CVR var1) { return MVplus(var1); }
	PUBLIC ND friend var operator-(CVR var1) { return MVminus(var1); }
	PUBLIC ND friend bool operator!(CVR var1) { return MVnot(var1); }

	// OS TIME/DATE
	///////////////

	ND var date() const;
	ND var time() const;
	ND var timedate() const;
	void ossleep(const int milliseconds) const;
	var oswait(const int milliseconds, CVR directory) const;
	ND var ostime() const;
#ifdef SWIG
#define DEFAULT_STRING
#define DEFAULT_DOT
#define DEFAULT_SPACE
#define DEFAULT_VM
#else
#define DEFAULT_STRING = ""
#define DEFAULT_DOT = "."
#define DEFAULT_SPACE = " "
#define DEFAULT_VM = VM_
#endif
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

	// CURSOR
	/////////

	ND var at(const int columnorcode) const;
	ND var at(const int column, const int row) const;
	bool getcursor();
	void setcursor() const;
	ND var getprompt() const;
	void setprompt() const;

	// STANDARD INPUT
	/////////////////

	bool hasinput(int milliseconds = 0);
	VARREF input();
	VARREF input(CVR prompt);
	VARREF inputn(const int nchars);
	ND bool eof() const;
	bool echo(const int on_off) const;

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
	CVR exchange(CVR var2) const;
	ND var clone() const;

	// MATH/BOOLEAN
	///////////////

	ND var abs() const;
	ND var mod(CVR divisor) const;
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
	// not uint so to get utf codepoints > 2^63 must provide negative ints
	// not providing implicit constructor from var to uint due to getting ambigious conversions
	// since int and uint are parallel priority in c++ implicit conversions
	ND var textchr(const int num) const;  // UTF8
	ND var str(const int num) const;
	ND var space() const;

	// STRING INFO
	//////////////

	// bool match(CVR matchstr,CVR options DEFAULT_STRING) const;
	ND var match(CVR matchstr, CVR options DEFAULT_STRING) const;
	ND var seq() const;     // ASCII
	ND var textseq() const; // TEXT
	ND var dcount(CVR substrx) const;
	ND var count(CVR substrx) const;
#ifndef SWIGPERL
	// swig-perl chokes on this one character version with "SWIG_AsVal_char not defined" so skip
	// it for now (can use slow VARREF version) do something similar to the python config in
	// exodus.i
	ND var count(const char charx) const;
#endif
	ND var length() const;
	ND var len() const;
	const char* data() const;
	bool isnum() const;

	ND bool starts(CVR vstr) const;
	ND bool starts(const char* cstr) const;
	ND bool starts(const char c) const;

	ND bool ends(CVR vstr) const;
	ND bool ends(const char* cstr) const;
	ND bool ends(const char c) const;

	ND bool contains(CVR vstr) const;
	ND bool contains(const char* cstr) const;
	ND bool contains(const char c) const;

	//static member for speed on std strings
	static int localeAwareCompare(const std::string& str1, const std::string& str2);
	//int localeAwareCompare(const std::string& str2) const;

	// STRING MUTATORS
	//////////////////

	// all return VARREF and are not const)

	VARREF converter(CVR oldchars, CVR newchars);
	VARREF converter(const char* oldchars, const char* newchars);
	VARREF textconverter(CVR oldchars, CVR newchars);
	VARREF swapper(CVR whatstr, CVR withstr);
	VARREF replacer(CVR regexstr, CVR replacementstr, CVR options DEFAULT_STRING);
	VARREF splicer(const int start1, const int length, CVR str);
	VARREF splicer(const int start1, CVR str);
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
	VARREF trimmer(const char* trimchar DEFAULT_SPACE);
	VARREF trimmerf(const char* trimchar DEFAULT_SPACE);
	VARREF trimmerb(const char* trimchar DEFAULT_SPACE);
	VARREF trimmer(CVR trimchar);
	VARREF trimmer(CVR trimchar, CVR options);
	VARREF trimmerf(CVR trimchar);
	VARREF trimmerb(CVR trimchar);
	VARREF fieldstorer(CVR sepchar, const int fieldno, const int nfields, CVR replacement);
	VARREF substrer(const int startindex);
	VARREF substrer(const int startindex, const int length);

	//VARREF sorter(CVR separator DEFAULT_STRING);

	VARREF lowerer();
	VARREF raiser();
	VARREF cropper();

	// SAME ON TEMPORARIES
	//////////////////////

	ND VARREF convert(CVR oldchars, CVR newchars) &&;
	ND VARREF textconvert(CVR oldchars, CVR newchars) &&;
	ND VARREF swap(CVR whatstr, CVR withstr) &&;
	ND VARREF replace(CVR regexstr, CVR replacementstr, CVR options DEFAULT_STRING) &&;
	ND VARREF splice(const int start1, const int length, CVR str) &&;
	ND VARREF splice(const int start1, CVR str) &&;
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
	ND VARREF trim(const char* trimchar DEFAULT_SPACE) &&;
	ND VARREF trimf(const char* trimchar DEFAULT_SPACE) &&;
	ND VARREF trimb(const char* trimchar DEFAULT_SPACE) &&;
	ND VARREF trim(CVR trimchar) &&;
	ND VARREF trim(CVR trimchar, CVR options) &&;
	ND VARREF trimf(CVR trimchar) &&;
	ND VARREF trimb(CVR trimchar) &&;
	ND VARREF fieldstore(CVR sepchar, const int fieldno, const int nfields, CVR replacement) &&;
	ND VARREF substr(const int startindex) &&;
	ND VARREF substr(const int startindex, const int length) &&;

	ND VARREF lower() &&;
	ND VARREF raise() &&;
	ND VARREF crop() &&;

	// SAME BUT NON-MUTATING
	////////////////////////

	// all are const

	ND var convert(CVR oldchars, CVR newchars) const&;
	ND var textconvert(CVR oldchars, CVR newchars) const&;
	ND var swap(CVR whatstr, CVR withstr) const&;
	ND var replace(CVR regexstr, CVR replacementstr, CVR options DEFAULT_STRING) const&;
	ND var splice(const int start1, const int length, CVR str) const&;
	ND var splice(const int start1, CVR str) const&;
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
	ND var trim(const char* trimchar DEFAULT_SPACE) const&;
	ND var trimf(const char* trimchar DEFAULT_SPACE) const&;
	ND var trimb(const char* trimchar DEFAULT_SPACE) const&;
	ND var trim(CVR trimchar) const&;
	ND var trim(CVR trimchar, CVR options) const&;
	ND var trimf(CVR trimchar) const&;
	ND var trimb(CVR trimchar) const&;
	ND var fieldstore(CVR sepchar, const int fieldno, const int nfields, CVR replacement) const&;

	ND var lower() const&;
	ND var raise() const&;
	ND var crop() const&;

	// OTHER STRING ACCESS
	//////////////////////

	ND var hash(const unsigned long long modulus = 0) const;
	ND var unique() const;

	// CONVERT TO DIM (returns a dim)
	// see also dim.split()
	ND dim split(CVR separator DEFAULT_STRING) const;

	ND var sort(CVR separator DEFAULT_STRING) const;

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
	ND var oconv(CVR convstr) const;
	ND var iconv(const char* convstr) const;
	ND var iconv(CVR convstr) const;

	ND var from_codepage(CVR codepage) const;
	ND var to_codepage(CVR codepage) const;

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

	//.a(...) stands for .attribute(...) or extract(...)
	// pickos
	// xxx=yyy<10>";
	// becomes c++
	// xxx=yyy.a(10);
	ND var a(const int fieldno, const int valueno = 0, const int subvalueno = 0) const;
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
	ND var sum(CVR sepchar) const;

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
	// locate(fieldno, valueno, subvalueno,target,setting,by DEFAULT_STRING)
	ND bool locate(CVR target) const;
	bool locate(CVR target, VARREF setting) const;
	bool locate(CVR target, VARREF setting, const int fieldno, const int valueno = 0) const;

	bool locateusing(CVR usingchar, CVR target, VARREF setting, const int fieldno = 0, const int valueno = 0, const int subvalueno = 0) const;
	bool locateusing(CVR usingchar, CVR target) const;

	// locateby without fieldno or valueno arguments uses character VM
	bool locateby(const char* ordercode, CVR target, VARREF setting) const;
	bool locateby(CVR ordercode, CVR target, VARREF setting) const;
	// locateby with fieldno=0 uses character FM
	bool locateby(const char* ordercode, CVR target, VARREF setting, const int fieldno, const int valueno = 0) const;
	bool locateby(CVR ordercode, CVR target, VARREF setting, const int fieldno, const int valueno = 0) const;

	// locatebyusing
	bool locatebyusing(const char* ordercode, const char* usingchar, CVR target, VARREF setting, const int fieldno = 0, const int valueno = 0, const int subvalueno = 0) const;
	bool locatebyusing(CVR ordercode, CVR usingchar, CVR target, VARREF setting, const int fieldno = 0, const int valueno = 0, const int subvalueno = 0) const;

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

	bool createindex(CVR fieldname, CVR dictfile DEFAULT_STRING) const;
	bool deleteindex(CVR fieldname) const;
	ND var listindexes(CVR filename DEFAULT_STRING, CVR fieldname DEFAULT_STRING) const;

	bool sqlexec(CVR sqlcmd) const;
	bool sqlexec(CVR sqlcmd, VARREF response) const;

	// bool selftest() const;
	ND var version() const;

	ND var lasterror() const;
	void lasterror(CVR msg) const;


	// DATABASE ACCESS
	/////////////////

	bool connect(CVR conninfo DEFAULT_STRING);
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

	ND var reccount(CVR filename DEFAULT_STRING) const;
	var flushindex(CVR filename DEFAULT_STRING) const;

	bool open(CVR dbfilename, CVR connection DEFAULT_STRING);
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
	ND var xlate(CVR filename, CVR fieldno, CVR mode) const;

	// DATABASE SORT/SELECT
	//////////////////////

	bool select(CVR sortselectclause DEFAULT_STRING);
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

	bool saveselect(CVR filename);

	// OS FILE SYSTEM
	/////////////////

	bool osopen() const;
	bool osopen(CVR filename, CVR locale DEFAULT_STRING) const;
	bool osbread(CVR osfilevar, VARREF offset, const int length);
	bool osbread(CVR osfilevar, CVR offset, const int length);
	bool osbwrite(CVR osfilevar, VARREF offset) const;
	bool osbwrite(CVR osfilevar, CVR offset) const;
	void osclose() const;
	bool osread(CVR osfilename, CVR codepage DEFAULT_STRING);
	bool oswrite(CVR osfilename, CVR codepage DEFAULT_STRING) const;
	bool osdelete() const;
	bool osdelete(CVR osfilename) const;
	bool osrename(CVR newosdir_or_filename) const;
	bool oscopy(CVR to_osfilename) const;
	bool osmove(CVR to_osfilename) const;
	ND var oslist(CVR path DEFAULT_DOT, CVR wildcard DEFAULT_STRING, const int mode = 0) const;
	ND var oslistf(CVR path DEFAULT_DOT, CVR wildcard DEFAULT_STRING) const;
	ND var oslistd(CVR path DEFAULT_DOT, CVR wildcard DEFAULT_STRING) const;
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
	bool osread(const char* osfilename, CVR codepage DEFAULT_STRING);

	// OS SHELL/ENVIRONMENT
	///////////////////////

	bool suspend() const;
	bool osshell() const;
	bool osshellread(CVR oscmd);
	bool osshellwrite(CVR oscmd) const;
	ND var ostempdirname() const;
	ND var ostempfilename() const;

	bool osgetenv(CVR name);
	bool ossetenv(CVR name) const;

	// EXECUTE/PERFORM CONTROL
	/////////////////////////

	// done in mvprogram now since they need to pass mvenvironment
	// var perform() const;
	// var execute() const;
	// chain should be similar to one of the above?
	// var chain() const;

	void stop(CVR text DEFAULT_STRING) const;
	void abort(CVR text DEFAULT_STRING) const;
	void abortall(CVR text DEFAULT_STRING) const;

	var debug(CVR DEFAULT_STRING) const;

	var logoff() const;

	// PRIVATE MEMBER FUNCTIONS
	//////////////////////////

   private:
	void createString() const;

	ND bool cursorexists();
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

	// faster primitive arguments
	ND var iconv_D(const char* conversion) const;
	// var iconv_MT(const char* conversion) const;
	ND var iconv_MT() const;
	ND var iconv_MD(const char* conversion) const;
	ND var iconv_HEX(const int ioratio) const;

	ND const std::string to_path_string() const;
	ND const std::string to_cmd_string() const;

	//VARREF localeAwareChangeCase(const int lowerupper);

	ND std::fstream* osopenx(CVR osfilename, CVR locale) const;

	friend class dim;
	friend class var_iter;

	//BEGIN - free function to create an iterator -> begin
	PUBLIC friend var_iter begin(CVR v);

	//END - free function to create an interator -> end
	PUBLIC friend var_iter end(CVR v);

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

PUBLIC ND bool MVeq(CVR var1, CVR var2);
PUBLIC ND bool MVlt(CVR var1, CVR var2);

PUBLIC ND bool MVlt(CVR var1, const int int2);
PUBLIC ND bool MVlt(const int int1, CVR var2);

//PUBLIC ND var MVplus(CVR var1);
//PUBLIC ND var MVminus(CVR var1);
//PUBLIC ND bool MVnot(CVR var1);

PUBLIC ND var MVadd(CVR var1, CVR var2);

PUBLIC ND var MVmul(CVR var1, CVR var2);

PUBLIC ND var MVdiv(CVR var1, CVR var2);

#ifndef SWIG
PUBLIC ND double exodusmodulus(const double v1, const double v2);
#endif

PUBLIC ND var MVmod(CVR var1, CVR var2);

// var^var reassign logical xor to be std::string concatenate!!!
//PUBLIC ND var MVcat(CVR var1, CVR var2);

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
		return var_->a(fn_);
	}

	//operator assign = old pick replace but with round instead of angle brackets
	void operator=(CVR replacement){
		var_->r(fn_, replacement);
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
		return var_->a(fn_, vn_);
	}

	//operator assign = old pick replace but with round instead of angle brackets
	void operator=(CVR replacement){
		var_->r(fn_, vn_, replacement);
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
		return var_->a(fn_, vn_, sn_);
	}

	//operator assign = old pick replace but with round instead of angle brackets
	void operator=(CVR replacement){
		var_->r(fn_, vn_, sn_, replacement);
	}

}; // class var_proxy3

class dim_iter;

//class var_brackets_proxy
class PUBLIC var_brackets_proxy {
   public:
	std::string& str1;
	mutable int charindex1;

	//implicit conversion to var
	operator var() const;

	//operator assign a char
	void operator=(char char1);
};

//class dim
class PUBLIC dim {

   private:
	int nrows_, ncols_;
	// NOTE: trying to implement data_ as boost smart array pointer (boost::scoped_array<var>
	// data_;) raises warning: as dim is PUBLIC, boost library should have DLL interface.
	// Choices: 1) leave memory allocation as is (refuse from scoped_array, or
	// 2) use pimpl metaphor with slight decrease in performance.
	// Constructors of dim are very simple, no exception expected between 'new' and return from
	// constructor As such, choice (1).

	// all redimensioning of this array (eg when copying arrays)
	// seem to be using ::redim() to accomplish redimensioning
	// so only the redim code is dangerous (danger in one place is manageable)
	// we choose NOT to implement 2) above (pimpl) in order
	// to provide exodus programmer greater/easier visibility into dimensiorned arrays when
	// debugging (cannot use boost scoped pointer here because mv.h is required by exodus
	// programmer who should not need boost)
	var* data_;
	bool initialised_;

   public:
	dim(int nrows, int ncols = 1);

	bool redim(int nrows, int ncols = 1);

	ND var join(CVR sepchar = FM_) const;

	// parenthesis operators often come in pairs
	ND VARREF operator()(int rowno, int colno = 1);

	// following const version is called if we do () on a dim which was defined as const xx
	ND VARREF operator()(int rowno, int colno = 1) const;

	ND var rows() const;
	ND var cols() const;

	// Q: why is this commented out?
	// A: we dont want to COPY vars out of an array when using it in rhs expression
	// var operator() (int row, int col=1) const;

	// destructor to (NOT VIRTUAL to save space since not expected to be a base class)
	// protected to prevent deriving from var since wish to save space and not provide virtual
	// destructor http://www.gotw.ca/publications/mill18.htm
	~dim();

	dim& operator=(const dim& sourcedim);

	//=var
	// The assignment operator should always return a reference to *this.
	// cant be (CVR var1) because seems to cause a problem with var1=var2 in function
	// parameters unfortunately causes problem of passing var by value and thereby unnecessary
	// contruction see also ^= etc
	dim& operator=(CVR sourcevar);
	dim& operator=(const int sourceint);
	dim& operator=(const double sourcedbl);

	// allow default construction for class variables later resized in class methods
	dim();

	// see also var::split
	// return the number of fields
	var split(CVR var1, CVR separator DEFAULT_STRING);
	dim& sort(bool reverse = false);

	bool read(CVR filehandle, CVR key);
	bool write(CVR filehandle, CVR key) const;

	bool osread(CVR osfilename, CVR codepage DEFAULT_STRING);
	bool oswrite(CVR osfilename, CVR codepage DEFAULT_STRING) const;

	// following is implemented on the dim class now
	// dim dimarray2();
	//

	// move constructor
	dim(dim&& sourcedim) noexcept;

	friend class dim_iter;

	//BEGIN - free function to create an iterator -> begin
	PUBLIC friend dim_iter begin(const dim& d);

	//END - free function to create an interator -> end
	PUBLIC friend dim_iter end(const dim& d);

   private:
	// Disable copy constructor (why? to stop inefficient copies?)
	// Copy constructor
	dim(const dim& sourcedim);

	dim& init(CVR var1);

}; // class dim

// Note that "for (var& var1: dim1)" with & allows direct access and update into the elements of the array dim1 via varx
// whereaS "for (var var1 : dim1)" gives a copy of each element which is slower allows updating var1 without updating dim1
// Both cases are useful

//class dim_iter
class PUBLIC dim_iter {

   private:

	const dim* pdim_;

	// Start from 1 ignoring element 0
	int index_ = 1;

   public:

	// Default constructor
	dim_iter() = default;

	// Construct from dim
	dim_iter(const dim& d1);

	// Check iter != iter (i.e. iter != string::npos)
	bool operator!=(const dim_iter& dim_iter1);

	// Dereference iter to a var&
	// return a reference to the actual dim element so it can be updated
	// iif use var& instead of var in "for (var& : dim1)"
	//operator var*();
	var& operator*();

	//iter++
	dim_iter operator++();

	//iter--
	dim_iter operator--();

	void end();

}; // class dim_iter


// class range
// Represents an interable range of ints (int_iter)
// Lower case class name so we can use in syntax like
// "for (int i : range(1 to 10))"
// https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp
#define INT_T int
class PUBLIC range
{
	using int_t = INT_T;

	int first_;
	int last_;

public:

	// Represent an incrementable and comparable int
	class Iterator
	{
		int int_;

	public:

		// Construct from an int
		Iterator(int_t int1) : int_(int1) {};

		// Return our int
		int_t operator*() const { return int_; }

		// Increment our int and return it
		Iterator& operator++() { int_++; return *this; }

		//Iterator operator++(int_t) { Iterator tmp = *this; ++(*this); return tmp; }

		// Allow checking two Iterators for inequality
		friend bool operator!= (const Iterator& a, const Iterator& b) { return a.int_ != b.int_; };

	};

	// Construct a range from two ints
	range( int_t first, int_t last) : first_(first), last_(last + 1) {};

	// Return our first int
	Iterator begin() { return Iterator(first_); }

	// Return our last int
	Iterator end()   { return Iterator(last_); }

};

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

inline const var DQ = DQ_;
inline const var SQ = SQ_;

#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__
inline const var OSSLASH = "\\";
inline const char OSSLASH_ = '\\';
#define SLASH_IS_BACKSLASH true
#else
inline const var OSSLASH = "/";
inline const char OSSLASH_ = '/';
#define SLASH_IS_BACKSLASH false
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

// following are all not thread safe since they are at global scope and not const
// perhaps they should be moved on to MvEnvironment mv
// and disallow their use except in ExodusPrograms ie not in plain functions
// dict functions have mv provided by arguments
/*

#ifndef EXO_MV_CPP
extern
PUBLIC var _STATUS;
#else
PUBLIC var _STATUS="";
#endif

#ifndef EXO_MV_CPP
extern
PUBLIC exodus::var EXECPATH;
#else
PUBLIC exodus::var EXECPATH="";
#endif

//FM separated words or quoted phrases from command line. quote marks are retained.
//trailing options in () or {} are stripped off and available in OPTIONS
#ifndef EXO_MV_CPP
extern
PUBLIC exodus::var COMMAND;
#else
PUBLIC exodus::var COMMAND="";
#endif

#ifndef EXO_MV_CPP
extern
PUBLIC exodus::var OPTIONS;
#else
PUBLIC exodus::var OPTIONS="";
#endif

//see also COMMAND and OPTIONS
#ifndef EXO_MV_CPP
extern
PUBLIC exodus::var SENTENCE;
#else
PUBLIC exodus::var SENTENCE="";
#endif
*/

// this is left a global copy for backtrace to get at it
//#ifndef EXO_MV_CPP
//	extern
//	PUBLIC exodus::var EXECPATH2;
//#else
//	PUBLIC exodus::var EXECPATH2="";
//#endif
PUBLIC inline exodus::var EXECPATH2 = "";

PUBLIC inline bool TERMINATE_req = false;
PUBLIC inline bool RELOAD_req = false;

//void PUBLIC output(CVR var1);
//void PUBLIC outputl(CVR var1 DEFAULT_STRING);
//void PUBLIC outputt(CVR var1 DEFAULT_STRING);

//void PUBLIC errput(CVR var1);
//void PUBLIC errputl(CVR var1 DEFAULT_STRING);

//void PUBLIC logput(CVR var1);
//void PUBLIC logputl(CVR var1 DEFAULT_STRING);

PUBLIC ND var backtrace();

#ifndef SWIG
std::string naturalorder(const std::string& string1);
#endif

PUBLIC ND int getenvironmentn();
PUBLIC void setenvironmentn(const int environmentn);

PUBLIC ND int getenvironmentn();
PUBLIC ND var getprocessn();
PUBLIC ND var getexecpath();

// mv exceptions

// MVStop is similar to MVError
// but doesnt get stack since stop() is called normally
class PUBLIC MVStop {
   public:
	explicit MVStop(CVR var1 DEFAULT_STRING);
	var description;
};

// MVAbort is similar to MVError
// but doesnt get stack since abort() is called normally
class PUBLIC MVAbort {
   public:
	explicit MVAbort(CVR var1 DEFAULT_STRING);
	var description;
};

// MVAbortAll is similar to MVError
// but doesnt get stack since abortall() is called normally
class PUBLIC MVAbortAll {
   public:
	explicit MVAbortAll(CVR var1 DEFAULT_STRING);
	var description;
};

// MVLogoff is similar to MVError
// but doesnt get stack since abortall() is called normally
class PUBLIC MVLogoff {
   public:
	explicit MVLogoff(CVR var1 DEFAULT_STRING);
	var description;
};

// provide a public base exception for all other exceptions so exodus programmers can catch mv
// exceptions generally
class PUBLIC MVError {
   public:
	explicit MVError(CVR description);
	var description;
	var stack;
};

// clang-format off

//individual exceptions are made public so exodus programmers can catch specific errors or even stop/abort/debug if they want
class PUBLIC MVDivideByZero         : public MVError {public: explicit MVDivideByZero         (CVR var1    );};
class PUBLIC MVNonNumeric           : public MVError {public: explicit MVNonNumeric           (CVR var1    );};
class PUBLIC MVIntOverflow          : public MVError {public: explicit MVIntOverflow          (CVR var1    );};
class PUBLIC MVIntUnderflow         : public MVError {public: explicit MVIntUnderflow         (CVR var1    );};
class PUBLIC MVOutOfMemory          : public MVError {public: explicit MVOutOfMemory          (CVR var1    );};
class PUBLIC MVUnassigned           : public MVError {public: explicit MVUnassigned           (CVR var1    );};
class PUBLIC MVUndefined            : public MVError {public: explicit MVUndefined            (CVR var1    );};
class PUBLIC MVInvalidPointer       : public MVError {public: explicit MVInvalidPointer       (CVR var1    );};
class PUBLIC MVDBException          : public MVError {public: explicit MVDBException          (CVR var1    );};
class PUBLIC MVNotImplemented       : public MVError {public: explicit MVNotImplemented       (CVR var1    );};
class PUBLIC MVDebug                : public MVError {public: explicit MVDebug                (CVR var1 DEFAULT_STRING);};
class PUBLIC MVArrayDimensionedZero : public MVError {public: explicit MVArrayDimensionedZero (                   );};
class PUBLIC MVArrayIndexOutOfBounds: public MVError {public: explicit MVArrayIndexOutOfBounds(CVR var1    );};
class PUBLIC MVArrayNotDimensioned  : public MVError {public: explicit MVArrayNotDimensioned  (                   );};

// clang-format on

//user defined literals suffix _var
PUBLIC var operator""_var(const char* cstr, std::size_t size);
PUBLIC var operator""_var(unsigned long long int i);
PUBLIC var operator""_var(long double d);

}  // namespace exodus

#endif    // MV_H
