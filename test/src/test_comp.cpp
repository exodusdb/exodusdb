#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>

//exodus/c++ > uses SMALLEST_NUMBER to compare equality
////0.0001 for pick/arev compatibility

programinit()

	//#include <l2.h>

	function main() {

	TRACE(SMALLEST_NUMBER)

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

	assert(not(bigs < bigs));
	assert(bigs > bigi);
	assert(not(bigs > bigd));

	assert(bigi < bigs);
	assert(not(bigi < bigi));
	assert(bigi < bigd);

	assert(not(bigd < bigs));
	assert(bigd > bigi);
	assert(not(bigd > bigd));

	assert(bigs > lits);
	assert(bigs > litd);

	assert(bigs > tinys);
	assert(bigs > tinyd);

	assert(bigs eq bigs);
	assert(bigs != bigi);
	assert(bigs eq bigd);

	assert(bigi != bigs);
	assert(bigi eq bigi);
	assert(bigi != bigd);

	assert(bigd eq bigs);
	assert(bigd != bigi);
	assert(bigd eq bigd);

	assert(bigs != lits);
	assert(bigs != litd);

	assert(bigs != tinys);
	assert(bigs != tinyd);

	assert(bigi > lits);
	assert(bigi > litd);

	assert(bigi > tinys);
	assert(bigi > tinyd);

	assert(bigd > lits);
	assert(bigd > litd);
	assert(bigd > tinys);
	assert(bigd > tinyd);

	assert(bigs > zeros);
	assert(bigs > zeroi);
	assert(bigs > zerod);

	assert(bigs > 0);
	assert(bigi > 0);
	assert(bigd > 0);

	assert(lits > 0);
	//assert(liti>0);
	assert(litd > 0);

	assert(not(tinys > 0));
	//assert(not(tinyi>0));
	assert(not(tinyd > 0));

	assert(not(zeros > 0));
	assert(not(zeroi > 0));
	//printl(zerod);
	assert(not(zerod > 0));

	assert(0 < bigs);
	assert(0 < bigi);
	assert(0 < bigd);

	assert(0 < lits);
	//assert(0<liti);
	assert(0 < litd);

	assert(not(0 < tinys));
	//assert(0<tinyi);
	assert(not(0 < tinyd));

	assert(not(0 < zeros));
	assert(not(0 < zeroi));
	assert(not(0 < zerod));

	assert(bigs != 0);
	assert(bigi != 0);
	assert(bigd != 0);

	assert(lits != 0);
	//assert(liti>0);
	assert(litd != 0);

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

	assert(0 != bigs);
	assert(0 != bigi);
	assert(0 != bigd);

	assert(0 != lits);
	//assert(0<liti);
	assert(0 != litd);

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
	assert(pwr(10, -4) != 0);	//.0001
	assert(pwr(10, -3) != 0);	//.001
	assert(pwr(10, -2) != 0);	//.01
	assert(pwr(10, -1) != 0);	//.1
	assert(pwr(10, 0) != 0);	//1
	assert(pwr(10, 1) != 0);	//10
	assert(pwr(10, 2) != 0);	//100
	assert(pwr(10, 3) != 0);	//1000

	assert(pwr(10, -10) eq 0.0);  //.0000000001
	assert(pwr(10, -9) eq 0.0);	  //.000000001
	assert(pwr(10, -8) eq 0.0);	  //.00000001
	assert(pwr(10, -7) eq 0.0);	  //.0000001
	assert(pwr(10, -6) eq 0.0);	  //.000001
	assert(pwr(10, -5) eq 0.0);	  //.00001
	assert(pwr(10, -4) != 0.0);	  //.0001
	assert(pwr(10, -3) != 0.0);	  //.001
	assert(pwr(10, -2) != 0.0);	  //.01
	assert(pwr(10, -1) != 0.0);	  //.1
	assert(pwr(10, 0) != 0.0);	  //1
	assert(pwr(10, 1) != 0.0);	  //10
	assert(pwr(10, 2) != 0.0);	  //100
	assert(pwr(10, 3) != 0.0);	  //1000

	assert(pwr(10, -10) eq 0.0);  //.0000000001
	assert(pwr(10, -9) eq 0.0);	  //.000000001
	assert(pwr(10, -8) eq 0.0);	  //.00000001
	assert(pwr(10, -7) eq 0.0);	  //.0000001
	assert(pwr(10, -6) eq 0.0);	  //.000001
	assert(pwr(10, -5) eq 0.0);	  //.00001
	assert(pwr(10, -4) != 0.0);	  //.0001
	assert(pwr(10, -3) != 0.0);	  //.001
	assert(pwr(10, -2) != 0.0);	  //.01
	assert(pwr(10, -1) != 0.0);	  //.1
	assert(pwr(10, 0) != 0.0);	  //1
	assert(pwr(10, 1) != 0.0);	  //10
	assert(pwr(10, 2) != 0.0);	  //100
	assert(pwr(10, 3) != 0.0);	  //1000

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
	assert((var(0.0001) > 0));
	assert((var(1e-4) > 0));

	//on arev gt le ge le comparison does not use the same logic as eq and bool
	assert(not(var(0.00001) > 0));
	assert(not(var(0.000000000000000001) > 0));
	assert(not(var(1e-5) > 0));
	assert(not(var(1e-18) > 0));
	printl("tt", pwr(10.0, -16));
	assert(not(pwr(10.0, -16) > 0));
	assert(not(pwr(10.0, -17) > 0));
	assert(not(pwr(10.0, -18) > 0));

	assert(not(pwr(10.0, -10) > 0));  //.0000000001
	assert(not(pwr(10.0, -9) > 0));	  //.000000001
	assert(not(pwr(10.0, -8) > 0));	  //.00000001
	assert(not(pwr(10.0, -7) > 0));	  //.0000001
	assert(not(pwr(10.0, -6) > 0));	  //.000001
	assert(not(pwr(10.0, -5) > 0));	  //.00001
	//
	assert((pwr(10.0, -4) > 0));  //.0001
	assert((pwr(10.0, -3) > 0));  //.001
	assert((pwr(10.0, -2) > 0));  //.01
	assert((pwr(10.0, -1) > 0));  //.1
	assert((pwr(10.0, 0) > 0));	  //1
	assert((pwr(10.0, 1) > 0));	  //10
	assert((pwr(10.0, 2) > 0));	  //100
	assert((pwr(10.0, 3) > 0));	  //1000

	var smaller			 = SMALLEST_NUMBER - 0.000000000001;
	var negative_smaller = -SMALLEST_NUMBER + 0.000000000001;

	// var double

	{
		printl("Test var double > int 0");

		printl("Smallest number is greater than 0");
		assert(var(SMALLEST_NUMBER) > 0);

		printl("Slightly smaller number is *not* greater than 0");
		assert(not(var(smaller) > 0));
	}
	{
		printl("Test var double > double");

		printl("Smallest number is greater than 0");
		assert(var(SMALLEST_NUMBER) > 0.0);

		printl("Slightly smaller number is *not* greater than 0");
		assert(not(var(smaller) > 0.0));
	}

	{
		printl("Test var double > var int  0");

		printl("Smallest number is greater than 0");
		assert(var(SMALLEST_NUMBER) > var(0));

		printl("Slightly smaller number is *not* greater than 0");
		assert(not(var(smaller) > var(0)));
	}
	{
		printl("Test var double > var double 0");

		printl("Smallest number is greater than 0");
		assert(var(SMALLEST_NUMBER) > var(0.0));

		printl("Slightly smaller number is *not* greater than 0");
		assert(not(var(smaller) > var(0.0)));
	}

	// <

	{
		printl("Test var double < var int  0");

		printl("Smallest neg number is < than 0");
		assert(var(-SMALLEST_NUMBER) < 0);

		printl("Slightly smaller neg number is *not* < than 0");
		assert(not(var(negative_smaller) < 0));
	}
	{
		printl("Test var double < var double 0");

		printl("Smallest number is < than 0.0");
		assert(var(-SMALLEST_NUMBER) < var(0.0));

		printl("Slightly smaller neg number is *not* greater than 0.0");
		assert(not(var(negative_smaller) < var(0.0)));
	}

	{
		// compare eq bool true/false

		// Differences from PickOS?
		// Leading spaces are not acceptable on otherwise numeric strings

		// Differences from Javascript
		// 0.999999999999 != 1 and  != true

		assert((var("") == true) == false); //same as javascript
		assert((var("") != true) == true); //same as javascript

		assert((var("") == false) == true); //javascript
		assert((var("") != false) == false); //javascript

//		assert(var("") == false);
		assert(var("") != true);

//		assert(var(" ") == false);//javascript
		assert(var(" ") != false);//exodus
		assert(var(" ") != true);


		assert(var("1") != false);
		assert(var("1") == true);

		assert(var("2") != false);
		assert(var("2") != true);

		assert(var("x") != false);
		assert(var("x") != true);

		assert(var("1.01") != false); // only zero and empty string are false
		assert(var("1.01") != true);  // not quite = 1

		assert(var("1.001") != false); // only zero and empty string are false
		assert(var("1.001") != true);  // not quite = 1

		assert(var("1.0001") != false); // only zero and empty string are false

//		assert(var("1.0001") != true);  // not quite = 1
//		assert(var("1.0001") != true);  // not quite = 1
		assert(var("1.00001") == true); //too small a difference from 1
		assert(var("1.00001") != false); //too small a difference from 1

		assert(var("1.0001") != false);//close enough to 1 to be true?
		assert(var("1.0001") == true);

		assert(var("2.0") != false);//neither true nor false
		assert(var("2.0") != true);

		assert(var("x") != false);//neither true nor false
		assert(var("x") != true);

		assert(0.9999 != true);// not close enough to 1
		assert(0.9999 != false);

		assert(0.99999 != true);// close enough to 1
		assert(0.99999 != false);

		assert(var(1.01) != false); // only number 1 is true
		assert(var(1.01) != true);  // not quite = 1

		assert(var(1.001) != false); // only zero and empty string are false
		assert(var(1.001) != true);  // not quite = 1

		assert(var(1.0001) != false); // Only zero and empty string are false
//		assert(var(1.0001) != true);  // not quite = 1
		assert(var(1.00001) == true); //too small a difference from 1
		assert(var(1.00001) != false); //too small a difference from 1

		assert(var(1.0001) != false);
		assert(var(1.0001) == true);

		assert(var(1) != false);
		assert(var(1) == true);

		assert(var(2) != false);//neither true nor false
		assert(var(2) != true);

		assert(var(1.0) != false);
		assert(var(1.0) == true);

		assert(var(2.0) != false);//neither true nor false
		assert(var(2.0) != true);

	}
	{
		assert( var("xxx") != true);
		assert( true != var("xxx"));

		assert(true eq var(1));
		assert(true eq var(1.0));

		assert(true eq var("1"));
		assert(true eq var("01"));
		assert(true eq var("+1"));
		assert(true eq var("+01"));
		assert(true eq var("1.0"));
		assert(true eq var("+1.0"));

		assert(true eq 1);
		assert(true eq 1);

		assert(true ne var(0));
		assert(true ne var(0.0));

		assert(var("") < true);
	}
	{
		assert(not(var("1x")   lt true));
		assert(not(var("x")    lt true));
		assert(    var(" ")    lt true);// javascript because leading spaces can be converted to number 0
//		assert(not(var(" ")    lt true));
		assert(not(var("1 ")   lt true));
		assert(not var(") 1")  lt true);
		assert(not(var(1)      lt true));
		assert(    var("")     lt true);
		assert(    var(0)      lt true);
		assert(    var("0")    lt true);
		assert(    var("0.1")  lt true);
		assert(    var("-0.1") lt true);

	}

	{
		assert(var(-0.1) lt false);
		assert(var(0.1) gt false);

		assert(var(0.9) lt true);
		assert(var(1.1) gt true);


		assert(var(-0.1) le false);
		assert(var(0.1) ge false);

		assert(var(0.9) le true);
		assert(var(1.1) ge true);


		assert(false gt var(-0.1));
		assert(false lt var(0.1));

		assert(true gt var(0.9));
		assert(true lt var(1.1));


		assert(false ge var(-0.1));
		assert(false le var(0.1));

		assert(true ge var(0.9));
		assert(true le var(1.1));

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

	{
		assert((var("x") == true) == false);
		assert((var("x") == false) == false);

		assert((var("x") != true) == true);
		assert((var("x") != false) == true);

		assert((var("x") < true) == false);
		assert((var("x") < false) == false);

		assert((var("x") <= true) == false);
		assert((var("x") <= false) == false);

		assert((var("x") > true) == true);
		assert((var("x") > false) == true);

		assert((var("x") >= true) == true);
		assert((var("x") >= false) == true);
	}

	{
		assert((true == var("x")) == false);
		assert((false == var("x")) == false);

		assert((true != var("x")) == true);
		assert((false != var("x")) == true);

		assert((true < var("x")) == true);
		assert((false < var("x")) == true);

		assert((true <= var("x")) == true);
		assert((false <= var("x")) == true);

		assert((true > var("x")) == false);
		assert((false > var("x")) == false);

		assert((true >= var("x")) == false);
		assert((false >= var("x")) == false);
	}


	for (var j = -1; j <= 1; j++) {
		for (var i = -6; i <= 1; i++) {
			var d = pwr(10, i);
			var d2 = d + j;
	//		TRACE(d)
	//		TRACE(d == true)
	//		TRACE(d == false)
			var line;
			printl();
			line = var("assert((var(") ^ d2 ^ ") == true)  == " ^ ((d2 == true)  ? "true " : "false") ^ ");";
			printl(line);
			line = var("assert((var(") ^ d2 ^ ") == false) == " ^ ((d2 == false) ? "true " : "false") ^ ");";
			printl(line);
		}
	}

	assert((var(0.000001) == true)  == false);
	assert((var(0.000001) == false) == true );
	assert((var(0.00001) == true)  == false);
	assert((var(0.00001) == false) == true );
	assert((var(0.0001) == true)  == false);
	assert((var(0.0001) == false) == false);
	assert((var(0.001) == true)  == false);
	assert((var(0.001) == false) == false);
	assert((var(0.01) == true)  == false);
	assert((var(0.01) == false) == false);
	assert((var(0.1) == true)  == false);
	assert((var(0.1) == false) == false);
	assert((var(1) == true)  == true );
	assert((var(1) == false) == false);
	assert((var(10) == true)  == false);
	assert((var(10) == false) == false);
	assert((var(100) == true)  == false);
	assert((var(100) == false) == false);
	assert((var(1000) == true)  == false);
	assert((var(1000) == false) == false);
	assert((var(10000) == true)  == false);
	assert((var(10000) == false) == false);
	assert((var(100000) == true)  == false);
	assert((var(100000) == false) == false);
	assert((var(1000000) == true)  == false);
	assert((var(1000000) == false) == false);

	assert((var(-0.999999) == true)  == false);
	assert((var(-0.999999) == false) == false);

	assert((var(-0.99999) == true)  == false);
	assert((var(-0.99999) == false) == false);

	assert((var(-0.9999) == true)  == false);
	assert((var(-0.9999) == false) == false);

	assert((var(-0.999) == true)  == false);
	assert((var(-0.999) == false) == false);

	assert((var(-0.99) == true)  == false);
	assert((var(-0.99) == false) == false);

	assert((var(-0.9) == true)  == false);
	assert((var(-0.9) == false) == false);

	assert((var(0) == true)  == false);
	assert((var(0) == false) == true );

	assert((var(9) == true)  == false);
	assert((var(9) == false) == false);

	assert((var(0.000001) == true)  == false);
	assert((var(0.000001) == false) == true );

	assert((var(0.00001) == true)  == false);
	assert((var(0.00001) == false) == true );

	assert((var(0.0001) == true)  == false);
	assert((var(0.0001) == false) == false);

	assert((var(0.001) == true)  == false);
	assert((var(0.001) == false) == false);

	assert((var(0.01) == true)  == false);
	assert((var(0.01) == false) == false);

	assert((var(0.1) == true)  == false);
	assert((var(0.1) == false) == false);

	assert((var(1) == true)  == true );
	assert((var(1) == false) == false);

	assert((var(10) == true)  == false);
	assert((var(10) == false) == false);

	assert((var(1.000001) == true)  == true );
	assert((var(1.000001) == false) == false);

	assert((var(1.00001) == true)  == true );
	assert((var(1.00001) == false) == false);

	assert((var(1.0001) == true)  == true );
	assert((var(1.0001) == false) == false);

	assert((var(1.001) == true)  == false);
	assert((var(1.001) == false) == false);

	assert((var(1.01) == true)  == false);
	assert((var(1.01) == false) == false);

	assert((var(1.1) == true)  == false);
	assert((var(1.1) == false) == false);

	assert((var(2) == true)  == false);
	assert((var(2) == false) == false);

	assert((var(11) == true)  == false);
	assert((var(11) == false) == false);

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
