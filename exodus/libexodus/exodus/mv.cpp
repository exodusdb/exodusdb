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

#define EXO_MV_CPP // indicates globals are to be defined (omit extern keyword)
#include <exodus/mv.h>
//#include <exodus/mvutf.h>
#include <exodus/mvexceptions.h>

namespace exodus
{

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
var::~var()
{
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
    : var_typ(VARTYP_UNA)
{
	//std::cout << "ctor()" << std::endl;

	// int xyz=3;
	// WARNING neither initialisers nor constructors are called in the following case !!!
	// var xxx=xxx.somefunction()
	// known as "undefined usage of uninitialised variable";
	// and not even a compiler warning in msvc8 or g++4.1.2

	// so the following test is put everywhere to protect against this type of accidental
	// programming if (var_typ&VARTYP_MASK) 	throw MVUndefined("funcname()"); should really
	// ensure a magic number and not just HOPE for some binary digits above bottom four 0-15
	// decimal 1111binary this could be removed in production code perhaps

	// debuggCONSTRUCT&&cout<<"CONSTRUCT: var()\n";

	// not a pointer anymore for speed
	// priv=new pimpl;

	// moved here from pimpl constructor
	// moved up to initializer
	// var_typ=VARTYP_UNA;

}

/* =default
// copy constructor
var::var(const var& rhs)
    : var_str(rhs.var_str),
      var_int(rhs.var_int),
      var_dbl(rhs.var_dbl),
      var_typ(rhs.var_typ)
{
	  // checking after copy for speed
	  // use initializers for speed and only check afterwards if copiedvar was assigned
	  THISIS("var::var(const var& rhs)") ISASSIGNED(rhs)

	  // not a pointer anymore for speed
	  // priv=new pimpl;
}*/

/* = default
// move constructor
var::var(const var&& rhs) noexcept
    : var_str(std::move(rhs.var_str)),
      var_int(rhs.var_int),
      var_dbl(rhs.var_dbl),
      var_typ(rhs.var_typ)
{
	//std::cout << "copy ctor var&" << std::endl;

	// skip this for speed since temporararies are unlikely to be unassigned
	// THISIS("var::var(const var&& rhs)")
	// ISASSIGNED(rhs)
}*/

// constructor for char*
// use initializers since cannot fail unless out of memory
var::var(const char* cstr1)
    : var_str(cstr1),
      var_typ(VARTYP_STR)
{
	//std::cout << "ctor char* :" <<var_str << std::endl;

	// protect against null pointer
	// probably already crashed from var_str initialiser above
	if (cstr1 == 0)
	{
		// THISIS("var::var(const char* cstr1)")
		throw MVInvalidPointer("Null pointer in var(const char*)");
	}
}

// constructor for std::string
// just use initializers since cannot fail unless out of memory
var::var(const std::string& str1)
    // this would validate all strings as being UTF8?
    //: var_str(boost::locale::conv::utf_to_utf<char>(str1))
    : var_str(str1),
      var_typ(VARTYP_STR)
{}

// constructor for bool
// just use initializers since cannot fail
var::var(const bool bool1) noexcept
    : var_int(bool1),
      var_typ(VARTYP_INT)
{}

// constructor for int
// just use initializers since cannot fail
var::var(const int int1) noexcept
    : var_int(int1),
      var_typ(VARTYP_INT)
{}

// constructor for long long
// just use initializers since cannot fail
var::var(const long long longlong1) noexcept
    : var_int(longlong1),
      var_typ(VARTYP_INT)
{}

// constructor for double
// just use initializers since cannot fail
var::var(const double double1) noexcept
    : var_dbl(double1),
      var_typ(VARTYP_DBL)
{}

// ctor for char
// use initializers since cannot fail (but could find how to init the char1)
var::var(const char char1) noexcept
    : var_str(1, char1),
      var_typ(VARTYP_STR)
{}

// ctor for memory block
// dont use initialisers and TODO protect against out of memory in expansion to string
var::var(const char* charstart, const size_t nchars)
    : var_str(charstart, nchars),
      var_typ(VARTYP_STR)
{}

// EXPLICIT AND AUTOMATIC CONVERSIONS
////////////////////////////////////

// someone recommends not to create more than one automatic converter
// to avoid the compiler error "ambiguous conversion"
//(explicit means it is not automatic)

// allow conversion to string (IS THIS USED FOR ANYTHING AT THE MOMENT?
// allows the usage of any string function
var::operator std::string() const
{
	THISIS("var::operator std::string")
	THISISSTRING()
	return var_str;
}

var::operator void*() const
{
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
var::operator bool() const
{
	return toBool();
}

/*
var::operator const char*() const
{
	return toString().c_str();
}
*/

#ifndef HASINTREFOP
var::operator int() const
{
	THISIS("var::operator int() const")
	THISISINTEGER()
	return int(var_int);
}

var::operator double() const
{
	THISIS("var::operator double() const")
	THISISDECIMAL()
	return var_dbl;
}

#else
var::operator int&() const
{
	THISIS("var::operator mv_int_t&()")
	THISISINTEGER()
	// TODO check that converting mvint_t (which is long long) to int doesnt cause any practical
	// problems) PROBABLY WILL! since we are returning a non const reference which allows
	// callers to set the int directly then clear any decimal and string cache flags which would
	// be invalid after setting the int alone
	//var_typ |= VARTYP_INT;
	return (int&)var_int;
}
var::operator double&() const
{
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
//=var
// The assignment operator should always return a reference to *this.
// cant be (const var& rhs) because seems to cause a problem with var1=var2 in function parameters
// unfortunately causes problem of passing var by value and thereby unnecessary contruction
// see also ^= etc
/* = default
var& var::operator=(const var& rhs)
{
	THISIS("var& var::operator= (const var& rhs)")
	THISISDEFINED()
	ISASSIGNED(rhs)

	// very important not to self assign!!!
	if (this == &rhs)
		return *this;

	// copy everything across
	var_str = rhs.var_str;
	var_dbl = rhs.var_dbl;
	var_int = rhs.var_int;
	var_typ = rhs.var_typ;

	return *this;
}*/

/* =default
// move assignment
var& var::operator=(const var&& rhs) noexcept
{
	//	THISIS("var& var::operator= (var rhs)")
	//	THISISDEFINED()

	// copy-and-swap idiom
	// 1. create a temporary copy of the variable to be copied BEFORE even arriving in this
	// routine
	// 2. swap this and temporary copy
	// 3. what was this is now in the temp will now be destructed correctly on exit
	using std::swap;
	swap(var_str, rhs.var_str);
	// just grab the rest
	var_dbl = rhs.var_dbl;
	var_int = rhs.var_int;
	var_typ = rhs.var_typ;

	return *this;
}*/

//=int
// The assignment operator should always return a reference to *this.
var& var::operator=(const int int1)
{
	// THISIS("var& var::operator= (const int int1)")
	// protect against unlikely syntax as follows:
	// var undefinedassign=undefinedassign=123';
	// !!RISK NOT CHECKING TO SPEED THINGS UP SINCE SEEMS TO WORK IN MSVC AND GCC
	// THISISDEFINED()

	var_int = int1;
	var_typ = VARTYP_INT; // reset to one unique type

	return *this;
}

//=double
// The assignment operator should always return a reference to *this.
var& var::operator=(const double double1)
{
	// THISIS("var& var::operator= (const double double1)")
	// protect against unlikely syntax as follows:
	// var undefinedassign=undefinedassign=9.9';
	// !!RISK NOT CHECKING TO SPEED THINGS UP SINCE SEEMS TO WORK IN MSVC AND GCC
	// THISISDEFINED()

	var_dbl = double1;
	var_typ = VARTYP_DBL; // reset to one unique type

	return *this;
}

//=char
// The assignment operator should always return a reference to *this.
var& var::operator=(const char char2)
{

	THISIS("var& var::operator= (const char char2)")
	// protect against unlikely syntax as follows:
	// var undefinedassign=undefinedassign=L'X';
	// this causes crash due to bad memory access due to setting string that doesnt exist
	// slows down all string settings so consider NOT CHECKING in production code
	THISISDEFINED() // ALN:TODO: this definition kind of misleading, try to find
			// ALN:TODO: or change name to something like: THISISNOTDEAD :)
			// ALN:TODO: argumentation: var with mvtyp=0 is NOT defined

	var_str = char2;
	var_typ = VARTYP_STR; // reset to one unique type

	return *this;
}

//=char*
// The assignment operator should always return a reference to *this.
var& var::operator=(const char* char2)
{
	THISIS("var& var::operator= (const char* char2)")
	// protect against unlikely syntax as follows:
	// var undefinedassign=undefinedassign="xxx";
	// this causes crash due to bad memory access due to setting string that doesnt exist
	// slows down all string settings so consider NOT CHECKING in production code
	THISISDEFINED()

	var_str = char2;
	var_typ = VARTYP_STR; // reset to one unique type

	return *this;
}

//=std::string
// The assignment operator should always return a reference to *this.
var& var::operator=(const std::string& string2)
{

	THISIS("var& var::operator= (const std::string& string2)")
	// protect against unlikely syntax as follows:
	// var undefinedassign=undefinedassign=std::string("xxx"";
	// this causes crash due to bad memory access due to setting string that doesnt exist
	// slows down all string settings so consider NOT CHECKING in production code
	THISISDEFINED()
	var_str = string2;
	var_typ = VARTYP_STR; // reset to one unique type

	return *this;
}
//^= is not templated since slightly slower to go through the creation of an var()

//^=var
// The assignment operator should always return a reference to *this.
var& var::operator^=(const var& rhs)
{
	THISIS("var& var::operator^=(const var& rhs)")
	THISISSTRING()
	ISSTRING(rhs)

	// tack it onto our string
	var_str += rhs.var_str;
	var_typ = VARTYP_STR; // reset to one unique type

	return *this;
}

//^=int
// The assignment operator should always return a reference to *this.
var& var::operator^=(const int int1)
{
	THISIS("var& var::operator^= (const int int1)")
	THISISSTRING()

	// var_str+=var(int1).var_str;
	var_str += intToString(int1);
	var_typ = VARTYP_STR; // reset to one unique type

	return *this;
}

//^=double
// The assignment operator should always return a reference to *this.
var& var::operator^=(const double double1)
{
	THISIS("var& var::operator^= (const double double1)")
	THISISSTRING()

	// var_str+=var(int1).var_str;
	var_str += dblToString(double1);
	var_typ = VARTYP_STR; // reset to one unique type

	return *this;
}

//^=char
// The assignment operator should always return a reference to *this.
var& var::operator^=(const char char1)
{
	THISIS("var& var::operator^= (const char char1)")
	THISISSTRING()

	// var_str+=var(int1).var_str;
	var_str += char1;
	var_typ = VARTYP_STR; // reset to one unique type

	return *this;
}

//^=char*
// The assignment operator should always return a reference to *this.
var& var::operator^=(const char* char1)
{
	THISIS("var& var::operator^= (const char* char1)")
	THISISSTRING()

	// var_str+=var(int1).var_str;
	// var_str+=std::string(char1);
	var_str += char1;
	var_typ = VARTYP_STR; // reset to one unique type

	return *this;
}

//^=std::string
// The assignment operator should always return a reference to *this.
var& var::operator^=(const std::string& string1)
{
	THISIS("var& var::operator^= (const std::string string1)")
	THISISSTRING()

	// var_str+=var(int1).var_str;
	var_str += string1;
	var_typ = VARTYP_STR; // reset to one unique type

	return *this;
}

// not handled by inbuilt conversion of var to long long& on the rhs

//#ifndef HASINTREFOP
//#else

// You must *not* make the postfix version return the 'this' object by reference; you have been
// warned.

// not returning void so is usable in expressions
// int argument indicates that this is POSTFIX override v++
var var::operator++(int)
{
	THISIS("var var::operator++ (int)")
	// full check done below to avoid double checking number type
	THISISDEFINED()

tryagain:
	// prefer int since ++ nearly always on integers
	if (var_typ & VARTYP_INT)
	{
		if (var_int == std::numeric_limits<mvint_t>::max())
			throw MVIntOverflow("operator++");
		var_int++;
		var_typ = VARTYP_INT; // reset to one unique type
	}
	else if (var_typ & VARTYP_DBL)
	{
		var_dbl++;
		var_typ = VARTYP_DBL; // reset to one unique type
	}
	else if (var_typ & VARTYP_STR)
	{
		// try to convert to numeric
		if (isnum())
			goto tryagain;

		THISISNUMERIC()
	}
	else
	{
		THISISNUMERIC()
	}

	// NO DO NOT! return *this ... postfix return a temporary!!! eg var(*this)
	return var(*this);
}

// not returning void so is usable in expressions
// int argument indicates that this is POSTFIX override v--
var var::operator--(int)
{
	THISIS("var var::operator-- (int)")
	// full check done below to avoid double checking number type
	THISISDEFINED()

tryagain:
	// prefer int since -- nearly always on integers
	if (var_typ & VARTYP_INT)
	{
		var_int--;
		var_typ = VARTYP_INT; // reset to one unique type
	}
	else if (var_typ & VARTYP_DBL)
	{
		var_dbl--;
		var_typ = VARTYP_DBL; // reset to one unique type
	}
	else if (var_typ & VARTYP_STR)
	{
		// try to convert to numeric
		if (isnum())
			goto tryagain;

		THISISNUMERIC()
	}
	else
	{
		THISISNUMERIC()
	}

	// NO DO NOT! return *this ... postfix must return a temporary!!! eg var(*this)
	return var(*this);
}

// not returning void so is usable in expressions
// no argument indicates that this is prefix override ++var
var& var::operator++()
{
	THISIS("var var::operator++ ()")
	// full check done below to avoid double checking number type
	THISISDEFINED()

tryagain:
	// prefer int since -- nearly always on integers
	if (var_typ & VARTYP_INT)
	{
		if (var_int == std::numeric_limits<mvint_t>::max())
			throw MVIntOverflow("operator++");
		var_int++;
		var_typ = VARTYP_INT; // reset to one unique type
	}
	else if (var_typ & VARTYP_DBL)
	{
		var_dbl++;
		var_typ = VARTYP_DBL; // reset to one unique type
	}
	else if (var_typ & VARTYP_STR)
	{
		// try to convert to numeric
		if (isnum())
			goto tryagain;

		THISISNUMERIC()
	}
	else
	{
		THISISNUMERIC()
	}

	// OK to return *this in prefix ++
	return *this;
}

// not returning void so is usable in expressions
// no argument indicates that this is prefix override --var
var& var::operator--()
{
	THISIS("var& var::operator-- ()")
	// full check done below to avoid double checking number type
	THISISDEFINED()

tryagain:
	// prefer int since -- nearly always on integers
	if (var_typ & VARTYP_INT)
	{
		var_int--;
		var_typ = VARTYP_INT; // reset to one unique type
	}
	else if (var_typ & VARTYP_DBL)
	{
		var_dbl--;
		var_typ = VARTYP_DBL; // reset to one unique type
	}
	else if (var_typ & VARTYP_STR)
	{
		// try to convert to numeric
		if (isnum())
			goto tryagain;

		THISISNUMERIC()
	}
	else
	{
		THISISNUMERIC()
	}

	// OK to return *this in prefix --
	return *this;
}

//+=var (very similar to version with on rhs)
// provided to disambiguate syntax like var1+=var2
var& var::operator+=(int int1)
{
	THISIS("var& var::operator+= (int int1)")
	THISISDEFINED()

tryagain:

	// dbl target
	// prefer double
	if (var_typ & VARTYP_DBL)
	{
		//+= int or dbl from source
		var_dbl += int1;
		var_typ = VARTYP_DBL; // reset to one unique type
		return *this;
	}

	// int target
	else if (var_typ & VARTYP_INT)
	{
		var_int += int1;
		var_typ = VARTYP_INT; // reset to one unique type
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
var& var::operator-=(int int1)
{
	THISIS("var& var::operator-= (int int1)")
	THISISDEFINED()

tryagain:
	// dbl target
	// prefer double
	if (var_typ & VARTYP_DBL)
	{
		var_dbl -= int1;
		var_typ = VARTYP_DBL; // reset to one unique type
		return *this;
	}

	// int target
	else if (var_typ & VARTYP_INT)
	{
		var_int -= int1;
		var_typ = VARTYP_INT; // reset to one unique type
		return *this;
	}

	// try to convert to numeric
	if (isnum())
		goto tryagain;

	THISISNUMERIC()
	throw MVNonNumeric(substr(1, 20) ^ "-= ");
}

// allow varx+=1.5 to compile
var& var::operator+=(double dbl1)
{
	(*this) += var(dbl1);
	return *this;
}

var& var::operator-=(double dbl1)
{
	(*this) -= var(dbl1);
	return *this;
}

//+=var
var& var::operator+=(const var& rhs)
{
	THISIS("var& var::operator+= (const var& rhs)")
	THISISDEFINED()
	ISNUMERIC(rhs)

tryagain:

	// dbl target
	// prefer double
	if (var_typ & VARTYP_DBL)
	{
		//+= int or dbl from source
		var_dbl += (rhs.var_typ & VARTYP_INT) ? rhs.var_int : rhs.var_dbl;
		var_typ = VARTYP_DBL; // reset to one unique type
		return *this;
	}

	// int target
	else if (var_typ & VARTYP_INT)
	{
		// int source
		if (rhs.var_typ & VARTYP_INT)
		{
			var_int += rhs.var_int;
			var_typ = VARTYP_INT; // reset to one unique type
			return *this;
		}
		// dbl source, convert target to dbl
		var_dbl = var_int + rhs.var_dbl;
		var_typ = VARTYP_DBL; // reset to one unique type
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
var& var::operator-=(const var& rhs)
{
	THISIS("var& var::operator-= (const var& rhs)")
	THISISDEFINED()
	ISNUMERIC(rhs)

tryagain:

	// int target
	if (var_typ & VARTYP_INT)
	{
		// int source
		if (rhs.var_typ & VARTYP_INT)
		{
			var_int -= rhs.var_int;
			var_typ = VARTYP_INT; // reset to one unique type
			return *this;
		}
		// dbl source, convert target to dbl
		var_dbl = var_int - rhs.var_dbl;
		var_typ = VARTYP_DBL; // reset to one unique type
		return *this;
	}

	// dbl target
	else if (var_typ & VARTYP_DBL)
	{
		//-= int or dbl from source
		var_dbl -= (rhs.var_typ & VARTYP_INT) ? rhs.var_int : rhs.var_dbl;
		var_typ = VARTYP_DBL; // reset to one unique type
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

// almost identical between MVeq and MVlt except where noted (and doubles compare only to 0.0001 accuracy)
DLL_PUBLIC bool MVeq(const var& lhs, const var& rhs)
{
	THISIS("bool MVeq(const var& lhs,const var& rhs)")
	ISDEFINED(lhs)
	ISDEFINED(rhs)

	// NB empty string is always less than anything except another empty string

	// 1. BOTH EMPTY or IDENTICAL STRINGS returns TRUE one empty results false
	if (lhs.var_typ & VARTYP_STR)
	{
		if (rhs.var_typ & VARTYP_STR)
		{
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
		}
		else
		{
			// if rhs isnt a string and lhs is empty then eq is false
			//(after checking that rhs is actually assigned)
			if (lhs.var_str.length() == 0)
			{
				if (!rhs.var_typ)
				{
					// throw MVUnassigned("eq(rhs)");
					ISASSIGNED(rhs)
				}
				// different from MVlt
				return false;
			}
		}
	}
	else
	{
		// if lhs isnt a string and rhs is an empty string then return eq false
		//(after checking that lhs is actually assigned)
		if ((rhs.var_typ & VARTYP_STR) && (rhs.var_str.length() == 0))
		{
			if (!lhs.var_typ)
			{
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
	if (lhs.isnum() && rhs.isnum())
	{
		//if (lhs.var_typ & VARTYP_INT)
		if (lhs.var_typ & VARTYP_DBL)
		{

			//DOUBLE v DOUBLE
			//if (rhs.var_typ & VARTYP_INT)
			if (rhs.var_typ & VARTYP_DBL)
			{
				// different from MVlt

				//return (lhs.var_intd == rhs.var_intd);

				// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)
				return (std::abs(lhs.var_dbl - rhs.var_dbl) < SMALLEST_NUMBER);
			}

			//DOUBLE V INT
			else
			{
				// different from MVlt (uses absolute)
				// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)

				//return (lhs.var_int == rhs.var_dbl);
				return (std::abs(lhs.var_dbl - double(rhs.var_int)) < SMALLEST_NUMBER);
			}
		}

		//INT v DOUBLE
		//if (rhs.var_typ & VARTYP_INTd)
		if (rhs.var_typ & VARTYP_DBL)
		{
			// different from MVlt (uses absolute)
			// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)

			//return (lhs.var_dbl == rhs.var_int);
			return (std::abs(double(lhs.var_int) - rhs.var_dbl) < SMALLEST_NUMBER);
		}

		//INT v INT
		else
		{
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
	return lhs.var_str == rhs.var_str;
}

// almost identical between MVeq and MVlt except where noted
DLL_PUBLIC bool MVlt(const var& lhs, const var& rhs)
{
	THISIS("bool MVlt(const var& lhs,const var& rhs)")
	ISDEFINED(lhs)
	ISDEFINED(rhs)

	// NB empty string is always less than anything except another empty string

	// 1. both empty or identical strings returns eq. one empty results false
	if (lhs.var_typ & VARTYP_STR)
	{
		if (rhs.var_typ & VARTYP_STR)
		{
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
		}
		else
		{
			// if rhs isnt a string and lhs is empty then eq is false
			// after checking that rhs is actually assigned
			if (lhs.var_str.length() == 0)
			{
				if (!rhs.var_typ)
				{
					// throw MVUnassigned("eq(rhs)");
					ISASSIGNED(rhs)
				}
				// different from MVeq
				return true;
			}
		}
	}
	else
	{
		// if lhs isnt a string and rhs is an empty string then return eq false
		// after checking that lhs is actually assigned
		if ((rhs.var_typ & VARTYP_STR) && (rhs.var_str.length() == 0))
		{
			if (!lhs.var_typ)
			{
				// throw MVUnassigned("eq(lhs)");
				ISASSIGNED(lhs)
			}
			// SAME as MVeq
			return false;
		}
	}

	// 2. both numerical strings
	if (lhs.isnum() && rhs.isnum())
	{
		if (lhs.var_typ & VARTYP_INT)
		{
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

// almost identical between MVeq and MVlt except where noted
// this is the var<int version for speed
DLL_PUBLIC bool MVlt(const var& lhs, const int int2)
{
	THISIS("bool MVlt(const var& lhs,const int int2)")
	ISDEFINED(lhs)

	// NB empty string is always less than anything except another empty string

	// 1. both empty or identical strings returns eq. one empty results false
	if (lhs.var_typ & VARTYP_STR)
	{
		// if rhs isnt a string and lhs is empty then eq is false
		// after checking that rhs is actually assigned
		if (lhs.var_str.length() == 0)
		{
			// different from MVeq
			return true;
		}
	}

	// 2. both numerical strings
	do
	{
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
	if (!(lhs.var_typ & VARTYP_STR))
	{
		// lhs.createString();
		ISSTRING(lhs)
	}
	// different from MVeq
	return lhs.var_str < intToString(int2);
}

// almost identical between MVeq and MVlt except where noted
// this is the int<var version for speed
DLL_PUBLIC bool MVlt(const int int1, const var& rhs)
{
	THISIS("bool MVlt(const int int1,const var& rhs)")
	ISDEFINED(rhs)

	// NB empty string is always less than anything except another empty string

	// 1. both empty or identical strings returns eq. one empty results false
	if (rhs.var_typ & VARTYP_STR)
	{
		if (rhs.var_str.length() == 0)
			// SAME as MVeq
			return false;
		// otherwise go on to test numerically then literally
	}

	// 2. both numerical strings
	do
	{
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
	if (!(rhs.var_typ & VARTYP_STR))
	{
		// lhs.createString();
		ISSTRING(rhs)
	}
	// different from MVeq
	return intToString(int1) < rhs.var_str;
}
// SEE ALSO MV2.CPP

//+var
//DLL_PUBLIC var operator+(const var& var1)
var MVplus(const var& var1)
{
	THISIS("var operator+(const var& var1)")
	ISDEFINED(var1)

	do
	{
		// dbl
		if (var1.var_typ & VARTYP_DBL)
			return var1.var_dbl;

		// int
		if (var1.var_typ & VARTYP_INT)
			return var1.var_int;

		// unassigned
		if (!var1.var_typ)
		{
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
var MVminus(const var& var1)
{
	THISIS("var operator-(const var& var1)")
	ISDEFINED(var1)

	do
	{
		// dbl
		if (var1.var_typ & VARTYP_DBL)
			return -var1.var_dbl;

		// int
		if (var1.var_typ & VARTYP_INT)
			return -var1.var_int;

		// unassigned
		if (!var1.var_typ)
		{
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
bool MVnot(const var& var1)
{
	THISIS("bool operator!(const var& var1)")
	ISASSIGNED(var1)

	// might need converting to work on void pointer
	// if bool replaced with void* (or made explicit instead of implict)
	// is there really any difference since the bool and void operators are defined identically?
	// return !(bool)(var1);
	return !(void*)(var1);
}

var MVadd(const var& lhs, const var& rhs)
{
	THISIS("var operator+(const var& lhs,const var& rhs)")
	ISNUMERIC(lhs)
	ISNUMERIC(rhs)

	if (lhs.var_typ & VARTYP_INT)
		if (rhs.var_typ & VARTYP_INT)
			return lhs.var_int + rhs.var_int; // only this returns an int, the following
							  // both return doubles
		else
			return lhs.var_int +
			       ((rhs.var_typ & VARTYP_INT) ? rhs.var_int : rhs.var_dbl);
	else
		return lhs.var_dbl + ((rhs.var_typ & VARTYP_INT) ? rhs.var_int : rhs.var_dbl);
}

var MVsub(const var& lhs, const var& rhs)
{
	THISIS("var operator-(const var& lhs,const var& rhs)")
	ISNUMERIC(lhs)
	ISNUMERIC(rhs)

	if (lhs.var_typ & VARTYP_INT)
		if (rhs.var_typ & VARTYP_INT)
			return lhs.var_int - rhs.var_int; // only this returns an int, the following
							  // both return doubles
		else
			return lhs.var_int -
			       ((rhs.var_typ & VARTYP_INT) ? rhs.var_int : rhs.var_dbl);
	else
		return lhs.var_dbl - ((rhs.var_typ & VARTYP_INT) ? rhs.var_int : rhs.var_dbl);
}

var MVmul(const var& lhs, const var& rhs)
{
	THISIS("var operator*(const var& lhs,const var& rhs)")
	ISNUMERIC(lhs)
	ISNUMERIC(rhs)

	if (lhs.var_typ & VARTYP_INT)
		if (rhs.var_typ & VARTYP_INT)
			return lhs.var_int * rhs.var_int; // only this returns an int, the following
							  // both return doubles
		else
			return lhs.var_int *
			       ((rhs.var_typ & VARTYP_INT) ? rhs.var_int : rhs.var_dbl);
	else
		return lhs.var_dbl * ((rhs.var_typ & VARTYP_INT) ? rhs.var_int : rhs.var_dbl);
}

var MVdiv(const var& lhs, const var& rhs)
{
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

var MVmod(const var& lhs, const var& rhs)
{
	THISIS("var operator%(const var& lhs,const var& rhs)")
	ISNUMERIC(lhs)
	ISNUMERIC(rhs)

	// integer version;
	if (lhs.var_typ & VARTYP_INT && rhs.var_typ & VARTYP_INT)
	{
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
var MVcat(const var& lhs, const var& rhs)
{
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
std::ostream& operator<<(std::ostream& ostream1, var var1)
{
	THISIS("std::ostream& operator<< (std::ostream& ostream1, var var1)")
	ISSTRING(var1)

	//replace various unprintable field marks with unusual ASCII characters
	//leave ESC as \x1B because it is used to control ANSI terminal control sequences
    std::string str = "\x1A\x1B\x1C\x1D\x1E\x1F";
    for (auto& c : var1.var_str) {
        if (c <= 0x1F && c >= 0x1A)
            c = "|\x1B\\]^~"[c - 0x1A];
    }

	// use toString() to avoid creating a constructor which logs here recursively
	// should this use a ut16/32 -> UTF8 code facet? or convert to UTF8 and output to ostream?
	ostream1 << var1.var_str;
	return ostream1;
}

std::istream& operator>>(std::istream& istream1, var& var1)
{
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

inline double exodusmodulus(const double top, const double bottom)
{
	return top - double(int(top / bottom) * bottom);
}

// TODO ensure locale doesnt produce like 123.456,78
std::string intToString(int int1)
{

	// TODO test ostringstream type creation speed and of slow then
	// consider using a thread global to avoid continual creation

	// see http://www.gotw.ca/publications/mill19.htm
	// and http://www.boost.org/libs/conversion/lexical_cast.htm
	// for background to choice of ostringstream for implementation

	// 1. sprintf rejected because snprintf is safer
	// 2. snprintf rejected because cannot handle wide characters
	// 3. strstream rejected because it is deprecated and might cease to be supported
	// but is the fastest if you use its ability to preallocate memory in one go
	// RECONSIDER IMPLEMENTING IN strstream for performance
	// 4. stringstream CHOSEN even though very slow versus snprintf
	// 5. boost:lexical_cast rejected because two step conversion is very slow
	// and unnecessary if you just want a string and doesnt allow precision control
	// useful for float2string

	// NB plain stringstream causes a memory leak in msvc8 before sp1
	std::ostringstream ss;
	ss << int1;
	// debuggFUNCTION&& cout<<"intToString(int "<<int1<<") returns '"<<s<<"'\n";
	return ss.str();
}

// TODO ensure locale doesnt produce like 123.456,78
// see 1997 http://www.cantrip.org/locale.html
std::string dblToString(double double1)
{

	//std::cout << abs(double1) << std::endl;
	//std::cout << double(0.000'000'001) << std::endl;
	//std::cout << (std::abs(double1)<double(0.000'000'001)) << std::endl;

	//fixed to precision six
	//return std::to_string(double1);

	// see intToString for choice of ostringstream for implementation
	// NB plain stringstream causes a memory leak in msvc8 before sp1
	std::ostringstream stringstream1;
	//if precision is changed, also change testmain
	//stringstream1.precision(16);
	//use precision 14 to avoid 1.1-1 = 1.000000000000001
	stringstream1.precision(14);

	std::string string1;
	//evade scientific format for small numbers
	if (std::abs(double1) < double(0.000'1)) {

		//treat very small numbers as zero
		//if (std::abs(double1)<double(0.000'000'000'1))
		//if (std::abs(double1)<double(0.000'000'000'000'001))
		if (std::abs(double1)<double(0.000'000'000'000'1))
			return "0.0";

	    stringstream1 << std::fixed;
	    stringstream1 << double1;
	    string1 = stringstream1.str();
	    while (string1.back() == '0')
    	    string1.pop_back();

	} else {
		stringstream1 << double1;
		string1 = stringstream1.str();
	}
	return string1;
}

var backtrace();

MVError::MVError(const var& description_) : description(description_)
{
	// capture the stack at point of creation i.e. when thrown
	this->stack = backtrace();
	((description.assigned() ? description : "") ^ "\n" ^ stack.convert(FM, "\n") ^ "\n").put(std::cerr);
}

MVUnassigned ::MVUnassigned(const var& var1) : MVError("MVUnassigned:" ^ var1) {}
MVDivideByZero ::MVDivideByZero(const var& var1) : MVError("MVDivideByZero:" ^ var1) {}
MVNonNumeric ::MVNonNumeric(const var& var1) : MVError("MVNonNumeric:" ^ var1) {}
MVIntOverflow ::MVIntOverflow(const var& var1) : MVError("MVIntOverflow:" ^ var1) {}
MVIntUnderflow ::MVIntUnderflow(const var& var1) : MVError("MVIntUnderflow:" ^ var1) {}
MVUndefined ::MVUndefined(const var& var1) : MVError("MVUndefined:" ^ var1) {}
MVOutOfMemory ::MVOutOfMemory(const var& var1) : MVError("MVOutOfMemory:" ^ var1) {}
MVInvalidPointer ::MVInvalidPointer(const var& var1) : MVError("MVInvalidPointer:" ^ var1) {}
MVDBException ::MVDBException(const var& var1) : MVError("MVDBException:" ^ var1) {}
MVNotImplemented ::MVNotImplemented(const var& var1) : MVError("MVNotImplemented:" ^ var1) {}
MVDebug ::MVDebug(const var& var1) : MVError("MVDebug" ^ var1) {}
MVStop ::MVStop(const var& var1) : description(var1) {}
MVAbort ::MVAbort(const var& var1) : description(var1) {}
MVAbortAll ::MVAbortAll(const var& var1) : description(var1) {}
MVLogoff ::MVLogoff(const var& var1) : description(var1) {}
MVArrayDimensionedZero ::MVArrayDimensionedZero() : MVError("MVArrayDimensionedZero:") {}
MVArrayIndexOutOfBounds ::MVArrayIndexOutOfBounds(const var& var1)
    : MVError("MVArrayIndexOutOfBounds:" ^ var1)
{
}
MVArrayNotDimensioned ::MVArrayNotDimensioned() : MVError("MVArrayNotDimensioned") {}

var operator""_var (const char* cstr, std::size_t size)
{
    return var(cstr, size);
}

var operator""_var (unsigned long long int i)
{
    return var(int(i));
}

var operator""_var (long double d)
{
    return var(double(d));
}
} // namespace exodus
