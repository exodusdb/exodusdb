#include <exodus/library.h>
libraryinit()

function main(in fromdate, in fromtime, io uptodate, io uptotime) {
	//c sys in,in,io,io

	var text = "";

	if (uptodate.unassigned()) {
		uptodate = var().date();
	}
	if (uptotime.unassigned()) {
		uptotime = ostime();
	}
	//uptodate=date()
	//CALL DOSTIME(uptotime)

	//NSECS=INT(uptotime-fromTIME)
	var nsecs = uptotime - fromtime;
	//IF NSECS ELSE NSECS=1
	//uptodate=date()
	if (fromdate ne uptodate) {
		nsecs += (uptodate - fromdate) * 24 * 3600;
	}

	//cater for bug where start date isnt known and time has crossed midnight
	//so the 2nd time is less than the first
	if (nsecs lt 0) {
		nsecs += 86400;
		}

	var weeks = (nsecs / 604800).floor();
	nsecs -= weeks * 604800;

	var days = (nsecs / 86400).floor();
	nsecs -= days * 86400;

	var hours = (nsecs / 3600).floor();
	nsecs -= hours * 3600;

	var minutes = (nsecs / 60).floor();
	nsecs -= minutes * 60;

	if (weeks) {
		text.r(-1, weeks ^ " week");
		if (weeks ne 1) {
			text ^= "s";
		}
	}
	if (days) {
		text.r(-1, days ^ " day");
		if (days ne 1) {
			text ^= "s";
		}
	}
	if (hours) {
		text.r(-1, hours ^ " hour");
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
	if (not(hours) and minutes lt 5) {
		if (nsecs) {
			if (minutes or (nsecs - 10 gt 0)) {
				nsecs = nsecs.oconv("MD00P");
			} else {
				//nsecs=(nsecs 'MD40P')+0
				nsecs = (nsecs.oconv("MD30P")) + 0;
				if (nsecs[1] eq ".") {
					nsecs.splicer(1, 0, "0");
				}
			}
			if (nsecs) {
				text.r(-1, nsecs ^ " sec");
				if (nsecs ne 1) {
					text ^= "s";
				}
			} else if (not(minutes)) {
zero:
				text.r(-1, "< 1 ms");
			} else {
				text.r(-1, "exactly");
			}
		} else {
			if (not(minutes)) {
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
