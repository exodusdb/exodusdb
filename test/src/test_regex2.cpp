#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#ifdef EXO_MODULE
	import std;
#else
#	include <iostream>
#	include <regex>
#	include <string>
#endif

#include <exodus/program.h>
programinit()

func main() {

	printl("z says 'Hello World!'");

// Evade clang 18 warning about for std::regex -Wdeprecated-declarations
// Probably need some additional "using" clause in std.cppm or #include <iostream> above to get some missing definitions
#ifndef EXO_MODULE
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
#endif

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

	{
		// rex object - rex
		// Private copy on heap - 160 ns/op
		// but prone to bad programming in loop
		rex rex1("def(abc)|ghf|[abc]+.*?def(abc)|ghf|[abc]+.*$");
		var t = "abc500";
		for (auto i [[maybe_unused]]: range(0, 1'000'000)) {
			t.replacer(rex1, "");
		}
	}
	{
		// User literal - _rex
		// Caching in std:: map - 190 ns/op
		errput("1'000'000 replacer ");
		var t = "abc500";
		for (auto i [[maybe_unused]]: range(0, 1'000'000)) {
			t.replacer("def(abc)|ghf|[abc]+.*?def(abc)|ghf|[abc]+.*$"_rex, "");
		}
		errput();
	}

	// Default regex options is multiline in exodus (because boost::regex was)
	// Whereas std::regex is default single line

	assert("abc\ndef\nhij"_var.match("^def$") == "def");
	assert("abc\ndef\nhij"_var.search("^def$") == "def");

	{	// options using rex()

		assert("abc\ndef\nhij"_var.match(rex("^def$", "")) == "def");
		assert("abc\ndef\nhij"_var.search(rex("^def$", "")) == "def");

		assert("abc\ndef\nhij"_var.match(rex("^def$", "m")) == "def");
		assert("abc\ndef\nhij"_var.search(rex("^def$", "m")) == "def");

		// fails on single line matching
		assert("abc\ndef\nhij"_var.match(rex("^def$", "s")) == "");
		assert("abc\ndef\nhij"_var.search(rex("^def$", "s")) == "");
	}
	{
		// options not using rex()

		assert("abc\ndef\nhij"_var.match("^def$", "") == "def");
		var pos = 0;
		assert("abc\ndef\nhij"_var.search("^def$", pos, "") == "def");

		assert("abc\ndef\nhij"_var.match("^def$", "m") == "def");
		pos = 0;
		assert("abc\ndef\nhij"_var.search("^def$", pos, "m") == "def");

		// fails on single line matching
		assert("abc\ndef\nhij"_var.match("^def$", "s") == "");
		pos = 0;
		assert("abc\ndef\nhij"_var.search("^def$", pos, "s") == "");
	}

	{
	    // Original tests with c ^ c (doubling)
	    std::cout << "Testing original doubling behavior:\n";
	    assert("abc"_var.replace("b"_rex, [](auto c) { return c ^ c; }) == "abbc");
	    assert("abcbd"_var.replace("b"_rex, [](auto c) { return c ^ c; }) == "abbcbbd");
	    assert("abcb"_var.replace("b"_rex, [](auto c) { return c ^ c; }) == "abbcbb");
	    assert("bcb"_var.replace("b"_rex, [](auto c) { return c ^ c; }) == "bbcbb");
	    assert("bb"_var.replace("b"_rex, [](auto c) { return c ^ c; }) == "bbbb");
	    assert("b"_var.replace("b"_rex, [](auto c) { return c ^ c; }) == "bb");
	    assert(""_var.replace("b"_rex, [](auto c) { return c ^ c; }) == "");
	    assert("ac"_var.replace("b"_rex, [](auto c) { return c ^ c; }) == "ac");
	    std::cout << "All doubling tests passed.\n";

	    // New tests with "" (empty string replacement)
	    std::cout << "\nTesting empty string replacement:\n";
//	    auto empty_replace = [](auto c) { return var(""); };  // Define once for reuse
	    auto empty_replace = [](auto) { return var(""); };  // Define once for reuse

	    var test1 = "abc"_var.replace("b"_rex, empty_replace);
	    std::cout << test1 << "\n";  // "ac"
	    assert(test1 == "ac");

	    var test2 = "abcbd"_var.replace("b"_rex, empty_replace);
	    std::cout << test2 << "\n";  // "acd"
	    assert(test2 == "acd");

	    var test3 = "abcb"_var.replace("b"_rex, empty_replace);
	    std::cout << test3 << "\n";  // "ac"
	    assert(test3 == "ac");

	    var test4 = "bcb"_var.replace("b"_rex, empty_replace);
	    std::cout << test4 << "\n";  // "c"
	    assert(test4 == "c");

	    var test5 = "bb"_var.replace("b"_rex, empty_replace);
	    std::cout << test5 << "\n";  // ""
	    assert(test5 == "");

	    var test6 = "b"_var.replace("b"_rex, empty_replace);
	    std::cout << test6 << "\n";  // ""
	    assert(test6 == "");

	    var test7 = ""_var.replace("b"_rex, empty_replace);
	    std::cout << test7 << "\n";  // ""
	    assert(test7 == "");

	    var test8 = "ac"_var.replace("b"_rex, empty_replace);
	    std::cout << test8 << "\n";  // "ac"
	    assert(test8 == "ac");

		// Additional edge cases
		std::cout << "\nTesting edge cases with empty replacement:\n";
		// 1. Overlapping potential (non-overlapping due to regex_search)
		assert("aaa"_var.replace("aa"_rex, empty_replace) == "a");  // "aa" matches once, leaves "a"
		// 2. Pattern at start
		assert("abc"_var.replace("a"_rex, empty_replace) == "bc");
		// 3. Pattern at end
		assert("abc"_var.replace("c"_rex, empty_replace) == "ab");
		// 4. Multi-character pattern
		assert("abcde"_var.replace("cd"_rex, empty_replace) == "abe");
		// 5. Special regex chars (assuming _rex doesn't escape)
		assert("a.b"_var.replace("a.b"_rex, empty_replace) == "");  // Matches "a.b" literally
		// 6. Empty pattern (might loop infinitely, but Boost handles it)
		assert("abc"_var.replace(""_rex, empty_replace) == "abc");  // No match or safe exit
		// 7. UTF-8 non-ASCII
		assert("café"_var.replace("é"_rex, empty_replace) == "caf");  // Multi-byte char
		// 8. Whitespace
		assert("a b c"_var.replace(" "_rex, empty_replace) == "abc");
		// 9. Case sensitivity
		assert("aBc"_var.replace("b"_rex, empty_replace) == "aBc");  // "b" != "B"
		// 10. Full string match
		assert("abc"_var.replace("abc"_rex, empty_replace) == "");
		std::cout << "All edge case tests passed.\n";

	}

	{
		// Check that replace using function groups to work on
		var words = "programinit libraryinit programexit libraryexit";
		words.replacer("(program|library)(init|exit)"_rex, [](auto groups){TRACE(groups); return groups.f(1,2).first(3) ^ groups.f(1,3);});
		assert(words == "proinit libinit proexit libexit");
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
