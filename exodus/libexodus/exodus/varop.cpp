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

#if EXO_MODULE
	import std;
#else
#	include <iostream>
#	include <string>
//#	include <utility> // for std::move
//#	include <array>
//
//#include <cassert>
#	include <limits>
////#include <sstream>
////#include <vector>
#endif

#include <exodus/var.h>
#include <exodus/varerr.h>
#include <exodus/varimpl.h>
#include <exodus/exoimpl.h>

namespace exo {

// Originally most help from Thinking in C++ Volume 1 Chapter 12
// http://www.camtp.uni-mb.si/books/Thinking-in-C++/TIC2Vone-distribution/html/Chapter12.html

// Also - very clearly written
// Addison Wesley Longman, Inc.
// C++ Primer, Third Edition 1998 Stanley B Lippman
// Chapter 15 Overloaded operators and User-Defined Conversions

// Could also use http://www.informit.com/articles/article.asp?p=25264&seqNum=1
// which is effectively about makeing objects behave like ordinary variable syntactically
// implementing smartpointers

//^= is not templated since slightly slower to go through the creation of an var()

//^=var
// The assignment operator must always return a reference to *this.
template<> PUBLIC VBR1 VARBASE1::operator^=(CBX rhs) & {

	assertString(__PRETTY_FUNCTION__);
	rhs.assertString(__PRETTY_FUNCTION__);

	// tack it onto our string
	var_str.append(rhs.var_str);
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//^=int
// The assignment operator must always return a reference to *this.
template<> PUBLIC VBR1 VARBASE1::operator^=(const int int1) & {

	assertString(__PRETTY_FUNCTION__);

	// var_str+=var(int1).var_str;
	var_str += std::to_string(int1);
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//^=double
// The assignment operator must always return a reference to *this.
template<> PUBLIC VBR1 VARBASE1::operator^=(const double double1) & {

	assertString(__PRETTY_FUNCTION__);

	// var_str+=var(int1).var_str;
	//var_str += mvd2s(double1);
	//var_typ = VARTYP_STR;  // reset to one unique type
	var temp(double1);
	temp.createString();
	var_str += temp.var_str;

	return *this;
}

//^=char
// The assignment operator must always return a reference to *this.
template<> PUBLIC VBR1 VARBASE1::operator^=(const char char1) & {

	assertString(__PRETTY_FUNCTION__);

	// var_str+=var(int1).var_str;
	var_str.push_back(char1);
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

#ifdef NOT_TEMPLATED_APPEND

//^=char*
// The assignment operator must always return a reference to *this.
template<> PUBLIC VBR1 VARBASE1::operator^=(const char* cstr) & {

	assertString(__PRETTY_FUNCTION__);

	// var_str+=var(int1).var_str;
	// var_str+=std::string(char1);
	var_str += cstr;
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//^=std::string
// The assignment operator must always return a reference to *this.
template<> PUBLIC VBR1 VARBASE1::operator^=(const std::string& string1) & {

	assertString(__PRETTY_FUNCTION__);

	// var_str+=var(int1).var_str;
	var_str += string1;
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

//^=std::string_view
// The assignment operator must always return a reference to *this.
template<> PUBLIC VBR1 VARBASE1::operator^=(SV sv1) & {

	assertString(__PRETTY_FUNCTION__);

	// var_str+=var(int1).var_str;
	var_str += sv1;
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}
#endif // NOT_TEMPLATED_APPEND

// You must *not* make the postfix version return the 'this' object by reference
//
// *** YOU HAVE BEEN WARNED ***
// not returning void so is usable in expressions
// int argument indicates that this is POSTFIX override v++
template<> PUBLIC RETVAR VARBASE1::operator++(int) & {

	// full check done below to avoid double checking number type
	assertDefined(__PRETTY_FUNCTION__);

	var priorvalue;

tryagain:
	// prefer int since ++ nearly always on integers
	if (var_typ & VARTYP_INT) {
		if (var_int == std::numeric_limits<decltype(var_int)>::max())
			UNLIKELY
			throw VarNumOverflow("operator++");
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

		//trigger VarNonNumeric
		assertNumeric(__PRETTY_FUNCTION__);

	} else {
		//trigger VarUnassigned
		assertNumeric(__PRETTY_FUNCTION__);
	}

	// NO DO NOT! return *this ... postfix return a temporary!!! eg var(*this)
	return priorvalue;
}

// not returning void so is usable in expressions
// int argument indicates that this is POSTFIX override v--
template<> PUBLIC RETVAR VARBASE1::operator--(int) & {

	// full check done below to avoid double checking number type
	assertDefined(__PRETTY_FUNCTION__);

	var priorvalue;

tryagain:
	// prefer int since -- nearly always on integers
	if (var_typ & VARTYP_INT) {
		if (var_int == std::numeric_limits<decltype(var_int)>::min())
			UNLIKELY
			throw VarNumUnderflow("operator--");
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

		//trigger VarNonNumeric
		assertNumeric(__PRETTY_FUNCTION__);

	} else {
		//trigger VarUnassigned
		assertNumeric(__PRETTY_FUNCTION__);
	}

	return priorvalue;
}

// not returning void so is usable in expressions
// no argument indicates that this is prefix override ++var
template<> PUBLIC VBR1 VARBASE1::operator++() & {

	// full check done below to avoid double checking number type
	assertDefined(__PRETTY_FUNCTION__);

tryagain:
	// prefer int since -- nearly always on integers
	if (var_typ & VARTYP_INT) {
		if (var_int == std::numeric_limits<decltype(var_int)>::max())
			UNLIKELY
			throw VarNumOverflow("operator++");
		var_int++;
		var_typ = VARTYP_INT;  // reset to one unique type
	} else if (var_typ & VARTYP_DBL) {
		var_dbl++;
		var_typ = VARTYP_DBL;  // reset to one unique type
	} else if (var_typ & VARTYP_STR) {
		// try to convert to numeric
		if (isnum())
			goto tryagain;

		//trigger VarNonNumeric
		assertNumeric(__PRETTY_FUNCTION__);

	} else {
		//trigger VarUnassigned
		assertNumeric(__PRETTY_FUNCTION__);
	}

	// OK to return *this in prefix ++
	return *this;
}

// not returning void so is usable in expressions
// no argument indicates that this is prefix override --var
template<> PUBLIC VBR1 VARBASE1::operator--() & {

	// full check done below to avoid double checking number type
	assertDefined(__PRETTY_FUNCTION__);

tryagain:
	// prefer int since -- nearly always on integers
	if (var_typ & VARTYP_INT) {
		if (var_int == std::numeric_limits<decltype(var_int)>::min())
			UNLIKELY
			throw VarNumUnderflow("operator--");
		var_int--;
		var_typ = VARTYP_INT;  // reset to one unique type

	} else if (var_typ & VARTYP_DBL) {
		var_dbl--;
		var_typ = VARTYP_DBL;  // reset to one unique type

	} else if (var_typ & VARTYP_STR) {
		// try to convert to numeric
		if (isnum())
			goto tryagain;

		//trigger VarNonNumeric
		assertNumeric(__PRETTY_FUNCTION__);

	} else {
		//trigger VarUnassigned
		assertNumeric(__PRETTY_FUNCTION__);
	}

	// OK to return *this in prefix --
	return *this;
}

/////////////////////////////////////////
// VAR Prefix/Postfix increment/decrement
/////////////////////////////////////////

//Forward to var_base

var  var::operator++(int) & {
	var orig = this->clone();
	var_base::operator++();
	return orig;
}

var  var::operator--(int) & {
	var orig = this->clone();
	var_base::operator--(0);
	return orig;
}

var& var::operator++() & {
	var_base::operator++(0);
	return *this;
}

var& var::operator--() & {
	var_base::operator--();
	return *this;
}

//////////////
// SELF ASSIGN
//////////////

//////
// VAR
//////

// ADD VAR

template<> PUBLIC VBR1 VARBASE1::operator+=(CBX rhs) & {

	rhs.assertNumeric(__PRETTY_FUNCTION__);

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {
		// use rhs double if available otherwise use its int
		// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
		var_dbl += (rhs.var_typ & VARTYP_DBL) ? rhs.var_dbl : static_cast<double>(rhs.var_int);
		// reset lhs to one unique type
		var_typ = VARTYP_DBL;
		return *this;
	}

	// lhs int
	else if (var_typ & VARTYP_INT) {

		// rhs double
		if (rhs.var_typ & VARTYP_DBL) {
			// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
			var_dbl = static_cast<double>(var_int) + rhs.var_dbl;
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
		LIKELY
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw VarNonNumeric(this->first_(128) ^ "+= ");
}

// MULTIPLY VAR

template<> PUBLIC VBR1 VARBASE1::operator*=(CBX rhs) & {

	rhs.assertNumeric(__PRETTY_FUNCTION__);

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {
		// use rhs double if available otherwise use its int
		// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
		var_dbl *= (rhs.var_typ & VARTYP_DBL) ? rhs.var_dbl : static_cast<double>(rhs.var_int);
		// reset lhs to one unique type
		var_typ = VARTYP_DBL;
		return *this;
	}

	// lhs int
	else if (var_typ & VARTYP_INT) {

		// rhs double
		if (rhs.var_typ & VARTYP_DBL) {
			//warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
			var_dbl = static_cast<double>(var_int) * rhs.var_dbl;
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
		LIKELY
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw VarNonNumeric(this->first_(128) ^ "+= ");
}

// SUBTRACT VAR

template<> PUBLIC VBR1 VARBASE1::operator-=(CBX rhs) & {

	rhs.assertNumeric(__PRETTY_FUNCTION__);

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {
		// use rhs double if available otherwise use its int
		// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
		var_dbl -= (rhs.var_typ & VARTYP_DBL) ? rhs.var_dbl : static_cast<double>(rhs.var_int);
		// reset lhs to one unique type
		var_typ = VARTYP_DBL;
		return *this;
	}

	// lhs int
	else if (var_typ & VARTYP_INT) {

		// use rhs double if available
		if (rhs.var_typ & VARTYP_DBL) {
			// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
			var_dbl = static_cast<double>(var_int) - rhs.var_dbl;
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
		LIKELY
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw VarNonNumeric(this->first_(128) ^ "+= ");
}


// DIVIDE VAR

template<> PUBLIC VBR1 VARBASE1::operator/=(CBX rhs) & {

	// Always returns a double because
	// 10/3 must be 3.3333333

	rhs.assertNumeric(__PRETTY_FUNCTION__);

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {

		// rhs double
		if (rhs.var_typ & VARTYP_DBL) {
			if (!rhs.var_dbl)
				UNLIKELY
				throw VarDivideByZero("div('" ^ this->first_(128) ^ "', '" ^ rhs.first_(128) ^
									 "')");
			var_dbl /= rhs.var_dbl;
		}

		// rhs double
		else {
			if (!rhs.var_int)
				UNLIKELY
				throw VarDivideByZero("div('" ^ this->first_(128) ^ "', '" ^ rhs.first_(128) ^
									 "')");
			// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
			var_dbl /= static_cast<double>(rhs.var_int);
		}

	}

	// lhs int
	else if (var_typ & VARTYP_INT) {

		// rhs double
		if (rhs.var_typ & VARTYP_DBL) {
			if (!rhs.var_dbl)
				UNLIKELY
				throw VarDivideByZero("div('" ^ this->first_(128) ^ "', '" ^ rhs.first_(128) ^
								 "')");
			// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
			var_dbl = static_cast<double>(this->var_int) / rhs.var_dbl;
		}

		// both are ints - must return a double
		else {
			if (!rhs.var_int)
				UNLIKELY
				throw VarDivideByZero("div('" ^ this->first_(128) ^ "', '" ^ rhs.first_(128) ^
				"')");
			// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
			var_dbl = static_cast<double>(this->var_int) / static_cast<double>(rhs.var_int);
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

template<> PUBLIC VBR1 VARBASE1::operator%=(CBX rhs) & {
	*this = this->mod(rhs);
	return *this;
}

/////////
// DOUBLE
/////////

// ADD DOUBLE

template<> PUBLIC VBR1 VARBASE1::operator+=(const double dbl1) & {

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL)
		var_dbl += dbl1;

	// lhs int
	else if (var_typ & VARTYP_INT)
		// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
		var_dbl = static_cast<double>(var_int) + dbl1;

	// try to convert to numeric
	else if (isnum())
		goto tryagain;

	else UNLIKELY {
		assertNumeric(__PRETTY_FUNCTION__);
		// Cant get here
		throw VarNonNumeric(this->first_(128) ^ "+= ");
	}

	// reset to one unique type
	var_typ = VARTYP_DBL;

	return *this;
}

// MULTIPLY DOUBLE

template<> PUBLIC VBR1 VARBASE1::operator*=(const double dbl1) & {

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL)
		var_dbl *= dbl1;

	// lhs int
	else if (var_typ & VARTYP_INT)
		// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
		var_dbl = static_cast<double>(var_int) * dbl1;

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

template<> PUBLIC VBR1 VARBASE1::operator-=(const double dbl1) & {

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL)
		var_dbl -= dbl1;

	// lhs int
	else if (var_typ & VARTYP_INT)
		// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
		var_dbl = static_cast<double>(var_int) - dbl1;

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

template<> PUBLIC VBR1 VARBASE1::operator/=(const double dbl1) & {

	if (!dbl1)
		UNLIKELY
		throw VarDivideByZero("div('" ^ this->first_(128) ^ "', '" ^ dbl1 ^
		"')");

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL)
		var_dbl /= dbl1;

	// lhs int
	else if (var_typ & VARTYP_INT)
		// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
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

template<> PUBLIC VBR1 VARBASE1::operator%=(const double rhs) & {
	*this = this->mod(rhs);
	return *this;
}

//////
// INT
//////

// ADD INT

template<> PUBLIC VBR1 VARBASE1::operator+=(const int int1) & {

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {
		// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
		var_dbl += static_cast<double>(int1);
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
		LIKELY
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw VarNonNumeric(this->first_(128) ^ "+= ");
}

// MULTIPLY INT

template<> PUBLIC VBR1 VARBASE1::operator*=(const int int1) & {

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {
		// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
		var_dbl *= static_cast<double>(int1);
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
		LIKELY
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw VarNonNumeric(this->first_(128) ^ "+= ");
}

//SUBTRACT INT

template<> PUBLIC VBR1 VARBASE1::operator-=(const int int1) & {

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {
		// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
		var_dbl -= static_cast<double>(int1);
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
		LIKELY
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw VarNonNumeric(this->first_(128) ^ "+= ");
}


//DIVIDE INT

template<> PUBLIC VBR1 VARBASE1::operator/=(const int int1) & {

	// Always return double

	if (!int1)
		UNLIKELY
		throw VarDivideByZero("div('" ^ this->first_(128) ^ "', '" ^ int1 ^
		"')");

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL)
		// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
		var_dbl /= static_cast<double>(int1);

	// both int
	else if (var_typ & VARTYP_INT)
		// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
		var_dbl = static_cast<double>(var_int) / static_cast<double>(int1);

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

template<> PUBLIC VBR1 VARBASE1::operator%=(const int rhs) & {
	*this = this->mod(rhs);
	return *this;
}

///////
// BOOL
///////

// ADD BOOL

template<> PUBLIC VBR1 VARBASE1::operator+=(const bool bool1) & {
	if (bool1)
		(*this)++;
	else
		assertNumeric(__PRETTY_FUNCTION__);
	return *this;
}

// MULTIPLY BOOL

template<> PUBLIC VBR1 VARBASE1::operator*=(const bool bool1) & {
	assertNumeric(__PRETTY_FUNCTION__);
	if (!bool1) {
		var_int = 0;
		var_typ = VARTYP_INT;
	}
	return *this;
}

// SUBTRACT BOOL

template<> PUBLIC VBR1 VARBASE1::operator-=(const bool bool1) & {
	if (bool1)
		(*this)--;
	else
		assertNumeric(__PRETTY_FUNCTION__);
	return *this;
}

// +var
template<> PUBLIC RETVAR VARBASE1::operator+() const {

	assertDefined(__PRETTY_FUNCTION__);
	assertNumeric(__PRETTY_FUNCTION__);

	//return static_cast<RETVAR>(*this);
	return *static_cast<const RETVAR*>(this);
}

// -var
template<> PUBLIC RETVAR VARBASE1::operator-() const {

	assertDefined(__PRETTY_FUNCTION__);

	do LIKELY {
		// dbl
		if (var_typ & VARTYP_DBL)
			return -var_dbl;

		// int
		if (var_typ & VARTYP_INT)
			return -var_int;

		// Try to convert to number and try again
		// Will check unassigned

	} while (this->isnum());

	// non-numeric
	this->assertNumeric(__PRETTY_FUNCTION__);

	// will never get here
	throw VarNonNumeric("+(" ^ this->first_(128) ^ ")");
}

// var^var we reassign the logical xor operator ^ to be string concatenate!!!
// slightly wrong precedence but at least we have a reliable concat operator to replace the + which
// is now reserved for forced ADDITION both according to fundamental PickOS principle
RETVAR var_cat_var(CBR lhs, CBR rhs) {

	lhs.assertString(__PRETTY_FUNCTION__);
	rhs.assertString(__PRETTY_FUNCTION__);
	// return lhs.var_str + rhs.var_str;

	var result(lhs.var_str);
	result.var_str.append(rhs.var_str);

	return result;
}

RETVAR var_cat_cstr(CBR lhs, const char* rhs) {

	lhs.assertString(__PRETTY_FUNCTION__);

	//return lhs.var_str + cstr;

	var result(lhs.var_str);
	result.var_str.append(rhs);

	return result;
}

RETVAR var_cat_char(CBR lhs, const char rhs) {

	lhs.assertString(__PRETTY_FUNCTION__);

	//return lhs.var_str + char2;

	var result(lhs.var_str);
	result.var_str.push_back(rhs);

	return result;
}

RETVAR cstr_cat_var(const char* lhs, CBR rhs) {

	rhs.assertString(__PRETTY_FUNCTION__);

	//return cstr + rhs.var_str;

	var result(lhs);
	result.var_str.append(rhs.var_str);

	return result;
}

////////////////////
// _var user literal
////////////////////

// "abc^def"_var
ND var operator""_var(const char* cstr, std::size_t size) {
    return var(cstr, size).fmiconverter();
}

// 123456_var
ND var operator""_var(unsigned long long int i) {
    return var(i);
}

// 123.456_var
ND var operator""_var(long double d) {
    return var(d);
}

} // namespace exo
