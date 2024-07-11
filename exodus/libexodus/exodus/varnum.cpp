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

// Use ryu if GNUC < 11 and ryu include available
//ryu            1234.5678 -> "1234.5678" 500ns
//ryu_printf     1234.5678 -> "1234.5678" 800ns
//sstream/printf 1234.5678 -> "1234.5678" 1800ns

// 1. TO_CHARS from Ubuntu 22.04
#if __GNUC__ >= 11 || __clang_major__ >=  14
#define USE_TO_CHARS
#include <array>

// 2. RYU
#elif __has_include(<ryu/ryu.h>)
#define USE_RYU
#include <ryu/ryu.h>

// 3. STRINGSTREAM
#else
#endif

#include <limits> //for std::numeric_limits<double>::digits10

#include <string>
#include <sstream>

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
#include <charconv>	 // for from_chars and to_chars

#ifndef M_PI
//#define M_PI 3.14159265358979323846f
#define M_PI 3.14159265
// 2643383279502884197169399375105820974944592307816406286208998f;
#endif

#include <exodus/varimpl.h>

namespace exo {

#ifdef USE_TO_CHARS
/////////////////////////////////////
// Wrapper function for std::to_chars
/////////////////////////////////////

static std::string double_to_chars_to_string(
		double double1,
		std::chars_format format = std::chars_format::general,
		int decimals = std::numeric_limits<double>::digits10 + 1
	) {

	// Local stack working space for to_chars
	constexpr auto MAX_CHARS = 24;
	std::array<char, MAX_CHARS> chars;

	// to_chars
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
	auto [ptr, ec] = std::to_chars(chars.data(), chars.data() + chars.size(), double1, format, decimals);
#pragma GCC diagnostic pop

	// Throw NON-NUMERIC if cannot convert
	if (ec != std::errc())
		throw VarNonNumeric("var::round: Cannot round " ^ var(double1) ^ " ndecimals: " ^ decimals ^ " to " ^ MAX_CHARS ^ " characters");

	// Convert to a string.
	// Hopefully using small string optimisation (SSO)
	return std::string(chars.data(), ptr - chars.data());
}
#endif

//////////////////////////////////////////
// Utility to convert double to std string
//////////////////////////////////////////

static std::string double_to_string(double double1) {

	// prevent "-0"
	if (!double1)
		return "0";

	int minus = double1 < 0 ? 1 : 0;

	//Precision on scientific output allows the full precision to obtained
	//regardless of the size of the number (big or small)
	//
	//Precision on fixed output only controls the precision after the decimal point
	//so the precision needs to be huge to cater for very small numbers eg < 10E-20

#ifdef USE_TO_CHARS

	// 1) USE_TO_CHARS

	// Use the new low level high performance double to chars converter
	// https://en.cppreference.com/w/cpp/utility/to_chars

//	// Use local data to avoid using heap
//	// The result chars are very likely to fit in std::string SSO without using the heap
//	// (15 bytes in gcc and 23 in clang)
//	constexpr auto MAX_CHARS = 24;
//	std::array<char, MAX_CHARS> chars;
//
//	//auto [ptr, ec] = std::to_chars(chars.data(), chars.data() + chars.size(), double1, std::chars_format::scientific);
//#pragma GCC diagnostic push
//#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
//	auto [ptr, ec] = std::to_chars(chars.data(), chars.data() + chars.size(), double1, std::chars_format::general, std::numeric_limits<double>::digits10 + 1);
//	//auto [ptr, ec] = std::to_chars(chars.data(), chars.data() + chars.size(), double1, std::chars_format::general, precision);
//#pragma GCC diagnostic pop
//
//	// Throw if non-numeric
//	if (ec != std::errc())
//		UNLIKELY
//		throw VarNonNumeric("double_to_string: Cannot convert double to " ^ var(MAX_CHARS) ^ " characters");
//
//	// Create a std string from the fixed array of char
//	// Probably it will use small string optimisation (SSO)
//	// - gcc 15 chars
//	// - clang 23 chars
//	std::string resultstr = std::string(chars.data(), ptr - chars.data());

	std::string resultstr = double_to_chars_to_string(double1);

	// Find the exponent if any
	const std::size_t epos = resultstr.find('e');

	// We are done if there is no exponent
	if (epos == std::string::npos)
		return resultstr;

#elif defined(USE_RYU)

	//std::cout << "ryu_printf decimal oconv" << std::endl;

	// TODO use std::array to avoid std::string going to the heap?
	std::string resultstr;
	resultstr.resize(24);

//#define USE_RYU_D2S
//#ifdef USE_RYU_D2S

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
//#elif 0
//
//	//740ns ryu d2exp() always output scientific
//
//	//ryu_printf compatible with nice rounding of all doubles but not perfect round tripping
//
//	//ryu_printf %e equivalent (always scientific format to properly control precision)
//	//followed by conversion to fixed format below
//	//using precision 15 (which means 16 digits in scientific format)
//	const int n = d2exp_buffered_n(double1, 15, resultstr.data());
//	resultstr.resize(n);
//	//resultstr is now something like "1.234567800000000e+03"
//	//std::cout << resultstr << std::endl;
//	//return resultstr;//650ns here. 743ns after changing to fixed point below (93ns)
//	const std::size_t epos = resultstr.find('e');
//
//#else
//
//	//???ns ryu d2fixed() always output fixed decimal point with fixed precision
//
//	//ryu_printf %f equivalent (always fixed format)
//	//
//	//But this suffers from precision being after decimal point instead of overall number of digits
//	// eg 1234.5678 -> "1234.56780000000003"
//	//printl( var("999999999999999.9")    + 0);
//	//             999999999999999.875
//	//Could truncate after 15 digits but this would not be rounded properly
//	const int n = d2fixed_buffered_n(double1, 16, resultstr.data());
//	resultstr.resize(n);
//	//remove trailing zeros
//	//while (resultstr.back() == '0')
//	//	resultstr.pop_back();
//	std::size_t lastdigit = resultstr.find_last_not_of('0');
//	if (lastdigit != std::string::npos)
//		resultstr.resize(lastdigit);
//	//remove trailing decimal point
//	if (resultstr.back() == '.')
//		resultstr.pop_back();
//	return resultstr;
//	const std::size_t epos = std::string::npos;
//#endif
//	//std::cout << resultstr << std::endl;

#else //USE_SSTREAM

	//1800ns //NOT USE_TO_CHARS or USE_RYU

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

	////////////////////////////////////////////
	//if not scientific format then return as is
	////////////////////////////////////////////
#ifndef ALWAYS_VIA_SCIENTIFIC
	if (epos == std::string::npos)
		return resultstr;
#endif
		//std::cout << "xxxxxxxxxxxxxxxxxxxxxxxxxxxx" << std::endl;

#endif	//USE_RYU / do not use RYU

// PROCESS THE OUTPUT OF THE ABOVE 3 ALTERNATIVES
/////////////////////////////////////////////////

	// Get exponent as a signed int from -308 to 308
	auto exponent = stoi(resultstr.substr(epos + 1));

	// Get the exponent as text with its leading + or - char
	auto exponent_text = resultstr.substr(epos);

	//Remove the exponent (everything starting at the E/e)
	if (epos != std::string::npos)
		resultstr.erase(epos);

	// Exponent 0 special treatment and exit
	if (!exponent) {

//#ifndef USE_RYU_D2S
		//remove trailing zeros and decimal point
		while (resultstr.back() == '0')
			resultstr.pop_back();
		if (resultstr.back() == '.')
			resultstr.pop_back();
			//resultstr.push_back('0');
//#ifdef USE_RYU
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
	if (std::abs(exponent) > std::numeric_limits<double>::digits10) {

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

////////////////////
// var::createstring
////////////////////

// mainly called in ISSTRING when not already a string
template<> PUBLIC void VARBASE1::createString() const {

	// TODO ensure ISDEFINED is called everywhere in advance
	// to avoid wasting time doing multiple calls to ISDEFINED
	// this->assertDefined(__PRETTY_FUNCTION__);

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

/////////////
// var::isnum
/////////////

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

// Explicit specialisation
//template<>           PUBLIC bool var_base<var_mid<var>>::isnum(void) const;

// Explicit instantiation
//template             PUBLIC bool var_base<var_mid<var>>::isnum(void) const;

template<> PUBLIC bool VARBASE1::isnum(void) const {

	// TODO make isnum private and ensure ISDEFINED is checked before all calls to isnum
	// to save the probably double check here
	this->assertDefined(__PRETTY_FUNCTION__);

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

template<> PUBLIC void VARBASE1::assertNumeric(const char* message, const char* varname/* = ""*/) const {
	if (!this->isnum()) {
		UNLIKELY
		//throw VarNonNumeric(var_base(varname) ^ " in " ^ var_base(message) ^ " data: " ^ var_str.substr(0,127));
		throw VarNonNumeric(std::string(varname) + " in '" + message + "' is '" + var_str.substr(0, 32) + "'");
	}
}

//////////////
// var::toBool
//////////////

template<> PUBLIC bool VARBASE1::toBool() const {

	// could be skipped for speed assuming that people will not write unusual "var x=f(x)" type
	// syntax as follows: var xx=xx?11:22;
	this->assertDefined(__PRETTY_FUNCTION__);

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
			return std::abs(var_dbl) >= SMALLEST_NUMBER;

		if (!(var_typ)) {
			this->assertAssigned(__PRETTY_FUNCTION__);
			throw VarUnassigned("toBool()");
		}

		// must be string - try to convert to numeric and do all tests again
		this->isnum();
	}
}

/////////////
// var::toInt
/////////////

template<> PUBLIC int VARBASE1::toInt() const {
	this->assertInteger(__PRETTY_FUNCTION__);
	return static_cast<int>(*this);
}

///////////////
// var::toInt64
///////////////

template<> PUBLIC int64_t VARBASE1::toInt64() const {
	this->assertInteger(__PRETTY_FUNCTION__);
	return static_cast<int64_t>(*this);
}

////////////////
// var::toDouble
////////////////

template<> PUBLIC double VARBASE1::toDouble() const {
	this->assertDecimal(__PRETTY_FUNCTION__);
	return var_dbl;
}

/////////////
// var::round
/////////////

var var::round(const int ndecimals) const {

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

		// loss of precision if var_int is uint64_t
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
	//if (std::abs(diff) < SMALLEST_NUMBER) {
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

#ifdef USE_TO_CHARS

	// Use the new low level high performance double to chars converter
	// https://en.cppreference.com/w/cpp/utility/to_chars

//	constexpr int maxchars {48};
//
//	// Local 
//	std::array<char, maxchars> chars;
//
//	// Fixed decimal places
//	// Specific number of decimal places
//	auto [ptr, ec] = std::to_chars(chars.data(), chars.data() + chars.size(), rounded_double, std::chars_format::fixed, ndecimals >= 0 ? ndecimals : 0);
//
//	// Throw if non-numeric
//	if (ec != std::errc())
//		//throw VarNonNumeric("var::round: Cannot convert to 24 characters");
//		throw VarNonNumeric("var::round: Cannot round " ^ var(fromdouble) ^ " ndecs: " ^ ndecimals ^ " to " ^ maxchars ^ " characters");
//
//	// Convert to a string. Hopefully using small string optimisation (SSO)
//	result.var_str = std::string(chars.data(), ptr - chars.data());

	result.var_str = double_to_chars_to_string(rounded_double, std::chars_format::fixed, ndecimals >= 0 ? ndecimals : 0);

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

///////////////
// var::integer
///////////////

// function name is "integer" instead of "int" because int is a reserved word in c/c++ for int datatype
// using the system int() function on a var e.g. int(varx) returns an int whereas this function returns a var
var var::integer() const {

    //-1.0 = -1

    //-0.9 = 0
    //-0.5 = 0
    //-0.1 = 0

    // 0   = 0

    // 0.1 = 0
    // 0.5 = 0
    // 0.9 = 0

    // 1.0 = 1

	this->assertInteger(__PRETTY_FUNCTION__);
	return var_int;
}

/////////////
// var::floor
/////////////

var var::floor() const {

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

	this->assertNumeric(__PRETTY_FUNCTION__);

	if (!(var_typ & VARTYP_INT)) {

		//warning: conversion from ‘double’ to ‘long int’ may change value [-Wfloat-conversion]
		var_int = static_cast<varint_t>(std::floor(var_dbl));
		//var_int = std::floor<varint_t>(var_dbl);

		// Add int flag
		var_typ |= VARTYP_INT;
	}

	return var_int;
}

} // namespace exo
