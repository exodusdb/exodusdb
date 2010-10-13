/*
Copyright (c) 2009 Stephen John Bush

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

#define EXODUS_RELEASE L"10.10"
#define EXODUS_PATCH L"10.10.0"

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
#	define toTstring(item) item.tostring()
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
#	include <exodus/mvimpl.h>
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

static const unsigned int mvtypemask=0xfffffff0;

/* this has been resolved somehow without removing the automatic conversion to int:
Remove automatic conversion to int which takes precedence over the automatic conversion to bool
and therefore causes a non-numeric error if you include a non-numeric value in an if statement like

 aa="xx"
 if (aa) {}

*/

//#include <iostream>
#include <iosfwd>
#include <string>

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

//annoyingly FM as L'\xFE' doesnt work because prevents "AAA" FM "BBB"
//const wchar_t FM=L'\xFE';

//the var versions of the above (without leading or trailing _)
//are defined AFTER the class declaration of "var"

//these macros (with leading and trailing _) are wstring versions of the pick delimiters
#define _IM_ L"\xFF"
#define _RM_ L"\xFF"
//#define _FM_ L"\376"
#define _FM_ L"\xFE"
#define _AM_ L"\xFE"
#define _VM_ L"\xFD"
#define _SM_ L"\xFC"
#define _SVM_ L"\xFC"
#define _TM_ L"\xFB"
#define _STM_ L"\xFA"
#define _SSTM_ L"\xF9"
#define _DQ_ L"\""
#define _SQ_ L"\'"

//these macros (with trailing _) are wchar versions of the pick delimiters
#define IM_ L'\xFF'
#define RM_ L'\xFF'
#define FM_ L'\xFE'
#define AM_ L'\xFE'
#define VM_ L'\xFD'
#define SVM_ L'\xFC'
#define SM_ L'\xFC'
#define TM_ L'\xFB'
#define STM_ L'\xFA'
#define SSTM_ L'\xF9'
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

//TODO ensure locale doesnt produce like 123.456,78
std::wstring intToString(int int1);

class varray;
class var__extractreplace;

//TODO ensure locale doesnt produce like 123.456,78
std::wstring dblToString(double double1);

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
Conversion never applies to this for member functions, and this extends to operators. If the operator was instead example operator + (example, example) in the global namespace, it would compile (or if pass-by-const-ref was used).

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

	std::wstring towstring() const;

	std::string tostring() const;

	//CONSTRUCTORS
	//////////////

	//default ctor to allow plain unassigned "var mv;" syntax
	var();

	//copy ctor
	var(const var& mv1);

	//ctor for wchar_t
	var(const wchar_t char1);

	//ctor for wchar_t*
	var(const wchar_t* cstr1);

	//ctor for block of wchar_t
	var(const wchar_t* cstr1, const size_t int1);

	//ctor for char to create
	MV_CONSTRUCTION_FROM_CHAR_EXPLICIT
	var(const char char1);

	//ctor for char*
	MV_CONSTRUCTION_FROM_CHAR_EXPLICIT
	var(const char* cstr1);

	//ctor for char memory block
	MV_CONSTRUCTION_FROM_CHAR_EXPLICIT
	var(const char* cstr1, const size_t int1);

	//ctor for std::wstring
	var(const std::wstring& str1);

	//ctor for std::string
	var(const std::string& str1);

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
	//something like .utf8() or .tostring().c_char()
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

	//wstring - replicates towstring()
	//would allow the usage of any std::wstring function but may result
	//in a lot of compilation failures due to "ambiguous overload"
	//unfortunately there is no "explicit" keyword as for constructors - coming in C++0X
	//for now prevent this to ensure efficient programming
	//maybe optionally allow them using a compiler macro?
	#ifndef MV_NO_NARROW
		operator std::wstring() const;
	#endif

	//string - replicates tostring()
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
	var__extractreplace operator() (int fieldn, int valuen=0, int subvaluen=0) const;
	var__extractreplace operator() (int fieldn, int valuen=0, int subvaluen=0);
	*/

	//UNARY OPERATORS
	/////////////////

	//=var
	//The assignment operator should always return a reference to *this.
	//cant be (const var& mv1) because seems to cause a problem with var1=var2 in function parameters
	//unfortunately causes problem of passing var by value and thereby unnecessary contruction
	//see also ^= etc
	var& operator =(const var& mv1);

	//=int
	var& operator = (const int int1);

	//=double
	var& operator = (const double double1);

	//=wchar_t
	var& operator = (const wchar_t char2);

	//=wchar_t*
	var& operator = (const wchar_t* char2);

	//=string
	var& operator = (const std::wstring string2);

	//=var
	var& operator ^=(const var& mv1);

	//=int
	var& operator ^= (const int int1);

	//=double
	var& operator ^= (const double double1);

	//=wchar_t
	var& operator ^= (const wchar_t char2);

	//=wchar_t*
	var& operator ^= (const wchar_t* char2);

	//=string
	var& operator ^= (const std::wstring string2);

    /*
    //postfix returning void so cannot be used in expressions (avoid unreadable programs)
	void operator ++ (int);
	void operator -- (int);

    //TODO: since preventing pre and postfix in expressions then force only use of prefix version since it is faster

    //prefix returning void so cannot be used in expressions (avoid unreadable programs)
	void operator ++ ();
	void operator -- ();
    */

    //postfix
	var operator ++ (int);
	var operator -- (int);

    //prefix
	var& operator ++ ();
	var& operator -- ();

    //+=var
	var& operator+= (const var& mv1);

	//-=var
	var& operator-= (const var& mv1);

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

	DLL_PUBLIC friend  var operator + (const var&,const var&);
	DLL_PUBLIC friend  var operator + (const var&,const wchar_t*);
	DLL_PUBLIC friend  var operator + (const var&,const int);
	DLL_PUBLIC friend  var operator + (const var&,const double);
	DLL_PUBLIC friend  var operator + (const wchar_t*,const var&);
	DLL_PUBLIC friend  var operator + (const int,const var&);
	DLL_PUBLIC friend  var operator + (const double,const var&);

	DLL_PUBLIC friend  var operator - (const var&,const var&);
	DLL_PUBLIC friend  var operator - (const var&,const wchar_t*);
	DLL_PUBLIC friend  var operator - (const var&,const int);
	DLL_PUBLIC friend  var operator - (const var&,const double);
	DLL_PUBLIC friend  var operator - (const wchar_t*,const var&);
	DLL_PUBLIC friend  var operator - (const int,const var&);
	DLL_PUBLIC friend  var operator - (const double,const var&);

	DLL_PUBLIC friend  var operator * (const var&,const var&);
	DLL_PUBLIC friend  var operator * (const var&,const wchar_t*);
	DLL_PUBLIC friend  var operator * (const var&,const int);
	DLL_PUBLIC friend  var operator * (const var&,const double);
	DLL_PUBLIC friend  var operator * (const wchar_t*,const var&);
	DLL_PUBLIC friend  var operator * (const int,const var&);
	DLL_PUBLIC friend  var operator * (const double,const var&);

	DLL_PUBLIC friend  var operator / (const var&,const var&);
	DLL_PUBLIC friend  var operator / (const var&,const wchar_t*);
	DLL_PUBLIC friend  var operator / (const var&,const int);
	DLL_PUBLIC friend  var operator / (const var&,const double);
	DLL_PUBLIC friend  var operator / (const wchar_t*,const var&);
	DLL_PUBLIC friend  var operator / (const int,const var&);
	DLL_PUBLIC friend  var operator / (const double,const var&);

	DLL_PUBLIC friend  var operator % (const var&,const var&);
	DLL_PUBLIC friend  var operator % (const var&,const wchar_t*);
	DLL_PUBLIC friend  var operator % (const var&,const int);
	DLL_PUBLIC friend  var operator % (const var&,const double);
	DLL_PUBLIC friend  var operator % (const wchar_t*,const var&);
	DLL_PUBLIC friend  var operator % (const int,const var&);
	DLL_PUBLIC friend  var operator % (const double,const var&);

	DLL_PUBLIC friend  var operator ^ (const var&,const var&);
	DLL_PUBLIC friend  var operator ^ (const var&,const wchar_t*);
	DLL_PUBLIC friend  var operator ^ (const var&,const int);
	DLL_PUBLIC friend  var operator ^ (const var&,const double);
	DLL_PUBLIC friend  var operator ^ (const wchar_t*,const var&);
	DLL_PUBLIC friend  var operator ^ (const int,const var&);
	DLL_PUBLIC friend  var operator ^ (const double,const var&);

	DLL_PUBLIC friend  bool operator < (const var&,const var&);
	DLL_PUBLIC friend  bool operator < (const var&,const wchar_t*);
	DLL_PUBLIC friend  bool operator < (const var&,const int);
	DLL_PUBLIC friend  bool operator < (const var&,const double);
	DLL_PUBLIC friend  bool operator < (const wchar_t*,const var&);
	DLL_PUBLIC friend  bool operator < (const int,const var&);
	DLL_PUBLIC friend  bool operator < (const double,const var&);
//#ifndef MV_NO_NARROW
	DLL_PUBLIC friend  bool operator < (const var&,const char*);
	DLL_PUBLIC friend  bool operator < (const char*,const var&);
//#endif

	DLL_PUBLIC friend  bool operator <=(const var&,const var&);
	DLL_PUBLIC friend  bool operator <=(const var&,const wchar_t*);
	DLL_PUBLIC friend  bool operator <=(const var&,const int);
	DLL_PUBLIC friend  bool operator <=(const var&,const double);
	DLL_PUBLIC friend  bool operator <=(const wchar_t*,const var&);
	DLL_PUBLIC friend  bool operator <=(const int,const var&);
	DLL_PUBLIC friend  bool operator <=(const double,const var&);
//#ifndef MV_NO_NARROW
	DLL_PUBLIC friend  bool operator <= (const var&,const char*);
	DLL_PUBLIC friend  bool operator <= (const char*,const var&);
//#endif

	DLL_PUBLIC friend  bool operator > (const var&,const var&);
	DLL_PUBLIC friend  bool operator > (const var&,const wchar_t*);
	DLL_PUBLIC friend  bool operator > (const var&,const int);
	DLL_PUBLIC friend  bool operator > (const var&,const double);
	DLL_PUBLIC friend  bool operator > (const wchar_t*,const var&);
	DLL_PUBLIC friend  bool operator > (const int,const var&);
	DLL_PUBLIC friend  bool operator > (const double,const var&);
//#ifndef MV_NO_NARROW
	DLL_PUBLIC friend  bool operator > (const var&,const char*);
	DLL_PUBLIC friend  bool operator > (const char*,const var&);
//#endif

	DLL_PUBLIC friend  bool operator >=(const var&,const var&);
	DLL_PUBLIC friend  bool operator >=(const var&,const wchar_t*);
	DLL_PUBLIC friend  bool operator >=(const var&,const int);
	DLL_PUBLIC friend  bool operator >=(const var&,const double);
	DLL_PUBLIC friend  bool operator >=(const wchar_t*,const var&);
	DLL_PUBLIC friend  bool operator >=(const int,const var&);
	DLL_PUBLIC friend  bool operator >=(const double,const var&);
//#ifndef MV_NO_NARROW
	DLL_PUBLIC friend  bool operator >=(const var&,const char*);
	DLL_PUBLIC friend  bool operator >=(const char*,const var&);
//#endif

	DLL_PUBLIC friend  bool operator ==(const var&,const var&);
	DLL_PUBLIC friend  bool operator ==(const var&,const wchar_t*);
	DLL_PUBLIC friend  bool operator ==(const var&,const int);
	DLL_PUBLIC friend  bool operator ==(const var&,const double);
	DLL_PUBLIC friend  bool operator ==(const wchar_t*,const var&);
	DLL_PUBLIC friend  bool operator ==(const int,const var&);
	DLL_PUBLIC friend  bool operator ==(const double,const var&);
//#ifndef MV_NO_NARROW
	DLL_PUBLIC friend  bool operator ==(const var&,const char*);
	DLL_PUBLIC friend  bool operator ==(const char*,const var&);
//#endif

	DLL_PUBLIC friend  bool operator !=(const var&,const var&);
	DLL_PUBLIC friend  bool operator !=(const var&,const wchar_t*);
	DLL_PUBLIC friend  bool operator !=(const var&,const int);
	DLL_PUBLIC friend  bool operator !=(const var&,const double);
	DLL_PUBLIC friend  bool operator !=(const wchar_t*,const var&);
	DLL_PUBLIC friend  bool operator !=(const int,const var&);
	DLL_PUBLIC friend  bool operator !=(const double,const var&);
//#ifndef MV_NO_NARROW
	DLL_PUBLIC friend  bool operator !=(const var&,const char*);
	DLL_PUBLIC friend  bool operator !=(const char*,const var&);
//#endif

	DLL_PUBLIC friend  var operator +(const var&);
	DLL_PUBLIC friend  var operator -(const var&);
	DLL_PUBLIC friend  bool operator !(const var&);

	//OS TIME/DATE
	var date() const;
	var time() const;
	var timedate() const;
	void ossleep(const int milliseconds) const;
	var ostime() const;

	//SYSTEM FILE/DIRECTORY OPERATIONS
	//TODO cache osfilehandles somehow (use var_mvint?)
	bool osopen(const var& osfilename);
	var& osbread(const var& filehandle, const int startoffset, const int length);
	void osbwrite(const var& filehandle, const int startoffset) const;
	void osclose() const;
	bool osread(const var& osfilename);
	bool oswrite(const var& osfilename) const;
	bool osdelete() const;
	bool osdelete(const var& osfilename) const;
	bool osrename(const var& newosfilename) const;
	bool oscopy(const var& to_osfilename) const;
	var oslist(const var& path=L".", const var& wildcard=L"", const int mode=0) const;
	var oslistf(const var& path=L".", const var& wildcard=L"") const;
	var oslistd(const var& path=L".", const var& wildcard=L"") const;
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
	bool osread(const char* osfilename);

	//libraries and subroutines/functions
	bool load(const var& libraryname) const;
	var call(const wchar_t* libraryname, const char* functionname) const;
	var call(const var& libraryname, const var& functionname) const;
	var call(const char* functionname) const;
	var call(const var& functionname) const;
	var call() const;

	//OS PROCESSING
	var suspend() const;
	var osshell() const;
	var osshellread() const;
	var osshellwrite(const var& writestr) const;
	bool osgetenv(const var& name);
	bool ossetenv(const var& name) const;
	void stop(const var& text=L"") const;
	void abort(const var& text=L"") const;
	var perform() const;
	var execute() const;
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
        DLL_PUBLIC friend std::istream& operator >> (std::istream& istream1, var& var1);
	DLL_PUBLIC friend  std::ostream& operator << (std::ostream& ostream1, const var& var1);
#else
        DLL_PUBLIC friend std::wistream& operator >> (std::wistream& wistream1, var& var1);
	DLL_PUBLIC friend  std::wostream& operator << (std::wostream& wostream1, const var& var1);
#endif
	//friend bool operator <<(const var&);

	//VARIABLE CONTROL
	bool assigned() const;
	bool unassigned() const;
	var& transfer(var& mv2);
	var& exchange(var& mv2);
	var addressof() const;
	void clear();
	void clearcommon();
//	var bitand(const var) const;
//	var bitor(const var) const;
//	var bitxor(const var) const;
	var bitnot() const;

	//MATH/BOOLEAN
	var abs() const;
	var pwr(const var& exponent) const;
	var rnd() const;
	void initrnd() const;
	var exponent() const;
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

	//STRING CREATION
	var chr() const;
	var chr(const int num) const;
	var str(const int num) const;
	var space() const;

	//STRING INFO
	bool match(const var& matchstr,const var& options=L"") const;
	var seq() const;
	var dcount(const var& substrx) const;
	var count(const var& substrx) const;
	var length() const;
	var len() const;
	const wchar_t* data() const;
	bool isnum() const;
	bool isalpha() const;

	//STRING MANIPULATIONS (all return var& and are not const)
	var& converter(const var& fromchars,const var& tochars);
	var& swapper(const var& fromstr,const var& tostr,const var& options=L"");
	var& splicer(const int start1,const int length,const var& str);
	var& quoter();
	var& unquoter();
	var& ucaser();
	var& lcaser();
	var& inverter();
	var& trimmer(const wchar_t* trimchar=L" ");
	var& trimmerf(const wchar_t* trimchar=L" ");
	var& trimmerb(const wchar_t* trimchar=L" ");
	var& trimmer(const var trimchar);
	var& trimmerf(const var trimchar);
	var& trimmerb(const var trimchar);
	var& fieldstorer(const var& sepchar,const int fieldn, const int nfields,const var& replacement);
	var& substrer(const int startx);
	var& substrer(const int startx,const int length);

	//STRING FILTERS
	var convert(const var& fromchars,const var& tochars) const;
	var swap(const var& fromstr,const var& tostr,const var& options=L"") const;
	var splice(const int start1,const int length,const var& str) const;
	var quote() const;
	var unquote() const;
	var ucase() const;
	var lcase() const;
	var invert() const;
	var trim(const wchar_t* trimchar=L" ") const;
	var trimf(const wchar_t* trimchar=L" ") const;
	var trimb(const wchar_t* trimchar=L" ") const;
	var trim(const var trimchar) const;
	var trimf(const var trimchar) const;
	var trimb(const var trimchar) const;
	var fieldstore(const var& sepchar,const int fieldn,const int nfields,const var& replacement) const;
	var hash() const;

	//STRING EXTRACTION
	//[x,y]
	//var.s(start,length) substring
	var substr(const int startx) const;
	var substr(const int startx,const int length) const;
	//TODO index needs to be reimplemented as ...
	//var index(const var& substr,const int occurence=1) const;
	var index(const var& substr,const int startchar1=1) const;
	var field(const var& substrx,const int fieldnx,const int nfieldsx=1) const;
	var field2(const var& substrx,const int fieldnx,const int nfieldsx=1) const;

	//I/O CONVERSION
	var oconv(const wchar_t* convstr) const;
	var oconv(const var& convstr) const;
	var iconv(const wchar_t* convstr) const;
	var iconv(const var& convstr) const;

	//STRING FUNCTIONS THAT USE THE HIGH END SEPARATORS
	void matparse(varray& varray1) const;
	//following is implemented on the varray class now
	//varray matunparse();
	//
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

	var replace(const int fieldn,const int valuen,const int subvaluen,const var& replacement) const;
	var extract(const int fieldn,const int valuen=0,const int subvaluen=0) const;
	var insert(const int fieldn,const int valuen,const int subvaluen,const var& insertion) const;
	var erase(const int fieldn, const int valuen=0, const int subvaluen=0) const;

	//mutable versions update and return source
	var& replacer(const int fieldn,const int valuen,const int subvaluen,const var& replacement);
	var& inserter(const int fieldn,const int valuen,const int subvaluen,const var& insertion);
	var& eraser(const int fieldn, const int valuen=0, const int subvaluen=0);
	//-er version could be extract and erase in one go
	//var& extracter(int fieldn,int valuen=0,int subvaluen=0) const;

	//should these be like extract, replace, insert, delete
	//locate(fieldn, valuen, subvaluen,target,setting,by=L"")
	bool locate(const var& target, var& setting, const int fieldn=0,const int valuen=0) const;
	//passing BY as a string for speed
	bool locateby(const var& target, const char* ordercode, var& setting, const int fieldn=0,const int valuen=0) const;
	bool locateby(const var& target, const var& ordercode, var& setting, const int fieldn=0,const int valuen=0) const;
	bool locateusing(const var& target, const var& usingchar, var& setting, const int fieldn=0, const int valuen=0, const int subvaluen=0) const;
	bool locateusing(const var& target, const var& usingchar) const;

	var sum(const var& sepchar=VM_) const;

	//var FILE I/O
	bool connect(const var& connectioninfo=L"");
	bool disconnect();

	bool begin() const;
	bool rollback() const;
	bool end() const;

	bool createfile(const var& filename,const var& options=L"");
	bool deletefile() const;
	bool clearfile() const;
	var listfiles() const;

	bool createindex(const var& fieldname,const var& dictfile=L"") const;
	bool deleteindex(const var& fieldname) const;
	var listindexes(const var& filename) const;

	bool open(const var& dbfilename);
	bool open(const var& dict,const var& dbfilename);
	void close();

	bool select(const var& sortselectclause = L"") const;
	void clearselect() const;
	bool readnext(var& key) const;
	bool readnext(var& key, var& valueno) const;

	bool selectrecord(const var& sortselectclause = L"") const;
	bool readnextrecord(var& key, var& record) const;

	bool lock(const var& key) const;
	void unlock(const var& key) const;
	void unlockall() const;

	bool read(const var& filehandle, const var& key);
	bool readv(const var& filehandle, const var& key, const int fieldn);
	bool write(const var& filehandle,const var& key) const;
	bool writev(const var& filehandle,const var& key,const int fieldn) const;
	bool deleterecord(const var& key) const;

	bool updaterecord(const var& file,const var& key) const;
	bool insertrecord(const var& file,const var& key) const;

	var calculate() const;
	var xlate(const var& filename,const var& fieldno, const wchar_t* mode) const;
	var xlate(const var& filename,const var& fieldno, const var& mode) const;
	bool sqlexec() const;
	bool sqlexec(var& errmsg) const;

	//bool selftest() const;


private:

	//make this direct for speed. var is a library and not intended to change often and the
	//main point of pimpl is to reduce mass recompilation due to changes of header file
	//pimpl* priv;
	//pimpl priv;
	//was in pimpl
	//all mutable because asking for a string can create it from an integer and vice versa
	mutable std::wstring var_mvstr;
	mutable wchar_t var_mvtype;
	mutable mvint_t var_mvint;
	mutable double var_mvdbl;

	void createString() const;

	bool selectx(const var& fieldnames, const var& sortselectclause) const;

	void* connection() const;

	var oconv_LR(const var& format) const;
	var oconv_T(const var& format) const;
	var oconv_D(const wchar_t* conversion) const;
	var oconv_MT(const wchar_t* conversion) const;
	var oconv_MD_MC(const wchar_t* conversion) const;
	var oconv_HEX(const int ioratio) const;

	var iconv_D(const wchar_t* conversion) const;
	var iconv_MT(const wchar_t* conversion) const;
	var iconv_MD_MC(const wchar_t* conversion) const;
	var iconv_HEX(const int ioratio) const;

	//bool locatex(std::wstring locatestring,)
	//locate within extraction
	bool locatex(const std::wstring& target,const char ordercode,const wchar_t usingchar,var& setting, int fieldn=0,int valuen=0,const int subvaluen=0) const;
	//hardcore std::wstring locate function given a section of a std::wstring and all parameters
	bool locateat(const std::wstring& target,size_t start_pos,size_t end_pos,const wchar_t order,const var& usingchar,var& setting)const;

	int localeAwareCompare(const std::wstring& str1, const std::wstring& str2) const;

}; //of class "var"

DLL_PUBLIC bool MVeq(const var& mv1,const var& mv2);
DLL_PUBLIC bool MVlt(const var& mv1,const var& mv2);

DLL_PUBLIC bool MVlt(const var& mv1,const int int2);
DLL_PUBLIC bool MVlt(const int int1,const var& mv2);

//== and !=
DLL_PUBLIC bool operator ==(const var&    mv1    ,const var&    mv2     );
DLL_PUBLIC bool operator ==(const var&    mv1    ,const wchar_t*  char2   );
DLL_PUBLIC bool operator ==(const var&    mv1    ,const int    int2    );
DLL_PUBLIC bool operator ==(const var&    mv1    ,const double double2 );
DLL_PUBLIC bool operator ==(const wchar_t*  char1  ,const var&    mv2     );
DLL_PUBLIC bool operator ==(const int    int1   ,const var&    mv2     );
DLL_PUBLIC bool operator ==(const double double1,const var&    mv2     );

DLL_PUBLIC bool operator !=(const var&    mv1    ,const var&    mv2     );
DLL_PUBLIC bool operator !=(const var&    mv1    ,const wchar_t*  char2   );
DLL_PUBLIC bool operator !=(const var&    mv1    ,const int    int2    );
DLL_PUBLIC bool operator !=(const var&    mv1    ,const double double2 );
DLL_PUBLIC bool operator !=(const wchar_t*  char1  ,const var&    mv2     );
DLL_PUBLIC bool operator !=(const int    int1   ,const var&    mv2     );
DLL_PUBLIC bool operator !=(const double double1,const var&    mv2     );

//< <= > >=
DLL_PUBLIC bool operator < (const var&    mv1    ,const var&    mv2     );
DLL_PUBLIC bool operator < (const var&    mv1    ,const wchar_t*  char2   );
DLL_PUBLIC bool operator < (const var&    mv1    ,const int    int2    );
DLL_PUBLIC bool operator < (const var&    mv1    ,const double double2 );
DLL_PUBLIC bool operator < (const wchar_t*  char1  ,const var&    mv2     );
DLL_PUBLIC bool operator < (const int    int1   ,const var&    mv2     );
DLL_PUBLIC bool operator < (const double double1,const var&    mv2     );

DLL_PUBLIC bool operator >=(const var&    mv1    ,const var&    mv2     );
DLL_PUBLIC bool operator >=(const var&    mv1    ,const wchar_t*  char2   );
DLL_PUBLIC bool operator >=(const var&    mv1    ,const int    int2    );
DLL_PUBLIC bool operator >=(const var&    mv1    ,const double double2 );
DLL_PUBLIC bool operator >=(const wchar_t*  char1  ,const var&    mv2     );
DLL_PUBLIC bool operator >=(const int    int1   ,const var&    mv2     );
DLL_PUBLIC bool operator >=(const double double1,const var&    mv2     );

DLL_PUBLIC bool operator > (const var&    mv1    ,const var&    mv2     );
DLL_PUBLIC bool operator > (const var&    mv1    ,const wchar_t*  char2   );
DLL_PUBLIC bool operator > (const var&    mv1    ,const int    int2    );
DLL_PUBLIC bool operator > (const var&    mv1    ,const double double2 );
DLL_PUBLIC bool operator > (const wchar_t*  char1  ,const var&    mv2     );
DLL_PUBLIC bool operator > (const int    int1   ,const var&    mv2     );
DLL_PUBLIC bool operator > (const double double1,const var&    mv2     );

DLL_PUBLIC bool operator <=(const var&    mv1    ,const var&    mv2     );
DLL_PUBLIC bool operator <=(const var&    mv1    ,const wchar_t*  char2   );
DLL_PUBLIC bool operator <=(const var&    mv1    ,const int    int2    );
DLL_PUBLIC bool operator <=(const var&    mv1    ,const double double2 );
DLL_PUBLIC bool operator <=(const wchar_t*  char1  ,const var&    mv2     );
DLL_PUBLIC bool operator <=(const int    int1   ,const var&    mv2     );
DLL_PUBLIC bool operator <=(const double double1,const var&    mv2     );

//+var
DLL_PUBLIC var operator+(const var& mv1);

//-var
DLL_PUBLIC var operator-(const var& mv1);

//!var
DLL_PUBLIC bool operator!(const var& mv1);

DLL_PUBLIC var MVadd(const var& mv1,const var& mv2);

DLL_PUBLIC var MVmul(const var& mv1,const var& mv2);

DLL_PUBLIC var MVdiv(const var& mv1,const var& mv2);

DLL_PUBLIC double neosysmodulus(const double v1,const double v2);

DLL_PUBLIC var MVmod(const var& mv1,const var& mv2);

//var^var reassign logical xor to be std::wstring concatenate!!!
DLL_PUBLIC var MVcat(const var& mv1,const var& mv2);

DLL_PUBLIC var operator+(const var&    mv1    ,const var&    mv2     );
DLL_PUBLIC var operator+(const var&    mv1    ,const wchar_t*  char2   );
DLL_PUBLIC var operator+(const var&    mv1    ,const int    int2    );
DLL_PUBLIC var operator+(const var&    mv1    ,const double double2 );
DLL_PUBLIC var operator+(const wchar_t*  char1  ,const var&    mv2     );
DLL_PUBLIC var operator+(const int    int1   ,const var&    mv2     );
DLL_PUBLIC var operator+(const double double1,const var&    mv2     );

DLL_PUBLIC var operator-(const var&    mv1    ,const var&    mv2     );
DLL_PUBLIC var operator-(const var&    mv1    ,const wchar_t*  char2   );
DLL_PUBLIC var operator-(const var&    mv1    ,const int    int2    );
DLL_PUBLIC var operator-(const var&    mv1    ,const double double2 );
DLL_PUBLIC var operator-(const wchar_t*  char1  ,const var&    mv2     );
DLL_PUBLIC var operator-(const int    int1   ,const var&    mv2     );
DLL_PUBLIC var operator-(const double double1,const var&    mv2     );

DLL_PUBLIC var operator*(const var&    mv1    ,const var&    mv2     );
DLL_PUBLIC var operator*(const var&    mv1    ,const wchar_t*  char2   );
DLL_PUBLIC var operator*(const var&    mv1    ,const int    int2    );
DLL_PUBLIC var operator*(const var&    mv1    ,const double double2 );
DLL_PUBLIC var operator*(const wchar_t*  char1  ,const var&    mv2     );
DLL_PUBLIC var operator*(const int    int1   ,const var&    mv2     );
DLL_PUBLIC var operator*(const double double1,const var&    mv2     );

DLL_PUBLIC var operator/(const var&    mv1    ,const var&    mv2     );
DLL_PUBLIC var operator/(const var&    mv1    ,const wchar_t*  char2   );
DLL_PUBLIC var operator/(const var&    mv1    ,const int    int2    );
DLL_PUBLIC var operator/(const var&    mv1    ,const double double2 );
DLL_PUBLIC var operator/(const wchar_t*  char1  ,const var&    mv2     );
DLL_PUBLIC var operator/(const int    int1   ,const var&    mv2     );
DLL_PUBLIC var operator/(const double double1,const var&    mv2     );

DLL_PUBLIC var operator%(const var&    mv1    ,const var&    mv2     );
DLL_PUBLIC var operator%(const var&    mv1    ,const wchar_t*  char2   );
DLL_PUBLIC var operator%(const var&    mv1    ,const int    int2    );
DLL_PUBLIC var operator%(const var&    mv1    ,const double double2 );
DLL_PUBLIC var operator%(const wchar_t*  char1  ,const var&    mv2     );
DLL_PUBLIC var operator%(const int    int1   ,const var&    mv2     );
DLL_PUBLIC var operator%(const double double1,const var&    mv2     );

DLL_PUBLIC var operator^(const var&    mv1    ,const var&    mv2     );
DLL_PUBLIC var operator^(const var&    mv1    ,const wchar_t*  char2   );
DLL_PUBLIC var operator^(const var&    mv1    ,const int    int2    );
DLL_PUBLIC var operator^(const var&    mv1    ,const double double2 );
DLL_PUBLIC var operator^(const wchar_t*  char1  ,const var&    mv2     );
DLL_PUBLIC var operator^(const int    int1   ,const var&    mv2     );
DLL_PUBLIC var operator^(const double double1,const var&    mv2     );

//allow use of wcout<<var
//shouldnt this be disallowed and only output to cout with conversion to utf8 allowed?
#if defined __MINGW32__
	DLL_PUBLIC std::ostream& operator << (std::ostream& o,const var& mv1);
#else
	DLL_PUBLIC std::wostream& operator << (std::wostream& o,const var& mv1);
#endif
//ostream& operator << (ostream& o,const var& mv1);

//#ifdef false //allow use of cin>>var
//wistream& operator >> (wistream& i,var mv1);
//#endif //allow use of cin>>var

class DLL_PUBLIC varray
{

public:

	varray(int rows, int cols=1);

	bool resize(int rows, int cols=1);

	var matunparse() const;

	// parenthesis operators often come in pairs
	//following is called REGARDLESS of LHS/RHS
	var& operator() (int row, int col=1);
	//following const version is called if we do () on a varray which was defined as const xx
	//probably will never be used
	var& operator() (int row, int col=1) const;

	varray& init(const var& mv1);

	//destructor to (NOT VIRTUAL to save space since not expected to be a base class)
	//protected to prevent deriving from var since wish to save space and not provide virtual destructor
	//http://www.gotw.ca/publications/mill18.htm
	~varray();

	varray& operator = (const varray& mva1);

	//allow default contruction for class variables later resized in class methods
	varray();

private:

	// Disable copy constructor (why?)
	// Copy constructor
	varray(const varray& m);

	int rows_, cols_;
	var* data_;

}; //of class "varray"

/*
class DLL_PUBLIC var__extractreplace : private var
{

public:

var__extractreplace(in var1, int fieldn, int valuen, int subvaluen)
	:
	_var1(var1),
	_fieldn(fieldn),
	_valuen(valuen),
	_subvaluen(subvaluen)
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
static const var IM = L"\xFF";
static const var RM = L"\xFF";
static const var AM = L"\xFE";
static const var FM = L"\xFE";
static const var VM = L"\xFD";
static const var SM = L"\xFC";
static const var SVM = L"\xFC";
static const var TM = L"\xFB";
static const var STM = L"\xFA";
static const var SSTM = L"\xF9";
static const var DQ = L"\"";
static const var SQ = L"'";

#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__
const var _SLASH = L"\\";
#else
const var _SLASH = L"/";
#endif

//being global const means that ucase() and lcase()
//can only be fixed for ASCII
//perhaps make a version of ucase/lcase that
//receives an MvEnvironment in the parameters.
//CF LOWERCASE which is
const var _LOWER_CASE=L"abcdefghijklmnopqrstuvwxyz";
const var _UPPER_CASE=L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

//this is not thread safe since it is at global scope and should be per thread
#ifndef NEO_MV_CPP
extern
#else
#endif
DLL_PUBLIC
var _STATUS;

#ifndef NEO_MV_CPP
extern
#else
#endif
DLL_PUBLIC
exodus::var _EXECPATH;

//FM separated words or quoted phrases from command line. quote marks are retained.
//trailing options in () or {} are stripped off and available in _OPTIONS
#ifndef NEO_MV_CPP
extern
#else
#endif
DLL_PUBLIC
exodus::var _COMMAND;

#ifndef NEO_MV_CPP
extern
#else
#endif
DLL_PUBLIC
exodus::var _OPTIONS;

//deprecated and replaced by _COMMAND and _OPTIONS
#ifndef NEO_MV_CPP
extern
#else
#endif
DLL_PUBLIC
exodus::var _SENTENCE;

void DLL_PUBLIC output(const var& mv1);
void DLL_PUBLIC outputl(const var& mv1=L"");
void DLL_PUBLIC outputt(const var& mv1=L"");

void DLL_PUBLIC errput(const var& mv1);
void DLL_PUBLIC errputl(const var& mv1=L"");

void DLL_PUBLIC logput(const var& mv1);
void DLL_PUBLIC logputl(const var& mv1=L"");

var DLL_PUBLIC backtrace();

class DLL_PUBLIC MVException
{
public: MVException(const var& description);
		var description;
};

std::string naturalorder(const std::string& string1);

int DLL_PUBLIC getenvironmentn();
void DLL_PUBLIC setenvironmentn(const int environmentn);

int DLL_PUBLIC getenvironmentn();
var DLL_PUBLIC getprocessn();

}//namespace exodus

#include <exodus/mvlibs.h>

#endif //MV_H
