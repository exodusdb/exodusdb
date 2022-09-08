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
#if __GNUC__ >= 11
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
#include <fast_float/fast_float.h>
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

namespace exodus {

std::string mvd2s(double double1) {

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

	std::array<char, 24> chars;
	//auto [ptr, ec] = std::to_chars(chars.data(), chars.data() + chars.size(), double1, std::chars_format::scientific);
	auto [ptr, ec] = std::to_chars(chars.data(), chars.data() + chars.size(), double1, std::chars_format::general, std::numeric_limits<double>::digits10 + 1);

	// Throw if non-numeric
	if (ec != std::errc())
		throw VarNonNumeric("mvd2s: Cannot convert double to 24 characters");

	// Convert to a string. Hopefully using small string optimisation (SSO)
	std::string s = std::string(chars.data(), ptr - chars.data());

//	return s;

	// Find the exponent if any
	const std::size_t epos = s.find('e');

	// We are done if there is no exponent
	if (epos == std::string::npos)
		return s;

#elif defined(USE_RYU)

	//std::cout << "ryu_printf decimal oconv" << std::endl;

	std::string s;
	s.resize(24);

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
	const int n = d2s_buffered_n(double1, s.data());
	s.resize(n);
	//s is now something like "1.2345678E3" or "0E0" always in E format.
	//std::cout << s << std::endl;
	//return s;//435ns here. 500ns after conversion to fixed decimal place below (65ns)
	std::size_t epos = s.find('E');

	// Convert ryu exponents 'E' to 'e'
	s.replace(epos, 1, "e");

	// Add ryu exponent's missing '+'
	if (s[epos + 1] != '-')
		s.insert(epos + 1, 1, '+');

	// Add ryu exponents leading '0'
	if (epos == s.size() - 3)
		s.insert(epos + 2, 1, '0');

	// Add ryu missing ".0" if like "1e+07"
	if (epos == 1) {
		s.insert(1, ".0");
		epos += 2;
	} else if (s[0] == '-' && epos == 2) {
		s.insert(2, ".0");
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
//	const int n = d2exp_buffered_n(double1, 15, s.data());
//	s.resize(n);
//	//s is now something like "1.234567800000000e+03"
//	//std::cout << s << std::endl;
//	//return s;//650ns here. 743ns after changing to fixed point below (93ns)
//	const std::size_t epos = s.find('e');
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
//	const int n = d2fixed_buffered_n(double1, 16, s.data());
//	s.resize(n);
//	//remove trailing zeros
//	//while (s.back() == '0')
//	//	s.pop_back();
//	std::size_t lastdigit = s.find_last_not_of('0');
//	if (lastdigit != std::string::npos)
//		s.resize(lastdigit);
//	//remove trailing decimal point
//	if (s.back() == '.')
//		s.pop_back();
//	return s;
//	const std::size_t epos = std::string::npos;
//#endif
//	//std::cout << s << std::endl;

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
	std::string s = ss.str();

	std::size_t epos = s.find('e');

	////////////////////////////////////////////
	//if not scientific format then return as is
	////////////////////////////////////////////
#ifndef ALWAYS_VIA_SCIENTIFIC
	if (epos == std::string::npos)
		return s;
#endif
		//std::cout << "xxxxxxxxxxxxxxxxxxxxxxxxxxxx" << std::endl;

#endif	//USE_RYU / do not use RYU

/////////////////////////////////////////////////
// PROCESS THE OUTPUT OF THE ABOVE 3 ALTERNATIVES
/////////////////////////////////////////////////

	// Get exponent as a signed int from -308 to 308
	auto exponent = stoi(s.substr(epos + 1));

	// Get the exponent as text with its leading + or - char
	auto exponent_text = s.substr(epos);

	//Remove the exponent (everything starting at the E/e)
	if (epos != std::string::npos)
		s.erase(epos);

	// Exponent 0 special treatment and exit
	if (!exponent) {

//#ifndef USE_RYU_D2S
		//remove trailing zeros and decimal point
		while (s.back() == '0')
			s.pop_back();
		if (s.back() == '.')
			s.pop_back();
			//s.push_back('0');
//#ifdef USE_RYU
		//single zero if none
		//if (s.size() == std::size_t(minus))
		if (s.empty() || s == "-")
			s.push_back('0');
//#endif
//#endif
		return s;
	}

	////////////////////////////////////////////
	//leave exponent in if abs(exponent) is > 15
	////////////////////////////////////////////
	//if (exponent < -6 or exponent > 12) {
	//if (abs(exponent> > 15) {
	if (std::abs(exponent) > std::numeric_limits<double>::digits10) {

		//remove trailing zeros
		while (s.back() == '0')
			s.pop_back();
		if (s.back() == '.')
			//s.push_back('0');
			s.pop_back();

		//append the exponent text
		s.append(exponent_text);

		return s;
	}

	//////////////////////////////////////////////////////////////////////////
	// "Small" exponents just move the decimal point and show without exponent
	//////////////////////////////////////////////////////////////////////////

	//positive exponent
	if (exponent > 0) {

		//remove decimal point
		s.erase(1 + minus, 1);

		//determine how many zeros need appending if any
		int addzeros = exponent - s.size() + 1 + minus;

		//debugging
		//std::cout << ss.str() << " " << s << " " << exponent << " " << s.size() << " " << addzeros << std::endl;

		//either append sufficient zeros
		if (addzeros > 0) {
			s.append(addzeros, '0');

		}

		//or insert decimal point within existing matissa
		else if (addzeros < 0) {

			//insert decimal point
			s.insert(exponent + minus + 1, 1, '.');

			goto removetrailing;
		}

		//negative exponent
	} else {

		//remove decimal point
		s.erase(1 + minus, 1);

		//prefix sufficient zeros
		s.insert(0 + minus, -exponent, '0');

		//insert decimal point
		s.insert(1 + minus, 1, '.');

removetrailing:
		//remove trailing zeros
		while (s.back() == '0')
			s.pop_back();
		//std::size_t lastdigit = s.find_last_not_of('0');
		//if (lastdigit != std::string::npos)
		//	s.resize(lastdigit+1);

		//remove trailing decimal point
		if (s.back() == '.')
			s.pop_back();
	}

	return s;
}


// mainly called in ISSTRING when not already a string
void var::createString() const {
	// TODO ensure ISDEFINED is called everywhere in advance
	// to avoid wasting time doing multiple calls to ISDEFINED
	// this->assertDefined(__PRETTY_FUNCTION__);

	// dbl - create string from dbl
	// prefer double
	if (var_typ & VARTYP_DBL) {
		var_str = mvd2s(var_dbl);
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
	if (var_typ & VARTYP_STR) {
		return;
	}

	// treat any other case as unassigned
	//(usually var_typ & VARTYP_UNA)
	throw VarUnassigned("createString()");
}

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
	pickos round rounds positive .5 up to 1 and negative .5 down to -1 etc
	-1.0=-1
	-0.9=-1
	-0.5=-1
	-0.1= 0
	 0  = 0
	 0.1= 0
	 0.5= 1.0
	 0.9= 1.0
	 1.0= 1.0
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

	std::array<char, 24> chars;
	auto [ptr, ec] = std::to_chars(chars.data(), chars.data() + chars.size(), rounded_double, std::chars_format::fixed, ndecimals >= 0 ? ndecimals : 0);

	// Throw if non-numeric
	if (ec != std::errc())
		throw VarNonNumeric("var::round: Cannot convert double to 24 characters");

	// Convert to a string. Hopefully using small string optimisation (SSO)
	result.var_str = std::string(chars.data(), ptr - chars.data());

#else
	// We might use ryu here if installed but will not bother as to_chars(double) is built-in to g++v11 in Ubuntu 22.04
	std::stringstream ss;
	ss.precision(ndecimals >= 0 ? ndecimals : 0);
	ss << std::fixed << rounded_double;
	result.var_str = ss.str();

#endif

	result.var_typ = VARTYP_STR;

	return result;
}

// function name is "integer" instead of "int" because int is a reserved word in c/c++ for int datatype
// using the system int() function on a var e.g. int(varx) returns an int whereas this function returns a var
var var::integer() const {

    //-1.0=-1
    //-0.9=0
    //-0.5=0
    //-0.1=0
    // 0  =0
    // 0.1=0
    // 0.5=0
    // 0.9=0
    // 1.0=1

	this->assertInteger(__PRETTY_FUNCTION__);
	return var_int;
}

var var::floor() const {

    //-1.0=-1
    //-0.9=-1
    //-0.5=-1
    //-0.1=-1
    // 0  =0
    // 0.1=0
    // 0.5=0
    // 0.9=0
    // 1.0=1

	this->assertNumeric(__PRETTY_FUNCTION__);

	if (!(var_typ & VARTYP_INT)) {

		var_int = std::floor(var_dbl);

		// Add int flag
		var_typ |= VARTYP_INT;
	}

	return var_int;
}

bool var::toBool() const {

	// could be skipped for speed assuming that people will not write unusual "var x=f(x)" type
	// syntax as follows: var xx=xx?11:22;
	this->assertDefined(__PRETTY_FUNCTION__);

	// identical code in void* and bool except returns void* and bool respectively
	while (true) {
		// ints are true except for zero
		if (var_typ & VARTYP_INT)
			return static_cast<bool>(var_int != 0);

		// non-numeric strings are true unless zero length
		if (var_typ & VARTYP_NAN)
			return !var_str.empty();

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

int64_t var::toInt() const {

	this->assertInteger(__PRETTY_FUNCTION__);

	//return static_cast<int>(var_int);
	return var_int;
}

//long long int var::toLong() const {
//
//	this->assertInteger(__PRETTY_FUNCTION__);
//
//	return static_cast<long long int>(var_int);
//}

double var::toDouble() const {

	this->assertDecimal(__PRETTY_FUNCTION__);

	return var_dbl;
}

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

bool var::isnum(void) const {


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
	int strlen = var_str.size();
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
	for (int ii = 0; ii < strlen; ii++) {
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

	char* first = var_str.data();
	char* last = first + strlen;

	// Skip leading + to be compatible with javascript
	// from_chars does not allow it.
	first += *first == '+';

	if (floating) {

		// to double
		auto [p, ec] = STD_OR_FASTFLOAT::from_chars(first, last, var_dbl);
		if (ec != std::errc() || p != last) {
			var_typ = VARTYP_NANSTR;
			return false;
		}

		// indicate that the var is a string and a double
		var_typ = VARTYP_DBLSTR;

	} else {

		// to long int
		auto [p, ec] = std::from_chars(first, last, var_int);
		if (ec != std::errc() || p != last) {
			var_typ = VARTYP_NANSTR;
			return false;
		}

		// indicate that the var is a string and a long int
		var_typ = VARTYP_INTSTR;
	}

	return true;
}

} // namespace exodus
