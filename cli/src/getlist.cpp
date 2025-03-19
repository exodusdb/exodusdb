#include <exodus/program.h>
programinit()

func main() {

	let newlistid = "default";

	let listid = COMMAND.f(2);
	if (not listid)
		abort("syntax is: getlist LISTID");

	bool silent = OPTIONS.count("S") - OPTIONS.count("s");

	if (not getlist(listid))
		abort("Cannot read " ^ listid);

	logputl("Using select list ", listid, ".");

	if (not hasnext())
		if (not silent)
			logputl("No items selected.");

	if (not savelist(newlistid))
		abort(lasterror());

	if (not silent)
		logputl("Items saved in ", newlistid, ".");

//	if (not deletelist(listid))
//		if (not silent)
//			loglasterror();

	return 0;
}

programexit()
