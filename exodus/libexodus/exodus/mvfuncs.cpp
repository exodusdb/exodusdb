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
//prevent implicit conversion from cstr to wstring
//so code must be like var x=L"xyz" instead of just "xyz"
//to perhaps prevent conversion at runtime to the var wstring
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
	return var(__DATE__).iconv(L"D").oconv(L"D")^L" "^var(__TIME__);
}

bool var::eof() const
{
	//THISIS(L"bool var::eof() const")
	//THISISDEFINED()

	return (std::cin.eof());
}

//for nchars, use int instead of var to trigger error at point of calling not here
bool var::input(const var& prompt, const int nchars)
{
	THISIS(L"bool var::inputl(const var& prompt, const int nchars")
	THISISDEFINED()
	ISSTRING(prompt)

	//TODO implement nchars including -1

	if (prompt.length()) {
		prompt.output();
		std::cout<<std::flush;
	}
	if (nchars<0) {
		var_str=L"";
		var_typ=pimpl::VARTYP_STR;
		//while (true) {
			int nc;
var(L"before peek").outputl();
			nc=std::cin.peek();
var(L"after peek").outputl();

var(nc).oconv(L"HEX").outputl(L"peek=");
var(EOF).oconv(L"HEX").outputl(L"EOF=");
//			if (nc==EOF)
//				break;
var(EOF).oconv(L"HEX").outputl();
var(nc).oconv(L"HEX").outputl();
			//TODO consider converting from say utf8
			char char1;
var(L"aaaa").outputl();
			std::cin.get(char1);
var(L"bbb").outputl();
			var_str+=char1;
		//}
var(L"ccc").outputl();
		return true;
	}

	return input();

	//evade warning: unused parameter
	if (nchars) {};
}

bool var::input()
{
	THISIS(L"bool var::inputl()")
	THISISDEFINED()

	std::string tempstr;

	if (std::cin.eof())
	{
		var_str=L"";
		var_typ=pimpl::VARTYP_STR;
		return false;
	}

	std::getline(std::cin,tempstr);

	//pressing crtl+d indicates eof on unix or ctrl+Z on dos/windows?
	if (std::cin.eof())
	{
		var_str=L"";
		var_typ=pimpl::VARTYP_STR;
		return false;
	}

	//convert from utf8 to internal format - utf16 or utf32 depending on platform
	//var_str=wstringfromUTF8((UTF8*)tempstr.data(),(int)tempstr.length());
	var_str=boost::locale::conv::utf_to_utf<wchar_t>(tempstr);
	var_typ=pimpl::VARTYP_STR;

	return true;
}

void var::stop(const var& text) const
{
	THISIS(L"void var::stop(const var& text) const")
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
	THISIS(L"void var::abort(const var& text) const")
	ISSTRING(text)

	//exit(1);
	throw MVAbort(text);
}

bool var::assigned() const
{
	//THISIS(L"bool var::assigned() const")

	//treat undefined as unassigned
	//undefined is a state where we are USING the variable before its contructor has been called!
	//which is possible (in syntax like var xx.osread()?)
	//and also when passing default variables to functions in the functors on gcc
	//THISISDEFINED()
	if (!this||(*this).var_typ&mvtypemask)
		return false;

	return var_typ!=pimpl::VARTYP_UNA;
}

bool var::unassigned() const
{
	//THISIS(L"bool var::unassigned() const")

	//see explanation above in assigned
	//THISISDEFINED()
	if (!this||((*this).var_typ&mvtypemask))
		return true;

	return !var_typ;
}

//pick int() is actually the same as floor. using integer() instead of int() because int is a reserved word in c/c++ for int datatype
var var::integer() const
{
	THISIS(L"var var::integer() const")
	THISISNUMERIC()

	/*pick integer means floor()
	-1.0=1
	-0.9=1
	-0.5=1
	-0.1=1
	 0  =0
	 0.1=0
	 0.5=0
	 0.9=0
	 1.0=1
	*/

	if (var_typ&pimpl::VARTYP_INT)
		return var_int;

	//could save the integer conversion here but would require mentality to save BOTH int and double
	//currently its possible since space is reserved for both but this may change

	//floor
	return std::floor(var_dbl);

}

//integer and floor are the same
var var::floor() const
{
	THISIS(L"var var::floor() const")
	THISISNUMERIC()

	if (var_typ&pimpl::VARTYP_INT)
		return var_int;

	//could save the integer conversion here but would require mentality to save BOTH int and double
	//currently its possible since space is reserved for both but this may change

	//floor
	return std::floor(var_dbl);

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

	THISIS(L"var var::round() const")
	THISISNUMERIC()

	double result;
	if (var_typ&pimpl::VARTYP_INT)
	{
		if (not ndecimals)
			return var_int;
		//loss of precision if var_int is long long
		result=int(var_int);
	}
	else
		result=var_dbl;

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
	THISIS(L"bool var::toBool() const")
	//could be skipped for speed assuming that people will not write unusual "var x=f(x)" type syntax as follows:
	//var xx=xx?11:22;
	THISISDEFINED()

	//identical code in void* and bool except returns void* and bool respectively
	do
	{
		//doubles are true unless zero
		//check double first dbl on guess that tests will be most often on financial numbers
		if (var_typ&pimpl::VARTYP_DBL)
			return (bool)(var_dbl!=0);

		//ints are true except for zero
		if (var_typ&pimpl::VARTYP_INT)
			return (bool)(var_int!=0);

		//non-numeric strings are true unless zero length
		if (var_typ&pimpl::VARTYP_NAN)
			return (bool)(var_str.length()!=0);

		if (!(var_typ))
		{
			THISISASSIGNED()
			throw MVUnassigned(L"toBool()");
		}

		//must be string - try to convert to numeric and do all tests again
		isnum();

	} while (true);

}

int var::toInt() const
{
	THISIS(L"int var::toInt() const")
	THISISNUMERIC()

	//loss of precision if var_int is long long
	return (var_typ&pimpl::VARTYP_INT) ? int(var_int) : int(var_dbl);
}

int var::toLong() const
{
	THISIS(L"int var::toLong() const")
	THISISNUMERIC()

	//loss of precision if var_int is long long
	return (var_typ&pimpl::VARTYP_INT) ? long(var_int) : long(var_dbl);
}

double var::toDouble() const
{
	THISIS(L"double var::toDouble() const")
	THISISNUMERIC()

	return (var_typ&pimpl::VARTYP_INT) ? double(var_int) : var_dbl;
}

void var::createString() const
{
	THISIS(L"void var::createString() const")
	//TODO ensure ISDEFINED is called everywhere in advance
	//to avoid wasting time doing multiple calls to ISDEFINED
	THISISDEFINED()

	switch (var_typ)
	{
		//skip this test for speed and only call the function if required
		//str already. no action required
		//put this first guessing that toWString is most often called on strings;
		//case pimpl::VARTYP_STR:
		//	return;

		//int - create string from int
		case pimpl::VARTYP_INT:
			//loss of precision if var_int is long long
			var_str=intToString(int(var_int));
			break;

		//dbl - create string from dbl
		case pimpl::VARTYP_DBL:
			var_str=dblToString(var_dbl);
			break;

		//unassigned - throw
		case pimpl::VARTYP_UNA:
			throw MVUnassigned(L"createString()");

		default:
			//arrives here only after int or dbl has been converted to string
			//add the "string available" bit flag using the "bitor equals" operator
			var_typ|=pimpl::VARTYP_STR;
	}

}

std::wstring var::toWString() const
{
	THISIS(L"std::wstring var::toWString() const")
	THISISSTRING()

	return var_str;
}

//DLL_PUBLIC
std::string var::toString2() const
{
        if (var_typ&mvtypemask)
                return "";
        return toString();
}

//DLL_PUBLIC
std::string var::toString() const
{
	THISIS(L"std::string var::toString() const")
	THISISSTRING()
	return boost::locale::conv::utf_to_utf<char>(var_str);
}

var var::length() const
{
	THISIS(L"var var::length() const")
	THISISSTRING()

	return int(var_str.length());
}

//synonym for length for compatibility with pick's len()
var var::len() const
{
	THISIS(L"var var::len() const")
	THISISSTRING()

	return int(var_str.length());
}

const wchar_t* var::data() const
{
	THISIS(L"const wchar_t* var::data() const")
	THISISSTRING()

	return var_str.data();
}


var var::trim(const var& trimchar) const
{
	THISIS(L"var var::trim(const var& trimchar) const")
	ISSTRING(trimchar)

	return trim(trimchar.var_str.c_str());
}

var var::trim(const var& trimchar, const var& options) const
{
	THISIS(L"var var::trim(const var& trimchar, const var& options) const")
	ISSTRING(trimchar)
	ISSTRING(options)

	if (options==L"F") {
		return trimf(trimchar.var_str.c_str());
	}
	else if (options==L"B") {
		return trimb(trimchar.var_str.c_str());
	}
	else if (options==L"FB") {
		return trimf(trimchar.var_str.c_str()).trimb(trimchar.var_str.c_str());
	}
	return trim(trimchar.var_str.c_str());
}

var& var::trimmer(const var& trimchar)
{
	THISIS(L"var& var::trimmer(const var& trimchar)")
	ISSTRING(trimchar)

	return trimmer(trimchar.var_str.c_str());
}

var& var::trimmer(const var& trimchar, const var& options)
{
	THISIS(L"var var::trimmer(const var& trimchar, const var& options) const")
	ISSTRING(trimchar)
	ISSTRING(options)

	if (options==L"F") {
		return trimmerf(trimchar.var_str.c_str());
	}
	else if (options==L"B") {
		return trimmerb(trimchar.var_str.c_str());
	}
	else if (options==L"FB") {
		return trimmerf(trimchar.var_str.c_str()).trimmerb(trimchar.var_str.c_str());
	}
	return trimmer(trimchar.var_str.c_str());
}

var var::trimf(const var& trimchar) const
{
	THISIS(L"var var::trimf(const var& trimchar) const")
	ISSTRING(trimchar)

	return trimf(trimchar.var_str.c_str());
}

var& var::trimmerf(const var& trimchar)
{
	THISIS(L"var& var::trimmerf(const var& trimchar)")
	ISSTRING(trimchar)

	return trimmerf(trimchar.var_str.c_str());
}

var var::trimb(const var& trimchar) const
{
	THISIS(L"var var::trimb(const var& trimchar) const")
	ISSTRING(trimchar)

	return trimb(trimchar.var_str.c_str());
}

var& var::trimmerb(const var& trimchar)
{
	THISIS(L"var& var::trimmerb(const var& trimchar)")
	ISSTRING(trimchar)

	return trimmerb(trimchar.var_str.c_str());
}

var var::trimf(const wchar_t* trimchar) const
{
	THISIS(L"var var::trimf(const wchar_t* trimchar) const")
	THISISSTRING()

	return var(*this).trimmerf(trimchar);
}

var& var::trimmerf(const wchar_t* trimchar)
{
	THISIS(L"var& var::trimmerf(const wchar_t* trimchar)")
	THISISSTRING()

	std::wstring::size_type start_pos;
	start_pos=var_str.find_first_not_of(trimchar);

	if (start_pos==std::wstring::npos)
	{
		*this=L"";
		return *this;
	}

	//return var(var_str.substr(start_pos));
	var_str.erase(0,start_pos);
	return *this;

}

var var::trimb(const wchar_t* trimchar) const
{
	THISIS(L"var var::trimb(const wchar_t* trimchar) const")
	THISISSTRING()

	return var(*this).trimmerb(trimchar);
}

var& var::trimmerb(const wchar_t* trimchar)
{
	THISIS(L"var& var::trimmerb(const wchar_t* trimchar)")
	THISISSTRING()

	std::wstring::size_type end_pos;
	end_pos=var_str.find_last_not_of(trimchar);

	if (end_pos==std::wstring::npos)
	{
		*this=L"";
		return *this;
	}

	//return var(var_str.substr(0,end_pos+1));
	var_str.erase(end_pos+1);

	return *this;

}


var var::trim(const wchar_t* trimchar) const
{
	THISIS(L"var var::trim(const wchar_t* trimchar) const")
	THISISSTRING()

	return var(*this).trimmer(trimchar);
}

var& var::trimmer(const wchar_t* trimchar)
{

	//reimplement with boost string trim_if algorithm
	//http://www.boost.org/doc/libs/1_39_0/doc/html/string_algo/reference.html

	THISIS(L"var& var::trimmer(const wchar_t* trimchar)")
	THISISSTRING()

	//find the first non blank
	std::wstring::size_type start_pos;
	start_pos=var_str.find_first_not_of(trimchar);

	//if all blanks return empty string
	if (start_pos==std::wstring::npos)
	{
		*this=L"";
		return *this;
	}

	//erase leading spaces
	if (start_pos!=0) var_str.erase(0,start_pos);

	//find the last non blank
	std::wstring::size_type end_pos;
	//end_pos=var_str.find_last_not_of(trimchar);
	end_pos=var_str.find_last_not_of(trimchar,var_str.length()-1);

	//erase trailing spaces
	var_str.erase(end_pos+1);

	//find the starting position of any embedded spaces
	start_pos=std::wstring::npos;
	while (true)
	{
		//find a space
		start_pos=var_str.find_last_of(trimchar,start_pos);

		//if no (more) spaces then return the string
		if (start_pos==std::wstring::npos||start_pos<=0) return *this;

		//find the first non-space thereafter
		end_pos=var_str.find_last_not_of(trimchar,start_pos-1);

		//if first non space character is not one before the space
		if (end_pos<start_pos-1)
		{
			var_str.erase(end_pos+1,start_pos-end_pos-1);
		}
		if (end_pos<=0) break;
		start_pos=end_pos-1;
	}
	return *this;

}

var var::invert() const
{
	THISIS(L"var& var::invert()")
	THISISSTRING()
	//return var(*this).inverter();
	var tt=*this;
	tt.inverter();
	return tt;
}

var& var::inverter()
{
	THISIS(L"var& var::inverter()")
	THISISSTRING()

	//wchar_t invertbits=-1;
	//invert only the lower 8 bits to keep the resultant characters within the the same unicode page
	wchar_t invertbits=255;
	for (size_t ii = 0; ii < var_str.size(); ii++)
		//xor each w_char with the bits we want to toggle
        	var_str[ii] = var_str[ii] ^ invertbits;
}

var var::ucase() const
{
	return var(*this).ucaser();
}

var& var::ucaser()
{
	THISIS(L"var& var::ucaser()")
	THISISSTRING()

	return localeAwareChangeCase(2);
}

var var::lcase() const
{
	return var(*this).lcaser();
}

var& var::lcaser()
{
	THISIS(L"var& var::lcaser()")
	THISISSTRING()

	return localeAwareChangeCase(1);
}

var var::unique() const
{
	THISIS(L"var var::unique()")
	THISISSTRING()

        //linemark
        var result = L"";
        var start = 0;
	var bit,term,xx;
        while (true) {
                bit=(*this).remove(start, term);
                if (bit.length()) {
                        if (not(result.locateusing(bit, VM, xx))) {
                                result ^= bit ^ VM;
                        }
                }
		if (not term)
			break;
        }//loop;
        result.splicer(-1, 1, L"");
        return result;

}

var var::seq() const
{
	THISIS(L"var var::seq() const")
	THISISSTRING()

	if (var_str.length()==0) return 0;

	return (int) (unsigned int) var_str[0];

}

var var::chr(const int int1) const
{
	//doesnt use *this at all (do we need a version that does?)

	return var((wchar_t) int1);
}

var var::quote() const
{
	THISIS(L"var var::quote() const")
	THISISSTRING()

	return var(*this).quoter();
}

var& var::quoter()
{
	THISIS(L"var& var::quoter()")
	THISISSTRING()

	//NB this is std::wstring "replace" not var field replace
	var_str.replace(0,0,L"\"");
	var_str+=L'"';
	return *this;

}

var var::squote() const
{
	THISIS(L"var var::squote() const")
	THISISSTRING()

	return var(*this).squoter();
}

var& var::squoter()
{
	THISIS(L"var& var::squoter()")
	THISISSTRING()

	//NB this is std::wstring "replace" not var field replace
	var_str.replace(0,0,L"'");
	var_str+=L'\'';
	return *this;

}

var var::unquote() const
{
	THISIS(L"var var::unquote() const")
	THISISSTRING()

	return var(*this).unquoter();
}

var& var::unquoter()
{
	THISIS(L"var& var::unquoter()")
	THISISSTRING()

	//removes MATCHING beginning and terminating " or ' characters
	//also removes a SINGLE " or ' on the grounds that you probably want to eliminate all such characters

	//no change if no length
	size_t len=var_str.length();
	if (!len)
		return *this;

	wchar_t char0=var_str[0];

	//no change if not starting " or '
	if (char0!=L'\"' && char0!=L'\'')
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
	THISIS(L"var var::splice(const int start1,const int length,const var& newstr) const")
	THISISSTRING()

	return var(*this).splicer(start1,length,newstr);
}

var var::splice(const int start1,const var& newstr) const
{
	THISIS(L"var var::splice(const int start1,const var& newstr) const")
	THISISSTRING()

	return var(*this).splicer(start1,newstr);
}

var& var::splicer(const int start1,const int length,const var& newstr)
{
	THISIS(L"var& var::splicer(const int start1,const int length,const var& newstr)")
	THISISSTRING()
	ISSTRING(newstr)

	//prepare a new var
	//functionmode var newmv=var(var_str);
	//proceduremode

//TODO make sure start and length work like REVELATION and HANDLE NEGATIVE LENGTH!
	int start1b;
	if (start1>0) start1b=start1;
	else if (start1<0)
    {
        start1b=int(var_str.length())+start1+1;
        if (start1b<1) start1b=1;
    }
	else start1b=1;

	int lengthb;
	if (length>=0) lengthb=length;
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

	var_str.replace(start1b-1,lengthb,newstr.var_str);

	return *this;

}

var& var::splicer(const int start1, const var& newstr)
{
	THISIS(L"var& var::splicer(const int start1, const var& newstr)")
	THISISSTRING()
	ISSTRING(newstr)

	//prepare a new var
	//functionmode var newmv=var(var_str);
	//proceduremode

//TODO make sure start and length work like REVELATION and HANDLE NEGATIVE LENGTH!
	int start1b;
	if (start1>0) start1b=start1;
	else if (start1<0)
    {
        start1b=int(var_str.length())+start1+1;
        if (start1b<1) start1b=1;
    }
	else start1b=1;

	var_str.replace(start1b-1,var_str.length(),newstr.var_str);

	return *this;

}

var& var::transfer(var& var2)
{
	THISIS(L"var& var::transfer(var& var2)")
	//transfer even unassigned vars (but not uninitialised ones)
	THISISDEFINED()
	ISDEFINED(var2)

	var_str.swap(var2.var_str);
	var2.var_typ=var_typ;
	var2.var_int=var_int;
	var2.var_dbl=var_dbl;

	var_str=L"";
	var_int=0;
	var_typ=pimpl::VARTYP_INTSTR;

	return var2;
}

var& var::clone(var& var2)
{
	THISIS(L"var& var::clone(var& var2)")
	//clone even unassigned vars (but not uninitialised ones)
	THISISDEFINED()
	ISDEFINED(var2)

	var_typ=var2.var_typ;
	var_str=var2.var_str;
	var_int=var2.var_int;
	var_dbl=var2.var_dbl;

	return var2;
}

//kind of const needed in calculatex
const var& var::exchange(const var& var2) const
{
	THISIS(L"var& var::exchange(var& var2)")
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
	THISIS(L"var var::str(const int num) const")
	THISISSTRING()

	var newstr=L"";
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
	THISIS(L"var var::space() const")
	THISISNUMERIC()

	var newstr=L"";
	int nspaces=(*this).round().toInt();
	if (nspaces>0)
		newstr.var_str.resize(nspaces,L' ');

	return newstr;
}

var var::crop() const
{
	THISIS(L"var var::crop() const")
	THISISSTRING()

	return var(*this).cropper();
}

var& var::cropper()
{
	THISIS(L"var& var::cropper()")
	THISISSTRING()

	//aaaFbbbFVSSccc
	//aaaFbbbFVSSccc

	//aaaFbbbFVSSFRdd
	//aaaFbbb     Rdd

	//considered several strategies

	//scan forwards and copy to a new string
	//could not work out an simple efficient algorithm
	//the only simple algorithm was to always check the last characters on the growing new string
	//and if like VM VM when adding FM then trim the last characters
	//this wouldnt be efficient in the common case of many excess VMs since then would be all added and then all trimmed

	//scan backwards and mark characters to be deleted and then copy only the non-marked characters to a new string
	//problem to decide what character to use as deletion character - if hardcoded it would prevent that forever from being used in data
	//otherwise to scan the whole string for a non-occuring character to use as a deletion character

	//scan backwards copying the characters (backwards) to a new string skipping the VM in the case of VM FM sequences
	//at the end IF there have been any skips then reverse the string into the result

	//a LAZY version that only copies to newstr what necessary (imagine a string with trailing marks and a few fields to be cropped from the middle 
    //xxxxxxCxxxCxxxxCxxxxCyyyyyyyyyyyyyyyZZZ
	//newstr will only contain the xxxxxxxxxxxxxx bit (reversed)
	//yyyyyyyyyyyy is the tail of the original string which hasnt any cropping done in it but EXCLUDING any trailing marks
	//to get the full cropped string back you need to
	//1) reverse(newstr) ie xxx
	//2) concat yyy ie originalstring.substr(croppoint onwards excluding zzz)

	std::wstring newstr=L"";

	std::wstring::reverse_iterator iter=var_str.rbegin();
	std::wstring::reverse_iterator iterend=var_str.rend();

	//move the REVERSE iterator backwards to the first non-field character
	int ntrailing2trim=0;
	while (iter!=iterend && (*iter)>=SSTM_ && (*iter)<=RM_)
	{
		++ntrailing2trim;
		++iter;
	}

	//all separator characters - return empty string
	if (iter == iterend)
	{
		var_str=newstr;
		return *this;
	}

	int ncropped=0;

	do
	{
		wchar_t lastchar=*iter;

		if (ncropped)
			newstr+=lastchar;

		++iter;

		//skip over lower preceeding separators (ge VM before FM)
		//remember we are iterating in REVERSER
		//so ++iter moves to the character to the LEFT not right
		do
		{
			if (iter == iterend )
				goto cropperexit;

			if ((*iter) < SSTM_ || (*iter) >= lastchar )
				break;

			++ncropped;

			++iter;

		} while (true);

	} while (true);

cropperexit:

	//IF anything was cropped then reverse the new string back into the beginning of the old string
	//and delete the gap
	if (ncropped)
	{
		std::wstring::reverse_iterator newiter=newstr.rbegin();
		std::wstring::reverse_iterator newiterend=newstr.rend();
		std::wstring::iterator olditer=var_str.begin();

		while (newiter!=newiterend)
			(*olditer++)=(*newiter++);

		var_str.erase(newstr.length(),ncropped);

	}

	if (ntrailing2trim)
		var_str.erase(var_str.length()-ntrailing2trim);

	return *this;
}

var var::lower() const
{
	THISIS(L"var var::lower() const")
	THISISSTRING()

	return var(*this).lowerer();
}

var& var::lowerer()
{
	THISIS(L"var& var::lowerer()")
	THISISSTRING()

	converter(_RM_ _FM_ _VM_ _SM_ _TM_ _STM_,
			  _FM_ _VM_ _SM_ _TM_ _STM_ _SSTM_);

	return *this;
}

var var::raise() const
{
	THISIS(L"var var::raise() const")
	THISISSTRING()

	return var(*this).raiser();
}

var& var::raiser()
{
	THISIS(L"var& var::raiser()")
	THISISSTRING()

	converter(_FM_ _VM_ _SM_ _TM_ _STM_ _SSTM_,
			  _RM_ _FM_ _VM_ _SM_ _TM_ _STM_);

	return *this;
}

var var::convert(const var& oldchars,const var& newchars) const
{
	THISIS(L"var var::convert(const var& oldchars,const var& newchars) const")
	THISISSTRING()

	return var(*this).converter(oldchars,newchars);
}

var& var::converter(const var& oldchars,const var& newchars)
{
	THISIS(L"var& var::converter(const var& oldchars,const var& newchars)")
	THISISSTRING()
	ISSTRING(oldchars)
	ISSTRING(newchars)

	std::wstring::size_type
	pos=std::wstring::npos;

	while (true)
	{
		//locate (backwards) any of the from characters
		pos=var_str.find_last_of(oldchars.var_str,pos);

		if (pos==std::wstring::npos) break;

		//find which from character we have found
		int fromcharn=int(oldchars.var_str.find(var_str[pos]));

		if (fromcharn<int(newchars.var_str.length()))
		 var_str.replace(pos,1,newchars.var_str.substr(fromcharn,1));
		else
		 var_str.erase(pos,1);

		if (pos==0) break;
		pos--;
	}

	return *this;

}

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
	THISIS(L"bool var::isnum(void) const")
	//TODO make isnum private and ensure ISDEFINED is checked before all calls to isnum
	//to save the probably double check here
	THISISDEFINED()

	//is numeric already
	if (var_typ&pimpl::VARTYP_INTDBL)
		return true;

	//is known not numeric already
	//maybe put this first if comparison operations on strings are more frequent than numeric operations on numbers
	if (var_typ&pimpl::VARTYP_NAN)
		return false;

	//not assigned error
	if (!var_typ)
		throw MVUnassigned(L"isnum()");

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
		wchar_t cc=var_str[ii];
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
				case L'.':
					if (point)	// 2nd point - is non-numeric
					{
						var_typ=pimpl::VARTYP_NANSTR;
						return false;
					}
					point=true;
					break;

				case L'-':
				case L'+':
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
			wchar_t bigletter;
			char smalletters[sizeof(wchar_t)/sizeof(char)];
		} abcd;
		abcd.bigletter=0x0035;
		std::cout<<atoi(abcd.smalletters)<<std::endl;
		*/

		// http://www.globalyzer.com/gi/help/gihelp/unsafeMethod/atof.htm :
		// "... _wtof is supported only on Windows platforms"
		// "... Recommended Replacements: ANSI UTF-16 wcstod"
///		std::string result(var_str.begin(),var_str.end());
///		var_dbl=atof(result.c_str());
		var_dbl=wcstod(var_str.c_str(), 0);
		var_typ=pimpl::VARTYP_DBLSTR;
	}
	else
	{
		//var_int=_wtoi(var_str.c_str());
		//TODO optimise

		//change from
///		std::string result(var_str.begin(),var_str.end());
///		var_int=atoi(result.c_str());
		var_int=wcstol(var_str.c_str(), 0, 10);
		var_typ=pimpl::VARTYP_INTSTR;
	}
	//indicate isNumeric
	return true;
}

/*
bool var::isnum_old(void) const
{
	THISIS(L"bool var::isnum(void) const")
	//TODO make isnum private and ensure ISDEFINED is checked before all calls to isnum
	//to save the probably double check here
	THISISDEFINED()

	//is numeric already
	if (var_typ&pimpl::VARTYP_INTDBL)
		return true;

	//is known not numeric already
	//maybe put this first if comparison operations on strings are more frequent than numeric operations on numbers
	if (var_typ&pimpl::VARTYP_NAN)
		return false;

	//not assigned error
	if (!var_typ)
		throw MVUnassigned(L"isnum()");

	//if not a number and not unassigned then it is an unknown string

	bool point=false;
	bool digit=false;

	//otherwise find test
	//a number is optional minus followed by digits and max of one decimal point
	//NB going backwards - for speed?
	//TODO: check if going backwards is slow for variable width multi-byte character strings
	//and replace with forward scanning
	for (int ii=(int)var_str.length()-1;ii>=0;--ii)
	{

		wchar_t cc=var_str[ii];

		//check > 9 first on guess that non-numeric string characters will be after '9'
		//to evade the || < '0' test as well
		//note: high end characters appear negative and crash isdigit
		//could this be solved by defining wchar_t as unsigned wchar_t?
//			if (!isdigit(cc))
		if (cc>'9'||cc<'0')
		{

			//check non-digit characters
			switch (cc)
			{
				case L'.':

					//more than one point is non-numeric
					if (point) return false;

					//indicate point has been found
					point=true;
					break;

				case L'-':
				case L'+':

					//non-numeric if +/- is not the first character or is the only character
					//if (ii) goto nan;
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
		else
			digit=true;
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

		//union switchitup
		//{
		//	wchar_t bigletter;
		//	char smalletters[sizeof(wchar_t)/sizeof(char)];
		//} abcd;
		//abcd.bigletter=0x0035;
		//std::cout<<atoi(abcd.smalletters)<<std::endl;

		std::string result(var_str.begin(),var_str.end());
		var_dbl=atof(result.c_str());
		var_typ=pimpl::VARTYP_DBLSTR;
	}
	else		//ALN:TODO: long int wcstol (const wchar_t *restrict string, wchar_t **restrict tailptr, int base)
	{			//ALN:TODO: ... should be OK here

		//var_int=_wtoi(var_str.c_str());
		//TODO optimise

		//change from
		std::string result(var_str.begin(),var_str.end());
		var_int=atoi(result.c_str());

		//change to something like this? //ALN:TODO: finish and test !

		//wchar_t * err_char;
		//var_int=wcstol(result.c_str(), & err_char, 10);
		//if (

		var_typ=pimpl::VARTYP_INTSTR;
	}

	//indicate isNumeric
	return true;

}*/

/////////
// output
/////////

const var& var::put(std::ostream& ostream1) const
{
	THISIS(L"const var& var::put(std::ostream& ostream1) const")
	THISISSTRING()

	//prevent output to wcout suppressing output to cout (by non-exodus routines)
	//http://gcc.gnu.org/ml/gcc-bugs/2006-05/msg01196.html
	//TODO optimise by calling once instead of every call to output()
	if (!desynced_with_stdio)
	{
		std::ios::sync_with_stdio(false);
		desynced_with_stdio=true;
	}

	//verify conversion to UTF8
	std::string tempstr=(*this).toString();
	ostream1.write(tempstr.data(),(std::streamsize) tempstr.length());
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
//TODO make a char and wchar_t version for speed
var var::dcount(const var& substrx) const
{
	THISIS(L"var var::dcount(const var& substrx) const")
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
	THISIS(L"var var::count(const var& substrx) const")
	THISISSTRING()
	ISSTRING(substrx)

	std::wstring substr=substrx.var_str;
	if (substr==L"")
		return 0;

	//find the starting position of the field or return L""
	std::wstring::size_type start_pos=0;
	int fieldno=0;
	while (true)
	{
		start_pos=var_str.find(substr,start_pos);
		//past of of string?
		if (start_pos==std::wstring::npos)
			return fieldno;
		start_pos++;
		fieldno++;
	}

}

var var::count(const wchar_t charx) const
{
	THISIS(L"var var::count(const char charx) const")
	THISISSTRING()

	//find the starting position of the field or return L""
	std::wstring::size_type start_pos=0;
	int fieldno=0;
	while (true)
	{
		start_pos=var_str.find_first_of(charx, start_pos);
		//past of of string?
		if (start_pos==std::wstring::npos)
			return fieldno;
		start_pos++;
		fieldno++;
	}

}

var var::index2(const var& substrx,const int startchar1) const
{
	THISIS(L"var var::index2(const var& substrx,const int startchar1) const")
	THISISSTRING()
	ISSTRING(substrx)

	//convert to a string for .find
	std::wstring substr=substrx.var_str;
	if (substr==L"")
		return var(0);

	//find the starting position of the field or return L""
	std::wstring::size_type start_pos=startchar1-1;
	start_pos=var_str.find(substr,start_pos);

	//past of of string?
	if (start_pos==std::wstring::npos) return var(0);

	return var((int)start_pos+1);

}

var var::index(const var& substrx,const int occurrenceno) const
{
	THISIS(L"var var::index(const var& substrx,const int occurrenceno) const")
	THISISSTRING()
	ISSTRING(substrx)

	//convert to a string for .find
	std::wstring substr=substrx.var_str;
	if (substr==L"")
		return var(0);

	std::wstring::size_type start_pos=0;

	//negative and 0th occurrence mean the first
	int countdown=occurrenceno>=1?occurrenceno:1;

	for (;;)
	{

		//find the starting position of the field or return L""
		start_pos=var_str.find(substr,start_pos);

		//past of of string?
		if (start_pos==std::wstring::npos)
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

//moved to mvprogram
//var var::perform() const
//{
//	THISIS(L"var var::perform() const")
//	THISISSTRING()

//	std::cout<<"var::perform not implemented yet "<<toString()<<std::endl;
//	return L"";
//}

//moved to mvprogram
//var var::execute() const
//{
//	THISIS(L"var var::execute() const")
//	THISISSTRING()
//
//	std::cout<<"var::execute not implemented yet. perform instead"<<toString()<<std::endl;
//	return (*this).perform();
//	return L"";
//}

//var var::chain() const
//{
//	THISIS(L"var var::chain() const")
//	THISISSTRING()
//
//	std::cout<<"var::chain not implemented yet "<<toString()<<std::endl;
//	return L"";
//}

var var::debug() const
{
	//THISIS(L"var var::debug() const")

	std::cout<<"var::debug()"<<std::endl<<std::flush;
	//"good way to break into the debugger by causing a seg fault"
    // *(int *)0 = 0;
	//throw var(L"Debug Statement");

#ifdef raise
	raise(SIGTRAP);
#endif

	throw MVDebug();
	return L"";
}

var var::debug(const var& var1) const
{
	//THISIS(L"var var::debug(const var& var1) const")

	std::wcout<<L"var::debug()"<<std::endl<<std::flush;
	//good way to break into the debugger by causing a seg fault
	#ifdef _MSC_VER
		//throw var(L"Debug Statement");
		throw MVDebug(var1);
	#else
		*(int *)0 = 0;
	#endif

	return L"";

	//evade warning: unused parameter var1
	if (var1){};
}

var var::logoff() const
{
	//THISIS(L"var var::logoff() const")
	//THISISSTRING("var.logoff()")

	std::cout<<"var::logoff not implemented yet "<<std::endl;
	return L"";
}

var var::abs() const
{
	THISIS(L"var var::abs() const")
	THISISNUMERIC()

	if (var_typ&pimpl::VARTYP_INT)
	{
		if (var_int<0) return -var_int;
		return var_int;
	}
	if (var_typ&pimpl::VARTYP_DBL)
	{
		if (var_dbl<0) return -var_dbl;
		return std::floor(var_dbl);
	}
	throw MVException(L"abs(unknown mvtype=" ^ var(var_typ) ^ L")");
}

var var::mod(const var& divisor) const
{
	THISIS(L"var var::mod(const var& divisor) const")
	THISISNUMERIC()
	ISNUMERIC(divisor)

	if (var_typ&pimpl::VARTYP_INT)
	{
		if (divisor.var_typ&pimpl::VARTYP_INT)
		{
			if ((var_int<0 && divisor.var_int>=0) || (divisor.var_int<0 && var_int>=0))
				//multivalue version of mod
				return (var_int%divisor.var_int)+divisor.var_int;
			else
				return var_int%divisor.var_int;
		}
		else
		{

			var_dbl=double(var_int);
			//following would cache the double value but is it worth it?
			//var_typ=var_typ&pimpl::VARTYP_DBL;

			if ((var_int<0 && divisor.var_dbl>=0) || (divisor.var_dbl<0 && var_int>=0))
				//multivalue version of mod
				return fmod(var_dbl,divisor.var_dbl)+divisor.var_dbl;
			else
				return fmod(var_dbl,divisor.var_dbl);
		}
	}
	else
	{
		if (divisor.var_typ&pimpl::VARTYP_INT)
		{
			divisor.var_dbl=double(divisor.var_int);
			//following would cache the double value but is it worth it?
			//divisor.var_typ=divisor.var_typ&pimpl::VARTYP_DBL;

			if ((var_dbl<0 && divisor.var_int>=0) || (divisor.var_int<0 && var_dbl>=0))
				//multivalue version of mod
				return fmod(var_dbl,divisor.var_dbl)+divisor.var_dbl;
			else
				return fmod(var_dbl,divisor.var_dbl);
		}
		else
		{
			//return fmod(double(var_int),divisor.var_dbl);
			if ((var_dbl<0 && divisor.var_dbl>=0) || (divisor.var_dbl<0 && var_dbl>=0))
				//multivalue version of mod
				return fmod(var_dbl,divisor.var_dbl)+divisor.var_dbl;
			else
				return fmod(var_dbl,divisor.var_dbl);
		}
	}
	//throw MVException(L"abs(unknown mvtype=" ^ var(var_typ) ^ L")");
}

var var::mod(const int divisor) const
{
	THISIS(L"var var::mod(const int divisor) const")
	THISISNUMERIC()

	if (var_typ&pimpl::VARTYP_INT)
	{
			if ((var_int<0 && divisor>=0) || (divisor<0 && var_int>=0))
				//multivalue version of mod
				return (var_int%divisor)+divisor;
			else
				return var_int%divisor;
	}
	else
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
	//throw MVException(L"abs(unknown mvtype=" ^ var(var_typ) ^ L")");
}

/*
	var sqrt() const;
	var loge() const;
*/

var var::sin() const
{
	THISIS(L"var var::sin() const")
	THISISNUMERIC()

	if (var_typ&pimpl::VARTYP_DBL)
		return std::sin(var_dbl*M_PI/180);

//	if (var_typ&pimpl::VARTYP_INT)
		return std::sin(double(var_int)*M_PI/180);

	throw MVException(L"sin(unknown mvtype=" ^ var(var_typ) ^ L")");
}

var var::cos() const
{
	THISIS(L"var var::cos() const")
	THISISNUMERIC()

	if (var_typ&pimpl::VARTYP_DBL)
		return std::cos(var_dbl*M_PI/180);

//	if (var_typ&pimpl::VARTYP_INT)
		return std::cos(double(var_int)*M_PI/180);

	throw MVException(L"cos(unknown mvtype=" ^ var(var_typ) ^ L")");
}

var var::tan() const
{
	THISIS(L"var var::tan() const")
	THISISNUMERIC()

	if (var_typ&pimpl::VARTYP_DBL)
		return std::tan(var_dbl*M_PI/180);

//	if (var_typ&pimpl::VARTYP_INT)
		return std::tan(double(var_int)*M_PI/180);

	throw MVException(L"tan(unknown mvtype=" ^ var(var_typ) ^ L")");
}

var var::atan() const
{
	THISIS(L"var var::atan() const")
	THISISNUMERIC()

	if (var_typ&pimpl::VARTYP_DBL)
		return std::atan(var_dbl)/M_PI*180;

//	if (var_typ&pimpl::VARTYP_INT)
		return std::atan(double(var_int))/M_PI*180;

	throw MVException(L"sin(unknown mvtype=" ^ var(var_typ) ^ L")");
}

var var::loge() const
{
	THISIS(L"var var::loge() const")
	THISISNUMERIC()

	if (var_typ&pimpl::VARTYP_DBL)
		return std::log(var_dbl);

//	if (var_typ&pimpl::VARTYP_INT)
		return std::log(double(var_int));

	throw MVException(L"loge(unknown mvtype=" ^ var(var_typ) ^ L")");
}

var var::sqrt() const
{
	THISIS(L"var var::sqrt() const")
	THISISNUMERIC()

	if (var_typ&pimpl::VARTYP_DBL)
		return std::sqrt(var_dbl);

//	if (var_typ&pimpl::VARTYP_INT)
		return std::sqrt(double(var_int));

	throw MVException(L"sqrt(unknown mvtype=" ^ var(var_typ) ^ L")");
}

var var::pwr(const var& exponent) const
{
	THISIS(L"var var::pwr(const var& exponent) const")
	THISISNUMERIC()
	ISNUMERIC(exponent)

	if (var_typ&pimpl::VARTYP_DBL)
		return std::pow(var_dbl,exponent.toDouble());

//	if (var_typ&pimpl::VARTYP_INT)
		return std::pow(double(var_int),exponent.toDouble());

	throw MVException(L"pow(unknown mvtype=" ^ var(var_typ) ^ L")");
}

var var::exp() const
{
	THISIS(L"var var::exp() const")
	THISISNUMERIC()

	if (var_typ&pimpl::VARTYP_DBL)
		return std::exp(var_dbl);

//	if (var_typ&pimpl::VARTYP_INT)
		return std::exp(double(var_int));

	throw MVException(L"exp(unknown mvtype=" ^ var(var_typ) ^ L")");
}

var var::at(const int column) const
{
	//THISIS(L"var var::at(const int column) const")

	//hard coded for xterm at the moment
	//http://www.xfree86.org/current/ctlseqs.html

	//move to column 0
	if (column==0)
		//return "\x1b[G";
		return L"\r";//works on more terminals

	//move to column
	if (column>0)
	{
		std::wstring tempstr=L"\x1B[";
		tempstr+=intToString(column+1);
		tempstr+=L"G";
		return tempstr;
	}
	//clear the screen and home the cursor
	if (column==-1)
		return L"\x1B[2J\x1B[H";
		//return L"\x0c";//works on more terminals

	//move the cursor to top left home
	if (column==-2)
		return L"\x1B[H";

	//clear from cursor to end of screen
	if (column==-3)
		return L"\x1B[J";

	//clear from cursor to end of line
	if (column==-4)
		return L"\x1B[K";

	//clear line and move cursor to column 0
	if (column==-40)
		return L"\r\x1B[K";

	return L"";
}

var var::at(const int column,const int row) const
{
	//THISIS(L"var var::at(const int column,const int row) const")

	std::wstring tempstr=L"\x1B[";
	tempstr+=intToString(row+1);
	tempstr+=L";";
	tempstr+=intToString(column+1);
	tempstr+=L"H";
	return tempstr;
}

var var::getcursor() const
{
	THISIS(L"var var::getcursor() const")

	return L"";

	/* TODO

	//following works EXCEPT displays codes on screen and requires press Enter

	//std::cout<<"var::getcursor() not implemented yet "<<std::endl;

	//http://ascii-table.com/ansi-escape-sequences-vt-100.php
	//esc 6n = get cursor position - DSR
	//"\x1b6n" report cursor position as \x1b rown ; coln R

	//turn echo off perhaps glibc/misc/getpass.c __tcgetattr __tcsetattr
	//...

	//output the magic ansi terminal code
	var(L"\x1b[6n").output();

	//this->ossleep(100);

	//read back the response - sadly this HANGS until Enter is pressed
	std::string tempstr;
	std::getline(std::cin, tempstr, 'R');

	//convert string to var/wstring
	return boost::locale::conv::utf_to_utf<wchar_t>(tempstr);

	*/
}

void var::setcursor() const
{
	THISIS(L"void var::setcursor() const")

	THISISDEFINED()

	//std::cout<<"var::getcursor() not implemented yet "<<std::endl;

	return;
}

var var::getprompt() const
{
	THISIS(L"var var::getprompt() const")
	THISISDEFINED()

	std::cout<<"var::getprompt() not implemented yet "<<std::endl;
	return L"";
}

void var::setprompt() const
{
	THISIS(L"void var::setprompt() const")
	THISISDEFINED()

	std::cout<<"var::setprompt() not implemented yet "<<std::endl;
	return;
}


var var::xlate(const var& filename,const var& fieldno, const var& mode) const
{
	THISIS(L"var var::xlate(const var& filename,const var& fieldno, const var& mode) const")
	ISSTRING(mode)

	return xlate(filename,fieldno,mode.var_str.c_str());
}

//TODO provide a version with int fieldno to handle the most frequent case
// although may also support dictid (of target file) instead of fieldno

var var::xlate(const var& filename,const var& fieldno, const wchar_t* mode) const
{
	THISIS(L"var var::xlate(const var& filename,const var& fieldno, const wchar_t* mode) const")
	THISISSTRING()
	ISSTRING(filename)
	//until we support fieldnames ISSTRING(fieldno)
	ISNUMERIC(fieldno)

	//open the file (skip this for now since sql doesnt need "open"
	var file;
	//if (!file.open(filename))
	//{
	//	_STATUS=filename^L" does not exist";
	//	record=L"";
	//	return record;
	//}
	file=filename;

	var sep;
	if (fieldno.length())
		sep=_VM_;
	else
		sep=_RM_;

	var response=L"";
	var nmv=(*this).dcount(_VM_);
	for (var vn=1;vn<=nmv;++vn) {

		if (vn>1)
			response^=sep;

		//read the record
		var key=(*this).a(1,vn);
		var record;
		if (!record.read(file,key))
		{
			//if record doesnt exist then "", or original key if mode is "C"

			//no record and mode C returns the key
			//gcc warning: comparison with string literal results in unspecified behaviour
			//if (mode==L"C")
			if (*mode==*L"C")
				response^=*this;

			//no record and mode X or anything else returns ""
			continue;
		}

		//extract the field or field 0 means return the whole record
		if (fieldno) {

			//numeric fieldno not zero return field
			//if (fieldno.isnum())
				response ^= record.a(fieldno);

			//non-numeric fieldno do calculate
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
