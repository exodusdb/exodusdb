#include <exodus/program.h>
programinit()

function main() {

	let dbname = COMMAND.f(2);
	if (not dbname)
		abort("Syntax is dbcreate [dbname] {S}");

	if (!dbname.dbcreate(dbname)) {
		if (not OPTIONS.contains("S"))
			errputl(var().lasterror());
		abort("");
	}

	if (not OPTIONS.contains("S"))
		printl(dbname.quote(), "database created");

	return 0;
}

programexit()

