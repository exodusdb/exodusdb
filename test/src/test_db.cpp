#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>
//#include <sstream>

#include <exodus/program.h>

// TESTING

// if it runs ok with no static assertion failures
// 1. compile test_main && test_main

// then test no changes in output with the following
// 2. compile test_main && test_main > t_test_main 2> /dev/null && diff test_main.out t_test_main

// DEVELOPING

// any variable output (including dates/times/random numbers) must use errput (not print or output)
//
// if you add to or change the output in any way then update the expected output file
//
// test_main 1> test_main.out

programinit()

	function main() {

	//Pass if no default database connection
	if (not connect()) {
		printl("No default db connection to perform db testing.");
		printl("Test passed");
		return 0;
	}

	//Skip if fast testing required
	if (osgetenv("EXO_FAST_TEST")) {
		printl("EXO_FAST_TEST - skipping test.");
		printl("Test passed");
		return 0;
	}

	var response;
	assert(var().sqlexec("select exodus_extract_date_array(''||chr(29)||'1',0,0,0);", response));
	TRACE(response.field(RM, 2))
	assert(response.field(RM, 2) eq "{NULL,1968-01-01}");
	var().sqlexec("select exodus_extract_time_array(''||chr(29)||'1'||chr(29)||'86400'||chr(29)||'86401'||chr(29)||'43200',0,0,0);", response);
	printl(response);
	assert(response.field(RM, 2) eq "{NULL,00:00:01,00:00:00,00:00:01,12:00:00}");

	var filename = "xo_test_db_temp1";

	printl("test open/read/write/delete on file 'DOS'");
	var dosfile;
	assert(open("DOS", dosfile));
	assert(open("dos", dosfile));
	assert(write("xyz", dosfile, "t_xyz.txt"));
	assert(read(RECORD, dosfile, "t_xyz.txt"));
	assert(RECORD eq "xyz");
	assert(deleterecord(dosfile, "t_xyz.txt"));
	assert(not read(RECORD, dosfile, "t_xyz.txt"));

	var trec;
	deletefile(filename);
	assert(createfile(filename));

	printl("Test rename, and rename back");
	assert(renamefile(filename, filename ^ "_renamed"));
	assert(renamefile(filename ^ "_renamed", filename));

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
	clearfile(filename);
	assert(filename.reccount() eq 0);

	printl("select list should still be active despite the above");
	assert(LISTACTIVE);

	printl("clearselect should deactivate any active list");
	//assert(clearselect());
	clearselect();
	assert(not LISTACTIVE);

	//check writeo creates a cached record readable by reado but not read
	{
		var k1 = "tempkey";

		//write to cache
		var xyz = "xyz";
		assert(xyz.writeo(filename, k1));

		//ensure wasnt written to real file
		assert(not xyz.read(filename, k1));
		//and ensure failure to read results in unassigned variable
		//assert(unassigned(xyz));
		//ensure failure to read results in no change to record
		//TRACE(xyz)
		//assert(xyz eq "xyz");
		assert(xyz.unassigned());

		//read from cache ok
		assert(xyz.reado(filename, k1));

		//writing empty record to cache is like a deletion
		xyz = "";
		assert(xyz.writeo(filename, k1));
		assert(not xyz.reado(filename, k1));
		//and ensure failure to reado results in unassigned variable
		//assert(unassigned(xyz));
		//ensure failure to read results in no change to record
		//TRACE(xyz)
		//assert(xyz eq "");
		assert(xyz.unassigned());

		//writing to real file also clears cache forcing a real reread
		xyz = "abc";
		assert(xyz.writeo(filename, k1));
		assert(xyz.reado(filename, k1));
		xyz = "123";
		assert(xyz.write(filename, k1));
		assert(xyz.reado(filename, k1));
		assert(xyz eq "123");

		//not testing deleteo because write and delete always call it?

		//deleting a record deletes it from both real and cache file
		assert(deleterecord(filename, k1));
		assert(not xyz.read(filename, k1));
		assert(not xyz.reado(filename, k1));

		//writing empty record to cache is like a deletion
		xyz = "";
		assert(xyz.writeo(filename, k1));
		//deletion from cache
		assert(not xyz.reado(filename, k1));

		assert(not xyz.read(filename, k1));
	}

	{
		printl("Create a temp file");
		var tempfilename = "xo_test_db_deleterecord";
		var tempfile	 = tempfilename;
		deletefile(tempfile);
		if (createfile(tempfile))
			outputl(tempfile);
		for (int i = 1; i <= 10; ++i)
			write(i on tempfile, i);
		tempfile.reccount().outputl("nrecs");

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
		assert(write("aa^bb^cc"_var on tempfile, "X"));

		{
			printl("Update single field");
			assert(writev("22" on tempfile, "X", 2));
			var rec;
			assert(read(rec from tempfile, "X"));
			assert(rec eq "aa^22^cc"_var);

			printl("Check single field updated");
			assert(readv(rec from tempfile, "X", 2));
			assert(rec eq "22"_var);

			printl("Writev on empty record");
			assert(writev("22" on tempfile, "Y", 2));
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
			assert(formlist(tempfile, "ZZZ"));
			assert(LISTACTIVE);

			assert(readnext(ID));
			assert(ID eq "aa");

			assert(readnext(ID));
			assert(ID eq "bb");

			assert(readnext(ID));
			assert(ID eq "cc");

			assert(not readnext(ID));

			printl("Make an active list from a multivalued record field");
			assert(write("aa^bb^c1]c2]c3^dd"_var on tempfile, "ZZZ"));
			assert(formlist(tempfile, "ZZZ", 3));
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
	} catch (VarError e) {
		printl("OK failure to read a record makes the record variable unassigned. For programming safety");
	}

	{
		printl("Check can write a whole dim array to one record");
		dim d1 = {11, 22, 33};
		assert(d1.write(filename, "D1"));
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
		assert(dimwrite(d1 on filename, "D1"));
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
		conn1.connect("");
		conn1.disconnect();
	}
	var dbname2 = "exodus2b";
	var dbname3 = "exodus3b";
	var dbname4 = "exodus4b";
	{
		var conn1;
		assert(conn1.connect(""));	// creates new connection with default parameters (connection string)

		//remove any existing test databases
		for (var dbname : dbname2 ^ FM ^ dbname3 ^ FM ^ dbname4) {
			var conndb2;
			if (conndb2.connect(dbname)) {
				conndb2.disconnect();
				printl("verify delete", dbname);
				printl("======================");
				if (not conn1.dbdelete(dbname))
					outputl(conn1.lasterror());
			}
		}

		TRACE(lasterror());
		assert(lasterror() eq var().lasterror());

		printl("verify CANNOT connect to non-existent deleted database2");
		printl("=======================================================");
		assert(not conn1.connect(dbname2));

		printl("verify CANNOT connect to non-existent deleted database3");
		printl("=======================================================");
		assert(not conn1.connect(dbname3));

		conn1.disconnect();
	}

	{  // Lets test" WITHOUT transaction
		//	global connect/disconnect
		//	locks/unlocks
		//	createfile/deletefile
		connect();	// global connection
		var file = "NANOTABLE";
		deletefile(file);
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
		connect();	// global connection
		begintrans();
		var file = "NANOTABLE";
		deletefile(file);
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
		committrans();

		disconnect();  // global connection
	}

	{
		printl("dbcreate using free function");

		assert(dbcreate(dbname2));
		printl("dblist using free function");

		assert(dblist().convert(FM, VM).locate(dbname2));
		printl("dbcopy using free function");

		assert(dbcopy(dbname2, dbname3));
		printl("dbdelete using free function");

		assert(dbdelete(dbname2));
		printl("dbdelete using free function");

		var tempfilename = "xo_temp_" ^ ospid();
		printl("createfile using free function");
		assert(createfile(tempfilename));

		assert(write("11^22^33"_var on tempfilename, "key1"));

		assert(read(RECORD from tempfilename, "key1"));
		assert(RECORD eq "11^22^33"_var);

		assert(write("11.^22.^33."_var on tempfilename, "key2"));
		assert(reccount(tempfilename) eq 2);

		assert(reado(RECORD from tempfilename, "key2"));
		assert(RECORD                 eq "11.^22.^33."_var);
		assert(reccount(tempfilename) eq 2);

		// Test write to cache but not file
		assert(writeo("111^222^333"_var on tempfilename, "key9"));
		assert(not read(RECORD from tempfilename, "key9"));
		// Read from cache should succeed
		assert(reado(RECORD from tempfilename, "key9"));
		assert(RECORD eq "111^222^333"_var);

		assert(lock(tempfilename, "k1"));
		assert(not lock(tempfilename, "k1"));
		assert(lock(tempfilename, "k2"));
		assert(lock(tempfilename, "k3"));
		unlock(tempfilename, "k1");
		assert(lock(tempfilename, "k1"));
		unlockall();
		assert(lock(tempfilename, "k1"));
		assert(lock(tempfilename, "k2"));
		assert(lock(tempfilename, "k3"));
		unlockall();

		printl("clearfile using free function");
		assert(clearfile(tempfilename));

		assert(reccount(tempfilename) eq 0);

		var tempfilename2 = tempfilename ^ "_2";
		assert(renamefile(tempfilename, tempfilename2));

		{
			assert(begintrans());
			assert(write("xyz" on tempfilename2, "key1"));
			assert(read(RECORD from tempfilename2, "key1"));
			assert(statustrans());
			assert(rollbacktrans());
			assert(not statustrans());
			assert(not read(RECORD from tempfilename2, "key1"));
		}

		{
			assert(begintrans());
			assert(write("xyz" on tempfilename2, "key1"));
			assert(read(RECORD from tempfilename2, "key1"));
			assert(statustrans());
			assert(committrans());
			assert(not statustrans());
			assert(read(RECORD from tempfilename2, "key1"));
		}

		printl("deletefile using free function");
		assert(deletefile(tempfilename2));

		printl("dbdelete using free function");
		assert(dbdelete(dbname3));

		printl("done");
	}

	var table2name = "TABLE2";
	var table3name = "TABLE2";

	{

		printl("create dbs exodus2 and exodus3");
		var conn1;
		assert(conn1.connect(""));
		assert(conn1.dbcreate(dbname2));
		assert(conn1.dbcreate(dbname3));

		// Check dblist
		var dbnames = dblist().convert(FM, VM);
		assert(dbnames.locate(dbname2));
		assert(dbnames.locate(dbname3));

		printl("create table2 on exodus2 and table3 on exodus3 - interleaved");
		var conndb2, conndb3;
		//var table2="TABLE2";
		//var table3="TABLE3";
		var table2 = table2name;
		var table3 = table3name;
		assert(conndb2.connect(dbname2));
		assert(conndb3.connect(dbname3));
		assert(not table2.open(table2name, conndb2));
		assert(not table3.open(table3name, conndb3));
		printl(table2);
		printl(table3);
		assert(conndb2.createfile(table2));
		assert(conndb3.createfile(table3));

		if (table2name ne table3name) {
			assert(not table2.open(table2, conndb3));
			assert(not table3.open(table3, conndb2));
		}

		//check created on the right db
		assert(conndb2.createfile(table2 ^ "_db2"));
		assert(conndb3.createfile(table3 ^ "_db3"));
		var table2x;
		var table3x;
		assert(not table2x.open(table2 ^ "_db2", conndb3));
		assert(not table3x.open(table3 ^ "_db3", conndb2));

		assert(table2.open(table2, conndb2));
		assert(table3.open(table3, conndb3));
		assert(write("2.1111", table2, "2.111"));
		assert(write("3.1111", table3, "3.111"));
		assert(write("2.2222", table2, "2.222"));
		assert(write("3.2222", table3, "3.222"));
		assert(write("2.3333", table2, "2.333"));
		assert(write("3.3333", table3, "3.333"));

		var tt;
		assert(!read(tt, table3, "2.111"));
		assert(!read(tt, table2, "3.111"));

		//test locks are per connection
		assert(conndb2.createfile("TESTLOCKS"));
		assert(conndb3.createfile("TESTLOCKS"));
		var testlocks2;
		var testlocks3;
		assert(testlocks2.open("TESTLOCKS", conndb2));
		assert(testlocks3.open("TESTLOCKS", conndb3));
		assert(testlocks2.lock("123"));
		assert(testlocks3.lock("123"));
		assert(not testlocks2.lock("123"));
		assert(not testlocks3.lock("123"));
		assert(testlocks2.unlock("123"));
		assert(testlocks3.unlock("123"));
		assert(testlocks2.lock("123"));
		assert(testlocks3.lock("123"));

		conndb2.disconnect();
		conndb3.disconnect();
	}
	{
		//test with two connections to the same database that locks work between the two
		var conndb2a, conndb2b;
		assert(conndb2a.connect(dbname2));
		assert(conndb2b.connect(dbname2));
		var table2a, table2b;
		assert(table2a.open(table2name, conndb2a));
		assert(table2b.open(table3name, conndb2b));

		//lock on TABLE2 on db2
		assert(table2a.lock("123X"));

		//cannot repeat lock
		assert(not table2a.lock("123X"));

		//cannot unlock if not locked
		assert(not table2b.unlock("123X"));

		//lock "same" file and key lock on same db DIFFERENT connection
		assert(not table2b.lock("123X"));

		//unlocking on original connection
		assert(table2a.unlock("123X"));

		//should remove lock on the other connection since it is the same database
		assert(table2b.lock("123X"));

		conndb2a.disconnect();
		conndb2b.disconnect();
	}
	//var table2name = "TABLE2";
	//var table3name = "TABLE2";
	{
		printl("Go through table2 in exodus2 db and through table3 in exodus3 db");
		var conndb2, conndb3;
		assert(conndb2.connect(dbname2));
		assert(conndb3.connect(dbname3));
		var table2, table3;
		assert(table2.open(table2name, conndb2));
		assert(table3.open(table3name, conndb3));

		conndb2.begintrans();
		printl("select db2 " ^ table2name);
		table2.select();

		conndb3.begintrans();
		printl("select db3 " ^ table3name);
		table3.select();
		var record2, id2, record3, id3;

		//if (!table2.readnext( record2, id2,MV))
		//	printl("couldnt readnext table2");
		//if (!table3.readnext( record3, id3,MV))
		//	printl("couldnt readnext table23");

		assert(table2.readnext(record2, id2, MV) and table3.readnext(record3, id3, MV));
		assert(record2.outputl() eq "2.1111" and id2.outputl() eq "2.111" and record3.outputl() eq "3.1111" and id3.outputl() eq "3.111");

		assert(table2.readnext(record2, id2, MV) and table3.readnext(record3, id3, MV));
		assert(record2.outputl() eq "2.2222" and id2.outputl() eq "2.222" and record3.outputl() eq "3.2222" and id3.outputl() eq "3.222");

		assert(table2.readnext(record2, id2, MV) and table3.readnext(record3, id3, MV));
		assert(record2 eq "2.3333" and id2 eq "2.333" and record3 eq "3.3333" and id3 eq "3.333");

		assert(not table2.readnext(record2, id2, MV) and not table3.readnext(record3, id3, MV));

		/* rather slow to check so skip
		printl("check CANNOT delete databases while a connection is open");
		//NB try to delete db2 from conndb3 and vice versa
		assert(not conndb3.dbdelete(dbname2));
		assert(not conndb2.dbdelete(dbname3));
*/
		conndb2.committrans();
		conndb3.committrans();

		conndb2.disconnect();
		conndb3.disconnect();

		printl("check copy db exodus2b to exodus4b");
		var conn4;
		assert(conn4.dbcopy(dbname2, dbname4));
		//printl(conn4.getlasterror());
		assert(conn4.connect(dbname4));
		printl("check can open table2 on copied database exodus4");
		var table2b, table3b;
		assert(table2b.open(table2name, conn4));

		///test attach
		{

			var uniquefilename = "TEMP_test_attach";
			var otherdbname	   = dbname4;

			//delete the file first in case already exists
			deletefile(uniquefilename);

			//the file is not available in the default connection
			assert(not open(uniquefilename));

			//open a connection
			var conn1;
			assert(conn1.connect(otherdbname));

			//delete the file first in case already exists
			conn1.deletefile(uniquefilename);

			//create the file
			assert(conn1.createfile(uniquefilename));

			//file exists in connection's listfiles
			assert(conn1.listfiles().contains(uniquefilename.lcase()));

			//file can be opened via the connection
			//assert(open(uniquefilename, conn1));
			assert(conn1.open(uniquefilename, conn1));

			//"detach" the file so that it is not in cache
			conn1.detach(uniquefilename);

			//file is not available without specifying connection
			//without detach above, file handle caching it WOULD find it
			assert(not open(uniquefilename));

			//test "attach"

			//cannot "attach" files that do not exist on the connection
			assert(not conn1.attach("test_db_97876_" ^ uniquefilename));

			// Check "attach" the file via the connection
			assert(conn1.attach(uniquefilename));

			//file can now be opened without specifying the connection
			assert(open(uniquefilename));

			//"detach" the file
			conn1.detach(uniquefilename);

			//the file is no longer available without specifying connection
			assert(not open(uniquefilename));

			conn1.disconnect();
		}

		///test attach "all" dict files
		{

			var uniquefilename = "dict.test_attach";
			var otherdbname	   = dbname4;

			//delete the file first in case already exists
			deletefile(uniquefilename);

			//the file is not available in the default connection
			assert(not open(uniquefilename));

			//open a connection
			var conn1;
			assert(conn1.connect(otherdbname));

			//delete the file first in case already exists
			conn1.deletefile(uniquefilename);

			//create the file
			assert(conn1.createfile(uniquefilename));

			//file exists in connection's listfiles
			assert(conn1.listfiles().contains(uniquefilename.lcase()));

			//file can be opened via the connection
			//assert(open(uniquefilename, conn1));
			assert(conn1.open(uniquefilename, conn1));

			//"detach" the file - so it will be removed from the cache
			conn1.detach(uniquefilename);

			//file is not available without specifying connection
			//without detach above, file handle caching WOULD find it
			assert(not open(uniquefilename));

			//test "attach"

			//"attach" all "dict" file via the connection using "dict"
			assert(conn1.attach("dict"));

			//file can now be opened without specifying the connection
			assert(open(uniquefilename));

			//"detach" the file
			conn1.detach(uniquefilename);

			//the file is no longer available without specifying connection
			assert(not open(uniquefilename));

			conn1.disconnect();
		}

		printl("remove any test databases");
		//connect to exodus first cant delete db if connected to it.
		var conn1;
		assert(conn1.connect("exodus"));
		assert(conn1.dbdelete(dbname2));
		assert(conn1.dbdelete(dbname3));
		conn4.disconnect();
		assert(conn1.dbdelete(dbname4));

		conn1.disconnect();
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
