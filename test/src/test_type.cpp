#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>

programinit()

function main() {

	{
		// Named conversions

		var x = "-2.89";

		assert(x.toInt() eq -2);

		assert(x.toDouble() eq -2.89);

		// assert(x.toChar() eq "-"); //compiler warning
		assert(x.toChar() eq '-');

		assert(std::string(x.c_str()) eq "-2.89");

		assert(std::string(static_cast<const char*>(x)) eq "-2.89");

	}

	{
		// var has no implicit char conversion so it converts via an int instead
		var x = seq("a"); //what is the character number of "a"?
		TRACE(x)
		char c = x;
		assert(c == 'a');
	}

	{
		// Implicit conversions

		var x = "-2.89";

		int i = x;
		std::cout << i << std::endl;
		assert(i eq -2);

		unsigned int ui = x;
		std::cout << ui << std::endl;
		assert(ui eq 4294967294);
		assert(ui eq -2);// using an small negative, signed int, to set an unsigned int results in a very large number (2's complement)

		double d = x;
		std::cout << d << std::endl;
		assert(d eq -2.89);

		// var has no implicit char conversion so it converts via an int instead
		char c = x;
		std::cout << int(c) << std::endl;
		assert(c == -2);

		//automatic conversion to char requires a numeric var
		try {
			int c2 = var("xyz");
			// Should never get here
			assert(false);
			std::cout << int(c2) << std::endl;
			//assert(c2 ==?);
		} catch (VarNonNumeric e) {
			printl(e.description);
		}

		std::string s = x;
		std::cout << s << std::endl;
		assert(s == "-2.89");

		assert(std::string(static_cast<const char*>(x)) eq "-2.89");

	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()

