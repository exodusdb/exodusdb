#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

func main() {

	printl("test_funcs says 'Hello World!'");

	{
		//test clone unassigned
		var v1;
		var v2 = v1.clone();
		var v3 = clone(v1);
	}
	{
		//test clone assigned
		var v1 = "qqq";
		var v2 = v1.clone();
		assert(v2 == "qqq");
		var v3 = clone(v2);
		assert(v3 == "qqq");
	}
	{
		//test move unassigned fails
		var v1;
		try {var v2 = v1.move();assert(false);} catch (VarUnassigned e) {};
		try {var v2 = move(v1);assert(false);} catch (VarUnassigned e) {};
	}
    {
		// test dump()

		var s1 = str("x", 32);
		TRACE(dump(s1))

		var s2 = s1.isnum();
		TRACE(dump(s1))

		var i1 = 123_var;
		TRACE(dump(i1))

		var d1 = 123.45_var;
		TRACE(dump(d1))

		var of1;
		if (not osopen("/etc/hosts" to of1))
			abort(lasterror());
		TRACE(dump(of1))

		//TRACE: dump(s1) = "var:0x7ffd25f72338 typ:1 str:0x6525ab29be70 "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx""
		//TRACE: dump(s1) = "var:0x7ffd25f72338 typ:9 nan: str:0x6525ab29be70 "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx""
		//TRACE: dump(i1) = "var:0x7ffd25f72270 typ:2 int:123"
		//TRACE: dump(d1) = "var:0x7ffd25f721f8 typ:4 dbl:123.45"
		//TRACE: dump(of1) = "var:0x7ffd25f721c0 typ:25 osfile: int:0 dbl:1 nan: str: "/etc/hosts""

    }

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
		// var.move()

		// .move() to a temporary leaves v1 as empty string guaranteed
		var v1 = space(100);
		auto addr1 = static_cast<const void*>(v1.c_str()); // Where exactly are v1's 100 spaces

		var v2 = v1.move();
//		assert(v1.unassigned());
		assert(v1 == "");
		assert(v2 == space(100));

		// Check that the spaces are still the same spaces
		auto addr2 = static_cast<const void*>(v2.c_str()); // Where exactly are v2's 100 spaces

		// Verify that the spaces themselves have NOT moved
		assert(addr1 == addr2);

	}

	{
		// std::move is DIFFERENT

		// MOVES the string
		// but doesnt mark the source as unassigned
		var v1 = space(100);
		var v2 = std::move(v1);
		assert(! v1.unassigned()); // Still assigned
		assert(v1 == ""); // But var str is gone.
		assert(v2 == space(100));
	}

    {
		// free function
		var v1 = "aa";
		var v2 = "bb";
		v2 = move(v1);
		assert(v1 == "");
		assert(v2 == "aa");
    }

	{
		// Move from unassigned variables is not allowed.
		var v1;
		var v2 = "bb";
		try {
			v2 = move(v1);
			assert(false);
		} catch (VarUnassigned e) {};
		//      assert(v1 == "");
		//      assert(v2 == "aa");
	}

	{
		// swap on unassigned variables
		var v1 = "q";
		var v2;

		v1.swap(v2);
		assert(v1.unassigned());
		assert(v2 == "q");

		v1.swap(v2);
		assert(v2.unassigned());
		assert(v1 == "q");
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

		s1.swap(s2);
		assert(s1         eq "2");
		assert(s2         eq "1");
	}
	{
		var started = ostime().outputl("ostime=");
		ossleep(200);
		var stopped = ostime();
		// avoid testing if time crosses 00:00:00
		if (stopped gt started)
			assert(ostime() - started gt 0.1);
	}
	{
		assert(ospid().outputl("ospid=").match("\\d+"));
	}
	{
		try {
			abort("asd");
			assert(false);
		} catch (ExoAbort& e) {
			printl(e.message);
		}
	}
	{
		var x = "asd";
		assert(len(x)     eq 3);
		assert(textlen(x) eq 3);
		assert(textwidth(x) eq 3);

		x = "Γιάννης";
		assert(len(x)     eq 14);
		assert(textlen(x) eq 7);
		assert(textwidth(x) eq 7);
	}

	{
		// OK - Check double width emoji with "cat face"
		assert(len("\U0001f431"_var.outputl("✅︎ 2 = ")) == 4);
		assert(len("🐱"_var.outputl("✅︎ 2 = ")) == 4);
		assert(textlen("🐱"_var.outputl("✅︎ 2 = ")) == 1);
		assert(textwidth("🐱"_var.outputl("✅︎ 2 = ")) == 2);

		// OK - Any unprintable ASCII control chars count as zero
		assert(textwidth("\u0001"_var.outputl("✅ control chars 0 - 32 should be 0 ")) == 0);

		// OK - Combining e and Grave Accent gives width 1
		assert(textwidth("e\u0300"_var.outputl("✅︎ 1 = ")).outputl() == 1);

		// KO - Check Combining waving han (width 2) AND light skin tone (width 2) should be combine into width 2 of the hand only
		assert(textwidth("\U0001F44B\U0001F3FB"_var.outputl("❌ 4 should be 2 ")) == 4);
		assert(textwidth("👋🏻"_var.outputl("❌ 4 should be 2 ")) == 4);

		// KO - Unicode unprintable ZERO WIDTH SPACE
		assert(textwidth("\u0200"_var.outputl("❌ 1 should 0 ZERO_WIDTH SPACE ")).outputl() == 1);

	}
	{
		//convert complex utf8 characters to utf8 characters
		var yiannis = "Γιάννης";
		textconverter(yiannis, "άης", "12");
		assert(yiannis eq "Γι1νν2");

		// on temporaries
		assert("Γιάννης"_var.textconvert("άης", "12") eq "Γι1νν2");
	}

	{
		//swap byte strings with byte strings
		var yiannis = "Γιάννης";
		replacer(yiannis, "Γιά", "ννης");
		assert(yiannis eq "ννηςννης");
		var str1 = "go to heaven";
		replacer(str1, "o ", "___");
		assert(str1 eq "g___t___heaven");
	}
	{
		// \d \D and [:number:]
		var x = "a1b23c99";
		replacer(x, R"(\d)"_rex, "N");
		assert(x                               eq "aNbNNcNN");

		x = "a1b23c99";
		replacer(x, R"(\D)"_rex, "N");
		assert(x        eq "N1N23N99");
		//if (BOOST_REGEX) {
		x = "a1b23c99";
		replacer(x, "[[:number:]]"_rex, "N");
		assert(x        eq "aNbNNcNN");
		x = "a1b23c99";
		replacer(x, "[^[:number:]]"_rex, "N");
		assert(x        eq "N1N23N99");
		//}
	}
	{
		var g = "Γιάννης";
		assert(ucase(g).outputl("Uppercase " ^ g.quote() ^ " = ") eq "ΓΙΆΝΝΗΣ");
		ucaser(g);
		g.outputl("ucaser=");
		assert(g        eq "ΓΙΆΝΝΗΣ");
	}
	{
		var g = "Γιάννης AbCd";
		assert(lcase(g) eq "γιάννης abcd");
		lcaser(g);
		assert(g.outputl("lcaser=")        eq "γιάννης abcd");
	}
	{
		var g = "γιάννης γιάννης abcd";
		assert(tcase(g) eq "Γιάννης Γιάννης Abcd");
		tcaser(g);
		assert(g.outputl("tcaser=")        eq "Γιάννης Γιάννης Abcd");
	}
	{
		var g = "Γιάννης Γιάννης Abcd";
		assert(fcase(g) eq "γιάννησ γιάννησ abcd");
		fcaser(g);
		assert(g.outputl("fcaser=")        eq "γιάννησ γιάννησ abcd");
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
		assert(quote("").outputl()  eq _DQ _DQ);
		assert(squote("").outputl() eq "''");

		assert(quote(" ")  eq _DQ " " _DQ);
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
		x.cutter(-3);
		x ^= "Q";
		assert(x eq "abcQ");

		x.cutter(-1);
		x ^= "XYZ";
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
		y.prefixer("");
		assert(y eq "abc");

		var z = "";
		assert(z.prefix("") eq z);
		z.prefixer("");
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

	printl("append/appender member and free functions");
	{
		// + - * / % self assign functions do not return a value so cannot be used in expressions or chained.
		var x = "";
		var y = "2";
		x += 1;
		//(x += 1) += "2"; // does not compile

		// ^= is different

		// Chaining appends efficiently
		///////////////////////////////

		// Appending large strings requires execution efficiency to avoid making unnecessary copies of strings and use of the heap.
		var v1, v2;
		v1 = "", v2 = "xxx";

		// Efficient coding as long as vars are being used on the rhs
		// INEFFICINT execution since a temporary (v2 ^ "yyy") is formed first
		v1 = "a", v2 = "xxx";
		v1 ^= v2 ^ "yyy" ^ 3 ^ 'q' ^ 2.3;
		assert(v1.outputl("v1=") == "axxxyyy3q2.3");

		// Compromise sadly doesnt compile because of wrong operator association of ^=.
		//v1 ^= v2 ^= "yyy";
	//	assert(v1.outputl("v1=") == "axxxyyy");

		// INEFFICIENT coding. Both writing and reading
		// Efficient execution
		v1 = "a", v2 = "xxx";
		v1 ^= v2;
		v1 ^= "yyy";
		v1 ^= 3;
		v1 ^= 'q';
		v1 ^= 2.3;
		assert(v1.outputl("v1=") == "axxxyyy3q2.3");

		// INEFFICIENT coding. Both writing and reading
		// Efficient execution.
		v1 = "a", v2 = "xxx";
		((((v1 ^= v2) ^= "yyy") ^= 3) ^= 'q') ^= 2.3;
		assert(v1.outputl("v1=") == "axxxyyy3q2.3");

		// THEREFORE USE multiargument append() or appender()

		// Very fast even with mixed types. var, cstr, char, int and double
		// The only speed issue is that it checks for VNA on every operation.

		// var member functions

		v1 = "a", v2 = "xxx";
		v1 = v1.append(v2, "yyy", 3, 'q', 2.3);
		assert(v1.outputl("v1=") == "axxxyyy3q2.3");

		v1 = "a", v2 = "xxx";
		v1.appender(v2, "yyy", 3, 'q', 2.3);
		assert(v1.outputl("v1=") == "axxxyyy3q2.3");

		// oxodus free functions

		v1 = "a", v2 = "xxx";
		v1 = append(v1, v2, "yyy", 3, 'q', 2.3);
		assert(v1.outputl("v1=") == "axxxyyy3q2.3");

		v1 = "a", v2 = "xxx";
		appender(v1, v2, "yyy", 3, 'q', 2.3);
		assert(v1.outputl("v1=") == "axxxyyy3q2.3");

		// var member functions on temporaries

		v1 = "a", v2 = "xxx";
		v1 = var("qqq").append(v2, "yyy", 3, 'q', 2.3);
		assert(v1.outputl("v1=") == "qqqxxxyyy3q2.3");

		// oxodus free functions on temporaries

		v1 = "a", v2 = "xxx";
		v1 = append("qqq", v2, "yyy", 3, 'q', 2.3);
		assert(v1.outputl("v1=") == "qqqxxxyyy3q2.3");

	}

	printl(elapsedtimetext());
	printl("Test passed");
	return 0;
}

programexit()
