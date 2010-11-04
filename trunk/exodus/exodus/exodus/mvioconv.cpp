/*
Copyright (c) 2009 Stephen John Bush

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

#define MV_NO_NARROW

//C4530: C++ exception handler used, but unwind semantics are not enabled. 
#pragma warning (disable: 4530)

#include <iostream> //for wcout
#include <sstream> //for conv MX
#include <cmath>

#include <exodus/mvimpl.h>
#include <exodus/mv.h>
#include <exodus/mvexceptions.h>

static const int HEX_PER_WCHAR=sizeof(wchar_t)*2;
using namespace std;

namespace exodus
{

var var::iconv(const var& convstr) const
{
	THISIS(L"var var::iconv(const var& convstr) const")
	ISSTRING(convstr)

	return iconv(convstr.towstring().c_str());
}

/**
converts from external format to internal depending on conversion
 
@param convstr
A string containing a conversion instruction

@return
The result in internal format

*/
var var::iconv(const wchar_t* convstr) const
{
	THISIS(L"var var::iconv(const wchar_t* convstr) const")
	THISISSTRING()

	//empty string in, empty string out
	if (var_mvtyp&pimpl::MVTYPE_STR && var_mvstr.length()==0)
		return L"";

	if (convstr==L"MT")
	{
		var hours=field(L":",1);
		if (!hours.isnum()) return *this;
		var mins=field(L":",2);
		if (!mins.isnum()) return *this;
		var secs=field(L":",3);
		if (!secs.isnum()) return *this;

		return hours*3600+mins*60+secs;

	}

	//REMOVE the remove logic out of the L# R# and T# here

	var part;
	var charn = 1;
	var terminator;
	var output = L"";

	const wchar_t* conversionchar=convstr;

	//check first character
	switch (*conversionchar)
	{
		//D
		case L'D':

			do  {

				//very similar subfield remove code for most conversions except TLR which always format "" and []
				part=remove(charn, terminator);
				//if len(part) or terminator then

				if (part.var_mvtyp&pimpl::MVTYPE_STR && part.var_mvstr.length()==0)
					{}
				else
					output ^= part.iconv_D(convstr);

				if (!terminator)
					break;
				output ^= var().chr(256 - terminator);
			} while (true);

			return output;
			break;

		//MD, MC, MT, MX
		case L'M':

			//point to 2nd character
			++conversionchar;

			while (true) {

				//very similar subfield remove code for most conversions except TLR which always format "" and []
				part=remove(charn, terminator);
				//if len(part) or terminator then

				//null string
				if (part.var_mvtyp&pimpl::MVTYPE_STR && part.var_mvstr.length()==0)
					{}

				//do convstr on a number
				else
				{

					//check second character
					switch (*conversionchar)
					{
						//MD or MC
						case L'D':
						case L'C':

throw MVException(L"iconv(MD/MC) not implemented yet");
//							output ^= part.iconv_MD_MC(convstr);
							break;

						//MT
						case L'T':
							output ^= part.iconv_MT(convstr);
							break;

						//MX number to hex (not string to hex)
						case L'X':
throw MVNotImplemented(L"iconv('MX')");
							std::wostringstream ss;
							ss <<std::hex<<std::uppercase<<part.round().toInt();
							output ^= ss.str();

							break;
					}

				}

 				if (!terminator)
					break;
				output ^= var().chr(256 - terminator);
			}

			return output;
			break;

		//[NUMBER [DATE [TIME
		case L'[':

			//empty string in, empty string out
			if (var_mvtyp&pimpl::MVTYPE_STR && var_mvstr.length()==0)
				return L"";

			//check second character
			switch (convstr[1])
			{
				//[NUMBER
				case L'N':
					//non-numeric returned unconverted
					//if (!isnum())
					//	return *this;

					//return oconv_MD_MC(convstr);
					//TODO workout options after [NUMBER,
					return *this;
					break;

				//[DATE
				case L'D':
					return iconv_D(L"D");
					break;

				//[TIME
				case L'T':
					return iconv_MT(L"MT");
					break;
			}

		//L#, R#, T#
		case L'L':
		case L'R':
		case L'T':
			return L"";
			break;

		//HEX
		case L'H':
			//empty string in, empty string out
			if (var_mvtyp&pimpl::MVTYPE_STR && var_mvstr.length()==0)
				return L"";

//TODO allow high end separators in without conversion (instead of failing as non-hex digits)

			//check 2nd character is E, 3rd character is X and next character is null, or a digit
			if ((*(++conversionchar)=='E') && (*(++conversionchar)=='X'))
			{
				//point to one character after HEX
				++conversionchar;

				switch (*conversionchar)
				{
					case L'\0':
						return iconv_HEX(HEX_PER_WCHAR);
						break;
					case L'2':
						return iconv_HEX(2);
						break;
					case L'4':
						return iconv_HEX(4);
						break;
					case L'8':
						return iconv_HEX(8);
						break;
				}

				//return oconv_HEX(HEX_IO_RATIO);
				break;
			}

			break;

		//empty convstr string - no conversion
		case L'\0':
			return (*this);
	}

	//TODO implement
	std::wcout<<L"iconv "<<convstr<< L" not implemented yet "<<std::endl;

	return *this;

}

var var::oconv_T(const var& format) const
{

	//expecting only "T#99" with no mask at the moment

	var just = (format.field(L"#", 1, 1)).substr(1, 1);

	var width2 = format.field(L"#", 2, 1);

	//leave unconverted if non-numeric width
	if (!width2.isnum())
		return *this;

	//
	int width=width2;

	//get padding character from "L(?)" or space
	wchar_t fillchar;
	if (format.var_mvstr.length()>=4 && format.var_mvstr[1]==L'(' && format.var_mvstr[3]==L')')
		fillchar=format.var_mvstr[2];
	else
		fillchar=L' ';

	var output = L"";

	var terminator;
	int nwords;
	var charn = 1;
	std::wstring spacing;

	//process each part between high sep chars
	while (true) {

		//extract characters up to the next high separator character
		var part=remove(charn, terminator);

		if (width) {

			//more complex processing folding on spaces
			//part.converter(TM,L" ");
			part.trimmerf().trimmerb();

			//simple processing if part is less than width
			int partlen=part.length();
			if (partlen<=width) {

				//output ^= part;
				//output ^= var(width-partlen).space();
				part.var_mvstr.resize(width,fillchar);
				output ^= part.var_mvstr;

				if (!terminator)
					break;

				output ^= var().chr(256 - terminator);

				continue;
			}

		

			nwords = part.count(L" ") + 1;

			for (int wordn = 1; wordn <= nwords; wordn++) {
	
				var word = part.field(L" ", wordn, 1);
	
				int wordlen = word.length();
	
				if (wordn > 1) {
					if (!wordlen)
						continue;
					output ^= TM;
				}
	
				//long words get tm inserted every width characters
				for (int ii = 1; ii <= wordlen; ii+=width) {
					if (ii > 1)
						output ^= TM;
					output ^= word.substr(ii, width);
				};//ii;
	
				int remaining = width - (wordlen % width);
	
				if (wordlen == 0 or remaining not_eq width) {
	
					if (remaining <= 1) {
						if (remaining)
							output ^= fillchar;
					}else{
	
						//try to squeeze in following words into the remaining space
						while (remaining > 1 && wordn<nwords) {
							var nextword = part.field(L" ", wordn + 1, 1);
	
							int nextwordlen=nextword.length();
							if (nextwordlen + 1 > remaining)
								break;
	
							wordn += 1;
							output ^= fillchar;
							output ^= nextword;
							remaining -= nextwordlen + 1;
						}//loop;
	
						//output ^= var(remaining).space();
						spacing.resize(remaining,fillchar);
						output ^= spacing;

					}
	
				}
	
			};//wordn;
	
		}

		if (!terminator)
			break;

		output ^= var().chr(256 - terminator);

	}//loop parts

	return output;

}

var var::oconv_MD_MC(const wchar_t* conversion) const
{

	//http://www.d3ref.com/index.php?token=basic.masking.function

	//TODO implement nextchar as a pointer to eliminate charn and convlen

	//not numeric or plain MD does no conversion
	size_t convlen=wcslen(conversion);
	if (!isnum()||convlen<=2)
		return *this;

	//default conversions
	int ndecimals=-1;
	int movedecs=-1;
	bool dontmovepoint=false;
	bool septhousands=false;
	wchar_t trailer=L'\0';
	wchar_t prefixchar=L'\0';

	//get pointer to the third character (after the MD/MC bit)
	size_t charn=2;

	//get the first (next) character
	wchar_t nextchar=conversion[charn];

	//following up to two digits are ndecimals, or ndecimals and movedecimals
	//look for a digit
	if (isdigit(nextchar))
	{

		//first digit is either ndecimals or ndecimals and movedecimals
		ndecimals=nextchar-'0';
		movedecs=ndecimals;

		//are we done
		if (charn>=convlen) goto convert;

		//look for a second digit
		charn++;
		nextchar=conversion[charn];
		if (isdigit(nextchar))
		{
			//get movedecimals
			movedecs=nextchar-'0';

			//are we done
			if (charn>=convlen) goto convert;

			//move to the next character
			charn++;
			nextchar=conversion[charn];
		}
	}

	while (true)
	{
		switch (nextchar)
		{
			case L'P':
				dontmovepoint=true;
				break;

			case L',':
				septhousands=true;
				break;

			case L'D':
				trailer=L'D';
				break;

			case L'C':
				trailer=L'C';
				break;

			case L'-':
				trailer=L'-';
				break;

			default:
				if (prefixchar==L'\0')
					prefixchar=nextchar;
				break;
			}
		//move to next character if any otherwise break
		if (charn>=convlen) break;
		charn++;
			nextchar=conversion[charn];
	}

convert:
	var newmv=*this;

	//move decimals
	if (!dontmovepoint&&movedecs)
		newmv=newmv/pow(10.0,movedecs);

	//rounding
	newmv=newmv.round(ndecimals);

	var part1=newmv.field(L".",1);
	var part2=newmv.field(L".",2);
	int part2len=part2.length();

	//thousand separators
	if (septhousands)
	{
		int part1len=part1.length();
		if (part1len>3)
		{
			var thousandsep=(conversion[1]==L'C') ? L'.' : L',';
			for (int ii=part1len-2;ii>1;ii-=3)
				part1.splicer(ii,0,thousandsep);
		}
	}

	//fixed decimals
	if (ndecimals>0)
	{
		//append decimal point
		part1^=(conversion[1]==L'C')?L',':L'.';

		part1^=part2^std::string(ndecimals-part2len,'0');
	}

	//trailing minus, DB or CR
	switch (trailer)
	{
		case L'\0':
			break;

		case L'-':
			if (part1.substr(1,1)==L"-")
			{
				part1.splicer(1,1,L"");
				part1^=L"-";
			}
			else part1^=L" ";
			break;

		case L'C':
			if (part1.substr(1,1)==L"-")
			{
				part1.splicer(1,1,L"");
				part1^=L"CR";
			}
			else part1^=L"DR";
			break;

		case L'D':
			if (part1.substr(1,1)==L"-")
			{
				part1.splicer(1,1,L"");
				part1^=L"DR";
			}
			else part1^=L"CR";
			break;
	}

	if (prefixchar!=L'\0')
		part1.splicer(1,0,prefixchar);

	return part1;

}

var var::oconv_LR(const var& format) const
{

	//TODO convert to C instead of var for speed
	//and implement full mask options eg L#2-#3-#4 etc

	var varwidth = format.field(L"#", 2, 1);
	var just = (format.field(L"#", 1, 1)).substr(1, 1);

	if (!varwidth.isnum())
		return *this;
	int width=varwidth.toInt();

	//get padding character from "L(?)" or space
	wchar_t fillchar;
	if (format.var_mvstr.length()>=4 && format.var_mvstr[1]==L'(' && format.var_mvstr[3]==L')')
		fillchar=format.var_mvstr[2];
	else
		fillchar=L' ';

	var output = L"";
	var terminator;

	var part;
	int remaining;
	var charn = 1;
	while (true) {

		part=remove(charn, terminator);
		//if len(part) or terminator then

		//TODO optimise with pos1() and pos2()
		//or substr "upto" next character (space here)

		if (width) {

			remaining=width-part.length();
			if (remaining>0) {
				if (just == L"L") {
					//output ^= part;
					//output ^= remaining.space();
					part.var_mvstr.resize(width,fillchar);
					output ^= part;
				} else {
					//output ^= remaining.space();
					//output ^= part;
					part.var_mvstr.insert(0,remaining,fillchar);
					output ^= part;
				}
			} else {
				if (just == L"L")
					//output ^= part.substr(1,width);
					output ^= part.var_mvstr.substr(0,width);
				else
					//output ^= part.substr(-width);
					output ^= part.var_mvstr.substr(part.var_mvstr.length()-width,width);
			}
		}

		//BREAK;
		if (!terminator)
			break;;

		output ^= var().chr(256 - terminator);
	}//loop;

	return output;

}

var var::oconv(const var& conversion) const
{
	THISIS(L"var var::oconv(const var& conversion) const")
	THISISDEFINED()
	ISSTRING(conversion)

	return oconv(conversion.towstring().c_str());

}

//fast version for common programming example where conversion is provided as a hard coded L Wstring
//but application programs source code is usually going to usually be "D2" and not L"D2" so provide a narrow char* version?
//possibly most oconvs will come from variables (eg read from dicts) so will be wstring format
var var::oconv(const wchar_t* conversion) const
{
	THISIS(L"var var::oconv(const wchar_t* conversion) const")
	THISISSTRING()

	//REMOVE the remove logic out of the L# R# and T# here

	var part;
	var charn = 1;
	var terminator;
	var output = L"";

	const wchar_t* conversionchar=conversion;

	//check first character
	switch (*conversionchar)
	{
		//D
		case L'D':

			do  {

				//very similar subfield remove code for most conversions except TLR which always format "" and []
				part=remove(charn, terminator);
				//if len(part) or terminator then

				if (part.var_mvtyp&pimpl::MVTYPE_STR && part.var_mvstr.length()==0)
					{}
				else if (!part.isnum())
					output ^= part;
				else
					output ^= part.oconv_D(conversion);

				if (!terminator)
					break;
				output ^= var().chr(256 - terminator);
			} while (true);

			return output;
			break;

		//MD, MC, MT, MX, ML, MR
		case L'M':

			//point to 2nd character
			++conversionchar;

			while (true) {

				//very similar subfield remove code for most conversions except TLR which always format "" and []
				part=remove(charn, terminator);
				//if len(part) or terminator then

				//null string
				if (part.var_mvtyp&pimpl::MVTYPE_STR && part.var_mvstr.length()==0)
					{}

				//non-numeric are left unconverted
				else if (!part.isnum())
					output ^= part;

				//do conversion on a number
				else
				{

					//check second character
					switch (*conversionchar)
					{
						//MD or MC
						case L'D':
						case L'C':
						case L'R':

							output ^= part.oconv_MD_MC(conversion);
							break;

						//MT
						case L'T':
							output ^= part.oconv_MT(conversion);
							break;

						//MX number to hex (not string to hex)
						case L'X':

							std::wostringstream ss;
							ss <<std::hex<<std::uppercase<<part.round().toInt();
							output ^= ss.str();

							break;
					}

				}

				if (!terminator)
					break;
				output ^= var().chr(256 - terminator);
			}

			return output;
			break;

		//[NUMBER [DATE [TIME
		case L'[':

			//empty string in, empty string out
			if (var_mvtyp&pimpl::MVTYPE_STR && var_mvstr.length()==0)
				return L"";

			//check second character
			switch (conversion[1])
			{
				//[NUMBER
				case L'N':
					//non-numeric returned unconverted
					//if (!isnum())
					//	return *this;

					//return oconv_MD_MC(conversion);
					//TODO workout options after [NUMBER,
					return *this;
					break;

				//[DATE
				case L'D':
					if (!isnum())
						return *this;

					return oconv_D(L"D");
					break;

				//[TIME
				case L'T':
					if (!isnum())
						return *this;

					return oconv_MT(L"MT");
					break;
			}

		//L#, R#
		//format even empty strings
		case L'L':
		case L'R':
			return oconv_LR(conversion);
			break;

		//T#
		//format even empty strings
		case L'T':
			return oconv_T(conversion);
			break;

		//HEX (unlike arev it converts high separator characters)
		case L'H':

			//empty string in, empty string out
			if (var_mvtyp&pimpl::MVTYPE_STR && var_mvstr.length()==0)
				return L"";

			//check 2nd character is E, 3rd character is X and next character is null, or a digit
			if ((*(++conversionchar)=='E') && (*(++conversionchar)=='X'))
			{
				//point to one character after HEX
				++conversionchar;

				switch (*conversionchar)
				{
					case L'\0':
						return oconv_HEX(HEX_PER_WCHAR);
						break;
					case L'2':
						return oconv_HEX(2);
						break;
					case L'4':
						return oconv_HEX(4);
						break;
					case L'8':
						return oconv_HEX(8);
						break;
				}

				//return oconv_HEX(HEX_IO_RATIO);
				break;
			}

			break;

		//empty conversion string - no conversion
		case L'\0':
			return (*this);
	}

	//TODO implement
	std::wcout<<L"oconv "<<conversion<< L" not implemented yet "<<std::endl;

	//unknown conversions are simply ignored in AREV
	return *this;
}

var var::oconv_HEX(const int ioratio) const
{

	//decided to output 8 fixed hex digits per character to represent the full range of unicode characters regardless of platform
	//1. output 8 fixed hex digits per character to represent
	//logic is that this would be consistent and easy to postprocess programmatically and visually
	//and that the inefficiencies are not important since hex isnt a storage format
	//HEX8 and HEX16 codes could be implemented to cater for the alternatives below.
	//Rejected alternatives:
	//2. convert to utf8 and represent each byte as two hex digits.
	//3. convert to utf16 and represent each byte pair as four hex digits
	//4. convert as 1. or 3 depending on platform (word length 4 or 2 chars)

	std::wostringstream ss;
	int nchars=length();
	ss.flags ( ios::right | ios::hex | ios::uppercase);
	//ss.setbase(16) useful to set numerically instead of ios::hex
	ss.fill('0');
	//perhaps convert to use iterators especially to allow for variable width characters under utf16.
	for (int charn=0;charn<nchars;++charn)
	{
		ss.width(ioratio);//must be called every time
		ss << int((*this).var_mvstr[charn]);
	}
	return ss.str();

}

//use macro to ensure inlined instead of using "inline" function
#define ADD_NYBBLE_OR_FAIL \
	nybble=var_mvstr[posn++]; \
	if (nybble<L'0') \
		return L""; \
	if (nybble<=L'9') \
		nybble-=L'0'; \
	else if (nybble>=L'A'&&nybble<=L'F') \
		nybble-=L'7'; \
	else if (nybble>=L'a'&&nybble<=L'f') \
		nybble-=L'W'; \
	else return L""; \
	outchar+=nybble; \

var var::iconv_HEX(const int ioratio) const
{
	//ioratio
	//2 hex digits to one char
	//4 hex digits to one wchar of size 2
	//8 hex digits to one wchar of size 4

	//empty string in, empty string out
	size_t endposn=var_mvstr.length();
	if (!endposn)
		return L"";

	std::wstring textstr=L"";

	size_t posn=0;

	//work out how many hex digits in first character to cater for shortfall
	//eg 7 hex digits 1234567 with ioratio of 4 would be consumed as 0123 4567
	size_t ratio=endposn%ioratio;
	if (!ratio)
		ratio=ioratio;

	wchar_t nybble;
	do
	{
		unsigned int outchar=0;
		switch (ratio)
		{
			//really only need as many cases as input/output ratio
			//this is "loop unrolling
			case 8:
				ADD_NYBBLE_OR_FAIL
				outchar<<=4;
			case 7:
				ADD_NYBBLE_OR_FAIL
				outchar<<=4;
			case 6:
				ADD_NYBBLE_OR_FAIL
				outchar<<=4;
			case 5:
				ADD_NYBBLE_OR_FAIL
				outchar<<=4;
			case 4:
				ADD_NYBBLE_OR_FAIL
				outchar<<=4;
			case 3:
				ADD_NYBBLE_OR_FAIL
				outchar<<=4;
			case 2:
				ADD_NYBBLE_OR_FAIL
				outchar<<=4;
			case 1:
				ADD_NYBBLE_OR_FAIL
				//no shift on last nybble in since it is loaded into the right (right) four bits
				//outchar<<=4;
	}

	//std::wcout<<std::hex<<std::showbase<<outchar<<std::endl;
	textstr+=outchar;

	//only really needs to be done after the 1st outchar
	ratio=ioratio;

	} while (posn<endposn);

	return textstr;
}

#undef ADD_NYBBLE_OR_FAIL


var var::iconv_MT(const wchar_t* conversion) const
{
	//ignore everything else and just get first three groups of digits "99 99 99"
	//remove leading and trailing non-digits and replace internal strings of non-digits with single space
	var time=(*this).swap( L"^\\D+|\\D+$", L"", L"r").swap( L"\\D+", L" ", L"r");

	int hours=time.field(L" ",1).toInt();
	int mins=time.field(L" ",2).toInt();
	int secs=time.field(L" ",3).toInt();

	int inttime=hours*3600+mins*60+secs;

	if (inttime>=86400)
		return L"";

	//PM
	if (inttime<43200&&(*this).index(L"P"))
		inttime+=43200;
	else if (inttime>43200&&(*this).index(L"A"))
		inttime-=43200;

	return inttime;

}

} //of namespace exodus
