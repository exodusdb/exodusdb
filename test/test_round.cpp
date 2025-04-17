#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

func main() {
	{
		assert(oconv(1234.4449, "MD20P").squote().outputl()             eq "'1234.44'");
		assert(oconv(1234.4449999999999999, "MD20P").squote().outputl() eq "'1234.45'");

		assert(round(123449.0000000000000, -2).squote().outputl() eq "'123400'");

		assert(round(123449.0000000000000, -2).squote().outputl() eq "'123400'");
		assert(round(123449.9000000000000, -2).squote().outputl() eq "'123400'");

		// Unusual result
		//normal rounding would not round up 49.999 to 50 but it is so close to 50 that we do
		//assert(round(123449.9990000000000, -2).squote().outputl() eq "'123500'");
		//assert(round(123449.9999990000000, -2).squote().outputl() eq "'123500'");
		assert(round(123449.9999999999999, -2).squote().outputl() eq "'123500'");

		assert(round(123449.9990000000000, -2).squote().outputl() eq "'123400'");
		assert(round(123449.9999990000000, -2).squote().outputl() eq "'123400'");
		//assert(round(123449.9999999999999, -2).squote().outputl() eq "'123400'");

		assert(round(123456.7890000000000, -2).squote().outputl() eq "'123500'");
		assert(round(123450.0000000000000, -2).squote().outputl() eq "'123500'");
	}
	{
		assert(oconv(-1234.4449, "MD20P").squote().outputl()      eq "'-1234.44'");

		// Unusual result
		assert(oconv(-1234.4449999999999999, "MD20P").squote().outputl() eq "'-1234.45'");

		assert(round(-123449.0000000000000, -2).squote().outputl() eq "'-123400'");

		assert(round(-123449.0000000000000, -2).squote().outputl() eq "'-123400'");
		assert(round(-123449.9000000000000, -2).squote().outputl() eq "'-123400'");

		// Unusual result
		//normal rounding would not round up 49.999 to 50 but it is so close to 50 that we do
		//assert(round(-123449.9990000000000, -2).squote().outputl() eq "'-123500'");
		//assert(round(-123449.9999990000000, -2).squote().outputl() eq "'-123500'");
		assert(round(-123449.9999999999999, -2).squote().outputl() eq "'-123500'");

		assert(round(-123449.9990000000000, -2).squote().outputl() eq "'-123400'");
		assert(round(-123449.9999990000000, -2).squote().outputl() eq "'-123400'");
		//assert(round(-123449.9999999999999, -2).squote().outputl() eq "'-123400'");

		assert(round(-123456.7890000000000, -2).squote().outputl() eq "'-123500'");
		assert(round(-123450.0000000000000, -2).squote().outputl() eq "'-123500'");
	}

	{
		assert(oconv("1234.4449", "MD20P").squote().outputl()             eq "'1234.44'");
		// Unusual result
		assert(oconv("1234.4449999999999999", "MD20P").squote().outputl() eq "'1234.45'");

		assert(round("123449.0000000000000", -2).squote().outputl() eq "'123400'");

		assert(round("123449.0000000000000", -2).squote().outputl() eq "'123400'");
		assert(round("123449.9000000000000", -2).squote().outputl() eq "'123400'");

		// Unusual result
		//normal rounding would not round up 49.999 to 50 but it is so close to 50 that we do
		//assert(round("123449.9990000000000", -2).squote().outputl() eq "'-123500'");
		//assert(round("123449.9999990000000", -2).squote().outputl() eq "'-123500'");
		assert(round("123449.9999999999999", -2).squote().outputl() eq "'123500'");

		assert(round("123449.9990000000000", -2).squote().outputl() eq "'123400'");
		assert(round("123449.9999990000000", -2).squote().outputl() eq "'123400'");
		//assert(round("123449.9999999999999", -2).squote().outputl() eq "'123400'");

		assert(round("123456.7890000000000", -2).squote().outputl() eq "'123500'");
		assert(round("123450.0000000000000", -2).squote().outputl() eq "'123500'");
	}

	{

		assert(round(2.5)    eq 3);
		assert(round(2.4999) eq 2);
		assert(round(1.5)    eq 2);
		//assert(round(1.4999) eq 1);
		assert(round(1.499)  eq 1);
		assert(round(0.5)    eq 1);
		//assert(round(0.4999) eq 0);
		assert(round(0.499)  eq 0);
		assert(round(0.1)    eq 0);

		assert(round(-2.5)    eq - 3);
		assert(round(-2.4999) eq - 2);
		assert(round(-1.5)    eq - 2);
		//assert(round(-1.4999) eq -1);
		assert(round(-1.499)  eq - 1);
		assert(round(-0.5)    eq - 1);
		//assert(round(-0.4999) eq 0);
		assert(round(-0.499)  eq 0);
		assert(round(-0.1)    eq 0);

		//round

		assert(round(var("6000.50") / 20, 2).outputl()  eq 300.03);
		assert(round(var("-6000.50") / 20, 2).outputl() eq -300.03);
		assert(round(var(6000.50) / 20, 2).outputl()    eq 300.03);
		assert(round(var(-6000.50) / 20, 2).outputl()   eq -300.03);
		assert(round(var(6000.50f) / 20, 2).outputl()   eq 300.03);
		assert(round(var(-6000.50f) / 20, 2).outputl()  eq -300.03);
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

}; // programexit()
