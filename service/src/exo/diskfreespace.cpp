#include <exodus/library.h>
libraryinit()

function main(in path0) {

	var path;
	if (path0.unassigned() or path0.contains(":") or not(path0))
		// case we are presented with a DOS path
		path = ".";
	else
		path = path0;

	var cmd = "df --output=avail " ^ path;
	// TRACE(cmd)
	var available = osshellread(cmd).convert("\x0D\x0A", "  ").trim().field(" ", 2);

	if (available == "" or not available.isnum())
		available = 999999999;
	else
		// available*=1024; // not implemented yet?
		available = available * 1024;

	return available;
}

libraryexit()
