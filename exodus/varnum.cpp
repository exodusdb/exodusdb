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

/* using fastfloat instead of ryu because it is > 3 time faster
	but it doesnt prevent excessively long decimal ASCII input
	and unconfirmed if it does supports round trip ascii->double->ascii
	//from test_perf.cpp
	var v("1234.5678");  //23ns
	//v.isnum();         //+ 56ns ( 79ns using fastfloat)
	//v.isnum();         //+184ns (207ns using ryu)
	//v.isnum();         //+194ns (217ns using std::stod)
	//v.isnum();         //+???ns (???ns using <charconv> from_chars)
*/

#if EXO_MODULE > 1
	import std;
#else
#	include <charconv>
#	include <array>
#endif

// EXO_USE_TO_CHARS and EXO_USE_RYU are decided in vardefs.h
// Use ryu if GNUC < 11 and ryu include available

//ryu            1234.5678 -> "1234.5678" 500ns
//ryu_printf     1234.5678 -> "1234.5678" 800ns
//sstream/printf 1234.5678 -> "1234.5678" 1800ns

//#include <limits> //for std::numeric_limits<double>::digits10

//module #include <string>
//#include <sstream>

//fastfloat  "1234.5678" -> 1234.5678  56ns (but does it round trip?)
//ryu        "1234.5678" -> 1234.5678 184ns
//std::stod  "1234.5678" -> 1234.5678 192ns (but does it round trip?)
//from_chars "1234.5678" -> 1234.5678 ???ns (but does it round trip?)

//Eisel-Lemire algorithm seems to be fastest "parse chars to double" according to Lavavej at Microsoft
//Feb 2021 https://reviews.llvm.org/D70631 "There have been a couple of algorithmic developments very recently
// - for parsing (from_chars), the Eisel-Lemire algorithm is much faster than the technique used here.
// I would have used it if it were available back then!"
//
//double only available in gcc 11 onwards. msvc has it from 2017 or 19
//https://github.com/fastfloat/fast_float
#if __has_include(<fast_float/fast_float.h>)
#define HAS_FASTFLOAT
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#pragma clang diagnostic ignored "-Wextra-semi-stmt"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Winline"
#include <fast_float/fast_float.h>
#pragma GCC diagnostic pop
#define STD_OR_FASTFLOAT fast_float

#elif __GNUC__ >= 11
#define USE_CHARCONV
#define STD_OR_FASTFLOAT std

//#elif __has_include(<ryu/ryu.h>)
//#define HAS_RYU
//#include <ryu/ryu.h>
#endif

//gcc 10 doesnt include conv from and to floating point
//#include <charconv>	 // for from_chars and to_chars

#ifndef M_PI
//#define M_PI 3.14159265358979323846f
#define M_PI 3.14159265
// 2643383279502884197169399375105820974944592307816406286208998f;
#endif

#include <exodus/varimpl.h>

namespace exo {

#ifdef EXO_USE_TO_CHARS
/////////////////////////////////////
// Wrapper function for std::to_chars
/////////////////////////////////////

// Regarding random walk on accumulating floating point errors
// over many calculations as can happen in report totals.
// The total error over many operations is proportional to the square root of the number of operations.
// "How many ops (e.g. additions) are required to have a 90% chance of total error > 10^3 x a single error"
// if x = 10^3 x single error then    63,290,144 ops
// if x = 10^4 x single error then 6,329,014,300 ops
// Grok response to
// "if I move one cm left or right at random, how long before there is a 90% chance that I will have moved 10 meters away from my starting point?'

// Given the above then I choose to discard three digits of decimal accuracy
// from double when converting to strings to avoid floating point inaccuracy compared to decimal digits.
#define EXO_MAX_PRECISION std::numeric_limits<double>::digits10 // 15 for IEEE 8 byte double
//#define TO_STRING_NDECS EXO_MAX_PRECISION - 3 // 12 digits total e.g. 1.234'567'890'12
thread_local int TO_STRING_NDECS = EXO_MAX_PRECISION - 3; // i.e. 12 assuming var using double

// Set precision (decimal places) for floating point comparison. Useful in scientific or engineering applications.
// var: New precision between -308 and 308 inclusive.
// Returns: New precision if successful or old precision if not.
// The default precision is 4 which corresponds to 0.0001.
// By default, printing a raw var double smaller than 0.0001 without using oconv() or round() renders "0".
PUBLIC var VB1::setprecision(int new_precision) /*const*/ {
//	this->assertInteger(__PRETTY_FUNCTION__);
	// For double: -307 to +308 (binary -1021/+1024 due to 20 bit exponent)
	if (new_precision >= std::numeric_limits<double>::min_exponent10 and new_precision <= std::numeric_limits<double>::max_exponent10) {
		EXO_PRECISION = new_precision;
		EXO_SMALLEST_NUMBER = 1.0 / std::pow(10.0, EXO_PRECISION);
	} else
		std::cerr << "Error: Precision " << new_precision << " must be between " << std::numeric_limits<double>::min_exponent10 << " and " << std::numeric_limits<double>::max_exponent10 << std::endl;
	return EXO_PRECISION;
}

PUBLIC var VB1::getprecision() /*const*/ {
	return EXO_PRECISION;
}

// Used by:
// VB1::round() using std::chars_format::fixed and default precision (ndecs)
static std::string double_to_chars_to_string(double in_double, int in_decimals) {

	// Local stack working space for to_chars
	constexpr auto MAX_CHARS = 24; // 24
	std::array<char, MAX_CHARS> out_chars;

	// to_chars
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
	auto [ptr, ec] = std::to_chars(out_chars.data(), out_chars.data() + out_chars.size(), in_double, std::chars_format::fixed, in_decimals);
#pragma GCC diagnostic pop

	// Throw NON-NUMERIC if cannot convert
	if (ec != std::errc()) {
		auto e = std::make_error_code(ec);
		throw VarNonNumeric(
			"var::round: Cannot round " ^ var(std::to_string(in_double)).trimlast("0") ^
			" to " ^ in_decimals ^ " ndecimals" ^
			" using only " ^ MAX_CHARS ^ " characters." ^
			" Error: " ^ e.message() ^
			" (" ^ e.value() ^ ")"
		);
	}

	// Convert array to a std::string probably using SSO
	return std::string(out_chars.data(), ptr - out_chars.data());
}
#endif

//////////////////////////////////////////
// Utility to convert double to std string
//////////////////////////////////////////

static std::string double_to_string(const double double1) {

	std::string resultstr;

	// Disregard floating point/decimal errors and -0
//	if (std::abs(double1) < 1e-12) {
//	if (std::abs(double1) < EXO_SMALLEST_PRECISION_NUMBER) {
	if (std::abs(double1) < EXO_SMALLEST_NUMBER) {
		resultstr = "0";
		return resultstr;
	}

	// 1. Scientific precision is the total number of significant digits
	//
	// e.g. 1233.5678     is 8 significant digits.
	//      1.2345678e+03 is 8 significant digits.
	//
	// 2. C++ precision is the number of digits after the decimal point
	//
	// e.g. 1233.5678     is 4 digit precision in general format
	//      1.2345678e+03 is 7 digit precision in scientific format
	//
	// c++ precision using scientific output allows the full precision to be obtained
	// regardless of the size of the number (big or small)
	//
	// C++ Precision on fixed output only controls the precision after the decimal point
	// so the precision needs to be huge to cater for very small numbers eg < 10E-20

///////////////////////////////////
#ifdef EXO_USE_TO_CHARS // METHOD 1
///////////////////////////////////

	// 1) EXO_USE_TO_CHARS

	// Use the new low level high performance double to chars converter
	// https://en.cppreference.com/w/cpp/utility/to_chars

	// Local stack working space for to_chars
	constexpr auto MAX_CHARS = 24; // 24
	std::array<char, MAX_CHARS> out_chars;

	// to_chars
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
	auto [ptr, ec] = std::to_chars(out_chars.data(), out_chars.data() + out_chars.size(), double1, std::chars_format::general, TO_STRING_NDECS);
#pragma GCC diagnostic pop

	// Throw NON-NUMERIC if cannot convert
	if (ec != std::errc()) {
		auto e = std::make_error_code(ec);
		throw VarNonNumeric(
			"var::createString: Cannot convert:" ^ var(std::to_string(double1)).trimlast("0") ^
			" to " ^ TO_STRING_NDECS ^ " ndecimals" ^
			" using only " ^ MAX_CHARS ^ " characters." ^
			" Error: " ^ e.message() ^
			" (" ^ e.value() ^ ")"
		);
	}

	// Convert to a string. Normally will use SSO and not the heap.
	resultstr = std::string(out_chars.data(), ptr - out_chars.data());

	// Find the exponent if any
	const std::size_t epos = resultstr.find('e');
	// We are done if there is no exponent
	if (epos == std::string::npos)
		return resultstr;

	// 1e-10 -> "0.0000000001"

//std::cout << "double1  =" << double1 << std::endl;
//std::cout << "resultstr=" << resultstr << std::endl;

//////////////////////////////////////
#elif defined(EXO_USE_RYU) // METHOD 2
//////////////////////////////////////

	//std::cout << "ryu_printf decimal oconv" << std::endl;

	// TODO use std::array to avoid std::string going to the heap?
	std::string resultstr;
	resultstr.resize(24);

	// 500ns using ryu d2s() which always outputs scientific even 0E0 for zero.

	//ryu perfect round trip ASCII->double->ASCII but not perfect for calculated or compiled doubles

	//ryu output (max precision for round tripping ASCII->double->ASCII)
	//always scientific format to variable precision (cannot control precision)
	//followed by conversion to fixed format below
	//
	//But this only work perfectly for doubles obtained from ASCII.
	//i.e. calculated doubles are sometimes wrong
	//e.g. 10.0/3.0 -> 3.3333333333333335 whereas "3.3333333333333333" -> double -> "3.3333333333333333"
	//e.g. 1234567890.00005678 -> "1234567890.0000567"
	const int n = d2s_buffered_n(double1, resultstr.data());
	resultstr.resize(n);
	//resultstr is now something like "1.2345678E3" or "0E0" always in E format.
	//std::cout << resultstr << std::endl;
	//return resultstr;//435ns here. 500ns after conversion to fixed decimal place below (65ns)
	std::size_t epos = resultstr.find('E');

	// Convert ryu exponents 'E' to 'e'
	resultstr.replace(epos, 1, "e");

	// Add ryu exponent'resultstr missing '+'
	if (resultstr[epos + 1] != '-')
		resultstr.insert(epos + 1, 1, '+');

	// Add ryu exponents leading '0'
	if (epos == resultstr.size() - 3)
		resultstr.insert(epos + 2, 1, '0');

	// Add ryu missing ".0" if like "1e+07"
	if (epos == 1) {
		resultstr.insert(1, ".0");
		epos += 2;
	} else if (resultstr[0] == '-' && epos == 2) {
		resultstr.insert(2, ".0");
		epos += 2;
	}

////////////////////////////////
#else // USE_SSTREAM // OPTION 3
////////////////////////////////

	//1800ns //NOT EXO_USE_TO_CHARS or EXO_USE_RYU

	//std::cout << "std:sstream decimal oconv" << std::endl;

	std::ostringstream ss;

	//EITHER use 15 for which 64-bit IEEE 754 type double guarantees
	//roundtrip double/text/double for 15 decimal digits
	//
	//OR use precision 14 to avoid 1.1-1 = 0.1000000000000001
	//but this will only reduce errors after a single calculation
	//since errors are cumulative (random walk)
	//
	// Note that scientific precision "15" gives us
	// 16 total digits "1.234567890123456e+00"
	//

	//if not fixed precision then precision is relative to first digit, not decimal point
	//therefore very small numbers can have 15 digits of precision eg.
	//0.000000000000000000001234567890123456

	//use digits10 +1 if not using auto formatting (scientific for large/small numbers)

	//there appears to be little or no speed difference between always going via scientific format
	//and default method which only goes via scientific format for very large or small numbers
#define ALWAYS_VIA_SCIENTIFIC
#ifdef ALWAYS_VIA_SCIENTIFIC
	//use digits10 if using scientific because we automatically get one additional on the left
	//ss.precision(15);
	ss.precision(std::numeric_limits<double>::digits10);
	//ss.precision(16);
	ss << std::scientific;
#else
	//use digits10 +1 if not using auto formatting (scientific for large/small numbers)
	ss.precision(16);
	//ss.precision(12);
#endif

	ss << double1;
	std::string resultstr = ss.str();

	std::size_t epos = resultstr.find('e');

	//if not scientific format then return as is
	////////////////////////////////////////////
#ifndef ALWAYS_VIA_SCIENTIFIC
	if (epos == std::string::npos)
		return resultstr;
#endif
		//std::cout << "xxxxxxxxxxxxxxxxxxxxxxxxxxxx" << std::endl;

#endif	//EXO_USE_RYU / do not use RYU


////////////////////////////////////////////////////////////
// PROCESS THE SCIENTIFIC OUTPUT OF THE ABOVE 3 ALTERNATIVES
////////////////////////////////////////////////////////////

	//                                             cout << dbl   cout << var
	//              ".000012345678901234567890"    1.23457e-05   1.23456789012e-05
	// "1234567890123.4567890"                     1.23457e+12   1.23456789012e+12
	// return resultstr;

	// Get trailing exponent as a signed int from -308 to 308
	auto exponent = stoi(resultstr.substr(epos + 1));

	// Get the exponent substring with its leading + or - char
	auto exponent_text = resultstr.substr(epos);

	//Remove the trailing exponent from the result (everything starting at the E/e)
	if (epos != std::string::npos)
		resultstr.erase(epos);

//std::cerr << "SCIENTIFIC=" << resultstr << std::endl;

	// Exponent 0 special treatment and exit
	if (!exponent) {

//#ifndef EXO_USE_RYU_D2S
		//remove trailing zeros and decimal point
		while (resultstr.back() == '0')
			resultstr.pop_back();
		if (resultstr.back() == '.')
			resultstr.pop_back();
			//resultstr.push_back('0');
//#ifdef EXO_USE_RYU
		//single zero if none
		//if (resultstr.size() == std::size_t(minus))
		if (resultstr.empty() || resultstr == "-")
			resultstr.push_back('0');
//#endif
//#endif
		return resultstr;
	}

	//leave exponent in if abs(exponent) is > 15
	////////////////////////////////////////////
	//if (exponent < -6 or exponent > 12) {
	//if (abs(exponent> > 15) {
//	if (std::abs(exponent) > std::numeric_limits<double>::digits10) {
//	if (exponent > std::numeric_limits<double>::digits10) {
	if (std::abs(exponent) >= TO_STRING_NDECS) {

		//remove trailing zeros
		while (resultstr.back() == '0')
			resultstr.pop_back();
		if (resultstr.back() == '.')
			//resultstr.push_back('0');
			resultstr.pop_back();

		//append the exponent text
		resultstr.append(exponent_text);

		return resultstr;
	}

	// "Small" exponents just move the decimal point and show without exponent
	//////////////////////////////////////////////////////////////////////////

	int minus = double1 < 0 ? 1 : 0;

	//positive exponent
	if (exponent > 0) {

		//remove decimal point
		resultstr.erase(1 + minus, 1);

		//determine how many zeros need appending if any
		int addzeros = exponent - static_cast<int>(resultstr.size()) + 1 + minus;

		//debugging
		//std::cout << ss.str() << " " << resultstr << " " << exponent << " " << resultstr.size() << " " << addzeros << std::endl;

		//either append sufficient zeros
		if (addzeros > 0) {
			resultstr.append(addzeros, '0');

		}

		//or insert decimal point within existing matissa
		else if (addzeros < 0) {

			//insert decimal point
			resultstr.insert(exponent + minus + 1, 1, '.');

			goto removetrailing;
		}

		//negative exponent
	} else {

		// c 'std::cerr << var(0.1) + var(0.2) - var(0.3) << std::endl;' {i}
		// 5.55111512313e-17
//		if (-exponent > TO_STRING_NDECS) {
		if (-exponent > EXO_PRECISION) {
			resultstr = "0";
			return resultstr;
		}
		//else if (exponent <= 16)
		//	 return resultstr;

		//remove decimal point
		resultstr.erase(1 + minus, 1);

		//prefix sufficient zeros
		resultstr.insert(0 + minus, -exponent, '0');

		//insert decimal point
		resultstr.insert(1 + minus, 1, '.');

removetrailing:
		//remove trailing zeros
		while (resultstr.back() == '0')
			resultstr.pop_back();
		//std::size_t lastdigit = resultstr.find_last_not_of('0');
		//if (lastdigit != std::string::npos)
		//	resultstr.resize(lastdigit+1);

		//remove trailing decimal point
		if (resultstr.back() == '.')
			resultstr.pop_back();
	}

	return resultstr;
}

/////////////////////////
// var_base::createstring
/////////////////////////

// mainly called in ISSTRING when not already a string
PUBLIC void VB1::createString() const {

	// TODO ensure ISVAR is called everywhere in advance
	// to avoid wasting time doing multiple calls to ISVAR
	// this->assertVar(__PRETTY_FUNCTION__);

	// dbl - create string from dbl
	// prefer double
	if (var_typ & VARTYP_DBL) {
		var_str = double_to_string(var_dbl);
		var_typ |= VARTYP_STR;
		return;
	}

	// int - create string from int
	if (var_typ & VARTYP_INT) {
		var_str = std::to_string(var_int);
		var_typ |= VARTYP_STR;
		return;
	}
	// already a string (unlikely since only called when not a string)
	if (var_typ & VARTYP_STR) LIKELY {
		return;
	}

	// treat any other case as unassigned
	//(usually var_typ & VARTYP_UNA)
	throw VarUnassigned("createString()");
}

//////////////////
// var_base::isnum
//////////////////

// RULES need updating since we are now allowing E/e scientific notation
//
// numeric is one of four regular expressions or zero length string
//^$			zero length string
//[+-]?9+		eg 999
//[+-]?9+.		eg 999.
//[+-]?.9+		eg .999
//[+-]?9+.9+	eg 999.999
// where the last four examples may also have a + or - character prefix

// be careful that the following are NOT numeric. regexp [+-]?[.]?
// + - . +. -.

// rules
// 0. zero length string is numeric integer 0
// 1. any + or - must be the first character
// 2. point may occur 0 or 1 times
// 3. digits (0-9) must occur 1 or more times (but see rule 0.)
// 4. all characters mean non-numeric

PUBLIC bool VB1::isnum(void) const {

	// TODO make isnum private and ensure ISVAR is checked before all calls to isnum
	// to save the probably double check here
	this->assertVar(__PRETTY_FUNCTION__);

	// Known to be numeric already
	if (var_typ & VARTYP_INTDBL)
		return true;

	// Known to be not numeric already
	// maybe put this first if comparison operations on strings are more frequent than numeric
	// operations on numbers
	if (var_typ & VARTYP_NAN)
		return false;

	// Not assigned error
	if (!var_typ)
		throw VarUnassigned("isnum()");

	// Empty string is zero. Leave the string as "".
	auto strlen = var_str.size();
	if (strlen == 0) {
		var_int = 0;
		var_typ = VARTYP_INTSTR;
		return true;
	}

	// Preparse the string to detect if integer or decimal or many types of non-numeric
	// We need to detect NAN ASAP because string comparison always attempts to compare numerically.
	// The parsing does not need to be perfect since we will rely
	// on from_chars for the final parsing
	bool floating = false;
	bool has_sign = false;
	for (std::size_t ii = 0; ii < strlen; ii++) {
		char cc = var_str[ii];

		// +   2B
		// -   2D
		// .   2E
		// 0-9 30-39
		// E   45
		// e   65

		// Ideally we put the most divisive test first
		// but, assuming that non-numeric strings are tested more frequently
		// then following is perhaps not the best choice.
		if (cc >= '0' and cc <= '9')
			continue;

		switch (cc) {

		case '.':
			floating = true;
			break;

		case '-':
		case '+':
			// Disallow more than one sign eg "+-999"
			// Disallow a single + since it will be trimmed off below
			if (has_sign or strlen == 0) {
				var_typ = VARTYP_NANSTR;
				return false;
			}
			has_sign = true;
			break;

		case 'e':
		case 'E':
			floating = true;
			// Allow a sign again after any e/E
			has_sign = false;
			break;

		default:
			var_typ = VARTYP_NANSTR;
			return false;
		}
	}

#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
	char* firstchar = var_str.data();
	char* lastchar = firstchar + strlen;

	// Skip leading + to be compatible with javascript
	// from_chars does not allow it.
	firstchar += *firstchar == '+';
#pragma GCC diagnostic pop

	if (floating) {

		// to double
		auto [p, ec] = STD_OR_FASTFLOAT::from_chars(firstchar, lastchar, var_dbl);
		if (ec != std::errc() || p != lastchar) {
			var_typ = VARTYP_NANSTR;
			return false;
		}

		// indicate that the var is a string and a double
		var_typ = VARTYP_DBLSTR;

	} else {

		// to long int
		auto [p, ec] = std::from_chars(firstchar, lastchar, var_int);
		if (ec != std::errc() || p != lastchar) {
			var_typ = VARTYP_NANSTR;
			return false;
		}

		// indicate that the var is a string and a long int
		var_typ = VARTYP_INTSTR;
	}

	return true;
}

PUBLIC RETVAR VB1::num(void) const {
	RETVAR result;
	if (this->isnum())
		result = this->clone();
	else
		result = 0;
	return result;
}

PUBLIC void VB1::assertNumeric(const char* message, const char* varname/* = ""*/) const {
	if (!this->isnum()) {
		UNLIKELY
		//throw VarNonNumeric(var_base(varname) ^ " in " ^ var_base(message) ^ " data: " ^ var_str.substr(0,127));
		throw VarNonNumeric(std::string(varname) + " in '" + message + "' is '" + var_str.substr(0, 32) + "'");
	}
}

///////////////////
// var_base::toBool
///////////////////

PUBLIC bool VB1::toBool() const {

	// could be skipped for speed assuming that people will not write unusual "var x=f(x)" type
	// syntax as follows: var xx=xx?11:22;
	this->assertVar(__PRETTY_FUNCTION__);

	// identical code in void* and bool except returns void* and bool respectively
	while (true) {

		// ints are true except for zero
		if (var_typ & VARTYP_INT)
			//return static_cast<bool>(var_int != 0);
			return var_int != 0;

		// non-numeric strings are true unless empty
		// but empty string is numeric int 0
		// so NAN -> true
		if (var_typ & VARTYP_NAN)
			return true;

		// doubles are true unless zero
		// check double first dbl on guess that tests will be most often on financial
		// numbers
		// TODO should we consider very small numbers to be the same as zero?
		if (var_typ & VARTYP_DBL)
			//return (bool)(var_dbl != 0);
			//return std::abs(var_dbl)>=0.00000000001;
			//pickos print (0.00001 or 0)    ... prints 0 (bool)
			//pickos print (0.00005=0.00006) ... prints 0 (==)
			//pickos print (0.00005<0.00006) ... prints 1 (<)
			return std::abs(var_dbl) >= EXO_SMALLEST_NUMBER;

		if (!(var_typ)) {
			this->assertAssigned(__PRETTY_FUNCTION__);
			throw VarUnassigned("toBool()");
		}

		// must be string - try to convert to numeric and do all tests again
		if (this->isnum()) {}
	}
}

//////////////////
// var_base::toInt
//////////////////

PUBLIC int VB1::toInt() const {
	this->assertInteger(__PRETTY_FUNCTION__);
	return static_cast<int>(var_int);
}

////////////////////
// var_base::toInt64
////////////////////

PUBLIC std::int64_t VB1::toInt64() const {
	this->assertInteger(__PRETTY_FUNCTION__);
	return static_cast<std::int64_t>(var_int);
}

///////////////////
// var_base::toSize
///////////////////

PUBLIC std::size_t VB1::toSize() const {
	this->assertInteger(__PRETTY_FUNCTION__);
	return static_cast<std::size_t>(var_int);
}

/////////////////////
// var_base::toDouble
/////////////////////

PUBLIC double VB1::toDouble() const {
	this->assertDecimal(__PRETTY_FUNCTION__);
	return var_dbl;
}

//////////////////
// var_base::round
//////////////////

RETVAR VB1::round(const int ndecimals) const {

/*
	"Round half away from zero" or "Commercial rounding"
	https://en.wikipedia.org/wiki/Rounding#Round_half_away_from_zero

	The other tie-breaking method commonly taught and used is the round half
	away from zero (or round half toward infinity), namely:

	If the fraction of x is exactly 0.5, then y = x + 0.5 if x is positive,
	and y = x − 0.5 if x is negative.

	For example, 23.5 gets rounded to 24, and −23.5 gets rounded to −24.

	This can be more efficient on binary computers because only the first
	omitted bit needs to be considered to determine if it rounds up (on a 1)
	or down (on a 0). This is one method used when rounding to significant
	figures due to its simplicity.

	This method, also known as commercial rounding,[citation needed] treats
	positive and negative values symmetrically, and therefore is free of
	overall positive/negative bias if the original numbers are positive or
	negative with equal probability. It does, however, still have bias away
	from zero.

	It is often used for currency conversions and price roundings (when the
	amount is first converted into the smallest significant subdivision of
	the currency, such as cents of a euro) as it is easy to explain by just
	considering the first fractional digit, independently of supplementary
	precision digits or sign of the amount (for strict equivalence between
	the paying and recipient of the amount).
*/

/*
	pickos round rounds all positive .5/1.5/2.5 up to 1/2/3 and negative .5/1.5/2.5 down to -1/-2/-3 etc
	(not bankers rounding which is evens 0.5/2.5 etc. rounds towards zero (0/2) and odds 1.5/3.5 roundsaway from zero 2/4)
	-1.5=-2
	-1.0=-1
	-0.9=-1
	-0.5=-1
	-0.1= 0
	 0  = 0
	 0.1= 0
	 0.5= 1.0
	 0.9= 1.0
	 1.0= 1.0
	 1.5= 2.0

*/

	//var_str is always set on return

	// if n=0 could save the integer conversion here but would require mentality to save BOTH
	// int and double currently its possible since space is reserved for both but this may
	// change

	this->assertNumeric(__PRETTY_FUNCTION__);

	var result;

	// Sadly we cannot use var_int even if available since
	// var_int may have been created from var_dbl using trunc()

	// prefer double
	double fromdouble;
	if (var_typ & VARTYP_DBL) {
		fromdouble = var_dbl;
	}

	//otherwise use var_int
	else {

		//if zero decimal places required then use simply use the var_int
		if (not ndecimals) {
			//result=*this;
			result.var_int = var_int;
			result.var_typ = VARTYP_INT;
			// We could precalculate the string and double very fast
			// but they may not be required and be wasted effort
			result.createString();
			return result;
		}

		// loss of precision if var_int is std::uint64_t
		fromdouble = static_cast<double>(var_int);
	}

	//unfortunately c+ round(double) does not work well with decimal numbers
	//because some decimal numbers ending .5 are represented in binary
	//internally as .499999999999999965234
	//which will round DOWN not up
	//
	//For example 6000.50/5 is 300.025 really but
	//in binary it is 300.024999999999999
	//
	//Therefore we must do manual rounding up where
	//adding 0.5 closely agrees with the next higher integer
	//
	//both pickdb and c++ rounding tie break goes away from zero.

	if (std::abs(fromdouble) < 0.000'000'000'1)
		fromdouble = 0;

	double scale = std::pow(10.0, ndecimals);
	double scaled_double = fromdouble * scale;
	double ceil2 = std::ceil(scaled_double);
	double diff = (scaled_double + 0.5) - ceil2;
	//if very close to 0.5 mark then round up/down using ceil/floor
	double rounded_double;
	//if (std::abs(diff) < EXO_SMALLEST_NUMBER) {
	//if (std::abs(diff) < 0.000'000'000'000'1) {
	if (std::abs(diff) < 0.000'000'000'1) {
		if (fromdouble >= 0)
			rounded_double = ceil2 / scale;
		else
			rounded_double = std::floor(scaled_double) / scale;
	}
	//otherwise use standard rounding
	else {
		rounded_double = std::round(scaled_double) / scale;
	}

	//TRACE(ndecimals)
	//TRACE(fromdouble)
	//TRACE(scale)
	//TRACE(scaled_double)
	//TRACE(scaled_double + 0.5)
	//TRACE(ceil)
	//TRACE(diff)
	//TRACE(rounded_double)

#ifdef EXO_USE_TO_CHARS

	// Use the new low level high performance double to chars converter
	// https://en.cppreference.com/w/cpp/utility/to_chars

	// std::chars_format::fixed
	result.var_str = double_to_chars_to_string(rounded_double, ndecimals >= 0 ? ndecimals : 0);

#else
	// We might use ryu here if installed but will not bother as to_chars(double) is built-in to g++v11 in Ubuntu 22.04
	std::stringstream ss;
	ss.precision(ndecimals >= 0 ? ndecimals : 0);
	ss << std::fixed << rounded_double;
	result.var_str = ss.str();

#endif

	result.var_dbl = rounded_double;
	result.var_typ = VARTYP_DBLSTR;

	return result;
}

} // namespace exo
