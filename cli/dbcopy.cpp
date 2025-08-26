#include <exodus/program.h>
programinit()

func main() {

	let sourcedb = COMMAND.f(2);
	let targetdb = COMMAND.f(3);
	if (not sourcedb or not targetdb)
		abort("Syntax is dbcopy [from_dbname] [to_dbname] {S}");

	if (!sourcedb.dbcopy(sourcedb, targetdb)) {
		if (not OPTIONS.contains("S"))
			errputl(var().lasterror());
		abort("");
	}

	if (not OPTIONS.contains("S"))
		printl(sourcedb.quote(), "database copied to", targetdb.quote());


	// Warn user of copied foreign tables
	var dbconn;
	if (not dbconn.connect(targetdb))
		abort(lasterror());

	var foreigntables;
	if (not dbconn.sqlexec("SELECT foreign_server_name,foreign_table_name FROM information_schema.foreign_tables;", foreigntables)) {
		abort(lasterror());
	}

	if (foreigntables) {
		if (not OPTIONS.contains("S")) {
			logputl("WARNING: The following foreign tables from " ^ sourcedb.quote() ^ " have been preserved in " ^ targetdb.quote());
			logputl("Use 'dbattach' to change this.");
			logputl(foreigntables.replace(RM, "\n").replace(FM, " "));
		}
	}
	return 0;
}

}; // programexit()


