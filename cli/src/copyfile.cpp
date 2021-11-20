#include <exodus/program.h>
programinit()

	var file1;
	var file2;
	var targetfilename;
	var sourcedb;
	var targetdb;
	var targetname;
	var targetdir;
	dim sql_text_in;
	var nlines;
	var ln;
	var sourcefilename;
	var recn;
	var dictonly;

function main() {

	if (not COMMAND.a(2) or not COMMAND.a(3)) {

		var syntax =
		"Syntax is copyfile [SOURCE:][SOURCEFILENAME,...] [TARGET:][TARGETFILENAME] {OPTIONS}\n"
		"\n"
		"SOURCE can be a database name or an sql file containing COPY data like that produced by pg_dump\n"
		"\n"
		"TARGET can be a database name or a dir path ending /\n"
		"\n"
		"SOURCE and TARGET must be followed by : otherwise the default database will used.";

		abort(syntax);
	}

	// dat files - db files and records stored as os dirs and filenames
	//
	// Linux files cannot contain / or char(0)
	// https://en.wikipedia.org/wiki/Filename#Comparison_of_filename_limitations
	//
	// Therefore dat files cannot contain the above characters for now.
	// Fortunately most dat files are dictionaries which tend to be more limited
	//  by sql column name restrictions

	//parse source
	var sourcename = "";
	var sourcefilenames = COMMAND.a(2);
	if (index(sourcefilenames,":")) {
		sourcename = field(sourcefilenames,":",1);
		sourcefilenames = field(sourcefilenames,":",2);
	}

	// / present in source without : treated as if : appended
	if (sourcefilenames.index("/") and not sourcename) {
		sourcename = sourcefilenames;
		sourcefilenames = "";
	}

	//parse target
	targetname = "";
	var targetfilenames = COMMAND.a(3);
	if (index(targetfilenames,":")) {
		targetname = field(targetfilenames,":",1);
		targetfilenames = field(targetfilenames,":",2);
	}

	// / present in target without : treated as if : was appended
	if (targetfilenames.index("/") and not targetname) {
		targetname = targetfilenames;
		targetfilenames = "";
	}

	//connect to source db if source not a path or .sql file
	sourcedb = "";
	if (not sourcename.index("/") and not sourcename.ends(".sql") and not sourcedb.connect(sourcename))
		abort(sourcename.quote() ^ " Cannot connect to source");

	if (targetname.index("/")) {

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
		sourcefilenames.converter(",", FM);

	for (const var& temp : sourcefilenames) {

		sourcefilename = temp;

		//skip dict.all which is an sql view of all dict files
		if (sourcefilename eq "dict.all")
			continue;

		//option to skip all but dict files
		if (sourcedb and dictonly and sourcefilename.substr(1,5) ne "dict.")
			continue;

		// source is a db
		if (sourcedb) {

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
				if (not OPTIONS.index("C") or not targetdb.createfile(targetfilename) or not file2.open(targetfilename, targetdb))
					abort(targetfilename.quote() ^ " cannot be opened in target db " ^ targetname);
			}
		}

		// Target is an os dir. Create it if source is a db and we therefore know the targetfilename.
		else {

			targetfilename = sourcefilename;

			// Duplicate code in main() and getrec()
			targetdir = targetname ^ targetfilename ^ "/";
			osmkdir(targetdir);
		}

		//speed up
		if (targetdb)
			targetdb.begintrans();

		//select source file if source is a db
		if (sourcedb) {
			printl(sourcefilename);
			file1.select(sourcefilename ^ " (R)");
		}

		//process source file
		recn = 0;
		while(getrec()) {

			//user interrupt
			if (not mod(recn,1000)) {
				//print(at(-40), sourcefilename, recn ^ ".", ID);
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
				gosub escape_text(RECORD);
			}
			if (exists) {

				//skip update if no change
				if (RECORD eq oldrec) {
					print("\tNot changed");
					continue;
				}

				printl("\tChanged");
			} else {
				printl("\tNew");
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
			print(at(-40) ^ recn ^ ".", ID);
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
				targetfilename = targetfilename.substr(8);

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

			// Open the target file
			if (targetdb) {
				if (not file2.open(targetfilename, targetdb) )
					abort(targetfilename.quote() ^ " cannot be opened in target db " ^ targetname);
			}

			// Target is an os dir. Create it.
			// Duplicate code in main() and getrec()
			else {
				targetdir = targetname ^ targetfilename ^ "/";
				osmkdir(targetdir);
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
	RECORD.splicer(1,ID.length()+1,"");

	gosub unescape_sql(ID);
	gosub unescape_sql(RECORD);
	//TRACE(ID)

	print(at(-40) ^ recn ^ ".", ID);

	return true;
}

subroutine escape_text(io record) {
	//escape new lines and backslashes
	record.swapper("\\", "\\\\");
	record.swapper("\n", "\\n");

	//replace FM with new lines
	record.converter(FM, "\n");
	return;
}

subroutine unescape_sql(io arg1) {

	if (not arg1.index("\\"))
		return;

	arg1.swapper("\\n","\n");
	arg1.swapper("\\b","\b");
	arg1.swapper("\\t","\t");
	arg1.swapper("\\v","\v");
	arg1.swapper("\\f","\f");
	arg1.swapper("\\r","\r");
	arg1.swapper("\\\\","\\");

	return;

}

programexit()
