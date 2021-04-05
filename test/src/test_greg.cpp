#include <cassert>

#include <exodus/program.h>
programinit()

function main() {
	printl("test_greg says 'Hello World!'");

	assert(oconv(date(),"D") == "5 APR 2021");

	printl("Test passed");

	return 0;
}

programexit()

