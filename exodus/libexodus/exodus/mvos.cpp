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

//boost wpath is only in boost v1.34+ but we hardcoded path at the moment which
//prevents unicode in osfilenames etc

//EXCELLENT!
//http://www.regular-expressions.info/
//http://www.regular-expressions.info/unicode.html 

//show c++ version for info
#include <boost/preprocessor/stringize.hpp>
#pragma message "__cplusplus=" BOOST_PP_STRINGIZE(__cplusplus)

#define USE_BOOST_REGEX

#ifndef USE_BOOST_REGEX
#	include <regex>
#	define std_boost std

#else
#	define std_boost boost
#	include <boost/regex/config.hpp>

//{{ use boost's utf8 facet for codecvt since it is stable
#	define BOOST_UTF8_BEGIN_NAMESPACE namespace boost { namespace filesystem { namespace detail {
#	define BOOST_UTF8_DECL
#	define BOOST_UTF8_END_NAMESPACE }}}

#	include <boost/detail/utf8_codecvt_facet.hpp>
// this requires also to link with the library: libboost_filesystem-*.lib
// For example (for debug under MSVS2008: libboost_filesystem-vc90-mt-gd-1_38.lib

#	undef BOOST_UTF8_END_NAMESPACE
#	undef BOOST_UTF8_DECL
#	undef BOOST_UTF8_BEGIN_NAMESPACE
//}}

#	if !defined(BOOST_HAS_ICU) && !defined(BOOST_HASNT_ICU)
#		define BOOST_HAS_ICU
#	endif
#	ifdef BOOST_HAS_ICU
#		pragma message "BOOST_HAS_ICU"
#		include <boost/regex/icu.hpp>
#	endif
#	include <boost/regex.hpp>

#endif

#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>

//regex.assign("", std_boost::regex::extended|std_boost::regex_constants::icase);

//TODO check all error handling
//http://www.ibm.com/developerworks/aix/library/au-stdfs/index.html
//catch(boost::filesystem::filesystem_error e) { 
//#include <boost/filesystem.hpp>
#define C17
#ifdef C17
#include <experimental/filesystem>
namespace stdfs = std::experimental::filesystem;
#else
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/config.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
namespace stdfs = boost::filesystem;
#endif
#include <boost/thread/tss.hpp>

//#include NullCodecvt.h

//#include <boost/date_time/gregorian/gregorian.hpp>
//not needed on ubuntu 14.04 x64 c++11 build so comment out - maybe needed on other platforms
//#include <boost/date_time/posix_time/posix_time.hpp>

//for sleep
#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>

//for rnd and initrnd
#include <boost/random.hpp>

//for exodus_once
#include <boost/thread/once.hpp>

//#include <istream>
//#include <ostream>
#include <locale>
#include <fstream>
#include <cstdlib> //for getenv and setenv/putenv
#include <algorithm> //for count in osrename

//to get whole environment
extern char **environ;
#include <boost/locale.hpp>

//boost changed from TIME_UTC to TIME_UTC_ (when?) to avoid a new standard TIME_UTC macro in C11 time.h
#include <boost/version.hpp>
#if BOOST_VERSION < 105000
#define TIME_UTC_ TIME_UTC
#endif

//for initrnd from string
#include <MurmurHash2_64.h>

//oshell capturing output needs posix popen
#if defined(_MSC_VER) && !defined(popen)
#define popen _popen
#define pclose _pclose
#endif

#define MV_NO_NARROW

#include <exodus/mvimpl.h>
#include <exodus/mv.h>
//#include <exodus/mvutf.h>
#include <exodus/mvexceptions.h>

#include "mvhandles.h"

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
	if (!setlocale (LC_COLLATE, "en_US.utf8"))
		{};
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

// this object caches wfstream * pointers, to avoid multiple reopening files
//extern MvHandlesCache mv_handles_cache;
// Lifecircle of wfstream object:
//	- created (by new) and opened in osbread()/osbwrite();
//	- pointer to created object stored in h_cache;
//  - when user calls osclose(), the stream is closed and the object is deleted, and removed from h_cache;
//	- if user forgets to call osclose(), the stream remains opened (alive) until
//		~MvHandlesCache for h_cache closes/deletes all registered objects.


/*ALN:TEST: following class is to investigate its destruction, tests show, that
	destructor is called on pointer, passed as 2nd parameter to utf8_locale(),
	from locale::Locimp destructor
**
class my_utf8_codecvt_facet: public boost::filesystem::detail::utf8_codecvt_facet
{
  public:
	my_utf8_codecvt_facet()
	: boost::filesystem::detail::utf8_codecvt_facet()
	{
		i++;
	}

	~my_utf8_codecvt_facet()
	{
		--i;
	}
  private:
	static int i;
};

int my_utf8_codecvt_facet::i = 0;
*/

std::locale get_locale(const var& locale_name) // throw (MVException)
{
	//assume is checked prior to calling since this is an internal exodus function
	//THISIS(L"std::locale get_locale(const var& locale_name)")
	//ISSTRING(locale_name)

	if (not locale_name.length() || locale_name == L"utf8")
	{
//		typedef wchar_t ucs4_t;
		std::locale old_locale;
//		std::locale utf8_locale(old_locale, new boost::exodus::detail::utf8_codecvt_facet<ucs4_t>);

		//if (boost_utf8_facet.get() == 0)
		//	boost_utf8_facet.reset(new boost::filesystem::detail::utf8_codecvt_facet());
		//std::locale utf8_locale(old_locale, boost_utf8_facet.get());

		std::locale utf8_locale(old_locale, new boost::filesystem::detail::utf8_codecvt_facet());
//ALN:TEST:		std::locale utf8_locale(old_locale, new my_utf8_codecvt_facet());
		return utf8_locale;
	}
	else
	{
		try {
			if (locale_name.length()) {
				std::locale mylocale(locale_name.toString().c_str());
				return mylocale;
			} else {
				//dont trust default locale since on osx 10.5.6 it fails!
				std::locale mylocale("C");
				return mylocale;
			}
		} catch(std::runtime_error re) {
			throw MVException(L"get_locale cannot create locale for " ^ locale_name);
		}
	}
}

bool checknotabsoluterootfolder(std::wstring dirname)
{
	//safety - will not rename/remove top level folders
	//cwd to top level and delete relative
	//top level folder has only one slash either at the beginning or, on windows, like x:\ .
	//NB copy/same code in osrmdir and osrename
	if (
			(
				!SLASH_IS_BACKSLASH
				&& dirname[0] ==SLASH_
				&& std::count(dirname.begin(), dirname.end(), SLASH_) < 3
			)
		||	(
				SLASH_IS_BACKSLASH
				&& (dirname[1] == L':')
				&& (dirname[2] == SLASH_)
			)
		)
	{
		std::wcout << "Forced removal/renaming of top two level directories by absolute path is not supported for safety but you can use cwd() and relative path." <<dirname << std::endl;
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

	//var time=(*this).swap(L"^\\D+|\\D+$", L"", L"r").swap(L"\\D+", L" ", L"r");

	static std_boost::wregex surrounding_nondigits_regex(L"^\\D+|\\D+$",std_boost::regex::extended|std_boost::regex_constants::icase);

	static std_boost::wregex inner_nondigits_regex(L"\\D+",std_boost::regex::extended|std_boost::regex_constants::icase);

	var time=var(std_boost::regex_replace(toTstring((*this)),surrounding_nondigits_regex, L""));
	time=var(std_boost::regex_replace(toTstring((time)),inner_nondigits_regex, L" "));

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

#ifndef BOOST_HAS_ICU
#	define boost_mvstr toTstring((*this))
#	define boost_regex_replace std_boost::regex_replace
	static const std_boost::wregex
		digits_regex		(L"\\d+"		,std_boost::regex::extended), // \d numeric
		alpha_regex			(L"[^\\W\\d]+"	,std_boost::regex::extended), // \a alphabetic
		alphanum_regex		(L"\\w+"		,std_boost::regex::extended), // \w alphanumeric
		non_digits_regex	(L"[^\\d]+"		,std_boost::regex::extended), // \D non-numeric
		non_alpha_regex		(L"[\\W\\d]+"	,std_boost::regex::extended), // \A non-alphabetic
		non_alphanum_regex	(L"\\W+"		,std_boost::regex::extended); // \W non-alphanumeric
#else
#	define boost_mvstr var_mvstr
#	define boost_regex_replace std_boost::u32regex_replace
	static const std_boost::u32regex
		digits_regex=boost::make_u32regex(L"\\d+"	,std_boost::regex::extended); //\d numeric
	static const std_boost::u32regex
		alpha_regex=boost::make_u32regex(L"[^\\W\\d]+"	,std_boost::regex::extended); // \a alphabetic
	static const std_boost::u32regex
		alphanum_regex=boost::make_u32regex(L"\\w+"	,std_boost::regex::extended); // \w alphanumeric
	static const std_boost::u32regex
		non_digits_regex=boost::make_u32regex(L"[^\\d]+",std_boost::regex::extended); // \D non-numeric
	static const std_boost::u32regex
		non_alpha_regex=boost::make_u32regex(L"[\\W\\d]+",std_boost::regex::extended); // \A non-alphabetic
	static const std_boost::u32regex
		non_alphanum_regex=boost::make_u32regex(L"\\W+"	,std_boost::regex::extended); // \W non-alphanumeric
#endif

	//negate if /
	if (*conversionchar==L'/')
	{
		++conversionchar;
		switch (*conversionchar)
		{
			// MC/N return everything except digits i.e. remove all digits 0123456789
			case 'N':
			{
				//var_mvstr=std_boost::regex_replace(toTstring((*this)),digits_regex, L"");
				var_mvstr=boost_regex_replace(boost_mvstr,digits_regex, L"");
				break;
			}

			// MC/A return everything except "alphabetic" i.e remove all "alphabetic"
			case 'A':
			{
				//var_mvstr=std_boost::regex_replace(toTstring((*this)),alpha_regex, L"");
				var_mvstr=boost_regex_replace(boost_mvstr,alpha_regex, L"");
				break;
			}
			// MC/B return everything except "alphanumeric" remove all "alphanumeric"
			case 'B':
			{
				//var_mvstr=std_boost::regex_replace(toTstring((*this)),alphanum_regex, L"");
				var_mvstr=boost_regex_replace(boost_mvstr,alphanum_regex, L"");
				break;
			}
		}
		return *this;
	}

	//http://www.boost.org/doc/libs/1_37_0/libs/regex/doc/html/boost_regex/ref/regex_replace.html
	//std_boost::regex_replace

	switch (*conversionchar)
	{
		// MCN return only digits i.e. remove all non-digits
		case 'N':
		{
			//var_mvstr=std_boost::regex_replace(toTstring((*this)),non_digits_regex, L"");
			var_mvstr=boost_regex_replace(boost_mvstr,non_digits_regex, L"");
			break;
		}
		// MCA return only "alphabetic" i.e. remove all "non-alphabetic"
		case 'A':
		{
			//var_mvstr=std_boost::regex_replace(toTstring((*this)),non_alpha_regex, L"");
			var_mvstr=boost_regex_replace(boost_mvstr,non_alpha_regex, L"");
			break;
		}
		// MCB return only "alphanumeric" i.e. remove all "non-alphanumeric"
		case 'B':
		{
			//var_mvstr=std_boost::regex_replace(toTstring((*this)),non_alphanum_regex, L"");
			var_mvstr=boost_regex_replace(boost_mvstr,non_alphanum_regex, L"");
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
//decimal constants is unsigned only in C99" ie this number exceed max SIGNED integer
//		(*threads_random_base_generator).seed(static_cast<unsigned int>(std::time(0)+2375472354));
		(*threads_random_base_generator).seed(static_cast<unsigned int>(std::time(0)+2075472354));
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
	THISISDEFINED()

	//get/init the base generator
	random_base_generator_type* threads_random_base_generator=get_random_base_generator();

	uint64_t seed;
	if (this->isnum())
		seed=this->toLong();
	else {
		//seed=0;
	        //for (size_t ii = 0; ii < var_mvstr.size(); ii++)
	        //        seed+=var_mvstr[ii];
		seed=MurmurHash64((wchar_t*)var_mvstr.data(),int(var_mvstr.length()*sizeof(wchar_t)),0);
	}
	//set the new seed
	//logputl(L"Seeding random number generator to " ^ (*this));
	//(*threads_random_base_generator).seed(static_cast<unsigned int>((*this).toInt() ));
	//(*threads_random_base_generator).seed(static_cast<unsigned int>(seed));
	(*threads_random_base_generator).seed(static_cast<uint64_t>(seed));
}

//only here really because boost regex is included here for file matching
bool var::match(const var& matchstr, const var& options) const
{
	THISIS(L"bool var::match(const var& matchstr, const var& options) const")
	THISISSTRING()
	ISSTRING(matchstr)

	//TODO fully implement (w=wildcard)
	if (options.index(L"w"))
	{
		if (matchstr==L""||var_mvstr==matchstr||matchstr==L"*.*")
			{}
		else if (matchstr[1]==L"*"&&substr(-matchstr.length()+1)==matchstr[2])
			{}
		else if (matchstr[-1]==L"*"&&substr(1,matchstr.length()-1)==matchstr.substr(1,matchstr.length()-1))
			{}
		else
			return false;
		return true;
	}

	//http://www.boost.org/doc/libs/1_38_0/libs/regex/doc/html/boost_regex/syntax/basic_syntax.html

	//TODO automatic caching of regular expressions or new exodus datatype to handle them

#ifndef BOOST_HAS_ICU
	std_boost::wregex regex;
	try
	{
		if (options.index(L"i"))
			regex.assign(toTstring(matchstr), std_boost::regex::extended|std_boost::regex_constants::icase);
		else
			regex.assign(toTstring(matchstr), std_boost::regex::extended|std_boost::regex_constants::icase);
	}
	catch (std_boost::regex_error& e)
    {
		throw MVException(var(e.what()).quote() ^ L" is an invalid regular expression");
	}

        return regex_match(toTstring((*this)), regex);
#else
	std_boost::u32regex regex;
	try
	{
		if (options.index(L"i"))
                        regex=boost::make_u32regex(matchstr.var_mvstr, std_boost::regex::extended|std_boost::regex_constants::icase);
		else
                        regex=boost::make_u32regex(matchstr.var_mvstr, std_boost::regex::extended);
	}
	catch (std_boost::regex_error& e)
    {
		throw MVException(var(e.what()).quote() ^ L" is an invalid regular expression");
	}

        return u32regex_match(var_mvstr, regex);
#endif

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

	if (options.length()!=0)
	{
#ifndef BOOST_HAS_ICU
		//http://www.cplusplus.com/reference/regex/basic_regex/assign/


                //TODO automatic caching of regular expressions or new exodus datatype to handle them
                std_boost::wregex regex;
                try
                {
                        if (options==(L"ri"))
                                regex.assign(toTstring(what), std_boost::regex::extended|std_boost::regex_constants::icase);
                        else if (options==(L"r"))
                                regex.assign(toTstring(what), std_boost::regex::extended);
                        else if (options==(L"i"))
                                regex.assign(toTstring(what), std_boost::regex::extended|std_boost::regex_constants::icase|std_boost::regex::literal);
                        else
                                regex.assign(toTstring(what), boost::regex_constants::literal);
                //boost::wregex toregex_regex(with.var_mvstr, boost::regex::extended);
                }
                catch (std_boost::regex_error& e)
                {
                        throw MVException(var(e.what()).quote() ^ L" is an invalid regular expression");
                }
/*
		//TODO automatic caching of regular expressions or new exodus datatype to handle them
		std_boost::wregex regex;
		try
		{
			if (options==(L"i") || options==(L"ri"))
                                regex.assign(toTstring(what), std_boost::regex::ECMAScript|std_boost::regex_constants::icase);
			else
				//regex.assign(toTstring(what), std_boost::regex_constants::literal);
				regex.assign(toTstring(what));
		//std_boost::wregex toregex_regex(with.var_mvstr, std_boost::regex::extended);
		}
		catch (std_boost::regex_error& e)
		{
			throw MVException(var(e.what()).quote() ^ L" is an invalid regular expression");
		}
*/
		//return regex_match(var_mvstr, expression);

		//std::wostringstream outputstring(std::ios::out | std::ios::binary);
    //std::ostream_iterator<wchar_t, wchar_t> oiter(outputstring);
		//std_boost::regex_replace(oiter, var_mvstr.begin(), var_mvstr.end(),regex_regex, with, boost::match_default | boost::format_all);
                var_mvstr=var(std_boost::regex_replace(toTstring((*this)),regex, toTstring(with))).var_mvstr;

#else
		//http://www.boost.org/doc/libs/1_38_0/libs/regex/doc/html/boost_regex/syntax/basic_syntax.html

		//TODO automatic caching of regular expressions or new exodus datatype to handle them
		std_boost::u32regex regex;
		try
		{
			/*
			if (options.index(L"i"))
				regex=boost::make_u32regex(what.var_mvstr,
					std_boost::regex::extended|std_boost::regex_constants::icase);
			else
				regex=boost::make_u32regex(what.var_mvstr,
					std_boost::regex::extended);
			//std_boost::wregex toregex_regex(with.var_mvstr, std_boost::regex::extended);
			*/
			if (options==(L"ri"))
				regex=boost::make_u32regex(what.var_mvstr,std_boost::regex::extended|std_boost::regex_constants::icase);
			else if (options==(L"r"))
				regex=boost::make_u32regex(what.var_mvstr,std_boost::regex::extended);
			else if (options==(L"i"))
				regex=boost::make_u32regex(what.var_mvstr,std_boost::regex_constants::icase|std_boost::regex::literal);
			else
				regex=boost::make_u32regex(what.var_mvstr,std_boost::regex::literal);
		//std_boost::wregex toregex_regex(with.var_mvstr, std_boost::regex::extended);
		}
		catch (std_boost::regex_error& e)
		{
			throw MVException(var(e.what()).quote() ^ L" is an invalid regular expression");
		}

		//return regex_match(var_mvstr, expression);

		//std::wostringstream outputstring(std::ios::out | std::ios::binary);
		//std::ostream_iterator<wchar_t, wchar_t> oiter(outputstring);
		//std_boost::regex_replace(oiter, var_mvstr.begin(), var_mvstr.end(),regex_regex, with, boost::match_default | boost::format_all);
		var_mvstr=std_boost::u32regex_replace(var_mvstr,regex, with.var_mvstr);

#endif

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

	//return whole environment if blank envvarname
	if (envvarname.var_mvstr.length()==0) {
		var_mvstr=L"xxx";
		var_mvtyp=pimpl::MVTYPE_STR;

		int i = 1;
		char *s = *environ;
 		for (; s; i++) {
			//printf("%s\n", s);
			var_mvstr.append(boost::locale::conv::utf_to_utf<wchar_t>(s));
			var_mvstr.append(L"\n");
			s = *(environ+i);
		}
		return true;
	}

	#pragma warning (disable : 4996)
	const char* cvalue=std::getenv(envvarname.toString().c_str());
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
//var(L"USING PUTENV").outputl();
	//is this safe on windows??
	//https://www.securecoding.cert.org/confluence/display/seccode/POS34-C.+Do+not+call+putenv()+with+a+pointer+to+an+automatic+variable+as+the+argument
	std::string tempstr=envvarname.toString();
	tempstr+="=";
	tempstr+=toString();
//var(tempstr).outputl(L"temp std:string");
//std::cout<<tempstr<<" "<<tempstr.length()<<std::endl;

	//this will NOT work reliably since putenv will NOT COPY the local (i.e. temporary) variable string

	//#pragma warning (disable : 4996)
	const int result=putenv((char*)(tempstr.c_str()));
	if (result==0)
		return true;
	else
		return false;

#else
	return setenv((char*)(envvarname.toString().c_str()),(char*)(toString().c_str()),1);
#endif

}

var var::osshell() const
{
	THISIS(L"var var::osshell() const")
	//will be checked again by toString()
	//but put it here so any unassigned error shows in osshell
	THISISSTRING()

	breakoff();
	int shellresult=system(toString().c_str());
	breakon();

	return shellresult;
}

var var::osshellread() const
{
	THISIS(L"var var::osshellread() const")
	//will be checked again by toString()
	//but put it here so any unassigned error shows in osshell
	THISISSTRING()

	var output=L"";

	//fflush?

	//"r" means read
	std::FILE *cmd=popen(toString().c_str(), "r");
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
	//will be checked again by toString()
	//but put it here so any unassigned error shows in osshell
	THISISSTRING()
	ISSTRING(writestr)

	//"w" means read
	std::FILE *cmd=popen(toString().c_str(), "w");
	//return a code to indicate program failure. but can this ever happen?
	if (cmd==NULL)
		return 1;
	fputs(writestr.toString().c_str(),cmd);

	//return the process termination status (pity cant do this for read too)
	return pclose(cmd);

}

var var::suspend() const
{
	//THISIS(L"var var::suspend() const")

	breakoff();
	//use dummy to avoid warning in gcc4 "warning: ignoring return value of int system(const char*), declared with attribute warn_unused_result"
	int dummy=system(toString().c_str());
	breakon();

	return L"";

	//evade warning: unused variable 'dummy'
	if (dummy) {};
}

void var::osflush() const
{
	//THISIS(L"void var::osflush() const")

	std::wcout<<L"var::osflush ignored - not implemented yet"<<std::endl;
	return;
}

bool var::osopen() const
{
	THISIS(L"bool var::osopen()")
	THISISSTRING()

	//if reopening an osfile that is already opened then close and reopen
	//dont call if not necessary
	if (THIS_IS_OSFILE())
		osclose();

	return osopenx(*this, L"")!=0;

}

bool var::osopen(const var& osfilename, const var& locale) const
{
	THISIS(L"bool var::osopen(const var& osfilename, const var& locale)")
	THISISDEFINED()
	ISSTRING(osfilename)

	//if reopening an osfile that is already opened then close and reopen
	//dont call if not necessary
	if (THIS_IS_OSFILE())
		osclose();

	return osopenx(osfilename, locale)!=0;

}

static void del_wfstream(void * handle)
{
	delete (std::wfstream *) handle;
}

std::wfstream* var::osopenx(const var& osfilename, const var& locale) const
{

	//IDENTICAL code in osbread and osbwrite
	//Try to get the cached file handle. the usual case is that you osopen a file before doing osbwrite/osbread
	//Using wfstream instead of wofstream so that we can mix reads and writes on the same filehandle
	std::wfstream * pmyfile = 0;
	if (THIS_IS_OSFILE())
	{
		pmyfile = (std::wfstream *) mv_handles_cache.get_handle((int) this->var_mvint, this->var_mvstr);
		if (pmyfile == 0)		// nonvalid handle
		{
			this->var_mvint = 0;
//			this->var_mvtyp ^= pimpl::MVTYPE_OSFILE;	// clear bit
			this->var_mvtyp ^= pimpl::MVTYPE_OSFILE | pimpl::MVTYPE_INT;	//only STR bit should remains
		}
	}

	//if the file has NOT already been opened then open it now with the current default locale and add it to the cache.
	//but normally the filehandle will have been previously opened with osopen and perhaps a specific locale.
	if (pmyfile == 0)
	{

		//delay checking until necessary
		THISIS(L"bool var::osopenx(const var& osfilename, const var& locale)")
		ISSTRING(osfilename)
		ISSTRING(locale)

		pmyfile = new std::wfstream;

		//what is the purpose of the following?
		//to prevent locale conversion if writing narrow string to wide stream or vice versa
		//imbue BEFORE opening or after flushing
		//myfile.imbue(std::locale(std::locale::classic(), new NullCodecvt));
/*
		try
		{
			pmyfile->imbue(std::locale(locale.toString().c_str()));
		}
		catch (...)
		{
			throw MVException(locale^L" is not supported on this system");
		}
*/
		pmyfile->imbue(get_locale(locale));

		//open the file for i/o (fail if the file doesnt exist and do NOT delete any existing file)
		//binary and in/out to allow reading and writing from same file handle
		pmyfile->open(osfilename.toString().c_str(), std::ios::out | std::ios::in | std::ios::binary);
		if (! (*pmyfile))
		{
			delete pmyfile;
			return 0;
		}

		//cache the file handle (we use the int to store the "file number"
		//and NAN to prevent isnum trashing mvint in the possible case that the osfilename is an integer
		//can addhandle fail?
		this->var_mvint = mv_handles_cache.add_handle(pmyfile, del_wfstream, osfilename.var_mvstr);
		this->var_mvtyp = pimpl::MVTYPE_NANSTR_OSFILE;
		this->var_mvstr = osfilename.var_mvstr;
	}

	return pmyfile;
}

//on unix or with iconv we could convert to or any character format
//for sequential output we could use popen to progressively read and write with no seek facility
//popen("iconv -f utf-16 -t utf-8 >targetfile","w",filehandle) followed by progressive writes
//popen("iconv -f utf-32 -t utf-16 <sourcefile","r",filehandle) followed by progressive reads
//use utf-16 or utf-32 depending on windows or unix

bool var::osread(const var& osfilename, const var& locale)
{
	THISIS(L"bool var::osread(const var& osfilename)")
	//will be checked by nested osread
	//THISISDEFINED()
	ISSTRING(osfilename)
	return osread(osfilename.toString().c_str(), locale);
}

bool var::osread(const char* osfilename, const var& locale)
{

	//osread unicode eg FM fails on ubuntu linux 13.10 x64 for some reason
	//oswrite works! ... use osbreadx 9999999 as workaround
	//maybe try vector/pushback instead of file.read(
	//http://www.boost.org/doc/libs/1_55_0/libs/serialization/doc/codecvt.html

	THISIS(L"bool var::osread(const var& osfilename)")
	THISISDEFINED()

	//osread returns empty string in any case
	var_mvstr=L"";
	var_mvtyp=pimpl::MVTYPE_STR;

	//what is the purpose of the following?
	//RAW IO to prevent locale conversion if writing narrow string to wide stream or vice versa
	//imbue BEFORE opening or after flushing
	//myfile.imbue(std::locale(std::locale::classic(), new NullCodecvt));

	//get a file structure configured to the right locale (locale provides a CodeCvt facet)
	std::wifstream myfile;
	myfile.imbue(get_locale(locale));

	//open in binary (and position "at end" to find the file size with tellg)
	myfile.open(osfilename, std::ios::binary | std::ios::in | std::ios::ate );
	if (!myfile)
		return false;

	//determine the file size since we are going to read it all
	//NB tellg and seekp goes by bytes regardless of normal/wide stream
	unsigned int bytesize;
	//	#pragma warning( disable: 4244 )
	//warning C4244: '=' : conversion from 'std::streamoff' to 'unsigned int', possible loss of data
	bytesize = (unsigned int) myfile.tellg();

	//if empty file then done ok
	if (bytesize==0)
	{
		myfile.close();
		return	true;
	}

	//get file size * wchar memory to load the file or fail
	boost::scoped_array<wchar_t> memblock(new wchar_t [bytesize]);
	if (memblock==0)
	{
		throw MVOutOfMemory(L"Could not obtain "^var(int(bytesize*sizeof(wchar_t)))^L" bytes of memory to read "^var(osfilename));
		//myfile.close();
		//return false;
	}

	//read the file into the reserved memory block
	myfile.seekg (0, std::ios::beg);
	myfile.read (memblock.get(), (unsigned int) bytesize);

	bool failed = myfile.fail();

	// NOTE: in "utf8" mode, number of read characters in memblock is less then requested.
	//		As such, to prevent garbage in tail of returned string, do:
	//	#pragma warning( disable: 4244 )
	//warning C4244: '=' : conversion from 'std::streamoff' to 'unsigned int', possible loss of data
	bytesize = (unsigned int) myfile.gcount();
	myfile.close();

	//failure can indicate that we didnt get as many characters as requested due to
	//utf8 encoding in file
	if (failed && !bytesize) {
		return false;
	}

	//ALN:JFI: actually we could use std::string 'tempstr' in place of 'memblock' by hacking
	//	.data member and reserve() or resize(), thus avoiding buffer-to-buffer-copying
	var_mvstr=std::wstring(memblock.get(), (unsigned int) bytesize);

	return true;
}

bool var::oswrite(const var& osfilename, const var& locale) const
{
	THISIS(L"bool var::oswrite(const var& osfilename) const")
	THISISSTRING()
	ISSTRING(osfilename)

	//what is the purpose of the following?
	//to prevent locale conversion if writing narrow string to wide stream or vice versa
	//imbue BEFORE opening or after flushing
	//myfile.imbue(std::locale(std::locale::classic(), new NullCodecvt));

	//get a file structure configured to the right locale (locale provides a CodeCvt facet)
	std::wofstream myfile;
	myfile.imbue(get_locale(locale));

	//although the stream works with wchar_t, its parameter file name is narrow char *
	//delete any previous file,
	myfile.open(osfilename.toString().c_str(), std::ios::trunc | std::ios::out | std::ios::binary);
	if (!myfile)
		return false;

	//write out the full string or fail
	myfile.write(var_mvstr.data(), int(var_mvstr.length()));
	bool failed = myfile.fail();
	myfile.close();
	return ! failed;

}

bool var::osbwrite(const var& osfilevar, var& startoffset) const
{
	//osfilehandle is just the filename but buffers the "file number" in the mvint too

	THISIS(L"bool var::osbwrite(const var& osfilevar, var& startoffset) const")
	THISISSTRING()
	//test the following only if necessary in osopenx
	//ISSTRING(osfilename)

	//get the buffered file handle/open on the fly
	std::wfstream * pmyfile = osfilevar.osopenx(osfilevar,L"");
	if (pmyfile == 0)
		return false;

	//NB seekp goes by bytes regardless of the fact that it is a wide stream
	//myfile.seekp (startoffset*sizeof(wchar_t));
	//startoffset should be in bytes except for fixed multibyte code pages like UTF16 and UTF32
	pmyfile->seekp (static_cast<long> (startoffset.var_mvint));	// avoid warning, see comments to seekg()

	//NB but write length goes by number of wide characters (not bytes)
	pmyfile->write(var_mvstr.data(),int(var_mvstr.length()));

	//on windows, wfstream will try to convert to current locale codepage so
	//if you are trying to write an exodus string containing a GREEK CAPITAL GAMMA
	//unicode \x0393 and current codepage is *NOT* CP1253 (Greek)
	//then c++ wiofstream cannot convert \x0393 to a single byte (in CP1253)
	if (pmyfile->fail())
	{
		//saved in cache, DO NOT CLOSE!
		//myfile.close();
		return false;
	}

	//pass back the file pointer offset
	startoffset = (int) pmyfile->tellp();

	//although slow, ensure immediate visibility of osbwrites
	pmyfile->flush();

	// saved in cache, DO NOT CLOSE!
	//myfile.close();

	return true;
}

var& var::osbread(const var& osfilevar, var& startoffset, const int bytesize)
{
	THISIS(L"var& var::osbread(const var& osfilevar, const int startoffset, const int size")
	THISISDEFINED()
	//will be done if necessary in osopenx()
	//ISSTRING(osfilename)

	//default is to return empty string in any case
	var_mvstr=L"";
	var_mvtyp=pimpl::MVTYPE_STR;

	//strange case request to read 0 bytes
	if (bytesize<=0)
		return *this;

	//get the buffered file handle/open on the fly
	std::wfstream * pmyfile = osfilevar.osopenx(osfilevar,L"");
	if (pmyfile == 0)
		return *this;
/*
	//NB all file sizes are in bytes NOT characters despite this being a wide character fstream
	// Position get pointer at the end of file, as expected it to be if we open file anew
	pmyfile->seekg(0, std::ios::end);
	unsigned int maxsize = pmyfile->tellg();

var(int(maxsize)).outputl(L"maxsize=");
	//return "" if start reading past end of file
	if ((unsigned long)(int)startoffset>=maxsize)	// past EOF
		return *this;

*/
	//seek to the startoffset
        //if (pmyfile->tellg() != static_cast<long> (startoffset.var_mvint))
	{
		if (pmyfile->fail())
			pmyfile->clear();
		//pmyfile->seekg (static_cast<long> (startoffset.var_mvint), std::ios::beg);	// 'std::streampos' usually 'long'
		//seekg always seems to result in tellg being -1 in linux (Ubunut 10.04 64bit)
		pmyfile->rdbuf()->pubseekpos(static_cast<long> (startoffset.var_mvint));
	}
//var((int) pmyfile->tellg()).outputl(L"2 tellg=");

	//get a memory block to read into
	boost::scoped_array<wchar_t> memblock(new wchar_t [bytesize]);
	if (memblock==0)
		throw MVOutOfMemory(L"Could not obtain "^var(int(bytesize*sizeof(wchar_t)))^L" bytes of memory to read "^osfilevar);
		//return *this;

	//read the data (converting characters on the fly)
	pmyfile->read (memblock.get(), bytesize);

	//bool failed = pmyfile.fail();
	if (pmyfile->fail())
	{
		pmyfile->clear();
		pmyfile->seekg(0, std::ios::end);
	}

	//update the startoffset function argument
	//if (readsize > 0)
	startoffset = (int) pmyfile->tellg();

	//transfer the memory block to this variable's string
	//(is is possible to read directly into string data() avoiding a memory copy?
	//get the number of CHARACTERS read - utf8 bytes (except ASCII) convert to fewer wchar characters.
	//int readsize = pmyfile->gcount();
	//	#pragma warning( disable: 4244 )
	//warning C4244: '=' : conversion from 'std::streamoff' to 'unsigned int', possible loss of data
	var_mvstr.assign(memblock.get(), (unsigned int) pmyfile->gcount());

	return *this;

}

void var::osclose() const
{
	//THISIS(L"void var::osclose() const")
	//THISISSTRING()
	if (THIS_IS_OSFILE())
	{
		mv_handles_cache.del_handle((int) var_mvint);
		var_mvint = 0L;
		var_mvtyp ^= pimpl::MVTYPE_OSFILE | pimpl::MVTYPE_INT;	//only STR bit should remains
	}
	// in all other cases, the files should be closed.
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
	myfile.open(newosdir_or_filename.toString().c_str(), std::ios::binary );
	if (myfile)
	{
		//RELEASE
		myfile.close();
		return false;
	}

	//safety
	if (!checknotabsoluterootfolder(toWString()))
		return false;

	return !std::rename(toString().c_str(),newosdir_or_filename.toString().c_str());

}

bool var::oscopy(const var& to_osfilename) const
{
	THISIS(L"bool var::oscopy(const var& to_osfilename) const")
	THISISSTRING()
	ISSTRING(to_osfilename)

    //stdfs::wpath frompathx(toTstring((*this)).c_str());
    //stdfs::wpath topathx(toTstring(to_osfilename).c_str());
    stdfs::path frompathx((*this).toString().c_str());
    stdfs::path topathx(to_osfilename.toString().c_str());
    try
    {
		//will not overwrite so this is redundant
		//option to overwrite is not in initial versions of boost copy_file
		if (stdfs::exists(topathx)) return false;

		stdfs::copy_file(frompathx, topathx);
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
	osfilename.osclose();		// in case this is cached opened file handle
	return !std::remove(osfilename.toString().c_str());
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
		//stdfs::wpath pathx(toTstring((*this)).c_str());
		stdfs::path pathx((*this).toString().c_str());

        if (!stdfs::exists(pathx)) return L"";
        //is_regular is only in boost > 1.34
		//if (!stdfs::is_regular(pathx)) return L"";
        if (stdfs::is_directory(pathx)) return L"";

        //get last write datetime
        std::time_t last_write_time=std::chrono::system_clock::to_time_t(stdfs::last_write_time(pathx));
        //convert to ptime
        boost::posix_time::ptime ptimex = boost::posix_time::from_time_t(last_write_time);
        //convert to mv date and time
        int mvdate, mvtime;
        ptime2mvdatetime(ptimex, mvdate, mvtime);

		return int(stdfs::file_size(pathx)) ^ FM ^ mvdate ^ FM ^ int(mvtime);

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
		//stdfs::wpath pathx(toTstring((*this)).c_str());
		stdfs::path pathx((*this).toString().c_str());

		if (stdfs::exists(pathx)) return false;
		stdfs::create_directories(pathx);
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
		//stdfs::wpath pathx(toTstring((*this)).c_str());
		stdfs::path pathx((*this).toString().c_str());

        if (!stdfs::exists(pathx)) return false;
        if (!stdfs::is_directory(pathx)) return false;

		if (evenifnotempty)
		{

			//safety
			if (!checknotabsoluterootfolder(toWString()))
				return false;

			stdfs::remove_all(pathx);
		}
		else
			stdfs::remove(pathx);
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
    //stdfs::wpath pathx(toTstring((*this)).c_str());
    try
    {

		//boost 1.33 throws an error with files containing ~ or $ chars but 1.38 doesnt
		stdfs::path pathx((*this).toString().c_str());

        if (!stdfs::exists(pathx)) return L"";
        if (!stdfs::is_directory(pathx)) return L"";

        //get last write datetime
        std::time_t last_write_time=std::chrono::system_clock::to_time_t(stdfs::last_write_time(pathx));
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
	//std_boost::wregex re;
	std_boost::regex re;
	if (spec)
	{
		try
		{
			// Set up the regular expression for case-insensitivity
			//re.assign(toTstring(spec).c_str(), std_boost::regex_constants::icase);
			re.assign(spec.toString().c_str(), std_boost::regex_constants::icase);
			filter=true;
		}
		catch (std_boost::regex_error& e)
		{
			std::wcout << spec.var_mvstr << L" is not a valid regular expression: \""
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

#if BOOST_FILESYSTEM_VERSION >= 3 or defined(C17)
#define LEAForFILENAME path().filename().string()
#define COMMAstdfsNATIVE
#else
#define LEAForFILENAME leaf()
#define COMMAstdfsNATIVE ,stdfs::native
#endif
	//get handle to folder
	//wpath or path before boost 1.34
	//stdfs::wpath full_path(stdfs::initial_path<stdfs::wpath>());
    //full_path = stdfs::system_complete(stdfs::wpath(toTstring(path), stdfs::native ));
	//stdfs::path full_path(stdfs::initial_path());
	//initial_path always return the cwd at the time it is first called which is almost useless
	stdfs::path full_path(stdfs::current_path());
	full_path = stdfs::system_complete
		(
			stdfs::path
			(
				path.toString().c_str() COMMAstdfsNATIVE
			)
		);

	//quit if it isnt a folder
	if (!stdfs::is_directory(full_path))
		return filelist;

	//errno=0;
    //stdfs::wdirectory_iterator end_iter;
    //for (stdfs::wdirectory_iterator dir_itr(full_path );
    stdfs::directory_iterator end_iter;
    for (stdfs::directory_iterator dir_itr(full_path );
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
		if (filter&&!std_boost::regex_match(dir_itr->LEAForFILENAME, re))
			continue;

		//using .leaf instead of .status provided in boost 1.34 .. but does it work/efficiently
        	//if (stdfs::is_directory(dir_itr->status() ) )
		if (stdfs::is_directory(*dir_itr ) )
		{
			if (getfolders)
				filelist^=FM ^ dir_itr->LEAForFILENAME;
		}
		//is_regular is only in boost > 1.34
		//else if (stdfs::is_regular(dir_itr->status() ) )
		else// if (stdfs::is_regular(dir_itr->status() ) )
		{
			if (getfiles)
				filelist^=FM ^ dir_itr->LEAForFILENAME;
		}
		//else
		//{
		//  //++other_count;
		//  //std::wcout << dir_itr->path().leaf() << L" [other]\n";
		//}
      	}
      	catch (const std::exception & ex )
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

bool var::oscwd(const var& newpath) const
{
	THISIS(L"var var::oscwd(const var& newpath) const")
	//doesnt use *this - should syntax be changed to setcwd? and getcwd()?
	THISISDEFINED()//not needed if *this not used
	ISSTRING(newpath)

	//http://www.boost.org/doc/libs/1_38_0/libs/filesystem/doc/reference.html#Attribute-functions
	//wont compile on boost 1.33 so comment it out and make non-functional
	//until we have a reliable way to detect boost version
	//boost::filesystem::current_path(newpath.toString());

	//return oscwd();

	try {
		stdfs::current_path(newpath.toString());
	}
	catch (...) {
		//filesystem error: cannot set current path: No such file or directory
		//ignore all errors
		return false;
	}

	return true;

}

var var::oscwd() const
{
	THISIS(L"var var::oscwd() const")
	//doesnt use *this - should syntax be changed to ossetcwd? and osgetcwd()?
	THISISDEFINED()//not needed if *this not used

	//TODO consider using complete() or system_complete()
	//"[Note: When portable behavior is required, use complete(). When operating system dependent behavior is required, use system_complete()."

	//http://www.boost.org/doc/libs/1_38_0/libs/filesystem/doc/reference.html#Attribute-functions
	//std::string currentpath=boost::filesystem::current_path().string();
	std::string currentpath=stdfs::current_path().string();

	return var(currentpath).convert(L"/",SLASH);

/*
	//http://www.boost.org/libs/filesystem/doc/tr2_proposal.html#Class-template-basic_path
	//boost::filesystem::path full_path(boost::filesystem::current_path());
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
    boost::xtime_get(&xt, boost::TIME_UTC_);
    xt.nsec += milliseconds*1000000;
    boost::thread::sleep(xt);

//	usleep((int)(1000.0*milliseconds));
//	Sleep(1000*milliseconds);
}

}// namespace exodus
