#include <exodus/library.h>
libraryinit()

var processcategory;
var processrec;
var processparameters;
var processresult;
var processcomments;
var timex;

function main(io processid, in processcategory0, in /*sparevar*/ = "", in processparameters0 = "", in processresult0 = "", in processcomments0 = "") {

	// obsolete since moved to processes from definitions

	// "heartbeat" called from messaging

//	if (processid.unassigned()) {
//		processid = "";
//	}
//	// 	if (sparevar.unassigned()) {
//	// 		sparevar = "";
//	// 	}
//	if (processcategory0.unassigned()) {
//		processcategory = "";
//	} else {
//		processcategory = processcategory0;
//	}
//	if (processrec.unassigned()) {
//		processrec = "";
//	}
//	if (processparameters0.unassigned()) {
//		processparameters = "";
//	} else {
//		processparameters = processparameters0;
//	}
//	if (processresult0.unassigned()) {
//		processresult = "";
//	} else {
//		processresult = processresult0;
//	}
//	if (processcomments0.unassigned()) {
//		processcomments = "";
//	} else {
//		processcomments = processcomments0;
//	}
	processid.defaulter("");
	processcategory = processcategory0.or_default("");
	processrec.defaulter("");
	processparameters = processparameters0.or_default("");
	processresult = processresult0.or_default("");
	processcomments = processcomments0.or_default("");

	// if definitions else
	if (not DEFINITIONS.open("DEFINITIONS", "")) {
		return 0;
	}
	// if definitions else return
	// end

	if (processid == "") {
		processid = SYSTEM.f(4);
	}

	if (processcategory == "HEARTBEAT") {

		FILEERRORMODE = 1;
		if (not processrec.read(DEFINITIONS, "PROCESS*" ^ processid)) {
			processrec = "";
		}
		if (not FILEERROR) {
			processrec(11) = date();
			processrec(12) = time();

			processrec.write(DEFINITIONS, "PROCESS*" ^ processid);
		}
	} else if (processcategory == "LOGOFF" or processcategory == "DELETE") {

		DEFINITIONS.deleterecord("PROCESS*" ^ processid);

		processid = "";

	} else {

		// generate a new processid and record
		if (processid == "") {
			gosub newprocessid(processid);

		} else {

			if (not processrec.read(DEFINITIONS, "PROCESS*" ^ processid)) {
				gosub newprocessid(processid);
			}

			processrec(11) = date();
			processrec(12) = time();
		}

		if (processcategory) {
			processrec(6) = processcategory;
		}

		if (processparameters) {
			processrec(7) = processparameters;
		}
		if (processresult) {
			processrec(13) = processresult;
		}
		if (processcomments) {
			processrec(14) = processcomments;
		}

		processrec.write(DEFINITIONS, "PROCESS*" ^ processid);
	}
	// L468:
	return 0;
}

subroutine newprocessid(io processid) {
	// newprocessid(io processid)
	timex = ostime();

	while (true) {
		processid = var(999999999).rnd().last(8);
		if (not var().read(DEFINITIONS, "PROCESS*" ^ processid)) {
			break;
		}
	}  // loop;

	processrec(1) = date();
	processrec(2) = timex;
	processrec(3) = STATION.trim();
	processrec(4) = SYSTEM.f(17);
	processrec(5) = USERNAME;
	return;
}

libraryexit()
