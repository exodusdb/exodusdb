#include <exodus/program.h>
programinit()

#include <hashpass.h>
#include <initcompany.h>
#include <initgeneral.h>
#include <initgeneral2.h>

#include <srv_common.h>
#include <req_common.h>

//var xx;

function main() {

	// NB numbers and names MUST match those in the matching common .h files
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
	mv.namedcommon[req_common_no] = new req_common;
	mv.namedcommon[srv_common_no] = new srv_common;
	// mv.namedcommon[agy_common_no]=new agy_common;
#pragma GCC diagnostic pop

	/*cat ~/pickos/dic/ *.SQL | psql -h 127.0.0.1 -U exodus exodus*/

	// gen.addresses="addresses";
	// gen.timesheets="timesheets";
	// gen.companies="companies";
	// gen.currencies="currencies";
	// gen.documents="documents";

	//createfile("VOC");
	//createfile("DICT.DEFINITIONS");
	//createfile("DEFINITIONS");
	//createfile("PROCESSES");
	//createfile("REQUESTLOG");
	//createfile("SYS_MESSAGES");
	//createfile("COMPANIES");
	//createfile("DOCUMENTS");
	//createfile("USERS");
	//createfile("LOCKS");
	//createfile("STATISTICS");
	//createfile("CHANGELOG");

	if (not open("VOC") and not createfile("VOC"))
		abort(lasterror());

	if (not open("DICT.DEFINITIONS") and not createfile("DICT.DEFINITIONS"))
		abort(lasterror());

	if (not open("DEFINITIONS") and not createfile("DEFINITIONS"))
		abort(lasterror());

	if (not open("PROCESSES") and not createfile("PROCESSES"))
		abort(lasterror());

	if (not open("REQUESTLOG") and not createfile("REQUESTLOG"))
		abort(lasterror());

	if (not open("SYS_MESSAGES") and not createfile("SYS_MESSAGES"))
		abort(lasterror());

	if (not open("COMPANIES") and not createfile("COMPANIES"))
		abort(lasterror());

	if (not open("DOCUMENTS") and not createfile("DOCUMENTS"))
		abort(lasterror());

	if (not open("USERS") and not createfile("USERS"))
		abort(lasterror());

	if (not open("LOCKS") and not createfile("LOCKS"))
		abort(lasterror());

	if (not open("STATISTICS") and not createfile("STATISTICS"))
		abort(lasterror());

	if (not open("CHANGELOG") and not createfile("CHANGELOG"))
		abort(lasterror());

	SYSTEM(33) = "X";

	// module
	APPLICATION = "EXODUS";

	// user
	// USERNAME="exodus";
	USERNAME = "EXODUS";

	// database
	var databasecode = osgetenv("EXO_DATA");
	if (not databasecode)
		databasecode = "exodus";
	SYSTEM(17) = databasecode;

	// process no
	SYSTEM(24) = THREADNO;

	// system mode
	SYSTEM(33, 1) = "1";
	// user
	SYSTEM(17)	  = "exodus";  //database code
	SYSTEM(23)	  = "EXODUS";  //database name
	SYSTEM(33, 2) = "EXODUS";  //connection
	SYSTEM(33, 3) = "EXODUS";

	// f5 key
	PRIORITYINT(2) = "x";

	// create database dir
	let datadir = "../data/";
	if (not osdir(datadir))
		abort(datadir.quote() ^ " data directory does not exist");
	let databasedir = datadir ^ databasecode;
	if (osmkdir(databasedir)) {
		//osshell("chmod g+rws " ^ databasedir);
		if (not osshell("chmod g+rws " ^ databasedir)) {
			abort(lasterror());
		}
		//osshell("setfacl -d -m g::rw " ^ databasedir);
		if (not osshell("setfacl -d -m g::rw " ^ databasedir)) {
			abort(lasterror());
		}
	}

	// ensure MARKETS file exists and has at least an ALL markets record
	if (not open("MARKETS")) {
		//createfile("MARKETS");
		if (not createfile("MARKETS")) {
			abort(lasterror());
		}
	}
	select("MARKETS");
	if (hasnext())
		clearselect();
	else
		write("All Markets", "MARKETS", "ALL");

	/*
	let agp="";
	if (not read(agp,"DEFINITIONS","AGENCY.PARAMS")) {
		agp=invert("DEVELOPMENT DATABASE");
		agp(4) = 1; //short dates
		// agp(5) = 1;//split extras
		agp(7) = 1;//wording on schedules
		write(agp,"DEFINITIONS","AGENCY.PARAMS");
	}
	*/

	// exodus pass
	var rec = "";
	// printl(hashpass("ALSOWINE"));
	rec(7) = hashpass("ALSOWINE");
	write(rec on "DEFINITIONS", "EXODUS");

	// call initgeneral();
	perform("initgeneral LOGIN");
	/*
	call initcompany();

    //call initgeneral2("CREATEALERTS");
    call initgeneral2("UPDATEIPNOS4EXODUS");
    //call initgeneral2("GETENV");
    call initgeneral2("FIXURLS");
    call initgeneral2("UPDATEUSERS");
    call initgeneral2("TRIMREQUESTLOG");
    //call initgeneral2("LASTLOGWARNING");
    //call initgeneral2("OSCLEANUP");
	*/

	// kick off with initgeneral or some custom command
	let cmd = SENTENCE.field(" ", 2, 9999);
	if (cmd)
		perform(cmd);
	else
		perform("listen");

	return 0;
}

programexit()
