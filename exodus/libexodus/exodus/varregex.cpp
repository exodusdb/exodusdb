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

/* UNICODE regex
https://www.boost.org/doc/libs/1_78_0/libs/regex/doc/html/boost_regex/ref/non_std_strings/icu/unicode_iter.html

u32regex_iterator

Type u32regex_iterator is in all respects the same as regex_iterator
except that since the regular expression type is always u32regex it only
takes one template parameter (the iterator type). It also calls u32regex_search internally,
allowing it to interface correctly with UTF-8, UTF-16, and UTF-32 data:
*/

#include <iostream>
#include <utility> //for swap
#include <string>

#define USE_BOOST
#ifdef USE_BOOST
#define std_boost boost
#include <boost/regex/config.hpp>
#include <boost/regex/icu.hpp>
#include <boost/regex.hpp>

#else // USE_BOOST not defined
#include <regex>
#define std_boost std
#endif

#include <exodus/varimpl.h>
#include <exodus/varoshandle.h>

namespace exodus {

// ICONV_MT can be moved back to mvioconv.cpp if it stops using regular expressions
// regular expressions for ICONV_MT
// var var::iconv_MT(const char* conversion) const
var var::iconv_MT() const {
	//THISIS("var var::iconv_MT() const")
	//assertString(function_sig);
	// ignore everything else and just get first three groups of digits "99 99 99"
	// remove leading and trailing non-digits and replace internal strings of non-digits with
	// single space

	// var time=(*this).swap("^\\D+|\\D+$", "", "r").swap("\\D+", " ", "r");

	static std_boost::regex surrounding_nondigits_regex("^\\D+|\\D+$",
														std_boost::regex_constants::icase);

	static std_boost::regex inner_nondigits_regex("\\D+", std_boost::regex_constants::icase);

	var time = var(std_boost::regex_replace(var_str, surrounding_nondigits_regex, ""));
	time = var(std_boost::regex_replace(time.var_str, inner_nondigits_regex, " "));

	int hours = time.field(" ", 1).toInt();
	int mins = time.field(" ", 2).toInt();
	int secs = time.field(" ", 3).toInt();

	int inttime = hours * 3600 + mins * 60 + secs;

	if (inttime >= 86400)
		return "";

	// PM
	if (inttime < 43200 && (*this).contains("P"))
		inttime += 43200;
	else if (inttime >= 43200 && (*this).contains("A"))
		inttime -= 43200;

	return inttime;
}

// From here on we need to use Boost's u32_regex, u32_match, u32_replace
// in order to correctly process no-ASCII/multibyte characters

#ifdef USE_BOOST
#define REGEX boost::u32regex
#define REGEX_REPLACE boost::u32regex_replace

	using syntax_flags_typ = int;

    static const boost::u32regex digits_regex = boost::make_u32regex("\\d+");  //\d numeric
    static const boost::u32regex alpha_regex =
        boost::make_u32regex("[^\\W\\d]+");  // \a alphabetic
    static const boost::u32regex alphanum_regex =
        boost::make_u32regex("\\w+");  // \w alphanumeric
    static const boost::u32regex non_digits_regex =
        boost::make_u32regex("[^\\d]+");  // \D non-numeric
    static const boost::u32regex non_alpha_regex =
        boost::make_u32regex("[\\W\\d]+");  // \A non-alphabetic
    static const boost::u32regex non_alphanum_regex =
        boost::make_u32regex("\\W+");  // \W non-alphanumeric

#else // USE_BOOST not defined
#define REGEX std::regex
#define REGEX_REPLACE std::regex_replace
	using syntax_flags_typ = std::regex::flag_type;

	static const std::regex digits_regex("\\d+");  //\d numeric
	static const std::regex alpha_regex("[^\\W\\d]+");	 // \a alphabetic
	static const std::regex alphanum_regex("\\w+");  // \w alphanumeric
	static const std::regex non_digits_regex("[^\\d]+");  // \D non-numeric
	static const std::regex non_alpha_regex("[\\W\\d]+");	// \A non-alphabetic
	static const std::regex non_alphanum_regex("\\W+");  // \W non-alphanumeric

#endif

// OCONV_MR can be moved back to mvioconv.cpp if it stops using regular expressions
// regular expressions for ICONV_MC
VARREF var::oconv_MR(const char* conversionchar) {
	//THISIS("VARREF var::oconv_MR(const char* conversionchar)")
	//assertString(function_sig);
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
	l=lowercase u=uppercase t=titlecase o=other Nd=decimal digitboost_regexp
	\N{UNICODE CHARACTER NAME}	Match the named character.
	\p{UNICODE PROPERTY NAME}	Match any character with the specified Unicode Property.
	\P{UNICODE PROPERTY NAME}	Match any character not having the specified Unicode
	Property.
	*/

	// negate if /
	if (*conversionchar == '/') {
		++conversionchar;
		switch (*conversionchar) {
			// MC/N return everything except digits i.e. remove all digits 0123456789
			case 'N': {
				//var_str=std_boost::regex_replace(toTstring((*this)),digits_regex, "");
				var_str = REGEX_REPLACE(var_str, digits_regex, "");
				break;
			}

			// MC/A return everything except "alphabetic" i.e remove all "alphabetic"
			case 'A': {
				//var_str=std_boost::regex_replace(toTstring((*this)),alpha_regex, "");
				var_str = REGEX_REPLACE(var_str, alpha_regex, "");
				break;
			}
			// MC/B return everything except "alphanumeric" remove all "alphanumeric"
			case 'B': {
				//var_str=std_boost::regex_replace(toTstring((*this)),alphanum_regex, "");
				var_str = REGEX_REPLACE(var_str, alphanum_regex, "");
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
			// var_str=std_boost::regex_replace(toTstring((*this)),non_digits_regex, "");
			var_str = REGEX_REPLACE(var_str, non_digits_regex, "");
			break;
		}
		// MCA return only "alphabetic" i.e. remove all "non-alphabetic"
		case 'A': {
			// var_str=std_boost::regex_replace(toTstring((*this)),non_alpha_regex, "");
			var_str = REGEX_REPLACE(var_str, non_alpha_regex, "");
			break;
		}
		// MCB return only "alphanumeric" i.e. remove all "non-alphanumeric"
		case 'B': {
			// var_str=std_boost::regex_replace(toTstring((*this)),non_alphanum_regex, "");
			var_str = REGEX_REPLACE(var_str, non_alphanum_regex, "");
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

syntax_flags_typ get_regex_syntax_flags(SV options) {
	// determine options from string

	// default
	// ECMAScript	Use the Modified ECMAScript regular expression grammar
	// collate	Character ranges of the form "[a-b]" will be locale sensitive.
	syntax_flags_typ regex_syntax_flags = std_boost::regex_constants::collate;

	// i = icase
	//if (options.contains("i"))
	if (options.find('i') != std::string::npos)
		regex_syntax_flags |= std_boost::regex_constants::icase;

	// m = multiline (TODO should be present c++17 onwards)
	// Specifies that ^ shall match the beginning of a line and $ shall match the end of a line,
	// if the ECMAScript engine is selected. if (options.contains("m"))
	//	regex_syntax_flags|=std_boost::regex_constants::multiline;

	// b = basic (withdrawn after c++17?)
	// Use the basic POSIX regular expression grammar
	// if (options.contains("b"))
	//	regex_syntax_flags|=std_boost::regex_constants::basic;

	// e = extended
	// Use the extended POSIX regular expression grammar
	//if (options.contains("e"))
	if (options.find('e') != std::string::npos)
		regex_syntax_flags |= std_boost::regex_constants::extended;

	// l - literal TODO manually implement
	// ignore all usual regex special characters
	// BOOST only option
#ifdef USE_BOOST
	//if (options.contains("l"))
	if (options.find('l') != std::string::npos)
		regex_syntax_flags |= std_boost::regex_constants::literal;
#endif

	return regex_syntax_flags;
}

// should be in mvfuncs.cpp - here really because boost regex is included here for file matching
var var::match(CVR matchstr, SV options) const {

	// VISUALISE REGULAR EXPRESSIONS GRAPHICALLY!
	// https:www.debuggex.com

	THISIS("bool var::match(CVR matchstr, SV options) const")
	assertString(function_sig);
	ISSTRING(matchstr)

	// wild cards like
	// *.* or *.???
	// *abcde
	// abcde*
	//if (options.contains("w")) {
	if (options.find("w") != std::string::npos) {

		// rules of glob - converting glob to regex

		// 0. the following regexe special characters are not special so, if present in
		// match expression, they must be escaped
		// https://stackoverflow.com/questions/1252992/how-to-escape-a-string-for-use-in-boost-regex
		// const std_boost::regex esc("[.^$|()\\[\\]{}*+?\\\\]");
		// allow * and ? in glob expressions
		// Special chars ". ^ $ | ( ) [ ] { } + \"
		const std_boost::regex regex_special_chars{R"raw([.^$|()\[\]{}+\\])raw"};

#ifdef USE_BOOST
		const std::string replacement_for_regex_special = R"raw(\\$&)raw";
#else
		const std::string replacement_for_regex_special = R"raw(\$&)raw";
#endif

		std::string matchstr2 = std_boost::regex_replace(matchstr.var_str, regex_special_chars,
											  replacement_for_regex_special);

		// 1. force to match whole string
		var matchstr3 = "^" + matchstr2 + "$";

		// 2. * matches zero or more characters like .* in regex
		matchstr3.swapper("*", ".*");

		// 3. ? matches any one character
		matchstr3.swapper("?", ".");
		//TRACE(matchstr3)

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

	REGEX regex;
	try {
#ifdef USE_BOOST
        regex = boost::make_u32regex(matchstr.var_str, get_regex_syntax_flags(options));
#else
		regex=std::regex(matchstr.var_str, get_regex_syntax_flags(options));
#endif
	} catch (std_boost::regex_error& e) {
		throw VarError("Error: Invalid regex " ^ matchstr.quote() ^ " " ^ var(e.what()).quote());
	}

	/*
		//create iterators to matches
		//https://www.boost.org/doc/libs/1_70_0/libs/regex/doc/html/boost_regex/ref/non_std_strings/icu/unicode_iter.html
		//auto iter {std_boost::make_u32regex_token_iterator(var_str,regex)};
		const int subs[] = {1, 2, 3, 0};
		boost::u32regex_token_iterator<std::string::const_iterator> iter
	   {std_boost::make_u32regex_token_iterator(var_str,regex,subs)}; decltype(iter) end {};

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
	//	iter {std_boost::make_u32regex_token_iterator(var_str,regex,{0,1,2,3,4,etc or -1})};
	// token_iterator allow you to access none-matching parts of the string for parsing stuff
	// but doesnt return an iterator with an array of groups

	// construct our iterators:
#ifdef USE_BOOST
	boost::u32regex_iterator<std::string::const_iterator> iter{boost::make_u32regex_iterator(var_str, regex)};
#else
	std::regex_iterator<std::string::const_iterator> iter(var_str.begin(), var_str.end(), regex);
#endif
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

}

// simple case sensitive substr replacement
var var::swap(SV what, SV with) const& {
	var newmv = *this;
	return newmv.swapper(what, with);
}

// on temporaries
VARREF var::swap(SV what, SV with) && {
	return this->swapper(what, with);
}

// in-place
VARREF var::swapper(SV what, SV with) {

	THISIS("VARREF var::swapper(SV what, SV with)")
	assertStringMutator(function_sig);
	//ISSTRING(what)
	//ISSTRING(with)

	// nothing to do if oldstr is ""
	if (what.empty())
		return *this;

	// Optimise for single character replacement
	// No measurable speedup
	//if (what.size() == 1 and with.size() == 1) {
	//	std::replace(var_str.begin(), var_str.end(), what[0], with[0]);
	//	return *this;
	//}

	// find the starting position of the field or return
	std::string::size_type start_pos = 0;
	while (true) {
		start_pos = var_str.find(what, start_pos);
		// past of of string?
		if (start_pos == std::string::npos)
			return *this;
		var_str.replace(start_pos, what.size(), with);
		start_pos += with.size();
	}

	return *this;
}

//regex based string replacement
// only here really because boost regex is included here for file matching
var var::regex_replace(CVR regexstr, CVR replacementstr, SV options) const& {
	var newmv = *this;
	return newmv.regex_replacer(regexstr, replacementstr, options);
}

// on temporary
VARREF var::regex_replace(CVR regexstr, CVR replacementstr, SV options) && {
	return this->regex_replacer(regexstr, replacementstr, options);
}

// in-place
VARREF var::regex_replacer(CVR regexstr, CVR replacementstr, SV options) {

	THISIS(
		"VARREF var::regex_replacer(CVR regexstr, CVR replacementstr, SV options)")
	assertStringMutator(function_sig);
	ISSTRING(regexstr)
	ISSTRING(replacementstr)

	// http://www.boost.org/doc/libs/1_38_0/libs/regex/doc/html/boost_regex/syntax/basic_syntax.html

	// TODO automatic caching of regular expressions or new exodus datatype to handle them
	REGEX regex1;
	try {
		//regex = std_boost::regex(regexstr.var_str, get_regex_syntax_flags(options));
#ifdef USE_BOOST
        regex1 = boost::make_u32regex(regexstr.var_str, get_regex_syntax_flags(options));
#else
		regex1=std::regex(regexstr.var_str, get_regex_syntax_flags(options));
#endif
	} catch (std_boost::regex_error& e) {
		throw VarError("Error: Invalid regex " ^ regexstr.quote() ^ " " ^ var(e.what()));
	}

	// return regex_match(var_str, expression);

	// std::ostringstream outputstring(std::ios::out | std::ios::binary);
	// std::ostream_iterator<char, char> oiter(outputstring);
	// std_boost::regex_replace(oiter, var_str.begin(), var_str.end(),regex_regex, with,
	// boost::match_default | boost::format_all);
	var_str = REGEX_REPLACE(var_str, regex1, replacementstr.var_str);
	return *this;
}

} // namespace exodus
