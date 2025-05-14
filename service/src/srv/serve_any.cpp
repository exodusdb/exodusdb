#include <vector>

#include <exodus/program.h>
programinit()

function main() {

	// Save pid so systemd can send restart signal to us
	// Similar code in serve_exo and server_APP_CODE
	let pidfilename = [](){
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
	}();

	var service_command = COMMAND.field(FM, 2);

	// Either one job per database on the command line
	var databasecodes = COMMAND.field(FM, 3, 9999);

	// Or default to the environment database
	if (not databasecodes) {
		databasecodes = osgetenv("EXO_DATA");

		// Automatic three jobs except for test databases
		if (not databasecodes.ends("_test"))
			databasecodes ^= FM ^ databasecodes ^ FM ^ databasecodes;
	}

	let options = " {" ^ OPTIONS ^ "}";

	let ndatabases = fcount(databasecodes, FM);
	if (ndatabases eq 1) {

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
		abort(lasterror());

	// if (RELOAD_req)
	// 	return 1;

	// assume systemd has Restart=always
	// https://www.freedesktop.org/software/systemd/man/systemd.service.html
	return 0;
}

}; // programexit()
