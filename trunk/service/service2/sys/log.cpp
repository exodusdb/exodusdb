#include <exodus/library.h>
libraryinit()

#include <openfile.h>
#include <readbakpars.h>
#include <sendmail.h>


var station;
var time;
var log;
var bakpars;
var ver;

function main(in programname0, in text0) {
	var s33 = SYSTEM.a(33);

	var programname;
	var text;
	if (programname.unassigned())
		programname = "";
	else
		programname = programname0;
	if (text.unassigned())
		text = "";
	else
		text = text0;

	//update the log
	///////////////

	var year = ((var().date()).oconv("D")).substr(-4, 4);
	if (s33) {
		station = SYSTEM.a(40, 2);
	}else{
		station = STATION.trim();
		}
	//call dostime(time);//seconds and hundredths since midnight
	time = ostime().round(2);

	var text2 = lower(text);
	text2.converter("|", VM);
	text2.trimmer(VM);

	if (openfile("LOG" ^ year, log, "DEFINITIONS")) {

getlogkey:
		//call dostime(time);
		time = var(int(ostime()*100)/100).oconv("MD20P");
		var logkey = station.trim() ^ "*" ^ USERNAME ^ "*" ^ var().date() ^ "*" ^ time;
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
		body.r(-1, "Station=" ^ station);
		body.r(-1, "Source=" ^ programname);

		body.r(-1, FM ^ text2);

		body.converter(FM ^ VM ^ SVM ^ TM ^ STM ^ "|", "\r" "\r" "\r" "\r" "\r" "\r");
		body.converter("\n", "");
		body.trimmer("\r");
		body.swapper("\r", "\r\n");

		//sendmail - if it fails, there will be an entry in the log
		var errormsg;
		call sendmail(emailaddrs, "", subject, body,"","",errormsg);

	}

	//restore interactivity
	SYSTEM.r(33, s33);

	return 0;

}


libraryexit()
