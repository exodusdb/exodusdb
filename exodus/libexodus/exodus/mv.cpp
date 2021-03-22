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

// C4530: C++ exception handler used, but unwind semantics are not enabled.
#pragma warning(disable : 4530)
#include <iostream>

// TODO: check that "-" and "-." is NOT numeric

// TODO make mvtype a bit field indicating multiple types present
// or overlap the string/integer/float variables to save space

#include <limits>
#include <sstream>
#include <vector>
#include <cassert>

//ryu            1234.5678 -> "1234.5678" 500ns
//ryu_printf     1234.5678 -> "1234.5678" 800ns
//sstream/printf 1234.5678 -> "1234.5678" 1800ns
#if __has_include(<ryu/ryu.h>)
#define USE_RYU
#include <ryu/ryu.h>
#endif

#define EXO_MV_CPP	// indicates globals are to be defined (omit extern keyword)
#include <exodus/mv.h>
//#include <exodus/mvutf.h>
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

// DESTRUCTOR
/////////////
var::~var() {
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

// CONSTRUCTORS
//////////////

// default constructor to allow definition unassigned "var mv";
var::var()
	: var_typ(VARTYP_UNA) {
	//std::cout << "ctor()" << std::endl;

	// int xyz=3;
	// WARNING neither initialisers nor constructors are called in the following case !!!
	// var xxx=xxx.somefunction()
	// known as "undefined usage of uninitialised variable";
	// and not even a compiler warning in msvc8 or g++4.1.2

	// so the following test is put everywhere to protect against this type of accidental
	// programming if (var_typ&VARTYP_MASK) throw MVUndefined("funcname()"); should really
	// ensure a magic number and not just HOPE for some binary digits above bottom four 0-15
	// decimal 1111binary this could be removed in production code perhaps

	// debuggCONSTRUCT&&cout<<"CONSTRUCT: var()\n";

	// not a pointer anymore for speed
	// priv=new pimpl;

	// moved here from pimpl constructor
	// moved up to initializer
	// var_typ=VARTYP_UNA;
}

// copy constructor
var::var(const var& rhs)
    : var_str(rhs.var_str),
      var_int(rhs.var_int),
      var_dbl(rhs.var_dbl),
      var_typ(rhs.var_typ)
{
	// use initializers (why?) and only check afterwards if copiedvar was assigned
	THISIS("var::var(const var& rhs)")
	ISASSIGNED(rhs)

	//std::clog << "copy ctor const var&" << std::endl;

	// not a pointer anymore for speed
	// priv=new pimpl;
}

// move constructor
var::var(var&& rhs) noexcept
    : var_str(std::move(rhs.var_str)),
      var_int(rhs.var_int),
      var_dbl(rhs.var_dbl),
      var_typ(rhs.var_typ)
{
	//std::clog << "move ctor var&& noexcept" << std::endl;

	// skip this for speed since temporararies are unlikely to be unassigned
	// THISIS("var::var(var&& rhs) noexcept")
	// ISASSIGNED(rhs)
}

// ctor for char
// use initializers since cannot fail (but could find how to init the char1)
var::var(const char char1) noexcept
	: var_str(1, char1),
	  var_typ(VARTYP_STR) {}

// constructor for char*
// use initializers since cannot fail unless out of memory
var::var(const char* cstr1)
	: var_str(cstr1),
	  var_typ(VARTYP_STR) {
	//std::cout << "ctor char* :" <<var_str << std::endl;

	// protect against null pointer
	// probably already crashed from var_str initialiser above
	if (cstr1 == 0) {
		// THISIS("var::var(const char* cstr1)")
		throw MVInvalidPointer("Null pointer in var(const char*)");
	}
}

#ifndef ALL_IN_ONE_STRING_CONSTRUCTOR

// constructor for const std::string
// just use initializers since cannot fail unless out of memory
var::var(const std::string& str1)
	// this would validate all strings as being UTF8?
	//: var_str(boost::locale::conv::utf_to_utf<char>(str1))
	: var_str(str1),
	  var_typ(VARTYP_STR) {}

// constructor for temporary std::string
// just use initializers since cannot fail unless out of memory
var::var(std::string&& str1) noexcept
	: var_str(std::move(str1)),
	  var_typ(VARTYP_STR) {}

#endif

// constructor for bool
// just use initializers since cannot fail
var::var(const bool bool1) noexcept
	: var_int(bool1),
	  var_typ(VARTYP_INT) {}

// constructor for int
// just use initializers since cannot fail
var::var(const int int1) noexcept
	: var_int(int1),
	  var_typ(VARTYP_INT) {}

// constructor for long long
// just use initializers since cannot fail
var::var(const long long longlong1) noexcept
	: var_int(longlong1),
	  var_typ(VARTYP_INT) {}

// constructor for double
// just use initializers since cannot fail
var::var(const double double1) noexcept
	: var_dbl(double1),
	  var_typ(VARTYP_DBL) {}

// ctor for memory block
// dont use initialisers and TODO protect against out of memory in expansion to string
var::var(const char* charstart, const size_t nchars)
	: var_str(charstart, nchars),
	  var_typ(VARTYP_STR) {}

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

// UNARY OPERATORS
/////////////////

// copy assignment
// var1 = var2
// The assignment operator should always return a reference to *this.
// cant be (const var& rhs) because seems to cause a problem with var1=var2 in function parameters
// unfortunately causes problem of passing var by value and thereby unnecessary contruction
// see also ^= etc
var& var::operator=(const var& rhs)
{
	THISIS("var& var::operator= (const var& rhs)")
	THISISDEFINED()//could be skipped for speed?
	ISASSIGNED(rhs)

	//std::clog << "copy assignment" <<std::endl;

	// important not to self assign
	if (this == &rhs)
		return *this;

	// copy everything across
	var_str = rhs.var_str;
	var_dbl = rhs.var_dbl;
	var_int = rhs.var_int;
	var_typ = rhs.var_typ;

	return *this;
}

// move assignment
// var = temporary var
var& var::operator=(var&& rhs) noexcept
{
	// skip this for speed?
	// THISIS("var& var::operator= (var rhs)")
	// THISISDEFINED()

	// skip this for speed since temporararies are unlikely to be unassigned
	// THISIS("var::var(var&& rhs) noexcept")
	// ISASSIGNED(rhs)

	//std::clog << "move assignment" <<std::endl;

	// important not to self assign
	if (this == &rhs)
		return *this;

	// move everything over
	var_str = std::move(rhs.var_str);
	var_dbl = rhs.var_dbl;
	var_int = rhs.var_int;
	var_typ = rhs.var_typ;

	return *this;
}

//=int
// The assignment operator should always return a reference to *this.
var& var::operator=(const int int1) {
	// THISIS("var& var::operator= (const int int1)")
	// protect against unlikely syntax as follows:
	// var undefinedassign=undefinedassign=123';
	// !!RISK NOT CHECKING TO SPEED THINGS UP SINCE SEEMS TO WORK IN MSVC AND GCC
	// THISISDEFINED()

	var_int = int1;
	var_typ = VARTYP_INT;  // reset to one unique type

	return *this;
}

//=double
// The assignment operator should always return a reference to *this.
var& var::operator=(const double double1) {
	// THISIS("var& var::operator= (const double double1)")
	// protect against unlikely syntax as follows:
	// var undefinedassign=undefinedassign=9.9';
	// !!RISK NOT CHECKING TO SPEED THINGS UP SINCE SEEMS TO WORK IN MSVC AND GCC
	// THISISDEFINED()

	var_dbl = double1;
	var_typ = VARTYP_DBL;  // reset to one unique type

	return *this;
}

//=char
// The assignment operator should always return a reference to *this.
var& var::operator=(const char char2) {

	THISIS("var& var::operator= (const char char2)")
	// protect against unlikely syntax as follows:
	// var undefinedassign=undefinedassign=L'X';
	// this causes crash due to bad memory access due to setting string that doesnt exist
	// slows down all string settings so consider NOT CHECKING in production code
	THISISDEFINED()	 // ALN:TODO: this definition kind of misleading, try to find
	// ALN:TODO: or change name to something like: THISISNOTDEAD :)
	// ALN:TODO: argumentation: var with mvtyp=0 is NOT defined

	var_str = char2;
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//=char*
// The assignment operator should always return a reference to *this.
var& var::operator=(const char* char2) {
	THISIS("var& var::operator= (const char* char2)")
	// protect against unlikely syntax as follows:
	// var undefinedassign=undefinedassign="xxx";
	// this causes crash due to bad memory access due to setting string that doesnt exist
	// slows down all string settings so consider NOT CHECKING in production code
	THISISDEFINED()

	var_str = char2;
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//=std::string variable (lvalue)
// The assignment operator should always return a reference to *this.
var& var::operator=(const std::string& string2) {

	THISIS("var& var::operator= (const std::string& string2)")
	// protect against unlikely syntax as follows:
	// var undefinedassign=undefinedassign=std::string("xxx"";
	// this causes crash due to bad memory access due to setting string that doesnt exist
	// slows down all string settings so consider NOT CHECKING in production code
	THISISDEFINED()
	var_str = string2;
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}
//=std::string temporary (rvalue)
// The assignment operator should always return a reference to *this.
var& var::operator=(const std::string&& string2) {

	THISIS("var& var::operator= (const std::string&& string2)")
	// protect against unlikely syntax as follows:
	// var undefinedassign=undefinedassign=std::string("xxx"";
	// this causes crash due to bad memory access due to setting string that doesnt exist
	// slows down all string settings so consider NOT CHECKING in production code
	THISISDEFINED()
	var_str = std::move(string2);
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}
//^= is not templated since slightly slower to go through the creation of an var()

//^=var
// The assignment operator should always return a reference to *this.
var& var::operator^=(const var& rhs) {
	THISIS("var& var::operator^=(const var& rhs)")
	THISISSTRING()
	ISSTRING(rhs)

	// tack it onto our string
	var_str += rhs.var_str;
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//^=int
// The assignment operator should always return a reference to *this.
var& var::operator^=(const int int1) {
	THISIS("var& var::operator^= (const int int1)")
	THISISSTRING()

	// var_str+=var(int1).var_str;
	var_str += intToString(int1);
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//^=double
// The assignment operator should always return a reference to *this.
var& var::operator^=(const double double1) {
	THISIS("var& var::operator^= (const double double1)")
	THISISSTRING()

	// var_str+=var(int1).var_str;
	var_str += dblToString(double1);
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//^=char
// The assignment operator should always return a reference to *this.
var& var::operator^=(const char char1) {
	THISIS("var& var::operator^= (const char char1)")
	THISISSTRING()

	// var_str+=var(int1).var_str;
	var_str += char1;
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//^=char*
// The assignment operator should always return a reference to *this.
var& var::operator^=(const char* char1) {
	THISIS("var& var::operator^= (const char* char1)")
	THISISSTRING()

	// var_str+=var(int1).var_str;
	// var_str+=std::string(char1);
	var_str += char1;
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//^=std::string
// The assignment operator should always return a reference to *this.
var& var::operator^=(const std::string& string1) {
	THISIS("var& var::operator^= (const std::string string1)")
	THISISSTRING()

	// var_str+=var(int1).var_str;
	var_str += string1;
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

// not handled by inbuilt conversion of var to long long& on the rhs

//#ifndef HASINTREFOP
//#else

// You must *not* make the postfix version return the 'this' object by reference; you have been
// warned.

// not returning void so is usable in expressions
// int argument indicates that this is POSTFIX override v++
var var::operator++(int) {
	THISIS("var var::operator++ (int)")
	// full check done below to avoid double checking number type
	THISISDEFINED()

	var priorvalue;

tryagain:
	// prefer int since ++ nearly always on integers
	if (var_typ & VARTYP_INT) {
		if (var_int == std::numeric_limits<mvint_t>::max())
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
var var::operator--(int) {
	THISIS("var var::operator-- (int)")
	// full check done below to avoid double checking number type
	THISISDEFINED()

	var priorvalue;

tryagain:
	// prefer int since -- nearly always on integers
	if (var_typ & VARTYP_INT) {
		if (var_int == std::numeric_limits<mvint_t>::min())
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
var& var::operator++() {
	THISIS("var var::operator++ ()")
	// full check done below to avoid double checking number type
	THISISDEFINED()

tryagain:
	// prefer int since -- nearly always on integers
	if (var_typ & VARTYP_INT) {
		if (var_int == std::numeric_limits<mvint_t>::max())
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
var& var::operator--() {
	THISIS("var& var::operator-- ()")
	// full check done below to avoid double checking number type
	THISISDEFINED()

tryagain:
	// prefer int since -- nearly always on integers
	if (var_typ & VARTYP_INT) {
		if (var_int == std::numeric_limits<mvint_t>::min())
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
var& var::operator+=(int int1) {
	THISIS("var& var::operator+= (int int1)")
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
	throw MVNonNumeric(substr(1, 20) ^ "+= ");
}

//-=var (very similar to version with on rhs)
// provided to disambiguate syntax like var1+=var2
var& var::operator-=(int int1) {
	THISIS("var& var::operator-= (int int1)")
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
	throw MVNonNumeric(substr(1, 20) ^ "-= ");
}

// allow varx+=1.5 to compile
var& var::operator+=(double dbl1) {
	(*this) += var(dbl1);
	return *this;
}

var& var::operator-=(double dbl1) {
	(*this) -= var(dbl1);
	return *this;
}

//+=var
var& var::operator+=(const var& rhs) {
	THISIS("var& var::operator+= (const var& rhs)")
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
	throw MVNonNumeric(substr(1, 20) ^ "+= ");
}

//-=var
var& var::operator-=(const var& rhs) {
	THISIS("var& var::operator-= (const var& rhs)")
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
	throw MVNonNumeric(substr(1, 20) ^ "-= ");
}

//#endif

/* more accurate way of comparing two decimals using EPSILON and ulp
   not using it simply to make better emulation of pick/arev at least initially

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

inline bool almost_equal(double x, mvint_t y, int ulp)
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
	//crude arev and pick(?) method
	return (std::abs(x - y) < SMALLEST_NUMBER);
}

// almost identical code in MVeq and MVlt except where noted
// NOTE doubles compare only to 0.0001 accuracy)
DLL_PUBLIC bool MVeq(const var& lhs, const var& rhs) {
	THISIS("bool MVeq(const var& lhs,const var& rhs)")
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
			if (lhs.var_str.length() == 0)
				// different from MVlt
				return false;
			if (rhs.var_str.length() == 0)
				// SAME as MVlt
				return false;
			// otherwise go on to test numerically then literally
		} else {
			// if rhs isnt a string and lhs is empty then eq is false
			//(after checking that rhs is actually assigned)
			if (lhs.var_str.length() == 0) {
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
		if ((rhs.var_typ & VARTYP_STR) && (rhs.var_str.length() == 0)) {
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
DLL_PUBLIC bool MVlt(const var& lhs, const var& rhs) {
	THISIS("bool MVlt(const var& lhs,const var& rhs)")
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
			if (lhs.var_str.length() == 0)
				// different from MVeq
				return true;
			if (rhs.var_str.length() == 0)
				// SAME as MVeq
				return false;
			// otherwise go on to test numerically then literally
		} else {
			// if rhs isnt a string and lhs is empty then eq is false
			// after checking that rhs is actually assigned
			if (lhs.var_str.length() == 0) {
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
		if ((rhs.var_typ & VARTYP_STR) && (rhs.var_str.length() == 0)) {
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
DLL_PUBLIC bool MVlt(const var& lhs, const int int2) {
	THISIS("bool MVlt(const var& lhs,const int int2)")
	ISDEFINED(lhs)

	// NB empty string is always less than anything except another empty string

	// 1. both empty or identical strings returns eq. one empty results false
	if (lhs.var_typ & VARTYP_STR) {
		// if rhs isnt a string and lhs is empty then eq is false
		// after checking that rhs is actually assigned
		if (lhs.var_str.length() == 0) {
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
	return lhs.var_str < intToString(int2);
}

// similar to MVeq and MVlt - this is the int<var version for speed
// NOTE doubles compare only to 0.0001 accuracy)
DLL_PUBLIC bool MVlt(const int int1, const var& rhs) {
	THISIS("bool MVlt(const int int1,const var& rhs)")
	ISDEFINED(rhs)

	// NB empty string is always less than anything except another empty string

	// 1. both empty or identical strings returns eq. one empty results false
	if (rhs.var_typ & VARTYP_STR) {
		if (rhs.var_str.length() == 0)
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
	return intToString(int1) < rhs.var_str;
}
// SEE ALSO MV2.CPP

//+var
//DLL_PUBLIC var operator+(const var& var1)
var MVplus(const var& var1) {
	THISIS("var operator+(const var& var1)")
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
	throw MVNonNumeric("+(" ^ var1.substr(1, 20) ^ ")");
}

//-var (identical to +var above except for two additional - signs)
//DLL_PUBLIC var operator-(const var& var1)
var MVminus(const var& var1) {
	THISIS("var operator-(const var& var1)")
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
	throw MVNonNumeric("+(" ^ var1.substr(1, 20) ^ ")");
}

//! var
//DLL_PUBLIC bool operator!(const var& var1)
bool MVnot(const var& var1) {
	THISIS("bool operator!(const var& var1)")
	ISASSIGNED(var1)

	// might need converting to work on void pointer
	// if bool replaced with void* (or made explicit instead of implict)
	// is there really any difference since the bool and void operators are defined identically?
	// return !(bool)(var1);
	return !(void*)(var1);
}

var MVadd(const var& lhs, const var& rhs) {
	THISIS("var operator+(const var& lhs,const var& rhs)")
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

var MVsub(const var& lhs, const var& rhs) {
	THISIS("var operator-(const var& lhs,const var& rhs)")
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

var MVmul(const var& lhs, const var& rhs) {
	THISIS("var operator*(const var& lhs,const var& rhs)")
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

var MVdiv(const var& lhs, const var& rhs) {
	THISIS("var operator/(const var& lhs,const var& rhs)")
	ISNUMERIC(lhs)
	ISNUMERIC(rhs)

	// always returns a double

	double bottom = (rhs.var_typ & VARTYP_INT) ? double(rhs.var_int) : rhs.var_dbl;
	if (!bottom)
		throw MVDivideByZero("div('" ^ lhs.substr(1, 20) ^ "', '" ^ rhs.substr(1, 20) ^
							 "')");

	double top = (lhs.var_typ & VARTYP_INT) ? double(lhs.var_int) : lhs.var_dbl;
	return top / bottom;
}

var MVmod(const var& lhs, const var& rhs) {
	THISIS("var operator%(const var& lhs,const var& rhs)")
	ISNUMERIC(lhs)
	ISNUMERIC(rhs)

	// integer version;
	if (lhs.var_typ & VARTYP_INT && rhs.var_typ & VARTYP_INT) {
		if (!rhs.var_int)
			throw MVDivideByZero("div('" ^ lhs.substr(1, 20) ^ "', '" ^
								 rhs.substr(1, 20) ^ "')");
		return lhs.var_int % rhs.var_int;
	}

	double bottom = (rhs.var_typ & VARTYP_INT) ? double(rhs.var_int) : rhs.var_dbl;
	if (!bottom)
		throw MVDivideByZero("div('" ^ lhs.substr(1, 20) ^ "', '" ^ rhs.substr(1, 20) ^
							 "')");

	double top = (lhs.var_typ & VARTYP_INT) ? double(lhs.var_int) : lhs.var_dbl;
	return exodusmodulus(top, bottom);
}

// var^var we reassign the logical xor operator ^ to be string concatenate!!!
// slightly wrong precedence but at least we have a reliable concat operator to replace the + which
// is now reserved for ADDITION
var MVcat(const var& lhs, const var& rhs) {
	THISIS("var operator^(const var& lhs,const var& rhs)")
	ISSTRING(lhs)
	ISSTRING(rhs)

	return lhs.var_str + rhs.var_str;
}

/* treat as "hidden friends"

//PLUS
DLL_PUBLIC var operator+(const var& lhs, const var& rhs) { return MVadd(lhs, rhs); }
DLL_PUBLIC var operator+(const var& lhs, const char* char2) { return MVadd(lhs, var(char2)); }
DLL_PUBLIC var operator+(const var& lhs, const int int2) { return MVadd(lhs, var(int2)); }
DLL_PUBLIC var operator+(const var& lhs, const double double2) { return MVadd(lhs, var(double2)); }
//DLL_PUBLIC var operator+(const var& lhs, const bool bool2) { return MVadd(lhs, var(bool2)); }
DLL_PUBLIC var operator+(const char* char1, const var& rhs) { return MVadd(var(char1), rhs); }
DLL_PUBLIC var operator+(const int int1, const var& rhs) { return MVadd(var(int1), rhs); }
DLL_PUBLIC var operator+(const double double1, const var& rhs) { return MVadd(var(double1), rhs); }
//DLL_PUBLIC var operator+(const bool bool1, const var& rhs) { return MVadd(var(bool1), rhs); }
//rvalues
DLL_PUBLIC var operator+(var&& lhs, const var& rhs) { return lhs+=rhs; }
DLL_PUBLIC var operator+(var&& lhs, const char* char2) { return lhs+=char2; }
DLL_PUBLIC var operator+(var&& lhs, const int int2) { return lhs+=int2; }
DLL_PUBLIC var operator+(var&& lhs, const double double2) { return lhs+=double2; }
//DLL_PUBLIC var operator+(var&& lhs, const bool bool2) { return lhs+=bool2; }
//DLL_PUBLIC var operator+(const char* char1, var&& rhs) { return rhs+=char1; }
//DLL_PUBLIC var operator+(const int int1, var&& rhs) { return rhs+=int1; }
//DLL_PUBLIC var operator+(const double double1, var&& rhs) { return rhs+=double1; }
//DLL_PUBLIC var operator+(const bool bool1, var&& rhs) { return rhs+=bool1; }

//MINUS
DLL_PUBLIC var operator-(const var& lhs, const var& rhs) { return MVsub(lhs, rhs); }
DLL_PUBLIC var operator-(const var& lhs, const char* char2) { return MVsub(lhs, var(char2)); }
DLL_PUBLIC var operator-(const var& lhs, const int int2) { return MVsub(lhs, var(int2)); }
DLL_PUBLIC var operator-(const var& lhs, const double double2) { return MVsub(lhs, var(double2)); }
//DLL_PUBLIC var operator-(const var& lhs, const bool bool2) { return MVsub(lhs, var(bool2)); }
DLL_PUBLIC var operator-(const char* char1, const var& rhs) { return MVsub(var(char1), rhs); }
DLL_PUBLIC var operator-(const int int1, const var& rhs) { return MVsub(var(int1), rhs); }
DLL_PUBLIC var operator-(const double double1, const var& rhs) { return MVsub(var(double1), rhs); }
//DLL_PUBLIC var operator-(const bool bool1, const var& rhs) { return MVsub(var(bool1), rhs); }
//rvalues
DLL_PUBLIC var operator-(var&& lhs, const var& rhs) { return lhs-=rhs; }
DLL_PUBLIC var operator-(var&& lhs, const char* char2) { return lhs-=char2; }
DLL_PUBLIC var operator-(var&& lhs, const int int2) { return lhs-=int2; }
DLL_PUBLIC var operator-(var&& lhs, const double double2) { return lhs-=double2; }

//MULTIPLY
DLL_PUBLIC var operator*(const var& lhs, const var& rhs) { return MVmul(lhs, rhs); }
DLL_PUBLIC var operator*(const var& lhs, const char* char2) { return MVmul(lhs, var(char2)); }
DLL_PUBLIC var operator*(const var& lhs, const int int2) { return MVmul(lhs, var(int2)); }
DLL_PUBLIC var operator*(const var& lhs, const double double2) { return MVmul(lhs, var(double2)); }
//DLL_PUBLIC var operator*(const var& lhs, const bool bool2) { return MVmul(lhs, var(bool2)); }
DLL_PUBLIC var operator*(const char* char1, const var& rhs) { return MVmul(var(char1), rhs); }
DLL_PUBLIC var operator*(const int int1, const var& rhs) { return MVmul(var(int1), rhs); }
DLL_PUBLIC var operator*(const double double1, const var& rhs) { return MVmul(var(double1), rhs); }
//DLL_PUBLIC var operator*(const bool bool1, const var& rhs) { return MVmul(var(bool1), rhs); }
//rvalues - pending implementation of var*=
//DLL_PUBLIC var operator*(var&& lhs, const var& rhs) { return lhs*=rhs; }
//DLL_PUBLIC var operator*(var&& lhs, const char* char2) { return lhs*=char2; }
//DLL_PUBLIC var operator*(var&& lhs, const int int2) { return lhs*=int2; }
//DLL_PUBLIC var operator*(var&& lhs, const double double2) { return lhs*=double2; }

//DIVIDE
DLL_PUBLIC var operator/(const var& lhs, const var& rhs) { return MVdiv(lhs, rhs); }
DLL_PUBLIC var operator/(const var& lhs, const char* char2) { return MVdiv(lhs, var(char2)); }
DLL_PUBLIC var operator/(const var& lhs, const int int2) { return MVdiv(lhs, var(int2)); }
DLL_PUBLIC var operator/(const var& lhs, const double double2) { return MVdiv(lhs, var(double2)); }
// disallow divide by boolean to prevent possible runtime divide by zero
// DLL_PUBLIC var operator/ (const var&     lhs    ,const bool     bool2   ){return
// MVdiv(lhs,var(bool2)  );}
DLL_PUBLIC var operator/(const char* char1, const var& rhs) { return MVdiv(var(char1), rhs); }
DLL_PUBLIC var operator/(const int int1, const var& rhs) { return MVdiv(var(int1), rhs); }
DLL_PUBLIC var operator/(const double double1, const var& rhs) { return MVdiv(var(double1), rhs); }
//DLL_PUBLIC var operator/(const bool bool1, const var& rhs) { return MVdiv(var(bool1), rhs); }

//MODULO
DLL_PUBLIC var operator%(const var& lhs, const var& rhs) { return MVmod(lhs, rhs); }
DLL_PUBLIC var operator%(const var& lhs, const char* char2) { return MVmod(lhs, var(char2)); }
DLL_PUBLIC var operator%(const var& lhs, const int int2) { return MVmod(lhs, var(int2)); }
DLL_PUBLIC var operator%(const var& lhs, const double double2) { return MVmod(lhs, var(double2)); }
// disallow divide by boolean to prevent possible runtime divide by zero
// DLL_PUBLIC var operator% (const var&     lhs    ,const bool    bool2   ){return
// MVmod(lhs,var(bool2)  );}
DLL_PUBLIC var operator%(const char* char1, const var& rhs) { return MVmod(var(char1), rhs); }
DLL_PUBLIC var operator%(const int int1, const var& rhs) { return MVmod(var(int1), rhs); }
DLL_PUBLIC var operator%(const double double1, const var& rhs) { return MVmod(var(double1), rhs); }
//DLL_PUBLIC var operator%(const bool bool1, const var& rhs) { return MVmod(var(bool1), rhs); }

//CONCATENATE
// NB do *NOT* support concatenate with bool or vice versa!!!
// to avoid compiler doing wrong precendence issue between ^ and logical operators
DLL_PUBLIC var operator^(const var& lhs, const var& rhs) { return MVcat(lhs, rhs); }
DLL_PUBLIC var operator^(const var& lhs, const char* char2) { return MVcat(lhs, var(char2)); }
DLL_PUBLIC var operator^(const var& lhs, const int int2) { return MVcat(lhs, var(int2)); }
DLL_PUBLIC var operator^(const var& lhs, const double double2) { return MVcat(lhs, var(double2)); }
DLL_PUBLIC var operator^(const char* char1, const var& rhs) { return MVcat(var(char1), rhs); }
DLL_PUBLIC var operator^(const int int1, const var& rhs) { return MVcat(var(int1), rhs); }
DLL_PUBLIC var operator^(const double double1, const var& rhs) { return MVcat(var(double1), rhs); }
//rvalues
DLL_PUBLIC var operator^(var&& lhs, const var& rhs) { return lhs^=rhs; }
DLL_PUBLIC var operator^(var&& lhs, const char* char2) { return lhs^=char2; }
DLL_PUBLIC var operator^(var&& lhs, const int int2) { return lhs^=int2; }
DLL_PUBLIC var operator^(var&& lhs, const double double2) { return lhs^=double2; }

*/

//#if defined __MINGW32__
// allow use of cout<<var
//pass by value (make a copy) because we are going to convert FM to ^ etc
//TODO provide a version that works on temporaries?
//DLL_PUBLIC
std::ostream& operator<<(std::ostream& ostream1, var var1) {
	THISIS("std::ostream& operator<< (std::ostream& ostream1, var var1)")
	ISSTRING(var1)

	//replace various unprintable field marks with unusual ASCII characters
	//leave ESC as \x1B because it is used to control ANSI terminal control sequences
	//std::string str = "\x1A\x1B\x1C\x1D\x1E\x1F";
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

std::istream& operator>>(std::istream& istream1, var& var1) {
	THISIS("std::istream& operator>> (std::istream& istream1,var& var1)")
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

inline double exodusmodulus(const double top, const double bottom) {
	return top - double(int(top / bottom) * bottom);
}

std::string intToString(int int1) {

	//600ns
	// NB plain stringstream causes a memory leak in msvc8 before sp1
	//std::ostringstream ss;
	//ss << int1;
	//// debuggFUNCTION&& cout<<"intToString(int "<<int1<<") returns '"<<s<<"'\n";
	//return ss.str();

	//40ns
	return std::to_string(int1);
}

// TODO ensure locale doesnt produce like 123.456,78
// see 1997 http://www.cantrip.org/locale.html
std::string dblToString(double double1) {

	//std::cout << abs(double1) << std::endl;
	//std::cout << double(0.000'000'001) << std::endl;
	//std::cout << (std::abs(double1)<double(0.000'000'001)) << std::endl;

	/*

	//fixed to precision six
	//return std::to_string(double1);

	// see intToString for choice of ostringstream for implementation
	// NB plain stringstream causes a memory leak in msvc8 before sp1
	std::ostringstream stringstream1;
	//if precision is changed, also change testmain

	//stringstream1.precision(16);
	//use precision 14 to avoid 1.1-1 = 1.000000000000001
    stringstream1 << std::fixed;

	std::string str1;
	double abs_double1 = std::abs(double1);

	//evade scientific format for small numbers
	if (abs_double1 < 0.0001d) {

		//treat very small numbers as zero
		//if (std::abs(double1)<double(0.000'000'000'1))
		//if (std::abs(double1)<double(0.000'000'000'000'001))
		if (std::abs(double1)<0.000'000'000'000'1d)
			return "0";

		stringstream1.precision(14-std::log10(double1));
		//stringstream1.precision(14);

	} else if (abs_double1>1000000) {
		stringstream1.precision(14-std::log10(double1));
		//stringstream1.precision(6);

	} else {
		//stringstream1.precision(14-std::log10(double1));
		stringstream1.precision(14);
	}

	//std::clog << double1 << " " << std::log10(double1) << std::endl;
    stringstream1 << double1;
    str1 = stringstream1.str();
    while (str1.back() == '0')
	    str1.pop_back();
	if (str1.back() == '.')
		str1.pop_back();

	return str1;
	*/

	/*
	std::vector<char> buf(64); // note +1 for null terminator
	//std::snprintf(&buf[0], buf.size(), "%.16g", double1);
	std::snprintf(&buf[0], buf.size(), "%.17g", double1);

	return std::string(buf.data());
	*/

	int minus = double1 < 0 ? 1 : 0;

	//Precision on scientific output allows the full precision to obtained
	//regardless of the size of the number (big or small)
	//
	//Precision on fixed output only controls the precision after the decimal point
	//so the precision needs to be huge to cater for very small numbers eg < 10E-20

#ifdef USE_RYU

	//std::cout << "ryu_printf decimal oconv" << std::endl;

	std::string s;
	s.resize(24);

#define USE_RYU_D2S
#ifdef USE_RYU_D2S  // 500ns using ryu d2s() which always outputs scientific even 0E0 for zero.

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

#elif 0 //740ns ryu d2exp() always output scientific

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

#else //???ns ryu d2fixed() always output fixed decimal point with fixed precision

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

#else //1800ns

	//std::cout << "std:sstream decimal oconv" << std::endl;

	std::ostringstream ss;

	//EITHER use 15 for which 64-bit IEEE 754 type double guarantees
	//roundtrip double/text/double for 15 decimal digits
	//
	//OR use precision 14 to avoid 1.1-1 = 1.000000000000001
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

#endif //USE_RYU

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
		if (s.size() == 0 || s == "-")
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

var backtrace();

MVError::MVError(const var& description_)
	: description(description_) {
	// capture the stack at point of creation i.e. when thrown
	this->stack = backtrace();
	((description.assigned() ? description : "") ^ "\n" ^ stack.convert(FM, "\n") ^ "\n").put(std::cerr);

	if (description.osgetenv("EXO_DEBUG")) {
		description.debug();
	}
}

MVUnassigned ::MVUnassigned(const var& var1)
	: MVError("MVUnassigned:" ^ var1) {}
MVDivideByZero ::MVDivideByZero(const var& var1)
	: MVError("MVDivideByZero:" ^ var1) {}
MVNonNumeric ::MVNonNumeric(const var& var1)
	: MVError("MVNonNumeric:" ^ var1) {}
MVIntOverflow ::MVIntOverflow(const var& var1)
	: MVError("MVIntOverflow:" ^ var1) {}
MVIntUnderflow ::MVIntUnderflow(const var& var1)
	: MVError("MVIntUnderflow:" ^ var1) {}
MVUndefined ::MVUndefined(const var& var1)
	: MVError("MVUndefined:" ^ var1) {}
MVOutOfMemory ::MVOutOfMemory(const var& var1)
	: MVError("MVOutOfMemory:" ^ var1) {}
MVInvalidPointer ::MVInvalidPointer(const var& var1)
	: MVError("MVInvalidPointer:" ^ var1) {}
MVDBException ::MVDBException(const var& var1)
	: MVError("MVDBException:" ^ var1) {}
MVNotImplemented ::MVNotImplemented(const var& var1)
	: MVError("MVNotImplemented:" ^ var1) {}
MVDebug ::MVDebug(const var& var1)
	: MVError("MVDebug" ^ var1) {}
MVStop ::MVStop(const var& var1)
	: description(var1) {}
MVAbort ::MVAbort(const var& var1)
	: description(var1) {}
MVAbortAll ::MVAbortAll(const var& var1)
	: description(var1) {}
MVLogoff ::MVLogoff(const var& var1)
	: description(var1) {}
MVArrayDimensionedZero ::MVArrayDimensionedZero()
	: MVError("MVArrayDimensionedZero:") {}
MVArrayIndexOutOfBounds ::MVArrayIndexOutOfBounds(const var& var1)
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
