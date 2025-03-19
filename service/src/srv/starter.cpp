#include <exodus/library.h>
#include <srv_common.h>

libraryinit()

//#include <videorw.h>

#include <service_common.h>


var startmode;
var temp;
var reply;	// num

func main(in startmode0, out starttime, out startdate, out /*startbuffer*/) {

//	if (startmode0.unassigned()) {
//		startmode = "";
//	} else {
//		startmode = startmode0;
//	}
	startmode = startmode0.or_default("");
	if (startmode) {
		temp = "|" ^ startmode ^ "||";
	} else {
		temp = "";
	}

	startdate = date();
	starttime = ostime();

	// autostart in system mode
	// if @volumes else return 1

	//call videorw(0, 0, CRTWIDE - 1, CRTHIGH - 1, "R", startbuffer);

	if (not interactive_) {
		return 1;
	}

	if (startmode == "CONFIRM") {
		reply = "";
inpreply:
		call note("OK to start?|(Type Yes or No)", "RC", reply);

		if (reply == "YES") {
			reply = 1;
		} else if (reply == "NO") {
			reply = 2;
		} else {
			goto inpreply;
		}

	} else {
		reply = "Y";
		// IF DECIDE(TEMP:'OK to start ?|','',REPLY) ELSE REPLY=2
		call note(temp ^ "OK to start ? (Y/n)", "RC", reply);
		reply = reply.starts("Y");
		if (not reply) {
			printl("Cancelled");
		}
	}

	if (reply == 1) {
		return 1;
	}
	return 0;
}

libraryexit()
