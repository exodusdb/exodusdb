#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

// TESTING

// if it runs ok with no static assertion failures
// 1. compile test_main && test_main

// then test no changes in output with the following
// 2. compile test_main && test_main gt t_test_main 2> /dev/null && diff test_main.out t_test_main

// DEVELOPING

// any variable output (including dates/times/random numbers) must use errput (not print or output)
//
// if you add to or change the output in any way then update the expected output file
//
// test_main 1> test_main.out

// Create all temporary files ending _temp so that they are deleted on disconnection

	function main() {

	// Quit if no default database connection
	if (not connect()) {
		//Pass if allowed
		if (osgetenv("EXO_NODATA") or true) {
			printx("Test passed. Not really. ");
		}
		printl("No default db connection to perform db testing.");
		return 0;
	}

//	//Skip if fast testing required
//	if (osgetenv("EXO_FAST_TEST")) {
//		printl("EXO_FAST_TEST - skipping test.");
//		printl("Test passed");
//		return 0;
//	}

	var response;
	assert(var().sqlexec("select exodus.extract_date_array(''||chr(29)||'1',0,0,0);", response));
	TRACE(response.field(RM, 2))
	assert(response.field(RM, 2) eq "{NULL,1968-01-01}");
	assert(var().sqlexec("select exodus.extract_time_array(''||chr(29)||'1'||chr(29)||'86400'||chr(29)||'86401'||chr(29)||'43200',0,0,0);", response));
	printl(response);
	assert(response.field(RM, 2) eq "{NULL,00:00:01,00:00:00,00:00:01,12:00:00}");

	var filename = "xo_test_db_temp";

	printl("test open/read/write/delete on file 'DOS'");
	var dosfile;
	assert(open("DOS", dosfile));
	assert(open("dos", dosfile));
//	assert(write("xyz", dosfile, "t_xyz.txt"));
	write("xyz", dosfile, "t_xyz.txt");
	assert(read(RECORD, dosfile, "t_xyz.txt"));
	assert(RECORD eq "xyz");
	assert(deleterecord(dosfile, "t_xyz.txt"));
	assert(not read(RECORD, dosfile, "t_xyz.txt"));

	var trec;
	if (not deletefile(filename)) {}
	assert(createfile(filename));

	printl("Test rename, and rename back");
	let renamed_filename = "renamed_" ^ filename;
	if (not deletefile(renamed_filename)) {}
	assert(renamefile(filename, renamed_filename) or (loglasterror() and false));
	assert(renamefile(renamed_filename, filename) or (loglasterror() and false));

	printl("Test reccount");
	for (var recn : range(1, 10))
		write(recn on filename, recn);
	assert(filename.reccount() eq 10);

	printl("Select 3 records");
	assert(select(filename ^ " 1 2 3"));
	assert(LISTACTIVE);

	printl("savelist");
	var listid = rnd(999999);
	assert(savelist(listid));
	assert(not LISTACTIVE);

	printl("Check lists record");
	var lists = "lists";
	var list;
	assert(read(list from lists, listid));
	assert(list eq "1^2^3"_var);

	printl("Cannot savelist without active list");
	assert(not savelist(listid));

	printl("getlist");
	assert(getlist(listid));
	assert(LISTACTIVE);

	printl("deletelist");
	assert(deletelist(listid));

	printl("select list should still be active despite the above");
	assert(LISTACTIVE);

	printl("deletelist if doesnt exist is not an error");
	assert(deletelist(listid));

	printl("select list should still be active despite the above");
	assert(LISTACTIVE);

	printl("Test clearfile");
	assert(clearfile(filename));
	assert(filename.reccount() eq 0);

	printl("select list should still be active despite the above");
	assert(LISTACTIVE);

	printl("clearselect should deactivate any active list");
	//assert(clearselect());
	clearselect();
	assert(not LISTACTIVE);

	//check writec creates a cached record readable by readc but not read
	{
		var k1 = "tempkey";

		//write to cache
		var xyz = "xyz";
//		assert(xyz.writec(filename, k1));
		xyz.writec(filename, k1);

		//ensure wasnt written to real file
		assert(not xyz.read(filename, k1));
		//and ensure failure to read results in unassigned variable
		//assert(unassigned(xyz));
		//ensure failure to read results in no change to record
		//TRACE(xyz)
		//assert(xyz eq "xyz");
		assert(xyz.unassigned());

		//read from cache ok
		assert(xyz.readc(filename, k1));

		//writing empty record to cache is like a deletion
		xyz = "";
//		assert(xyz.writec(filename, k1));
		xyz.writec(filename, k1);
		assert(not xyz.readc(filename, k1));
		//and ensure failure to readc results in unassigned variable
		//assert(unassigned(xyz));
		//ensure failure to read results in no change to record
		//TRACE(xyz)
		//assert(xyz eq "");
		assert(xyz.unassigned());

		//writing to real file also clears cache forcing a real reread
		xyz = "abc";
//		assert(xyz.writec(filename, k1));
		xyz.writec(filename, k1);
		assert(xyz.readc(filename, k1));
		xyz = "123";
//		assert(xyz.write(filename, k1));
		xyz.write(filename, k1);
		assert(xyz.readc(filename, k1));
		assert(xyz eq "123");

		//not testing deletec because write and delete always call it?

		//deleting a record deletes it from both real and cache file
		assert(deleterecord(filename, k1));
		assert(not xyz.read(filename, k1));
		assert(not xyz.readc(filename, k1));

		//writing empty record to cache is like a deletion
		xyz = "";
//		assert(xyz.writec(filename, k1));
		xyz.writec(filename, k1);
		//deletion from cache
		assert(not xyz.readc(filename, k1));

		assert(not xyz.read(filename, k1));
	}

	{
		printl("Create a temp file");
		var tempfilename = "xo_test_db_deleterecord_temp";
		var tempfile	 = tempfilename;
		if (not deletefile(tempfile)) {}
		assert(createfile(tempfile));
		assert(not read(RECORD from tempfile, "%RECORDS%"));
		for (int i = 1; i le 10; ++i) {
			//assert(write(i on tempfile, i));
//			assert(write(var(i) on tempfile, var(i)));
			write(var(i) on tempfile, var(i));
		}

		// Check reading magic key %RECORDS% returns all keys in natural order
		assert(read(RECORD from tempfile, "%RECORDS%"));
		TRACE(RECORD)
		assert(RECORD eq "1^2^3^4^5^6^7^8^9^10"_var);

		printl("Check deleterecord works on a select list");
		//		select(tempfile ^ " with ID ge 5");
		select(tempfilename ^ " with ID ge '5'");
		deleterecord(tempfile);
		assert(tempfile.reccount() eq 5);

		printl("Check deleterecord works command style list of keys");
		deleterecord(tempfilename ^ " '1' 2 \"3\"");
		assert(tempfile.reccount() eq 2);

		printl("Check deleterecord one key using 2nd parameter works");
		deleterecord(tempfilename, "10");
		assert(tempfile.reccount() eq 1);

		printl("Prepare a test record");
//		assert(write("aa^bb^cc"_var on tempfile, "X"));
		write("aa^bb^cc"_var on tempfile, "X");

		{
			printl("Update single field");
//			assert(writef("22" on tempfile, "X", 2));
			writef("22" on tempfile, "X", 2);
			var rec;
			assert(read(rec from tempfile, "X"));
			assert(rec eq "aa^22^cc"_var);

			printl("Check single field updated");
			assert(readf(rec from tempfile, "X", 2));
			assert(rec eq "22"_var);

			printl("writef on empty record");
//			assert(writef("22" on tempfile, "Y", 2));
			writef("22" on tempfile, "Y", 2);
			assert(read(rec from tempfile, "Y"));
			assert(rec eq "^22"_var);

			printl("Check that insertrecord works only if record DOESNT exist");
			assert(insertrecord("11^22^33"_var on tempfile, "ZZZ"));
			assert(read(rec from tempfile, "ZZZ"));
			assert(rec eq "11^22^33"_var);
			//
			assert(not insertrecord("aa^bb^cc" on tempfile, "ZZZ"));
			assert(read(rec from tempfile, "ZZZ"));
			assert(rec eq "11^22^33"_var);

			printl("Check that updaterecord works only if record DOES exist");
			assert(not updaterecord("aa^bb^cc"_var on tempfile, "QQQ"));
			assert(not read(rec from tempfile, "QQQ"));
			//
			assert(updaterecord("aa^bb^cc"_var on tempfile, "ZZZ"));
			assert(read(rec from tempfile, "ZZZ"));
			assert(rec eq "aa^bb^cc"_var);

			printl("Make an active list from a record");
//			assert(formlist(tempfile, "ZZZ"));
			assert(selectkeys(rec));
			assert(LISTACTIVE);

			assert(readnext(ID));
			assert(ID eq "aa");

			assert(readnext(ID));
			assert(ID eq "bb");

			assert(readnext(ID));
			assert(ID eq "cc");

			assert(not readnext(ID));

			printl("Make an active list from a multivalued record field");
//			assert(write("aa^bb^c1]c2]c3^dd"_var on tempfile, "ZZZ"));
//			write("aa^bb^c1]c2]c3^dd"_var on tempfile, "ZZZ");
			selectkeys("aa^bb^c1]c2]c3^dd"_var.f(3).raise());
//			assert(formlist(tempfile, "ZZZ", 3));
			assert(LISTACTIVE);

			assert(readnext(ID));
			assert(ID eq "c1");

			assert(readnext(ID));
			assert(ID eq "c2");

			assert(readnext(ID));
			assert(ID eq "c3");

			assert(not readnext(ID));
		}
	}

	//NFD - Decomposed Normal Form                                                                     │
	//NFC - Compact Normal Form                                                                        │
	var decomp_a  = "\x61\xCC\x81";	 //"á";                                                                 │
	var compact_a = "\xC3\xA1";		 //"á";

	//test write decomp can be read by compact and deleted by compact
	write("temp", filename, decomp_a);
	assert(read(trec, filename, compact_a));
	assert(trec eq "temp");
	deleterecord(filename, compact_a);
	assert(not read(trec, filename, compact_a));

	//test write compact can be read by decomp and deleted by decomp
	write("temp", filename, compact_a);
	trec = "";
	assert(read(trec, filename, decomp_a));

	// Check failure to read a record turns the record variable into unassigned and unusable
	// unlike pickos which leaves the variable untouched.
	var rec = "abc";
	assert(not rec.read("dict.voc", "LJLKJLKJLKJLKJLKJw"));
	try {
		printl(rec.quote());
		assert(false);
	} catch (VarError&) {
		printl("OK failure to read a record makes the record variable unassigned. For programming safety");
	}

	{
		printl("Check can write a whole dim array to one record");
		dim d1 = {11, 22, 33};
//		assert(d1.write(filename, "D1"));
		d1.write(filename, "D1");
		var rec;

		printl("Verify record back into a var");
		assert(read(rec from filename, "D1"));
		assert(rec eq "11^22^33"_var);

		printl("Verify record back into a dim");
		dim d2;
		assert(d2.read(filename, "D1"));
		assert(d2.join() eq d1.join());
	}

	{
		// free functions

		printl("Check can write a whole dim array to one record");
		dim d1 = {11, 22, 33};
//		assert(dimwrite(d1 on filename, "D1"));
		dimwrite(d1 on filename, "D1");
		var rec;

		printl("Verify record back into a var");
		assert(read(rec from filename, "D1"));
		assert(rec eq "11^22^33"_var);

		printl("Verify record back into a dim");
		dim d2;
		assert(dimread(d2 from filename, "D1"));
		assert(d2.join() eq d1.join());
	}

	{
		var conn1;
		//connect normally doesnt touch default connection
		//!!! EXCEPT if default connection is empty then it sets it
		//default connection is empty to start with
		//OR if you disconnect a connection with the same number (ie the same connection)
		assert(conn1.connect(""));
	}  // one connection is lost here (hangs)

	{
		var conn1;
		assert(conn1.connect(""));
		conn1.disconnect();
	}

	{  // Lets test" WITHOUT transaction
		//	global connect/disconnect
		//	locks/unlocks
		//	createfile/deletefile
		assert(connect());	// global connection
		var file = "NANOTABLE_temp";
		if (not deletefile(file)) {}
		assert(createfile(file));

		//check 1 can be locked and not relocked
		assert(file.lock("1"));
		assert(not file.lock("1"));

		//check 2 can be locked and not relocked
		assert(file.lock("2"));
		assert(not file.lock("2"));

		//check 1 is still locked and can be unlocked and relocked
		assert(not file.lock("1"));
		assert(not file.statustrans());
		assert(file.unlock("1"));
		assert(file.lock("1"));

		//check 2 is still locked and can be unlocked and relocked
		assert(not file.lock("2"));
		assert(file.unlock("2"));
		assert(file.lock("2"));

		//check unlockall
		//Note: this unlocks locks on ALL FILES locks on the same connection
		//NOT just the locks on the file in question
		//TODO make it work per file and per connection?
		assert(file.unlockall());
		assert(file.lock("1"));
		assert(file.lock("2"));

		assert(deletefile(file));

		disconnect();  // global connection
	}

	{  // Lets test" WITH trsnsaction
		//	global connect/disconnect
		//	locks/unlocks
		//	createfile/deletefile
		assert(connect());	// global connection
		assert(begintrans());
		var file = "NANOTABLE";
		if (not deletefile(file)) {}
		assert(createfile(file));

		//check 1 can be locked and CAN BE relocked since in a transaction
		assert(file.lock("1") eq 1);
		assert(file.lock("1") eq 2);

		//check 2 can be locked and CAN BE relocked since in a transaction
		assert(file.lock("2") eq 1);
		assert(file.lock("2") eq 2);

		//check 1 is still locked
		assert(file.lock("1"));
		assert(file.statustrans());
		assert(not file.unlock("1"));  // check CANNOT unlock inside transactions
		assert(file.lock("1"));

		//check 2 is still locked and CANNOT be unlocked inside transactions
		assert(file.lock("2"));
		assert(not file.unlock("2"));  //CANNOT unlock unside trsnsactions
		assert(file.lock("2"));

		//check unlockall
		//Note: this unlocks locks on ALL FILES locks on the same connection
		//NOT just the locks on the file in question
		//TODO make it work per file and per connection?
		assert(not file.unlockall());  //should do nothing inside transactions
		assert(file.lock("1"));
		assert(file.lock("2"));

		assert(deletefile(file));
		assert(committrans());

		disconnect();  // global connection
	}

	{
		let db1x = "test_db1x";
		let db2x = "test_db2x";
		let fname = "dbf1";

		// Preclean 1
		auto _ = dbdelete(db1x);
		assert(not dblist().locateusing(FM, db1x));

		// Preclean 2
		auto _ = dbdelete(db2x);
		assert(not dblist().locateusing(FM, db2x));

		// dbcreate 1 and connect
		assert(dbcreate(db1x));
		assert(not dbcreate(db1x));
		assert(dblist().locateusing(FM, db1x));
		var conn1;
		assert(conn1.connect(db1x));

		// Create a file on 1 and disconnect
		assert(conn1.createfile(fname));
		assert(not conn1.createfile(fname));
		assert(conn1.listfiles().locateusing(FM, fname));
		conn1.disconnect();

		// dbcopy 1 to 2 and connect to 2
		assert(dbcopy(db1x, db2x));
		assert(dblist().locateusing(FM, db2x));

		// Check the file exists on 2
		{
			var conn2;
			assert(conn2.connect(db2x));
			assert(conn2.listfiles().locateusing(FM, fname));
			var file;
			assert(file.open(fname, conn2));

			// Rename file on 2
			let fname2 = fname ^ "_2";
			assert(conn2.renamefile(fname, fname2));
			assert(conn2.listfiles().locateusing(FM, fname2));
			assert(not conn2.listfiles().locateusing(FM, fname));
			assert(not conn2.deletefile(fname));

			// Delete the renamed file on 2 and disconnect
			assert(conn2.deletefile(fname2));
			assert(not conn2.listfiles().locateusing(FM, fname2));
			conn2.disconnect();
		}

		// Connect to 1, delete the file and disconnect
		{
			var conn1;
			assert(conn1.connect(db1x));
			assert(conn1.deletefile(fname));
			conn1.disconnect();
		}

		// Delete 2
		assert(dbdelete(db2x));
		assert(not dbdelete(db2x));

		// Verify to is required in dbcopy
		assert(not dbcopy(db1x, ""));

		// Create 2 from 1 using 2 argument version of dbcreate
		assert(dbcreate(db2x, db1x));
		assert(dblist().locateusing(FM, db2x));

		// Delete 2
		assert(dbdelete(db2x));
		assert(not dbdelete(db2x));

		// Delete 1
		assert(dbdelete(db1x));
		assert(not dbdelete(db1x));

	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
