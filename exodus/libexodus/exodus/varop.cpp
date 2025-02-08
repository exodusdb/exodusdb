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

#ifdef SELF_OP_ARE_CHAINABLE
#	define VBR_THIS (*this)
#else
#	undef VBR1
#	define VBR1 void
#	define VBR_THIS
#endif

//////////////////
// SELF ASSIGN VAR
//////////////////

// VAR += VAR
/////////////

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
		return VBR_THIS;
	}

	// lhs int
	else if (var_typ & VARTYP_INT) {

		// rhs double
		if (rhs.var_typ & VARTYP_DBL) {
			// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
			var_dbl = static_cast<double>(var_int) + rhs.var_dbl;
			// reset lhs to one unique type
			var_typ = VARTYP_DBL;
			return VBR_THIS;
		}

		// both are ints
		var_int += rhs.var_int;
		// reset lhs to one unique type
		var_typ = VARTYP_INT;
		return VBR_THIS;
	}

	// try to convert to numeric
	if (isnum())
		LIKELY
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw VarNonNumeric(this->first_(128) ^ "+= ");
}

// VAR -= VAR
/////////////

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
		return VBR_THIS;
	}

	// lhs int
	else if (var_typ & VARTYP_INT) {

		// use rhs double if available
		if (rhs.var_typ & VARTYP_DBL) {
			// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
			var_dbl = static_cast<double>(var_int) - rhs.var_dbl;
			// reset lhs to one unique type
			var_typ = VARTYP_DBL;
			return VBR_THIS;
		}

		// both are ints
		var_int -= rhs.var_int;
		// reset lhs to one unique type
		var_typ = VARTYP_INT;
		return VBR_THIS;
	}

	// try to convert to numeric
	if (isnum())
		LIKELY
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw VarNonNumeric(this->first_(128) ^ "+= ");
}

// VAR *= VAR
/////////////

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
		return VBR_THIS;
	}

	// lhs int
	else if (var_typ & VARTYP_INT) {

		// rhs double
		if (rhs.var_typ & VARTYP_DBL) {
			//warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
			var_dbl = static_cast<double>(var_int) * rhs.var_dbl;
			// reset lhs to one unique type
			var_typ = VARTYP_DBL;
			return VBR_THIS;
		}

		// both are ints
		var_int *= rhs.var_int;
		// reset lhs to one unique type
		var_typ = VARTYP_INT;
		return VBR_THIS;
	}

	// try to convert to numeric
	if (isnum())
		LIKELY
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw VarNonNumeric(this->first_(128) ^ "+= ");
}

// VAR /= VAR
/////////////

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

	return VBR_THIS;
}

// VAR %= VAR
/////////////

template<> PUBLIC VBR1 VARBASE1::operator%=(CBX rhs) & {
	*this = this->mod(rhs);
	return VBR_THIS;
}

/////////////////////
// SELF ASSIGN DOUBLE
/////////////////////

// VAR += double
////////////////

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

	return VBR_THIS;
}

// VAR -= DOUBLE
////////////////

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

	return VBR_THIS;
}

// VAR *= DOUBLE
////////////////

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

	return VBR_THIS;
}

// VAR /= double
////////////////

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

	return VBR_THIS;
}

// VAR %= double
////////////////

template<> PUBLIC VBR1 VARBASE1::operator%=(const double rhs) & {
	*this = this->mod(rhs);
	return VBR_THIS;
}

//////////////////
// SELF ASSIGN INT
//////////////////

// VAR += int
/////////////

template<> PUBLIC VBR1 VARBASE1::operator+=(const int int1) & {

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {
		// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
		var_dbl += static_cast<double>(int1);
		// reset to one unique type
		var_typ = VARTYP_DBL;
		return VBR_THIS;
	}

	// both int
	else if (var_typ & VARTYP_INT) {
		var_int += int1;
		// reset to one unique type
		var_typ = VARTYP_INT;
		return VBR_THIS;
	}

	// try to convert to numeric
	if (isnum())
		LIKELY
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw VarNonNumeric(this->first_(128) ^ "+= ");
}

// VAR -= int
/////////////

template<> PUBLIC VBR1 VARBASE1::operator-=(const int int1) & {

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {
		// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
		var_dbl -= static_cast<double>(int1);
		// reset to one unique type
		var_typ = VARTYP_DBL;
		return VBR_THIS;
	}

	// both int
	else if (var_typ & VARTYP_INT) {
		var_int -= int1;
		// reset to one unique type
		var_typ = VARTYP_INT;
		return VBR_THIS;
	}

	// try to convert to numeric
	if (isnum())
		LIKELY
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw VarNonNumeric(this->first_(128) ^ "+= ");
}


// VAR *= int
/////////////

template<> PUBLIC VBR1 VARBASE1::operator*=(const int int1) & {

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {
		// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
		var_dbl *= static_cast<double>(int1);
		// reset to one unique type
		var_typ = VARTYP_DBL;
		return VBR_THIS;
	}

	// both int
	else if (var_typ & VARTYP_INT) {
		var_int *= int1;
		// reset to one unique type
		var_typ = VARTYP_INT;
		return VBR_THIS;
	}

	// try to convert to numeric
	if (isnum())
		LIKELY
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw VarNonNumeric(this->first_(128) ^ "+= ");
}

// VAR /= int
/////////////

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
	return VBR_THIS;
}


// VAR %= int
/////////////

template<> PUBLIC VBR1 VARBASE1::operator%=(const int rhs) & {
	*this = this->mod(rhs);
	return VBR_THIS;
}

///////////////////
// SELF ASSIGN BOOL
///////////////////

// VAR += bool

template<> PUBLIC VBR1 VARBASE1::operator+=(const bool bool1) & {
	if (bool1)
		(*this)++;
	else
		assertNumeric(__PRETTY_FUNCTION__);
	return VBR_THIS;
}

// VAR -= bool

template<> PUBLIC VBR1 VARBASE1::operator-=(const bool bool1) & {
	if (bool1)
		(*this)--;
	else
		assertNumeric(__PRETTY_FUNCTION__);
	return VBR_THIS;
}

// VAR *= bool

template<> PUBLIC VBR1 VARBASE1::operator*=(const bool bool1) & {
	assertNumeric(__PRETTY_FUNCTION__);
	if (!bool1) {
		var_int = 0;
		var_typ = VARTYP_INT;
	}
	return VBR_THIS;
}

// VAR /= bool

// Doesnt exist because divide by zero if false is runtime error

} // namespace exo
