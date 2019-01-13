#include <exodus/library.h>
libraryinit()


#include <gen.h>

function main(in alerttype0, in runasuser, out task, out title, out request, out datax) {
	//c fin in,in,out,out,out,out

	//CREATEALERT COLLECT FINANCE COLLECTION 8,12,17 usercode

	var alerttype = alerttype0.field(":", 1);
	var alertoptions = (alerttype0.field(":", 2, 9999)).convert(":", FM);

	if (runasuser.unassigned()) {
		{}
	}

	if (alerttype == "COLLECTION") {

		title = "Pending Collections";

		task = "COLLECTION ACCESS";
		request = "LISTCOLLECTIONS";

		datax = "";
		//next action from/to
		datax.r(3, "{30DAYSAGO}");
		datax.r(4, "{TODAY}");
		datax.r(6, 1);

	} else {
		call mssg(DQ ^ (alerttype ^ DQ) ^ " unrecognised alert type in FINANCE.ALERTS");
		var().stop();
	}
//L177:
	return 0;

}


libraryexit()
