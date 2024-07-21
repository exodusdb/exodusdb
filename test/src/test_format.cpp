//import std;
#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

// exodus format/println functions
// Require this additional header BEFORE any other exodus_header.
// Slows down compilation a bit so not included in standard exodus headers
#include <exodus/format.h>

#include <exodus/program.h>

programinit()

function main() {

#ifndef EXO_FORMAT
	printl("EXO_FORMAT is not available so std::format a var is not possible.");
#else

#ifndef EXO_MODULE_INITIAL_BUILD
	printl(std::format("EXO_FORMAT is defined so formatting a var using std::format is possible. e.g. {}", "'hello'"));
#endif

	var x = 12.3456;
	assert(x.format("{:.2f}").outputl() == "12.35");
	{
		var fmt = "{:.2f}";
		assert(xvformat(fmt, x).outputl() == "12.35");
		assert(xvformat(fmt, 12.3456).outputl() == "12.35");
		assert(x.vformat(fmt).outputl() == "12.35");
	}
	{
		var fmt = "{:.2f} and {:.3}";
		assert(xvformat(fmt, 12.3456, 1.23456).outputl() == "12.35 and 1.23");
		assert(x.vformat(fmt, 1.23456).outputl() == "12.35 and 1.23");
	}

	// format will work on consteval format strings
	printl(xformat("abc {} ghi", "def"));
	printl(xformat("abc {} ghi", "def"));

	// need vformat if fmt is not consteval
	var v1 = "iii {} kkk";
	std::string s1 = "xxx {} zzz";
	auto c1 = "xxx {} zzz";

	// Will not compile. need vformat.
//	assert(xformat(v1, "jjj") == "iii jjj kkk");
//	assert(xformat(s1, "yyy") == "xxx yyy zzz");
//	assert(xformat(c1, "yyy") == "xxx yyy zzz");
	assert(xvformat(v1, "jjj") == "iii jjj kkk");
	assert(xvformat(s1, "yyy") == "xxx yyy zzz");
	assert(xvformat(c1, "yyy") == "xxx yyy zzz");
	assert(var("jjj").vformat(v1) == "iii jjj kkk");
	assert(var("yyy").vformat(s1) == "xxx yyy zzz");
	assert(var("yyy").vformat(c1) == "xxx yyy zzz");
	// vprint is required for variable format strings
	// Whill not compile. vprint/vprintln is required
//	print(v1, "yyy");
//	print(s1, "yyy");
//	println(v1, "yyy");
//	println(s1, "yyy");
	vprint(v1, "yyy");
	vprint(s1, "yyy");
	vprintln(v1, "yyy");
	vprintln(s1, "yyy");

	printl(fmt::format("abc {} ghi", "def"));

	{
//		C++ format code "f" rounding has a DIFFERENT rounding rule compared to exodus format code "MD"
//
//		"f" fmt code rounds 0.5 towards zero on even numbers like 0.5 2.5 4.5 e.g. 0.5 -> 0
//		and away from zero on 1.5 3.5 5.5 etc. e.g. 1.5 -> 2

//		WRONG
		assert(xformat("{:.0f}x", 3.5) == "4x");
		assert(xformat("{:.0f}x", 2.5) == "2x");   // WRONG
		assert(xformat("{:.0f}x", 1.5) == "2x");
		assert(xformat("{:.0f}x", 0.5) == "0x");   // WRONG
		assert(xformat("{:.0f}x", 0.0) == "0x");
		assert(xformat("{:.0f}x", -0.0) == "-0x"); // WRONG
		assert(xformat("{:.0f}x", -0.5) == "-0x"); // WRONG
		assert(xformat("{:.0f}x", -1.5) == "-2x");
		assert(xformat("{:.0f}x", -2.5) == "-2x"); // WRONG
		assert(xformat("{:.0f}x", -3.5) == "-4x");
	}
	{
		// *** Note *** EXODUS format codes can ONLY be used on parameters that are exodus var type

		// CORRECT
		//0.5 -0.5 round to 1 and -1 respectively
		//1.5 -1.5 round to 2 and -2 respectively
		assert(xformat("{::MD00}x", var(3.5)) == "4x");
		assert(xformat("{::MD00}x", var(2.5)) == "3x");
		assert(xformat("{::MD00}x", var(1.5)) == "2x");
		assert(xformat("{::MD00}x", var(0.5)) == "1x");
		assert(xformat("{::MD00}x", var(0.0)) == "0x");
		assert(xformat("{::MD00}x", var(-0.0)) == "0x");
		assert(xformat("{::MD00}x", var(-0.5)) == "-1x");
		assert(xformat("{::MD00}x", var(-1.5)) == "-2x");
		assert(xformat("{::MD00}x", var(-2.5)) == "-3x");
		assert(xformat("{::MD00}x", var(-3.5)) == "-4x");
	}

	{
		// Various exodus conversions using the {::XXXXXX} pattern

		// Multivalued formats allow padding
		//std::string s1 = "{::MD2PR#12}"_var;
		//assert(xformat(s1, var(123.456)).outputl() == "      123.46");
		var v1 = "{::MD2P|R#12}";
		var s1 = "{::MD2P|R#12}";
		//var v1 = "{::MD2P]R#12}"_var;
		//TRACE(xformat(v1, var(123.456)));

		// format ... v1/s1 etc. DOES NOT COMPILE
		//assert(xformat(v1, var(123.456)).outputl() == "      123.46");
		//assert(xformat(s1, var(123.456)).outputl() == "      123.46");

		// vformat ... v1/s1 DOES COMPILE
		assert(xvformat(v1, var(123.456)).outputl() == "      123.46");
		assert(xvformat(s1, var(123.456)).outputl() == "      123.46");

		assert(xformat("{::MD2P|R#12}", var(123.456)).outputl() == "      123.46");
		//assert(xformat("{::MD2P]R#12}"_var, var(123456)).outputl() == "   123456.00");

		assert(xformat("{::MD20}", var(123.456)).outputl() == "123.46");
		assert(xformat("{::MD20}", var(123456)).outputl() == "123456.00");
		// P has no effect if both number of decimals (2) and number to move (0) are specified
		assert(xformat("{::MD20P}", var(123.456)).outputl() == "123.46");
		assert(xformat("{::MD20P}", var(123456)).outputl() == "123456.00");

		// P = Preserve decimal point position

		// A single 2 means MOVE the decimal place left by two AND show 2 decimal places
		assert(xformat("{::MD2}", var(123.456)).outputl() == "1.23");
		assert(xformat("{::MD2}", var(123456)).outputl() == "1234.56");

		// P means "if a decimal point is found then do not move the decimal point"
		// otherwise a single 2 means MOVE the decimal point left by 2 AND use use two decimal places
		assert(xformat("{::MD2P}", var(123.456)).outputl() == "123.46");
		assert(xformat("{::MD2P}", var(123456)).outputl() == "123456.00");

		assert(xformat("{::MTS}", var(60*60*12 + 1)) == "12:00:01");

		assert(xformat("{::D4/E}", var(20000)) == "03/10/2022");
	}

	assert(xformat("{:9.1f}", 123.456) == "    123.5");
	assert(xformat("{:9.1f}", -123.456) == "   -123.5");
//	assert(std::format("{:9.1f}", var(123.456)) == "    123.5");
	assert(xformat("{:9.1f}", var(123.456)) == "    123.5");
	assert(xformat("{:9.1f}", var(-123.456)) == "   -123.5");
	assert(xformat("{:9d}", var(1234.56)) == "     1234");
	assert(xformat("{:9d}", var(-1234.56)) == "    -1234");
	//assert(xformat("{0}", var("\u03a3")) == "Σ");
	//assert(xformat("{0}", var("Σ")) == "Σ");
//stop();

	assert(xformat("{}",1000.0001) == "1000.0001");
	assert(xformat("{}",1000.00001) == "1000.00001");
	assert(xformat("{}",1000.000001) == "1000.000001");
	assert(xformat("{}",1000.0000001) == "1000.0000001");
	assert(xformat("{}",1000.00000001) == "1000.00000001");
	assert(xformat("{}",1000.000000001) == "1000.000000001");
	assert(xformat("{}",1000.0000000001) == "1000.0000000001");
	assert(xformat("{}",1000.00000000001) == "1000.00000000001");
	assert(xformat("{}",1000.000000000001) == "1000.000000000001");
	assert(xformat("{}",1000.0000000000001) == "1000.0000000000001");
	assert(xformat("{}",1000.00000000000001) == "1000");
	assert(xformat("{}",1000.000000000000001) == "1000");
	assert(xformat("{}",1000.0000000000000001) == "1000");

	assert(xformat("{}",var(1000.0001)) == "1000.0001");
	assert(xformat("{}",var(1000.00001)) == "1000.00001");
	assert(xformat("{}",var(1000.000001)) == "1000.000001");
	assert(xformat("{}",var(1000.0000001)) == "1000.0000001");
	assert(xformat("{}",var(1000.00000001)) == "1000.00000001");
	assert(xformat("{}",var(1000.000000001)) == "1000.000000001");
	assert(xformat("{}",var(1000.0000000001)) == "1000.0000000001");
	assert(xformat("{}",var(1000.00000000001)) == "1000.00000000001");
	assert(xformat("{}",var(1000.000000000001)) == "1000.000000000001");
	assert(xformat("{}",var(1000.0000000000001)) == "1000");
	assert(xformat("{}",var(1000.00000000000001)) == "1000");
	assert(xformat("{}",var(1000.000000000000001)) == "1000");
	assert(xformat("{}",var(1000.0000000000000001)) == "1000");

	assert(xformat("{:b}", var(256)) == "100000000");
	assert(xformat("{:B}", var(256)) == "100000000");
	assert(xformat("{:o}", var(256)) == "400");
	assert(xformat("{:x}", var(256)) == "100");
	assert(xformat("{:X}", var(256)) == "100");

	// CAT FACE
	// Unicode character 0x1F431
	// four bytes of utf-8
	// double width character on output
	assert(var("🐱").oconv("HEX").outputl("catface= hex ") == "F09F90B1");
	assert(var("🐱") == "\xF0\x9F\x90\xB1");
	assert(var("🐱") == "\U0001F431");

//g++ 11 in Ubuntu 22.04 has format but a bug in width of unicode characters
//#if EXO_FORMAT == 1
#if EXO_FORMAT == 1 and __GNUG__ == 13
#warning EXO_FORMAT == 1 and _GNUG__ == 13 has error in formatting width of multibyte utf-8
	// Incorrect in g++13.2
	// Width is calculated as bytes not utf-8 character widths
	// Cat = four bytes so takes up "4" positions in formatting
	assert(xformat("{:.^5s}", var("🐱")) == "🐱.");

#else
#warning Note: We are using fmtlib library instead of std::format for now since it appears more consistent with double width unicode chars
	//__clang_major__ 18
	// Correct in fmt library
	assert(xformat("{:.^5s}", "🐱") == ".🐱..");
//	assert(xformat("{:.5s}", "🐱🐱🐱") == "🐱🐱🐱"); //libfmt8 fails here TODO DONT SKIP IF NOT libfmt8
//	assert(xformat("{:.<5.5s}", "🐱🐱🐱") == "🐱🐱🐱"); //and here

	assert(xformat("{:^5s}", "🐱") == " 🐱  ");
//	assert(xformat("{:.5s}", "🐱🐱🐱") == "🐱🐱🐱"); //libfmt8 fails
//	assert(xformat("{:<5.5s}", "🐱🐱🐱") == "🐱🐱🐱"); //and here
#endif

	var pi = 3.14f;
	assert(xformat("{:10f}", pi) == "  3.140000");
	assert(xformat("{:.5f}", pi) == "3.14000");
	assert(xformat("{:10.5f}", pi) == "   3.14000");

	assert(xformat("a {} b {} c", 100, 200) == "a 100 b 200 c");
	assert(xformat("a {0} b {1} c", var("x"), var("y")) == "a x b y c");
	TRACE(xformat("{:9.1f}", 123.456))
	TRACE(xformat("{:9.1f}", var(123.456)))
	TRACE(xformat("{}", var(123.456)))
	TRACE(xformat("{}", var(12345)))
	TRACE(xformat("{}", var("abcdefg")))

//#endif

	{
		// Not format but multivalued oconv is useful in format

		assert("20000]]21000"_var.oconv("DY-4]R#12"_var) == "  2022-10-03]            ]  2025-06-29"_var);
		assert("20000]]21000"_var.oconv("DY-4]L#12"_var) == "2022-10-03  ]            ]2025-06-29  "_var);
		assert("20000]]21000"_var.oconv("DY-4]C#12"_var) == " 2022-10-03 ]            ] 2025-06-29 "_var);

		//TRACE("jsdgh cjhgs djhg sdjcg jsdhg cjsgdcjgsdjcgh jhg sjd ghc sdkjch kshj kj hskdjh cskjd hckjhkj kshdckhj ksdh "_var.oconv("T#12"))

		assert("jsdgh cjhgs djhg sdjcg jsdhg cjsgdcjgsdjcgh jhg sjd ghc sdkjch kshj kj hskdjh cskjd hckjhkj kshdckhj ksdh "_var.oconv("T#12") == "jsdgh cjhgs |djhg sdjcg  |jsdhg       |cjsgdcjgsdjc|gh jhg sjd  |ghc sdkjch  |kshj kj     |hskdjh cskjd|hckjhkj     |kshdckhj    |ksdh        "_var);

		assert("1234.56789"_var.oconv("MD20]R#10"_var) == "   1234.57");

		// subvalues
		assert("1234.56789}2345.6789"_var.oconv("MD20]R#10"_var) == "   1234.57}   2345.68"_var);
		//multivalues
		assert("1234.56789]2345.6789"_var.oconv("MD20]R#10"_var) == "   1234.57]   2345.68"_var);
		//fields
		assert("1234.56789^2345.6789"_var.oconv("MD20]R#10"_var) == "   1234.57^   2345.68"_var);

		//empty fields: sole, inner, leading and trailing
		assert(""_var.oconv("MD20]R#10"_var) == "          "_var);
		assert("1234.56789^^2345.6789"_var.oconv("MD20]R#10"_var) == "   1234.57^          ^   2345.68"_var);
		assert("^1234.56789^2345.6789"_var.oconv("MD20]R#10"_var) == "          ^   1234.57^   2345.68"_var);
		assert("1234.56789^2345.6789^"_var.oconv("MD20]R#10"_var) == "   1234.57^   2345.68^          "_var);

		assert("1234.56789"_var.oconv("MD20"_var) == "1234.57");
		assert("1234.56789"_var.oconv("MD20]MD10"_var) == "1234.6");
		assert("1234.56789]2345.6789"_var.oconv("MD20"_var) == "1234.57]2345.68"_var);
		assert("1234.56789]2345.6789"_var.oconv("MD20]MD10"_var) == "1234.6]2345.7"_var);

	}

	{	// Dynamic width possible using vars only with standard c++ format specifiers at the moment
		// must convert to string - see next block

		// https://en.cppreference.com/w/cpp/utility/format/spec
		assert(xformat("{:.^5s}",   "🐱").squote().outputl()     == "'.🐱..'");

		assert(xformat("{:.^5s}",   var("🐱"), 5).squote().outputl()     == "'.🐱..'");

		// fmtlib v10 fails following
		//assert(xformat("{:.5s}",    "🐱🐱🐱").squote().outputl() == "'🐱🐱'");
		//assert(xformat("{:.<5.5s}", "🐱🐱🐱").squote().outputl() == "'🐱🐱.'");

		// Not using dynamic arguments
		//assert(xformat(FMT_COMPILE("{:4}"), var(9)).squote().outputl() == "'9   '");
		assert(xformat("{:4}", var(9), 7, 8, 9).squote().outputl() == "'9   '");
		assert(xformat("{:}", var(9)).squote().outputl() == "'9'");
		assert(xformat("{}", var(9), 7, 8, 9).squote().outputl() == "'9'");

		// terminate called after throwing an instance of 'fmt::v10::format_error'
		//  what():  missing '}' in format string

		assert(xformat("{:{}}", var("x"), 7).squote().outputl() == "'x      '");

		// argument not found
		//assert(xformat("{:{}}", var(9), 4).squote().outputl() == "'9   '");
		//assert(xformat("{:{}}", var(9), 4).squote().outputl() == "'9   '");
		//assert(xformat("{:{}s}", var(9), 4).squote().outputl() == "'9   '");
		//assert(xformat("{:0<.{}}", var(9), 4).squote().outputl() == "'9   '");
		//assert(xformat("{:<{}}", var(9), 4).squote().outputl() == "'9   '");
		//assert(xformat("{0:<{}}", var(9), 4).squote().outputl() == "'9   '");

		// cannot switch from automatic to manual argument indexing
		//assert(xformat("{:<{1}}", var(9), 4).squote().outputl() == "'9   '");
		//assert(xformat("{0:<{1}}", var(9), 4).squote().outputl() == "'9   '");

	}

	{
		// oconv(9, "L#4");
		assert(xformat("{:{}s}", var(9).toString(), 4).squote().outputl() == "'9   '");
		assert(xformat("{:{}}", var(9).toString(), 4).squote().outputl() == "'9   '");
		assert(xformat("{:<{}}", var(9).toString(), 4).squote().outputl() == "'9   '");

		// oconv(9, "R#4");
		assert(xformat("{:>{}}", var(9).toString(), 4).squote().outputl() == "'   9'");

		// oconv(9, "C#4");
		assert(xformat("{:^{}}", var(9).toString(), 4).squote().outputl() == "' 9  '");

	}

	{
		// oconv(9, "L(0)#4");
		assert(xformat("{:0<{}}", var(9).toString(), 4).squote().outputl() == "'9000'");

		// oconv(9, "R(0)#4");
		assert(xformat("{:0>{}}", var(9).toString(), 4).squote().outputl() == "'0009'");

		// oconv(9, "C(0)#4");
		assert(xformat("{:0^{}}", var(9).toString(), 4).squote().outputl() == "'0900'");

	}

	{
		// Using d for integer output
		/////////////////////////////

		// oconv(9, "R(0)#4");
		assert(xformat("{:04d}", var(9)).squote().outputl() == "'0009'");
		assert(xformat("{:0>4d}", var(9)).squote().outputl() == "'0009'");

		// oconv(9, "L(0)4")
		assert(xformat("{:0<4d}", var(9)).squote().outputl() == "'9000'");

		// oconv(9, "C(0)4")
		assert(xformat("{:0^4d}", var(9)).squote().outputl() == "'0900'");

		// oconv(9, "R#4")
		assert(xformat("{:>4d}", var(9)).squote().outputl() == "'   9'");
//		assert(xformat("{:>04d}", var(9)).squote().outputl() == "'   9'");// 0 in the wrong place so ignored
		assert(xformat("{:>04d}", var(9)).squote().outputl() == "'0009'");// libfmt9

		// oconv(9, "L#4")
		assert(xformat("{:<4d}", var(9)).squote().outputl() == "'9   '");
//		assert(xformat("{:<04d}", var(9)).squote().outputl() == "'9   '"); // ditto
		assert(xformat("{:<04d}", var(9)).squote().outputl() == "'9000'"); // libfmt9

		// oconv(9, "C#4")
		assert(xformat("{:^4d}", var(9)).squote().outputl() == "' 9  '");
//		assert(xformat("{:^04d}", var(9)).squote().outputl() == "' 9  '"); // ditto
		assert(xformat("{:^04d}", var(9)).squote().outputl() == "'0900'"); // libfmt9
	}

	{
		var pi = 3.1415;
		assert(xformat("{:10f}", pi).squote().outputl() == "'  3.141500'");

		xformat("{:03}",7).squote().outputl(":03 7 = ");//007
		xformat("{:0{}}",7,4).squote().outputl(":0{} 7 = ");//0007

		// f = floating point after rounding
		assert(xformat("{:06f}",var(7.9)).squote().outputl() == "'7.900000'");
		assert(xformat("{:07f}",var(7.9)).squote().outputl() == "'7.900000'");
		assert(xformat("{:01f}",var(7.9)).squote().outputl() == "'7.900000'"); // why the same? leading zero does nothing?

		assert(xformat("{:.1f}",var(7.55)).squote().outputl() == "'7.5'");   // wrong!
		assert(xformat("{:.1f}",var(-7.55)).squote().outputl() == "'-7.5'"); // wrong!

		assert(xformat("{:.1f}",var(6.55)).squote().outputl() == "'6.5'");   // wrong!
		assert(xformat("{:.1f}",var(-6.55)).squote().outputl() == "'-6.5'"); // wrong!

		assert(xformat("{:.1f}",var(6.59)).squote().outputl() == "'6.6'");   // OK
		assert(xformat("{:.1f}",var(-6.59)).squote().outputl() == "'-6.6'"); // OK

		assert(xformat("{:.1f}",var(6.55)).squote().outputl() == "'6.5'");   // wrong!
		assert(xformat("{:.1f}",var(-6.55)).squote().outputl() == "'-6.5'"); // wrong!

		assert(xformat("{:06d}",var(8.9)).squote().outputl() == "'000008'");
		assert(xformat("{:010.6f}",var(7.9)).squote().outputl() == "'007.900000'");

		// d = digits for integer AFTER truncating
		assert(xformat("{:6d}",var(7.5)).squote().outputl() == "'     7'");
		assert(xformat("{:6d}",var(7.9)).squote().outputl() == "'     7'");
		assert(xformat("{:6d}",var(8.5)).squote().outputl() == "'     8'");
		assert(xformat("{:6d}",var(-7.5)).squote().outputl() == "'    -7'");
		assert(xformat("{:6d}",var(-8.5)).squote().outputl() == "'    -8'");
		assert(xformat("{:6d}",var(-8.9)).squote().outputl() == "'    -8'");

		//format("{::0{}}",var(7),4).squote().outputl(":0{} 7 = ");  //      7

		std::cout << fmt::format("{:03}",7) << " :03 7 = " << std::endl;

//		// OK - Check double width emoji
//		assert("🐱"_var.oconv("L(.)#5").outputl() == "🐱...");
//		assert("🐱"_var.oconv("L(.)#1").outputl() == "\xf0"); // WRONG!
//
//		assert("🐱"_var.oconv("R(.)#5").outputl() == "...🐱");
//		assert("🐱"_var.oconv("R(.)#1").outputl() == "\xb1"); // WRONG!
//
//		assert("🐱"_var.oconv("C(.)#5").outputl() == ".🐱..");
//		assert("🐱"_var.oconv("C(.)#1").outputl() == "\xf0"); // WRONG!

		// OK - Check double width emoji DO NOT get formatted properly by L# R# C#
		assert("🐱"_var.oconv("L(.)#5").outputl() == "🐱.");  // WRONG
		assert("🐱"_var.oconv("L(.)#1").outputl() == "\xf0"); // WRONG

		assert("🐱"_var.oconv("R(.)#5").outputl() == ".🐱");
		assert("🐱"_var.oconv("R(.)#1").outputl() == "\xb1");

		assert("🐱"_var.oconv("C(.)#5").outputl() == "🐱.");
		assert("🐱"_var.oconv("C(.)#1").outputl() == "\xf0"); // WRONG!

	}

	{
		// Check exo::textwidth which uses OS wcswidth function except that control characters count as zero

		// Implementation of os wcswidth for terminal output of wide characters
		// https://mitchellh.com/writing/grapheme-clusters-in-terminals
		// list of terminal program support. Need switch on/off for programming?

		// GRAPHEME CLUSTER EXAMPLE
		// 1. U+1F9D1 🧑(Adult face) (2 cells)
		// 2. U+200D (ZWJ) Zero Width Joiner (Zero cells)
		// 3. U+1F33E 🌾 (Ear of Rice) (2 cells)
		//
		// Should show as a farmer with 2 cells wide
		// But on Linux Gnome Terminal. shows as 5 cells wide
		//
		//var farmer = "🧑‍🌾"; Not showing face on Ubuntu 20.04
		var farmer = "\U0001F9D1\u0200\U0001F33E";
		printl("Grapheme clustering not supported on Linux gnome terminal 20.04 22.04 24.04?");
		var farmerwidth = farmer.output("Farmer Grapheme Cluster = ").textwidth().outputl(" textwidth = ");
		// Ubuntu 20.04 = 5, Ubuntu 22.04 and 24.04 = 4
		assert(farmerwidth == 4 or farmerwidth == 5);

		// OK - Combining e and Grave Accent gives width 1
		assert(textwidth("e\u0300"_var.outputl("✅︎ 1 = ")) == 1);

		// KO - Check Combining waving han (width 2) AND light skin tone (width 2) should be combine into width 2 of the hand only
		assert(textwidth("\U0001F44B\U0001F3FB"_var.outputl("❌ 4 should be 2 ")) == 4);
		assert(textwidth("👋🏻"_var.outputl("❌ 4 should be 2 ")) == 4);

		// OK - Any unprintable count as zero
		var v1 = "abc" "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f";
		//v1.squote().outputl();
		assert(textwidth(v1).outputl("✅ control chars 0 - 32 should be 0") == 3);
	}

	{
		var v1 {"abc"};
		var v2 = 12.345;
		var v10 = 10;

		assert(xformat("{:.>{}.{}f}", v2, 10, 5).squote().outputl() == "'..12.34500'");

		assert(xformat("'{:13f}'", v2).outputl() == "'    12.345000'");

		// dynamic pad width and precision on standard format specifiers like f (fixed decimals)
		assert(xformat("'{:{}.{}f}'", v2, 12, 2).outputl() == "'       12.35'");
		assert(xformat("{:.>{}.{}}", v1, 10, 5).outputl() == ".......abc");
		assert(xformat("'{:{}b}'", v10, 12, 2).outputl() == "'        1010'");
		assert(xformat("'{:0f}'", v2).outputl() == "'12.345000'");
		assert(xformat("'{::HEX}'", v10).outputl() == "'3130'");
		assert(xformat("a {} b {} c", 100, 200).outputl() == "a 100 b 200 c");

	}
#endif // EXO_FORMAT

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}
programexit()
