#include <exodus/program.h>
programinit()

func main() {

	bool silent = OPTIONS.count("S") - OPTIONS.count("s");

	// Check filename exists to avoid select throwing an error;
	let filename = COMMAND.f(2).isnum() ? COMMAND.f(3) : COMMAND.f(2);
	if (not filename) {
		abort("Syntax: is sselect [maxnrecs] filename [with ... ] [by ...]");
	}

	if (not var().open(filename))
		abort(lasterror());

	var listid = "default";
	if (getlist(listid) and not silent)
		logputl("Using select list ", listid, ".");

	select(COMMAND.remove(1).convert(FM, " "));

	if (not hasnext()) {
		if (listid)
			deletelist(listid);
		if (not silent)
			logputl("0 keys selected.");
		abort();
	}

	if (not savelist(listid))
		abort(lasterror());

	if (not silent)
		logputl(lasterror(), " keys saved in ", listid, ".");

	return 0;
}

}; // programexit()
