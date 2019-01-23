#include <exodus/library.h>
libraryinit()



var logfile;
var datax;
var time2;//num

function main(in msg0, io time0) {
	//c sys in,io

	var interactive = not SYSTEM.a(33);
	if (interactive) {
		return 0;
	}

	//logfilename=field(FIELD2(@rollout.file,'\',-1),'.',1):'.LOG'
	var logfilename = ROLLOUTFILE;
	logfilename.splicer(-3, 3, "LOG");
	var logfilelength = logfilename.osfile().a(1);
	if (not logfilelength) {
		call oswrite("", logfilename);
	}
	var temposfilename83 = logfilename;
	if (not(logfile.osopen(logfilename))) {
		return 0;
	}

	if (msg0 == "GETLASTLOG") {
		var ptr = logfilelength - 1024;
		call osbread(datax, logfile,  ptr, 1024);
		logfile.osclose();
		datax.converter("\r\n", FM ^ FM);
		time0 = logfilename ^ " " ^ field2(datax, FM, -3, 3);
		return 0;
	}

	time2 = ostime();
	if (time0.unassigned()) {
		time0 = time2;
	}

	var entry = var().date().oconv("D2-J");
	entry ^= " " ^ time2.oconv("MTS");
	entry ^= time2.field(".", 2).oconv("MD20P");
	entry ^= " " ^ (time2 - time0).oconv("MD20P");
	//entry:=sep:sep:sep:sep
	entry ^= " " ^ msg0;

	printl(entry);

	entry ^= "\r\n";

	time0 = time2;

	call osbwrite(entry, logfile,  logfilelength);
	logfile.osclose();

	return 0;

}


libraryexit()
