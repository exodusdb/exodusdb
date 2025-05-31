#include <vector>

#include <exodus/program.h>
programinit()

function main() {

	let syntax =
			"Syntax:\n"
			"	serve_any LIBNAME DBNAME ... {OPTIONS}\n"
			"\n"
			"Example:\n"
			"	serve_any serve_exo exodus\n"
			"\n"
			"LIBNAME:\n"
			"	The libname without lib/so prefix/suffix.\n"
			"\n"
			"DBNAME:\n"
			"	The database name(s) to serve.\n"
			"	Repeat the same name for multiple parallel connections.\n"
			"	$EXO_DATA will be used if DBNAME is omitted.\n"
			"\n"
			"OPTIONS:\n"
			"	Are forwarded to LIBNAME if any.\n"
			"\n"
			"Environment variables:\n"
			"	EXO_SERVICE_CODE and/or EXO_DATA\n"
			"\n"
			"pidfile:\n"
			"	/run/exodus/$EXO_SERVICE_CODE or $EXO_DATA\n";
	;

	// Save pid so systemd can send restart signal to us
	// /run/exodus/xxxxxxxx.pid
	let pidfilename = createpidfile();

	// Get service command
	let service_command = COMMAND.field(FM, 2);
	if (not service_command)
		abort("serve_any: Syntax error: LIBNAME is missing but required.\n\n" ^ syntax);

	// Check service lib exists
	if (not libinfo(service_command))
		abort("serve_any: Error: LIBNAME " ^ quote(service_command) ^ " does not exist or cannot be loaded.\n\n" ^ syntax);

	// Either one job per database on the command line
	var databasecodes = COMMAND.field(FM, 3, 9999);

	// Or default to the environment database
	if (not databasecodes) {
		databasecodes = osgetenv("EXO_DATA");

//		// Automatic three jobs except for test databases
//		if (not databasecodes.ends("_test"))
//			databasecodes ^= FM ^ databasecodes ^ FM ^ databasecodes;
	}

	// Check database codes
	if (not databasecodes)
		abort("serve_any: Syntax error: DBNAME is missing and EXO_DATA env var is not set. One or the other is required.\n\n" ^ syntax);

	// Options
	let options = " {" ^ OPTIONS ^ "}";

	// Perform one or run multiple services in parallel.
	let ndatabases = fcount(databasecodes, FM);
	if (ndatabases eq 1) {

		// Dont run (start a new thread) if only one database
		perform (service_command ^ " " ^ databasecodes ^ options);

	} else {

		// Run a few processes in parallel to handle concurrent requests
		// to prevent long running processes like reports
		// blocking casual users doing data entry or short queries.
		for (var databasecode : databasecodes)
			run(service_command ^ " " ^ databasecode ^ options);

		// Wait for all jobs to finish.
		for (auto& _ : run_results()) {}

		printl("INFO:", THREADNO, service_command ^ ": All", ndatabases, "thread(s) finished.");

	}

	if (osfile(pidfilename) and not osremove(pidfilename))
		loglasterror();

	// if (RELOAD_req)
	// 	return 1;

	// assume systemd has Restart=always
	// https://www.freedesktop.org/software/systemd/man/systemd.service.html
	return 0;
}

func createpidfile() {

	let rundir = "/run/exodus/";
	if (not osdir(rundir) and not osmkdir(rundir))
		loglasterror();

	var servicecode = osgetenv("EXO_SERVICE_CODE");
	if (not servicecode)
		servicecode = osgetenv("EXO_DATA");

	let pidfilename = rundir ^ servicecode ^ ".pid";
	let pid			= ospid();

	if (not oswrite(pid, pidfilename))
		loglasterror();

	return pidfilename;

}

}; // programexit()
