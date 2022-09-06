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

#include <cmath>

#include <varimpl.h>

#ifndef M_PI
//#define M_PI 3.14159265358979323846f
#define M_PI 3.14159265 
// 2643383279502884197169399375105820974944592307816406286208998f;
#endif

namespace exodus {

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
inline double exodusmodulo_dbl(const double dividend, const double limit) {

	if (!limit)
		throw VarDivideByZero("mod('" ^ var(dividend) ^ "', '" ^ var(limit) ^ ")");

	double result;
	if (limit > 0) {
		[[likely]]
		result = fmod(dividend, limit);
		if (result < 0)
			[[unlikely]]
			result += limit;
	} else {
		result = -fmod(-dividend, -limit);
		if (result > 0)
			result += limit;
	}
	return result;
}

inline int exodusmodulo_int(const int dividend, const int limit) {

	if (!limit)
		throw VarDivideByZero("mod('" ^ var(dividend) ^ "', '" ^ var(limit) ^ ")");

	int result;
	if (limit > 0) {
		[[likely]]
		result = dividend % limit;
		if (result < 0)
			[[unlikely]]
			result += limit;
	} else {
		result = -(-dividend % -limit);
		if (result > 0)
			result += limit;
	}
	return result;
}

var var::mod(CVR limit) const {

	THISIS("var var::mod(CVR limit) const")
	assertNumeric(function_sig);
	ISNUMERIC(limit)

	// prefer double dividend
	if (this->var_typ & VARTYP_DBL) {

		// use limit's double if available otherwise create it from limit's int/long
		if (!(limit.var_typ & VARTYP_DBL)) {
			limit.var_dbl = double(limit.var_int);
			// Dont register that the limit has a double
			//limit.var_typ = limit.var_typ & VARTYP_DBL;
		}

mod_doubles:
		return exodusmodulo_dbl(var_dbl, limit.var_dbl);
	}

	// if limit has a double then prefer it and convert this to double
	if (limit.var_typ & VARTYP_DBL) {
		var_dbl = double(this->var_int);
		// Dont register that this has a double
		//var_typ = var_typ & VARTYP_DBL;
		goto mod_doubles;
	}

	//otherwise both ints/longs
	return exodusmodulo_int(this->var_int, limit.var_int);
}

var var::mod(double limit) const {

	THISIS("var var::mod(double limit) const")
	assertNumeric(function_sig);

	// ensure double dividend
	if (not (this->var_typ & VARTYP_DBL)) {
		this->var_dbl = double(this->var_int);
		// Dont register that this has a double
		//this->var_typ = this->var_typ & VARTYP_DBL;
	}

	return exodusmodulo_dbl(this->var_dbl, limit);
}

var var::mod(const int limit) const {

	THISIS("var var::mod(const int limit) const")
	assertNumeric(function_sig);

	// prefer double dividend
	if (this->var_typ & VARTYP_DBL) {
		return exodusmodulo_dbl(this->var_dbl, double(limit));
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
	// cannot get here
	throw VarError("abs(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::sin() const {

	THISIS("var var::sin() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::sin(var_dbl * M_PI / 180);
	else
		return std::sin(double(var_int) * M_PI / 180);

	// cannot get here
	throw VarError("sin(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::cos() const {

	THISIS("var var::cos() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::cos(var_dbl * M_PI / 180);
	else
		return std::cos(double(var_int) * M_PI / 180);

	// cannot get here
	throw VarError("cos(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::tan() const {

	THISIS("var var::tan() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::tan(var_dbl * M_PI / 180);
	else
		return std::tan(double(var_int) * M_PI / 180);

	// cannot get here
	throw VarError("tan(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::atan() const {

	THISIS("var var::atan() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::atan(var_dbl) / M_PI * 180;
	else
		return std::atan(double(var_int)) / M_PI * 180;

	// cannot get here
	throw VarError("atan(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::loge() const {

	THISIS("var var::loge() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::log(var_dbl);
	else
		return std::log(double(var_int));

	// cannot get here
	throw VarError("loge(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::sqrt() const {

	THISIS("var var::sqrt() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::sqrt(var_dbl);

	//	if (this->var_typ & VARTYP_INT)
	return std::sqrt(double(var_int));

	throw VarError("sqrt(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::pwr(CVR exponent) const {

	THISIS("var var::pwr(CVR exponent) const")
	assertNumeric(function_sig);
	ISNUMERIC(exponent)

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::pow(var_dbl, exponent.toDouble());
	else
		return std::pow(double(var_int), exponent.toDouble());

	// cannot get here
	throw VarError("pow(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::exp() const {

	THISIS("var var::exp() const")
	assertNumeric(function_sig);

	// prefer double
	if (this->var_typ & VARTYP_DBL)
		return std::exp(var_dbl);
	else
		return std::exp(double(var_int));

	// cannot get here
	throw VarError("exp(unknown mvtype=" ^ var(var_typ) ^ ")");
}

} // namespace exodus

