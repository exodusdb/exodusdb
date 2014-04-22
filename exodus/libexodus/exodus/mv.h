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

//prevent swig perl linking errors about win32_abort win32_select win32_connect
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

#ifndef MV_H
#define MV_H 1

#define EXODUS_RELEASE L"11.5"
#define EXODUS_PATCH L"11.5.0"

//if installing with autotools then for latest version of boost and other installation macro
//download the snapshot from here. AX_BOOST_DATE_TIME etc
//http://git.savannah.gnu.org/gitweb/?p=autoconf-archive.git;a=tree;f=m4

//http://stackoverflow.com/questions/538134/exporting-functions-from-a-dll-with-dllexport
//Using dllimport and dllexport in C++ Classes
//http://msdn.microsoft.com/en-us/library/81h27t8c(VS.80).aspx

#ifdef __MINGW32__
#	define NARROW_IO
#	define wcout cout
#	define wostringstream ostringstream
#	define wstringstream stringstream
#	define wcerr cerr
#	define wofstream ofstream
#	define wifstream ifstream
#	define wpath path
#	define wregex regex
#	define wdirectory_iterator directory_iterator
#	define Tstring string
#	define toTstring(item) item.toString()
#else
#	define toTstring(item) item.var_mvstr
#	define Tstring wstring
#endif

#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__
#	ifdef BUILDING_LIBRARY
#		ifdef __GNUC__
#			define DLL_PUBLIC __attribute__((dllexport))
#		else
#			define DLL_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also support this syntax.
#		endif
#	else
#		ifdef __GNUC__
#			define DLL_PUBLIC __attribute__((dllimport))
#		else
#			define DLL_PUBLIC __declspec(dllimport) // Note: actually gcc seems to also support this syntax.
#		endif
#	endif
#	define DLL_LOCAL
#else
#	if __GNUC__ >= 4
		//use g++ -fvisibility=hidden to make all hidden except those marked DLL_PUBLIC ie "default"
#		define DLL_PUBLIC __attribute__ ((visibility("default")))
#		define DLL_LOCAL  __attribute__ ((visibility("hidden")))
#	else
#		define DLL_PUBLIC
#		define DLL_LOCAL
#	endif
#endif

//TODO .rounder(n), .round(n) and round(x,n) functions something like the following
//round .5 up and -.5 down to nearest integer as per pick financial rounding concept
//1234.5678
//n=2 gives 1234.57
//n=0 gives 1235
//n=-2 gives 1200
//storage could be changed to integer if n<=0 or left probably better since likely to be added to other similar ints thereafter
//if (var_mvdbl>=0)
//	var_mvdbl=long long int(var_mvdbl+0.5)
//else
//	var_mvdbl=long long int(var_mvdbl-0.5);

//pimpl forward declaration
//#ifdef _DEBUG
//forced now that pimpl isnt private OR even a pointer!
#if 1
	//IF was private pointer then only required for debugging
	//and couldcan be commented out in production code
#ifdef SWIG
#	include "mvimpl.h"
#else
#	include <exodus/mvimpl.h>
#endif

#else
	class pimpl;
#endif

/*		gcc	msc
was for 32bit (needs revising after int became long long (64bit/8byte even in 32bit impl)
sizeof
char:	1	1
wchar_t:4	2
string:	4	32
wstring:4	32
int:	4	4
double:	8	8
var:	20	48
*/
static const unsigned int mvtypemask=0xffffff80;
//static const unsigned int mvtypemask=0xfffffff0;
/* this has been resolved somehow without removing the automatic conversion to int:
Remove automatic conversion to int which takes precedence over the automatic conversion to bool
and therefore causes a non-numeric error if you include a non-numeric value in an if statement like

 aa="xx"
 if (aa) {}

*/

//#include <iostream>
#include <iosfwd>
#include <string>
#include <vector>

//#include <boost/thread/tss.hpp>

//questionable MS language "extensions" disable some standard C++ keywords ... restore them
//http://members.ozemail.com.au/~geoffch/samples/programming/msvc/cl/options/z$e.htm
//complete list of c/c++ keywords is
//and, and_eq, bitand, bitor, compl, not, not_eq, or, or_eq, xor and xor_eq
#ifdef _MSC_VER
#define and &&
#define not_eq !=
#define not !
#define or ||
#endif

//http://www.viva64.com/content/articles/64-bit-development/?f=20_issues_of_porting_C++_code_on_the_64-bit_platform.html
typedef long long mvint_t;

namespace exodus {

//annoyingly FM as L'\u02FE' doesnt work because prevents "AAA" FM "BBB"
//const wchar_t FM=L'\u02FE';

//the var versions of the above (without leading or trailing _)
//are defined AFTER the class declaration of "var"

//decided to use unicode characters 0x02F8-0x02FF instead of the classic 00F8-00FF which are latin accented characters
//NB only unicode characters 0-07ff fit in 2 bytes in utf8. Therefore we dont use unicode PUA at E000-E8FF
//the eight unicode characters 02F8-02FF are defined but rarely used and dont seem to be very important.
//could also have used the undefined 05F8-05FF characters but risk important characters being defined there later

//would be 256 if RM was character number 255. used in var::remove()
#define LASTDELIMITERCHARNOPLUS1 0x0300

//leading and trailing _ wchar* versions of classic pick delimiters
//_RM_, _RM and RM_ versions (wchar*, char* and wchar respectively)
#define _RM_ L"\u02FF"	//Record Mark
#define _FM_ L"\u02FE"	//Field Mark
#define _VM_ L"\u02FD"	//Value Mark
#define _SM_ L"\u02FC"	//Subvalue Mark
#define _TM_ L"\u02FB"	//Text Mark
#define _STM_ L"\u02FA"	//Subtext Mark
#define _SSTM_ L"\u02F9" //SubSubtext Mark

//aliases for different implementations of multivalue
#define _IM_ _RM_
#define _AM_ _FM_
#define _SVM_ _SM_

#define _DQ_ L"\""
#define _SQ_ L"\'"

//trailing _ wchar versions of classic pick delimiters
//_RM_, _RM and RM_ versions (wchar*, char* and wchar respectively)
#define RM_ L'\u02FF'	//Record Mark
#define FM_ L'\u02FE'	//Field Mark
#define VM_ L'\u02FD'	//Value Mark
#define SM_ L'\u02FC'	//Subvalue Mark
#define TM_ L'\u02FB'	//Text Mark
#define STM_ L'\u02FA'	//Subtext Mark
#define SSTM_ L'\u02F9' //SubSubtext Mark

//aliases for different implementations of multivalue
#define IM_ RM_
#define AM_ FM_
#define SVM_ SM_

#define DQ_ L'\"'
#define SQ_ L'\''

//the argument for utf16 http://www.unicode.org/notes/tn12/ (utf8/32 cannot handle binary because of illegal byte sequences)
//win32/java/icu/python is utf16 but situation is not so clear on unix (where wchar_t is 32bit)
//but see http://std.dkuug.dk/JTC1/SC22/WG14/www/docs/n1040.pdf for <uchar.h>

//default to allow conversion from char to allow writing xx="abc" instead of xx=L"abc" because ease of use comes first for the application programmer
//but dont allow it in the library itself because performance comes first
#ifdef MV_NO_NARROW
	#define MV_CONSTRUCTION_FROM_CHAR_EXPLICIT explicit
#else
	#define MV_CONSTRUCTION_FROM_CHAR_EXPLICIT
#endif

class dim;
class var__extractreplace;
class PFstream;

#ifndef SWIG

//TODO ensure locale doesnt produce like 123.456,78
std::wstring intToString(int int1);

//TODO ensure locale doesnt produce like 123.456,78
std::wstring dblToString(double double1);

#endif

//#define arg const var& - too likely to conflict with things eg in postgres library
//#define call - conflicts with dynamic so/dll load/call

//most help from Thinking in C++ Volume 1 Chapter 12
//http://www.camtp.uni-mb.si/books/Thinking-in-C++/TIC2Vone-distribution/html/Chapter12.html

//could also use http://www.informit.com/articles/article.asp?p=25264&seqNum=1
//which is effectively about makeing objects behave like ordinary variable syntactically
//implementing smartpointers

//explanation 1.
//we need a thread specific environment number so that functions written in C++
//and used in database calculations (via a callback to a different thread).
//Database calculation functions written in c++ should use this environment number
//as an index into arrays or vectors of global data and not use simple thread specific storage
//explanation 2.
//the SELECT statement often needs to sort/select on calculated fields and we provide a way for the database to
//call procedures in the main program using pipes and a separate thread to serve the pipe.
//to enhance programmability the procedures can have the environment of the thread that issued the SELECT statement
//so we have two threads essentially using the same environment but because one is waiting for the
//database to complete the select statement, the other thread can have free access to the same environment
//each main thread gets a supporting thread the first time it does a SELECT
//explanation 3.

/*TODO check if the following guidance is followed in class var
Conversion never applies to this for member functions, and this extends to operators. If the operator was instead example operator+ (example, example) in the global namespace, it would compile (or if pass-by-const-ref was used).

As a result, symmetric operators like + and - are generally implemented as non-members, whereas the compound assignment operators like += and -= are implemented as members (they also change data, meaning they should be members). And, since you want to avoid code duplication, the symmetric operators can be implemented in terms of the compound assignment ones (as in your code example, although convention recommends making the temporary inside the function).
*/

//the destructor is public non-virtual (supposedly to save space)
//TODO check sizeof var with virtual destructor
//since this class has no virtual functions it should still be save to derive from it and do delete()
//http://www.parashift.com/c++-faq-lite/virtual-functions.html#faq-20.7
class DLL_PUBLIC var
{

protected:

public:
	//destructor to (NOT VIRTUAL to save space since not expected to be a base class)
	//protected to prevent deriving from var since wish to save space and not provide virtual destructor
	//http://www.gotw.ca/publications/mill18.htm
	~var();

	//CONVERSIONS
	/////////////

	bool toBool() const;

	//standard c/c++ int() in other words simply take the number to the left of the point. -1.5 becomes -1 and 1.5 becomes 1
	int toInt() const;

	double toDouble() const;

	std::wstring toWString() const;

	std::string toString() const;

	//weird version for perl that outputs "" if undefined
	std::string toString2() const;

	//CONSTRUCTORS
	//////////////

	//default ctor to allow plain unassigned "var mv;" syntax
	var();

	//copy ctor
	var(const var& var1);

	//ctor for wchar_t*
	//place first before wchar_t so SWIG isnt tempted to use char to acquire strings resulting in ONE character strings)
	var(const wchar_t* wcstr1);

	//ctor for wchar_t
#ifndef SWIG
	var(const wchar_t wchar1);
#endif
	//ctor for block of wchar_t
	//to be implemented?
	//var(const wchar_t* cstr1, const size_t int1);

#ifndef SWIGJAVA
	//ctor for char*
	//place first before char so SWIG isnt tempted to use char to acquire strings resulting in ONE character strings)
	MV_CONSTRUCTION_FROM_CHAR_EXPLICIT
	var(const char* cstr1);
#endif

	//ctor for char to create
#ifndef SWIG
	MV_CONSTRUCTION_FROM_CHAR_EXPLICIT
	var(const char char1);
#endif
	//ctor for char memory block
	MV_CONSTRUCTION_FROM_CHAR_EXPLICIT
	var(const char* cstr1, const size_t int1);

	//ctor for std::wstring
	var(const std::wstring& wstr1);

	//swig java duplicates this with var(std::wstring&) above
#if !defined(SWIGJAVA) && !defined(SWIGCSHARP)
	//ctor for std::string
	var(const std::string& str1);
#endif

	//ctor for bool (dont allow this if no constructor for char* otherwise " " is converted to 1 in widechar only compilations
	MV_CONSTRUCTION_FROM_CHAR_EXPLICIT
	var(const bool bool1);

	//ctor for int
	var(const int int1);

	//ctor for long long
	var(const long long longlong1);

	//ctor for double
	var(const double double1);

	//AUTOMATIC CONVERSIONS TO bool, void* and int
	//////////////////////////////////////////////

	//someone recommends not to create more than one automatic converter
	//to avoid the compiler error "ambiguous conversion"
	//explicit keyword only works on conversion TO var not conversion FROM var

	//commented to allow conversion to int
	//if you uncomment this then also uncomment the definition in var.cpp
	//unfortunately int takes precedence over bool somehow (unless both const/not const?)
	//so if x is not numeric "if (x)" gives a non-numeric error.
	//automatic conversion to int would be nice to avoid large numbers of specialised function calls
	//I think like var::extract(int, int, int);

	//automatic conversion to int
	//necessary to allow conversion to int in many functions like extract(x,y,z)

	//if you uncomment this then also uncomment the definition in var.cpp
	//uncommented to allow simpler functions
	//NB const probably determines priority of choice between automatic conversion
	//and C++ uses bool and int conversion in certain cases to convert to int and bool
	//therefore have chosen to make both bool and int "const" since they dont make
	//and changes to the base object.
	operator int() const;
	
	//remove because causes "ambiguous" with -short_wchar on linux
	//operator unsigned int() const;

	//recommended to provide automatic conversion to VOID POINTER instead of direct to bool
	//since void* is auto converted to bool nicely for natural "if (xyz)" syntax
	//and doesnt suffer the "ambiguous bool promotion to int" issue that automatic conversion to bool has
	//why this is working here now is probably because of non-portable compiler characteristics or lack of source examples
	//Note: The boost::shared_ptr class support an implicit conversion to bool which is free from unintended conversion to arithmetic types. 
	// http://www.informit.com/guides/content.aspx?g=cplusplus&seqNum=297
	//necessary to allow var to be used standalone in "if (xxx)" but see mv.h for discussion of using void* instead of bool
	operator void*() const;

//this stops using var as indexes of arrays on msvc since msvc converts bool and int ambiguous to index number
//g++ seems happy to make int higher and unambigous with bool but probably non-standard c++
//msvc (at least) cant do without this since it seems unwilling to convert void* as a bool
//therefore we include it and restrict indexing on ints and not var eg for (int ii=0 .... instead of for (var ii=0 ...
//#ifndef _MSVC
	operator bool() const;
//#endif

	//NB && and || operators are NOT overloaded because var is set to convert to boolean automatically
	//this is fortunate because "shortcircuiting" of && and || doesnt happen when overloaded

	//operator size_t() const;

	//Perhaps should NOT allow automatic convertion to char* since it assumes a conversion to utf8
	//and cannot hold char(0) perhaps and force people to use
	//something like .utf8() or .toString().c_char()
	//This was added to allow the arguments of osread type functions which need cstrings to be cstrings
	//so that calls with fixed filenames etc dont require a conversion to var and back again
	//The other solution would be to declare a parallel set of function with var arguments
	//operator const char*() const;

	//convert to c_str may or may not be necessary in order to do wcout<<var
	//NB not needed because we overide the << operator itself
	//and causes a problem with unwanted conversions to strings
	//maybe should throw an error is 0x00 present in the string
	//allow the use of any cstring function
	//var::operator const wchar_t*();

	//EXPLICIT CONVERSIONS TO
	/////////////////////////

	//wstring - replicates toWString()
	//would allow the usage of any std::wstring function but may result
	//in a lot of compilation failures due to "ambiguous overload"
	//unfortunately there is no "explicit" keyword as for constructors - coming in C++0X
	//for now prevent this to ensure efficient programming
	//maybe optionally allow them using a compiler macro?
	#ifndef MV_NO_NARROW
		operator std::wstring() const;
	#endif

	//string - replicates toString()
	//would allow the usage of any std::string function but may result
	//in a lot of compilation failures due to "ambiguous overload"
	//unfortunately there is no "explicit" keyword as for constructors - coming in C++0X
	//for now prevent this to ensure efficient programming
	#ifndef MV_NO_NARROW
		operator std::string() const;
	#endif

	//allow conversion to wchar_t (takes first wchar_t or wchar_t 0 if zero length string)
	//would allow the usage of any wchar_t function but may result
	//in a lot of compilation failures due to "ambiguous overload"
	//unfortunately there is no "explicit" keyword as for constructors - coming in C++0X
	//operator wchar_t() const;

	/* extract and replace using operator() not implemented yet

	//extract using () int int int
	//instead of
	//xyz=abc.extract(1,2,3);
	//you can do just (sadly no way to use pick/mv angle brackets like "abc<1,2,3>"
	//and [] brackets would only allow one dimension eg abc[1]

	there is a problem that c++ doesnt call different methods depending on LHS/RHS
	the same problem exists for operator[] see http://codepad.org/wZzgpCjA
	An idea is that var::operator() will produce a "specialvar" that will
	perform extract if a var is wanted or a replace if assignment is done
	but it hasnt been established if it is possible to make the specialvar behave like a normal var
	in all situation eg would the following work?
	var1(1,2,3).outputl();
	also the creation and deletion of the temp object may hit performance.
	One alternative would be to use () only for extraction (and [] only for replacement of fields?)

	//dynamic array extract/replace eg: abc=xyz(1,2,3) and xyz(1,2,3)="abc";
	var__extractreplace operator() (int fieldno, int valueno=0, int subvalueno=0) const;
	var__extractreplace operator() (int fieldno, int valueno=0, int subvalueno=0);
	*/
	var operator() (int fieldno, int valueno=0, int subvalueno=0) const;
	//dont declare this so we force use of the above which returns a temporary on the RHS
	//var& operator() (int fieldno, int valueno=0, int subvalueno=0);

	//extract a character first=1 last=-1 etc
	var operator[] (int charno) const;

	//UNARY OPERATORS
	/////////////////

	//=var
	//The assignment operator should always return a reference to *this.
	//cant be (const var& var1) because seems to cause a problem with var1=var2 in function parameters
	//unfortunately causes problem of passing var by value and thereby unnecessary contruction
	//see also ^= etc
	var& operator=(const var& var1);

	//=int
	var& operator= (const int int1);

	//=double
	var& operator= (const double double1);

	//=wchar_t
	var& operator= (const wchar_t char2);

	//=wchar_t*
	var& operator= (const wchar_t* char2);

	//=string
	var& operator= (const std::wstring string2);

	//=var
	var& operator^=(const var& var1);

	//=int
	var& operator^= (const int int1);

	//=double
	var& operator^= (const double double1);

	//=wchar_t
	var& operator^= (const wchar_t char2);

	//=wchar_t*
	var& operator^= (const wchar_t* char2);

	//=string
	var& operator^= (const std::wstring string2);

    /*
    //postfix returning void so cannot be used in expressions (avoid unreadable programs)
	void operator++ (int);
	void operator-- (int);

    //TODO: since preventing pre and postfix in expressions then force only use of prefix version since it is faster

    //prefix returning void so cannot be used in expressions (avoid unreadable programs)
	void operator++ ();
	void operator-- ();
    */

    //postfix
	var operator++ (int);
	var operator-- (int);

    //prefix
	var& operator++ ();
	var& operator-- ();

    //+=var
	var& operator+= (const var& var1);

	//-=var
	var& operator-= (const var& var1);

	DLL_PUBLIC friend var MVadd(const var&,const var&);
	DLL_PUBLIC friend var MVsub(const var&,const var&);
	DLL_PUBLIC friend var MVmul(const var&,const var&);
	DLL_PUBLIC friend var MVdiv(const var&,const var&);
	DLL_PUBLIC friend var MVmod(const var&,const var&);
	DLL_PUBLIC friend var MVcat(const var&,const var&);

	DLL_PUBLIC friend bool MVeq(const var&,const var&);
	DLL_PUBLIC friend  bool MVlt(const var&,const var&);
	//following specialisations done for speed of var in for loops
	DLL_PUBLIC friend  bool MVlt(const var&,const int);
	DLL_PUBLIC friend  bool MVlt(const int,const var&);

	DLL_PUBLIC friend  var operator+ (const var&,const var&);
	DLL_PUBLIC friend  var operator+ (const var&,const wchar_t*);
	DLL_PUBLIC friend  var operator+ (const var&,const int);
	DLL_PUBLIC friend  var operator+ (const var&,const double);
	DLL_PUBLIC friend  var operator+ (const var&,const bool);
	DLL_PUBLIC friend  var operator+ (const wchar_t*,const var&);
	DLL_PUBLIC friend  var operator+ (const int,const var&);
	DLL_PUBLIC friend  var operator+ (const double,const var&);
	DLL_PUBLIC friend  var operator+ (const bool,const var&);

	DLL_PUBLIC friend  var operator- (const var&,const var&);
	DLL_PUBLIC friend  var operator- (const var&,const wchar_t*);
	DLL_PUBLIC friend  var operator- (const var&,const int);
	DLL_PUBLIC friend  var operator- (const var&,const double);
	DLL_PUBLIC friend  var operator- (const var&,const bool);
	DLL_PUBLIC friend  var operator- (const wchar_t*,const var&);
	DLL_PUBLIC friend  var operator- (const int,const var&);
	DLL_PUBLIC friend  var operator- (const double,const var&);
	DLL_PUBLIC friend  var operator- (const bool,const var&);

	DLL_PUBLIC friend  var operator* (const var&,const var&);
	DLL_PUBLIC friend  var operator* (const var&,const wchar_t*);
	DLL_PUBLIC friend  var operator* (const var&,const int);
	DLL_PUBLIC friend  var operator* (const var&,const double);
	DLL_PUBLIC friend  var operator* (const var&,const bool);
	DLL_PUBLIC friend  var operator* (const wchar_t*,const var&);
	DLL_PUBLIC friend  var operator* (const int,const var&);
	DLL_PUBLIC friend  var operator* (const double,const var&);
	DLL_PUBLIC friend  var operator* (const bool,const var&);

	DLL_PUBLIC friend  var operator/ (const var&,const var&);
	DLL_PUBLIC friend  var operator/ (const var&,const wchar_t*);
	DLL_PUBLIC friend  var operator/ (const var&,const int);
	DLL_PUBLIC friend  var operator/ (const var&,const double);
	//disallow divide by boolean to prevent possible runtime divide by zero
	//DLL_PUBLIC friend  var operator/ (const var&,const bool);
	DLL_PUBLIC friend  var operator/ (const wchar_t*,const var&);
	DLL_PUBLIC friend  var operator/ (const int,const var&);
	DLL_PUBLIC friend  var operator/ (const double,const var&);
	DLL_PUBLIC friend  var operator/ (const bool,const var&);

	DLL_PUBLIC friend  var operator% (const var&,const var&);
	DLL_PUBLIC friend  var operator% (const var&,const wchar_t*);
	DLL_PUBLIC friend  var operator% (const var&,const int);
	DLL_PUBLIC friend  var operator% (const var&,const double);
	//disallow divide by boolean to prevent possible runtime divide by zero
	//DLL_PUBLIC friend  var operator/ (const var&,const bool);
	DLL_PUBLIC friend  var operator% (const var&,const bool);
	DLL_PUBLIC friend  var operator% (const wchar_t*,const var&);
	DLL_PUBLIC friend  var operator% (const int,const var&);
	DLL_PUBLIC friend  var operator% (const double,const var&);
	DLL_PUBLIC friend  var operator% (const bool,const var&);

	//NB do NOT allow concat with bool or vice versa - find reason why
	DLL_PUBLIC friend  var operator^ (const var&,const var&);
	DLL_PUBLIC friend  var operator^ (const var&,const wchar_t*);
	DLL_PUBLIC friend  var operator^ (const var&,const int);
	DLL_PUBLIC friend  var operator^ (const var&,const double);
	DLL_PUBLIC friend  var operator^ (const wchar_t*,const var&);
	DLL_PUBLIC friend  var operator^ (const int,const var&);
	DLL_PUBLIC friend  var operator^ (const double,const var&);

	DLL_PUBLIC friend  bool operator< (const var&,const var&);
	DLL_PUBLIC friend  bool operator< (const var&,const wchar_t*);
	DLL_PUBLIC friend  bool operator< (const var&,const int);
	DLL_PUBLIC friend  bool operator< (const var&,const double);
	DLL_PUBLIC friend  bool operator< (const wchar_t*,const var&);
	DLL_PUBLIC friend  bool operator< (const int,const var&);
	DLL_PUBLIC friend  bool operator< (const double,const var&);
//#ifndef MV_NO_NARROW
	DLL_PUBLIC friend  bool operator< (const var&,const char*);
	DLL_PUBLIC friend  bool operator< (const char*,const var&);
//#endif

	DLL_PUBLIC friend  bool operator<= (const var&,const var&);
	DLL_PUBLIC friend  bool operator<= (const var&,const wchar_t*);
	DLL_PUBLIC friend  bool operator<= (const var&,const int);
	DLL_PUBLIC friend  bool operator<= (const var&,const double);
	DLL_PUBLIC friend  bool operator<= (const wchar_t*,const var&);
	DLL_PUBLIC friend  bool operator<= (const int,const var&);
	DLL_PUBLIC friend  bool operator<= (const double,const var&);
//#ifndef MV_NO_NARROW
	DLL_PUBLIC friend  bool operator<= (const var&,const char*);
	DLL_PUBLIC friend  bool operator<= (const char*,const var&);
//#endif

	DLL_PUBLIC friend  bool operator> (const var&,const var&);
	DLL_PUBLIC friend  bool operator> (const var&,const wchar_t*);
	DLL_PUBLIC friend  bool operator> (const var&,const int);
	DLL_PUBLIC friend  bool operator> (const var&,const double);
	DLL_PUBLIC friend  bool operator> (const wchar_t*,const var&);
	DLL_PUBLIC friend  bool operator> (const int,const var&);
	DLL_PUBLIC friend  bool operator> (const double,const var&);
//#ifndef MV_NO_NARROW
	DLL_PUBLIC friend  bool operator> (const var&,const char*);
	DLL_PUBLIC friend  bool operator> (const char*,const var&);
//#endif

	DLL_PUBLIC friend  bool operator>= (const var&,const var&);
	DLL_PUBLIC friend  bool operator>= (const var&,const wchar_t*);
	DLL_PUBLIC friend  bool operator>= (const var&,const int);
	DLL_PUBLIC friend  bool operator>= (const var&,const double);
	DLL_PUBLIC friend  bool operator>= (const wchar_t*,const var&);
	DLL_PUBLIC friend  bool operator>= (const int,const var&);
	DLL_PUBLIC friend  bool operator>= (const double,const var&);
//#ifndef MV_NO_NARROW
	DLL_PUBLIC friend  bool operator>= (const var&,const char*);
	DLL_PUBLIC friend  bool operator>= (const char*,const var&);
//#endif

	DLL_PUBLIC friend  bool operator== (const var&,const var&);
	DLL_PUBLIC friend  bool operator== (const var&,const wchar_t*);
	DLL_PUBLIC friend  bool operator== (const var&,const int);
	DLL_PUBLIC friend  bool operator== (const var&,const double);
	DLL_PUBLIC friend  bool operator== (const var&,const bool);
	DLL_PUBLIC friend  bool operator== (const wchar_t*,const var&);
	DLL_PUBLIC friend  bool operator== (const int,const var&);
	DLL_PUBLIC friend  bool operator== (const double,const var&);
	DLL_PUBLIC friend  bool operator== (const bool,const var&);
//#ifndef MV_NO_NARROW
	DLL_PUBLIC friend  bool operator== (const var&,const char*);
	DLL_PUBLIC friend  bool operator== (const char*,const var&);
//#endif

	DLL_PUBLIC friend  bool operator!= (const var&,const var&);
	DLL_PUBLIC friend  bool operator!= (const var&,const wchar_t*);
	DLL_PUBLIC friend  bool operator!= (const var&,const int);
	DLL_PUBLIC friend  bool operator!= (const var&,const double);
	DLL_PUBLIC friend  bool operator!= (const var&,const bool);
	DLL_PUBLIC friend  bool operator!= (const wchar_t*,const var&);
	DLL_PUBLIC friend  bool operator!= (const int,const var&);
	DLL_PUBLIC friend  bool operator!= (const double,const var&);
	DLL_PUBLIC friend  bool operator!= (const bool,const var&);
//#ifndef MV_NO_NARROW
	DLL_PUBLIC friend  bool operator!= (const var&,const char*);
	DLL_PUBLIC friend  bool operator!= (const char*,const var&);
//#endif

	//unary operators +var -var !var
	DLL_PUBLIC friend  var operator+ (const var&);
	DLL_PUBLIC friend  var operator- (const var&);
	DLL_PUBLIC friend  bool operator! (const var&);

	//OS TIME/DATE
	var date() const;
	var time() const;
	var timedate() const;
	void ossleep(const int milliseconds) const;
	var ostime() const;
#ifdef SWIG
#	define DEFAULTNULL
#	define DEFAULTDOT
#	define DEFAULTSPACE
#	define DEFAULTVM
#else
#	define DEFAULTNULL =L""
#	define DEFAULTDOT =L"."
#	define DEFAULTSPACE =L" "
#	define DEFAULTVM =VM_
#endif
	//SYSTEM FILE/DIRECTORY OPERATIONS
	//TODO cache osfilehandles somehow (use var_mvint?)
	bool osopen() const;
	bool osopen(const var& filename, const var& locale DEFAULTNULL) const;
	var& osbread(const var& osfilevar, var& startoffset, const int length);
	bool osbwrite(const var& osfilevar, var& startoffset) const;
	void osclose() const;
	bool osread(const var& osfilename, const var& locale DEFAULTNULL);
	bool oswrite(const var& osfilename, const var& locale DEFAULTNULL) const;
	bool osdelete() const;
	bool osdelete(const var& osfilename) const;
	bool osrename(const var& newosdir_or_filename) const;
	bool oscopy(const var& to_osfilename) const;
	var oslist(const var& path DEFAULTDOT, const var& wildcard DEFAULTNULL, const int mode=0) const;
	var oslistf(const var& path DEFAULTDOT, const var& wildcard DEFAULTNULL) const;
	var oslistd(const var& path DEFAULTDOT, const var& wildcard DEFAULTNULL) const;
	var osfile() const;
	var osdir() const;
	bool osmkdir() const;
	bool osrmdir(bool evenifnotempty=false) const;
	//TODO check for threadsafe
	var oscwd() const;
	var oscwd(const var& path) const;
	void osflush() const;

	//TODO add performance enhancing char* argumented versions of many os functions
	//to avoid unnecessary conversion to and from var format
	//same again but this time allowing native strings without needing automatic conversion of var->char*
	//this is to only to avoid convertion too and from var
	//but will usage of hard coded filenames etc really be in fast loops
	//and performance related? perhaps only provide
	bool osread(const char* osfilename, const var& locale DEFAULTNULL);

	//libraries and subroutines/functions
/* replaced by  #include funcname.h
	bool load(const var& libraryname) const;
	var call(const wchar_t* libraryname, const char* functionname) const;
	var call(const var& libraryname, const var& functionname) const;
	var call(const char* functionname) const;
	var call(const var& functionname) const;
	var call() const;
*/
	//OS PROCESSING
	var suspend() const;
	var osshell() const;
	var osshellread() const;
	var osshellwrite(const var& writestr) const;
	bool osgetenv(const var& name);
	bool ossetenv(const var& name) const;
	void stop(const var& text DEFAULTNULL) const;

	void abort(const var& text DEFAULTNULL) const;
//	var perform() const;
//	var execute() const;
	var chain() const;
	var logoff() const;
	var debug() const;
	var debug(const var&) const;
	void breakon() const;
	void breakoff() const;

	//STANDARD OUTPUT
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

	//CURSOR
	var at(const int columnorcode) const;
	var at(const int column,const int row) const;
	var getcursor() const;
	void setcursor() const;
	var getprompt() const;
	void setprompt() const;

	//STANDARD INPUT
	bool input();
	bool input(const var& prompt, const int int1=0);
	bool eof() const;

#if defined __MINGW32__
	DLL_PUBLIC friend std::istream& operator>> (std::istream& istream1, var& var1);
	DLL_PUBLIC friend  std::ostream& operator<< (std::ostream& ostream1, const var& var1);
#else
	DLL_PUBLIC friend std::wistream& operator>> (std::wistream& wistream1, var& var1);
	DLL_PUBLIC friend  std::wostream& operator<< (std::wostream& wostream1, const var& var1);
#endif
	//friend bool operator<<(const var&);

	//VARIABLE CONTROL
	bool assigned() const;
	bool unassigned() const;
	var& transfer(var& var2);
	const var& exchange(const var& var2) const;
	var& clone(var& var2);
/*no implemented yet
	var addressof() const;
	void clear();
	void clearcommon();
//	var bitand(const var) const;
//	var bitor(const var) const;
//	var bitxor(const var) const;
	var bitnot() const;
*/
	//MATH/BOOLEAN
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
	//integer() represents pick int() because int() is reserved word in c/c++
	//Note that integer like pick int() is the same as floor()
	//whereas the usual c/c++ int() simply take the next integer nearest 0 (ie cuts of any fractional decimal places)
	//to get the usual c/c++ effect use toInt() (although toInt() returns an int instead of a var like normal exodus functions)
	var integer() const;
	var floor() const;
	var round(const int ndecimals=0) const;

	//LOCALE
	bool setxlocale() const;
	var& getxlocale();

	//STRING CREATION
//	var chr() const;
	var chr(const int num) const;
	var str(const int num) const;
	var space() const;

	//STRING INFO
	bool match(const var& matchstr,const var& options DEFAULTNULL) const;
	var seq() const;
	var dcount(const var& substrx) const;
	var count(const var& substrx) const;
#ifndef SWIGPERL
//swig-perl chokes on this one character version with "SWIG_AsVal_wchar_t not defined" so skip it for now (can use slow var& version)
//do something similar to the python config in exodus.i
	var count(const wchar_t charx) const;
#endif
	var length() const;
	var len() const;
	const wchar_t* data() const;
	bool isnum() const;
	bool isnum_old() const;
//	bool isalpha() const;

	//STRING MANIPULATIONS (all return var& and are not const)
	var& converter(const var& oldchars,const var& newchars);
	var& swapper(const var& oldstr,const var& newstr,const var& options DEFAULTNULL);
	var& splicer(const int start1,const int length,const var& str);
	var& splicer(const int start1,const var& str);
	var& quoter();
	var& squoter();
	var& unquoter();
	var& ucaser();
	var& lcaser();
	//var& inverter();
	var& trimmer(const wchar_t* trimchar DEFAULTSPACE);
	var& trimmerf(const wchar_t* trimchar DEFAULTSPACE);
	var& trimmerb(const wchar_t* trimchar DEFAULTSPACE);
	var& trimmer(const var trimchar);
	var& trimmerf(const var trimchar);
	var& trimmerb(const var trimchar);
	var& fieldstorer(const var& sepchar,const int fieldno, const int nfields,const var& replacement);
	var& substrer(const int startx);
	var& substrer(const int startx,const int length);

	//STRING FILTERS
	var convert(const var& oldchars,const var& newchars) const;
	var swap(const var& oldstr,const var& newstr,const var& options DEFAULTNULL) const;
	var splice(const int start1,const int length,const var& str) const;
	var splice(const int start1,const var& str) const;
	var quote() const;
	var squote() const;
	var unquote() const;
	var ucase() const;
	var lcase() const;
	//var invert() const;
	var trim(const wchar_t* trimchar DEFAULTSPACE) const;
	var trimf(const wchar_t* trimchar DEFAULTSPACE) const;
	var trimb(const wchar_t* trimchar DEFAULTSPACE) const;
	var trim(const var trimchar) const;
	var trimf(const var trimchar) const;
	var trimb(const var trimchar) const;
	var fieldstore(const var& sepchar,const int fieldno,const int nfields,const var& replacement) const;
//	var hash() const;
	var unique() const;

	//STRING EXTRACTION
	//[x,y]
	//var.s(start,length) substring
	var substr(const int startx) const;
	var substr(const int startx,const int length) const;
	var index(const var& substr,const int occurrenceno=1) const;
	var index2(const var& substr,const int startchar1=1) const;
	var field(const var& substrx,const int fieldnx,const int nfieldsx=1) const;
	var field2(const var& substrx,const int fieldnx,const int nfieldsx=1) const;

	//I/O CONVERSION
	var oconv(const wchar_t* convstr) const;
	var oconv(const var& convstr) const;
	var iconv(const wchar_t* convstr) const;
	var iconv(const var& convstr) const;

	//STRING FUNCTIONS THAT USE THE HIGH END SEPARATORS
	//return a substr from any starting character
	//stops at the next character FF-F8, updating the starting character
	//and indicates the delimiter found (1-8) or 0 if off end of string
	//NB parameters are not const
	//?return the starting position of a substr starting at character nn or 0 if not found

	var lower() const;
	var raise() const;
	var crop() const;

	//mutable versions update and return source
	var& lowerer();
	var& raiser();
	var& cropper();

	var remove(var& startx, var& delimiterno) const;

	//mutable versions update and return source
	//-er version to update too?
	//var& remover(var& startx,var& length) const;

	var replace(const int fieldno,const int valueno,const int subvalueno,const var& replacement) const;
	var replace(const int fieldno,const int valueno,const var& replacement) const;
	var replace(const int fieldno,const var& replacement) const;

	var insert(const int fieldno,const int valueno,const int subvalueno,const var& insertion) const;
//to be implemented?
//	var insert(const int fieldno,const int valueno,const var& insertion) const;
//	var insert(const int fieldno,const var& insertion) const;

	var erase(const int fieldno, const int valueno=0, const int subvalueno=0) const;

	var a(const int fieldno,const int valueno=0,const int subvalueno=0) const;
	var extract(const int fieldno,const int valueno=0,const int subvalueno=0) const;

	//mutable versions update and return source
	var& r(const int fieldno,const int valueno,const int subvalueno,const var& replacement);
	var& r(const int fieldno,const int valueno,const var& replacement);
	var& r(const int fieldno,const var& replacement);

	var& inserter(const int fieldno,const int valueno,const int subvalueno,const var& insertion);
	var& inserter(const int fieldno,const int valueno,const var& insertion);
	var& inserter(const int fieldno,const var& insertion);

	var& eraser(const int fieldno, const int valueno=0, const int subvalueno=0);
	//-er version could be extract and erase in one go
	//var& extracter(int fieldno,int valueno=0,int subvalueno=0) const;

	//should these be like extract, replace, insert, delete
	//locate(fieldno, valueno, subvalueno,target,setting,by DEFAULTNULL)
	bool locate(const var& target, var& setting, const int fieldno=0,const int valueno=0) const;
	//passing "by" as a string for speed
	bool locateby(const var& target, const char* ordercode, var& setting, const int fieldno=0,const int valueno=0) const;
	bool locateby(const var& target, const var& ordercode, var& setting, const int fieldno=0,const int valueno=0) const;
	bool locateusing(const var& target, const var& usingchar, var& setting, const int fieldno=0, const int valueno=0, const int subvalueno=0) const;
	bool locateusing(const var& target, const var& usingchar) const;
	var sum(const var& sepchar) const;
	var sum() const;

	//var FILE I/O

	bool connect(const var& conninfo DEFAULTNULL);
	bool disconnect();
	bool setdefaultconnection();

	bool begintrans() const;
	bool rollbacktrans() const;
	bool committrans() const;
	bool statustrans() const;

	bool createdb(const var& dbname) const;
	bool deletedb(const var& dbname) const;
	bool createdb(const var& dbname, var& errmsg) const;
	bool deletedb(const var& dbname, var& errmsg) const;

	bool createfile(const var& filename) const;
	bool deletefile(const var& filename) const;
	bool clearfile(const var& filename) const;
	var listfiles() const;

	bool createindex(const var& fieldname,const var& dictfile DEFAULTNULL) const;
	bool deleteindex(const var& fieldname) const;
	var listindexes(const var& filename DEFAULTNULL) const;

	bool open(const var& dbfilename, const var& dbconnection DEFAULTNULL);
	void close();

	bool select(const var& sortselectclause DEFAULTNULL) const;
	void clearselect() const;
	bool readnext(var& key) const;
	bool readnext(var& key, var& valueno) const;

	bool selectrecord(const var& sortselectclause DEFAULTNULL) const;
	bool readnextrecord(var& record, var& key) const;

	//1=ok, 0=failed, ""=already locked
	var lock(const var& key) const;
	void unlock(const var& key) const;
	void unlockall() const;

	bool read(const var& filehandle, const var& key);
	bool readv(const var& filehandle, const var& key, const int fieldno);
	bool write(const var& filehandle,const var& key) const;
	bool writev(const var& filehandle,const var& key,const int fieldno) const;
	bool deleterecord(const var& key) const;
	bool updaterecord(const var& file,const var& key) const;
	bool insertrecord(const var& file,const var& key) const;

	var getlasterror() const;

	/* MvEnvironment function now
	var calculate() const;
*/
	var xlate(const var& filename,const var& fieldno, const wchar_t* mode) const;
	var xlate(const var& filename,const var& fieldno, const var& mode) const;

	bool sqlexec(const var& SqlToExecute) const;	// this: filename, parm1: SQL to execute
//	bool sqlexec(int connection_id = 0) const;
// outdated: 20101203:	bool sqlexec(var& errmsg, int connection_id) const;	// this: SQL to execute
	bool sqlexec(const var& sqlcmd, var& errmsg) const;

	//bool selftest() const;
	var version() const;

private:

	//make this direct for speed. var is a library and not intended to change often and the
	//main point of pimpl is to reduce mass recompilation due to changes of header file
	//pimpl* priv;
	//pimpl priv;
	//was in pimpl
	//all mutable because asking for a string can create it from an integer and vice versa
	//warning C4251: xxx needs to have dll-interface to be used by clients of class yyy
#ifdef _MSC_VER
	#pragma warning( disable: 4251 )
#endif
	mutable std::wstring var_mvstr;
#ifdef _MSC_VER
	#pragma warning( 4: 4251 )
#endif
	mutable mvint_t var_mvint;
	mutable double var_mvdbl;
	//initialise type last
	mutable wchar_t var_mvtyp;

	void createString() const;

	void setlasterror(const var& msg) const;
	void setlasterror() const;

	bool selectx(const var& fieldnames, const var& sortselectclause) const;

	// retrieves cid from *this, or uses default connection, or autoconnect with default connection string
	// On return *this contains connection ID and type pimpl::MVTYPE_NANSTR_DBCONN
	int connection_id() const;

	// finds connection of this variable:
	// if this is not filename SQLOPENED variable, returns thread default connection or attempts a default connect()
	void* connection() const;

	// gets lock_table, associated with connection, associated with this object
	void* get_lock_table() const;

	var build_conn_info(const var& conninfo) const;

	var getdictexpression(const var& mainfilename, const var& filename, const var& dictfilename, const var& dictfile, const var& fieldname, var& joins, bool forsort_or_select_or_index=false) const;

	//TODO check if can speed up by returning reference to converted self like MC
	var oconv_LR(const var& format) const;
	var oconv_T(const var& format) const;
	var oconv_D(const wchar_t* conversion) const;
	var oconv_MT(const wchar_t* conversion) const;
	var oconv_MD(const wchar_t* conversion) const;
	var& oconv_MC(const wchar_t* conversion);
	var oconv_HEX(const int ioratio) const;

	var iconv_D(const wchar_t* conversion) const;
	//var iconv_MT(const wchar_t* conversion) const;
	var iconv_MT() const;
	var iconv_MD(const wchar_t* conversion) const;
	var iconv_HEX(const int ioratio) const;

	//bool locatex(std::wstring locatestring,)
	//locate within extraction
	bool locatex(const std::wstring& target,const char ordercode,const wchar_t usingchar,var& setting, int fieldno=0,int valueno=0,const int subvalueno=0) const;
	//hardcore std::wstring locate function given a section of a std::wstring and all parameters
	bool locateat(const std::wstring& target,size_t start_pos,size_t end_pos,const wchar_t order,const var& usingchar,var& setting)const;

	int localeAwareCompare(const std::wstring& str1, const std::wstring& str2) const;
	var& localeAwareChangeCase(const int lowerupper);

	std::wfstream* osopenx(const var& osfilename, const var& locale) const;

	friend class dim;

	bool THIS_IS_DBCONN() const	{ return (this->var_mvtyp & pimpl::MVTYPE_DBCONN) != 0; }
	bool THIS_IS_OSFILE() const	{ return (this->var_mvtyp & pimpl::MVTYPE_OSFILE) != 0; }

}; //of class "var"

DLL_PUBLIC bool MVeq(const var& var1, const var& var2);
DLL_PUBLIC bool MVlt(const var& var1, const var& var2);

DLL_PUBLIC bool MVlt(const var& var1, const int  int2);
DLL_PUBLIC bool MVlt(const int  int1, const var& var2);

//== and !=
DLL_PUBLIC bool operator== (const var&     var1    ,const var&     var2    );
DLL_PUBLIC bool operator== (const var&     var1    ,const wchar_t* char2   );
DLL_PUBLIC bool operator== (const var&     var1    ,const int      int2    );
DLL_PUBLIC bool operator== (const var&     var1    ,const double   double2 );
DLL_PUBLIC bool operator== (const var&     var1    ,const bool     bool2   );
DLL_PUBLIC bool operator== (const wchar_t* char1   ,const var&     var2    );
DLL_PUBLIC bool operator== (const int      int1    ,const var&     var2    );
DLL_PUBLIC bool operator== (const double   double1 ,const var&     var2    );
DLL_PUBLIC bool operator== (const bool     bool1   ,const var&     var2    );

DLL_PUBLIC bool operator!= (const var&     var1    ,const var&     var2    );
DLL_PUBLIC bool operator!= (const var&     var1    ,const wchar_t* char2   );
DLL_PUBLIC bool operator!= (const var&     var1    ,const int      int2    );
DLL_PUBLIC bool operator!= (const var&     var1    ,const double   double2 );
DLL_PUBLIC bool operator!= (const var&     var1    ,const bool     bool2   );
DLL_PUBLIC bool operator!= (const wchar_t* char1   ,const var&     var2    );
DLL_PUBLIC bool operator!= (const int      int1    ,const var&     var2    );
DLL_PUBLIC bool operator!= (const double   double1 ,const var&     var2    );
DLL_PUBLIC bool operator!= (const bool     bool1   ,const var&     var2    );

//< <= > >=
DLL_PUBLIC bool operator<  (const var&     var1    ,const var&     var2    );
DLL_PUBLIC bool operator<  (const var&     var1    ,const wchar_t* char2   );
DLL_PUBLIC bool operator<  (const var&     var1    ,const int      int2    );
DLL_PUBLIC bool operator<  (const var&     var1    ,const double   double2 );
DLL_PUBLIC bool operator<  (const wchar_t* char1   ,const var&     var2    );
DLL_PUBLIC bool operator<  (const int      int1    ,const var&     var2    );
DLL_PUBLIC bool operator<  (const double   double1 ,const var&     var2    );

DLL_PUBLIC bool operator>= (const var&     var1    ,const var&     var2    );
DLL_PUBLIC bool operator>= (const var&     var1    ,const wchar_t* char2   );
DLL_PUBLIC bool operator>= (const var&     var1    ,const int      int2    );
DLL_PUBLIC bool operator>= (const var&     var1    ,const double   double2 );
DLL_PUBLIC bool operator>= (const wchar_t* char1   ,const var&     var2    );
DLL_PUBLIC bool operator>= (const int      int1    ,const var&     var2    );
DLL_PUBLIC bool operator>= (const double   double1 ,const var&     var2    );

DLL_PUBLIC bool operator>  (const var&     var1    ,const var&     var2    );
DLL_PUBLIC bool operator>  (const var&     var1    ,const wchar_t* char2   );
DLL_PUBLIC bool operator>  (const var&     var1    ,const int      int2    );
DLL_PUBLIC bool operator>  (const var&     var1    ,const double   double2 );
DLL_PUBLIC bool operator>  (const wchar_t* char1   ,const var&     var2    );
DLL_PUBLIC bool operator>  (const int      int1    ,const var&     var2    );
DLL_PUBLIC bool operator>  (const double   double1 ,const var&     var2    );

DLL_PUBLIC bool operator<= (const var&     var1    ,const var&     var2    );
DLL_PUBLIC bool operator<= (const var&     var1    ,const wchar_t* char2   );
DLL_PUBLIC bool operator<= (const var&     var1    ,const int      int2    );
DLL_PUBLIC bool operator<= (const var&     var1    ,const double   double2 );
DLL_PUBLIC bool operator<= (const wchar_t* char1   ,const var&     var2    );
DLL_PUBLIC bool operator<= (const int      int1    ,const var&     var2    );
DLL_PUBLIC bool operator<= (const double   double1 ,const var&     var2    );

//+var
DLL_PUBLIC var operator+ (const var& var1);

//-var
DLL_PUBLIC var operator- (const var& var1);

//!var
DLL_PUBLIC bool operator! (const var& var1);

DLL_PUBLIC var MVadd(const var& var1,const var& var2);

DLL_PUBLIC var MVmul(const var& var1,const var& var2);

DLL_PUBLIC var MVdiv(const var& var1,const var& var2);

#ifndef SWIG
DLL_PUBLIC double neosysmodulus(const double v1,const double v2);
#endif

DLL_PUBLIC var MVmod(const var& var1,const var& var2);

//var^var reassign logical xor to be std::wstring concatenate!!!
DLL_PUBLIC var MVcat(const var& var1,const var& var2);

DLL_PUBLIC var operator+ (const var&    var1    ,const var&    var2     );
DLL_PUBLIC var operator+ (const var&    var1    ,const wchar_t*  char2   );
DLL_PUBLIC var operator+ (const var&    var1    ,const int    int2    );
DLL_PUBLIC var operator+ (const var&    var1    ,const double double2 );
DLL_PUBLIC var operator+ (const var&    var1    ,const bool bool1 );
DLL_PUBLIC var operator+ (const wchar_t*  char1  ,const var&    var2     );
DLL_PUBLIC var operator+ (const int    int1   ,const var&    var2     );
DLL_PUBLIC var operator+ (const double double1,const var&    var2     );
DLL_PUBLIC var operator+ (const bool    bool1 ,const var&    var2     );

DLL_PUBLIC var operator- (const var&    var1    ,const var&    var2     );
DLL_PUBLIC var operator- (const var&    var1    ,const wchar_t*  char2   );
DLL_PUBLIC var operator- (const var&    var1    ,const int    int2    );
DLL_PUBLIC var operator- (const var&    var1    ,const double double2 );
DLL_PUBLIC var operator- (const wchar_t*  char1  ,const var&    var2     );
DLL_PUBLIC var operator- (const int    int1   ,const var&    var2     );
DLL_PUBLIC var operator- (const double double1,const var&    var2     );

DLL_PUBLIC var operator* (const var&    var1    ,const var&    var2     );
DLL_PUBLIC var operator* (const var&    var1    ,const wchar_t*  char2   );
DLL_PUBLIC var operator* (const var&    var1    ,const int    int2    );
DLL_PUBLIC var operator* (const var&    var1    ,const double double2 );
DLL_PUBLIC var operator* (const wchar_t*  char1  ,const var&    var2     );
DLL_PUBLIC var operator* (const int    int1   ,const var&    var2     );
DLL_PUBLIC var operator* (const double double1,const var&    var2     );

DLL_PUBLIC var operator/ (const var&    var1    ,const var&    var2     );
DLL_PUBLIC var operator/ (const var&    var1    ,const wchar_t*  char2   );
DLL_PUBLIC var operator/ (const var&    var1    ,const int    int2    );
DLL_PUBLIC var operator/ (const var&    var1    ,const double double2 );
DLL_PUBLIC var operator/ (const wchar_t*  char1  ,const var&    var2     );
DLL_PUBLIC var operator/ (const int    int1   ,const var&    var2     );
DLL_PUBLIC var operator/ (const double double1,const var&    var2     );

DLL_PUBLIC var operator% (const var&    var1    ,const var&    var2     );
DLL_PUBLIC var operator% (const var&    var1    ,const wchar_t*  char2   );
DLL_PUBLIC var operator% (const var&    var1    ,const int    int2    );
DLL_PUBLIC var operator% (const var&    var1    ,const double double2 );
DLL_PUBLIC var operator% (const wchar_t*  char1  ,const var&    var2     );
DLL_PUBLIC var operator% (const int    int1   ,const var&    var2     );
DLL_PUBLIC var operator% (const double double1,const var&    var2     );

//NB we should probably NEVER add operator^(var& var1, bool)
//this is a trick to avoid a problem that exodus concat operator
//has the wrong precedence versus the logical operators
//a ^ b > c //we should prevent this from compiling because
//exodus will treat this like a^(b>c) which is WRONG
//multivalue basic requires it to be (a^b)>c
//however c++ ^ operator has lower priority than >
//whereas multivalue concatenate operator should have HIGHER priority than >
//and no way to change priority of operators in c++
//therefore to prevent silent non-standard behaviour from causing
//insidious bugs it is better to refuse to compile a request to
//concatenate boolean as "1" or "0" in Exodus
//a refusal to compile can be solved explicitly by putting brackets
//to force compiler to do the right thing multivalue wise
//(a^b)>c
//or, if you do want to concatenate the result of a comparison do this
//a^var(b>c)

DLL_PUBLIC var operator^(const var&    var1    ,const var&    var2     );
DLL_PUBLIC var operator^(const var&    var1    ,const wchar_t*  char2   );
DLL_PUBLIC var operator^(const var&    var1    ,const int    int2    );
DLL_PUBLIC var operator^(const var&    var1    ,const double double2 );
DLL_PUBLIC var operator^(const wchar_t*  char1  ,const var&    var2     );
DLL_PUBLIC var operator^(const int    int1   ,const var&    var2     );
DLL_PUBLIC var operator^(const double double1,const var&    var2     );

//allow use of wcout<<var
//shouldnt this be disallowed and only output to cout with conversion to utf8 allowed?
#if defined __MINGW32__
	DLL_PUBLIC std::ostream& operator<< (std::ostream& o,const var& var1);
#else
	DLL_PUBLIC std::wostream& operator<< (std::wostream& o,const var& var1);
#endif
//ostream& operator<< (ostream& o,const var& var1);

//#ifdef false //allow use of cin>>var
//wistream& operator>> (wistream& i,var var1);
//#endif //allow use of cin>>var

class DLL_PUBLIC dim
{

public:

	dim(int nrows, int ncols=1);

	bool redim(int nrows, int ncols=1);

	var unparse() const;

	// parenthesis operators often come in pairs
	var& operator() (int rowno, int colno=1);

	//following const version is called if we do () on a dim which was defined as const xx
	var& operator() (int rowno, int colno=1) const;

	//Q: why is this commented out?
	//A: we dont want to COPY vars out of an array when using it in rhs expression
	//var operator() (int row, int col=1) const;

	//destructor to (NOT VIRTUAL to save space since not expected to be a base class)
	//protected to prevent deriving from var since wish to save space and not provide virtual destructor
	//http://www.gotw.ca/publications/mill18.htm
	~dim();

	dim& operator= (const dim& dim1);

	//=var
	//The assignment operator should always return a reference to *this.
	//cant be (const var& var1) because seems to cause a problem with var1=var2 in function parameters
	//unfortunately causes problem of passing var by value and thereby unnecessary contruction
	//see also ^= etc
	dim& operator= (const var& var1);
	dim& operator= (const int int1);
	dim& operator= (const double dbl1);

	//allow default construction for class variables later resized in class methods
	dim();

	//return the number of fields
	var parse(const var& var1);

	bool read(const var& filehandle, const var& key);

	bool write(const var& filehandle, const var& key) const;

	//following is implemented on the dim class now
	//dim dimarray2();
	//

private:

	// Disable copy constructor (why?)
	// Copy constructor
	dim(const dim& m);

	dim& init(const var& var1);

	int nrows_, ncols_;
	// NOTE: trying to implement data_ as boost smart array pointer (boost::scoped_array<var> data_;)
	// raises warning: as dim is DLL_PUBLIC, boost library should have DLL interface.
	// Choices: 1) leave memory allocation as is (refuse from scoped_array, or
	// 2) use pimpl metaphor with slight decrease in performance.
	// Constructors of dim are very simple, no exception expected between 'new' and return from constructor
	// As such, choice (1).

	//all redimensioning of this array (eg when copying arrays)
	//seem to be using ::redim() to accomplish redimensioning
	//so only the redim code is dangerous (danger in one place is manageable)
	//we choose NOT to implement 2) above (pimpl) in order
	//to provide exodus programmer greater/easier visibility into dimensiorned arrays when debugging
	//(cannot use boost scoped pointer here because mv.h is required by exodus programmer who should not need boost)
	var* data_;
	bool initialised_;

}; //of class "dim"

/*
class DLL_PUBLIC var__extractreplace : private var
{

public:

var__extractreplace(in var1, int fieldno, int valueno, int subvalueno)
	:
	_var1(var1),
	_fieldn(fieldno),
	_valuen(valueno),
	_subvaluen(subvalueno)
	;

	//destructor to (NOT VIRTUAL to save space since not expected to be a base class)
	//protected to prevent deriving from var since wish to save space and not provide virtual destructor
	//http://www.gotw.ca/publications/mill18.htm
	~var__extractreplace();

	//automatic conversion to var
	//must be on the RHS so do an extract
	operator var() const;

	//assignment
	//must be on the LHS so do the replacement
	var operator= (const var& var1);

private:

	// Disable copy constructor (why?)
	// Copy constructor
	var__extractreplace(const var__extractreplace& m);

	var& _var1
	int _fieldn;
	int _valuen;
	int _subvaluen;

}; //of class var__extractreplace
*/

//must be after class declaration
#ifdef SWIG
//swig cant handle these as wide character L"" for some reason
static const var FM = _FM;
static const var VM = _VM;
static const var SM = _SM;
static const var SVM = _SVM;
static const var TM = _TM;
static const var STM = _STM;
static const var SSTM = _SSTM;

static const var IM = _IM;
static const var RM = _RM;
static const var AM = _AM;

static const var DQ = _DQ;
static const var SQ = _SQ;

#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__
const var SLASH = "\\";
static const char SLASH_ = '\\';
#define SLASH_IS_BACKSLASH true
#else
const var SLASH = "/";
static const char SLASH_ = '/';
#define SLASH_IS_BACKSLASH false
#endif

//being global const means that ucase() and lcase()
//can only be fixed for ASCII
//perhaps make a version of ucase/lcase that
//receives an MvEnvironment in the parameters.
//CF LOWERCASE which is
const var LOWERCASE_="abcdefghijklmnopqrstuvwxyz";
const var UPPERCASE_="ABCDEFGHIJKLMNOPQRSTUVWXYZ";

#if defined(_WIN64) or defined(_LP64)
const var PLATFORM_="x64";
#else
const var PLATFORM_="x86";
#endif

/////
#else
/////

static const var FM = _FM_;
static const var VM = _VM_;
static const var SM = _SM_;
static const var SVM = _SVM_;
static const var TM = _TM_;
static const var STM = _STM_;
static const var SSTM = _SSTM_;

static const var IM = _IM_;
static const var RM = _RM_;
static const var AM = _AM_;

static const var DQ = _DQ_;
static const var SQ = _SQ_;

#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__
const var SLASH = L"\\";
static const char SLASH_ = L'\\';
#define SLASH_IS_BACKSLASH true
#else
const var SLASH = L"/";
static const char SLASH_ = L'/';
#define SLASH_IS_BACKSLASH false
#endif

//being global const means that ucase() and lcase()
//can only be fixed for ASCII
//perhaps make a version of ucase/lcase that
//receives an MvEnvironment in the parameters.
//CF LOWERCASE which is
const var LOWERCASE_=L"abcdefghijklmnopqrstuvwxyz";
const var UPPERCASE_=L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

#if defined(_WIN64) or defined(_LP64)
const var PLATFORM_=L"x64";
#else
const var PLATFORM_=L"x86";
#endif

#endif

#ifndef EXO_MV_CPP
extern
DLL_PUBLIC int DBTRACE;
#else
DLL_PUBLIC int DBTRACE=false;
#endif

//following are all not thread safe since they are at global scope and not const
//perhaps they should be moved on to MvEnvironment mv
//and disallow their use except in ExodusPrograms ie not in plain functions
//dict functions have mv provided by arguments
/*

#ifndef EXO_MV_CPP
extern
DLL_PUBLIC var _STATUS;
#else
DLL_PUBLIC var _STATUS=L"";
#endif

#ifndef EXO_MV_CPP
extern
DLL_PUBLIC exodus::var EXECPATH;
#else
DLL_PUBLIC exodus::var EXECPATH=L"";
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
DLL_PUBLIC exodus::var OPTIONS=L"";
#endif

//see also COMMAND and OPTIONS
#ifndef EXO_MV_CPP
extern
DLL_PUBLIC exodus::var SENTENCE;
#else
DLL_PUBLIC exodus::var SENTENCE=L"";
#endif
*/

//this is left a global copy for backtrace to get at it
#ifndef EXO_MV_CPP
extern
DLL_PUBLIC exodus::var EXECPATH2;
#else
DLL_PUBLIC exodus::var EXECPATH2=L"";
#endif

void DLL_PUBLIC output(const var& var1);
void DLL_PUBLIC outputl(const var& var1 DEFAULTNULL);
void DLL_PUBLIC outputt(const var& var1 DEFAULTNULL);

void DLL_PUBLIC errput(const var& var1);
void DLL_PUBLIC errputl(const var& var1 DEFAULTNULL);

void DLL_PUBLIC logput(const var& var1);
void DLL_PUBLIC logputl(const var& var1 DEFAULTNULL);

var DLL_PUBLIC backtrace();

#ifndef SWIG
std::string naturalorder(const std::string& string1);
#endif

int DLL_PUBLIC getenvironmentn();
void DLL_PUBLIC setenvironmentn(const int environmentn);

int DLL_PUBLIC getenvironmentn();
var DLL_PUBLIC getprocessn();
var DLL_PUBLIC getexecpath();

//mv exceptions

//provide a public base exception for all other exceptions so exodus programmers can catch mv exceptions generally
class DLL_PUBLIC MVException
{
public: MVException(const var& description);
		var description;
};

//individual exceptions are made public so exodus programmers can catch specific errors or even stop/abort/debug if they want
class DLL_PUBLIC MVDivideByZero         : public MVException {public: MVDivideByZero         (const var& var1    );};
class DLL_PUBLIC MVNonNumeric           : public MVException {public: MVNonNumeric           (const var& var1    );};
class DLL_PUBLIC MVOutOfMemory          : public MVException {public: MVOutOfMemory          (const var& var1    );};
class DLL_PUBLIC MVUnassigned           : public MVException {public: MVUnassigned           (const var& var1    );};
class DLL_PUBLIC MVUndefined            : public MVException {public: MVUndefined            (const var& var1    );};
class DLL_PUBLIC MVInvalidPointer       : public MVException {public: MVInvalidPointer       (const var& var1    );};
class DLL_PUBLIC MVDBException          : public MVException {public: MVDBException          (const var& var1    );};
class DLL_PUBLIC MVNotImplemented       : public MVException {public: MVNotImplemented       (const var& var1    );};
class DLL_PUBLIC MVDebug                : public MVException {public: MVDebug                (const var& var1 DEFAULTNULL);};
class DLL_PUBLIC MVStop                 : public MVException {public: MVStop                 (const var& var1 DEFAULTNULL);};
class DLL_PUBLIC MVAbort                : public MVException {public: MVAbort                (const var& var1 DEFAULTNULL);};
class DLL_PUBLIC MVArrayDimensionedZero : public MVException {public: MVArrayDimensionedZero (                   );};
class DLL_PUBLIC MVArrayIndexOutOfBounds: public MVException {public: MVArrayIndexOutOfBounds(const var& var1    );};
class DLL_PUBLIC MVArrayNotDimensioned  : public MVException {public: MVArrayNotDimensioned  (                   );};

}//namespace exodus

//#include <exodus/mvlibs.h>

#endif //MV_H
