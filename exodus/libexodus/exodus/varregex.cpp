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
////////////////////////////////////////////////////////////////////////////////////////////

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

/* UNICODE regex
https://www.boost.org/doc/libs/1_78_0/libs/regex/doc/html/boost_regex/ref/non_std_strings/icu/unicode_iter.html

REGEX SYNTAX normal = perl/javascript
https://www.boost.org/doc/libs/1_80_0/libs/regex/doc/html/boost_regex/ref/syntax_option_type/syntax_option_type_synopsis.html

u32regex_iterator

Type u32regex_iterator is in all respects the same as regex_iterator
except that since the regular expression type is always u32regex it only
takes one template parameter (the iterator type). It also calls u32regex_search internally,
allowing it to interface correctly with UTF-8, UTF-16, and UTF-32 data:
*/

#include <iostream>
#include <utility> //for replace
#include <string>
#include <boost/algorithm/string/replace.hpp>

#define USE_BOOST
#ifdef USE_BOOST
#	define std_boost boost
#	include <boost/regex/icu.hpp>
#	include <boost/regex.hpp>

#else // USE_BOOST not defined
#	include <regex>
#	define std_boost std
#endif

#include <exodus/varimpl.h>
#include <exodus/varoshandle.h>

namespace exodus {

// From here on we need to use Boost's u32_regex, u32_match, u32_replace
// in order to correctly process no-ASCII/multibyte characters

#ifdef USE_BOOST
#	define REGEX boost::u32regex
#	define REGEX_REPLACE boost::u32regex_replace

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
#	define REGEX std::regex
#	define REGEX_REPLACE std::regex_replace
	using syntax_flags_typ = std::regex::flag_type;

	static const std::regex digits_regex("\\d+");  //\d numeric

	static const std::regex alpha_regex("[^\\W\\d]+");	 // \a alphabetic

	static const std::regex alphanum_regex("\\w+");  // \w alphanumeric

	static const std::regex non_digits_regex("[^\\d]+");  // \D non-numeric

	static const std::regex non_alpha_regex("[\\W\\d]+");	// \A non-alphabetic

	static const std::regex non_alphanum_regex("\\W+");  // \W non-alphanumeric

#endif

static syntax_flags_typ get_regex_syntax_flags(SV options) {
	// determine options from string

	// Default flavour is ECMAScript/Perl
	// https://www.boost.org/doc/libs/1_80_0/libs/regex/doc/html/boost_regex/ref/syntax_option_type/syntax_option_type_perl.html
	// https://en.cppreference.com/w/cpp/regex/syntax_option_type

	// default
	// collate	Character ranges of the form "[a-b]" will be locale sensitive.
	// if the normal/ECMAScript/perl engine is selected.
	syntax_flags_typ regex_syntax_flags = std_boost::regex_constants::collate;

	// i = icase
	//if (options.contains("i"))
	if (options.find('i') != std::string::npos)
		regex_syntax_flags |= std_boost::regex_constants::icase;

	// m = multiline (The default in boost but not std::regex
	// Specifies that ^ shall match the beginning of a line and $ shall match the end of a line,
	// if the normal/perl/ECMAScript engine is selected.
	//if (options.find('m') != std::string::npos)
	//	regex_syntax_flags|=std_boost::regex_constants::multiline;

	// s = single/no multiline (The default in std::regex but not boost
	// Specifies that ^ shall match the beginning of a line and $ shall match the end of a line,
	// if the normal/perl/ECMAScript engine is selected.
	if (options.find('s') != std::string::npos)
		regex_syntax_flags|=std_boost::regex_constants::no_mod_m;

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

static thread_local std::map<std::string, REGEX> thread_regexes;

// Caches regex giving very approx 10x speed up
static REGEX& getregex(SV matchstr, SV options) {

	auto cache_key = std::string(matchstr);
	cache_key.push_back(FM_);
	cache_key += options;

	auto mapiter = thread_regexes.find(cache_key);
	if (mapiter != thread_regexes.end())
		return mapiter->second;

	try {
		[[maybe_unused]] auto [mapiter, success] = thread_regexes.emplace(
			cache_key,
			boost::make_u32regex(
				std::string(matchstr),
				// Analyse the options requested
				get_regex_syntax_flags(options)
			)
		);
		return mapiter->second;
	}
	catch (boost::wrapexcept<std::out_of_range>& e) {
		throw VarError("Error: (1) Invalid data during match of " ^ var(matchstr).quote() ^ ". " ^ var(e.what()));
	}
	catch (std_boost::regex_error& e) {
		throw VarError("Error: Invalid regex string " ^ var(matchstr).quote() ^ ". " ^ var(e.what()).quote());
	}

	// Cannot get here
	//std::unreachable();
	//throw VarError("getreg");
}

// should be in mvfuncs.cpp - here really because boost regex is included here for file matching
var var::match(SV matchstr, SV options) const {

	// VISUALISE REGULAR EXPRESSIONS GRAPHICALLY!
	// https:www.debuggex.com

	THISIS("bool var::match(SV matchstr, SV options) const")
	assertString(function_sig);
	//ISSTRING(matchstr)

	// wild cards like
	// *.* or *.???
	// *abcde
	// abcde*
	//if (options.contains("w")) { //c++20
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

		//std::string matchstr2 = std_boost::regex_replace(matchstr.var_str, regex_special_chars,
		std::string matchstr2 = std_boost::regex_replace(std::string(matchstr), regex_special_chars,
											  replacement_for_regex_special);

		// 1. force to match whole string
		var matchstr3 = "^" + matchstr2 + "$";

		// 2. * matches zero or more characters like .* in regex
		matchstr3.replacer("*", ".*");

		// 3. ? matches any one character
		matchstr3.replacer("?", ".");
		//TRACE(matchstr3)

		// having created a suitable regex str, recursively call match with it
		return this->match(matchstr3);
	}

	REGEX& regex = getregex(matchstr, options);

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

	// https://stackoverflow.com/questions/26320987/what-is-the-difference-between-regex-token-iterator-and-regex-iterator
	// boost::u32regex_token_iterator<std::string::const_iterator>
	//	iter {std_boost::make_u32regex_token_iterator(var_str,regex,{0,1,2,3,4,etc or -1})};
	// token_iterator allow you to access none-matching parts of the string for parsing stuff
	// but doesnt return an iterator with an array of groups

	// construct our iterators:
#ifdef USE_BOOST
	boost::u32regex_iterator<std::string::const_iterator> iter;
	try {
		iter = boost::make_u32regex_iterator(var_str, regex);
	} catch (boost::wrapexcept<std::out_of_range>& e) {
		throw VarError("Error: (2) Invalid match string " ^ var(matchstr).quote() ^ ". " ^ var(e.what()));
	}
#else
	std::regex_iterator<std::string::const_iterator> iter(var_str.begin(), var_str.end(), regex);
#endif

	var result;
	result.var_typ = VARTYP_STR;

	// Using declarative functional style "for_each with lambda" instead of "while (iter!=end)" loop
	decltype(iter) end{};
	std::for_each(iter, end,
		[&result](auto match_results) {

			// group 0 is complete match
			// any subgroups are groupn 1+
//			for (int groupn = 0; uint(groupn) <= match_results.size(); ++groupn) {
			for (int groupn = 0; uint(groupn) < match_results.size(); ++groupn) {
				// std::cout<< match_results[0] << std::endl;
				result.var_str.append(match_results[groupn]);
				//result.var_str.append(var(groupn).toString());

				// any submatch groups are separated by VM
				result.var_str.push_back(VM_);
			}

			// Remove trailing VM
			if (!result.var_str.empty()) {
//				while (result.var_str.back() == VM_)
				result.var_str.pop_back();
			}

			// muliple matches are separated by FM
			result.var_str.push_back(FM_);
		}
	);

	// Remove trailing FM
	if (!result.var_str.empty())
		result.var_str.pop_back();

	return result;

}

// Simple non-regex case sensitive substr replacement
/////////////////////////////////////////////////////

// constant
var var::replace(SV what, SV with) const& {

	//return this->clone().replacer(what, with);}
	return boost::algorithm::replace_all_copy(var_str, what, with);
}

// mutator
VARREF var::replacer(SV what, SV with) {

	THISIS("VARREF var::replacer(SV what, SV with)")
	assertStringMutator(function_sig);

	// nothing to do if oldstr is ""
	if (what.empty())
		return *this;

//	// Optimise for single character replacement
//	// No measurable speedup
//	//if (what.size() == 1 and with.size() == 1) {
//	//	std::replace(var_str.begin(), var_str.end(), what[0], with[0]);
//	//	return *this;
//	//}
//
//	// find the starting position of the field or return
//	std::string::size_type start_pos = 0;
//	while (true) {
//		start_pos = var_str.find(what, start_pos);
//		// past of of string?
//		if (start_pos == std::string::npos)
//			return *this;
//		var_str.replace(start_pos, what.size(), with);
//		start_pos += with.size();
//	}

	boost::algorithm::replace_all(var_str, what, with);

	return *this;

}

// Regex based string replacement
/////////////////////////////////

// only here really because boost regex is included here for file matching

// option  f - first occurrence only

// constant
var var::regex_replace(SV regexstr, SV replacementstr, SV options) const& {
	return this->clone().regex_replacer(regexstr, replacementstr, options);
}

// mutator
VARREF var::regex_replacer(SV regexstr, SV replacementstr, SV options) {

	THISIS("VARREF var::regex_replacer(SV regexstr, SV replacementstr, SV options)")
	assertStringMutator(function_sig);
	//ISSTRING(regexstr)
	//ISSTRING(replacementstr)

	// http://www.boost.org/doc/libs/1_38_0/libs/regex/doc/html/boost_regex/syntax/basic_syntax.html

	// Build the regex object with the given options
	REGEX& regex = getregex(regexstr, options);

	// return regex_match(var_str, expression);

	// One liner to replace the first occurrence only
	//var_str = REGEX_REPLACE(var_str, regex1, std::string(replacementstr));

	// Get an ostringstream object and an iterator on it
	std::ostringstream oss1(std::ios::out | std::ios::binary);
	std::ostream_iterator<char, char> oiter(oss1);

	auto match_flags = std_boost::match_flag_type(boost::match_default);
	// Option f = first only, otherwise all
	//c++23 if (options.contains("f"))
	if (options.find("f") != std::string::npos)
		match_flags |= boost::format_first_only;
	else
		match_flags |= boost::format_all;

	// Generate output
	try {
		std_boost::u32regex_replace(
			oiter,
			var_str.begin(),
			var_str.end(),
			regex,
			//replacementstr,
			std::string(replacementstr),
			//boost::match_default | boost::format_all
			match_flags
		);
	} catch (boost::wrapexcept<std::out_of_range>& e) {
		var errmsg = "Error: (3) Invalid data or replacement during regex replace of " ^ var(regexstr).quote() ^ " with " ^ var(replacementstr).quote() ^ ". " ^ var(e.what());
		//TODO Show some of the invalid UTF8 bytes maybe.
		//Cannot show actual data because it could be a security breach.
		//errmsg ^= ". Data[1,128] = " ^ this->first(128).quote();
		throw VarError(errmsg);
	}

	// Acquire the output
	var_str = std::move(oss1).str();

	return *this;
}

// ICONV_MT can be moved back to mvioconv.cpp if it stops using regular expressions
///////////

// regular expressions for ICONV_MT
// var var::iconv_MT(const char* conversion) const
var var::iconv_MT() const {
	//THISIS("var var::iconv_MT() const")
	//assertString(function_sig);
	// ignore everything else and just get first three groups of digits "99 99 99"
	// remove leading and trailing non-digits and replace internal strings of non-digits with
	// single space

	// var time=(*this).replace("^\\D+|\\D+$", "", "r").replace("\\D+", " ", "r");

	static const std_boost::regex surrounding_nondigits_regex("^\\D+|\\D+$",
														std_boost::regex_constants::icase);

	static const std_boost::regex inner_nondigits_regex("\\D+", std_boost::regex_constants::icase);

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

// OCONV_MR can be moved back to mvioconv.cpp if it stops using regular expressions
///////////

// regular expressions for ICONV_MC
VARREF var::oconv_MR(const char* conversion) {
	//THISIS("VARREF var::oconv_MR(const char* conversionchar)")
	//assertString(function_sig);
	// conversionchar arrives pointing to 3rd character (eg A in MCA)

#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
    const char* conversionchar = conversion;

    // Skip over leading "MR"
    conversionchar += 2;
#pragma GCC diagnostic pop

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

#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
		++conversionchar;
#pragma GCC diagnostic pop

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
			// No conversion if not N, A or B
			default:;
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
		// No conversion if not N, A, B, L or U
		default:;
	}

	return *this;
}

} // namespace exodus
