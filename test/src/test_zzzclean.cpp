#include <exodus/program.h>
programinit()

function main() {
	var filenames = listfiles();
	for (var filename : filenames) {
		if (filename.index("xo_"))
			deletefile(filename);
	}
	printl("Test passed");
	return 0;
}

programexit()

