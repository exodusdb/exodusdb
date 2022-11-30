#include <exodus/library.h>
libraryinit()

#include <otherusers.h>

function main(in /*mode*/, in datasetcode, out result) {

	let shutdownfilename = datasetcode.lcase() ^ ".end";
	if (shutdownfilename.osfile()) {
		result = 2;
		return 0;
	}

	//call oswrite(_EOL "BACKUP", shutdownfilename);
	if (not oswrite(_EOL "BACKUP", shutdownfilename)) {
		abort(lasterror());
	}

	// wait for 120 secs for other database users to quit
	// randomise randomising doesnt make any difference
	printl();
	printl("Waiting up to 120 seconds for other ", datasetcode, " processes to close:");
	let ii = "";
	for (const var ii : range(1, 120)) {
		// /BREAK;
		if (not otherusers(datasetcode))
			break;
		print(".");
		call ossleep(1000 * 1);
		if (esctoexit()) {
			// ii = 99999;
			break;
		}
	}  // ii;

	//shutdownfilename.osremove();
	if (shutdownfilename.osfile() and not shutdownfilename.osremove()) {
		abort(lasterror());
	}

	if (ii >= 120) {
		result = 0;
		printl(" Failed");
	} else {
		result = 1;
		printl(" Success");
	}

	return 0;
}

libraryexit()
