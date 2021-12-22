#include <exodus/program.h>
programinit()

	var last_sync_date;
	var last_sync_time;

function main() {

	var datpath = COMMAND.a(2);

	if (not datpath) {
		//datpath = osgetenv("EXO_HOME") ^ "/dat";
		if (not datpath.osgetenv("EXO_HOME"))
			datpath = osgetenv("HOME");
		datpath ^= "/dat";
	}

	var force = index(OPTIONS, "F");
	var verbose = index(OPTIONS, "V");

	// Skip if no definitions file
	var definitions;
	if (not open("DEFINITIONS",definitions)) {
		//abort("Error: sync_dat - No DEFINITIONS file");
		definitions = "";
	}

	// Get the date and time of last sync
	var last_sync;
	var definitions_key = "LAST_SYNC_DATE_TIME*DAT";
	if (not definitions or not read(last_sync,definitions, definitions_key))
		last_sync = "";
	last_sync_date = last_sync.a(1);
	last_sync_time = last_sync.a(2);

	// Skip if nothing new
	var datinfo = osdir(datpath);
	var dirtext = "sync_dat: " ^ datpath ^ " " ^ datinfo.a(2).oconv("D-Y") ^ " " ^ datinfo.a(3).oconv("MTS");
	if (not force and not is_newer(datinfo)) {
		if (verbose)
			printl(dirtext, "No change.");
		return 0;
	}
	printl(dirtext,"Scanning ...");

	begintrans();

	// Process each subdir in turn. each one represents a db file.
	var dirnames = oslistd(datpath ^ "/*");
	for (var dirname : dirnames) {

		var dirpath = datpath ^ "/" ^ dirname ^ "/";

//		// Skip dirs which are not newer i.e. have no newer records
//		if (not force) {
//			var dirinfo = osdir(dirpath);
//			if (not is_newer(dirinfo)) {
//				if (verbose)
//					printl("Nothing new in", dirpath, dirinfo.a(2).oconv("D-Y"), dirinfo.a(3).oconv("MTS"));
//				continue;
//			}
//		}

		// Open or create the target db file
		var dbfile;
		var dbfilename = dirname;
		if (not open(dbfilename, dbfile)) {
			createfile(dbfilename);
			if (not open(dbfilename, dbfile)) {
				errputl("Error: sync_dat cannot create " ^ dbfilename);
				continue;
			}
		}

		// Process each file/record in the subdir
		var osfilenames = oslistf(dirpath ^ "*");
		for (var osfilename : osfilenames) {

			ID = osfilename;

			if (not ID)
				continue;

			if (ID.starts(".") or ID.ends(".swp"))
				continue;

			// Skip files/records which are not newer
			var filepath = dirpath ^ ID;
			if (not force and not is_newer(osfile(filepath))) {
				if (verbose)
					printl("Not newer", dbfilename, ID);
				continue;
			}

			if (not osread(RECORD from filepath)) {
				errputl("Error: sync_dat cannot read " ^ ID ^ " from " ^ filepath);
				continue;
			}
			gosub unescape_text(RECORD);

			//get any existing record
			var oldrecord;
			if (not read(oldrecord from dbfile, ID)) {
				oldrecord = "";
			}

			if (RECORD eq oldrecord) {
				if (verbose)
					printl("Not changed", dbfilename, ID);
			} else {
				// Write the RECORD
				write(RECORD on dbfile, ID);
				printl("sync_dat:", dbfilename, ID, "WRITTEN");
			}

			// Create pgsql using dict2sql
			// DONT SKIP SINCE PGSQL FUNCTIONS MUST BE IN EVERY DATABASE
			if (dbfilename.starts("dict.") and RECORD.index("/" "*pgsql")) {
				var cmd = "dict2sql " ^ dbfilename ^ " " ^ ID;
				//cmd ^= " {V}";
				if (not osshell(cmd))
					errputl("Error: sync_dat: In dict2sql for " ^ ID ^ " in " ^ dbfilename);
			}
		}
	}

	// Record the current sync date and time
	if (definitions)
		write(date() ^ FM ^ time() on definitions, definitions_key);

	committrans();

	return 0;
}

function is_newer(in fsinfo) {

    int fsinfo_date = fsinfo.a(2);

    if (fsinfo_date > last_sync_date)
        return true;

    if (fsinfo_date < last_sync_date)
        return false;

    return fsinfo.a(3) > last_sync_time;

}

//WARNING: KEEP AS REVERSE OF escape_text() IN COPYFILE
//identical code in copyfile and sync_dat
subroutine unescape_text(io record) {

	//replace new lines with FM
	record.converter("\n", FM);

	//unescape new lines
	record.swapper("\\n", "\n");

	//unescape backslashes
	record.swapper("\\\\", "\\");

	return;
}

programexit()

