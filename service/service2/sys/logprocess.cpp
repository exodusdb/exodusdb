#include <exodus/library.h>
libraryinit()

#include <gen.h>

var processrec;
var timex;
var xx;

function main(io processid, in processcategory, in processparameters="", in processresult="", in processcomments="") {
	//obsolete since moved to processes from definitions

	//"heartbeat" called from messaging

	//if definitions else
	if (not(DEFINITIONS.open("DEFINITIONS", ""))) {
		return 0;
	}

	if (processid == "") {
		processid = SYSTEM.a(4);
	}

	if (processcategory == "HEARTBEAT") {

		//FILEERRORMODE = 1;
		if (not processrec.read(DEFINITIONS, "PROCESS*" ^ processid)) {
			processrec = "";
		}
		//if (not FILEERROR) {
			processrec.r(11, var().date());
			processrec.r(12, var().time());

			processrec.write(DEFINITIONS, "PROCESS*" ^ processid);

		//}

	} else if (processcategory == "LOGOFF" or processcategory == "DELETE") {

		DEFINITIONS.deleterecord("PROCESS*" ^ processid);
		processid = "";

	}else{

		//generate a new processid and record
		if (processid == "") {
			gosub newprocessid(processid);
		}else{

			if (not(processrec.read(DEFINITIONS, "PROCESS*" ^ processid))) {
				gosub newprocessid(processid);
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

	return 0;

}

subroutine newprocessid(io processid){

	var timex=ostime().round(2);

	//create a new process id randomly
	do {
		processid = (rnd(99999999)).substr(-8, 8);
		printl("New process Id:" ^ processid);
	} while (xx.read(DEFINITIONS, "PROCESS*" ^ processid));

	processrec="";
	processrec.r(1, var().date());
	processrec.r(2, timex);
	processrec.r(3, STATION);
	processrec.r(4, SYSTEM.a(17));
	processrec.r(5, USERNAME);

	return;
}

libraryexit()
