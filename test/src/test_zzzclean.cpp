#include <exodus/program.h>
programinit()

function main() {

	//Pass if no default database connection
	if (not connect()) {
		printl("No default db connection to perform db testing.");
		printl("Test passed");
		return 0;
	}

	var filenames = listfiles();
	for (var filename : filenames) {
		if (filename.index("xo_"))
			deletefile(filename);
	}
	printl("Test passed");
	return 0;
}

programexit()

