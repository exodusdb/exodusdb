#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

	function main() {
	printl("test_funcs says 'Hello World!'");

	{
		printl("Tabbed output");
		printt("a", "b", "c");
		printl();

		printl("Prefixed output");
		"test"_var.quote().output("test=");

		printl("Unbuffered output to stderr");
		errputl("Some err msg");

		printl("log buffered output to stderr");
		logputl("Some log msg");
	}

	{
		var x = "";
		assert(assigned(x));

		var y;
		assert(unassigned(y));
	}
	{
		// move
		var s1 = "1";
		var s2 = "2";
		move(s1, s2);
		assert(s1         eq "");
		assert(s2         eq "1");
	}
	{
		// swap
		var s1 = "1";
		var s2 = "2";
		swap(s1, s2);
		assert(s1         eq "2");
		assert(s2         eq "1");

		std::swap(s1, s2);
		assert(s1         eq "1");
		assert(s2         eq "2");
	}
	{
		var started = ostime().dump("ostime=");
		ossleep(200);
		var stopped = ostime();
		// avoid testing if time crosses 00:00:00
		if (stopped > started)
			assert(ostime() - started > 0.1);
	}
	{
		assert(ospid().dump("ospid=").match("\\d+"));
	}
	{
		try {
			abort("asd");
			assert(false);
		} catch (MVAbort v) {
			printl("Caught MVAbort");
		}
	}
	{
		var x = "asd";
		assert(len(x)     eq 3);
		assert(textlen(x) eq 3);

		x = "Γιάννης";
		assert(len(x)     eq 14);
		assert(textlen(x) eq 7);
	}

	{
		//convert complex utf8 characters to utf8 characters
		var yiannis = "Γιάννης";
		assert(textconverter(yiannis, "άης", "12") eq "Γι1νν2");

		// on temporaries
		assert("Γιάννης"_var.textconvert("άης", "12") eq "Γι1νν2");
	}

	{
		//swap byte strings with byte strings
		var yiannis = "Γιάννης";
		assert(replacer(yiannis, "Γιά", "ννης") eq "ννηςννης");
		var str1 = "go to heaven";
		assert(replacer(str1, "o ", "___")      eq "g___t___heaven");
	}
	{
		// \d \D and [:number:]
		var x = "a1b23c99";
		regex_replacer(x, "\\d", "N");
		assert(x                               eq "aNbNNcNN");

		x = "a1b23c99";
		regex_replacer(x, "\\D", "N");
		assert(x        eq "N1N23N99");
		//if (BOOST_REGEX) {
		x = "a1b23c99";
		regex_replacer(x, "[[:number:]]", "N");
		assert(x        eq "aNbNNcNN");
		x = "a1b23c99";
		regex_replacer(x, "[^[:number:]]", "N");
		assert(x        eq "N1N23N99");
		//}
	}
	{
		var g = "Γιάννης";
		assert(ucase(g) eq "ΓΙΆΝΝΗΣ");
		ucaser(g).dump("ucaser=");
		assert(g        eq "ΓΙΆΝΝΗΣ");
	}
	{
		var g = "Γιάννης AbCd";
		assert(lcase(g) eq "γιάννης abcd");
		lcaser(g).dump("lcaser=");
		assert(g        eq "γιάννης abcd");
	}
	{
		var g = "γιάννης γιάννης abcd";
		assert(tcase(g) eq "Γιάννης Γιάννης Abcd");
		tcaser(g).dump("tcaser=");
		assert(g        eq "Γιάννης Γιάννης Abcd");
	}
	{
		var g = "Γιάννης Γιάννης Abcd";
		assert(fcase(g) eq "γιάννησ γιάννησ abcd");
		fcaser(g).dump("fcaser=");
		assert(g        eq "γιάννησ γιάννησ abcd");
	}

	{
		assert(unique("x`y`x`z"_var) eq convert("x`y`z", "`", RM));
		assert(unique("x^y^x^z"_var) eq convert("x^y^z", "^", FM));
		assert(unique("x]y]x]z"_var) eq convert("x]y]z", "]", VM));
		assert(unique("x}y}x}z"_var) eq convert("x}y}z", "}", SM));
		assert(unique("x|y|x|z"_var) eq convert("x|y|z", "|", TM));
		assert(unique("x~y~x~z"_var) eq convert("x~y~z", "~", ST));
	}

	{
		var x;
		x = "x`y`x`z"_var;
		uniquer(x);
		assert(x eq convert("x`y`z", "`", RM));
		x = "x^y^x^z"_var;
		uniquer(x);
		assert(x eq convert("x^y^z", "^", FM));
		x = "x]y]x]z"_var;
		uniquer(x);
		assert(x eq convert("x]y]z", "]", VM));
		x = "x}y}x}z"_var;
		uniquer(x);
		assert(x eq convert("x}y}z", "}", SM));
		x = "x|y|x|z"_var;
		uniquer(x);
		assert(x eq convert("x|y|z", "|", TM));
		x = "x~y~x~z"_var;
		uniquer(x);
		assert(x eq convert("x~y~z", "~", ST));
	}

	{
		var g = "Γιάννης Γιάννης Abcd";
		assert(fcount(g, "Γ") eq 3);
		assert(count(g, "Γ")  eq 2);

		// Empty substr count eq "" not zero
		assert(count("xyz", "")  eq "");
		assert(fcount("xyz", "") eq "");

		// Count fields, not separators
		assert(fcount("", FM)  eq 0);
		assert(fcount(FM, FM)  eq 2);
		assert(fcount(" ", FM) eq 1);

		// Count characters, works with utf8
		assert(count("", FM)  eq 0);
		assert(count(FM, FM)  eq 1);
		assert(count(" ", FM) eq 0);
	}

	{
		assert(quote("")  eq "\"\"");
		assert(squote("") eq "''");

		assert(quote(" ")  eq "\" \"");
		assert(squote(" ") eq "' '");

		assert(unquote(quote(""))  eq "");
		assert(unquote(squote("")) eq "");
	}
	{
		// on rvalues
		assert(unquote("") eq "");

		assert(unquote(_DQ) eq _DQ);
		assert(unquote(_SQ) eq _SQ);

		assert(unquote(_DQ _SQ) eq _DQ _SQ);
		assert(unquote(_SQ _DQ) eq _SQ _DQ);

		assert(unquote(_DQ _DQ) eq "");
		assert(unquote(_SQ _SQ) eq "");

		assert(unquote(_DQ _DQ _DQ) eq _DQ);
		assert(unquote(_SQ _SQ _SQ) eq _SQ);

		assert(unquote(_DQ "x" _DQ) eq "x");
		assert(unquote(_SQ "x" _SQ) eq "x");
	}
	{
		// on accessor on lvalues
		var x;
		x = ""; assert(unquote(x) eq "");

		x = _DQ; assert(unquote(x) eq _DQ);
		x = _SQ; assert(unquote(x) eq _SQ);

		x = _DQ _SQ; assert(unquote(x) eq _DQ _SQ);
		x = _SQ _DQ; assert(unquote(x) eq _SQ _DQ);

		x = _DQ _DQ; assert(unquote(x) eq "");
		x = _SQ _SQ; assert(unquote(x) eq "");

		x = _DQ _DQ _DQ; assert(unquote(x) eq _DQ);
		x = _SQ _SQ _SQ; assert(unquote(x) eq _SQ);

		x = _DQ "x" _DQ; assert(unquote(x) eq "x");
		x = _SQ "x" _SQ; assert(unquote(x) eq "x");
	}

	{
		// on mutator on lvalues
		var x;
		x = ""; x.unquoter(); assert(x eq "");

		x = _DQ; x.unquoter(); assert(x eq _DQ);
		x = _SQ; x.unquoter(); assert(x eq _SQ);

		x = _DQ _SQ; x.unquoter(); assert(x eq _DQ _SQ);
		x = _SQ _DQ; x.unquoter(); assert(x eq _SQ _DQ);

		x = _DQ _DQ; x.unquoter(); assert(x eq "");
		x = _SQ _SQ; x.unquoter(); assert(x eq "");

		x = _DQ _DQ _DQ; x.unquoter(); assert(x eq _DQ);
		x = _SQ _SQ _SQ; x.unquoter(); assert(x eq _SQ);

		x = _DQ "x" _DQ; x.unquoter(); assert(x eq "x");
		x = _SQ "x" _SQ; x.unquoter(); assert(x eq "x");
	}

	{
		assert(space(-2) eq "");
		assert(space(-1) eq "");
		assert(space(0)  eq "");
		assert(space(1)  eq " ");
		assert(space(3)  eq "   ");

		assert(str("x", -2) eq "");
		assert(str("x", -1) eq "");
		assert(str("x", 0)  eq "");
		assert(str("x", 1)  eq "x");
		assert(str("x", 3)  eq "xxx");

		assert(str("x", 0) eq "");
		assert(str("x", 1) eq "x");
		assert(str("x", 3) eq "xxx");

		assert(str("xy", 0) eq "");
		assert(str("xy", 1) eq "xy");
		assert(str("xy", 3) eq "xyxyxy");

		assert(str("ά", 1) eq "ά");
		assert(str("ά", 3) eq "άάά");

		assert(str("άν", 1) eq "άν");
		assert(str("άν", 3) eq "άνάνάν");
	}

	// Splice on temporaries
	//assert(var("abc").pasteall(-1, "XY")    eq "abXY");
	assert(var("abc").paste(-1, 999999, "XY")    eq "abXY");
	assert(var("abc").paste(-2, 1, "XY") eq "aXYc");

	{
		var x = "abcdef";
		x.cutter(-3) ^= "Q";
		assert(x eq "abcQ");

		x.cutter(-1) ^= "XYZ";
		assert(x eq "abcXYZ");
	}

//	{
//		var x = "abc";
//
//		assert(pasteall(x, 5, "XYZ") eq "abcXYZ");
//		assert(pasteall(x, 4, "XYZ") eq "abcXYZ");
//		assert(pasteall(x, 3, "XYZ") eq "abXYZ");
//		assert(pasteall(x, 2, "XYZ") eq "aXYZ");
//		assert(pasteall(x, 1, "XYZ") eq "XYZ");
//
//		assert(pasteall(x, 0, "XYZ") eq "XYZ");
//
//		assert(pasteall(x, -1, "XYZ") eq "abXYZ");
//		assert(pasteall(x, -2, "XYZ") eq "aXYZ");
//		assert(pasteall(x, -3, "XYZ") eq "XYZ");
//		assert(pasteall(x, -4, "XYZ") eq "XYZ");
//	}

	{
		// test adding prefix

		var x = "";
		assert(prefix(x,"xyz") eq "xyz");

		x.prefixer("xyz");
		assert(x eq "xyz");

		assert(prefix(x,"abc") eq "abcxyz");

		x.prefixer("abc");
		assert(x eq "abcxyz");

		var y = "abc";
		assert(y.prefix("") eq y);
		assert(y.prefixer("") eq "abc");
		assert(y eq "abc");

		var z = "";
		assert(z.prefix("") eq z);
		assert(z.prefixer("") eq "");
		assert(z eq "");

	}

	{
		// test paster insert
		var x;
		x = "abc"; paster(x, -5, "x"); assert(x eq "xabc");
		x = "abc"; paster(x, -4, "x"); assert(x eq "xabc");
		x = "abc"; paster(x, -3, "x"); assert(x eq "xabc");
		x = "abc"; paster(x, -2, "x"); assert(x eq "axbc");
		x = "abc"; paster(x, -1, "x"); assert(x eq "abxc");
		x = "abc"; paster(x,  0, "x"); assert(x eq "xabc");
		x = "abc"; paster(x,  1, "x"); assert(x eq "xabc");
		x = "abc"; paster(x,  2, "x"); assert(x eq "axbc");
		x = "abc"; paster(x,  3, "x"); assert(x eq "abxc");
		x = "abc"; paster(x,  4, "x"); assert(x eq "abcx");
		x = "abc"; paster(x,  5, "x"); assert(x eq "abcx");
	}

	{
		// test paste insert
		var x = "abc";
		assert(paste(x, -5, "x") eq "xabc");
		assert(paste(x, -4, "x") eq "xabc");
		assert(paste(x, -3, "x") eq "xabc");
		assert(paste(x, -2, "x") eq "axbc");
		assert(paste(x, -1, "x") eq "abxc");
		assert(paste(x,  0, "x") eq "xabc");
		assert(paste(x,  1, "x") eq "xabc");
		assert(paste(x,  2, "x") eq "axbc");
		assert(paste(x,  3, "x") eq "abxc");
		assert(paste(x,  4, "x") eq "abcx");
		assert(paste(x,  5, "x") eq "abcx");
	}

	{
		// test paste insert empty
		var x = "abc";
		assert(paste(x, -5, "") eq "abc");
		assert(paste(x, -4, "") eq "abc");
		assert(paste(x, -3, "") eq "abc");
		assert(paste(x, -2, "") eq "abc");
		assert(paste(x, -1, "") eq "abc");
		assert(paste(x,  0, "") eq "abc");
		assert(paste(x,  1, "") eq "abc");
		assert(paste(x,  2, "") eq "abc");
		assert(paste(x,  3, "") eq "abc");
		assert(paste(x,  4, "") eq "abc");
		assert(paste(x,  5, "") eq "abc");
	}

	{
		// test paste insert into empty
		var x = "";
		assert(paste(x, -2, "abc") eq "abc");
		assert(paste(x, -1, "abc") eq "abc");
		assert(paste(x,  0, "abc") eq "abc");
		assert(paste(x,  1, "abc") eq "abc");
		assert(paste(x,  2, "abc") eq "abc");
	}

//	{ try to do without this function since scarcely used
//		var x;
//		x = "abc";
//		assert(pasterall(x, 5, "XYZ") eq "abcXYZ");
//		x = "abc";
//		assert(pasterall(x, 4, "XYZ") eq "abcXYZ");
//		x = "abc";
//		assert(pasterall(x, 3, "XYZ") eq "abXYZ");
//		x = "abc";
//		assert(pasterall(x, 2, "XYZ") eq "aXYZ");
//		x = "abc";
//		assert(pasterall(x, 1, "XYZ") eq "XYZ");
//
//		x = "abc";
//		assert(pasterall(x, 0, "XYZ") eq "XYZ");
//
//		x = "abc";
//		assert(pasterall(x, -1, "XYZ") eq "abXYZ");
//		x = "abc";
//		assert(pasterall(x, -2, "XYZ") eq "aXYZ");
//		x = "abc";
//		assert(pasterall(x, -3, "XYZ") eq "XYZ");
//		x = "abc";
//		assert(pasterall(x, -4, "XYZ") eq "XYZ");
//	}
//
	printl(elapsedtimetext());
	printl("Test passed");
	return 0;
}

programexit()
