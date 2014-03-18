#include <exodus/library.h>
libraryinit()

function main(in msg0, io time0) {
	//jbase linemark

	var interactive = not SYSTEM.a(33);
	//if (interactive) {
	//	return 0;
	//}
	//return 0

	var time2 = ostime().round(2);
	if (time0.unassigned()) {
		time0 = time2;
	}

	var entry = (var().date()).oconv("D");
	entry ^= "\t" ^ time2.oconv("MTS");
	//entry ^= "\t" ^ (time2.field(".", 2)).oconv("MD33P");
	entry ^= "\t" ^ (time2 - time0).oconv("MD33P");
	entry ^= "\t" ^ msg0;

	//if @username='NEOSYS' then
	printl(entry);
	//end

	entry ^= "\r\n";

	time0 = time2;

	//logfilename=field(FIELD2(@rollout.file,'\',-1),'.',1):'.LOG'
	var logfilename = "NEO" ^ SYSTEM.a(24);//ROLLOUTFILE;
	logfilename.splicer(-3, 3, "LOG");
	var logfilelength = logfilename.osfile().a(1);
	if (not logfilelength) {
		call oswrite("", logfilename);
	}
	var logfile;
	if (logfile.osopen(logfilename)) {
		call osbwrite(entry, logfile, logfilelength);
		logfile.osclose();
	}

	return 0;

}

libraryexit()
