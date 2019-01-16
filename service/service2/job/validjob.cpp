#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <generalsubs.h>

#include <gen.h>
#include <agy.h>

var groupothers;
var groupusers;
var xx;
var msg2;

function main(in mode0, in jobno, io job, io msg) {
	//c job in,in,io,io

	//mode must be ACCESS, UPDATE or LIST
	//LIST is the same as ACCESS but buffers security and group users
	//returns 1, or 0 with msg appended

	var mode = mode0;

	if (not job) {
		if (not(job.read(agy.jobs, jobno))) {
			job = "";
		}
	}

	var executivecode = job.a(8);

	//always access own jobs or jobs with no executive
	if ((executivecode == USERNAME) or not executivecode) {
		return 1;
	}

	var jobgroup = executivecode.xlate("USERS", 21, "X");
	var usergroup = USERNAME.xlate("USERS", 21, "X");
	if (jobgroup == usergroup) {
		groupothers = "GROUP";
	}else{
		groupothers = "OTHERS";
	}

	//if listing then create and use a buffer of usercode that can be accessed
	// * for all - 0 for none - list of usercodes of the same group
	if (mode == "LIST") {

		mode = "ACCESS";

		//use buffer if same user and not first record
		if ((AW.a(40) == USERNAME) and (RECCOUNT > 1)) {
			groupusers = AW.a(41);

			//get the buffer if different user or is first record
		}else{

			AW.r(40, USERNAME);

			//pass if ACCESS OTHER
			if (authorised("JOB ACCESS OTHERS", xx)) {
				AW.r(41, "*");
				return 1;

			//get group users if ACCESS GROUP
			} else if (authorised("JOB ACCESS GROUP", xx)) {
				call generalsubs("GETGROUPUSERS");
				ANS.transfer(groupusers);
				AW.r(41, groupusers);

			//fail if no access to GROUP or OTHER users
			} else {
				AW.r(41, 0);
				return 0;
			}
//L404:
			{}
		}

		//pass if ACCESS OTHERS
		if (groupusers == "*") {
			return 1;
		}

		//fail if not ACCESS GROUP
		if (not groupusers) {
			return 0;
		}

		//fail if job not in same group
		if (not(groupusers.locateusing(executivecode, VM, xx))) {
			return 0;
		}

		//pass
		return 1;

	}

	//JOB ACCESS OTHERS
	//JOB ACCESS GROUP
	//JOB UPDATE OTHERS
	//JOB UPDATE GROUP
	if (not(authorised("JOB " ^ mode ^ " " ^ groupothers, msg2))) {
		msg.r(-1, FM ^ "Job " ^ (DQ ^ (ID ^ DQ)) ^ " belongs to|Executive: " ^ executivecode ^ FM ^ "Group: " ^ jobgroup);
		msg.r(-1, FM ^ msg2);
		msg.converter(FM, "|");
		return 0;
	}

	return 1;

}


libraryexit()
