#include <exodus/program.h>
programinit()

	var targetfilename;
	var sourcedb;
	var targetdb;
	var targetname;
	var targetdir;
	dim sql_text_in;
	var nlines;
	var ln;
	var sourcefilename;
	var file1;
	var file2;
	var recn;
	var dictonly;
	var nsame;
	var nchanged;
	var nnew;

function main() {

	if (not COMMAND.f(2) or not COMMAND.f(3)) {

		var syntax =
		"Syntax is copyfile [SOURCE:][SOURCEFILENAME,...] [TARGET:][TARGETFILENAME] {OPTIONS}\n"
		"\n"
		"SOURCE can be a database name or an sql file containing COPY data like that produced by pg_dump\n"
		"\n"
		"TARGET can be a database name or an OS dir path ending /\n"
		"\n"
		"SOURCE and TARGET must be followed by : otherwise the default database will used.\n"
		"\n"
		"OPTIONS only apply if target is a database.\n"
		"\n"
		"O - Overwrite is required to overwrite any existing records otherwise they are skipped.\n"
		"\n"
		"N - New is required to add new records to the target otherwise they are skipped.\n"
		"\n"
		//"M - Move causes any copied records to be deleted from the source.\n"
		//"\n"
		//"D - Delete causes any records in the target which dont exist in the source to be deleted.\n"
		//"\n"
		"C - Create is required if the target file does not exist and should be created. Implies Overwrite and New.\n"
		"\n"
		"R - Leave raw vm/sm/tm when copying to OS dir\n"
		"\n";

		abort(syntax);
	}

	// dat files - db files and records stored as os dirs and os files
	//
	// Linux files cannot contain / or char(0) characters
	// https://en.wikipedia.org/wiki/Filename#Comparison_of_filename_limitations
	//
	// Therefore dat file name and keys cannot contain the above characters for now.
	// Fortunately most dat files are dictionaries which tend to be more limited
	//  by sql column name restrictions.

	var allow_overwrite  = OPTIONS.contains("O");
	var allow_new        = OPTIONS.contains("N");
	//var move_option    = OPTIONS.contains("M");
	//var delete_option  = OPTIONS.contains("D");
	var allow_createfile = OPTIONS.contains("C");

	if (allow_createfile) {
		allow_overwrite = 1;
		allow_new = 1;
	}

	// Raw option
	var txt5fmt = "TX";
	// Raw option only converts FM to NL and leaves VM SM TM ST as are
	if (OPTIONS.contains("R"))
		txt5fmt ^= "1";

	//parse source
	var sourcename = "";
	var sourcefilenames = COMMAND.f(2);
	if (index(sourcefilenames,":")) {
		sourcename = field(sourcefilenames,":",1);
		sourcefilenames = field(sourcefilenames,":",2);
	}

	// / present in source without : treated as if : appended
	if (sourcefilenames.contains("/") and not sourcename) {
		sourcename = sourcefilenames;
		sourcefilenames = "";
	}

	//parse target
	targetname = "";
	var targetfilenames = COMMAND.f(3);
	if (index(targetfilenames,":")) {
		targetname = field(targetfilenames,":",1);
		targetfilenames = field(targetfilenames,":",2);
	}

	// / present in target without : treated as if : was appended
	if (targetfilenames.contains("/") and not targetname) {
		targetname = targetfilenames;
		targetfilenames = "";
	}

	//connect to source db if source not a path or .sql file
	sourcedb = "";
	if (not sourcename.contains("/") and not sourcename.ends(".sql") and not sourcedb.connect(sourcename))
		abort(sourcename.quote() ^ " Cannot connect to source");

	if (targetname.contains("/")) {

		//flag target is not a db
		targetdb = "";

		//target name will be a dir path
		if (targetname[-1] ne "/")
			targetname ^= "/";
	} else {
		//connect to target db
		if (not targetdb.connect(targetname))
			abort(targetname.quote() ^ " Cannot connect to target");
	}

	//dict means filter all and only dict files
	dictonly = sourcefilenames eq "dict";
	if (dictonly and sourcedb)
		sourcefilenames = "";

	//determine all files in source db if source not .sql
	if (sourcedb and not sourcefilenames) {
		sourcefilenames = sourcedb.listfiles();
		targetfilenames = "";
	}

	//go through files one by one if source is a db
	if (sourcedb)
		sourcefilenames.trimmer(",").converter(",", FM);

	for (const var& temp : sourcefilenames) {

		// Note that if the source is an sql os file name then
		// sourcefilename is actually a *list* of desired db filenames separated by commas
		// whereas if source is a db name then sourcefilename contains a single db filename

		sourcefilename = temp;

		//skip dict.all which is an sql view of all dict files
		if (sourcefilename eq "dict.all")
			continue;

		//option to skip all but dict files
		if (sourcedb and dictonly and not sourcefilename.starts("dict."))
			continue;

		// source is a db
		if (sourcedb) {

			//reset counters - duplicated in main() and getrec()
			nsame = 0;
			nchanged = 0;
			nnew =0;

			//open the source file
			if (not file1.open(sourcefilename, sourcedb) )
				abort(sourcefilename.quote() ^ " cannot be opened in source db " ^ sourcename);
		}

		// source is a .sql file
		else {

			//read in the .sql file text
			targetfilename = "";
			var txt;
			if (not txt.osread(sourcename))
				abort(sourcename.quote() ^ " does not exist or cannot be accessed");

			//split the sql text into an fixed array
			nlines = sql_text_in.split(txt,"\n");
			ln = 0;

		}


		// target is a database file. Open it.
		if (sourcedb and targetdb) {
			targetfilename = targetfilenames;
			if (not targetfilename)
				targetfilename = sourcefilename;

			//open target file
			if (not file2.open(targetfilename, targetdb)) {
				if (not allow_createfile) {
					abort(targetfilename.quote() ^ " does not exist in target db " ^ targetname ^ " and no (C)reate option provided.");
				}
				if (not targetdb.createfile(targetfilename) or not file2.open(targetfilename, targetdb))
					abort(targetfilename.quote() ^ " cannot be created in target db " ^ targetname);
			}
		}

		// Target is an os dir. Create it if source is a db and we therefore know the targetfilename.
		else {

			targetfilename = sourcefilename;

			// Duplicate code in main() and getrec()
			targetdir = targetname ^ targetfilename ^ "/";
			if (not osdir(targetdir) and not osmkdir(targetdir))
				abort(targetdir.quote() ^ " dir could not be created");
		}

		// Speed up updates
		if (targetdb)
			targetdb.begintrans();

		// Select source file if source is a db
		// In ID order to be consistent so diff will work better
		if (sourcedb) {
			printl(sourcefilename);
			file1.select(sourcefilename ^ " BY ID (R)");
		}

		//process source file
		recn = 0;
		while(getrec()) {

			//user interrupt
			if (not mod(recn,1000)) {
				//print(AT(-40), sourcefilename, recn ^ ".", ID);
				if (esctoexit())
					abort("");
			}

			//skip if not changed
			var oldrec;
			var exists;
			if (targetdb) {
				exists = oldrec.read(file2,ID);
			} else {
				exists = oldrec.osread(targetdir ^ ID);
				//escape into special txt format
				RECORD = RECORD.oconv(txt5fmt);
			}
			if (exists) {

				//skip update if no change
				if (RECORD eq oldrec) {
					nsame++;
					print("\tNot changed");
					continue;
				}

				nchanged++;
				print("\tChanged");
				if (not allow_overwrite) {
					printl("\t - skipped");
					continue;
				}
				printl();

			} else {

				nnew++;
				print("\tNew");
				if (not allow_new) {
					printl("\t - skipped");
					continue;
				}
				printl();
			}

			// Write to db file
			if (targetdb) {
				RECORD.write(file2, ID);
			}

			// Write to os file
			else {
				if (not oswrite(RECORD, targetdir ^ ID))
					abort("copyfile could not write " ^ targetdir ^ ID);
			}
		}

		print(AT(-40));
		//if (nsame or nchanged or nnew) {
		if (nchanged or nnew) {
			printl("Same:",nsame,"Changed:",nchanged, "New:",nnew);
			//printl();
		}

		//commit all target db updates
		if (targetdb)
			targetdb.committrans();

		printl();

	}

	return 0;
}

///////////////////
function getrec() {
///////////////////

	// If source is database then simply readnext RECORD and ID
	if (sourcedb) {

		var result = file1.readnext(RECORD, ID, MV);

		if (result) {
			recn++;
			print(AT(-40) ^ recn ^ ".", ID);
		}

		return result;
	}

	//otherwise get the next file and record from the source .sql file
	while (true) {

		ln++;

		if (ln gt nlines)
			return false;

		RECORD = sql_text_in(ln);

		// If we dont have a filename then find a line starting COPY and extract filename from it
		if (not targetfilename) {

			if (RECORD.field(" ", 1) != "COPY")
				continue;

			targetfilename = RECORD.field(" ", 2);

			// Remove public. from start of filename but not dict.
			if (targetfilename.starts("public."))
				targetfilename = targetfilename.cut(7);

			// Skip unwanted files
			if (not dictonly and sourcefilename and not sourcefilename.locateusing(targetfilename, ","))
				continue;

			// Skip file if not dict. and only dicts wanted
			if (dictonly and not targetfilename.starts("dict.")) {
				targetfilename = "";
				continue;
			}

			// Output filename by itself
			printl();
			printl(targetfilename);
			recn = 0;

			//reset counters - duplicated in main() and getrec()
			nsame = 0;
			nchanged = 0;
			nnew =0;

			// Open the target file
			if (targetdb) {
				if (not file2.open(targetfilename, targetdb) )
					abort(targetfilename.quote() ^ " cannot be opened in target db " ^ targetname);
			}

			// Target is an os dir. Create it.
			// Duplicate code in main() and getrec()
			else {
				targetdir = targetname ^ targetfilename ^ "/";
				if (not osdir(targetdir) and not osmkdir(targetdir))
					abort(targetdir.quote() ^ " dir could not be created.");
			}

			continue;

		}

		// A line with just "\." indicates no more records for the current file
		if (RECORD eq "\\.") {

			// Trigger a search for the next line starting COPY
			targetfilename = "";
			continue;
		}

		break;

	}

	recn++;

	ID = RECORD.field("\t",1);
	//RECORD[1,len(ID)+1] = ""
	RECORD.splicer(1,ID.len()+1,"");

	gosub unescape_sql(ID);
	gosub unescape_sql(RECORD);
	//TRACE(ID)

	print(AT(-40) ^ recn ^ ".", ID);

	return true;
}

subroutine unescape_sql(io arg1) {

	if (not arg1.contains("\\"))
		return;

	arg1.replacer("\\n","\n");
	arg1.replacer("\\b","\b");
	arg1.replacer("\\t","\t");
	arg1.replacer("\\v","\v");
	arg1.replacer("\\f","\f");
	arg1.replacer("\\r","\r");
	arg1.replacer("\\\\","\\");

	return;

}

programexit()
