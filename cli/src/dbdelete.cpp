#include <exodus/program.h>
programinit()

function main() {
	var dbname = COMMAND.a(2);
	if (not dbname)
		abort("Syntax is dbdelete [dbname] ... {S}");

	var errorno = 0;
	COMMAND.remover(1);
	for (var dbname : COMMAND) {
		if (!dbname.dbdelete(dbname)) {
			if (not OPTIONS.index("S"))
				errputl(var().lasterror());
			errorno = 1;
		} else if (not OPTIONS.index("S"))
			printl(dbname.quote(), "database deleted.");
	}

	return errorno;
}

programexit()

