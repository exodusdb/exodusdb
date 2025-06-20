#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

func main() {

    printl("test_more2c says 'Hello World!'");

	var i  = 3;
	var d  = 7.5;
	var z  = "";
	var si = "8";
	var sd = "7.7";
	var sn = "xyz";

	//compare all different kinds of lhs with rhs double

	{
		//empty string is never equal to any double
		assert(var("") ne 3.3);
		assert(var("") ne 0.0);

		var xi = "";
		TRACE(xi.toInt())
		xi.outputl("xi");
		assert(xi ne 0.0);

		var xd = "";
		TRACE(xd.toDouble())
		xd.outputl("xd");
		assert(xd ne 0.0);

		assert(var(0.0)  eq 0.0);
		assert(var(3.3)  eq 3.3);
		assert(var(-3.3) eq -3.3);
		assert(var(3.4) ne 3.3);

		assert(var("0.0")  eq 0.0);
		assert(var("3.3")  eq 3.3);
		assert(var("-3.3") eq -3.3);
		assert(var("3.4") ne 3.3);

		assert(var(0)  eq 0.0);
		assert(var(0) ne 3.3);
		assert(var(4) ne 3.3);
		assert(var(3)  eq 3.0);
		assert(var(-3) eq -3.0);

		assert(var("0")  eq 0.0);
		assert(var("0") ne 3.3);
		assert(var("4") ne 3.3);
		assert(var("3")  eq 3.0);
		assert(var("-3") eq -3.0);

		assert(var("xxxx") ne 3.0);
	}
	{
		//empty string is never equal to any double
		assert(var("") ne 3);
		assert(var("") ne 0);

		var xi = "";
		TRACE(xi.toInt())
		xi.outputl("xi");
		assert(xi ne 0);

		var xd = "";
		TRACE(xd.toDouble())
		xd.outputl("xd");
		assert(xd ne 0);

		assert(var(0)   eq 0);
		assert(var(0.)  eq 0);
		assert(var(3.)  eq 3);
		assert(var(-3.) eq -3);
		assert(var(4.) ne 3);

		assert(var("0.0") eq 0);
		assert(var("3.")  eq 3);
		assert(var("-3.") eq -3);
		assert(var("3.0") ne 0);

		assert(var(0)  eq 0);
		assert(var(0)  eq -0);
		assert(var(0) ne 3);
		assert(var(4) ne 3);
		assert(var(3)  eq 3);
		assert(var(-3) eq -3);

		assert(var("0")  eq 0);
		assert(var("-0") eq 0);
		assert(var("0") ne 3);
		assert(var("4") ne 3);
		assert(var("3")  eq 3);
		assert(var("-3") eq -3);

		assert(var("xxxx") ne 3);
		assert(var("xxxx") ne 0);
		assert(var("xxxx") ne -0);
	}

	{
		var x = 1;

		//////////
		//positive
		//////////

		//lvalue

		printl(x++, 1);
		printl(x++, 2);
		printl(x, 3);
		printl(++++x, 4);
		printl(x, 4);

		x = 1;
		assert(x++ eq 1);
		assert(x   eq 2);
		assert(++x eq 3);
		assert(x   eq 3);

		//rvalue

		x = 1;
		//assert(x.f(1)++ eq 1);//pointless. should not compile
		assert(x.f(1)     eq 1);
		//assert(++x.f(1) eq 2);//will not compile because "." priority gt "++"
		//assert((++x).f(1) eq 2); // SADLY WILL NOT COMPILE because ++x produces a var& and ++ is implemented in var_base ATM
		//assert(x.f(1)     eq 2);

		//////////
		//negative
		//////////

		//lvalue

		x = 1;
		assert(x-- eq 1);
		assert(x   eq 0);
		assert(--x eq -1);
		assert(x   eq -1);

		//rvalue

		x = 1;
		//assert(x.f(1)-- eq 1);//pointless. should not compile
		assert(x.f(1)     eq 1);
		//assert(--x.f(1) eq 2);//will not compile because "." priority gt "--"
		//assert((--x).f(1) eq 0);// SADLY WILL NOT COMPILE because ++x produces a var& and ++ is implemented in var_base ATM
		//assert(x.f(1)     eq 0);
	}

	{
		assert('1' + var(1.1)   eq 2.1);
		assert("1" + var(1.1)   eq 2.1);
		assert("1.1" + var(1.3) eq 2.4);
		assert(1 + var(2.1)     eq 3.1);
		assert(1.1 + var(2.1)   eq 3.2);

		assert(var(1.1) + '1'   eq 2.1);
		assert(var(1.1) + "1"   eq 2.1);
		assert(var(1.3) + "1.1" eq 2.4);
		assert(var(2.1) + 1     eq 3.1);
		assert(var(2.1) + 1.1   eq 3.2);

		assert('1' - var(1.1)   eq -0.1);
		assert("1" - var(1.1)   eq -0.1);
		assert("1.1" - var(1.3) eq -0.2);
		assert(1 - var(2.1)     eq -1.1);
		assert(1.1 - var(2.1)   eq -1.0);

		assert(var(1.1) - '1'   eq 0.1);
		assert(var(1.1) - "1"   eq 0.1);
		assert(var(1.3) - "1.1" eq 0.2);
		assert(var(2.1) - 1     eq 1.1);
		assert(var(2.1) - 1.1   eq 1.0);
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

}; // programexit()
