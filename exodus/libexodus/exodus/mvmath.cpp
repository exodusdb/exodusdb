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

#include <mv.h>
#include <mvexceptions.h>

#ifndef M_PI
//#define M_PI 3.14159265358979323846f
#define M_PI 3.14159265 
// 2643383279502884197169399375105820974944592307816406286208998f;
#endif

namespace exodus {

var var::abs() const {
	THISIS("var var::abs() const")
	THISISNUMERIC()

	// prefer double
	if (var_typ & VARTYP_DBL) {
		if (var_dbl < 0)
			return -var_dbl;
		return (*this);
	} else {
		if (var_int < 0)
			return -var_int;
		return var_int;
	}
	// cannot get here
	throw MVError("abs(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::mod(CVR divisor) const {
	THISIS("var var::mod(CVR divisor) const")
	THISISNUMERIC()
	ISNUMERIC(divisor)

	// NB NOT using c++ % operator which until c++11 had undefined behaviour if divisor was negative
	// from c++11 % the sign of the result after a negative divisor is always the same as the
	// dividend

    //following is what c++ fmod does (a mathematical concept)
    //assert(mod(-2.3,var(1.499)).round(3).outputl() eq -0.801);
    //assert(mod(2.3,var(-1.499)).round(3).outputl() eq 0.801);
    //BUT arev and qm ensure that the result is somewhere from 0 up to or down to
    //(but not including) the divisor

	// prefer double dividend
	if (var_typ & VARTYP_DBL) {

		// use divisor's double if available otherwise create it from divisor's int/long
		if (!(divisor.var_typ & VARTYP_DBL)) {
			divisor.var_dbl = double(divisor.var_int);
			//divisor.var_typ = divisor.var_typ & VARTYP_DBL;
		}

mod_doubles:
	    //method is ... do the fmod and if the result is not the same sign as the divisor, add the divisor
		//if ((var_dbl < 0 && divisor.var_dbl >= 0) ||
		//	(divisor.var_dbl < 0 && var_dbl >= 0)) {
		//	return fmod(var_dbl, divisor.var_dbl) + divisor.var_dbl;
		//}
		//else {
		//	return fmod(var_dbl, divisor.var_dbl);
		//}

		return var_dbl - std::floor(var_dbl / divisor.var_dbl) * divisor.var_dbl;
	}

	// prefer double divisor
	if (divisor.var_typ & VARTYP_DBL) {
		var_dbl = double(var_int);
		//var_typ = var_typ & VARTYP_DBL;
		goto mod_doubles;
	}

	//both ints/longs

	//if ((var_int < 0 && divisor.var_int >= 0) ||
	//	(divisor.var_int < 0 && var_int >= 0))
	//	return (var_int % divisor.var_int) + divisor.var_int;
	//else
	//	return var_int % divisor.var_int;

	double double1 = double(var_int);
	return double1 - std::floor(double1 / divisor.var_int) * divisor.var_int;
}

var var::mod(const int divisor) const {
	THISIS("var var::mod(const int divisor) const")
	THISISNUMERIC()

	// see ::mod(CVR divisor) for comments about c++11 % operator

	// prefer double dividend
	if (var_typ & VARTYP_DBL) {
		//if ((var_dbl < 0 && divisor >= 0) || (divisor < 0 && var_dbl >= 0)) {
		//	// multivalue version of mod
		//	double divisor2 = double(divisor);
		//	return fmod(var_dbl, divisor2) + divisor2;
		//} else
		//	return fmod(var_dbl, double(divisor));

		//double divisor2 = double(divisor);
		//double result = fmod(var_dbl, divisor2);
		//if (result < 0 && divisor2 > 0)
		//	result += divisor2;
		//return result;

		return var_dbl - std::floor(var_dbl / divisor) * divisor;
	}

	//if ((var_int < 0 && divisor >= 0) || (divisor < 0 && var_int >= 0))
	//	// multivalue version of mod
	//	return (var_int % divisor) + divisor;
	//else
	//	return var_int % divisor;

	//mvint_t result = var_int % divisor;
	//if (result < 0 && divisor > 0)
	//	result += divisor;
	//return result;

	auto double1 = double(var_int);
	return double1 - std::floor(double1 / divisor) * divisor;
}

/*
	var sqrt() const;
	var loge() const;
*/

var var::sin() const {
	THISIS("var var::sin() const")
	THISISNUMERIC()

	// prefer double
	if (var_typ & VARTYP_DBL)
		return std::sin(var_dbl * M_PI / 180);
	else
		return std::sin(double(var_int) * M_PI / 180);

	// cannot get here
	throw MVError("sin(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::cos() const {
	THISIS("var var::cos() const")
	THISISNUMERIC()

	// prefer double
	if (var_typ & VARTYP_DBL)
		return std::cos(var_dbl * M_PI / 180);
	else
		return std::cos(double(var_int) * M_PI / 180);

	// cannot get here
	throw MVError("cos(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::tan() const {
	THISIS("var var::tan() const")
	THISISNUMERIC()

	// prefer double
	if (var_typ & VARTYP_DBL)
		return std::tan(var_dbl * M_PI / 180);
	else
		return std::tan(double(var_int) * M_PI / 180);

	// cannot get here
	throw MVError("tan(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::atan() const {
	THISIS("var var::atan() const")
	THISISNUMERIC()

	// prefer double
	if (var_typ & VARTYP_DBL)
		return std::atan(var_dbl) / M_PI * 180;
	else
		return std::atan(double(var_int)) / M_PI * 180;

	// cannot get here
	throw MVError("atan(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::loge() const {
	THISIS("var var::loge() const")
	THISISNUMERIC()

	// prefer double
	if (var_typ & VARTYP_DBL)
		return std::log(var_dbl);
	else
		return std::log(double(var_int));

	// cannot get here
	throw MVError("loge(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::sqrt() const {
	THISIS("var var::sqrt() const")
	THISISNUMERIC()

	// prefer double
	if (var_typ & VARTYP_DBL)
		return std::sqrt(var_dbl);

	//	if (var_typ & VARTYP_INT)
	return std::sqrt(double(var_int));

	throw MVError("sqrt(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::pwr(CVR exponent) const {
	THISIS("var var::pwr(CVR exponent) const")
	THISISNUMERIC()
	ISNUMERIC(exponent)

	// prefer double
	if (var_typ & VARTYP_DBL)
		return std::pow(var_dbl, exponent.toDouble());
	else
		return std::pow(double(var_int), exponent.toDouble());

	// cannot get here
	throw MVError("pow(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::exp() const {
	THISIS("var var::exp() const")
	THISISNUMERIC()

	// prefer double
	if (var_typ & VARTYP_DBL)
		return std::exp(var_dbl);
	else
		return std::exp(double(var_int));

	// cannot get here
	throw MVError("exp(unknown mvtype=" ^ var(var_typ) ^ ")");
}

} // namespace exodus

