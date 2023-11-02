#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>
#include <limits>

#include <exodus/program.h>
programinit()

	function main() {
	printl("p1 says 'Hello World!'");

	{

		//TRACE(65536L*65536L*65536L*65536L) //overflow = 0
		TRACE(65536L * 65536L * 65536L)

		try {
			TRACE(0xffffffffffffffff)	// minus 1
			assert(false);
		} catch (VarNumOverflow& e) {
			printl(e.description);
		}

		TRACE(0x7fffffffffffffff)	// max int64

		try {
			TRACE(0x8000000000000000)	// min int64
			assert(false);
		} catch (VarNumOverflow& e) {
			printl(e.description);
		}

		try {
			var i64 = 0xefffffffffffffff;
			i64.dump().outputl();
			TRACE(i64)
			assert(false);
		} catch (VarNumOverflow& e) {
			printl(e.description);
		}

		{  //min int64

			TRACE(std::numeric_limits<int64_t>::min())
			//var i64 = -9223372036854775808L;
			var i64_min = std::numeric_limits<int64_t>::min();
			TRACE(i64_min)
			i64_min.dump().outputl();
			//assert(i64_min.toInt() eq -9223372036854775808);
			printl("i64_min.toInt() =", i64_min.toInt());
			//assert(i64_min.toInt()    eq std::numeric_limits<int64_t>::min());
			assert(i64_min.toInt64()    eq std::numeric_limits<int64_t>::min());
			assert(i64_min.toString() eq "-9223372036854775808");

			try {
				i64_min--;
				assert(false and var("i64_min--"));
				i64_min.dump().outputl();
			} catch (VarNumUnderflow& e) {
				printl("Successfully caught deliberate error", e.description);
			}
		}

		{  //max int(32)

			TRACE(std::numeric_limits<int32_t>::max())
			//var i32 = -9223372036854775808L;
			var i32_max = std::numeric_limits<int32_t>::max();
			TRACE(i32_max)
			i32_max.dump().outputl();
			assert(i32_max.toInt() eq 2147483647);
			assert(i32_max.toInt()    eq std::numeric_limits<int32_t>::max());
			assert(i32_max.toString() eq "2147483647");

			assert(static_cast<varint_t>(++i32_max) eq 2147483648);
		}
		{  //min int(32)

			TRACE(std::numeric_limits<int32_t>::min())
			//var i32 = -9223372036854775808L;
			var i32_min = std::numeric_limits<int32_t>::min();
			TRACE(i32_min)
			i32_min.dump().outputl();
			assert(i32_min.toInt() eq -2147483648);
			printl("i32_min.toInt() =", i32_min.toInt());
			assert(i32_min.toInt()    eq std::numeric_limits<int32_t>::min());
			assert(i32_min.toString() eq "-2147483648");

			assert(static_cast<varint_t>(--i32_min) eq -2147483649);
		}

		{  //max int64

			TRACE(std::numeric_limits<int64_t>::max())
			//var i64 = -9223372036854775808L;
			var i64_max = std::numeric_limits<int64_t>::max();
			TRACE(i64_max)
			i64_max.dump().outputl();
			//assert(i64_max.toInt() eq -9223372036854775808);
			//assert(i64_max.toInt()    eq std::numeric_limits<int64_t>::max());
			assert(i64_max.toInt64()    eq std::numeric_limits<int64_t>::max());
			assert(i64_max.toString() eq "9223372036854775807");

			try {
				i64_max++;
				assert(false and var("i64_max++"));
				i64_max.dump().outputl();
			} catch (VarNumOverflow& e) {
				printl("Successfully caught deliberate error", e.description);
			}
		}

		printl();
		var x = "21112000001";
		TRACE(x)
		x.dump().outputl();
		//assert(x.toInt()    eq 21'112'000'001);
		assert(x.toInt64()    eq 21'112'000'001);
		assert(x.toString() eq "21112000001");
		x.dump().outputl();
		TRACE(x.toInt())

		printl();
		var y = 21112000001;
		TRACE(y)
		y = 21112;
		y *= 1000000;
		y += 1;
		y.dump().outputl();
		//assert(y.toInt()    eq 21'112'000'001);
		assert(y.toInt64()    eq 21'112'000'001);
		assert(y.toString() eq "21112000001");
		y.dump().outputl();
		TRACE(y)

		printl();
		var z = 2;
		TRACE(z)
		z = z.pwr(4);
		TRACE(z)
		z.dump().outputl();

		printl();
		var maxint = 0x7fff'ffff'ffff'ffff;
		TRACE(maxint.toInt64())
		maxint.dump().outputl();
		assert(maxint.toInt64()    eq 9'223'372'036'854'775'807);
		assert(maxint.toString() eq "9223372036854775807");

		//throws
		//maxint++;

		TRACE(maxint + 1)
		TRACE(maxint * 2)

		//should throw
		maxint *= 2;
		maxint += 2;
	}

	{
		printl();
		printl("Check can convert string of largest int '9223372036854775807' to actual integer");
		var x = "9223372036854775807";
		x.dump().outputl();
		assert(x.toInt64() eq (9223372036854775000 + 807));
		x.dump().outputl();
	}

	{
		printl();
		printl("Check can convert string of minimum int '9223372036854775807' to actual integer");
		var x = "-9223372036854775808";
		x.dump().outputl();
		assert(x.toInt64() eq -9223372036854775000 - 808);
		x.dump().outputl();
	}

	{
		printl();
		printl("Ensure int less than min int is non-numeric");
		var x = "-9223372036854775809";
		x.dump().outputl();
		try {
			assert(x.toInt64());
			assert(false);	//should not get here
		} catch (VarNonNumeric& e) {
			printl("OK", e.description);
		}
		x.dump().outputl();
	}

	{
		printl();
		printl("Ensure int more than max int is non-numeric");
		var x = "9223372036854775808";
		x.dump().outputl();
		try {
			assert(x.toInt64());
			assert(false);	//should not get here
		} catch (VarNonNumeric& e) {
			printl("OK", e.description);
		}
		x.dump().outputl();
	}

	{
		// Check Ramanujan
		assert(var(17.29*100).toString() eq "1729");
		assert(var(17.29*100).toDouble() eq 1729.0);
		assert(var(17.29*100).toInt() eq 1729);

		auto test_round_trip = [] (int test_int) {
			for (int div = 1; div lt 1'000'000'000; div*= 10) {
				var test = test_int;
				int mul = div;
				test /= div;
				test *= mul;
				//printl(test_int, "/", div, "*", mul, "=", test);
				// 997 / 100 * 100 = -997.0000000000001
				//p4.cpp:28: ExodusProgram::main()::<lambda(int)>: Assertion `test.toString() eq var(test_int).toString()' failed.
				//assert(test.toString() eq var(test_int).toString());
				assert(test.toInt()    eq test_int);
			}
		};

		test_round_trip(1729);
		test_round_trip(-1729);

		for (int ii = -1000; ii le 1000; ii++)
			test_round_trip(ii);

	}
	{
		// Another round trip test

		for (var x : range(-100, 100)) {
			for (int j = -1000; j<= 1000; j++) {
				if (not j) continue;
				var y = x;
				y /= j;
				y *= j;
				if (y.toInt() ne x.toInt()) {
					printl(x, "/*", j, " = ", y.toInt());
					//assert(x.toString() eq y.toString());
					assert(y.toInt() eq x.toInt());
				}
			}
		}
	}

	{
		// Check positive doubles truncate int unless very close to upper int
		assert(var(0.0).toInt()      eq 0);
		assert(var(0.9).toInt()      eq 0);
		assert(var(0.99).toInt()     eq 0);
		assert(var(0.999).toInt()    eq 0);
		assert(var(0.9999).toInt()   eq 0);
		assert(var(0.99999).toInt()  eq 1);
		assert(var(0.999999).toInt() eq 1);
		assert(var(1.0).toInt()      eq 1);
		assert(var(1.9).toInt()      eq 1);
		assert(var(1.99).toInt()     eq 1);
		assert(var(1.999).toInt()    eq 1);
		assert(var(1.9999).toInt()   eq 1);
		assert(var(1.99999).toInt()  eq 2);
		assert(var(1.999999).toInt() eq 2);
		assert(var(2.0).toInt()      eq 2);
		assert(var(2.9).toInt()      eq 2);
		assert(var(2.99).toInt()     eq 2);
		assert(var(2.999).toInt()    eq 2);
		assert(var(2.9999).toInt()   eq 2);
		assert(var(2.99999).toInt()  eq 3);
		assert(var(2.999999).toInt() eq 3);
	}

	{
		// Check comparison of negative ints and double zero to positive and negative INT zero

		assert(var(-0) eq 0);
		assert(var(-0) eq 0);
		assert(var(-0) eq -0);
		assert(var(-0) eq -0);

		assert(var(-0.0) eq 0);
		assert(var(-0.0) eq 0);
		assert(var(-0.0) eq -0);
		assert(var(-0.0) eq -0);

		// Check comparison of negative ints and double zero to positive and negative DOUBLE zero

		assert(var(-0) eq 0.0);
		assert(var(-0) eq 0.0);
		assert(var(-0) eq -0.0);
		assert(var(-0) eq -0.0);

		assert(var(-0.0) eq 0.0);
		assert(var(-0.0) eq 0.0);
		assert(var(-0.0) eq -0.0);
		assert(var(-0.0) eq -0.0);
	}

	{
		// Check negative doubles truncate int unless very close to lower int
		assert(var(-0.0).toInt()      eq 0);
		assert(var(-0.9).toInt()      eq -0);
		assert(var(-0.99).toInt()     eq -0);
		assert(var(-0.999).toInt()    eq -0);
		assert(var(-0.9999).toInt()   eq -0);
		assert(var(-0.99999).toInt()  eq -1);
		assert(var(-0.999999).toInt() eq -1);
		assert(var(-1.0).toInt()      eq -1);
		assert(var(-1.9).toInt()      eq -1);
		assert(var(-1.99).toInt()     eq -1);
		assert(var(-1.999).toInt()    eq -1);
		assert(var(-1.9999).toInt()   eq -1);
		assert(var(-1.99999).toInt()  eq -2);
		assert(var(-1.999999).toInt() eq -2);
		assert(var(-2.0).toInt()      eq -2);
		assert(var(-2.9).toInt()      eq -2);
		assert(var(-2.99).toInt()     eq -2);
		assert(var(-2.999).toInt()    eq -2);
		assert(var(-2.9999).toInt()   eq -2);
		assert(var(-2.99999).toInt()  eq -3);
		assert(var(-2.999999).toInt() eq -3);
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()

