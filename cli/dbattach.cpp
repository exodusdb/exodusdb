#include <exodus/program.h>
programinit()

func main() {

	let foreign_dbcode = COMMAND.f(2);
	var filenames = COMMAND.field(FM, 3, 999999);

	if (not foreign_dbcode and not OPTIONS) {

		let syntax =
			"NAME\n"
			"    dbattach - Attach foreign database files to the current default database\n"
			"\n"
			"SYNOPSIS\n"
			"    dbattach [TARGETDB] [FILENAME,...] {OPTION...}\n"
			"\n"
			"EXAMPLE\n"
			"    EXO_DATA=db1 dbattach db2 file1 filen2 {F}\n"
			"\n"
			"    file1 and file2 in db1 are foreign and actually resident in db2\n"
			"\n"
			"OPTIONS\n"
			"    F   Forcibly delete any normal files in the current default database.\n"
			"\n"
			"    R   Detach foreign files\n"
			"    RR  Remove foreign database connection.\n"
			"    RRR Remove all foreign database connections and attachments.\n"
			"\n"
			"    L   List attached foreign files in the current default database\n"
			"\n"
			"NOTES\n"
			"    Attachments are permanent until re-attached or removed.\n"
			"\n"
			"    Indexes on attached files behave strangely.\n"
			"\n"
			"    Detaching will result in missing files because attaching {F} deletes them.\n"
			"    Fix by pg_dump detached files from source and importing into target\n"
			"\n"
			"ISSUES\n"
			"    TARGETDB must be on the same connection as the current default database.\n"
			"\n"
			"    Current user must have access to the foreign database.\n"
			"\n"
			"    If EXO_USER and EXO_PASS are not set then exodus defaults are used.\n"
			"\n"
			"    exodus .config file is not used currently.\n"
			;

		abort(syntax);
	}

	// TODO Get user from .config/Allow control
	var dbcode = osgetenv("EXO_DATA");
	if (not dbcode)
		dbcode = "exodus";

	// TODO Get user from .config/Allow control
	var dbuser1 = osgetenv("EXO_USER");
	if (not dbuser1)
		dbuser1 = "exodus";

	// TODO Allow control over dbuser2/dbpass2
	let dbuser2 = dbuser1;
	var dbpass2 = osgetenv("EXO_PASS");
	if (not dbpass2)
		dbpass2 = "somesillysecret";

	// Default connection
	var conn1;
	if (not conn1.connect())
		abort(conn1.lasterror());

	var sql;

	if (OPTIONS.contains("L")) {

		sql = "select foreign_server_name, foreign_table_name from information_schema.foreign_tables;";
		var result;
		if (not conn1.sqlexec(sql, result))
			abort(conn1.lasterror());

		// change RM to FM etc. and remove column headings
		result.lowerer();
		result.remover(1);

		//format for printing
		result.converter(FM, "\n");
		result.converter(VM, " ");

		printl(result);
		return 0;
	}

	////////////////////////////////////////////////////////////////////////////
	// Create an interdb connection from default database to the target database
	////////////////////////////////////////////////////////////////////////////

	// If no filenames are provided then just setup the interdb connection
	if (not filenames or OPTIONS.contains("R")) {

		/////////////////////////////////////////////////////////////////////////
		// Must be done as a superuser eg postgres or exodus with superuser power
		/////////////////////////////////////////////////////////////////////////

		// Check we are a superuser
		sql = "ALTER USER " ^ dbuser1 ^ " WITH SUPERUSER";
		if (not conn1.sqlexec(sql))
			abort(conn1.lasterror());

		// Reset all interdb connections
		if (OPTIONS.contains("RRR")) {
			if (not conn1.sqlexec("DROP EXTENSION IF EXISTS postgres_fdw CASCADE"))
				abort(conn1.lasterror());
			return 0;
		}
		// Install extension required to establish interdb connections
		if (not conn1.sqlexec("CREATE EXTENSION IF NOT EXISTS postgres_fdw WITH SCHEMA public"))
			abort(conn1.lasterror());

		if (not foreign_dbcode)
			return 0;

		// Reset the interdb connection to the target db. Remove all connected tables.
		if (not conn1.sqlexec("DROP SERVER IF EXISTS " ^ foreign_dbcode ^ " CASCADE"))
			abort(conn1.lasterror());

		// Create an interdb connection to the target server
		if (OPTIONS.contains("RR"))
			return 0;

		if (not conn1.sqlexec("CREATE SERVER IF NOT EXISTS " ^ foreign_dbcode ^ " FOREIGN DATA WRAPPER postgres_fdw OPTIONS (host 'localhost', dbname '" ^ foreign_dbcode ^ "', port '5432')"))
			abort(conn1.lasterror());

		// Allow current user to use the interdb connection
		if (not conn1.sqlexec("GRANT USAGE ON FOREIGN SERVER " ^ foreign_dbcode ^ " TO " ^ dbuser1))
			abort(conn1.lasterror());

		// Remove any existing user/connection configuration
		if (not conn1.sqlexec("DROP USER MAPPING IF EXISTS FOR " ^ dbuser1 ^ " SERVER " ^ foreign_dbcode))
			abort(conn1.lasterror());

		if (OPTIONS.contains("R"))
			return 0;

		// Configure user/connection parameters. Target dbuser and dbpass.
		if (not conn1.sqlexec("CREATE USER MAPPING FOR " ^ dbuser1 ^ " SERVER " ^ foreign_dbcode ^ " OPTIONS (user '" ^ dbuser2 ^ "', password '" ^dbpass2 ^ "')"))
			abort(conn1.lasterror());

		//printl("OK user " ^ dbuser1 ^ " in db " ^ dbcode ^ " now has access to db " ^ foreign_dbcode);

		return 0;
	}

	///////////////////////////////////////////////////////////////////
	// Attach target db files as apparent files in the default database
	///////////////////////////////////////////////////////////////////

	// Remainder must be done as operating user eg exodus
	//\psql -h localhost -p 5432 adlinec3 exodus

	// Convert filenames to a comma separated list
	filenames.converter(",", FM);
	filenames.trimmer(FM);

	for (var filename : filenames) {

		if (not filename)
			continue;

		//logputl("Remove any existing connected foreign table " ^ filename);
		if (conn1.open(filename) and not conn1.sqlexec("DROP FOREIGN TABLE IF EXISTS " ^ filename))
			{}//abort(conn1.lasterror());

		// Option to forcibly delete any existing files in the local/default database
		if (OPTIONS.contains("F")) {

			//logputl("Remove any existing table " ^ filename);
			//if (not conn1.sqlexec("DROP TABLE IF EXISTS " ^ filenames.replace(","," ")))
			//      abort(conn1.lasterror());
			// after dropping foreign table, this open will check for local table
			if (conn1.open(filename) and not conn1.deletefile(filename))
				{}//logputl(conn1.lasterror());
		}

		//logputl("Connect to the foreign table " ^ filename);
		// creates the local foreign table
		let sql = "IMPORT FOREIGN SCHEMA public LIMIT TO (" ^ filename ^ ") FROM SERVER " ^ foreign_dbcode ^ " INTO public";
		if (not conn1.sqlexec(sql)) {
			let lasterror = conn1.lasterror();

			// If the server is not already attached then attach it
			// ERROR:  server "adlinek_test" does not exist
			//  sqlstate:42704^IMPORT FOREIGN SCHEMA public LIMIT TO (markets) FROM SERVER adlinek_test INTO public
			if (lasterror.contains("sqlstate:42704")) {
				if (not osshell(SENTENCE.field(" ", 1, 2)))
					abort("");
			}

			if (not conn1.sqlexec(sql))
				abort(conn1.lasterror());
		}

	}

	return 0;
}

}; // programexit()
