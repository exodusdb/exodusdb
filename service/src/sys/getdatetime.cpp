#include <exodus/library.h>
libraryinit()

var tt;//num

function main(out localdate, out localtime, out sysdate, out systime, out utcdate, out utctime) {
	//c sys out,out,out,out,out,out

	//see CHANGETZ which seems to go through all times (dates?) and changes them
	//this should be done ONCE to standardise on gmt/utc really
	//really need a DBTZ to determine datetime for storage in the database

	//global tt

	//ensure time and date are taken in the same day
	//by ensuring time is not less than time1
	//which could happen over midnight
	while (true) {
		var systime1 = var().time();
		sysdate = var().date();
		systime = var().time();
		///BREAK;
		if (systime >= systime1) break;
	}//loop;

	//no timezone info
	if (not SW) {
		localdate = sysdate;
		localtime = systime;
		//assume system is on gmt/utc or database contains non-gmt datetimes
		utcdate = sysdate;
		utctime = systime;
		goto exit;
	}

	//@sw<1> is the ADJUSTMENT to get user time from server time, therefore add
	localtime = systime;
	localdate = sysdate;
	tt = SW.a(1);
	if (tt) {
		localtime += tt;
		if (localtime >= 86400) {
			//assume offset cannot be more than 24 hours!
			localdate = sysdate + 1;
			localtime -= 86400;
		} else if (localtime < 0) {
			//assume offset cannot be less than 24 hours!
			localdate = sysdate - 1;
			localtime += 86400;
		}
	}

	//@sw<2> is the difference from gmt/utc to server time, therefore subtract
	utctime = systime;
	utcdate = sysdate;

	//following is irrelevent until we support user tz when server tz ISNT gmt/utc
	//
	//remove server tz to get gmt/utc. if server is ahead of gmt then serv tz is +
	tt = SW.a(2);
	if (tt) {
		utctime -= tt;
		if (utctime >= 86400) {
			//assume tz cannot be more than 24 hours!
			utcdate = sysdate + 1;
			utctime -= 86400;
		} else if (utctime < 0) {
			//assume tz cannot be less than 24 hours!
			utcdate = sysdate - 1;
			utctime += 86400;
		}
	}

/////
exit:
/////
	if (SENTENCE == "GETDATETIME") {
		var msg = "";
		msg.r(-1, "User:   " ^ localdate.oconv("D"));
		msg ^= " " ^ localtime.oconv("MTH");
		msg.r(-1, "Server: " ^ sysdate.oconv("D"));
		msg ^= " " ^ systime.oconv("MTH");
		msg.r(-1, "GMT/UTC:" ^ utcdate.oconv("D"));
		msg ^= " " ^ utctime.oconv("MTH");
		call note(msg);
	}

	return 0;
}

libraryexit()
