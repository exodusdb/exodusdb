#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <log2.h>

#include <system_common.h>

#include <sys_common.h>
#include <win_common.h>

#define origfullrec_ win.registerx(7)

	var taskn;	//num
var oldtaskn;
var newtaskn;
var xx;
var logtime;
var tn;
var vn;
var taskn2;

function main(in mode) {
	//jbase

	if (mode eq "GETCHANGEDTASKS") {

		var emailtx2 = "";

		//email changed tasks
		var newtasks = RECORD.f(10);
		var newlocks = RECORD.f(11);
		var oldtasks = origfullrec_.f(10);
		var oldlocks = origfullrec_.f(11);
		var ntasks	 = newtasks.fcount(VM);
		for (taskn = 1; taskn <= ntasks; ++taskn) {
			var task = newtasks.f(1, taskn);
			if (task) {
				var newlock = newlocks.f(1, taskn);
				if (oldtasks.locate(task, oldtaskn)) {
					var oldlock = oldlocks.f(1, oldtaskn);
					if (newlock ne oldlock) {
						//changed
						emailtx2(-1) = FM ^ "Task : " ^ task ^ " *CHANGED*" ^ FM ^ "Lock : " ^ newlock ^ FM ^ " was : " ^ oldlock;
					}
				} else {
					emailtx2(-1) = FM ^ "Task : " ^ task ^ " *CREATED*" ^ FM ^ "Lock : " ^ newlock;
				}
			}
		}  //taskn;

		ntasks = oldtasks.fcount(VM);
		for (taskn = 1; taskn <= ntasks; ++taskn) {
			var task = oldtasks.f(1, taskn);
			if (task) {
				if (not(newtasks.locate(task, newtaskn))) {
					var oldlock	 = oldlocks.f(1, taskn);
					emailtx2(-1) = FM ^ "Task : " ^ task ^ " *DELETED*" ^ FM ^ "Lock : " ^ oldlock;
				}
			}
		}  //taskn;

		ANS = emailtx2;

	} else if (mode eq "FIXUSERPRIVS") {

		//obsolete tasks
		call authorised(
			"%DELETE%"
			"CHANGE NETWORK TYPE",
			xx);
		call authorised(
			"%DELETE%"
			"CHANGE PRINTER DEFINITIONS",
			xx);
		call authorised(
			"%DELETE%"
			"CHANGE PRINTER TYPE",
			xx);
		call authorised(
			"%DELETE%"
			"CHANGE SCREEN TYPE",
			xx);
		call authorised(
			"%DELETE%"
			"CHANGE SCREEN COLORS",
			xx);
		call authorised(
			"%DELETE%"
			"CHANGE VIEWER",
			xx);
		call authorised(
			"%DELETE%"
			"CHANGE VIEWER CENTRAL",
			xx);
		call authorised(
			"%DELETE%"
			"ACCESS OPERATING SYSTEM",
			xx);
		call authorised(
			"%DELETE%"
			"OPERATING SYSTEM ACCESS",
			xx);
		call authorised(
			"%DELETE%"
			"OTHER OPTIONS ACCESS",
			xx);
		call authorised(
			"%DELETE%"
			"OTHER OPTIONS UPDATE",
			xx);

		//new safe method for renaming tasks
		//copy from old if missing, then delete old
		//is safer method in case install old program and get the old task again!
		call authorised("CURRENCY EXCHANGE RATE OVERRIDE", xx, "JOURNAL POST OVERRIDE EXCHANGE RATE");
		call authorised(
			"%DELETE%"
			"JOURNAL POST OVERRIDE EXCHANGE RATE",
			xx);

		//old method

		var newuserprivs = SECURITY;

		//customisation to convert old databases at ptcy/patsalides
		//which had blank passwords not allowed in the new system
		if (SECURITY.f(1).locate("MICHAEL", xx)) {
			if (newuserprivs.osread("PATSALID.CFG")) {
			}
		}

		var t10 = newuserprivs.f(10);

		t10.converter(".", " ");

		call log2("*rename some keys", logtime);
		t10.replacer(" - ", " ");
		t10.replacer("MEDIA INVOICE - INVOICE WITHOUT ACCOUNT", "MEDIA INVOICE CREATE WITHOUT ACCOUNT");
		t10.replacer("JOB FILE UPDATE OWN JOB", "JOB FILE UPDATE OTHERS JOBS");
		t10.replacer("JOB QUOTE", "JOB ESTIMATE");
		t10.replacer(" FILE ", " ");
		t10.replacer("RECURRING ENTRIES", "RECURRING");
		t10.replacer("JOB ESTIMATE CREATE OWN ORDER NO", "JOB ESTIMATE CREATE OWN NO");
		t10.replacer("VEHICLE RATE ", "RATECARDS ");
		t10.replacer("MEDIA COSTS ", "MEDIA COST ");

		t10.replacer("BALANCE ACCESS", "FINANCIAL REPORT ACCESS");
		t10.replacer("GENERAL REPORT ", "FINANCIAL STATEMENT ");
		t10.replacer("FINANCIAL STATEMENT DESIGN CREATE", "FINANCIAL REPORT CREATE");
		t10.replacer("FINANCIAL STATEMENT DESIGN MODIFY", "FINANCIAL REPORT UPDATE");
		t10.replacer("FINANCIAL STATEMENT ", "FINANCIAL REPORT ");

		t10.replacer("ANALYSIS REPORT DESIGN CREATE", "BILLING REPORT CREATE");
		t10.replacer("ANALYSIS REPORT DESIGN MODIFY", "BILLING REPORT UPDATE");
		t10.replacer("ADS EXPORT", "MEDIA DIARY EXPORT");
		t10.replacer(VM ^ "ANALYSIS ACCESS", VM ^ "BILLING REPORT ACCESS");
		t10.replacer("LEDGER RE-OPEN", "LEDGER REOPEN");

		if (t10.f(1).locate("POSTING", tn)) {
			if (newuserprivs.f(11, tn) eq "") {
				newuserprivs(11, tn) = "UA";
			}
			t10(1, tn) = "JOURNAL POST";
		}
		//swap vm:'POSTING' with vm:'JOURNAL POST' in t10
		t10.replacer(VM ^ "REPOSTING", VM ^ "JOURNAL REPOST");
		t10.replacer(VM ^ "UNPOSTING", VM ^ "JOURNAL UNPOST");
		t10.replacer(VM ^ "POST ", VM ^ "JOURNAL POST ");
		t10.replacer("DAYBOOK", "JOURNAL");
		t10.replacer("JOURNAL JOURNAL", "JOURNAL");
		t10.replacer("RATECARDS ", "RATECARD ");
		t10.replacer("UNITS ", "UNIT ");
		//swap 'SUPPLIER INVOICE ACCESS' with 'MEDIA ':'SUPPLIER INVOICE ACCESS' in t10
		//swap 'SUPPLIER INVOICE CREATE' with 'MEDIA ':'SUPPLIER INVOICE CREATE' in t10
		//swap 'SUPPLIER INVOICE UPDATE' with 'JOB ':'SUPPLIER INVOICE UPDATE' in t10

		t10.replacer("OWN ORDER NO", "OWN NO");
		t10.replacer("OWN ESTIMATE NO", "OWN NO");
		t10.replacer("OWN JOB NO", "OWN NO");

		call log2("*fix an error", logtime);
		while (true) {
			///BREAK;
			if (not(t10.contains("MEDIA MEDIA ")))
				break;
			t10.replacer("MEDIA MEDIA ", "MEDIA ");
		}  //loop;
		while (true) {
			///BREAK;
			if (not(t10.contains("JOB JOB ")))
				break;
			t10.replacer("JOB JOB ", "JOB ");
		}  //loop;

		t10.trimmer();
		newuserprivs(10) = t10;

		call log2("*make sure all users have access to company file", logtime);
		if (newuserprivs.f(10).locate("COMPANY ACCESS", vn)) {
			if (newuserprivs.f(11, vn) eq "AA") {
				newuserprivs(11, vn) = "";
			}
		}

		call log2("*delete some obsolete tasks", logtime);
		var	 obsoletetasks = "COMPANY ACCESS PARTIAL";
		obsoletetasks(-1)  = "MARKET ACCESS PARTIAL";
		for (const var ii : range(1, 9999)) {
			var tt = obsoletetasks.f(ii);
			///BREAK;
			if (not tt)
				break;
			if (newuserprivs.f(10).locate(tt, taskn)) {
				newuserprivs.remover(10, taskn);
				newuserprivs.remover(11, taskn);
			}
		}  //ii;

		//ensure certain documents cannot be deleted
		if (newuserprivs.f(10).locate("JOB ORDER DELETE", taskn)) {
			newuserprivs(11, taskn) = "EXODUS";
		}
		if (newuserprivs.f(10).locate("JOB ESTIMATE DELETE", taskn)) {
			newuserprivs(11, taskn) = "EXODUS";
		}
		if (newuserprivs.f(10).locate("JOB DELETE", taskn)) {
			newuserprivs(11, taskn) = "EXODUS";
		}

		call log2("*delete any superfluous tasks", logtime);
		var	 tasks	= newuserprivs.f(10);
		var	 locks	= newuserprivs.f(11);
		var	 ntasks = tasks.fcount(VM);
		for (taskn = ntasks; taskn >= 1; --taskn) {
			var lockx = locks.f(1, taskn);
			var task  = tasks.f(1, taskn);
			if (lockx eq "" and (task.ends(DQ))) {
deletetask:
				newuserprivs.remover(10, taskn);
				newuserprivs.remover(11, taskn);
			} else {
				//delete duplicate tasks
				if (tasks.f(1).locate(task, taskn2)) {
					if (taskn2 lt taskn) {
						goto deletetask;
					}
				}
			}
		}  //taskn;

		if (newuserprivs ne SECURITY) {
			call log2("*write userprivs back", logtime);
			SECURITY = newuserprivs;
			if (VOLUMES) {
				SECURITY.invert().write(DEFINITIONS, "SECURITY");
			} else {
				SECURITY.write(DEFINITIONS, "SECURITY");
			}
		}
	}

	return 0;
}

libraryexit()
