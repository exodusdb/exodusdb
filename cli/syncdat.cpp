#include <exodus/program.h>
programinit()

	let force = index(OPTIONS, "F");
	let generate = index(OPTIONS, "G");
	let ignore_nodb = index(OPTIONS, "I");
	let verbose = index(OPTIONS, "V");

	var last_sync_date;
	var last_sync_time;

	var errors = "";

func main() {

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
			"    I = Ignore if no DB\n"
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
			"\n"
			"NOTES\n"
			"\n"
			"An empty dat file causes deletion of the database RECORD.\n"
			"\n"
			"An empty dat dir or a dir containing the file 'SYNCDAT_DELETE'\n"
			"causes deletion of the database FILE\n"
		);
		return 0;
	}

	// Quit if no default database connection
	if (not connect()) {
		let msg = "No default db connection to perform syncdat, ";
		if (ignore_nodb)
			stop(msg ^ "and option I - Ignore if no db. Continuing.");
		else
			abort(msg ^ "and no option I to ignore. Aborting.");
	}

	// "Elvis" operator and temporaries dont mix.
	// DONT USE USE TEMPORARY CONDITION WITHOUT PROVIDING THE MIDDLE ARGUMENT
	// Creates random runtime crash
	//let homedir = osgetenv("EXO_HOME") ?: osgetenv("HOME");
	let exo_home = osgetenv("EXO_HOME");
	let homedir = exo_home ?: osgetenv("HOME");
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
	let definitions_key = "LAST_SYNCDATE_TIME*DAT";
	if (not definitions or not read(last_sync, definitions, definitions_key))
		last_sync = "";
	last_sync_date = last_sync.f(1);
	last_sync_time = last_sync.f(2);

	// Skip if nothing new
	let datinfo = osdir(datpath);
	if (not datinfo) {
		abort(prefix ^ " Error: " ^ datpath.quote() ^ " dat dir missing");
	}
	printl(prefix, "Scanning", datpath, datinfo.f(2).oconv("D-Y"), datinfo.f(3).oconv("MTS"));
	if (not force and not is_newer(datinfo)) {
		if (verbose)
			printl("No change since", last_sync_date.oconv("D-Y"), last_sync_time.oconv("MTS"));
		return 0;
	}

	//errors = "";
	if (not begintrans()) {
		errors(-1) = lasterror();
		loglasterror();
	}

	// Process each subdir in turn. each one represents a db file.

	var dict_cpp_filenames = "";

	for (var dirname : dirnames) {

		var dbfile;
		let dbfilename = dirname;
		let isdict = dbfilename.starts("dict.");

		let dirpath = homedatpath ^ "/" ^ dirname ^ "/";

// Dont skip old dirs, since we skip old files below , and it doesnt take much time to scan dirs
//		// Skip dirs which are not newer i.e. have no newer records
//		if (not force and not generate) {
//			let dirinfo = osdir(dirpath);
//			if (not is_newer(dirinfo)) {
//				//if (verbose)
//					printl("Nothing new in", dirpath, dirinfo.f(2).oconv("D-Y"), dirinfo.f(3).oconv("MTS"));
//				continue;
//			}
//		}

		let osfilenames = oslistf(dirpath ^ "*").sort();

		if (not generate) {

			// Empty dirs or dirs containing a file SYNCDAT_DELETE
			// cause deletion of the database file with the same name as the dir
			if (osfilenames.lower().locate("SYNCDAT_DELETE") or osfilenames == "") {
				if (var().open(dbfilename)) {

					//if (verbose)
					logputl("syncdat: DELETING " ^ dbfilename.quote());

					// TODO delete all functions first for S dict items with pgsql

					if (not var("").deletefile(dbfilename)) {
						errors(-1) = lasterror();
						loglasterror();
						logputl(dbfilename, "could not be deleted");
						continue;
					}
				}
				continue; // next dirname
			}

			// Open or create the target db file
			if (not open(dbfilename, dbfile)) {
				if (not createfile(dbfilename)) {
					loglasterror();
					continue;
				}
				if (not open(dbfilename, dbfile)) {
					errors(-1) = lasterror();
					loglasterror();
					continue;
				}
			}

		}

		var new_cpp_text = "#include <exodus/dict.h>\n\n";
		var dict2sql_ids = "";

		var common_lines = "";

		// Process each dat file/record in the subdir
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

			// Add it to new_cpp_text
			if (generate and isdict and RECORD.f(1) == "S") {

				var dictsrc = RECORD(8);
				dictsrc.converter(VM, "\n");

				dictsrc.replacer("function main(", "func main(");
				let hasfuncmain = dictsrc.contains("func main(");

				// dict intro
				var line1 = (hasfuncmain ? "\nlibraryinit(" : "\ndictinit(") ^ ID.lcase() ^ ")";
//				let line1 = "\ndictinit(" ^ ID.lcase() ^ ")";
				new_cpp_text ^= line1 ^ "\n";
//				new_cpp_text ^= "/" "/" ^ str("-", len(line1) - 3) ^ "\n";

                // Remove #include <xxx_common.h> lines
				var lines = dictsrc.match(R"__(#include\s*[<"][a-z]{2,3}_common.h[>"]\s*\n)__").sort().unique();
				if (lines) {
					dictsrc.replacer(R"__(#include\s*[<"][a-z]{2,3}_common.h[>"]\s*\n)__"_rex, "");
					common_lines ^= lines.convert("\n", "") ^ FM;
				}

				new_cpp_text ^= dictsrc ^ "\n";

				// Close func main opened in dictinit
				if (not hasfuncmain)
					new_cpp_text ^= '}';

				// dict outro
				new_cpp_text ^= "};\n";

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

				if (RECORD == oldrecord) {
					if (exists and RECORD.len() == 0) {
						// Delete the RECORD
						if (not deleterecord(dbfile, ID))
							loglasterror();
						else
							printl(prefix, dbfilename, ID, "Deleted");
					} else {
						if (verbose)
							printl("Not changed", dbfilename, ID);
					}
				} else {
					if (RECORD.len() == 0) {
						// Delete the RECORD
						if (not deleterecord(dbfile, ID))
							loglasterror();
						else
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

		// Load and changed functions into database
		if (dict2sql_ids) {
			let cmd = "dict2sql " ^ dbfilename ^ " " ^ dict2sql_ids;
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

			// Work out dir where to put the .cpp file
			// ~/exodus/service/dat -> ~/exodus/service/dic
			let dicpath = datpath.cut(-3) ^ "dic";
			// Create it if necessary
			if (not osdir(dicpath)) {
				if (not osmkdir(dicpath)) {
					errputl(lasterror());
					continue;
				}
			}

			// Work out where the dict_xxxxxx.cpp file path and name
			// ~/exodus/service/dat/dict.voc
			// ~/exodus/service/dic/dict_voc.cpp
			let dict_cpp_filename = (dicpath ^ "/" ^ dirname).replace("/dat/", "/dic/").replace("dict.", "dict_") ^ ".cpp";

			// delete obsolete old dict_xxxxxxx.cpp files in per module dirs
			{
				let incfilename = homedir ^ "/inc/" ^ dbfilename.convert(".", "_") ^ ".h";

				var incfiletext;
				if (osread(incfiletext from incfilename)) {

					// First line of /root/inc/dict_units.h
					//generated by exodus "compile /root/neosys/src/gen/dict_units.cpp"

					let olddict_cpp_filename = incfiletext.field(DQ, 2).field(" ", 2);

					if (not olddict_cpp_filename.ends(dict_cpp_filename)) {
						if (osremove(olddict_cpp_filename)) {
							printl("Removed old cpp file:", olddict_cpp_filename);
						} else {
							errputl(lasterror());
							errors(-1) = lasterror();
						}
					}
				}
			}

			var old_cpp_text;
			if (not osread(old_cpp_text from dict_cpp_filename))
				old_cpp_text = "";

			// Remove one of the /n/n appended after every libraryexit()
			new_cpp_text.popper();

			// All #insert <xxx.common.h> at the beginning.
			if (common_lines) {
				common_lines.uniquer();
				common_lines.converter(FM, "\n");
				common_lines ^= "\n";
				//new_cpp_text.prefixer(common_lines);
				new_cpp_text.fieldstorer("\n", 3, -1, common_lines);
			}

			// Limit blank lines to one.
			new_cpp_text.replacer("\n\n\n+"_rex, "\n\n");
			new_cpp_text.trimmerboth("\n");
			new_cpp_text ^= '\n';

			// Update
			if (new_cpp_text == old_cpp_text) {
				if (verbose)
					printl("Already up to date", dict_cpp_filename);
			} else {
				//if (not oswrite(new_cpp_text on dbfilename))
				if (not oswrite(new_cpp_text on dict_cpp_filename))
					abort("syncdat cannot write " ^ dict_cpp_filename);
				printl("Updated", dict_cpp_filename);
			}

		dict_cpp_filenames ^= dict_cpp_filename ^ " ";

		} // of store and compile generated _cpp_ file

	} // next dat dir

	// Compile, or just generate headers.
	if (dict_cpp_filenames) {
		let options = "S" ^ generate ? "h" : "";
		let cmd = "compile " ^ dict_cpp_filenames ^ "{" ^ options ^ "}";
		printl(cmd);
		if (not osshell(cmd))
			abort("syncdat could not compile one or more dict_xxxx.cpp files");
	}

	// Record the current sync date and time
	if (not generate and definitions) {
		write(date() ^ FM ^ time() on definitions, definitions_key);
	}

	if (not committrans())
		errors(-1) = lasterror();

	if (errors)
		errors.errputl("\nsyncdat: Errors: ");

	return errors != "";

}

func is_newer(in fsinfo) {

    int fsinfo_date = fsinfo.f(2);

    if (fsinfo_date > last_sync_date)
        return true;

    if (fsinfo_date < last_sync_date)
        return false;

    return fsinfo.f(3) > last_sync_time;

}

}; // programexit()
