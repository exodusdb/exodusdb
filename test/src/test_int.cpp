#include <cassert>
#include <limits>

#include <exodus/program.h>
programinit()

function main() {
	printl("p1 says 'Hello World!'");

	{

		//TRACE(65536L*65536L*65536L*65536L) //overflow = 0
		TRACE(65536L*65536L*65536L)

		TRACE(0xffffffffffffffff); // minus 1
		TRACE(0x7fffffffffffffff); // max int64
		TRACE(0x8000000000000000); // min int64

		var i64 = 0xefffffffffffffff;
		i64.dump();
		TRACE(i64);

		{ //min int

			TRACE(std::numeric_limits<int64_t>::min())
			//var i64 = -9223372036854775808L;
			var i64_min = std::numeric_limits<int64_t>::min();
			TRACE(i64_min)
			i64_min.dump();
			//assert(i64_min.toInt() eq -9223372036854775808);
			assert(i64_min.toInt() eq std::numeric_limits<int64_t>::min());
			assert(i64_min.toString() eq "-9223372036854775808");

			try {
				i64_min--;
				assert(false and var("i64_min--"));
				i64_min.dump();
			} catch(MVIntUnderflow e) {
				printl("Successfully caught deliberate error", e.description);
			}

		}

		{ //max int

			TRACE(std::numeric_limits<int64_t>::max())
			//var i64 = -9223372036854775808L;
			var i64_max = std::numeric_limits<int64_t>::max();
			TRACE(i64_max)
			i64_max.dump();
			//assert(i64_max.toInt() eq -9223372036854775808);
			assert(i64_max.toInt() eq std::numeric_limits<int64_t>::max());
			assert(i64_max.toString() eq "9223372036854775807");

			try {
				i64_max++;
				assert(false and var("i64_max++"));
				i64_max.dump();
			} catch(MVIntOverflow e) {
				printl("Successfully caught deliberate error", e.description);
			}

		}

		printl();
		var x = "21112000001";
		TRACE(x)
		x.dump();
		assert(x.toInt() eq 21112000001);
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
		assert(y.toInt() eq 21112000001);
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
		var maxint = 0x7fffffffffffffff;
		TRACE(maxint.toInt())
		maxint.dump();
		assert(maxint.toInt() eq 9223372036854775807);
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
		assert(x.toInt() == (9223372036854775000 + 807));
		x.dump();
	}

	{
		printl();
		printl("Check can convert string of minimum int '9223372036854775807' to actual integer");
		var x = "-9223372036854775808";
		x.dump();
		assert(x.toInt() == -9223372036854775000-808);
		x.dump();
	}

	{
		printl();
		printl("Ensure int less than min int is non-numeric");
		var x = "-9223372036854775809";
		x.dump();
		try {
			assert(x.toInt());
			assert(false);//should not get here
		} catch (MVNonNumeric e) {
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
			assert(x.toInt());
			assert(false);//should not get here
		} catch (MVNonNumeric e) {
			printl("OK", e.description);
		}
		x.dump();
	}

	printl("Test passed");

	return 0;
}

programexit()

