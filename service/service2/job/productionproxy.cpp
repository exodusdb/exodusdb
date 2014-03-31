#include <exodus/library.h>
libraryinit()

#include <readagp.h>
#include <authorised.h>
#include <agencysubs.h>
#include <generalsubs2.h>
#include <locking.h>
#include <unlockall.h>
#include <updtasks.h>
#include <timesheetsubs.h>
#include <select2.h>
#include <quote2.h>
#include <prodordersubs.h>
//#include <ubr.h>
#include <generalsubs.h>
#include <sysmsg.h>
#include <log2.h>

#include <agy.h>
#include <gen.h>
#include <win.h>

var newtask;
var locklist;
var taskid;
var usercodes;
var taskdata;
var usercode;
var islocked;//num
var detailederror;
var nblocktasks;//num
var returndata;
var limitfields;
var limitchecks;
var limitvalues;
var cmd;
var totsize;//num
var modex;
var subr;
var mattasks;
var ntasks;//num
var xx;
var mode;
var tasks;
var proxyname;
var logtime;

function main() {
	//subroutine jobsproxy(request,datax,response)
	var thisname="productionproxy";

	if (not iscommon(gen)) {
		return invalid("gen common is not initialised in " ^ thisname);
	}
	if (not iscommon(agy)) {
		return invalid("agy common is not initialised in " ^ thisname);
	}
	//var().clearcommon();
	win.valid = 1;
	USER4 = "";
	var nbsp = "&nbsp;";

	proxyname="PRODUCTIONPROXY";

	log2("-----productioproxy init", logtime);

	if (!agy_isdefined) {
		call mssg("INITAGENCY NOT DONE");
		return invalid();
	}

	//refresh agency.params every 10 seconds
	call readagp();

	var xx;
	if (xx.read(gen._definitions, "PENDINGUPDATES")) {
		perform("PENDINGUPDATES");
	}

	call cropper(USER0);
	call cropper(USER1);

	mode = USER0.a(1).ucase();
	win.datafile = USER0.a(2);
	var key = USER0.a(3);
	var maxnrecs = 100;

	USER3 = "OK";

	if (mode == "TEST") {

	} else if (mode == "INACTIVEJOBS") {
		USER1.transfer(PSEUDO);
		perform("INACTIVEJOBS");

	} if (mode == "AMENDORDERNO") {

		if (not(authorised("PRODUCTION INVOICE UPDATE", USER4))) {
			return invalid();
		}

		ID = USER0.a(2);
		var neworderno = USER0.a(3);

		win.datafile = "PRODUCTION.INVOICES";
		win.srcfile = agy.productioninvoices;
		if (not(DICT.open("DICT.PRODUCTION.INVOICES", ""))) {
			call fsmsg();
			return invalid();
		}

		if (not(win.srcfile.lock( ID))) {
			USER4 = DQ ^ (ID ^ DQ) ^ " is currently locked. Try again later";
			return invalid();
		}
		win.wlocked = 1;

		if (not(RECORD.read(win.srcfile, ID))) {
			USER4 = DQ ^ (ID ^ DQ) ^ " does not exist in AMENDORDERNO";
unlockamend:
			win.srcfile.unlock( ID);
			return invalid();
		}

		win.orec = RECORD;

		call agencysubs("CHKCLOSEDPERIOD", USER4);
		if (USER4) {
			goto unlockamend;
		}

		RECORD.r(7, neworderno);

		var origstatus = RECORD.a(11);
		RECORD.r(11, "AMENDED");

		call generalsubs2("PREWRITE");

		RECORD.r(11, origstatus);

		RECORD.write(win.srcfile, ID);

		win.srcfile.unlock( ID);

	} else if (mode == "ADDTASK") {

		USER1.transfer(newtask);
		var jobno = newtask.a(1);

		//get exclusive access to add by locking tasks JOBNO
		if (not(locking("LOCK", "TASKS", jobno, xx, locklist, 9, USER4))) {
			return invalid();
		}

		taskid = USER0.a(2);
		if (not taskid) {
			taskid = newtask.a(5);
			if (taskid == "") {
				USER4 = "taskid is missing in " ^ proxyname ^ ",ADDTASK " ^ newtask;
				return invalid();
			}
		}

		usercodes = newtask.a(2);
		var nusers = usercodes.count(VM) + 1;

		for (var usern = 1; usern <= nusers; ++usern) {
			newtask.r(2, usercodes.a(1, usern));

			call updtasks("ADD", taskid, newtask, USER4);
			//if msg then goto invalid
			if (USER4) {
				call mssg(USER4);
			}

		};//usern;

	} else if (mode == "UPDATETASK") {

		//given new task data fields do appropriate updates
		//currently only looks at
		//Field 3 STATUS

		//prepare
		var tasks;
		if (not(tasks.open("TASKS", ""))) {
			call fsmsg();
			return invalid();
		}
		USER1.transfer(taskdata);

		taskid = USER0.a(2);
		if (not taskid) {
			taskid = taskdata.a(5);
			if (not taskid) {
				USER4 = "taskid is missing in " ^ proxyname;
				return invalid();
			}
		}

		if (USER0.a(3) == "STATUS") {
			if (USER0.a(4)) {
				taskdata.r(3, USER0.a(4));
			}else{
				USER4 = "newstatus is missing in " ^ proxyname;
				return invalid();
			}
		}

		//get exclusive access to the task
		if (not(locking("LOCK", "TASKS", taskid, xx, locklist, 9, USER4))) {
			return invalid();
		}

		var oldtask;
		if (not(oldtask.read(tasks, taskid))) {
			USER4 = DQ ^ (taskid ^ DQ) ^ " is missing from tasks";
			return invalid();
		}

		//check updating same data
		if (taskdata.a(6) ne oldtask.a(6)) {
			USER4 = DQ ^ (taskid ^ DQ) ^ " task has been updated by someone else.";
			USER4.r(1, "Please refresh your screen and try again.");
			return invalid();
		}

		USER3 = "OK " ^ taskid;
		newtask = oldtask;
		var addresponse = "";
		if (taskdata.a(3) and taskdata.a(3) ne oldtask.a(3)) {
			newtask.r(3, taskdata.a(3));
			newtask.r(4, var().date() ^ "." ^ (var().time()).oconv("R(0)#5"));
			addresponse = " Status now " ^ newtask.a(3) ^ " (from " ^ oldtask.a(3) ^ ")";
		}

		if (newtask ne oldtask) {
			call updtasks("UPDATE", taskid, newtask, USER4);
			if (USER4) {
				return invalid();
			}
			//return the new task data including new date_time
			USER1 = newtask;
			USER3 ^= addresponse;
		}else{
			USER4 = "Nothing to update in " ^ proxyname ^ ",updatetask";
			return invalid();
		}

	} else if (mode == "APPROVETIMESHEET") {

		//check basic authorisation
		if (not(authorised("TIMESHEET ADMINISTRATION",xx))) {
			if (not(authorised("TIMESHEET APPROVAL", USER4, "TAP"))) {
				call mssg(USER4);
				return invalid();
			}
		}

		//prepare for calling timesheet.subs,prewrite
		win.datafile = "TIMESHEETS";
		win.srcfile = gen.timesheets;
		if (not(DICT.open("DICT", "TIMESHEETS"))) {
			call fsmsg();
			return invalid();
		}

		usercodes = USER1.a(1);
		if (not usercodes) {
			call mssg("Nothing to approve");
			return invalid();
		}

		var idates = USER1.a(2);

		var htx = "";
		htx = "<h3>Timesheet Approval Results</h3>";
		htx ^= "<table cellspacing=2 cellpadding=2>";
		htx ^= "<thead><tr>";
		htx ^= "<td>User Code</td>";
		htx ^= "<td>Date</td>";
		htx ^= "<td>Version</td>";
		htx ^= "<td>Result</td>";
		htx ^= "<td>Comment</td>";
		htx ^= "</tr></thead>";
		var allok = 1;

		var nsheets = usercodes.count(VM) + 1;
		var tt = idates.count(VM) + 1;
		if (tt > nsheets) {
			nsheets = tt;
		}

		for (var sheetn = 1; sheetn <= nsheets; ++sheetn) {

			var code = usercodes.a(1, sheetn);
			var idate = idates.a(1, sheetn);
			var timestamp = USER1.a(3, sheetn);

			//alternate version compacts everything into field1 USERCODE*DATE*TIMESTAMP
			if (not idate) {
				idate = code.field("*", 2);
			}
			if (not timestamp) {
				timestamp = code.field("*", 3);
			}

			//usercode may be omitted on repeats to reduce URL length
			tt = code.field("*", 1);
			if (tt) {
				usercode = tt;
			}

			htx ^= "<tr>";
			htx ^= "<td>" ^ usercode ^ "</td>";
			htx ^= "<td>" ^ idate.oconv("[DATE,4*]") ^ "</td>";
			htx ^= "<td>" ^ timestamp.oconv("[DATETIME,4*]") ^ "</td>";

			if (usercode) {
				ID = usercode.fieldstore("*", 2, 1, idate);
				if (lockrecord("TIMESHEETS", gen.timesheets, ID, "", 3)) {
					islocked = 1;
				}else{
					islocked = 0;
					//detailederror=' is being updated by somebody'
					//goto approvalerror
					//just go ahead and update it now and the other person
					//will not be able to update due to timestamp protection
				}
				if (RECORD.read(gen.timesheets, ID)) {
					if (RECORD.a(8) == "APPROVED") {
						var nn = (RECORD.a(30)).count(VM) + 1;
						tt = RECORD.a(30, nn);
						if (tt == USERNAME) {
							tt = "you";
						}
						var detail = "It was already approved by " ^ tt ^ " on " ^ (RECORD.a(31, nn)).oconv("[DATETIME,4*]");
						USER1.r(10, sheetn, "OK");
						USER1.r(11, sheetn, detail);
						htx ^= "<td>OK</td>";
						htx ^= "<td>" ^ detail ^ "</td>";

					} else if (RECORD.a(6) ne timestamp) {
						detailederror = "It has been changed since the version you are approving. Please wait until the next approval email.";
approvalerror:
						allok = 0;
						//datax<10,sheetn>='OK'
						//datax<11,sheetn>=detail
						//htx:='<td>OK</td>'
						//htx:='<td>':detail:'</td>'

						USER1.r(10, sheetn, "FAILURE");
						USER1.r(11, sheetn, detailederror);
						htx ^= "<td>FAILURE</td>";
						htx ^= "<td>" ^ detailederror ^ "</td>";

					}else{

						win.orec = RECORD;

						//change the timesheet status to approved
						RECORD.r(8, "APPROVED");

						//verify is ok for saving (sufficient hours etc)
						//and ensure version is logged
						win.wlocked = 1;
						win.valid = 1;
						call timesheetsubs("PREWRITE.APPROVING");

						if (not win.valid) {
							var detail = "Cannot approve";
							USER4.transfer(detailederror);
							goto approvalerror;
						}

						RECORD.write(gen.timesheets, ID);
						USER1.r(10, sheetn, "Success");
						//datax<11,sheetn>=''
						htx ^= "<td>" "Success" "</td>";
						htx ^= "<td>" ^ nbsp ^ "</td>";

						//flush any indexes but only if we have control otherwise hangs
						//how to flush indexing if locked - at init.general?
						if (islocked) {
							call timesheetsubs("POSTWRITE");
						}

					}

				}else{
					if (usercode.xlate("USERS", 0, "X")) {
						detailederror = "It doesnt exist or has been deleted";
					}else{
						detailederror = "user " ^ (DQ ^ (usercode ^ DQ)) ^ " doesnt exist";
					}
					goto approvalerror;
				}
				call unlockrecord("TIMESHEETS", gen.timesheets, ID);
			}
		};//sheetn;

		htx ^= "</table>";
		htx.transfer(USER1);
		if (allok) {
			USER3 = "OK";
		}else{
			USER3 = "Errors occurred";
		}

	} else if (mode == "GETMYJOBS") {

		USER1 = "";
		nblocktasks = 0;

		//all users's task's
		if (authorised("TASK ACCESS ALL", xx, "AP2")) {
			usercodes = "";

		//current user's team's tasks only
		} else if (authorised("TASK ACCESS TEAM", xx, "AP2")) {
			var deptcode = USERNAME.xlate("USERS", 21, "X");
			var filenamex = "USERS";
			var linkfilename2 = "";
			var sortselect = " WITH DEPARTMENT_CODE2 " ^ (DQ ^ (deptcode ^ DQ));
			var dictids = "USER_CODE";
			var options = "";
			call select2(filenamex, linkfilename2, sortselect, dictids, options,
				returndata, USER3, limitfields, limitchecks, limitvalues, maxnrecs);
			if (returndata == "") {
				//should never happen since we got the dept code from the current user
				usercodes = USERNAME;
			}else{
				returndata.transfer(usercodes);
			}

			//current user's tasks only
		}else{
			usercodes = USERNAME;
		}

		//add uncompleted tasks for the user
		cmd = "SSELECT TASKS BY KEY";
		if (usercodes) {
			cmd ^= " AND WITH USER_CODE " ^ quote2(usercodes);
		}
		totsize = 0;
		if (not addtasks())
			return invalid();

		//add uncompleted tasks that the user requested
		cmd = "SSELECT TASKS BY KEY";
		if (usercodes) {
			cmd ^= " AND WITH PARENT_USER_CODE " ^ quote2(usercodes);
		}
		if (not addtasks())
			return invalid();

	} else if (mode == "VAL.SUPPINVNO") {
		if (not opendatafile())
			return 0;
		if (not(DICT.open("DICT", "PRODUCTION.ORDERS"))) {
			call fsmsg();
			return 0;
		}
		win.is = USER0.a(3);
		win.isorig = "";
		RECORD = "";
		RECORD.r(10, USER0.a(4));

		call prodordersubs("VAL.SUPPINV");

		if (not USER4 and USER3 and USER3 ne "OK") {
			USER3.splicer(1, 0, "OK ");
		}

	} else if (mode == "GETNEXTNO") {

		if (win.datafile == "PRODUCTION.ORDERS") {

			modex = "DEF.ORDER.NO";
			subr = "PRODORDER.SUBS";

		} else if (win.datafile == "PRODUCTION.INVOICES") {

			modex = "DEF.QUOTE.NO";
			subr = "PRODINV.SUBS";

		} else if (win.datafile == "JOBS") {
			subr = "JOB.SUBS";
			modex = "DEF.JOB.NO." ^ USER0.a(3);

		} else if (not(win.srcfile.open(win.datafile, ""))) {
			call mssg(DQ ^ (win.datafile ^ DQ) ^ " IS INVALID IN " ^ proxyname ^ ",GETNEXTNO");
			return errorresponse();
		}

		perform(subr^" "^modex);

		USER1 = win.isdflt;

	} else if (USER0 == "F2.EXECUTIVE") {
		call agencysubs(USER0,xx);
		USER1 = ANS;
		DATA = "";

	} else if (mode == "COSTESTIMATEPRINT") {
		PSEUDO = USER1;
		execute("prodinvs");

		gosub checkoutputfileexists();

	} else if (mode.field(".", 1) == "UPDATEATTACHMENTS") {
		if (USER1 and (mode.field(".", 2)).length() == 3) {
			USER1.converter(VM, FM);
			USER1.write(gen._definitions, "ENCLOSURES.PROD" ^ mode.field(".", 2));
			USER3 = "OK";
		}

	} else if (mode == "PRINTJOB" or mode == "JOBPRINT") {

		//quick printsend eg from timesheets f6 link
		if (mode == "PRINTJOB") {
			PSEUDO = USER0;

			//JOBPRINT from a form
		}else{
			PSEUDO = FM ^ USER1;
		}

		//execute 'PRINTJOB ':@pseudo<2>
		execute("printjob");
		gosub checkoutputfileexists();

	} else if (mode == "JOBLIST") {
		PSEUDO = USER1;
		var emaildirect = PSEUDO.a(59);

		if ((PSEUDO.a(1)).substr(1, 8) == "PURCHASE") {
			cmd = "LISTPRODORDERS";
		} else if ((PSEUDO.a(1)).substr(1, 9) == "ESTIMATES") {
			cmd = "LISTPRODINVS";
		} else {
			cmd = "LISTJOBS";
		}

		perform(cmd);

		if (not emaildirect) {
			gosub checkoutputfileexists();
		}

	} else if (mode == "TIMESHEETANALYSIS") {
		PSEUDO = USER1;
		execute("analtime");
		gosub checkoutputfileexists();

	} else if (mode == "TIMESHEETPRINT" or mode == "TIMESHEETALERTS") {

		PSEUDO = USER1;
		USER3 = "OK";
		USER4 = "";

		cmd = "ANALTIME2";
		if (PSEUDO.a(1)) {
			cmd ^= " 1/" ^ PSEUDO.a(1).field(".", 2) ^ "/" ^ PSEUDO.a(1).field(".", 1);
		}
		perform(cmd);
		if (PSEUDO.a(2) >= 3) {
			//analtime2 now returns response not msg (except in error)
			//response=msg
			//should not return any report url (since opt 3+ is email)
			USER1 = "";
			USER3.swapper(FM, "\r\n");
			//if response='' then response='OK'
			if ((USER3 == "" or USER3 == "OK") and mode == "TIMESHEETPRINT") {
				USER3 = "No emails required/sent";
			}
		}else{
			gosub checkoutputfileexists();
		}

	//keep for compatibility with old mac version of front end?
	} else if (mode == "PRINTMONTHLYTIMESHEET") {

		//@pseudo=request
		PSEUDO = "";

		execute("analtime2 " ^ USER0.a(2));

		gosub checkoutputfileexists();

	} else if (USER0 == "TIMESHEET.POSTINIT") {
		win.datafile = "TIMESHEETS";
		call timesheetsubs("POSTINIT2");
		USER1 = "";
		for (var ii = 1; ii <= 10; ++ii) {
			USER1 ^= RM ^ win.registerx(ii);
		};//ii;
		//datax[1,0]=''

	} else if (USER0.field(",", 1) == "GETUSERACTIVITIES") {

		usercode = USER0.field(",", 2);
		if (usercode == "") {
			usercode = USERNAME;
		}

		//get dept
		call generalsubs("GETUSERDEPT," ^ usercode);
		var deptcode = ANS.trim();
		deptcode.converter("0123456789", "");

		//select
		//perform 'SSELECT JOB_TYPES WITH DEPARTMENT ':quote(deptcode)
		call select2("JOB_TYPES", "", "WITH DEPARTMENT " ^ (DQ ^ (deptcode ^ DQ)), "CODE NAME", "XML", USER1, USER3, "", "", "");
		var activitycodes = USER1;
		activitycodes.converter(FM, VM);

		USER1 = activitycodes;

	} else {
		USER3 = "System Error: " ^ (DQ ^ (USER0 ^ DQ)) ^ " unrecognised request in " ^ proxyname;
		return errorexit();
	}

	call unlockall(locklist);
	return 0;
}

function errorexit() {
	call unlockall(locklist);
	USER3 = "Error: " ^ USER3;
	return 1;
}

function errorresponse() {
	USER4.converter("|", FM);
	USER4.trimmerf(FM).trimmerb(FM);
	USER4.converter("||", FM ^ FM);
	USER4.swapper(FM ^ FM, "\r\n");
	USER4.swapper(FM, " ");
	USER3 = "Error: " ^ USER4;
	return 1;
}

function opendatafile() {
	if (not(win.srcfile.open(win.datafile, ""))) {
		USER4 = "The " ^ (DQ ^ (win.datafile ^ DQ)) ^ " file is not available";
		gosub errorresponse();
		return false;
	}

	var temp = win.datafile;
	if (temp.substr(-3, 3) == "IES") {
		temp.splicer(-3, 3, "Y");
	}
	if (temp[-1] == "S") {
		temp.splicer(-1, 1, "");
	}
	if (not(authorised(temp ^ " ACCESS", USER4, ""))) {
		gosub errorresponse();
		return false;
	}

	if (not(DICT.open("DICT", win.datafile))) {
		USER4 = "The " ^ (DQ ^ ("DICT." ^ win.datafile ^ DQ)) ^ " file is not available";
		gosub errorresponse();
		return false;
	}
	return true;
}

subroutine checkoutputfileexists() {
	if ((SYSTEM.a(2)).osfile().a(1) > 5) {
		USER1 = SYSTEM.a(2);
		USER3 = "OK";
		if (USER4) {
			USER3 ^= " " ^ USER4;
		}
		USER4 = "";
	}else{
		USER3 = USER4;
		//USER3 ^= FM ^ USER4;
		if (USER3 == "") {
			USER3 = "Error: No output file in  " ^ proxyname ^ "," ^ mode;
			call sysmsg(USER3);
		}
	}

	return;
}

function addtasks() {

	//ignore after a certain number
	if (USER1.length() > 32000) {
		return true;
	}

	var tasks;
	if (not(tasks.open("TASKS", ""))) {
		call fsmsg();
		return invalid();
	}

	var maxtaskfn = 100;
	dim mattasks(maxtaskfn);

	cmd ^= " AND WITH STATUS NE \"Completed\" AND WITH STATUS NE \"Cancelled\"";
	cmd ^= " AND WITH AUTHORISED";

	var tt = cmd.index(" AND WITH ", 1);
	if (tt) {
		cmd.splicer(tt, 4, "");
	}

	var("").select(cmd);

	//get mattasks and ntasks
	if (not addtasks2())
		return invalid();

	//add new tasks to end of current tasks
	for (var fn = 1; fn <= maxtaskfn; ++fn) {
		tt = mattasks(fn);
		if (tt.length()) {
			if (USER1.length() + tt.length() > 32000) {
				return true;
			}
			USER1.r(fn, nblocktasks + 1, mattasks(fn));
		}
	};//fn;
	nblocktasks += ntasks;

	return true;

}

function addtasks2() {

	ntasks = "";
	mattasks = "";
	USER3 = "OK";
	var prevtaskids = USER1.a(5);

	while (var("").readnext(taskid)) {

		//limit number
		if (totsize > 60000) {
			var().clearselect();
			return true;
		}

		//skip existing
		if (prevtaskids.locate(taskid, xx, 1)) {
			continue;
		}

		var task;
		if (not(task.read(tasks, taskid))) {
			continue;
		}

		//force key into field 5 to enable updating
		task.r(5, taskid);

		ntasks += 1;

		task = lower(task);
		var nfs = task.count(VM) + 1;
		for (var fn = 1; fn <= nfs; ++fn) {
			var tt = task.a(1, fn);
			if (tt) {
				mattasks[fn].r(1, ntasks, tt);
				totsize += tt.length() + 1;
			}
		};//fn;

	}

	return true;

}

function invalid(in msg) {
	USER4=msg;
	return invalid();
}

function invalid() {
	call unlockall(locklist);
	USER3 = "";
	call mssg(USER4);
	return 0;
}

libraryexit()
