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
#	include <string>
#	include <limits>
#endif

#include <exodus/varb.h>
#include <exodus/var.h>

namespace exo {

/////////////
/// AT and []
/////////////

RETVAR VB1::at(const int charno) const {

	THISIS("var  var::at(const int charno) const")
	assertString(function_sig);

	int nchars = static_cast<int>(var_str.size());

	// beyond end of string return ""
	// get this test out of the way first since it only has to be done later on anyway
	if (charno > nchars)
		return "";

	// within string return the character
	// handle positive indexing first for speed on the assumption
	// that it is commoner than negative indexing
	if (charno > 0)
		return var_str[charno - 1];

	// character 0 return the first character or "" if none
	// have to get this special case out of the way first
	// despite it being unusual
	// since it only has to be done later anyway
	if (charno == 0) {
		if (nchars)
			return var_str[0];
		else
			return "";
	}

	// have to check this later so check it now
	if (!nchars)
		return "";

	// convert negative index to positive index
	int charno2 = nchars + charno;

	// if index is now 0 or positive then return the character
	if (charno2 >= 0)
		return var_str[charno2];	// no need for -1 here

	// otherwise so negative as to point before beginning of string
	// and rule is to return the first character in that case
	return var_str[0];
}

// Deprecated
RETVAR VB1::operator[](const int pos1) const {
	return this->at(pos1);
}

///////////////////
// UNARY PLUS/MINUS
///////////////////

// +var

RETVAR VB1::operator+() const {

	assertVar(__PRETTY_FUNCTION__);
	assertNumeric(__PRETTY_FUNCTION__);

	//return static_cast<RETVAR>(*this);
	return *static_cast<const RETVAR*>(this);
}

// -var

RETVAR VB1::operator-() const {

	assertVar(__PRETTY_FUNCTION__);

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


/////////////////
// SELF CONCAT ^=
/////////////////

// ^= var

VBR1 VB1::operator^=(CBR rhs) & {

	assertString(__PRETTY_FUNCTION__);
	rhs.assertString(__PRETTY_FUNCTION__);

	if (var_str.empty()) {
//		(this*) = rhs.clone();
		var_typ = rhs.var_typ;
		var_int = rhs.var_int;
		var_dbl = rhs.var_dbl;
		var_str = rhs.var_str;
	} else {
		// Append it onto our string
		var_str.append(rhs.var_str);
		var_typ = VARTYP_STR;  // Reset to string only
	}

	return *this;
}

VBR1 VB1::operator^=(TBR rhs) & {

	assertString(__PRETTY_FUNCTION__);
	rhs.assertString(__PRETTY_FUNCTION__);

	if (var_str.empty()) {
		var_typ = rhs.var_typ;
		var_int = rhs.var_int;
		var_dbl = rhs.var_dbl;
		var_str = std::move(rhs.var_str);
	} else {
		// Append it onto our string
		var_str.append(rhs.var_str);
		var_typ = VARTYP_STR;  // Reset to string only
	}

	return *this;
}

// ^= int

VBR1 VB1::operator^=(const int int1) & {

	assertString(__PRETTY_FUNCTION__);

	if (var_str.empty()) {
		var_str = std::to_string(int1);
		var_int = int1;
		var_typ = VARTYP_INTSTR;
	} else {
		var_str += std::to_string(int1);
		var_typ = VARTYP_STR;  // Reset to string only
	}

	return *this;
}

// ^= double

VBR1 VB1::operator^=(const double double1) & {

	assertString(__PRETTY_FUNCTION__);

	var temp = double1;
	temp.createString();
	if (var_str.empty()) {
		*this = temp;
	} else {
		var_str += temp.var_str;
		var_typ = VARTYP_STR; // Reset to string only
	}

	return *this;
}

// ^= char

VBR1 VB1::operator^=(const char char1) & {

	assertString(__PRETTY_FUNCTION__);

	// var_str+=var(int1).var_str;
	var_str.push_back(char1);
	var_typ = VARTYP_STR;  // Reset to string only

	return *this;
}

#ifdef NOT_TEMPLATED_APPEND

// ^= char*

VBR1 VB1::operator^=(const char* cstr) & {

	assertString(__PRETTY_FUNCTION__);

	var_str += cstr;
	var_typ = VARTYP_STR;  // Reset to string only

	return *this;
}

// ^= std::string

VBR1 VB1::operator^=(const std::string& string1) & {

	assertString(__PRETTY_FUNCTION__);

	if (var_str.empty()) {
		// TODO move if temporary
		var_str = string1;
	} else {
		var_str += string1;
	}
	var_typ = VARTYP_STR;  // Reset to string only

	return *this;
}

// ^= std::string_view

VBR1 VB1::operator^=(SV sv1) & {

	assertString(__PRETTY_FUNCTION__);

	var_str += sv1;
	var_typ = VARTYP_STR;  // Reset to string only

	return *this;
}

#endif // NOT_TEMPLATED_APPEND


///////////////////////////////////
// SELF INCREMENT/DECREMENT POSTFIX
///////////////////////////////////

// VAR ++

// You must *not* make the postfix version return the 'this' object by reference
//
// *** YOU HAVE BEEN WARNED ***
// Not returning void so is usable in expressions
// The int argument indicates that this is POSTFIX override v++
RETVAR VB1::operator++(int) & {

	// Full check done below to avoid double checking number type
	assertVar(__PRETTY_FUNCTION__);

	var priorvalue;

tryagain:
	// Prefer int since ++ nearly always on integers
	if (var_typ & VARTYP_INT) {
		if (var_int == std::numeric_limits<decltype(var_int)>::max())
			UNLIKELY
			throw VarNumOverflow("operator++");
		priorvalue = var(var_int);
		var_int++;
		var_typ = VARTYP_INT;  // Reset to one unique type

	} else if (var_typ & VARTYP_DBL) {
		priorvalue = var_dbl;
		var_dbl++;
		var_typ = VARTYP_DBL;  // Reset to one unique type

	} else if (var_typ & VARTYP_STR) {
		// Try to convert to numeric
		if (isnum())
			goto tryagain;

		// Trigger VarNonNumeric
		assertNumeric(__PRETTY_FUNCTION__);

	} else {
		// Trigger VarUnassigned
		assertNumeric(__PRETTY_FUNCTION__);
	}

	// NO DO NOT! return *this ... postfix return a temporary!!! eg var(*this)
	return priorvalue;
}

// VAR --

// Not returning void so is usable in expressions
// The int argument indicates that this is POSTFIX override v--
RETVAR VB1::operator--(int) & {

	// Full check done below to avoid double checking number type
	assertVar(__PRETTY_FUNCTION__);

	var priorvalue;

tryagain:
	// Prefer int since -- nearly always on integers
	if (var_typ & VARTYP_INT) {
		if (var_int == std::numeric_limits<decltype(var_int)>::min())
			UNLIKELY
			throw VarNumUnderflow("operator--");
		priorvalue = var(var_int);
		var_int--;
		var_typ = VARTYP_INT;  // Reset to one unique type

	} else if (var_typ & VARTYP_DBL) {
		priorvalue = var_dbl;
		var_dbl--;
		var_typ = VARTYP_DBL;  // Reset to one unique type

	} else if (var_typ & VARTYP_STR) {
		// Try to convert to numeric
		if (isnum())
			goto tryagain;

		// Trigger VarNonNumeric
		assertNumeric(__PRETTY_FUNCTION__);

	} else {
		// Trigger VarUnassigned
		assertNumeric(__PRETTY_FUNCTION__);
	}

	return priorvalue;
}

//////////////////////////////////
// SELF INCREMENT/DECREMENT PREFIX
//////////////////////////////////

// ++ VAR

// Not returning void so is usable in expressions
// No argument indicates that this is prefix override ++var
RETVARREF VB1::operator++() & {

	// Full check done below to avoid double checking number type
	assertVar(__PRETTY_FUNCTION__);

tryagain:
	// Prefer int since -- nearly always on integers
	if (var_typ & VARTYP_INT) {
		if (var_int == std::numeric_limits<decltype(var_int)>::max())
			UNLIKELY
			throw VarNumOverflow("operator++");
		var_int++;
		var_typ = VARTYP_INT;  // Reset to one unique type
	} else if (var_typ & VARTYP_DBL) {
		var_dbl++;
		var_typ = VARTYP_DBL;  // Reset to one unique type
	} else if (var_typ & VARTYP_STR) {
		// Try to convert to numeric
		if (isnum())
			goto tryagain;

		// Trigger VarNonNumeric
		assertNumeric(__PRETTY_FUNCTION__);

	} else {
		// Trigger VarUnassigned
		assertNumeric(__PRETTY_FUNCTION__);
	}

	// OK to return *this in prefix ++
	return *this;
}

// -- VAR

// Not returning void so is usable in expressions
// No argument indicates that this is prefix override --var
RETVARREF VB1::operator--() & {

	// Full check done below to avoid double checking number type
	assertVar(__PRETTY_FUNCTION__);

tryagain:
	// Prefer int since -- nearly always on integers
	if (var_typ & VARTYP_INT) {
		if (var_int == std::numeric_limits<decltype(var_int)>::min())
			UNLIKELY
			throw VarNumUnderflow("operator--");
		var_int--;
		var_typ = VARTYP_INT;  // Reset to one unique type

	} else if (var_typ & VARTYP_DBL) {
		var_dbl--;
		var_typ = VARTYP_DBL;  // Reset to one unique type

	} else if (var_typ & VARTYP_STR) {
		// Try to convert to numeric
		if (isnum())
			goto tryagain;

		// Trigger VarNonNumeric
		assertNumeric(__PRETTY_FUNCTION__);

	} else {
		// Trigger VarUnassigned
		assertNumeric(__PRETTY_FUNCTION__);
	}

	// OK to return *this in prefix --
	return *this;
}

//C/C++                Int  Float	Definition
//
//%, div               Yes  No   Truncated[c]
//
//fmod (C)
//std::fmod (C++)      No   Yes  Truncated[11]
//
//remainder (C)
//std::remainder (C++) No   Yes  Rounded
static double exodusmodulo_dbl(const double dividend, const double limit) {

	if (!limit)
		UNLIKELY
		throw VarDivideByZero("mod('" ^ var(dividend) ^ "', '" ^ var(limit) ^ ")");
//
//	double result;
//	if (limit > 0) {
//		LIKELY
//		result = std::fmod(dividend, limit);
//		if (result < 0)
//			UNLIKELY
//			result += limit;
//	} else {
//		result = -std::fmod(-dividend, -limit);
//		if (result > 0)
//			result += limit;
//	}
//	return result;
    return dividend - limit * std::floor(dividend / limit);
}

static varint_t exodusmodulo_int(const varint_t dividend, const varint_t limit) {

	if (!limit)
		UNLIKELY
		throw VarDivideByZero("mod('" ^ var(dividend) ^ "', '" ^ var(limit) ^ ")");

	// There is no implementation that doesnt have two branches.
	// A ternary would be more concise code but not allow us to insert LIKELY/UNLIKELY.
	varint_t result;
	if (limit > 0) {
		LIKELY
		result = dividend % limit;
		if (result < 0)
			UNLIKELY
			result += limit;
	} else {
		result = -(-dividend % -limit);
		if (result > 0)
			result += limit;
	}
	return result;

	//Doesnt work because int / int doesnt provide "floored division"
	// return dividend - limit * (dividend / limit);
}

RETVAR VB1::mod(CBR limit) const {

	THISIS("var  var::mod(in limit) const")
	assertNumeric(function_sig);
	ISNUMERIC(limit)

	// prefer double dividend
	if (this->var_typ & VARTYP_DBL) {

		// use limit's double if available otherwise create it from limit's int/long
		if (!(limit.var_typ & VARTYP_DBL)) {
			limit.var_dbl = static_cast<double>(limit.var_int);
			// Dont register that the limit has a double
			//limit.var_typ = limit.var_typ & VARTYP_DBL;
		}

mod_doubles:
		return exodusmodulo_dbl(var_dbl, limit.var_dbl);
	}

	// if limit has a double then prefer it and convert this to double
	if (limit.var_typ & VARTYP_DBL) {
		var_dbl = static_cast<double>(this->var_int);
		// Dont register that this has a double
		//var_typ = var_typ & VARTYP_DBL;
		goto mod_doubles;
	}

	//otherwise both ints/longs
	return exodusmodulo_int(this->var_int, limit.var_int);
}


RETVAR VB1::mod(double limit) const {

	THISIS("var  var::mod(double limit) const")
	assertNumeric(function_sig);

	// ensure double dividend
	if (not (this->var_typ & VARTYP_DBL)) {
		this->var_dbl = static_cast<double>(this->var_int);
		// Dont register that this has a double
		//this->var_typ = this->var_typ & VARTYP_DBL;
	}

	return exodusmodulo_dbl(this->var_dbl, limit);
}

RETVAR VB1::mod(const int limit) const {

	THISIS("var  var::mod(const int limit) const")
	assertNumeric(function_sig);

	// prefer double dividend
	if (this->var_typ & VARTYP_DBL) {
		return exodusmodulo_dbl(this->var_dbl, static_cast<double>(limit));
	}

	// otherwise both ints
	return exodusmodulo_int(var_int, limit);
}

} // namespace exo