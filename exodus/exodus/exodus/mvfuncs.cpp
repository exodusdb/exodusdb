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

//for high performance exodus library code
//prevent implicit conversion from cstr to wstring
//so code must be like var x=L"xyz" instead of just "xyz"
//to perhaps prevent conversion at runtime to the var wstring
//optimising compilers may make this 
#define MV_NO_NARROW

//C4530: C++ exception handler used, but unwind semantics are not enabled. 
#pragma warning (disable: 4530)

#include <iostream> //cin and cout
#include <cmath> //for floor
#include <cstdlib>//for exit

#ifndef M_PI
//#define M_PI 3.14159265358979323846f
#define M_PI 3.14159265
//2643383279502884197169399375105820974944592307816406286208998f;
#endif

#include <exodus/mvimpl.h>
#include <exodus/mv.h>
#include <exodus/mvutf.h>
#include <exodus/mvexceptions.h>

//std::ios::sync_with_stdio(false);
bool desynced_with_stdio=false;

//TODO check that all string increase operations dont crash the system

namespace exodus {

bool var::eof() const
{
	THISIS(L"bool var::eof() const")
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
	return input();
}

bool var::input()
{
	THISIS(L"bool var::inputl()")
	THISISDEFINED()

	std::string tempstr;

	if (std::cin.eof())
	{
		var_mvstr=L"";
		var_mvtyp=pimpl::MVTYPE_STR;
		return false;
	}

	std::getline(std::cin,tempstr);

	//pressing crtl+d indicates eof on unix or ctrl+Z on dos/windows?
	if (std::cin.eof())
	{
		var_mvstr=L"";
		var_mvtyp=pimpl::MVTYPE_STR;
		return false;
	}

	//convert from utf8 to internal format - utf16 or utf32 depending on platform
	var_mvstr=wstringfromUTF8((UTF8*)tempstr.data(),(int)tempstr.length());
	var_mvtyp=pimpl::MVTYPE_STR;

	return true;
}

void var::stop(const var& text) const
{
	THISIS(L"void var::stop(const var& text) const")
	ISDEFINED(text)

	if (text != L"")
	text.outputl();
	exit(0);
}

void var::abort(const var& text) const
{
	THISIS(L"void var::abort(const var& text) const")
	ISDEFINED(text)

	if (text != L"")
	text.outputl();
	exit(1);
}

bool var::assigned() const
{
	THISIS(L"bool var::assigned() const")

	//treat undefined as unassigned
	//undefined is a state where we are USING the variable before its contructor has been called!
	//which is possible (in syntax like var xx.osread()?)
	//and also when passing default variables to functions in the functors on gcc
	//THISISDEFINED()
	if (!this||(*this).var_mvtyp&mvtypemask)
		return false;

	return var_mvtyp!=pimpl::MVTYPE_UNA;
}

bool var::unassigned() const
{
	THISIS(L"bool var::unassigned() const")

	//see explanation above in assigned
	//THISISDEFINED()
	if (!this||((*this).var_mvtyp&mvtypemask))
		return true;

	return !var_mvtyp;
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

	if (var_mvtyp&pimpl::MVTYPE_INT)
		return var_mvint;

	//could save the integer conversion here but would require mentality to save BOTH int and double
	//currently its possible since space is reserved for both but this may change

	//floor
	return std::floor(var_mvdbl);

}

//integer and floor are the same
var var::floor() const
{
	THISIS(L"var var::floor() const")
	THISISNUMERIC()

	if (var_mvtyp&pimpl::MVTYPE_INT)
		return var_mvint;

	//could save the integer conversion here but would require mentality to save BOTH int and double
	//currently its possible since space is reserved for both but this may change

	//floor
	return std::floor(var_mvdbl);

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
	if (var_mvtyp&pimpl::MVTYPE_INT)
	{
		if (not ndecimals)
			return var_mvint;
		//loss of precision if var_mvint is long long
		result=int(var_mvint);
	}
	else
		result=var_mvdbl;

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
		if (var_mvtyp&pimpl::MVTYPE_DBL)
			return (bool)(var_mvdbl!=0);

		//ints are true except for zero
		if (var_mvtyp&pimpl::MVTYPE_INT)
			return (bool)(var_mvint!=0);

		//non-numeric strings are true unless zero length
		if (var_mvtyp&pimpl::MVTYPE_NAN)
			return (bool)(var_mvstr.length()!=0);

		if (!(var_mvtyp))
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

	//loss of precision if var_mvint is long long
	return (var_mvtyp&pimpl::MVTYPE_INT) ? int(var_mvint) : int(var_mvdbl);
}

double var::toDouble() const
{
	THISIS(L"double var::toDouble() const")
	THISISNUMERIC()

	return (var_mvtyp&pimpl::MVTYPE_INT) ? double(var_mvint) : var_mvdbl;
}

void var::createString() const
{
	THISIS(L"void var::createString() const")
	//TODO ensure ISDEFINED is called everywhere in advance
	//to avoid wasting time doing multiple calls to ISDEFINED
	THISISDEFINED()

	switch (var_mvtyp)
	{
		//skip this test for speed and only call the function if required
		//str already. no action required
		//put this first guessing that towstring is most often called on strings;
		//case pimpl::MVTYPE_STR:
		//	return;

		//int - create string from int
		case pimpl::MVTYPE_INT:
			//loss of precision if var_mvint is long long
			var_mvstr=intToString(int(var_mvint));
			break;

		//dbl - create string from dbl
		case pimpl::MVTYPE_DBL:
			var_mvstr=dblToString(var_mvdbl);
			break;

		//unassigned - throw
		case pimpl::MVTYPE_UNA:
			throw MVUnassigned(L"createString()");

		default:
			//arrives here only after int or dbl has been converted to string
			//add the "string available" bit flag using the "bitor equals" operator
			var_mvtyp|=pimpl::MVTYPE_STR;
	}

}

std::wstring var::towstring() const
{
	THISIS(L"std::wstring var::towstring() const")
	THISISSTRING()

	return var_mvstr;
}

var var::length() const
{
	THISIS(L"var var::length() const")
	THISISSTRING()

	return int(var_mvstr.length());
}

//synonym for length for compatibility with pick's len()
var var::len() const
{
	THISIS(L"var var::len() const")
	THISISSTRING()

	return int(var_mvstr.length());
}

const wchar_t* var::data() const
{
	THISIS(L"const wchar_t* var::data() const")
	THISISSTRING()

	return var_mvstr.data();
}


var var::trim(const var trimchar) const
{
	THISIS(L"var var::trim(const var trimchar) const")
	ISSTRING(trimchar)
	
	return trim(trimchar.var_mvstr.c_str());
}

var& var::trimmer(const var trimchar)
{
	THISIS(L"var& var::trimmer(const var trimchar)")
	ISSTRING(trimchar)
	
	return trimmer(trimchar.var_mvstr.c_str());
}

var var::trimf(const var trimchar) const
{
	THISIS(L"var var::trimf(const var trimchar) const")
	ISSTRING(trimchar)
	
	return trimf(trimchar.var_mvstr.c_str());
}

var& var::trimmerf(const var trimchar)
{
	THISIS(L"var& var::trimmerf(const var trimchar)")
	ISSTRING(trimchar)
	
	return trimmerf(trimchar.var_mvstr.c_str());
}

var var::trimb(const var trimchar) const
{
	THISIS(L"var var::trimb(const var trimchar) const")
	ISSTRING(trimchar)

	return trimb(trimchar.var_mvstr.c_str());
}

var& var::trimmerb(const var trimchar)
{
	THISIS(L"var& var::trimmerb(const var trimchar)")
	ISSTRING(trimchar)
	
	return trimmerb(trimchar.var_mvstr.c_str());
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
	start_pos=var_mvstr.find_first_not_of(trimchar);

	if (start_pos==std::wstring::npos)
	{
		*this=L"";
		return *this;
	}

	//return var(var_mvstr.substr(start_pos));
	var_mvstr.erase(0,start_pos);
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
	end_pos=var_mvstr.find_last_not_of(trimchar);

	if (end_pos==std::wstring::npos)
	{
		*this=L"";
		return *this;
	}

	//return var(var_mvstr.substr(0,end_pos+1));
	var_mvstr.erase(end_pos+1);

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
	start_pos=var_mvstr.find_first_not_of(trimchar);

	//if all blanks return empty string
	if (start_pos==std::wstring::npos)
	{
		*this=L"";
		return *this;
	}

	//erase leading spaces
	if (start_pos!=0) var_mvstr.erase(0,start_pos);

	//find the last non blank
	std::wstring::size_type end_pos;
	//end_pos=var_mvstr.find_last_not_of(trimchar);
	end_pos=var_mvstr.find_last_not_of(trimchar,var_mvstr.length()-1);

	//erase trailing spaces
	var_mvstr.erase(end_pos+1);

	//find the starting position of any embedded spaces
	start_pos=std::wstring::npos;
	while (true)
	{
		//find a space
		start_pos=var_mvstr.find_last_of(trimchar,start_pos);

		//if no (more) spaces then return the string
		if (start_pos==std::wstring::npos||start_pos<=0) return *this;

		//find the first non-space thereafter
		end_pos=var_mvstr.find_last_not_of(trimchar,start_pos-1);

		//if first non space character is not one before the space
		if (end_pos<start_pos-1)
		{
			var_mvstr.erase(end_pos+1,start_pos-end_pos-1);
		}
		if (end_pos<=0) break;
		start_pos=end_pos-1;
	}
	return *this;

}

var var::ucase() const
{
	THISIS(L"var var::ucase() const")
	THISISSTRING()

	return var(*this).ucaser();
}

var& var::ucaser()
{
	THISIS(L"var& var::ucaser()")
	THISISSTRING()

	//TODO unicode/native conversion
	converter(LOWER_CASE, _UPPER_CASE);
	return *this;
}

var var::lcase() const
{
	THISIS(L"var var::lcase() const")
	THISISSTRING()

	return var(*this).lcaser();
}

var& var::lcaser()
{
	THISIS(L"var& var::lcaser()")
	THISISSTRING()

	//TODO unicode/native conversion
	converter(_UPPER_CASE,LOWER_CASE);
	return *this;
}

/*
var var::invert() const
{
	THISIS(L"var var::invert() const")
	THISISSTRING()

	return var(*this).inverter();
}

var& var::inverter()
{
	THISIS(L"var& var::inverter()")
	THISISSTRING()

	//TODO IMPLEMENT as xor with -1
	//or perhaps with with binary 2^16 ie 1111111111111111 to retain top two bytes of UT32
	//so that it doesnt go totally illegal
//	throw MVNotImplemented(L"inverter()");

    return *this;

}
*/

var var::seq() const
{
	THISIS(L"var var::seq() const")
	THISISSTRING()

	if (var_mvstr.length()==0) return 0;

	return (int) (unsigned int) var_mvstr[0];

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
	var_mvstr.replace(0,0,L"\"");
	var_mvstr+=L'"';
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
	size_t len=var_mvstr.length();
	if (!len)
		return *this;

	wchar_t char0=var_mvstr[0];

	//no change if not starting " or '
	if (char0!=L'\"' && char0!=L'\'')
		return *this;

	//no change if terminating character ne starting character
	if (var_mvstr[len-1]!=char0)
		return *this;

	//erase first (and last character if more than one)
	var_mvstr.erase(0,1);
	if (len)
		var_mvstr.erase(len-2,1);

	return *this;

}

var var::splice(const int start1,const int length,const var& newstr) const
{
	THISIS(L"var var::splice(const int start1,const int length,const var& newstr) const")
	THISISSTRING()

	return var(*this).splicer(start1,length,newstr);
}

var& var::splicer(const int start1,const int length,const var& newstr)
{
	THISIS(L"var& var::splicer(const int start1,const int length,const var& newstr)")
	THISISSTRING()
	ISSTRING(newstr)

	//prepare a new var
	//functionmode var newmv=var(var_mvstr);
	//proceduremode

//TODO make sure start and length work like REVELATION and HANDLE NEGATIVE LENGTH!
	int start1b;
	if (start1>0) start1b=start1;
	else if (start1<0)
    {
        start1b=int(var_mvstr.length())+start1+1;
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

	var_mvstr.replace(start1b-1,lengthb,newstr.var_mvstr);

	return *this;

}

var& var::transfer(var& var2)
{
	THISIS(L"var& var::transfer(var& var2)")
	//transfer even unassigned vars (but not uninitialised ones)
	THISISDEFINED()
	ISDEFINED(var2)

	var_mvstr.swap(var2.var_mvstr);
	var2.var_mvtyp=var_mvtyp;
	var2.var_mvint=var_mvint;
	var2.var_mvdbl=var_mvdbl;

	var_mvstr=L"";
	var_mvint=0;
	var_mvtyp=pimpl::MVTYPE_INTSTR;

	return var2;
}

var& var::clone(var& var2)
{
	THISIS(L"var& var::clone(var& var2)")
	//clone even unassigned vars (but not uninitialised ones)
	THISISDEFINED()
	ISDEFINED(var2)

	var_mvtyp=var2.var_mvtyp;
	var_mvstr=var2.var_mvstr;
	var_mvint=var2.var_mvint;
	var_mvdbl=var2.var_mvdbl;

	return var2;
}

var& var::exchange(var& var2)
{
	THISIS(L"var& var::exchange(var& var2)")
	//exchange even unassigned vars (but not uninitialised ones)
	THISISDEFINED()
	ISDEFINED(var2)

	//intermediary copies of var2
	int mvtypex=var2.var_mvtyp;
	mvint_t mvintx=var2.var_mvint;
	double mvdblx=var2.var_mvdbl;

	//do string first since it is the largest and most likely to fail
	var_mvstr.swap(var2.var_mvstr);

	//copy mv to var2
	var2.var_mvtyp=var_mvtyp;
	var2.var_mvint=var_mvint;
	var2.var_mvdbl=var_mvdbl;

	//copy intermediaries to mv
	var_mvtyp=mvtypex;
	var_mvint=mvintx;
	var_mvdbl=mvdblx;

	return var2;
}

var var::str(const int num) const
{
	THISIS(L"var var::str(const int num) const")
	THISISSTRING()

	var newstr=L"";
	if (num<0)
		return newstr;

	int basestrlen=int(var_mvstr.length());
	if (basestrlen==1)
		newstr.var_mvstr.resize(num,var_mvstr.at(0));
	else if (basestrlen)
		for (int ii=num;ii>0;--ii)
			newstr^=var_mvstr;
	
	return newstr;
}

var var::space() const
{
	THISIS(L"var var::space() const")
	THISISNUMERIC()

	var newstr=L"";
	int nspaces=(*this).round().toInt();
	if (nspaces>0)
		newstr.var_mvstr.resize(nspaces,L' ');

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

	std::wstring::reverse_iterator iter=var_mvstr.rbegin();
	std::wstring::reverse_iterator iterend=var_mvstr.rend();

	//move the REVERSE iterator backwards to the first non-field character
	int ntrailing2trim=0;
	while ( iter!=iterend && (*iter)>=SSTM_ && (*iter)<=RM_)
	{
		++ntrailing2trim;
		++iter;
	}

	//all separator characters - return empty string
	if (iter == iterend)
	{
		var_mvstr=newstr;
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
			if ( iter == iterend )
				goto cropperexit;

			if ( (*iter) < SSTM_ || (*iter) >= lastchar )
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
		std::wstring::iterator olditer=var_mvstr.begin();

		while (newiter!=newiterend)
			(*olditer++)=(*newiter++);

		var_mvstr.erase(newstr.length(),ncropped);

	}

	if (ntrailing2trim)
		var_mvstr.erase(var_mvstr.length()-ntrailing2trim);
	
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

	//converter(L"\xFF\xFE\xFD\xFC\xFB\xFA",L"\xFE\xFD\xFC\xFB\xFA\xF9");
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

		//converter(L"\xFE\xFD\xFC\xFB\xFA\xF9",L"\xFF\xFE\xFD\xFC\xFB\xFA");
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
		pos=var_mvstr.find_last_of(oldchars.var_mvstr,pos);

		if (pos==std::wstring::npos) break;

		//find which from character we have found
		int fromcharn=int(oldchars.var_mvstr.find(var_mvstr[pos]));

		if (fromcharn<int(newchars.var_mvstr.length()))
		 var_mvstr.replace(pos,1,newchars.var_mvstr.substr(fromcharn,1));
		else
		 var_mvstr.erase(pos,1);

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
	if (var_mvtyp&pimpl::MVTYPE_INTDBL)
		return true;

	//is known not numeric already
	//maybe put this first if comparison operations on strings are more frequent than numeric operations on numbers
	if (var_mvtyp&pimpl::MVTYPE_NAN)
		return false;

	//not assigned error
	if (!var_mvtyp)
		throw MVUnassigned(L"isnum()");

	//if not a number and not unassigned then it is an unknown string

	bool point=false;
	bool digit=false;

	//otherwise find test
	//a number is optional minus followed by digits and max of one decimal point
	//NB going backwards - for speed?
	//TODO: check if going backwards is slow for variable width multi-byte character strings
	//and replace with forward scanning
	for (int ii=(int)var_mvstr.length()-1;ii>=0;--ii)
	{

		wchar_t cc=var_mvstr[ii];

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
						var_mvtyp=pimpl::MVTYPE_NANSTR;
						return false;
					}
					break;

				//any other character mean non-numeric
				default:
					var_mvtyp=pimpl::MVTYPE_NANSTR;
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
		if (var_mvstr.length())
		// goto nan;
		{
			var_mvtyp=pimpl::MVTYPE_NANSTR;
			return false;
		}

		//zero length string is integer 0
		var_mvint=0;
		var_mvtyp=pimpl::MVTYPE_INTSTR;
		return true;

	}

	//get and save the number as int or double
	if (point)
	{
		//var_mvdbl=_wtof(var_mvstr.c_str());
		//var_mvdbl=_wtof(var_mvstr.c_str());
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

		std::string result(var_mvstr.begin(),var_mvstr.end());
		var_mvdbl=atof(result.c_str());
		var_mvtyp=pimpl::MVTYPE_DBLSTR;
	}
	else		//ALN:TODO: long int wcstol (const wchar_t *restrict string, wchar_t **restrict tailptr, int base)
	{			//ALN:TODO: ... should be OK here
		//var_mvint=_wtoi(var_mvstr.c_str());
		//var_mvdbl=_wtof(var_mvstr.c_str());
		//TODO optimise
		std::string result(var_mvstr.begin(),var_mvstr.end());
		//var_mvdbl=atof(result.c_str());
		var_mvint=atoi(result.c_str());
		var_mvtyp=pimpl::MVTYPE_INTSTR;
	}

	//indicate isNumeric
	return true;

}

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
	std::string tempstr=(*this).tostring();
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
	return put(std::clog);
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

	if (var_mvstr.length()==0)
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

	std::wstring substr=substrx.var_mvstr;
	if (substr==L"")
		return 0;

	//find the starting position of the field or return L""
	std::wstring::size_type start_pos=0;
	int fieldno=0;
	while (true)
	{
		start_pos=var_mvstr.find(substr,start_pos);
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
		start_pos=var_mvstr.find_first_of(charx, start_pos);
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
	std::wstring substr=substrx.var_mvstr;
	if (substr==L"")
		return var(0);

	//find the starting position of the field or return L""
	std::wstring::size_type start_pos=startchar1-1;
	start_pos=var_mvstr.find(substr,start_pos);

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
	std::wstring substr=substrx.var_mvstr;
	if (substr==L"")
		return var(0);

	std::wstring::size_type start_pos=0;

	//negative and 0th occurrence mean the first
	int countdown=occurrenceno>=1?occurrenceno:1;

	for (;;)
	{

		//find the starting position of the field or return L""
		start_pos=var_mvstr.find(substr,start_pos);

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

var var::perform() const
{
	THISIS(L"var var::perform() const")
	THISISSTRING()

	std::cout<<"var::perform not implemented yet "<<tostring()<<std::endl;
	return L"";
}

var var::chain() const
{
	THISIS(L"var var::chain() const")
	THISISSTRING()

	std::cout<<"var::chain not implemented yet "<<tostring()<<std::endl;
	return L"";
}

var var::execute() const
{
	THISIS(L"var var::execute() const")
	THISISSTRING()

	std::cout<<"var::execute not implemented yet "<<tostring()<<std::endl;
	return L"";
}

var var::debug() const
{
	THISIS(L"var var::debug() const")

	std::cout<<"var::debug()"<<std::endl<<std::flush;
	//"good way to break into the debugger by causing a seg fault"
    //*(int *)0 = 0;
	//throw var(L"Debug Statement");
	throw MVDebug();
	return L"";
}

var var::debug(const var& var1) const
{
	THISIS(L"var var::debug(const var& var1) const")

	std::wcout<<L"var::debug()"<<std::endl<<std::flush;
	//good way to break into the debugger by causing a seg fault
	#ifdef _MSC_VER
		//throw var(L"Debug Statement");
		throw MVDebug(var1);
	#else
		*(int *)0 = 0;
	#endif

	return L"";
}

var var::logoff() const
{
	THISIS(L"var var::logoff() const")
	//THISISSTRING("var.logoff()")

	std::cout<<"var::logoff not implemented yet "<<std::endl;
	return L"";
}

var var::abs() const
{
	THISIS(L"var var::abs() const")
	THISISNUMERIC()

	if (var_mvtyp&pimpl::MVTYPE_INT)
	{
		if (var_mvint<0) return -var_mvint;
		return var_mvint;
	}
	if (var_mvtyp&pimpl::MVTYPE_DBL)
	{
		if (var_mvdbl<0) return -var_mvdbl;
		return std::floor(var_mvdbl);
	}
	throw MVException(L"abs(unknown mvtype=" ^ var(var_mvtyp) ^ L")");
}

var var::mod(const var& divisor) const
{
	THISIS(L"var var::mod(const var& divisor) const")
	THISISNUMERIC()
	ISNUMERIC(divisor)

	if (var_mvtyp&pimpl::MVTYPE_INT)
	{
		if (divisor.var_mvtyp&pimpl::MVTYPE_INT)
		{
			if ((var_mvint<0 && divisor.var_mvint>=0) || (divisor.var_mvint<0 && var_mvint>=0))
				//multivalue version of mod
				return (var_mvint%divisor.var_mvint)+divisor.var_mvint;
			else
				return var_mvint%divisor.var_mvint;
		}
		else
		{

			var_mvdbl=double(var_mvint);
			//following would cache the double value but is it worth it?
			//var_mvtyp=var_mvtyp&pimpl::MVTYPE_DBL;

			if ((var_mvint<0 && divisor.var_mvdbl>=0) || (divisor.var_mvdbl<0 && var_mvint>=0))
				//multivalue version of mod
				return fmod(var_mvdbl,divisor.var_mvdbl)+divisor.var_mvdbl;
			else
				return fmod(var_mvdbl,divisor.var_mvdbl);
		}
	}
	else
	{
		if (divisor.var_mvtyp&pimpl::MVTYPE_INT)
		{
			divisor.var_mvdbl=double(divisor.var_mvint);
			//following would cache the double value but is it worth it?
			//divisor.var_mvtyp=divisor.var_mvtyp&pimpl::MVTYPE_DBL;

			if ((var_mvdbl<0 && divisor.var_mvint>=0) || (divisor.var_mvint<0 && var_mvdbl>=0))
				//multivalue version of mod
				return fmod(var_mvdbl,divisor.var_mvdbl)+divisor.var_mvdbl;
			else
				return fmod(var_mvdbl,divisor.var_mvdbl);
		}
		else
		{
			//return fmod(double(var_mvint),divisor.var_mvdbl);
			if ((var_mvdbl<0 && divisor.var_mvdbl>=0) || (divisor.var_mvdbl<0 && var_mvdbl>=0))
				//multivalue version of mod
				return fmod(var_mvdbl,divisor.var_mvdbl)+divisor.var_mvdbl;
			else
				return fmod(var_mvdbl,divisor.var_mvdbl);
		}
	}
	//throw MVException(L"abs(unknown mvtype=" ^ var(var_mvtyp) ^ L")");
}

var var::mod(const int divisor) const
{
	THISIS(L"var var::mod(const int divisor) const")
	THISISNUMERIC()

	if (var_mvtyp&pimpl::MVTYPE_INT)
	{
			if ((var_mvint<0 && divisor>=0) || (divisor<0 && var_mvint>=0))
				//multivalue version of mod
				return (var_mvint%divisor)+divisor;
			else
				return var_mvint%divisor;
	}
	else
	{
			if ((var_mvdbl<0 && divisor>=0) || (divisor<0 && var_mvdbl>=0))
			{
				//multivalue version of mod
				double divisor2=double(divisor);
				return fmod(var_mvdbl,divisor2)+divisor2;
			}
			else
				return fmod(var_mvdbl,double(divisor));
	}
	//throw MVException(L"abs(unknown mvtype=" ^ var(var_mvtyp) ^ L")");
}

/*
	var sqrt() const;
	var loge() const;
*/

var var::sin() const
{
	THISIS(L"var var::sin() const")
	THISISNUMERIC()

	if (var_mvtyp&pimpl::MVTYPE_DBL)
		return std::sin(var_mvdbl*M_PI/180);

//	if (var_mvtyp&pimpl::MVTYPE_INT)
		return std::sin(double(var_mvint)*M_PI/180);

	throw MVException(L"sin(unknown mvtype=" ^ var(var_mvtyp) ^ L")");
}

var var::cos() const
{
	THISIS(L"var var::cos() const")
	THISISNUMERIC()

	if (var_mvtyp&pimpl::MVTYPE_DBL)
		return std::cos(var_mvdbl*M_PI/180);

//	if (var_mvtyp&pimpl::MVTYPE_INT)
		return std::cos(double(var_mvint)*M_PI/180);

	throw MVException(L"cos(unknown mvtype=" ^ var(var_mvtyp) ^ L")");
}

var var::tan() const
{
	THISIS(L"var var::tan() const")
	THISISNUMERIC()

	if (var_mvtyp&pimpl::MVTYPE_DBL)
		return std::tan(var_mvdbl*M_PI/180);

//	if (var_mvtyp&pimpl::MVTYPE_INT)
		return std::tan(double(var_mvint)*M_PI/180);

	throw MVException(L"tan(unknown mvtype=" ^ var(var_mvtyp) ^ L")");
}

var var::atan() const
{
	THISIS(L"var var::atan() const")
	THISISNUMERIC()

	if (var_mvtyp&pimpl::MVTYPE_DBL)
		return std::atan(var_mvdbl*M_PI/180);

//	if (var_mvtyp&pimpl::MVTYPE_INT)
		return std::atan(double(var_mvint)*M_PI/180);

	throw MVException(L"sin(unknown mvtype=" ^ var(var_mvtyp) ^ L")");
}

var var::loge() const
{
	THISIS(L"var var::loge() const")
	THISISNUMERIC()

	if (var_mvtyp&pimpl::MVTYPE_DBL)
		return std::log(var_mvdbl);

//	if (var_mvtyp&pimpl::MVTYPE_INT)
		return std::log(double(var_mvint));

	throw MVException(L"loge(unknown mvtype=" ^ var(var_mvtyp) ^ L")");
}

var var::sqrt() const
{
	THISIS(L"var var::sqrt() const")
	THISISNUMERIC()

	if (var_mvtyp&pimpl::MVTYPE_DBL)
		return std::sqrt(var_mvdbl);

//	if (var_mvtyp&pimpl::MVTYPE_INT)
		return std::sqrt(double(var_mvint));

	throw MVException(L"sqrt(unknown mvtype=" ^ var(var_mvtyp) ^ L")");
}

var var::pwr(const var& exponent) const
{
	THISIS(L"var var::pwr(const var& exponent) const")
	THISISNUMERIC()
	ISNUMERIC(exponent)

	if (var_mvtyp&pimpl::MVTYPE_DBL)
		return std::pow(var_mvdbl,exponent.toDouble());

//	if (var_mvtyp&pimpl::MVTYPE_INT)
		return std::pow(double(var_mvint),exponent.toDouble());

	throw MVException(L"pow(unknown mvtype=" ^ var(var_mvtyp) ^ L")");
}

var var::exp() const
{
	THISIS(L"var var::exp() const")
	THISISNUMERIC()

	if (var_mvtyp&pimpl::MVTYPE_DBL)
		return std::exp(var_mvdbl);

//	if (var_mvtyp&pimpl::MVTYPE_INT)
		return std::exp(double(var_mvint));

	throw MVException(L"exp(unknown mvtype=" ^ var(var_mvtyp) ^ L")");
}

var var::at(const int column) const
{
	THISIS(L"var var::at(const int column) const")

	//*this is not used

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
	THISIS(L"var var::at(const int column,const int row) const")

	//*this is not used

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

	//*this is not used

	//http://www.xfree86.org/current/ctlseqs.html
	//"\x1b6n" report cursor position as \x1b rown ; coln R
	std::cout<<"var::getcursor() not implemented yet "<<std::endl;
	return L"";
}

void var::setcursor() const
{
	THISIS(L"void var::setcursor() const")

	THISISDEFINED()

	std::cout<<"var::getcursor() not implemented yet "<<std::endl;
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


} // namespace exodus
