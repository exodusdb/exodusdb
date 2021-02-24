#include <exodus/library.h>
libraryinit()

#include <videorw.h>

var startmode;
var temp;
var reply;//num

function main(in startmode0, out starttime, out startdate, out startbuffer) {
	//c sys in,out,out,out

	var interactive = not(SYSTEM.a(33));

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

	//autostart in system mode
	//if @volumes else return 1

	call videorw(0, 0, CRTWIDE - 1, CRTHIGH - 1, "R", startbuffer);

	if (not interactive) {
		return 1;
	}

	if (startmode eq "CONFIRM") {
		reply = "";
inpreply:
		call note("OK to start?|(Type Yes or No)", "RC", reply, "");

		if (reply eq "YES") {
			reply = 1;
		} else if (reply eq "NO") {
			reply = 2;
		} else {
			goto inpreply;
		}

	}else{
		reply = "Y";
		//IF DECIDE(TEMP:'OK to start ?|','',REPLY) ELSE REPLY=2
		call note(temp ^ "OK to start ? (Y/n)", "RC", reply);
		reply = reply[1] eq "Y";
		if (not reply) {
			printl("Cancelled");
		}
	}

	if (reply eq 1) {
		return 1;
	}
	return 0;
}

libraryexit()
