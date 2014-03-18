#include <exodus/library.h>
libraryinit()

#include <openfile.h>
#include <readbakpars.h>
#include <sendmail.h>


var STATION;
var time;
var log;
var bakpars;
var ver;

function main() {
	var s33 = SYSTEM.a(33);

	if (programname.unassigned()) {
		programname = "";
	}
	if (text.unassigned()) {
		text = "";
	}

	//update the log
	///////////////

	var year = ((var().date()).oconv("D")).substr(-4, 4);
	if (s33) {
		STATION = SYSTEM.a(40, 2);
	}else{
		STATION.trimmer();
		}
	call dostime(time);

	var text2 = lower(text);
	text2.converter("|", VM);
	text2 = trim2(text2, VM);

	if (openfile("LOG" ^ year, log, "DEFINITIONS")) {

getlogkey:
		call dostime(time);
		var logkey = STATION.trim() ^ "*" ^ USERNAME ^ "*" ^ var().date() ^ "*" ^ time;
		var xx;
		if (xx.read(log, logkey)) {
			goto getlogkey;
		}

		var entry = programname;
		entry.r(2, text2);
		entry.r(3, SYSTEM.a(17));
		entry.write(log, logkey);
	}

	//send a logging email address
	/////////////////////////////

	//get backup parameters
	call readbakpars(bakpars);

	var emailaddrs = bakpars.a(14);
	if (programname ne "SYSMSG" and programname ne "SENDMAIL" and emailaddrs) {

		//determine subject
		var subject = "NEOSYS Log: " ^ SYSTEM.a(17) ^ " " ^ programname;

		var body = "";
		body.r(-1, "Date=" ^ (var().date()).oconv("D") ^ " " ^ time.oconv("MTS") ^ " Local");
		body.r(-1, "Server=" ^ SYSTEM.a(44).trim());
		body.r(-1, "Install=" ^ var().osdir());
		if (ver.osread("general\\version.dat")) {
			body.r(-1, "Version=" ^ ver.a(1));
		}
		body.r(-1, "Database=" ^ SYSTEM.a(45).trim() ^ " " ^ SYSTEM.a(17));
		body.r(-1, "Process=" ^ SYSTEM.a(24));
		body.r(-1, "User=" ^ USERNAME);
		body.r(-1, "Station=" ^ STATION);
		body.r(-1, "Source=" ^ programname);

		body.r(-1, FM ^ text2);

		body.converter(FM ^ VM ^ SVM ^ TM ^ STM ^ "|", "\r" "\r" "\r" "\r" "\r" "\r");
		body.converter("\n", "");
		body = trim2(body, "\r");
		body.swapper("\r", "\r\n");

		//sendmail - if it fails, there will be an entry in the log
		call sendmail(emailaddrs, "", subject, body);

	}

	//restore interactivity
	SYSTEM.r(33, s33);

	return 0;

}


libraryexit()