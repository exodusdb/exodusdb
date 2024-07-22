#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing

#include <cassert>
#include <iostream>
#include <regex>
#include <string>
 
#include <exodus/program.h>
programinit()

function main() {

	printl("z says 'Hello World!'");

	{
		// Example of std::regex from cppref
		// https://en.cppreference.com/w/cpp/regex/syntax_option_type

	    std::string str = "zzxayyzz";
	    std::regex re1(".*(a|xayy)"); // ECMA
	    std::regex re2(".*(a|xayy)", std::regex::extended); // POSIX
	 
	    std::cout << "Searching for .*(a|xayy) in zzxayyzz:\n";
	    std::smatch m;
	    std::regex_search(str, m, re1);
	    std::cout << "  ECMA (depth first search) match: " << m[0] << '\n';
	    std::regex_search(str, m, re2);
	    std::cout << "  POSIX (leftmost longest)  match: " << m[0] << '\n';

		// Clang 5-18 CORRECTLY say:

		// Searching for .*(a|xayy) in zzxayyzz:
		//   ECMA (depth first search) match: zzxa
		//   POSIX (leftmost longest)  match: zzxayy

		//g++ 10-14 INCORRECTLY say

		// Searching for .*(a|xayy) in zzxayyzz:
		//   ECMA (depth first search) match: zzxa
		//   POSIX (leftmost longest)  match: zzxa

	}

	// EXODUS regex options

	// w - glob not regex (for match)

	// l - literal

	// i - case insensitive

	// p - ECMAScript/Perl default
	// b - basic posix
	// e - extended posix

	// m - multiline. default in boost
	// s - single line. default in std::regex

	{
		// Derived from
		// https://en.cppreference.com/w/cpp/regex/syntax_option_type
		var v1 = "zzxayyzz";
		var rv = ".*(a|xayy)";
		assert(v1.replace(rex(rv, ""  ), R"/(\(\1\))/").outputl() == "(a)yyzz" ); // ECMA           depth first
		assert(v1.replace(rex(rv, "b" ), R"/(\(\1\))/").outputl() == v1        ); // BASIC POSIX    no change here
		assert(v1.replace(rex(rv, "e" ), R"/(\(\1\))/").outputl() == "(xayy)zz"); // EXTENDED POSIX leftmost longest
		assert(v1.replace(rex(rv, "a" ), R"/(\(\1\))/").outputl() == "(xayy)zz"); // AWK
		assert(v1.replace(rex(rv, "g" ), R"/(\(\1\))/").outputl() == v1        ); // GREP = BASIC plus \n
		assert(v1.replace(rex(rv, "eg"), R"/(\(\1\))/").outputl() == "(xayy)zz"); // EGREP
	}
        // single-line. ^ only matches beginning of string and not any embedded \n chars
        assert(replace("aaa\nbbb\nccc\n", rex("^b.*?$", "s"), "QQQ").outputl() eq "aaa\nbbb\nccc\n");

        // multi-line ^ matches beginning of string AND any embedded \n chars
        assert(replace("aaa\nbbb\nccc\n", rex("^b.*?$", "m"), "QQQ").outputl() eq "aaa\nQQQ\nccc\n");

        // DEFAULT multiline ^ matches beginning of string AND any embedded \n chars
        assert(replace("aaa\nbbb\nccc\n", rex("^b.*?$", "m"), "QQQ").outputl() eq "aaa\nQQQ\nccc\n");

	{
		var v1 = "zzxa\nyyzz";
		var rv = ".*";
		assert(v1.replace(rex(rv, ""  ), R"/(\(\1\))/").outputl() == "()()"); // ECMA           depth first
		assert(v1.replace(rex(rv, "b" ), R"/(\(\1\))/").outputl() == "()()"); // BASIC POSIX    no change here
		assert(v1.replace(rex(rv, "e" ), R"/(\(\1\))/").outputl() == "()()"); // EXTENDED POSIX leftmost longest
		assert(v1.replace(rex(rv, "a" ), R"/(\(\1\))/").outputl() == "()()"); // AWK
		assert(v1.replace(rex(rv, "g" ), R"/(\(\1\))/").outputl() == "()()"); // GREP
		assert(v1.replace(rex(rv, "eg"), R"/(\(\1\))/").outputl() == "()()"); // EXTENDED GREP

		// s - single line
		assert(v1.replace(rex(rv, "s"  ), R"/(\(\1\))/").outputl() == "()()"); // ECMA           depth first
		assert(v1.replace(rex(rv, "sb" ), R"/(\(\1\))/").outputl() == "()()"); // BASIC POSIX    no change here
		assert(v1.replace(rex(rv, "se" ), R"/(\(\1\))/").outputl() == "()()"); // EXTENDED POSIX leftmost longest
		assert(v1.replace(rex(rv, "sa" ), R"/(\(\1\))/").outputl() == "()()"); // AWK
		assert(v1.replace(rex(rv, "sg" ), R"/(\(\1\))/").outputl() == "()()"); // GREP
		assert(v1.replace(rex(rv, "seg"), R"/(\(\1\))/").outputl() == "()()"); // EXTENDED GREP

		// m - multiline
		assert(v1.replace(rex(rv, "m"  ), R"/(\(\1\))/").outputl() == "()()"); // ECMA           depth first
		assert(v1.replace(rex(rv, "mb" ), R"/(\(\1\))/").outputl() == "()()"); // BASIC POSIX    no change here
		assert(v1.replace(rex(rv, "me" ), R"/(\(\1\))/").outputl() == "()()"); // EXTENDED POSIX leftmost longest
		assert(v1.replace(rex(rv, "ma" ), R"/(\(\1\))/").outputl() == "()()"); // AWK
		assert(v1.replace(rex(rv, "mg" ), R"/(\(\1\))/").outputl() == "()()"); // GREP
		assert(v1.replace(rex(rv, "meg"), R"/(\(\1\))/").outputl() == "()()"); // EXTENDED GREP

	}

	{
		var v1 = "zzxa\nyyzz";
		var rv = "^.*$";
		assert(v1.replace(rex(rv, ""  ), R"/(\(\1\))/").outputl() == "()"); // ECMA           depth first
		assert(v1.replace(rex(rv, "b" ), R"/(\(\1\))/").outputl() == "()"); // BASIC POSIX    no change here
		assert(v1.replace(rex(rv, "e" ), R"/(\(\1\))/").outputl() == "()"); // EXTENDED POSIX leftmost longest
		assert(v1.replace(rex(rv, "a" ), R"/(\(\1\))/").outputl() == "()"); // AWK
		assert(v1.replace(rex(rv, "g" ), R"/(\(\1\))/").outputl() == "()"); // GREP
		assert(v1.replace(rex(rv, "eg"), R"/(\(\1\))/").outputl() == "()"); // EXTENDED GREP

		// s - single line
		assert(v1.replace(rex(rv, "s"  ), R"/(\(\1\))/").outputl() == "()"); // ECMA           depth first
		assert(v1.replace(rex(rv, "sb" ), R"/(\(\1\))/").outputl() == "()"); // BASIC POSIX    no change here
		assert(v1.replace(rex(rv, "se" ), R"/(\(\1\))/").outputl() == "()"); // EXTENDED POSIX leftmost longest
		assert(v1.replace(rex(rv, "sa" ), R"/(\(\1\))/").outputl() == "()"); // AWK
		assert(v1.replace(rex(rv, "sg" ), R"/(\(\1\))/").outputl() == "()"); // GREP
		assert(v1.replace(rex(rv, "seg"), R"/(\(\1\))/").outputl() == "()"); // EXTENDED GREP

		// m - multiline
		assert(v1.replace(rex(rv, "m"  ), R"/(\(\1\))/").outputl() == "()"); // ECMA           depth first
		assert(v1.replace(rex(rv, "mb" ), R"/(\(\1\))/").outputl() == "()"); // BASIC POSIX    no change here
		assert(v1.replace(rex(rv, "me" ), R"/(\(\1\))/").outputl() == "()"); // EXTENDED POSIX leftmost longest
		assert(v1.replace(rex(rv, "ma" ), R"/(\(\1\))/").outputl() == "()"); // AWK
		assert(v1.replace(rex(rv, "mg" ), R"/(\(\1\))/").outputl() == "()"); // GREP
		assert(v1.replace(rex(rv, "meg"), R"/(\(\1\))/").outputl() == "()"); // EXTENDED GREP

	}
	{
		var v1 = "zzxa\nayzz";
		var rv = "^a.*";
		assert(v1.replace(rex(rv, ""  ), R"/(\(\1\))/").outputl() == "zzxa\n()"); // ECMA           depth first
		assert(v1.replace(rex(rv, "m" ), R"/(\(\1\))/").outputl() == "zzxa\n()"); // ECMA           depth first
		assert(v1.replace(rex(rv, "s" ), R"/(\(\1\))/").outputl() == v1); // ECMA           depth first
	}

	{
		// Unfortunately .* GREEDILY gobbles up new lines as well!
		// Despite claim for "multiline" regex
		var v1 = "zzxa\nayzz\n123";
		var rv = "^a.*";
		assert(v1.replace(rex(rv, ""  ), R"/(\(\1\))/").outputl() == "zzxa\n()"); // ECMA           depth first
		assert(v1.replace(rex(rv, "m" ), R"/(\(\1\))/").outputl() == "zzxa\n()"); // ECMA           depth first
		assert(v1.replace(rex(rv, "s" ), R"/(\(\1\))/").outputl() == v1); // ECMA           depth first
	}

	{
		// UNLESS you put a NON-GREEDY limit on it
		// e.g. .*?$
		var v1 = "zzxa\nayzz\n123";
		var rv = "^a.*?$";
		assert(v1.replace(rex(rv, ""  ), R"/(\(\1\))/").outputl() == "zzxa\n()\n123"); // ECMA           depth first
		assert(v1.replace(rex(rv, "m" ), R"/(\(\1\))/").outputl() == "zzxa\n()\n123"); // ECMA           depth first
		assert(v1.replace(rex(rv, "s" ), R"/(\(\1\))/").outputl() == v1); // ECMA           depth first
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
