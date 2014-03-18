#include <exodus/library.h>
libraryinit()


function main() {

	if (locklist.unassigned()) {
		locklist = "";
	}
	if (ntries.unassigned()) {
		ntries = "";
	}
	var msg = "";

	var lockfilename = "VOUCHERS";
	var lockkey = "*";

	var lockdesc = "FINANCIAL POSTING";
	if (not ntries) {
		ntries = 99;
	}

	if (mode == "LOCK") {

		if (not(locking("RELOCK", lockfilename, lockkey, lockdesc, locklist, ntries, msg))) {
			return 0;
		}

		goto 174;
	}
	if (mode == "UNLOCK") {

		if (not(locking("UNLOCK", lockfilename, lockkey, lockdesc, locklist, ntries, msg))) {
			return 0;
		}

		goto 174;
	}
	if (1) {

		msg = DQ ^ (mode ^ DQ) ^ " invalid mode in LOCKPOSTING";
		return 0;

	}
L174:
	return 1;

}


libraryexit()