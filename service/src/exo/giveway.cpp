#include <exodus/library.h>
libraryinit()

	var nowtime;		//num
var longprocesslockno;	//num

function main() {
	if (USERNAME == "xEXODUS") {
		return 0;
	}
	//nb called from esc.to.exit so do not do selects etc which call esc.to.exit
	//otherwise infinite recursion

	//quit if interactive
	if (not SYSTEM.f(33)) {
		return 0;
	}

	var abortafterseconds = 600 + 10;

	//longprocesssecs=5*(rnd(50)+51)/50
	var longprocesssecs = 3;

	//random 1 to 2 seconds
	var sleepms = 1000 * (var(50).rnd() + 51) / 50;

	//random 3 to 4 seconds
	var sleepeverysecs = 2 + 1 * (var(50).rnd() + 51) / 50;

	//get time now
	//call dostime(nowtime);
	nowtime = ostime().round(2);
	//if no start time then no aborting (in initialisation and may be converting)
	//or running a long maintenance process that must not be aborted
	var starttime = SYSTEM.f(25);
	if (starttime == "") {
		abortafterseconds = 99999;
	}

	if (nowtime < starttime) {
		nowtime += 24 * 60 * 60;
	}

	//determine processing time of current request
	var processingtime = nowtime - starttime;

	//quit if not a long process
	if (processingtime < longprocesssecs) {
		return 0;
	}

	//print 'long processing time: ':processingtime

	//determine time since last sleep
	var processingtime2 = nowtime - SYSTEM.f(26);

	//quit if not worked enough since last sleep

	if (processingtime2 < sleepeverysecs) {
		return 0;
	}

	/*
	//if otherusers2(system<9>) then
	if (not SYSTEM.f(48)) {

		//allow (ncpus-1) processes to run at full speed
		var nprocesslocks = SYSTEM.f(9) - 1;
		if (nprocesslocks) {
			longprocesslockno = otherusers2(nprocesslocks);
			SYSTEM(48) = longprocesslockno;
		} else {
			longprocesslockno = 0;
		}

		if (not longprocesslockno) {
			cout << "elapsedsecs: "<< (processingtime2 * 1000).oconv("MD00P")<< " sleepeveryms:"<< sleepeverysecs * 1000<< " sleepingms:"<< sleepms<< " ...";
			cout << processingtime.oconv("MD00P");
			cout << " activems:";
			var cmd = "WAITING.EXE Q$W$E$R$T$.Y$U 0 " ^ sleepms;
			//print cmd
			cmd.osperform();
			cout << endl;
		}

	}
*/

	//save the time last slept or return to avoid checking otherusers all the time
	nowtime	   = ostime().round(2);
	SYSTEM(26) = nowtime;

	//do this after the delay so that parent processes that do not abort
	//still get delayed
	//print processingtime,abortafterseconds
	if (processingtime > abortafterseconds) {
		return 1;
	}

	return 0;
}

libraryexit()
