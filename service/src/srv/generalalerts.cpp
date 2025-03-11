#include <exodus/library.h>
#include <srv_common.h>

libraryinit()

#include <service_common.h>


function main(in alerttype0, in /*runasuser*/, out tasks, out title, out request, out datax) {

	// 1) list of all active users
	// at 7am on the 14th of each month (ie regularly but not everyday)
	// CREATEALERT CURRUSERS GENERAL CURRUSERS 7:14 EXODUS (ROS)

	let alerttype	 = alerttype0.field(":", 1);
	let alertoptions = alerttype0.field(":", 2, 9999).convert(":", FM);

	let mode = "";

	if (alerttype == "CURRUSERS") {
		tasks	= "AUTHORISATION ACCESS";
		title	= "List of Current Users";
		request = "LISTAUTH.USERS";
		datax	= "";

	} else {
		call note(alerttype.quote() ^ " unrecognised alert type in GENERAL.ALERTS");
		stop();
	}

	return 0;
}

libraryexit()
