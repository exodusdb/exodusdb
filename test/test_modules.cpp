#ifndef EXO_MODULE_INITIAL_BUILD
#	ifdef EXO_MODULE
		import std;
#	endif
#endif

//#include <exodus/format.h>

#include <exodus/program.h>
programinit()

func main() {

	printl("test_modules says 'Hello World!'");

#ifdef EXO_MODULE_INITIAL_BUILD
#else
#	ifdef EXO_MODULE
//	printl(std::format("Using EXODUS as a module using 'import var;' and 'import std;'. {}", "So 'std::format' is available."));
#	else
		printl("NOT using EXODUS as a module.");
#	endif
#endif

    printl(elapsedtimetext());
    printl("Test passed");

	return 0;
}

}; // programexit()
