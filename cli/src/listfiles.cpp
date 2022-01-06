#include <exodus/program.h>
programinit()

function main() {

	var dbname = COMMAND.a(2);
	var connection = "";
	if (dbname and not connection.connect(dbname)) {
		abort(lasterror());
	}

	printl(connection.listfiles().convert(FM, "\n"));

	return 0;
}

programexit()
