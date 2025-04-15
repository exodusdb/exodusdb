#include <exodus/program.h>
programinit()

func main() {

	let dbname = COMMAND.f(2);
	if (not dbname)
		abort("Syntax is dbdelete [dbname] ... {S}");

	var errorno = 0;
	COMMAND.remover(1);
	for (var dbname : COMMAND) {
		if (!dbname.dbdelete(dbname)) {
			if (not OPTIONS.contains("S"))
				errputl(var().lasterror());
			errorno = 1;
		} else if (not OPTIONS.contains("S"))
			printl(dbname.quote(), "database deleted.");
	}

	return errorno;
}

}; // programexit()
