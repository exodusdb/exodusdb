#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <iomanip>

#include <exodus/program.h>

programinit()

// Test for IMPLICIT round trip type -> var -> type
template<typename TYPE>
void test_min_max(std::string typetitle, int max_min = 0)
{
	// Test largest positive number round trips into/out of var

	if (max_min != 2) {
		TYPE max = std::numeric_limits<TYPE>::max();
		printl("\nTesting IMPLICIT conversion from and to", typetitle, max, "MAX");
		TYPE vmax = var(max);
		printl(vmax);
		/*TRACE(vmax);*/
		printl(vmax - max);
		std::cout << vmax << " + " << max << " = " << std::setprecision(std::numeric_limits<TYPE>::digits10 + 1) << vmax + max << std::endl;
		std::cout << vmax << " + " << max << " = " << std::setprecision(40) << vmax + max << std::endl;
		assert(vmax eq max);
	}

	if (max_min != 1) {
		// Test lowest negative number round trips into/out of var
		// NB
		// ::min is the smallest POSITIVE number
		// ::lowest is the greatest NEGATIVE number
		TYPE low = std::numeric_limits<TYPE>::lowest();
		printl("\nTesting IMPLICIT conversion from and to ", typetitle, low, "low");
		TYPE vlow = var(low);
		printl(vlow);
		/*TRACE(vlow);*/
		printl(vlow - low);
		std::cout << vlow << " + " << low << " = " << std::setprecision(std::numeric_limits<TYPE>::digits10 + 1) << vlow + low << std::endl;
		std::cout << vlow << " + " << low << " = " << std::setprecision(40) << vlow + low << std::endl;
		assert(vlow eq low);
	}
}

function main() {

	{
		try {
			//x = 18446744073709551614ULL;
			var x = 18446744073709551614ULL;
			TRACE(x);
			assert(x eq -2);
			//		assert(x eq 18446744073709551614ULL)
		} catch (VarNumOverflow e) {
			errputl(" thrown ", e.description);
		}
	}

	{
		// Named conversions

		var x = "-2.89";

		assert(x.toInt() eq - 2);

		assert(x.toDouble() eq - 2.89);

		// assert(x.toChar() eq "-"); //compiler warning
		assert(x.toChar() eq '-');

		assert(std::string(x.c_str()) eq "-2.89");

		assert(std::string(static_cast<const char*>(x)) eq "-2.89");
	}

	{
		// var has no IMPLICIT char conversion so it converts via an int instead
		var x = seq("a");  //what is the character number of "a"?
		TRACE(x)
		char c = x;
		assert(c eq 'a');
	}

	{
		// IMPLICIT conversions

		var x = "-2.89";

		printl("IMPLICIT conversion to all types of signed ints.");

		int i = x;
		std::cout << i << std::endl;
		assert(i eq - 2);

		long int l = x;
		std::cout << l << std::endl;
		assert(l eq - 2);

		long long int ll = x;
		std::cout << ll << std::endl;
		assert(ll eq - 2);

		int64_t i64 = x;
		std::cout << i64 << std::endl;
		assert(i64 eq - 2);

		printl("Verify cannot convert negative vars to unsigned int types");
		try {
			unsigned int ui = x;
			assert(false && ui);
		} catch (VarNonPositive e) {
			//} catch(VarNumUnderflow e) {
			errputl(" thrown ", e.description);
		}
		//		std::cout << ui << std::endl;
		//		assert(ui eq 4294967294);
		//		assert(ui eq -2);// using an small negative, signed int, to set an unsigned int results in a very large number (2's complement)

		printl("Verify cannot convert negative vars to unsigned uint types");
		try {
			uint64_t ull = x;
			assert(false && ull);
		} catch (VarNonPositive e) {
			//} catch(VarNumUnderflow e) {
			errputl(" thrown ", e.description);
		}
		//		std::cout << ull << std::endl;
		//		assert(ull eq 18446744073709551614ULL);
		//		assert(ull eq -2);// using an small negative, signed int, to set an unsigned int results in a very large number (2's complement)

		//std::numeric_limits<double>::max()
		{
			printl("\nIMPLICIT conversion from and to float");
			printl("WARNING conversion from and to float is inexact because var holds only DOUBLES");
			float f = var(-2.89f);
			TRACE(f)
			printl(f);
			printl(f + 2.89f);
			std::cout << "f + 2.89f = " << std::setprecision(std::numeric_limits<float>::digits10 + 1) << f + 2.89f << std::endl;
			assert(f eq -2.89f);

			printl("\nIMPLICIT conversion from and to double");
			printl("Note conversion from and to double is exact because var holds DOUBLES");
			double d = var(-2.89);
			TRACE(d)
			printl(d);
			std::cout << "d + 2.89 = " << std::setprecision(std::numeric_limits<double>::digits10 + 1) << d + 2.89 << std::endl;
			assert(d eq -2.89);

			printl("\nIMPLICIT conversion to from and to long double");
			printl("WARNING conversion from and to long double is inexact because var holds only DOUBLES");
			long double ld = var(-2.89L);
			TRACE(ld)
			printl(ld);
			std::cout << "ld + 2.89L = " << std::setprecision(std::numeric_limits<long double>::digits10 + 1) << ld + 2.89L << std::endl;
			/////////////////////
			assert(ld ne -2.89L);
			/////////////////////
			//assert(ld + 2.89L eq -1.242495689668388081e-16L);
			assert(ld + 2.89L eq -1.242495689668388081e-16);

		}

		{
			printl("IMPLICIT conversion from and to bool");

			bool t = var(true);
			assert(t == true);

			bool f = var(false);
			assert(f == false);
		}

		printl("IMPLICIT to char conversion is via var.toInt().");
		printl("WARNING: It doesnt take the first char of the string.");
		char c = x;
		std::cout << int(c) << std::endl;
		assert(c eq -2);

		printl("IMPLICIT conversion to char requires a numeric var");
		try {
			int c2 = var("xyz");
			// Should never get here
			assert(false);
			std::cout << int(c2) << std::endl;
			//assert(c2 ==?);
		} catch (VarNonNumeric e) {
			errputl(" thrown ", e.description);
		}

		printl("IMPLICIT conversion to std::string");
		std::string s = x;
		std::cout << s << std::endl;
		assert(s eq "-2.89");

		assert(std::string(static_cast<const char*>(x)) eq "-2.89");

		//
		//assert(std::string(static_cast<char*>(x)) eq "-2.89");

		{
			var xyz = "xyz";

			// doesnt compile because var doesnt have an IMPLICIT conversion to non-const char*
			//std::cout << (char*) xyz;

			std::cout << (const char*)xyz;
		}
		//assert(std::string(static_cast<char*>(x)) eq "-2.89");
	}

	{
		printl("IMPLICIT conversion of non-negative numbers to various types of unsigned ints");

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
		//assert(ui eq -2);

		uint64_t ull = x;
		std::cout << ull << std::endl;
		assert(ull eq 4294967294ULL);
		//assert(ull eq -2);// using an small negative, signed int, to set an unsigned int results in a very large number (2's complement)
	}

	{
		printl("\n===INTS===");

		//test_min_max<short>("char");
		test_min_max<short>("short");
		test_min_max<int>("int");
		test_min_max<long>("long");
		test_min_max<long long>("long long");
		test_min_max<int8_t>("int8_t");
		test_min_max<int16_t>("int16_6");
		test_min_max<int32_t>("int32_t");
		test_min_max<int64_t>("int64_t");

		test_min_max<unsigned short>("unsigned short");
		test_min_max<unsigned int>("unsigned int");
		//test_min_max<unsigned long>("unsigned long"); //too large for var's SIGNED int64_t
		//test_min_max<unsigned long long>("unsigned long long") // too large for var's SIGNED int64_t
		test_min_max<uint8_t>("uint8_t");
		test_min_max<uint16_t>("uint16_t");
		test_min_max<uint32_t>("uint32_t");
		//test_min_max<uint64_t>("unit64_t"); //too large var's SIGNED int64_t

//		test_min_max<int128_t>();
//		test_min_max<__int128>();

	}
	{
		printl("\n===INT OVERFLOW===");

		// unsigned long - Check too large var's SIGNED int64_t
		try {
			test_min_max<unsigned long>("unsigned long", 1);
			assert(false);
		}
		catch (VarNumOverflow e) {
			errputl(" thrown ", e.description);
		}

		// unsigned long int - Check too large var's SIGNED int64_t
		try {
			test_min_max<unsigned long long>("unsigned long long", 1);
			assert(false);
		}
		catch (VarNumOverflow e) {
			errputl(" thrown ", e.description);
		}

		// uint64_t - Check too large var's SIGNED int64_t
		try {
			test_min_max<uint64_t>("uint64_t", 1);
			assert(false);
		}
		catch (VarNumOverflow e) {
			errputl(" thrown ", e.description);
		}
	}
	{
		print("\n===INT UNDERFLOW===");

		printl("unsigned ints can never underflow on conversion to vars");

//		// unsigned long - Check too low negative var's SIGNED int64_t
//		try {
//			test_min_max<unsigned long>("unsigned long", 2);
//			assert(false);
//		}
//		catch (VarNumUnderflow e) {
//			errputl(" thrown ", e.description);
//		}
//
//		// unsigned long int - Check too low negative var's SIGNED int64_t
//		try {
//			test_min_max<unsigned long long>("unsigned long long", 2);
//			assert(false);
//		}
//		catch (VarNumUnderflow e) {
//			errputl(" thrown ", e.description);
//		}
//
//		// uint64_t - Check too low negative var's SIGNED int64_t
//		try {
//			test_min_max<uint64_t>("uint64_t", 2);
//			assert(false);
//		}
//		catch (VarNumUnderflow e) {
//			errputl(" thrown ", e.description);
//		}
	}
	{

		print("\n===FLOATS===");

		test_min_max<float>("float");
		test_min_max<double>("double");
		//test_min_max<long double>("long double"); // Can cause under and overflow

		// Verify long doubles can cause overflow

		//test_min_max<long double>("long double");
		// long double - Check too large for var's DOUBLE
		try {
			test_min_max<long double>("long double", 1 /*max*/);
			assert(false);
		}
		catch (VarNumOverflow e) {
			errputl(" thrown ", e.description);
		}

		// Verify long double can cause underflow

		// long double - Check too low negative for var's DOUBLE
		try {
			test_min_max<long double>("long double", 2 /*low*/);
			assert(false);
		}
		catch (VarNumUnderflow e) {
			errputl(" thrown ", e.description);
		}

	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
