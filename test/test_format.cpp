#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

// Ignore a warning from gcc. TODO proper solution?
#if __GNUC__ == 14
///usr/include/c++/14/bits/stl_algobase.h:452:30: warning:
// ‘void* __builtin_memmove(void*, const void*, long unsigned int)’ writing between 5 and 922
//3372036854775807 bytes into a region of size 4 overflows the destination [-Wstringop-overflow=]
//  452 |             __builtin_memmove(__result, __first, sizeof(_Tp) * _Num);
//      |             ~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#	pragma GCC diagnostic ignored "-Wstringop-overflow="
#endif

// exodus format/println functions
// Require this additional header BEFORE any other exodus_header.
// Slows down compilation a bit so not included in standard exodus headers
//#include <exodus/format.h>

#include <exodus/program.h>
programinit()

func main() {
//#undef EXO_FORMAT
#ifndef EXO_FORMAT
	printl("EXO_FORMAT is not available so std::format a var is not possible.");
#else
//#ifndef EXO_MODULE_INITIAL_BUILD
//	printl(std::format("EXO_FORMAT is defined so formatting a var using std::format is possible. e.g. {}", "'hello'"));
//#endif

	var x = 12.3456;
	assert(x.format("{:.2f}").outputl() == "12.35");
	{
		var fmt = "{:.2f}";
		assert(format(fmt, x).outputl() == "12.35");
		assert(format(fmt, 12.3456).outputl() == "12.35");
		assert(x.format(fmt).outputl() == "12.35");
	}
	{
		var fmt = "{:.2f} and {:.3}";
		assert(format(fmt, 12.3456, 1.23456).outputl() == "12.35 and 1.23");
// wont compile
//		assert(x.format(fmt, 1.23456).outputl() == "12.35 and 1.23");
	}

	// format will work on consteval format strings
	printl(format("abc {} ghi", "def"));
	printl(format("abc {} ghi", "def"));

	// need format if fmt is not consteval
	var v1 = "iii {} kkk";
	std::string s1 = "xxx {} zzz";
	auto c1 = "xxx {} zzz";

	// Will not compile. need format.
//	assert(format(v1, "jjj") == "iii jjj kkk");
//	assert(format(s1, "yyy") == "xxx yyy zzz");
//	assert(format(c1, "yyy") == "xxx yyy zzz");
	assert(format(v1, "jjj") == "iii jjj kkk");

// std::string doesnt work on g++14
//	if (format(s1, "yyy")) {}
//	assert(format(s1, "yyy") == "xxx yyy zzz");

	assert(format(c1, "yyy") == "xxx yyy zzz");
	assert(var("jjj").format(v1) == "iii jjj kkk");
	assert(var("yyy").format(s1) == "xxx yyy zzz");
	assert(var("yyy").format(c1) == "xxx yyy zzz");
	// print is required for variable format strings
	// Whill not compile. print/println is required
//	print(v1, "yyy");
//	print(s1, "yyy");
//	println(v1, "yyy");
//	println(s1, "yyy");
	print(v1, "yyy");
	print(s1, "yyy");
	println(v1, "yyy");
	println(s1, "yyy");

//	printl(fmt::format("abc {} ghi", "def"));

	{
//		C++ format code "f" rounding has a DIFFERENT rounding rule compared to exodus format code "MD"
//
//		"f" fmt code rounds 0.5 towards zero on even numbers like 0.5 2.5 4.5 e.g. 0.5 -> 0
//		and away from zero on 1.5 3.5 5.5 etc. e.g. 1.5 -> 2

//		WRONG
		assert(format("{:.0f}x", 3.5) == "4x");
		assert(format("{:.0f}x", 2.5) == "2x");   // WRONG
		assert(format("{:.0f}x", 1.5) == "2x");
		assert(format("{:.0f}x", 0.5) == "0x");   // WRONG
		assert(format("{:.0f}x", 0.0) == "0x");
		assert(format("{:.0f}x", -0.0) == "-0x"); // WRONG
		assert(format("{:.0f}x", -0.5) == "-0x"); // WRONG
		assert(format("{:.0f}x", -1.5) == "-2x");
		assert(format("{:.0f}x", -2.5) == "-2x"); // WRONG
		assert(format("{:.0f}x", -3.5) == "-4x");
	}
	{
		// *** Note *** EXODUS format codes can ONLY be used on parameters that are exodus var type

		// CORRECT
		//0.5 -0.5 round to 1 and -1 respectively
		//1.5 -1.5 round to 2 and -2 respectively
		assert(format("{::MD00}x", var(3.5)) == "4x");
		assert(format("{::MD00}x", var(2.5)) == "3x");
		assert(format("{::MD00}x", var(1.5)) == "2x");
		assert(format("{::MD00}x", var(0.5)) == "1x");
		assert(format("{::MD00}x", var(0.0)) == "0x");
		assert(format("{::MD00}x", var(-0.0)) == "0x");
		assert(format("{::MD00}x", var(-0.5)) == "-1x");
		assert(format("{::MD00}x", var(-1.5)) == "-2x");
		assert(format("{::MD00}x", var(-2.5)) == "-3x");
		assert(format("{::MD00}x", var(-3.5)) == "-4x");
	}

	{
		// Various exodus conversions using the {::XXXXXX} pattern

		// Multivalued formats allow padding
		//std::string s1 = "{::MD2PR#12}"_var;
		//assert(format(s1, var(123.456)).outputl() == "      123.46");
		var v1 = "{::MD2P|R#12}";
		var s1 = "{::MD2P|R#12}";
		//var v1 = "{::MD2P]R#12}"_var;
		//TRACE(format(v1, var(123.456)));

		// format ... v1/s1 etc. DOES NOT COMPILE
		//assert(format(v1, var(123.456)).outputl() == "      123.46");
		//assert(format(s1, var(123.456)).outputl() == "      123.46");

		// format ... v1/s1 DOES COMPILE
		assert(format(v1, var(123.456)).outputl() == "      123.46");
		assert(format(s1, var(123.456)).outputl() == "      123.46");

		assert(format("{::MD2P|R#12}", var(123.456)).outputl() == "      123.46");
		//assert(format("{::MD2P]R#12}"_var, var(123456)).outputl() == "   123456.00");

		assert(format("{::MD20}", var(123.456)).outputl() == "123.46");
		assert(format("{::MD20}", var(123456)).outputl() == "123456.00");
		// P has no effect if both number of decimals (2) and number to move (0) are specified
		assert(format("{::MD20P}", var(123.456)).outputl() == "123.46");
		assert(format("{::MD20P}", var(123456)).outputl() == "123456.00");

		// P = Preserve decimal point position

		// A single 2 means MOVE the decimal place left by two AND show 2 decimal places
		assert(format("{::MD2}", var(123.456)).outputl() == "1.23");
		assert(format("{::MD2}", var(123456)).outputl() == "1234.56");

		// P means "if a decimal point is found then do not move the decimal point"
		// otherwise a single 2 means MOVE the decimal point left by 2 AND use use two decimal places
		assert(format("{::MD2P}", var(123.456)).outputl() == "123.46");
		assert(format("{::MD2P}", var(123456)).outputl() == "123456.00");

		assert(format("{::MTS}", var(60*60*12 + 1)) == "12:00:01");

		assert(format("{::D4/E}", var(20000)) == "03/10/2022");
	}

	assert(format("{:9.1f}", 123.456) == "    123.5");
	assert(format("{:9.1f}", -123.456) == "   -123.5");
//	assert(std::format("{:9.1f}", var(123.456)) == "    123.5");
	assert(format("{:9.1f}", var(123.456)) == "    123.5");
	assert(format("{:9.1f}", var(-123.456)) == "   -123.5");
	assert(format("{:9d}", var(1234.56)) == "     1234");
	assert(format("{:9d}", var(-1234.56)) == "    -1234");
	//assert(format("{0}", var("\u03a3")) == "Σ");
	//assert(format("{0}", var("Σ")) == "Σ");
//stop();

	assert(format("{}",1000.0001) == "1000.0001");
	assert(format("{}",1000.00001) == "1000.00001");
	assert(format("{}",1000.000001) == "1000.000001");
	assert(format("{}",1000.0000001) == "1000.0000001");
	assert(format("{}",1000.00000001) == "1000.00000001");
	assert(format("{}",1000.000000001) == "1000.000000001");
	assert(format("{}",1000.0000000001) == "1000.0000000001");
	assert(format("{}",1000.00000000001) == "1000.00000000001");
	assert(format("{}",1000.000000000001) == "1000.000000000001");
	assert(format("{}",1000.0000000000001) == "1000.0000000000001");
	assert(format("{}",1000.00000000000001) == "1000");
	assert(format("{}",1000.000000000000001) == "1000");
	assert(format("{}",1000.0000000000000001) == "1000");

	assert(format("{}",var(1000.0001)) == "1000.0001");
	assert(format("{}",var(1000.00001)) == "1000.00001");
	assert(format("{}",var(1000.000001)) == "1000.000001");
	assert(format("{}",var(1000.0000001)) == "1000.0000001");
	assert(format("{}",var(1000.00000001)) == "1000.00000001");
	assert(format("{}",var(1000.000000001)) == "1000.000000001");
	assert(format("{}",var(1000.0000000001)) == "1000.0000000001");
	assert(format("{}",var(1000.00000000001)) == "1000.00000000001");
	assert(format("{}",var(1000.000000000001)) == "1000.000000000001");
	assert(format("{}",var(1000.0000000000001)) == "1000");
	assert(format("{}",var(1000.00000000000001)) == "1000");
	assert(format("{}",var(1000.000000000000001)) == "1000");
	assert(format("{}",var(1000.0000000000000001)) == "1000");

	assert(format("{:b}", var(256)) == "100000000");
	assert(format("{:B}", var(256)) == "100000000");
	assert(format("{:o}", var(256)) == "400");
	assert(format("{:x}", var(256)) == "100");
	assert(format("{:X}", var(256)) == "100");

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
	assert(format("{:.^5s}", var("🐱")) == "🐱.");

#else
#warning Note: We are using fmtlib library instead of std::format for now since it appears more consistent with double width unicode chars
	//__clang_major__ 18
	// Correct in fmt library
	assert(format("{:.^5s}", "🐱") == ".🐱..");

	assert(format("{:^5s}", "🐱") == " 🐱  ");

#if FMT_VERSION == 0 || FMT_VERSION >= 90000
	assert(format("{:.5s}", "🐱🐱🐱") == "🐱🐱🐱"); //libfmt8 fails here TODO DONT SKIP IF NOT libfmt8
	assert(format("{:.<5.5s}", "🐱🐱🐱") == "🐱🐱🐱"); //and here

	assert(format("{:.5s}", "🐱🐱🐱") == "🐱🐱🐱"); //libfmt8 fails
	assert(format("{:<5.5s}", "🐱🐱🐱") == "🐱🐱🐱"); //and here
#endif

#endif

	var pi = 3.14f;
	assert(format("{:10f}", pi) == "  3.140000");
	assert(format("{:.5f}", pi) == "3.14000");
	assert(format("{:10.5f}", pi) == "   3.14000");

	assert(format("a {} b {} c", 100, 200) == "a 100 b 200 c");
	assert(format("a {0} b {1} c", var("x"), var("y")) == "a x b y c");
	TRACE(format("{:9.1f}", 123.456))
	TRACE(format("{:9.1f}", var(123.456)))
	TRACE(format("{}", var(123.456)))
	TRACE(format("{}", var(12345)))
	TRACE(format("{}", var("abcdefg")))

//#endif

	{
		// Not format but multivalued oconv is useful in format

//		assert("20000]]21000"_var.oconv("DY-4]R#12"_var) == "  2022-10-03]            ]  2025-06-29"_var);
//		assert("20000]]21000"_var.oconv("DY-4]L#12"_var) == "2022-10-03  ]            ]2025-06-29  "_var);
//		assert("20000]]21000"_var.oconv("DY-4]C#12"_var) == " 2022-10-03 ]            ] 2025-06-29 "_var);

		// DY means year only now
		assert("20000]]21000"_var.oconv("DY-4]R#12"_var) == "        2022]            ]        2025"_var);
		assert("20000]]21000"_var.oconv("DY-4]L#12"_var) == "2022        ]            ]2025        "_var);
		assert("20000]]21000"_var.oconv("DY-4]C#12"_var) == "    2022    ]            ]    2025    "_var);

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
		assert(format("{:.^5s}",   "🐱").squote().outputl()     == "'.🐱..'");

		assert(format("{:.^5s}",   var("🐱"), 5).squote().outputl()     == "'.🐱..'");

		// fmtlib v10 fails following
		//assert(format("{:.5s}",    "🐱🐱🐱").squote().outputl() == "'🐱🐱'");
		//assert(format("{:.<5.5s}", "🐱🐱🐱").squote().outputl() == "'🐱🐱.'");

		// Not using dynamic arguments
		//assert(format(FMT_COMPILE("{:4}"), var(9)).squote().outputl() == "'9   '");
		assert(format("{:4}", var(9), 7, 8, 9).squote().outputl() == "'9   '");
		assert(format("{:}", var(9)).squote().outputl() == "'9'");
		assert(format("{}", var(9), 7, 8, 9).squote().outputl() == "'9'");

		// terminate called after throwing an instance of 'fmt::v10::format_error'
		//  what():  missing '}' in format string

		assert(format("{:{}}", var("x"), 7).squote().outputl() == "'x      '");

		// argument not found
		//assert(format("{:{}}", var(9), 4).squote().outputl() == "'9   '");
		//assert(format("{:{}}", var(9), 4).squote().outputl() == "'9   '");
		//assert(format("{:{}s}", var(9), 4).squote().outputl() == "'9   '");
		//assert(format("{:0<.{}}", var(9), 4).squote().outputl() == "'9   '");
		//assert(format("{:<{}}", var(9), 4).squote().outputl() == "'9   '");
		//assert(format("{0:<{}}", var(9), 4).squote().outputl() == "'9   '");

		// cannot switch from automatic to manual argument indexing
		//assert(format("{:<{1}}", var(9), 4).squote().outputl() == "'9   '");
		//assert(format("{0:<{1}}", var(9), 4).squote().outputl() == "'9   '");

	}

		///root/exodus/test/src/test_format.cpp:333:30: error: call of overloaded ‘format(const char [7], std::string, int)’ is ambiguous
#if 0
	{
		// oconv(9, "L#4");
		assert(format("{:{}s}", var(9).toString(), 4).squote().outputl() == "'9   '");
		assert(format("{:{}}", var(9).toString(), 4).squote().outputl() == "'9   '");
		assert(format("{:<{}}", var(9).toString(), 4).squote().outputl() == "'9   '");

		// oconv(9, "R#4");
		assert(format("{:>{}}", var(9).toString(), 4).squote().outputl() == "'   9'");

		// oconv(9, "C#4");
		assert(format("{:^{}}", var(9).toString(), 4).squote().outputl() == "' 9  '");
	}

	{
		// oconv(9, "L(0)#4");
		assert(format("{:0<{}}", var(9).toString(), 4).squote().outputl() == "'9000'");

		// oconv(9, "R(0)#4");
		assert(format("{:0>{}}", var(9).toString(), 4).squote().outputl() == "'0009'");

		// oconv(9, "C(0)#4");
		assert(format("{:0^{}}", var(9).toString(), 4).squote().outputl() == "'0900'");

	}
#endif


	{
		// Using d for integer output
		/////////////////////////////

		// oconv(9, "R(0)#4");
		assert(format("{:04d}", var(9)).squote().outputl() == "'0009'");
		assert(format("{:0>4d}", var(9)).squote().outputl() == "'0009'");

		// oconv(9, "L(0)4")
		assert(format("{:0<4d}", var(9)).squote().outputl() == "'9000'");

		// oconv(9, "C(0)4")
		assert(format("{:0^4d}", var(9)).squote().outputl() == "'0900'");

		// oconv(9, "R#4")
		assert(format("{:>4d}", var(9)).squote().outputl() == "'   9'");

		// oconv(9, "L#4")
		assert(format("{:<4d}", var(9)).squote().outputl() == "'9   '");

		// oconv(9, "C#4")
		assert(format("{:^4d}", var(9)).squote().outputl() == "' 9  '");
//#if FMT_VERSION >= 110000
#if 1
		assert(format("{:>04d}", var(9)).squote().outputl() == "'   9'");// 0 in the wrong place so ignored
		assert(format("{:<04d}", var(9)).squote().outputl() == "'9   '"); // ditto
		assert(format("{:^04d}", var(9)).squote().outputl() == "' 9  '"); // ditto
#else
		assert(format("{:>04d}", var(9)).squote().outputl() == "'0009'");// libfmt9
		assert(format("{:<04d}", var(9)).squote().outputl() == "'9000'"); // libfmt9
		assert(format("{:^04d}", var(9)).squote().outputl() == "'0900'"); // libfmt9
#endif
	}

	{
		var pi = 3.1415;
		assert(format("{:10f}", pi).squote().outputl() == "'  3.141500'");

		format("{:03}",7).squote().outputl(":03 7 = ");//007
		format("{:0{}}",7,4).squote().outputl(":0{} 7 = ");//0007

		// f = floating point after rounding
		assert(format("{:06f}",var(7.9)).squote().outputl() == "'7.900000'");
		assert(format("{:07f}",var(7.9)).squote().outputl() == "'7.900000'");
		assert(format("{:01f}",var(7.9)).squote().outputl() == "'7.900000'"); // why the same? leading zero does nothing?

		assert(format("{:.1f}",var(7.55)).squote().outputl() == "'7.5'");   // wrong!
		assert(format("{:.1f}",var(-7.55)).squote().outputl() == "'-7.5'"); // wrong!

		assert(format("{:.1f}",var(6.55)).squote().outputl() == "'6.5'");   // wrong!
		assert(format("{:.1f}",var(-6.55)).squote().outputl() == "'-6.5'"); // wrong!

		assert(format("{:.1f}",var(6.59)).squote().outputl() == "'6.6'");   // OK
		assert(format("{:.1f}",var(-6.59)).squote().outputl() == "'-6.6'"); // OK

		assert(format("{:.1f}",var(6.55)).squote().outputl() == "'6.5'");   // wrong!
		assert(format("{:.1f}",var(-6.55)).squote().outputl() == "'-6.5'"); // wrong!

		assert(format("{:06d}",var(8.9)).squote().outputl() == "'000008'");
		assert(format("{:010.6f}",var(7.9)).squote().outputl() == "'007.900000'");

		// d = digits for integer AFTER truncating
		assert(format("{:6d}",var(7.5)).squote().outputl() == "'     7'");
		assert(format("{:6d}",var(7.9)).squote().outputl() == "'     7'");
		assert(format("{:6d}",var(8.5)).squote().outputl() == "'     8'");
		assert(format("{:6d}",var(-7.5)).squote().outputl() == "'    -7'");
		assert(format("{:6d}",var(-8.5)).squote().outputl() == "'    -8'");
		assert(format("{:6d}",var(-8.9)).squote().outputl() == "'    -8'");

		//format("{::0{}}",var(7),4).squote().outputl(":0{} 7 = ");  //      7

//		std::cout << fmt::format("{:03}",7) << " :03 7 = " << std::endl;

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

		assert(format("{:.>{}.{}f}", v2, 10, 5).squote().outputl() == "'..12.34500'");

		assert(format("'{:13f}'", v2).outputl() == "'    12.345000'");

		// dynamic pad width and precision on standard format specifiers like f (fixed decimals)
		assert(format("'{:{}.{}f}'", v2, 12, 2).outputl() == "'       12.35'");
		assert(format("{:.>{}.{}}", v1, 10, 5).outputl() == ".......abc");
		assert(format("'{:{}b}'", v10, 12, 2).outputl() == "'        1010'");
		assert(format("'{:0f}'", v2).outputl() == "'12.345000'");
		assert(format("'{::HEX}'", v10).outputl() == "'3130'");
		assert(format("a {} b {} c", 100, 200).outputl() == "a 100 b 200 c");

	}

	printl(format("test free function print ... {}", var(3.5)));
	print("test free function print ... {}", var(3.5));
	println("test exo free function println ... {}", var(3.5));

#endif // EXO_FORMAT

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}
}; // programexit()
