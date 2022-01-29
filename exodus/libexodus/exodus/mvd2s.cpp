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

// Use ryu if GNUC < 11 and ryu include available
//ryu            1234.5678 -> "1234.5678" 500ns
//ryu_printf     1234.5678 -> "1234.5678" 800ns
//sstream/printf 1234.5678 -> "1234.5678" 1800ns

// 1. TO_CHARS
#if __GNUC__ >= 11
#define USE_TO_CHARS
#include <charconv>
#include <array>

// 2. RYU
#elif __has_include(<ryu/ryu.h>)
#define USE_RYU
#include <ryu/ryu.h>

// 3. STRINGSTREAM
#else
#include <sstream>
#endif

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

	std::array<char, 24> chars;

	auto [ptr, ec] = std::to_chars(chars.data(), chars.data() + chars.size(), double1);
	if (ec != std::errc())
		throw MVNonNumeric("Cannot convert double1 to 24 characters");

	std::string s = std::string(chars.data(), ptr - chars.data());
	const std::size_t epos = s.find('e');
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
	ss.precision(15);
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

	//////////////////////////////////////////////////
	//leave exponent in if exponent is <= -7 or >= +13
	//////////////////////////////////////////////////
	if (exponent < -6 or exponent > 12) {

		//remove trailing zeros
		while (s.back() == '0')
			s.pop_back();
		if (s.back() == '.')
			s.push_back('0');
			//s.pop_back();

		//append the exponent again
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

#ifndef USE_RYU_D2S
			goto removetrailing;
#endif
		}

		//negative exponent
	} else {

		//remove decimal point
		s.erase(1 + minus, 1);

		//prefix sufficient zeros
		s.insert(0 + minus, -exponent, '0');

		//insert decimal point
		s.insert(1 + minus, 1, '.');

#ifndef USE_RYU_D2S
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
#endif
	}

	return s;
}

} //namespace exodus
