#include <exodus/library.h>
libraryinit()

#include <unique.h>
#include <sendmail.h>
#include <select2.h>
#include <sysmsg.h>
#include <updtasks.h>

#include <gen.h>
#include <agy.h>

var newtaskid;
var xx;
var oldtask;
var result;
var response;
var errmsg2;
var body;
var subject;
var tousercodes;
var toaddress;
var tousercode;

function main(in mode, io oldtaskid, io task, io errmsg) {

	errmsg = "";

	var autoupdatesubtasks = 1;

	var tasks;
	if (not(tasks.open("TASKS", ""))) {
		errmsg = DQ ^ ("TASKS" ^ DQ) ^ " file cannot be opened";
		return 0;
	}

	var dictjobs;
	if (not(dictjobs.open("DICT.JOBS", ""))) {
		errmsg = DQ ^ ("DICT.JOBS" ^ DQ) ^ " file cannot be opened";
		return 0;
	}

	var jobno = task.a(1);
	if (jobno == "") {
		errmsg = "Jobno is missing in upd.tasks for " ^ oldtaskid;
		return 0;
	}

	var job;
	if (not(job.read(agy.jobs, jobno))) {
		job = "";
	}

	if (not oldtaskid) {
		oldtaskid = jobno ^ "*1";
	}

	if (mode == "ADD") {

		//find first unused job task id
		//start at 1 to check for duplicate users too
		//jobtaskno=field(oldtaskid,'*',2)
		var jobtaskno = 1;
		while (true) {
			newtaskid = oldtaskid.field("*", 1) ^ "*" ^ jobtaskno;
			var taskx;
			if (not(taskx.read(tasks, newtaskid))) {
				taskx = "";
			}
		///BREAK;
		if (not taskx.length()) break;;

			//prevent adding duplicate tasks for the same user
			if (taskx.a(2) == task.a(2)) {
				if (not(var("Completed" ^ VM ^ "Cancelled").locateusing(taskx.a(3), VM, xx))) {
					errmsg = DQ ^ (task.a(2) ^ DQ) ^ " is already " ^ taskx.a(3) ^ " on this job (" ^ newtaskid ^ ")";
					return 0;
				}
			}

			jobtaskno += 1;
		}//loop;

		//make sure existing task is started
		if (not(lockrecord("TASKS", tasks, oldtaskid, xx, 9))) {
			errmsg = DQ ^ (oldtaskid ^ DQ) ^ " could not lock task to change its status to \"Started\"";
			return 0;
		}
		var oldtask;
		if (oldtask.read(tasks, oldtaskid)) {
			if (oldtask.a(3) == "Allocated") {
				oldtask.r(3, "Started");
				call updtasks("UPDATE", oldtaskid, oldtask, errmsg);
				if (errmsg) {
					return 0;
				}
			}
		}
		call unlockrecord("TASKS", tasks, oldtaskid);

		oldtask = "";

		//default mode UPDATE
	}else{
		newtaskid = oldtaskid;
		var oldtask;
		if (not(oldtask.read(tasks, oldtaskid))) {
			oldtask = "";
		}

		//check no direct subtasks are pending (ie not completed or cancelled)
		var newstatus = task.a(3);
		var oldstatus = oldtask.a(3);
		if (newstatus ne oldstatus) {
			if (newstatus == "Completed" or newstatus == "Cancelled" or newstatus == "Suspended") {

				var sortselect = " WITH PARENT_TASK_ID " ^ (DQ ^ (newtaskid ^ DQ));
				sortselect ^= " AND WITH STATUS NE \"Cancelled\"";
				sortselect ^= " AND WITH STATUS NE \"Completed\"";
				sortselect ^= " AND WITH STATUS NE " ^ (DQ ^ (newstatus ^ DQ));
				var dictids = "TASK_ID";
				var options = "";
				call select2("TASKS", "", sortselect, dictids, options, result, response);
				if (response ne "OK") {
					errmsg = response;
					return 0;
				}

				if (result) {
					var nn = result.count(FM) + 1;
					for (var ii = 1; ii <= nn; ++ii) {
						var subtaskid = result.a(ii, 1);
						var subtask;
						if (subtask.read(tasks, subtaskid)) {
							if (not autoupdatesubtasks) {
								errmsg.r(-1, "Request for " ^ subtask.a(2) ^ " must be completed or cancelled first");
							}else{
								//if completing or cancelling then
								//mark uncompleted/uncancelled subtasks as complete
								subtask.r(3, newstatus);
								call updtasks("UPDATE", subtaskid, subtask, errmsg2);
								if (errmsg2) {
									errmsg.r(-1, errmsg2);
								}
							}
						}else{
							//if not(autocompletesubtasks) then
							// errmsg<-1>='Task ID ':subtaskid:' is missing'
							// end
						}
					};//ii;

					if (not autoupdatesubtasks) {
						return 0;
					}

				}

			}
		}

		//errmsg='testingfail'
		//return 0

	}

	//ensure parent user code is user code of parent task
	//or current user in case missing for some reason
	var parenttaskid = task.a(7);
	var parenttask = "";
	if (parenttaskid) {
		if (parenttask.read(tasks, parenttaskid)) {
			task.r(8, parenttask.a(2));
		}else{
			//fall back to current usercode
			task.r(8, USERNAME);
		}
	}

	//default requestor is from configuration
	if (task.a(8) == "") {
		task.r(8, agy.agp.a(106));
	}

	//default user is current user
	if (task.a(2) == "") {
		task.r(2, USERNAME);
	}

	//default status is Created
	if (task.a(3) == "") {
		task.r(3, "Created");
	}

	//default status time
	if (task.a(4) == "") {
		task.r(4, var().date() ^ "." ^ (var().time()).oconv("R(0)#5"));
	}

	//clear copy of key
	task.r(5, "");

	//////////////////
	//update tasks file
	//////////////////
	task.write(tasks, newtaskid);
	//call flushindex("TASKS");

	//refresh the new updatetimestamp DATE_TIME
	if (not(task.read(tasks, newtaskid))) {
		{}
	}

	var requestor_usercode = task.a(8);
	var requestee_usercode = task.a(2);

	jobno = newtaskid.field("*", 1);
	if (task.a(3) ne oldtask.a(3)) {

		//make first few lines of body
		body = "";
		//if oldtask then
		// body<-1>='Job ':jobno:' ':task<3>:' (was ':oldtask<3>:')'
		// subject=body<1>
		//end else
		// body<-1>='New Job ':jobno
		// subject=body<1>
		// end
		var action = task.a(3);
		if (oldtask.a(3) == "Suspended" and action == "Started") {
			action = "Resumed";
		}
		body.r(-1, "Job " ^ jobno ^ " " ^ action);
		//if oldtask then
		// body:=' (was ':oldtask<3>:')'
		// end

		var clientname = calculate("CLIENT_NAME", dictjobs, jobno, job, 0);
		body ^= " " ^ clientname;

		subject = body.a(1);

		body.r(-1, calculate("FULL_BRAND_NAME", dictjobs, jobno, job, 0));
		body ^= ", " ^ calculate("DESCRIPTION1", dictjobs, jobno, job, 0);

		body.swapper(FM, "\r\n\r\n");

		var baselinks = SYSTEM.a(114);
		var baselinkdescs = SYSTEM.a(115);
		var nlinks = baselinks.count(VM) + (baselinks ne "");
		for (var linkn = 1; linkn <= nlinks; ++linkn) {
			body.r(-1, FM ^ baselinkdescs.a(1, linkn));
			body.r(-1, baselinks.a(1, linkn) ^ "jobs/jobs.htm?key=" ^ jobno);
			body.r(-1, baselinks.a(1, linkn) ^ "jobs/myjobs.htm");
		};//linkn;

		body.converter(FM, var().chr(10));
		//swap fm with \0D0A\ in body
		//solve failure to line break in outlook
		//swap char(13):'http' with char(13):char(13):'http' in body

		tousercodes = "";
		toaddress = "";
		var ccaddress = "";

		if (var("NEOSYS.ID").osfile() or requestor_usercode ne USERNAME) {
			//email requestor change in status
			tousercode = requestor_usercode;
			gosub adduser();
		}

		if (var("NEOSYS.ID").osfile() or requestee_usercode ne USERNAME) {
			//email requestee change in status
			tousercode = requestee_usercode;
			gosub adduser();
		}

		var parentparent_usercode = parenttask.a(8);
		if (var("NEOSYS.ID").osfile() or parentparent_usercode ne USERNAME) {
			//email requestee change in status
			tousercode = parentparent_usercode;
			gosub adduser();
		}

		if (toaddress) {
			gosub sendmail2(errmsg);
		}

	}

	return 0;

}

subroutine adduser() {
	if (not(tousercodes.locateusing(tousercode, VM, xx))) {
		tousercodes.r(1, -1, tousercode);
		var emailaddress = tousercode.xlate("USERS", 7, "X");
		if (emailaddress) {
			toaddress.r(1, -1, emailaddress);
		}
	}
	return;

}

subroutine sendmail2(io errmsg) {

	if (not toaddress) {
		return;
	}

	toaddress = unique(toaddress);

	//move multiple to addresses to cc address
	var ccaddress = toaddress.field(VM, 2, 9999);
	ccaddress.converter(VM, ";");
	toaddress = toaddress.a(1, 1);

	var attachfilename = "";
	var deletex = 0;
	//errmsg=''
	call sendmail(toaddress, ccaddress, subject, body, attachfilename, deletex, errmsg);
	if (errmsg == "OK") {
		errmsg = "";
	}

	if (errmsg) {
		call sysmsg(errmsg);
	}

	return;

}


libraryexit()
