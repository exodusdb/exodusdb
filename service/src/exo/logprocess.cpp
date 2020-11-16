#include <exodus/library.h>
libraryinit()

var processcategory;
var processrec;
var processparameters;
var processresult;
var processcomments;
var timex;

function main(io processid, in processcategory0, in sparevar="", in processparameters0="", in processresult0="", in processcomments0="") {
	//c sys io,in,"","","",""

	//obsolete since moved to processes from definitions

	//"heartbeat" called from messaging

	if (processid.unassigned()) {
		processid = "";
	}
	if (sparevar.unassigned()) {
		{}
	}
	if (processcategory0.unassigned()) {
		processcategory = "";
	}else{
		processcategory = processcategory0;
		}
	if (processrec.unassigned()) {
		processrec = "";
	}
	if (processparameters0.unassigned()) {
		processparameters = "";
	}else{
		processparameters = processparameters0;
	}
	if (processresult0.unassigned()) {
		processresult = "";
	}else{
		processresult = processresult0;
	}
	if (processcomments0.unassigned()) {
		processcomments = "";
	}else{
		processcomments = processcomments0;
	}

	//if definitions else
	if (not(DEFINITIONS.open("DEFINITIONS", ""))) {
		return 0;
	}
	// if definitions else return
	// end

	if (processid == "") {
		processid = SYSTEM.a(4);
	}

	if (processcategory == "HEARTBEAT") {

		FILEERRORMODE = 1;
		if (not(processrec.read(DEFINITIONS, "PROCESS*" ^ processid))) {
			processrec = "";
		}
		if (not(FILEERROR)) {
			processrec.r(11, var().date());
			processrec.r(12, var().time());

			processrec.write(DEFINITIONS, "PROCESS*" ^ processid);

		}
	} else if (processcategory == "LOGOFF" or processcategory == "DELETE") {

		DEFINITIONS.deleterecord("PROCESS*" ^ processid);
		
		processid = "";

	} else {

		//generate a new processid and record
		if (processid == "") {
			gosub newprocessid( processid);

		}else{

			if (not(processrec.read(DEFINITIONS, "PROCESS*" ^ processid))) {
				gosub newprocessid( processid);
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

		processrec.write(DEFINITIONS, "PROCESS*" ^ processid);

	}
//L468:
	return 0;

}

subroutine newprocessid(io processid) {
	//newprocessid(io processid)
	timex = ostime();

	while (true) {
		processid = var(999999999).rnd().substr(-8,8);
		var xx;
		if (not(xx.read(DEFINITIONS, "PROCESS*" ^ processid))) {
			xx = "";
		}
	///BREAK;
	if (not xx) break;;
	}//loop;

	processrec.r(1, var().date());
	processrec.r(2, timex);
	processrec.r(3, STATION.trim());
	processrec.r(4, SYSTEM.a(17));
	processrec.r(5, USERNAME);
	return;

}


libraryexit()
