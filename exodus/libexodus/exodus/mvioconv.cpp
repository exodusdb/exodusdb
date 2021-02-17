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

// C4530: C++ exception handler used, but unwind semantics are not enabled.
#pragma warning(disable : 4530)

#include <cmath>
#include <iostream> //for wcout
#include <sstream>  //for conv MX
#include <string.h>

#include <exodus/mv.h>
#include <exodus/mvexceptions.h>

static const int HEX_PER_CHAR = sizeof(char) * 2;
// using namespace std;

namespace exodus
{

var var::iconv(const var& convstr) const
{
	THISIS("var var::iconv(const var& convstr) const")
	ISSTRING(convstr)

	return iconv(convstr.var_str.c_str());
}

/**
converts from external format to internal depending on conversion


@param convstr
A string containing a conversion instruction

@return
The result in internal format

*/
var var::iconv(const char* convstr) const
{
	THISIS("var var::iconv(const char* convstr) const")
	THISISSTRING()

	// empty string in, empty string out
	if (var_typ & VARTYP_STR && var_str.length() == 0)
		return "";

	// REMOVE the remove logic out of the L# R# and T# here

	var part;
	var charn = 1;
	var terminator;
	var output = "";

	const char* conversionchar = convstr;

	// check first character
	switch (*conversionchar)
	{
	// D
	case 'D':

		do
		{

			// very similar subfield remove code for most conversions except TLR which
			// always format "" and [] part=remove(charn, terminator);
			part = this->substr2(charn, terminator);
			// if len(part) or terminator then

			if (part.var_typ & VARTYP_STR && part.var_str.length() == 0)
			{
			}
			else
				output ^= part.iconv_D(convstr);

			if (!terminator)
				break;
			output ^= var().chr(LASTDELIMITERCHARNOPLUS1 - terminator.toInt());
		} while (true);

		return output;
		break;

	// MD, MC, MT, MX
	case 'M':

		// point to 2nd character
		++conversionchar;

		while (true)
		{

			// very similar subfield remove code for most conversions except TLR which
			// always format "" and [] part=remove(charn, terminator);
			part = this->substr2(charn, terminator);
			// if len(part) or terminator then

			// null string
			if (part.var_typ & VARTYP_STR && part.var_str.length() == 0)
			{
			}

			// do convstr on a number
			else
			{

				// check second character
				switch (*conversionchar)
				{

				// MD MC - Decimal places
				case 'D':
				case 'C':

					throw MVError(
					    "iconv MD and MC are not implemented yet");
					//							output
					//^= part.iconv_MD(convstr);
					break;

				// MT
				case 'T':
					// output ^= part.iconv_MT(convstr);
					output ^= part.iconv_MT();
					break;

				// MR - replace iconv is same as oconv!
				case 'R':
					output ^= part.oconv_MR(conversionchar);
					break;

				// MX number to hex (not string to hex)
				case 'X':
					throw MVNotImplemented("iconv('MX')");
					// std::ostringstream ss;
					// ss << std::hex << std::uppercase << part.round().toInt();
					// output ^= ss.str();
					// break;
				}
			}

			if (!terminator)
				break;
			output ^= var().chr(LASTDELIMITERCHARNOPLUS1 - terminator.toInt());
		}

		return output;
		break;

	// iconv L#, R#, T#, C# do nothing
	case 'L':
	case 'R':
	case 'T':
	case 'C':
		// return "";
		return convstr;
		break;

	// HEX
	case 'H':
		// empty string in, empty string out
		if (var_typ & VARTYP_STR && var_str.length() == 0)
			return "";

		// TODO allow high end separators in without conversion (instead of failing as
		// non-hex digits)

		// check 2nd character is E, 3rd character is X and next character is null, or a
		// digit
		if ((*(++conversionchar) == 'E') && (*(++conversionchar) == 'X'))
		{
			// point to one character after HEX
			++conversionchar;

			switch (*conversionchar)
			{
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

		throw MVError("Custom conversions like (" ^ var(convstr) ^
				  ") must be called like a function iconv(input,conversion) not "
				  "like a method, input.iconv(conversion)");
		break;

	// empty convstr string - no conversion
	case '\0':
		return (*this);
	}

	// TODO implement
	// std::wcout<<"iconv "<<convstr<< " not implemented yet "<<std::endl;
	throw MVError("iconv '" ^ var(convstr) ^ "' not implemented yet ");

	return *this;
}

var var::oconv_T(const var& format) const
{

	// expecting only "T#99" with no mask at the moment

	var just = (format.field("#", 1, 1))[1];

	var width2 = format.field("#", 2, 1);

	// leave unconverted if non-numeric width
	if (!width2.isnum())
		return *this;

	//
	int width = width2;

	// get padding character from "L(?)" or space
	char fillchar;
	if (format.var_str.length() >= 4 && format.var_str[1] == '(' && format.var_str[3] == ')')
		fillchar = format.var_str[2];
	else
		fillchar = ' ';

	var output = "";

	var terminator;
	int nwords;
	var charn = 1;
	std::string spacing;

	// process each part between high sep chars
	while (true)
	{

		// extract characters up to the next high separator character
		// var part=remove(charn, terminator);
		var part = this->substr2(charn, terminator);

		if (width)
		{

			// more complex processing folding on spaces
			// part.converter(TM," ");
			part.trimmerf().trimmerb();

			// simple processing if part is less than width
			int partlen = part.length();
			if (partlen <= width)
			{

				// output ^= part;
				// output ^= var(width-partlen).space();
				part.var_str.resize(width, fillchar);
				output ^= part.var_str;

				if (!terminator)
					break;

				output ^= var().chr(LASTDELIMITERCHARNOPLUS1 - terminator.toInt());

				continue;
			}

			nwords = part.count(" ") + 1;

			for (int wordn = 1; wordn <= nwords; wordn++)
			{

				var word = part.field(" ", wordn, 1);

				int wordlen = word.length();

				if (wordn > 1)
				{
					if (!wordlen)
						continue;
					output ^= TM;
				}

				// long words get tm inserted every width characters
				for (int ii = 1; ii <= wordlen; ii += width)
				{
					if (ii > 1)
						output ^= TM;
					output ^= word.substr(ii, width);
				}; // ii;

				int remaining = width - (wordlen % width);

				if (wordlen == 0 or remaining not_eq width)
				{

					if (remaining <= 1)
					{
						if (remaining)
							output ^= fillchar;
					}
					else
					{

						// try to squeeze in following words into the
						// remaining space
						while (remaining > 1 && wordn < nwords)
						{
							var nextword =
							    part.field(" ", wordn + 1, 1);

							int nextwordlen = nextword.length();
							if (nextwordlen + 1 > remaining)
								break;

							wordn += 1;
							output ^= fillchar;
							output ^= nextword;
							remaining -= nextwordlen + 1;
						} // loop;

						// output ^= var(remaining).space();
						spacing.resize(remaining, fillchar);
						output ^= spacing;
					}
				}

			}; // wordn;
		}

		if (!terminator)
			break;

		output ^= var().chr(LASTDELIMITERCHARNOPLUS1 - terminator.toInt());

	} // loop parts

	return output;
}

var var::oconv_MD(const char* conversion) const
{

	// http://www.d3ref.com/index.php?token=basic.masking.function

	// TODO consider implementing nextchar as a pointer to eliminate charn and convlen

	// no conversion in the following cases
	// 1. zero length string
	// 2. non-numeric string
	// 3. plain "MD" conversion without any trailing digits
	size_t convlen = strlen(conversion);
	if (((this->var_typ & VARTYP_STR) && !var_str.length()) || !(this->isnum()) || convlen <= 2)
		return *this;

	// default conversion options
	int ndecimals = -1;
	int movedecs = -1;
	bool dontmovepoint = false;
	bool septhousands = false;
	//bool z_flag = false;
	char trailer = '\0';
	char prefixchar = '\0';

	// get pointer to the third character (after the MD/MC bit)
	size_t charn = 2;

	// get the first (next) character
	char nextchar = conversion[charn];

	// following up to two digits are ndecimals, or ndecimals and movedecimals
	// look for a digit
	if (isdigit(nextchar))
	{

		// first digit is either ndecimals or ndecimals and movedecimals
		ndecimals = nextchar - '0';
		movedecs = ndecimals;

		// are we done
		if (charn >= convlen)
			goto convert;

		// look for a second digit
		charn++;
		nextchar = conversion[charn];
		if (isdigit(nextchar))
		{
			// get movedecimals
			movedecs = nextchar - '0';

			// are we done
			if (charn >= convlen)
				goto convert;

			// move to the next character
			charn++;
			nextchar = conversion[charn];
		}
	}

	while (true)
	{
		switch (nextchar)
		{
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
			//z_flag = true;
			if (!(this->toBool()))
				return "";
			break;

		case 'X':
			//do no conversion
			return  *this;
			break;

		default:
			if (prefixchar == '\0')
				prefixchar = nextchar;
			break;
		}
		// move to next character if any otherwise break
		if (charn >= convlen)
			break;
		charn++;
		nextchar = conversion[charn];
	}

convert:

	var newmv = (*this);

	// move decimals
	if (!dontmovepoint && movedecs)
		newmv = newmv / pow(10.0, movedecs);

	// rounding
	newmv = newmv.round(ndecimals);

	var part1 = newmv.field(".", 1);
	var part2 = newmv.field(".", 2);
	int part2len = part2.length();

	// thousand separators
	if (septhousands)
	{
		int part1len = part1.length();
		if (part1len > 3)
		{
			var thousandsep = (conversion[1] == 'C') ? '.' : ',';
			var minii = part1[1] == "-" ? 2 : 1;
			for (int ii = part1len - 2; ii > minii; ii -= 3)
			{
				part1.splicer(ii, 0, thousandsep);
			}
		}
	}

	// fixed decimals
	if (ndecimals > 0)
	{
		// append decimal point
		part1 ^= (conversion[1] == 'C') ? ',' : '.';

		if (ndecimals == part2len)
			part1 ^= part2;
		else if (ndecimals > part2len)
			part1 ^= part2 ^ std::string(ndecimals - part2len, '0');
		else /*if (ndecimals < part2len)*/
			part1 ^= part2.substr(1,ndecimals);
	}

	// trailing minus, DB or CR or wrap negative with "<...>"
	switch (trailer)
	{
	case '\0':
		break;

	case '<':
		if (part1[1] == "-")
		{
			part1.splicer(1, 1, "<");
			part1 ^= ">";
		}
		break;

	case '-':
		if (part1[1] == "-")
		{
			part1.splicer(1, 1, "");
			part1 ^= "-";
		}
		else
			part1 ^= " ";
		break;

	case 'C':
		if (part1[1] == "-")
		{
			part1.splicer(1, 1, "");
			part1 ^= "CR";
		}
		else
			part1 ^= "DR";
		break;

	case 'D':
		if (part1[1] == "-")
		{
			part1.splicer(1, 1, "");
			part1 ^= "DR";
		}
		else
			part1 ^= "CR";
		break;
	}

	if (prefixchar != '\0')
		part1.splicer(1, 0, prefixchar);

	return part1;
}

var var::oconv_LRC(const var& format) const
{

	// TODO convert to C instead of var for speed
	// and implement full mask options eg L#2-#3-#4 etc

	var varwidth = format.field("#", 2, 1);
	var just = (format.field("#", 1, 1))[1];

	if (!varwidth.isnum())
		return *this;
	int width = varwidth.toInt();

	// get padding character from "L(?)" or space
	char fillchar;
	if (format.var_str.length() >= 4 && format.var_str[1] == '(' && format.var_str[3] == ')')
		fillchar = format.var_str[2];
	else
		fillchar = ' ';

	var output = "";
	var terminator;

	var part;
	int remaining;
	var charn = 1;
	while (true)
	{

		// part=remove(charn, terminator);
		part = this->substr2(charn, terminator);
		// if len(part) or terminator then

		// TODO optimise with pos1() and pos2()
		// or substr "upto" next character (space here)

		if (width)
		{

			remaining = width - part.length();
			if (remaining > 0)
			{
				if (just == "L")
				{
					// output ^= part;
					// output ^= remaining.space();
					part.var_str.resize(width, fillchar);
					output ^= part;
				}
				else if (just == "R")
				{
					// output ^= remaining.space();
					// output ^= part;
					part.var_str.insert(0, remaining, fillchar);
					output ^= part;
				}
				else //"C"
				{
					part.var_str.insert(0, remaining / 2, fillchar);
					output ^= part;
					output.var_str.resize(width, fillchar);
				}
			}
			else
			{
				if (just == "R")
				{
					// take the last n characters
					output ^= part.var_str.substr(part.var_str.length() - width,
								      width);
				}
				else // L or C
				{
					// take the first n characters
					output ^= part.var_str.substr(0, width);
				}
			}
		}

		// BREAK;
		if (!terminator)
			break;
		;

		output ^= var().chr(LASTDELIMITERCHARNOPLUS1 - terminator.toInt());
	} // loop;

	return output;
}

var var::oconv(const var& conversion) const
{
	THISIS("var var::oconv(const var& conversion) const")
	THISISDEFINED()
	ISSTRING(conversion)

	return oconv(conversion.var_str.c_str());
}

// fast version for common programming example where conversion is provided as a hard coded string
// but application programs source code is usually going to usually be "D2" and not "D2" so provide
// a narrow char* version? possibly most oconvs will come from variables (eg read from dicts) so
// will be string format
var var::oconv(const char* conversion) const
{
	THISIS("var var::oconv(const char* conversion) const")
	// TODO this should be THISISASSIGNED since no point converting numbers to strings for many
	// oconvs
	THISISSTRING()

	// REMOVE the remove logic out of the L# R# and T# here

	var part;
	var charn = 1;
	var terminator;
	var output = "";

	const char* conversionchar = conversion;

	// check first character
	switch (*conversionchar)
	{
	// D
	case 'D':

		do
		{

			// very similar subfield remove code for most conversions except TLR which
			// always format "" and [] part=remove(charn, terminator);
			part = this->substr2(charn, terminator);
			// if len(part) or terminator then

			if (part.var_typ & VARTYP_STR && part.var_str.length() == 0)
			{
			}
			else if (!part.isnum())
				output ^= part;
			else
				output ^= part.oconv_D(conversion);

			if (!terminator)
				break;
			output ^= var().chr(LASTDELIMITERCHARNOPLUS1 - terminator.toInt());
		} while (true);

		return output;
		break;

	// MD, MC, MT, MX, ML, MR
	case 'M':

		// point to 2nd character
		++conversionchar;

		while (true)
		{

			// very similar subfield remove code for most conversions except TLR which
			// always format "" and [] part=remove(charn, terminator);
			part = this->substr2(charn, terminator);
			// if len(part) or terminator then

			// null string
			// if (part.var_typ&VARTYP_STR && part.var_str.length()==0)
			//	{}
			bool notemptystring =
			    !(part.var_typ & VARTYP_STR && part.var_str.length() == 0);

			// MR ... character replacement
			if (*conversionchar == 'R')
			{
				if (notemptystring)
					output ^= part.oconv_MR(++conversionchar);
			}

			// non-numeric are left unconverted for MD/MT/MX
			else if (!part.isnum())
				output ^= part;

			// do conversion on a number
			else
			{

				// check second character
				switch (*conversionchar)
				{
				// MD and MC - decimal places
				case 'D':
				case 'C':
					// may treat empty string as zero
					output ^= part.oconv_MD(conversion);
					break;

				// MT - time
				case 'T':
					// point to the remainder of the conversion after the MT
					if (notemptystring)
					{
						output ^= part.oconv_MT(++conversionchar);
					}

					break;

				// MX - number to hex (not string to hex)
				case 'X':
					if (notemptystring)
					{
						std::ostringstream ss;
						ss << std::hex << std::uppercase
						   << part.round().toInt();
						output ^= ss.str();
					}

					break;
				}
			}

			if (!terminator)
				break;
			output ^= var().chr(LASTDELIMITERCHARNOPLUS1 - terminator.toInt());
		}

		return output;
		break;

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
		return oconv_T(conversion);
		break;

	// HEX (unlike arev it converts high separator characters)
	case 'H':

		// empty string in, empty string out
		if (var_typ & VARTYP_STR && var_str.length() == 0)
			return "";

		// check 2nd character is E, 3rd character is X and next character is null, or a
		// digit
		if ((*(++conversionchar) == 'E') && (*(++conversionchar) == 'X'))
		{
			// point to one character after HEX
			++conversionchar;

			switch (*conversionchar)
			{
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
		if (var_typ & VARTYP_STR && var_str.length() == 0)
			return "";
		if (this->toBool())
			return var(conversion).substr(2).field(",", 1);
		else
			return var(conversion).substr(2).field(",", 2);
		break;

	// custom conversion should not be called via ::oconv
	case '[':

		throw MVError("Custom conversions like (" ^ var(conversion) ^
				  ") must be called like a function oconv(input,conversion) not "
				  "like a method, input.oconv(conversion)");
		break;

	// empty conversion string - no conversion
	case '\0':
		return (*this);

	//default:
	//	throw MVError("oconv " ^ var(*conversionchar).oconv("HEX").substr(1,6) ^ " not implemented yet ");
	}

	// TODO implement
	// std::wcout<<"oconv "<<conversion<< " not implemented yet "<<std::endl;
	throw MVError("oconv '" ^ var(conversion).substr(1,6) ^ "' not implemented yet ");

	// unknown conversions are simply ignored in AREV
	return *this;
}

var var::oconv_HEX([[maybe_unused]] const int ioratio) const
{

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
		int nchars=length();
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
	int nchars = this->length();
	for (int charn = 0; charn < nchars; ++charn)
	{
		char const byte = this->var_str[charn];
		result += hex_chars[(byte & 0xF0) >> 4];
		result += hex_chars[(byte & 0x0F) >> 0];
	}

	return result;
}

// use macro to ensure inlined instead of using "inline" function
#define ADD_NYBBLE_OR_FAIL                                                                         \
	nybble = var_str[posn++];                                                                  \
	if (nybble < '0')                                                                          \
		return "";                                                                         \
	if (nybble <= '9')                                                                         \
		nybble -= '0';                                                                     \
	else if (nybble >= 'A' && nybble <= 'F')                                                   \
		nybble -= '7';                                                                     \
	else if (nybble >= 'a' && nybble <= 'f')                                                   \
		nybble -= 'W';                                                                     \
	else                                                                                       \
		return "";                                                                         \
	outchar += nybble;

var var::iconv_HEX(const int ioratio) const
{
	// ioratio
	// 2 hex digits to one char
	// 4 hex digits to one wchar of size 2
	// 8 hex digits to one wchar of size 4

	// empty string in, empty string out
	size_t endposn = var_str.length();
	if (!endposn)
		return "";

	std::string textstr = "";

	size_t posn = 0;

	// work out how many hex digits in first character to cater for shortfall
	// eg 7 hex digits 1234567 with ioratio of 4 would be consumed as 0123 4567
	size_t ratio = endposn % ioratio;
	if (!ratio)
		ratio = ioratio;

	char nybble;
	do
	{
		unsigned int outchar = 0;
		switch (ratio)
		{
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

		// std::wcout<<std::hex<<std::showbase<<outchar<<std::endl;
		textstr += outchar;

		// only really needs to be done after the 1st outchar
		ratio = ioratio;

	} while (posn < endposn);

	return textstr;
}

#undef ADD_NYBBLE_OR_FAIL

} // namespace exodus
