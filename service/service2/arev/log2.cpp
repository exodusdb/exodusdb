#include <exodus/library.h>
libraryinit()

#include <osbwritex.h>


var time2;//num
var logfile;

function main(in msg0, in time0) {
	//jbase linemark

	var interactive = not SYSTEM.a(33);
	if (interactive) {
		return 0;
	}
	//return 0

	call dostime(time2);
	if (time0.unassigned()) {
		time0 = time2;
	}

	var entry = (var().date()).oconv("D");
	entry ^= "\t" ^ time2.oconv("MTS");
	entry ^= "\t" ^ (time2.field(".", 2)).oconv("MD33P");
	entry ^= "\t" ^ (time2 - time0).oconv("MD33P");
	entry ^= "\t" "\t" "\t" "\t";
	entry ^= "\t" ^ msg0;

	//if @username='NEOSYS' then
	cout << entry << endl;
	//end

	entry ^= "\r\n";

	time0 = time2;

	//logfilename=field(FIELD2(@rollout.file,'\',-1),'.',1):'.LOG'
	var logfilename = ROLLOUTFILE;
	logfilename.splicer(-3, 3, "LOG");
	var logfilelength = logfilename.osfile().a(1);
	if (not logfilelength) {
		call oswrite("", logfilename);
	}
	if (logfile.osopen(logfilename)) {
		call osbwritex(entry, logfile, logfilename, logfilelength);
		logfile.osclose();
	}

	return 0;

}


libraryexit()