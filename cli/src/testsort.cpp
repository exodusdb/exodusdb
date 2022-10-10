#include <exodus/program.h>
programinit()

	// For the sake of multivalue gurus new to exodus programming this is written
	// with multivalue-mimicking "everything is a global function" syntax
	// instead of exodus's oo-style syntax "xxx.yyy().zzz()".
	//
	var filename = "xo_clients";

function main() {

	// No real need for this especially because a) it does a default connection and b) we are not creating a connection for following db operations
	// it is here just to remind that we can connect to specific servers and dataabases if desired
	//if (not connect())
	//	abort("Cannot connect to database. Please check configuration or run configexodus.");

	var dictfilename = "dict." ^ filename;

	// Leave the test data files around for playing with after the program finishes
	var cleanup = false;

	// Always delete and start from scratch (ignore fact that files will not exist the first time)
	//if (cleanup) {
	deletefile(filename);
	deletefile(dictfilename);
	//	}

	printl("\nOpen or create test file ", filename);

	var file;
	if (not open(filename, file)) {
		createfile(filename);
		if (not open(filename, file))
			abort("Cannot create/open " ^ filename);
	}

	printl("\nOpen or create the test files dictionary ", dictfilename);

	var dictfile;
	if (not open(dictfilename, dictfile)) {
		createfile(dictfilename);
		if (not open(dictfilename, dictfile))
			abort("Cannot create/open dictionary " ^ dictfilename);
	}

	printl("\nPrepare some dictionary records");

	
	var dictrecs =
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

		var key = trim(field(dictrec, "|", 1));
		var rec = field(dictrec, "|", 2, 999999);

		printl(key, " : ", rec);

		rec.trimmer();
		rec.replacer(" |", "|");

		rec.converter("|", FM);

		write(rec, dictfile, key);

		// Check we can read the record back
		var rec2;
		if (read(rec2, dictfile, key)) {
			if (rec2 ne rec)
				printl("record differs?!");
		} else
			printl("Cant read ", key, " back");
	}

	var rec;
	if (not read(rec, dictfile, "BALANCE"))
		printl("Cant read 'balance' record from dictionary");

	printl("\nNB 'name_and_type' dictionary item S type calls dict_xo_clients.cpp library function!");

	printl("\nClear the client file");
	clearfile(filename);

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

	let maxrec = 100;

	for (var rec : recs) {
		var key = field(rec, "|", 1);
		rec = field(rec, "|", 2, 9999);
		printl(key, ": ", rec);
		while (index(rec, " |"))
			replacer(rec, " |", "|");
		// OO syntax seems more readable here, so use it.
		//write(trimlast(convert(rec, "|", FM)), file, trim(key));
		rec.convert("|", FM).trimlast().write(file, key.trim());
	}

	var prefix = "select " ^ filename;

	gosub sortselect(file, prefix ^ " by code (R)");

	gosub sortselect(file, prefix ^ " by balance by code (R)");

	gosub sortselect(file, prefix ^ " by timestamp (R)");

	gosub sortselect(file, prefix ^ " with type 'B' by balance (R)");

	var cmd = "list " ^ filename ^ " id-supp";
	printl("\nList the file using ", quote(cmd));
	osshell(cmd);

	cmd = "list " ^ dictfilename;
	printl("\nList the dict using ", quote(cmd));
	osshell(cmd);

	if (cleanup) {
		printl("\nCleaning up. Delete the files");
		deletefile(file);
		deletefile(dictfile);
	}

	printl("\nJust type 'list' to see the syntax of list");
	printl("or list dict." ^ filename ^ " to see the dictionary");
	printl("or just listdict " ^ filename ^ " for the same");
	printl("Type 'edic ~/exodus/cli/src/testsort' to view, edit and recompile this program.");

	return 0;
}

subroutine sortselect(in file, in sortselectcmd) {

	printl("\nsselect the data - ", sortselectcmd);

	//	begintrans();

	if (!select(sortselectcmd)) {
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

	//	rollbacktrans();
}

programexit()
