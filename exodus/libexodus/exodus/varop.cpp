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
#include <string>
#include <utility> // for std::move

//#include <cassert>
#include <limits>
//#include <sstream>
//#include <vector>

#include <var.h>
#include <varerr.h>
//#include <exodus/varimpl.h>
//#include <exodus/exoimpl.h>

namespace exodus {

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
			[[unlikely]]
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
var var::operator--(int) & {

	// full check done below to avoid double checking number type
	assertDefined(__PRETTY_FUNCTION__);

	var priorvalue;

tryagain:
	// prefer int since -- nearly always on integers
	if (var_typ & VARTYP_INT) {
		if (var_int == std::numeric_limits<decltype(var_int)>::min())
			[[unlikely]]
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
VARREF var::operator++() & {

	// full check done below to avoid double checking number type
	assertDefined(__PRETTY_FUNCTION__);

tryagain:
	// prefer int since -- nearly always on integers
	if (var_typ & VARTYP_INT) {
		if (var_int == std::numeric_limits<decltype(var_int)>::max())
			[[unlikely]]
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
VARREF var::operator--() & {

	// full check done below to avoid double checking number type
	assertDefined(__PRETTY_FUNCTION__);

tryagain:
	// prefer int since -- nearly always on integers
	if (var_typ & VARTYP_INT) {
		if (var_int == std::numeric_limits<decltype(var_int)>::min())
			[[unlikely]]
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
		// warning: conversion from ‘exodus::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
		var_dbl += (rhs.var_typ & VARTYP_DBL) ? rhs.var_dbl : static_cast<double>(rhs.var_int);
		// reset lhs to one unique type
		var_typ = VARTYP_DBL;
		return *this;
	}

	// lhs int
	else if (var_typ & VARTYP_INT) {

		// rhs double
		if (rhs.var_typ & VARTYP_DBL) {
			// warning: conversion from ‘exodus::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
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
		[[likely]]
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw VarNonNumeric(this->first(128) ^ "+= ");
}

// MULTIPLY VAR

VARREF var::operator*=(CVR rhs) &{

	rhs.assertNumeric(__PRETTY_FUNCTION__);

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {
		// use rhs double if available otherwise use its int
		// warning: conversion from ‘exodus::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
		var_dbl *= (rhs.var_typ & VARTYP_DBL) ? rhs.var_dbl : static_cast<double>(rhs.var_int);
		// reset lhs to one unique type
		var_typ = VARTYP_DBL;
		return *this;
	}

	// lhs int
	else if (var_typ & VARTYP_INT) {

		// rhs double
		if (rhs.var_typ & VARTYP_DBL) {
			//warning: conversion from ‘exodus::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
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
		[[likely]]
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw VarNonNumeric(this->first(128) ^ "+= ");
}

// SUBTRACT VAR

VARREF var::operator-=(CVR rhs) &{

	rhs.assertNumeric(__PRETTY_FUNCTION__);

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {
		// use rhs double if available otherwise use its int
		// warning: conversion from ‘exodus::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
		var_dbl -= (rhs.var_typ & VARTYP_DBL) ? rhs.var_dbl : static_cast<double>(rhs.var_int);
		// reset lhs to one unique type
		var_typ = VARTYP_DBL;
		return *this;
	}

	// lhs int
	else if (var_typ & VARTYP_INT) {

		// use rhs double if available
		if (rhs.var_typ & VARTYP_DBL) {
			// warning: conversion from ‘exodus::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
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
		[[likely]]
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw VarNonNumeric(this->first(128) ^ "+= ");
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
				[[unlikely]]
				throw VarDivideByZero("div('" ^ this->first(128) ^ "', '" ^ rhs.first(128) ^
									 "')");
			var_dbl /= rhs.var_dbl;
		}

		// rhs double
		else {
			if (!rhs.var_int)
				[[unlikely]]
				throw VarDivideByZero("div('" ^ this->first(128) ^ "', '" ^ rhs.first(128) ^
									 "')");
			// warning: conversion from ‘exodus::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
			var_dbl /= static_cast<double>(rhs.var_int);
		}

	}

	// lhs int
	else if (var_typ & VARTYP_INT) {

		// rhs double
		if (rhs.var_typ & VARTYP_DBL) {
			if (!rhs.var_dbl)
				[[unlikely]]
				throw VarDivideByZero("div('" ^ this->first(128) ^ "', '" ^ rhs.first(128) ^
								 "')");
			// warning: conversion from ‘exodus::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
			var_dbl = static_cast<double>(this->var_int) / rhs.var_dbl;
		}

		// both are ints - must return a double
		else {
			if (!rhs.var_int)
				[[unlikely]]
				throw VarDivideByZero("div('" ^ this->first(128) ^ "', '" ^ rhs.first(128) ^
				"')");
			// warning: conversion from ‘exodus::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
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

VARREF var::operator%=(CVR rhs) &{
	*this = this->mod(rhs);
	return *this;
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
		// warning: conversion from ‘exodus::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
		var_dbl = static_cast<double>(var_int) + dbl1;

	// try to convert to numeric
	else if (isnum())
		goto tryagain;

	else [[unlikely]] {
		assertNumeric(__PRETTY_FUNCTION__);
		// Cant get here
		throw VarNonNumeric(this->first(128) ^ "+= ");
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
		// warning: conversion from ‘exodus::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
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

VARREF var::operator-=(const double dbl1) &{

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL)
		var_dbl -= dbl1;

	// lhs int
	else if (var_typ & VARTYP_INT)
		// warning: conversion from ‘exodus::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
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

VARREF var::operator/=(const double dbl1) &{

	if (!dbl1)
		[[unlikely]]
		throw VarDivideByZero("div('" ^ this->first(128) ^ "', '" ^ dbl1 ^
		"')");

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL)
		var_dbl /= dbl1;

	// lhs int
	else if (var_typ & VARTYP_INT)
		// warning: conversion from ‘exodus::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
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

VARREF var::operator%=(const double rhs) &{
	*this = this->mod(rhs);
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
		// warning: conversion from ‘exodus::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
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
		[[likely]]
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw VarNonNumeric(this->first(128) ^ "+= ");
}

// MULTIPLY INT

VARREF var::operator*=(const int int1) &{

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {
		// warning: conversion from ‘exodus::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
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
		[[likely]]
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw VarNonNumeric(this->first(128) ^ "+= ");
}

//SUBTRACT INT

VARREF var::operator-=(const int int1) &{

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL) {
		// warning: conversion from ‘exodus::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
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
		[[likely]]
		goto tryagain;

	assertNumeric(__PRETTY_FUNCTION__);

	// Cant get here
	throw VarNonNumeric(this->first(128) ^ "+= ");
}


//DIVIDE INT

VARREF var::operator/=(const int int1) &{

	// Always return double

	if (!int1)
		[[unlikely]]
		throw VarDivideByZero("div('" ^ this->first(128) ^ "', '" ^ int1 ^
		"')");

tryagain:

	// lhs double
	if (var_typ & VARTYP_DBL)
		// warning: conversion from ‘exodus::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
		var_dbl /= static_cast<double>(int1);

	// both int
	else if (var_typ & VARTYP_INT)
		// warning: conversion from ‘exodus::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
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

VARREF var::operator%=(const int rhs) &{
	*this = this->mod(rhs);
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

// +var
var var::operator+() const{

	assertDefined(__PRETTY_FUNCTION__);
	assertNumeric(__PRETTY_FUNCTION__);

	return *this;
}

// -var
var var::operator-() const{

	assertDefined(__PRETTY_FUNCTION__);

	do [[likely]] {
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
	throw VarNonNumeric("+(" ^ this->first(128) ^ ")");
}

// var^var we reassign the logical xor operator ^ to be string concatenate!!!
// slightly wrong precedence but at least we have a reliable concat operator to replace the + which
// is now reserved for forced ADDITION both according to fundamental PickOS principle
var var_cat_var(CVR lhs, CVR rhs) {

	lhs.assertString(__PRETTY_FUNCTION__);
	rhs.assertString(__PRETTY_FUNCTION__);

	// return lhs.var_str + rhs.var_str;

	var result(lhs.var_str);
	result.var_str.append(rhs.var_str);

	return result;
}

var var_cat_cstr(CVR lhs, const char* rhs) {

	lhs.assertString(__PRETTY_FUNCTION__);

	//return lhs.var_str + cstr;

	var result(lhs.var_str);
	result.var_str.append(rhs);

	return result;
}

var var_cat_char(CVR lhs, const char rhs) {

	lhs.assertString(__PRETTY_FUNCTION__);

	//return lhs.var_str + char2;

	var result(lhs.var_str);
	result.var_str.push_back(rhs);

	return result;
}

var cstr_cat_var(const char* lhs, CVR rhs) {

	rhs.assertString(__PRETTY_FUNCTION__);

	//return cstr + rhs.var_str;

	var result(lhs);
	result.var_str.append(rhs.var_str);

	return result;
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

std::istream& operator>>(std::istream& istream1, VARREF into_str1) {

	into_str1.assertDefined(__PRETTY_FUNCTION__);

	into_str1.var_str.clear();
	into_str1.var_typ = VARTYP_STR;

	//std::string tempstr;
	istream1 >> std::noskipws >> into_str1.var_str;

	// this would verify all input is valid utf8
	// into_str1.var_str=boost::locale::conv::utf_to_utf<char>(into_str1)

	return istream1;
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

//#endif

} // namespace exodus
