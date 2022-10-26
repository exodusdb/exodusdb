#include <exodus/program.h>
programinit()

	let force = index(OPTIONS, "F");
	let generate = index(OPTIONS, "G");
	let verbose = index(OPTIONS, "V");

	var last_sync_date;
	var last_sync_time;

function main() {

	if (index(OPTIONS, "H")) {
		logputl(
			"NAME\n"
			"\n"
			"    syncdat\n"
			"\n"
			"DESCRIPTION\n"
			"\n"
			"    Copies dat files into database files.\n"
			"    or generates and compiles dict_*.cpp files.\n"
			"\n"
			"SYNTAX\n"
			"\n"
			"    syncdat [datpath] [filename]... [{OPTIONS}]\n"
			"\n"
			"    datpath defaults to $EXO_HOME/dat or $HOME/dat\n"
			"\n"
			"OPTIONS {inside braces}\n"
			"\n"
			"    F = Force. Dont skip dirs/files older than\n"
			"               last syncdat recorded in DEFINITIONS file\n"
			"\n"
			"    G = Generate and compile dict_*.cpp\n"
			"\n"
			"    V = Verbose\n"
			"\n"
			"EXAMPLES\n"
			"\n"
			"    syncdat # Do all dat files not done already\n"
			"\n"
			"    syncdat {FG} # Do all dat files\n"
			"                  # Generate/compile dict*.cpp files.\n"
			"\n"
			"    syncdat '' voc dict.voc # Just the voc and dict.voc files\n"
		);
		return 0;
	}

	let homedir = osgetenv("EXO_HOME") ?: osgetenv("HOME");
	let homedatpath = homedir ^ "/dat";

	// Get list of dirs from command line or ~/dat
	// BUT we will always process items found in ~/dat
	// so that any merged items from exodus and apps will
	// be correctly added and cpp files fully generated
	//let datpath = COMMAND.f(2) ?: homedatpath;
	let f2 = COMMAND.f(2);
	let datpath = f2 ?: homedatpath;
	let f3 = COMMAND.field(FM, 3, 999999);
	let dirnames = f3 ?: oslistd(datpath ^ "/" "*").sort();

	let txtfmt = "TX";

	let prefix = THREADNO ^ ": syncdat:";

	// Warn if no definitions file
	var definitions;
	if (not open("DEFINITIONS", definitions)) {
		if (not generate)
			//abort(prefix ^ " Warning: No DEFINITIONS file");
			logputl(prefix, "Warning: No DEFINITIONS file");
		definitions = "";
	}

	// Get the date and time of last sync
	var last_sync;
	var definitions_key = "LAST_SYNCDATE_TIME*DAT";
	if (not definitions or not read(last_sync, definitions, definitions_key))
		last_sync = "";
	last_sync_date = last_sync.f(1);
	last_sync_time = last_sync.f(2);

	// Skip if nothing new
	var datinfo = osdir(datpath);
	if (not datinfo) {
		abort(prefix ^ " Error: " ^ datpath.quote() ^ " dat dir missing");
	}
	printl(prefix, "Scanning", datpath, datinfo.f(2).oconv("D-Y"), datinfo.f(3).oconv("MTS"));
	if (not force and not is_newer(datinfo)) {
		if (verbose)
			printl("No change since", last_sync_date.oconv("D-Y"), last_sync_time.oconv("MTS"));
		return 0;
	}

	var errors = "";
	if (not begintrans()) {
		errors(-1) = lasterror();
		loglasterror();
	}

	// Process each subdir in turn. each one represents a db file.

	var dictcppfilenames = "";

	for (var dirname : dirnames) {

		let dbfilename = dirname;
		let isdict = dbfilename.starts("dict.");

		let dirpath = homedatpath ^ "/" ^ dirname ^ "/";

// Dont skip old dirs, since we skip old files below , and it doesnt take much time to scan dirs
//		// Skip dirs which are not newer i.e. have no newer records
//		if (not force and not generate) {
//			var dirinfo = osdir(dirpath);
//			if (not is_newer(dirinfo)) {
//				//if (verbose)
//					printl("Nothing new in", dirpath, dirinfo.f(2).oconv("D-Y"), dirinfo.f(3).oconv("MTS"));
//				continue;
//			}
//		}

		// Open or create the target db file
		var dbfile;
		if (not generate and not open(dbfilename, dbfile)) {
			createfile(dbfilename);
			if (not open(dbfilename, dbfile)) {
				errors(-1) = lasterror();
				loglasterror();
				continue;
			}
		}

		var newcpptext = "#include <exodus/library.h>\n";
		var dict2sql_ids = "";

		// Process each dat file/record in the subdir
		//printl(prefix, dirpath);
		let osfilenames = oslistf(dirpath ^ "*").sort();
		for (var osfilename : osfilenames) {

			ID = osfilename;

			if (not ID)
				continue;

			if (ID.starts(".") or ID.ends(".swp"))
				continue;

			// Get the dat record
			let filepath = dirpath ^ ID;
			if (not osread(RECORD from filepath)) {
				//errputl(prefix, "Error: Cannot read " ^ ID ^ " from " ^ filepath);
				errors(-1) = lasterror();
				loglasterror();
				continue;
			}

			// unescape to convert from txt to pickos format.
			RECORD = RECORD.iconv(txtfmt);

			// RECORD may be empty indicating that it should be deleted
			// if present in the target dbfile

			// Very similar code in edir and syncdat
			let f1 = RECORD.f(1);
			if (generate and isdict and f1 and var("FS").contains(RECORD.f(1))) {

				// Check justification
				if (not var("LRTC").contains(RECORD.f(9)))
					abort("syncdat: Error: In " ^ filepath.quote() ^ " Field 9 of F/S dict items cannot be " ^ RECORD.f(9).quote() ^ "\nField 9 of F/S dict items must be L, R, C, T.");

				// Check width
				if (not RECORD.f(10).isnum())
					abort("syncdat: Error: In " ^ filepath.quote() ^ " Field 10 of F/S items cannot be " ^ RECORD.f(10).quote() ^ "\nField 10 of F/S items must be numeric");
			}

			// Add it to newcpptext
			if (generate and isdict and RECORD.f(1) eq "S") {

				// dict intro
				var line1 = "\nlibraryinit(" ^ ID.lcase() ^ ")";
				newcpptext ^= line1 ^ "\n";
				newcpptext ^= "/" "/" ^ str("-", len(line1) - 3) ^ "\n";

				var dictsrc = RECORD(8);
				dictsrc.converter(VM, "\n");

				// Add dict function intro
				var addfunctionmain = not dictsrc.contains("function main()");
				if (addfunctionmain) {
					newcpptext ^= "function main() {\n";

					// Add closing brace before pgsql , if present
					var t = dictsrc.index("\n/" "*pgsql");
					if (t) {
						dictsrc.paster(t, 1, "\n}\n");
						dictsrc.replacer("\n\n}", "\n}");
						addfunctionmain = false;
					}
				}
//TRACE(ID)
//TRACE(dictsrc)
//stop();
				newcpptext ^= dictsrc ^ "\n";

				// Close function main if not already done
				if (addfunctionmain) {
					if (newcpptext[-1] ne "\n")
						newcpptext ^= '\n';
					newcpptext ^= "}\n";
				}

				// dict outro
				newcpptext ^= "libraryexit(" ^ ID.lcase() ^ ")\n\n";

			} // generate and isdict and S item

			if (not generate) {

				// Skip on dir date time above

				// Skip files/records which are not newer
				// because reloading pg functions in slow
				// to force updates, perhaps touch dat dat/* and dat/*/*
				if (not force and not is_newer(osfile(filepath))) {
					if (verbose)
						printl("Not newer", dbfilename, ID);
					continue;
				}

				// Moved up so we can generate complete dict_xxxxxxxx.cpp source
				//if (not osread(RECORD from filepath)) {
				//	errputl(prefix, "Error: Cannot read " ^ ID ^ " from " ^ filepath);
				//	continue;
				//}
				//gosub unescape_text(RECORD);

				//get any existing record
				var oldrecord;
				var exists = true;
				if (not read(oldrecord from dbfile, ID)) {
					oldrecord = "";
					exists = false;
				}

				if (RECORD eq oldrecord) {
					if (exists and RECORD.len() eq 0) {
						// Delete the RECORD
						deleterecord(dbfile, ID);
						printl(prefix, dbfilename, ID, "Deleted");
					} else {
						if (verbose)
							printl("Not changed", dbfilename, ID);
					}
				} else {
					if (RECORD.len() eq 0) {
						// Delete the RECORD
						deleterecord(dbfile, ID);
						printl("syncdat:", dbfilename, ID, "Deleted");
					} else {
						// Write the RECORD
						write(RECORD on dbfile, ID);
						printl("syncdat:", dbfilename, ID, "Written");
					}
				}

				// Create pgsql using dict2sql
				// DONT SKIP SINCE PGSQL FUNCTIONS MUST BE IN EVERY DATABASE
				if (dbfilename.starts("dict.") and RECORD.contains("/" "*pgsql")) {
					dict2sql_ids ^= ID ^ " ";
				}

			} // of not generate

		} // next dat file

		// Load and chanegd functions into database
		if (dict2sql_ids) {
			var cmd = "dict2sql " ^ dbfilename ^ " " ^ dict2sql_ids;
			//cmd ^= " {V}";
			if (verbose)
				cmd.logputl();
			if (not osshell(cmd)) {
				//errputl(prefix, "Error: In dict2sql for " ^ dbfilename);
				errors(-1) = lasterror();
				loglasterror();
			}
		}

		// Store and compile the generated dict_xxxxxxx.cpp text
		if (generate and isdict) {

			var dictcppfilename = "dict_" ^ dirname;

			var incfilename = homedir ^ "/inc/" ^ dbfilename.convert(".", "_") ^ ".h";

			var incfiletext;
			if (not osread(incfiletext from incfilename)) {
				//abort("syncdat cannot read " ^ incfilename);
				errputl(prefix, "Warning: Cannot read " ^ incfilename);
				continue;
			}

			dictcppfilename = incfiletext.field(DQ, 2).field(" ", 2);
			var oldcpptext;
			if (not osread(oldcpptext from dictcppfilename))
				oldcpptext = "";

			// Remove one of the /n/n appended after every libraryexit()
			newcpptext.popper();

			// Update
			if (newcpptext eq oldcpptext) {
				if (verbose)
					printl("Already up to date", dictcppfilename);
			} else {
				//if (not oswrite(newcpptext on dbfilename))
				if (not oswrite(newcpptext on dictcppfilename))
					abort("syncdat cannot write " ^ dictcppfilename);
				printl("Updated", dictcppfilename);
			}

		dictcppfilenames ^= dictcppfilename ^ " ";

		} // of store and compile generated cpp file

	} // next dat dir

	if (dictcppfilenames) {
		var cmd = "compile " ^ dictcppfilenames ^ "{S}";
		printl(cmd);
		if (not osshell(cmd))
			abort("syncdat could not compile one or more dict_xxxx.cpp files");
	}

	// Record the current sync date and time
	if (not generate and definitions)
		write(date() ^ FM ^ time() on definitions, definitions_key);

	if (not committrans())
		errors(-1) = lasterror();

	if (errors)
		errors.errputl("\nsyncdat: Errors: ");

	return errors ne "";

}

function is_newer(in fsinfo) {

    int fsinfo_date = fsinfo.f(2);

    if (fsinfo_date > last_sync_date)
        return true;

    if (fsinfo_date < last_sync_date)
        return false;

    return fsinfo.f(3) > last_sync_time;

}

programexit()
