#include <exodus/library.h>
libraryinit()

#include <openfile.h>
#include <getbackpars.h>
#include <sendmail.h>

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
	}else{
		programname = programname0;
	}
	if (text0.unassigned()) {
		text = "";
	}else{
		text = text0;
	}

	//update the log
	///////////////

	var year = var().date().oconv("D").substr(-4,4);
	if (s33) {
		xstation = SYSTEM.a(40, 2);
		}else{
		xstation = STATION.trim();
	}
	time = ostime();

	var text2 = lower(text);
	text2.converter("|", VM);
	text2 = trim(text2, VM);

	if (openfile("LOG" ^ year, log, "DEFINITIONS")) {

getlogkey:
		time = ostime();
		var logkey = xstation.trim() ^ "*" ^ USERNAME ^ "*" ^ var().date() ^ "*" ^ time;
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
	call getbackpars(bakpars);

	var emailaddrs = bakpars.a(14);
	if ((programname ne "SYSMSG" and programname ne "SENDMAIL") and emailaddrs) {

		//determine subject
		var subject = "EXODUS Log: " ^ SYSTEM.a(17) ^ " " ^ programname;

		var body = "";
		body.r(-1, "Date=" ^ var().date().oconv("D") ^ " " ^ time.oconv("MTS") ^ " Local");
		body.r(-1, "Server=" ^ SYSTEM.a(44).trim());
		body.r(-1, "Install=" ^ oscwd());
		//osread ver from 'general\version.dat' then
		var verfilename = "general/version.dat";
		if (VOLUMES) {
			//detect Windows with @volumes since we dont include GENERAL.SUBS for OSSLASH
			verfilename.converter("/", "\\");
		}
		if (ver.osread(verfilename)) {
			body.r(-1, "Version=" ^ ver.a(1));
		}
		body.r(-1, "Database=" ^ SYSTEM.a(45).trim() ^ " " ^ SYSTEM.a(17));
		body.r(-1, "Process=" ^ SYSTEM.a(24));
		body.r(-1, "User=" ^ USERNAME);
		body.r(-1, "Station=" ^ xstation);
		body.r(-1, "Source=" ^ programname);

		body.r(-1, FM ^ text2);

		body.converter(FM ^ VM ^ SVM ^ TM ^ STM ^ "|", "\r" "\r" "\r" "\r" "\r" "\r");
		body.converter("\n", "");
		body = trim(body, "\r");
		body.swapper("\r", "\r\n");

		//sendmail - if it fails, there will be an entry in the log
		call sendmail(emailaddrs, "", subject, body, "", "", xx);
	}

	//restore interactivity
	SYSTEM.r(33, s33);

	return 0;
}

libraryexit()
