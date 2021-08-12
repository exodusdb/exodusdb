#include <exodus/program.h>
programinit()

function main() {
	var dbname = COMMAND.a(2);
	dbcreate(dbname);
	return 0;
}

programexit()

