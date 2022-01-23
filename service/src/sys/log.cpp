#include <exodus/library.h>
libraryinit()

#include <openfile.h>
#include <getbackpars.h>
#include <sendmail.h>

#include <sys_common.h>

var programname;
var text;
var xstation;
var time;
var log;
var xx;
var bakpars;
var ver;

function main(in programname0, in text0) {
	//c sys

	var s33 = SYSTEM.a(33);

	if (programname0.unassigned()) {
		programname = "";
	} else {
		programname = programname0;
	}
	if (text0.unassigned()) {
		text = "";
	} else {
		text = text0;
	}

	//update the log
	///////////////

	var year = var().date().oconv("D").substr(-4, 4);
	if (s33) {
		xstation = SYSTEM.a(40, 2);
		} else {
		xstation = STATION.trim();
	}
	time = ostime();

	var text2 = lower(text);
	text2.converter("|", VM);
	text2 = trim(text2, VM);

	// Open or create a LOG file per year
	// TODO use only one file and clear old entries perhaps once a year
	//if (openfile("LOG" ^ year, log, "DEFINITIONS")) {
	var logfilename = "LOG" ^ year;
	if (not log.open(logfilename)) {
		createfile(logfilename);
		if (not log.open(logfilename))
			log = "";
	}
	if (log) {

getlogkey:
		time = ostime();
		var logkey = xstation.trim() ^ "*" ^ USERNAME ^ "*" ^ var().date() ^ "*" ^ time;
		if (xx.read(log, logkey)) {
			goto getlogkey;
		}

		var entry = programname;
		pickreplacer(entry, 2, text2);
		pickreplacer(entry, 3, SYSTEM.a(17));
		entry.write(log, logkey);
	}

	//send a logging email address
	/////////////////////////////

	//get backup parameters
	call getbackpars(bakpars);

	var emailaddrs = bakpars.a(14);
	if ((programname ne "SYSMSG" and programname ne "SENDMAIL") and emailaddrs) {

		//determine subject
		var subject = "EXODUS Log: " ^ SYSTEM.a(17) ^ " " ^ programname;

		var body = "";
		body ^= FM ^ "Date=" ^ var().date().oconv("D") ^ " " ^ time.oconv("MTS") ^ " Local";
		body ^= FM ^ "Server=" ^ SYSTEM.a(44).trim();
		body ^= FM ^ "Install=" ^ oscwd();
		//osread ver from 'general\version.dat' then
		var verfilename = "general/version.dat";
		if (VOLUMES) {
			//detect Windows with @volumes since we dont include GENERAL.SUBS for OSSLASH
			verfilename.converter("/", "\\");
		}
		if (ver.osread(verfilename)) {
			body ^= FM ^ "Version=" ^ ver.a(1);
		}
		body ^= FM ^ "Database=" ^ SYSTEM.a(45).trim() ^ " " ^ SYSTEM.a(17);
		body ^= FM ^ "Process=" ^ SYSTEM.a(24);
		body ^= FM ^ "User=" ^ USERNAME;
		body ^= FM ^ "Station=" ^ xstation;
		body ^= FM ^ "Source=" ^ programname;

		body ^= FM ^ FM ^ text2;

		body.converter(FM ^ VM ^ SVM ^ TM ^ STM ^ "|", "\r" "\r" "\r" "\r" "\r" "\r");
		body.converter("\n", "");
		body = trim(body, "\r");
		body.swapper("\r", "\r\n");

		//sendmail - if it fails, there will be an entry in the log
		call sendmail(emailaddrs, "", subject, body, "", "", xx);
	}

	//restore interactivity
	SYSTEM(33) = s33;

	return 0;
}

libraryexit()
