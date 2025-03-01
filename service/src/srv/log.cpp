#include <exodus/library.h>
libraryinit()

#include <getbackpars.h>
#include <sendmail.h>

var programname;
var text;
var xstation;
var time;
var log;
//var xx;
var bakpars;
var ver;

function main(in programname0, in text0) {

	let s33 = SYSTEM.f(33);

//	if (programname0.unassigned()) {
//		programname = "";
//	} else {
//		programname = programname0;
//	}
//	if (text0.unassigned()) {
//		text = "";
//	} else {
//		text = text0;
//	}
	programname = programname0.or_default("");
	text = text0.or_default("");

	// update the log
	// /////////////

	let year = date().oconv("DY");
	if (s33) {
		xstation = SYSTEM.f(40, 2);
	} else {
		xstation = STATION.trim();
	}
	time = ostime();

	var text2 = lower(text);
	text2.converter("|", VM);
	text2 = trim(text2, VM);

	// Open or create a LOG file per year
	// TODO use only one file and clear old entries perhaps once a year
	// if (openfile("LOG" ^ year, log, "DEFINITIONS")) {
	let logfilename = "LOG" ^ year;
	if (not log.open(logfilename)) {
		//createfile(logfilename);
		if (not createfile(logfilename)) {
			abort(lasterror());
		}
		if (not log.open(logfilename))
			log = "";
	}
	if (log) {

getlogkey:
		time	   = ostime();
		let logkey = xstation.trim() ^ "*" ^ USERNAME ^ "*" ^ date() ^ "*" ^ time;
		if (var().read(log, logkey)) {
			goto getlogkey;
		}

		var entry = programname;
		updater(entry, 2, text2);
		updater(entry, 3, SYSTEM.f(17));
		entry.write(log, logkey);
	}

	// send a logging email address
	// ///////////////////////////

	// get backup parameters
	call getbackpars(bakpars);

	let emailaddrs = bakpars.f(14);
	if ((programname != "SYSMSG" and programname != "SENDMAIL") and emailaddrs) {

		// determine subject
		let subject = "EXODUS Log: " ^ SYSTEM.f(17) ^ " " ^ programname;

		var body = "";
		body ^= _FM "Date=" ^ date().oconv("D") ^ " " ^ time.oconv("MTS") ^ " Local";
		body ^= _FM "Server=" ^ SYSTEM.f(44).trim();
		body ^= _FM "Install=" ^ oscwd();
		// osread ver from 'general\version.dat' then
		let verfilename = "general/version.dat";
//		if (VOLUMES) {
//			// detect Windows with @volumes since we dont include GENERAL.SUBS for OSSLASH
//			verfilename.converter("/", "\\");
//		}
		if (ver.osread(verfilename)) {
			body ^= _FM "Version=" ^ ver.f(1);
		}
		body ^= _FM "Database=" ^ SYSTEM.f(45).trim() ^ " " ^ SYSTEM.f(17);
		body ^= _FM "Process=" ^ SYSTEM.f(24);
		body ^= _FM "User=" ^ USERNAME;
		body ^= _FM "Station=" ^ xstation;
		body ^= _FM "Source=" ^ programname;

		body ^= FM ^ FM ^ text2;

		body.converter(_FM _VM _SM _TM _ST "|", "\r\r\r\r\r\r");
		body.converter("\n", "");
		body = trim(body, "\r");
		body.replacer("\r", "\r\n");

		// sendmail - if it fails, there will be an entry in the log
		call sendmail(emailaddrs, "", subject, body, "", "");
	}

	// restore interactivity
	SYSTEM(33) = s33;

	return 0;
}

libraryexit()
