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


////////////////////
// USER LITERAL _var
////////////////////

// "abc^def"_var

ND var operator""_var(const char* cstr, std::size_t size) {

	var rvo = var(cstr, size);

	// Convert _VISIBLE_FMS to _ALL_FMS
	for (char& c : rvo.var_str) {
		switch (c) {
			// Most common first to perhaps aid optimisation
			case VISIBLE_FM_: c = FM_; break;
			case VISIBLE_VM_: c = VM_; break;
			case VISIBLE_SM_: c = SM_; break;
			case VISIBLE_TM_: c = TM_; break;
			case VISIBLE_ST_: c = ST_; break;
			case VISIBLE_RM_: c = RM_; break;
			// All other chars left unconverted
			default:;
		}
	}

	return rvo;

}

// 123456_var
/////////////

ND var operator""_var(unsigned long long int i) {
    return var(i);
}

// 123.456_var
//////////////

ND var operator""_var(long double d) {
    return var(d);
}


///////////////////
// UNARY PLUS/MINUS
///////////////////

// +var

template<> PUBLIC RETVAR VARBASE1::operator+() const {

	assertVar(__PRETTY_FUNCTION__);
	assertNumeric(__PRETTY_FUNCTION__);

	//return static_cast<RETVAR>(*this);
	return *static_cast<const RETVAR*>(this);
}

// -var

template<> PUBLIC RETVAR VARBASE1::operator-() const {

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

// The assignment operator must always return a reference to *this. Why?
template<> PUBLIC VBR1 VARBASE1::operator^=(CBX rhs) & {

	assertString(__PRETTY_FUNCTION__);
	rhs.assertString(__PRETTY_FUNCTION__);

	// tack it onto our string
	var_str.append(rhs.var_str);
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

// ^= int

template<> PUBLIC VBR1 VARBASE1::operator^=(const int int1) & {

	assertString(__PRETTY_FUNCTION__);

	// var_str+=var(int1).var_str;
	var_str += std::to_string(int1);
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

// ^= double

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

// ^= char

template<> PUBLIC VBR1 VARBASE1::operator^=(const char char1) & {

	assertString(__PRETTY_FUNCTION__);

	// var_str+=var(int1).var_str;
	var_str.push_back(char1);
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

#ifdef NOT_TEMPLATED_APPEND

// ^= char*

// The assignment operator must always return a reference to *this.
template<> PUBLIC VBR1 VARBASE1::operator^=(const char* cstr) & {

	assertString(__PRETTY_FUNCTION__);

	// var_str+=var(int1).var_str;
	// var_str+=std::string(char1);
	var_str += cstr;
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

// ^= std::string

// The assignment operator must always return a reference to *this.
template<> PUBLIC VBR1 VARBASE1::operator^=(const std::string& string1) & {

	assertString(__PRETTY_FUNCTION__);

	// var_str+=var(int1).var_str;
	var_str += string1;
	var_typ = VARTYP_STR;  // reset to one unique type

	return *this;
}

// ^= std::string_view

// The assignment operator must always return a reference to *this.
template<> PUBLIC VBR1 VARBASE1::operator^=(SV sv1) & {

	assertString(__PRETTY_FUNCTION__);

	// var_str+=var(int1).var_str;
	var_str += sv1;
	var_typ = VARTYP_STR;  // reset to one unique type

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
template<> PUBLIC RETVAR VARBASE1::operator++(int) & {

	// full check done below to avoid double checking number type
	assertVar(__PRETTY_FUNCTION__);

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

// VAR --

// Not returning void so is usable in expressions
// The int argument indicates that this is POSTFIX override v--
template<> PUBLIC RETVAR VARBASE1::operator--(int) & {

	// full check done below to avoid double checking number type
	assertVar(__PRETTY_FUNCTION__);

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

//////////////////////////////////
// SELF INCREMENT/DECREMENT PREFIX
//////////////////////////////////

// ++ VAR

// Not returning void so is usable in expressions
// No argument indicates that this is prefix override ++var
template<> PUBLIC VBR1 VARBASE1::operator++() & {

	// full check done below to avoid double checking number type
	assertVar(__PRETTY_FUNCTION__);

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

// -- VAR

// Not returning void so is usable in expressions
// No argument indicates that this is prefix override --var
template<> PUBLIC VBR1 VARBASE1::operator--() & {

	// full check done below to avoid double checking number type
	assertVar(__PRETTY_FUNCTION__);

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


////////////////////////////////////////
// SELF INCREMENT/DECREMENT var versions
////////////////////////////////////////

// All forwarded to var_base

// var ++

var  var::operator++(int) & {
	var orig = this->clone();
	var_base::operator++();
	return orig;
}

// var --

var  var::operator--(int) & {
	var orig = this->clone();
	var_base::operator--(0);
	return orig;
}

// ++ var

var& var::operator++() & {
	var_base::operator++(0);
	return *this;
}

// -- var

var& var::operator--() & {
	var_base::operator--();
	return *this;
}

} // namespace exo