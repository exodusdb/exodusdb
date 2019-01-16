#include <exodus/library.h>
libraryinit()

#include <readagp.h>
#include <authorised.h>
#include <agencysubs.h>
#include <generalsubs2.h>
#include <locking.h>
#include <updtasks.h>
#include <timesheetsubs.h>
#include <quote2.h>
#include <select2.h>
#include <prodordersubs.h>
#include <generalsubs.h>
#include <convpdf.h>
#include <sysmsg.h>
#include <validjob.h>

#include <agy.h>
#include <gen.h>
#include <win.h>

var stationery;
var xx;
var mode;
var newtask;
var locklist;
var taskid;
var usercodes;
var tasks;
var taskdata;
var tt;
var usercode;
var islocked;//num
var detailederror;
var nblocktasks;//num
var deptcode;
var returndata;
var limitfields;
var limitchecks;
var limitvalues;
var cmd;
var cmd2;
var totsize;//num
var errors;
var mattasks;
var ntasks;//num

function main() {
	//!subroutine jobsproxy(request,datax,response)
	//c job

	//global stationery,xx,tasks,mode,tt

	//var().clearcommon();

	win.valid = 1;
	USER4 = "";
	var nbsp = "&nbsp;";
	var printopts = "";
	stationery = "";

	//refresh agency.params every 10 seconds
	call readagp();

	if (xx.read(DEFINITIONS, "PENDINGUPDATES")) {
		execute("PENDINGUPDATES");
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

	} else if (mode == "INACTIVEJOBEXECS") {
		USER1.transfer(PSEUDO);
		PSEUDO.r(3, "JOBS");
		perform("INACTIVEEXECUTIVES");

	} else if (mode == "AMENDORDERNO") {

		if (not(authorised("PRODUCTION INVOICE UPDATE", USER4))) {
			gosub unlockinvalid();
		}

		ID = USER0.a(2);
		var neworderno = USER0.a(3);

		win.datafile = "PRODUCTION_INVOICES";
		win.srcfile = agy.productioninvoices;
		if (not(DICT.open("DICT.PRODUCTION_INVOICES", ""))) {
			call fsmsg();
			var().stop();
		}

		if (not(win.srcfile.lock( ID))) {
			USER4 = DQ ^ (ID ^ DQ) ^ " is currently locked. Try again later";
			gosub unlockinvalid();
		}
		win.wlocked = 1;

		if (not(RECORD.read(win.srcfile, ID))) {
			USER4 = DQ ^ (ID ^ DQ) ^ " does not exist in AMENDORDERNO";
unlockamend:
			win.srcfile.unlock( ID);
			gosub unlockinvalid();
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
		if (not(locking("LOCK", "TASKS", jobno, "", locklist, 9, USER4))) {
			gosub unlockinvalid();
		}

		taskid = USER0.a(2);
		if (not taskid) {
			taskid = newtask.a(5);
			if (taskid == "") {
				USER4 = "taskid is missing in productionproxy,addtask " ^ newtask;
				gosub unlockinvalid();
			}
		}

		usercodes = newtask.a(2);
		var nusers = usercodes.count(VM) + 1;

		for (var usern = 1; usern <= nusers; ++usern) {
			newtask.r(2, usercodes.a(1, usern));

			call updtasks("ADD", taskid, newtask, USER4);
			//if msg then gosub unlockinvalid
			if (USER4) {
				call mssg(USER4);
			}

		};//usern;

	} else if (mode == "UPDATETASK") {

		//given new task data fields do appropriate updates
		//currently only looks at
		//Field 3 STATUS

		//prepare
		if (not(tasks.open("TASKS", ""))) {
			call fsmsg();
			var().stop();
		}
		USER1.transfer(taskdata);

		taskid = USER0.a(2);
		if (not taskid) {
			taskid = taskdata.a(5);
			if (not taskid) {
				USER4 = "taskid is missing in productionproxy";
				gosub unlockinvalid();
			}
		}

		if (USER0.a(3) == "STATUS") {
			if (USER0.a(4)) {
				taskdata.r(3, USER0.a(4));
			}else{
				USER4 = "newstatus is missing in productionproxy";
				gosub unlockinvalid();
			}
		}

		//get exclusive access to the task
		if (not(locking("LOCK", "TASKS", taskid, "", locklist, 9, USER4))) {
			gosub unlockinvalid();
		}

		var oldtask;
		if (not(oldtask.read(tasks, taskid))) {
			USER4 = DQ ^ (taskid ^ DQ) ^ " is missing from tasks";
			gosub unlockinvalid();
		}

		//check updating same data
		if (taskdata.a(6) ne oldtask.a(6)) {
			USER4 = DQ ^ (taskid ^ DQ) ^ " task has been updated by someone else.";
			USER4.r(1, "Please refresh your screen and try again.");
			gosub unlockinvalid();
		}

		USER3 = "OK " ^ taskid;
		newtask = oldtask;
		var addresponse = "";
		if (taskdata.a(3) and taskdata.a(3) ne oldtask.a(3)) {
			newtask.r(3, taskdata.a(3));
			newtask.r(4, var().date() ^ "." ^ var().time().oconv("R(0)#5"));
			addresponse = " Status now " ^ newtask.a(3) ^ " (from " ^ oldtask.a(3) ^ ")";
		}

		if (newtask ne oldtask) {
			call updtasks("UPDATE", taskid, newtask, USER4);
			if (USER4) {
				gosub unlockinvalid();
			}
			//return the new task data including new date_time
			USER1 = newtask;
			USER3 ^= addresponse;
		}else{
			USER4 = "Nothing to update in productionproxy,updatetask";
			gosub unlockinvalid();
		}

	} else if (mode == "APPROVETIMESHEET") {

		//check basic authorisation
		if (not(authorised("TIMESHEET ADMINISTRATION", xx))) {
			if (not(authorised("TIMESHEET APPROVAL", USER4, "TAP"))) {
				call mssg(USER4);
				var().stop();
			}
		}

		//prepare for calling timesheet.subs,prewrite
		win.datafile = "TIMESHEETS";
		win.srcfile = gen.timesheets;
		if (not(DICT.open("dict_TIMESHEETS"))) {
			call fsmsg();
			var().stop();
		}

		usercodes = USER1.a(1);
		if (not usercodes) {
			call mssg("Nothing to approve");
			var().stop();
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
		tt = idates.count(VM) + 1;
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
						var nn = RECORD.a(30).count(VM) + 1;
						tt = RECORD.a(30, nn);
						if (tt == USERNAME) {
							tt = "you";
						}
						var detail = "It was already approved by " ^ tt ^ " on " ^ RECORD.a(31, nn).oconv("[DATETIME,4*]");
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

					} else {

						win.orec = RECORD;

						//change the timesheet status to approved
						RECORD.r(8, "APPROVED");

						//verify is ok for saving (sufficient hours etc)
						//and ensure version is logged
						win.wlocked = 1;
						win.valid = 1;
						call timesheetsubs("PREWRITE.APPROVING");

						if (not(win.valid)) {
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
//L2055:
					//dummy comment to assist decompilation to c++
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
			deptcode = USERNAME.xlate("USERS", 21, "X");
			var filenamex = "USERS";
			var linkfilename2 = "";
			var sortselect = " WITH DEPARTMENT_CODE2 " ^ quote2(deptcode);
			var dictids = "USER_CODE";
			var options = "";
			call select2(filenamex, linkfilename2, sortselect, dictids, options, returndata, USER3, limitfields, limitchecks, limitvalues, maxnrecs);
			if (returndata == "") {
				//should never happen since we got the dept code from the current user
				usercodes = USERNAME;
			}else{
				returndata.transfer(usercodes);
			}

		//current user's tasks only
		} else {
			usercodes = USERNAME;
		}
//L2373:
		cmd = "SSELECT TASKS BY-DSND STATUS_DATETIME";
		cmd ^= " AND WITH STATUS_DATETIME GE " ^ (DQ ^ ((var().date() - 365).oconv("[DATE,4*]") ^ DQ));
		cmd2 = "";

		//add uncompleted tasks for the user
		if (usercodes) {
			cmd2 = " AND WITH USER_CODE " ^ quote2(usercodes);
		}
		totsize = 0;
		gosub addtasks();

		//add uncompleted tasks that the user requested
		if (usercodes) {
			cmd2 = " AND WITH PARENT_USER_CODE " ^ quote2(usercodes);
		}
		gosub addtasks();

	} else if (mode == "VAL.SUPPINVNO") {
		gosub opendatafile();
		if (not(DICT.open("dict_PRODUCTION_ORDERS"))) {
			call fsmsg();
			return 0;
		}
		win.is = USER0.a(3);
		win.isorig = "";
		RECORD = "";
		RECORD.r(10, USER0.a(4));
		call prodordersubs("VAL.SUPPINV");
		if ((not USER4 and USER3) and USER3 ne "OK") {
			USER3.splicer(1, 0, "OK ");
		}

	} else if (mode == "COSTESTIMATEPRINT") {
		PSEUDO = USER1;
		stationery = PSEUDO.a(39);
		execute("PRODINVS");

		gosub checkoutputfileexists();

	} else if (mode.field(".", 1) == "UPDATEATTACHMENTS") {
		if (USER1 and ((mode.field(".", 2)).length() == 3)) {
			USER1.converter(VM, FM);
			USER1.write(DEFINITIONS, "ENCLOSURES.PROD" ^ mode.field(".", 2));
			USER3 = "OK";
		}

	} else if ((mode == "PRINTJOB") or (mode == "JOBPRINT")) {

		//quick printsend eg from timesheets f6 link
		if (mode == "PRINTJOB") {
			PSEUDO = USER0;

			//JOBPRINT from a form
		}else{
			PSEUDO = FM ^ USER1;
		}

		stationery = PSEUDO.a(3 + 1);
		execute("PRINTJOB");
		gosub checkoutputfileexists();

	} else if (mode == "JOBLIST") {
		PSEUDO = USER1;
		var emaildirect = PSEUDO.a(59);

		if (PSEUDO.a(1).substr(1,8) == "PURCHASE") {
			cmd = "LISTPRODORDERS";
		} else if (PSEUDO.a(1).substr(1,9) == "ESTIMATES") {
			cmd = "LISTPRODINVS";
		} else {
			cmd = "LISTJOBS";
		}
		execute(cmd);

		if (not emaildirect) {
			gosub checkoutputfileexists();
		}

	} else if (mode == "TIMESHEETANALYSIS") {
		PSEUDO = USER1;
		execute("ANALTIME");
		gosub checkoutputfileexists();

	} else if ((mode == "TIMESHEETPRINT") or (mode == "TIMESHEETALERTS")) {

		PSEUDO = USER1;
		var pseudox = USER1;
		USER3 = "OK";
		USER4 = "";

		cmd = "ANALTIME2";
		if (PSEUDO.a(1)) {
			cmd ^= " 1/" ^ PSEUDO.a(1).field(".", 2) ^ "/" ^ PSEUDO.a(1).field(".", 1);
		}
		perform(cmd);
		if (pseudox.a(2) >= 3) {
			//analtime2 now returns response not msg (except in error)
			//response=msg
			//should not return any report url (since opt 3+ is email)
			USER1 = "";
			USER3.swapper(FM, "\r\n");
			//if response='' then response='OK'
			if (((USER3 == "") or (USER3 == "OK")) and (mode == "TIMESHEETPRINT")) {
				USER3 = "No emails required/sent";
			}
		}else{
			gosub checkoutputfileexists();
		}

	//keep for compatibility with old mac version of front end
	} else if (mode == "PRINTMONTHLYTIMESHEET") {

		//@pseudo=request
		PSEUDO = "";
		execute("ANALTIME2 " ^ USER0.a(2));
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

		//VERY similar code in timesheet.subs and jobproxy GETACTIVITIES
		//TODO deduplicate

		usercode = USER0.field(",", 2);
		//if usercode='' then usercode=@username

		//get dept
		if (usercode) {
			call generalsubs("GETUSERDEPT," ^ usercode);
			deptcode = ANS.trim();
			deptcode.converter("0123456789", "");
		}else{
			deptcode = "";
		}

		//select
		call select2("JOB_TYPES", "", "WITH DEPARTMENT " ^ quote2(deptcode), "CODE NAME", "XML", USER1, USER3, "", "", "");
		var activitycodes = USER1;
		activitycodes.converter(FM, VM);

		USER1 = activitycodes;

	} else {
		USER3 = "System Error: " ^ (DQ ^ (USER0 ^ DQ)) ^ " unrecognised request in PRODUCTIONPROXY";
		var().stop();
	}
//L3367:
/////
	//exit:
	/////
	call locking("UNLOCKALL", "", "", "", locklist, 0, xx);
	var().stop();
	return 0;

}

subroutine opendatafile() {
	if (not(win.srcfile.open(win.datafile, ""))) {
		USER4 = "The " ^ (DQ ^ (win.datafile ^ DQ)) ^ " file is not available";
		gosub errorresponse();
		var().stop();
	}

	var temp = win.datafile;
	if (temp.substr(-3,3) == "IES") {
		temp.splicer(-3, 3, "Y");
	}
	if (temp[-1] == "S") {
		temp.splicer(-1, 1, "");
	}
	if (not(authorised(temp ^ " ACCESS", USER4, ""))) {
		gosub errorresponse();
		var().stop();
	}

	if (not(DICT.open("DICT", win.datafile))) {
		USER4 = "The " ^ (DQ ^ ("DICT." ^ win.datafile ^ DQ)) ^ " file is not available";
		gosub errorresponse();
		var().stop();
	}
	return;

}

subroutine errorresponse() {
	USER4.converter("|", FM);
	USER4 = trim(USER4, FM, "FB");
	USER4.converter("||", FM ^ FM);
	USER4.swapper(FM ^ FM, "\r\n");
	USER4.swapper(FM, " ");
	USER3 = "Error: " ^ USER4;
	var().stop();
	return;

}

subroutine checkoutputfileexists() {
	if (SYSTEM.a(2).osfile().a(1) > 5) {

		//convert to pdf
		if (stationery > 2) {
			call convpdf(stationery, errors);
			if (errors) {
				USER4.r(-1, errors);
			}
		}

		USER1 = SYSTEM.a(2);
		USER3 = "OK";
		if (USER4) {
			USER3 ^= " " ^ USER4;
		}
		USER4 = "";
	}else{
		USER3 = USER4;
		if (USER3 == "") {
			USER3 = "Error: No output file in PRODUCTIONPROXY " ^ mode;
			call sysmsg(USER3);
		}
	}

	return;

}

subroutine addtasks() {

	if (USER1.length() > 60000) {
		return;
	}

	if (not(tasks.open("TASKS", ""))) {
		call fsmsg();
		var().stop();
	}

	var maxtaskfn = 100;
	dim mattasks(maxtaskfn);

	var fullcmd = cmd ^ cmd2;
	fullcmd ^= " AND WITH STATUS NE \"Completed\" AND WITH STATUS NE \"Cancelled\"";
	fullcmd ^= " AND WITH AUTHORISED";

	tt = fullcmd.index(" AND WITH ", 1);
	if (tt) {
		fullcmd.splicer(tt, 4, "");
	}

	perform(fullcmd ^ " (S)");

	//get mattasks and ntasks
	gosub addtasks2();

	//add new tasks to end of current tasks
	for (var fn = 1; fn <= maxtaskfn; ++fn) {
		tt = mattasks(fn);
		if (tt.length()) {
			if (USER1.length() + tt.length() > 60000) {
				return;
			}
			USER1.r(fn, nblocktasks + 1, mattasks(fn));
		}
	};//fn;
	nblocktasks += ntasks;

	return;

}

subroutine addtasks2() {

	ntasks = "";
	mattasks="";
	USER3 = "OK";
	var prevtaskids = USER1.a(5);

nexttask:
/////////

	if (totsize > 60000) {
		var().clearselect();
		return;
	}

	if (not(readnext(taskid))) {
		return;
	}

	if (prevtaskids.locateusing(taskid, VM, xx)) {
		goto nexttask;
	}

	var task;
	if (not(task.read(tasks, taskid))) {
		goto nexttask;
	}

	//remove jobs not authorised unless task is the users
	if (task.a(2) ne USERNAME) {
		var jobno = taskid.field("*", 1);
		if (not(validjob("LIST", jobno, xx, USER4))) {
			goto nexttask;
		}
	}

	//force key into field 5 to enable updating
	task.r(5, taskid);

	ntasks += 1;

	task = lower(task);
	var nfs = task.count(VM) + 1;
	for (var fn = 1; fn <= nfs; ++fn) {
		tt = task.a(1, fn);
		if (tt) {
			mattasks(fn).r(1, ntasks, tt);
			totsize += tt.length() + 1;
		}
	};//fn;

	goto nexttask;

}

subroutine unlockinvalid() {
	call locking("UNLOCKALL", "", "", "", locklist, 0, xx);
	USER3 = "";
	call mssg(USER4);
	var().stop();
	return;

}


libraryexit()
