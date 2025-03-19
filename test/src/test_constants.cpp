#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

func main() {
	printl("test_constants says 'Hello World!'");

	printl("No actual tests at the moment");

	printl("Exodus lib was build on OS:", _OS_NAME, _OS_VERSION);
	printl("test_constants was compiled with:", _COMPILER, _COMPILER_VERSION, "c++", _CPP_STANDARD);

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
