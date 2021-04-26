#include <exodus/library.h>
libraryinit()

#include <roundrobin.h>

#include <sys_common.h>

var errormsg;

function main(in mode, in params, io result, io msg) {
	//c sys in,in,io,io

	#include <system_common.h>

	//output:
	//empty = ok
	//result
	//msg if any error

	//eg 60 60 60 DOS EMAILS.DAT
	//means window of 60*60 seconds rolling every 60 seconds
	//max events 60 and store in DOS EMAILS.DAT file so per installation
	//could be DEFINITIONS EMAILS.ROUNDROBIN to be per database
	#define secsperperiod_ params.a(1)
	#define periodsperwindow_ params.a(2)
	#define maxeventsperwindow_ params.a(3)
	#define roundrobinfilename_ params.a(4)
	#define roundrobinkey_ params.a(5)

	result = "";
	msg = "";

	//roundrobin
	//minutes are the number of minutes since 1/1/1970 provided by date()/time()
	//field 1 = the last minute that was updated
	//field 2 = 60 multivalues for minutes of the hour

	if (mode eq "TEST") {

		//here we have maximum of 6 events per last 6x6 seconds
		var params2 = "";
		params2.r(1, 6);
		params2.r(2, 6);
		params2.r(3, 6);
		params2.r(4, "DOS");
		params2.r(5, "TEST.DAT");
		//osdelete params2<5>

		printl();
		while (true) {
			///BREAK;
			if (not(not(esctoexit()))) break;
			call ossleep(1000*var(3).rnd());
			call roundrobin("ONEVENT", params2, result, errormsg);
		}//loop;

	} else if (mode eq "ONEVENT") {

		var voc;
		if (not(voc.open("VOC", ""))) {
			msg = "ROUNDROBIN: CANNOT OPEN VOC";
			return 0;
		}

		var roundrobinfile;
		if (not(roundrobinfile.open(roundrobinfilename_, ""))) {
			msg = "ROUNDROBIN: CANNOT OPEN " ^ roundrobinfilename_;
			return 0;
		}

		var roundrobinlock = "ROUNDROBIN*" ^ roundrobinfilename_ ^ "*" ^ roundrobinkey_;

		//try to lock for 9 seconds since other locks should be brief
		if (not(lockrecord("VOC", voc, roundrobinlock, "", 9))) {
			msg = "ROUNDROBIN: CANNOT LOCK VOC," ^ roundrobinlock;
			return 0;
		}

		//garbagecollect;

		//get the round robin data
		var roundrobin;
		if (not(roundrobin.read(roundrobinfile, roundrobinkey_))) {
			roundrobin = "";
		}
		if (roundrobin.index(var().chr(0))) {
			roundrobin.converter(var().chr(0), "");
			var(var().date() ^ FM ^ var().time()).oswrite("rrobin");
		}

		//determine current and last timeperiod
		//currentperiodn=date()*24*secsperperiod+mod(time(),secsperperiod)
		var currentperiodn = ((var().date() * 24 * 60 * 60 + var().time()) / secsperperiod_).floor();
		var lastperiodn = roundrobin.a(1);

		//prevent catch up longer than periodsperwindow (add 2 for safety)
		if (currentperiodn - lastperiodn gt periodsperwindow_ + 2) {
			lastperiodn = currentperiodn - periodsperwindow_ - 2;
		}

		//clear any skipped periods since last update
		//nb start from lastperiodn+1 to avoid clearing current period multiple times
		//ie clear only on the first time that we arrive in it
		for (var periodn = lastperiodn + 1; periodn <= currentperiodn; ++periodn) {
			roundrobin.r(2, periodn % periodsperwindow_ + 1, "");
		} //periodn;

		//record the current period as the last period so that in the next call
		//we can clear skipped periods (but not the current period again)
		roundrobin.r(1, currentperiodn);
		var currentbreakn = currentperiodn % periodsperwindow_ + 1;

		if (roundrobin.a(2).sum() lt maxeventsperwindow_) {

			result = 1;

			//increment the current number of events in the current period
			roundrobin.r(2, currentbreakn, roundrobin.a(2, currentbreakn) + 1);

		} else {
			result = 0;
		}

		//garbagecollect;

		if (roundrobin.index(var().chr(0))) {
			roundrobin.converter(var().chr(0), "");
			var(var().date() ^ FM ^ var().time()).oswrite("RRW");
		}
		roundrobin.write(roundrobinfile, roundrobinkey_);

	//for testing
	//print lastperiodn,currentperiodn,currentbreakn,result,roundrobin<2>

		call unlockrecord("VOC", voc, roundrobinlock);

	}

	return 0;
}

libraryexit()
