#include <exodus/program.h>
programinit()

	var last_sync_date;
	var last_sync_time;

function main() {

	if (index(OPTIONS, "H")) {
		logputl(
			"NAME\n"
			"\n"
			"    sync_dat\n"
			"\n"
			"DESCRIPTION\n"
			"\n"
			"    Copies dat files into database files.\n"
			"    Generates and compiles dict_*.cpp files.\n"
			"\n"
			"SYNTAX\n"
			"\n"
			"    sync_dat [datpath] [filename]... [{OPTIONS}]\n"
			"\n"
			"    datpath defaults to $EXO_HOME/dat or $HOME/dat\n"
			"\n"
			"OPTIONS {inside braces}\n"
			"\n"
			"    F = Force. Dont skip dirs/files older than\n"
			"               last sync_dat recorded in DEFINITIONS file\n"
			"\n"
			"    G = Also generate and compile dict_*.cpp\n"
			"\n"
			"EXAMPLES\n"
			"\n"
			"    sync_dat # Do all dat files not done already\n"
			"\n"
			"    sync_dat {FG} # Do all dat files\n"
			"                  # Generate/compile dict*.cpp files.\n"
			"\n"
			"    sync_dat '' voc dict.voc # Just the voc and dict.voc files\n"
		);
		stop();
	}

	var homedir;
	if (not homedir.osgetenv("EXO_HOME"))
		homedir = osgetenv("HOME");

	var datpath = COMMAND.f(2);
	if (not datpath) {
		datpath = homedir ^ "/dat";
	}

	var dirnames = COMMAND.field(FM, 3, 999999);

	var force = index(OPTIONS, "F");
	var generate = index(OPTIONS, "G");
	var verbose = index(OPTIONS, "V");

	var txtfmt = "TX";

	// Skip if no definitions file
	var definitions;
	if (generate or not open("DEFINITIONS", definitions)) {
		//abort("Error: sync_dat - No DEFINITIONS file");
		definitions = "";
	}

	// Get the date and time of last sync
	var last_sync;
	var definitions_key = "LAST_SYNC_DATE_TIME*DAT";
	if (not definitions or not read(last_sync, definitions, definitions_key))
		last_sync = "";
	last_sync_date = last_sync.f(1);
	last_sync_time = last_sync.f(2);

	// Skip if nothing new
	var datinfo = osdir(datpath);
	if (not datinfo) {
		abort("Error: syncdat: " ^ datpath.quote() ^ " dat dir missing");
	}
	var dirtext = "sync_dat: " ^ datpath ^ " " ^ datinfo.f(2).oconv("D-Y") ^ " " ^ datinfo.f(3).oconv("MTS");
	if (not force and not is_newer(datinfo)) {
		if (verbose)
			printl(dirtext, "No change.");
		return 0;
	}
	printl(THREADNO ^ ":", "Scanning", dirtext);

	begintrans();

	// Process each subdir in turn. each one represents a db file.
	if (not dirnames) {
		dirnames = oslistd(datpath ^ "/" "*").sort();
	}
	//printl(THREADNO ^ ":", "Scanning", dirnames.convert(FM, "^"));

	for (var dirname : dirnames) {

		var dirpath = datpath ^ "/" ^ dirname ^ "/";

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
		var dbfilename = dirname;
		if (not open(dbfilename, dbfile)) {
			createfile(dbfilename);
			if (not open(dbfilename, dbfile)) {
				errputl("Error: sync_dat cannot create " ^ dbfilename);
				continue;
			}
		}

		var isdict = dbfilename.starts("dict.");
		var newcpptext = "#include <exodus/library.h>\n";

		// Process each dat file/record in the subdir
		//printl(THREADNO ^ ":", dirpath);
		var osfilenames = oslistf(dirpath ^ "*").sort();
		for (var osfilename : osfilenames) {

			ID = osfilename;

			if (not ID)
				continue;

			if (ID.starts(".") or ID.ends(".swp"))
				continue;

			// Get the dat record
			var filepath = dirpath ^ ID;
			if (not osread(RECORD from filepath)) {
				errputl("Error: sync_dat cannot read " ^ ID ^ " from " ^ filepath);
				continue;
			}

			// unescape to convert from txt to pickos format.
			RECORD = RECORD.iconv(txtfmt);

			// RECORD may be empty indicating that it should be deleted
			// if present in the target dbfile

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
					var t = dictsrc.index("\n/*pgsql");
					if (t) {
						dictsrc.splicer(t, 1, "\n}\n");
						dictsrc.swapper("\n\n}", "\n}");
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
				//	errputl("Error: sync_dat cannot read " ^ ID ^ " from " ^ filepath);
				//	continue;
				//}
				//gosub unescape_text(RECORD);

				//get any existing record
				var oldrecord;
				if (not read(oldrecord from dbfile, ID)) {
					oldrecord = "";
				}

				if (RECORD eq oldrecord) {
					if (RECORD.len() eq 0) {
						// Delete the RECORD again
						deleterecord(dbfile, ID);
						printl("sync_dat:", dbfilename, ID, "DELETED");
					} else {
						if (verbose)
							printl("Not changed", dbfilename, ID);
					}
				} else {
					if (RECORD.len() eq 0) {
						// Write the RECORD
						write(RECORD on dbfile, ID);
						printl("sync_dat:", dbfilename, ID, "WRITTEN");
					} else {
						// Delete the RECORD
						deleterecord(dbfile, ID);
						printl("sync_dat:", dbfilename, ID, "DELETED");
					}
				}

				// Create pgsql using dict2sql
				// DONT SKIP SINCE PGSQL FUNCTIONS MUST BE IN EVERY DATABASE
				if (dbfilename.starts("dict.") and RECORD.contains("/" "*pgsql")) {
					var cmd = "dict2sql " ^ dbfilename ^ " " ^ ID;
					//cmd ^= " {V}";
					if (not osshell(cmd))
						errputl("Error: sync_dat: In dict2sql for " ^ ID ^ " in " ^ dbfilename);
				}

			} // of not generate

		} // next dat file

		// Store and compile the generated dict_xxxxxxx.cpp text
		if (generate and isdict) {

			var dictcppfilename = "dict_" ^ dirname;

			var incfilename = homedir ^ "/inc/" ^ dbfilename.convert(".", "_") ^ ".h";

			var incfiletext;
			if (not osread(incfiletext from incfilename)) {
				//abort("sync_dat cannot read " ^ incfilename);
				errputl("sync_dat cannot read " ^ incfilename);
				continue;
			}

			dictcppfilename = incfiletext.field(DQ, 2).field(" ", 2);
			var oldcpptext;
			if (not osread(oldcpptext from dictcppfilename))
				oldcpptext = "";

			// Update
			if (newcpptext eq oldcpptext) {
				printl("Already up to date", dictcppfilename);
			} else {
				//if (not oswrite(newcpptext on dbfilename))
				if (not oswrite(newcpptext on dictcppfilename))
					abort("sync_dat cannot write " ^ dictcppfilename);
				printl("Updated", dictcppfilename);
			}

			// Compile
			if (force or newcpptext ne oldcpptext) {
				var cmd = "compile " ^ dictcppfilename;
				printl(cmd);
				if (not osshell(cmd))
					abort("sync_dat could not compile " ^ dictcppfilename);
			}

		} // of store and compile generated cpp file

	} // next dat dir

	// Record the current sync date and time
	if (not generate and definitions)
		write(date() ^ FM ^ time() on definitions, definitions_key);

	committrans();

	return 0;
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

