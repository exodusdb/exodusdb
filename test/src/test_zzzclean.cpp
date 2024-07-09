#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>
#include <exodus/program.h>
programinit()

	function main() {

	// Quit if no default database connection
	if (not connect()) {
		//Pass if allowed
		if (osgetenv("EXO_NODATA") or true) {
			printx("Test passed. Not really. ");
		}
		printl("No default db connection to perform db testing.");
		return 0;
	}

	//Pass if parallel testing since we cannot clean up until afterwards
	if (osgetenv("CTEST_PARALLEL_LEVEL")) {
		printl("Cannot cleanup if parallel testing.");
		printl("Test passed");
		return 0;
	}

	var filenames = listfiles();

	//	var dictconn;
	//	if (dictconn.connect("exodus")) {
	//		 ^ FM ^ connect("exodus").listfiles();

	for (var filename : filenames.unique()) {
		if (filename.contains("xo_")) {
			printl("Deleting", filename);
			assert(deletefile(filename));
		}
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
