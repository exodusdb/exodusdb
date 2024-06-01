#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>

programinit()

function main() {

#if EXO_FORMAT

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
		//var v1 = "{::MD2P]R#12}"_var;
		//TRACE(format(v1, var(123.456)));
		assert(format(v1, var(123.456)).outputl() == "      123.46");
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

#elif EXO_FORMAT
#warning Using fmt library instead of std::format
	//__clang_major__ 18
	// Correct in fmt library
	assert(format("{:.^5s}", "🐱") == ".🐱..");
	assert(format("{:.5s}", "🐱🐱🐱") == "🐱🐱🐱");
	assert(format("{:.<5.5s}", "🐱🐱🐱") == "🐱🐱🐱");

	assert(format("{:^5s}", "🐱") == " 🐱  ");
	assert(format("{:.5s}", "🐱🐱🐱") == "🐱🐱🐱");
	assert(format("{:<5.5s}", "🐱🐱🐱") == "🐱🐱🐱");
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

#endif

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

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}
programexit()
