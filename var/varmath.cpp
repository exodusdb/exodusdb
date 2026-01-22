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

#if EXO_MODULE > 1
	import std;
#else
#	include <math.h>
#	include <cmath>
#	include <cstdint> // for std::int64_t etc.
#	include <cmath>   // for std::fmod etc.
#	include <cstring> // for std::strstr etc.
#endif

//#include <cerrno>

#include "varimpl.h"

#ifndef M_PI
//#define M_PI 3.14159265358979323846f
#define M_PI 3.14159265 
// 2643383279502884197169399375105820974944592307816406286208998f;
#endif

namespace exo {

// Some template instatiations are at the end of this file

/*
	let sqrt() const;
	let loge() const;
*/

/**
 * \param[in]  val Initialising parameter for data.
 * \param[out] dat Data pointer where the new object should be stored.
 *
 * \return True if the object was created, false if not
 *         (i.e., we're out of memory)
 */
var  var_base::abs() const {

	THISIS("var  var::abs() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL) {
		if (var_dbl < 0)
			return -var_dbl;
		return this->clone();
	} else {
		if (var_int < 0)
			return -var_int;
		return var_int;
	}
}

var  var_base::sin() const {

	THISIS("var  var::sin() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::sin(var_dbl * M_PI / 180);
	else
		return std::sin(static_cast<double>(var_int) * M_PI / 180);
}

var  var_base::cos() const {

	THISIS("var  var::cos() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::cos(var_dbl * M_PI / 180);
	else
		return std::cos(static_cast<double>(var_int) * M_PI / 180);
}

var  var_base::tan() const {

	THISIS("var  var::tan() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::tan(var_dbl * M_PI / 180);
	else
		return std::tan(static_cast<double>(var_int) * M_PI / 180);
}

var  var_base::atan() const {

	THISIS("var  var::atan() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::atan(var_dbl) / M_PI * 180;
	else
		return std::atan(static_cast<double>(var_int)) / M_PI * 180;
}

var  var_base::loge() const {

	THISIS("var  var::loge() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::log(var_dbl);
	else
		return std::log(static_cast<double>(var_int));
}

var  var_base::sqrt() const {

	THISIS("var  var::sqrt() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::sqrt(var_dbl);

	//	if (this->var_typ & VARTYP_INT)
	return std::sqrt(static_cast<double>(var_int));
}

var  var_base::pwr(in exponent) const {

	THISIS("var  var::pwr(in exponent) const")
	assertNumeric(function_sig);
	ISNUMERIC(exponent)

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::pow(var_dbl, exponent.toDouble());
	else
		return std::pow(static_cast<double>(var_int), exponent.toDouble());
}

var  var_base::exp() const {

	THISIS("var  var::exp() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::exp(var_dbl);
	else
		return std::exp(static_cast<double>(var_int));
}

///////////////
// var_base::integer
///////////////

// function name is "integer" instead of "int" because int is a reserved word in c/c++ for int datatype
// using the system int() function on a var e.g. int(varx) returns an int whereas this function returns a var
var  var_base::integer() const {

	//-1.0 = -1

	//-0.9 = 0
	//-0.5 = 0
	//-0.1 = 0

	// 0   = 0

	// 0.1 = 0
	// 0.5 = 0
	// 0.9 = 0

	// 1.0 = 1

	THISIS("var  var::integer() const")
	this->assertInteger(function_sig);

	return var_int;
}

//////////////////
// var_base::floor
//////////////////

var var_base::floor() const {

	// Goes to the closest integer towards negative infinity

	//-1.9 - -2
	//-1.5 = -2
	//-1.0 = -1

	//-0.9 = -1
	//-0.5 = -1
	//-0.1 = -1

	// 0   = 0

	// 0.1 = 0
	// 0.5 = 0
	// 0.9 = 0

	// 1.0 = 1
	// 1.5 = 1
	// 1.9 = 1

	THISIS("var  var::floor() const")
	this->assertNumeric(function_sig);

	// Always use double if available.
	// Because the int may have been created from the double - not using floor.
	if (var_typ & VARTYP_DBL) {
		//warning: conversion from ‘double’ to ‘long int’ may change value [-Wfloat-conversion]
		return static_cast<varint_t>(std::floor(var_dbl));
	}

	return var_int;
}

} // namespace exo
