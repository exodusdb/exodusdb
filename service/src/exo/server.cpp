#include <exodus/program.h>
programinit()

#include <win_common.h>
#include <gen_common.h>
#include <agy_common.h>
#include <initcompany.h>
#include <hashpass.h>
#include <initgeneral.h>
#include <initgeneral2.h>

var xx;

function main() {

	//NB numbers and names MUST match those in the matching common .h files
	mv.labelledcommon[win_common_no]=new win_common;
	mv.labelledcommon[gen_common_no]=new gen_common;
	mv.labelledcommon[agy_common_no]=new agy_common;

	/*cat ~/arev/dic/ *.SQL | psql -h 127.0.0.1 -U exodus exodus*/

	//gen.addresses="addresses";
	//gen.timesheets="timesheets";
	//gen.companies="companies";
	//gen.currencies="currencies";
	//gen.documents="documents";
	//gen.units="units";

	createfile("VOC");
	createfile("DICT_DEFINITIONS");
	createfile("DEFINITIONS");
	createfile("PROCESSES");
	createfile("REQUESTLOG");
	createfile("SYS_MESSAGES");
	createfile("COMPANIES");
	createfile("DOCUMENTS");
	createfile("USERS");
	createfile("LOCKS");
	createfile("STATISTICS");
	createfile("CHANGELOG");

	SYSTEM.r(33,"X");

	//module
	APPLICATION="EXODUS";

	//user
	USERNAME="exodus";

	//database
	var databasecode="exodus";
	SYSTEM.r(17,databasecode);

	//process no
	SYSTEM.r(24,PROCESSNO);

	//f5 key
	PRIORITYINT.r(2,"x");

	//create database dir
	var datadir="../data/";
	if (not osdir(datadir))
		abort(datadir.quote() ^ " data directory does not exist");
	var databasedir=datadir ^ databasecode;
	if (osmkdir(databasedir)) {
		osshell("chmod g+rws " ^ databasedir);
		osshell("setfacl -d -m g::rw " ^ databasedir);
	}

	//ensure MARKETS file exists and has at least an ALL markets record
	if (not open("MARKETS")) {
		createfile("MARKETS");
	}
	select("MARKETS");
	if (hasnext())
		clearselect();
	else
		write("All Markets","MARKETS","ALL");

	/*
	var agp="";
	if (not read(agp,"DEFINITIONS","AGENCY.PARAMS")) {
		agp=invert("DEVELOPMENT DATABASE");
		agp.r(4,1);//short dates
		//agp.r(5,1);//split extras
		agp.r(7,1);//wording on schedules
		write(agp,"DEFINITIONS","AGENCY.PARAMS");
	}
	*/

	//exodus pass
	var rec="";
	printl(hashpass("ALSOWINE"));
	write(rec.r(7,hashpass("ALSOWINE")),"DEFINITIONS","EXODUS");

	//call initgeneral();
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

	//kick off with initgeneral or some custom command
	var cmd=SENTENCE.field(" ",2,9999);
	if (cmd)
		perform(cmd);
	else {
		//batch mode
		SYSTEM.r(33,1,"1");
		//user
		SYSTEM.r(33,2,"EXODUS");
		SYSTEM.r(33,3,"EXODUS");

		perform("listen");
	}

	return 0;
}

programexit()
