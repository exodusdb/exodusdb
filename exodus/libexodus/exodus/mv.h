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

#ifndef MV_H
#define MV_H 1

// prevent swig perl linking errors about win32_abort win32_select win32_connect
#if defined(SWIGPERL)
#if defined connect
#undef connect
#endif
#if defined select
#undef select
#endif
#if defined abort
#undef abort
#endif
#endif

#define EXODUS_RELEASE "19.5"
#define EXODUS_PATCH "19.5.0"

// if installing with autotools then for latest version of boost and other installation macro
// download the snapshot from here. AX_BOOST_DATE_TIME etc
// http://git.savannah.gnu.org/gitweb/?p=autoconf-archive.git;a=tree;f=m4

// http://stackoverflow.com/questions/538134/exporting-functions-from-a-dll-with-dllexport
// Using dllimport and dllexport in C++ Classes
// http://msdn.microsoft.com/en-us/library/81h27t8c(VS.80).aspx

#define SMALLEST_NUMBER 1e-13

#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__
#ifdef BUILDING_LIBRARY
#ifdef __GNUC__
#define DLL_PUBLIC __attribute__((dllexport))
#else
#define DLL_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also support this syntax.
#endif
#else
#ifdef __GNUC__
#define DLL_PUBLIC __attribute__((dllimport))
#else
#define DLL_PUBLIC __declspec(dllimport) // Note: actually gcc seems to also support this syntax.
#endif
#endif
#define DLL_LOCAL
#else
#if __GNUC__ >= 4
// use g++ -fvisibility=hidden to make all hidden except those marked DLL_PUBLIC ie "default"
#define DLL_PUBLIC __attribute__((visibility("default")))
#define DLL_LOCAL __attribute__((visibility("hidden")))
#else
#define DLL_PUBLIC
#define DLL_LOCAL
#endif
#endif

// TODO .rounder(n), .round(n) and round(x,n) functions something like the following
// round .5 up and -.5 down to nearest integer as per pick financial rounding concept
// 1234.5678
// n=2 gives 1234.57
// n=0 gives 1235
// n=-2 gives 1200
// storage could be changed to integer if n<=0 or left probably better since likely to be added to
// other similar ints thereafter if (var_dbl>=0) 	var_dbl=long long int(var_dbl+0.5) else
// var_dbl=long
// long int(var_dbl-0.5);

/*		gcc	msc
was for 32bit (needs revising after int became long long (64bit/8byte even in 32bit impl)
sizeof
char:	1	1
char:4	2
string:	4	32
wstring:4	32
int:	4	4
double:	8	8
var:	20	48
*/

/* this has been resolved somehow without removing the automatic conversion to int:
Remove automatic conversion to int which takes precedence over the automatic conversion to bool
and therefore causes a non-numeric error if you include a non-numeric value in an if statement like

 aa="xx"
 if (aa) {}

*/

// only for some emergency debugging
#include <iostream>

#include <iosfwd>
#include <string>
#include <vector>

// questionable MS language "extensions" disable some standard C++ keywords ... restore them
// http://members.ozemail.com.au/~geoffch/samples/programming/msvc/cl/options/z$e.htm
// complete list of c/c++ keywords is
// and, and_eq, bitand, bitor, compl, not, not_eq, or, or_eq, xor and xor_eq
#ifdef _MSC_VER
#define and &&
#define not_eq !=
#define not !
#define or ||
#endif

// http://www.viva64.com/content/articles/64-bit-development/?f=20_issues_of_porting_C++_code_on_the_64-bit_platform.html
using mvint_t = long long;

#define	ND [[nodiscard]]

namespace exodus
{

// the var versions of the above (without leading or trailing _)
// are defined AFTER the class declaration of "var"

// Decided to use ASCII characters 0x18-0x1F instead of the classic 00F8-00FF which are latin
// accented characters

// would be 256 if RM was character number 255. is 1F+1 used in var::remove()
#define LASTDELIMITERCHARNOPLUS1 0x20

// also defined in extract.c and exodusmacros.h

// leading and trailing _ char* versions of classic pick delimiters
// also in ADECOM
#define _RM_ "\x1F"  // Record Mark
#define _FM_ "\x1E"  // Field Mark
#define _VM_ "\x1D"  // Value Mark
#define _SM_ "\x1C"  // Subvalue Mark
#define _TM_ "\x1B"  // Text Mark
#define _STM_ "\x1A" // Subtext Mark

// aliases for different implementations of multivalue
#define _IM_ _RM_
#define _AM_ _FM_
#define _SVM_ _SM_

#define _DQ_ "\""
#define _SQ_ "\'"

// trailing _ char versions of classic pick delimiters
#define RM_ '\x1F'  // Record Mark
#define FM_ '\x1E'  // Field Mark
#define VM_ '\x1D'  // Value Mark
#define SM_ '\x1C'  // Subvalue Mark
#define TM_ '\x1B'  // Text Mark
#define STM_ '\x1A' // Subtext Mark

// aliases for different implementations of multivalue
#define IM_ RM_
#define AM_ FM_
#define SVM_ SM_

#define DQ_ '\"'
#define SQ_ '\''

// the argument for utf16 http://www.unicode.org/notes/tn12/ (utf8/32 cannot handle binary because
// of illegal byte sequences) win32/java/icu/python is utf16 but situation is not so clear on unix
// (where char is 32bit) but see http://std.dkuug.dk/JTC1/SC22/WG14/www/docs/n1040.pdf for <uchar.h>

class dim;
class var__extractreplace;
// class PFstream;

#ifndef SWIG

// TODO ensure locale doesnt produce like 123.456,78
std::string intToString(int int1);

// TODO ensure locale doesnt produce like 123.456,78
std::string dblToString(double double1);

#endif

//#define arg const var& - too likely to conflict with things eg in postgres library
//#define call - conflicts with dynamic so/dll load/call

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
// string:	32
// int:		8
// double:	8
// char:		4
// var:		56

#include <cstdint>

class var_brackets_proxy;

class VARTYP
{
      public:
	// constructors from uint and it
	// VARTYP(uint rhs) {flags=rhs;std::cout<<"uint32 ctor "<< rhs << std::endl;};
	// VARTYP(int rhs) {flags=rhs;std::cout<<"int ctor "<< rhs << std::endl;if (rhs==9999) throw
	// "kjhkjhkkjh";};
	VARTYP(uint rhs) : flags_(rhs){};

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
	VARTYP& operator=(const uint newflags)
	{
		flags_ = newflags;
		return *this;
	};

	// bitwise mutators
	VARTYP& operator^=(const uint rhs)
	{
		flags_ ^= rhs;
		return *this;
	};
	VARTYP& operator|=(const uint rhs)
	{
		flags_ |= rhs;
		return *this;
	};
	VARTYP& operator&=(const uint rhs)
	{
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
};

// WARNING these VARTYP constants must be initialised before any var variables are
// NOT they are declared inline which presumably makes them the same in all compilation units
// but it also seems to ensure that they are initialised BEFORE any var variables
// theoretically they should be because C++ says
// this is mandatory because initialising var variables REQUIRES these constants to be available
// with their correct values otherwise any such var, when used later on, will throw Unassigned
// Variable Used since its var_typ will be zero

#define VTC(VARNAME, VARVALUE) inline const uint VARNAME{VARVALUE};

// throw an exception if used an unassigned variable
// inline const VARTYP VARTYP_UNA =0x0;
VTC(VARTYP_UNA, 0x0)

// assigned string - unknown if numeric or not
VTC(VARTYP_STR, 0x1)

// following are numeric
VTC(VARTYP_INT, 0x2)
VTC(VARTYP_DBL, 0x4)

// indicated known non-numeric string
VTC(VARTYP_NAN, 0x8)

// following are numeric
VTC(VARTYP_OSFILE, 0x10)
VTC(VARTYP_DBCONN, 0x20)

// flag combinations
VTC(VARTYP_INTDBL, VARTYP_INT | VARTYP_DBL)
VTC(VARTYP_INTSTR, VARTYP_INT | VARTYP_STR)
VTC(VARTYP_DBLSTR, VARTYP_DBL | VARTYP_STR)
VTC(VARTYP_NANSTR, VARTYP_NAN | VARTYP_STR)
VTC(VARTYP_NOTNUMFLAGS, ~(VARTYP_INT | VARTYP_DBL | VARTYP_NAN))

VTC(VARTYP_NANSTR_OSFILE, VARTYP_NANSTR | VARTYP_OSFILE)
VTC(VARTYP_NANSTR_DBCONN, VARTYP_NANSTR | VARTYP_DBCONN)

//VTC(VARTYP_DESTRUCTED, 0xFFFFF0)

// const char mvtypemask=0x80;
//VTC(VARTYP_MASK, 0x80)
VTC(VARTYP_MASK, ~(VARTYP_STR | VARTYP_NAN | VARTYP_INT | VARTYP_DBL | VARTYP_OSFILE | VARTYP_OSFILE | VARTYP_DBCONN))

// class var
//"final" to prevent inheritance because var has a destructor which is non-virtual to save space and time
class DLL_PUBLIC var final
{

      public:
	// CONSTRUCTORS
	///////////////

	// default constructor to allow plain unassigned "var mv;" syntax
	var();

	// copy constructor
	var(const var& var1) = default;
	// var(var var1);

	// move constructor
	var(var&& var1) = default;

	// copy assignment
	//=var
	// The assignment operator should always return a reference to *this.
	// cant be (const var& var1) because seems to cause a problem with var1=var2 in function
	// parameters unfortunately causes problem of passing var by value and thereby unnecessary
	// contruction see also ^= etc
	var& operator=(const var& var1) = default;
	// var& operator=(var var1) noexcept;

	// move assigment
	var& operator=(var&& var1) = default;

	//WARNING: non-virtual destructor - so cannot create derived classes
	~var();

	// O
#ifndef SWIGJAVA
	// constructor for char*
	// place first before char so SWIG isnt tempted to use char to acquire strings resulting in
	// ONE character strings)
	//	MV_CONSTRUCTION_FROM_CHAR_EXPLICIT
	var(const char* cstr1);
#endif

	// constructor for char to create
#ifndef SWIG
	//	MV_CONSTRUCTION_FROM_CHAR_EXPLICIT
	var(const char char1) noexcept;
#endif
	// constructor for char memory block
	//	MV_CONSTRUCTION_FROM_CHAR_EXPLICIT
	var(const char* cstr1, const size_t int1);

	// swig java duplicates this with var(std::string&) above
#if !defined(SWIGJAVA) && !defined(SWIGCSHARP)
	// constructor for std::string
	var(const std::string& str1);
#endif

	// constructor for bool (dont allow this if no constructor for char* otherwise " " is
	// converted to 1 in widechar only compilations
	//	MV_CONSTRUCTION_FROM_CHAR_EXPLICIT
	var(const bool bool1) noexcept;

	// constructor for int
	var(const int int1) noexcept;

	// suppressing for now since ambiguous with int
	//#if 0
	// constructor for long long
	var(const long long longlong1) noexcept;
	//#endif

	// constructor for double
	var(const double double1) noexcept;

	// NAMED CONVERSIONS
	////////////////////

	bool toBool() const;

	// standard c/c++ int() in other words simply take the number to the left of the point. -1.5
	// becomes -1 and 1.5 becomes 1
	int toInt() const;
	int toLong() const;

	double toDouble() const;

	std::u32string to_u32string() const;
	void from_u32string(std::u32string) const;

	std::string toString() const;

	// weird version for perl that outputs "" if undefined
	std::string toString2() const;

	// AUTOMATIC CONVERSIONS
	////////////////////////

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
#define HASINTREFOP
#ifndef HASINTREFOP
	operator int() const;
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

	// provide a reference to the (long long) int inside a var
	// This allows passing by reference to the guts of var for speed
	// it is safe because changing the int has no side effects
#ifdef HASINTREFOP
	operator int&() const;
	operator double&() const;
#endif
	// string - replicates toString()
	// would allow the usage of any std::string function but may result
	// in a lot of compilation failures due to "ambiguous overload"
	// unfortunately there is no "explicit" keyword as for constructors - coming in C++0X
	// for now prevent this to ensure efficient programming
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
	var operator()(int fieldno, int valueno = 0, int subvalueno = 0) const;
	// DONT declare this so we force use of the above const version that produces a temporary
	//var& operator()(int fieldno, int valueno = 0, int subvalueno = 0);
	/* SADLY no way to get a different operator() function called when on the left hand side of assign
   	therefore not possible to create syntax like "xyz(1,2)="xx";" to REPLACE fields, values and
   	subvalues so operator() is defined only for the more common xyz=extract(1,2,3); i.e. on the right
   	hand side. http://codepad.org/MzzhlRkb
	*/

	// BRACKETS []
	//////////////

	// extract a character first=1 last=-1 etc
	var operator[](const int charno) const;

	/* sadly this all works EXCEPT that var[99].anymethod doesnt work
	so would have to implement all var methods on the proxy object
	var_brackets_proxy operator[](int charno) const&;
	var operator[](int charno) &&;
	*/

	// UNARY OPERATORS
	/////////////////

	//=int
	var& operator=(const int int1);

	//=double
	var& operator=(const double double1);

	//=char
	var& operator=(const char char2);

	//=char*
	var& operator=(const char* char2);

	//=string
	var& operator=(const std::string& string2);

	//=var
	var& operator^=(const var& var1);

	//=int
	var& operator^=(const int int1);

	//=double
	var& operator^=(const double double1);

	//=char
	var& operator^=(const char char2);

	//=char*
	var& operator^=(const char* char2);

	//=string
	var& operator^=(const std::string& string2);

	/*
	//postfix returning void so cannot be used in expressions (avoid unreadable programs)
	void operator++ (int);
	void operator-- (int);

	//TODO: since preventing pre and postfix in expressions then force only use of prefix
	version since it is faster

	//prefix returning void so cannot be used in expressions (avoid unreadable programs)
	void operator++ ();
	void operator-- ();
	*/

#ifndef HASINTREFOP
#else
	// postfix
	var operator++(int);
	var operator--(int);

	// prefix
	var& operator++();
	var& operator--();
	//+=var
	var& operator+=(int int1);
	var& operator+=(double dbl1);
	//-=var
	var& operator-=(int int1);
	var& operator-=(double dbl1);

	// not handled by inbuilt conversion of var to int
	//+=var
	var& operator+=(const var& var1);
	//-=var
	var& operator-=(const var& var1);
#endif

	// FRIENDS
	/////////

	DLL_PUBLIC friend var MVadd(const var&, const var&);
	DLL_PUBLIC friend var MVsub(const var&, const var&);
	DLL_PUBLIC friend var MVmul(const var&, const var&);
	DLL_PUBLIC friend var MVdiv(const var&, const var&);
	DLL_PUBLIC friend var MVmod(const var&, const var&);
	DLL_PUBLIC friend var MVcat(const var&, const var&);

	DLL_PUBLIC friend bool MVeq(const var&, const var&);
	DLL_PUBLIC friend bool MVlt(const var&, const var&);
	// following specialisations done for speed of var in for loops
	DLL_PUBLIC friend bool MVlt(const var&, const int);
	DLL_PUBLIC friend bool MVlt(const int, const var&);

	DLL_PUBLIC friend var operator+(const var&, const var&);
	DLL_PUBLIC friend var operator+(const var&, const char*);
	DLL_PUBLIC friend var operator+(const var&, const int);
	DLL_PUBLIC friend var operator+(const var&, const double);
	//DLL_PUBLIC friend var operator+(const var&, const bool);
	DLL_PUBLIC friend var operator+(const char*, const var&);
	DLL_PUBLIC friend var operator+(const int, const var&);
	DLL_PUBLIC friend var operator+(const double, const var&);
	//DLL_PUBLIC friend var operator+(const bool, const var&);
	//rvalues
	DLL_PUBLIC friend var operator+(var&&, const var&);
	DLL_PUBLIC friend var operator+(var&&, const char*);
	DLL_PUBLIC friend var operator+(var&&, const int);
	DLL_PUBLIC friend var operator+(var&&, const double);

	DLL_PUBLIC friend var operator-(const var&, const var&);
	DLL_PUBLIC friend var operator-(const var&, const char*);
	DLL_PUBLIC friend var operator-(const var&, const int);
	DLL_PUBLIC friend var operator-(const var&, const double);
	//DLL_PUBLIC friend var operator-(const var&, const bool);
	DLL_PUBLIC friend var operator-(const char*, const var&);
	DLL_PUBLIC friend var operator-(const int, const var&);
	DLL_PUBLIC friend var operator-(const double, const var&);
	//DLL_PUBLIC friend var operator-(const bool, const var&);
	//rvalues
	DLL_PUBLIC friend var operator+(var&&, const var&);
	DLL_PUBLIC friend var operator+(var&&, const char*);
	DLL_PUBLIC friend var operator+(var&&, const int);
	DLL_PUBLIC friend var operator+(var&&, const double);

	DLL_PUBLIC friend var operator*(const var&, const var&);
	DLL_PUBLIC friend var operator*(const var&, const char*);
	DLL_PUBLIC friend var operator*(const var&, const int);
	DLL_PUBLIC friend var operator*(const var&, const double);
	//DLL_PUBLIC friend var operator*(const var&, const bool);
	DLL_PUBLIC friend var operator*(const char*, const var&);
	DLL_PUBLIC friend var operator*(const int, const var&);
	DLL_PUBLIC friend var operator*(const double, const var&);
	//DLL_PUBLIC friend var operator*(const bool, const var&);

	DLL_PUBLIC friend var operator/(const var&, const var&);
	DLL_PUBLIC friend var operator/(const var&, const char*);
	DLL_PUBLIC friend var operator/(const var&, const int);
	DLL_PUBLIC friend var operator/(const var&, const double);
	// disallow divide by boolean to prevent possible runtime divide by zero
	// DLL_PUBLIC friend  var operator/ (const var&,const bool);
	DLL_PUBLIC friend var operator/(const char*, const var&);
	DLL_PUBLIC friend var operator/(const int, const var&);
	DLL_PUBLIC friend var operator/(const double, const var&);
	//DLL_PUBLIC friend var operator/(const bool, const var&);

	DLL_PUBLIC friend var operator%(const var&, const var&);
	DLL_PUBLIC friend var operator%(const var&, const char*);
	DLL_PUBLIC friend var operator%(const var&, const int);
	DLL_PUBLIC friend var operator%(const var&, const double);
	// disallow divide by boolean to prevent possible runtime divide by zero
	// DLL_PUBLIC friend  var operator/ (const var&,const bool);
	DLL_PUBLIC friend var operator%(const var&, const bool);
	DLL_PUBLIC friend var operator%(const char*, const var&);
	DLL_PUBLIC friend var operator%(const int, const var&);
	DLL_PUBLIC friend var operator%(const double, const var&);
	//DLL_PUBLIC friend var operator%(const bool, const var&);

	// NB do NOT allow concat with bool or vice versa - find reason why
	DLL_PUBLIC friend var operator^(const var&, const var&);
	DLL_PUBLIC friend var operator^(const var&, const char*);
	DLL_PUBLIC friend var operator^(const var&, const int);
	DLL_PUBLIC friend var operator^(const var&, const double);
	DLL_PUBLIC friend var operator^(const char*, const var&);
	DLL_PUBLIC friend var operator^(const int, const var&);
	DLL_PUBLIC friend var operator^(const double, const var&);
	//rvalues
	DLL_PUBLIC friend var operator^(var&&, const var&);
	DLL_PUBLIC friend var operator^(var&&, const char*);
	DLL_PUBLIC friend var operator^(var&&, const int);
	DLL_PUBLIC friend var operator^(var&&, const double);

#ifdef SPACESHIP_OPERATOR
	// spaceship operator in c++17? eliminates need to define all six < > <= >= == != operators
	DLL_PUBLIC friend bool operator<=>(const var&, const var&);
	DLL_PUBLIC friend bool operator<=>(const var&, const char*);
	DLL_PUBLIC friend bool operator<=>(const var&, const int);
	DLL_PUBLIC friend bool operator<=>(const var&, const double);
	DLL_PUBLIC friend bool operator<=>(const char*, const var&);
	DLL_PUBLIC friend bool operator<=>(const int, const var&);
	DLL_PUBLIC friend bool operator<=>(const double, const var&);
	DLL_PUBLIC friend bool operator<=>(const var&, const char*);
	DLL_PUBLIC friend bool operator<=>(const char*, const var&);
#else
	DLL_PUBLIC friend bool operator<(const var&, const var&);
	DLL_PUBLIC friend bool operator<(const var&, const char*);
	DLL_PUBLIC friend bool operator<(const var&, const int);
	DLL_PUBLIC friend bool operator<(const var&, const double);
	DLL_PUBLIC friend bool operator<(const char*, const var&);
	DLL_PUBLIC friend bool operator<(const int, const var&);
	DLL_PUBLIC friend bool operator<(const double, const var&);
	DLL_PUBLIC friend bool operator<(const var&, const char*);
	DLL_PUBLIC friend bool operator<(const char*, const var&);

	DLL_PUBLIC friend bool operator<=(const var&, const var&);
	DLL_PUBLIC friend bool operator<=(const var&, const char*);
	DLL_PUBLIC friend bool operator<=(const var&, const int);
	DLL_PUBLIC friend bool operator<=(const var&, const double);
	DLL_PUBLIC friend bool operator<=(const char*, const var&);
	DLL_PUBLIC friend bool operator<=(const int, const var&);
	DLL_PUBLIC friend bool operator<=(const double, const var&);
	DLL_PUBLIC friend bool operator<=(const var&, const char*);
	DLL_PUBLIC friend bool operator<=(const char*, const var&);

	DLL_PUBLIC friend bool operator>(const var&, const var&);
	DLL_PUBLIC friend bool operator>(const var&, const char*);
	DLL_PUBLIC friend bool operator>(const var&, const int);
	DLL_PUBLIC friend bool operator>(const var&, const double);
	DLL_PUBLIC friend bool operator>(const char*, const var&);
	DLL_PUBLIC friend bool operator>(const int, const var&);
	DLL_PUBLIC friend bool operator>(const double, const var&);
	DLL_PUBLIC friend bool operator>(const var&, const char*);
	DLL_PUBLIC friend bool operator>(const char*, const var&);

	DLL_PUBLIC friend bool operator>=(const var&, const var&);
	DLL_PUBLIC friend bool operator>=(const var&, const char*);
	DLL_PUBLIC friend bool operator>=(const var&, const int);
	DLL_PUBLIC friend bool operator>=(const var&, const double);
	DLL_PUBLIC friend bool operator>=(const char*, const var&);
	DLL_PUBLIC friend bool operator>=(const int, const var&);
	DLL_PUBLIC friend bool operator>=(const double, const var&);
	DLL_PUBLIC friend bool operator>=(const var&, const char*);
	DLL_PUBLIC friend bool operator>=(const char*, const var&);

	DLL_PUBLIC friend bool operator==(const var&, const var&);
	DLL_PUBLIC friend bool operator==(const var&, const char*);
	DLL_PUBLIC friend bool operator==(const var&, const int);
	DLL_PUBLIC friend bool operator==(const var&, const double);
	DLL_PUBLIC friend bool operator==(const var&, const bool);
	DLL_PUBLIC friend bool operator==(const char*, const var&);
	DLL_PUBLIC friend bool operator==(const int, const var&);
	DLL_PUBLIC friend bool operator==(const double, const var&);
	DLL_PUBLIC friend bool operator==(const bool, const var&);
	DLL_PUBLIC friend bool operator==(const var&, const char*);
	DLL_PUBLIC friend bool operator==(const char*, const var&);

	DLL_PUBLIC friend bool operator!=(const var&, const var&);
	DLL_PUBLIC friend bool operator!=(const var&, const char*);
	DLL_PUBLIC friend bool operator!=(const var&, const int);
	DLL_PUBLIC friend bool operator!=(const var&, const double);
	DLL_PUBLIC friend bool operator!=(const var&, const bool);
	DLL_PUBLIC friend bool operator!=(const char*, const var&);
	DLL_PUBLIC friend bool operator!=(const int, const var&);
	DLL_PUBLIC friend bool operator!=(const double, const var&);
	DLL_PUBLIC friend bool operator!=(const bool, const var&);
	DLL_PUBLIC friend bool operator!=(const var&, const char*);
	DLL_PUBLIC friend bool operator!=(const char*, const var&);
#endif
	// unary operators +var -var !var
	DLL_PUBLIC friend var operator+(const var&);
	DLL_PUBLIC friend var operator-(const var&);
	DLL_PUBLIC friend bool operator!(const var&);

	// OS TIME/DATE
	///////////////

	var date() const;
	var time() const;
	var timedate() const;
	void ossleep(const int milliseconds) const;
	var ostime() const;
#ifdef SWIG
#define DEFAULTNULL
#define DEFAULTDOT
#define DEFAULTSPACE
#define DEFAULTVM
#else
#define DEFAULTNULL = ""
#define DEFAULTDOT = "."
#define DEFAULTSPACE = " "
#define DEFAULTVM = VM_
#endif
	void breakon() const;
	void breakoff() const;

	// STANDARD OUTPUT
	//////////////////

	const var& output() const;
	const var& outputl() const;
	const var& outputt() const;

	const var& output(const var& var1) const;
	const var& outputl(const var& var1) const;
	const var& outputt(const var& var1) const;

	const var& logput() const;
	const var& logputl() const;

	const var& logput(const var& var1) const;
	const var& logputl(const var& var1) const;

	const var& errput() const;
	const var& errputl() const;

	const var& errput(const var& var1) const;
	const var& errputl(const var& var1) const;

	const var& put(std::ostream& ostream1) const;

	// CURSOR
	/////////

	var at(const int columnorcode) const;
	var at(const int column, const int row) const;
	var getcursor() const;
	void setcursor() const;
	var getprompt() const;
	void setprompt() const;

	// STANDARD INPUT
	/////////////////

	bool hasinput();
	bool input();
	bool input(const var& prompt, const int nchars = 0);
	bool eof() const;

	DLL_PUBLIC friend std::istream& operator>>(std::istream& istream1, var& var1);
	DLL_PUBLIC friend std::ostream& operator<<(std::ostream& ostream1, const var& var1);

	// friend bool operator<<(const var&);

	// VARIABLE CONTROL
	///////////////////

	bool assigned() const;
	bool unassigned() const;
	var& unassigned(const var& defaultvalue);

	var& transfer(var& destinationvar);
	const var& exchange(const var& var2) const;
	var clone() const;

	/*no implemented yet
	var addressof() const;
	void clear();
	void clearcommon();
//	var bitand(const var) const;
//	var bitor(const var) const;
//	var bitxor(const var) const;
	var bitnot() const;
	*/

	// MATH/BOOLEAN
	///////////////

	var abs() const;
	var mod(const var& divisor) const;
	var mod(const int divisor) const;
	var pwr(const var& exponent) const;
	var rnd() const;
	void initrnd() const;
	var exp() const;
	var sqrt() const;
	var sin() const;
	var cos() const;
	var tan() const;
	var atan() const;
	var loge() const;
	// integer() represents pick int() because int() is reserved word in c/c++
	// Note that integer like pick int() is the same as floor()
	// whereas the usual c/c++ int() simply take the next integer nearest 0 (ie cuts of any
	// fractional decimal places) to get the usual c/c++ effect use toInt() (although toInt()
	// returns an int instead of a var like normal exodus functions)
	var integer() const;
	var floor() const;
	var round(const int ndecimals = 0) const;

	// LOCALE
	/////////

	bool setxlocale() const;
	var& getxlocale();

	// STRING CREATION
	//////////////////

	//	var chr() const;
	// version 1 chr - only char 0 - 255 returned in a single byte
	// bytes 128-255 are not valid utf-8 so cannot be written to database/postgres
	var chr(const int num) const; // ASCII
	// version 2 textchr - returns utf8 byte sequences for all unicode code points
	// not uint so to get utf codepoints > 2^63 must provide negative ints
	// not providing implicit constructor from var to uint due to getting ambigious conversions
	// since int and uint are parallel priority in c++ implicit conversions
	var textchr(const int num) const; // UTF8
	var str(const int num) const;
	var space() const;

	// STRING INFO
	//////////////

	// bool match(const var& matchstr,const var& options DEFAULTNULL) const;
	var match(const var& matchstr, const var& options DEFAULTNULL) const;
	var seq() const;     // ASCII
	var textseq() const; // TEXT
	var dcount(const var& substrx) const;
	var count(const var& substrx) const;
#ifndef SWIGPERL
	// swig-perl chokes on this one character version with "SWIG_AsVal_char not defined" so skip
	// it for now (can use slow var& version) do something similar to the python config in
	// exodus.i
	var count(const char charx) const;
#endif
	var length() const;
	var len() const;
	const char* data() const;
	bool isnum() const;

	//static member for speed on std strings
	static int localeAwareCompare(const std::string& str1, const std::string& str2);
	//int localeAwareCompare(const std::string& str2) const;

	// STRING MUTATORS
	//////////////////

	// all return var& and are not const)

	var& converter(const var& oldchars, const var& newchars);
	var& swapper(const var& whatstr, const var& withstr);
	var& replacer(const var& regexstr, const var& replacementstr,
		      const var& options DEFAULTNULL);
	var& splicer(const int start1, const int length, const var& str);
	var& splicer(const int start1, const var& str);
	var& quoter();
	var& squoter();
	var& unquoter();
	var& ucaser();     // utf8
	var& lcaser();     // utf8
	var& tcaser();     // utf8
	var& fcaser();     // utf8
	var& normalizer(); // utf8
	var& inverter();   // utf8
	var& trimmer(const char* trimchar DEFAULTSPACE);
	var& trimmerf(const char* trimchar DEFAULTSPACE);
	var& trimmerb(const char* trimchar DEFAULTSPACE);
	var& trimmer(const var& trimchar);
	var& trimmer(const var& trimchar, const var& options);
	var& trimmerf(const var& trimchar);
	var& trimmerb(const var& trimchar);
	var& fieldstorer(const var& sepchar, const int fieldno, const int nfields,
			 const var& replacement);
	var& substrer(const int startindex);
	var& substrer(const int startindex, const int length);

	var& lowerer();
	var& raiser();
	var& cropper();

	// SAME ON TEMPORARIES
	//////////////////////

	var& convert(const var& oldchars, const var& newchars) &&;
	var& swap(const var& whatstr, const var& withstr) &&;
	var& replace(const var& regexstr, const var& replacementstr,
		      const var& options DEFAULTNULL) &&;
	var& splice(const int start1, const int length, const var& str) &&;
	var& splice(const int start1, const var& str) &&;
	var& quote() &&;
	var& squote() &&;
	var& unquote() &&;
	var& ucase() &&;     // utf8
	var& lcase() &&;     // utf8
	var& tcase() &&;     // utf8
	var& fcase() &&;     // utf8
	var& normalize() &&; // utf8
	var& invert() &&;   // utf8
	var& trim(const char* trimchar DEFAULTSPACE) &&;
	var& trimf(const char* trimchar DEFAULTSPACE) &&;
	var& trimb(const char* trimchar DEFAULTSPACE) &&;
	var& trim(const var& trimchar) &&;
	var& trim(const var& trimchar, const var& options) &&;
	var& trimf(const var& trimchar) &&;
	var& trimb(const var& trimchar) &&;
	var& fieldstore(const var& sepchar, const int fieldno, const int nfields,
			 const var& replacement) &&;
	var& substr(const int startindex) &&;
	var& substr(const int startindex, const int length) &&;

	var& lower() &&;
	var& raise() &&;
	var& crop() &&;

	// SAME BUT NON-MUTATING
	////////////////////////

	// all are const

	ND var convert(const var& oldchars, const var& newchars) const&;
	ND var swap(const var& whatstr, const var& withstr) const&;
	ND var replace(const var& regexstr, const var& replacementstr,
		    const var& options DEFAULTNULL) const&;
	ND var splice(const int start1, const int length, const var& str) const&;
	ND var splice(const int start1, const var& str) const&;
	ND var quote() const&;
	ND var squote() const&;
	ND var unquote() const&;
	ND var ucase() const&;	// utf8
	ND var lcase() const&;	// utf8
	ND var tcase() const&;	// utf8
	ND var fcase() const&;	// utf8
	ND var normalize() const&;	// utf8
	ND var invert() const&;	// utf8
	ND var trim(const char* trimchar DEFAULTSPACE) const&;
	ND var trimf(const char* trimchar DEFAULTSPACE) const&;
	ND var trimb(const char* trimchar DEFAULTSPACE) const&;
	ND var trim(const var& trimchar) const&;
	ND var trim(const var& trimchar, const var& options) const&;
	ND var trimf(const var& trimchar) const&;
	ND var trimb(const var& trimchar) const&;
	ND var fieldstore(const var& sepchar, const int fieldno, const int nfields,
		       const var& replacement) const&;

	ND var lower() const&;
	ND var raise() const&;
	ND var crop() const&;

	// OTHER STRING ACCESS
	//////////////////////

	var hash(const unsigned long long modulus = 0) const;
	ND var unique() const;

	// CONVERT TO DIM (returns a dim)
	// see also dim.split()
	dim split() const;

	// STRING EXTRACTION varx[x,y] -> varx.substr(start,length)

	// NOTE char=byte ... NOT utf-8 code point
	// NOTE 1 based indexing. byte 1 = first byte as per mv conventions for all indexing (except
	// offset in osbread) NOTE start byte may be negative to count backwards -1=last byte

	// v1 - returns bytes from some char number up to the end of the string
	// equivalent to substr(x) from javascript except it is 1 based
	var substr(const int startindex) const&;

	// v2 - returns a given number of bytes starting from some byte
	// both start and length can be negative
	// negative length extracts characters up to the starting byte IN REVERSE
	// 'abcde'.substr(4,-3) -> 'dcb'
	var substr(const int startindex, const int length) const&;

	// v3 - returns bytes from some byte number upto the first of a given list of bytes
	// this is something like std::string::find_first_of but doesnt return the delimiter found
	var substr(const int startindex, const var& delimiterchars, int& endindex) const;

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
	// does NOT remove anything from the source string var remove(var& startindex, var&
	// delimiterno) const;
	var substr2(var& startstopindex, var& delimiterno) const;

	var index(const var& substr, const int occurrenceno = 1) const;
	var index2(const var& substr, const int startchar1 = 1) const;
	var field(const var& substrx, const int fieldnx, const int nfieldsx = 1) const;
	// version that treats fieldn -1 as the last field, -2 the penultimate field etc. - TODO
	// should probably make field() do this
	var field2(const var& substrx, const int fieldnx, const int nfieldsx = 1) const;

	// I/O CONVERSION
	/////////////////

	var oconv(const char* convstr) const;
	var oconv(const var& convstr) const;
	var iconv(const char* convstr) const;
	var iconv(const var& convstr) const;

	var from_codepage(const var& codepage) const;
	var to_codepage(const var& codepage) const;

	// CLASSIC MV STRING FUNCTIONS
	/////////////////////////////

	// this function hardly occurs anywhere in exodus code and should probably be renamed to
	// something better it was called replace() in pick/arev but we are now using "replace()" to
	// change substrings using regex (similar to the old pick swap function) its mutator function
	// is .r()
	ND var pickreplace(const int fieldno, const int valueno, const int subvalueno,
			const var& replacement) const;
	ND var pickreplace(const int fieldno, const int valueno, const var& replacement) const;
	ND var pickreplace(const int fieldno, const var& replacement) const;

	// cf mutator inserter()
	ND var insert(const int fieldno, const int valueno, const int subvalueno,
		   const var& insertion) const&;
	ND var insert(const int fieldno, const int valueno, const var& insertion) const&;
	ND var insert(const int fieldno, const var& insertion) const&;

	/// remove() was delete() in pick/arev
	// var erase(const int fieldno, const int valueno=0, const int subvalueno=0) const;
	ND var remove(const int fieldno, const int valueno = 0, const int subvalueno = 0) const;

	//.a(...) stands for .attribute(...) or extract(...)
	// pick/revelation
	// xxx=yyy<10>";
	// becomes c++
	// xxx=yyy.a(10);
	ND var a(const int fieldno, const int valueno = 0, const int subvalueno = 0) const;
	ND var extract(const int fieldno, const int valueno = 0, const int subvalueno = 0) const;

	// SAME AS ABOVE ON TEMPORARIES
	///////////////////////////////

	var& insert(const int fieldno, const int valueno, const int subvalueno,
		   const var& insertion) &&;
	var& insert(const int fieldno, const int valueno, const var& insertion) &&;
	var& insert(const int fieldno, const var& insertion) &&;

	// MV STRING FILTERS
	////////////////////

	ND var sum(const var& sepchar) const;
	ND var sum() const;

	// binary ops + - * / : on mv strings 10]20]30
	// e.g. var("10]20]30").mv("+","2]3]4")
	// result is "12]23]34"
	ND var mv(const char* opcode, const var& var2) const;

	// MV STRING MUTATORS
	////////////////////

	// mutable versions update and return source
	// r stands for "replacer" abbreviated due to high incidience in code
	// pick/revelation
	// xyz<10>="abc";
	// becomes c++
	//  xyz.r(10,"abc");

	// r() is short for replacer() since it is probably the most common var function after a()
	var& r(const int fieldno, const int valueno, const int subvalueno, const var& replacement);
	var& r(const int fieldno, const int valueno, const var& replacement);
	var& r(const int fieldno, const var& replacement);

	var& inserter(const int fieldno, const int valueno, const int subvalueno,
		      const var& insertion);
	var& inserter(const int fieldno, const int valueno, const var& insertion);
	var& inserter(const int fieldno, const var& insertion);

	// var& eraser(const int fieldno, const int valueno=0, const int subvalueno=0);
	var& remover(const int fieldno, const int valueno = 0, const int subvalueno = 0);
	//-er version could be extract and erase in one go
	// var& extracter(int fieldno,int valueno=0,int subvalueno=0) const;

	// MV STRING LOCATORS
	////////////////////

	// should these be like extract, replace, insert, delete
	// locate(fieldno, valueno, subvalueno,target,setting,by DEFAULTNULL)
	bool locate(const var& target) const;
	bool locate(const var& target, var& setting) const;
	bool locate(const var& target, var& setting, const int fieldno,
		    const int valueno = 0) const;

	bool locateusing(const var& usingchar, const var& target, var& setting,
			 const int fieldno = 0, const int valueno = 0,
			 const int subvalueno = 0) const;
	bool locateusing(const var& usingchar, const var& target) const;

	// locateby without fieldno or valueno arguments uses character VM
	bool locateby(const char* ordercode, const var& target, var& setting) const;
	bool locateby(const var& ordercode, const var& target, var& setting) const;
	// locateby with fieldno=0 uses character FM
	bool locateby(const char* ordercode, const var& target, var& setting, const int fieldno,
		      const int valueno = 0) const;
	bool locateby(const var& ordercode, const var& target, var& setting, const int fieldno,
		      const int valueno = 0) const;

	// locatebyusing
	bool locatebyusing(const char* ordercode, const char* usingchar, const var& target,
			   var& setting, const int fieldno = 0, const int valueno = 0) const;
	bool locatebyusing(const var& ordercode, const var& usingchar, const var& target,
			   var& setting, const int fieldno = 0, const int valueno = 0) const;

	// DATABASE MANAGEMENT
	/////////////////////

	bool createdb(const var& dbname) const;
	bool copydb(const var& from_dbname, const var& to_dbname) const;
	bool deletedb(const var& dbname) const;

	bool createfile(const var& filename) const;
	bool renamefile(const var& filename, const var& newfilename) const;
	bool deletefile(const var& filename) const;
	bool clearfile(const var& filename) const;
	var listfiles() const;

	bool createindex(const var& fieldname, const var& dictfile DEFAULTNULL) const;
	bool deleteindex(const var& fieldname) const;
	var listindexes(const var& filename DEFAULTNULL, const var& fieldname DEFAULTNULL) const;

	bool sqlexec(const var& sqlcmd) const;
	bool sqlexec(const var& sqlcmd, var& response) const;

	// bool selftest() const;
	var version() const;

	var getlasterror() const;

	// DATABASE ACCESS
	/////////////////

	bool connect(const var& conninfo DEFAULTNULL);
	bool disconnect();
	int getdefaultconnectionid() const;
	bool setdefaultconnectionid() const;

	// var() is a db connection or default connection
	bool begintrans() const;
	bool rollbacktrans() const;
	bool committrans() const;
	bool statustrans() const;
	void clearcache() const;

	var reccount(const var& filename DEFAULTNULL) const;
	var flushindex(const var& filename DEFAULTNULL) const;

	bool open(const var& dbfilename, const var& connection DEFAULTNULL);
	void close();

	// 1=ok, 0=failed, ""=already locked
	var lock(const var& key) const;
	// void unlock(const var& key) const;
	// void unlockall() const;
	bool unlock(const var& key) const;
	bool unlockall() const;

	bool read(const var& filehandle, const var& key);
	bool reado(const var& filehandle, const var& key);
	bool readv(const var& filehandle, const var& key, const int fieldno);
	bool write(const var& filehandle, const var& key) const;
	bool writev(const var& filehandle, const var& key, const int fieldno) const;
	bool deleterecord(const var& key) const;
	bool updaterecord(const var& filehandle, const var& key) const;
	bool insertrecord(const var& filehandle, const var& key) const;

	// MvEnvironment function now to allow access to RECORD ID DICT etc. and call external
	// functions
	// var calculate() const;

	var xlate(const var& filename, const var& fieldno, const char* mode) const;
	var xlate(const var& filename, const var& fieldno, const var& mode) const;

	// DATABASE SORT/SELECT
	//////////////////////

	bool select(const var& sortselectclause DEFAULTNULL);
	void clearselect();

	bool savelist(const var& listname);
	bool getlist(const var& listname);
	bool makelist(const var& listname, const var& keys);
	bool deletelist(const var& listname) const;
	bool formlist(const var& keys, const var& fieldno=0);

	bool saveselect(const var& filename);

	bool hasnext() const;
	bool readnext(var& key);
	bool readnext(var& key, var& valueno);

	bool selectrecord(const var& sortselectclause DEFAULTNULL);
	bool readnextrecord(var& record, var& key);
	bool readnextrecord(var& record, var& key, var& valueno);

	// OS FILE SYSTEM
	/////////////////

	bool osopen() const;
	bool osopen(const var& filename, const var& locale DEFAULTNULL) const;
	bool osbread(const var& osfilevar, var& offset, const int length);
	bool osbread(const var& osfilevar, const var& offset, const int length);
	bool osbwrite(const var& osfilevar, var& offset) const;
	bool osbwrite(const var& osfilevar, const var& offset) const;
	void osclose() const;
	bool osread(const var& osfilename, const var& codepage DEFAULTNULL);
	bool oswrite(const var& osfilename, const var& codepage DEFAULTNULL) const;
	bool osdelete() const;
	bool osdelete(const var& osfilename) const;
	bool osrename(const var& newosdir_or_filename) const;
	bool oscopy(const var& to_osfilename) const;
	var oslist(const var& path DEFAULTDOT, const var& wildcard DEFAULTNULL,
		   const int mode = 0) const;
	var oslistf(const var& path DEFAULTDOT, const var& wildcard DEFAULTNULL) const;
	var oslistd(const var& path DEFAULTDOT, const var& wildcard DEFAULTNULL) const;
	var osfile() const;
	var osdir() const;
	bool osmkdir() const;
	bool osrmdir(bool evenifnotempty = false) const;
	// TODO check for threadsafe
	var oscwd() const;
	bool oscwd(const var& newpath) const;
	void osflush() const;

	// TODO add performance enhancing char* argumented versions of many os functions
	// to avoid unnecessary conversion to and from var format
	// same again but this time allowing native strings without needing automatic conversion of
	// var->char* this is to only to avoid convertion too and from var but will usage of hard
	// coded filenames etc really be in fast loops and performance related? perhaps only provide
	bool osread(const char* osfilename, const var& codepage DEFAULTNULL);

	// OS SHELL/ENVIRONMENT
	///////////////////////

	bool suspend() const;
	bool osshell() const;
	bool osshellread(const var& oscmd);
	bool osshellwrite(const var& oscmd) const;
	var ostempdirname() const;
	var ostempfilename() const;

	bool osgetenv(const var& name);
	bool ossetenv(const var& name) const;

	// EXECUTE/PERFORM CONTROL
	/////////////////////////

	// done in mvprogram now since they need to pass mvenvironment
	// var perform() const;
	// var execute() const;
	// chain should be similar to one of the above?
	// var chain() const;

	void stop(const var& text DEFAULTNULL) const;
	void abort(const var& text DEFAULTNULL) const;
	void abortall(const var& text DEFAULTNULL) const;

	var debug() const;
	var debug(const var&) const;

	var logoff() const;

	// DATA MEMBERS (all private)
	//////////////

      private:
	// 1. not using pimpl idiom in order to maximise performance
	// 2. all mutable because asking for a string can create it from an integer and vice versa
	// 3. warning C4251: xxx needs to have dll-interface to be used by clients of class yyy
#ifdef _MSC_VER
#pragma warning(disable : 4251)
#endif
	mutable std::string var_str;
#ifdef _MSC_VER
#pragma warning(4 : 4251)
#endif
	mutable mvint_t var_int;
	mutable double var_dbl;
	// initialise type last
	mutable VARTYP var_typ;

	// PRIVATE MEMBER FUNCTIONS
	//////////////////////////

      private:
	void createString() const;

	void setlasterror(const var& msg) const;
	void setlasterror() const;

	bool cursorexists() const;
	bool selectx(const var& fieldnames, const var& sortselectclause);

	// retrieves cid from *this, or uses default connection, or autoconnect with default
	// connection string On return *this contains connection ID and type VARTYP_NANSTR_DBCONN
	int getconnectionid_ordefault() const;
	int getconnectionid() const;

	// finds connection of this variable:
	// if this is not filename SQLOPENED variable, returns thread default connection or attempts
	// a default connect()
	// returning a void pointer in order to not have to include postgres headers in mv.h
	// will have to be cast to (PGconn *)
	void* connection() const;

	// gets lock_table, associated with connection, associated with this object
	void* get_lock_table() const;

	var build_conn_info(const var& conninfo) const;

	var getdictexpression(const var& mainfilename, const var& filename, const var& dictfilename,
			      const var& dictfile, const var& fieldname, var& joins, var& froms,
			      var& selects, var& ismv,
			      bool forsort_or_select_or_index = false) const;

	// TODO check if can speed up by returning reference to converted self like MC
	// left/right justification
	var oconv_LRC(const var& format) const;
	// text justification
	var oconv_T(const var& format) const;
	// date
	var oconv_D(const char* conversion) const;
	// time
	var oconv_MT(const char* conversion) const;
	// decimal
	var oconv_MD(const char* conversion) const;
	// character replacement
	var& oconv_MR(const char* conversion);
	// hex
	var oconv_HEX(const int ioratio) const;

	// faster primitive arguments
	var iconv_D(const char* conversion) const;
	// var iconv_MT(const char* conversion) const;
	var iconv_MT() const;
	var iconv_MD(const char* conversion) const;
	var iconv_HEX(const int ioratio) const;

	const std::string to_path_string() const;
	const std::string to_cmd_string() const;

	//var& localeAwareChangeCase(const int lowerupper);

	std::fstream* osopenx(const var& osfilename, const var& locale) const;

	friend class dim;
	friend class var_iter;

	bool THIS_IS_DBCONN() const { return ((this->var_typ & VARTYP_DBCONN) != VARTYP_UNA); }
	bool THIS_IS_OSFILE() const { return ((this->var_typ & VARTYP_OSFILE) != VARTYP_UNA); }
	// bool THIS_IS_DBCONN() const	{ return this->var_typ & VARTYP_DBCONN; }
	// bool THIS_IS_OSFILE() const	{ return this->var_typ & VARTYP_OSFILE; }

}; // of class "var"

DLL_PUBLIC ND bool MVeq(const var& var1, const var& var2);
DLL_PUBLIC ND bool MVlt(const var& var1, const var& var2);

DLL_PUBLIC ND bool MVlt(const var& var1, const int int2);
DLL_PUBLIC ND bool MVlt(const int int1, const var& var2);

//== and !=
DLL_PUBLIC ND bool operator==(const var& var1, const var& var2);
DLL_PUBLIC ND bool operator==(const var& var1, const char* char2);
DLL_PUBLIC ND bool operator==(const var& var1, const int int2);
DLL_PUBLIC ND bool operator==(const var& var1, const double double2);
DLL_PUBLIC ND bool operator==(const var& var1, const bool bool2);
DLL_PUBLIC ND bool operator==(const char* char1, const var& var2);
DLL_PUBLIC ND bool operator==(const int int1, const var& var2);
DLL_PUBLIC ND bool operator==(const double double1, const var& var2);
DLL_PUBLIC ND bool operator==(const bool bool1, const var& var2);

DLL_PUBLIC ND bool operator!=(const var& var1, const var& var2);
DLL_PUBLIC ND bool operator!=(const var& var1, const char* char2);
DLL_PUBLIC ND bool operator!=(const var& var1, const int int2);
DLL_PUBLIC ND bool operator!=(const var& var1, const double double2);
DLL_PUBLIC ND bool operator!=(const var& var1, const bool bool2);
DLL_PUBLIC ND bool operator!=(const char* char1, const var& var2);
DLL_PUBLIC ND bool operator!=(const int int1, const var& var2);
DLL_PUBLIC ND bool operator!=(const double double1, const var& var2);
DLL_PUBLIC ND bool operator!=(const bool bool1, const var& var2);

//< <= > >=
DLL_PUBLIC ND bool operator<(const var& var1, const var& var2);
DLL_PUBLIC ND bool operator<(const var& var1, const char* char2);
DLL_PUBLIC ND bool operator<(const var& var1, const int int2);
DLL_PUBLIC ND bool operator<(const var& var1, const double double2);
DLL_PUBLIC ND bool operator<(const char* char1, const var& var2);
DLL_PUBLIC ND bool operator<(const int int1, const var& var2);
DLL_PUBLIC ND bool operator<(const double double1, const var& var2);

DLL_PUBLIC ND bool operator>=(const var& var1, const var& var2);
DLL_PUBLIC ND bool operator>=(const var& var1, const char* char2);
DLL_PUBLIC ND bool operator>=(const var& var1, const int int2);
DLL_PUBLIC ND bool operator>=(const var& var1, const double double2);
DLL_PUBLIC ND bool operator>=(const char* char1, const var& var2);
DLL_PUBLIC ND bool operator>=(const int int1, const var& var2);
DLL_PUBLIC ND bool operator>=(const double double1, const var& var2);

DLL_PUBLIC ND bool operator>(const var& var1, const var& var2);
DLL_PUBLIC ND bool operator>(const var& var1, const char* char2);
DLL_PUBLIC ND bool operator>(const var& var1, const int int2);
DLL_PUBLIC ND bool operator>(const var& var1, const double double2);
DLL_PUBLIC ND bool operator>(const char* char1, const var& var2);
DLL_PUBLIC ND bool operator>(const int int1, const var& var2);
DLL_PUBLIC ND bool operator>(const double double1, const var& var2);

DLL_PUBLIC ND bool operator<=(const var& var1, const var& var2);
DLL_PUBLIC ND bool operator<=(const var& var1, const char* char2);
DLL_PUBLIC ND bool operator<=(const var& var1, const int int2);
DLL_PUBLIC ND bool operator<=(const var& var1, const double double2);
DLL_PUBLIC ND bool operator<=(const char* char1, const var& var2);
DLL_PUBLIC ND bool operator<=(const int int1, const var& var2);
DLL_PUBLIC ND bool operator<=(const double double1, const var& var2);

//+var
DLL_PUBLIC var operator+(const var& var1);

//-var
DLL_PUBLIC var operator-(const var& var1);

//! var
DLL_PUBLIC ND bool operator!(const var& var1);

DLL_PUBLIC var MVadd(const var& var1, const var& var2);

DLL_PUBLIC var MVmul(const var& var1, const var& var2);

DLL_PUBLIC var MVdiv(const var& var1, const var& var2);

#ifndef SWIG
DLL_PUBLIC double exodusmodulus(const double v1, const double v2);
#endif

DLL_PUBLIC var MVmod(const var& var1, const var& var2);

// var^var reassign logical xor to be std::string concatenate!!!
DLL_PUBLIC var MVcat(const var& var1, const var& var2);

//PLUS
DLL_PUBLIC var operator+(const var& var1, const var& var2);
DLL_PUBLIC var operator+(const var& var1, const char* char2);
DLL_PUBLIC var operator+(const var& var1, const int int2);
DLL_PUBLIC var operator+(const var& var1, const double double2);
//DLL_PUBLIC var operator+(const var& var1, const bool bool1);
DLL_PUBLIC var operator+(const char* char1, const var& var2);
DLL_PUBLIC var operator+(const int int1, const var& var2);
DLL_PUBLIC var operator+(const double double1, const var& var2);
//DLL_PUBLIC var operator+(const bool bool1, const var& var2);
//rvalues
DLL_PUBLIC var operator+(var&& var1, const var& var2);
DLL_PUBLIC var operator+(var&& var1, const char* char2);
DLL_PUBLIC var operator+(var&& var1, const int int2);
DLL_PUBLIC var operator+(var&& var1, const double double2);

//MINUS
DLL_PUBLIC var operator-(const var& var1, const var& var2);
DLL_PUBLIC var operator-(const var& var1, const char* char2);
DLL_PUBLIC var operator-(const var& var1, const int int2);
DLL_PUBLIC var operator-(const var& var1, const double double2);
DLL_PUBLIC var operator-(const char* char1, const var& var2);
DLL_PUBLIC var operator-(const int int1, const var& var2);
DLL_PUBLIC var operator-(const double double1, const var& var2);
//rvalues
DLL_PUBLIC var operator-(var&& var1, const var& var2);
DLL_PUBLIC var operator-(var&& var1, const char* char2);
DLL_PUBLIC var operator-(var&& var1, const int int2);
DLL_PUBLIC var operator-(var&& var1, const double double2);

//MULTIPLY
DLL_PUBLIC var operator*(const var& var1, const var& var2);
DLL_PUBLIC var operator*(const var& var1, const char* char2);
DLL_PUBLIC var operator*(const var& var1, const int int2);
DLL_PUBLIC var operator*(const var& var1, const double double2);
DLL_PUBLIC var operator*(const char* char1, const var& var2);
DLL_PUBLIC var operator*(const int int1, const var& var2);
DLL_PUBLIC var operator*(const double double1, const var& var2);

//DIVIDE
DLL_PUBLIC var operator/(const var& var1, const var& var2);
DLL_PUBLIC var operator/(const var& var1, const char* char2);
DLL_PUBLIC var operator/(const var& var1, const int int2);
DLL_PUBLIC var operator/(const var& var1, const double double2);
DLL_PUBLIC var operator/(const char* char1, const var& var2);
DLL_PUBLIC var operator/(const int int1, const var& var2);
DLL_PUBLIC var operator/(const double double1, const var& var2);

//MODULO
DLL_PUBLIC var operator%(const var& var1, const var& var2);
DLL_PUBLIC var operator%(const var& var1, const char* char2);
DLL_PUBLIC var operator%(const var& var1, const int int2);
DLL_PUBLIC var operator%(const var& var1, const double double2);
DLL_PUBLIC var operator%(const char* char1, const var& var2);
DLL_PUBLIC var operator%(const int int1, const var& var2);
DLL_PUBLIC var operator%(const double double1, const var& var2);

// NB we should probably NEVER add operator^(var& var1, bool)
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

DLL_PUBLIC var operator^(const var& var1, const var& var2);
DLL_PUBLIC var operator^(const var& var1, const char* char2);
DLL_PUBLIC var operator^(const var& var1, const int int2);
DLL_PUBLIC var operator^(const var& var1, const double double2);
DLL_PUBLIC var operator^(const char* char1, const var& var2);
DLL_PUBLIC var operator^(const int int1, const var& var2);
DLL_PUBLIC var operator^(const double double1, const var& var2);
//rvalues
DLL_PUBLIC var operator^(var&& var1, const var& var2);
DLL_PUBLIC var operator^(var&& var1, const char* char2);
DLL_PUBLIC var operator^(var&& var1, const int int2);
DLL_PUBLIC var operator^(var&& var1, const double double2);

// allow use of wcout<<var
// shouldnt this be disallowed and only output to cout with conversion to utf8 allowed?
#if defined __MINGW32__
DLL_PUBLIC std::ostream& operator<<(std::ostream& o, const var& var1);
#else
DLL_PUBLIC std::ostream& operator<<(std::ostream& o, const var& var1);
#endif
// ostream& operator<< (ostream& o,const var& var1);

//#ifdef false //allow use of cin>>var
// wistream& operator>> (wistream& i,var var1);
//#endif //allow use of cin>>var

class DLL_PUBLIC var_brackets_proxy
{
public:
	std::string& str1;
	mutable	int charindex1;

	//implicit conversion to var
	operator var() const;

	//operator assign a char
	void operator=(char char1);
};

//class dim
class DLL_PUBLIC dim
{

      public:
	dim(int nrows, int ncols = 1);

	bool redim(int nrows, int ncols = 1);

	var join(const var& sepchar = FM_) const;

	// parenthesis operators often come in pairs
	var& operator()(int rowno, int colno = 1);

	// following const version is called if we do () on a dim which was defined as const xx
	var& operator()(int rowno, int colno = 1) const;

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
	// cant be (const var& var1) because seems to cause a problem with var1=var2 in function
	// parameters unfortunately causes problem of passing var by value and thereby unnecessary
	// contruction see also ^= etc
	dim& operator=(const var& sourcevar);
	dim& operator=(const int sourceint);
	dim& operator=(const double sourcedbl);

	// allow default construction for class variables later resized in class methods
	dim();

	// see also var::split
	// return the number of fields
	var split(const var& var1);
	dim& sort(bool reverse = false);

	bool read(const var& filehandle, const var& key);

	bool write(const var& filehandle, const var& key) const;

	// following is implemented on the dim class now
	// dim dimarray2();
	//

	// move constructor
	dim(dim&& sourcedim) noexcept;

      private:
	// Disable copy constructor (why? to stop inefficient copies?)
	// Copy constructor
	dim(const dim& sourcedim);

	dim& init(const var& var1);

	int nrows_, ncols_;
	// NOTE: trying to implement data_ as boost smart array pointer (boost::scoped_array<var>
	// data_;) raises warning: as dim is DLL_PUBLIC, boost library should have DLL interface.
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

}; // of class "dim"

class DLL_PUBLIC var_iter
{
private:

    var* data;
    std::string::size_type index = 0;
    std::string::size_type index2 = std::string::npos;
    var field;

public:

    //CONSTRUCTOR from a var
    var_iter(var& v);

    //check iter != iter (i.e. iter != end()
    bool operator != (var_iter& vi);

    //CONVERSION - conversion to var
    operator var*();

    //INCREMENT
    var_iter operator ++ ();

    //allow free function access to internals of var_iter
    friend var_iter end(var& v);

};

//BEGIN - free function to create an iterator -> begin
DLL_PUBLIC var_iter begin(var& v);

//END - free function to create an interator -> end
DLL_PUBLIC var_iter end(var& v);

// must be after class declaration

inline const var FM = _FM_;
inline const var VM = _VM_;
inline const var SM = _SM_;
inline const var SVM = _SVM_;
inline const var TM = _TM_;
inline const var STM = _STM_;

inline const var IM = _IM_;
inline const var RM = _RM_;
inline const var AM = _AM_;

inline const var DQ = _DQ_;
inline const var SQ = _SQ_;

#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__
inline const var SLASH = "\\";
inline const char SLASH_ = '\\';
#define SLASH_IS_BACKSLASH true
#else
inline const var SLASH = "/";
inline const char SLASH_ = '/';
#define SLASH_IS_BACKSLASH false
#endif

// being global const means that ucase() and lcase()
// can only be fixed for ASCII
// perhaps make a version of ucase/lcase that
// receives an MvEnvironment in the parameters.
// CF LOWERCASE which is
inline const var LOWERCASE_ = "abcdefghijklmnopqrstuvwxyz";
inline const var UPPERCASE_ = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

#if defined(_WIN64) or defined(_LP64)
inline const var PLATFORM_ = "x64";
#else
inline const var PLATFORM_ = "x86";
#endif

//#ifndef EXO_MV_CPP
//	extern
//	DLL_PUBLIC int DBTRACE;
//#else
//	DLL_PUBLIC int DBTRACE=false;
//#endif

DLL_PUBLIC inline int DBTRACE = false;

// following are all not thread safe since they are at global scope and not const
// perhaps they should be moved on to MvEnvironment mv
// and disallow their use except in ExodusPrograms ie not in plain functions
// dict functions have mv provided by arguments
/*

#ifndef EXO_MV_CPP
extern
DLL_PUBLIC var _STATUS;
#else
DLL_PUBLIC var _STATUS="";
#endif

#ifndef EXO_MV_CPP
extern
DLL_PUBLIC exodus::var EXECPATH;
#else
DLL_PUBLIC exodus::var EXECPATH="";
#endif

//FM separated words or quoted phrases from command line. quote marks are retained.
//trailing options in () or {} are stripped off and available in OPTIONS
#ifndef EXO_MV_CPP
extern
DLL_PUBLIC exodus::var COMMAND;
#else
DLL_PUBLIC exodus::var COMMAND="";
#endif

#ifndef EXO_MV_CPP
extern
DLL_PUBLIC exodus::var OPTIONS;
#else
DLL_PUBLIC exodus::var OPTIONS="";
#endif

//see also COMMAND and OPTIONS
#ifndef EXO_MV_CPP
extern
DLL_PUBLIC exodus::var SENTENCE;
#else
DLL_PUBLIC exodus::var SENTENCE="";
#endif
*/

// this is left a global copy for backtrace to get at it
//#ifndef EXO_MV_CPP
//	extern
//	DLL_PUBLIC exodus::var EXECPATH2;
//#else
//	DLL_PUBLIC exodus::var EXECPATH2="";
//#endif
DLL_PUBLIC inline exodus::var EXECPATH2 = "";

//void DLL_PUBLIC output(const var& var1);
//void DLL_PUBLIC outputl(const var& var1 DEFAULTNULL);
//void DLL_PUBLIC outputt(const var& var1 DEFAULTNULL);

//void DLL_PUBLIC errput(const var& var1);
//void DLL_PUBLIC errputl(const var& var1 DEFAULTNULL);

//void DLL_PUBLIC logput(const var& var1);
//void DLL_PUBLIC logputl(const var& var1 DEFAULTNULL);

var DLL_PUBLIC backtrace();

#ifndef SWIG
std::string naturalorder(const std::string& string1);
#endif

int DLL_PUBLIC getenvironmentn();
void DLL_PUBLIC setenvironmentn(const int environmentn);

int DLL_PUBLIC getenvironmentn();
var DLL_PUBLIC getprocessn();
var DLL_PUBLIC getexecpath();

// mv exceptions

// MVStop is similar to MVException
// but doesnt get stack since stop() is called commonly and normally
class DLL_PUBLIC MVStop
{
      public:
	explicit MVStop(const var& var1 DEFAULTNULL);
	var description;
};

// provide a public base exception for all other exceptions so exodus programmers can catch mv
// exceptions generally
class DLL_PUBLIC MVException
{
      public:
	explicit MVException(const var& description);
	var description;
	var stack;
};

// clang-format off

//individual exceptions are made public so exodus programmers can catch specific errors or even stop/abort/debug if they want
class DLL_PUBLIC MVDivideByZero		: public MVException {public: explicit MVDivideByZero         (const var& var1    );};
class DLL_PUBLIC MVNonNumeric		: public MVException {public: explicit MVNonNumeric           (const var& var1    );};
class DLL_PUBLIC MVIntOverflow		: public MVException {public: explicit MVIntOverflow          (const var& var1    );};
class DLL_PUBLIC MVIntUnderflow		: public MVException {public: explicit MVIntUnderflow         (const var& var1    );};
class DLL_PUBLIC MVOutOfMemory		: public MVException {public: explicit MVOutOfMemory          (const var& var1    );};
class DLL_PUBLIC MVUnassigned		: public MVException {public: explicit MVUnassigned           (const var& var1    );};
class DLL_PUBLIC MVUndefined		: public MVException {public: explicit MVUndefined            (const var& var1    );};
class DLL_PUBLIC MVInvalidPointer	: public MVException {public: explicit MVInvalidPointer       (const var& var1    );};
class DLL_PUBLIC MVDBException		: public MVException {public: explicit MVDBException          (const var& var1    );};
class DLL_PUBLIC MVNotImplemented	: public MVException {public: explicit MVNotImplemented       (const var& var1    );};
class DLL_PUBLIC MVDebug		: public MVException {public: explicit MVDebug                (const var& var1 DEFAULTNULL);};
//class DLL_PUBLIC MVStop                 : public MVException {public: explicit MVStop                 (const var& var1 DEFAULTNULL);};
class DLL_PUBLIC MVAbort		: public MVException {public: explicit MVAbort                (const var& var1 DEFAULTNULL);};
class DLL_PUBLIC MVAbortAll		: public MVException {public: explicit MVAbortAll             (const var& var1 DEFAULTNULL);};
class DLL_PUBLIC MVArrayDimensionedZero	: public MVException {public: explicit MVArrayDimensionedZero (                   );};
class DLL_PUBLIC MVArrayIndexOutOfBounds: public MVException {public: explicit MVArrayIndexOutOfBounds(const var& var1    );};
class DLL_PUBLIC MVArrayNotDimensioned	: public MVException {public: explicit MVArrayNotDimensioned  (                   );};

// clang-format on

} // namespace exodus

//#include <exodus/mvlibs.h>

#endif // MV_H
