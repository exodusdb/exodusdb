#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

function main() {

	// TODO find some way to check output while testing
	{
		// errput
		errputl();
		errput("1","2");
		errputl("3");
		errputl("123");

		// logput
		logputl();
		logput("a","b");
		logputl("c");
		logputl("abc");

		// output
		outputl();
		output("x","y");
		outputl("z");
		outputl("xyz");

		// print
		printl();
		printx("x","y");
		printl("z");
		printl("x yz");

		// printt
		printl();
		printt("x","y");
		printl("z");
		printl("x\ty\tz");
	}

	printl(elapsedtimetext());
	printl("Test passed (not really)");

	return 0;
}
programexit()
