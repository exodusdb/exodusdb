#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

//exodus/c++ gt uses exo_smallest_number to compare equality
////0.0001 for pick/arev compatibility

	//#include <l2.h>

	let exo_smallest_number = pwr(10, -getprecision());

func main() {

	TRACE(exo_smallest_number)

	var bigs = "10000000000000";
	var bigi = 100000000;
	var bigd = 1.0e13;
	//printl(bigs);
	//printl(bigi);
	//printl(bigd);

#define AREV_ROUNDING
#ifdef AREV_ROUNDING

	//var lits ="0.0000000000001";
	var lits = "0.0001";
	//var liti =1e-14;
	//var litd =1e-13;
	var litd = 1e-4;
	//printl(lits);
	//printl(litd);

	//var tinys="0.00000000000001";
	var tinys = "0.00001";
	//var tinyi=1e-14;
	var tinyd = 1e-5;
	//printl(tinys);
	//printl(tinyd);
#else
	//var lits ="0.0000000000001";
	var lits = "0.0000000001";
	//var liti =1e-14;
	//var litd =1e-13;
	var litd = 1e-10;
	//printl(lits);
	//printl(litd);

	//var tinys="0.00000000000001";
	var tinys = "0.00000000001";
	//var tinyi=1e-14;
	var tinyd = 1e-14;
	//printl(tinys);
	//printl(tinyd);
#endif

	var zeros = "0";
	var zeroi = 0;
	var zerod = 0.0;
	//printl(zeros);
	//printl(zeroi);
	//printl(zerod);

	//big <

	assert(not(bigs lt bigs));
	assert(bigs gt bigi);
	assert(not(bigs gt bigd));

	assert(bigi lt bigs);
	assert(not(bigi lt bigi));
	assert(bigi lt bigd);

	assert(not(bigd lt bigs));
	assert(bigd gt bigi);
	assert(not(bigd gt bigd));

	assert(bigs gt lits);
	assert(bigs gt litd);

	assert(bigs gt tinys);
	assert(bigs gt tinyd);

	assert(bigs eq bigs);
	assert(bigs ne bigi);
	assert(bigs eq bigd);

	assert(bigi ne bigs);
	assert(bigi eq bigi);
	assert(bigi ne bigd);

	assert(bigd eq bigs);
	assert(bigd ne bigi);
	assert(bigd eq bigd);

	assert(bigs ne lits);
	assert(bigs ne litd);

	assert(bigs ne tinys);
	assert(bigs ne tinyd);

	assert(bigi gt lits);
	assert(bigi gt litd);

	assert(bigi gt tinys);
	assert(bigi gt tinyd);

	assert(bigd gt lits);
	assert(bigd gt litd);
	assert(bigd gt tinys);
	assert(bigd gt tinyd);

	assert(bigs gt zeros);
	assert(bigs gt zeroi);
	assert(bigs gt zerod);

	assert(bigs gt 0);
	assert(bigi gt 0);
	assert(bigd gt 0);

	assert(lits gt 0);
	//assert(liti>0);
	assert(litd gt 0);

	assert(not(tinys gt 0));
	//assert(not(tinyi>0));
	assert(not(tinyd gt 0));

	assert(not(zeros gt 0));
	assert(not(zeroi gt 0));
	//printl(zerod);
	assert(not(zerod gt 0));

	assert(0 lt bigs);
	assert(0 lt bigi);
	assert(0 lt bigd);

	assert(0 lt lits);
	//assert(0<liti);
	assert(0 lt litd);

	assert(not(0 lt tinys));
	//assert(0<tinyi);
	assert(not(0 lt tinyd));

	assert(not(0 lt zeros));
	assert(not(0 lt zeroi));
	assert(not(0 lt zerod));

	assert(bigs ne 0);
	assert(bigi ne 0);
	assert(bigd ne 0);

	assert(lits ne 0);
	//assert(liti>0);
	assert(litd ne 0);

	//checking a floating point number against zero accurately is impossible
	//because the its effective epsilon cannot be calculated because the
	//original numbers that were used to calculate the result and number
	//,and are needed to calculate the effective epsilon - error range,
	// and that is all we have, are no longer available.
	//
	//comparing every numeric calculation's result at the time
	//would not solve this because the result of series of calculations
	//on large numbers may result in zero only at the last stage
	//
	//the result of any floating point calculation is only known to be
	//within a range of +/- epsilon*the size of the arguments
	//so errors multiply and propagate
	//
	//therefore in exodus we pick a mid-range "SMALLEST NUMBER"
	//and use this to check for being zero
	//and MVeq this does special check if either of the arguments are zero
	//
	//floating point errors occur in the decimal digits about 17/18 digits
	//to the right of the largest digit. They do not occur simply at about 17/18
	//digits to the right of the decimal point as naive expectation might assume.

	//bool using_epsilon = true;
	//if (not using_epsilon) {
	assert(tinys eq 0);
	//assert(not(tinyi>0));
	assert(tinyd eq 0);
	assert(tinys eq 0.0);
	//assert(not(tinyi>0));
	assert(tinyd eq 0.0);
	//}

	//check we can
	assert(not tinys);
	assert(not tinyd);

	assert(zeros eq 0);
	assert(zeroi eq 0);
	assert(zerod eq 0);

	assert(0 ne bigs);
	assert(0 ne bigi);
	assert(0 ne bigd);

	assert(0 ne lits);
	//assert(0<liti);
	assert(0 ne litd);

	//if (not using_epsilon) {
	assert(0   eq tinys);
	//assert(0<tinyi);
	assert(0   eq tinyd);
	assert(0.0 eq tinys);
	//assert(0<tinyi);
	assert(0.0 eq tinyd);
	//}

	assert(0 eq zeros);
	assert(0 eq zeroi);
	assert(0 eq zerod);

	assert(pwr(10, -10) eq 0);	//.0000000001
	assert(pwr(10, -9) eq 0);	//.000000001
	assert(pwr(10, -8) eq 0);	//.00000001
	assert(pwr(10, -7) eq 0);	//.0000001
	assert(pwr(10, -6) eq 0);	//.000001
	assert(pwr(10, -5) eq 0);	//.00001
	assert(pwr(10, -4) ne 0);	//.0001
	assert(pwr(10, -3) ne 0);	//.001
	assert(pwr(10, -2) ne 0);	//.01
	assert(pwr(10, -1) ne 0);	//.1
	assert(pwr(10, 0) ne 0);	//1
	assert(pwr(10, 1) ne 0);	//10
	assert(pwr(10, 2) ne 0);	//100
	assert(pwr(10, 3) ne 0);	//1000

	assert(pwr(10, -10) eq 0.0);  //.0000000001
	assert(pwr(10, -9) eq 0.0);	  //.000000001
	assert(pwr(10, -8) eq 0.0);	  //.00000001
	assert(pwr(10, -7) eq 0.0);	  //.0000001
	assert(pwr(10, -6) eq 0.0);	  //.000001
	assert(pwr(10, -5) eq 0.0);	  //.00001
	assert(pwr(10, -4) ne 0.0);	  //.0001
	assert(pwr(10, -3) ne 0.0);	  //.001
	assert(pwr(10, -2) ne 0.0);	  //.01
	assert(pwr(10, -1) ne 0.0);	  //.1
	assert(pwr(10, 0) ne 0.0);	  //1
	assert(pwr(10, 1) ne 0.0);	  //10
	assert(pwr(10, 2) ne 0.0);	  //100
	assert(pwr(10, 3) ne 0.0);	  //1000

	assert(pwr(10, -10) eq 0.0);  //.0000000001
	assert(pwr(10, -9) eq 0.0);	  //.000000001
	assert(pwr(10, -8) eq 0.0);	  //.00000001
	assert(pwr(10, -7) eq 0.0);	  //.0000001
	assert(pwr(10, -6) eq 0.0);	  //.000001
	assert(pwr(10, -5) eq 0.0);	  //.00001
	assert(pwr(10, -4) ne 0.0);	  //.0001
	assert(pwr(10, -3) ne 0.0);	  //.001
	assert(pwr(10, -2) ne 0.0);	  //.01
	assert(pwr(10, -1) ne 0.0);	  //.1
	assert(pwr(10, 0) ne 0.0);	  //1
	assert(pwr(10, 1) ne 0.0);	  //10
	assert(pwr(10, 2) ne 0.0);	  //100
	assert(pwr(10, 3) ne 0.0);	  //1000

	assert(var(0.0000)             eq var(0.00001));
	assert(var(0.0001 / 10)        eq var(0.00001));
	assert(var(0.0001 / 10.0)      eq var(0.00001));
	assert(var(0.0001) / 10        eq var(0.00001));
	assert(var(0.0001) / 10.0      eq var(0.00001));
	assert(var(0.0001) / var(10)   eq var(0.00001));
	assert(var(0.0001) / var(10.0) eq var(0.00001));

	var t = 1 / 3;
	printl(var(10.0 / 3));
	printl(var(10.0 / 3) * 3);
	assert(var(10.0 / 3) * 3                                 eq 10);
	assert(var(10.0 / 3 * 3)                                 eq 10);
	assert(var(10.0) / var(3) * 3                            eq 10);
	TRACE((var(10.0) / var(3)).outputl() * var(3).outputl())
	assert((var(10.0) / var(3)).outputl() * var(3).outputl() eq 10);

	//on exodus/c++ all numbers different by 0.0001 are considered the same
	//whereas on pick/arev they are considered different down to
	assert((var(0.0001) gt 0));
	assert((var(1e-4) gt 0));

	//on arev gt le ge le comparison does not use the same logic as eq and bool
	assert(not(var(0.00001) gt 0));
	assert(not(var(0.000000000000000001) gt 0));
	assert(not(var(1e-5) gt 0));
	assert(not(var(1e-18) gt 0));
	printl("tt", pwr(10.0, -16));
	assert(not(pwr(10.0, -16) gt 0));
	assert(not(pwr(10.0, -17) gt 0));
	assert(not(pwr(10.0, -18) gt 0));

	assert(not(pwr(10.0, -10) gt 0));  //.0000000001
	assert(not(pwr(10.0, -9) gt 0));	  //.000000001
	assert(not(pwr(10.0, -8) gt 0));	  //.00000001
	assert(not(pwr(10.0, -7) gt 0));	  //.0000001
	assert(not(pwr(10.0, -6) gt 0));	  //.000001
	assert(not(pwr(10.0, -5) gt 0));	  //.00001
	//
	assert((pwr(10.0, -4) gt 0));  //.0001
	assert((pwr(10.0, -3) gt 0));  //.001
	assert((pwr(10.0, -2) gt 0));  //.01
	assert((pwr(10.0, -1) gt 0));  //.1
	assert((pwr(10.0, 0) gt 0));	  //1
	assert((pwr(10.0, 1) gt 0));	  //10
	assert((pwr(10.0, 2) gt 0));	  //100
	assert((pwr(10.0, 3) gt 0));	  //1000

	var smaller			 = exo_smallest_number - 0.000000000001;
	var negative_smaller = -exo_smallest_number + 0.000000000001;

	// var double

	{
		printl("Test var double gt int 0");

		printl("Smallest number is greater than 0");
		assert(var(exo_smallest_number) gt 0);

		printl("Slightly smaller number is *not* greater than 0");
		assert(not(var(smaller) gt 0));
	}
	{
		printl("Test var double gt double");

		printl("Smallest number is greater than 0");
		assert(var(exo_smallest_number) gt 0.0);

		printl("Slightly smaller number is *not* greater than 0");
		assert(not(var(smaller) gt 0.0));
	}

	{
		printl("Test var double gt var int  0");

		printl("Smallest number is greater than 0");
		assert(var(exo_smallest_number) gt var(0));

		printl("Slightly smaller number is *not* greater than 0");
		assert(not(var(smaller) gt var(0)));
	}
	{
		printl("Test var double gt var double 0");

		printl("Smallest number is greater than 0");
		assert(var(exo_smallest_number) gt var(0.0));

		printl("Slightly smaller number is *not* greater than 0");
		assert(not(var(smaller) gt var(0.0)));
	}

	// <

	{
		printl("Test var double lt var int  0");

		printl("Smallest neg number is lt than 0");
		assert(var(-exo_smallest_number) lt 0);

		printl("Slightly smaller neg number is *not* lt than 0");
		assert(not(var(negative_smaller) lt 0));
	}
	{
		printl("Test var double lt var double 0");

		printl("Smallest number is lt than 0.0");
		assert(var(-exo_smallest_number) lt var(0.0));

		printl("Slightly smaller neg number is *not* greater than 0.0");
		assert(not(var(negative_smaller) lt var(0.0)));
	}

	{
		// compare eq bool true/false

		// Differences from PickOS?
		// Leading spaces are not acceptable on otherwise numeric strings

		// Differences from Javascript
		// 0.999999999999 ne 1 and ne true

		assert(var("") eq false);   // Same as javascript
		assert(var("") ne true);

//		assert(var(" ") eq false); // Javascript
		assert(var(" ") ne false); // Exodus
//		assert(var(" ") ne true);
		assert(var(" ") eq true);  // Bug fixed in 2025

		assert(var("1") ne false);
		assert(var("1") eq true);

		assert(var("2") ne false);
		assert(var("2") eq true);

		assert(var("x") ne false);
		assert(var("x") eq true); // bug fixed in 2025

		assert(var("1.01") ne false); // only zero and empty string are false
		assert(var("1.01") eq true);  // not quite = 1

		assert(var("1.001") ne false); // only zero and empty string are false
		assert(var("1.001") eq true);  // not quite = 1

		assert(var("1.0001") ne false); // only zero and empty string are false

		assert(var("0.00001") eq false);
		assert(var("0.00001") ne true);

		assert(var("-0.00001") ne true);
		assert(var("-0.00001") eq false);

		assert(var("2.0") ne false);
		assert(var("2.0") eq true);

		assert(var("x") ne false);
		assert(var("x") eq true);

		// C++ rules promote bool true to 1.0 and false to 0.0
		assert(0.9999 ne false);  // false 0.0
		assert(0.9999 ne true);   // true 1.0

		assert(0.99999 ne false); // false 0.0
		assert(0.99999 ne true);  // true 1.0

	}
	{
		assert( var("xxx") eq true);
		assert( true eq var("xxx"));

		assert(true eq var(1));
		assert(true eq var(1.0));

		assert(true eq var("1"));
		assert(true eq var("01"));
		assert(true eq var("+1"));
		assert(true eq var("+01"));
		assert(true eq var("1.0"));
		assert(true eq var("+1.0"));

		assert(true ne 3);   // 1 eq 3
		assert(true ne 3.0); // 1.0 eq 3.0

		assert(true ne var(0));   // true ne false
		assert(true ne var(0.0)); // true ne false

		assert(var("") lt true);  // false lt true
	}
	{
		assert(not(var("1x")   lt true));
		assert(not(var("x")    lt true));
		assert(not(var(" ")    lt true));// javascript because leading spaces can be converted to number 0
		assert(not(var("1 ")   lt true));
		assert(not var(") 1")  lt true);
		assert(not(var(1)      lt true));
		assert(    var("")     lt true);
		assert(    var(0)      lt true);
		assert(    var("0")    lt true);
		assert(not(var("0.1")  lt true));
		assert(not(var("-0.1") lt true));
	}

	{
		assert(!(var(0) eq true));
		assert( (var(0) ne true));
		assert( (var(0) lt true));
		assert( (var(0) le true));
		assert(!(var(0) gt true));
		assert(!(var(0) ge true));

		assert( (var(0) eq false));
		assert(!(var(0) ne false));
		assert(!(var(0) lt false));
		assert( (var(0) le false));
		assert(!(var(0) gt false));
		assert( (var(0) ge false));

		assert( (var(1) eq true));
		assert(!(var(1) ne true));
		assert(!(var(1) lt true));
		assert( (var(1) le true));
		assert(!(var(1) gt true));
		assert( (var(1) ge true));

		assert(!(var(1) eq false));
		assert( (var(1) ne false));
		assert(!(var(1) lt false));
		assert(!(var(1) le false));
		assert( (var(1) gt false));
		assert( (var(1) ge false));
	}

	{
		assert(!(true eq var(0)));
		assert( (true ne var(0)));
		assert(!(true lt var(0)));
		assert(!(true le var(0)));
		assert( (true gt var(0)));
		assert( (true ge var(0)));

		assert( (false eq var(0)));
		assert(!(false ne var(0)));
		assert(!(false lt var(0)));
		assert( (false le var(0)));
		assert(!(false gt var(0)));
		assert( (false ge var(0)));

		assert( (true eq var(2)));
		assert(!(true ne var(2)));
		assert( (true lt var(2)));
		assert( (true le var(2)));
		assert(!(true gt var(2)));
		assert( (true ge var(2)));

		assert(!(false eq var(2)));
		assert( (false ne var(2)));
		assert( (false lt var(2)));
		assert( (false le var(2)));
		assert(!(false gt var(2)));
		assert(!(false ge var(2)));

	}

	{
		assert(  var(true) eq var(true));
		assert(!(var(true) ne var(true)));
		assert(!(var(true) lt var(true)));
		assert(!(var(true) gt var(true)));
		assert(  var(true) le var(true));
		assert(  var(true) ge var(true));
	}

	{
		assert(!(var(true) eq var(false)));
		assert( (var(true) ne var(false)));
		assert(!(var(true) lt var(false)));
		assert( (var(true) gt var(false)));
		assert(!(var(true) le var(false)));
		assert( (var(true) ge var(false)));
	}

	{
		assert(!(var(false) eq var(true)));
		assert( (var(false) ne var(true)));
		assert( (var(false) lt var(true)));
		assert(!(var(false) gt var(true)));
		assert(  var(false) le var(true));
		assert(!(var(false) ge var(true)));
	}

	{
		assert( (var(false) eq var(false)));
		assert(!(var(false) ne var(false)));
		assert(!(var(false) lt var(false)));
		assert(!(var(false) gt var(false)));
		assert( (var(false) le var(false)));
		assert( (var(false) ge var(false)));
	}

	for (var j = -1; j le 1; j++) {
		for (var i = -6; i le 1; i++) {
			var d = pwr(10, i);
			var d2 = d + j;
	//		TRACE(d)
	//		TRACE(d eq true)
	//		TRACE(d eq false)
			var line;
			printl();
			line = var("assert((var(") ^ d2 ^ ") eq true)  eq " ^ ((d2 eq true)  ? "true " : "false") ^ ");";
			printl(line);
			line = var("assert((var(") ^ d2 ^ ") eq false) eq " ^ ((d2 eq false) ? "true " : "false") ^ ");";
			printl(line);
		}
	}

	assert((var(0.000001) eq true)  eq false);
	assert((var(0.000001) eq false) eq true );
	assert((var(0.00001) eq true)  eq false);
	assert((var(0.00001) eq false) eq true );
	assert((var(0.0001) eq true)  eq true);
	assert((var(0.0001) eq false) eq false);
	assert((var(0.001) eq true)  eq true);
	assert((var(0.001) eq false) eq false);
	assert((var(0.01) eq true)  eq true);
	assert((var(0.01) eq false) eq false);
	assert((var(0.1) eq true)  eq true);
	assert((var(0.1) eq false) eq false);
	assert((var(1) eq true)  eq true );
	assert((var(1) eq false) eq false);
	assert((var(10) eq true)  eq true);
	assert((var(10) eq false) eq false);

	assert((var(-0.999999) eq true)  eq true);
	assert((var(-0.999999) eq false) eq false);

	assert((var(-0.99999) eq true)  eq true);
	assert((var(-0.99999) eq false) eq false);

	assert((var(-0.9999) eq true)  eq true);
	assert((var(-0.9999) eq false) eq false);

	assert((var(-0.999) eq true)  eq true);
	assert((var(-0.999) eq false) eq false);

	assert((var(-0.99) eq true)  eq true);
	assert((var(-0.99) eq false) eq false);

	assert((var(-0.9) eq true)  eq true);
	assert((var(-0.9) eq false) eq false);

	assert((var(0) eq true)  eq false);
	assert((var(0) eq false) eq true );

	assert((var(9) eq true)  eq true);
	assert((var(9) eq false) eq false);

	assert((var(0.000001) eq true)  eq false);
	assert((var(0.000001) eq false) eq true );

	assert((var(0.00001) eq true)  eq false);
	assert((var(0.00001) eq false) eq true );

	assert((var(0.0001) eq true)  eq true);
	assert((var(0.0001) eq false) eq false);

	assert((var(0.001) eq true)  eq true);
	assert((var(0.001) eq false) eq false);

	assert((var(0.01) eq true)  eq true);
	assert((var(0.01) eq false) eq false);

	assert((var(0.1) eq true)  eq true);
	assert((var(0.1) eq false) eq false);

	assert((var(1) eq true)  eq true );
	assert((var(1) eq false) eq false);

	assert((var(10) eq true)  eq true);
	assert((var(10) eq false) eq false);

	assert((var(1.000001) eq true)  eq true );
	assert((var(1.000001) eq false) eq false);

	assert((var(2) eq true)  eq true);
	assert((var(2) eq false) eq false);

	assert((var(11) eq true)  eq true);
	assert((var(11) eq false) eq false);

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

}; // programexit()
