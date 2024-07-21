#include <exodus/program.h>
programinit()

function main() {

	printl("test_modules says 'Hello World!'");

#ifdef EXO_MODULE
	printl(std::format("Using EXODUS as a module using 'import var;' and 'import std;'. {}", "So 'std::format' is available."));
#else
	printl("NOT using EXODUS as a module.");
#endif

    printl(elapsedtimetext());
    printl("Test passed");

	return 0;
}

programexit()
