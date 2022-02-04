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

#include <cassert>
#include <limits>
#include <sstream>
#include <vector>

#define EXO_MV_CPP	// indicates globals are to be defined (omit extern keyword)
#include <exodus/mv.h>

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

void var::throwUndefined(CVR message) const {
	throw MVUndefined(message);
}

void var::throwUnassigned(CVR message) const {
	throw MVUnassigned(message);
}

void var::throwNonNumeric(CVR message) const {
	throw MVNonNumeric(message);
}

//// copy constructor - not inline merely because of lack of THISIS etc in mv.h
//var::var(CVR rhs)
//	: var_str(rhs.var_str),
//	var_int(rhs.var_int),
//	var_dbl(rhs.var_dbl),
//	var_typ(rhs.var_typ) {
//
//	// use initializers and only check afterwards if copiedvar was assigned (why?)
//
	THISIS("var::var(CVR rhs)")
//	rhs.assertAssigned(functionname);
//
//	//std::clog << "copy ctor CVR " << rhs.var_str << std::endl;
//}

// someone recommends not to create more than one automatic converter
// to avoid the compiler error "ambiguous conversion"
//(explicit means it is not automatic)

// allow conversion to string (IS THIS USED FOR ANYTHING AT THE MOMENT?
// allows the usage of any string function
var::operator std::string() const {

	THISIS("var::operator std::string")
	assertString(functionname);
	return var_str;
}

var::operator void*() const {

	THISIS("var::operator void*() const")
	// could be skipped for speed if can be proved there is no way in c++ syntax that would
	// result in an attempt to convert an uninitialised object to void* since there is a bool
	// conversion when does c++ use automatic conversion to void* note that exodus operator !
	// uses (void*) trial elimination of operator void* seems to cause no problems but without
	// full regression testing
	assertDefined(functionname);

	return (void*)toBool();
}

// supposed to be replaced with automatic void() and made explicit but just seems to force int
// conversion during "if (var)" necessary to allow var to be used standalone in "if (xxx)" but see
// mv.h for discussion of using void* instead of bool #ifndef _MSC_VER
var::operator bool() const {
	return toBool();
}

var::operator int() const {

	THISIS("var::operator int() const")
	//converts string or double to int using pickos int() which is floor()
	//unlike c/c++ int() function which rounds to nearest even number (negtive or positive)
	assertInteger(functionname);
	return static_cast<int>(var_int);
}

var::operator long long() const {

	THISIS("var::operator long long() const")
	assertInteger(functionname);
	return static_cast<long long>(var_int);
}

var::operator double() const {

	THISIS("var::operator double() const")
	assertDecimal(functionname);
	return static_cast<double>(var_dbl);
}

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
	assertDefined(functionname);	 //could be skipped for speed?
	rhs.assertAssigned(functionname);

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

// UNARY OPERATORS
//////////////////

//^= is not templated since slightly slower to go through the creation of an var()

//^=var
// The assignment operator should always return a reference to *this.
VARREF var::operator^=(CVR rhs) & {

	THISIS("VARREF var::operator^=(CVR rhs) &")
	assertString(functionname);
	rhs.assertString(functionname);

	// tack it onto our string
	var_str.append(rhs.var_str);
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//^=int
// The assignment operator should always return a reference to *this.
VARREF var::operator^=(const int int1) & {

	THISIS("VARREF var::operator^=(const int int1) &")
	assertString(functionname);

	// var_str+=var(int1).var_str;
	var_str += std::to_string(int1);
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//^=double
// The assignment operator should always return a reference to *this.
VARREF var::operator^=(const double double1) & {

	THISIS("VARREF var::operator^=(const double double1) &")
	assertString(functionname);

	// var_str+=var(int1).var_str;
	//var_str += mvd2s(double1);
	//var_typ = VARTYP_STR;  // reset to one unique type
	var temp(double1);
	temp.createString();
	var_str += std::move(temp.var_str);

	return *this;
}

//^=char
// The assignment operator should always return a reference to *this.
VARREF var::operator^=(const char char1) & {

	THISIS("VARREF var::operator^=(const char char1) &")
	assertString(functionname);

	// var_str+=var(int1).var_str;
	var_str.push_back(char1);
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//^=char*
// The assignment operator should always return a reference to *this.
VARREF var::operator^=(const char* cstr) & {

	THISIS("VARREF var::operator^=(const char* cstr) &")
	assertString(functionname);

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
	assertString(functionname);

	// var_str+=var(int1).var_str;
	var_str += string1;
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

// You must *not* make the postfix version return the 'this' object by reference
//
// *** YOU HAVE BEEN WARNED ***

// not returning void so is usable in expressions
// int argument indicates that this is POSTFIX override v++
var var::operator++(int) & {

	THISIS("var var::operator++(int) &")
	// full check done below to avoid double checking number type
	assertDefined(functionname);

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
		assertNumeric(functionname);

	} else {
		//trigger MVUnassigned
		assertNumeric(functionname);
	}

	// NO DO NOT! return *this ... postfix return a temporary!!! eg var(*this)
	return priorvalue;
}

// not returning void so is usable in expressions
// int argument indicates that this is POSTFIX override v--
var var::operator--(int) & {

	THISIS("var var::operator--(int) & ")
	// full check done below to avoid double checking number type
	assertDefined(functionname);

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
		assertNumeric(functionname);

	} else {
		//trigger MVUnassigned
		assertNumeric(functionname);
	}

	return priorvalue;
}

// not returning void so is usable in expressions
// no argument indicates that this is prefix override ++var
VARREF var::operator++() & {

	THISIS("var var::operator++() &")
	// full check done below to avoid double checking number type
	assertDefined(functionname);

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
		assertNumeric(functionname);

	} else {
		//trigger MVUnassigned
		assertNumeric(functionname);
	}

	// OK to return *this in prefix ++
	return *this;
}

// not returning void so is usable in expressions
// no argument indicates that this is prefix override --var
VARREF var::operator--() & {

	THISIS("VARREF var::operator--() &")
	// full check done below to avoid double checking number type
	assertDefined(functionname);

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
		assertNumeric(functionname);

	} else {
		//trigger MVUnassigned
		assertNumeric(functionname);
	}

	// OK to return *this in prefix --
	return *this;
}

//+=var (very similar to version with on rhs)
// provided to disambiguate syntax like var1+=var2
VARREF var::operator+=(const int int1) & {

	THISIS("VARREF var::operator+=(const int int1) &")
	assertDefined(functionname);

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

	assertNumeric(functionname);
	throw MVNonNumeric(substr(1, 128) ^ "+= ");
}

//+='1' (very similar to version with on rhs)
// provided to disambiguate syntax like var1 += '1'

VARREF var::operator+=(const char char1) & {

	THISIS("VARREF var::operator+=(const char char1) &")
	var charx = char1;
	charx.assertNumeric(functionname);
	return this->operator+=(int(charx.var_int));
}

//-='1' (very similar to version with on rhs)
// provided to disambiguate syntax like var1 -= '1'
VARREF var::operator-=(const char char1) & {

	THISIS("VARREF var::operator-=(const char char1) &")
	var charx = char1;
	charx.assertNumeric(functionname);
	return this->operator-=(int(charx.var_int));
}

//-=var (very similar to version with on rhs)
// provided to disambiguate syntax like var1+=var2
VARREF var::operator-=(const int int1) & {

	THISIS("VARREF var::operator-=(int int1) &")
	assertDefined(functionname);

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

	assertNumeric(functionname);
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
	assertDefined(functionname);

tryagain:

	// dbl target
	// prefer double
	if (var_typ & VARTYP_DBL) {
		rhs.assertNumeric(functionname);
		//+= int or dbl from source
		var_dbl += (rhs.var_typ & VARTYP_INT) ? rhs.var_int : rhs.var_dbl;
		var_typ = VARTYP_DBL;  // reset to one unique type
		return *this;
	}

	// int target
	else if (var_typ & VARTYP_INT) {
		rhs.assertNumeric(functionname);
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

	assertNumeric(functionname);
	throw MVNonNumeric(substr(1, 128) ^ "+= ");
}

//-=var
VARREF var::operator-=(CVR rhs) & {

	THISIS("VARREF var::operator-=(CVR rhs) &")
	assertDefined(functionname);

tryagain:

	// int target
	if (var_typ & VARTYP_INT) {
		rhs.assertNumeric(functionname);
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
		rhs.assertNumeric(functionname);
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

	assertNumeric(functionname);
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
	lhs.assertDefined(functionname);
	rhs.assertDefined(functionname);

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
					rhs.assertAssigned(functionname);
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
				lhs.assertAssigned(functionname);
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
	lhs.assertDefined(functionname);
	rhs.assertDefined(functionname);

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
					rhs.assertAssigned(functionname);
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
				lhs.assertAssigned(functionname);
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
	lhs.assertDefined(functionname);

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
		lhs.assertString(functionname);
	}
	// different from MVeq
	return lhs.var_str < std::to_string(int2);
}

// similar to MVeq and MVlt - this is the int<var version for speed
// NOTE doubles compare only to 0.0001 accuracy)
PUBLIC bool MVlt(const int int1, CVR rhs) {

	THISIS("bool MVlt(const int int1,CVR rhs)")
	rhs.assertDefined(functionname);

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
		rhs.assertString(functionname);
	}
	// different from MVeq
	return std::to_string(int1) < rhs.var_str;
}
// SEE ALSO MV2.CPP

//+var
//PUBLIC var operator+(CVR var1)
var MVplus(CVR var1) {

	THISIS("var operator+(CVR var1)")
	var1.assertDefined(functionname);

	do {
		// dbl
		if (var1.var_typ & VARTYP_DBL)
			return var1.var_dbl;

		// int
		if (var1.var_typ & VARTYP_INT)
			return var1.var_int;

		// unassigned
		if (!var1.var_typ) {
			var1.assertAssigned(functionname);
			throw MVUnassigned("+()");
		}
	}
	// must be string - try to convert to numeric
	while (var1.isnum());

	// non-numeric
	var1.assertNumeric(functionname);
	// will never get here
	throw MVNonNumeric("+(" ^ var1.substr(1, 128) ^ ")");
}

//-var (identical to +var above except for two additional - signs)
//PUBLIC var operator-(CVR var1)
var MVminus(CVR var1) {

	THISIS("var operator-(CVR var1)")
	var1.assertDefined(functionname);

	do {
		// dbl
		if (var1.var_typ & VARTYP_DBL)
			return -var1.var_dbl;

		// int
		if (var1.var_typ & VARTYP_INT)
			return -var1.var_int;

		// unassigned
		if (!var1.var_typ) {
			var1.assertAssigned(functionname);
			throw MVUnassigned("+()");
		}
	}
	// must be string - try to convert to numeric
	while (var1.isnum());

	// non-numeric
	var1.assertNumeric(functionname);
	// will never get here
	throw MVNonNumeric("+(" ^ var1.substr(1, 128) ^ ")");
}

//! var
//PUBLIC bool operator!(CVR var1)
bool MVnot(CVR var1) {

	THISIS("bool operator!(CVR var1)")
	var1.assertAssigned(functionname);

	// might need converting to work on void pointer
	// if bool replaced with void* (or made explicit instead of implict)
	// is there really any difference since the bool and void operators are defined identically?
	// return !(bool)(var1);
	return !(void*)(var1);
}

var MVadd(CVR lhs, CVR rhs) {

	THISIS("var operator+(CVR lhs,CVR rhs)")
	lhs.assertNumeric(functionname);
	rhs.assertNumeric(functionname);

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
	lhs.assertNumeric(functionname);
	rhs.assertNumeric(functionname);

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
	lhs.assertNumeric(functionname);
	rhs.assertNumeric(functionname);

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
	lhs.assertNumeric(functionname);
	rhs.assertNumeric(functionname);

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
	lhs.assertNumeric(functionname);
	rhs.assertNumeric(functionname);

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
	lhs.assertString(functionname);
	rhs.assertString(functionname);

	return lhs.var_str + rhs.var_str;
}

var MVcat(CVR lhs, const char* cstr) {

	THISIS("var operator^(CVR lhs,const char* cstr)")
	lhs.assertString(functionname);

	return lhs.var_str + cstr;
}

var MVcat(const char* cstr, CVR rhs) {

	THISIS("var operator^(const char* cstr, CVR rhs)")
	rhs.assertString(functionname);

	return cstr + rhs.var_str;
}

var MVcat(CVR lhs, const char char2) {

	THISIS("var operator^(CVR lhs,const char char2)")
	lhs.assertString(functionname);

	return lhs.var_str + char2;
}
/*

VARREF var::operator^(CVR vstr) {

	THISIS("VARREF var::operator^(CVR vstr)")
	assertString(functionname);
	vstr.assertString(functionname);

	var_str += vstr.var_str;

	return *this;
}

VARREF var::operator^(const char* cstr) {
	std::clog <<("var operator^(const char& cstr)") << std::endl;

	THISIS("VARREF var::operator^(const char& cstr)")
	assertString(functionname);

	var_str += cstr;

	return *this;
}

VARREF var::operator^(const std::string& stdstr) {

	THISIS("VARREF var::operator^(const std::string& stdstr)");
	assertString(functionname);
	stdstr.assertString(functionname);

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
	var1.assertString(functionname);

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
	var1.assertDefined(functionname);

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
	return var(cstr, size).convert(VISIBLE_FMS, _RM_ _FM_ _VM_ _SM_ _TM_ _STM_);
}

var operator""_var(unsigned long long int i) {
	return var(int(i));
}

var operator""_var(long double d) {
	return var(double(d));
}

}  // namespace exodus
