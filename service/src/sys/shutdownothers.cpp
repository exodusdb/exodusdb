#include <exodus/library.h>
libraryinit()

#include <otherusers.h>

function main(in mode, in datasetcode, out result) {
	//c sys in,in,out

	var shutdownfilename = datasetcode.lcase() ^ ".end";
	if (shutdownfilename.osfile()) {
		result = 2;
		return 0;
	}
	if (not mode) {
		{}
	}

	call oswrite("\r\n" "BACKUP", shutdownfilename);

	//wait for 120 secs for other database users to quit
	//randomise randomising doesnt make any difference
	printl();
	printl("Waiting up to 120 seconds for other ", datasetcode, " processes to close:");
	var ii = "";
	for (var ii = 1; ii <= 120; ++ii) {
		///BREAK;
		if (not(otherusers(datasetcode))) break;
		print(".");
		call ossleep(1000*1);
		if (esctoexit()) {
			ii = 99999;
		}
	};//ii;

	shutdownfilename.osdelete();

	if (ii ge 120) {
		result = 0;
		printl(" Failed");
	}else{
		result = 1;
		printl(" Success");
	}

	return 0;
}

libraryexit()
