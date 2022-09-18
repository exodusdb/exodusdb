#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

function main() {
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
		x="ab";assert(x.cutter(-3) eq "");assert(x eq "");
		x="ab";assert(x.cutter(-2) eq "");assert(x eq "");
		x="ab";assert(x.cutter(-1) eq "a");assert(x eq "a");
		x="ab";assert(x.cutter(-0) eq "ab");assert(x eq "ab");
		x="ab";assert(x.cutter(-1) eq "a");assert(x eq "a");
		x="ab";assert(x.cutter(-2) eq "");assert(x eq "");
		x="ab";assert(x.cutter(-3) eq "");assert(x eq "");
		//assert(c.cutter(-3) eq "");assert(x eq ""); // doesnt compile

		//rvalues (doesnt make a copy)
		assert(var("ab").cut(-3) eq "");
		assert(var("ab").cut(-2) eq "");
		assert(var("ab").cut(-1) eq "a");
		assert(var("ab").cut(-0) eq "ab");
		assert(var("ab").cut(-1) eq "a");
		assert(var("ab").cut(-2) eq "");
		assert(var("ab").cut(-3) eq "");

		//rvalues (no detectable difference)
		assert(var("ab").cutter(-3) eq "");
		assert(var("ab").cutter(-2) eq "");
		assert(var("ab").cutter(-1) eq "a");
		assert(var("ab").cutter(-0) eq "ab");
		assert(var("ab").cutter(-1) eq "a");
		assert(var("ab").cutter(-2) eq "");
		assert(var("ab").cutter(-3) eq "");
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()

