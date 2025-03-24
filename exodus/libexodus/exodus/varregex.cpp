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

// VISUALISE REGULAR EXPRESSIONS GRAPHICALLY
// https:www.debuggex.com

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

#if EXO_MODULE
	import std;
#else
#	include <cstring>	// for str::strlen etc.
#	include <iostream>
#	include <utility> //for replace
#	include <string>
#endif

#include <boost/algorithm/string/replace.hpp>

#define EXO_BOOST_REGEX
#ifdef EXO_BOOST_REGEX
#	define std_boost boost
#	include <boost/regex/icu.hpp>
#	include <boost/regex.hpp>

#else // EXO_BOOST_REGEX not defined
#	if EXO_MODULE
#	else
#		include <regex>
#	endif
#	define std_boost std
#endif

#include <exodus/varimpl.h>
#include <exodus/varoshandle.h>

#include <exodus/rex.h>

namespace exo {

	using let = const var;

// From here on we need to use Boost's u32_regex, u32_match, u32_replace
// in order to correctly process no-ASCII/multibyte characters

#ifdef EXO_BOOST_REGEX
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

#else // EXO_BOOST_REGEX not defined
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

constexpr static syntax_flags_typ get_syntax_flags(SV regex_options) {

	// Default flavour is ECMAScript/Perl
	// https://www.boost.org/doc/libs/1_80_0/libs/regex/doc/html/boost_regex/ref/syntax_option_type/syntax_option_type_perl.html
	// https://en.cppreference.com/w/cpp/regex/syntax_option_type

	// Determine regex_options from string

	// Using only c++11 std::regex standardized syntax options
	// Except for w - wild cards/globs and l - literal

	// f - first only. Only for replace (not match or search)

	// w - wildcard glob style (e.g. *.cfg) not regex. Only for match and search. Not replace.

	// l - literal (any regex chars are treated as normal chars)

	// i - case insensitive

	// p - ECMAScript/Perl (the default)
	// b - Basic POSIX (same as sed)
	// e - Extended POSIX
	// a - awk
	// g - grep
	// eg - egrep or grep -E

	// char ranges like a-z will be locale sensitive if ECMAScript

	// m - Multiline. Default in boost (and therefore exodus)
	// s - Single line. Default in std::regex

	// c - Use cache to store regex engine (for rex constructed objects)
	//     ""_rex constructed objects always use the cache of regex engines. One entry per regex_str and options combination.

	// default - collate - Character ranges of the form "[a-b]" will be locale sensitive.
	// IF? the normal/ECMAScript/perl engine is selected.
	syntax_flags_typ regex_syntax_flags = std_boost::regex_constants::collate;

	// i = icase - Character matching should be performed without regard to case.
	if (regex_options.find('i') != std::string::npos) {
		regex_syntax_flags |= std_boost::regex_constants::icase;
	}

	// m = multiline (The default in boost but not std::regex
	// Specifies that ^ shall match the beginning of a line and $ shall match the end of a line,
	// if the normal/perl/ECMAScript engine is selected.
#ifndef EXO_BOOST_REGEX
	if (regex_options.find('m') != std::string::npos) {
		regex_syntax_flags|=std_boost::regex_constants::multiline;
	}
#endif

	// s = single/no multiline (The default in std::regex but not boost)
	// Specifies that ^ shall match the beginning of a line and $ shall match the end of a line,
	// if the normal/perl/ECMAScript engine is selected.
	if (regex_options.find('s') != std::string::npos) {
		regex_syntax_flags|=std_boost::regex_constants::no_mod_m;
	}

	// l - literal TODO manually implement
	// ignore all usual regex special characters
	// BOOST only option
#ifdef EXO_BOOST_REGEX
	//if (regex_options.contains("l"))
	if (regex_options.find('l') != std::string::npos) {
		regex_syntax_flags |= std_boost::regex_constants::literal;
	}
	else
#endif

	// g = grep - Use the regular expression grammar used by the grep utility in POSIX.
	// eg = egrep (like grep -E) This is effectively the same as the basic option with the addition of newline '\n' as an alternation separator.
	if (regex_options.find('g') != std::string::npos) {
		if (regex_options.find('e') != std::string::npos)
			regex_syntax_flags |= std_boost::regex_constants::egrep;
		else
			regex_syntax_flags |= std_boost::regex_constants::grep;
	}

	// e = extended - Use the extended POSIX regular expression grammar
	else if (regex_options.find('e') != std::string::npos) {
			regex_syntax_flags |= std_boost::regex_constants::extended;
	}

	// b = basic - Use the basic POSIX regular expression grammar
	else if (regex_options.find('b') != std::string::npos) {
		regex_syntax_flags|=std_boost::regex_constants::basic;
	}

	// a = awk - Use the regular expression grammar used by the awk utility in POSIX
	// Use the extended POSIX regular expression grammar
	else if (regex_options.find('a') != std::string::npos) {
		regex_syntax_flags |= std_boost::regex_constants::awk;
	}

	// n = nosubs - When performing matches, all marked sub-expressions (expr) are treated
	// as non-marking sub-expressions (?:expr).
	// No matches are stored in the supplied std::regex_match structure and mark_count() is zero.
	if (regex_options.find('n') != std::string::npos) {
		regex_syntax_flags |= std_boost::regex_constants::nosubs;
	}

	return regex_syntax_flags;
}

/////////////////////
// regex_engine maker
/////////////////////

static REGEX varregex_make_regex_engine(SV regex_str, SV regex_options) {

	//u32regex engine
	//TRACE(sizeof(REGEX)) // 16 bytes

	try {
		return boost::make_u32regex(
//				std::string(regex_str),
			regex_str.begin(),
			regex_str.end(),
			// Analyse the regex_options requested
			get_syntax_flags(regex_options)
		);
	}
	catch (boost::wrapexcept<std::out_of_range>& e) {
		throw VarError("Error: (1) Invalid data during match of " ^ var(regex_str).first(512).quote() ^ ". " ^ var(e.what()));
	}
	catch (std_boost::regex_error& e) {
		throw VarError("Error: Invalid regex string " ^ var(regex_str).first(128).first(512).quote() ^ ". " ^ var(e.what()).quote());
	}
	catch (...) {
		throw VarError("Error: Unknown error in regex string " ^ var(regex_str).first(128).first(512).quote() ^ ".");
	}

	// Cannot get here
	//std::unreachable();
	//throw VarError("getreg");
}

//////////////////
// getregex_engine helper/cache
//////////////////

// Thread_local cache
static thread_local std::map<std::string, REGEX> thread_regexes;

// Caching regex giving very approx 10x speed up
static REGEX& varregex_get_regex_engine(SV regex_str, SV regex_options) {

	// TODO merge with get_syntax_flags?
	// TODO arg should be just a rex?

	// Prepare key to search cache
	// cache with regex_str and regex_options since regex_options change the effect
	auto cache_key = std::string(regex_str);
	if (! regex_options.empty()) {
		cache_key.push_back(FM_);
		cache_key += regex_options;
	}

	//u32regex engine
	//TRACE(sizeof(REGEX)) // 16 bytes

	// Find and return a pre built regex engine
	auto mapiter1 = thread_regexes.find(cache_key);
	if (mapiter1 != thread_regexes.end())
		return mapiter1->second;

//	TRACE("make cache")

	// Make and save a new regex engine
	// https://en.cppreference.com/w/cpp/container/map/emplace
	auto [mapiter2, success] = thread_regexes.emplace(
		cache_key,

		// Throws VarError if regex_str is invalid
		varregex_make_regex_engine(regex_str, regex_options)
	);

	// Can only fail if key already exists but we already checked that it did not.
	// Can normally throw above if regex_str is invalid.
	if (! success)
		throw VarError("Error: " ^ var(__PRETTY_FUNCTION__) ^ " failed to add regex cache " ^ var(regex_str).first(512).quote() ^ ".");

	return mapiter2->second;
}

////////
// match
////////

// should be in mvfuncs.cpp - here really because boost regex is included here for file matching
var  var::match(SV regex_str, SV regex_options) const {

	THISIS("var  var::match(SV regex_str, SV regex_options) const")
	assertString(function_sig);
	//ISSTRING(regex_str)

	// wild cards like
	// *.* or *.???
	// *abcde
	// abcde*
	//if (regex_options.contains("w")) { //c++20
	if (regex_options.find('w') != std::string::npos) {

		// rules of glob - converting glob to regex

		// 0. the following regexe special characters are not special so, if present in
		// match expression, they must be escaped
		// https://stackoverflow.com/questions/1252992/how-to-escape-a-string-for-use-in-boost-regex
		// const std_boost::regex esc("[.^$|()\\[\\]{}*+?\\\\]");
		// allow * and ? in glob expressions
		// Special chars ". ^ $ | ( ) [ ] { } + \"
		const std_boost::regex regex_special_chars{R"__([.^$|()\[\]{}+\\])__"};

#ifdef EXO_BOOST_REGEX
		const std::string replacement_for_regex_special = R"__(\\$&)__";
#else
		const std::string replacement_for_regex_special = R"__(\$&)__";
#endif

		//std::string regex2 = std_boost::regex_replace(regex.var_str, regex_special_chars,
		std::string regex2 = std_boost::regex_replace(std::string(regex_str), regex_special_chars,
											  replacement_for_regex_special);

		// 1. force to match whole string
		var regex3 = "^" + regex2 + "$";

		// 2. * matches zero or more characters like .* in regex
		regex3.replacer("*", ".*");

		// 3. ? matches any one character
		regex3.replacer("?", ".");
		//TRACE(regex3)

		// having created a suitable regex str, recursively call match with it
		return this->match(regex3);
	}

	return this->match(rex(regex_str, regex_options));
}

// should be in mvfuncs.cpp - here really because boost regex is included here for file matching
var  var::match(const rex& regex) const {

	// VISUALISE REGULAR EXPRESSIONS GRAPHICALLY
	// https:www.debuggex.com

	THISIS("var  var::match(const rex& regex) const")
	assertString(function_sig);

	// Get the engine from the rex if present otherwise get one from the cache (or construct one in the cache)
//	std::cout << &regex << std::endl;
//	TRACE(regex.pimpl_ == nullptr)
	REGEX& regex_engine = regex.pimpl_ ? *static_cast<boost::u32regex*>(regex.pimpl_) : varregex_get_regex_engine(regex.regex_str_, regex.options_);

	// construct our iterators:
#ifdef EXO_BOOST_REGEX
	boost::u32regex_iterator<std::string::const_iterator> iter;
	try {
		iter = boost::make_u32regex_iterator(var_str, regex_engine);
	} catch (boost::wrapexcept<std::out_of_range>& e) {
		throw VarError("Error: (2a) Invalid match string " ^ var(regex.regex_str_).quote() ^ ". " ^ var(e.what()));
	}
#else
	std::regex_iterator<std::string::const_iterator> iter(var_str.begin(), var_str.end(), regex_engine);
#endif
	decltype(iter) end{};

	// Prepare to return empty string
	let result;
	result.var_typ = VARTYP_STR;

	// NOT using declarative functional style "for_each with lambda" instead of "while (iter!=end)" loop
	// Because we need to optionally break out after first match
//	std::for_each(iter, end,
//		[&result](auto match_results) {

//	bool firstonly = regex.options_.find('f') != std::string::npos;
	bool firstonly = regex.options_.contains("f");
	while (iter != end) {

		auto match_results = *iter;

		// group 0 is complete match
		// any subgroups are groupn 1+
//			for (int groupn = 0; uint(groupn) <= match_results.size(); ++groupn) {
		for (int groupn = 0; uint(groupn) < match_results.size(); ++groupn) {
			// std::cout<< match_results[0] << std::endl;
			result.var_str.append(match_results[groupn]);
			//result.var_str.append(var(groupn).toString());

			// Groups are separated by VM
			result.var_str.push_back(VM_);
		}

		// Remove trailing VM
		if (!result.var_str.empty()) {
			result.var_str.pop_back();
		}

		// Multiple matches are separated by FM
		result.var_str.push_back(FM_);

		if (firstonly)
			break;

		iter++;
	}

	// Remove trailing FM
	if (!result.var_str.empty())
		result.var_str.pop_back();

	return result;

}

/////////
// search
/////////

// should be in mvfuncs.cpp - here really because boost regex is included here for file matching
var  var::search(SV regex_str, io startchar1, SV regex_options) const {

	THISIS("var  var::search(SV regex_str, int startchar1, SV regex_options) const")
	assertString(function_sig);

	// Note that option f - first only is deliberately not implemented in search

	// startchar1 can unassigned or numeric
	if (startchar1.unassigned())
		startchar1 = 1;
	else
		startchar1.assertInteger(__PRETTY_FUNCTION__, "startchar1");

	// TODO deduplicate in ::match and ::search

	// wild cards like
	// *.* or *.???
	// *abcde
	// abcde*
	//if (regex_options.contains("w")) { //c++20
	if (regex_options.find('w') != std::string::npos) {

		// rules of glob - converting glob to regex

		// 0. the following regexe special characters are not special so, if present in
		// match expression, they must be escaped
		// https://stackoverflow.com/questions/1252992/how-to-escape-a-string-for-use-in-boost-regex
		// const std_boost::regex esc("[.^$|()\\[\\]{}*+?\\\\]");
		// allow * and ? in glob expressions
		// Special chars ". ^ $ | ( ) [ ] { } + \"
		const std_boost::regex regex_special_chars{R"__([.^$|()\[\]{}+\\])__"};

#ifdef EXO_BOOST_REGEX
		const std::string replacement_for_regex_special = R"__(\\$&)__";
#else
		const std::string replacement_for_regex_special = R"__(\$&)__";
#endif

		//std::string regex2 = std_boost::regex_replace(regex.var_str, regex_special_chars,
		std::string regex2 = std_boost::regex_replace(std::string(regex_str), regex_special_chars,
											  replacement_for_regex_special);

		// 1. force to match whole string
		var regex3 = "^" + regex2 + "$";

		// 2. * matches zero or more characters like .* in regex
		regex3.replacer("*", ".*");

		// 3. ? matches any one character
		regex3.replacer("?", ".");
		//TRACE(regex3)

		// having created a suitable regex str, recursively call match with it
		return this->search(regex3, startchar1);
	}

    return this->search(rex(regex_str, regex_options), startchar1);

}

// Version that accepts a rex object for speed
var  var::search(const rex& regex, io startchar1) const {

	THISIS("var  var::search(const rex& regex, int startchar1) const")
	assertString(function_sig);

	// Get the engine from the rex if present otherwise get one from the cache (or construct one in the cache)
//	std::cout << &regex << std::endl;
//	TRACE(regex.pimpl_ == nullptr)
	REGEX& regex_engine = regex.pimpl_ ? *static_cast<boost::u32regex*>(regex.pimpl_) : varregex_get_regex_engine(regex.regex_str_, regex.options_);

	// https://stackoverflow.com/questions/26320987/what-is-the-difference-between-regex-token-iterator-and-regex-iterator
	// boost::u32regex_token_iterator<std::string::const_iterator>
	//	iter {std_boost::make_u32regex_token_iterator(var_str,regex,{0,1,2,3,4,etc or -1})};
	// token_iterator allow you to access none-matching parts of the string for parsing stuff
	// but doesnt return an iterator with an array of groups

	// Default return empty string
	let result = "";
	result.var_typ = VARTYP_STR;

	// Negative start treated same as zero
	if (startchar1 < 1) UNLIKELY
		startchar1 = 1;

	// Excessive start returns no match and pointing to end of string i.e. null byte
	if (startchar1 > static_cast<int>(var_str.size())) UNLIKELY {
		// Return empty string
		//startchar1 = var_str.length() + 1;
		startchar1 = 0;
		return result;
	}

	// Construct our iterators:

#ifdef EXO_BOOST_REGEX
	boost::u32regex_iterator<const char*> iter;
	try {
//		iter = boost::make_u32regex_iterator(std::advance(var_str.begin(), startchar1), regex);
		// Unfortunately boost::make_u32regex_iterator only supports std::string and const char*
		// and we need start in the middle of std::string so we must use const char*
		// so any \0 byte in var_str will terminate the match.
		// TODO how to create an iterator over std::string but starting in the middle.
		///root/exodus/exodus/libexodus/exodus/varregex.cpp:488:55: warning: unsafe pointer arithmetic [-Wunsafe-buffer-usage]
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
		// Sadly u32regex will not take a std::string_view so we point into the middle of a std::string (unsafe)
		iter = boost::make_u32regex_iterator(var_str.data() + startchar1.var_int - 1, regex_engine);
#pragma clang diagnostic pop
	} catch (boost::wrapexcept<std::out_of_range>& e) {
		throw VarError("Error: (2b) Invalid match string " ^ var(regex.regex_str_).quote() ^ ". " ^ var(e.what()));
	}
#else
	std::regex_iterator<std::string::const_iterator> iter(std::advance(var_str.begin(), startchar1 - 1), var_str.end(), regex_engine);
#endif

	// Quit if no match
	decltype(iter) end{};
	if (iter ==  end) {
		// Return empty string
		//startchar1 = var_str.length() + 1;
		startchar1 = 0;
		return result;
	}

	// Get access to various match data
	// https://en.cppreference.com/w/cpp/regex/regex_search
	auto& match_results = *iter;

	// Calculate the new startchar1 to be returned as an io_arg.
	// The end of the matching sequence (one after last char of match)
	startchar1.var_int = match_results[0].second - var_str.data() + 1;
	startchar1.var_typ = VARTYP_INT;

	// Group 0 is complete match of all groups
	// Group 1+ are any subgroups
	for (uint groupn = 0; groupn < match_results.size(); ++groupn) {

		// Return the matching string
		result.var_str.append(match_results[groupn]);

		// Groups are separated by VM
		result.var_str.push_back(VM_);
	}

	// Remove trailing VM after all groups
	if (!result.var_str.empty())
		result.var_str.pop_back();

	return result;

}

// mutator
IO   var::replacer(const rex& regex, SomeFunction(in match_str)) REF {
    *this = this->replace(regex, sf);
    return THIS;
}

// const
var  var::replace(const rex& regex, SomeFunction(in match_str)) const {

	THISIS("var  var::replace(const rex& regex, SomeFunction(in match_str)) const")
	assertString(function_sig);

	// Get the engine from the rex if present otherwise get one from the cache (or construct one in the cache)
//	std::cout << &regex << std::endl;
//	TRACE(regex.pimpl_ == nullptr)
	REGEX& regex_engine = regex.pimpl_ ? *static_cast<boost::u32regex*>(regex.pimpl_) : varregex_get_regex_engine(regex.regex_str_, regex.options_);

	// https://stackoverflow.com/questions/26320987/what-is-the-difference-between-regex-token-iterator-and-regex-iterator
	// boost::u32regex_token_iterator<std::string::const_iterator>
	//	iter {std_boost::make_u32regex_token_iterator(var_str,regex,{0,1,2,3,4,etc or -1})};
	// token_iterator allow you to access none-matching parts of the string for parsing stuff
	// but doesnt return an iterator with an array of groups

	// Default return empty string
	var nrvo = "";

	// Get 1st iter (1st match)

#ifdef EXO_BOOST_REGEX
	boost::u32regex_iterator<const char*> iter;
	try {
//		iter = boost::make_u32regex_iterator(std::advance(var_str.begin(), startchar1), regex);
		// Unfortunately boost::make_u32regex_iterator only supports std::string and const char*
		// and we need start in the middle of std::string so we must use const char*
		// so any \0 byte in var_str will terminate the match.
		// TODO how to create an iterator over std::string but starting in the middle.
		///root/exodus/exodus/libexodus/exodus/varregex.cpp:488:55: warning: unsafe pointer arithmetic [-Wunsafe-buffer-usage]
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
		// Sadly u32regex will not take a std::string_view so we point into the middle of a std::string (unsafe)
		iter = boost::make_u32regex_iterator(var_str.data(), regex_engine);
#pragma clang diagnostic pop
	} catch (boost::wrapexcept<std::out_of_range>& e) {
		throw VarError("Error: (2b) Invalid match string " ^ var(regex.regex_str_).quote() ^ ". " ^ var(e.what()));
	}
#else
	std::regex_iterator<std::string::const_iterator> iter(std::advance(var_str.begin(), startchar1 - 1), var_str.end(), regex_engine);
#endif

	// end_iter
	decltype(iter) end_iter{};

	// Maintain a last iterator to use for the ending.
//	std::string::const_iterator last_iter = var_str.begin();
	auto last_pos0 = 0;

	for (;iter != end_iter; ++iter) {

		// Get access to various match data
		// https://en.cppreference.com/w/cpp/regex/regex_search
		//auto& match_results = *iter;

		// Get match data
//		const boost::match_results<Iter>& match_date = *iter;
		auto match_data = *iter;

		// Group 0 is complete match of all groups
		// Group 1+ are any subgroups
		std::string matches = "";
		for (std::size_t groupn = 0; groupn < match_data.size(); ++groupn) {

			// Return the matching string
			matches.append(match_data[groupn].str());

			// Groups are separated by VM
			matches.push_back(VM_);
		}

		// Remove trailing VM after all groups
		if (not matches.empty())
			matches.pop_back();

		// Append the unmatched bit before the match
		nrvo.var_str.append(match_data.prefix());

		// Append the replacement
//		nrvo.var_str.append(sf(matches));
		nrvo ^= sf(matches);

		// Point to after the match
		// /usr/bin/../lib/gcc/x86_64-linux-gnu/99/../../../../include/c++/99/bits/stl_iterator.h:1047:11:
		// note: candidate function (the implicit copy assignment operator) not viable:
		// no known conversion from 'const char *const' to 'const __normal_iterator<char *, basic_string<char>>' for 1st argument
		//last_iter = match_data[0].second;
		last_pos0 = match_data[0].second - var_str.data();
	}

	// Append the unmatched tail (from last_pos0 to end)
	if (last_pos0 < var_str.size()) {
		nrvo.var_str.append(var_str.data() + last_pos0, var_str.size() - last_pos0);
	}

	return nrvo;

}





//////////
// replace
//////////

// Simple non-regex case sensitive substr replacement

// constant
var  var::replace(SV what, SV with) const& {

	//return this->clone().replacer(what, with);}
	return boost::algorithm::replace_all_copy(var_str, what, with);
}

// mutator
IO   var::replacer(SV what, SV with) REF {

	THISIS("void var::replacer(SV what, SV with) &")
	assertStringMutator(function_sig);

	// nothing to do if oldstr is ""
	if (what.empty())
		return THIS;

	boost::algorithm::replace_all(var_str, what, with);

	return THIS;

}

// Regex based string replacement
/////////////////////////////////

// only here really because boost regex is included here for file matching

// mutator
IO   var::replacer(const rex& regex, SV replacement) REF {
	*this = this->replace(regex, replacement);
	return THIS;
}

// const
var  var::replace(const rex& regex, SV replacement) const& {

//	std::cout << "\n" << &regex << " " << __PRETTY_FUNCTION__ << std::endl;

	THISIS("var  var::replace(const rex& regex, SV replacement) const")
	assertString(function_sig);

	// http://www.boost.org/doc/libs/1_38_0/libs/regex/doc/html/boost_regex/syntax/basic_syntax.html

	// Build the regex object with the given regex_options
//	REGEX& regex_engine = varregex_get_regex_engine(regex.regex_str_, regex.options_);

	// Get the engine from the rex if present otherwise get one from the cache (or construct one in the cache)
//	std::cout << &regex << std::endl;
//	TRACE(regex.pimpl_ == nullptr)
	REGEX& regex_engine = regex.pimpl_ ? *static_cast<boost::u32regex*>(regex.pimpl_) : varregex_get_regex_engine(regex.regex_str_, regex.options_);

	// Get an ostringstream object and an iterator on it
	std::ostringstream oss1(std::ios::out | std::ios::binary);
	std::ostream_iterator<char, char> oiter(oss1);

	// match_flags
	//////////////

	// https://en.cppreference.com/w/cpp/regex/match_flag_type

	// match_flags are only used in replacements
	auto match_flags = std_boost::match_flag_type(boost::match_default);
	// Option f = first only, otherwise all
	//c++23 if (regex_options.contains("f"))

	// TODO check syntax letters do not overlap between syntax_flags and match_flags

	// f - first only
	if (regex.options_.var_str.find('f') != std::string::npos)
		match_flags |= boost::format_first_only;
	else
		match_flags |= boost::format_all;

	// Generate output
	//////////////////

	try {
		std_boost::u32regex_replace(
			oiter,
			var_str.begin(),
			var_str.end(),
			regex_engine,
			// Sadly boost regex cannot handle string_view
			//replacement,
			std::string(replacement),
			match_flags
		);
	} catch (boost::wrapexcept<std::out_of_range>& e) {
		let errmsg =
			"Error: (3) Invalid data or replacement during regex replace of "
			^ var(regex.regex_str_).quote()
			^ " with "
			^ var(replacement).quote()
			^ ". "
			^ var(e.what())
		;
		//TODO Show some of the invalid UTF8 bytes maybe.
		//Cannot show actual data because it could be a security breach.
		//errmsg ^= ". Data[1,128] = " ^ this->first(128).quote();
		throw VarError(errmsg);
	}

	// Acquire and return the output
	let result = std::move(oss1).str();
	return result;

}

// OCONV_MR can be moved back to mvioconv.cpp if it stops using regular expressions
///////////

// Regular expressions for OCONV_MR - Note: io and non-const
io   var::oconv_MR(const char* conversion) {

	THISIS("io   var::oconv_MR(const char* conversion) const")

	// abort if no 3rd char
	if (*conversion == '\0')
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
	if (*conversion == '/') {

#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
		++conversion;
#pragma GCC diagnostic pop

		switch (*conversion) {

			// MR/N return everything except digits. Remove all digits 0123456789 (unicode digits?)
			case 'N': {
				//var_str=std_boost::regex_replace(toTstring((*this)),digits_regex, "");
				var_str = REGEX_REPLACE(var_str, digits_regex, "");
				break;
			}

			// MR/A return everything except "alphabetic". Remove all "alphabetic"
			case 'A': {
				//var_str=std_boost::regex_replace(toTstring((*this)),alpha_regex, "");
				var_str = REGEX_REPLACE(var_str, alpha_regex, "");
				break;
			}
			// MR/B return everything except "alphanumeric". Remove all "alphanumeric"
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

	switch (*conversion) {
		// MRN return only digits i.e. remove all non-digits
		case 'N': {
			// var_str=std_boost::regex_replace(toTstring((*this)),non_digits_regex, "");
			var_str = REGEX_REPLACE(var_str, non_digits_regex, "");
			break;
		}
		// MRA return only "alphabetic" i.e. remove all "non-alphabetic"
		case 'A': {
			// var_str=std_boost::regex_replace(toTstring((*this)),non_alpha_regex, "");
			var_str = REGEX_REPLACE(var_str, non_alpha_regex, "");
			break;
		}
		// MRB return only "alphanumeric" i.e. remove all "non-alphanumeric"
		case 'B': {
			// var_str=std_boost::regex_replace(toTstring((*this)),non_alphanum_regex, "");
			var_str = REGEX_REPLACE(var_str, non_alphanum_regex, "");
			break;
		}
		// MRL to lower case
		case 'L': {
			lcaser();
			break;
		}
		// MRU to upper case
		case 'U': {
			ucaser();
			break;
		}
		// MRT to tcase (capitalise)
		case 'T': {
			tcaser();
			break;
		}
		// No conversion if not N, A, B, L, U or T
		default:;
	}

	return *this;
}

//////
// rex
//////

// Private default constructor
rex::rex() {
}

// Public constructor
rex::rex(SV regex_str, SV options)
	:
	regex_str_(regex_str),
	options_(options) {

	// Skip creation of regex engine and use the regex cache
	// every time this rex is used
	if (options.find('c') == std::string::npos)
		return;

	// Create a regex engine on the heap
	// and store a pointer to it in our member data "pimpl_"
	// Destructor deletes it from the heap

	pimpl_ = static_cast<boost::u32regex*>(
		new boost::u32regex(
			varregex_make_regex_engine(regex_str, options)
		)
	);
}

// Destructor - Removes regex engine, if any, from the heap
rex::~rex() {
	delete static_cast<boost::u32regex*>(pimpl_);
}

// User defined literal suffix "_rex" e.g. "[a-z]"_rex
ND rex operator""_rex(const char* cstr, std::size_t size) {

	// For user defined literal suffix _rex we will
	// using on the fly creation and caching of regex engines
	// since suffixes are typically used in throw-away contexts
	// e.g. as arguments of replace function
	// return rex(std::string_view(cstr, size));
	// so there is no point in creating a regex engine on the heap

	// Capture the given regex str.
	// It will be used later on demand to get the regex engine
	// from the cache (or make it and save it in the cache)
	// Arguments reversed to avoid ambiguity between constructors
	return rex(cstr, size, true);

}

} // namespace exo
