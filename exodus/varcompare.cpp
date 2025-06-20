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
#	include <limits>
#endif

#include <exodus/var.h>
//import exovar;
#include <exodus/varerr.h>

namespace exo {

// NOTE: INSTANTIATIONS ARE AT THE END OF THIS FILE

inline bool almost_equal(double lhs_dbl, double rhs_dbl) {
	//crude pickos method
	return (std::abs(lhs_dbl - rhs_dbl) < EXO_SMALLEST_NUMBER);
}

/* more accurate way of comparing two decimals using EPSILON and ulp
   not using it simply to make better emulation of pickos at least initially

//comparing floating point numbers is a VERY complex matter since c++ double uses BINARY NOT DECIMAL
//
//https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
//
//https://www.theregister.com/2006/08/12/floating_point_approximation/
//https://www.theregister.com/2006/09/20/floating_point_numbers_2/
//
//derived from https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
inline bool almost_equal_not_zero(double x, double y, int ulp)
{
	// the machine epsilon has to be scaled to the magnitude of the values used
	// and multiplied by the desired precision in ULPs (units in the last place)
	return std::fabs(x-y) <= std::numeric_limits<double>::epsilon() * std::fabs(x+y) * ulp
		// unless the result is subnormal
		|| std::fabs(x-y) < std::numeric_limits<double>::min();
}

inline bool almost_equal(double x, decltype(var_int) y, int ulp)
{
	if (y == 0)
		return (std::abs(x) < EXO_SMALLEST_NUMBER);
	else
		return almost_equal_not_zero(x, static_cast<double>(y), ulp);
}

inline bool almost_equal(double x, double y, int ulp)
{
	if (y == 0.0)
		return (std::abs(x) < EXO_SMALLEST_NUMBER);
	else if (x == 0.0)
		return (std::abs(y) < EXO_SMALLEST_NUMBER);
	else
		return almost_equal_not_zero(x, y, ulp);
}
*/

bool bool_lt_bool(const bool lhs, const bool rhs) {

	// AXIOMS
	// false == false
	// true  == true
	// false <  TRUE

	// LEMMAS
	// true  < true  -> false
	// false < false -> false
	// true  < false -> false

	// TRUTH TABLE
	//       rhs: true    false
	// lhs:       ----    -----
	// true       false   false
	// false      TRUE    false

	return !lhs && rhs;

}

// toBool is the only authority on whether var is true or false
bool var_lt_bool(CBR1 lhs, const bool rhs_bool) {
//	return lhs.toBool() < rhs_bool;
	return bool_lt_bool(lhs.toBool(), rhs_bool);
}

// toBool is the only authority on whether var is true or false
bool bool_lt_var(const bool lhs_bool, CBR1 rhs) {
	return bool_lt_bool(lhs_bool, rhs.toBool());
}

// almost identical code in var_eq_var and var_lt_var except where noted
// NOTE doubles compare only to 0.0001 accuracy)
bool var_eq_var(CBR1 lhs, CBR1 rhs) {

	lhs.assertVar(__PRETTY_FUNCTION__);
	rhs.assertVar(__PRETTY_FUNCTION__);

	// NB empty string is always less than anything except another empty string

	// 1. BOTH EMPTY or IDENTICAL STRINGS returns TRUE one empty results false
	if (lhs.var_typ & VARTYP_STR) {
		if (rhs.var_typ & VARTYP_STR) {
			// we have two strings
			// if they are both the same (including both empty) then eq is true
			if (lhs.var_str == rhs.var_str)
				// different from var_lt_var
				return true;
			// otherwise if either is empty then return eq false
			//(since empty string is ONLY eq to another empty string)
			if (lhs.var_str.empty())
				// different from var_lt_var
				return false;
			if (rhs.var_str.empty())
				// SAME as var_lt_var
				return false;
			// otherwise go on to test numerically then literally
		} else {
			// if rhs isnt a string and lhs is empty then eq is false
			//(after checking that rhs is actually assigned)
			if (lhs.var_str.empty()) {
				if (!rhs.var_typ) {
					rhs.assertAssigned(__PRETTY_FUNCTION__);
				}
				// different from var_lt_var
				return false;
			}
		}
	} else {
		// if lhs isnt a string and rhs is an empty string then return eq false
		//(after checking that lhs is actually assigned)
		if ((rhs.var_typ & VARTYP_STR) && (rhs.var_str.empty())) {
			if (!lhs.var_typ) {
				lhs.assertAssigned(__PRETTY_FUNCTION__);
			}
			// SAME as var_lt_var
			return false;
		}
	}

	// 2. BOTH NUMERIC STRINGS
	// exact match on decimal numbers is often inaccurate since they are approximations of real
	// numbers
	// match on decimal in preference to int
	if (lhs.isnum() && rhs.isnum()) {
		//if (lhs.var_typ & VARTYP_INT)
		if (lhs.var_typ & VARTYP_DBL) {

			//DOUBLE v DOUBLE
			//if (rhs.var_typ & VARTYP_INT)
			if (rhs.var_typ & VARTYP_DBL) {
				// different from var_lt_var

				//return (lhs.var_intd == rhs.var_intd);

				// (DOUBLES ONLY COMPARE TO ACCURACY EXO_SMALLEST_NUMBER was 0.0001)
				//return (std::abs(lhs.var_dbl - rhs.var_dbl) < EXO_SMALLEST_NUMBER);
				return almost_equal(lhs.var_dbl, rhs.var_dbl);
			}

			//DOUBLE V INT
			else {
				// different from var_lt_var (uses absolute)
				// (DOUBLES ONLY COMPARE TO ACCURACY EXO_SMALLEST_NUMBER was 0.0001)

				//return (lhs.var_int == rhs.var_dbl);
				//return (std::abs(lhs.var_dbl - static_cast<double>(rhs.var_int)) < EXO_SMALLEST_NUMBER);
				// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
				return almost_equal(lhs.var_dbl, static_cast<double>(rhs.var_int));
			}
			// std::unreachable();
		}

		//INT v DOUBLE
		//if (rhs.var_typ & VARTYP_INTd)
		if (rhs.var_typ & VARTYP_DBL) {
			// different from var_lt_var (uses absolute)
			// (DOUBLES ONLY COMPARE TO ACCURACY EXO_SMALLEST_NUMBER was 0.0001)

			//return (lhs.var_dbl == rhs.var_int);
			//return (std::abs(static_cast<double>(lhs.var_int) - rhs.var_dbl) < EXO_SMALLEST_NUMBER);
			//return almost_equal(lhs.var_int, rhs.var_dbl, 2);
			//put lhs int 2nd argument to invoke the fastest implmentation
			// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
			return almost_equal(rhs.var_dbl, static_cast<double>(lhs.var_int));
		}

		//INT v INT
		else {
			// different from var_lt_var (uses absolute)

			// (DOUBLES ONLY COMPARE TO ACCURACY EXO_SMALLEST_NUMBER was 0.0001)
			//return (lhs.var_dbl == rhs.var_dbl);
			//return (std::abs(lhs.var_dbl-rhs.var_dbl) < EXO_SMALLEST_NUMBER);

			return (lhs.var_int == rhs.var_int);
		}
		// std::unreachable();
	}

	// 3. BOTH NON-NUMERIC STRINGS
	if (!(lhs.var_typ & VARTYP_STR))
		lhs.createString();
	if (!(rhs.var_typ & VARTYP_STR))
		rhs.createString();
	// different from var_lt_var
	//return lhs.localeAwareCompare(lhs.var_str, rhs.var_str) == 0;
	return lhs.var_str == rhs.var_str;
}

// almost identical between var_eq_var and var_lt_var except where noted
// NOTE doubles compare only to 0.0001 accuracy)
bool var_lt_var(CBR1 lhs, CBR1 rhs) {

	lhs.assertVar(__PRETTY_FUNCTION__);
	rhs.assertVar(__PRETTY_FUNCTION__);

	// NB empty string is always less than anything except another empty string

	// 1. both empty or identical strings returns eq. one empty results false
	if (lhs.var_typ & VARTYP_STR) {
		if (rhs.var_typ & VARTYP_STR) {
			// we have two strings
			// if they are both the same (including both empty) then eq is true
			if (lhs.var_str == rhs.var_str)
				// different from var_eq_var
				return false;
			// otherwise if either is empty then return eq false
			//(since empty string is ONLY eq to another empty string)
			if (lhs.var_str.empty())
				// different from var_eq_var
				return true;
			if (rhs.var_str.empty())
				// SAME as var_eq_var
				return false;
			// otherwise go on to test numerically then literally
		} else {
			// if rhs isnt a string and lhs is empty then eq is false
			// after checking that rhs is actually assigned
			if (lhs.var_str.empty()) {
				if (!rhs.var_typ) {
					rhs.assertAssigned(__PRETTY_FUNCTION__);
				}
				// different from var_eq_var
				return true;
			}
		}
	} else {
		// if lhs isnt a string and rhs is an empty string then return eq false
		// after checking that lhs is actually assigned
		if ((rhs.var_typ & VARTYP_STR) && (rhs.var_str.empty())) {
			if (!lhs.var_typ) {
				lhs.assertAssigned(__PRETTY_FUNCTION__);
			}
			// SAME as var_eq_var
			return false;
		}
	}

	// 2. both numerical strings
	if (lhs.isnum() && rhs.isnum()) {
		if (lhs.var_typ & VARTYP_INT) {
			if (rhs.var_typ & VARTYP_INT)
				// different from var_eq_var
				return (lhs.var_int < rhs.var_int);
			else
				// different from var_eq_var
				// (DOUBLES ONLY COMPARE TO ACCURACY EXO_SMALLEST_NUMBER was 0.0001)
				//return (static_cast<double>(lhs.var_int) < rhs.var_dbl);
				return (rhs.var_dbl - static_cast<double>(lhs.var_int)) >= EXO_SMALLEST_NUMBER;
			//return ((rhs.var_dbl - static_cast<double>(lhs.var_int) >= EXO_SMALLEST_NUMBER);
			// std::unreachable();
		}
		if (rhs.var_typ & VARTYP_INT)
			// different from var_eq_var
			// (DOUBLES ONLY COMPARE TO ACCURACY EXO_SMALLEST_NUMBER was 0.0001)
			//return (lhs.var_dbl < static_cast<double>(rhs.var_int));
			return (static_cast<double>(rhs.var_int) - lhs.var_dbl) >= EXO_SMALLEST_NUMBER;
		else
			// different from var_eq_var
			// (DOUBLES ONLY COMPARE TO ACCURACY EXO_SMALLEST_NUMBER was 0.0001)
			//return (lhs.var_dbl < rhs.var_dbl);
			return (rhs.var_dbl - lhs.var_dbl) >= EXO_SMALLEST_NUMBER;
		// std::unreachable();
	}

	// 3. either or both non-numerical strings
	if (!(lhs.var_typ & VARTYP_STR))
		lhs.createString();
	if (!(rhs.var_typ & VARTYP_STR))
		rhs.createString();
	// different from var_eq_var
	// return lhs.var_str<rhs.var_str;
	return lhs.localeAwareCompare(lhs.var_str, rhs.var_str) < 0;
}

// similar to var_eq_var and var_lt_var - this is the var<int version for speed
// NOTE doubles compare only to 0.0001 accuracy)
bool var_lt_int(CBR1 lhs, const int rhs_int) {

	lhs.assertVar(__PRETTY_FUNCTION__);

	// Empty string is always less than anything except another empty string
	if (lhs.var_typ & VARTYP_STR && lhs.var_str.empty())
			return true;

	// LHS numerical?
	do {

		// Compare dbl by preference because int might be a conversion with loss of precision
		if (lhs.var_typ & VARTYP_DBL)
			// (DOUBLES ONLY COMPARE TO ACCURACY EXO_SMALLEST_NUMBER was 0.0001)
			return (static_cast<double>(rhs_int) - lhs.var_dbl) >= EXO_SMALLEST_NUMBER;

		//compare int if available
		if (lhs.var_typ & VARTYP_INT)
			return (lhs.var_int < rhs_int);

		// Try to convert to number and try again
		// Will check unassigned
	} while (lhs.isnum());

	// Non-numerical lhs
	return lhs.var_str < std::to_string(rhs_int);

}

// NOTE doubles compare only to 0.0001 accuracy)
bool int_lt_var(const int lhs_int, CBR1 rhs) {

	rhs.assertVar(__PRETTY_FUNCTION__);

	// Empty string is always less than anything except another empty string
	if (rhs.var_typ & VARTYP_STR && rhs.var_str.empty())
			return false;

	// RHS numerical?
	do {

		// Compare dbl by preference because int might be a conversion with loss of precision
		if (rhs.var_typ & VARTYP_DBL) {
			// (DOUBLES ONLY COMPARE TO ACCURACY EXO_SMALLEST_NUMBER was 0.0001)
			return (rhs.var_dbl - static_cast<double>(lhs_int)) >= EXO_SMALLEST_NUMBER;
		}

		//compare int if available
		if (rhs.var_typ & VARTYP_INT)
			return (lhs_int < rhs.var_int);

		// Try to convert to number and try again
		// Will check unassigned
	} while (rhs.isnum());

	// Non-numerical rhs so compare as strings
	return std::to_string(lhs_int) < rhs.var_str;

}

// NOTE doubles compare only to 0.0001 accuracy)
bool var_lt_dbl(CBR1 lhs, const double rhs_dbl) {

	lhs.assertVar(__PRETTY_FUNCTION__);

	// Empty string is always less than anything except another empty string
	if (lhs.var_typ & VARTYP_STR && lhs.var_str.empty())
		return true;

	// 2. LHS numerical?
	do {

		// LHS DBL
		if (lhs.var_typ & VARTYP_DBL)
			// (DOUBLES ONLY COMPARE TO ACCURACY EXO_SMALLEST_NUMBER was 0.0001)
			return (rhs_dbl - lhs.var_dbl) >= EXO_SMALLEST_NUMBER;

		// LHS INT
		if (lhs.var_typ & VARTYP_INT)
			// (DOUBLES ONLY COMPARE TO ACCURACY EXO_SMALLEST_NUMBER was 0.0001)
			return (rhs_dbl - static_cast<double>(lhs.var_int)) >= EXO_SMALLEST_NUMBER;

		// Try to convert to number and try again
		// Will check unassigned

	} while (lhs.isnum());

	// Non-numerical lhs
	return lhs.var_str < std::to_string(rhs_dbl);

}

// NOTE doubles compare only to 0.0001 accuracy)
bool dbl_lt_var(const double lhs_dbl, CBR1 rhs) {

	rhs.assertVar(__PRETTY_FUNCTION__);

	// Empty string is always less than anything except another empty string
	if (rhs.var_typ & VARTYP_STR && rhs.var_str.empty())
		return false;

	// RHS Numerical?
	do {

		// RHS double
		if (rhs.var_typ & VARTYP_DBL)
			// (DOUBLES ONLY COMPARE TO ACCURACY EXO_SMALLEST_NUMBER was 0.0001)
			return (rhs.var_dbl - lhs_dbl) >= EXO_SMALLEST_NUMBER;

		// RHS int
		if (rhs.var_typ & VARTYP_INT)
			// (DOUBLES ONLY COMPARE TO ACCURACY EXO_SMALLEST_NUMBER was 0.0001)
			return (static_cast<double>(rhs.var_int) - lhs_dbl) >= EXO_SMALLEST_NUMBER;

		// Try to convert to number and try again
		// Will check unassigned

	} while (rhs.isnum());

	// Non-numerical rhs
	return std::to_string(lhs_dbl) < rhs.var_str;

}

// NOTE doubles compare only to 0.0001 accuracy)
bool var_eq_dbl(CBR1 lhs, const double rhs_dbl) {

	lhs.assertVar(__PRETTY_FUNCTION__);

	// NB empty string is always less than anything except another empty string

	// 1. EMPTY STRING always false
	if (lhs.var_typ & VARTYP_STR && lhs.var_str.empty()) {
		//std::clog << "var_eq_dbl 1. Empty string can never equal any number" << std::endl;
		return false;
	}

	do {

		// 2. LHS DBL
		if (lhs.var_typ & VARTYP_DBL) {

			// (DOUBLES ONLY COMPARE TO ACCURACY EXO_SMALLEST_NUMBER was 0.0001)
			//return (std::abs(lhs.var_dbl - rhs.var_dbl) < EXO_SMALLEST_NUMBER);
			//std::clog << "var_eq_dbl 2. lhs double " << lhs.var_dbl << " compare to double " << rhs_dbl << std::endl;
			return almost_equal(lhs.var_dbl, rhs_dbl);

		}

		// 3. LHS INT
		else if (lhs.var_typ & VARTYP_INT) {

			// LHS INT
			// different from var_lt_var (uses absolute)
			// (DOUBLES ONLY COMPARE TO ACCURACY EXO_SMALLEST_NUMBER was 0.0001)

			//return (lhs.var_dbl == rhs.var_int);
			//return (std::abs(static_cast<double>(lhs.var_int) - rhs.var_dbl) < EXO_SMALLEST_NUMBER);
			//return almost_equal(lhs.var_int, rhs.var_dbl, 2);
			//put lhs int 2nd argument to invoke the fastest implmentation
			//std::clog << "var_eq_dbl 3. lhs int " << lhs.var_int << " compare to double " << rhs_dbl << std::endl;
			// warning: conversion from ‘exo::varint_t’ {aka ‘long int’} to ‘double’ may change value [-Wconversion]
			return almost_equal(static_cast<double>(lhs.var_int), rhs_dbl);

		}

		// Try to convert to number and try again
		// Will check unassigned

	} while (lhs.isnum());

	// 4. NON-NUMERIC STRING - always false
	//std::clog << "var_eq_dbl 4. Non-numeric string can never equal any number" << std::endl;;
	return false;

}

// toBool is the only authority on whether var is true or false
bool var_eq_bool(CBR1 lhs, const bool rhs_bool) {
	return lhs.toBool() == rhs_bool;
}

// NOTE doubles compare only to 0.0001 accuracy)
bool var_eq_int(CBR1 lhs, const int rhs_int) {

	lhs.assertVar(__PRETTY_FUNCTION__);

	// 1. EMPTY STRING always false
	if (lhs.var_typ & VARTYP_STR && lhs.var_str.empty()) {
		//std::clog << "var_eq_int 1. Empty string can never equal any number" << std::endl;
		return false;
	}

	do {

		// 2. LHS DBL
		if (lhs.var_typ & VARTYP_DBL) {

			// (DOUBLES ONLY COMPARE TO ACCURACY EXO_SMALLEST_NUMBER was 0.0001)
			//return (std::abs(lhs.var_dbl - rhs.var_dbl) < EXO_SMALLEST_NUMBER);
			//std::clog << "var_eq_int 2. lhs double " << lhs.var_dbl << " compare to int " << rhs_int << std::endl;
			return almost_equal(lhs.var_dbl, rhs_int);

		}

		// 3. LHS INT
		else if (lhs.var_typ & VARTYP_INT) {
			//std::clog << "var_eq_int 3. lhs int " << lhs.var_int << " compare to int " << rhs_int << std::endl;
			return lhs.var_int == rhs_int;

		}

		// Try to convert to number and try again
		// Will check unassigned

	}  while (lhs.isnum());

	// 4. NON-NUMERIC STRING - always false
	//std::clog << "var_eq_int 4. Non-numeric string can never equal any number" << std::endl;;
	return false;

}

} // namespace exo
