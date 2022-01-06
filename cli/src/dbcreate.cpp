#include <exodus/program.h>
programinit()

function main() {

	var dbname = COMMAND.a(2);
	if (not dbname)
		abort("Syntax is dbcreate [dbname] {S}");

	if (!dbname.dbcreate(dbname)) {
		if (not OPTIONS.index("S"))
			errputl(var().lasterror());
		abort("");
	}

	if (not OPTIONS.index("S"))
		printl(dbname.quote(), "database created");

	return 0;
}

programexit()

