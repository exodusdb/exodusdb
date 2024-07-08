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

//#include <math.h>
#include <cmath>
#include <errno.h>

#include <varimpl.h>

#ifndef M_PI
//#define M_PI 3.14159265358979323846f
#define M_PI 3.14159265 
// 2643383279502884197169399375105820974944592307816406286208998f;
#endif

namespace exo {

// Some template instatiations are at the end of this file

//C/C++                Int  Float	Definition
//
//%, div               Yes  No   Truncated[c]
//
//fmod (C)
//std::fmod (C++)      No   Yes  Truncated[11]
//
//remainder (C)
//std::remainder (C++) No   Yes  Rounded

//PICKOS modulo limits the value instead of doing a kind of remainder as per c++ % operator
// [0 , limit) if limit is positive
// (limit, 0] if limit is positive
// unlike c++ which is acts more like a divisor/remainder function
// Note that PICKOS definition is symmetrical about 0 the limit of 0
//i.e. mod(x,y) == -(mod(-x,-y))
static double exodusmodulo_dbl(const double dividend, const double limit) {

	if (!limit)
		UNLIKELY
		throw VarDivideByZero("mod('" ^ var(dividend) ^ "', '" ^ var(limit) ^ ")");

	double result;
	if (limit > 0) {
		LIKELY
		result = fmod(dividend, limit);
		if (result < 0)
			UNLIKELY
			result += limit;
	} else {
		result = -fmod(-dividend, -limit);
		if (result > 0)
			result += limit;
	}
	return result;
}

static varint_t exodusmodulo_int(const varint_t dividend, const varint_t limit) {

	if (!limit)
		UNLIKELY
		throw VarDivideByZero("mod('" ^ var(dividend) ^ "', '" ^ var(limit) ^ ")");

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
}

template<> PUBLIC RETVAR VARBASE1::mod(CBX limit) const {

	THISIS("var var::mod(CVR limit) const")
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


template<> PUBLIC RETVAR VARBASE1::mod(double limit) const {

	THISIS("var var::mod(double limit) const")
	assertNumeric(function_sig);

	// ensure double dividend
	if (not (this->var_typ & VARTYP_DBL)) {
		this->var_dbl = static_cast<double>(this->var_int);
		// Dont register that this has a double
		//this->var_typ = this->var_typ & VARTYP_DBL;
	}

	return exodusmodulo_dbl(this->var_dbl, limit);
}


template<> PUBLIC RETVAR VARBASE1::mod(const int limit) const {

	THISIS("var var::mod(const int limit) const")
	assertNumeric(function_sig);

	// prefer double dividend
	if (this->var_typ & VARTYP_DBL) {
		return exodusmodulo_dbl(this->var_dbl, static_cast<double>(limit));
	}

	// otherwise both ints
	return exodusmodulo_int(var_int, limit);
}

/*
	var sqrt() const;
	var loge() const;
*/

var var::abs() const {

	THISIS("var var::abs() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL) {
		if (var_dbl < 0)
			return -var_dbl;
		return (*this);
	} else {
		if (var_int < 0)
			return -var_int;
		return var_int;
	}
}

var var::sin() const {

	THISIS("var var::sin() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::sin(var_dbl * M_PI / 180);
	else
		return std::sin(static_cast<double>(var_int) * M_PI / 180);
}

var var::cos() const {

	THISIS("var var::cos() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::cos(var_dbl * M_PI / 180);
	else
		return std::cos(static_cast<double>(var_int) * M_PI / 180);
}

var var::tan() const {

	THISIS("var var::tan() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::tan(var_dbl * M_PI / 180);
	else
		return std::tan(static_cast<double>(var_int) * M_PI / 180);
}

var var::atan() const {

	THISIS("var var::atan() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::atan(var_dbl) / M_PI * 180;
	else
		return std::atan(static_cast<double>(var_int)) / M_PI * 180;
}

var var::loge() const {

	THISIS("var var::loge() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::log(var_dbl);
	else
		return std::log(static_cast<double>(var_int));
}

var var::sqrt() const {

	THISIS("var var::sqrt() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::sqrt(var_dbl);

	//	if (this->var_typ & VARTYP_INT)
	return std::sqrt(static_cast<double>(var_int));
}

var var::pwr(CVR exponent) const {

	THISIS("var var::pwr(CVR exponent) const")
	assertNumeric(function_sig);
	ISNUMERIC(exponent)

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::pow(var_dbl, exponent.toDouble());
	else
		return std::pow(static_cast<double>(var_int), exponent.toDouble());
}

var var::exp() const {

	THISIS("var var::exp() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::exp(var_dbl);
	else
		return std::exp(static_cast<double>(var_int));
}

} // namespace exo
