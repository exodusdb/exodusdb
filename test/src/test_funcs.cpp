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
		// Transfer
		var s1 = "1";
		var s2 = "2";
		transfer(s1, s2);
		assert(s1         eq "");
		assert(s2         eq "1");
	}
	{
		// Exchange
		var s1 = "1";
		var s2 = "2";
		exchange(s1, s2);
		assert(s1         eq "2");
		assert(s2         eq "1");
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
		assert(swapper(yiannis, "Γιά", "ννης") eq "ννηςννης");
		var str1 = "go to heaven";
		assert(swapper(str1, "o ", "___")      eq "g___t___heaven");
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
	assert(var("abc").splice(-1, "XY")    eq "abXY");
	assert(var("abc").splice(-2, 1, "XY") eq "aXYc");

	{
		var x = "abc";

		assert(splice(x, 5, "XYZ").outputl() eq "abcXYZ");
		assert(splice(x, 4, "XYZ").outputl() eq "abcXYZ");
		assert(splice(x, 3, "XYZ").outputl() eq "abXYZ");
		assert(splice(x, 2, "XYZ").outputl() eq "aXYZ");
		assert(splice(x, 1, "XYZ").outputl() eq "XYZ");

		assert(splice(x, 0, "XYZ").outputl() eq "XYZ");

		assert(splice(x, -1, "XYZ").outputl() eq "abXYZ");
		assert(splice(x, -2, "XYZ").outputl() eq "aXYZ");
		assert(splice(x, -3, "XYZ").outputl() eq "XYZ");
		assert(splice(x, -4, "XYZ").outputl() eq "XYZ");
	}

	{
		var x;
		x = "abc";
		assert(splicer(x, 5, "XYZ").outputl() eq "abcXYZ");
		x = "abc";
		assert(splicer(x, 4, "XYZ").outputl() eq "abcXYZ");
		x = "abc";
		assert(splicer(x, 3, "XYZ").outputl() eq "abXYZ");
		x = "abc";
		assert(splicer(x, 2, "XYZ").outputl() eq "aXYZ");
		x = "abc";
		assert(splicer(x, 1, "XYZ").outputl() eq "XYZ");

		x = "abc";
		assert(splicer(x, 0, "XYZ").outputl() eq "XYZ");

		x = "abc";
		assert(splicer(x, -1, "XYZ").outputl() eq "abXYZ");
		x = "abc";
		assert(splicer(x, -2, "XYZ").outputl() eq "aXYZ");
		x = "abc";
		assert(splicer(x, -3, "XYZ").outputl() eq "XYZ");
		x = "abc";
		assert(splicer(x, -4, "XYZ").outputl() eq "XYZ");
	}

	printl(elapsedtimetext());
	printl("Test passed");
	return 0;
}

programexit()
