#include <exodus/library.h>
libraryinit()

func main(in path0) {

	var path = path0.or_default(".");

	let cmd = "df --output=avail " ^ path;
	var available = osshellread(cmd).convert("\x0D\x0A", "  ").trim().field(" ", 2);

	if (available == "" or not available.isnum())
		available = 999999999;
	else
		available = available * 1024;

	return available;
}

}; // libraryexit()
