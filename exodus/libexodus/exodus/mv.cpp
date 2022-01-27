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

#include <iostream>

// TODO: check that "-" and "-." is NOT numeric

// TODO make mvtype a bit field indicating multiple types present
// or overlap the string/integer/float variables to save space

#include <cassert>
#include <limits>
#include <sstream>
#include <vector>
#include <charconv>//for to_chars

#if __GNUC__ >= 11
#define USE_TO_CHARS
#include <array>
#else

// Use ryu if GNUC < 11 and ryu include available
//ryu            1234.5678 -> "1234.5678" 500ns
//ryu_printf     1234.5678 -> "1234.5678" 800ns
//sstream/printf 1234.5678 -> "1234.5678" 1800ns
#if __has_include(<ryu/ryu.h>)
#define USE_RYU
#include <ryu/ryu.h>
#endif

#endif

#define EXO_MV_CPP	// indicates globals are to be defined (omit extern keyword)
#include <exodus/mv.h>
#include <exodus/mvexceptions.h>

namespace exodus {

// most help from Thinking in C++ Volume 1 Chapter 12
// http://www.camtp.uni-mb.si/books/Thinking-in-C++/TIC2Vone-distribution/html/Chapter12.html

// Also - very clearly written
// Addison Wesley Longman, Inc.
// C++ Primer, Third Edition 1998 Stanley B Lippman
// Chapter 15 Overloaded operators and User-Defined Conversions

// could also use http://www.informit.com/articles/article.asp?p=25264&seqNum=1
// which is effectively about makeing objects behave like ordinary variable syntactically
// implementing smartpointers

// NOTE
// if priv variables are moved directly into the var object then
// then mvtype etc should be initialised in proper initialisers to gain performance in those
// compilers
// that initialise basic types. ie prevent initialisation AND assignment
// currently the default priv object initialisation of mvint to 0 is inefficient since is ALSO
// assigned in most var constructions

//// DESTRUCTOR
//// defined in class for inline/optimisation
///////////////
//var::~var() {
//	//std::cout << "dtor:" << var_str << std::endl;
//
//	// not a pimpl style pointer anymore for speed
//	// delete priv;
//
//	// try to ensure any memory is not later recognises as initialised memory
//	//(exodus tries to detect undefined use of uninitialised objects at runtime - that dumb
//	// compilers allow without warning) this could be removed in production code perhaps set all
//	// unused bits to 1 to ease detection of usage of uninitialised variables (bad c++ syntax
//	// like var x=x+1; set all used bits to 0 to increase chance of detecting unassigned
//	// variables var_typ=(char)0xFFFFFFF0;
//	//var_typ = VARTYP_MASK;
//}

// CONSTRUCTORS
//////////////

//// default constructor to allow definition unassigned "var mv";
//// defined in class for inline/optimisation
//var::var()
//	: var_typ(VARTYP_UNA) {
//	//std::cout << "ctor()" << std::endl;
//
//	// int xyz=3;
//	// WARNING neither initialisers nor constructors are called in the following case !!!
//	// var xxx=xxx.somefunction()
//	// known as "undefined usage of uninitialised variable";
//	// and not even a compiler warning in msvc8 or g++4.1.2
//
//	// so the following test is put everywhere to protect against this type of accidental
//	// programming if (var_typ&VARTYP_MASK) throw MVUndefined("funcname()"); should really
//	// ensure a magic number and not just HOPE for some binary digits above bottom four 0-15
//	// decimal 1111binary this could be removed in production code perhaps
//
//	// debuggCONSTRUCT&&cout<<"CONSTRUCT: var()\n";
//
//	// not a pointer anymore for speed
//	// priv=new pimpl;
//
//	// moved here from pimpl constructor
//	// moved up to initializer
//	// var_typ=VARTYP_UNA;
//}

// copy constructor
var::var(CVR rhs)
	: var_str(rhs.var_str),
	  var_int(rhs.var_int),
	  var_dbl(rhs.var_dbl),
	  var_typ(rhs.var_typ){
		  // use initializers (why?) and only check afterwards if copiedvar was assigned
		  THISIS("var::var(CVR rhs)")
			  ISASSIGNED(rhs)

		  //std::clog << "copy ctor CVR " << rhs.var_str << std::endl;

		  // not a pointer anymore for speed
		  // priv=new pimpl;
	  }

#ifndef INLINE_CONSTRUCTION

	// move constructor
	// var(TVR fromvar) noexcept;  // = default;
	// defined in class for inline/optimisation
	// move constructor
	var::var(TVR rhs) noexcept
		: var_str(std::move(rhs.var_str)),
		var_int(rhs.var_int),
		var_dbl(rhs.var_dbl),
		var_typ(rhs.var_typ) {

		//std::clog << "move ctor TVR noexcept " << rhs.var_str << std::endl;

		// skip this for speed since temporararies are unlikely to be unassigned
		// THISIS("var::var(TVR rhs) noexcept")
		// ISASSIGNED(rhs)
	}

	// constructor for bool
	// defined in class for inline/optimisation
	// just use initializers since cannot fail
	var::var(const bool bool1) noexcept
		: var_int(bool1),
		  var_typ(VARTYP_INT) {}

	// constructor for int
	// defined in class for inline/optimisation
	// just use initializers since cannot fail
	var::var(const int int1) noexcept
		: var_int(int1),
		  var_typ(VARTYP_INT) {}

	// constructor for long long
	// defined in class for inline/optimisation
	// just use initializers since cannot fail
	var::var(const long long longlong1) noexcept
		: var_int(longlong1),
		  var_typ(VARTYP_INT) {}

	// constructor for double
	// defined in class for inline/optimisation
	// just use initializers since cannot fail
	var::var(const double double1) noexcept
		: var_dbl(double1),
		  var_typ(VARTYP_DBL) {}

	// ctor for char
	// defined in class for inline/optimisation
	// use initializers since cannot fail (but could find how to init the char1)
	var::var(const char char1) noexcept
		: var_str(1, char1),
		  var_typ(VARTYP_STR) {}

	// constructor for char*
	// defined in class for inline/optimisation
	// use initializers since cannot fail unless out of memory
	var::var(const char* cstr1)
		: var_str(cstr1),
		  var_typ(VARTYP_STR) {
		//std::cout << "ctor char* :" <<var_str << std::endl;

		// protect against null pointer
		// probably already crashed from var_str initialiser above
		if (cstr1 == 0) {
			// THISIS("var::var(const char* cstr1)")
			throw ("Null pointer in var::var(const char*)");
		}
	}

	// ctor for memory block
	// defined in class for inline/optimisation
	// dont use initialisers and TODO protect against out of memory in expansion to string
	var::var(const char* charstart, const size_t nchars)
		: var_str(charstart, nchars),
		  var_typ(VARTYP_STR) {}

	//in c++20 but not g++ v9.3
	//constexpr var::var(const std::string& str1);

	// constructor for const std::string
	// defined in class for inline/optimisation
	// just use initializers since cannot fail unless out of memory
	var::var(const std::string& str1)
		// this would validate all strings as being UTF8?
		//: var_str(boost::locale::conv::utf_to_utf<char>(str1))
		: var_str(str1),
		  var_typ(VARTYP_STR) {}

	// constructor for temporary std::string
	// defined in class for inline/optimisation
	// just use initializers since cannot fail unless out of memory
	var::var(std::string&& str1) noexcept
		: var_str(std::move(str1)),
		  var_typ(VARTYP_STR) {}

#endif // not INLINE_CONSTRUCTION

// TODO ensure locale doesnt produce like 123.456,78
// see 1997 http://www.cantrip.org/locale.html
std::string dblToString(double double1) {

	//std::cout << abs(double1) << std::endl;
	//std::cout << double(0.000'000'001) << std::endl;
	//std::cout << (std::abs(double1)<double(0.000'000'001)) << std::endl;

	int minus = double1 < 0 ? 1 : 0;

	//Precision on scientific output allows the full precision to obtained
	//regardless of the size of the number (big or small)
	//
	//Precision on fixed output only controls the precision after the decimal point
	//so the precision needs to be huge to cater for very small numbers eg < 10E-20

#ifdef USE_TO_CHARS
	std::array<char, 24> chars;

	auto [ptr, ec] = std::to_chars(chars.data(), chars.data() + chars.size(), double1);
	if (ec != std::errc())
		throw MVNonNumeric("Cannot convert double1 to 24 characters");

	//std::cout << std::string_view
	//	(chars.data(), ptr);              // C++20, uses string_view(first, last)
		//   (str.data(), ptr - str.data()); // C++17, uses string_view(ptr, length)
	std::string s = std::string(chars.data(), ptr - chars.data());
	const std::size_t epos = s.find('e');
	if (epos == std::string::npos)
		return s;

#elif defined(USE_RYU)

	//std::cout << "ryu_printf decimal oconv" << std::endl;

	std::string s;
	s.resize(24);

#define USE_RYU_D2S
#ifdef USE_RYU_D2S	// 500ns using ryu d2s() which always outputs scientific even 0E0 for zero.

	//ryu perfect round trip ASCII->double->ASCII but not perfect for calculated or compiled doubles

	//ryu output (max precision for round tripping ASCII->double->ASCII)
	//always scientific format to variable precision (cannot control precision)
	//followed by conversion to fixed format below
	//
	//But this only work perfectly for doubles obtained from ASCII.
	//i.e. calculated doubles are sometimes wrong
	//e.g. 10.0/3.0 -> 3.3333333333333335 whereas "3.3333333333333333" -> double -> "3.3333333333333333"
	//e.g. 1234567890.00005678 -> "1234567890.0000567"
	const int n = d2s_buffered_n(double1, s.data());
	s.resize(n);
	//s is now something like "1.2345678E3" or "0E0" always in E format.
	//std::cout << s << std::endl;
	//return s;//435ns here. 500ns after conversion to fixed decimal place below (65ns)
	const std::size_t epos = s.find('E');

#elif 0	 //740ns ryu d2exp() always output scientific

	//ryu_printf compatible with nice rounding of all doubles but not perfect round tripping

	//ryu_printf %e equivalent (always scientific format to properly control precision)
	//followed by conversion to fixed format below
	//using precision 15 (which means 16 digits in scientific format)
	const int n = d2exp_buffered_n(double1, 15, s.data());
	s.resize(n);
	//s is now something like "1.234567800000000e+03"
	//std::cout << s << std::endl;
	//return s;//650ns here. 743ns after changing to fixed point below (93ns)
	const std::size_t epos = s.find('e');

#else  //???ns ryu d2fixed() always output fixed decimal point with fixed precision

	//ryu_printf %f equivalent (always fixed format)
	//
	//But this suffers from precision being after decimal point instead of overall number of digits
	// eg 1234.5678 -> "1234.56780000000003"
	//printl( var("999999999999999.9")    + 0);
	//             999999999999999.875
	//Could truncate after 15 digits but this would not be rounded properly
	const int n = d2fixed_buffered_n(double1, 16, s.data());
	s.resize(n);
	//remove trailing zeros
	//while (s.back() == '0')
	//	s.pop_back();
	std::size_t lastdigit = s.find_last_not_of('0');
	if (lastdigit != std::string::npos)
		s.resize(lastdigit);
	//remove trailing decimal point
	if (s.back() == '.')
		s.pop_back();
	return s;
	const std::size_t epos = std::string::npos;
#endif
	//std::cout << s << std::endl;

	//NOT USE_RYU
#else  //1800ns

	//std::cout << "std:sstream decimal oconv" << std::endl;

	std::ostringstream ss;

	//EITHER use 15 for which 64-bit IEEE 754 type double guarantees
	//roundtrip double/text/double for 15 decimal digits
	//
	//OR use precision 14 to avoid 1.1-1 = 0.1000000000000001
	//but this will only reduce errors after a single calculation
	//since errors are cumulative (random walk)
	//
	// Note that scientific precision "15" gives us
	// 16 total digits "1.234567890123456e+00"
	//

	//if not fixed precision then precision is relative to first digit, not decimal point
	//therefore very small numbers can have 15 digits of precision eg.
	//0.000000000000000000001234567890123456

	//use digits10 +1 if not using auto formatting (scientific for large/small numbers)

	//there appears to be little or no speed difference between always going via scientific format
	//and default method which only goes via scientific format for very large or small numbers
//#define ALWAYS_VIA_SCIENTIFIC
#ifdef ALWAYS_VIA_SCIENTIFIC
	//use digits10 if using scientific because we automatically get one additional on the left
	ss.precision(15);
	ss << std::scientific;
#else
	//use digits10 +1 if not using auto formatting (scientific for large/small numbers)
	ss.precision(16);
#endif

	ss << double1;
	std::string s = ss.str();

	std::size_t epos = s.find('e');

	////////////////////////////////////////////
	//if not scientific format then return as is
	////////////////////////////////////////////
#ifndef ALWAYS_VIA_SCIENTIFIC
	if (epos == std::string::npos)
		return s;
#endif
		//std::cout << "xxxxxxxxxxxxxxxxxxxxxxxxxxxx" << std::endl;

#endif	//USE_RYU

	// Algorithm
	//
	// Output in C++ scientific format to required precision
	// using sstream, ryu or to_chars. (use ryu currently)
	// then modify that string into normal format as follows:
	//
	// 1. remove the trailing "e+99" exponent
	// 2. add zeroes to the front or back of the number if exponent is not 0
	// 3. move the decimal point to the correct position.
	// 4. remove any trailing 0's and decimal point
	//
	// eg
	//
	//  1.230000000000000e+00   ->  1.23
	// -1.230000000000000e+00   -> -1.23
	//  1.230000000000000e+01   ->  12.3
	//  1.230000000000000e-01   ->  0.0123
	//  1.230000000000000e+04   ->  12300
	//  1.230000000000000e-04   ->  0.000123

	//NOTE for small numbers we should use precision
	//depending on how small the number is

	auto exponent = stoi(s.substr(epos + 1));

	if (epos != std::string::npos)
		s.erase(epos);

	//exponent 0
	if (!exponent) {

#ifndef USE_RYU_D2S
		//remove trailing zeros and decimal point
		while (s.back() == '0')
			s.pop_back();
		if (s.back() == '.')
			s.pop_back();
#ifdef USE_RYU
		//single zero if none
		//if (s.size() == std::size_t(minus))
		if (s.empty() || s == "-")
			s.push_back('0');
#endif
#endif
		return s;
	}

	//positive exponent
	else if (exponent > 0) {

		//remove decimal point
		s.erase(1 + minus, 1);

		//determine how many zeros need appending if any
		int addzeros = exponent - s.size() + 1 + minus;

		//debugging
		//std::cout << ss.str() << " " << s << " " << exponent << " " << s.size() << " " << addzeros << std::endl;

		//either append sufficient zeros
		if (addzeros > 0) {
			s.append(addzeros, '0');

		}

		//or insert decimal point within existing matissa
		else if (addzeros < 0) {

			//insert decimal point
			s.insert(exponent + minus + 1, 1, '.');

#ifndef USE_RYU_D2S
			goto removetrailing;
#endif
		}

		//negative exponent
	} else {

		//remove decimal point
		s.erase(1 + minus, 1);

		//prefix sufficient zeros
		s.insert(0 + minus, -exponent, '0');

		//insert decimal point
		s.insert(1 + minus, 1, '.');

#ifndef USE_RYU_D2S
removetrailing:
		//remove trailing zeros
		while (s.back() == '0')
			s.pop_back();
		//std::size_t lastdigit = s.find_last_not_of('0');
		//if (lastdigit != std::string::npos)
		//	s.resize(lastdigit+1);

		//remove trailing decimal point
		if (s.back() == '.')
			s.pop_back();
#endif
	}

	return s;
}

// mainly called in ISSTRING when not already a string
void var::createString() const {
	// THISIS("void var::createString() const")
	// TODO ensure ISDEFINED is called everywhere in advance
	// to avoid wasting time doing multiple calls to ISDEFINED
	// THISISDEFINED()

	// dbl - create string from dbl
	// prefer double
	if (var_typ & VARTYP_DBL) {
		var_str = dblToString(var_dbl);
		var_typ |= VARTYP_STR;
		return;
	}

	// int - create string from int
	if (var_typ & VARTYP_INT) {
		// loss of precision if var_int is long long
		var_str = std::to_string(int(var_int));
		var_typ |= VARTYP_STR;
		return;
	}
	// already a string (unlikely since only called when not a string)
	if (var_typ & VARTYP_STR) {
		return;
	}

	// treat any other case as unassigned
	//(usually var_typ & VARTYP_UNA)
	throw MVUnassigned("createString()");
}


// EXPLICIT AND AUTOMATIC CONVERSIONS
////////////////////////////////////

// someone recommends not to create more than one automatic converter
// to avoid the compiler error "ambiguous conversion"
//(explicit means it is not automatic)

// allow conversion to string (IS THIS USED FOR ANYTHING AT THE MOMENT?
// allows the usage of any string function
var::operator std::string() const {
	THISIS("var::operator std::string")
	THISISSTRING()
	return var_str;
}

var::operator void*() const {
	THISIS("var::operator void*() const")
	// could be skipped for speed if can be proved there is no way in c++ syntax that would
	// result in an attempt to convert an uninitialised object to void* since there is a bool
	// conversion when does c++ use automatic conversion to void* note that exodus operator !
	// uses (void*) trial elimination of operator void* seems to cause no problems but without
	// full regression testing
	THISISDEFINED()

	return (void*)toBool();
}

// supposed to be replaced with automatic void() and made explicit but just seems to force int
// conversion during "if (var)" necessary to allow var to be used standalone in "if (xxx)" but see
// mv.h for discussion of using void* instead of bool #ifndef _MSC_VER
var::operator bool() const {
	return toBool();
}

/*
var::operator const char*() const
{
	return toString().c_str();
}
*/

#ifndef HASINTREFOP
var::operator int() const {
	THISIS("var::operator int() const")
	//converts string or double to int using pickos int() which is floor()
	//unlike c/c++ int() function which rounds to nearest even number (negtive or positive)
	THISISINTEGER()
	return int(var_int);
}

var::operator double() const {
	THISIS("var::operator double() const")
	THISISDECIMAL()
	return var_dbl;
}

#else
var::operator int&() const {
	THISIS("var::operator mv_int_t&()")
	//converts string or double to int using pickos int() which is floor()
	//unlike c/c++ int() function which rounds to nearest even number (negtive or positive)
	THISISINTEGER()
	// TODO check that converting mvint_t (which is long long) to int doesnt cause any practical
	// problems) PROBABLY WILL! since we are returning a non const reference which allows
	// callers to set the int directly then clear any decimal and string cache flags which would
	// be invalid after setting the int alone
	//var_typ |= VARTYP_INT;
	return (int&)var_int;
}
var::operator double&() const {
	THISIS("var::operator double&()")
	THISISDECIMAL()
	// since we are returning a non const reference which allows callers to set the dbl directly
	// then clear any int and string cache flags which would be invalid after setting the dbl
	// alone
	//var_typ |= VARTYP_DBL;
	return (double&)var_dbl;
}
#endif

// remove because causes "ambiguous" with -short_wchar on linux
/*
var::operator unsigned int() const
{
	THISIS("var::operator int() const")
	THISISDEFINED()

	do
	{
		//prioritise int since conversion to int perhaps more likely to be an int already
		if (var_typ&VARTYP_INT)
			return var_int;
		if (var_typ&VARTYP_DBL)
			return int(var_dbl);
		if (var_typ&VARTYP_NAN)
			throw MVNonNumeric("int(" ^ substr(1,20) ^ ")");
		if (!(var_typ))
		{
			THISISASSIGNED()
			throw MVUnassigned("int(var)");
		}
	}
	//must be string - try to convert to numeric
	while (isnum());

	THISISNUMERIC()
	throw MVNonNumeric("int(" ^ substr(1,20) ^ ")");

}
*/

/*
//necessary to allow use of var inside STL containers
var::operator size_t() const
{
	return (size_t) operator int();
}
*/

/*
var::operator const char*()
{
	if (var_typ&VARTYP_MASK)
		throw MVUndefined("const char*()");
	cout<<"CONVERT: operator const char*() returns '"<<var_str.c_str()<<"'\n";
	return var_str.c_str();
}
*/

/////////////
// ASSIGNMENT
/////////////

// copy assignment
// var1 = var2
// The assignment operator should always return a reference to *this.
// cant be (CVR rhs) because seems to cause a problem with var1=var2 in function parameters
// unfortunately causes problem of passing var by value and thereby unnecessary contruction
// see also ^= etc
VOID_OR_VARREF var::operator=(CVR rhs) & {
	THISIS("VARREF var::operator=(CVR rhs) &")
	THISISDEFINED()	 //could be skipped for speed?
	ISASSIGNED(rhs)

	//std::clog << "copy assignment" <<std::endl;

	// important not to self assign
	if (this == &rhs)
		return VOID_OR_THIS;

	// copy everything across
	var_str = rhs.var_str;
	var_dbl = rhs.var_dbl;
	var_int = rhs.var_int;
	var_typ = rhs.var_typ;

	return VOID_OR_THIS;
}

#ifndef INLINE_ASSIGNMENT

	// move assignment
	// defined in class for inline/optimisation
	// var = temporary var
	VOID_OR_VARREF var::operator=(TVR rhs) & noexcept {
		// skip this for speed?
		// THISIS("VARREF var::operator= (var rhs)")
		// THISISDEFINED()

		// skip this for speed since temporararies are unlikely to be unassigned
		// THISIS("var::var(TVR rhs) noexcept")
		// ISASSIGNED(rhs)

		//std::clog << "move assignment" <<std::endl;

		// important not to self assign
		if (this == &rhs)
			return VOID_OR_THIS;

		// move everything over
		var_str = std::move(rhs.var_str);
		var_dbl = rhs.var_dbl;
		var_int = rhs.var_int;
		var_typ = rhs.var_typ;

		return VOID_OR_THIS;
	}

	// int assignment
	// The assignment operator should always return a reference to *this.
	VOID_OR_VARREF var::operator=(const int int1) & {
		// THISIS("VARREF var::operator= (const int int1)")
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
	VOID_OR_VARREF var::operator=(const double double1) & {
		// THISIS("VARREF var::operator= (const double double1)")
		// protect against unlikely syntax as follows:
		// var undefinedassign=undefinedassign=9.9';
		// !!RISK NOT CHECKING TO SPEED THINGS UP SINCE SEEMS TO WORK IN MSVC AND GCC
		// THISISDEFINED()

		var_dbl = double1;
		var_typ = VARTYP_DBL;  // reset to one unique type

		return VOID_OR_THIS;
	}

	// char assignment
	// The assignment operator should always return a reference to *this.
	VOID_OR_VARREF var::operator=(const char char2) & {

		//THISIS("VARREF var::operator= (const char char2) &")
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
	VOID_OR_VARREF var::operator=(const char* cstr) & {
		//THISIS("VARREF var::operator= (const char* cstr2) &")
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
	VOID_OR_VARREF var::operator=(const std::string& string2) & {

		//THISIS("VARREF var::operator= (const std::string& string2) &")
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
	VOID_OR_VARREF var::operator=(const std::string&& string2) & {

		//THISIS("VARREF var::operator= (const std::string&& string2) &")
		// protect against unlikely syntax as follows:
		// var undefinedassign=undefinedassign=std::string("xxx"";
		// this causes crash due to bad memory access due to setting string that doesnt exist
		// slows down all string settings so consider NOT CHECKING in production code
		//THISISDEFINED()

		var_str = std::move(string2);
		var_typ = VARTYP_STR;  // reset to one unique type

		return VOID_OR_THIS;
	}

#endif // not INLINE_ASSIGNMENT

// UNARY OPERATORS
//////////////////

//^= is not templated since slightly slower to go through the creation of an var()

//^=var
// The assignment operator should always return a reference to *this.
VARREF var::operator^=(CVR rhs) & {
	THISIS("VARREF var::operator^=(CVR rhs) &")
	THISISSTRING()
	ISSTRING(rhs)

	// tack it onto our string
	var_str.append(rhs.var_str);
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//^=int
// The assignment operator should always return a reference to *this.
VARREF var::operator^=(const int int1) & {
	THISIS("VARREF var::operator^=(const int int1) &")
	THISISSTRING()

	// var_str+=var(int1).var_str;
	var_str += std::to_string(int1);
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//^=double
// The assignment operator should always return a reference to *this.
VARREF var::operator^=(const double double1) & {
	THISIS("VARREF var::operator^=(const double double1) &")
	THISISSTRING()

	// var_str+=var(int1).var_str;
	var_str += dblToString(double1);
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//^=char
// The assignment operator should always return a reference to *this.
VARREF var::operator^=(const char char1) & {
	THISIS("VARREF var::operator^=(const char char1) &")
	THISISSTRING()

	// var_str+=var(int1).var_str;
	var_str.push_back(char1);
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//^=char*
// The assignment operator should always return a reference to *this.
VARREF var::operator^=(const char* cstr) & {
	THISIS("VARREF var::operator^=(const char* cstr) &")
	THISISSTRING()

	// var_str+=var(int1).var_str;
	// var_str+=std::string(char1);
	var_str += cstr;
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//^=std::string
// The assignment operator should always return a reference to *this.
VARREF var::operator^=(const std::string& string1) & {
	THISIS("VARREF var::operator^=(const std::string string1) &")
	THISISSTRING()

	// var_str+=var(int1).var_str;
	var_str += string1;
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

// not handled by inbuilt conversion of var to long long& on the rhs

//#ifndef HASINTREFOP
//#else

// You must *not* make the postfix version return the 'this' object by reference
// *** YOU HAVE BEEN WARNED ***

// not returning void so is usable in expressions
// int argument indicates that this is POSTFIX override v++
var var::operator++(int) & {
	THISIS("var var::operator++(int) &")
	// full check done below to avoid double checking number type
	THISISDEFINED()

	var priorvalue;

tryagain:
	// prefer int since ++ nearly always on integers
	if (var_typ & VARTYP_INT) {
		if (var_int == std::numeric_limits<decltype(var_int)>::max())
			throw MVIntOverflow("operator++");
		priorvalue = var(var_int);
		var_int++;
		var_typ = VARTYP_INT;  // reset to one unique type

	} else if (var_typ & VARTYP_DBL) {
		priorvalue = var_dbl;
		var_dbl++;
		var_typ = VARTYP_DBL;  // reset to one unique type

	} else if (var_typ & VARTYP_STR) {
		// try to convert to numeric
		if (isnum())
			goto tryagain;

		//trigger MVNonNumeric
		THISISNUMERIC()

	} else {
		//trigger MVUnassigned
		THISISNUMERIC()
	}

	// NO DO NOT! return *this ... postfix return a temporary!!! eg var(*this)
	return priorvalue;
}

// not returning void so is usable in expressions
// int argument indicates that this is POSTFIX override v--
var var::operator--(int) & {
	THISIS("var var::operator--(int) & ")
	// full check done below to avoid double checking number type
	THISISDEFINED()

	var priorvalue;

tryagain:
	// prefer int since -- nearly always on integers
	if (var_typ & VARTYP_INT) {
		if (var_int == std::numeric_limits<decltype(var_int)>::min())
			throw MVIntUnderflow("operator--");
		priorvalue = var(var_int);
		var_int--;
		var_typ = VARTYP_INT;  // reset to one unique type

	} else if (var_typ & VARTYP_DBL) {
		priorvalue = var_dbl;
		var_dbl--;
		var_typ = VARTYP_DBL;  // reset to one unique type

	} else if (var_typ & VARTYP_STR) {
		// try to convert to numeric
		if (isnum())
			goto tryagain;

		//trigger MVNonNumeric
		THISISNUMERIC()

	} else {
		//trigger MVUnassigned
		THISISNUMERIC()
	}

	return priorvalue;
}

// not returning void so is usable in expressions
// no argument indicates that this is prefix override ++var
VARREF var::operator++() & {
	THISIS("var var::operator++() &")
	// full check done below to avoid double checking number type
	THISISDEFINED()

tryagain:
	// prefer int since -- nearly always on integers
	if (var_typ & VARTYP_INT) {
		if (var_int == std::numeric_limits<decltype(var_int)>::max())
			throw MVIntOverflow("operator++");
		var_int++;
		var_typ = VARTYP_INT;  // reset to one unique type
	} else if (var_typ & VARTYP_DBL) {
		var_dbl++;
		var_typ = VARTYP_DBL;  // reset to one unique type
	} else if (var_typ & VARTYP_STR) {
		// try to convert to numeric
		if (isnum())
			goto tryagain;

		//trigger MVNonNumeric
		THISISNUMERIC()

	} else {
		//trigger MVUnassigned
		THISISNUMERIC()
	}

	// OK to return *this in prefix ++
	return *this;
}

// not returning void so is usable in expressions
// no argument indicates that this is prefix override --var
VARREF var::operator--() & {
	THISIS("VARREF var::operator--() &")
	// full check done below to avoid double checking number type
	THISISDEFINED()

tryagain:
	// prefer int since -- nearly always on integers
	if (var_typ & VARTYP_INT) {
		if (var_int == std::numeric_limits<decltype(var_int)>::min())
			throw MVIntUnderflow("operator--");
		var_int--;
		var_typ = VARTYP_INT;  // reset to one unique type

	} else if (var_typ & VARTYP_DBL) {
		var_dbl--;
		var_typ = VARTYP_DBL;  // reset to one unique type

	} else if (var_typ & VARTYP_STR) {
		// try to convert to numeric
		if (isnum())
			goto tryagain;

		//trigger MVNonNumeric
		THISISNUMERIC()

	} else {
		//trigger MVUnassigned
		THISISNUMERIC()
	}

	// OK to return *this in prefix --
	return *this;
}

//+=var (very similar to version with on rhs)
// provided to disambiguate syntax like var1+=var2
VARREF var::operator+=(const int int1) & {
	THISIS("VARREF var::operator+=(const int int1) &")
	THISISDEFINED()

tryagain:

	// dbl target
	// prefer double
	if (var_typ & VARTYP_DBL) {
		//+= int or dbl from source
		var_dbl += int1;
		var_typ = VARTYP_DBL;  // reset to one unique type
		return *this;
	}

	// int target
	else if (var_typ & VARTYP_INT) {
		var_int += int1;
		var_typ = VARTYP_INT;  // reset to one unique type
		return *this;
	}

	// last case(s) should be much less frequent since result of attempt to
	// convert strings to number is cached and only needs to be done once

	// nan (dont bother with this here because it is exceptional and will be caught below anyway
	// else if (var_typ&VARTYP_NAN)
	//	throw MVNonNumeric("var::+= " ^ *this);

	// try to convert to numeric
	if (isnum())
		goto tryagain;

	THISISNUMERIC()
	throw MVNonNumeric(substr(1, 128) ^ "+= ");
}

//+='1' (very similar to version with on rhs)
// provided to disambiguate syntax like var1 += '1'

VARREF var::operator+=(const char char1) & {
	THISIS("VARREF var::operator+=(const char char1) &")
	var charx = char1;
	ISNUMERIC(charx)
	return this->operator+=(int(charx.var_int));
}

//-='1' (very similar to version with on rhs)
// provided to disambiguate syntax like var1 -= '1'
VARREF var::operator-=(const char char1) & {
	THISIS("VARREF var::operator-=(const char char1) &")
	var charx = char1;
	ISNUMERIC(charx)
	return this->operator-=(int(charx.var_int));
}

//-=var (very similar to version with on rhs)
// provided to disambiguate syntax like var1+=var2
VARREF var::operator-=(const int int1) & {
	THISIS("VARREF var::operator-=(int int1) &")
	THISISDEFINED()

tryagain:
	// dbl target
	// prefer double
	if (var_typ & VARTYP_DBL) {
		var_dbl -= int1;
		var_typ = VARTYP_DBL;  // reset to one unique type
		return *this;
	}

	// int target
	else if (var_typ & VARTYP_INT) {
		var_int -= int1;
		var_typ = VARTYP_INT;  // reset to one unique type
		return *this;
	}

	// try to convert to numeric
	if (isnum())
		goto tryagain;

	THISISNUMERIC()
	throw MVNonNumeric(substr(1, 128) ^ "-= ");
}

// allow varx+=1.5 to compile
VARREF var::operator+=(const double dbl1) & {
	(*this) += var(dbl1);
	return *this;
}

VARREF var::operator-=(const double dbl1) & {
	(*this) -= var(dbl1);
	return *this;
}

//+=var
VARREF var::operator+=(CVR rhs) & {
	THISIS("VARREF var::operator+=(CVR rhs) &")
	THISISDEFINED()

tryagain:

	// dbl target
	// prefer double
	if (var_typ & VARTYP_DBL) {
		ISNUMERIC(rhs)
		//+= int or dbl from source
		var_dbl += (rhs.var_typ & VARTYP_INT) ? rhs.var_int : rhs.var_dbl;
		var_typ = VARTYP_DBL;  // reset to one unique type
		return *this;
	}

	// int target
	else if (var_typ & VARTYP_INT) {
		ISNUMERIC(rhs)
		// int source
		if (rhs.var_typ & VARTYP_INT) {
			var_int += rhs.var_int;
			var_typ = VARTYP_INT;  // reset to one unique type
			return *this;
		}
		// dbl source, convert target to dbl
		var_dbl = var_int + rhs.var_dbl;
		var_typ = VARTYP_DBL;  // reset to one unique type
		return *this;
	}

	// last case(s) should be much less frequent since result of attempt to
	// convert strings to number is cached and only needs to be done once

	// nan (dont bother with this here because it is exceptional and will be caught below anyway
	// else if (var_typ&VARTYP_NAN)
	//	throw MVNonNumeric("var::+= " ^ *this);

	// try to convert to numeric
	if (isnum())
		goto tryagain;

	THISISNUMERIC()
	throw MVNonNumeric(substr(1, 128) ^ "+= ");
}

//-=var
VARREF var::operator-=(CVR rhs) & {
	THISIS("VARREF var::operator-=(CVR rhs) &")
	THISISDEFINED()

tryagain:

	// int target
	if (var_typ & VARTYP_INT) {
		ISNUMERIC(rhs)
		// int source
		if (rhs.var_typ & VARTYP_INT) {
			var_int -= rhs.var_int;
			var_typ = VARTYP_INT;  // reset to one unique type
			return *this;
		}
		// dbl source, convert target to dbl
		var_dbl = var_int - rhs.var_dbl;
		var_typ = VARTYP_DBL;  // reset to one unique type
		return *this;
	}

	// dbl target
	else if (var_typ & VARTYP_DBL) {
		ISNUMERIC(rhs)
		//-= int or dbl from source
		var_dbl -= (rhs.var_typ & VARTYP_INT) ? rhs.var_int : rhs.var_dbl;
		var_typ = VARTYP_DBL;  // reset to one unique type
		return *this;
	}

	// last case(s) should be much less frequent since result of attempt to
	// convert strings to number is cached and only needs to be done once

	// nan (dont bother with this here because it is exceptional and will be caught below anyway
	// else if (var_typ&VARTYP_NAN)
	//	throw MVNonNumeric("var::-= " ^ *this);

	// try to convert to numeric
	if (isnum())
		goto tryagain;

	THISISNUMERIC()
	throw MVNonNumeric(substr(1, 128) ^ "-= ");
}

//#endif

/* more accurate way of comparing two decimals using EPSILON and ulp
   not using it simply to make better emulation of pickos at least initially

//comparing floating point numbers is a VERY complex matter since c++ double uses BINARY NOT DECIMAL
//
//https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
//
//https://www.theregister.com/2006/08/12/floating_point_approximation/
//https://www.theregister.com/2006/09/20/floating_point_numbers_2/
//
//derived from https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
inline bool almost_equal_not_zero(double x, double y, int ulp)
{
	// the machine epsilon has to be scaled to the magnitude of the values used
	// and multiplied by the desired precision in ULPs (units in the last place)
	return std::fabs(x-y) <= std::numeric_limits<double>::epsilon() * std::fabs(x+y) * ulp
		// unless the result is subnormal
		|| std::fabs(x-y) < std::numeric_limits<double>::min();
}

inline bool almost_equal(double x, decltype(var_int) y, int ulp)
{
	if (y == 0)
		return (std::abs(x) < SMALLEST_NUMBER);
	else
		return almost_equal_not_zero(x, double(y), ulp);
}

inline bool almost_equal(double x, double y, int ulp)
{
	if (y == 0.0)
		return (std::abs(x) < SMALLEST_NUMBER);
	else if (x == 0.0)
		return (std::abs(y) < SMALLEST_NUMBER);
	else
		return almost_equal_not_zero(x, y, ulp);
}
*/

inline bool almost_equal(double x, double y, int) {
	//crude pickos method
	return (std::abs(x - y) < SMALLEST_NUMBER);
}

// almost identical code in MVeq and MVlt except where noted
// NOTE doubles compare only to 0.0001 accuracy)
PUBLIC bool MVeq(CVR lhs, CVR rhs) {
	THISIS("bool MVeq(CVR lhs,CVR rhs)")
	ISDEFINED(lhs)
	ISDEFINED(rhs)

	// NB empty string is always less than anything except another empty string

	// 1. BOTH EMPTY or IDENTICAL STRINGS returns TRUE one empty results false
	if (lhs.var_typ & VARTYP_STR) {
		if (rhs.var_typ & VARTYP_STR) {
			// we have two strings
			// if they are both the same (including both empty) then eq is true
			if (lhs.var_str == rhs.var_str)
				// different from MVlt
				return true;
			// otherwise if either is empty then return eq false
			//(since empty string is ONLY eq to another empty string)
			if (lhs.var_str.empty())
				// different from MVlt
				return false;
			if (rhs.var_str.empty())
				// SAME as MVlt
				return false;
			// otherwise go on to test numerically then literally
		} else {
			// if rhs isnt a string and lhs is empty then eq is false
			//(after checking that rhs is actually assigned)
			if (lhs.var_str.empty()) {
				if (!rhs.var_typ) {
					// throw MVUnassigned("eq(rhs)");
					ISASSIGNED(rhs)
				}
				// different from MVlt
				return false;
			}
		}
	} else {
		// if lhs isnt a string and rhs is an empty string then return eq false
		//(after checking that lhs is actually assigned)
		if ((rhs.var_typ & VARTYP_STR) && (rhs.var_str.empty())) {
			if (!lhs.var_typ) {
				// throw MVUnassigned("eq(lhs)");
				ISASSIGNED(lhs)
			}
			// SAME as MVlt
			return false;
		}
	}

	// 2. BOTH NUMERIC STRINGS
	// exact match on decimal numbers is often inaccurate since they are approximations of real
	// numbers
	// match on decimal in preference to int
	if (lhs.isnum() && rhs.isnum()) {
		//if (lhs.var_typ & VARTYP_INT)
		if (lhs.var_typ & VARTYP_DBL) {

			//DOUBLE v DOUBLE
			//if (rhs.var_typ & VARTYP_INT)
			if (rhs.var_typ & VARTYP_DBL) {
				// different from MVlt

				//return (lhs.var_intd == rhs.var_intd);

				// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)
				//return (std::abs(lhs.var_dbl - rhs.var_dbl) < SMALLEST_NUMBER);
				return almost_equal(lhs.var_dbl, rhs.var_dbl, 2);
			}

			//DOUBLE V INT
			else {
				// different from MVlt (uses absolute)
				// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)

				//return (lhs.var_int == rhs.var_dbl);
				//return (std::abs(lhs.var_dbl - double(rhs.var_int)) < SMALLEST_NUMBER);
				return almost_equal(lhs.var_dbl, rhs.var_int, 2);
			}
		}

		//INT v DOUBLE
		//if (rhs.var_typ & VARTYP_INTd)
		if (rhs.var_typ & VARTYP_DBL) {
			// different from MVlt (uses absolute)
			// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)

			//return (lhs.var_dbl == rhs.var_int);
			//return (std::abs(double(lhs.var_int) - rhs.var_dbl) < SMALLEST_NUMBER);
			//return almost_equal(lhs.var_int, rhs.var_dbl, 2);
			//put lhs int 2nd argument to invoke the fastest implmentation
			return almost_equal(rhs.var_dbl, lhs.var_int, 2);
		}

		//INT v INT
		else {
			// different from MVlt (uses absolute)

			// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)
			//return (lhs.var_dbl == rhs.var_dbl);
			//return (std::abs(lhs.var_dbl-rhs.var_dbl) < SMALLEST_NUMBER);

			return (lhs.var_int == rhs.var_int);
		}
	}

	// 3. BOTH NON-NUMERIC STRINGS
	if (!(lhs.var_typ & VARTYP_STR))
		lhs.createString();
	if (!(rhs.var_typ & VARTYP_STR))
		rhs.createString();
	// different from MVlt
	//return lhs.localeAwareCompare(lhs.var_str, rhs.var_str) == 0;
	return lhs.var_str == rhs.var_str;
}

// almost identical between MVeq and MVlt except where noted
// NOTE doubles compare only to 0.0001 accuracy)
PUBLIC bool MVlt(CVR lhs, CVR rhs) {
	THISIS("bool MVlt(CVR lhs,CVR rhs)")
	ISDEFINED(lhs)
	ISDEFINED(rhs)

	// NB empty string is always less than anything except another empty string

	// 1. both empty or identical strings returns eq. one empty results false
	if (lhs.var_typ & VARTYP_STR) {
		if (rhs.var_typ & VARTYP_STR) {
			// we have two strings
			// if they are both the same (including both empty) then eq is true
			if (lhs.var_str == rhs.var_str)
				// different from MVeq
				return false;
			// otherwise if either is empty then return eq false
			//(since empty string is ONLY eq to another empty string)
			if (lhs.var_str.empty())
				// different from MVeq
				return true;
			if (rhs.var_str.empty())
				// SAME as MVeq
				return false;
			// otherwise go on to test numerically then literally
		} else {
			// if rhs isnt a string and lhs is empty then eq is false
			// after checking that rhs is actually assigned
			if (lhs.var_str.empty()) {
				if (!rhs.var_typ) {
					// throw MVUnassigned("eq(rhs)");
					ISASSIGNED(rhs)
				}
				// different from MVeq
				return true;
			}
		}
	} else {
		// if lhs isnt a string and rhs is an empty string then return eq false
		// after checking that lhs is actually assigned
		if ((rhs.var_typ & VARTYP_STR) && (rhs.var_str.empty())) {
			if (!lhs.var_typ) {
				// throw MVUnassigned("eq(lhs)");
				ISASSIGNED(lhs)
			}
			// SAME as MVeq
			return false;
		}
	}

	// 2. both numerical strings
	if (lhs.isnum() && rhs.isnum()) {
		if (lhs.var_typ & VARTYP_INT) {
			if (rhs.var_typ & VARTYP_INT)
				// different from MVeq
				return (lhs.var_int < rhs.var_int);
			else
				// different from MVeq
				// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)
				//return (double(lhs.var_int) < rhs.var_dbl);
				return (rhs.var_dbl - double(lhs.var_int)) >= SMALLEST_NUMBER;
			//return ((rhs.var_dbl - double(lhs.var_int) >= SMALLEST_NUMBER);
		}
		if (rhs.var_typ & VARTYP_INT)
			// different from MVeq
			// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)
			//return (lhs.var_dbl < double(rhs.var_int));
			return (double(rhs.var_int) - lhs.var_dbl) >= SMALLEST_NUMBER;
		else
			// different from MVeq
			// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)
			//return (lhs.var_dbl < rhs.var_dbl);
			return (rhs.var_dbl - lhs.var_dbl) >= SMALLEST_NUMBER;
	}

	// 3. either or both non-numerical strings
	if (!(lhs.var_typ & VARTYP_STR))
		lhs.createString();
	if (!(rhs.var_typ & VARTYP_STR))
		rhs.createString();
	// different from MVeq
	// return lhs.var_str<rhs.var_str;
	return lhs.localeAwareCompare(lhs.var_str, rhs.var_str) < 0;
}

// similar to MVeq and MVlt - this is the var<int version for speed
// NOTE doubles compare only to 0.0001 accuracy)
PUBLIC bool MVlt(CVR lhs, const int int2) {
	THISIS("bool MVlt(CVR lhs,const int int2)")
	ISDEFINED(lhs)

	// NB empty string is always less than anything except another empty string

	// 1. both empty or identical strings returns eq. one empty results false
	if (lhs.var_typ & VARTYP_STR) {
		// if rhs isnt a string and lhs is empty then eq is false
		// after checking that rhs is actually assigned
		if (lhs.var_str.empty()) {
			// different from MVeq
			return true;
		}
	}

	// 2. both numerical strings
	do {
		if (lhs.var_typ & VARTYP_INT)
			// different from MVeq
			return (lhs.var_int < int2);

		if (lhs.var_typ & VARTYP_DBL)
			// different from MVeq
			// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)
			//return (lhs.var_dbl < int2);
			return (double(int2) - lhs.var_dbl) >= SMALLEST_NUMBER;
		//return (double(int2) - lhs.var_dbl) >= SMALLEST_NUMBER;
	}
	// go back and try again if can be converted to number
	while (lhs.isnum());

	// 3. either or both non-numerical strings
	if (!(lhs.var_typ & VARTYP_STR)) {
		// lhs.createString();
		ISSTRING(lhs)
	}
	// different from MVeq
	return lhs.var_str < std::to_string(int2);
}

// similar to MVeq and MVlt - this is the int<var version for speed
// NOTE doubles compare only to 0.0001 accuracy)
PUBLIC bool MVlt(const int int1, CVR rhs) {
	THISIS("bool MVlt(const int int1,CVR rhs)")
	ISDEFINED(rhs)

	// NB empty string is always less than anything except another empty string

	// 1. both empty or identical strings returns eq. one empty results false
	if (rhs.var_typ & VARTYP_STR) {
		if (rhs.var_str.empty())
			// SAME as MVeq
			return false;
		// otherwise go on to test numerically then literally
	}

	// 2. both numerical strings
	do {
		if (rhs.var_typ & VARTYP_INT)
			// different from MVeq
			return (int1 < rhs.var_int);
		if (rhs.var_typ & VARTYP_DBL) {
			// different from MVeq
			// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)
			//return (int1 < rhs.var_dbl);
			return (rhs.var_dbl - double(int1)) >= SMALLEST_NUMBER;
		}
	}
	// go back and try again if can be converted to number
	while (rhs.isnum());

	// 3. either or both non-numerical strings
	if (!(rhs.var_typ & VARTYP_STR)) {
		// lhs.createString();
		ISSTRING(rhs)
	}
	// different from MVeq
	return std::to_string(int1) < rhs.var_str;
}
// SEE ALSO MV2.CPP

//+var
//PUBLIC var operator+(CVR var1)
var MVplus(CVR var1) {
	THISIS("var operator+(CVR var1)")
	ISDEFINED(var1)

	do {
		// dbl
		if (var1.var_typ & VARTYP_DBL)
			return var1.var_dbl;

		// int
		if (var1.var_typ & VARTYP_INT)
			return var1.var_int;

		// unassigned
		if (!var1.var_typ) {
			ISASSIGNED(var1)
			throw MVUnassigned("+()");
		}
	}
	// must be string - try to convert to numeric
	while (var1.isnum());

	// non-numeric
	ISNUMERIC(var1)
	// will never get here
	throw MVNonNumeric("+(" ^ var1.substr(1, 128) ^ ")");
}

//-var (identical to +var above except for two additional - signs)
//PUBLIC var operator-(CVR var1)
var MVminus(CVR var1) {
	THISIS("var operator-(CVR var1)")
	ISDEFINED(var1)

	do {
		// dbl
		if (var1.var_typ & VARTYP_DBL)
			return -var1.var_dbl;

		// int
		if (var1.var_typ & VARTYP_INT)
			return -var1.var_int;

		// unassigned
		if (!var1.var_typ) {
			ISASSIGNED(var1)
			throw MVUnassigned("+()");
		}
	}
	// must be string - try to convert to numeric
	while (var1.isnum());

	// non-numeric
	ISNUMERIC(var1)
	// will never get here
	throw MVNonNumeric("+(" ^ var1.substr(1, 128) ^ ")");
}

//! var
//PUBLIC bool operator!(CVR var1)
bool MVnot(CVR var1) {
	THISIS("bool operator!(CVR var1)")
	ISASSIGNED(var1)

	// might need converting to work on void pointer
	// if bool replaced with void* (or made explicit instead of implict)
	// is there really any difference since the bool and void operators are defined identically?
	// return !(bool)(var1);
	return !(void*)(var1);
}

var MVadd(CVR lhs, CVR rhs) {
	THISIS("var operator+(CVR lhs,CVR rhs)")
	ISNUMERIC(lhs)
	ISNUMERIC(rhs)

	//identical code in MVadd and MVsub except for +/-
	//identical code in MVadd, MVsub, MVmul except for +,-,*
	if (lhs.var_typ & VARTYP_DBL)
		return lhs.var_dbl + ((rhs.var_typ & VARTYP_DBL) ? rhs.var_dbl : double(rhs.var_int));
	else if (rhs.var_typ & VARTYP_DBL)
		return lhs.var_int + rhs.var_dbl;
	else
		return lhs.var_int + rhs.var_int;
}

var MVsub(CVR lhs, CVR rhs) {
	THISIS("var operator-(CVR lhs,CVR rhs)")
	ISNUMERIC(lhs)
	ISNUMERIC(rhs)

	//identical code in MVadd, MVsub, MVmul except for +,-,*
	if (lhs.var_typ & VARTYP_DBL)
		return lhs.var_dbl - ((rhs.var_typ & VARTYP_DBL) ? rhs.var_dbl : double(rhs.var_int));
	else if (rhs.var_typ & VARTYP_DBL)
		return lhs.var_int - rhs.var_dbl;
	else
		return lhs.var_int - rhs.var_int;
}

var MVmul(CVR lhs, CVR rhs) {
	THISIS("var operator*(CVR lhs,CVR rhs)")
	ISNUMERIC(lhs)
	ISNUMERIC(rhs)

	//identical code in MVadd, MVsub, MVmul except for +,-,*
	if (lhs.var_typ & VARTYP_DBL)
		return lhs.var_dbl * ((rhs.var_typ & VARTYP_DBL) ? rhs.var_dbl : double(rhs.var_int));
	else if (rhs.var_typ & VARTYP_DBL)
		return lhs.var_int * rhs.var_dbl;
	else
		return lhs.var_int * rhs.var_int;
}

var MVdiv(CVR lhs, CVR rhs) {
	THISIS("var operator/(CVR lhs,CVR rhs)")
	ISNUMERIC(lhs)
	ISNUMERIC(rhs)

	// always returns a double because 10/3 must be 3.3333333

	if (lhs.var_typ & VARTYP_DBL) {
		// 1. double ... double
		if (rhs.var_typ & VARTYP_DBL) {
			if (!rhs.var_dbl)
				throw MVDivideByZero("div('" ^ lhs.substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
									 "')");
			return lhs.var_dbl / rhs.var_dbl;
		}
		// 2. double ... int
		else {
			if (!rhs.var_int)
				throw MVDivideByZero("div('" ^ lhs.substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
									 "')");
			return lhs.var_dbl / rhs.var_int;
		}
	}
	// 3. int ... double
	else if (rhs.var_typ & VARTYP_DBL) {
		if (!rhs.var_dbl)
			throw MVDivideByZero("div('" ^ lhs.substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
								 "')");
		return static_cast<double>(lhs.var_int) / rhs.var_dbl;
	}
	// 4. int ... int
	else {
		if (!rhs.var_int)
			throw MVDivideByZero("div('" ^ lhs.substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
								 "')");
		return static_cast<double>(lhs.var_int) / rhs.var_int;
	}
}

double exodusmodulus(const double top, const double bottom) {
#define USE_PICKOS_MODULUS
#ifdef USE_PICKOS_MODULUS

	// note that exodus var int() is floor function as per pickos standard
	// whereas c/c++ int() function is round to nearest even number (negative or positive)

	//https://pickos.neosys.com/x/pAEz.html
	//var(i) - (int(var(i)/var(j)) * var(j))) ... where int() is pickos int() (i.e. floor)
	return top - double(floor(top / bottom) * bottom);

#else
	//return top - double(int(top / bottom) * bottom);
	//return top % bottom;//doesnt compile (doubles)

	//fmod - The floating-point remainder of the division operation x/y calculated by this function is exactly the value x - n*y, where n is x/y with its fractional part truncated.
	//https://en.cppreference.com/w/c/numeric/math/fmod
	//return std::fmod(top,bottom);

	//remainder - The IEEE floating-point remainder of the division operation x/y calculated by this function is exactly the value x - n*y, where the value n is the integral value nearest the exact value x/y. When |n-x/y| = ½, the value n is chosen to be even.
	//https://en.cppreference.com/w/c/numeric/math/remainder
	return std::remainder(top, bottom);

#endif
}

var MVmod(CVR lhs, CVR rhs) {
	THISIS("var operator%(CVR lhs,CVR rhs)")
	ISNUMERIC(lhs)
	ISNUMERIC(rhs)

	//returns an integer var IIF both arguments are integer vars
	//otherwise returns a double var

	if (lhs.var_typ & VARTYP_DBL) {
		// 1. double ... double
		if (rhs.var_typ & VARTYP_DBL) {
			if (!rhs.var_dbl)
				throw MVDivideByZero("mod('" ^ lhs.substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
									 "')");
			return exodusmodulus(lhs.var_dbl, rhs.var_dbl);
		}
		// 2. double ... int
		else {
			if (!rhs.var_int)
				throw MVDivideByZero("mod('" ^ lhs.substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
									 "')");
			return exodusmodulus(lhs.var_dbl, rhs.var_int);
		}
	}
	// 3. int ... double
	else if (rhs.var_typ & VARTYP_DBL) {
		if (!rhs.var_dbl)
			throw MVDivideByZero("mod('" ^ lhs.substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
								 "')");
		return exodusmodulus(lhs.var_int, rhs.var_dbl);
	}
	// 4. int ... int
	else {
		if (!rhs.var_int)
			throw MVDivideByZero("mod('" ^ lhs.substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
								 "')");
		//return exodusmodulus(lhs.var_int, rhs.var_int);
		return lhs.var_int % rhs.var_int;
	}
}

// var^var we reassign the logical xor operator ^ to be string concatenate!!!
// slightly wrong precedence but at least we have a reliable concat operator to replace the + which
// is now reserved for forced ADDITION both according to fundamental PickOS principle
var MVcat(CVR lhs, CVR rhs) {
	THISIS("var operator^(CVR lhs,CVR rhs)")
	ISSTRING(lhs)
	ISSTRING(rhs)

	return lhs.var_str + rhs.var_str;
}

var MVcat(CVR lhs, const char* cstr) {
	THISIS("var operator^(CVR lhs,const char* cstr)")
	ISSTRING(lhs)

	return lhs.var_str + cstr;
}

var MVcat(const char* cstr, CVR rhs) {
	THISIS("var operator^(const char* cstr, CVR rhs)")
	ISSTRING(rhs)

	return cstr + rhs.var_str;
}

var MVcat(CVR lhs, const char char2) {
	THISIS("var operator^(CVR lhs,const char char2)")
	ISSTRING(lhs)

	return lhs.var_str + char2;
}
/*

VARREF var::operator^(CVR vstr) {
	THISIS("VARREF var::operator^(CVR vstr)")
	THISISSTRING()
	ISSTRING(vstr)

	var_str += vstr.var_str;

	return *this;
}

VARREF var::operator^(const char* cstr) {
	std::clog <<("var operator^(const char& cstr)") << std::endl;
	THISIS("VARREF var::operator^(const char& cstr)")
	THISISSTRING()

	var_str += cstr;

	return *this;
}

VARREF var::operator^(const std::string& stdstr) {
	THISIS("VARREF var::operator^(const std::string& stdstr)");
	THISISSTRING()
	ISSTRING(stdstr)

	var_str += stdstr.var_str;

	return *this;
}
*/

//#if defined __MINGW32__
// allow use of cout<<var
//pass by value (make a copy) because we are going to convert FM to ^ etc
//TODO provide a version that works on temporaries?
//PUBLIC
std::ostream& operator<<(std::ostream& ostream1, var var1) {
	THISIS("std::ostream& operator<<(std::ostream& ostream1, var var1)")
	ISSTRING(var1)

	//replace various unprintable field marks with unusual ASCII characters
	//leave ESC as \x1B because it is used to control ANSI terminal control sequences
	//std::string str = "\x1A\x1B\x1C\x1D\x1E\x1F";
	// |\x1B}]^~  or in high to low ~^]}\x1B|     or in TRACE() ... ~^]}_|
	std::string str = "\x1E\x1D\x1C\x1B\x1A\x1F";  //order by frequency of occurrence
	for (auto& c : var1.var_str) {
		if (c >= 0x1A && c <= 0x1F)
			c = "|\x1B}]^~"[c - 0x1A];
	}

	// use toString() to avoid creating a constructor which logs here recursively
	// should this use a ut16/32 -> UTF8 code facet? or convert to UTF8 and output to ostream?
	ostream1 << var1.var_str;
	return ostream1;
}

std::istream& operator>>(std::istream& istream1, VARREF var1) {
	THISIS("std::istream& operator>>(std::istream& istream1,VARREF var1)")
	ISDEFINED(var1)

	std::string tempstr;
	istream1 >> std::noskipws >> tempstr;

	var1.var_typ = VARTYP_STR;
	// this would verify all input is valid utf8
	// var1.var_str=boost::locale::conv::utf_to_utf<char>(tempstr)
	var1.var_str = tempstr;
	return istream1;
}

//#endif

var backtrace();

MVError::MVError(CVR description_)
	: description(description_) {
	// capture the stack at point of creation i.e. when thrown
	this->stack = backtrace();
	((description.assigned() ? description : "") ^ "\n" ^ stack.convert(FM, "\n") ^ "\n").put(std::cerr);

	//break into debugger if EXO_DEBUG is set to non-zero
	//therwise allow catch at a higher level or abort
	var exo_debug;
	exo_debug.osgetenv("EXO_DEBUG");
	if (exo_debug) {
		var().debug();
	}
}

MVUnassigned ::MVUnassigned(CVR var1)
	: MVError("MVUnassigned:" ^ var1) {}
MVDivideByZero ::MVDivideByZero(CVR var1)
	: MVError("MVDivideByZero:" ^ var1) {}
MVNonNumeric ::MVNonNumeric(CVR var1)
	: MVError("MVNonNumeric:" ^ var1) {}
MVIntOverflow ::MVIntOverflow(CVR var1)
	: MVError("MVIntOverflow:" ^ var1) {}
MVIntUnderflow ::MVIntUnderflow(CVR var1)
	: MVError("MVIntUnderflow:" ^ var1) {}
MVUndefined ::MVUndefined(CVR var1)
	: MVError("MVUndefined:" ^ var1) {}
MVOutOfMemory ::MVOutOfMemory(CVR var1)
	: MVError("MVOutOfMemory:" ^ var1) {}
MVInvalidPointer ::MVInvalidPointer(CVR var1)
	: MVError("MVInvalidPointer:" ^ var1) {}
MVDBException ::MVDBException(CVR var1)
	: MVError("MVDBException:" ^ var1) {}
MVNotImplemented ::MVNotImplemented(CVR var1)
	: MVError("MVNotImplemented:" ^ var1) {}
MVDebug ::MVDebug(CVR var1)
	: MVError("MVDebug" ^ var1) {}
MVStop ::MVStop(CVR var1)
	: description(var1) {}
MVAbort ::MVAbort(CVR var1)
	: description(var1) {}
MVAbortAll ::MVAbortAll(CVR var1)
	: description(var1) {}
MVLogoff ::MVLogoff(CVR var1)
	: description(var1) {}
MVArrayDimensionedZero ::MVArrayDimensionedZero()
	: MVError("MVArrayDimensionedZero:") {}
MVArrayIndexOutOfBounds ::MVArrayIndexOutOfBounds(CVR var1)
	: MVError("MVArrayIndexOutOfBounds:" ^ var1) {
}
MVArrayNotDimensioned ::MVArrayNotDimensioned()
	: MVError("MVArrayNotDimensioned") {}

var operator""_var(const char* cstr, std::size_t size) {
	return var(cstr, size);
}

var operator""_var(unsigned long long int i) {
	return var(int(i));
}

var operator""_var(long double d) {
	return var(double(d));
}
}  // namespace exodus
