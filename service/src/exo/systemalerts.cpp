#include <exodus/library.h>
libraryinit()

	// used as a template to call the actual functions like generalalerts, financealerts, mediaalerts, jobalerts

function main(in alerttype0, in /*runasuser*/, out tasks, out title, out request, out datax) {

	var alerttype	 = alerttype0.field(":", 1);
	var alertoptions = (alerttype0.field(":", 2, 9999)).convert(":", FM);

	var mode = "";

	// dummy example

	if (alerttype == "AAA") {
		tasks	= "XXX";
		title	= "YYY";
		request = "ZZZ";
		datax	= "";

	} else {
		call mssg(DQ ^ (alerttype ^ DQ) ^ " unrecognised alert type in systemalerts");
		stop();
	}
	// L150:
	return 0;
}

libraryexit()
