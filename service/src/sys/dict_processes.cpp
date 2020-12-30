#include <exodus/library.h>

libraryinit(last_updated2)
//------------------------

var uptodate;
var uptotime;

function main() {
	/*;
	dostimenow=date()+24873+time()/86400;
	elapsedsecs=int((dostimenow-@record<27>)*86400);
	ans='';

	interval=604800;
	intervalname='weeks';
	gosub addintervaldesc;

	interval=interval/7;
	intervalname='days';
	gosub addintervaldesc;

	interval=interval/24;
	intervalname='hours';
	gosub addintervaldesc;

	interval=interval/60;
	intervalname='mins';
	gosub addintervaldesc;

	interval=1;
	intervalname='secs';
	gosub addintervaldesc;

	return trim(ans);

	////////////////
	addintervaldesc:
	////////////////
	if interval=1 or elapsedsecs>interval then;
		nintervals=int(elapsedsecs/interval);
		elapsedsecs-=nintervals*interval;
		if nintervals=1 then intervalname[-1,1]='';
		ans:=' ':nintervals:' ':intervalname;
		end;
	return 0;
	*/

	var fromdate = RECORD.a(27).floor() - 24873;
	var fromtime = RECORD.a(27).field(".", 2) / 86400;
	return elapsedtimetext(fromdate, fromtime, uptodate, uptotime);
}
libraryexit(last_updated2)

libraryinit(maintenance)
//----------------------
function main() {
	if (RECORD.a(33)) {
		ANS = "";
	}else{
		ANS = "Yes";
	}
	return ANS;
}
libraryexit(maintenance)

libraryinit(pid)
//--------------
function main() {
	return RECORD.a(54, 5);
}
libraryexit(pid)

libraryinit(status)
//-----------------
#include <processlocking.h>

var islocked;

function main() {
	//NB add any new statuses to MONITORING program
	if (RECORD.a(52)) {
		ANS = "Closed";
		return ANS;
	}
	call processlocking("ISLOCKED", ID, islocked);
	if (not islocked) {
		ANS = "Crashed";
		return ANS;
	}

	var dostimenow = var().date() + 24873 + var().time() / 86400;
	//10 mins
	var hungtime = 10 * 60 / 86400;
	var r33 = RECORD.a(33);
	if (r33 and (dostimenow - RECORD.a(27) gt hungtime)) {
		ANS = "Hung";
		return ANS;
	}
	if (r33) {
		ANS = "OK";
	}else{
		ANS = "Maintenance";
	}
	if (RECORD.a(53)) {
		ANS ^= " " ^ RECORD.a(53);
	}
	return ANS;
}
libraryexit(status)
