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

CVR var::dump(SV text) const {
	std::clog << "DUMP: " << text << " ";
	if (var_typ & VARTYP_STR)
		std::clog << "str: \"" << var_str << "\" ";
	if (var_typ & VARTYP_INT)
		std::clog << "int:" << var_int << " ";
	if (var_typ & VARTYP_DBL)
		std::clog << "dbl:" << var_dbl << " ";
	std::clog << "typ:" << var_typ << std::endl;
	return *this;
}

VARREF var::dump(SV text) {
	std::clog << "DUMP: " << text << " ";
	if (var_typ & VARTYP_STR)
		std::clog << "str: \"" << var_str << "\" ";
	if (var_typ & VARTYP_INT)
		std::clog << "int:" << var_int << " ";
	if (var_typ & VARTYP_DBL)
		std::clog << "dbl:" << var_dbl << " ";
	std::clog << "typ:" << var_typ << std::endl;
	return *this;
}

double exodusmodulus(const double top, const double bottom) {
#define USE_PICKOS_MODULUS
#ifdef USE_PICKOS_MODULUS

	// note that exodus var int() is floor function as per pickos standard
	// whereas c/c++ int() function is round to nearest even number (negative or positive)

	//https://pickos.neosys.com/x/pAEz.html
	//var(i) - (int(var(i)/var(j)) * var(j))) ... where int() is pickos int() (i.e. floor)
	return top - static_cast<double>(floor(top / bottom) * bottom);

#else
	//return top - static_cast<double>(int(top / bottom) * bottom);
	//return top % bottom;//doesnt compile (doubles)

	//fmod - The floating-point remainder of the division operation x/y calculated by this function is exactly the value x - n*y, where n is x/y with its fractional part truncated.
	//https://en.cppreference.com/w/c/numeric/math/fmod
	//return std::fmod(top,bottom);

	//remainder - The IEEE floating-point remainder of the division operation x/y calculated by this function is exactly the value x - n*y, where the value n is the integral value nearest the exact value x/y. When |n-x/y| = ½, the value n is chosen to be even.
	//https://en.cppreference.com/w/c/numeric/math/remainder
	return std::remainder(top, bottom);

#endif
}

//var::var(CVR rhs)
//	: var_str(rhs.var_str),
//	var_int(rhs.var_int),
//	var_dbl(rhs.var_dbl),
//	var_typ(rhs.var_typ) {
//
//	// use initializers and only check afterwards if copiedvar was assigned (why?)
//
//	rhs.assertAssigned(__PRETTY_FUNCTION__);
//
//	//std::clog << "copy ctor CVR " << rhs.var_str << std::endl;
//}

// someone recommends not to create more than one automatic converter
// to avoid the compiler error "ambiguous conversion"
//(explicit means it is not automatic)

///**
//* Automatic conversion to std::string
//* @return std::string
//*/
//var::operator std::string() const {
//
//	assertString(__PRETTY_FUNCTION__);
//
//	return var_str;
//}

///**
//* Automatic conversion to std::string_view
//* @return std::string_view
//*/
//var::operator std::string_view() const {
//
//	assertString(__PRETTY_FUNCTION__);
//
//	return std::string_view(var_str);
//}

///**
//* Automatic conversion to bool expressed as void*
//* @return void* 0 or 1
//*/
//var::operator void*() const {
//
//	// result in an attempt to convert an uninitialised object to void* since there is a bool
//	// conversion when does c++ use automatic conversion to void* note that exodus operator !
//	// uses (void*) trial elimination of operator void* seems to cause no problems but without
//	// full regression testing
//	assertDefined(__PRETTY_FUNCTION__);
//
//	return (void*)toBool();
//}

//// supposed to be replaced with automatic void() and made explicit but just seems to force int
//// conversion during "if (var)" necessary to allow var to be used standalone in "if (xxx)" but see
//// mv.h for discussion of using void* instead of bool #ifndef _MSC_VER
//var::operator bool() const {
//	return toBool();
//}

//var::operator int() const {
//
//	//converts string or double to int using pickos int() which is floor()
//	//unlike c/c++ int() function which rounds to nearest even number (negtive or positive)
//	assertInteger(__PRETTY_FUNCTION__);
//	return static_cast<int>(var_int);
//}
//
//var::operator long long() const {
//
//	assertInteger(__PRETTY_FUNCTION__);
//	return static_cast<long long>(var_int);
//}
//
//var::operator double() const {
//
//	assertDecimal(__PRETTY_FUNCTION__);
//	return static_cast<double>(var_dbl);
//}
//
//var::operator long double() const {
//
//	assertDecimal(__PRETTY_FUNCTION__);
//	return static_cast<long double>(var_dbl);
//}

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

	assertDefined(__PRETTY_FUNCTION__);	 //could be skipped for speed?
	rhs.assertAssigned(__PRETTY_FUNCTION__);

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

	assertString(__PRETTY_FUNCTION__);
	rhs.assertString(__PRETTY_FUNCTION__);

	// tack it onto our string
	var_str.append(rhs.var_str);
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//^=int
// The assignment operator should always return a reference to *this.
VARREF var::operator^=(const int int1) & {

	assertString(__PRETTY_FUNCTION__);

	// var_str+=var(int1).var_str;
	var_str += std::to_string(int1);
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//^=double
// The assignment operator should always return a reference to *this.
VARREF var::operator^=(const double double1) & {

	assertString(__PRETTY_FUNCTION__);

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

	assertString(__PRETTY_FUNCTION__);

	// var_str+=var(int1).var_str;
	var_str.push_back(char1);
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//^=char*
// The assignment operator should always return a reference to *this.
VARREF var::operator^=(const char* cstr) & {

	assertString(__PRETTY_FUNCTION__);

	// var_str+=var(int1).var_str;
	// var_str+=std::string(char1);
	var_str += cstr;
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//^=std::string
// The assignment operator should always return a reference to *this.
VARREF var::operator^=(const std::string& string1) & {

	assertString(__PRETTY_FUNCTION__);

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

	// full check done below to avoid double checking number type
	assertDefined(__PRETTY_FUNCTION__);

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
		assertNumeric(__PRETTY_FUNCTION__);

	} else {
		//trigger MVUnassigned
		assertNumeric(__PRETTY_FUNCTION__);
	}

	// NO DO NOT! return *this ... postfix return a temporary!!! eg var(*this)
	return priorvalue;
}

// not returning void so is usable in expressions
// int argument indicates that this is POSTFIX override v--
var var::operator--(int) & {

	// full check done below to avoid double checking number type
	assertDefined(__PRETTY_FUNCTION__);

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
		assertNumeric(__PRETTY_FUNCTION__);

	} else {
		//trigger MVUnassigned
		assertNumeric(__PRETTY_FUNCTION__);
	}

	return priorvalue;
}

// not returning void so is usable in expressions
// no argument indicates that this is prefix override ++var
VARREF var::operator++() & {

	// full check done below to avoid double checking number type
	assertDefined(__PRETTY_FUNCTION__);

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
		assertNumeric(__PRETTY_FUNCTION__);

	} else {
		//trigger MVUnassigned
		assertNumeric(__PRETTY_FUNCTION__);
	}

	// OK to return *this in prefix ++
	return *this;
}

// not returning void so is usable in expressions
// no argument indicates that this is prefix override --var
VARREF var::operator--() & {

	// full check done below to avoid double checking number type
	assertDefined(__PRETTY_FUNCTION__);

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
		assertNumeric(__PRETTY_FUNCTION__);

	} else {
		//trigger MVUnassigned
		assertNumeric(__PRETTY_FUNCTION__);
	}

	// OK to return *this in prefix --
	return *this;
}

//////////////
// SELF ASSIGN
//////////////

//////
// VAR
//////

// ADD VAR

VARREF var::operator+=(CVR rhs) &{

	rhs.assertNumeric(__PRETTY_FUNCTION__);

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {
		// use rhs double if available otherwise use its int
		var_dbl += (rhs.var_typ & VARTYP_DBL) ? rhs.var_dbl : rhs.var_int;
		// reset lhs to one unique type
		var_typ = VARTYP_DBL;
		return *this;
	}

	// lhs int
	else if (var_typ & VARTYP_INT) {

		// rhs double
		if (rhs.var_typ & VARTYP_DBL) {
			var_dbl = var_int + rhs.var_dbl;
			// reset lhs to one unique type
			var_typ = VARTYP_DBL;
			return *this;
		}

		// both are ints
		var_int += rhs.var_int;
		// reset lhs to one unique type
		var_typ = VARTYP_INT;
		return *this;
	}

	// try to convert to numeric
	if (isnum())
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw MVNonNumeric(substr(1, 128) ^ "+= ");
}

// MULTIPLY VAR

VARREF var::operator*=(CVR rhs) &{

	rhs.assertNumeric(__PRETTY_FUNCTION__);

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {
		// use rhs double if available otherwise use its int
		var_dbl *= (rhs.var_typ & VARTYP_DBL) ? rhs.var_dbl : rhs.var_int;
		// reset lhs to one unique type
		var_typ = VARTYP_DBL;
		return *this;
	}

	// lhs int
	else if (var_typ & VARTYP_INT) {

		// rhs double
		if (rhs.var_typ & VARTYP_DBL) {
			var_dbl = var_int * rhs.var_dbl;
			// reset lhs to one unique type
			var_typ = VARTYP_DBL;
			return *this;
		}

		// both are ints
		var_int *= rhs.var_int;
		// reset lhs to one unique type
		var_typ = VARTYP_INT;
		return *this;
	}

	// try to convert to numeric
	if (isnum())
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw MVNonNumeric(substr(1, 128) ^ "+= ");
}

// SUBTRACT VAR

VARREF var::operator-=(CVR rhs) &{

	rhs.assertNumeric(__PRETTY_FUNCTION__);

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {
		// use rhs double if available otherwise use its int
		var_dbl -= (rhs.var_typ & VARTYP_DBL) ? rhs.var_dbl : rhs.var_int;
		// reset lhs to one unique type
		var_typ = VARTYP_DBL;
		return *this;
	}

	// lhs int
	else if (var_typ & VARTYP_INT) {

		// use rhs double if available
		if (rhs.var_typ & VARTYP_DBL) {
			var_dbl = var_int - rhs.var_dbl;
			// reset lhs to one unique type
			var_typ = VARTYP_DBL;
			return *this;
		}

		// both are ints
		var_int -= rhs.var_int;
		// reset lhs to one unique type
		var_typ = VARTYP_INT;
		return *this;
	}

	// try to convert to numeric
	if (isnum())
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw MVNonNumeric(substr(1, 128) ^ "+= ");
}


// DIVIDE VAR

VARREF var::operator/=(CVR rhs) &{

	// Always returns a double because
	// 10/3 must be 3.3333333

	rhs.assertNumeric(__PRETTY_FUNCTION__);

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {

		// rhs double
		if (rhs.var_typ & VARTYP_DBL) {
			if (!rhs.var_dbl)
				throw MVDivideByZero("div('" ^ this->substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
									 "')");
			var_dbl /= rhs.var_dbl;
		}

		// rhs double
		else {
			if (!rhs.var_int)
				throw MVDivideByZero("div('" ^ this->substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
									 "')");
			var_dbl /= rhs.var_int;
		}

	}

	// lhs int
	else if (var_typ & VARTYP_INT) {

		// rhs double
		if (rhs.var_typ & VARTYP_DBL) {
			if (!rhs.var_dbl)
				throw MVDivideByZero("div('" ^ this->substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
								 "')");
			var_dbl = static_cast<double>(this->var_int) / rhs.var_dbl;
		}

		// both are ints - must return a double
		else {
			if (!rhs.var_int)
				throw MVDivideByZero("div('" ^ this->substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
				"')");
			var_dbl = static_cast<double>(this->var_int) / rhs.var_int;
		}

	}

	// try to convert to numeric
	else if (isnum())
		goto tryagain;

	else
		assertNumeric(__PRETTY_FUNCTION__);

	// reset lhs to one unique type
	var_typ = VARTYP_DBL;

	return *this;
}


// MODULO VAR

VARREF var::operator%=(CVR rhs) &{

	rhs.assertNumeric(__PRETTY_FUNCTION__);

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {

		// rhs double
		if (rhs.var_typ & VARTYP_DBL) {
			if (!rhs.var_dbl)
				throw MVDivideByZero("mod('" ^ this->substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
									 "')");
			var_dbl = exodusmodulus(var_dbl, rhs.var_dbl);
		}

		// rhs double
		else {
			if (!rhs.var_int)
				throw MVDivideByZero("mod('" ^ this->substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
									 "')");
			var_dbl = exodusmodulus(var_dbl, rhs.var_int);
		}

		// reset lhs to one unique type
		var_typ = VARTYP_DBL;
		return *this;
	}

	// lhs int
	else if (var_typ & VARTYP_INT) {

		// rhs double
		if (rhs.var_typ & VARTYP_DBL) {
			if (!rhs.var_dbl)
				throw MVDivideByZero("mod('" ^ this->substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
								 "')");
			var_dbl = exodusmodulus(var_int, rhs.var_dbl);

			// reset lhs to one unique type
			var_typ = VARTYP_DBL;
			return *this;
		}

		// both are ints - must return a int
		if (!rhs.var_int)
			throw MVDivideByZero("mod('" ^ this->substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
								 "')");
		var_int = this->var_int % rhs.var_int;

		// reset lhs to one unique type
		var_typ = VARTYP_INT;
		return *this;
	}

	// try to convert to numeric
	if (isnum())
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw MVNonNumeric(substr(1, 128) ^ "+= ");
}


/////////
// DOUBLE
/////////

// ADD DOUBLE

VARREF var::operator+=(const double dbl1) &{

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL)
		var_dbl += dbl1;

	// lhs int
	else if (var_typ & VARTYP_INT)
		var_dbl = static_cast<double>(var_int) + dbl1;

	// try to convert to numeric
	else if (isnum())
		goto tryagain;

	else {
		assertNumeric(__PRETTY_FUNCTION__);
		// Cant get here
		throw MVNonNumeric(substr(1, 128) ^ "+= ");
	}

	// reset to one unique type
	var_typ = VARTYP_DBL;

	return *this;
}

// MULTIPLY DOUBLE

VARREF var::operator*=(const double dbl1) &{

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL)
		var_dbl *= dbl1;

	// lhs int
	else if (var_typ & VARTYP_INT)
		var_dbl = var_int * dbl1;

	// try to convert to numeric
	else if (isnum())
		goto tryagain;

	else
		assertNumeric(__PRETTY_FUNCTION__);

	// reset to one unique type
	var_typ = VARTYP_DBL;

	return *this;
}

// SUBTRACT DOUBLE

VARREF var::operator-=(const double dbl1) &{

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL)
		var_dbl -= dbl1;

	// lhs int
	else if (var_typ & VARTYP_INT)
		var_dbl = var_int - dbl1;

	// try to convert to numeric
	else if (isnum())
		goto tryagain;

	else
		assertNumeric(__PRETTY_FUNCTION__);

	// reset to one unique type
	var_typ = VARTYP_DBL;

	return *this;
}


// DIVIDE DOUBLE

VARREF var::operator/=(const double dbl1) &{

	if (!dbl1)
		throw MVDivideByZero("div('" ^ this->substr(1, 128) ^ "', '" ^ dbl1 ^
		"')");

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL)
		var_dbl /= dbl1;

	// lhs int
	else if (var_typ & VARTYP_INT)
		var_dbl = static_cast<double>(var_int) / dbl1;

	// try to convert to numeric
	else if (isnum())
		goto tryagain;

	else
		assertNumeric(__PRETTY_FUNCTION__);

	// reset to one unique type
	var_typ = VARTYP_DBL;

	return *this;
}


// MODULO DOUBLE

VARREF var::operator%=(const double dbl1) &{

	if (!dbl1)
		throw MVDivideByZero("mod('" ^ this->substr(1, 128) ^ "', '" ^ dbl1 ^
		"')");

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL)
		var_dbl = exodusmodulus(var_dbl, dbl1);

	// lhs int
	else if (var_typ & VARTYP_INT)
		var_dbl = exodusmodulus(var_int, dbl1);

	// try to convert to numeric
	else if (isnum())
		goto tryagain;

	else
		assertNumeric(__PRETTY_FUNCTION__);

	// reset to one unique type
	var_typ = VARTYP_DBL;

	return *this;
}


//////
// INT
//////

// ADD INT

VARREF var::operator+=(const int int1) &{

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {
		var_dbl += int1;
		// reset to one unique type
		var_typ = VARTYP_DBL;
		return *this;
	}

	// both int
	else if (var_typ & VARTYP_INT) {
		var_int += int1;
		// reset to one unique type
		var_typ = VARTYP_INT;
		return *this;
	}

	// try to convert to numeric
	if (isnum())
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw MVNonNumeric(substr(1, 128) ^ "+= ");
}

// MULTIPLY INT

VARREF var::operator*=(const int int1) &{

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {
		var_dbl *= int1;
		// reset to one unique type
		var_typ = VARTYP_DBL;
		return *this;
	}

	// both int
	else if (var_typ & VARTYP_INT) {
		var_int *= int1;
		// reset to one unique type
		var_typ = VARTYP_INT;
		return *this;
	}

	// try to convert to numeric
	if (isnum())
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw MVNonNumeric(substr(1, 128) ^ "+= ");
}

//SUBTRACT INT

VARREF var::operator-=(const int int1) &{

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {
		var_dbl -= int1;
		// reset to one unique type
		var_typ = VARTYP_DBL;
		return *this;
	}

	// both int
	else if (var_typ & VARTYP_INT) {
		var_int -= int1;
		// reset to one unique type
		var_typ = VARTYP_INT;
		return *this;
	}

	// try to convert to numeric
	if (isnum())
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw MVNonNumeric(substr(1, 128) ^ "+= ");
}


//DIVIDE INT

VARREF var::operator/=(const int int1) &{

	// Always return double

	if (!int1)
		throw MVDivideByZero("div('" ^ this->substr(1, 128) ^ "', '" ^ int1 ^
		"')");

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL)
		var_dbl /= int1;

	// both int
	else if (var_typ & VARTYP_INT)
		var_dbl = static_cast<double>(var_int) / int1;

	// try to convert to numeric
	else if (isnum())
		goto tryagain;

	else
		assertNumeric(__PRETTY_FUNCTION__);

	// reset to one unique type
	var_typ = VARTYP_DBL;
	return *this;
}


//MODULO INT

VARREF var::operator%=(const int int1) &{

	if (!int1)
		throw MVDivideByZero("mod('" ^ this->substr(1, 128) ^ "', '" ^ int1 ^
		"')");

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {
		var_dbl = exodusmodulus(var_dbl, int1);
		// reset to one unique type
		var_typ = VARTYP_DBL;
		return *this;
	}

	// both int
	else if (var_typ & VARTYP_INT) {
		var_int %= int1;
		// reset to one unique type
		var_typ = VARTYP_INT;
		return *this;
	}

	// try to convert to numeric
	if (isnum())
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw MVNonNumeric(substr(1, 128) ^ "+= ");
}


///////
// CHAR
///////

// ADD CHAR

VARREF var::operator+=(const char char1) &{
	(*this) += var(char1);
	return *this;
}

// MULTIPLY CHAR

VARREF var::operator*=(const char char1) &{
	(*this) *= var(char1);
	return *this;
}

// SUBTRACT CHAR

VARREF var::operator-=(const char char1) &{
	(*this) -= var(char1);
	return *this;
}


// DIVIDE CHAR

VARREF var::operator/=(const char char1) &{
	(*this) /= var(char1);
	return *this;
}


// MODULO CHAR

VARREF var::operator%=(const char char1) &{
	(*this) %= var(char1);
	return *this;
}


///////
// CSTR
///////

// ADD CSTR

VARREF var::operator+=(const char* chars1) &{
	(*this) += var(chars1);
	return *this;
}

// MULTIPLY CSTR

VARREF var::operator*=(const char* chars1) &{
	(*this) *= var(chars1);
	return *this;
}

// SUBTRACT CSTR

VARREF var::operator-=(const char* chars1) &{
	(*this) -= var(chars1);
	return *this;
}

// DIVIDE CSTR

VARREF var::operator/=(const char* chars1) &{
	(*this) /= var(chars1);
	return *this;
}

// MODULO CSTR

VARREF var::operator%=(const char* chars1) &{
	(*this) %= var(chars1);
	return *this;
}

///////
// BOOL
///////

// ADD BOOL

VARREF var::operator+=(const bool bool1) &{
	if (bool1)
		(*this)++;
	else
		assertNumeric(__PRETTY_FUNCTION__);
	return *this;
}

// MULTIPLY BOOL

VARREF var::operator*=(const bool bool1) &{
	assertNumeric(__PRETTY_FUNCTION__);
	if (!bool1) {
		var_int = 0;
		var_typ = VARTYP_INT;
	}
	return *this;
}

// SUBTRACT BOOL

VARREF var::operator-=(const bool bool1) &{
	if (bool1)
		(*this)--;
	else
		assertNumeric(__PRETTY_FUNCTION__);
	return *this;
}

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
		return almost_equal_not_zero(x, static_cast<double>(y), ulp);
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

PUBLIC bool MVlt_bool(const bool lhs, const bool rhs) {

	//return true if lhs is "less true" than rhs

	//true  < true  -> false
	//true  < false -> false
	//false < true  -> TRUE
	//false < false -> false
	//
	//     rhs: true    false
	// lhs:     ----    -----
	// true     false   false
	// false    TRUE    false

	//std::clog << lhs << " " << rhs << " " << (rhs && !lhs) << std::endl;
	return rhs && !lhs;

}

// almost identical code in MVeq and MVlt except where noted
// NOTE doubles compare only to 0.0001 accuracy)
PUBLIC bool MVeq(CVR lhs, CVR rhs) {

	lhs.assertDefined(__PRETTY_FUNCTION__);
	rhs.assertDefined(__PRETTY_FUNCTION__);

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
					rhs.assertAssigned(__PRETTY_FUNCTION__);
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
				lhs.assertAssigned(__PRETTY_FUNCTION__);
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
				//return (std::abs(lhs.var_dbl - static_cast<double>(rhs.var_int)) < SMALLEST_NUMBER);
				return almost_equal(lhs.var_dbl, rhs.var_int, 2);
			}
		}

		//INT v DOUBLE
		//if (rhs.var_typ & VARTYP_INTd)
		if (rhs.var_typ & VARTYP_DBL) {
			// different from MVlt (uses absolute)
			// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)

			//return (lhs.var_dbl == rhs.var_int);
			//return (std::abs(static_cast<double>(lhs.var_int) - rhs.var_dbl) < SMALLEST_NUMBER);
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

	lhs.assertDefined(__PRETTY_FUNCTION__);
	rhs.assertDefined(__PRETTY_FUNCTION__);

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
					rhs.assertAssigned(__PRETTY_FUNCTION__);
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
				lhs.assertAssigned(__PRETTY_FUNCTION__);
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
				//return (static_cast<double>(lhs.var_int) < rhs.var_dbl);
				return (rhs.var_dbl - static_cast<double>(lhs.var_int)) >= SMALLEST_NUMBER;
			//return ((rhs.var_dbl - static_cast<double>(lhs.var_int) >= SMALLEST_NUMBER);
		}
		if (rhs.var_typ & VARTYP_INT)
			// different from MVeq
			// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)
			//return (lhs.var_dbl < static_cast<double>(rhs.var_int));
			return (static_cast<double>(rhs.var_int) - lhs.var_dbl) >= SMALLEST_NUMBER;
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
PUBLIC bool MVlt_int(CVR lhs, const int int2) {

	lhs.assertDefined(__PRETTY_FUNCTION__);

	// Empty string is always less than anything except another empty string
	if (lhs.var_typ & VARTYP_STR && lhs.var_str.empty())
			return true;

	// LHS numerical?
	do {

		//compare int is already available
		if (lhs.var_typ & VARTYP_INT)
			return (lhs.var_int < int2);

		//otherwise compare dbl
		if (lhs.var_typ & VARTYP_DBL)
			// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)
			return (static_cast<double>(int2) - lhs.var_dbl) >= SMALLEST_NUMBER;
	}

	// go back and try again if can be converted to number
	// Will check unassigned
	while (lhs.isnum());

	// Non-numerical lhs
	return lhs.var_str < std::to_string(int2);

}

// NOTE doubles compare only to 0.0001 accuracy)
PUBLIC bool MVlt_int(const int int1, CVR rhs) {

	rhs.assertDefined(__PRETTY_FUNCTION__);

	// Empty string is always less than anything except another empty string
	if (rhs.var_typ & VARTYP_STR && rhs.var_str.empty())
			return false;

	// RHS numerical?
	do {

		//compare int if already available
		if (rhs.var_typ & VARTYP_INT)
			return (int1 < rhs.var_int);

		//otherwise compare dbl
		if (rhs.var_typ & VARTYP_DBL) {
			// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)
			return (rhs.var_dbl - static_cast<double>(int1)) >= SMALLEST_NUMBER;
		}
	}
	// go back and try again if can be converted to number
	// Will check unassigned
	while (rhs.isnum());

	// Non-numerical rhs
	return std::to_string(int1) < rhs.var_str;
}

// NOTE doubles compare only to 0.0001 accuracy)
PUBLIC bool MVlt_dbl(CVR lhs, const double dbl2) {

	lhs.assertDefined(__PRETTY_FUNCTION__);

	// Empty string is always less than anything except another empty string
	if (lhs.var_typ & VARTYP_STR && lhs.var_str.empty())
		return true;

	// 2. LHS numerical?
	do {

		// LHS DBL
		if (lhs.var_typ & VARTYP_DBL)
			// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)
			return (dbl2 - lhs.var_dbl) >= SMALLEST_NUMBER;

		// LHS INT
		if (lhs.var_typ & VARTYP_INT)
			// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)
			return (dbl2 - static_cast<double>(lhs.var_int)) >= SMALLEST_NUMBER;

	}
	// go back and try again if can be converted to number
	// will throw if unassigned
	while (lhs.isnum());

	// Non-numerical lhs
	return lhs.var_str < std::to_string(dbl2);

}

// NOTE doubles compare only to 0.0001 accuracy)
PUBLIC bool MVlt_dbl(const double dbl1, CVR rhs) {

	rhs.assertDefined(__PRETTY_FUNCTION__);

	// Empty string is always less than anything except another empty string
	if (rhs.var_typ & VARTYP_STR && rhs.var_str.empty())
		return false;

	// RHS Numerical?
	do {

		// RHS double
		if (rhs.var_typ & VARTYP_DBL)
			// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)
			return (rhs.var_dbl - dbl1) >= SMALLEST_NUMBER;

		// RHS int
		if (rhs.var_typ & VARTYP_INT)
			// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)
			return (static_cast<double>(rhs.var_int) - dbl1) >= SMALLEST_NUMBER;

	}
	// go back and try again if can be converted to number
	// Will check assigned
	while (rhs.isnum());

	// Non-numerical rhs
	return std::to_string(dbl1) < rhs.var_str;
}

// NOTE doubles compare only to 0.0001 accuracy)
PUBLIC bool MVeq_dbl(CVR lhs, const double dbl1) {

	lhs.assertDefined(__PRETTY_FUNCTION__);

	// NB empty string is always less than anything except another empty string

	// 1. EMPTY STRING always false
	if (lhs.var_typ & VARTYP_STR && lhs.var_str.empty()) {
		//std::clog << "MVeq_dbl 1. Empty string can never equal any number" << std::endl;
		return false;
	}

	do {

		// 2. LHS DBL
		if (lhs.var_typ & VARTYP_DBL) {

			// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)
			//return (std::abs(lhs.var_dbl - rhs.var_dbl) < SMALLEST_NUMBER);
			//std::clog << "MVeq_dbl 2. lhs double " << lhs.var_dbl << " compare to double " << dbl1 << std::endl;
			return almost_equal(lhs.var_dbl, dbl1, 2);

		}

		// 3. LHS INT
		else if (lhs.var_typ & VARTYP_INT) {

			// LHS INT
			// different from MVlt (uses absolute)
			// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)

			//return (lhs.var_dbl == rhs.var_int);
			//return (std::abs(static_cast<double>(lhs.var_int) - rhs.var_dbl) < SMALLEST_NUMBER);
			//return almost_equal(lhs.var_int, rhs.var_dbl, 2);
			//put lhs int 2nd argument to invoke the fastest implmentation
			//std::clog << "MVeq_dbl 3. lhs int " << lhs.var_int << " compare to double " << dbl1 << std::endl;
			return almost_equal(dbl1, lhs.var_int, 2);

		}
	}
	// try to convert to numeric string
	while (lhs.isnum());

	// 4. NON-NUMERIC STRING - always false
	//std::clog << "MVeq_dbl 4. Non-numeric string can never equal any number" << std::endl;;
	return false;

}

// NOTE doubles compare only to 0.0001 accuracy)
PUBLIC bool MVeq_int(CVR lhs, const int int1) {

	lhs.assertDefined(__PRETTY_FUNCTION__);

	// 1. EMPTY STRING always false
	if (lhs.var_typ & VARTYP_STR && lhs.var_str.empty()) {
		//std::clog << "MVeq_int 1. Empty string can never equal any number" << std::endl;
		return false;
	}

	do {

		// 2. LHS DBL
		if (lhs.var_typ & VARTYP_DBL) {

			// (DOUBLES ONLY COMPARE TO ACCURACY SMALLEST_NUMBER was 0.0001)
			//return (std::abs(lhs.var_dbl - rhs.var_dbl) < SMALLEST_NUMBER);
			//std::clog << "MVeq_int 2. lhs double " << lhs.var_dbl << " compare to int " << int1 << std::endl;
			return almost_equal(lhs.var_dbl, int1, 2);

		}

		// 3. LHS INT
		else if (lhs.var_typ & VARTYP_INT) {
			//std::clog << "MVeq_int 3. lhs int " << lhs.var_int << " compare to int " << int1 << std::endl;
			return lhs.var_int == int1;

		}
	}
	// try to convert to numeric string
	while (lhs.isnum());

	// 4. NON-NUMERIC STRING - always false
	//std::clog << "MVeq_int 4. Non-numeric string can never equal any number" << std::endl;;
	return false;

}

// +var
var var::operator+() const{

	assertDefined(__PRETTY_FUNCTION__);
	assertNumeric(__PRETTY_FUNCTION__);

	return *this;
}

// -var
var var::operator-() const{

	assertDefined(__PRETTY_FUNCTION__);

	do {
		// dbl
		if (var_typ & VARTYP_DBL)
			return -var_dbl;

		// int
		if (var_typ & VARTYP_INT)
			return -var_int;

	}
	// must be string - try to convert to numeric
	while (this->isnum());

	// non-numeric
	this->assertNumeric(__PRETTY_FUNCTION__);
	// will never get here
	throw MVNonNumeric("+(" ^ this->substr(1, 128) ^ ")");
}

// !var
bool var::operator!() const {

	return !this->toBool();
}

// Replaced by operator+=
//var MVadd(CVR lhs, CVR rhs) {
//
//	lhs.assertNumeric(__PRETTY_FUNCTION__);
//	rhs.assertNumeric(__PRETTY_FUNCTION__);
//
//	//identical code in MVadd and MVsub except for +/-
//	//identical code in MVadd, MVsub, MVmul except for +,-,*
//	if (lhs.var_typ & VARTYP_DBL)
//		return lhs.var_dbl + ((rhs.var_typ & VARTYP_DBL) ? rhs.var_dbl : static_cast<double>(rhs.var_int));
//	else if (rhs.var_typ & VARTYP_DBL)
//		return lhs.var_int + rhs.var_dbl;
//	else
//		return lhs.var_int + rhs.var_int;
//}

// Replaced by operator-=
//var MVsub(CVR lhs, CVR rhs) {
//
//	lhs.assertNumeric(__PRETTY_FUNCTION__);
//	rhs.assertNumeric(__PRETTY_FUNCTION__);
//
//	//identical code in MVadd, MVsub, MVmul except for +,-,*
//	if (lhs.var_typ & VARTYP_DBL)
//		return lhs.var_dbl - ((rhs.var_typ & VARTYP_DBL) ? rhs.var_dbl : static_cast<double>(rhs.var_int));
//	else if (rhs.var_typ & VARTYP_DBL)
//		return lhs.var_int - rhs.var_dbl;
//	else
//		return lhs.var_int - rhs.var_int;
//}

// Replaced by operator*=
//var MVmul(CVR lhs, CVR rhs) {
//
//	lhs.assertNumeric(__PRETTY_FUNCTION__);
//	rhs.assertNumeric(__PRETTY_FUNCTION__);
//
//	//identical code in MVadd, MVsub, MVmul except for +,-,*
//	if (lhs.var_typ & VARTYP_DBL)
//		return lhs.var_dbl * ((rhs.var_typ & VARTYP_DBL) ? rhs.var_dbl : static_cast<double>(rhs.var_int));
//	else if (rhs.var_typ & VARTYP_DBL)
//		return lhs.var_int * rhs.var_dbl;
//	else
//		return lhs.var_int * rhs.var_int;
//}

// Replaced by operator/=
//var MVdiv(CVR lhs, CVR rhs) {
//
//	lhs.assertNumeric(__PRETTY_FUNCTION__);
//	rhs.assertNumeric(__PRETTY_FUNCTION__);
//
//	// always returns a double because 10/3 must be 3.3333333
//
//	if (lhs.var_typ & VARTYP_DBL) {
//		// 1. double ... double
//		if (rhs.var_typ & VARTYP_DBL) {
//			if (!rhs.var_dbl)
//				throw MVDivideByZero("div('" ^ lhs.substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
//									 "')");
//			return lhs.var_dbl / rhs.var_dbl;
//		}
//		// 2. double ... int
//		else {
//			if (!rhs.var_int)
//				throw MVDivideByZero("div('" ^ lhs.substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
//									 "')");
//			return lhs.var_dbl / rhs.var_int;
//		}
//	}
//	// 3. int ... double
//	else if (rhs.var_typ & VARTYP_DBL) {
//		if (!rhs.var_dbl)
//			throw MVDivideByZero("div('" ^ lhs.substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
//								 "')");
//		return static_cast<double>(lhs.var_int) / rhs.var_dbl;
//	}
//	// 4. int ... int
//	else {
//		if (!rhs.var_int)
//			throw MVDivideByZero("div('" ^ lhs.substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
//								 "')");
//		return static_cast<double>(lhs.var_int) / rhs.var_int;
//	}
//}
//

// Replaced by operator%=
//var MVmod(CVR lhs, CVR rhs) {
//
//	lhs.assertNumeric(__PRETTY_FUNCTION__);
//	rhs.assertNumeric(__PRETTY_FUNCTION__);
//
//	//returns an integer var IIF both arguments are integer vars
//	//otherwise returns a double var
//
//	if (lhs.var_typ & VARTYP_DBL) {
//		// 1. double ... double
//		if (rhs.var_typ & VARTYP_DBL) {
//			if (!rhs.var_dbl)
//				throw MVDivideByZero("mod('" ^ lhs.substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
//									 "')");
//			return exodusmodulus(lhs.var_dbl, rhs.var_dbl);
//		}
//		// 2. double ... int
//		else {
//			if (!rhs.var_int)
//				throw MVDivideByZero("mod('" ^ lhs.substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
//									 "')");
//			return exodusmodulus(lhs.var_dbl, rhs.var_int);
//		}
//	}
//	// 3. int ... double
//	else if (rhs.var_typ & VARTYP_DBL) {
//		if (!rhs.var_dbl)
//			throw MVDivideByZero("mod('" ^ lhs.substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
//								 "')");
//		return exodusmodulus(lhs.var_int, rhs.var_dbl);
//	}
//	// 4. int ... int
//	else {
//		if (!rhs.var_int)
//			throw MVDivideByZero("mod('" ^ lhs.substr(1, 128) ^ "', '" ^ rhs.substr(1, 128) ^
//								 "')");
//		//return exodusmodulus(lhs.var_int, rhs.var_int);
//		return lhs.var_int % rhs.var_int;
//	}
//}

// var^var we reassign the logical xor operator ^ to be string concatenate!!!
// slightly wrong precedence but at least we have a reliable concat operator to replace the + which
// is now reserved for forced ADDITION both according to fundamental PickOS principle
var MVcat_var_var(CVR lhs, CVR rhs) {

	lhs.assertString(__PRETTY_FUNCTION__);
	rhs.assertString(__PRETTY_FUNCTION__);

	return lhs.var_str + rhs.var_str;
}

var MVcat_var_cstr(CVR lhs, const char* cstr) {

	lhs.assertString(__PRETTY_FUNCTION__);

	return lhs.var_str + cstr;
}

var MVcat_cstr_var(const char* cstr, CVR rhs) {

	rhs.assertString(__PRETTY_FUNCTION__);

	return cstr + rhs.var_str;
}

var MVcat_var_char(CVR lhs, const char char2) {

	lhs.assertString(__PRETTY_FUNCTION__);

	return lhs.var_str + char2;
}
/*

VARREF var::operator^(CVR vstr) {

	assertString(__PRETTY_FUNCTION__);
	vstr.assertString(__PRETTY_FUNCTION__);

	var_str += vstr.var_str;

	return *this;
}

VARREF var::operator^(const char* cstr) {
	std::clog <<("var operator^(const char& cstr)") << std::endl;

	assertString(__PRETTY_FUNCTION__);

	var_str += cstr;

	return *this;
}

VARREF var::operator^(const std::string& stdstr) {

	assertString(__PRETTY_FUNCTION__);
	stdstr.assertString(__PRETTY_FUNCTION__);

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

	var1.assertString(__PRETTY_FUNCTION__);

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

	var1.assertDefined(__PRETTY_FUNCTION__);

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
	return var(static_cast<double>(d));
}

}  // namespace exodus
