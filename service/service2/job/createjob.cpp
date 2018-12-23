#include <exodus/library.h>
libraryinit()

//#include <flushindex.h>

#include <gen.h>
#include <agy.h>

var recordx;

function main(io job, in jobno) {
	//c job io,in

	//in unlikely case that someone is creating "timesheet client job" then
	//this will fail here resulting in missing "timesheet client job"
	//however cannot hang or cancel transaction in this POSTWRITE code
	if (lockrecord("JOBS", agy.jobs, jobno, recordx, 9)) {

		//in general.subs2 and client.subs,postwrite
		var versionfn = 29;
		var statusfn = 7;
		var firstversion = "A";
		var defaultstatus = "0";

		job.r(statusfn, defaultstatus);
		job.r(versionfn + 1, USERNAME);
		job.r(versionfn + 2, var().date() ^ "." ^ (var().time()).oconv("R(0)#5"));
		job.r(versionfn + 3, STATION.trim());
		job.r(versionfn + 4, firstversion);
		job.r(versionfn + 5, defaultstatus);

		job.write(agy.jobs, jobno);

		call unlockrecord("JOBS", agy.jobs, jobno);

//		call flushindex("JOBS");

	}

	return 0;

}


libraryexit()
