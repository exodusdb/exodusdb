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

//for high performance exodus library code
//prevent implicit conversion from cstr to string
//so code must be like var x="xyz" instead of just "xyz"
//to perhaps prevent conversion at runtime to the var string
//optimising compilers may make this
#define MV_NO_NARROW

//C4530: C++ exception handler used, but unwind semantics are not enabled.
#ifdef _MSC_VER
#pragma warning (disable: 4530)
#endif

#ifndef _MSC_VER
#include <signal.h>
#endif

#include <iostream> //cin and cout
#include <cmath> //for floor
#include <cstdlib>//for exit
#include <boost/locale.hpp>
//#include <codecvt>
//#include <locale>
#include <algorithm>

//#include "ConvertUTF.h"

/*
Binary    Hex          Comments
0xxxxxxx  0x00..0x7F   Only byte of a 1-byte character encoding
10xxxxxx  0x80..0xBF   Continuation bytes (1-3 continuation bytes)
110xxxxx  0xC0..0xDF   First byte of a 2-byte character encoding
1110xxxx  0xE0..0xEF   First byte of a 3-byte character encoding
11110xxx  0xF0..0xF4   First byte of a 4-byte character encoding
*/

#ifndef M_PI
//#define M_PI 3.14159265358979323846f
#define M_PI 3.14159265
//2643383279502884197169399375105820974944592307816406286208998f;
#endif

#include <exodus/mv.h>
//#include <exodus/mvutf.h>
#include <exodus/mvexceptions.h>

//std::ios::sync_with_stdio(false);
bool desynced_with_stdio=false;

//TODO check that all string increase operations dont crash the system

namespace exodus {

var var::version() const
{
	return var(__DATE__).iconv("D").oconv("D")^" "^var(__TIME__);
}

bool var::eof() const
{
	//THISIS("bool var::eof() const")
	//THISISDEFINED()

	return (std::cin.eof());
}

bool var::hasinput() {
	bool haskey(void);
	return haskey();
}

//for nchars, use int instead of var to trigger error at point of calling not here
bool var::input(const var& prompt, const int nchars)
{
	THISIS("bool var::inputl(const var& prompt, const int nchars")
	THISISDEFINED()
	ISSTRING(prompt)

	//TODO implement nchars including -1

	if (prompt.length()) {
		prompt.output();
		std::cout<<std::flush;
	}

	if (nchars<0) {

		var_str="";
		var_typ=pimpl::VARTYP_STR;

		char char1;
		while (true) {
			int nc;
			//TODO consider converting from say utf8
			int getkey(void);
			//quit if error or EOF
			char1=getkey();
			if (char1<0)
				break;
			var_str+=char1;
		}
		return var_str.length()>0;
	} else if (nchars>0) {

		var_str="";
		var_typ=pimpl::VARTYP_STR;

		char char1;
		while (true) {
			int nc;
			//TODO consider converting from say utf8
			int getkey(void);
			char1=getkey();

			//try again after a short delay if no key and not enough characters yet
			if (char1<0) {
				this->ossleep(10);
				continue;
			}

			//Enter/Return key always returns whatever has been entered so far
			if (char1<0 || char1==0x0d)
				break;

			//add the character to the output
			var_str+=char1;

			//quit if got the desired number of characters
			if (var_str.length()>=nchars)
				break;

		}
		return var_str.length()>0;
	}

	return this->input();
}

bool var::input()
{
	THISIS("bool var::inputl()")
	THISISDEFINED()

	var_str="";
	var_typ=pimpl::VARTYP_STR;

	//pressing crtl+d indicates eof on unix or ctrl+Z on dos/windows?
	if (std::cin.eof())
		return false;

	std::getline(std::cin,var_str);

	return var_str.length()>0;
}

void var::stop(const var& text) const
{
	THISIS("void var::stop(const var& text) const")
	ISSTRING(text)

	//exit(0);
	throw MVStop(text);
}

/*
//swig for perl wants it on windows!
void var::win32_abort(const var& text) const
{
	abort(text);
}
*/

void var::abort(const var& text) const
{
	THISIS("void var::abort(const var& text) const")
	ISSTRING(text)

	//exit(1);
	throw MVAbort(text);
}

void var::abortall(const var& text) const
{
	THISIS("void var::abortall(const var& text) const")
	ISSTRING(text)

	//exit(1);
	throw MVAbort(text);
}

bool var::assigned() const
{
	//THISIS("bool var::assigned() const")

	//treat undefined as unassigned
	//undefined is a state where we are USING the variable before its contructor has been called!
	//which is possible (in syntax like var xx.osread()?)
	//and also when passing default variables to functions in the functors on gcc
	//THISISDEFINED()

	if (!this||(*this).var_typ & mvtypemask)
		return false;

	return var_typ!=pimpl::VARTYP_UNA;
}

bool var::unassigned() const
{
	//see explanation above in assigned
	//THISIS("bool var::unassigned() const")
	//THISISDEFINED()

	if (!this||((*this).var_typ & mvtypemask))
		return true;

	return !var_typ;
}

var& var::unassigned(const var& defaultvalue) {

	//see explanation above in assigned
        //THISISDEFINED()

        THISIS("var& var::unassigned(const var& defaultvalue) const")
        ISASSIGNED(defaultvalue)

        //?allow undefined usage like var xyz=xyz.readnext();
        //if (var_typ & mvtypemask)

        if (this->unassigned())
        {
                //throw MVUndefined("unassigned( ^ defaultvalue ^")");
                //var_str="";
                //var_typ=pimpl::VARTYP_STR;
                *this=defaultvalue;
        }
        return *this;
}

//pick int() is actually the same as floor. using integer() instead of int() because int is a reserved word in c/c++ for int datatype
var var::integer() const
{
	THISIS("var var::integer() const")
	THISISNUMERIC()

	/*pick integer means floor()
	-1.0=-1
	-0.9=-1
	-0.5=-1
	-0.1=-1
	 0  =0
	 0.1=0
	 0.5=0
	 0.9=0
	 1.0=1
	*/

	//prefer int
	if (var_typ & pimpl::VARTYP_INT)
		return var_int;

	//floor
	var_int=std::floor(var_dbl);
	var_typ|=pimpl::VARTYP_INT;
	return var_int;

}

//integer and floor are the same
var var::floor() const
{
	return this->integer();

}

var var::round(const int ndecimals) const
{
	/*pick round rounds positive .5 up to 1 and negative .5 down to -1 etc
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

	//if n=0 could save the integer conversion here but would require mentality to save BOTH int and double
	//currently its possible since space is reserved for both but this may change

	THISIS("var var::round() const")
	THISISNUMERIC()

	double result;
	//prefer double
	if (var_typ & pimpl::VARTYP_DBL)
		result=var_dbl;
	else
	{
		if (not ndecimals)
			return var_int;
		//loss of precision if var_int is long long
		result=int(var_int);
	}

	//scale it up (or down)
	double scale=pow(10.0,ndecimals);
	result*=scale;

	//round .5 up and -.5 down to nearest integer as per pick financial rounding concept
	long long int result2;
	if (result>=0)
		result2=(long long int)(result+0.5);
	else
		result2=(long long int)(result-0.5);

	//scale it back down (or back up)
	result=result2/scale;

	return result;

}

bool var::toBool() const
{
	THISIS("bool var::toBool() const")
	//could be skipped for speed assuming that people will not write unusual "var x=f(x)" type syntax as follows:
	//var xx=xx?11:22;
	THISISDEFINED()

	//identical code in void* and bool except returns void* and bool respectively
	do
	{
		//ints are true except for zero
		if (var_typ & pimpl::VARTYP_INT)
			return (bool)(var_int!=0);

		//non-numeric strings are true unless zero length
		if (var_typ & pimpl::VARTYP_NAN)
			//return (bool)(var_str.length()!=0);
			return !var_str.empty();

		//doubles are true unless zero
		//check double first dbl on guess that tests will be most often on financial numbers
		//TODO should we consider very small numbers to be the same as zero?
		if (var_typ & pimpl::VARTYP_DBL)
			return (bool)(var_dbl!=0);

		if (!(var_typ))
		{
			THISISASSIGNED()
			throw MVUnassigned("toBool()");
		}

		//must be string - try to convert to numeric and do all tests again
		isnum();

	} while (true);

}

int var::toInt() const
{
	THISIS("int var::toInt() const")
	THISISNUMERIC()

	//loss of precision if var_int is long long
	return (var_typ & pimpl::VARTYP_INT) ? int(var_int) : int(var_dbl);
}

int var::toLong() const
{
	THISIS("int var::toLong() const")
	THISISNUMERIC()

	//loss of precision if var_int is long long
	return (var_typ & pimpl::VARTYP_INT) ? long(var_int) : long(var_dbl);
}

double var::toDouble() const
{
	THISIS("double var::toDouble() const")
	THISISNUMERIC()

	//return double by preference
	return (var_typ & pimpl::VARTYP_DBL) ? double(var_dbl) : double(var_int);
}

//mainly called in ISSTRING when not already a string
void var::createString() const
{
	//THISIS("void var::createString() const")
	//TODO ensure ISDEFINED is called everywhere in advance
	//to avoid wasting time doing multiple calls to ISDEFINED
	//THISISDEFINED()

	//dbl - create string from dbl
	//prefer double
	if (var_typ & pimpl::VARTYP_DBL) {
		var_str=dblToString(var_dbl);
		var_typ|=pimpl::VARTYP_STR;
		return;
	}

	//int - create string from int
	if (var_typ & pimpl::VARTYP_INT) {
		//loss of precision if var_int is long long
		var_str=intToString(int(var_int));
		var_typ|=pimpl::VARTYP_STR;
		return;
	}
	//already a string (unlikely since only called when not a string)
	if (var_typ & pimpl::VARTYP_STR) {
		return;
	}

	//treat any other case as unassigned
	//(usually var_typ & pimpl::VARTYP_UNA)
	throw MVUnassigned("createString()");

}

//DLL_PUBLIC
std::string var::toString() const
{
	THISIS("std::string var::toString() const")
	THISISSTRING()

        return var_str;
}

var var::length() const
{
	THISIS("var var::length() const")
	THISISSTRING()

	return int(var_str.length());
}

//synonym for length for compatibility with pick's len()
var var::len() const
{
	THISIS("var var::len() const")
	THISISSTRING()

	return int(var_str.length());
}

const char* var::data() const
{
	THISIS("const char* var::data() const")
	THISISSTRING()

	return var_str.data();
}


var var::trim(const var& trimchar) const
{
	THISIS("var var::trim(const var& trimchar) const")
	ISSTRING(trimchar)

	return trim(trimchar.var_str.c_str());
}

var var::trim(const var& trimchar, const var& options) const
{
	THISIS("var var::trim(const var& trimchar, const var& options) const")
	ISSTRING(trimchar)
	ISSTRING(options)

	if (options=="F") {
		return trimf(trimchar.var_str.c_str());
	}
	else if (options=="B") {
		return trimb(trimchar.var_str.c_str());
	}
	else if (options=="FB") {
		return trimf(trimchar.var_str.c_str()).trimb(trimchar.var_str.c_str());
	}
	return trim(trimchar.var_str.c_str());
}

var& var::trimmer(const var& trimchar)
{
	THISIS("var& var::trimmer(const var& trimchar)")
	ISSTRING(trimchar)

	return trimmer(trimchar.var_str.c_str());
}

var& var::trimmer(const var& trimchar, const var& options)
{
	THISIS("var var::trimmer(const var& trimchar, const var& options) const")
	ISSTRING(trimchar)
	ISSTRING(options)

	if (options=="F") {
		return trimmerf(trimchar.var_str.c_str());
	}
	else if (options=="B") {
		return trimmerb(trimchar.var_str.c_str());
	}
	else if (options=="FB") {
		return trimmerf(trimchar.var_str.c_str()).trimmerb(trimchar.var_str.c_str());
	}
	return trimmer(trimchar.var_str.c_str());
}

var var::trimf(const var& trimchar) const
{
	THISIS("var var::trimf(const var& trimchar) const")
	ISSTRING(trimchar)

	return trimf(trimchar.var_str.c_str());
}

var& var::trimmerf(const var& trimchar)
{
	THISIS("var& var::trimmerf(const var& trimchar)")
	ISSTRING(trimchar)

	return trimmerf(trimchar.var_str.c_str());
}

var var::trimb(const var& trimchar) const
{
	THISIS("var var::trimb(const var& trimchar) const")
	ISSTRING(trimchar)

	return trimb(trimchar.var_str.c_str());
}

var& var::trimmerb(const var& trimchar)
{
	THISIS("var& var::trimmerb(const var& trimchar)")
	ISSTRING(trimchar)

	return trimmerb(trimchar.var_str.c_str());
}

var var::trimf(const char* trimchar) const
{
	THISIS("var var::trimf(const char* trimchar) const")
	THISISSTRING()

	return var(*this).trimmerf(trimchar);
}

var& var::trimmerf(const char* trimchar)
{
	THISIS("var& var::trimmerf(const char* trimchar)")
	THISISSTRING()

	std::string::size_type start_pos;
	start_pos=var_str.find_first_not_of(trimchar);

	if (start_pos==std::string::npos)
	{
		*this="";
		return *this;
	}

	//return var(var_str.substr(start_pos));
	var_str.erase(0,start_pos);

	//clear numeric flags in case changed from/to numeric
	var_typ&=pimpl::VARTYP_NOTNUMFLAGS;

	return *this;

}

var var::trimb(const char* trimchar) const
{
	THISIS("var var::trimb(const char* trimchar) const")
	THISISSTRING()

	return var(*this).trimmerb(trimchar);
}

var& var::trimmerb(const char* trimchar)
{
	THISIS("var& var::trimmerb(const char* trimchar)")
	THISISSTRING()

	std::string::size_type end_pos;
	end_pos=var_str.find_last_not_of(trimchar);

	if (end_pos==std::string::npos)
	{
		*this="";
		return *this;
	}

	//return var(var_str.substr(0,end_pos+1));
	var_str.erase(end_pos+1);

        //clear numeric flags in case changed from/to numeric
        var_typ&=pimpl::VARTYP_NOTNUMFLAGS;

	return *this;

}


var var::trim(const char* trimchar) const
{
	THISIS("var var::trim(const char* trimchar) const")
	THISISSTRING()

	return var(*this).trimmer(trimchar);
}

var& var::trimmer(const char* trimchar)
{

	//reimplement with boost string trim_if algorithm
	//http://www.boost.org/doc/libs/1_39_0/doc/html/string_algo/reference.html

	THISIS("var& var::trimmer(const char* trimchar)")
	THISISSTRING()

	//find the first non blank
	std::string::size_type start_pos;
	start_pos=var_str.find_first_not_of(trimchar);

	//if all blanks return empty string
	if (start_pos==std::string::npos)
	{
		*this="";
		return *this;
	}

	//erase leading spaces
	if (start_pos!=0) var_str.erase(0,start_pos);

	//find the last non blank
	std::string::size_type end_pos;
	//end_pos=var_str.find_last_not_of(trimchar);
	end_pos=var_str.find_last_not_of(trimchar,var_str.length()-1);

	//erase trailing spaces
	var_str.erase(end_pos+1);

	//for speed and safety do this once, regardless of any trimming actually done
        //clear numeric flags in case changed from/to numeric
        var_typ&=pimpl::VARTYP_NOTNUMFLAGS;

	//find the starting position of any embedded spaces
	start_pos=std::string::npos;
	while (true)
	{
		//find a space
		start_pos=var_str.find_last_of(trimchar,start_pos);

		//if no (more) spaces then return the string
		if (start_pos==std::string::npos||start_pos<=0)
			return *this;

		//find the first non-space thereafter
		end_pos=var_str.find_last_not_of(trimchar,start_pos-1);

		//if first non space character is not one before the space
		if (end_pos<start_pos-1)
		{
			var_str.erase(end_pos+1,start_pos-end_pos-1);
		}
		if (end_pos<=0)
			break;
		start_pos=end_pos-1;
	}
	return *this;

}

//virtually identical code for invert and textinvert
var var::invert() const
{
	THISIS("var& var::invert()")
	THISISSTRING()
	var tt=*this;
	tt.inverter();
	return tt;
}

//virtually identical code for invert and textinvert
var var::textinvert() const
{
	THISIS("var& var::textinvert()")
	THISISSTRING()
	var tt=*this;
	tt.textinverter();
	return tt;
}

//inverts bits of ASCII bytes - ignoring multibyte unicode
var& var::inverter()
{
	THISIS("var& var::inverter()")
	THISISSTRING()

	//clear numeric flags in case changed from/to numeric
	var_typ&=pimpl::VARTYP_NOTNUMFLAGS;

	//only transform bytes 0-127
	std::transform(var_str.begin(), var_str.end(), var_str.begin(),
                   [](char32_t c) -> char32_t { return (c>127) ? c : (c ^ char32_t(127)); });

	return *this;
}

//inverts lower 8 bits of UTF8 codepoints
//THROWS on invalid UTF8 bytes
var& var::textinverter()
{
	THISIS("var& var::textinverter()")
	THISISSTRING()

	//xor each unicode code point, with the bits we want to toggle ... ie the bottom 8
	//since we will keep inversion within the same 256 byte pages of unicode codepoints
	//TODO invert directly in the UTF8 bytes - requires some cleverness

	//clear numeric flags in case changed from/to numeric
	var_typ&=pimpl::VARTYP_NOTNUMFLAGS;

	//convert to char32.t string - four bytes per code point
	std::u32string wstring1=boost::locale::conv::utf_to_utf<char32_t>(var_str);

	//invert only the lower 8 bits to keep the resultant code points within the same unicode 256 byte page
	//char32_t invertbits=255;
	//for (size_t ii = 0; ii < wstring1.length(); ii++)
	//	wstring1[ii] = wstring1[ii] ^ invertbits;
	std::transform(wstring1.begin(), wstring1.end(), wstring1.begin(),
                   [](char32_t c) -> char32_t { return c ^ char32_t(255); });

	//convert back to utf8
	var_str=boost::locale::conv::utf_to_utf<char>(wstring1);

	return *this;
}

var var::ucase() const
{
	return var(*this).ucaser();
}

var& var::ucaser()
{
	THISIS("var& var::ucaser()")
	THISISSTRING()

	//cannot chaneg to/from numeric
        //clear numeric flags in case changed from/to numeric
        //var_typ&=pimpl::VARTYP_NOTNUMFLAGS;

	return localeAwareChangeCase(2);
/*
int32_t ucasemap_utf8ToLower 	( 	const UCaseMap *  	csm,
		char *  	dest,
		int32_t  	destCapacity,
		const char *  	src,
		int32_t  	srcLength,
		UErrorCode *  	pErrorCode 
	) 	
*/
}

var var::lcase() const
{
	return var(*this).lcaser();
}

var& var::lcaser()
{
	THISIS("var& var::lcaser()")
	THISISSTRING()

	//cannot change to/from numeric
        //clear numeric flags in case changed from/to numeric
        //var_typ&=pimpl::VARTYP_NOTNUMFLAGS;

	return localeAwareChangeCase(1);
}

var var::unique() const
{
	THISIS("var var::unique()")
	THISISSTRING()

        //linemark
        var result = "";
        var start = 0;
	var bit;
	var delimiter;
	var sepchar=VM;
	bool founddelimiter=false;
        while (true) {

                bit=this->remove(start, delimiter);

		if (!founddelimiter && delimiter)
			//sepchar=RM_-int(delimiter)+1;
			sepchar=var().chr(RM.seq()-delimiter+1);

                if (bit.length()) {
                        if (not(result.locateusing(sepchar,bit))) {
                                if (delimiter)
					result ^= bit ^ sepchar;
                        }
                }
		if (not delimiter)
			break;
        }//loop;
        result.splicer(-1, 1, "");
        return result;

}

//BINARY - 1st byte
const var var::seq() const
{
	THISIS("var var::seq() const")
	THISISSTRING()

	if (var_str.length()==0)
		return 0;

	int byteno=var_str[0];
	if (byteno>=0)
		return byteno;
	else
		return byteno+256;

}

//UTF8 - 1st UTF code point
const var var::textseq() const
{
	THISIS("var var::textseq() const")
	THISISSTRING()
/*
	if (var_str.length()==0)
		return 0;

	int byteno=var_str[0];
	if (byteno>=0)
		return byteno;
	else
		return byteno+256;
*/
	//get four bytes from input string since in UTF8 a unicode code point may occupy up to 4 bytes
	std::basic_string<char32_t> str1=boost::locale::conv::utf_to_utf<char32_t>(var_str.substr(0,4));

	return int(uint32_t(str1[0]));
}

//only returns BINARY bytes 0-255 (128-255) cannot be stored in the database unless with other bytes making up UTF8
var var::chr(const int charno) const
{
	return char(charno % 256);
}

//returns unicode 1-4 byte sequences (in utf8)
var var::textchr(const int utf_codepoint) const
{
	//doesnt use *this at all (do we need a version that does?)

	//return var((char) int1);

	std::wstring wstr1;
	wstr1.push_back(wchar_t(uint32_t(utf_codepoint)));
	return boost::locale::conv::utf_to_utf<char>(wstr1);

/*
	//make a converter
	std::wstring_convert<std::codecvt_utf8<char32_t>,char32_t> converter;

	//make a one character wide string;
	std::u32string wstring1 {uint32_t(int1)};

	//convert to utf8 and return
	std::string result=converter.to_bytes(wstring1);
	return result;
*/

/*	//using code from ConvertUTF.c

	#define CONV_INT_TO_UTF8_MAX_OUTPUT_BYTES 4

	const UTF32   u32_char    = uint(utf_codepoint);
	const UTF32*  u32_ptr     = &u32_char;

	UTF8   utf8_bytes[CONV_INT_TO_UTF8_MAX_OUTPUT_BYTES] = {0};
	UTF8*  utf8_ptr      = &utf8_bytes[0];

	//ConversionResult ConvertUTF32toUTF8 (
	//        const UTF32** sourceStart, const UTF32* sourceEnd, 
	//        UTF8** targetStart, UTF8* targetEnd, ConversionFlags flags);

	ConversionFlags flags;
	ConversionResult result=ConvertUTF32toUTF8 (
		&u32_ptr, u32_ptr+1,
		&utf8_ptr, &utf8_bytes[CONV_INT_TO_UTF8_MAX_OUTPUT_BYTES], flags);

	//std::cout << "'" << utf8_bytes << "'" << std::endl;
	return std::string((char*)(utf8_bytes));
*/

/*
	//create a converter between string/bytes and wstring/char32
	std::wstring_convert<std::codecvt_utf8<char32_t>,char32_t> converter;

	//convert to four bytes per code point
	std::u32string wstring1 = converter.from_bytes(var_str);

	//invert only the lower 8 bits to keep the resultant code points within the same unicode 256 byte page
	char32_t invertbits=255;
	for (size_t ii = 0; ii < wstring1.length(); ii++)
		wstring1[ii] = wstring1[ii] ^ invertbits;

	//convert back to utf8
	var_str=converter.to_bytes(wstring1);
*/
}

var var::quote() const
{
	THISIS("var var::quote() const")
	THISISSTRING()

	return var(*this).quoter();
}

var& var::quoter()
{
	THISIS("var& var::quoter()")
	THISISSTRING()

        //clear numeric flags in case changed from numeric
        var_typ&=pimpl::VARTYP_NOTNUMFLAGS;

	//NB this is std::string "replace" not var field replace
	var_str.replace(0,0,"\"");
	var_str+='"';
	return *this;

}

var var::squote() const
{
	THISIS("var var::squote() const")
	THISISSTRING()

	return var(*this).squoter();
}

var& var::squoter()
{
	THISIS("var& var::squoter()")
	THISISSTRING()

        //clear numeric flags in case changed from numeric
        var_typ&=pimpl::VARTYP_NOTNUMFLAGS;

	//NB this is std::string "replace" not var field replace
	var_str.replace(0,0,"'");
	var_str+='\'';
	return *this;

}

var var::unquote() const
{
	THISIS("var var::unquote() const")
	THISISSTRING()

	return var(*this).unquoter();
}

var& var::unquoter()
{
	THISIS("var& var::unquoter()")
	THISISSTRING()

        //clear numeric flags in case changed to numeric
        var_typ&=pimpl::VARTYP_NOTNUMFLAGS;

	//removes MATCHING beginning and terminating " or ' characters
	//also removes a SINGLE " or ' on the grounds that you probably want to eliminate all such characters

	//no change if no length
	size_t len=var_str.length();
	if (!len)
		return *this;

	char char0=var_str[0];

	//no change if not starting " or '
	if (char0!='\"' && char0!='\'')
		return *this;

	//no change if terminating character ne starting character
	if (var_str[len-1]!=char0)
		return *this;

	//erase first (and last character if more than one)
	var_str.erase(0,1);
	if (len)
		var_str.erase(len-2,1);

	return *this;

}

var var::splice(const int start1,const int length,const var& newstr) const
{
	THISIS("var var::splice(const int start1,const int length,const var& newstr) const")
	THISISSTRING()

	return var(*this).splicer(start1,length,newstr);
}

var var::splice(const int start1,const var& newstr) const
{
	THISIS("var var::splice(const int start1,const var& newstr) const")
	THISISSTRING()

	return var(*this).splicer(start1,newstr);
}

var& var::splicer(const int start1,const int length,const var& newstr)
{
	THISIS("var& var::splicer(const int start1,const int length,const var& newstr)")
	THISISSTRING()
	ISSTRING(newstr)

	//prepare a new var
	//functionmode var newmv=var(var_str);
	//proceduremode

	//TODO make sure start and length work like REVELATION and HANDLE NEGATIVE LENGTH!

	int start1b;
	if (start1>0) {
		start1b=start1;
	}
	else if (start1<0)
	{
		start1b=int(var_str.length())+start1+1;
        	if (start1b<1)
			start1b=1;
	}
	else
		start1b=1;

	int lengthb;
	if (length>=0)
		lengthb=length;
	else {
		//cannot go before start of string
		if ((start1b+length)<=0)
		{
			start1b=1;
			lengthb=start1b;
		}
		else
		{
			start1b+=length+1;
			lengthb=-length;
		}
	}

        //clear numeric flags in case changed from/to numeric
        var_typ&=pimpl::VARTYP_NOTNUMFLAGS;

	if (start1b>var_str.length())
		var_str+=newstr.var_str;
	else
		var_str.replace(start1b-1,lengthb,newstr.var_str);

	return *this;

}

var& var::splicer(const int start1, const var& newstr)
{
	THISIS("var& var::splicer(const int start1, const var& newstr)")
	THISISSTRING()
	ISSTRING(newstr)

	//prepare a new var
	//functionmode var newmv=var(var_str);
	//proceduremode

//TODO make sure start and length work like REVELATION and HANDLE NEGATIVE LENGTH!
	int start1b;
	if (start1>0)
		start1b=start1;
	else if (start1<0)
	{
		start1b=int(var_str.length())+start1+1;
		if (start1b<1)
			start1b=1;
    	}
	else
		start1b=1;

        //clear numeric flags in case changed from/to numeric
        var_typ&=pimpl::VARTYP_NOTNUMFLAGS;

	if (start1b>var_str.length())
		var_str+=newstr.var_str;
	else
		var_str.replace(start1b-1,var_str.length(),newstr.var_str);

	return *this;

}

var& var::transfer(var& destinationvar)
{
	THISIS("var& var::transfer(var& destinationvar)")
	//transfer even unassigned vars (but not uninitialised ones)
	THISISDEFINED()
	ISDEFINED(destinationvar)

	destinationvar.var_str.swap(var_str);
	destinationvar.var_typ=var_typ;
	destinationvar.var_int=var_int;
	destinationvar.var_dbl=var_dbl;

	var_str="";
	var_typ=pimpl::VARTYP_STR;

	return destinationvar;
}

var var::clone() const
{
	THISIS("var var::clone(var& destinationvar)")
	//clone even unassigned vars (but not uninitialised ones)
	THISISDEFINED()

	var clone;
	clone.var_typ=var_typ;
	clone.var_str=var_str;
	clone.var_int=var_int;
	clone.var_dbl=var_dbl;

	return clone;
}

//kind of const needed in calculatex
const var& var::exchange(const var& var2) const
{
	THISIS("var& var::exchange(var& var2)")
	//exchange even unassigned vars (but not uninitialised ones)
	THISISDEFINED()
	ISDEFINED(var2)

	//intermediary copies of var2
	int mvtypex=var2.var_typ;
	mvint_t mvintx=var2.var_int;
	double mvdblx=var2.var_dbl;

	//do string first since it is the largest and most likely to fail
	var_str.swap(var2.var_str);

	//copy mv to var2
	var2.var_typ=var_typ;
	var2.var_int=var_int;
	var2.var_dbl=var_dbl;

	//copy intermediaries to mv
	var_typ=mvtypex;
	var_int=mvintx;
	var_dbl=mvdblx;

	return var2;
}

var var::str(const int num) const
{
	THISIS("var var::str(const int num) const")
	THISISSTRING()

	var newstr="";
	if (num<0)
		return newstr;

	int basestrlen=int(var_str.length());
	if (basestrlen==1)
		newstr.var_str.resize(num,var_str.at(0));
	else if (basestrlen)
		for (int ii=num;ii>0;--ii)
			newstr^=var_str;

	return newstr;
}

var var::space() const
{
	THISIS("var var::space() const")
	THISISNUMERIC()

	var newstr="";
	int nspaces=(*this).round().toInt();
	if (nspaces>0)
		newstr.var_str.resize(nspaces,' ');

	return newstr;
}

var var::crop() const
{
	THISIS("var var::crop() const")
	THISISSTRING()

	return var(*this).cropper();
}

var& var::cropper()
{
	THISIS("var& var::cropper()")
	THISISSTRING()

	std::string newstr="";

	std::string::iterator iter=var_str.begin();
	std::string::iterator iterend=var_str.end();

	while (iter != iterend) {

		char charx=(*iter);
		++iter;

		//simply append ordinary characters
		if (charx<STM_ || charx>RM_)
		{
			newstr.push_back(charx);
			continue;
		}

		//found a separator

		//remove any lower separators from the end of the string
		while (!newstr.empty()) {
			char lastchar=newstr.back();
			if (lastchar >= STM_ && lastchar < charx)
				newstr.pop_back();
			else
				break;
		}

		//append the separator
		newstr.push_back(charx);
	}

	//remove any trailing separators
	while (!newstr.empty() && newstr.back()>=STM_ && newstr.back()<=RM_) {
		newstr.pop_back();
	}

        //clear numeric flags in case changed from/to numeric
        var_typ&=pimpl::VARTYP_NOTNUMFLAGS;

	var_str=newstr;
	//swap(var_str,newstr);

	return *this;
}

var var::lower() const
{
	THISIS("var var::lower() const")
	THISISSTRING()

	return var(*this).lowerer();
}

var& var::lowerer()
{
	THISIS("var& var::lowerer()")
	THISISSTRING()

	//note: rotate lowest sep to highest
	///TODO consider converting lowest sep to char 0
	this->converter(_RM_ _FM_ _VM_ _SM_ _TM_ _STM_,
			  _FM_ _VM_ _SM_ _TM_ _STM_ _RM_);

	return *this;
}

var var::raise() const
{
	THISIS("var var::raise() const")
	THISISSTRING()

	return var(*this).raiser();
}

var& var::raiser()
{
	THISIS("var& var::raiser()")
	THISISSTRING()

	//note: rotate highest sep to lowest
	//TODO consider converting highest sep to char 0
	this->converter(_FM_ _VM_ _SM_ _TM_ _STM_ _RM_,
			  _RM_ _FM_ _VM_ _SM_ _TM_ _STM_);

	return *this;
}

var var::convert(const var& oldchars,const var& newchars) const
{
	THISIS("var var::convert(const var& oldchars,const var& newchars) const")
	THISISSTRING()

	//return var(*this).converter(oldchars,newchars);
	var temp=var(*this).converter(oldchars,newchars);
	return temp;
}

var& var::converter(const var& oldchars,const var& newchars)
{
	THISIS("var& var::converter(const var& oldchars,const var& newchars)")
	THISISSTRING()
	ISSTRING(oldchars)
	ISSTRING(newchars)

	std::string::size_type pos=std::string::npos;

        //clear numeric flags in case changed from/to numeric
        var_typ&=pimpl::VARTYP_NOTNUMFLAGS;

	while (true)
	{
		//locate (backwards) any of the from characters
		//because we might be removing characters
		pos=var_str.find_last_of(oldchars.var_str,pos);

		if (pos==std::string::npos)
			break;

		//find which from character we have found
		int fromcharn=int(oldchars.var_str.find(var_str[pos]));

		if (fromcharn<int(newchars.var_str.length()))
			var_str.replace(pos,1,newchars.var_str.substr(fromcharn,1));
		else
			var_str.erase(pos,1);

		if (pos==0)
			break;
		pos--;
	}

	return *this;

}

/*
var& var::textconverter(const var& oldchars,const var& newchars)
{
	THISIS("var& var::converter(const var& oldchars,const var& newchars)")
	THISISSTRING()
	ISSTRING(oldchars)
	ISSTRING(newchars)

	std::string::size_type pos=std::string::npos;

        //clear numeric flags in case changed from/to numeric
        var_typ&=pimpl::VARTYP_NOTNUMFLAGS;

	while (true)
	{
		//locate (backwards) any of the from characters
		//because we might be removing characters
		pos=var_str.find_last_of(oldchars.var_str,pos);

		if (pos==std::string::npos)
			break;

		//find which from character we have found
		int fromcharn=int(oldchars.var_str.find(var_str[pos]));

		if (fromcharn<int(newchars.var_str.length()))
			var_str.replace(pos,1,newchars.var_str.substr(fromcharn,1));
		else
			var_str.erase(pos,1);

		if (pos==0)
			break;
		pos--;
	}

	return *this;

}
*/

//numeric is one of four regular expressions or zero length string
//^$			zero length string
//[+-]?9+		eg 999
//[+-]?9+.		eg 999.
//[+-]?.9+		eg .999
//[+-]?9+.9+	eg 999.999
//where the last four examples may also have a + or - character prefix

//be careful that the following are NOT numeric. regexp [+-]?[.]?
// + - . +. -.

//rules
//0. zero length string is numeric integer 0
//1. any + or - must be the first character
//2. point may occur 0 or 1 times
//3. digits (0-9) must occur 1 or more times (but see rule 0.)
//4. all characters mean non-numeric

bool var::isnum(void) const
{
	THISIS("bool var::isnum(void) const")
	//TODO make isnum private and ensure ISDEFINED is checked before all calls to isnum
	//to save the probably double check here
	THISISDEFINED()

	//is numeric already
	if (var_typ & pimpl::VARTYP_INTDBL)
		return true;

	//is known not numeric already
	//maybe put this first if comparison operations on strings are more frequent than numeric operations on numbers
	if (var_typ & pimpl::VARTYP_NAN)
		return false;

	//not assigned error
	if (!var_typ)
		throw MVUnassigned("isnum()");

	//if not a number and not unassigned then it is an unknown string

	bool point=false;
	bool digit=false;

	//otherwise find test
	//a number is optional minus followed by digits and max of one decimal point
	//NB going backwards - for speed?
	//TODO: check if going backwards is slow for variable width multi-byte character strings
	//and replace with forward scanning

	// Suggestion: statistically, non numeric strings are postfixed by numbers but not prefixed.
	// Example, natural names for DB tables are CUSTOMERS1 and REPORT2009, but not 1CUSTOMER and 2009REPORT
//	for (int ii=(int)var_str.length()-1;ii>=0;--ii)
	for (int ii=0; ii < (int)var_str.length(); ii++)
	{
		char cc=var_str[ii];
		// + 2B		- 2D
		// . 2E		number 30-39
		if (cc > '9')		// for sure not a number
		{
			var_typ=pimpl::VARTYP_NANSTR;
			return false;
		}
		if (cc >= '0')
		{
			digit=true;
		}
		else
		{
			switch (cc)
			{
				case '.':
					if (point)	// 2nd point - is non-numeric
					{
						var_typ=pimpl::VARTYP_NANSTR;
						return false;
					}
					point=true;
					break;

				case '-':
				case '+':
					//non-numeric if +/- is not the first character or is the only character
					if (ii)
					{
						var_typ=pimpl::VARTYP_NANSTR;
						return false;
					}
					break;

				//any other character mean non-numeric
				default:
					var_typ=pimpl::VARTYP_NANSTR;
					return false;
			}
		}
	}

	//strategy:
	//Only test the length of string once in the relatively common cases (ie where
	//the string fails numeric testing or there is at least one digit in the string)
	//Only in the rarer cases of the string being zero length
	//(or being one of special cases + - . +. -.) will the length be checked twice
	// digit=false
	// test length zero so no loop
	// if (!digit)
	//  test length zero again

	//perhaps slightly unusual cases here
	//zero length string and strings like + - . +. -.
	if (!digit)
	{
		//must be at least one digit unless zero length string
		if (var_str.length())
		// goto nan;
		{
			var_typ=pimpl::VARTYP_NANSTR;
			return false;
		}

		//zero length string is integer 0
		var_int=0;
		var_typ=pimpl::VARTYP_INTSTR;
		return true;
	}

	//get and save the number as int or double
	if (point)
	{
		//var_dbl=_wtof(var_str.c_str());
		//var_dbl=_wtof(var_str.c_str());
		//TODO optimise with code based on the following idea?
		/*
		union switchitup
		{
			char bigletter;
			char smalletters[sizeof(char)/sizeof(char)];
		} abcd;
		abcd.bigletter=0x0035;
		std::cout<<atoi(abcd.smalletters)<<std::endl;
		*/

		// http://www.globalyzer.com/gi/help/gihelp/unsafeMethod/atof.htm :
		// "... _wtof is supported only on Windows platforms"
		// "... Recommended Replacements: ANSI UTF-16 strtod"
///		std::string result(var_str.begin(),var_str.end());
///		var_dbl=atof(result.c_str());
		var_dbl=strtod(var_str.c_str(), 0);
		var_typ=pimpl::VARTYP_DBLSTR;
	}
	else
	{
		//var_int=_wtoi(var_str.c_str());
		//TODO optimise

		//change from
///		std::string result(var_str.begin(),var_str.end());
///		var_int=atoi(result.c_str());
		var_int=strtol(var_str.c_str(), 0, 10);
		var_typ=pimpl::VARTYP_INTSTR;
	}
	//indicate isNumeric
	return true;
}

/////////
// output
/////////

const var& var::put(std::ostream& ostream1) const
{
	THISIS("const var& var::put(std::ostream& ostream1) const")
	THISISSTRING()

	//prevent output to cout suppressing output to cout (by non-exodus routines)
	//http://gcc.gnu.org/ml/gcc-bugs/2006-05/msg01196.html
	//TODO optimise by calling once instead of every call to output()
	if (!desynced_with_stdio)
	{
		std::ios::sync_with_stdio(false);
		desynced_with_stdio=true;
	}

	//verify conversion to UTF8
	//std::string tempstr=(*this).toString();

	ostream1.write(var_str.data(),(std::streamsize) var_str.length());
	return *this;
}

const var& var::output() const
{
	return put(std::cout);
}

const var& var::outputl() const
{
	put(std::cout);
	std::cout << std::endl;
	return *this;
}

const var& var::outputt() const
{
	put(std::cout);
	std::cout << '\t';
	return *this;
}


const var& var::output(const var& str) const
{
	str.put(std::cout);
	return put(std::cout);
}

const var& var::outputl(const var& str) const
{
	str.put(std::cout);
	put(std::cout);
	std::cout << std::endl;
	return *this;
}

const var& var::outputt(const var& str) const
{
	std::cout << "\t";
	str.put(std::cout);
	std::cout << "\t";
	put(std::cout);
	return *this;
}



const var& var::errput() const
{
	return put(std::cerr);
}

const var& var::errputl() const
{
	put(std::cerr);
	std::cerr << std::endl;
	return *this;
}

const var& var::errput(const var& str) const
{
	str.put(std::cerr);
	return put(std::cerr);
}

const var& var::errputl(const var& str) const
{
	str.put(std::cerr);
	put(std::cerr);
	std::cerr << std::endl;
	return *this;
}


//clog is unbuffered version of cerr?
// 3>xyz.log captures nothing on windows
const var& var::logput() const
{
	put(std::clog);
	std::clog.flush();
	return *this;
}

const var& var::logputl() const
{
	put(std::clog);
	std::clog << std::endl;
	return *this;
}


const var& var::logput(const var& str) const
{
	str.put(std::clog);
	return put(std::clog);
}

const var& var::logputl(const var& str) const
{
	str.put(std::clog);
	put(std::clog);
	std::clog << std::endl;
	return *this;
}



////////
//DCOUNT
////////
//TODO make a char and char version for speed
var var::dcount(const var& substrx) const
{
	THISIS("var var::dcount(const var& substrx) const")
	THISISSTRING()
	ISSTRING(substrx)

	if (var_str.length()==0)
		return 0;

	return count(substrx)+1;

}


///////
//COUNT
///////

var var::count(const var& substrx) const
{
	THISIS("var var::count(const var& substrx) const")
	THISISSTRING()
	ISSTRING(substrx)

	std::string substr=substrx.var_str;
	if (substr=="")
		return 0;

	//find the starting position of the field or return ""
	std::string::size_type start_pos=0;
	int fieldno=0;
	while (true)
	{
		start_pos=var_str.find(substr,start_pos);
		//past of of string?
		if (start_pos==std::string::npos)
			return fieldno;
		start_pos++;
		fieldno++;
	}

}

var var::count(const char charx) const
{
	THISIS("var var::count(const char charx) const")
	THISISSTRING()

	//find the starting position of the field or return ""
	std::string::size_type start_pos=0;
	int fieldno=0;
	while (true)
	{
		start_pos=var_str.find_first_of(charx, start_pos);
		//past of of string?
		if (start_pos==std::string::npos)
			return fieldno;
		start_pos++;
		fieldno++;
	}

}

var var::index2(const var& substrx,const int startchar1) const
{
	THISIS("var var::index2(const var& substrx,const int startchar1) const")
	THISISSTRING()
	ISSTRING(substrx)

	//convert to a string for .find
	std::string substr=substrx.var_str;
	if (substr=="")
		return var(0);

	//find the starting position of the field or return ""
	std::string::size_type start_pos=startchar1-1;
	start_pos=var_str.find(substr,start_pos);

	//past of of string?
	if (start_pos==std::string::npos) return var(0);

	return var((int)start_pos+1);

}

var var::index(const var& substrx,const int occurrenceno) const
{
	THISIS("var var::index(const var& substrx,const int occurrenceno) const")
	THISISSTRING()
	ISSTRING(substrx)

	//convert to a string for .find
	std::string substr=substrx.var_str;
	if (substr=="")
		return var(0);

	std::string::size_type start_pos=0;

	//negative and 0th occurrence mean the first
	int countdown=occurrenceno>=1?occurrenceno:1;

	for (;;)
	{

		//find the starting position of the field or return ""
		start_pos=var_str.find(substr,start_pos);

		//past of of string?
		if (start_pos==std::string::npos)
			return 0;

		--countdown;

		//found the right occurrence
		if (countdown==0)
			return ((int)start_pos+1);

		//skip to character after substr (not just next character)
		start_pos+=substr.length();

	}

	//should never get here
	return 0;

}

var var::debug() const
{
	//THISIS("var var::debug() const")

	std::cout<<"var::debug()"<<std::endl<<std::flush;
	//"good way to break into the debugger by causing a seg fault"
    // *(int *)0 = 0;
	//throw var("Debug Statement");

#ifdef raise
	raise(SIGTRAP);
#else
	for(var ii=0; ii<3;ii++) {__asm__("int3");}
#endif

	//throw MVDebug();
	return "";
}

var var::debug(const var& var1) const
{
	//THISIS("var var::debug(const var& var1) const")

	std::cout<<"var::debug()"<<std::endl<<std::flush;
	//good way to break into the debugger by causing a seg fault
	#ifdef _MSC_VER
		//throw var("Debug Statement");
		throw MVDebug(var1);
	#else
		*(int *)0 = 0;
	#endif

	return "";

	//evade warning: unused parameter var1
	if (var1){};
}

var var::logoff() const
{
	//THISIS("var var::logoff() const")
	//THISISSTRING("var.logoff()")

	std::cout<<"var::logoff not implemented yet "<<std::endl;
	return "";
}

var var::abs() const
{
	THISIS("var var::abs() const")
	THISISNUMERIC()

	//prefer double
	if (var_typ & pimpl::VARTYP_DBL)
	{
		if (var_dbl<0)
			return -var_dbl;
		return std::floor(var_dbl);
	}
	else
	{
		if (var_int<0)
			return -var_int;
		return var_int;
	}
	//cannot get here
	throw MVException("abs(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::mod(const var& divisor) const
{
	THISIS("var var::mod(const var& divisor) const")
	THISISNUMERIC()
	ISNUMERIC(divisor)

	//NB using c++ % operator which until c++11 had undefined behaviour if divisor was negative
	//from c++11 % the sign of the result after a negative divisor is always the same as the dividend

	//prefer double dividend
	if (var_typ & pimpl::VARTYP_DBL)
	{
		//prefer double divisor
		if (divisor.var_typ & pimpl::VARTYP_DBL)
		{
			//return fmod(double(var_int),divisor.var_dbl);
			if ((var_dbl<0 && divisor.var_dbl>=0) || (divisor.var_dbl<0 && var_dbl>=0))
				//multivalue version of mod
				return fmod(var_dbl,divisor.var_dbl)+divisor.var_dbl;
			else
				return fmod(var_dbl,divisor.var_dbl);
		}
		else
		{
			divisor.var_dbl=double(divisor.var_int);
			//following would cache the double value but is it worth it?
			//divisor.var_typ=divisor.var_typ & pimpl::VARTYP_DBL;

			if ((var_dbl<0 && divisor.var_int>=0) || (divisor.var_int<0 && var_dbl>=0))
				//multivalue version of mod
				return fmod(var_dbl,divisor.var_dbl)+divisor.var_dbl;
			else
				return fmod(var_dbl,divisor.var_dbl);
		}
	}
	else
	{
		//prefer double divisor
		if (divisor.var_typ & pimpl::VARTYP_DBL)
		{

			var_dbl=double(var_int);
			//following would cache the double value but is it worth it?
			//var_typ=var_typ & pimpl::VARTYP_DBL;

			if ((var_int<0 && divisor.var_dbl>=0) || (divisor.var_dbl<0 && var_int>=0))
				//multivalue version of mod
				return fmod(var_dbl,divisor.var_dbl)+divisor.var_dbl;
			else
				return fmod(var_dbl,divisor.var_dbl);
		}
		else
		{
			if ((var_int<0 && divisor.var_int>=0) || (divisor.var_int<0 && var_int>=0))
				//multivalue version of mod
				return (var_int % divisor.var_int)+divisor.var_int;
			else
				return var_int % divisor.var_int;
		}
	}
	//cannot get here
	throw MVException("mod(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::mod(const int divisor) const
{
	THISIS("var var::mod(const int divisor) const")
	THISISNUMERIC()

	//see ::mod(const var& divisor) for comments about c++11 % operator

	//prefer double dividend
	if (var_typ & pimpl::VARTYP_DBL)
	{
			if ((var_dbl<0 && divisor>=0) || (divisor<0 && var_dbl>=0))
			{
				//multivalue version of mod
				double divisor2=double(divisor);
				return fmod(var_dbl,divisor2)+divisor2;
			}
			else
				return fmod(var_dbl,double(divisor));
	}
	else
	{
			if ((var_int<0 && divisor>=0) || (divisor<0 && var_int>=0))
				//multivalue version of mod
				return (var_int % divisor)+divisor;
			else
				return var_int % divisor;
	}
	//cannot get here
	throw MVException("mod(unknown mvtype=" ^ var(var_typ) ^ ")");
}

/*
	var sqrt() const;
	var loge() const;
*/

var var::sin() const
{
	THISIS("var var::sin() const")
	THISISNUMERIC()

	//prefer double
	if (var_typ & pimpl::VARTYP_DBL)
		return std::sin(var_dbl*M_PI/180);
	else
		return std::sin(double(var_int)*M_PI/180);

	//cannot get here
	throw MVException("sin(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::cos() const
{
	THISIS("var var::cos() const")
	THISISNUMERIC()

	//prefer double
	if (var_typ & pimpl::VARTYP_DBL)
		return std::cos(var_dbl*M_PI/180);
	else
		return std::cos(double(var_int)*M_PI/180);

	//cannot get here
	throw MVException("cos(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::tan() const
{
	THISIS("var var::tan() const")
	THISISNUMERIC()

	//prefer double
	if (var_typ & pimpl::VARTYP_DBL)
		return std::tan(var_dbl*M_PI/180);
	else
		return std::tan(double(var_int)*M_PI/180);

	//cannot get here
	throw MVException("tan(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::atan() const
{
	THISIS("var var::atan() const")
	THISISNUMERIC()

	//prefer double
	if (var_typ & pimpl::VARTYP_DBL)
		return std::atan(var_dbl)/M_PI*180;
	else
		return std::atan(double(var_int))/M_PI*180;

	//cannot get here
	throw MVException("atan(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::loge() const
{
	THISIS("var var::loge() const")
	THISISNUMERIC()

	//prefer double
	if (var_typ & pimpl::VARTYP_DBL)
		return std::log(var_dbl);
	else
		return std::log(double(var_int));

	//cannot get here
	throw MVException("loge(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::sqrt() const
{
	THISIS("var var::sqrt() const")
	THISISNUMERIC()

	//prefer double
	if (var_typ & pimpl::VARTYP_DBL)
		return std::sqrt(var_dbl);

//	if (var_typ & pimpl::VARTYP_INT)
		return std::sqrt(double(var_int));

	throw MVException("sqrt(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::pwr(const var& exponent) const
{
	THISIS("var var::pwr(const var& exponent) const")
	THISISNUMERIC()
	ISNUMERIC(exponent)

	//prefer double
	if (var_typ & pimpl::VARTYP_DBL)
		return std::pow(var_dbl,exponent.toDouble());
	else
		return std::pow(double(var_int),exponent.toDouble());

	//cannot get here
	throw MVException("pow(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::exp() const
{
	THISIS("var var::exp() const")
	THISISNUMERIC()

	//prefer double
	if (var_typ & pimpl::VARTYP_DBL)
		return std::exp(var_dbl);
	else
		return std::exp(double(var_int));

	//cannot get here
	throw MVException("exp(unknown mvtype=" ^ var(var_typ) ^ ")");
}

var var::at(const int column) const
{
	//THISIS("var var::at(const int column) const")

	//hard coded for xterm at the moment
	//http://www.xfree86.org/current/ctlseqs.html

	//move to column 0
	if (column==0)
		//return "\x1b[G";
		return "\r";//works on more terminals

	//move to column
	if (column>0)
	{
		std::string tempstr="\x1B[";
		tempstr+=intToString(column+1);
		tempstr+="G";
		return tempstr;
	}
	//clear the screen and home the cursor
	if (column==-1)
		return "\x1B[2J\x1B[H";
		//return "\x0c";//works on more terminals

	//move the cursor to top left home
	if (column==-2)
		return "\x1B[H";

	//clear from cursor to end of screen
	if (column==-3)
		return "\x1B[J";

	//clear from cursor to end of line
	if (column==-4)
		return "\x1B[K";

	//clear line and move cursor to column 0
	if (column==-40)
		return "\r\x1B[K";

	return "";
}

var var::at(const int column,const int row) const
{
	//THISIS("var var::at(const int column,const int row) const")

	std::string tempstr="\x1B[";
	tempstr+=intToString(row+1);
	tempstr+=";";
	tempstr+=intToString(column+1);
	tempstr+="H";
	return tempstr;
}

var var::getcursor() const
{
	THISIS("var var::getcursor() const")

	return "";

	/* TODO

	//following works EXCEPT displays codes on screen and requires press Enter

	//std::cout<<"var::getcursor() not implemented yet "<<std::endl;

	//http://ascii-table.com/ansi-escape-sequences-vt-100.php
	//esc 6n = get cursor position - DSR
	//"\x1b6n" report cursor position as \x1b rown ; coln R

	//turn echo off perhaps glibc/misc/getpass.c __tcgetattr __tcsetattr
	//...

	//output the magic ansi terminal code
	var("\x1b[6n").output();

	//this->ossleep(100);

	//read back the response - sadly this HANGS until Enter is pressed
	std::string tempstr;
	std::getline(std::cin, tempstr, 'R');

	//convert string to var/string
	return boost::locale::conv::utf_to_utf<char>(tempstr);

	*/
}

void var::setcursor() const
{
	THISIS("void var::setcursor() const")

	THISISDEFINED()

	//std::cout<<"var::getcursor() not implemented yet "<<std::endl;

	return;
}

var var::getprompt() const
{
	THISIS("var var::getprompt() const")
	THISISDEFINED()

	std::cout<<"var::getprompt() not implemented yet "<<std::endl;
	return "";
}

void var::setprompt() const
{
	THISIS("void var::setprompt() const")
	THISISDEFINED()

	std::cout<<"var::setprompt() not implemented yet "<<std::endl;
	return;
}


var var::xlate(const var& filename,const var& fieldno, const var& mode) const
{
	THISIS("var var::xlate(const var& filename,const var& fieldno, const var& mode) const")
	ISSTRING(mode)

	return xlate(filename,fieldno,mode.var_str.c_str());
}

//TODO provide a version with int fieldno to handle the most frequent case
// although may also support dictid (of target file) instead of fieldno

var var::xlate(const var& filename,const var& fieldno, const char* mode) const
{
	THISIS("var var::xlate(const var& filename,const var& fieldno, const char* mode) const")
	THISISSTRING()
	ISSTRING(filename)
	//until we support fieldnames ISSTRING(fieldno)
	ISNUMERIC(fieldno)

	//open the file (skip this for now since sql doesnt need "open"
	var file;
	//if (!file.open(filename))
	//{
	//	_STATUS=filename^" does not exist";
	//	record="";
	//	return record;
	//}
	file=filename;

	var sep;
	if (fieldno.length())
		sep=_VM_;
	else
		sep=_RM_;

	var response="";
	var nmv=(*this).dcount(_VM_);
	for (var vn=1;vn<=nmv;++vn) {

		if (vn>1)
			response^=sep;

		//read the record
		var key=(*this).a(1,vn);
		var record;
		if (!record.reado(file,key))
		{
			//if record doesnt exist then "", or original key if mode is "C"

			//no record and mode C returns the key
			//gcc warning: comparison with string literal results in unspecified behaviour
			//if (mode=="C")
			if (*mode==*"C")
				response^=*this;

			//no record and mode X or anything else returns ""
			continue;
		}

		//extract the field or field 0 means return the whole record
		if (fieldno) {

			//numeric fieldno not zero return field
			//if (fieldno.isnum())

			//throw non-numeric error if fieldno not numeric
			response ^= record.a(fieldno);

			//non-numeric fieldno - cannot call calculate from here
			//return calculate(fieldno,filename,mode);
			continue;

		}

		//fieldno "" returns whole record
		if (!fieldno.length()) {
			response^=record;
			continue;
		}

		//field no 0 returns key
		response^=key;

	}

	return response;

}

} // namespace exodus
