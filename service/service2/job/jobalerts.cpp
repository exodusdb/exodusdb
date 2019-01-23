#include <exodus/library.h>
libraryinit()


#include <gen.h>

function main(in alerttype0, in runasuser, out tasks, out title, out request, out datax) {
	//c job in,in,out,out,out,out

	//0) close all jobs with ts but not ts in the last 365 days
	//at 7am on the 14th of each month (ie regularly but not everyday)
	//CREATEALERT INACTJOB JOB INACTIVEJOBS:365:C 7:14 NEOSYS (ROS)

	//0) stop all executives with no jobs started in the last 365 days
	//at 7am on the 14th of each month (ie regularly but not everyday)
	//CREATEALERT INACTJOBEXEC JOB INACTIVEJOBEXECS:365:S 7:14 NEOSYS (ROS)

	//1) pending estimates - to execs at 7am - runas user JOE
	//CREATEALERT PENDEST1 JOB PENDEST::TOEXEC 7 JOE (ROS)
	//ditto but only status delivered ... to joe and jane
	//CREATEALERT PENDEST2 JOB PENDEST:DELIVERED 7 JOE,JANE (ROS)
	//pending older than 30 days, ordered by executive emailed to joe and jane
	//CREATEALERT PENDEST3 JOB PENDEST::-30:BYEXEC 7 JOE,JANE (ROS)

	if (runasuser.unassigned()) {
		{}
	}

	var alerttype = alerttype0.field(":", 1);
	var alertoptions = alerttype0.field(":", 2, 9999).convert(":", FM);

	var mode = "";

	if (alerttype == "INACTIVEJOBS") {
		tasks = "JOB UPDATE";
		title = "Inactive Jobs";
		request = "INACTIVEJOBS";
		var ageindays = alertoptions.a(1);
		if (not(ageindays.match("1N0N"))) {
			call mssg(DQ ^ (ageindays ^ DQ) ^ " age in days should be numeric");
			var().stop();
		}
		datax = "{TODAY-" ^ ageindays ^ "}";
		var closing = alertoptions.a(2);
		if (not((closing == "C") or (closing == ""))) {
			call mssg(DQ ^ (closing ^ DQ) ^ " parameter 2 must be C for Close or blank to List");
			var().stop();
		}
		datax.r(2, closing);

	} else if (alerttype == "INACTIVEJOBEXECS") {
		tasks = "JOB UPDATE";
		title = "Inactive Job Executives";
		request = "INACTIVEJOBEXECS";
		var ageindays = alertoptions.a(1);
		if (not(ageindays.match("1N0N"))) {
			call mssg(DQ ^ (ageindays ^ DQ) ^ " age in days should be numeric");
			var().stop();
		}
		datax = "{TODAY-" ^ ageindays ^ "}";
		var stopping = alertoptions.a(2);
		if (not((stopping == "S") or (stopping == ""))) {
			call mssg(DQ ^ (stopping ^ DQ) ^ " parameter 2 must be S for Stop or blank");
			var().stop();
		}
		datax.r(2, stopping);

	} else if (alerttype == "PENDEST") {
		title = "Pending Estimates";

		tasks = "PRODUCTION ESTIMATE ACCESS";
		request = "JOBLIST";

		mode = "ESTIMATESPENDING";
		//fromdate=''
		//uptodate=''
		var fromdate = "{OPERATIONS_OPEN_DATE}";
		var uptodate = "{TODAY}";
		var orderby = "";
		var emailtarget = "";
		if (alertoptions.index("TOEXEC")) {
			alertoptions.swapper("TOEXEC", "");
			orderby = 1;
			emailtarget = "EXECUTIVE";
		} else if (alertoptions.index("BYEXEC")) {
			alertoptions.swapper("BYEXEC", "");
			orderby = 1;
		}
//L455:
		var statuses = alertoptions.a(1);
		statuses.converter(",", VM);
		var dateadj = alertoptions.a(2);
		if (dateadj.isnum()) {
			uptodate = "{TODAY-" ^ dateadj.abs() ^ "}";
		}

		datax = "";
		datax.r(1, mode);

		//fromdate
		datax.r(30, fromdate);

		//uptodate
		datax.r(31, uptodate);

		//datax<16>=orderby;*EXECUTIVE_CODE or blank
		datax.r(40, orderby);
		datax.r(59, emailtarget);
		datax.r(43, statuses);

	} else {
		call mssg(DQ ^ (alerttype ^ DQ) ^ " unrecognised alert type in MEDIA.ALERTS");
		var().stop();
	}
//L596:
	return 0;

}


libraryexit()
