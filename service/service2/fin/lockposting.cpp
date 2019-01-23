#include <exodus/library.h>
libraryinit()

#include <locking.h>

function main(in mode, io locklist, in ntries, io msg) {
	//c fin in,io,in,io

	if (locklist.unassigned()) {
		locklist = "";
	}
	if (ntries.unassigned()) {
		ntries = "";
	}
	msg = "";

	var lockfilename = "VOUCHERS";
	var lockkey = "*";

	var lockdesc = "FINANCIAL POSTING";
	if (not ntries) {
		ntries = 99;
	}

	if (mode == "LOCK") {

		//relock so if we have already locked then no problem ie relock succeeds
		if (not(locking("RELOCK", lockfilename, lockkey, lockdesc, locklist, ntries, msg))) {
			return 0;
		}

	} else if (mode == "UNLOCK") {

		if (not(locking("UNLOCK", lockfilename, lockkey, lockdesc, locklist, ntries, msg))) {
			return 0;
		}

	} else {

		msg = DQ ^ (mode ^ DQ) ^ " invalid mode in LOCKPOSTING";
		return 0;

	}
//L176:
	return 1;

}


libraryexit()
