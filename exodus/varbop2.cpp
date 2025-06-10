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

///////////////////
// UNARY PLUS/MINUS
///////////////////

// +var

template<> PUBLIC RETVAR VB1::operator+() const {

	assertVar(__PRETTY_FUNCTION__);
	assertNumeric(__PRETTY_FUNCTION__);

	//return static_cast<RETVAR>(*this);
	return *static_cast<const RETVAR*>(this);
}

// -var

template<> PUBLIC RETVAR VB1::operator-() const {

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

template<> PUBLIC VBR1 VB1::operator^=(CBR rhs) & {

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

template<> PUBLIC VBR1 VB1::operator^=(TBR rhs) & {

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

template<> PUBLIC VBR1 VB1::operator^=(const int int1) & {

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

template<> PUBLIC VBR1 VB1::operator^=(const double double1) & {

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

template<> PUBLIC VBR1 VB1::operator^=(const char char1) & {

	assertString(__PRETTY_FUNCTION__);

	// var_str+=var(int1).var_str;
	var_str.push_back(char1);
	var_typ = VARTYP_STR;  // Reset to string only

	return *this;
}

#ifdef NOT_TEMPLATED_APPEND

// ^= char*

template<> PUBLIC VBR1 VB1::operator^=(const char* cstr) & {

	assertString(__PRETTY_FUNCTION__);

	var_str += cstr;
	var_typ = VARTYP_STR;  // Reset to string only

	return *this;
}

// ^= std::string

template<> PUBLIC VBR1 VB1::operator^=(const std::string& string1) & {

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

template<> PUBLIC VBR1 VB1::operator^=(SV sv1) & {

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
template<> PUBLIC RETVAR VB1::operator++(int) & {

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
template<> PUBLIC RETVAR VB1::operator--(int) & {

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
template<> PUBLIC VBR1 VB1::operator++() & {

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
template<> PUBLIC VBR1 VB1::operator--() & {

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

} // namespace exo