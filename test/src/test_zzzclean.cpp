#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>
#include <exodus/program.h>
programinit()

	function main() {

	//Pass if no default database connection
	if (not connect() and osgetenv("EXO_NODATA")) {
		printl("No default db connection to perform db testing.");
		printl("Test passed");
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
