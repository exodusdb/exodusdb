#include <exodus/program.h>
programinit()

function main() {

	bool silent = OPTIONS.count("S") - OPTIONS.count("s");

	var listid = "default";
	if (getlist(listid) and not silent)
		logputl("Using select list ", listid, ".");

	select(COMMAND.remove(1).convert(FM, " "));

	if (not hasnext()) {
		if (listid)
			deletelist(listid);
		if (not silent)
			logputl("0 items selected.");
		abort();
	}

	if (not savelist(listid))
		abort(lasterror());

	if (not silent)
		logputl(lasterror(), " items saved in ", listid, ".");

	return 0;
}

programexit()
