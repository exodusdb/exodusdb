#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <log2.h>

#include <gen_common.h>
#include <win_common.h>

var taskn;//num
var oldtaskn;
var newtaskn;
var xx;
var logtime;
var tn;
var vn;
var taskn2;

function main(in mode) {
	//c sys
	//jbase
	#include <general_common.h>

	#define origfullrec win.registerx(7)

	if (mode eq "GETCHANGEDTASKS") {

		var emailtx2 = "";

		//email changed tasks
		var newtasks = RECORD.a(10);
		var newlocks = RECORD.a(11);
		var oldtasks = origfullrec.a(10);
		var oldlocks = origfullrec.a(11);
		var ntasks = newtasks.count(VM) + (newtasks ne "");
		for (taskn = 1; taskn <= ntasks; ++taskn) {
			var task = newtasks.a(1, taskn);
			if (task) {
				var newlock = newlocks.a(1, taskn);
				if (oldtasks.locate(task,oldtaskn)) {
					var oldlock = oldlocks.a(1, oldtaskn);
					if (newlock ne oldlock) {
						//changed
						emailtx2.r(-1, FM ^ "Task : " ^ task ^ " *CHANGED*" ^ FM ^ "Lock : " ^ newlock ^ FM ^ " was : " ^ oldlock);
					}
				}else{
					emailtx2.r(-1, FM ^ "Task : " ^ task ^ " *CREATED*" ^ FM ^ "Lock : " ^ newlock);
				}
			}
		};//taskn;

		ntasks = oldtasks.count(VM) + (oldtasks ne "");
		for (taskn = 1; taskn <= ntasks; ++taskn) {
			var task = oldtasks.a(1, taskn);
			if (task) {
				if (not(newtasks.locate(task,newtaskn))) {
					var oldlock = oldlocks.a(1, taskn);
					emailtx2.r(-1, FM ^ "Task : " ^ task ^ " *DELETED*" ^ FM ^ "Lock : " ^ oldlock);
				}
			}
		};//taskn;

		ANS = emailtx2;

	} else if (mode eq "FIXUSERPRIVS") {

		//obsolete tasks
		call authorised("%DELETE%" "CHANGE NETWORK TYPE", xx);
		call authorised("%DELETE%" "CHANGE PRINTER DEFINITIONS", xx);
		call authorised("%DELETE%" "CHANGE PRINTER TYPE", xx);
		call authorised("%DELETE%" "CHANGE SCREEN TYPE", xx);
		call authorised("%DELETE%" "CHANGE SCREEN COLORS", xx);
		call authorised("%DELETE%" "CHANGE VIEWER", xx);
		call authorised("%DELETE%" "CHANGE VIEWER CENTRAL", xx);
		call authorised("%DELETE%" "ACCESS OPERATING SYSTEM", xx);
		call authorised("%DELETE%" "OPERATING SYSTEM ACCESS", xx);
		call authorised("%DELETE%" "OTHER OPTIONS ACCESS", xx);
		call authorised("%DELETE%" "OTHER OPTIONS UPDATE", xx);

		//new safe method for renaming tasks
		//copy from old if missing, then delete old
		//is safer method in case install old program and get the old task again!
		call authorised("CURRENCY EXCHANGE RATE OVERRIDE", xx, "JOURNAL POST OVERRIDE EXCHANGE RATE");
		call authorised("%DELETE%" "JOURNAL POST OVERRIDE EXCHANGE RATE", xx);

		//old method

		var newuserprivs = SECURITY;

		//customisation to convert old databases at ptcy/patsalides
		//which had blank passwords not allowed in the new system
		if (SECURITY.a(1).locate("MICHAEL",xx)) {
			if (newuserprivs.osread("PATSALID.CFG")) {
				if (VOLUMES) {
					newuserprivs = newuserprivs.invert();
				}
			}
		}

		var t10 = newuserprivs.a(10);

		t10.converter(".", " ");

		call log2("*rename some keys", logtime);
		t10.swapper(" - ", " ");
		t10.swapper("MEDIA INVOICE - INVOICE WITHOUT ACCOUNT", "MEDIA INVOICE CREATE WITHOUT ACCOUNT");
		t10.swapper("JOB FILE UPDATE OWN JOB", "JOB FILE UPDATE OTHERS JOBS");
		t10.swapper("PRODUCTION QUOTE", "PRODUCTION ESTIMATE");
		t10.swapper(" FILE ", " ");
		t10.swapper("RECURRING ENTRIES", "RECURRING");
		t10.swapper("PRODUCTION ESTIMATE CREATE OWN ORDER NO", "PRODUCTION ESTIMATE CREATE OWN NO");
		t10.swapper("VEHICLE RATE ", "RATECARDS ");
		t10.swapper("MEDIA COSTS ", "MEDIA COST ");

		t10.swapper("BALANCE ACCESS", "FINANCIAL REPORT ACCESS");
		t10.swapper("GENERAL REPORT ", "FINANCIAL STATEMENT ");
		t10.swapper("FINANCIAL STATEMENT DESIGN CREATE", "FINANCIAL REPORT CREATE");
		t10.swapper("FINANCIAL STATEMENT DESIGN MODIFY", "FINANCIAL REPORT UPDATE");
		t10.swapper("FINANCIAL STATEMENT ", "FINANCIAL REPORT ");

		t10.swapper("ANALYSIS REPORT DESIGN CREATE", "BILLING REPORT CREATE");
		t10.swapper("ANALYSIS REPORT DESIGN MODIFY", "BILLING REPORT UPDATE");
		t10.swapper("ADS EXPORT", "MEDIA DIARY EXPORT");
		t10.swapper(VM ^ "ANALYSIS ACCESS", VM ^ "BILLING REPORT ACCESS");
		t10.swapper("LEDGER RE-OPEN", "LEDGER REOPEN");

		if (t10.a(1).locate("POSTING",tn)) {
			if (newuserprivs.a(11, tn) eq "") {
				newuserprivs.r(11, tn, "UA");
			}
			t10.r(1, tn, "JOURNAL POST");
		}
		//swap vm:'POSTING' with vm:'JOURNAL POST' in t10
		t10.swapper(VM ^ "REPOSTING", VM ^ "JOURNAL REPOST");
		t10.swapper(VM ^ "UNPOSTING", VM ^ "JOURNAL UNPOST");
		t10.swapper(VM ^ "POST ", VM ^ "JOURNAL POST ");
		t10.swapper("DAYBOOK", "JOURNAL");
		t10.swapper("JOURNAL JOURNAL", "JOURNAL");
		t10.swapper("RATECARDS ", "RATECARD ");
		t10.swapper("UNITS ", "UNIT ");
		//swap 'SUPPLIER INVOICE ACCESS' with 'MEDIA ':'SUPPLIER INVOICE ACCESS' in t10
		//swap 'SUPPLIER INVOICE CREATE' with 'MEDIA ':'SUPPLIER INVOICE CREATE' in t10
		//swap 'SUPPLIER INVOICE UPDATE' with 'PRODUCTION ':'SUPPLIER INVOICE UPDATE' in t10

		t10.swapper("OWN ORDER NO", "OWN NO");
		t10.swapper("OWN ESTIMATE NO", "OWN NO");
		t10.swapper("OWN JOB NO", "OWN NO");

		call log2("*fix an error", logtime);
		while (true) {
			///BREAK;
			if (not(t10.index("MEDIA MEDIA "))) break;
			t10.swapper("MEDIA MEDIA ", "MEDIA ");
		}//loop;
		while (true) {
			///BREAK;
			if (not(t10.index("PRODUCTION PRODUCTION "))) break;
			t10.swapper("PRODUCTION PRODUCTION ", "PRODUCTION ");
		}//loop;

		t10.trimmer();
		newuserprivs.r(10, t10);

		call log2("*make sure all users have access to company file", logtime);
		if (newuserprivs.a(10).locate("COMPANY ACCESS",vn)) {
			if (newuserprivs.a(11, vn) eq "AA") {
				newuserprivs.r(11, vn, "");
			}
		}

		call log2("*delete some obsolete tasks", logtime);
		var obsoletetasks = "COMPANY ACCESS PARTIAL";
		obsoletetasks.r(-1, "MARKET ACCESS PARTIAL");
		for (var ii = 1; ii <= 9999; ++ii) {
			var tt = obsoletetasks.a(ii);
			///BREAK;
			if (not tt) break;
			if (newuserprivs.a(10).locate(tt,taskn)) {
				newuserprivs.remover(10, taskn);
				newuserprivs.remover(11, taskn);
			}
		};//ii;

		//ensure certain documents cannot be deleted
		if (newuserprivs.a(10).locate("PRODUCTION ORDER DELETE",taskn)) {
			newuserprivs.r(11, taskn, "EXODUS");
		}
		if (newuserprivs.a(10).locate("PRODUCTION ESTIMATE DELETE",taskn)) {
			newuserprivs.r(11, taskn, "EXODUS");
		}
		if (newuserprivs.a(10).locate("JOB DELETE",taskn)) {
			newuserprivs.r(11, taskn, "EXODUS");
		}

		call log2("*delete any superfluous tasks", logtime);
		var tasks = newuserprivs.a(10);
		var locks = newuserprivs.a(11);
		var ntasks = tasks.count(VM) + (tasks ne "");
		for (taskn = ntasks; taskn >= 1; --taskn) {
			var lockx = locks.a(1, taskn);
			var task = tasks.a(1, taskn);
			if (lockx eq "" and (task[-1] eq DQ)) {
deletetask:
				newuserprivs.remover(10, taskn);
				newuserprivs.remover(11, taskn);
			}else{
				//delete duplicate tasks
				if (tasks.a(1).locate(task,taskn2)) {
					if (taskn2 lt taskn) {
						goto deletetask;
					}
				}
			}
		};//taskn;

			/*;
			//move menus to users - detect any non-numeric and move to users 37
			//update users without locking since this will be done really only once
			//at first startup with the new version of programs
			//if definitions is cleared and it reconverts then nothing will be done
			//if userprivs<3,*,1> remains numeric ie date or datetime
			convkey='CONVERTED*MENUS';
			read xx from definitions,convkey else;
				open 'USERS' to users then;
					menuorexpired=newuserprivs<3>;
					tt=menuorexpired;
					convert '0123456789.':vm to '' in tt;
					if tt then;
						//go through them one by one in case subvalued in future
						nusers=count(menuorexpired,vm)+1;
						for usern=1 to nusers;
							menux=menuorexpired<1,usern,1>;
							if num(menux) else;

								usercode=newuserprivs<1,usern>;
								read user from users,usercode then;
									if user<34>='' then;
										user<34>=menux;
										write user on users,usercode;
										end;
									end;

								newuserprivs<3,usern,1>='';

								end;
							next;
						end;
					write date() on definitions,convkey;
					end;
				end;
			*/

		if (newuserprivs ne SECURITY) {
			call log2("*write userprivs back", logtime);
			SECURITY = newuserprivs;
			if (VOLUMES) {
				SECURITY.invert().write(DEFINITIONS, "SECURITY");
			}else{
				SECURITY.write(DEFINITIONS, "SECURITY");
			}
		}

	}

	return 0;
}

libraryexit()
