#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>
#include <limits>
#include <iomanip>
#include <cmath>

// 1. TO_CHARS from Ubuntu 22.04
#if __GNUC__ >= 11
#define USE_TO_CHARS_G

// 2. RYU
#elif __has_include(<ryu/ryu.h>)
#define USE_RYU

// 3. STRINGSTREAM
#else
#endif


#include <exodus/program.h>

using namespace std;

//think of ASCII decimal characters as "external format" i.e. that which humans see and enter
//
//think of double as "internal format" i.e. that which computers can work with
//
//think of converting ASCII decimal -> double as "input conversion" (ICONV) i.e from "external format" to "internal format"
//
//think of converting double -> >ASCII decimal as "output conversion" (OCONV) i.e. from "internal format" to "external format"

programinit()

	double dd1;
	var dv1;
	var dv2;
	var sv1;
	var sv2;

function main() {

	printl("\n========= test_precision ==========");

    {

		printl("On 10 byte floats/doubles, 22/7 -> 3.14285714285714286");

		// TODO may need adjustment when we switch from ryu to c++ std::to_chars()
		printl("On exodus 8 byte doubles,  22/7 -> 3.142857142857143");
		var v22 = 22;
		var v7 = 7;
		assert((v22/v7).toString() == "3.142857142857143");
    }

	{

		assert(!var("123e+3e").isnum());
		assert(!var("123e+3.").isnum());
		assert(!var("123e+3+").isnum());
		assert(!var("123e+3-").isnum());

		printl("\nASSUMING precision(15) in");
		printl("mv.cpp std::string dblToString(double double1)");

		printl("\nNumber of decimal digits that can be round tripped between ASCII decimal and DOUBLE binary without change\nMaximum reliable precision (decimal digits) is ");
		TRACE(numeric_limits<double>::digits10)
		printl("Actually we get one more digit of precision from the left hand side.");

		//example of max precision pure c++ stream conversion double to ASCII
		std::cout << std::setprecision(15) << 1.234567890123456000000 << std::endl;
		std::cout << std::setprecision(15) << 1.234567890123000000000 << std::endl;
		std::cout << std::setprecision(15) << 999999.999999999 << std::endl;
		std::cout << std::setprecision(15) << 1000000.99999999 << std::endl;
		std::cout << std::setprecision(15) << 1999999.99999999 << std::endl;
		std::cout << std::setprecision(15) << 99999999.9999999 << std::endl;
		std::cout << std::setprecision(15) << 999999999.999999 << std::endl;
		std::cout << std::setprecision(15) << 999999999999999.1 << std::endl;
		printl("The following trigger scientific format");
		std::cout << std::setprecision(15) << 999999999999999.9 << std::endl;
		std::cout << std::setprecision(15) << 0.12345678901234567890 << std::endl;
		std::cout << std::setprecision(15) << 0.00005678901234567890 << std::endl;

		cout << "\nsize of double used by var = " << sizeof(double) << std::endl;
		cout <<" size of long double = " << sizeof(long double) << endl;

		//cout << setprecision(16) << fixed << 1234567890123456.789012345678901234567890 << endl;
		//cout << setprecision(16) << fixed << 999999999999999.9 << endl;

		assert( (var(9.36749)        ^ "x").outputl() == "9.36749x");
		assert(((var("9.36749") + 0) ^ "x").outputl() == "9.36749x");

#ifdef USE_TO_CHARS_G
		//assert((var(1234567890123456789.)  ^ "x").outputl() == "1234567890123456768x");
		//assert((var(12345678901234567890.) ^ "x").outputl() == "12345678901234567168x");
		assert((var(1234567890123456789.)  ^ "x").outputl() == "1.234567890123457e+18x");
		assert((var(12345678901234567890.) ^ "x").outputl() == "1.234567890123457e+19x");

		assert((var("999999999999999.9")    + 0).outputl().toString() == "999999999999999.9");
		assert((var("9999999999999999.9")   + 0).outputl().toString() == "1e+16");

#elif defined(USE_TO_CHARS_S)
		assert((var(1234567890123456789.)  ^ "x").outputl() == "1.2345678901234568e+18x");
		assert((var(12345678901234567890.) ^ "x").outputl() == "1.2345678901234567e+19x");

		assert((var("999999999999999.9")    + 0).outputl().toString() == "9.999999999999999e+14");
		//assert((var("9999999999999999.9")   + 0).outputl().toString() == "1.0e16");
		assert((var("9999999999999999.9")   + 0).outputl().toString() == "1e+16");

#elif defined(USE_RYU)
		assert((var(1234567890123456789.)  ^ "x").outputl() == "1.2345678901234568e+18x");
		assert((var(12345678901234567890.) ^ "x").outputl() == "1.2345678901234567e+19x");
#else
		assert(var(1234567890.0)               .outputl().squote() eq "'1234567890'");
		assert(var(1234567890123.0)            .outputl().squote() eq "'1234567890123'");
		assert(var(12345678901234.0)           .outputl().squote() eq "'12345678901234'");
		assert(var(123456789012345.0)          .outputl().squote() eq "'123456789012345'");

		assert((var(1234567890123456789.)  ^ "x").outputl() == "1.234567890123457e+18x");
		assert((var(12345678901234567890.) ^ "x").outputl() == "1.234567890123457e+19x");
#endif

#ifndef USE_TO_CHARS_G
		assert((var("999999999999999.9")    + 0).outputl().toString() == "999999999999999.9");
		printl(var(9999999999999999.9).toDouble());
		TRACE( var("9999999999999999.9")   + 0);
		//assert((var("9999999999999999.9")   + 0).outputl().toString() == "1.0e+16");
		assert((var("9999999999999999.9")   + 0).outputl().toString() == "1e+16");
#endif

//#endif

		assert(   var("9999999999999999.9").outputl().isnum());
		assert(   var("0.99999999999999999").outputl().isnum());

		assert((var("99999999999999999.9")  + 0).outputl().toString() == "1e+17");
		assert((var("999999999999999999.9")  + 0).outputl().toString() == "1e+18");

		assert((var(999999999999999.9)    ^ "x").outputl() == "999999999999999.9x");
		assert((var(9999999999999999.9)   ^ "x").outputl() == "1e+16x");
		assert((var(99999999999999999.9)  ^ "x").outputl() == "1e+17x");
		assert((var(999999999999999999.9) ^ "x").outputl() == "1e+18x");

		assert(var(10)/var(3).outputl() == "3.3333333333333333");

		printl("\ntoo many decimal points get ROUNDED");
		dd1 = 0.00005678901234567890;
		dv1 = dd1;
		dv2 = "0.00005678901234567890d";
		sv1 = dv1^"x";
		//var sv2 = "0.00005678901235x";
#ifdef USE_TO_CHARS_G
		//sv2 = "5.67890123456789e-05x";
		sv2 = "0.0000567890123456789x";
#else
		sv2 = "0.0000567890123456789x";
#endif
		gosub out();
		assert(sv1 == sv2);

		printl("\ntoo many digits of precision get truncated");
		dd1 = 1234567890.00005678;
		dv1 = dd1;
		dv2 = "1234567890.00005678d";
		sv1 = dv1 ^ "x";
		TRACE(var(dd1))
		TRACE(var(dv1))
		TRACE(var(dv2))

//#if defined(USE_TO_CHARS_G)
//		sv2 = "1234567890.0000567x";
#if defined(USE_RYU)
		sv2 = "1234567890.0000567x";
#else
		sv2 = "1234567890.000057x";
#endif
		TRACE(sv1)
		TRACE(sv2)
		//concat:  1234567890.0000567x
		//Target:  1234567890.000057x
		gosub out();
		assert(sv1 == sv2);

		printl("\nsmall numbers are ok");
		dd1 = 0.00005678;
		dv1 = dd1;
		dv2 = "0.00005678d";
		sv1 = dv1^"x";
//#ifdef USE_TO_CHARS_G
//		sv2 = "5.678e-05x";
//#else
		sv2 = "0.00005678x";
//#endif
		gosub out();
		assert(sv1 == sv2);

		printl("\nsmallest decimal number");
		dd1=0.0000000000001;
		dv1 = dd1;
		dv2="0.0000000000001d";
		sv1 = dv1^"x";
//#ifdef USE_TO_CHARS_G
//		sv2="1e-13x";
		sv2="0.0000000000001x";
//#else
//		sv2="1.0e-13x";
//#endif
		gosub out();
		assert(sv1 == sv2);

		/*
		printl("\nless than smallest decimal number becomes 0.0");
		dd1=0.000000000000009;
		dv1 = dd1;
		dv2="0.000000000000009d";
		sv1 = dv1^"x";
		sv2="0x";
		gosub out();
		assert(sv1 == sv2);
		*/

		printl("\ncheck largest number (14x9) works without scientific format");
		dd1= 9999999999999.9;
		dv1 = dd1;
		dv2="9999999999999.9d";
		sv1=dv1^"x";
		sv2="9999999999999.9x";
		gosub out();
		assert(sv1 == sv2);

		printl("\nVery large numbers are rounded and lose accuracy");
		dd1=999999999999999999.9;
		dv1 = dd1;
		dv2="999999999999999999.9d";
		sv1=dv1^"x";
		//sv2="1E18x";
		sv2="1e+18x";

		gosub out();
		assert(sv1 == sv2);

		printl("\nVery large numbers are rounded and lose accuracy");
		dd1=12345678901234567890123456789.0;
		dv1 = dd1;
		dv2="12345678901234567890123456789.0d";
		sv1=dv1^"x";
#if defined(USE_RYU)
		sv2="1.2345678901234568e+28x";
#else
		sv2="1.234567890123457e+28x";
#endif
		gosub out();
		assert(sv1 == sv2);

		printl("\ncheck excessive number DONT get converted to scientific");
		dd1=99999999999999.9;
		dv1 = dd1;
		dv2="99999999999999.9d";
		sv1=dv1^"x";
#ifdef USE_RYU
		sv2="99999999999999.9x";
#else
		//sv2="99999999999999.9x";
		sv2="99999999999999.91x";
#endif
		gosub out();
		assert(sv1 == sv2);

		//test2 tests both positive and negative round trip chars to chars

		assert(test2("00000000000000000000.00000000000000000000",                   "0x"));

		assert(test2("-0", "0x"));
		//assert(test2("+0", "0x"));
//#ifdef USE_RYU
//		assert(test2("-1", "-1.0x"));
//#else
		assert(test2("-1", "-1x"));
//#endif
		//assert(test2("+1", "+1x"));
		assert(test2("-2", "-2x"));
		//assert(test2("+2", "+2x"));

		assert(test2("-0.", "0x"));
		//assert(test2("+0.", "0x"));
		assert(test2("-1.", "-1x"));
		//assert(test2("+1.", "+1x"));
		assert(test2("-2.", "-2x"));
		//assert(test2("+2.", "+2x"));

		assert(test2("-0.0", "0x"));
		//assert(test2("+0.0", "0x"));
		assert(test2("-1.0", "-1x"));
		//assert(test2("+1.0", "+1x"));
		assert(test2("-2.0", "-2x"));
		//assert(test2("+2.0", "+2x"));

		assert(test2("-00.0", "0x"));
		//assert(test2("+00.0", "0x"));
		assert(test2("-01.0", "-1x"));
		//assert(test2("+01.0", "+1x"));
		assert(test2("-02.0", "-2x"));
		//assert(test2("+02.0", "+2x"));

		assert(test2("-00.00", "0x"));
		//assert(test2("+00.00", "0x"));
		assert(test2("-01.00", "-1x"));
		//assert(test2("+01.00", "+1x"));
		assert(test2("-02.00", "-2x"));
		//assert(test2("+02.00", "+2x"));

/* Code generator for large number tests

#include <exodus/program.h>
programinit()

function main() {

    var t3 = str("1234567890",3);
    var t6 = str("0000000000",12);

    for (var i : range(1,90)) {
        var t7 = t6.splice(i,30,t3);
        t7.splicer(61,0,".");
        t7.trimmerf("0").trimmerb("0");
        //printl(t7+0);

        printl("assert(test2(" ^ t7.quote() ^ ", " ^ ((t7 + 0) ^ "x").quote() ^ "));");

    }
    return 0;
}

programexit()
*/

#ifdef USE_RYU
#elif defined(USE_RYU)
		assert(test2("12345678901234567890123456789000000000000000000000000000000.", "1.2345678901234568e+58x"));
		assert(test2("1234567890123456789012345678900000000000000000000000000000.", "1.2345678901234568e+57x"));
		assert(test2("123456789012345678901234567890000000000000000000000000000.", "1.2345678901234569e+56x"));
		assert(test2("12345678901234567890123456789000000000000000000000000000.", "1.2345678901234567e+55x"));
		assert(test2("1234567890123456789012345678900000000000000000000000000.", "1.2345678901234568e+54x"));
		assert(test2("123456789012345678901234567890000000000000000000000000.", "1.2345678901234568e+53x"));
		assert(test2("12345678901234567890123456789000000000000000000000000.", "1.2345678901234568e+52x"));
		assert(test2("1234567890123456789012345678900000000000000000000000.", "1.2345678901234568e+51x"));
		assert(test2("123456789012345678901234567890000000000000000000000.", "1.2345678901234569e+50x"));
		assert(test2("12345678901234567890123456789000000000000000000000.", "1.2345678901234567e+49x"));
		assert(test2("1234567890123456789012345678900000000000000000000.", "1.2345678901234568e+48x"));
		assert(test2("123456789012345678901234567890000000000000000000.", "1.2345678901234568e+47x"));
		assert(test2("12345678901234567890123456789000000000000000000.", "1.2345678901234568e+46x"));
		assert(test2("1234567890123456789012345678900000000000000000.", "1.2345678901234568e+45x"));
		assert(test2("123456789012345678901234567890000000000000000.", "1.2345678901234567e+44x"));
		assert(test2("12345678901234567890123456789000000000000000.", "1.2345678901234568e+43x"));
		assert(test2("1234567890123456789012345678900000000000000.", "1.2345678901234567e+42x"));
		assert(test2("123456789012345678901234567890000000000000.", "1.2345678901234568e+41x"));
		assert(test2("12345678901234567890123456789000000000000.", "1.2345678901234568e+40x"));
		assert(test2("1234567890123456789012345678900000000000.", "1.2345678901234568e+39x"));
		assert(test2("123456789012345678901234567890000000000.", "1.2345678901234568e+38x"));
		assert(test2("12345678901234567890123456789000000000.", "1.2345678901234568e+37x"));
		assert(test2("1234567890123456789012345678900000000.", "1.2345678901234568e+36x"));
		assert(test2("123456789012345678901234567890000000.", "1.2345678901234568e+35x"));
		assert(test2("12345678901234567890123456789000000.", "1.234567890123457e+34x"));
		assert(test2("1234567890123456789012345678900000.", "1.2345678901234568e+33x"));
		assert(test2("123456789012345678901234567890000.", "1.2345678901234569e+32x"));
		assert(test2("12345678901234567890123456789000.", "1.2345678901234567e+31x"));
		assert(test2("1234567890123456789012345678900.", "1.2345678901234568e+30x"));
		assert(test2("123456789012345678901234567890.", "1.2345678901234568e+29x"));
		assert(test2("12345678901234567890123456789.", "1.2345678901234568e+28x"));
		assert(test2("1234567890123456789012345678.9", "1.2345678901234569e+27x"));
		assert(test2("123456789012345678901234567.89", "1.2345678901234568e+26x"));
		assert(test2("12345678901234567890123456.789", "1.2345678901234568e+25x"));
		assert(test2("1234567890123456789012345.6789", "1.2345678901234568e+24x"));
		assert(test2("123456789012345678901234.56789", "1.2345678901234569e+23x"));
		assert(test2("12345678901234567890123.456789", "1.2345678901234568e+22x"));
		assert(test2("1234567890123456789012.3456789", "1.2345678901234568e+21x"));
		assert(test2("123456789012345678901.23456789", "1.2345678901234568e+20x"));
		assert(test2("12345678901234567890.123456789", "1.2345678901234567e+19x"));
		assert(test2("1234567890123456789.0123456789", "1.2345678901234568e+18x"));
		assert(test2("123456789012345678.90123456789", "1.2345678901234568e+17x"));
		assert(test2("12345678901234567.890123456789", "1.2345678901234568e+16x"));
		assert(test2("1234567890123456.7890123456789", "1234567890123456.8x"));
		assert(test2("123456789012345.67890123456789", "123456789012345.67x"));
		assert(test2("12345678901234.567890123456789", "12345678901234.568x"));
		assert(test2("1234567890123.4567890123456789", "1234567890123.4568x"));
		assert(test2("123456789012.34567890123456789", "123456789012.34567x"));
		assert(test2("12345678901.234567890123456789", "12345678901.234568x"));
		assert(test2("1234567890.1234567890123456789", "1234567890.1234567x"));
		assert(test2("123456789.01234567890123456789", "123456789.01234567x"));
		assert(test2("12345678.901234567890123456789", "12345678.901234567x"));
		assert(test2("1234567.8901234567890123456789", "1234567.8901234567x"));
		assert(test2("123456.78901234567890123456789", "123456.78901234567x"));
		assert(test2("12345.678901234567890123456789", "12345.678901234567x"));
		assert(test2("1234.5678901234567890123456789", "1234.567890123457x"));
		assert(test2("123.45678901234567890123456789", "123.45678901234568x"));
		assert(test2("12.345678901234567890123456789", "12.345678901234567x"));
		assert(test2("1.2345678901234567890123456789", "1.2345678901234567x"));
		assert(test2(".12345678901234567890123456789", "0.12345678901234568x"));
		assert(test2(".012345678901234567890123456789", "0.012345678901234568x"));
		assert(test2(".0012345678901234567890123456789", "0.0012345678901234567x"));
		assert(test2(".00012345678901234567890123456789", "0.00012345678901234567x"));
		assert(test2(".000012345678901234567890123456789", "0.000012345678901234568x"));
		assert(test2(".0000012345678901234567890123456789", "0.0000012345678901234567x"));
		assert(test2(".00000012345678901234567890123456789", "0.00000012345678901234568x"));
		assert(test2(".000000012345678901234567890123456789", "0.000000012345678901234567x"));
		assert(test2(".0000000012345678901234567890123456789", "0.0000000012345678901234568x"));
		assert(test2(".00000000012345678901234567890123456789", "0.00000000012345678901234568x"));
		assert(test2(".000000000012345678901234567890123456789", "0.000000000012345678901234568x"));
		assert(test2(".0000000000012345678901234567890123456789", "0.0000000000012345678901234569x"));
		assert(test2(".00000000000012345678901234567890123456789", "0.0000000000001234567890123457x"));
		assert(test2(".000000000000012345678901234567890123456789", "0.000000000000012345678901234568x"));
		assert(test2(".0000000000000012345678901234567890123456789", "0.0000000000000012345678901234568x"));
		assert(test2(".00000000000000012345678901234567890123456789", "1.2345678901234568e-16x"));
		assert(test2(".000000000000000012345678901234567890123456789", "1.2345678901234568e-17x"));
		assert(test2(".0000000000000000012345678901234567890123456789", "1.2345678901234568e-18x"));
		assert(test2(".00000000000000000012345678901234567890123456789", "1.2345678901234568e-19x"));
		assert(test2(".000000000000000000012345678901234567890123456789", "1.2345678901234569e-20x"));
		assert(test2(".0000000000000000000012345678901234567890123456789", "1.2345678901234568e-21x"));
		assert(test2(".00000000000000000000012345678901234567890123456789", "1.2345678901234568e-22x"));
		assert(test2(".000000000000000000000012345678901234567890123456789", "1.2345678901234567e-23x"));
		assert(test2(".0000000000000000000000012345678901234567890123456789", "1.2345678901234567e-24x"));
		assert(test2(".00000000000000000000000012345678901234567890123456789", "1.2345678901234568e-25x"));
		assert(test2(".000000000000000000000000012345678901234567890123456789", "1.2345678901234568e-26x"));
		assert(test2(".0000000000000000000000000012345678901234567890123456789", "1.2345678901234567e-27x"));
		assert(test2(".00000000000000000000000000012345678901234567890123456789", "1.2345678901234567e-28x"));
		assert(test2(".000000000000000000000000000012345678901234567890123456789", "1.2345678901234567e-29x"));
		assert(test2(".0000000000000000000000000000012345678901234567890123456789", "1.2345678901234568e-30x"));
#else
//#if defined(USE_TO_CHARS_G)
		assert(test2("123456789012345678901234567890000000000000000000000000000000.", "1.234567890123457e+59x"));
		assert(test2("12345678901234567890123456789000000000000000000000000000000.", "1.234567890123457e+58x"));
		assert(test2("1234567890123456789012345678900000000000000000000000000000.", "1.234567890123457e+57x"));
		assert(test2("123456789012345678901234567890000000000000000000000000000.", "1.234567890123457e+56x"));
		assert(test2("12345678901234567890123456789000000000000000000000000000.", "1.234567890123457e+55x"));
		assert(test2("1234567890123456789012345678900000000000000000000000000.", "1.234567890123457e+54x"));
		assert(test2("123456789012345678901234567890000000000000000000000000.", "1.234567890123457e+53x"));
		assert(test2("12345678901234567890123456789000000000000000000000000.", "1.234567890123457e+52x"));
		assert(test2("1234567890123456789012345678900000000000000000000000.", "1.234567890123457e+51x"));
		assert(test2("123456789012345678901234567890000000000000000000000.", "1.234567890123457e+50x"));
		assert(test2("12345678901234567890123456789000000000000000000000.", "1.234567890123457e+49x"));
		assert(test2("1234567890123456789012345678900000000000000000000.", "1.234567890123457e+48x"));
		assert(test2("123456789012345678901234567890000000000000000000.", "1.234567890123457e+47x"));
		assert(test2("12345678901234567890123456789000000000000000000.", "1.234567890123457e+46x"));
		assert(test2("1234567890123456789012345678900000000000000000.", "1.234567890123457e+45x"));
		assert(test2("123456789012345678901234567890000000000000000.", "1.234567890123457e+44x"));
		assert(test2("12345678901234567890123456789000000000000000.", "1.234567890123457e+43x"));
		assert(test2("1234567890123456789012345678900000000000000.", "1.234567890123457e+42x"));
		assert(test2("123456789012345678901234567890000000000000.", "1.234567890123457e+41x"));
		assert(test2("12345678901234567890123456789000000000000.", "1.234567890123457e+40x"));
		assert(test2("1234567890123456789012345678900000000000.", "1.234567890123457e+39x"));
		assert(test2("123456789012345678901234567890000000000.", "1.234567890123457e+38x"));
		assert(test2("12345678901234567890123456789000000000.", "1.234567890123457e+37x"));
		assert(test2("1234567890123456789012345678900000000.", "1.234567890123457e+36x"));
		assert(test2("123456789012345678901234567890000000.", "1.234567890123457e+35x"));
		assert(test2("12345678901234567890123456789000000.", "1.234567890123457e+34x"));
		assert(test2("1234567890123456789012345678900000.", "1.234567890123457e+33x"));
		assert(test2("123456789012345678901234567890000.", "1.234567890123457e+32x"));
		assert(test2("12345678901234567890123456789000.", "1.234567890123457e+31x"));
		assert(test2("1234567890123456789012345678900.", "1.234567890123457e+30x"));
		assert(test2("123456789012345678901234567890.", "1.234567890123457e+29x"));
		assert(test2("12345678901234567890123456789.", "1.234567890123457e+28x"));
		assert(test2("1234567890123456789012345678.9", "1.234567890123457e+27x"));
		assert(test2("123456789012345678901234567.89", "1.234567890123457e+26x"));
		assert(test2("12345678901234567890123456.789", "1.234567890123457e+25x"));
		assert(test2("1234567890123456789012345.6789", "1.234567890123457e+24x"));
		assert(test2("123456789012345678901234.56789", "1.234567890123457e+23x"));
		assert(test2("12345678901234567890123.456789", "1.234567890123457e+22x"));
		assert(test2("1234567890123456789012.3456789", "1.234567890123457e+21x"));
		assert(test2("123456789012345678901.23456789", "1.234567890123457e+20x"));
		assert(test2("12345678901234567890.123456789", "1.234567890123457e+19x"));
		assert(test2("1234567890123456789.0123456789", "1.234567890123457e+18x"));
		assert(test2("123456789012345678.90123456789", "1.234567890123457e+17x"));
		assert(test2("12345678901234567.890123456789", "1.234567890123457e+16x"));
		assert(test2("1234567890123456.7890123456789", "1234567890123457x"));
		assert(test2("123456789012345.67890123456789", "123456789012345.7x"));
		assert(test2("12345678901234.567890123456789", "12345678901234.57x"));
		assert(test2("1234567890123.4567890123456789", "1234567890123.457x"));
		assert(test2("123456789012.34567890123456789", "123456789012.3457x"));
		assert(test2("12345678901.234567890123456789", "12345678901.23457x"));
		assert(test2("1234567890.1234567890123456789", "1234567890.123457x"));
		assert(test2("123456789.01234567890123456789", "123456789.0123457x"));
		assert(test2("12345678.901234567890123456789", "12345678.90123457x"));
		assert(test2("1234567.8901234567890123456789", "1234567.890123457x"));
		assert(test2("123456.78901234567890123456789", "123456.7890123457x"));
		assert(test2("12345.678901234567890123456789", "12345.67890123457x"));
		assert(test2("1234.5678901234567890123456789", "1234.567890123457x"));
		assert(test2("123.45678901234567890123456789", "123.4567890123457x"));
		assert(test2("12.345678901234567890123456789", "12.34567890123457x"));
		assert(test2("1.2345678901234567890123456789", "1.234567890123457x"));
		assert(test2(".12345678901234567890123456789", "0.1234567890123457x"));
		assert(test2(".012345678901234567890123456789", "0.01234567890123457x"));
		assert(test2(".0012345678901234567890123456789", "0.001234567890123457x"));
		assert(test2(".00012345678901234567890123456789", "0.0001234567890123457x"));
		assert(test2(".000012345678901234567890123456789", "0.00001234567890123457x"));
		assert(test2(".0000012345678901234567890123456789", "0.000001234567890123457x"));
		assert(test2(".00000012345678901234567890123456789", "0.0000001234567890123457x"));
		assert(test2(".000000012345678901234567890123456789", "0.00000001234567890123457x"));
		assert(test2(".0000000012345678901234567890123456789", "0.000000001234567890123457x"));
		assert(test2(".00000000012345678901234567890123456789", "0.0000000001234567890123457x"));
		assert(test2(".000000000012345678901234567890123456789", "0.00000000001234567890123457x"));
		assert(test2(".0000000000012345678901234567890123456789", "0.000000000001234567890123457x"));
		assert(test2(".00000000000012345678901234567890123456789", "0.0000000000001234567890123457x"));
		assert(test2(".000000000000012345678901234567890123456789", "0.00000000000001234567890123457x"));
		assert(test2(".0000000000000012345678901234567890123456789", "0.000000000000001234567890123457x"));
		assert(test2(".00000000000000012345678901234567890123456789", "1.234567890123457e-16x"));
		assert(test2(".000000000000000012345678901234567890123456789", "1.234567890123457e-17x"));
		assert(test2(".0000000000000000012345678901234567890123456789", "1.234567890123457e-18x"));
		assert(test2(".00000000000000000012345678901234567890123456789", "1.234567890123457e-19x"));
		assert(test2(".000000000000000000012345678901234567890123456789", "1.234567890123457e-20x"));
		assert(test2(".0000000000000000000012345678901234567890123456789", "1.234567890123457e-21x"));
		assert(test2(".00000000000000000000012345678901234567890123456789", "1.234567890123457e-22x"));
		assert(test2(".000000000000000000000012345678901234567890123456789", "1.234567890123457e-23x"));
		assert(test2(".0000000000000000000000012345678901234567890123456789", "1.234567890123457e-24x"));
		assert(test2(".00000000000000000000000012345678901234567890123456789", "1.234567890123457e-25x"));
		assert(test2(".000000000000000000000000012345678901234567890123456789", "1.234567890123457e-26x"));
		assert(test2(".0000000000000000000000000012345678901234567890123456789", "1.234567890123457e-27x"));
		assert(test2(".00000000000000000000000000012345678901234567890123456789", "1.234567890123457e-28x"));
		assert(test2(".000000000000000000000000000012345678901234567890123456789", "1.234567890123457e-29x"));
		assert(test2(".0000000000000000000000000000012345678901234567890123456789", "1.234567890123457e-30x"));
#endif

	}

	{
		printl("Test consecutive numbers at maximum precision from 1.234567890123450 to 1.234567890123509");

		printl((var(1.234567890123450) ^ "x") , "1.23456789012345x");

		assert((var(1.234567890123450) ^ "x").outputl() == "1.23456789012345x");//0
		assert((var(1.234567890123451) ^ "x").outputl() == "1.234567890123451x");
		assert((var(1.234567890123452) ^ "x").outputl() == "1.234567890123452x");
		assert((var(1.234567890123453) ^ "x").outputl() == "1.234567890123453x");
		assert((var(1.234567890123454) ^ "x").outputl() == "1.234567890123454x");
		assert((var(1.234567890123455) ^ "x").outputl() == "1.234567890123455x");
		assert((var(1.234567890123456) ^ "x").outputl() == "1.234567890123456x");
		assert((var(1.234567890123457) ^ "x").outputl() == "1.234567890123457x");
		assert((var(1.234567890123458) ^ "x").outputl() == "1.234567890123458x");
		assert((var(1.234567890123459) ^ "x").outputl() == "1.234567890123459x");
		assert((var(1.234567890123460) ^ "x").outputl() == "1.23456789012346x");//0
		assert((var(1.234567890123461) ^ "x").outputl() == "1.234567890123461x");
		assert((var(1.234567890123462) ^ "x").outputl() == "1.234567890123462x");
		assert((var(1.234567890123463) ^ "x").outputl() == "1.234567890123463x");
		assert((var(1.234567890123464) ^ "x").outputl() == "1.234567890123464x");
		assert((var(1.234567890123465) ^ "x").outputl() == "1.234567890123465x");
		assert((var(1.234567890123466) ^ "x").outputl() == "1.234567890123466x");
		assert((var(1.234567890123467) ^ "x").outputl() == "1.234567890123467x");
		assert((var(1.234567890123468) ^ "x").outputl() == "1.234567890123468x");
		assert((var(1.234567890123469) ^ "x").outputl() == "1.234567890123469x");
		assert((var(1.234567890123470) ^ "x").outputl() == "1.23456789012347x");//0
		assert((var(1.234567890123471) ^ "x").outputl() == "1.234567890123471x");
		assert((var(1.234567890123472) ^ "x").outputl() == "1.234567890123472x");
		assert((var(1.234567890123473) ^ "x").outputl() == "1.234567890123473x");
		assert((var(1.234567890123474) ^ "x").outputl() == "1.234567890123474x");
		assert((var(1.234567890123475) ^ "x").outputl() == "1.234567890123475x");
		assert((var(1.234567890123476) ^ "x").outputl() == "1.234567890123476x");
		assert((var(1.234567890123477) ^ "x").outputl() == "1.234567890123477x");
		assert((var(1.234567890123478) ^ "x").outputl() == "1.234567890123478x");
		assert((var(1.234567890123479) ^ "x").outputl() == "1.234567890123479x");
		assert((var(1.234567890123480) ^ "x").outputl() == "1.23456789012348x");//0
		assert((var(1.234567890123481) ^ "x").outputl() == "1.234567890123481x");
		assert((var(1.234567890123482) ^ "x").outputl() == "1.234567890123482x");
		assert((var(1.234567890123483) ^ "x").outputl() == "1.234567890123483x");
		assert((var(1.234567890123484) ^ "x").outputl() == "1.234567890123484x");
		assert((var(1.234567890123485) ^ "x").outputl() == "1.234567890123485x");
		assert((var(1.234567890123486) ^ "x").outputl() == "1.234567890123486x");
		assert((var(1.234567890123487) ^ "x").outputl() == "1.234567890123487x");
		assert((var(1.234567890123488) ^ "x").outputl() == "1.234567890123488x");
		assert((var(1.234567890123489) ^ "x").outputl() == "1.234567890123489x");
		assert((var(1.234567890123490) ^ "x").outputl() == "1.23456789012349x");//0
		assert((var(1.234567890123491) ^ "x").outputl() == "1.234567890123491x");
		assert((var(1.234567890123492) ^ "x").outputl() == "1.234567890123492x");
		assert((var(1.234567890123493) ^ "x").outputl() == "1.234567890123493x");
		assert((var(1.234567890123494) ^ "x").outputl() == "1.234567890123494x");
		assert((var(1.234567890123495) ^ "x").outputl() == "1.234567890123495x");
		assert((var(1.234567890123496) ^ "x").outputl() == "1.234567890123496x");
		assert((var(1.234567890123497) ^ "x").outputl() == "1.234567890123497x");
		assert((var(1.234567890123498) ^ "x").outputl() == "1.234567890123498x");
		assert((var(1.234567890123499) ^ "x").outputl() == "1.234567890123499x");
		assert((var(1.234567890123500) ^ "x").outputl() == "1.2345678901235x");//00
		assert((var(1.234567890123501) ^ "x").outputl() == "1.234567890123501x");
		assert((var(1.234567890123502) ^ "x").outputl() == "1.234567890123502x");
		assert((var(1.234567890123503) ^ "x").outputl() == "1.234567890123503x");
		assert((var(1.234567890123504) ^ "x").outputl() == "1.234567890123504x");
		assert((var(1.234567890123505) ^ "x").outputl() == "1.234567890123505x");
		assert((var(1.234567890123506) ^ "x").outputl() == "1.234567890123506x");
		assert((var(1.234567890123507) ^ "x").outputl() == "1.234567890123507x");
		assert((var(1.234567890123508) ^ "x").outputl() == "1.234567890123508x");
		assert((var(1.234567890123509) ^ "x").outputl() == "1.234567890123509x");
	}

	//default is (d)ouble thankfully not (f)loat
	printl(var(1e-11f));
	printl(var(1e-11));
	printl(var(1e-12f));
	printl(var(1e-12));
	printl(var(1e-13f));
	printl(var(1e-13));
	printl(var(1e-14f));
	printl(var(1e-14));
	//0.000000000009999999960041972
	//0.000000000009999999999999999
	//0.0000000000009999999960041972
	//0.000000000001
	//0.000000000000099999998245167
	//0.0000000000001
	//0.0000000000000099999998245167
	//0.00000000000001

	assert(var(1e-11).outputl() == "0.00000000001");
	assert(var(1e-12).outputl() == "0.000000000001");
	assert(var(1e-13).outputl() == "0.0000000000001");
	assert(var(1e-14).outputl() == "0");//comparing to zero is special (diff less than 0.0001)

	//assert(var(1e-11f).outputl().toString() == "0.000000000009999999960041972");
	//assert(var(1e-11f).outputl().toString() == "9.999999960041972e-12");
	assert(var(1e-11f).outputl().toString() == "0.000000000009999999960041972");
	printl(var(1e-11));

//#ifdef USE_TO_CHARS_G
//	assert(var(1e-11).outputl().toString() == "1e-11");
//#elif defined(USE_RYU)
//	assert(var(1e-11).outputl().toString() == "1e-11");
//#else
//	assert(var(1e-11).outputl().toString() == "9.999999999999999e-12");
//#endif
	assert(var(1e-12f).outputl().toString() == "0.0000000000009999999960041972");
	assert(var(1e-12 ).outputl().toString() == "0.000000000001");
	assert(var(1e-13f).outputl().toString() == "0.000000000000099999998245167");
	assert(var(1e-13 ).outputl().toString() == "0.0000000000001");
	assert(var(1e-14f).outputl().toString() == "0.0000000000000099999998245167");
	assert(var(1e-14 ).outputl().toString() == "0.00000000000001");

	//assert(var(1e-12f).outputl().toString() == "9.999999960041972e-13");
	//assert(var(1e-13f).outputl().toString() == "9.9999998245167e-14");
	//assert(var(1e-14f).outputl().toString() == "9.9999998245167e-15");

	assert(var(1e-12f).outputl().toString() == "0.0000000000009999999960041972");
	assert(var(1e-13f).outputl().toString() == "0.000000000000099999998245167");
	assert(var(1e-14f).outputl().toString() == "0.0000000000000099999998245167");

//#ifdef USE_TO_CHARS_G
//	assert(var(1e-12 ).outputl().toString() == "1e-12");
//	assert(var(1e-13 ).outputl().toString() == "1e-13");
//	assert(var(1e-14 ).outputl().toString() == "1e-14");
//#else
//	assert(var(1e-12 ).outputl().toString() == "1.0e-12");
//	assert(var(1e-13 ).outputl().toString() == "1.0e-13");
//	assert(var(1e-14 ).outputl().toString() == "1.0e-14");
//#endif

	for (int i=-1000; i<=1000; ++i) {
		std::stringstream ss;
		ss << i;
		assert((var(i) ^ "x").outputl() == ss.str() + "x");
	}

	printl("Checking -101.00 to +101.00 in steps of 0.01");
	var started = ostime();
	for (double d=-101.00; d<=101.00; d+=0.01) {
		std::stringstream ss;
		ss.precision(2);
		ss << std::fixed << d;
		var v {d};
		v=round(v,2);
		if ((v ^ "x") != ss.str() + "x") {
			TRACE(d)
			TRACE(ss.str())
			TRACE(v)
			assert((v ^ "x").outputl() == ss.str() + "x");
		}
	}
	var msecs = (ostime()-started)*1000;
	printl(msecs,"msecs");

	// 1/1 to 1/10^20
	for (const var i : range(0, 29)) {
		printl(i,var(pwr(10,-i)));
		printl(i,var(1/pwr(10,i)));
	}

	//assert((var(pwr(10,-26)) ^ "x").outputl() =="0.00000000000000000000000001x");//better calculation of 10^-26
	//assert((var(1/pwr(10,26)) ^ "x").outputl() =="0.000000000000000000000000009999999999999999x");//poorer calculation of 1/10^26
#ifdef USE_TO_CHARS_G
	assert((var(pwr(10,-26)) ^ "x").outputl() =="1e-26x");//better calculation of 10^-26
#else
	assert((var(pwr(10,-26)) ^ "x").outputl() =="1e-26x");//better calculation of 10^-26
#endif
	assert((var(1/pwr(10,26)) ^ "x").outputl() =="9.999999999999999e-27x");//poorer calculation of 1/10^26


	printl(((var(pwr(10,-26)) - var(1/pwr(10,26))) ^ "x"));
//#if defined(USE_RYU) or defined(USE_TO_CHARS_G)
//	assert(((var(pwr(10,-26)) - var(1/pwr(10,26))) ^ "x").outputl() == "1.4349296274686127e-42x");//ryu 
//#else
#ifdef USE_RYU
	assert(((var(pwr(10,-26)) - var(1/pwr(10,26))) ^ "x").outputl() == "1.4349296274686127e-42x");//sstream
#else
	assert(((var(pwr(10,-26)) - var(1/pwr(10,26))) ^ "x").outputl() == "1.434929627468613e-42x");//sstream
#endif
//#endif
	assert(var(pwr(10,-26)).outputl() == var(1/pwr(10,26)));

	//assert((var(pwr(10,-26)) ^ "x")  == "0.00000000000000000000000001x");
	//assert((var(1/pwr(10,26)) ^ "x").outputl() == "0.000000000000000000000000009999999999999999x");
	//assert((var(pwr(10,-27)) ^ "x")  == "0.000000000000000000000000001x");

#ifdef USE_TO_CHARS_G
	assert((var(pwr(10,-26)) ^ "x").output()  == "1e-26x");
	assert((var(pwr(10,-27)) ^ "x").output()  == "1e-27x");
#else
	assert((var(pwr(10,-26)) ^ "x").output()  == "1e-26x");
	assert((var(pwr(10,-27)) ^ "x").output()  == "1e-27x");
#endif
	assert((var(1/pwr(10,26)) ^ "x").outputl() == "9.999999999999999e-27x");

	{
		assert(var(1000.1)       .outputl().toString() == "1000.1");
		assert(var(1000)         .outputl().toString() == "1000");
		assert(var(10'000'000)   .outputl().toString() == "10000000");

//#ifdef USE_TO_CHARS_G
//		assert(var(0.0001)       .outputl().toString() == "1e-04");
//		assert(var(0.000'000'01) .outputl().toString() == "1e-08");
//		assert(var(-0.000'000'01).outputl().toString() == "-1e-08");
//		assert(var(-0.0001)      .outputl().toString() == "-1e-04");
//#else
//		assert(var(0.0001)       .outputl().toString() == "0.0001");
//		assert(var(0.000'000'01) .outputl().toString() == "1.0e-08");
//		assert(var(-0.000'000'01).outputl().toString() == "-1.0e-08");
//		assert(var(-0.0001)      .outputl().toString() == "-0.0001");
//#endif
//
		assert(var(0.0001)       .outputl().toString() == "0.0001");
		assert(var(0.000'000'01) .outputl().toString() == "0.00000001");
		assert(var(-0.000'000'01).outputl().toString() == "-0.00000001");
		assert(var(-0.0001)      .outputl().toString() == "-0.0001");

		assert(var(-1000.1)      .outputl().toString() == "-1000.1");
		assert(var(-1000)        .outputl().toString() == "-1000");
		assert(var(-10'000'000)  .outputl().toString() == "-10000000");
	}

	{
		assert(var("0") .round(10).outputl().squote() eq "'0.0000000000'");
		assert(var("0") .round(10).outputl().squote() eq "'0.0000000000'");
		assert(var("0.").round(10).outputl().squote() eq "'0.0000000000'");
		assert(var(".0").round(10).outputl().squote() eq "'0.0000000000'");

		assert(var(0) .round(10).outputl().squote() eq "'0.0000000000'");
		assert(var(0) .round(10).outputl().squote() eq "'0.0000000000'");
		assert(var(0.).round(10).outputl().squote() eq "'0.0000000000'");
		assert(var(.0).round(10).outputl().squote() eq "'0.0000000000'");

	}

	printl("Test passed");

	return 0;
}

subroutine out() {
		printl("code  : ", dv2);
		std::cout << "cout  :  " << std::fixed << std::setprecision(50) << dd1 << std::endl;
		printl("print : ", dv1);
		printl("concat: ", sv1);
		printl("Target: ", sv2);
}

function test2(const std::string str1, const std::string str2 = std::string()) {
	dd1 = stod(str1);
	dv1 = dd1;
	dv2 = str1;
	sv1 = dv1 ^ "x";
	var pad = (dd1>=0) ? " " : "";
	print(pad,str1,"->",pad,sv1,"target:",pad,str2);
	//printf(" printf:  %.17g\n", dd1);
	//print(std::format("{:10g}", dd1);
	osflush();
	//gosub out();

	if (str2.size()) {
		//assert(sv1 == str2);
		if (sv1 == str2) {
			printl(""," PASS");

			//also test negative
			if (str1[0] != '-' and str1[0] != '+') {
				if (var(str1).toDouble()) {
					//not zero
					return test2("-" + str1, "-" + str2);
				} else {
					//negative zero
					return test2("-" + str1, str2);
				}
			}

		} else {
			printl(" FAIL");
			return false;
		}
	} else
		printl(" UNTESTED");

	return true;
}

programexit()
