#include <exodus/library.h>
libraryinit()


#include <gen.h>

var processrec;
var timex;

function main(in processid, in processcategory, in sparevar, in processparameters, in processresult, in processcomments) {
	//obsolete since moved to processes from definitions

	//"heartbeat" called from messaging

	//jbase
	if (processid.unassigned()) {
		processid = "";
	}
	if (processcategory.unassigned()) {
		processcategory = "";
	}
	if (processrec.unassigned()) {
		processrec = "";
	}
	if (processparameters.unassigned()) {
		processparameters = "";
	}
	if (processresult.unassigned()) {
		processresult = "";
	}
	if (processcomments.unassigned()) {
		processcomments = "";
	}

	//if definitions else
	if (not(gen._definitions.open("DEFINITIONS", ""))) {
		return 0;
	}
	// if definitions else return 0
	// end

	if (processid == "") {
		processid = SYSTEM.a(4);
	}

	if (processcategory == "HEARTBEAT") {

		FILEERRORMODE = 1;
		if (not(processrec.read(gen._definitions, "PROCESS*" ^ processid))) {
			processrec = "";
		}
		if (not FILEERROR) {
			processrec.r(11, var().date());
			processrec.r(12, var().time());

			processrec.write(gen._definitions, "PROCESS*" ^ processid);

		}
		goto 517;
	}
	if (processcategory == "LOGOFF" or processcategory == "DELETE") {

		gen._definitions.deleterecord("PROCESS*" ^ processid);
		processid = "";

	}else{

		//generate a new processid and record
		if (processid == "") {
newprocessid:
			call dostime(timex);
			processid = (var(999999999999999).rnd()).substr(-8, 8);
			var xx;
			if (xx.read(gen._definitions, "PROCESS*" ^ processid)) {
				goto newprocessid;
			}

			processrec.r(1, var().date());
			processrec.r(2, timex);
			processrec.r(3, STATION.trim());
			processrec.r(4, SYSTEM.a(17));
			processrec.r(5, USERNAME);

		}else{

			if (not(processrec.read(gen._definitions, "PROCESS*" ^ processid))) {
				gosub newprocessid();
			}

			processrec.r(11, var().date());
			processrec.r(12, var().time());

		}

		if (processcategory) {
			processrec.r(6, processcategory);
		}

		if (processparameters) {
			processrec.r(7, processparameters);
		}
		if (processresult) {
			processrec.r(13, processresult);
		}
		if (processcomments) {
			processrec.r(14, processcomments);
		}

		processrec.write(gen._definitions, "PROCESS*" ^ processid);

	}
L517:

	return 0;

}


libraryexit()