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
#	include <cmath>    //for pow
#	include <algorithm> //for std::replace
#	include <cstring>   //for strlen
#	include <string>
#	include <sstream>
#	include <bitset>
#	include <array>
//#include <utility>   //std::unreachable()
//#include <stdexcept> // for std::invalid_argument
#endif

#include "ASCIIutil.h" // for ASCII_isdigit ASCII_isalpha ASCII_toupper

#include <exodus/var.h>
#include <exodus/varimpl.h>
#include <exodus/rex.h>

namespace exo {

#if defined(__clang__)
#	pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif

// Would be 256 if RM was character number 255.
// Last delimiter character is 0x1F (RM)
// Used in var::remove()
// #define LAST_DELIMITER_CHARNO_PLUS1 0x20
static constexpr int LAST_DELIMITER_CHARNO_PLUS1 = RM_ + 1;

static constexpr int HEX_PER_CHAR = sizeof(char) * 2;

/**
	Converts from external format to internal depending on conversion

	@param conversion A string containing a conversion instruction

	@return The result in internal format
*/
var  var::iconv(const char* conversion) const {

	THISIS("var  var::iconv(const char* conversion) const")
	assertString(function_sig);

	// empty string in, empty string out
	if (var_typ & VARTYP_STR && var_str.empty())
		return "";

	// REMOVE the remove logic out of the L# R# and T# here

	var part;
	var charn = 1;
	var terminator;
	var result = "";

	const char* pconversion = conversion;

	// check first character
	switch (*pconversion) {
		// D
		case 'D':

			do {

				// very similar subfield remove code for most conversions except TLR which
				// always format "" and [] part=remove(charn, terminator);
				part = this->substr2(charn, terminator);
				// if len(part) or terminator then

				if (part.var_typ & VARTYP_STR && part.var_str.empty()) {
				} else
					result ^= part.iconv_D(conversion);

				if (!terminator)
					break;
				result ^= var().chr(LAST_DELIMITER_CHARNO_PLUS1 - terminator.toInt());
			} while (true);

			return result;
			//std::unreachable();
			break;

		// "MD", "MC", "MT", "MX"
		case 'M':

			// point to 2nd character
			pconversion++;

			while (true) {

				// very similar subfield remove code for most conversions except TLR which
				// always format "" and [] part=remove(charn, terminator);
				part = this->substr2(charn, terminator);
				// if len(part) or terminator then

				// null string
				if (part.var_typ & VARTYP_STR && part.var_str.empty()) {
				}

				// do conversion on a number
				else {

					// check second character
					switch (*pconversion) {

						// "MT"
						case 'T':
							// result ^= part.iconv_MT(conversion);
							result ^= part.iconv_MT();
							break;

						// "MR" - replace iconv is same as oconv!
						case 'R':
							//result ^= part.oconv_MR(pconversion);
							result ^= part.oconv_MR(conversion);
							break;

						// "MX" number to hex (not string to hex)
						case 'X': UNLIKELY
							throw VarNotImplemented("iconv('MX')");
							// std::ostringstream ss;
							// ss << std::hex << std::uppercase << part.round().toInt();
							// result ^= ss.str();
							// break;
							//std::unreachable();
							break;

						// "MD" "MC" - Decimal places
						case 'D':
						case 'C': UNLIKELY

							throw VarError(
								"iconv MD and MC are not implemented yet");
							//							output
							//^= part.iconv_MD(conversion);
							//std::unreachable();
							break;

						// "MB" binary to decimal
						case 'B':

							// var_str is like "101010110" max 64 digits;
							part.trimmerfirst("0");
							if (part.var_str.size() > 64) {
								break;
							}

							try {

								auto int1 = static_cast<varint_t>(std::bitset<64>(part.var_str).to_ullong());

								// Cannot create using ordinary var construction from std::uint64_t
								// because they throw VarIntOverflow if bit 64 is set.
								var opart;
								opart.var_typ = VARTYP_INT;
								opart.var_int = int1;

								result ^= opart;
							}
							catch ([[maybe_unused]] std::invalid_argument& e) {
								// If anything but 0 and 1 in string
							}

							break;

						// No conversion unless one of MT, MR, MX, MD, MC, MB"
						default:;
					}
				}

				if (!terminator)
					break;
				result ^= var().chr(LAST_DELIMITER_CHARNO_PLUS1 - terminator.toInt());
			}

			return result;
			//std::unreachable();
			break;

		// iconv L#, R#, T#, C# do nothing. TX converts text to record format
		case 'L':
		case 'R':
		case 'T':
			// TX
			// TX1 TX2 TX3 TX4 TX5 for progressive conversion starting with FM
			pconversion++;
			if (*pconversion == 'X') {
				pconversion++;
				return iconv_TX(conversion);
			}
			[[fallthrough]];
		case 'C':
			// return "";
			return conversion;
			//std::unreachable();
			break;

		// HEX
		case 'H':
			// empty string in, empty string out
			if (var_typ & VARTYP_STR && var_str.empty())
				return "";

			// TODO allow high end separators in without conversion (instead of failing as
			// non-hex digits)

			// check 2nd character is E, 3rd character is X and next character is null, or a
			// digit
			if ((*(++pconversion) == 'E') && (*(++pconversion) == 'X')) {
				// point to one character after HEX
				pconversion++;

				switch (*pconversion) {
					case '\0':
						return iconv_HEX(HEX_PER_CHAR);
						//std::unreachable();
						break;
					case '2':
						return iconv_HEX(2);
						//std::unreachable();
						break;
					case '4':
						return iconv_HEX(4);
						//std::unreachable();
						break;
					case '8':
						return iconv_HEX(8);
						//std::unreachable();
						break;
					// No conversion if not one of HEX, HEX2, HEX4, HEX8
					default:;
				}

				// return oconv_HEX(HEX_IO_RATIO);
				break;
			}

			break;

		// custom io conversions should not be called via ::iconv or ::oconv since they have no
		// access to mv environment required to call external subroutines
		case '[': UNLIKELY

			throw VarError("Custom conversions like (" ^ var(conversion) ^
						  ") must be called like a function iconv(input,conversion) not "
						  "like a method, input.iconv(conversion)");
			break;

		// empty conversion string - no conversion
		case '\0':
			return (*this);

		// invalid conversion
		default:;
	}

	// TODO implement
	throw VarNotImplemented("iconv '" ^ var(conversion) ^ "' not implemented yet ");

	//std::unreachable();
	//return *this;
}

// ICONV_MT
///////////

//inline static bool varioconv_is_digit(char c) {
//	return (c & 0b1000'0000) == 0 and c >= '0' and c <= '9';
//}

int extract_first_digits_as_int(std::string::iterator& iter, std::string::iterator end) {

    while (iter != end) {
//        if (varioconv_is_digit(*iter)) {
        if (ASCII_isdigit(*iter)) {
            int num = 0;
//            while (iter != end && varioconv_is_digit(*iter)) {
            while (iter != end && ASCII_isdigit(*iter)) {
                num = num * 10 + (*iter - '0');
                ++iter;
            }
			return num;
        } else {
            ++iter;
        }
    }
	// If not found;
    return 0;
}

var  var::iconv_MT() const {

	THISIS("var  var::iconv_MT() const")
	assertString(function_sig);
//	assertString("var  var::iconv_MT() const");

	// Get the first three groups of digits "...99...99...99..."
	// regardless of all other leading, inner or trailing characters
	auto iter = var_str.begin();
	auto end = var_str.end();
	int hours = extract_first_digits_as_int(iter, end);
	int mins  = extract_first_digits_as_int(iter, end);
	int secs  = extract_first_digits_as_int(iter, end);

	int inttime = hours * 3600 + mins * 60 + secs;

	if (inttime >= 86400)
		return "";

	// TODO
	// 15:00PM should perhaps be invalid
	// 15:00AM should perhaps be invalid

	// PM
	if (inttime < 43200 && (*this).contains("P"))
		inttime += 43200;
	// AM
	else if (inttime >= 43200 && (*this).contains("A"))
		inttime -= 43200;

	return inttime;
}

std::string var::oconv_T(in format) const {

	THISIS("str  var::oconv_T(in format) const")

	// expecting only "T#99" with no mask at the moment

	// TODO to pure c++ for speed

	//var just = (format.field("#", 1, 1))[1];

	var width2 = format.field("#", 2, 1);

	// leave unconverted if non-numeric width
	if (!width2.isnum())
		return *this;

	//
	std::size_t width = width2;

	// get padding character from "L(?)" or space
	char fillchar;
	if (format.var_str.size() >= 4 && format.var_str[1] == '(' && format.var_str[3] == ')')
		fillchar = format.var_str[2];
	else
		fillchar = ' ';

	std::string result = "";

	var terminator;
	std::size_t nwords;
	var charn = 1;
	std::string spacing;

	// process each part between high sep chars
	while (true) {

		// extract characters up to the next high separator character
		// var part=remove(charn, terminator);
		var part = this->substr2(charn, terminator);

		if (width) {

			// more complex processing folding on spaces
			// part.converter(TM," ");
			part.trimmerboth();

			// simple processing if part is less than width
			auto partlen = part.var_str.size();
			if (partlen <= width) {

				// result ^= part;
				// result ^= var(width-partlen).space();
				part.var_str.resize(width, fillchar);
				result += part.var_str;

				if (!terminator)
					break;

				result.push_back(char(LAST_DELIMITER_CHARNO_PLUS1 - terminator.toInt()));

				continue;
			}

			nwords = part.count(" ") + 1;

			for (std::size_t wordn = 1; wordn <= nwords; wordn++) {

				var word = part.field(" ", static_cast<int>(wordn), 1);

				auto wordlen = word.var_str.size();

				if (wordn > 1) {
					if (!wordlen)
						continue;
					result.push_back(TM_);
				}

				// long words get tm inserted every width characters
				for (std::size_t ii = 1; ii <= wordlen; ii += width) {
					if (ii > 1)
						result.push_back(TM_);
					//result ^= word.b(ii, width);
					result.append(word.var_str, ii - 1, width);
				}	// ii;

				auto remaining = width - (wordlen % width);

				if (wordlen == 0 or remaining not_eq width) {

					if (remaining <= 1) {
						if (remaining)
							result.push_back(fillchar);
					} else {

						// try to squeeze in following words into the
						// remaining space
						while (remaining > 1 && wordn < nwords) {
							var nextword =
								part.field(" ", static_cast<int>(wordn) + 1, 1);

							auto nextwordlen = nextword.var_str.size();
							if (nextwordlen + 1 > remaining)
								break;

							wordn += 1;
							result.push_back(fillchar);
							result += nextword.var_str;
							remaining -= nextwordlen + 1;
						}  // loop;

						// result ^= var(remaining).space();
						spacing.resize(remaining, fillchar);
						result += spacing;
					}
				}

			}	// wordn;
		}

		if (!terminator)
			break;

		result.push_back(char(LAST_DELIMITER_CHARNO_PLUS1 - terminator.toInt()));

	}  // loop parts

	return result;
}

std::string var::oconv_MD(const char* conversion) const {

	THISIS("str  var::oconv_MD(const char* conversion) const")

	// http://www.d3ref.com/index.php?token=basic.masking.function

	// 1. Analyse conversion

	// get pointer to the third character (after the MD/MC bit)
	const char* pconversion = conversion;

	// Second letter must be D or C
	pconversion++;
	char thousandsep;
	char decimalsep;
	if (*pconversion == 'D') LIKELY {
		thousandsep = ',';
		decimalsep = '.';
	} else {
		thousandsep = '.';
		decimalsep = ',';
	}

	// get the third character
	pconversion++;
	char nextchar = *pconversion;

	// no conversion in the following cases
	// 1. zero length string
	// 2. non-numeric string
	// 3. plain "MD" conversion without any trailing digits
	//std::size_t convlen = strlen(conversion);
	if (((var_typ & VARTYP_STR) && !var_str.size()) || !(this->isnum()) || !nextchar)
		return *this;

	// default conversion options
	auto ndecimals = std::string::npos;
	auto movedecs = std::string::npos;
	bool dontmovepoint = false;
	bool septhousands = false;
	bool z_flag = false;
	char trailer = '\0';
	char prefixchar = '\0';

	// following up to two digits are ndecimals, or ndecimals and movedecimals
	// look for a digit
	//TODO allow A-I to act like 10 to 19 digits
	if (ASCII_isdigit(nextchar)) {

		// first digit is either ndecimals or ndecimals and movedecimals
		ndecimals = nextchar - '0';
		movedecs = ndecimals;

//		// are we done
//		if (pos >= convlen)
//			goto convert;

		// look for a second digit
		pconversion++;
		nextchar = *pconversion;
		if (ASCII_isdigit(nextchar)) {
			// get movedecimals
			movedecs = nextchar - '0';

//			// are we done
//			if (pos >= convlen)
//				goto convert;

			// move to the next character
			pconversion++;
			nextchar = *pconversion;
		}
	}

	while (nextchar) {

		switch (nextchar) {
			case 'P':
				dontmovepoint = true;
				break;

			case '.':
			case ',':
				septhousands = true;
				break;

			case 'D':
				trailer = 'D';
				break;

			case 'C':
				trailer = 'C';
				break;

			case '-':
				trailer = '-';
				break;

			case '<':
				trailer = '<';
				break;

			case 'Z':
				//Z means return empty string in the case of zero
				z_flag = true;
				//if (!(this->toBool()))
				//	return "";
				break;

			case 'X':
				//do no conversion
				return *this;
				//std::unreachable();
				break;
			default:
				if (prefixchar == '\0') {
					//MD140P conversion ie 0 for prefix, is invalid conversion
					if (nextchar == '0')
						return *this;
					prefixchar = nextchar;
				}
				break;
		}
		pconversion++;
		nextchar = *pconversion;
	}

	// 2. Create output

	// ndecimals is required for any conversion
	if (ndecimals == std::string::npos)
		return *this;

	var newmv = (*this);

	// move decimals
	if (!dontmovepoint && movedecs != std::string::npos)
		newmv = newmv / std::pow(10.0, movedecs);

	// rounding
	//if (ndecimals != std::string::npos) {
	newmv = newmv.round(static_cast<int>(ndecimals));
	if (!(newmv.var_typ & VARTYP_STR))
		newmv.createString();
	//}

	// Option to suppress zeros - if no digits 1-9
	if (z_flag and newmv.var_str.find_first_of("123456789") == std::string::npos) {
		newmv.var_str.clear();
		return newmv;
	}

	//var part1 = newmv.field(".", 1);
	//var part2 = newmv.field(".", 2);
	std::string strpart1;
	std::string strpart2;
	std::size_t decpos = newmv.var_str.find('.');
	if (decpos == std::string::npos) {
		strpart1 = newmv.var_str;
		strpart2 = "";
	} else {
		strpart1 = newmv.var_str.substr(0, decpos);
		strpart2 = newmv.var_str.substr(decpos + 1);
	}

	auto strpart2len = strpart2.size();

	// thousand separators
	if (septhousands) {
		auto strpart1len = strpart1.size();
		if (strpart1len > 3) {
//			std::size_t minii = strpart1.front() == '-' ? 2 : 1;
			std::size_t minpos = strpart1.front() == '-' ? 2 : 1;
//			for (std::size_t ii = strpart1len - 2; ii > minii; ii -= 3) {
//				//strpart1.paster(ii, 0, thousandsep);
//				strpart1.insert(ii - 1, 1, thousandsep);
//			}
			{
				auto pos = strpart1.size() - 2;
				while (pos > minpos) {
					strpart1.insert(pos - 1, 1, thousandsep);
					// Be careful not to allow pos < 0 since it is unsigned
					if (pos < 4)
						break;
					pos -= 3;
				}
			}
		}
	}

	// fixed decimals
	//if (ndecimals > 0) {
	if (ndecimals != 0) {
		// append decimal point
		//strpart1 ^= (conversion[1] == 'C') ? ',' : '.';
		strpart1.push_back(decimalsep);

		if (ndecimals == strpart2len)
			//strpart1 ^= strpart2;
			strpart1.append(strpart2);

		else if (ndecimals > strpart2len)
			//strpart1 ^= strpart2 ^ std::string(ndecimals - strpart2len, '0');
			strpart1.append(strpart2).append(std::string(ndecimals - strpart2len, '0'));

		else /*if (ndecimals < strpart2len)*/
			strpart1.append(strpart2.substr(0, ndecimals));

	}

	// trailing minus, DB or CR or wrap negative with "<...>"
	switch (trailer) {
		case '\0':
			break;

		case '<':
			if (strpart1.front() == '-') {
				strpart1.front() = '<';
				strpart1.push_back('>');
			}
			break;

		case '-':
			if (strpart1.front() == '-') {
				strpart1.erase(0, 1);
				strpart1.push_back('-');
			} else
				strpart1.push_back(' ');
			break;

		case 'C':
			if (strpart1.front() == '-') {
				strpart1.erase(0,1);
				//CR
				strpart1.push_back('C');
				strpart1.push_back('R');
			} else {
				//DR
				strpart1.push_back(' ');
				strpart1.push_back(' ');
			}
			break;

		case 'D':
			if (strpart1.front() == '-') {
				strpart1.erase(0,1);
				//DR
				strpart1.push_back('D');
				strpart1.push_back('B');
			} else {
				//CR
				strpart1.push_back(' ');
				strpart1.push_back(' ');
			}
			break;

		// No trailer if not one of <, -, C or D
		default:;
	}

	if (prefixchar != '\0')
		strpart1.insert(0, 1, prefixchar);

	return strpart1;
}

std::string var::oconv_LRC(in format) const {

	THISIS("str  var::oconv_LRC(in format) const")

	// TODO convert to C instead of var for speed
	// and implement full mask options eg L#2-#3-#4 etc

	var varwidth = format.field("#", 2, 1);
	var just = (format.field("#", 1, 1)).first();

	if (!varwidth.isnum() or varwidth < 0)
		return *this;
	std::size_t width = varwidth.toInt();

	// get padding character from "L(?)" or space
	char fillchar;
	if (format.var_str.size() >= 4 && format.var_str[1] == '(' && format.var_str[3] == ')')
		fillchar = format.var_str[2];
	else
		fillchar = ' ';

	std::string result = "";
	var terminator;

	var part;
	std::size_t remaining;
	var charn = 1;
	while (true) {

		// part=remove(charn, terminator);
		part = this->substr2(charn, terminator);
		// if len(part) or terminator then

		// TODO optimise with pos1() and pos2()
		// or substr "upto" next character (space here)

		if (width) {

			//if (remaining > 0) {
			if (width > part.var_str.size()) {
				remaining = width - part.var_str.size();
				if (just == "L") {
					// result ^= part;
					// result ^= remaining.space();
					part.var_str.resize(width, fillchar);
					result += part.var_str;
				} else if (just == "R") {
					// result ^= remaining.space();
					// result ^= part;
					part.var_str.insert(0, remaining, fillchar);
					result += part.var_str;
				} else	//"C"
				{
					part.var_str.insert(0, remaining / 2, fillchar);
					result += part.var_str;
					result.resize(width, fillchar);
				}
			} else {
				if (just == "R") {
					// take the last n characters
					result += part.var_str.substr(part.var_str.size() - width, width);
				} else	// L or C
				{
					// take the first n characters
					result += part.var_str.substr(0, width);
				}
			}
		}

		// BREAK;
		if (!terminator)
			break;

		result.push_back(char(LAST_DELIMITER_CHARNO_PLUS1 - terminator.toInt()));
	}  // loop;

	return result;
}

/* partial conversion to multibyte/grapheme formatting
   but only padding, not truncating
std::string var::oconv_LRC(in format) const {

	THISIS("str  var::oconv_LRC(in format) const")

	// TODO convert to C instead of var for speed
	// and implement full mask options eg L#2-#3-#4 etc

	var varwidth = format.field("#", 2, 1);
	var just = (format.field("#", 1, 1))[1];

	if (!varwidth.isnum() or varwidth < 0)
		return *this;
	std::size_t width = varwidth.toInt();

	// get padding character from "L(?)" or space
	char fillchar;
	if (format.var_str.size() >= 4 && format.var_str[1] == '(' && format.var_str[3] == ')')
		fillchar = format.var_str[2];
	else
		fillchar = ' ';

	std::string result = "";
	var terminator;

	var part;
	std::size_t remaining;
	var charn = 1;
	while (true) {

		// part=remove(charn, terminator);
		part = this->substr2(charn, terminator);
		// if len(part) or terminator then

		// TODO optimise with pos1() and pos2()
		// or substr "upto" next character (space here)

		if (width) {

			//if (remaining > 0) {
			//if (width > part.var_str.size()) {
			//	remaining = width - part.var_str.size();
			auto part_textwidth = part.textwidth();
			if (width > part_textwidth) {

				// Padding

				remaining = width - part_textwidth.toInt();
				if (just == "L") {
					result += part.var_str;
					result.append(remaining, fillchar);
				} else if (just == "R") {
					part.var_str.insert(0, remaining, fillchar);
					result += part.var_str;
				} else	//"C"
				{
					std::size_t nprefix = remaining / 2;
					part.var_str.insert(0, nprefix, fillchar);
					result += part.var_str;
					result.append(remaining - nprefix, fillchar);
				}
			} else {

				// Truncating

				if (just == "R") {
					// take the last n characters
					result += part.var_str.substr(part.var_str.size() - width, width);
				} else	// L or C
				{
					// take the first n characters
					result += part.var_str.substr(0, width);
				}
			}
		}

		// BREAK;
		if (!terminator)
			break;

		result.push_back(char(LAST_DELIMITER_CHARNO_PLUS1 - terminator.toInt()));
	}  // loop;

	return result;
}
*/

var  var::oconv(const char* conversion_in) const {

	THISIS("var  var::oconv(const char* conversion) const")
	assertAssigned(function_sig);

	// TODO provide a version that converts numeric vars directly to dates and times
	// without going via string

	//var part;
	var charn = 1;
	var terminator = 0;
	std::string outstr = "";

	// Convert any embedded VMs or '|' chars to \0 so that multiple conversions all look like cstr
	std::string all_conversions = conversion_in;
	std::replace(all_conversions.begin(), all_conversions.end(), VM_, '\0');
	std::replace(all_conversions.begin(), all_conversions.end(), '|', '\0');

	// Get the end of all conversions
	auto all_conversions_end = all_conversions.data() + all_conversions.size();

	// Multiple fields/values of input data
	// ------------------------------------
	while (true) {

		// NB HEX and T/TX conversions use the WHOLE input not just PART

		// If no number available then extract a field up to a field mark "terminator"
		// where FM = 5 VM = 4 etc. or 0 if none
		//
		// In case the input is numeric then avoid converting to a string
		// only to have to convert it back again for some conversions
		// e.g. oconv D converts numeric dates directly to text dates
		//
		//var part = this->substr2(charn, terminator);
		var part = (this->var_typ & VARTYP_INTDBL) ?
			this->clone()
		:
			this->substr2(charn, terminator)
		;

		// Start a the beginning
		const char* pconversion = all_conversions.data();

		// Multiple conversions
		// --------------------
		while (true) {

			// Save a pointer to the beginning of the conversion code
			const char* conversion = pconversion;

			// Find the end of conversion code (char \0)
			const char* conversion_end = conversion;
			while (*conversion_end != '\0') {
				 conversion_end++;
			}

			// Check 1st character of conversion code
			switch (*pconversion) {

				// D: Dates

				case 'D':

					if (part.var_typ & VARTYP_STR && part.var_str.empty()) {
						// Empty string returns empty string date

					} else if (!part.isnum()) {
						// Non-numeric dates are simply left untouched

					} else {
						// Note that D doesnt always return a date.
						// e.g. "DQ" returns 1-4 for quarter
						part = part.oconv_D(conversion);
					}
					break;

				// "MD", "MC", "MT", "MX", "MB", "MR"

				case 'M':

						pconversion++;

						// MR:

						if (*pconversion == 'R') {
							if (part.var_typ & VARTYP_STR && part.var_str.empty()) {
								// Empty string returns empty string

							} else {
								// Various character replacements
								part = part.oconv_MR(conversion).var_str;
							}
						}

						// Non-numeric are left unconverted for "MD", "MT", "MX"
						else if (!part.isnum()) {
						}

						// Various conversions on numbers
						else {

							// Check character after initial M
							switch (*pconversion) {

								// MD and MC - decimal places

								case 'D':
								case 'C':
									// Might treat empty string as zero (with Z option in conversion)
									part = part.oconv_MD(conversion);
									break;

								// "MT" - time e.g. 1 -> 1/1/1967

								case 'T':
									if (part.var_typ & VARTYP_STR && part.var_str.empty()) {
										// Empty string returns empty string
									} else {
										part = part.oconv_MT(conversion);
									}
									break;

								// MX - number to hexadecimal (not string to hex) e.g. 15 -> 0F

								case 'X':
									if (part.var_typ & VARTYP_STR && part.var_str.empty()) {
										// Empty string returns empty string

									} else {
										// Convert decimal to long integer
										if (!(part.var_typ & VARTYP_INT)) {
											part = part.round();  //actually to var_int i.e. int64_t
											part.toInt();
										}

										// Convert integer to hexadecimal
										std::ostringstream ss;
										ss << std::hex << std::uppercase
										   //   << part.round().toInt();
										   << part.var_int;

										part = ss.str();
									}
									break;

								// MB - number to ASCII binary eg 15 -> 1111

								case 'B':
									if (part.var_typ & VARTYP_STR && part.var_str.empty()) {
										// Empty string returns empty string

									} else {
										// Convert decimal to long
										if (!(part.var_typ & VARTYP_INT)) {
											part = part.round();  //actually to var_int i.e. int64_t
											//part.toLong();
											part.toInt();
										}

										// Convert 64 bits to string of up to 64 chars '1' or '0'
										part = std::bitset<64>(part.var_int).to_string();
									}

									break;

								// No conversion if not not one of MD, MC, MT, MX or MB
								default:;
							}
						}

					break;

				//TODO implement masking eg.
				// oconv("1234567890","L(###)###-####") -> "(123)456-7890"

				// L#, R#, C#

				case 'L':
				case 'R':
				case 'C':
					// Format even empty strings
					part = part.oconv_LRC(conversion);
					break;

				// T#... TX TXR

				case 'T':

					// NOTE: Using WHOLE input, not just PART
					terminator = 0;

					// Format even empty strings

					pconversion++;
					if (*pconversion == 'X') {
						part = this->oconv_TX(conversion);
					}
					// T
					else {
						part = this->oconv_T(conversion);
					}
					break;

				// HEX, HEX2, HEX4, HEX8

				case 'H':

					// check 2nd character is E, 3rd character is X and next character is null
					// or a digit
					if ((*(++pconversion)) == 'E' && (*(++pconversion) == 'X')) {

						// NOTE: Using WHOLE input, not just PART
						// HEX will use the whole input regardless of FM characters

						terminator = 0;

						if (this->var_typ & VARTYP_STR) {
							if (this->var_str.empty()) {
								// Empty string in, empty string out
								part = "";
								break;
							}
						} else {
							// HEX represents strings so create var_str from int or double
							ISSTRING(*this)
						}

						// The first char after "HEX" determines the width of hex codes
						pconversion++;
						switch (*pconversion) {
							case '\0':
								part = this->oconv_HEX(HEX_PER_CHAR);
								break;
							case '2':
								part = this->oconv_HEX(2);
								break;
							case '4':
								part = this->oconv_HEX(4);
								break;
							case '8':
								part = this->oconv_HEX(8);
								break;
							default:
								throw VarNotImplemented("oconv " ^ var(conversion).first(64).quote() ^ " not implemented yet ");
						}

					} else {
						throw VarNotImplemented("oconv " ^ var(conversion).first(64).quote() ^ " not implemented yet ");
					}

					break;

				// B:

				case 'B':

					if (part.var_typ & VARTYP_STR && part.var_str.empty()) {
						// Empty string in, empty string out

					} else {
						// From something like "BYes,No", extract field 1 (bool=1) or 2 (bool=0) after skipping the B
						part = var(conversion + 1).field(",", 2 - part.toBool()).var_str;
					}

					break;

				// Custom conversion should not be called via ::oconv ATM
				// TODO implement via thread_local array of registered exoprog functions
				case '[':

					throw VarError("Custom conversions like (" ^ var(pconversion - 1) ^
								  ") must be called like a function oconv(input,conversion) not "
								  "like a method, input.oconv(conversion)");
					break;

				// Empty conversion string - no conversion
				case '\0':
					return (*this);
					//break;

				default:
					throw VarNotImplemented("oconv " ^ var(conversion).first(64).quote() ^ " not implemented yet ");

			} // switch depending on first char of conversion

			// Quit conversion loop if there any no more conversions
			pconversion = conversion_end;
			if (pconversion == all_conversions_end)
				break;

			// Point to the start of the next conversion
			pconversion++;

		} // while (true) next conversion

		outstr += part.var_str;

		// Quit loop if no more input data parts
		if (!terminator)
			break;

		// Append a delimiter char and go back around to convert the next part
		outstr += char(LAST_DELIMITER_CHARNO_PLUS1 - terminator);

	} // while (true) next part

	return outstr;

}

std::string var::oconv_TX(const char* conversion) const {

	THISIS("str  var::oconv_TX(const char* conversion) const")

	var result = this->var_str;

	// Backslashes before NL are going to be used to indicate VM, SM etc
	// so we have to escape them somehow first
	// for usual case skip if no backslashes are present
	if (result.var_str.find('\\') != std::string::npos)
		//result.regex_replacer("\\\\([" _FM _VM _SM _TM _ST "])", "{Back_Slash}\\1");
		result.replacer("\\\\([" _FM _VM _SM _TM _ST "])"_rex, "{Back_Slash}\\1");

	// "\n" -> "\\n"
	result.replacer("\\n", "\\\\n");

	// LF -> literal "\n"
	result.replacer("\n", "\\n");

	// FM -> LF
	result.converter(_FM, "\n");

	// VM -> \ + LF
	// SM -> \\ + LF
	// TM -> \\\ + LF
	// ST -> \\\\ + LF
	const char raw = conversion[2];
	if (not raw) {
		result.replacer(_VM, "\\\n");
		result.replacer(_SM, "\\\\\n");
		result.replacer(_TM, "\\\\\\\n");
		result.replacer(_ST, "\\\\\\\\\n");
	}

	return result.var_str;
}

var  var::iconv_TX(const char* conversion) const {

	THISIS("var  var::iconv_TX(const char* conversion) const")

	var record = this->var_str;
	// \ + LF -> VM
	// \\ + LF -> SM
	// \\ + LF -> TM
	// \\\ + LF -> ST
	const char raw = conversion[2];
	if (not raw) {
		record.replacer("\\\\\\\\\n", _ST);
		record.replacer("\\\\\\\n", _TM);
		record.replacer("\\\\\n", _SM);
		record.replacer("\\\n", _VM);
	}

	// LF -> FM
	record.converter("\n", _FM);

	// "\n" -> LF
	record.replacer("\\n", "\n");

	// "\\n" -> "\n"
	record.replacer("\\\n", "\\n");

	// unescape backslash before FM
	// so we have to escape them somehow first
	// for usual case skip if no { is present
	if (record.var_str.find('{') != std::string::npos)
		//record.regex_replacer("{Back_Slash}([" _FM _VM _SM _TM _ST "])", "\\\\" "\\1");
		record.replacer("{Back_Slash}([" _FM _VM _SM _TM _ST "])"_rex, "\\\\" "\\1");

	return record;
}

	CONSTINIT_OR_CONSTEXPR
	std::array hex_digits {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

std::string var::oconv_HEX(const int /*ioratio*/) const {

	THISIS("str  var::oconv_HEX(const int) const")

	std::string result;
	for (auto byte : var_str) {
		result.push_back(hex_digits[(byte & 0xF0) >> 4]);
		result.push_back(hex_digits[(byte & 0x0F) >> 0]);
	}

	return result;
}

// use macro to ensure inlined instead of using "inline" function
#define ADD_NYBBLE_OR_FAIL                       \
	nybble = var_str[posn++];                    \
	if (nybble < '0')                            \
		return "";                               \
	if (nybble <= '9')                           \
		nybble = static_cast<char>(nybble - '0');\
	else if (nybble >= 'A' && nybble <= 'F')     \
		nybble = static_cast<char>(nybble - '7');\
	else if (nybble >= 'a' && nybble <= 'f')     \
		nybble = static_cast<char>(nybble - 'W');\
	else                                         \
		return "";                               \
	outchar += nybble;

var  var::iconv_HEX(const int ioratio) const {

	THISIS("var  var::iconv_HEX(const int ioratio) const")

	// ioratio
	// 2 hex digits to one char
	// 4 hex digits to one wchar of size 2
	// 8 hex digits to one wchar of size 4

	// empty string in, empty string out
	std::size_t endposn = var_str.size();
	if (!endposn)
		return "";

	std::string textstr;

	std::size_t posn = 0;

	// work out how many hex digits in first character to cater for shortfall
	// eg 7 hex digits 1234567 with ioratio of 4 would be consumed as 0123 4567
	std::size_t ratio = endposn % ioratio;
	if (!ratio)
		ratio = ioratio;

	char nybble;
	do {
		unsigned int outchar = 0;
		switch (ratio) {
			// really only need as many cases as input/output ratio
			// this is "loop unrolling
			case 8:
				ADD_NYBBLE_OR_FAIL
				outchar <<= 4;
				[[fallthrough]];
			case 7:
				ADD_NYBBLE_OR_FAIL
				outchar <<= 4;
				[[fallthrough]];
			case 6:
				ADD_NYBBLE_OR_FAIL
				outchar <<= 4;
				[[fallthrough]];
			case 5:
				ADD_NYBBLE_OR_FAIL
				outchar <<= 4;
				[[fallthrough]];
			case 4:
				ADD_NYBBLE_OR_FAIL
				outchar <<= 4;
				[[fallthrough]];
			case 3:
				ADD_NYBBLE_OR_FAIL
				outchar <<= 4;
				[[fallthrough]];
			case 2:
				ADD_NYBBLE_OR_FAIL
				outchar <<= 4;
				[[fallthrough]];
			case 1:
				ADD_NYBBLE_OR_FAIL
				// no shift on last nybble in since it is loaded into the right (right) four
				// bits outchar<<=4;
				[[fallthrough]];

			// Evade compiler warning
			default:;
		}

		textstr += static_cast<char>(outchar);

		// only really needs to be done after the 1st outchar
		ratio = ioratio;

	} while (posn < endposn);

	return textstr;
}

#undef ADD_NYBBLE_OR_FAIL

}  // namespace exo
