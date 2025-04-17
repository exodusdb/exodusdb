#include <exodus/program.h>
programinit()

func main() {

	let dbname = COMMAND.f(2);
	var connection = "";
	if (dbname and not connection.connect(dbname)) {
		abort(lasterror());
	}

	printl(connection.listfiles().convert(FM, "\n"));

	return 0;
}

}; // programexit()
