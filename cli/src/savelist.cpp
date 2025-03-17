#include <exodus/program.h>
programinit()

function main() {

	let newlistid = COMMAND.f(2);
	if (not newlistid)
		abort("syntax is: savelist LISTID");

	bool silent = OPTIONS.count("S") - OPTIONS.count("s");

	var listid = "default";
	if (not getlist(listid))
		abort("Cannot read " ^ listid);

	logputl("Using select list ", listid, ".");

	if (not hasnext())
		if (not silent)
			logputl("0 items selected.");

	if (not savelist(newlistid))
		abort(lasterror());

	if (not silent)
			logputl(lasterror() ^ " items saved in ", newlistid, ".");

	if (not deletelist(listid))
		if (not silent)
			loglasterror();

	return 0;
}

programexit()
