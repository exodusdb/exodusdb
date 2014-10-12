#include <exodus/library.h>
libraryinit()

#include <locking.h>

function main(in mode, io locklist, in ntries0, io msg) {

	if (locklist.unassigned()) {
		locklist = "";
	}

	var ntries;
	if (ntries0.unassigned()) {
		ntries = "";
	}
		ntries = ntries0;
	if (not ntries) {
		ntries = 99;
	}

	msg = "";

	var lockfilename = "VOUCHERS";
	var lockkey = "*";

	var lockdesc = "FINANCIAL POSTING";

	if (mode == "LOCK") {

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

	return 1;

}


libraryexit()
