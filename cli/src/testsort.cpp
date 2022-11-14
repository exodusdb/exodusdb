#include <exodus/program.h>
programinit()

	// For the sake of multivalue gurus new to exodus programming this is written
	// with multivalue-mimicking "everything is a global function" syntax
	// instead of exodus's oo-style syntax "xxx.yyy().zzz()".
	//
	let filename = "xo_clients";

function main() {

	// No real need for this especially because a) it does a default connection and b) we are not creating a connection for following db operations
	// it is here just to remind that we can connect to specific servers and dataabases if desired
	//if (not connect())
	//	abort("Cannot connect to database. Please check configuration or run configexodus.");

	//begintrans();
//	if (not begintrans())
//		abort(lasterror());

	let dictfilename = "dict." ^ filename;

	// Leave the test data files around for playing with after the program finishes
	let cleanup = false;

	// Always delete and start from scratch (ignore fact that files will not exist the first time)
	//if (cleanup) {
	//deletefile(filename);
	if (var().open(filename) and not deletefile(filename)) {
		abort(lasterror());
	}
	//deletefile(dictfilename);
	if (var().open(dictfilename) and not deletefile(dictfilename)) {
		abort(lasterror());
	}

	printl("\nOpen or create test file ", filename);

	var file;
	if (not open(filename, file)) {
		//createfile(filename);
		if (not createfile(filename))
			abort(lasterror());
		if (not open(filename, file))
			abort("Cannot create/open " ^ filename);
	}

	printl("\nOpen or create the test files dictionary ", dictfilename);

	var dictfile;
	if (not open(dictfilename, dictfile)) {
		//createfile(dictfilename);
		if (not createfile(dictfilename)) {
			abort(lasterror());
		}
		if (not open(dictfilename, dictfile))
			abort("Cannot create/open dictionary " ^ dictfilename);
	}

	printl("\nPrepare some dictionary records");


	let dictrecs =
		"CODE         |F|0|Code         ||||          ||L|8"  _FM
		"NAME         |F|1|Name         ||||          ||T|15" _FM
		"TYPE         |F|2|Type         ||||          ||L|5"  _FM
		"DATE_CREATED |F|3|Date Created ||||D4        ||L|12" _FM
		"TIME_CREATED |F|4|Time Created ||||MTH       ||L|12" _FM
		"BALANCE      |F|5|Balance      ||||MD20P     ||R|10" _FM
//		"TIMESTAMP    |F|6|Timestamp    ||||[DATETIME]||L|12"	_FM
		"TIMESTAMP    |F|6|Timestamp    ||||          ||R|12" _FM
		"NAME_AND_TYPE|S| |Name and Type||||          ||L|20" _FM
		"NAME_AND_CODE|S| |Name and Code||||          ||L|20" _FM
		"@CRT         |G| |CODE NAME TYPE NAME_AND_TYPE BALANCE DATE_CREATED TIME_CREATED TIMESTAMP"
	;

	printl("\nWrite the dictionary records to the dictionary");

	// Using modern c++ range based for loop
	for (var dictrec : dictrecs) {

		let key = trim(field(dictrec, "|", 1));
		var rec = field(dictrec, "|", 2, 999999);

		printl(key, " : ", rec);

		rec.trimmer();
		rec.replacer(" |", "|");

		rec.converter("|", FM);

		write(rec, dictfile, key);

		// Check we can read the record back
		var rec2;
		if (read(rec2, dictfile, key)) {
			if (rec2 != rec)
				printl("record differs?!");
		} else
			printl("Cant read ", key, " back");
	}

	var temprec;
	if (not read(temprec, dictfile, "BALANCE"))
		printl("Cant read 'balance' record from dictionary");

	printl("\nNB 'name_and_type' dictionary item S type calls dict_xo_clients.cpp library function!");

	printl("\nClear the client file");
	//clearfile(filename);
	if (var().open(filename) and not clearfile(filename)) {
		abort(lasterror());
	}

	printl("\nPrepare some data records in a readable format");

	var recs = "";
	recs ^= FM ^ "SB001|Client AAA |A |15070|76539|1000.00|15070.76539";
	recs ^= FM ^ "JB002|Client BBB |B |15000|50539|200.00 |15000.50539";
	recs ^= FM ^ "JB001|Client CCC |B |15010|60539|2000.00|15010.60539";
	recs ^= FM ^ "SB1  |Client SB1 |1 |     |     |       |           ";
	recs ^= FM ^ "JB2  |Client JB2 |2 |14000|10539|0      |14000.10539";
	recs ^= FM ^ "JB10 |Client JB10|10|14010|10539|2000.00|14010.10539";
	paster(recs, 1, 1, "");

	printl("\nWrite the data records to the data file");

	//let maxrec = 100;

	for (var rec : recs) {
		let key = field(rec, "|", 1);
		rec = field(rec, "|", 2, 9999);
		printl(key, ": ", rec);
		while (index(rec, " |"))
			replacer(rec, " |", "|");
		// OO syntax seems more readable here, so use it.
		//write(trimlast(convert(rec, "|", FM)), file, trim(key));
		rec.convert("|", FM).trimlast().write(file, key.trim());
	}

	gosub sortselect(file, " by code");

	gosub sortselect(file, " by balance by code");

	gosub sortselect(file, " by timestamp");

	gosub sortselect(file, " with type \"'B'\" by balance");

	var cmd = "list " ^ filename ^ " id-supp";
	printl("\nList the file using ", quote(cmd));
	//perform(cmd);
	if (not osshell(cmd))
		loglasterror();

	cmd = "list " ^ dictfilename;
	printl("\nList the dict using ", quote(cmd));
	//perform(cmd);
	if (not osshell(cmd))
		loglasterror();

	if (cleanup) {
		printl("\nCleaning up. Delete the files");
		//deletefile(file);
		if (var().open(filename) and not deletefile(file)) {
			abort(lasterror());
		}
		//deletefile(dictfile);
		if (var().open(dictfilename) and not deletefile(dictfile)) {
			abort(lasterror());
		}
	}

	//committrans();
	if (not committrans())
		abort(lasterror());

	printl("\nJust type 'list' to see the syntax of list");
	printl("or list dict." ^ filename ^ " to see the dictionary");
	printl("or just listdict " ^ filename ^ " for the same");
	printl("Type 'edic ~/exodus/cli/src/testsort' to view, edit and recompile this program.");

	return 0;
}

subroutine sortselect(in file, in sortselectclause) {

	printl("\nsselect the data - ", sortselectclause);

	if (!select("select xo_clients " ^ sortselectclause)) {
		printl("Cannot sselect in testsort");
		return;
	}

	printl("Read the data");

	var record;
	var key;

	// Could also use the readnextrecord() function here
	// if we had used the new selectrecord() function above
	while (readnext(RECORD, ID, MV)) {

		// No need to read the record if is was obtained in readnext above
		if (not RECORD and not read(RECORD, file, ID)) {
			printl(quote(ID), " missing from file");
			continue;
		}

		printl(ID, ": ", convert(RECORD, FM, "|"));
	}

	let cmd = "list " ^ filename ^ " " ^ sortselectclause ^ " id-supp";
	printl("\nList the file using ", quote(cmd));
	//perform(cmd) or loglasterror("testsort:" ^ cmd);
	if (not osshell(cmd))
		loglasterror();

	//	rollbacktrans();
}

programexit()
