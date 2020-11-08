#include <exodus/library.h>
libraryinit()

#include <videorw.h>

var startmode;
var temp;
var reply;//num

function main(in startmode0, out starttime, out startdate, out startbuffer) {
	//c sys in,out,out,out

	if (startmode0.unassigned()) {
		startmode = "";
	}else{
		startmode = startmode0;
	}
	if (startmode) {
		temp = "|" ^ startmode ^ "||";
	}else{
		temp = "";
	}

	startdate = var().date();
	starttime = ostime();

	//autostart in batch mode
	if (not(VOLUMES)) {
		return 1;
	}

	call videorw(0, 0, CRTWIDE - 1, CRTHIGH - 1, "R", startbuffer);

	if (startmode == "CONFIRM") {
		reply = "";
inpreply:
		call note("OK to start?|(Type Yes or No)", "RC", reply, "");

		if (reply == "YES") {
			reply = 1;
		} else if (reply == "NO") {
			reply = 2;
		} else {
			goto inpreply;
		}

	}else{
		if (not(decide(temp ^ "OK to start ?|", "", reply))) {
			reply = 2;
		}
	}

	if (reply == 1) {
		return 1;
	}
	return 0;
}

libraryexit()
