#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

func main() {
	printl("p6 says 'Hello World!'");

	{
		//lvalue const (makes a copy)
		let c = "ab";
		assert(c.cut(-3) eq "");
		assert(c.cut(-2) eq "");
		assert(c.cut(-1) eq "a");
		assert(c.cut(-0) eq "ab");
		assert(c.cut(-1) eq "a");
		assert(c.cut(-2) eq "");
		assert(c.cut(-3) eq "");

		//lvalues mutator
		var x = "ab";
		x="ab";x.cutter(-3);assert(x eq "");
		x="ab";x.cutter(-2);assert(x eq "");
		x="ab";x.cutter(-1);assert(x eq "a");
		x="ab";x.cutter(-0);assert(x eq "ab");
		x="ab";x.cutter(-1);assert(x eq "a");
		x="ab";x.cutter(-2);assert(x eq "");
		x="ab";x.cutter(-3);assert(x eq "");
		//assert(c.cutter(-3) eq "");assert(x eq ""); // doesnt compile

		//rvalues (doesnt make a copy)
		assert(var("ab").cut(-3) eq "");
		assert(var("ab").cut(-2) eq "");
		assert(var("ab").cut(-1) eq "a");
		assert(var("ab").cut(-0) eq "ab");
		assert(var("ab").cut(-1) eq "a");
		assert(var("ab").cut(-2) eq "");
		assert(var("ab").cut(-3) eq "");

	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

}; // programexit()
