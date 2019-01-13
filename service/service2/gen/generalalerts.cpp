#include <exodus/library.h>
libraryinit()


#include <gen.h>

function main(in alerttype0, in runasuser, out tasks, out title, out request, out datax) {
	//c gen in,in,out,out,out,out

	//1) list of all active users
	//at 7am on the 14th of each month (ie regularly but not everyday)
	//CREATEALERT CURRUSERS GENERAL CURRUSERS 7:14 NEOSYS (ROS)

	var alerttype = alerttype0.field(":", 1);
	var alertoptions = (alerttype0.field(":", 2, 9999)).convert(":", FM);
	if (runasuser.unassigned()) {
		{}
	}

	var mode = "";

	if (alerttype == "CURRUSERS") {
		tasks = "AUTHORISATION ACCESS";
		title = "List of Current Users";
		request = "LISTAUTH.USERS";
		datax = "";

	} else {
		call mssg(DQ ^ (alerttype ^ DQ) ^ " unrecognised alert type in GENERAL.ALERTS");
		var().stop();
	}
//L150:
	return 0;

}


libraryexit()
