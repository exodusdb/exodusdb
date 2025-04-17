#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

func main() {

	{
		var index;

		index = 8;
		assert(var("ababab").indexr("ab", index) eq 5);

		index = 7;
		assert(var("ababab").indexr("ab", index) eq 5);

		index = 6;
		assert(var("ababab").indexr("ab", index) eq 5);

		index = 5;
		assert(var("ababab").indexr("ab", index) eq 5);

		index = 4;
		assert(var("ababab").indexr("ab", index) eq 3);

		index = 3;
		assert(var("ababab").indexr("ab", index) eq 3);

		index = 2;
		assert(var("ababab").indexr("ab", index) eq 1);

		index = 1;
		assert(var("ababab").indexr("ab", index) eq 1);

		index = 0;
		assert(var("ababab").indexr("ab", index) eq 5);

		index = -1;
		assert(var("ababab").indexr("ab", index) eq 5);

		index = -2;
		assert(var("ababab").indexr("ab", index) eq 5);

		index = -3;
		assert(var("ababab").indexr("ab", index) eq 3);

		index = -4;
		assert(var("ababab").indexr("ab", index) eq 3);

		index = -5;
		assert(var("ababab").indexr("ab", index) eq 1);

		index = -6;
		assert(var("ababab").indexr("ab", index) eq 1);

		index = -7;
		assert(var("ababab").indexr("ab", index) eq 0);

		index = -8;
		assert(var("ababab").indexr("ab", index) eq 0);
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

}; // programexit()
