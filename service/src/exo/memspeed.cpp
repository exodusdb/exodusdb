#include <exodus/program.h>
programinit()

var ss1;
var ss2;
var xx;
var yy;
var secs;//num
var time1;//num
var time2;//num
var nn = 1000;

function main() {
	//call savescreenss1, ss2);
	printl(var().chr(12));
	var ntries = 0;
	var tot = 0;
	var ntots = 0;
	var minspeed = "";
	var maxspeed = "";
	var avgspeed = "";
	while (true) {
		xx.inputn(-1);
		///BREAK;
		if (not(xx eq "" and ntots lt 10)) break;
		yy = var("x").str(65530);
		gosub getspeed();
		ntries += 1;
		//if ntries>2 and secs then
		print(ntries, at(-4), at(0));
		if (secs) {

			print(ntries, " ");
			tot += nn / secs;
			ntots += 1;

			var speed = (nn / secs / 100000).oconv("MD20P");
			if (speed lt minspeed or not(minspeed)) {
				minspeed = speed;
			}
			if (speed gt maxspeed) {
				maxspeed = speed;
			}

			avgspeed = (tot / ntots / 100000).oconv("MD20P");

			printl(speed, " x=64Kb str/sec ", secs.oconv("MD10P"), " ", nn, " ", var().time());

		}
	}//loop;

	var msg = "";
	msg(-1) = "MEMSPEED Min/Avg/Max = " ^ minspeed ^ " / " ^ avgspeed ^ " / " ^ maxspeed;
	msg(-1) = "More is faster.";
	msg(-1) = "Typical speed for an entry level server in 2007 was 1";

	call mssg(msg.oconv("L#60"));

	//call restorescreenss1, ss2);
	stop();

	return "";
}

subroutine getspeed() {
	time1 = ostime();
	for (var ii = 1; ii <= nn; ++ii) {
		xx = yy;
	} //ii;
	time2 = ostime();
	secs = time2 - time1;

	//increase number of loops
	if (secs lt 1) {
		nn = nn * 2;
		secs = "";
	}
	if (secs gt 5) {
		nn = nn / 2;
		secs = "";
	}

	return;
}

programexit()
