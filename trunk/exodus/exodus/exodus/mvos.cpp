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

//boost wpath is only in boost v1.34+ but we hardcoded path at the moment which
//prevents unicode in osfilenames etc

#ifndef MVOS_H
#define MVOS_H

//EXCELLENT!
//http://www.regular-expressions.info/
//http://www.regular-expressions.info/unicode.html 
#include <boost/regex.hpp>
#include <boost/scoped_array.hpp>

//regex.assign("", boost::regex::extended|boost::regex_constants::icase);

//TODO check all error handling
//http://www.ibm.com/developerworks/aix/library/au-boostfs/index.html
//catch(boost::filesystem::filesystem_error e) { 
//#include <boost/filesystem.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/config.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
namespace boostfs = boost::filesystem;
#include <boost/thread/tss.hpp>

//#include NullCodecvt.h

//#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

//for sleep
#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>

//for rnd and initrnd
#include <boost/random.hpp>

//for exodus_once
#include <boost/thread/once.hpp>

//#include <istream>
//#include <ostream>
#include <fstream>
#include <cstdlib> //for getenv and setenv/putenv
//#include <stdio.h> //for getenv

//oshell capturing output needs posix popen
#if defined(_MSC_VER) && !defined(popen)
#define popen _popen
#define pclose _pclose
#endif

#define MV_NO_NARROW

#include <exodus/mvimpl.h>
#include <exodus/mv.h>
#include <exodus/mvutf.h>
#include <exodus/mvexceptions.h>

//#include "exodus/NullCodecvt.h"//used to prevent wifstream and wofstream widening/narrowing binary input/output to/from internal wchar_t

//traditional Unix file I/O interface declared in <fcntl.h>
// (under Unix and Linux) or <out.h> (Windows)

//FindFile family of Win32 SDK functions such as
// FindFirstFileEx, FindNextFile and CloseFind

//see http://www.boost.org/libs/filesystem/test/wide_test.cpp
//for boost filesystem wide operations and UTF8

boost::once_flag exodus_once_flag=BOOST_ONCE_INIT;

void exodus_once_func()
{
	//so wcscoll() sorts a<B etc instead of character number wise where B<a
	//wcscoll currently only used on non-pc/non-mac ... see var::localeAwareCompare
	//so has no effect on those platform for < <= etc
	//only en_US.utf-8 seems to be widely available on all platforms
	//.utf8 causes a utf-8 to wchar conversion when converting string to wstring by iterators .begin() .end()
	//and we want to be able to control conversion more accurately than an automatic conversion
	//en_US is missing from Ubuntu. en_IN is available on Ubuntu and Centos but not MacOSX
	setlocale (LC_COLLATE, "en_US.utf-8");
}

/* Ubuntu 8.04 locale -a
default installation seems to be only one language - in this installation en
C
en_AG
en_AU.utf8
en_BW.utf8
en_CA.utf8
en_DK.utf8
en_GB.utf8
en_HK.utf8
en_IE.utf8
en_IN
en_NG
en_NZ.utf8
en_PH.utf8
en_SG.utf8
en_US.utf8
en_ZA.utf8
en_ZW.utf8
POSIX
*/

/* mac osx 10.6 locale -a|grep en_
+many other languages
en_AU
en_AU.ISO8859-1
en_AU.ISO8859-15
en_AU.US-ASCII
en_AU.UTF-8
en_CA
en_CA.ISO8859-1
en_CA.ISO8859-15
en_CA.US-ASCII
en_CA.UTF-8
en_GB
en_GB.ISO8859-1
en_GB.ISO8859-15
en_GB.US-ASCII
en_GB.UTF-8
en_IE
en_IE.UTF-8
en_NZ
en_NZ.ISO8859-1
en_NZ.ISO8859-15
en_NZ.US-ASCII
en_NZ.UTF-8
en_US
en_US.ISO8859-1
en_US.ISO8859-15
en_US.US-ASCII
en_US.UTF-8
*/

/* centos 5.5 locale -a|grep en_
+many other languages
en_AU
en_AU.iso88591
en_AU.utf8
en_BW
en_BW.iso88591
en_BW.utf8
en_CA
en_CA.iso88591
en_CA.utf8
en_DK
en_DK.iso88591
en_DK.utf8
en_GB
en_GB.iso88591
en_GB.iso885915
en_GB.utf8
en_HK
en_HK.iso88591
en_HK.utf8
en_IE
en_IE@euro
en_IE.iso88591
en_IE.iso885915@euro
en_IE.utf8
en_IN
en_IN.utf8
en_NZ
en_NZ.iso88591
en_NZ.utf8
en_PH
en_PH.iso88591
en_PH.utf8
en_SG
en_SG.iso88591
en_SG.utf8
en_US
en_US.iso88591
en_US.iso885915
en_US.utf8
en_ZA
en_ZA.iso88591
en_ZA.utf8
en_ZW
en_ZW.iso88591
en_ZW.utf8
*/

class ExodusOnce
{
public:
	ExodusOnce()
	{
		boost::call_once(&exodus_once_func, exodus_once_flag);
	}
};
static ExodusOnce exodus_once_static;

namespace exodus{

bool checknotabsoluterootfolder(std::wstring dirname)
{
	//safety - will not rename/remove top level folders
	//cwd to top level and delete relative
	//top level folder has only one slash either at the beginning or, on windows, like x:\ .
	//NB copy/same code in osrmdir and osrename
	if (dirname[0] == SLASH_
		|| (SLASH_IS_BACKSLASH
		&& (dirname[1] == L':')
		&& (dirname[2] == SLASH_)))
	{
		std::wcout << "Forced removal/renaming of top level directories by absolute path is not supported for safety but you can use cwd() and relative path." <<dirname << std::endl;
		return false;
	}
	return true;
}

//ICONV_MT can be moved back to mvioconv.cpp if it stops using regular expressions
//regular expressions for ICONV_MT
//var var::iconv_MT(const wchar_t* conversion) const
var var::iconv_MT() const
{
	//ignore everything else and just get first three groups of digits "99 99 99"
	//remove leading and trailing non-digits and replace internal strings of non-digits with single space
	
	//var time=(*this).swap( L"^\\D+|\\D+$", L"", L"r").swap( L"\\D+", L" ", L"r");

	static boost::wregex surrounding_nondigits_regex(L"^\\D+|\\D+$",boost::regex::extended|boost::regex_constants::icase);

	static boost::wregex inner_nondigits_regex(L"\\D+",boost::regex::extended|boost::regex_constants::icase);

	var time=var(boost::regex_replace(toTstring((*this)),surrounding_nondigits_regex, L""));
	time=var(boost::regex_replace(toTstring((time)),inner_nondigits_regex, L" "));

	int hours=time.field(L" ",1).toInt();
	int mins=time.field(L" ",2).toInt();
	int secs=time.field(L" ",3).toInt();

	int inttime=hours*3600+mins*60+secs;

	if (inttime>=86400)
		return L"";

	//PM
	if (inttime<43200&&(*this).index(L"P"))
		inttime+=43200;
	else if (inttime>=43200&&(*this).index(L"A"))
		inttime-=43200;

	return inttime;

}

//OCONV_MC can be moved back to mvioconv.cpp if it stops using regular expressions
//regular expressions for ICONV_MC
var& var::oconv_MC(const wchar_t* conversionchar)
{
	//conversionchar arrives pointing to 3rd character (eg A in MCA)


	//abort if no 3rd char
	if (*conversionchar==L'\0')
		return (*this);

	//form of unicode specific regular expressions
	//http://www.regular-expressions.info/unicode.html

	//availability of unicode regular expressions
	//http://www.boost.org/doc/libs/1_35_0/libs/regex/doc/html/boost_regex/unicode.html

	/*
	\w=alphanumeric characters, \W all the rest
	\d=decimal characters, \D all the rest

	but there is no such code to indicate ALPHABETIC (NON_DECIMAL) characters only (i.e. \w but not \d)
	Imagine we also had the codes \a alphabetic characters, \A all the rest
	\a = [^\W\d] ... which is characters which are a) "NOT NON-WORD CHARACTERS" and b) "NOT DECIMAL DIGITS"
	\A = [\W\d]  ... which is a) NON-WORD CHARACTERS and b) decimal digits

	if boost has icu then \w and \d includes letters and decimal digits from all languages

	123456|abcdefghi|!"$%^&* ()-+[]
	dddddd|DDDDDDDDD|DDDDDDDDDDDDDD all characters divided into \d (decimal)		and \D (non-decimals)
	AAAAAA|aaaaaaaaa|AAAAAAAAAAAAAA all characters divided into \a (alphabetic)		and \A (non-alphabetic)
	wwwwww|wwwwwwwww|WWWWWWWWWWWWWW all characters divided into \w (alphanumeric)	and \W (non-alphanumeric)
	*/

	/* unicode regex extensions
	http://userguide.icu-project.org/strings/regexp
	http://www.regular-expressions.info/unicode.html
	sometimes unicode extension for regular expressions have \pL for \a and [^\pL] for \A
	Match a word character. Word characters are [\p{Ll}\p{Lu}\p{Lt}\p{Lo}\p{Nd}].
	l=lowercase u=uppercase t=titlecase o=other Nd=decimal digit
	\N{UNICODE CHARACTER NAME}	Match the named character.
	\p{UNICODE PROPERTY NAME}	Match any character with the specified Unicode Property.
	\P{UNICODE PROPERTY NAME}	Match any character not having the specified Unicode Property.
	*/

	static const boost::wregex
		digits_regex		(L"\\d*"		,boost::regex::extended), // \d numeric
		alpha_regex			(L"[^\\W\\d]*"	,boost::regex::extended), // \a alphabetic
		alphanum_regex		(L"\\w*"		,boost::regex::extended), // \w alphanumeric
		non_digits_regex	(L"[^\\d]*"		,boost::regex::extended), // \D non-numeric
		non_alpha_regex		(L"[\\W\\d]*"	,boost::regex::extended), // \A non-alphabetic
		non_alphanum_regex	(L"\\W*"		,boost::regex::extended); // \W non-alphanumeric

	//negate if /
	if (*conversionchar==L'/')
	{
		++conversionchar;
		switch (*conversionchar)
		{
			// MC/N return everything except digits i.e. remove all digits 0123456789
			case 'N':
			{
				var_mvstr=boost::regex_replace(toTstring((*this)),digits_regex, L"");
				break;
			}

			// MC/A return everything except "alphabetic" i.e remove all "alphabetic"
			case 'A':
			{
				var_mvstr=boost::regex_replace(toTstring((*this)),alpha_regex, L"");
				break;
			}
			// MC/B return everything except "alphanumeric" remove all "alphanumeric"
			case 'B':
			{
				var_mvstr=boost::regex_replace(toTstring((*this)),alphanum_regex, L"");
				break;
			}
		}
		return *this;
	}

	//http://www.boost.org/doc/libs/1_37_0/libs/regex/doc/html/boost_regex/ref/regex_replace.html
	//boost::regex_replace

	switch (*conversionchar)
	{
		// MCN return only digits i.e. remove all non-digits
		case 'N':
		{
			var_mvstr=boost::regex_replace(toTstring((*this)),non_digits_regex, L"");
			break;
		}
		// MCA return only "alphabetic" i.e. remove all "non-alphabetic"
		case 'A':
		{
			var_mvstr=boost::regex_replace(toTstring((*this)),non_alpha_regex, L"");
			break;
		}
		// MCB return only "alphanumeric" i.e. remove all "non-alphanumeric"
		case 'B':
		{
			var_mvstr=boost::regex_replace(toTstring((*this)),non_alphanum_regex, L"");
			break;
		}
		// MCL to lower case
		case 'L':
		{
			lcaser();
			break;
		}
		// MCU to upper case
		case 'U':
		{
			ucaser();
			break;
		}
	}

	return *this;
}

//BOOST RANDOM
//http://www.boost.org/doc/libs/1_38_0/libs/random/random_demo.cpp

//set the generator type to ...
//typedef boost::minstd_rand random_base_generator_type;
typedef boost::mt19937 random_base_generator_type;
boost::thread_specific_ptr<random_base_generator_type> tss_random_base_generators;

random_base_generator_type* get_random_base_generator()
{
	//get/init the base generator
	random_base_generator_type* threads_random_base_generator=tss_random_base_generators.get();
	if (!threads_random_base_generator)
	{
		tss_random_base_generators.reset(new random_base_generator_type);
		threads_random_base_generator=tss_random_base_generators.get();
		if (!threads_random_base_generator)
			throw MVException(L"Could not create random number generator");

		//seed to the os clock (secs since unix epoch)
		//Caveat: std::time(0) is not a very good truly-random seed.
		//logputl(L"Seeding random number generator to system clock");
		(*threads_random_base_generator).seed(static_cast<unsigned int>(std::time(0)+2375472354));
		//(*thread_base_generator).seed(static_cast<unsigned int>(var().ostime().toInt()));

	}
	return threads_random_base_generator;
}

var var::rnd() const
{
	THISIS(L"var var::rnd() const")
	THISISNUMERIC()

	//get/init the base generator
	random_base_generator_type* threads_random_base_generator=get_random_base_generator();

	//pick rnd(5) returns pseudo random integers in the range of 0-4
	int max=(*this).toInt()-1;
	if (max<0)
		return var(*this);

	//define a integer range (0 to n-1)
	boost::uniform_int<> uni_dist(0,max);

	//use the base generator and the range to create a specific generator
	boost::variate_generator<random_base_generator_type&, boost::uniform_int<> > uni(*threads_random_base_generator, uni_dist);

	//generate a pseudo random number
	return uni();
}

void var::initrnd() const
{
	THISIS(L"void var::initrnd(const var& seed) const")
	THISISNUMERIC()

	//get/init the base generator
	random_base_generator_type* threads_random_base_generator=get_random_base_generator();

	//set the new seed
	//logputl(L"Seeding random number generator to " ^ (*this));
	(*threads_random_base_generator).seed(static_cast<unsigned int>( (*this).toInt() ));
}

//only here really because boost regex is included here for file matching
bool var::match(const var& matchstr, const var& options) const
{
	THISIS(L"bool var::match(const var& matchstr, const var& options) const")
	THISISSTRING()
	ISSTRING(matchstr)

	//TODO fully implement
	if (options.index(L"w"))
	{
		if (matchstr==L""||var_mvstr==matchstr||matchstr==L"*.*"){}
		else if (matchstr.substr(1,1)==L"*"&&substr(-matchstr.length()+1)==matchstr.substr(2)){}
		else if (matchstr.substr(-1,1)==L"*"&&substr(1,matchstr.length()-1)==matchstr.substr(1,matchstr.length()-1)){}
		else return false;
		return true;
	}

	//http://www.boost.org/doc/libs/1_38_0/libs/regex/doc/html/boost_regex/syntax/basic_syntax.html

	//TODO automatic caching of regular expressions or new exodus datatype to handle them
	//TODO automatic caching of regular expressions or new exodus datatype to handle them
	boost::wregex regex;
	try
	{
		if (options.index(L"i"))
                        regex.assign(toTstring(matchstr), boost::regex::extended|boost::regex_constants::icase);
		else
                        regex.assign(toTstring(matchstr), boost::regex::extended|boost::regex_constants::icase);
	//boost::wregex toregex_regex(with.towstring(), boost::regex::extended);
	}
	catch (boost::regex_error& e)
    {
		throw MVException(var(e.what()).quote() ^ L" is an invalid regular expression");
	}

        return regex_match(toTstring((*this)), regex);

}

var var::swap(const var& what, const var& with, const var& options) const
{
	THISIS(L"var var::swap(const var& what, const var& with) const")
	THISISSTRING()

	return var(*this).swapper(what,with,options);
}

//only here really because boost regex is included here for file matching
var& var::swapper(const var& what, const var& with, const var& options)
{
	THISIS(L"var& var::swapper_regex(const var& regex, const var& with, const var& options)")
	THISISSTRING()
	ISSTRING(what)
	ISSTRING(with)
	ISSTRING(options)

	if (options.index(L"r"))
	{
		//http://www.boost.org/doc/libs/1_38_0/libs/regex/doc/html/boost_regex/syntax/basic_syntax.html

		//TODO automatic caching of regular expressions or new exodus datatype to handle them
		boost::wregex regex;
		try
		{
			if (options.index(L"i"))
                                regex.assign(toTstring(what), boost::regex::extended|boost::regex_constants::icase);
			else
                                regex.assign(toTstring(what), boost::regex::extended);
		//boost::wregex toregex_regex(with.towstring(), boost::regex::extended);
		}
		catch (boost::regex_error& e)
		{
			throw MVException(var(e.what()).quote() ^ L" is an invalid regular expression");
		}

		//return regex_match(var_mvstr, expression);

		//std::wostringstream outputstring(std::ios::out | std::ios::binary);
    //std::ostream_iterator<wchar_t, wchar_t> oiter(outputstring);
		//boost::regex_replace(oiter, var_mvstr.begin(), var_mvstr.end(),regex_regex, with, boost::match_default | boost::format_all);
                var_mvstr=var(boost::regex_replace(toTstring((*this)),regex, toTstring(with))).var_mvstr;

	}
	else
	{
		std::wstring str1=what.var_mvstr;
		std::wstring str2=with.var_mvstr;

		//nothing to do if oldstr is ""
		if (str1==L"")
			return *this;

		//find the starting position of the field or return
		std::wstring::size_type start_pos=0;
		while (true)
		{
			start_pos=var_mvstr.find(str1,start_pos);
			//past of of string?
			if (start_pos==std::wstring::npos) return *this;
			var_mvstr.replace(start_pos,str1.length(),str2);
			start_pos+=str2.length();
		}
	}

	return *this;
}


//in MVutils.cpp
void ptime2mvdatetime(const boost::posix_time::ptime& ptimex, int& mvdate, int& mvtime);

bool var::osgetenv(const var& envvarname)
{
	THISIS(L"bool var::osgetenv(const var& envvarname)")
	THISISDEFINED()
	ISSTRING(envvarname)

	#pragma warning ( disable : 4996)
	const char* cvalue=std::getenv(envvarname.tostring().c_str());
	if (cvalue==0)
	{
		var_mvstr=L"";
		var_mvtyp=pimpl::MVTYPE_STR;
		return false;
	}
	else
		*this=var(cvalue);
	var_mvtyp=pimpl::MVTYPE_STR;
	return true;
}

bool var::ossetenv(const var& envvarname) const
{
	THISIS(L"bool var::ossetenv(const var& envvarname) const")
	THISISDEFINED()
	ISSTRING(envvarname)

//#ifdef _MSC_VER
#ifndef setenv
        /* on windows this should be used
	BOOL WINAPI SetEnvironmentVariable(LPCTSTR lpName, LPCTSTR lpValue);
	*/

	//is this safe on windows??
	//https://www.securecoding.cert.org/confluence/display/seccode/POS34-C.+Do+not+call+putenv()+with+a+pointer+to+an+automatic+variable+as+the+argument
	std::string tempstr=envvarname.tostring();
	tempstr+="=";
	tempstr+=tostring();
	//#pragma warning ( disable : 4996)
	const int result=putenv((char*)(tempstr.c_str()));
	if (result==0)
		return true;
	else
		return false;

#else
	return setenv((char*)(envvarname.tostring().c_str()),(char*)(tostring().c_str()),1);
#endif

}

var var::osshell() const
{
	THISIS(L"var var::osshell() const")
	//will be checked again by tostring()
	//but put it here so any unassigned error shows in osshell
	THISISSTRING()

	breakoff();
	int shellresult=system(tostring().c_str());
	breakon();

	return shellresult;
}

var var::osshellread() const
{
	THISIS(L"var var::osshellread() const")
	//will be checked again by tostring()
	//but put it here so any unassigned error shows in osshell
	THISISSTRING()

	var output=L"";

	//"r" means read
	std::FILE *cmd=popen(tostring().c_str(), "r");
	//return a code to indicate program failure. but can this ever happen?
	if (cmd==NULL)
		return 1;
	//TODO buffer overflow check
    char cstr1[1024]={0x0};
	while (std::fgets(cstr1, sizeof(cstr1), cmd) !=NULL) {
		//std::printf("%s\n", result);
		//cannot trust that standard input is convertable from utf8
		//output.var_mvstr+=wstringfromUTF8((const UTF8*)result,strlen(result));
		std::string str1=cstr1;
		output.var_mvstr+=std::wstring(str1.begin(),str1.end());
	}

	//we are going to throw away the process termination status
	//because we are going to return the output text
	//int result=
	pclose(cmd);

    return output;

}

var var::osshellwrite(const var& writestr) const
{
	THISIS(L"var var::osshellwrite(const var& writestr) const")
	//will be checked again by tostring()
	//but put it here so any unassigned error shows in osshell
	THISISSTRING()
	ISSTRING(writestr)

	//"w" means read
	std::FILE *cmd=popen(tostring().c_str(), "w");
	//return a code to indicate program failure. but can this ever happen?
	if (cmd==NULL)
		return 1;
	fputs(writestr.tostring().c_str(),cmd);

	//return the process termination status (pity cant do this for read too)
	return pclose(cmd);

}

var var::suspend() const
{
	//THISIS(L"var var::suspend() const")

	breakoff();
	//use dummy to avoid warning in gcc4 "warning: ignoring return value of int system(const char*), declared with attribute warn_unused_result"
	int dummy=system(tostring().c_str());
	breakon();

	return L"";

	//evade warning: unused variable 'dummy'
	if (dummy) {};
}

void var::osflush() const
{
	//THISIS(L"void var::osflush() const")

	std::wcout<<L"var::flush not implemented yet"<<std::endl;
	return;
}


	//what is the purpose of the following?
	//to prevent locale conversion if writing narrow string to wide stream or vice versa
	//imbue BEFORE opening
	//	myfile.imbue( std::locale(std::locale::classic(), new NullCodecvt));
//var(int(sizeof(myfile))).outputl(L"filesize");
	//if (osfilename.var_mvtyp&pimpl::MVTYPE_INT)
	//	myfile=(std::fstream) var_mvint;
	//else


bool var::osopen(const var& osfilename)
{
	THISIS(L"bool var::osopen(const var& osfilename)")
	THISISDEFINED()
	ISSTRING(osfilename)

	//TODO buffer filehandles to prevent continual reopening

	if (!(osfilename.var_mvtyp&pimpl::MVTYPE_STR))
	{
		if (!osfilename.var_mvtyp)
			throw MVUnassigned(L"osopen(osfilename)");
		osfilename.createString();
	}

	std::string sfn = osfilename.tostring();
	// avoid "fstream * pfstream = new ..." complexities
	std::fstream fstr( sfn.c_str(), std::ios::out |std::ios::in | std::ios::binary);
	if( ! fstr)
		return false;

	var_mvstr=osfilename.var_mvstr;
	var_mvtyp=pimpl::MVTYPE_STR;
	fstr.close();
	return true;
}


//on unix or with iconv we could convert to or any character format
//for sequential output we could use popen to progressively read and write with no seek facility
//popen("iconv -f utf-16 -t utf-8 >targetfile","w",filehandle) followed by progressive writes
//popen("iconv -f utf-32 -t utf-16 <sourcefile","r",filehandle) followed by progressive reads
//use utf-16 or utf-32 depending on windows or unix

bool var::osread(const var& osfilename)
{
	THISIS(L"bool var::osread(const var& osfilename)")
	//will be checked by nested osread
	//THISISDEFINED()
	ISSTRING(osfilename)
	return osread(osfilename.tostring().c_str());
}

bool var::osread(const char* osfilename)
{
	THISIS(L"bool var::osread(const var& osfilename)")
	THISISDEFINED()
	//ISSTRING(osfilename)

	var_mvstr=L"";
	var_mvtyp=pimpl::MVTYPE_STR;

	//std::wifstream myfile;
	std::ifstream myfile;

	//what is the purpose of the following?
	//to prevent locale conversion if writing narrow string to wide stream or vice versa
	//imbue BEFORE opening
	//myfile.imbue( std::locale(std::locale::classic(), new NullCodecvt));

	//ios:ate to go to the end to find the size in the next statement with tellg
	//myfile.open(osfilename.tostring().c_str(), std::ios::binary | std::ios::ate );
	//binary!
	myfile.open(osfilename, std::ios::binary | std::ios::ate );
	if (!myfile)
		return false;

	//NB tellg and seekp goes by bytes regardless of normal/wide stream

	unsigned int bytesize;
    bytesize = myfile.tellg();

	if (bytesize>0)
	{
/*		char* memblock;
		memblock = new char [bytesize];
*/
		boost::scoped_array<char> memblock( new char [bytesize]);
   		if (memblock==0)
   		{
   			myfile.close();
   			return false;
   		}

    	myfile.seekg (0, std::ios::beg);
		myfile.read (memblock.get(), (unsigned int) bytesize);
    	myfile.close();

		//for now dont accept UTF8 in order to avoid failure if non-utf characters
		//var_mvstr=wstringfromUTF8((UTF8*) memblock, (unsigned int) bytesize);
		std::string tempstr=std::string( memblock.get(), bytesize);
		//ALN:JFI: actually we could use std::string 'tempstr' in place of 'memblock' by hacking
		//	.data member and reserve() or resize(), thus avoiding buffer-to-buffer-copying

		var_mvstr=std::wstring(tempstr.begin(),tempstr.end());

    	// scoped_array do it: delete[] memblock;
	}
	return true;
}

bool var::oswrite(const var& osfilename) const
{
	THISIS(L"bool var::oswrite(const var& osfilename) const")
	THISISSTRING()
	ISSTRING(osfilename)

	std::ofstream myfile;

	//what is the purpose of the following?
	//to prevent locale conversion if writing narrow string to wide stream or vice versa
	//imbue BEFORE opening
	//myfile.imbue( std::locale(std::locale::classic(), new NullCodecvt));

	//binary!
	myfile.open(osfilename.tostring().c_str(), std::ios::trunc | std::ios::out | std::ios::binary);
	if (!myfile)
	 return false;

	//myfile.write(var_mvstr.data(),int(var_mvstr.length()));
	std::string output=toUTF8(var_mvstr);
	myfile.write(output.data(),(unsigned int)output.length());
	if (myfile.fail())
	{
		myfile.close();
		return false;
	}

   	myfile.close();
	return true;

}

void var::osbwrite(const var& osfilehandle, const int startoffset) const
{
	THISIS(L"void var::osbwrite(const var& osfilehandle, const int startoffset) const")
	THISISSTRING()
	ISSTRING(osfilehandle)

	//TODO buffer filehandles to prevent continual reopening

	//TODO buffer the fileopen ... is ate needed here?
	//using wfstream instead of wofstream so that we can seek to the end of the file?
	std::wofstream myfile;

	//what is the purpose of the following?
	//to prevent locale conversion if writing narrow string to wide stream or vice versa
	//imbue BEFORE opening
    //myfile.imbue( std::locale(std::locale::classic(), new NullCodecvt));

	//binary!
	myfile.open(osfilehandle.tostring().c_str(), std::ios::out | std::ios::in | std::ios::binary | std::ios::ate);
	if (!myfile)
		return;

	//NB seekp goes by bytes regardless of the fact that it is a wide stream
	myfile.seekp (startoffset*sizeof(wchar_t));
	//NB but length goes by number of wide characters not bytes
#ifndef __MINGW32__
		myfile.write(var_mvstr.data(),int(var_mvstr.length()));
#else
	//TODO avoid string copy if wofstream not defined to be ofstream
        std::string tempstr=tostring();
        myfile.write(tempstr.data(),int(tempstr.length()));
#endif

	if (myfile.fail())
	{
		myfile.close();
		return;
	}

   	myfile.close();
	return;

}

var& var::osbread(const var& osfilehandle, const int startoffset, const int size)
{
	THISIS(L"var& var::osbread(const var& osfilehandle, const int startoffset, const int size)")
	THISISDEFINED()
	ISSTRING(osfilehandle)

	//TODO buffer filehandles to prevent continual reopening

	var_mvstr=L"";
	var_mvtyp=pimpl::MVTYPE_STR;

	if (size<=0) return *this;

	//avoiding wifstream due to non-availability on some platforms (mingw for starters)
	//and to allow full control over narrow/wide character conversion
	//std::wifstream myfile;
	std::ifstream myfile;

	//what is the purpose of the following?
	//to prevent locale conversion if writing narrow string to wide stream or vice versa
	//imbue BEFORE opening
	//myfile.imbue( std::locale(std::locale::classic(), new NullCodecvt));

	//binary!
	//use ::ate to position at end so tellg below can determine file size
	myfile.open(osfilehandle.tostring().c_str(), std::ios::binary | std::ios::ate);
	if (!myfile)
		return *this;
//NB all file sizes are in bytes NOT characters despite this being a wide character fstream
	unsigned int maxsize = myfile.tellg();
	//int readsize=maxsize-startoffset*sizeof(wchar_t);
	//if (readsize>size*2) readsize=size*sizeof(wchar_t);
	//unsigned int readsize=maxsize-startoffset;
	//unsigned int readsize=maxsize-startoffset;

	//reading beyond end of file
	if (startoffset>=0 && (unsigned int)startoffset>=maxsize)
	{
		myfile.close();
		return *this;
	}

	//allow negative offset to read from the back of the file
	//!cannot be unsigned and allow negative
	//unsigned int readfrom=startoffset;
	int readfrom=startoffset;
	if (readfrom<0) {
		readfrom+=maxsize;
		//but not so negative that it reads from before the beginning of the file
		if (readfrom<0)
			readfrom=0;
	}

	//limit reading up to end of file although probably happens automatically
	unsigned int readsize=size;
	if (readfrom+readsize>maxsize)
		readsize=maxsize-readfrom;

	//nothing to read
	if (readsize<=0)
	{
		myfile.close();
		return *this;
	}

	//new
	//wchar_t* memblock;
	//memblock = new wchar_t [readsize/sizeof(wchar_t)];
/*
	char* memblock;
	memblock = new char [readsize];
*/
	boost::scoped_array<char> memblock( new char [readsize]);
	if (memblock==0)
	{//TODO NEED TO THROW HERE
		myfile.close();
		return *this;
	}

	//myfile.seekg (startoffset/sizeof(wchar_t), std::ios::beg);
	myfile.seekg (startoffset, std::ios::beg);
	myfile.read (memblock.get(), readsize);
	myfile.close();

	//var_mvstr=std::wstring(memblock,readsize/sizeof(wchar_t));

	//for now dont accept UTF8 in order to avoid failure if non-utf characters
	//var_mvstr=wstringfromUTF8((UTF8*) memblock, (unsigned int) bytesize);
	//
	//following doesnt work in binary if locale ends in utf8 etc.
	//but we need a locale for string collation on linux and there isnt a common non-utf8 one
	//std::string tempstr=std::string(memblock, readsize);
	//following does a conversion depending on the locale
	//var_mvstr=std::wstring(tempstr.begin(),tempstr.end());
	var_mvstr=wstringfromchars(memblock.get(), readsize);

	//delete[] memblock;
//	var_mvtyp=pimpl::MVTYPE_STR;
	return *this;
}

void var::osclose() const
{
	THISIS(L"void var::osclose() const")
	THISISSTRING()

	//TODO buffer filehandles to prevent continual reopening

	return;
}

bool var::osrename(const var& newosdir_or_filename) const
{
	THISIS(L"bool var::osrename(const var& newosdir_or_filename) const")
	THISISSTRING()
	ISSTRING(newosdir_or_filename)
	
	//prevent overwrite of existing file
	//ACQUIRE
	std::wifstream myfile;
	//binary?
	myfile.open(newosdir_or_filename.tostring().c_str(), std::ios::binary );
	if (myfile)
	{
		//RELEASE
		myfile.close();
		return false;
	}

	//safety
	if (!checknotabsoluterootfolder(towstring()))
		return false;

	return !std::rename(tostring().c_str(),newosdir_or_filename.tostring().c_str());

}

bool var::oscopy(const var& to_osfilename) const
{
	THISIS(L"bool var::oscopy(const var& to_osfilename) const")
	THISISSTRING()
	ISSTRING(to_osfilename)
	
    //boostfs::wpath frompathx(toTstring((*this)).c_str());
    //boostfs::wpath topathx(toTstring(to_osfilename).c_str());
    boostfs::path frompathx((*this).tostring().c_str());
    boostfs::path topathx(to_osfilename.tostring().c_str());
    try
    {
		//will not overwrite so this is redundant
		//option to overwrite is not in initial versions of boost copy_file
		if (boostfs::exists(topathx)) return false;

		boostfs::copy_file(frompathx, topathx);
	}
	catch(...)
	{
		return false;
	}
	return true;

}

bool var::osdelete() const
{
	return osdelete(*this);
}

//not boost ... only removes files?
bool var::osdelete(const var& osfilename) const
{
	THISIS(L"bool var::osdelete(const var& osfilename) const")
	THISISDEFINED()
	ISSTRING(osfilename)
	
	return !std::remove(osfilename.tostring().c_str());
}

var var::oslistf(const var& path, const var& spec) const
{
	return oslist(path, spec, 1);
}

var var::oslistd(const var& path, const var& spec) const
{
	return oslist(path, spec, 2);
}

var var::osfile() const
{
	THISIS(L"var var::osfile() const")
	THISISSTRING()
	
	//get a handle and return L"" if doesnt exist or isnt a regular file
    try
    {
		//boost 1.33 throws an error with files containing ~ or $ chars but 1.38 doesnt
		//boostfs::wpath pathx(toTstring((*this)).c_str());
		boostfs::path pathx((*this).tostring().c_str());

        if (!boostfs::exists(pathx)) return L"";
        //is_regular is only in boost > 1.34
		//if (!boostfs::is_regular(pathx)) return L"";
        if (boostfs::is_directory(pathx)) return L"";

        //get last write datetime
        std::time_t last_write_time=boostfs::last_write_time(pathx);
        //convert to ptime
        boost::posix_time::ptime ptimex = boost::posix_time::from_time_t(last_write_time);
        //convert to mv date and time
        int mvdate, mvtime;
        ptime2mvdatetime(ptimex, mvdate, mvtime);

		return int(boostfs::file_size(pathx)) ^ FM ^ mvdate ^ FM ^ int(mvtime);

	}
	catch(...)
	{
		return L"";
	};
}

bool var::osmkdir() const
{
	THISIS(L"bool var::osmkdir() const")
	THISISSTRING()
	
    try
    {

		//boost 1.33 throws an error with files containing ~ or $ chars but 1.38 doesnt
		//boostfs::wpath pathx(toTstring((*this)).c_str());
		boostfs::path pathx((*this).tostring().c_str());

		if (boostfs::exists(pathx)) return false;
		boostfs::create_directories(pathx);
	}
	catch(...)
	{
		return false;
	}
	return true;
}

bool var::osrmdir(bool evenifnotempty) const
{
	THISIS(L"bool var::osrmdir(bool evenifnotempty) const")
	THISISSTRING()
	
	//get a handle and return L"" if doesnt exist or is NOT a directory
    try
    {

		//boost 1.33 throws an error with files containing ~ or $ chars but 1.38 doesnt
		//boostfs::wpath pathx(toTstring((*this)).c_str());
		boostfs::path pathx((*this).tostring().c_str());

        if (!boostfs::exists(pathx)) return false;
        if (!boostfs::is_directory(pathx)) return false;
		
		if (evenifnotempty)
		{

			//safety
			if (!checknotabsoluterootfolder(towstring()))
				return false;

			boostfs::remove_all(pathx);
		}
		else
			boostfs::remove(pathx);
	}
	catch(...)
	{
		return false;
	}

	return true;
}

var var::osdir() const
{
	THISIS(L"var var::osdir() const")
	THISISSTRING()

	//get a handle and return L"" if doesnt exist or is NOT a directory
    //boostfs::wpath pathx(toTstring((*this)).c_str());
    try
    {

		//boost 1.33 throws an error with files containing ~ or $ chars but 1.38 doesnt
		boostfs::path pathx((*this).tostring().c_str());

        if (!boostfs::exists(pathx)) return L"";
        if (!boostfs::is_directory(pathx)) return L"";

        //get last write datetime
        std::time_t last_write_time=boostfs::last_write_time(pathx);
        //convert to ptime
        boost::posix_time::ptime ptimex = boost::posix_time::from_time_t(last_write_time);
        //convert to mv date and time
        int mvdate, mvtime;
        ptime2mvdatetime(ptimex, mvdate, mvtime);

		return L"" ^ FM ^ mvdate ^ FM ^ int(mvtime);

	}
	catch(...)
	{
		return L"";
	};
}

var var::oslist(const var& path, const var& spec, const int mode) const
{
	THISIS(L"var var::oslist(const var& path, const var& spec, const int mode) const")
	THISISDEFINED()
	ISSTRING(path)
	ISSTRING(spec)

	//returns an fm separated list of files and/or folders

	//http://www.boost.org/libs/filesystem/example/simple_ls.cpp

	bool filter=false;
	//boost::wregex re;
	boost::regex re;
	if (spec)
	{
		try
		{
			// Set up the regular expression for case-insensitivity
			//re.assign(toTstring(spec).c_str(), boost::regex_constants::icase);
			re.assign(spec.tostring().c_str(), boost::regex_constants::icase);
			filter=true;
		}
		catch (boost::regex_error& e)
		{
			std::wcout << spec.towstring() << L" is not a valid regular expression: \""
			<< e.what() << L"\"" << std::endl;
			return L"";
		}
	}

	bool getfiles=true;
	bool getfolders=true;
	if (mode==1)
		getfolders=false;
	else if (mode==2)
		getfiles=false;

	var filelist=L"";

	//get handle to folder
	//wpath or path before boost 1.34
	//boostfs::wpath full_path( boostfs::initial_path<boostfs::wpath>());
    //full_path = boostfs::system_complete(boostfs::wpath( toTstring(path), boostfs::native ));
	boostfs::path full_path( boostfs::initial_path());
	full_path = boostfs::system_complete(boostfs::path( path.tostring().c_str(), boostfs::native ));

	//quit if it isnt a folder
	if (!boostfs::is_directory(full_path))
		return filelist;

	//errno=0;
    //boostfs::wdirectory_iterator end_iter;
    //for ( boostfs::wdirectory_iterator dir_itr( full_path );
    boostfs::directory_iterator end_iter;
    for ( boostfs::directory_iterator dir_itr( full_path );
          dir_itr != end_iter;
          ++dir_itr )
    {
	try
	{

		//dir_itr->path().leaf()  changed to dir_itr->leaf() in three places
		//also is_directory(dir_itr->status()) changed to is_directory(*dir_itr)
		//to avoid compile errors on boost 1.33
		//http://www.boost.org/doc/libs/1_33_1/libs/filesystem/doc/index.htm

        //skip unwanted items
		if (filter&&!boost::regex_match(dir_itr->leaf(), re))
			continue;

		//using .leaf instead of .status provided in boost 1.34 .. but does it work/efficiently
        	//if ( boostfs::is_directory( dir_itr->status() ) )
		if ( boostfs::is_directory( *dir_itr ) )
		{
			if (getfolders)
				filelist^=FM ^ dir_itr->leaf();
		}
		//is_regular is only in boost > 1.34
		//else if ( boostfs::is_regular( dir_itr->status() ) )
		else// if ( boostfs::is_regular( dir_itr->status() ) )
		{
			if (getfiles)
				filelist^=FM ^ dir_itr->leaf();
		}
		//else
		//{
		//  //++other_count;
		//  //std::wcout << dir_itr->path().leaf() << L" [other]\n";
		//}
      	}
      	catch ( const std::exception & ex )
      	{
		//evade warning: unused variable
		if (false) if (ex.what()) {};

		//++err_count;
		//std::wcout << dir_itr->path().leaf() << L" " << ex.what() << std::endl;
      	}

    }

	//delete first separator
	//NB splice is 1 based
	if (filelist!=L"") filelist.splicer(1,1,L"");

	return filelist;
}

var var::oscwd(const var& newpath) const
{
	THISIS(L"var var::oscwd(const var& newpath) const")
	//doesnt use *this - should syntax be changed to setcwd? and getcwd()?
	THISISDEFINED()//not needed if *this not used
	ISSTRING(newpath)

	//http://www.boost.org/doc/libs/1_38_0/libs/filesystem/doc/reference.html#Attribute-functions
	//wont compile on boost 1.33 so comment it out and make non-functional
	//until we have a reliable way to detect boost version
	//boost::filesystem::current_path(newpath.tostring());

	return oscwd();

}

var var::oscwd() const
{
	THISIS(L"var var::oscwd() const")
	//doesnt use *this - should syntax be changed to ossetcwd? and osgetcwd()?
	THISISDEFINED()//not needed if *this not used

	//TODO consider using complete() or system_complete()
	//"[Note: When portable behavior is required, use complete(). When operating system dependent behavior is required, use system_complete()."

	//http://www.boost.org/doc/libs/1_38_0/libs/filesystem/doc/reference.html#Attribute-functions
	std::string currentpath=boost::filesystem::current_path().string();

	return var(currentpath).convert(L"/",SLASH);

/*
	//http://www.boost.org/libs/filesystem/doc/tr2_proposal.html#Class-template-basic_path
	//boost::filesystem::path full_path( boost::filesystem::current_path());
	//get the current path as wstring
	std::string currentpath=boost::filesystem::current_path().string();
	std::wstring wcurrentpath=std::wstring(currentpath.begin(),currentpath.end());
    boost::filesystem::wpath full_path(wcurrentpath);

    return full_path.string();
*/
}

void var::ossleep(const int milliseconds) const
{
	THISIS(L"void var::ossleep(const int milliseconds) const")
	//doesnt use *this - should syntax be changed to setcwd? and getcwd()?
	THISISDEFINED()//not needed if *this not used

    boost::xtime xt;
    boost::xtime_get(&xt, boost::TIME_UTC);
    xt.nsec += milliseconds*1000000;
    boost::thread::sleep(xt);

//	usleep((int)(1000.0*milliseconds));
//	Sleep(1000*milliseconds);
}

}// namespace exodus

#endif
