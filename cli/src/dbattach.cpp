#include <exodus/program.h>
programinit()

function main() {

	var dbname2 = COMMAND.a(2);
	var filenames = COMMAND.field(FM, 3, 999999);

	if (not dbname2) {
		var syntax =
		"dbattach - Attach other database files into the current/default database\n"
		"\n"
		"Syntax is dbattach TARGETDB [FILENAME,...] {OPTION...}\n"
		"\n"
		"Options:\n"
		"\n"
		"  F means forcibly delete any normal files in the current/default database.\n"
		"\n"
		"  R means remove the current file attachments.\n"
		"  RR means remove the current user/database connection map.\n"
		"  RRR means remove all connections and attachments.\n"
		"\n"
		"If no filenames are provided then a database connection is created.\n"
		"This is *required* before filenames can be provided and attached.\n"
		"\n"
		"Any existing files will be DELETED.\n"
		"\n"
		"Note that any secondary indexes on the attached files are ineffective.\n"
		"\n"
		"Attachments are permanent until re-attached or removed.";
		abort(syntax);
	}

	var dbuser1 = "exodus";

	var dbname1 = osgetenv("EXO_DATA");
	if (not dbname1)
		dbname1 = "exodus";

	var dbuser2 = dbuser1;
	var dbpass2 = "somesillysecret";

	var conn1;
	if (not conn1.connect())
		abort(conn1.lasterror());

	var sql;

	////////////////////////////////////////////////////////////////////////////
	// Create an interdb connection from default database to the target database
	////////////////////////////////////////////////////////////////////////////

	// If no filenames are provided then just setup the interdb connection
	if (not filenames) {

		/////////////////////////////////////////////////////////////////////////
		// Must be done as a superuser eg postgres or exodus with superuser power
		/////////////////////////////////////////////////////////////////////////

		// Check we are a superuser
		sql = "ALTER USER " ^ dbuser1 ^ " WITH SUPERUSER";
		if (not conn1.sqlexec(sql))
			abort(conn1.lasterror());

		// Reset all interdb connections
		if (OPTIONS.index("RRR")) {
			if (not conn1.sqlexec("DROP EXTENSION IF EXISTS postgres_fdw CASCADE"))
				abort(conn1.lasterror());
			stop();
		}
		// Install extension required to establish interdb connections
		if (not conn1.sqlexec("CREATE EXTENSION IF NOT EXISTS postgres_fdw WITH SCHEMA public"))
			abort(conn1.lasterror());

		// Reset the interdb connection to the target db. Remove all connected tables.
		if (not conn1.sqlexec("DROP SERVER IF EXISTS " ^ dbname2 ^ " CASCADE"))
			abort(conn1.lasterror());

		// Create an interdb connection to the target server
		if (OPTIONS.index("RR"))
			exit(0);

		if (not conn1.sqlexec("CREATE SERVER IF NOT EXISTS " ^ dbname2 ^ " FOREIGN DATA WRAPPER postgres_fdw OPTIONS (host 'localhost', dbname '" ^ dbname2 ^ "', port '5432')"))
			abort(conn1.lasterror());

		// Allow current user to use the interdb connection
		if (not conn1.sqlexec("GRANT USAGE ON FOREIGN SERVER " ^ dbname2 ^ " TO " ^ dbuser1))
			abort(conn1.lasterror());

		// Remove any existing user/connection configuration
		if (not conn1.sqlexec("DROP USER MAPPING IF EXISTS FOR " ^ dbuser1 ^ " SERVER " ^ dbname2))
			abort(conn1.lasterror());

		if (OPTIONS.index("R"))
			exit(0);

		// Configure user/connection parameters. Target dbuser and dbpass.
		if (not conn1.sqlexec("CREATE USER MAPPING FOR " ^ dbuser1 ^ " SERVER " ^ dbname2 ^ " OPTIONS (user '" ^ dbuser2 ^ "', password '" ^dbpass2 ^ "')"))
			abort(conn1.lasterror());

		printl("OK user " ^ dbuser1 ^ " in db " ^ dbname1 ^ " now has access to db " ^ dbname2);

		return 0;
	}

	///////////////////////////////////////////////////////////////////
	// Attach target db files as apparent files in the default database
	///////////////////////////////////////////////////////////////////

	// Remainder must be done as operating user eg exodus
	//\psql -h localhost -p 5432 adlinec3 exodus

	// Convert filenames to a comma separated list
	filenames.converter(",", FM).trimmer(FM);

	for (var filename : filenames) {

		if (conn1.open(filename)) {

			// Option to forcibly delete any existing files in the default database
			if (OPTIONS.index("F")) {

				//logputl("Remove any existing table " ^ filename);
				//if (not conn1.sqlexec("DROP TABLE IF EXISTS " ^ filenames.swap(","," ")))
				//      abort(conn1.lasterror());
				if (not conn1.deletefile(filename))
					{};//logputl(conn1.lasterror());
			}

			//logputl("Remove any existing connected foreign table " ^ filename);
			if (conn1.open(filename) and not conn1.sqlexec("DROP FOREIGN TABLE IF EXISTS " ^ filename))
				{};//abort(conn1.lasterror());
		}

		//logputl("Connect to the foreign table " ^ filename);
		if (not conn1.sqlexec("IMPORT FOREIGN SCHEMA public LIMIT TO (" ^ filename ^ ")"
			" FROM SERVER " ^ dbname2 ^ " INTO public"))
			abort(conn1.lasterror());
	}

	return 0;
}

programexit()
