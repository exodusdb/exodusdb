#include <exodus/library.h>
libraryinit()

#include <roundrobin.h>

#include <gen_common.h>

var errormsg;

function main(in mode, in params, io result, io msg) {
	//c sys in,in,io,io

	#include <general_common.h>

	//output:
	//empty = ok
	//result
	//msg if any error

	//eg 60 60 60 DOS EMAILS.DAT
	//means window of 60*60 seconds rolling every 60 seconds
	//max events 60 and store in DOS EMAILS.DAT file so per installation
	//could be DEFINITIONS EMAILS.ROUNDROBIN to be per database
	#define secsperperiod params.a(1)
	#define periodsperwindow params.a(2)
	#define maxeventsperwindow params.a(3)
	#define roundrobinfilename params.a(4)
	#define roundrobinkey params.a(5)

	result = "";
	msg = "";

	//roundrobin
	//minutes are the number of minutes since 1/1/1970 provided by date()/time()
	//field 1 = the last minute that was updated
	//field 2 = 60 multivalues for minutes of the hour

	if (mode == "TEST") {

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

	} else if (mode == "ONEVENT") {

		var verbs;
		if (not(verbs.open("VERBS", ""))) {
			msg = "ROUNDROBIN: CANNOT OPEN VERBS";
			return 0;
		}

		var roundrobinfile;
		if (not(roundrobinfile.open(roundrobinfilename, ""))) {
			msg = "ROUNDROBIN: CANNOT OPEN " ^ roundrobinfilename;
			return 0;
		}

		var roundrobinlock = "ROUNDROBIN*" ^ roundrobinfilename ^ "*" ^ roundrobinkey;

		//try to lock for 9 seconds since other locks should be brief
		if (not(lockrecord("VERBS", verbs, roundrobinlock, "", 9))) {
			msg = "ROUNDROBIN: CANNOT LOCK VERBS," ^ roundrobinlock;
			return 0;
		}

		//garbagecollect;

		//get the round robin data
		var roundrobin;
		if (not(roundrobin.read(roundrobinfile, roundrobinkey))) {
			roundrobin = "";
		}
		if (roundrobin.index(var().chr(0))) {
			roundrobin.converter(var().chr(0), "");
			var(var().date() ^ FM ^ var().time()).oswrite("RRR");
		}

		//determine current and last timeperiod
		//currentperiodn=date()*24*secsperperiod+mod(time(),secsperperiod)
		var currentperiodn = ((var().date() * 24 * 60 * 60 + var().time()) / secsperperiod).floor();
		var lastperiodn = roundrobin.a(1);

		//prevent catch up longer than periodsperwindow (add 2 for safety)
		if (currentperiodn - lastperiodn > periodsperwindow + 2) {
			lastperiodn = currentperiodn - periodsperwindow - 2;
		}

		//clear any skipped periods since last update
		//nb start from lastperiodn+1 to avoid clearing current period multiple times
		//ie clear only on the first time that we arrive in it
		for (var periodn = lastperiodn + 1; periodn <= currentperiodn; ++periodn) {
			roundrobin.r(2, periodn % periodsperwindow + 1, "");
		};//periodn;

		//record the current period as the last period so that in the next call
		//we can clear skipped periods (but not the current period again)
		roundrobin.r(1, currentperiodn);
		var currentbreakn = currentperiodn % periodsperwindow + 1;

		if (roundrobin.a(2).sum() < maxeventsperwindow) {

			result = 1;

			//increment the current number of events in the current period
			roundrobin.r(2, currentbreakn, roundrobin.a(2, currentbreakn) + 1);

		}else{
			result = 0;
		}

		//garbagecollect;

		if (roundrobin.index(var().chr(0))) {
			roundrobin.converter(var().chr(0), "");
			var(var().date() ^ FM ^ var().time()).oswrite("RRW");
		}
		roundrobin.write(roundrobinfile, roundrobinkey);

	//for testing
	//print lastperiodn,currentperiodn,currentbreakn,result,roundrobin<2>

		call unlockrecord("VERBS", verbs, roundrobinlock);

	}

	return 0;
}

libraryexit()