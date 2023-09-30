#include <exodus/program.h>
programinit()

var ss1;
var ss2;
//var retval;
var secs;	// num
var time1;	// num
var time2;	// num
var nn = 1000;

function main() {
	// call savescreenss1, ss2);
	printl(chr(12));
	var ntries	 = 0;
	var tot		 = 0;
	var ntots	 = 0;
	var minspeed = "";
	var maxspeed = "";
	var avgspeed = "";
	while (true) {
		var reply;
		reply.inputn(-1);
		// /BREAK;
		if (not(reply == "" and ntots < 10))
			break;
		var	  yy = var("x").str(65530);
		var retval;
		gosub getspeed(retval);
		ntries += 1;
		// if ntries>2 and secs then
		// 		print(ntries, AT(-40));
		// Use retval to defeat optimiser
		if (secs and retval) {

			// Adjust time to account for change from var to int in loop
			secs *= 6;

			print(ntries, " ");
			tot += nn / secs;
			ntots += 1;

			let speed = (nn / secs / 1000000).oconv("MD20P");
			if (speed < minspeed or not(minspeed)) {
				minspeed = speed;
			}
			if (speed > maxspeed) {
				maxspeed = speed;
			}

			avgspeed = (tot / ntots / 1000000).oconv("MD20P");

			printl(speed, " x=i ", secs.oconv("MD10P"), " ", nn, " ", time());
		}
	}  // loop;

	var msg = "";
	msg(-1) = "PROCSPEED Min/Avg/Max = " ^ minspeed ^ " / " ^ avgspeed ^ " / " ^ maxspeed;
	msg(-1) = "More is faster.";
	msg(-1) = "Typical unoptimised speed for an entry level server in 2007 was 1";

	call mssg(msg);

	// call restorescreenss1, ss2);
	stop();

	return "";
}

subroutine getspeed(io retval) {
	time1 = ostime();
	// for (const var ii : range(1, nn)) {
	for (int ii = 1; ii <= nn; ++ii) {
		retval = ii;
	}  // ii;
	time2 = ostime();
	secs  = time2 - time1;

	// increase number of loops
	if (secs < 1) {
		nn	 = nn * 2;
		secs = "";
	}
	if (secs > 5) {
		nn	 = nn / 2;
		secs = "";
	}

	return;
}

programexit()
