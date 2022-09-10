#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>

programinit()

function main() {


	{
		try {
			//x = 18446744073709551614ULL;
			var x = 18446744073709551614ULL;
			TRACE(x);
			assert(x == -2);
	//		assert(x == 18446744073709551614ULL)
		} catch (VarIntOverflow e) {
			errputl(e.description);
		}
	}

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

		printl("Implicit conversion to all types of signed ints.");

		int i = x;
		std::cout << i << std::endl;
		assert(i eq -2);

		long int l = x;
		std::cout << l << std::endl;
		assert(l eq -2);

		long long int ll = x;
		std::cout << ll << std::endl;
		assert(ll eq -2);

		int64_t i64 = x;
		std::cout << i64 << std::endl;
		assert(i64 eq -2);

		printl("Verify cannot convert negative vars to unsigned int types");
		try {
			unsigned int ui = x;
			assert(false && ui);
		} catch(VarNonPositive e) {
		//} catch(VarIntUnderflow e) {
			errputl(e.description);
		}
//		std::cout << ui << std::endl;
//		assert(ui eq 4294967294);
//		assert(ui eq -2);// using an small negative, signed int, to set an unsigned int results in a very large number (2's complement)

		printl("Verify cannot convert negative vars to unsigned uint types");
		try {
			uint64_t ull = x;
			assert(false && ull);
		} catch(VarNonPositive e) {
		//} catch(VarIntUnderflow e) {
			errputl(e.description);
		}
//		std::cout << ull << std::endl;
//		assert(ull eq 18446744073709551614ULL);
//		assert(ull eq -2);// using an small negative, signed int, to set an unsigned int results in a very large number (2's complement)

		printl("Implicit conversion to double");
		double d = x;
		std::cout << d << std::endl;
		assert(d eq -2.89);

		printl("Implicit to char conversion is via var.toInt().");
		printl("WARNING: It doesnt take the first char of the string.");
		char c = x;
		std::cout << int(c) << std::endl;
		assert(c == -2);

		printl("Implicit conversion to char requires a numeric var");
		try {
			int c2 = var("xyz");
			// Should never get here
			assert(false);
			std::cout << int(c2) << std::endl;
			//assert(c2 ==?);
		} catch (VarNonNumeric e) {
			printl(e.description);
		}

		printl("Implicit conversion to std::string");
		std::string s = x;
		std::cout << s << std::endl;
		assert(s == "-2.89");

		assert(std::string(static_cast<const char*>(x)) eq "-2.89");

		// 
		//assert(std::string(static_cast<char*>(x)) eq "-2.89");

		{
			var xyz = "xyz";

			// doesnt compile because var doesnt have an implicit conversion to non-const char*
			//std::cout << (char*) xyz;

			std::cout << (const char*) xyz;
		}
		//assert(std::string(static_cast<char*>(x)) eq "-2.89");

	}

	{
		printl("Implicit conversion of non-negative numbers to various types of unsigned ints");

		var x = 4294967294;

		unsigned int ui = x;
		std::cout << "u int ui = 4294967294;\nui is " << ui << std::endl;
		assert(ui eq 4294967294);

		unsigned short usi = x;
		std::cout << "unsigned short usi = 4294967294;\nusi is " << usi << std::endl;
		//assert(usi eq 4294967294);
		assert(usi eq 65534);

		// following is questionable c/c++ behaviour allowing a comparison between signed and unsigned.
		// g++ doesnt always warn
		assert(ui == -2);

		uint64_t ull = x;
		std::cout << ull << std::endl;
		assert(ull eq 4294967294ULL);
		//assert(ull eq -2);// using an small negative, signed int, to set an unsigned int results in a very large number (2's complement)

	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()

