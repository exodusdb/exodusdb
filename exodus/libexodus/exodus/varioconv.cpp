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

#include <math.h> //for pow

#include <cstring>//for strlen
#include <string>
#include <sstream>
#include <bitset>

#include <exodus/varimpl.h>

namespace exodus {

// Would be 256 if RM was character number 255.
// Last delimiter character is 0x1F (RM)
// Used in var::remove()
// #define LASTDELIMITERCHARNOPLUS1 0x20
static constexpr int LASTDELIMITERCHARNOPLUS1 = FM_ + 2;

static constexpr int HEX_PER_CHAR = sizeof(char) * 2;

//var var::iconv(CVR convstr) const {
//
//	THISIS("var var::iconv(CVR convstr) const")
//	ISSTRING(convstr)
//
//	return iconv(convstr.var_str.c_str());
//}

/**
converts from external format to internal depending on conversion


@param convstr
A string containing a conversion instruction

@return
The result in internal format

*/
var var::iconv(const char* convstr) const {

	THISIS("var var::iconv(const char* convstr) const")
	assertString(function_sig);

	// empty string in, empty string out
	if (var_typ & VARTYP_STR && var_str.empty())
		return "";

	// REMOVE the remove logic out of the L# R# and T# here

	var part;
	var charn = 1;
	var terminator;
	var outx = "";

	const char* conversionchar = convstr;

	// check first character
	switch (*conversionchar) {
		// D
		case 'D':

			do {

				// very similar subfield remove code for most conversions except TLR which
				// always format "" and [] part=remove(charn, terminator);
				part = this->substr2(charn, terminator);
				// if len(part) or terminator then

				if (part.var_typ & VARTYP_STR && part.var_str.empty()) {
				} else
					outx ^= part.iconv_D(convstr);

				if (!terminator)
					break;
				outx ^= var().chr(LASTDELIMITERCHARNOPLUS1 - terminator.toInt());
			} while (true);

			return outx;
			break;

		// "MD", "MC", "MT", "MX"
		case 'M':

			// point to 2nd character
			++conversionchar;

			while (true) {

				// very similar subfield remove code for most conversions except TLR which
				// always format "" and [] part=remove(charn, terminator);
				part = this->substr2(charn, terminator);
				// if len(part) or terminator then

				// null string
				if (part.var_typ & VARTYP_STR && part.var_str.empty()) {
				}

				// do convstr on a number
				else {

					// check second character
					switch (*conversionchar) {

						// "MD" "MC" - Decimal places
						case 'D':
						case 'C':

							throw VarError(
								"iconv MD and MC are not implemented yet");
							//							output
							//^= part.iconv_MD(convstr);
							break;

						// "MT"
						case 'T':
							// outx ^= part.iconv_MT(convstr);
							outx ^= part.iconv_MT();
							break;

						// "MR" - replace iconv is same as oconv!
						case 'R':
							outx ^= part.oconv_MR(conversionchar);
							break;

						// "MX" number to hex (not string to hex)
						case 'X':
							throw VarNotImplemented("iconv('MX')");
							// std::ostringstream ss;
							// ss << std::hex << std::uppercase << part.round().toInt();
							// outx ^= ss.str();
							// break;

						// "MB" binary to decimal
						case 'B':

							// var_str is like "101010110" max 64 digits;
							part.trimmerfirst("0");
							if (part.var_str.size() > 64) {
								break;
							}

							try {

								auto int1 = static_cast<varint_t>(std::bitset<64>(part.var_str).to_ullong());

								// Cannot create using ordinary var construction from uint64_t
								// because they throw VarIntOverflow if bit 64 is set.
								var opart;
								opart.var_typ = VARTYP_INT;
								opart.var_int = int1;

								outx ^= opart;
							}
							catch (std::invalid_argument& e) {
								// If anything but 0 and 1 in string
							}

							break;
					}
				}

				if (!terminator)
					break;
				outx ^= var().chr(LASTDELIMITERCHARNOPLUS1 - terminator.toInt());
			}

			return outx;
			break;

		// iconv L#, R#, T#, C# do nothing. TX converts text to record format
		case 'L':
		case 'R':
		case 'T':
			// TX
			// TX1 TX2 TX3 TX4 TX5 for progressive conversion starting with FM
			++conversionchar;
			if (*conversionchar == 'X') {
				++conversionchar;
				return iconv_TX(*conversionchar);
			}
			[[fallthrough]];
		case 'C':
			// return "";
			return convstr;
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
			if ((*(++conversionchar) == 'E') && (*(++conversionchar) == 'X')) {
				// point to one character after HEX
				++conversionchar;

				switch (*conversionchar) {
					case '\0':
						return iconv_HEX(HEX_PER_CHAR);
						break;
					case '2':
						return iconv_HEX(2);
						break;
					case '4':
						return iconv_HEX(4);
						break;
					case '8':
						return iconv_HEX(8);
						break;
				}

				// return oconv_HEX(HEX_IO_RATIO);
				break;
			}

			break;

		// custom io conversions should not be called via ::iconv or ::oconv since they have no
		// access to mv environment required to call external subroutines
		case '[':

			throw VarError("Custom conversions like (" ^ var(convstr) ^
						  ") must be called like a function iconv(input,conversion) not "
						  "like a method, input.iconv(conversion)");
			break;

		// empty convstr string - no conversion
		case '\0':
			return (*this);
	}

	// TODO implement
	throw VarNotImplemented("iconv '" ^ var(convstr) ^ "' not implemented yet ");

	return *this;
}

var var::oconv_T(CVR format) const {

	// expecting only "T#99" with no mask at the moment

	var just = (format.field("#", 1, 1))[1];

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

	var outx = "";

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

				// outx ^= part;
				// outx ^= var(width-partlen).space();
				part.var_str.resize(width, fillchar);
				outx ^= part.var_str;

				if (!terminator)
					break;

				outx ^= var().chr(LASTDELIMITERCHARNOPLUS1 - terminator.toInt());

				continue;
			}

			nwords = part.count(" ") + 1;

			for (std::size_t wordn = 1; wordn <= nwords; wordn++) {

				var word = part.field(" ", static_cast<int>(wordn), 1);

				auto wordlen = word.var_str.size();

				if (wordn > 1) {
					if (!wordlen)
						continue;
					outx ^= TM;
				}

				// long words get tm inserted every width characters
				for (std::size_t ii = 1; ii <= wordlen; ii += width) {
					if (ii > 1)
						outx.var_str.push_back(TM_);
					//outx ^= word.b(ii, width);
					outx.var_str.append(word.var_str, ii - 1, width);
				}	// ii;

				auto remaining = width - (wordlen % width);

				if (wordlen == 0 or remaining not_eq width) {

					if (remaining <= 1) {
						if (remaining)
							outx ^= fillchar;
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
							outx ^= fillchar;
							outx ^= nextword;
							remaining -= nextwordlen + 1;
						}  // loop;

						// outx ^= var(remaining).space();
						spacing.resize(remaining, fillchar);
						outx ^= spacing;
					}
				}

			}	// wordn;
		}

		if (!terminator)
			break;

		outx ^= var().chr(LASTDELIMITERCHARNOPLUS1 - terminator.toInt());

	}  // loop parts

	return outx;
}

var var::oconv_MD(const char* conversion) const {

	// http://www.d3ref.com/index.php?token=basic.masking.function

	// TODO consider implementing nextchar as a pointer to eliminate charn and convlen

	// no conversion in the following cases
	// 1. zero length string
	// 2. non-numeric string
	// 3. plain "MD" conversion without any trailing digits
	size_t convlen = strlen(conversion);
	if (((var_typ & VARTYP_STR) && !var_str.size()) || !(this->isnum()) || convlen <= 2)
		return *this;

	// default conversion options
	auto ndecimals = std::string::npos;
	auto movedecs = std::string::npos;
	bool dontmovepoint = false;
	bool septhousands = false;
	bool z_flag = false;
	char trailer = '\0';
	char prefixchar = '\0';

	// get pointer to the third character (after the MD/MC bit)
	size_t pos = 2;

	// get the first (next) character
	char nextchar = conversion[pos];

	// following up to two digits are ndecimals, or ndecimals and movedecimals
	// look for a digit
	//TODO allow A-I to act like 10 to 19 digits
	if (isdigit(nextchar)) {

		// first digit is either ndecimals or ndecimals and movedecimals
		ndecimals = nextchar - '0';
		movedecs = ndecimals;

		// are we done
		if (pos >= convlen)
			goto convert;

		// look for a second digit
		pos++;
		nextchar = conversion[pos];
		if (isdigit(nextchar)) {
			// get movedecimals
			movedecs = nextchar - '0';

			// are we done
			if (pos >= convlen)
				goto convert;

			// move to the next character
			pos++;
			nextchar = conversion[pos];
		}
	}

	while (true) {
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
		// move to next character if any otherwise break
		if (pos >= convlen)
			break;
		pos++;
		nextchar = conversion[pos];
	}

convert:

	// ndecimals is required for any conversion
	if (ndecimals == std::string::npos)
		return *this;

	var newmv = (*this);

	// move decimals
	if (!dontmovepoint && movedecs != std::string::npos)
		newmv = newmv / pow(10.0, movedecs);

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
	std::string part1;
	std::string part2;
	std::string::size_type decpos = newmv.var_str.find('.');
	if (decpos == std::string::npos) {
		part1 = newmv.var_str;
		part2 = "";
	} else {
		part1 = newmv.var_str.substr(0, decpos);
		part2 = newmv.var_str.substr(decpos + 1);
	}

	auto part2len = part2.size();

	// thousand separators
	if (septhousands) {
		auto part1len = part1.size();
		if (part1len > 3) {
			char thousandsep = (conversion[1] == 'C') ? '.' : ',';
//			std::size_t minii = part1.front() == '-' ? 2 : 1;
			std::size_t minpos = part1.front() == '-' ? 2 : 1;
//			for (std::size_t ii = part1len - 2; ii > minii; ii -= 3) {
//				//part1.paster(ii, 0, thousandsep);
//				part1.insert(ii - 1, 1, thousandsep);
//			}
			{
				auto pos = part1.size() - 2;
				while (pos > minpos) {
					part1.insert(pos - 1, 1, thousandsep);
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
		//part1 ^= (conversion[1] == 'C') ? ',' : '.';
		part1.push_back((conversion[1] == 'C') ? ',' : '.');

		if (ndecimals == part2len)
			//part1 ^= part2;
			part1.append(part2);

		else if (ndecimals > part2len)
			//part1 ^= part2 ^ std::string(ndecimals - part2len, '0');
			part1.append(part2).append(std::string(ndecimals - part2len, '0'));

		else /*if (ndecimals < part2len)*/
			part1.append(part2.substr(0, ndecimals));

	}

	// trailing minus, DB or CR or wrap negative with "<...>"
	switch (trailer) {
		case '\0':
			break;

		case '<':
			if (part1.front() == '-') {
				part1.front() = '<';
				part1.push_back('>');
			}
			break;

		case '-':
			if (part1.front() == '-') {
				part1.erase(0, 1);
				part1.push_back('-');
			} else
				part1.push_back(' ');
			break;

		case 'C':
			if (part1.front() == '-') {
				part1.erase(0,1);
				//CR
				part1.push_back('C');
				part1.push_back('R');
			} else {
				//DR
				part1.push_back(' ');
				part1.push_back(' ');
			}
			break;

		case 'D':
			if (part1.front() == '-') {
				part1.erase(0,1);
				//DR
				part1.push_back('D');
				part1.push_back('B');
			} else {
				//CR
				part1.push_back(' ');
				part1.push_back(' ');
			}
			break;

	}

	if (prefixchar != '\0')
		part1.insert(0, 1, prefixchar);

	return part1;
}

var var::oconv_LRC(CVR format) const {

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

	var outx = "";
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
					// outx ^= part;
					// outx ^= remaining.space();
					part.var_str.resize(width, fillchar);
					outx ^= part;
				} else if (just == "R") {
					// outx ^= remaining.space();
					// outx ^= part;
					part.var_str.insert(0, remaining, fillchar);
					outx ^= part;
				} else	//"C"
				{
					part.var_str.insert(0, remaining / 2, fillchar);
					outx ^= part;
					outx.var_str.resize(width, fillchar);
				}
			} else {
				if (just == "R") {
					// take the last n characters
					outx ^= part.var_str.substr(part.var_str.size() - width,
												  width);
				} else	// L or C
				{
					// take the first n characters
					outx ^= part.var_str.substr(0, width);
				}
			}
		}

		// BREAK;
		if (!terminator)
			break;
		;

		outx ^= var().chr(LASTDELIMITERCHARNOPLUS1 - terminator.toInt());
	}  // loop;

	return outx;
}

//var var::oconv(CVR conversion) const {
//
//	THISIS("var var::oconv(CVR conversion) const")
//	assertDefined(function_sig);
//	ISSTRING(conversion)
//
//	return oconv(conversion.var_str.c_str());
//}

// fast version for common programming example where conversion is provided as a hard coded string
// but application programs source code is usually going to usually be "D2" and not "D2" so provide
// a narrow char* version? possibly most oconvs will come from variables (eg read from dicts) so
// will be string format
var var::oconv(const char* conversion) const {

	THISIS("var var::oconv(const char* conversion) const")
	// TODO this should be THISISASSIGNED since no point converting numbers to strings for many
	// oconvs
	assertString(function_sig);

	// REMOVE the remove logic out of the L# R# and T# here

	var part;
	var charn = 1;
	var terminator;
	var outx = "";

	const char* conversionchar = conversion;

	// check first character
	switch (*conversionchar) {
		// D
		case 'D':

			while (true) {

				// very similar subfield remove code for most conversions except TLR which
				// always format "" and [] part=remove(charn, terminator);
				part = this->substr2(charn, terminator);
				// if len(part) or terminator then

				if (part.var_typ & VARTYP_STR && part.var_str.empty()) {
				} else if (!part.isnum())
					outx ^= part;
				else
					outx ^= part.oconv_D(conversion);

				if (!terminator)
					break;
				outx ^= var().chr(LASTDELIMITERCHARNOPLUS1 - terminator.toInt());
			}

			return outx;
			break;

		// "MD", "MC", "MT", "MX", "ML", "MR"
		case 'M':

			// point to 2nd character
			++conversionchar;

			while (true) {

				// very similar subfield remove code for most conversions except TLR which
				// always format "" and [] part=remove(charn, terminator);
				part = this->substr2(charn, terminator);
				// if len(part) or terminator then

				// null string
				// if (part.var_typ&VARTYP_STR && part.var_str.empty())
				//	{}
				bool notemptystring =
					!(part.var_typ & VARTYP_STR && part.var_str.empty());

				// MR ... character replacement
				if (*conversionchar == 'R') {
					if (notemptystring)
						outx ^= part.oconv_MR(++conversionchar);
				}

				// non-numeric are left unconverted for "MD", "MT", "MX"
				else if (!part.isnum())
					outx ^= part;

				// do conversion on a number
				else {

					// check second character
					switch (*conversionchar) {
						// MD and MC - decimal places
						case 'D':
						case 'C':
							// may treat empty string as zero
							outx ^= part.oconv_MD(conversion);
							break;

						// "MT" - time
						case 'T':
							// point to the remainder of the conversion after the "MT"
							if (notemptystring) {
								outx ^= part.oconv_MT(conversionchar + 1);
							}

							break;

						// MX - number to hex (not string to hex)
						case 'X':
							if (notemptystring) {

								//convert decimal to long
								if (!(part.var_typ & VARTYP_INT)) {
									part = part.round();  //actually to var_int i.e. int64_t
									//part.toLong();
									part.toInt();
								}

								//convert to hex
								std::ostringstream ss;
								ss << std::hex << std::uppercase
								   //   << part.round().toInt();
								   << part.var_int;

								outx ^= ss.str();
							}

							break;

						// MB - number to binary
						case 'B':
							if (notemptystring) {

								//convert decimal to long
								if (!(part.var_typ & VARTYP_INT)) {
									part = part.round();  //actually to var_int i.e. int64_t
									//part.toLong();
									part.toInt();
								}

//								//convert to binary string
//								std::ostringstream ss;
//								ss << std::bitset<64>(part.var_int);
//
//								outx ^= ss.str();
								outx ^= std::bitset<64>(part.var_int).to_string();
							}

							break;
					}
				}

				if (!terminator)
					break;
				outx ^= var().chr(LASTDELIMITERCHARNOPLUS1 - terminator.toInt());
			}

			return outx;
			break;

		//TODO implement masking eg.
		// oconv("1234567890","L(###)###-####") -> "(123)456-7890"

		// L#, R#, C#
		// format even empty strings
		case 'L':
		case 'R':
		case 'C':
			return oconv_LRC(conversion);
			break;

		// T#
		// format even empty strings
		case 'T':
			++conversionchar;
			// TX
			// TX1 TX2 TX3 TX4 TX5 for progressive conversion starting with FM
			if (*conversionchar == 'X') {
				++conversionchar;
				return oconv_TX(*conversionchar);
			}
			else
				return oconv_T(conversion);
			break;

		// HEX (unlike pickos, it converts high separator characters)
		case 'H':

			// empty string in, empty string out
			if (var_typ & VARTYP_STR && var_str.empty())
				return "";

			// check 2nd character is E, 3rd character is X and next character is null, or a
			// digit
			if ((*(++conversionchar) == 'E') && (*(++conversionchar) == 'X')) {
				// point to one character after HEX
				++conversionchar;

				switch (*conversionchar) {
					case '\0':
						return oconv_HEX(HEX_PER_CHAR);
						break;
					case '2':
						return oconv_HEX(2);
						break;
					case '4':
						return oconv_HEX(4);
						break;
					case '8':
						return oconv_HEX(8);
						break;
				}

				// return oconv_HEX(HEX_IO_RATIO);
				break;
			}

			break;

		case 'B':
			// empty string in, empty string out
			if (var_typ & VARTYP_STR && var_str.empty())
				return "";
			if (this->toBool())
				return var(conversion).cut(1).field(",", 1);
			else
				return var(conversion).cut(1).field(",", 2);
			break;

		// custom conversion should not be called via ::oconv
		case '[':

			throw VarError("Custom conversions like (" ^ var(conversion) ^
						  ") must be called like a function oconv(input,conversion) not "
						  "like a method, input.oconv(conversion)");
			break;

		// empty conversion string - no conversion
		case '\0':
			return (*this);

			//default:
			//	throw MVNotImplmented("oconv " ^ var(*conversionchar).oconv("HEX").first(6) ^ " not implemented yet ");
	}

	// TODO implement
	throw VarNotImplemented("oconv '" ^ var(conversion).first(16) ^ "' not implemented yet ");

	// unknown conversions are simply ignored in pickos
	return *this;
}

var var::oconv_TX(const int raw) const {

	var txt = this->var_str;

	// Backslashes before NL are going to be used to indicate VM, SM etc
	// so we have to escape them somehow first
	// for usual case skip if no backslashes are present
	if (txt.var_str.find('\\') != std::string::npos)
		txt.regex_replacer("\\\\([" _FM _VM _SM _TM _ST "])", "{Back_Slash}\\1");

	// "\n" -> "\\n"
	txt.replacer("\\n", "\\\\n");

	// LF -> literal "\n"
	txt.replacer("\n", "\\n");

	// FM -> LF
	txt.converter(_FM, "\n");

	// VM -> \ + LF
	// SM -> \\ + LF
	// TM -> \\\ + LF
	// ST -> \\\\ + LF
	if (not raw) {
		txt.replacer(_VM, "\\\n");
		txt.replacer(_SM, "\\\\\n");
		txt.replacer(_TM, "\\\\\\\n");
		txt.replacer(_ST, "\\\\\\\\\n");
	}

	return txt;
}

var var::iconv_TX(const int raw) const {

	var record = this->var_str;
	// \ + LF -> VM
	// \\ + LF -> SM
	// \\ + LF -> TM
	// \\\ + LF -> ST
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
		record.regex_replacer("{Back_Slash}([" _FM _VM _SM _TM _ST "])", "\\\\" "\\1");

	return record;
}


var var::oconv_HEX([[maybe_unused]] const int ioratio) const {

	// this needs rewritting because we have changed from wstring to string for internal coding
	// decided to output 8 fixed hex digits per character to represent the full range of unicode
	// characters regardless of platform
	// 1. output 8 fixed hex digits per character to represent
	// logic is that this would be consistent and easy to postprocess programmatically and
	// visually and that the inefficiencies are not important since hex isnt a storage format
	// HEX8 and HEX16 codes could be implemented to cater for the alternatives below.
	// Rejected alternatives:
	// 2. convert to utf8 and represent each byte as two hex digits.
	// 3. convert to utf16 and represent each byte pair as four hex digits
	// 4. convert as 1. or 3 depending on platform (word length 4 or 2 chars)

	/*	std::ostringstream ss;
		int nchars=size();
		ss.flags (std::ios::right | std::ios::hex | std::ios::uppercase);
		//ss.setbase(16) useful to set numerically instead of ios::hex
		ss.fill('0');
		//perhaps convert to use iterators especially to allow for variable width characters
	   under utf16. for (int charn=0;charn<nchars;++charn)
		{
			ss.width(ioratio);//must be called every time
			ss << int((*this).var_str[charn]);
		}
		return ss.str();
	*/

	char const hex_chars[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
								'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

	std::string result;
	auto nchars = var_str.size();
	for (std::size_t charn = 0; charn < nchars; ++charn) {
		char const byte = var_str[charn];
		result += hex_chars[(byte & 0xF0) >> 4];
		result += hex_chars[(byte & 0x0F) >> 0];
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

var var::iconv_HEX(const int ioratio) const {
	// ioratio
	// 2 hex digits to one char
	// 4 hex digits to one wchar of size 2
	// 8 hex digits to one wchar of size 4

	// empty string in, empty string out
	size_t endposn = var_str.size();
	if (!endposn)
		return "";

	std::string textstr;

	size_t posn = 0;

	// work out how many hex digits in first character to cater for shortfall
	// eg 7 hex digits 1234567 with ioratio of 4 would be consumed as 0123 4567
	size_t ratio = endposn % ioratio;
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
		}

		textstr += static_cast<char>(outchar);

		// only really needs to be done after the 1st outchar
		ratio = ioratio;

	} while (posn < endposn);

	return textstr;
}

#undef ADD_NYBBLE_OR_FAIL

}  // namespace exodus
