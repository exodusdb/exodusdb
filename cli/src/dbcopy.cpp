#include <exodus/program.h>
programinit()

func main() {

	let dbname1 = COMMAND.f(2);
	let dbname2 = COMMAND.f(3);
	if (not dbname1 or not dbname2)
		abort("Syntax is dbcopy [from_dbname] [to_dbname] {S}");

	if (!dbname1.dbcopy(dbname1, dbname2)) {
		if (not OPTIONS.contains("S"))
			errputl(var().lasterror());
		abort("");
	}

	if (not OPTIONS.contains("S"))
		printl(dbname1.quote(), "database copied to", dbname2.quote());

	return 0;
}

}; // programexit()
