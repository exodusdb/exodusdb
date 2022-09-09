#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>
#include <iomanip> // for setprecision
#include <cmath> // for nextafter

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
programinit()

// Function to generate code to be asserted
function print_assert(double d1, in /*v2*/) {

	printl("assert(var(", d1, ").squote().outputl() eq ", var(d1).squote().quote(), ");");
	return true;
}

function main() {
	printl("test_precision2 says 'Hello World!'");

	// Code generator
	{
		print_assert(1.0E-200, var(1E-200));
		print_assert(1.0E-101, var(1E-101));
		print_assert(1.0E-100, var(1E-100));
		print_assert(1.0E-99, var(1E-99));
		print_assert(1.0E-10, var(1E-10));
		print_assert(1.0E-9, var(1E-9));
		print_assert(1.0E-8, var(1E-8));
		print_assert(1.0E-7, var(1E-7));
		print_assert(1.0E-6, var(1E-6));
		print_assert(1.0E-5, var(1E-5));
		print_assert(1.0E-4, var(1E-4));
		print_assert(1.0E-3, var(1E-3));
		print_assert(1.0E-2, var(1E-2));
		print_assert(1.0E-1, var(1E-1));
		print_assert(1.0E-0, var(1E-0));
		print_assert(1.0E0, var(1E0));
		print_assert(1.0E1, var(1E1));
		print_assert(1.0E2, var(1E2));
		print_assert(1.0E3, var(1E3));
		print_assert(1.0E4, var(1E4));
		print_assert(1.0E5, var(1E5));
		print_assert(1.0E6, var(1E6));
		print_assert(1.0E7, var(1E7));
		print_assert(1.0E8, var(1E8));
		print_assert(1.0E9, var(1E9));
		print_assert(1.0E10, var(1E10));
		print_assert(1.0E99, var(1E99));
		print_assert(1.0E100, var(1E100));
		print_assert(1.0E101, var(1E101));
		print_assert(1.0E200, var(1E200));
	}

	{	// generated code pasted back in

		assert(var( 1e-200 ).squote().outputl() eq  "'1e-200'" );
		assert(var( 1e-101 ).squote().outputl() eq  "'1e-101'" );
		assert(var( 1e-100 ).squote().outputl() eq  "'1e-100'" );
		assert(var( 1e-99 ).squote().outputl() eq  "'1e-99'" );
		assert(var( 1e-10 ).squote().outputl() eq  "'0.0000000001'" );
		assert(var( 1e-09 ).squote().outputl() eq  "'0.000000001'" );
		assert(var( 1e-08 ).squote().outputl() eq  "'0.00000001'" );
		assert(var( 1e-07 ).squote().outputl() eq  "'0.0000001'" );
		assert(var( 1e-06 ).squote().outputl() eq  "'0.000001'" );
		assert(var( 1e-05 ).squote().outputl() eq  "'0.00001'" );
		assert(var( 0.0001 ).squote().outputl() eq  "'0.0001'" );

		assert(var( 0.001 ).squote().outputl() eq  "'0.001'" );
		assert(var( 0.01 ).squote().outputl() eq  "'0.01'" );
		assert(var( 0.1 ).squote().outputl() eq  "'0.1'" );

		assert(var( 1.0 ).squote().outputl() eq  "'1'" );
		assert(var( 10.0 ).squote().outputl() eq  "'10'" );
		assert(var( 100.0 ).squote().outputl() eq  "'100'" );
		assert(var( 1000.0 ).squote().outputl() eq  "'1000'" );
		assert(var( 10000.0 ).squote().outputl() eq  "'10000'" );
		assert(var( 100000.0 ).squote().outputl() eq  "'100000'" );

		assert(var( 1 ).squote().outputl() eq  "'1'" );
		assert(var( 10 ).squote().outputl() eq  "'10'" );
		assert(var( 100 ).squote().outputl() eq  "'100'" );
		assert(var( 1000 ).squote().outputl() eq  "'1000'" );
		assert(var( 10000 ).squote().outputl() eq  "'10000'" );

#ifdef USE_TO_CHARS_G
		printl();
		//printl("DIFFERENT FORMATTING FOR INT AND DOUBLE?! to_chars(in) to_chars(double)");
		//assert(var( 100000.0 ).squote().outputl() eq  "'1e+05'" );
		assert(var( 100000.0 ).squote().outputl() eq  "'100000'" );
		assert(var( 100000 ).squote().outputl() eq  "'100000'" );

		printl();
#else
		assert(var( 100000 ).squote().outputl() eq  "'100000'" );
#endif

		assert(var( 1e+06 ).squote().outputl() eq  "'1000000'" );
		assert(var( 1e+07 ).squote().outputl() eq  "'10000000'" );
		assert(var( 1e+08 ).squote().outputl() eq  "'100000000'" );
		assert(var( 1e+09 ).squote().outputl() eq  "'1000000000'" );
		assert(var( 1e+10 ).squote().outputl() eq  "'10000000000'" );
		assert(var( 1e+99 ).squote().outputl() eq  "'1e+99'" );
		assert(var( 1e+100 ).squote().outputl() eq  "'1e+100'" );
		assert(var( 1e+101 ).squote().outputl() eq  "'1e+101'" );
		assert(var( 1e+200 ).squote().outputl() eq  "'1e+200'" );

	}

	{
		// other tests
		assert(var( "1000000000000000" ).isnum());
		assert(var( "10000000000000000" ).isnum());
		assert(var( "100000000000000000" ).isnum());
		assert(var( "1000000000000000000" ).isnum()); //1e18 is ok for input

		printl(var( "1000000000000000000" ).toDouble()); //1e18
		//printl(var( "10000000000000000000" ).toDouble()); //1e19 non-numeric

		assert(!var( "10000000000000000000" ).isnum());
		assert(!var( "100000000000000000000000000000" ).isnum());
		assert(!var( "1000000000000000000000000000000000000000" ).isnum());
		assert(!var( "10000000000000000000000000000000000000000000000000" ).isnum());

		assert(!var( "1000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000" ).isnum());
		assert(!var( "10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000" ).isnum());
		assert(!var( "100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000" ).isnum());
		assert(!var( "100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000" ).isnum());
	}

	{
		printl("Exact decimal representation of 0.3 is not exactly 0.3");
		std::cout << "0.3 approx. " << std::fixed << std::setprecision(100) << 0.3 << std::endl;
		std::cout << "0.3 next    " << std::fixed << std::setprecision(100) << std::nextafter(0.3, 0.4) << std::endl;
		std::cout << "jump        " << std::fixed << std::setprecision(100) << std::nextafter(0.3, 0.4) - 0.3 << std::endl;
	}

	{
		printl("Exact decimal representation of 0.1 is not exactly 0.3");
		std::cout << "0.1 approx. " << std::fixed << std::setprecision(100) << 0.1 << std::endl;
		std::cout << "0.1 next    " << std::fixed << std::setprecision(100) << std::nextafter(0.1, 0.0) << std::endl;
		std::cout << "jump       " << std::fixed << std::setprecision(100) << std::nextafter(0.1, 0.0) - 0.1 << std::endl;
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
