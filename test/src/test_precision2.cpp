#include <cassert>

#include <exodus/program.h>
programinit()

// Function to generate code to be asserted
function print_assert(double d1, in /*v2*/) {
	printl("assert(var(", d1, ") eq ", var(d1).quote(), ");");
	return true;
}

function main() {
	printl("test_precision2 says 'Hello World!'");

	print_assert(1E-200, var(1E-200));
	print_assert(1E-101, var(1E-101));
	print_assert(1E-100, var(1E-100));
	print_assert(1E-99, var(1E-99));
	print_assert(1E-10, var(1E-10));
	print_assert(1E-9, var(1E-9));
	print_assert(1E-8, var(1E-8));
	print_assert(1E-7, var(1E-7));
	print_assert(1E-6, var(1E-6));
	print_assert(1E-5, var(1E-5));
	print_assert(1E-4, var(1E-4));
	print_assert(1E-3, var(1E-3));
	print_assert(1E-2, var(1E-2));
	print_assert(1E-1, var(1E-1));
	print_assert(1E-0, var(1E-0));
	print_assert(1E0, var(1E0));
	print_assert(1E1, var(1E1));
	print_assert(1E2, var(1E2));
	print_assert(1E3, var(1E3));
	print_assert(1E4, var(1E4));
	print_assert(1E5, var(1E5));
	print_assert(1E6, var(1E6));
	print_assert(1E7, var(1E7));
	print_assert(1E8, var(1E8));
	print_assert(1E9, var(1E9));
	print_assert(1E10, var(1E10));
	print_assert(1E99, var(1E99));
	print_assert(1E100, var(1E100));
	print_assert(1E101, var(1E101));
	print_assert(1E200, var(1E200));

	assert(var( 1e-200 ) eq  "1e-200" );
	assert(var( 1e-101 ) eq  "1e-101" );
	assert(var( 1e-100 ) eq  "1e-100" );
	assert(var( 1e-99 ) eq  "1e-99" );
	assert(var( 1e-10 ) eq  "1e-10" );
	assert(var( 1e-09 ) eq  "1e-09" );
	assert(var( 1e-08 ) eq  "1e-08" );
	assert(var( 1e-07 ) eq  "1e-07" );
	assert(var( 1e-06 ) eq  "0.000001" );
	assert(var( 1e-05 ) eq  "0.00001" );
	assert(var( 0.0001 ) eq  "0.0001" );
	assert(var( 0.001 ) eq  "0.001" );
	assert(var( 0.01 ) eq  "0.01" );
	assert(var( 0.1 ) eq  "0.1" );
	assert(var( 1 ) eq  "1" );
	assert(var( 1 ) eq  "1" );
	assert(var( 10 ) eq  "10" );
	assert(var( 100 ) eq  "100" );
	assert(var( 1000 ) eq  "1000" );
	assert(var( 10000 ) eq  "10000" );
	assert(var( 100000 ) eq  "100000" );
	assert(var( 1e+06 ) eq  "1000000" );
	assert(var( 1e+07 ) eq  "10000000" );
	assert(var( 1e+08 ) eq  "100000000" );
	assert(var( 1e+09 ) eq  "1000000000" );
	assert(var( 1e+10 ) eq  "10000000000" );
	assert(var( 1e+19 ) eq  "1e19" );
	assert(var( 1e+29 ) eq  "1e29" );
	assert(var( 1e+39 ) eq  "1e39" );
	assert(var( 1e+49 ) eq  "1e49" );
	assert(var( 1e+99 ) eq  "1e99" );
	assert(var( 1e+100 ) eq  "1e100" );
	assert(var( 1e+101 ) eq  "1e101" );
	assert(var( 1e+200 ) eq  "1e200" );

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

	printl("Test passed");

	return 0;
}

programexit()

