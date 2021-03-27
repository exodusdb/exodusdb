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

// boost wpath is only in boost v1.34+ but we hardcoded path at the moment which
// prevents unicode in osfilenames etc

// EXCELLENT!
// http://www.regular-expressions.info/
// http://www.regular-expressions.info/unicode.html

//#include <istream>
//#include <ostream>
#include <algorithm>  //for count in osrename
#include <cstdlib>	  //for getenv and setenv/putenv
//#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <locale>

//used to convert to and from utf8 in osread and oswrite
#include <boost/locale.hpp>

// TODO check all error handling
// http://www.ibm.com/developerworks/aix/library/au-stdfs/index.html
// catch(boost::filesystem::filesystem_error e) {
//#include <boost/filesystem.hpp>
//#include <experimental/filesystem>
//namespace stdfs = std::experimental::filesystem;
//#include <filesystem>
//namespace stdfs = std::filesystem;
#if __has_include(<filesystem>)
#include <filesystem>
namespace stdfs = std::filesystem;
#define ABSOLUTE_OR_COMPLETE absolute
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace stdfs = std::experimental::filesystem;
#define ABSOLUTE_OR_COMPLETE system_complete
#else
error "Missing the <filesystem> header."
#endif

// show c++ version for info
//#include <boost/preprocessor/stringize.hpp>
//#pragma message "__cplusplus=" BOOST_PP_STRINGIZE(__cplusplus)

#define USE_BOOST_REGEX

#ifndef USE_BOOST_REGEX
#include <regex>
#define std_boost std

#else
#define std_boost boost
#include <boost/regex/config.hpp>

#if !defined(BOOST_HAS_ICU) && !defined(BOOST_HASNT_ICU)
#define BOOST_HAS_ICU
#endif
#ifdef BOOST_HAS_ICU
//#pragma message "BOOST_HAS_ICU"
#include <boost/regex/icu.hpp>
#endif
#include <boost/regex.hpp>

#endif

//#include <boost/scoped_ptr.hpp>
//#include <boost/scoped_array.hpp>

// regex.assign("", std_boost::regex::extended|std_boost::regex_constants::icase);

#include <boost/thread/tss.hpp>

//#include <boost/date_time/gregorian/gregorian.hpp>
// not needed on ubuntu 14.04 x64 c++11 build so comment out - maybe needed on other platforms
//#include <boost/date_time/posix_time/posix_time.hpp>

// for sleep
#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>

// for rnd and initrnd
#include <boost/random.hpp>

// for exodus_once
#include <boost/thread/once.hpp>

// traditional Unix file I/O interface declared in <fcntl.h>
// (under Unix and Linux) or <out.h> (Windows)

// FindFile family of Win32 SDK functions such as
// FindFirstFileEx, FindNextFile and CloseFind

// see http://www.boost.org/libs/filesystem/test/wide_test.cpp
// for boost filesystem wide operations and UTF8

// to get whole environment
extern char** environ;

// boost changed from TIME_UTC to TIME_UTC_ (when?) to avoid a new standard TIME_UTC macro in C11
// time.h #include <boost/version.hpp> #if BOOST_VERSION < 105000 #define TIME_UTC_ TIME_UTC #endif

// for initrnd from string
//#include <MurmurHash2_64.h>

// oshell capturing output needs posix popen
#if defined(_MSC_VER) && !defined(popen)
#define popen _popen
#define pclose _pclose
#endif

#include <exodus/mv.h>
//#include <exodus/mvutf.h>
#include <exodus/mvexceptions.h>

#include "mvhandles.h"

boost::once_flag exodus_once_flag = BOOST_ONCE_INIT;

void exodus_once_func() {
	// so wcscoll() sorts a<B etc instead of character number wise where B<a
	// wcscoll currently only used on non-pc/non-mac ... see var::localeAwareCompare
	// so has no effect on those platform for < <= etc
	// only en_US.utf-8 seems to be widely available on all platforms
	//.utf8 causes a utf-8 to wchar conversion when converting string to wstring by iterators
	//.begin() .end() and we want to be able to control conversion more accurately than an
	// automatic conversion en_US is missing from Ubuntu. en_IN is available on Ubuntu and
	// Centos but not MacOSX if (!setlocale (LC_COLLATE, "en_US.utf8"))
	if (!setlocale(LC_ALL, "en_US.utf8")) {
		if (!setlocale(LC_ALL, "C.UTF-8"))
			std::cout << "Cannot setlocale LC_COLLATE to en_US.utf8" << std::endl;
	};
	// std::cout << std::cout.getloc().name() << std::endl;
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

class ExodusOnce {
   public:
	ExodusOnce() { boost::call_once(&exodus_once_func, exodus_once_flag); }
};
static ExodusOnce exodus_once_static;

namespace exodus {

// this object caches fstream * pointers, to avoid multiple reopening files
// extern MvHandlesCache mv_handles_cache;
// Lifecircle of fstream object:
//	- created (by new) and opened in osbread()/osbwrite();
//	- pointer to created object stored in h_cache;
//  - when user calls osclose(), the stream is closed and the object is deleted, and removed from
//  h_cache;
//	- if user forgets to call osclose(), the stream remains opened (alive) until
//		~MvHandlesCache for h_cache closes/deletes all registered objects.

std::locale get_locale(const var& locale_name)	// throw (MVError)
{
	// assume is checked prior to calling since this is an internal exodus function
	// THISIS("std::locale get_locale(const var& locale_name)")
	// ISSTRING(locale_name)

	if (not locale_name.length() || locale_name == "utf8") {
		std::locale old_locale;

		//https://exceptionshub.com/read-unicode-utf-8-file-into-wstring.html
		//wif.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));

		std::locale utf8_locale(old_locale,
								new std::codecvt_utf8<wchar_t>);

		return utf8_locale;
	} else {
		try {
			if (locale_name.length()) {
				std::locale mylocale(locale_name.toString().c_str());
				return mylocale;
			} else {
				// dont trust default locale since on osx 10.5.6 it fails!
				std::locale mylocale("C");
				return mylocale;
			}
		} catch (std::runtime_error& re) {
			throw MVError("get_locale cannot create locale for " ^ locale_name);
		}
	}
}

bool checknotabsoluterootfolder(std::string dirname) {
	// safety - will not rename/remove top level folders
	// cwd to top level and delete relative
	// top level folder has only one slash either at the beginning or, on windows, like x:\ .
	// NB copy/same code in osrmdir and osrename
	if ((!SLASH_IS_BACKSLASH && dirname[0] == OSSLASH_ &&
		 std::count(dirname.begin(), dirname.end(), OSSLASH_) < 3) ||
		(SLASH_IS_BACKSLASH && (dirname[1] == ':') && (dirname[2] == OSSLASH_))) {
		std::cout
			<< "Forced removal/renaming of top two level directories by absolute path is "
			   "not supported for safety but you can use cwd() and relative path."
			<< dirname << std::endl;
		return false;
	}
	return true;
}

// ICONV_MT can be moved back to mvioconv.cpp if it stops using regular expressions
// regular expressions for ICONV_MT
// var var::iconv_MT(const char* conversion) const
var var::iconv_MT() const {
	// ignore everything else and just get first three groups of digits "99 99 99"
	// remove leading and trailing non-digits and replace internal strings of non-digits with
	// single space

	// var time=(*this).swap("^\\D+|\\D+$", "", "r").swap("\\D+", " ", "r");

	static std_boost::regex surrounding_nondigits_regex("^\\D+|\\D+$",
														std_boost::regex_constants::icase);

	static std_boost::regex inner_nondigits_regex("\\D+", std_boost::regex_constants::icase);

	var time = var(std_boost::regex_replace(this->var_str, surrounding_nondigits_regex, ""));
	time = var(std_boost::regex_replace(time.var_str, inner_nondigits_regex, " "));

	int hours = time.field(" ", 1).toInt();
	int mins = time.field(" ", 2).toInt();
	int secs = time.field(" ", 3).toInt();

	int inttime = hours * 3600 + mins * 60 + secs;

	if (inttime >= 86400)
		return "";

	// PM
	if (inttime < 43200 && (*this).index("P"))
		inttime += 43200;
	else if (inttime >= 43200 && (*this).index("A"))
		inttime -= 43200;

	return inttime;
}

// OCONV_MR can be moved back to mvioconv.cpp if it stops using regular expressions
// regular expressions for ICONV_MC
var& var::oconv_MR(const char* conversionchar) {
	// conversionchar arrives pointing to 3rd character (eg A in MCA)

	// abort if no 3rd char
	if (*conversionchar == '\0')
		return (*this);

	// in case changes to/from numeric
	var_typ = VARTYP_STR;

	// form of unicode specific regular expressions
	// http://www.regular-expressions.info/unicode.html

	// availability of unicode regular expressions
	// http://www.boost.org/doc/libs/1_35_0/libs/regex/doc/html/boost_regex/unicode.html

	/*
	\w=alphanumeric characters, \W all the rest
	\d=decimal characters, \D all the rest

	but there is no such code to indicate ALPHABETIC (NON_DECIMAL) characters only (i.e. \w but
	not \d) Imagine we also had the codes \a alphabetic characters, \A all the rest \a = [^\W\d]
	... which is characters which are a) "NOT NON-WORD CHARACTERS" and b) "NOT DECIMAL DIGITS"
	\A = [\W\d]  ... which is a) NON-WORD CHARACTERS and b) decimal digits

	if boost has icu then \w and \d includes letters and decimal digits from all languages

	123456|abcdefghi|!"$%^&* ()-+[]
	dddddd|DDDDDDDDD|DDDDDDDDDDDDDD all characters divided into \d (decimal)		and
	\D (non-decimals) AAAAAA|aaaaaaaaa|AAAAAAAAAAAAAA all characters divided into \a
	(alphabetic)		and \A (non-alphabetic) wwwwww|wwwwwwwww|WWWWWWWWWWWWWW all
	characters divided into \w (alphanumeric)	and \W (non-alphanumeric)
	*/

	/* unicode regex extensions
	http://userguide.icu-project.org/strings/regexp
	http://www.regular-expressions.info/unicode.html
	sometimes unicode extension for regular expressions have \pL for \a and [^\pL] for \A
	Match a word character. Word characters are [\p{Ll}\p{Lu}\p{Lt}\p{Lo}\p{Nd}].
	l=lowercase u=uppercase t=titlecase o=other Nd=decimal digit
	\N{UNICODE CHARACTER NAME}	Match the named character.
	\p{UNICODE PROPERTY NAME}	Match any character with the specified Unicode Property.
	\P{UNICODE PROPERTY NAME}	Match any character not having the specified Unicode
	Property.
	*/

#ifndef BOOST_HAS_ICU
#define boost_mvstr toTstring((*this))
#define boost_regex_replace std_boost::regex_replace
	static const std_boost::regex digits_regex("\\d+"),	 // \d numeric
		alpha_regex("[^\\W\\d]+"),						 // \a alphabetic
		alphanum_regex("\\w+"),							 // \w alphanumeric
		non_digits_regex("[^\\d]+"),					 // \D non-numeric
		non_alpha_regex("[\\W\\d]+"),					 // \A non-alphabetic
		non_alphanum_regex("\\W+");						 // \W non-alphanumeric
#else
#define boost_mvstr this->var_str
#define boost_regex_replace std_boost::u32regex_replace
	static const std_boost::u32regex digits_regex = boost::make_u32regex("\\d+");  //\d numeric
	static const std_boost::u32regex alpha_regex =
		boost::make_u32regex("[^\\W\\d]+");	 // \a alphabetic
	static const std_boost::u32regex alphanum_regex =
		boost::make_u32regex("\\w+");  // \w alphanumeric
	static const std_boost::u32regex non_digits_regex =
		boost::make_u32regex("[^\\d]+");  // \D non-numeric
	static const std_boost::u32regex non_alpha_regex =
		boost::make_u32regex("[\\W\\d]+");	// \A non-alphabetic
	static const std_boost::u32regex non_alphanum_regex =
		boost::make_u32regex("\\W+");  // \W non-alphanumeric
#endif

	// negate if /
	if (*conversionchar == '/') {
		++conversionchar;
		switch (*conversionchar) {
			// MC/N return everything except digits i.e. remove all digits 0123456789
			case 'N': {
				// this->var_str=std_boost::regex_replace(toTstring((*this)),digits_regex, "");
				this->var_str = boost_regex_replace(boost_mvstr, digits_regex, "");
				break;
			}

			// MC/A return everything except "alphabetic" i.e remove all "alphabetic"
			case 'A': {
				// this->var_str=std_boost::regex_replace(toTstring((*this)),alpha_regex, "");
				this->var_str = boost_regex_replace(boost_mvstr, alpha_regex, "");
				break;
			}
			// MC/B return everything except "alphanumeric" remove all "alphanumeric"
			case 'B': {
				// this->var_str=std_boost::regex_replace(toTstring((*this)),alphanum_regex, "");
				this->var_str = boost_regex_replace(boost_mvstr, alphanum_regex, "");
				break;
			}
		}
		return *this;
	}

	// http://www.boost.org/doc/libs/1_37_0/libs/regex/doc/html/boost_regex/ref/regex_replace.html
	// std_boost::regex_replace

	switch (*conversionchar) {
		// MCN return only digits i.e. remove all non-digits
		case 'N': {
			// this->var_str=std_boost::regex_replace(toTstring((*this)),non_digits_regex, "");
			this->var_str = boost_regex_replace(boost_mvstr, non_digits_regex, "");
			break;
		}
		// MCA return only "alphabetic" i.e. remove all "non-alphabetic"
		case 'A': {
			// this->var_str=std_boost::regex_replace(toTstring((*this)),non_alpha_regex, "");
			this->var_str = boost_regex_replace(boost_mvstr, non_alpha_regex, "");
			break;
		}
		// MCB return only "alphanumeric" i.e. remove all "non-alphanumeric"
		case 'B': {
			// this->var_str=std_boost::regex_replace(toTstring((*this)),non_alphanum_regex, "");
			this->var_str = boost_regex_replace(boost_mvstr, non_alphanum_regex, "");
			break;
		}
		// MCL to lower case
		case 'L': {
			lcaser();
			break;
		}
		// MCU to upper case
		case 'U': {
			ucaser();
			break;
		}
	}

	return *this;
}

// BOOST RANDOM
// http://www.boost.org/doc/libs/1_38_0/libs/random/random_demo.cpp

// set the generator type to ...
// using random_base_generator_type = boost::minstd_rand;
using random_base_generator_type = boost::mt19937;
boost::thread_specific_ptr<random_base_generator_type> tss_random_base_generators;

random_base_generator_type* get_random_base_generator() {
	// get/init the base generator
	random_base_generator_type* threads_random_base_generator =
		tss_random_base_generators.get();
	if (!threads_random_base_generator) {
		tss_random_base_generators.reset(new random_base_generator_type);
		threads_random_base_generator = tss_random_base_generators.get();
		if (!threads_random_base_generator)
			throw MVError("Could not create random number generator");

		// seed to the os clock (secs since unix epoch)
		// Caveat: std::time(0) is not a very good truly-random seed.
		// logputl("Seeding random number generator to system clock");
		// decimal constants is unsigned only in C99" ie this number exceed max SIGNED
		// integer
		//		(*threads_random_base_generator).seed(static_cast<unsigned
		// int>(std::time(0)+2375472354));
		(*threads_random_base_generator)
			.seed(static_cast<unsigned int>(std::time(0) + 2075472354));
		//(*thread_base_generator).seed(static_cast<unsigned int>(var().ostime().toInt()));
	}
	return threads_random_base_generator;
}

var var::rnd() const {
	THISIS("var var::rnd() const")
	THISISNUMERIC()

	// get/init the base generator
	random_base_generator_type* threads_random_base_generator = get_random_base_generator();

	// pick rnd(5) returns pseudo random integers in the range of 0-4
	int max = (*this).toInt() - 1;
	if (max < 0)
		return var(*this);

	// define a integer range (0 to n-1)
	boost::uniform_int<> uni_dist(0, max);

	// use the base generator and the range to create a specific generator
	boost::variate_generator<random_base_generator_type&, boost::uniform_int<>> uni(
		*threads_random_base_generator, uni_dist);

	// generate a pseudo random number
	return uni();
}

void var::initrnd() const {
	THISIS("void var::initrnd(const var& seed) const")
	THISISDEFINED()

	// get/init the base generator
	random_base_generator_type* threads_random_base_generator = get_random_base_generator();

	// seed from number
	uint64_t seed;
	if (this->isnum()) {
		seed = this->toLong();

		// seed from string
	} else {
		seed = 1;
		for (size_t ii = 0; ii < this->var_str.size(); ii++)
			seed *= this->var_str[ii];
		// seed=MurmurHash64((char*)this->var_str.data(),int(this->var_str.length()*sizeof(char)),0);
	}
	// set the new seed
	// logputl("Seeding random number generator to " ^ (*this));
	//(*threads_random_base_generator).seed(static_cast<unsigned int>((*this).toInt() ));
	//(*threads_random_base_generator).seed(static_cast<unsigned int>(seed));
	(*threads_random_base_generator).seed(static_cast<uint64_t>(seed));
}

int get_regex_syntax_flags(const var& options) {
	// determine options from string

	// default
	// ECMAScript	Use the Modified ECMAScript regular expression grammar
	// collate	Character ranges of the form "[a-b]" will be locale sensitive.
	int regex_syntax_flags = std_boost::regex_constants::collate;

	// i = icase
	if (options.index("i"))
		regex_syntax_flags |= std_boost::regex_constants::icase;

	// m = multiline (TODO should be present c++17 onwards)
	// Specifies that ^ shall match the beginning of a line and $ shall match the end of a line,
	// if the ECMAScript engine is selected. if (options.index("m"))
	//	regex_syntax_flags|=std_boost::regex_constants::multiline;

	// b = basic (withdrawn after c++17?)
	// Use the basic POSIX regular expression grammar
	// if (options.index("b"))
	//	regex_syntax_flags|=std_boost::regex_constants::basic;

	// e = extended
	// Use the extended POSIX regular expression grammar
	if (options.index("e"))
		regex_syntax_flags |= std_boost::regex_constants::extended;

	// l - literal TODO manually implement
	// ignore all usual regex special characters
	if (options.index("l"))
		regex_syntax_flags |= std_boost::regex_constants::literal;

	return regex_syntax_flags;
}

// should be in mvfuncs.cpp - here really because boost regex is included here for file matching
var var::match(const var& matchstr, const var& options) const {
	// VISUALISE REGULAR EXPRESSIONS GRAPHICALLY!
	// https:www.debuggex.com

	THISIS("bool var::match(const var& matchstr, const var& options) const")
	THISISSTRING()
	ISSTRING(matchstr)

	// wild cards like
	// *.* or *.???
	// *abcde
	// abcde*
	if (options.index("w")) {

		// rules of glob - converting glob to regex

		// 0. the following regexe special characters are not special so, if present in
		// match expression, they must be escaped
		// https://stackoverflow.com/questions/1252992/how-to-escape-a-string-for-use-in-boost-regex
		// const std_boost::regex esc("[.^$|()\\[\\]{}*+?\\\\]");
		// allow * and ? in glob expressions
		const std_boost::regex regex_special_chars{R"raw([.^$|()\[\]{}+\\])raw"};
		const std::string replacement_for_regex_special = R"raw(\\$&)raw";
		std::string matchstr2 = regex_replace(matchstr.var_str, regex_special_chars,
											  replacement_for_regex_special);

		// 1. force to match whole string
		var matchstr3 = "^" + matchstr2 + "$";

		// 2. * matches zero or more characters like .* in regex
		matchstr3.swapper("*", ".*");

		// 3. ? matches any one character
		matchstr3.swapper("?", ".");

		// having created a suitable regex str, recursively call match with it
		return this->match(matchstr3);
	}

	// TODO automatic caching of regular expressions or new exodus datatype to handle them

	/*
	//using ECMAScript grammar. It's the default grammar and offers far more features that the
	other grammars. Most C++ references talk as if C++11 implements regular expressions as
	defined in the ECMA-262v3 and POSIX standards. But in reality the C++ implementation is very
	loosely based these standards. The syntax is quite close. The only significant differences
	are that std::regex supports POSIX classes even in ECMAScript mode, and that it is a bit
	peculiar about which characters must be escaped (like curly braces and closing square
	brackets) and which must not be escaped (like letters).

	But there are important differences in the actual behavior of this syntax.
	The caret and dollar always match at embedded line breaks in std::regex, while in JavaScript
	and POSIX this is an option. Backreferences to non-participating groups fail to match as in
	most regex flavors, while in JavaScript they find a zero-length match. In JavaScript, \d and
	\w are ASCII-only while \s matches all Unicode whitespace. This is odd, but all modern
	browsers follow the spec. In std::regex all the shorthands are ASCII-only when using strings
	of char.
	*/

	// U32REGEX/MATCH/SEARCH/REPLACE/TOKEN_ITERATOR ARE THIN ITERATOR WRAPPERS TO MAKE UTF8 ETC
	// APPEAR LIKE UTF32
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// u32regex etc come from <boost/regex/icu.hpp>
	// the header <boost/regex/icu.hpp> provides a series of thin wrappers around these
	// algorithms,
	// called u32regex_match, u32regex_search, and u32regex_replace. These wrappers use
	// iterator-adapters internally
	// to make external UTF-8 or UTF-16 data look as though it's really a UTF-32 sequence, that
	// can then be passed on to the "real" algorithm.
	// https://www.boost.org/doc/libs/1_70_0/libs/regex/doc/html/boost_regex/ref/non_std_strings/icu/unicode_algo.html

	// ICU in BOOST REGEX
	// https://www.boost.org/doc/libs/1_34_1/libs/regex/doc/icu_strings.html

	std_boost::u32regex regex;
	try {
		regex = boost::make_u32regex(matchstr.var_str, get_regex_syntax_flags(options));
	} catch (std_boost::regex_error& e) {
		throw MVError(var(e.what()).quote() ^ " is an invalid regular expression");
	}

	/*
		//create iterators to matches
		//https://www.boost.org/doc/libs/1_70_0/libs/regex/doc/html/boost_regex/ref/non_std_strings/icu/unicode_iter.html
		//auto iter {std_boost::make_u32regex_token_iterator(this->var_str,regex)};
		const int subs[] = {1, 2, 3, 0};
		boost::u32regex_token_iterator<std::string::const_iterator> iter
	   {std_boost::make_u32regex_token_iterator(this->var_str,regex,subs)}; decltype(iter) end {};

		//cycle through matches, appending whatever is found to the output
		std::string found="";
		int ii {0};
		while (iter != end)
		{
			std::cout<< ++ii << " " <<*iter<<std::endl;
			//std::cout<<(*iter).first<<std::endl;
			//std::cout << iter->size() << std::endl;
			found.append(*iter);
			found.push_back(FM_);
			++iter;
		}
	*/
	std::string found;

	// https://stackoverflow.com/questions/26320987/what-is-the-difference-between-regex-token-iterator-and-regex-iterator
	// boost::u32regex_token_iterator<std::string::const_iterator>
	//	iter {std_boost::make_u32regex_token_iterator(this->var_str,regex,{0,1,2,3,4,etc or -1})};
	// token_iterator allow you to access none-matching parts of the string for parsing stuff
	// but doesnt return an iterator with an array of groups

	// construct our iterators:
	boost::u32regex_iterator<std::string::const_iterator> iter{
		std_boost::make_u32regex_iterator(this->var_str, regex)};
	decltype(iter) end{};
	std::for_each(iter, end,
				  // using declarative functional style "for_each with lambda" instead of old
				  // fashioned "while (iter!=end)" loop
				  [&found](auto what) {
					  for (int groupn = 0; uint(groupn) <= what.size(); ++groupn) {
						  // std::cout<< what[0] << std::endl;
						  found.append(what[groupn]);
						  found.push_back(VM_);
					  }
					  if (!found.empty())
						  while (found.back() == VM_)
							  found.pop_back();
					  found.push_back(FM_);
				  });

	if (!found.empty())
		found.pop_back();

	return found;

	/*	std_boost::smatch
			smatch1;
		//return u32regex_match(this->var_str, regex);
		while (u32regex_search(
			this->var_str,
			smatch1,
			regex,
			std_boost::regex_constants::match_default))
		{
			std::cout << smatch1.str() << '\n';
			for (size_t i = 0; i < smatch1.size(); ++i)
			{
				//std::cout << i << ": " << smatch1[i] << '\n';
				found.append(smatch1[i]);
				//separate groups by VM
				found.push_back(VM_);
			}
			//separate matches by FM
			if (!found.empty())
				found.back()=FM_;
		}
		if (!found.empty())
			found.pop_back();

		return found;
	*/
}

// simple case sensitive substr replacement
var var::swap(const var& what, const var& with) const& {
	var newmv = *this;
	return newmv.swapper(what, with);
}

// on temporaries
var& var::swap(const var& what, const var& with) && {
	return this->swapper(what, with);
}

// in-place
var& var::swapper(const var& what, const var& with) {
	THISIS("var& var::swapper(const var& what, const var& with)")
	THISISSTRINGMUTATOR()
	ISSTRING(what)
	ISSTRING(with)

	// nothing to do if oldstr is ""
	if (what.var_str.empty())
		return *this;

	// find the starting position of the field or return
	std::string::size_type start_pos = 0;
	while (true) {
		start_pos = this->var_str.find(what.var_str, start_pos);
		// past of of string?
		if (start_pos == std::string::npos)
			return *this;
		this->var_str.replace(start_pos, what.var_str.length(), with.var_str);
		start_pos += with.var_str.length();
	}

	return *this;
}

//regex based string replacement
// only here really because boost regex is included here for file matching
var var::replace(const var& regexstr, const var& replacementstr, const var& options) const& {
	var newmv = *this;
	return newmv.replacer(regexstr, replacementstr, options);
}

// on temporary
var& var::replace(const var& regexstr, const var& replacementstr, const var& options) && {
	return this->replacer(regexstr, replacementstr, options);
}

// in-place
var& var::replacer(const var& regexstr, const var& replacementstr, const var& options) {
	THISIS(
		"var& var::replacer(const var& regexstr, const var& replacementstr, const var& "
		"options)")
	THISISSTRINGMUTATOR()
	ISSTRING(regexstr)
	ISSTRING(replacementstr)
	ISSTRING(options)

	// http://www.boost.org/doc/libs/1_38_0/libs/regex/doc/html/boost_regex/syntax/basic_syntax.html

	// TODO automatic caching of regular expressions or new exodus datatype to handle them
	std_boost::u32regex regex;
	try {
		regex = boost::make_u32regex(regexstr.var_str, get_regex_syntax_flags(options));
	} catch (std_boost::regex_error& e) {
		throw MVError(var(e.what()).quote() ^ " is an invalid regular expression");
	}

	// return regex_match(this->var_str, expression);

	// std::ostringstream outputstring(std::ios::out | std::ios::binary);
	// std::ostream_iterator<char, char> oiter(outputstring);
	// std_boost::regex_replace(oiter, this->var_str.begin(), this->var_str.end(),regex_regex, with,
	// boost::match_default | boost::format_all);
	this->var_str = std_boost::u32regex_replace(this->var_str, regex, replacementstr.var_str);

	return *this;
}

bool var::osgetenv(const var& envvarname) {
	THISIS("bool var::osgetenv(const var& envvarname)")
	THISISDEFINED()
	ISSTRING(envvarname)

	// return whole environment if blank envvarname
	if (envvarname.var_str.length() == 0) {

		this->var_str.clear();
		var_typ = VARTYP_STR;

		int i = 1;
		char* s = *environ;
		for (; s; i++) {
			// printf("%s\n", s);
			// this->var_str.append(boost::locale::conv::utf_to_utf<wchar_t>(s));
			this->var_str.append(s);
			this->var_str.append("\n");
			s = *(environ + i);
		}
		return true;
	}

	//TIP if you cant seem to osgetenv vars set in bash, then ensure you set them in bash with "export"

#pragma warning(disable : 4996)
	const char* cvalue = std::getenv(envvarname.var_str.c_str());
	if (cvalue == 0) {
		this->var_str.clear();
		var_typ = VARTYP_STR;
		return false;
	} else
		*this = var(cvalue);
	return true;
}

bool var::ossetenv(const var& envvarname) const {
	THISIS("bool var::ossetenv(const var& envvarname) const")
	THISISSTRING()
	ISSTRING(envvarname)

//#ifdef _MSC_VER
#ifndef setenv

	/* on windows this should be used
	BOOL WINAPI SetEnvironmentVariable(LPCTSTR lpName, LPCTSTR lpValue);
	*/
	// var("USING PUTENV").outputl();
	// is this safe on windows??
	// https://www.securecoding.cert.org/confluence/display/seccode/POS34-C.+Do+not+call+putenv()+with+a+pointer+to+an+automatic+variable+as+the+argument
	//std::string tempstr = envvarname.toString();
	//tempstr += "=";
	//tempstr += toString();
	// var(tempstr).outputl("temp std:string");
	// std::cout<<tempstr<<" "<<tempstr.length()<<std::endl;

	// this will NOT work reliably since putenv will NOT COPY the local (i.e. temporary)
	// variable string

	//var("putenv " ^ var(tempstr) ).outputl();
	//#pragma warning (disable : 4996)
	//const int result = putenv((char*)(tempstr.c_str()));
	//putenv("EXO_DBNAME=C:\\");
	//std::cout<<getenv("EXO_DBNAME");

	//char winenv[1024];
	char* env = (char*)malloc(1024);
	snprintf(env, 1024, "%s=%s", envvarname.var_str.c_str(), this->var_str.c_str());
	//std::cout << winenv;
	int result = putenv(env);

	if (result == 0)
		return true;
	else
		return false;

#else
	var("setenv " ^ envvarname ^ "=" ^ (*this)).outputl();
	return setenv((char*)(envvarname.toString().c_str()), (char*)(toString().c_str()), 1);
#endif
}

var var::ostempdirname() const {
	std::error_code error_code;
	std::string dirname = std::string(stdfs::temp_directory_path(error_code));
	if (dirname.back() != OSSLASH_)
		dirname.push_back(OSSLASH_);
	return dirname;
}

//this will actually create files liek /tmp/~XXXXXX that should be deleted if not used
var var::ostempfilename() const {
	//TODO replace with mkstemp
	//https://cpp.hotexamples.com/examples/-/-/mkstemp/cpp-mkstemp-function-examples.html
	//return std::string(std::tmpnam(nullptr));
	var tempfilename = this->ostempdirname();

	char* tmp;
	tmp = new char[strlen(tempfilename.var_str.c_str()) + 8];
	strcpy(tmp, tempfilename.var_str.c_str());
	strcat(tmp, "~XXXXXX");
	int fd;
	if ((fd = mkstemp(tmp)) == -1) {
		delete tmp;
		//fprintf(stderr, "Failed creating temp file.\n");
		return "nope";
	}
	tempfilename = tmp;
	delete tmp;
	return tempfilename;
}

bool var::suspend() const {
	return this->osshell();
}

bool var::osshell() const {
	THISIS("var var::osshell() const")
	// will be checked again by toString()
	// but put it here so any unassigned error shows in osshell
	THISISSTRING()

	// breakoff();
	int shellresult = system(this->to_cmd_string().c_str());
	// breakon();

	return !shellresult;
}

bool var::osshellread(const var& oscmd) {
	THISIS("var var::osshellread() const")
	// will be checked again by toString()
	// but put it here so any unassigned error shows in osshell
	ISSTRING(oscmd)

	// default is to return empty string in any case
	this->var_str.clear();
	var_typ = VARTYP_STR;

	// fflush?

	//"r" means read
	std::FILE* pfile = popen(oscmd.to_cmd_string().c_str(), "r");
	// return a code to indicate program failure. but can this ever happen?
	if (pfile == NULL)
		return false;
	// TODO buffer overflow check
	char cstr1[4096] = {0x0};
	while (std::fgets(cstr1, sizeof(cstr1), pfile) != NULL) {
		// std::printf("%s\n", result);
		// cannot trust that standard input is convertable from utf8
		// output.var_str+=wstringfromUTF8((const UTF8*)result,strlen(result));
		//std::string str1 = cstr1;
		//output.var_str += std::string(str1.begin(), str1.end());
		//readstr.var_str += std::string(cstr1);
		this->var_str += cstr1;
	}

	//return true if no error code
	return !pclose(pfile);
}

bool var::osshellwrite(const var& writestr) const {
	THISIS("var var::osshellwrite(const var& writestr) const")
	// will be checked again by toString()
	// but put it here so any unassigned error shows in osshell
	THISISSTRING()
	ISSTRING(writestr)

	//"w" means read
	std::FILE* cmd = popen(this->to_cmd_string().c_str(), "w");
	// return a code to indicate program failure. but can this ever happen?
	if (cmd == NULL)
		return 1;

	// decided not to convert slashes here .. may be the wrong decision
	fputs(writestr.toString().c_str(), cmd);

	// return the process termination status
	return !pclose(cmd);
}

void var::osflush() const {
	std::cout << std::flush;
	return;
}

bool var::osopen() const {
	THISIS("bool var::osopen()")
	THISISSTRING()

	// if reopening an osfile that is already opened then close and reopen
	// dont call if not necessary
	if (THIS_IS_OSFILE())
		osclose();

	return this->osopenx(*this, "") != 0;
}

bool var::osopen(const var& osfilename, const var& locale) const {
	THISIS("bool var::osopen(const var& osfilename, const var& locale)")
	THISISDEFINED()
	ISSTRING(osfilename)

	// if reopening an osfile that is already opened then close and reopen
	// dont call if not necessary
	if (THIS_IS_OSFILE())
		osclose();

	return this->osopenx(osfilename, locale) != 0;
}

static void del_fstream(void* handle) {
	delete (std::fstream*)handle;
}

std::fstream* var::osopenx(const var& osfilename, const var& locale) const {

	// IDENTICAL code in osbread and osbwrite
	// Try to get the cached file handle. the usual case is that you osopen a file before doing
	// osbwrite/osbread Using fstream instead of ofstream so that we can mix reads and writes on
	// the same filehandle
	std::fstream* pmyfile = 0;
	if (THIS_IS_OSFILE()) {
		pmyfile =
			(std::fstream*)mv_handles_cache.get_handle((int)this->var_int, this->var_str);
		if (pmyfile == 0)  // nonvalid handle
		{
			var_int = 0;
			//			var_typ ^= VARTYP_OSFILE;	// clear bit
			var_typ ^= VARTYP_OSFILE | VARTYP_INT;	// only STR bit should remains
		}
	}

	// if the file has NOT already been opened then open it now with the current default locale
	// and add it to the cache. but normally the filehandle will have been previously opened
	// with osopen and perhaps a specific locale.
	if (pmyfile == 0) {

		// delay checking until necessary
		THISIS("bool var::osopenx(const var& osfilename, const var& locale)")
		ISSTRING(osfilename)
		ISSTRING(locale)

		pmyfile = new std::fstream;

		pmyfile->imbue(get_locale(locale));

		// open the file for i/o (fail if the file doesnt exist and do NOT delete any
		// existing file) binary and in/out to allow reading and writing from same file
		// handle
		pmyfile->open(osfilename.to_path_string().c_str(),
					  std::ios::out | std::ios::in | std::ios::binary);
		if (!(*pmyfile)) {
			//delete pmyfile;

			//try to open read-only
			pmyfile->open(osfilename.to_path_string().c_str(),
						  std::ios::in | std::ios::binary);
			if (!(*pmyfile)) {
				delete pmyfile;
				return 0;
			}
		}

		// cache the file handle (we use the int to store the "file number"
		// and NAN to prevent isnum trashing mvint in the possible case that the osfilename
		// is an integer can addhandle fail?
		this->var_int =
			mv_handles_cache.add_handle(pmyfile, del_fstream, osfilename.var_str);
		this->var_typ = VARTYP_NANSTR_OSFILE;
		this->var_str = osfilename.var_str;
	}

	return pmyfile;
}

// on unix or with iconv we could convert to or any character format
// for sequential output we could use popen to progressively read and write with no seek facility
// popen("iconv -f utf-16 -t utf-8 >targetfile","w",filehandle) followed by progressive writes
// popen("iconv -f utf-32 -t utf-16 <sourcefile","r",filehandle) followed by progressive reads
// use utf-16 or utf-32 depending on windows or unix

/* typical code pages on Linux (see "iconv -l")
ISO-8859-2
ISO-8859-3
ISO-8859-5
ISO-8859-6
ISO-8859-7
ISO-8859-8
ISO-8859-9
EUC-JP
EUC-KR
ISO-8859-13
ISO-8859-15
GBK
GB18030
UTF-8
GB2312
BIG5
KOI8-R
KOI8-U
CP1251
TIS-620
WINDOWS-31J
WINDOWS-936
WINDOWS-1250
WINDOWS-1251
WINDOWS-1252
WINDOWS-1253
WINDOWS-1254
WINDOWS-1255
WINDOWS-1256
WINDOWS-1257
WINDOWS-1258
*/

// no binary conversion is performed on input unless
// codepage is provided then exodus converts from the
// specified codepage (not locale) on input to utf-8 internally
bool var::osread(const var& osfilename, const var& codepage) {
	THISIS("bool var::osread(const var& osfilename, const var& codepage")
	ISSTRING(osfilename)
	return osread(osfilename.to_path_string().c_str(), codepage);
}

bool var::osread(const char* osfilename, const var& codepage) {

	THISIS("bool var::osread(const char* osfilename, const var& codepage")
	THISISDEFINED()

	// osread returns empty string in any case
	this->var_str.clear();
	var_typ = VARTYP_STR;

	// get a file structure
	std::ifstream myfile;

	// open in binary (and position "at end" to find the file size with tellg)
	myfile.open(osfilename, std::ios::binary | std::ios::in | std::ios::ate);
	if (!myfile)
		return false;

	// determine the file size since we are going to read it all
	// NB tellg and seekp goes by bytes regardless of normal/wide stream
	// max file size 4GB?
	unsigned int bytesize;
	//	#pragma warning( disable: 4244 )
	// warning C4244: '=' : conversion from 'std::streamoff' to 'unsigned int', possible loss of
	// data
	bytesize = (unsigned int)myfile.tellg();

	// if empty file then done ok
	if (bytesize == 0) {
		myfile.close();
		return true;
	}

	// reserve memory - now reading directly into var_str
	// get file size * wchar memory to load the file or fail
	// boost::scoped_array<char> memblock(new char [bytesize]);
	// std::unique_ptr<char[]> memblock(new char[bytesize]);
	// if (memblock==0)
	try {
		// emergency memory - will be deleted at } - useful if OOM
		std::unique_ptr<char[]> emergencymemory(new char[16384]);
		//std::unique_ptr emergencymemory(new char[16384]);

		// resize the string to receive the whole file
		this->var_str.resize(bytesize);
	} catch (std::bad_alloc& ex) {
		throw MVOutOfMemory("Could not obtain " ^ var(int(bytesize * sizeof(char))) ^
							" bytes of memory to read " ^ var(osfilename));
		// myfile.close();
		// return false;
	}

	// read the file into the reserved memory block
	myfile.seekg(0, std::ios::beg);
	// myfile.read (memblock.get(), (unsigned int) bytesize);
	//myfile.read(&var_str[0], (unsigned int)bytesize);
	//c++17 provides non-const access to data() :)
	myfile.read(this->var_str.data(), (unsigned int)bytesize);

	bool failed = myfile.fail();

	// in case we didnt read the whole file for some reason, remove garbage in the end of the
	// string 	#pragma warning( disable: 4244 ) warning C4244: '=' : conversion from
	// 'std::streamoff' to 'unsigned int', possible loss of data
	bytesize = (unsigned int)myfile.gcount();
	this->var_str.resize(bytesize);
	myfile.close();

	// failure can indicate that we didnt get as many characters as requested
	if (failed && !bytesize) {
		return false;
	}

	// ALN:JFI: actually we could use std::string 'tempstr' in place of 'memblock' by hacking
	//	.data member and reserve() or resize(), thus avoiding buffer-to-buffer-copying
	// this->var_str=std::string(memblock.get(), (unsigned int) bytesize);
	// SJB Done 20190604

	if (codepage)
		// this->var_str=boost::locale::conv::to_utf<char>(this->var_str,"ISO-8859-5")};
		this->var_str = boost::locale::conv::to_utf<char>(this->var_str, codepage.var_str);

	return true;
}

var var::to_codepage(const var& codepage) const {
	THISIS(
		"bool var::to_codepage(const var& codepage="
		") const")
	THISISSTRING()
	ISSTRING(codepage)

	//from utf8 to codepage
	return boost::locale::conv::from_utf<char>(this->var_str, codepage.var_str);
}

var var::from_codepage(const var& codepage) const {
	THISIS(
		"bool var::from_codepage(const var& codepage="
		") const")
	THISISSTRING()
	ISSTRING(codepage)

	//to utf from codepage
	return boost::locale::conv::to_utf<char>(this->var_str, codepage.var_str);
}

// no binary conversion is performed on output unless
// codepage is provided (not locale) then exodus assumes internally
// utf-8 and converts all output to the specified codepage
bool var::oswrite(const var& osfilename, const var& codepage) const {
	THISIS(
		"bool var::oswrite(const var& osfilename, const var& codepage="
		") const")
	THISISSTRING()
	ISSTRING(osfilename)

	// get a file structure
	std::ofstream myfile;

	// delete any previous file,
	myfile.open(osfilename.to_path_string().c_str(),
				std::ios::trunc | std::ios::out | std::ios::binary);
	if (!myfile)
		return false;

	// write out the full string or fail
	if (codepage) {
		std::string tempstr = boost::locale::conv::from_utf<char>(this->var_str, codepage.var_str);
		myfile.write(tempstr.data(), int(tempstr.length()));
	} else {
		myfile.write(this->var_str.data(), int(this->var_str.length()));
	}
	bool failed = myfile.fail();
	myfile.close();
	return !failed;
}

// a version that accepts a const offset ie ignores return value
//bool var::osbwrite(const var& osfilevar, const var& offset, const bool adjust) const
bool var::osbwrite(const var& osfilevar, const var& offset) const {
	return this->osbwrite(osfilevar, const_cast<var&>(offset));
}

//NOTE: unlike osread/oswrite which rely on iconv codepages to do any conversion
//osbread and osbwrite rely on the locale being passed in on the osopen stage

//bool var::osbwrite(const var& osfilevar, var& offset, const bool adjust) const
bool var::osbwrite(const var& osfilevar, var& offset) const {
	// osfilehandle is just the filename but buffers the "file number" in the mvint too

	THISIS(
		"bool var::osbwrite(const var& osfilevar, var& offset) "
		"const")
	THISISSTRING()
	// test the following only if necessary in osopenx
	// ISSTRING(osfilename)

	// get the buffered file handle/open on the fly
	std::fstream* pmyfile = osfilevar.osopenx(osfilevar, "");
	if (pmyfile == 0)
		return false;

	// std::cout << pmyfile->getloc().name();

	// NB seekp goes by bytes regardless of the fact that it is a wide stream
	// myfile.seekp (offset*sizeof(char));
	// offset should be in bytes except for fixed multibyte code pages like UTF16 and UTF32
	pmyfile->seekp(
		static_cast<long>(offset.var_int));	 // avoid warning, see comments to seekg()

	// NB but write length goes by number of wide characters (not bytes)
	pmyfile->write(this->var_str.data(), int(this->var_str.length()));

	// on windows, fstream will try to convert to current locale codepage so
	// if you are trying to write an exodus string containing a GREEK CAPITAL GAMMA
	// unicode \x0393 and current codepage is *NOT* CP1253 (Greek)
	// then c++ wiofstream cannot convert \x0393 to a single byte (in CP1253)
	if (pmyfile->fail()) {
		// saved in cache, DO NOT CLOSE!
		// myfile.close();
		return false;
	}

	// pass back the file pointer offset
	offset = (int)pmyfile->tellp();

	// although slow, ensure immediate visibility of osbwrites
	pmyfile->flush();

	// saved in cache, DO NOT CLOSE!
	// myfile.close();

	return true;
}

// a version that ignores output of offset
//var& var::osbread(const var& osfilevar, const var& offset, const int bytesize,
//		  const bool adjust)
bool var::osbread(const var& osfilevar, const var& offset, const int bytesize) {
	// var offset_nonconst;
	// if (offset.assigned())
	//	offset_nonconst=offset;
	return this->osbread(osfilevar, const_cast<var&>(offset), bytesize);
}

ssize_t count_excess_UTF8_bytes(std::string& str) {

	// Scans backward from the end of string.
	const char* cptr = &str.back();
	int num = 1;
	int numBytesToTruncate = 0;

	for (int i = 0; 6 > i; ++i) {
		numBytesToTruncate += 1;
		if ((*cptr & 0x80) == 0x80) {
			// If char bit starts with 1xxxxxxx
			// It's a part of unicode character!
			// Find the first byte in the unicode character!

			// if ((*cptr & 0xFC) == 0xFC) { if (num == 6) { return 0; } break; }
			// if ((*cptr & 0xF8) == 0xF8) { if (num == 5) { return 0; } break; }

			// If char binary is 11110000, it means it's a 4 bytes long unicode.
			if ((*cptr & 0xF0) == 0xF0) {
				if (num == 4) {
					return 0;
				}
				break;
			}

			// If char binary is 11100000, it means it's a 3 bytes long unicode.
			if ((*cptr & 0xE0) == 0xE0) {
				if (num == 3) {
					return 0;
				}
				break;
			}

			if ((*cptr & 0xC0) == 0xC0) {
				if (num == 2) {
					return 0;
				}
				break;
			}

			num += 1;
		} else {
			// If char bit does not start with 1, nothing to truncate!
			return 0;
		}

		cptr -= 1;

	}  // next char

	return numBytesToTruncate;
}

//NOTE if the locale is not C then any partial non-utf-8 bytes at the end (due to bytesize
//not being an exact number of valid utf-8 code units) are trimmed off the return value
//The new offset is changed to reflect the above and is simply increased by bytesize

//var& var::osbread(const var& osfilevar, var& offset, const int bytesize, const bool adjust)
bool var::osbread(const var& osfilevar, var& offset, const int bytesize) {
	THISIS("bool var::osbread(const var& osfilevar, var& offset, const int bytesize")
	THISISDEFINED()
	ISNUMERIC(offset)

	// will be done if necessary in osopenx()
	// ISSTRING(osfilename)

	// default is to return empty string in any case
	this->var_str.clear();
	var_typ = VARTYP_STR;

	// strange case request to read 0 bytes
	if (bytesize <= 0)
		return true;

	// get the buffered file handle/open on the fly
	std::fstream* pmyfile = osfilevar.osopenx(osfilevar, "");
	if (pmyfile == 0)
		return false;
	/*
		//NB all file sizes are in bytes NOT characters despite this being a wide character
	fstream
		// Position get pointer at the end of file, as expected it to be if we open file
	anew pmyfile->seekg(0, std::ios::end); unsigned int maxsize = pmyfile->tellg();

	var(int(maxsize)).outputl("maxsize=");
		//return "" if start reading past end of file
		if ((unsigned long)(int)offset>=maxsize)	// past EOF
			return *this;

	*/
	// seek to the offset
	// if (pmyfile->tellg() != static_cast<long> (offset.var_int))
	{
		if (pmyfile->fail())
			pmyfile->clear();
		// pmyfile->seekg (static_cast<long> (offset.var_int), std::ios::beg);	//
		// 'std::streampos' usually 'long' seekg always seems to result in tellg being -1 in
		// linux (Ubunut 10.04 64bit)
		pmyfile->rdbuf()->pubseekpos(static_cast<long>(offset.var_int));
	}
	// var((int) pmyfile->tellg()).outputl("2 tellg=");

	// get a memory block to read into
	// boost::scoped_array<char> memblock(new char [bytesize]);
	std::unique_ptr<char[]> memblock(new char[bytesize]);
	//std::unique_ptr memblock(new char[bytesize]);
	if (memblock == 0)
		throw MVOutOfMemory("Could not obtain " ^ var(int(bytesize * sizeof(char))) ^
							" bytes of memory to read " ^ osfilevar);
	// return *this;

	// read the data (converting characters on the fly)
	pmyfile->read(memblock.get(), bytesize);

	// bool failed = pmyfile.fail();
	if (pmyfile->fail()) {
		pmyfile->clear();
		pmyfile->seekg(0, std::ios::end);
	}

	// update the offset function argument
	// if (readsize > 0)
	offset = (int)pmyfile->tellg();

	// transfer the memory block to this variable's string
	//(is is possible to read directly into string data() avoiding a memory copy?
	// get the number of CHARACTERS read - utf8 bytes (except ASCII) convert to fewer wchar
	// characters. int readsize = pmyfile->gcount(); 	#pragma warning( disable: 4244 )
	// warning C4244: '=' : conversion from 'std::streamoff' to 'unsigned int', possible loss of
	// data
	this->var_str.assign(memblock.get(), (unsigned int)pmyfile->gcount());

	// trim off any excess utf8 bytes if utf8
	//	var(pmyfile->getloc().name()).outputl(L"loc name=");;
	if (pmyfile->getloc().name() != "C") {
		int nextrabytes = count_excess_UTF8_bytes(this->var_str);
		if (nextrabytes) {
			offset -= nextrabytes;
			this->var_str.resize(this->var_str.length() - nextrabytes);
		}
	}

	return true;
}

void var::osclose() const {
	// THISIS("void var::osclose() const")
	// THISISSTRING()
	if (THIS_IS_OSFILE()) {
		mv_handles_cache.del_handle((int)var_int);
		var_int = 0L;
		var_typ ^= VARTYP_OSFILE | VARTYP_INT;	// only STR bit should remains
	}
	// in all other cases, the files should be closed.
}

bool var::osrename(const var& newosdir_or_filename) const {
	THISIS("bool var::osrename(const var& newosdir_or_filename) const")
	THISISSTRING()
	ISSTRING(newosdir_or_filename)

	// prevent overwrite of existing file
	// ACQUIRE
	std::ifstream myfile;
	// binary?
	myfile.open(newosdir_or_filename.to_path_string().c_str(), std::ios::binary);
	if (myfile) {
		// RELEASE
		myfile.close();
		return false;
	}

	// safety
	if (!checknotabsoluterootfolder(toString()))
		return false;

	return !std::rename(this->to_path_string().c_str(),
						newosdir_or_filename.to_path_string().c_str());
}

bool var::oscopy(const var& to_osfile_or_dirname) const {
	THISIS("bool var::oscopy(const var& to_osfile_or_dirname) const")
	THISISSTRING()
	ISSTRING(to_osfile_or_dirname)

	stdfs::path frompathx(this->to_path_string().c_str());
	stdfs::path topathx(to_osfile_or_dirname.to_path_string().c_str());
	/*
	try
	{
		// will not overwrite so this is redundant
		// option to overwrite is not in initial versions of boost copy_file
		if (stdfs::exists(topathx))
			return false;

		stdfs::copy_file(frompathx, topathx);
	}
	catch (...)
	{
		return false;
	}
	*/
	//https://en.cppreference.com/w/cpp/filesystem/copy
	std::error_code error_code;
	copy(frompathx, topathx,
		 stdfs::copy_options::overwrite_existing | stdfs::copy_options::recursive | stdfs::copy_options::copy_symlinks,
		 error_code);

	return !error_code;
}

bool var::osdelete() const {
	return osdelete(*this);
}

// not boost ... only removes files?
bool var::osdelete(const var& osfilename) const {
	THISIS("bool var::osdelete(const var& osfilename) const")
	THISISDEFINED()
	ISSTRING(osfilename)
	osfilename.osclose();  // in case this is cached opened file handle
	return !std::remove(osfilename.to_path_string().c_str());
}

const std::string var::to_path_string() const {
	var part2 = this->field(" ", 2, 999999);
	if (part2.length()) {

#if defined WIN32 || defined _WIN32
		var part = this->field(" ", 1).convert("/", OSSLASH);
#else
		// printf("path=%s\n",this->convert("\\",OSSLASH).toString().c_str());
		var part = this->field(" ", 1).convert("\\", OSSLASH);

		// very similar code below
		// standardise on ALL AND ONLY lower case os file and path names
		// in order to allow uppercase, will have to find and remove all uppercase in the
		// old source code
		/*
		var lcpart = part.lcase();
		if (lcpart != part && part.substr(-3,3) != ".LK" && part.substr(-3,3) != ".OV" && part.substr(-9) != "/DATA.CFG" && !part.index("./data/"))
		{
			part.errputl("WARNING - UPPERCASE OS=");
//			part = lcpart;
		}
		*/
#endif
		return part.toString() + " " + part2.toString();
	} else {

#if defined WIN32 || defined _WIN32
		return this->convert("/", OSSLASH).toString();
#else
		// very similar code above
		// standardise on ALL AND ONLY lower case os file and path names
		// in order to allow uppercase, will have to find and remove all uppercase in the
		// old source code
		/*
		var lcthis = this->lcase();
		if (lcthis != (*this) && this->substr(-3,3) != ".LK" && this->substr(-3,3) != ".OV" && this->substr(-9) != "/DATA.CFG")
		{
			(*this).errputl("WARNING - UPPERCASE OS=");
//			return lcthis.convert("\\", OSSLASH).toString();
		}
		*/
		return this->convert("\\", OSSLASH).toString();
#endif
	}
}

const std::string var::to_cmd_string() const {

	// while converting from DOS convert all backslashes in first word to forward slashes on
	// linux or leave as if exodus on windows

	// warning if any backslashes unless at least one of them is followed by $ which indicates
	// valid usage as an escape character aiming to recode all old windows-only code
	if (this->index("\\") && !this->index("\\$"))
		this->errputl("WARNING BACKSLASHES IN OS COMMAND:");

	return this->field(" ", 1).to_path_string() + " " + this->field(" ", 2, 999999).toString();
}

//convert some clock to time_t (for osfile() and osdir()
template <typename TP>
std::time_t to_time_t(TP tp) {
	using namespace std::chrono;
	auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
	return system_clock::to_time_t(sctp);
}

// in MVdatetime.cpp
//void ptime2mvdatetime(const boost::posix_time::ptime& ptimex, int& mvdate, int& mvtime);
void ptime2mvdatetime(const boost::posix_time::ptime& ptimex, int& mvdate, int& mvtime);
//WARNING ACTUALLY DEFINED WITH BOOST POSIX TIME BUT IT IS THE SAME
//void ptime2mvdatetime(const stdfs::file_time_type& ptimex, int& mvdate, int& mvtime);

var var::osfile() const {
	THISIS("var var::osfile() const")
	THISISSTRING()

	// get a handle and return "" if doesnt exist or isnt a regular file
	try {
		// boost 1.33 throws an error with files containing ~ or $ chars but 1.38 doesnt
		stdfs::path pathx(this->to_path_string().c_str());

		if (!stdfs::exists(pathx))
			return "";
		// is_regular is only in boost > 1.34
		// if (! stdfs::is_regular(pathx)) return "";
		if (stdfs::is_directory(pathx))
			return "";

		//not using low level interface
		//https://stackoverflow.com/questions/21159047/get-the-creation-date-of-file-or-folder-in-c#21159305

		// SIMILAR CODE IN OSFILE AND OSDIR

		// get last write datetime
		//std::time_t last_write_time = std::chrono::system_clock::to_time_t(stdfs::last_write_time(pathx));
		stdfs::file_time_type file_time = stdfs::last_write_time(pathx);
		std::time_t last_write_time = to_time_t(file_time);

		// convert to posix time
		boost::posix_time::ptime ptimex = boost::posix_time::from_time_t(last_write_time);
		//stdfs::file_time_type ptimex = stdfs::last_write_time(pathx);

		// convert posix time to mv date and time
		//TODO Currently returns local date/time. Should it not be UTC?
		int mvdate, mvtime;
		ptime2mvdatetime(ptimex, mvdate, mvtime);

		//file_size() is only available for files not directories
		return int(stdfs::file_size(pathx)) ^ FM ^ mvdate ^ FM ^ int(mvtime);
	} catch (...) {
		return "";
	};
}

bool var::osmkdir() const {
	THISIS("bool var::osmkdir() const")
	THISISSTRING()

	try {

		// boost 1.33 throws an error with files containing ~ or $ chars but 1.38 doesnt
		stdfs::path pathx(this->to_path_string().c_str());

		if (stdfs::exists(pathx))
			return false;
		stdfs::create_directories(pathx);
	} catch (...) {
		return false;
	}
	return true;
}

bool var::osrmdir(bool evenifnotempty) const {
	THISIS("bool var::osrmdir(bool evenifnotempty) const")
	THISISSTRING()

	// get a handle and return "" if doesnt exist or is NOT a directory
	try {

		// boost 1.33 throws an error with files containing ~ or $ chars but 1.38 doesnt
		stdfs::path pathx(this->to_path_string().c_str());

		if (!stdfs::exists(pathx))
			return false;
		if (!stdfs::is_directory(pathx))
			return false;

		if (evenifnotempty) {

			// safety
			if (!checknotabsoluterootfolder(toString()))
				return false;

			stdfs::remove_all(pathx);
		} else
			stdfs::remove(pathx);
	} catch (...) {
		return false;
	}

	return true;
}

var var::osdir() const {
	THISIS("var var::osdir() const")
	THISISSTRING()

	// get a handle and return "" if doesnt exist or is NOT a directory
	// stdfs::wpath pathx(toTstring((*this)).c_str());
	try {

		// boost 1.33 throws an error with files containing ~ or $ chars but 1.38 doesnt
		stdfs::path pathx(this->to_path_string().c_str());

		if (!stdfs::exists(pathx))
			return "";
		if (!stdfs::is_directory(pathx))
			return "";

		//not using low level interface
		//https://stackoverflow.com/questions/21159047/get-the-creation-date-of-file-or-folder-in-c#21159305

		// SIMILAR CODE IN OSFILE AND OSDIR

		// get last write datetime
		//std::time_t last_write_time = std::chrono::system_clock::to_time_t(stdfs::last_write_time(pathx));
		stdfs::file_time_type file_time = stdfs::last_write_time(pathx);
		std::time_t last_write_time = to_time_t(file_time);

		// convert to posix time
		boost::posix_time::ptime ptimex = boost::posix_time::from_time_t(last_write_time);
		//stdfs::file_time_type ptimex = stdfs::last_write_time(pathx);

		// convert posix time to mv date and time
		int mvdate, mvtime;
		ptime2mvdatetime(ptimex, mvdate, mvtime);

		//file_size() is only available for files not directories
		//return int(stdfs::file_size(pathx)) ^ FM ^ mvdate ^ FM ^ int(mvtime);
		return FM ^ mvdate ^ FM ^ int(mvtime);

	} catch (...) {
		return "";
	};
}

var var::oslistf(const var& path, const var& spec) const {
	return this->oslist(path, spec, 1);
}

var var::oslistd(const var& path, const var& spec) const {
	return this->oslist(path, spec, 2);
}

/*
*@param	path	A directory or blank for current working directory. If spec if empty then the last part (after any slashes) is used as spec.
*@param	spec	Optional glob like "*",  "*.*", "*.???" etc. (CASE INSENSITIVE)
*@param	mode	1=files only, 2=directories only, otherwise both.
*@returns		List of directory and/or filenames depending on mode. fm separator
*/
var var::oslist(const var& path0, const var& spec0, const int mode) const {
	THISIS("var var::oslist(const var& path, const var& spec, const int mode) const")
	THISISDEFINED()
	ISSTRING(path0)
	ISSTRING(spec0)

	// returns an fm separated list of files and/or folders

	// http://www.boost.org/libs/filesystem/example/simple_ls.cpp

	var path = path0.to_path_string();
	var spec;
	if (spec0.length()) {
		spec = spec0;
	}
	// file globbing can and must be passed as tail end of path
	// perhaps could use <glob.h> in linux instead of regex
	else {
		spec = path.field2(OSSLASH, -1);
		path = path.substr(1, path.length() - spec.length());

		// escape all the regex special characters that are found in the strint
		// except the * ? which are glob special characters
		// regex concept here is ([specialchars]) replace with \$1 where $1 will be any of
		// the special chars note inside brackets, only  ^ - and ] are special characters
		// inside [] char alternative to include a ] in the list of characters inside [] it
		// must be the first character like []otherchars] of course all back slashes must be
		// doubled up in c++ source code
		spec.replacer("([][\\\\(){}|^$.+])", "\\\\$1");

		// glob * becomes .* in regex matching any number of any characters
		spec.swapper("*", ".*");

		// glob ? becomes . in regex matching any one character
		spec.swapper("?", ".");
	}

	bool filter = false;
	// std_boost::regex re;
	std_boost::regex re;
	if (spec) {
		try {
			// Set up the regular expression for case-insensitivity
			// re.assign(toTstring(spec).c_str(), std_boost::regex_constants::icase);
			re.assign(spec.toString().c_str(), std_boost::regex_constants::icase);
			filter = true;
		} catch (std_boost::regex_error& e) {
			std::cout << spec.var_str << " is not a valid regular expression: \""
					  << e.what() << "\"" << std::endl;
			return "";
		}
	}

	bool getfiles = true;
	bool getfolders = true;
	if (mode == 1)
		getfolders = false;
	else if (mode == 2)
		getfiles = false;

	var filelist = "";
	//#if BOOST_FILESYSTEM_VERSION >= 3 or defined(C17)
	//#define LEAForFILENAME path().filename().string()
	//#define COMMAstdfsNATIVE
	//#else
	//#define LEAForFILENAME leaf()
	//#define COMMAstdfsNATIVE ,stdfs::native
	//#endif
	// get handle to folder
	// wpath or path before boost 1.34
	// stdfs::wpath full_path(stdfs::initial_path<stdfs::wpath>());
	// full_path = stdfs::system_complete(stdfs::wpath(toTstring(path), stdfs::native ));
	// stdfs::path full_path(stdfs::initial_path());
	// initial_path always return the cwd at the time it is first called which is almost useless
	stdfs::path full_path(stdfs::current_path());
	//full_path =
	//    stdfs::system_complete(stdfs::path(path.to_path_string().c_str() COMMAstdfsNATIVE));
	//full_path =
	//    stdfs::absolute(stdfs::path(path.to_path_string().c_str()));
	if (path.length()) {
		//full_path = stdfs::absolute(stdfs::path(path.to_path_string().c_str()));
		std::error_code error_code;
		stdfs::path pathx = stdfs::path(path.to_path_string().c_str());
		full_path = stdfs::ABSOLUTE_OR_COMPLETE(pathx, error_code);
		if (error_code) {
			std::cerr << "'" << path.to_path_string() << "' path : " << error_code.message() << std::endl;
			return filelist;
		}
		//stdfs::path path1 = stdfs::path(".");
		//std::clog << "path1='" << path1 << "'" <<std::endl;

		//stdfs::path path_cwd = stdfs::current_path();
		//std::clog << "path_cwd='" << path_cwd << "'" <<std::endl;

		//stdfs::path pathx = stdfs::path(path.to_path_string().c_str());
		//stdfs::path pathx = stdfs::current_path();
		//stdfs::path pathx = stdfs::path(".");
		//std::clog << "pathx='" << pathx << "'" <<std::endl;
		//full_path =
		//    stdfs::absolute(pathx);
		//std::clog << "pathx='" << pathx << "'" <<std::endl;
	}
	//std::clog << "fullpath='" << full_path << "'" <<std::endl;

	// quit if it isnt a folder
	if (!stdfs::is_directory(full_path))
		return filelist;

	// errno=0;
	// stdfs::wdirectory_iterator end_iter;
	// for (stdfs::wdirectory_iterator dir_itr(full_path );
	stdfs::directory_iterator end_iter;
	for (stdfs::directory_iterator dir_itr(full_path); dir_itr != end_iter; ++dir_itr) {
		try {

			// skip unwanted items
			// dir_itr->path().leaf()  changed to dir_itr->leaf() in three places
			// also is_directory(dir_itr->status()) changed to is_directory(*dir_itr)
			// to avoid compile errors on boost 1.33
			// http://www.boost.org/doc/libs/1_33_1/libs/filesystem/doc/index.htm
			if (filter && !std_boost::regex_match(dir_itr->path().filename().string(), re))
				continue;

			// using .leaf instead of .status provided in boost 1.34 .. but does it
			// work/efficiently if (stdfs::is_directory(dir_itr->status() ) )
			if (stdfs::is_directory(*dir_itr)) {
				if (getfolders)
					filelist ^= FM ^ dir_itr->path().filename().string();
			}
			// is_regular is only in boost > 1.34
			// else if (stdfs::is_regular(dir_itr->status() ) )
			else  // if (stdfs::is_regular(dir_itr->status() ) )
			{
				if (getfiles)
					filelist ^= FM ^ dir_itr->path().filename().string();
			}
			// else
			//{
			//  //++other_count;
			//  //std::cout << dir_itr->path().leaf() << " [other]\n";
			//}
		} catch (const std::exception& ex) {
			// evade warning: unused variable
			if (false)
				if (ex.what()) {
				};

			//++err_count;
			// std::cout << dir_itr->path().leaf() << " " << ex.what() << std::endl;
		}
	}

	// delete first separator
	// NB splice is 1 based
	if (filelist != "")
		filelist.splicer(1, 1, "");

	return filelist;
}

bool var::oscwd(const var& newpath) const {
	THISIS("var var::oscwd(const var& newpath) const")
	// doesnt use *this - should syntax be changed to setcwd? and getcwd()?
	THISISDEFINED()	 // not needed if *this not used
	ISSTRING(newpath)

	// http://www.boost.org/doc/libs/1_38_0/libs/filesystem/doc/reference.html#Attribute-functions
	// wont compile on boost 1.33 so comment it out and make non-functional
	// until we have a reliable way to detect boost version
	// boost::filesystem::current_path(newpath.toString());

	// return oscwd();

	try {
		stdfs::current_path(newpath.toString());
	} catch (...) {
		// filesystem error: cannot set current path: No such file or directory
		// ignore all errors
		return false;
	}

	return true;
}

var var::oscwd() const {
	THISIS("var var::oscwd() const")
	// doesnt use *this - should syntax be changed to ossetcwd? and osgetcwd()?
	THISISDEFINED()	 // not needed if *this not used

	// TODO consider using complete() or system_complete()
	//"[Note: When portable behavior is required, use complete(). When operating system
	// dependent behavior is required, use system_complete()."

	// http://www.boost.org/doc/libs/1_38_0/libs/filesystem/doc/reference.html#Attribute-functions
	// std::string currentpath=boost::filesystem::current_path().string();
	std::string currentpath = stdfs::current_path().string();

	return var(currentpath).convert("/", OSSLASH);
}

var var::ospid() const {
	//THISIS("var var::ospid() const")
	return getpid();
}

void var::ossleep(const int milliseconds) const {
	THISIS("void var::ossleep(const int milliseconds) const")
	// doesnt use *this - should syntax be changed to setcwd? and getcwd()?
	THISISDEFINED()	 // not needed if *this not used

	boost::xtime xt;
	boost::xtime_get(&xt, boost::TIME_UTC_);
	xt.nsec += milliseconds * 1000000;
	boost::thread::sleep(xt);

	//	usleep((int)(1000.0*milliseconds));
	//	Sleep(1000*milliseconds);
}

}  // namespace exodus
