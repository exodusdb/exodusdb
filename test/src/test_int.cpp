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
			TRACE(0xffffffffffffffff);	// minus 1
			assert(false);
		} catch (VarNumOverflow e) {
			printl(e.description);
		}

		TRACE(0x7fffffffffffffff);	// max int64

		try {
			TRACE(0x8000000000000000);	// min int64
			assert(false);
		} catch (VarNumOverflow e) {
			printl(e.description);
		}

		try {
			var i64 = 0xefffffffffffffff;
			i64.dump();
			TRACE(i64);
			assert(false);
		} catch (VarNumOverflow e) {
			printl(e.description);
		}

		{  //min int64

			TRACE(std::numeric_limits<int64_t>::min())
			//var i64 = -9223372036854775808L;
			var i64_min = std::numeric_limits<int64_t>::min();
			TRACE(i64_min)
			i64_min.dump();
			//assert(i64_min.toInt() eq -9223372036854775808);
			printl("i64_min.toInt() =", i64_min.toInt());
			//assert(i64_min.toInt()    eq std::numeric_limits<int64_t>::min());
			assert(i64_min.toInt64()    eq std::numeric_limits<int64_t>::min());
			assert(i64_min.toString() eq "-9223372036854775808");

			try {
				i64_min--;
				assert(false and var("i64_min--"));
				i64_min.dump();
			} catch (VarNumUnderflow e) {
				printl("Successfully caught deliberate error", e.description);
			}
		}

		{  //max int(32)

			TRACE(std::numeric_limits<int32_t>::max())
			//var i32 = -9223372036854775808L;
			var i32_max = std::numeric_limits<int32_t>::max();
			TRACE(i32_max)
			i32_max.dump();
			assert(i32_max.toInt() eq 2147483647);
			assert(i32_max.toInt()    eq std::numeric_limits<int32_t>::max());
			assert(i32_max.toString() eq "2147483647");

			assert(static_cast<varint_t>(++i32_max) == 2147483648);
		}
		{  //min int(32)

			TRACE(std::numeric_limits<int32_t>::min())
			//var i32 = -9223372036854775808L;
			var i32_min = std::numeric_limits<int32_t>::min();
			TRACE(i32_min)
			i32_min.dump();
			assert(i32_min.toInt() eq -2147483648);
			printl("i32_min.toInt() =", i32_min.toInt());
			assert(i32_min.toInt()    eq std::numeric_limits<int32_t>::min());
			assert(i32_min.toString() eq "-2147483648");

			assert(static_cast<varint_t>(--i32_min) == -2147483649);
		}

		{  //max int64

			TRACE(std::numeric_limits<int64_t>::max())
			//var i64 = -9223372036854775808L;
			var i64_max = std::numeric_limits<int64_t>::max();
			TRACE(i64_max)
			i64_max.dump();
			//assert(i64_max.toInt() eq -9223372036854775808);
			//assert(i64_max.toInt()    eq std::numeric_limits<int64_t>::max());
			assert(i64_max.toInt64()    eq std::numeric_limits<int64_t>::max());
			assert(i64_max.toString() eq "9223372036854775807");

			try {
				i64_max++;
				assert(false and var("i64_max++"));
				i64_max.dump();
			} catch (VarNumOverflow e) {
				printl("Successfully caught deliberate error", e.description);
			}
		}

		printl();
		var x = "21112000001";
		TRACE(x)
		x.dump();
		//assert(x.toInt()    eq 21'112'000'001);
		assert(x.toInt64()    eq 21'112'000'001);
		assert(x.toString() eq "21112000001");
		x.dump();
		TRACE(x.toInt());

		printl();
		var y = 21112000001;
		TRACE(y)
		y = 21112;
		y *= 1000000;
		y += 1;
		y.dump();
		//assert(y.toInt()    eq 21'112'000'001);
		assert(y.toInt64()    eq 21'112'000'001);
		assert(y.toString() eq "21112000001");
		y.dump();
		TRACE(y);

		printl();
		var z = 2;
		TRACE(z)
		z = z.pwr(4);
		TRACE(z);
		z.dump();

		printl();
		var maxint = 0x7fff'ffff'ffff'ffff;
		TRACE(maxint.toInt64())
		maxint.dump();
		assert(maxint.toInt64()    eq 9'223'372'036'854'775'807);
		assert(maxint.toString() eq "9223372036854775807");

		//throws
		//maxint++;

		TRACE(maxint + 1);
		TRACE(maxint * 2);

		//should throw
		maxint *= 2;
		maxint += 2;
	}

	{
		printl();
		printl("Check can convert string of largest int '9223372036854775807' to actual integer");
		var x = "9223372036854775807";
		x.dump();
		assert(x.toInt64() eq (9223372036854775000 + 807));
		x.dump();
	}

	{
		printl();
		printl("Check can convert string of minimum int '9223372036854775807' to actual integer");
		var x = "-9223372036854775808";
		x.dump();
		assert(x.toInt64() eq -9223372036854775000 - 808);
		x.dump();
	}

	{
		printl();
		printl("Ensure int less than min int is non-numeric");
		var x = "-9223372036854775809";
		x.dump();
		try {
			assert(x.toInt64());
			assert(false);	//should not get here
		} catch (VarNonNumeric e) {
			printl("OK", e.description);
		}
		x.dump();
	}

	{
		printl();
		printl("Ensure int more than max int is non-numeric");
		var x = "9223372036854775808";
		x.dump();
		try {
			assert(x.toInt64());
			assert(false);	//should not get here
		} catch (VarNonNumeric e) {
			printl("OK", e.description);
		}
		x.dump();
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()

