#include <exodus/library.h>
libraryinit()


var uptotime;//num

function main(in fromdate, in fromtime) {
	//c sys
	//declare function unassigned

	var text = "";

	//if unassigned(uptodate) then uptodate=date()
	//if unassigned(uptotime) then CALL DOSTIME(uptotime)
	var uptodate = var().date();
	uptotime = ostime();

	//NSECS=INT(uptotime-fromTIME)
	var nsecs = uptotime - fromtime;
	//IF NSECS ELSE NSECS=1
	//uptodate=date()
	if (fromdate ne uptodate) {
		nsecs += (uptodate - fromdate) * 24 * 3600;
	}

	//cater for bug where start date isnt known and time has crossed midnight
	//so the 2nd time is less than the first
	if (nsecs < 0) {
		nsecs += 86400;
	}

	var hours = (nsecs / 3600).floor();
	nsecs -= hours * 3600;
	var minutes = (nsecs / 60).floor();
	nsecs -= minutes * 60;
	if (hours) {
		text ^= hours ^ " hour";
		if (hours ne 1) {
			text ^= "s";
		}
	}
	if (minutes) {
		text.r(-1, minutes ^ " min");
		if (minutes ne 1) {
			text ^= "s";
		}
	}
	if (not hours and minutes < 5) {
		if (nsecs) {
			if (minutes or nsecs - 10 > 0) {
				nsecs = nsecs.oconv("MD00P");
			}else{
				nsecs = (nsecs.oconv("MD40P")) + 0;
				if (nsecs[1] == ".") {
					nsecs.splicer(1, 0, "0");
				}
			}
			if (nsecs) {
				text.r(-1, nsecs ^ " sec");
				if (nsecs ne 1) {
					text ^= "s";
				}
			} else if (not minutes) {
zero:
				text.r(-1, "< 1 msec");
			} else {
				text.r(-1, "exactly");
			}
		}else{
			if (not minutes) {
				goto zero;
			}
			text.r(3, "exactly");
		}
	}

	text.swapper(FM ^ FM ^ FM, FM);
	text.swapper(FM ^ FM, FM);
	text.swapper(FM, ", ");

	return text;

}


libraryexit()
