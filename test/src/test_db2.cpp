#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>

programinit()

	function main() {

	//Pass if no default database connection
	if (not connect() and osgetenv("EXO_NODATA")) {
		printl("Test passed. Not really. No default db connection to perform db testing.");
		return 0;
	}

	// Check there are no hung transactions from cancelled test runs
	//var sql = "SELECT query,state,backend_start FROM pg_stat_activity WHERE state IN ('active', 'idle in transaction') AND pid <> pg_backend_pid();";
	var sql = "SELECT query,state,backend_start FROM pg_stat_activity WHERE state IN ('idle in transaction') AND pid <> pg_backend_pid();";
	TRACE(sql)
	assert(var().sqlexec(sql, ANS));
	if (ANS) {
		TRACE(ANS.convert(RM, _EOL).convert(FM, "\t"))
		errputl("Checking there are no hung transactions from cancelled test runs. There should be no 'idle in transaction'");
	}

	// Comment this out if you want to see anything in the database after an assert failure
	///////////////////////////////////////////////////////////////////////////////////////
	assert(begintrans());

	//  //Skip if fast testing required
	//	if (osgetenv("EXO_FAST_TEST")) {
	//		printl("EXO_FAST_TEST - skipping test.");
	//		printl("Test passed");
	//		return 0;
	//	}

	var xdataname = "xo_users";
	var xdictname = "dict.xo_users";

	if (deletefile(xdataname))
		logputl(xdataname, "deleted");
	else
		loglasterror();

	if (deletefile(xdictname))
		logputl(xdictname, "deleted");
	else
		loglasterror();

	assert(not listfiles().lower().locate(xdataname));
	assert(not listfiles().lower().locate(xdictname));

	printl("Create " ^ xdataname ^ " and " ^ xdictname);
	assert(createfile(xdataname) or (loglasterror() and false));
	assert(createfile(xdictname) or (loglasterror() and false));

	printl("Open " ^ xdataname ^ " and " ^ xdictname);
	assert(open(xdataname to FILE));
	assert(open(xdictname to DICT));

	printl(R"(
Create some dictionary records (field descriptions)
PERSON_NO    Type 'F', Key Field (0)
BIRTHDAY     Type 'F', Data Field 1
AGE IN DAYS  Type 'S', Source Code needs a dictionary subroutine library called dict_xo_users
AGE IN YEARS Type 'S', Source Code ditto
)");
	assert(write(convert("F|0|Person No||||||R|10", "|", FM), DICT, "PERSON_NO"));
	assert(write(convert("F|1|Birthday||||D||R|12", "|", FM), DICT, "BIRTHDAY"));
	assert(write(convert("S||Age in Days||||||R|10", "|", FM), DICT, "AGE_IN_DAYS"));
	assert(write(convert("S||Age in Years||||||R|10", "|", FM), DICT, "AGE_IN_YEARS"));

	printl("Create some users and their birthdays 11000=11 FEB 1998 .... 14000=30 APR 2006");
	assert(write("11000", FILE, "1"));
	assert(write("12000", FILE, "2"));
	assert(write("13000", FILE, "3"));
	assert(write("14000", FILE, "4"));

	printl("Check via dictionary giving a specific RECORD");
	//DICT.cleardbcache();
	TRACE("cleardbcache();")
	assert(calculate("BIRTHDAY", "dict.xo_users", "1", "99000", 0).outputl() eq 99000);

	printl("Same check using global");
	RECORD = "11000";
	ID	   = "2";
	assert(calculate("BIRTHDAY").outputl() eq 11000);

	//DBTRACE=true;
	//check can create and delete indexes
	//errmsg = {var_mvstr="ERROR:  function exodus.extract_date(bytea, integer, integer, integer) does not exist
	//use DBTRACE to see the error
	printl("CHECKING IF PGEXODUS POSTGRES PLUGIN IS INSTALLED");
	var pluginok = createindex(xdataname, "BIRTHDAY");
	if (not pluginok) {
		printl("Error: pgexodus, Exodus's plugin to PostgreSQL is not working. Run configexodus.");
	} else {

		// Check our index is present in the list
		var tt = listindex();
		tt.dump().outputl();
		TRACE(tt)
		TRACE(tt.oconv("HEX"))
		var tt2 = "xo_users]birthday"_var;
		tt2.dump().outputl();
		TRACE(tt2.oconv("HEX"))
		TRACE(tt2)
		var fn;
		assert(tt.locate(tt2, fn, 0));

		// Check our index is present in the list for our file
		assert(listindex(xdataname)             eq "xo_users]birthday"_var);

		// Check our index is present specifically
		assert(listindex(xdataname, "birthday") eq "xo_users]birthday"_var);

//		TRACE(listindex(xfilename, "birthday"))
//		TRACE(listindex(xfilename, "birthday").oswrite("z2"))
//		TRACE("xo_users]birthday"_var)
//
//		var x = listindex(xfilename, "birthday").oswrite("z2");
//		var y = "xo_users]birthday"_var;
//		assert(x eq y);

		assert(listindex() ne "");
		//ALN: do not delete to make subsequent select work::	assert(deleteindex(xfilename,"BIRTHDAY"));
		//		assert(listindex(xfilename) eq "");
	}

	printl("check can select and readnext through the records");
	printl("Beginning transaction");
	assert(begintrans());
	printl("Begun transaction");
	if (pluginok) {
		assert(select("select xo_users with BIRTHDAY between '1 JAN 2000' and '31 DEC 2003'"));
		assert(readnext(ID));
		assert(ID eq 2);
		assert(readnext(ID));
		assert(ID eq 3);
		assert(not readnext(ID));  //check no more
	}

	printl("Test dictionaries");
	//	if (not select("SELECT xo_users WITH AGE_IN_DAYS GE 0 AND WITH AGE_IN_YEARS GE 0"))
	//	if (not select("SELECT xo_users"))
	if (not select("SELECT xo_users (R)"))
		assert(false and var("Failed to Select xo_users!"));

	//DICT = "dict.xo_users";
	while (readnext(RECORD, ID, MV))
	//	while (readnext(ID))
	{
		printl("ID=" ^ ID ^ " RECORD=" ^ RECORD);

		// test calling dictionary items (only type F so no dict_xxxxx lib calls are done)
		assert(calculate("BIRTHDAY") eq("1" ^ ID ^ "000"));

		//		continue;
		//following requires dict_xo_users to be a dictionary library something like
		//edic dict_xo_users
		/*
#include <exodus/dict.h>

dict(AGE_IN_DAYS) {
	ANS=date()-RECORD(1);
}

dict(AGE_IN_YEARS) {
	ANS=calculate("AGE_IN_DAYS")/365.25;
}
*/
		//		print(" AGE_IN_DAYS=",calculate("AGE_IN_DAYS"));
		//		printl(" AGE_IN_YEARS=",calculate("AGE_IN_YEARS"));
	}

	//#endif

	assert(committrans());

	var filenames  = listfiles();
	var indexnames = listindex("test_people");

	/*
	var().begin();
	var testfile;
	if (!testfile.open("TESTFILE"))
		testfile.createfile("TESTFILE");
	var record1=var("x").str(300);
	var started=var().ostime();
	int nn=10000;
	if (0)
	{
		for (int ii=1;ii<nn;++ii)
		{
			if (!(ii%10000)) cout<<"Written "<<ii<<endl;
			record1.insertrecord(testfile,ii);
		}
	}
	var stopped=var().ostime();
	wcout<<(stopped-started)/nn;

	started=var().ostime();
	for (int ii=1;ii<nn;++ii)
	{
		if (!(ii%10000)) cout<<"Read "<<ii<<endl;
		record1.read(testfile,ii);
	}

	var().end();

	stopped=var().ostime();
	printl((stopped-started)/nn*1000000);

	wcin>>nn;

	//	testfile.deletefile();

	*/

	//	var().connectlocal("");

	var filenames2 = "xo_clients";
	filenames2 ^= FM ^ "xo_currencies";
	filenames2 ^= FM ^ "xo_markets";
	filenames2 ^= FM ^ "xo_ads";

	var tempfile;
	printl();
	var nfiles = fcount(filenames2, FM);
	for (int ii = 1; ii le nfiles; ++ii) {
		var filename = filenames2.f(ii);

		printl(filename);

		if (not open(filename, tempfile)) {
			printl("Creating " ^ filename);
			assert(createfile(filename));
		}

		if (not open("dict." ^ filename.lcase(), tempfile)) {
			printl("Creating dict." ^ filename);
			assert(createfile("dict." ^ filename));
		}
	}
	//	var("x:y:z:").fcount(":").outputl();
	//	var().stop();

	var ads;
	if (!ads.open("xo_ads")) {
		assert(createfile("xo_ads"));
		if (!ads.open("xo_ads"))
			printl("Cannot create xo_ads");
		//abort("Cannot create xo_ads");
	}

	printl("Create Currency dict recs");
	write("F" ^ FM ^ 0 ^ FM ^ "Currency Code" ^ FM ^ FM ^ FM ^ FM ^ FM ^ FM ^ "" ^ "10", "dict.xo_currencies", "CURRENCY_CODE");
	write("F" ^ FM ^ 1 ^ FM ^ "Currency Name" ^ FM ^ FM ^ FM ^ FM ^ FM ^ FM ^ "T" ^ "20", "dict.xo_currencies", "CURRENCY_NAME");

	printl("Create Markets dict recs");
	write("F" ^ FM ^ 1 ^ FM ^ "Market Code" ^ FM ^ FM ^ FM ^ FM ^ FM ^ FM ^ "" ^ "10", "dict.xo_markets", "CODE");
	write("F" ^ FM ^ 1 ^ FM ^ "Market Name" ^ FM ^ FM ^ FM ^ FM ^ FM ^ FM ^ "T" ^ "20", "dict.xo_markets", "NAME");

	printl("Create Ads dict recs");
	var dictrec = "";
	dictrec(1)	= "F";
	dictrec(2)	= "3";
	dictrec(3)	= "Brand Code";
	if (not dictrec.write("dict.xo_ads", "BRAND_CODE"))
		printl("cannot write dict_ads, BRAND_CODE");

	//oo style
	if (not ads.deleteindex("BRAND_CODE")) {}
	assert(ads.createindex("BRAND_CODE"));
	assert(ads.deleteindex("BRAND_CODE"));

	//procedural one arg
	assert(createindex("xo_ads BRAND_CODE"));
	assert(deleteindex("xo_ads BRAND_CODE"));

	//procedural two args
	assert(createindex("xo_ads", "BRAND_CODE"));
	assert(deleteindex("xo_ads", "BRAND_CODE"));

	// Check cannot delete non-existent index
	assert(not deleteindex("xo_ads", "BRAND_CODE"));

	//	var("").select("MARKETS","WITH CURRENCY_NAME = '' AND WITH AUTHORISED");
	//	var("").select("MARKETS","WITH AUTHORISED");
	//	var("").select("xo_ads","WITH AUTHORISED");
	//	ads.select("xo_ads","BY MARKET_CODE WITH MARKET_CODE 'BAH'");
	//	ads.select("xo_ads","BY MARKET_CODE");
	//	var().select("xo_ads");
	//	var("").select("SCHEDULES","WITH AUTHORISED");
	//	var("").select("SCHEDULES","");
	//MvLibs mvlibs;
	var key;
	int ii = 0;
	//	cin>>ii;
	var record;
	assert(begintrans());
	if (ads.select("SELECT xo_ads")) {
		while (ii lt 3 && ads.readnext(record, key, MV)) {
			++ii;
			if (!(ii % 10000))
				printl(" " ^ key);
			if (record.lcase().contains("QWEQWE"))
				print("?");
		}
	}
	clearselect();
	assert(committrans());

	//#ifdef FILE_IO_CACHED_HANDLES_EXCLUDED
	{  // test to reproduce cached_handles error
		var file1("t_FILE1.txt");
		assert(oswrite("", file1));
		var off1 = 0;
		assert(osbwrite("This text is written to the file 't_FILE1.txt'", file1, off1));

		var file2("t_FILE2.txt");
		assert(oswrite("", file2));
		var off2 = 0;
		assert(osbwrite("This text is written to the file 't_FILE2.txt'", file2, off2));

		var file1x = file1;	 // wicked copy of file handle
		file1x.osclose();	 // we could even do: var( file1).osclose();

		var file3("t_FILE3.txt");
		assert(oswrite("", file3));
		var off3 = 0;
		assert(osbwrite("This text is written to the file 't_FILE3.txt'", file3, off3));

		assert(osbwrite("THIS TEXT INTENDED FOR FILE 't_FILE1.txt' BUT IT GOES TO 't_FILE3.txt'", file1, off1));
	}
	//#endif

	var myclients;
	var clients_filename = "xo_clients";
	if (myclients.open(clients_filename)) {

		generate_xo_clients_dict_data(clients_filename);

		printl();
		printl("The following section requires data created by testsort.cpp");

		var key;

		//begintrans();

		if (var().open(clients_filename))
			printl("Could open " ^ clients_filename);
		else
			printl("Could NOT open " ^ clients_filename);

		printl();
		printl("1. test full output with no selection clause or preselect");
		assert(myclients.select());
		while (myclients.readnext(key)) {
			key.outputl("t1 key=");
		}

		printl();
		printl("2. test with default cursor (unassigned var) - select clause needs filename");
		assert(myclients.select("select " ^ clients_filename ^ " with type 'B'"));
		while (myclients.readnext(key)) {
			key.outputl("t2 key=");
		}

		printl();
		printl("3. tests with named cursor (assigned var) - if is file name then select clause can omit filename");
		myclients = clients_filename;
		printl("test preselect affects following select");
		assert(myclients.select("with type 'B'"));
		printl("Normally the following select would output all records but it only shows the preselected ones (2)");
		assert(myclients.select());
		while (myclients.readnext(key)) {
			key.outputl("t3 key=");
		}

		printl();
		printl("4. Normally the following select would output all records but it only shows 3 keys from makelist");
		var keys = "SB1" _FM "JB2" _FM "SB001";
		assert(myclients.makelist("", keys));
		myclients.outputl();
		//      myclients.select();
		//while(myclients.readnext(key)) {
		//	key.outputl("t4 key=");
		//}
		assert(myclients.readnext(key));
		assert(key eq "SB1");
		assert(myclients.readnext(key));
		assert(key eq "JB2");
		assert(myclients.readnext(key));
		assert(key eq "SB001");

		// Should fail if no more records and leave key untouched
		assert(not myclients.readnext(key));
		assert(key eq "SB001");
		//committrans();
		//rollbacktrans();

		assert(write("Client XB1^10000"_var on myclients, "XB1"));
		assert(write("Client XB2^20000"_var on myclients, "XB2"));

		// Check mvprogram's xlate using X and C mode9
		assert(xlate(clients_filename, "XB1", 1, "C")  eq "Client XB1");
		assert(xlate(clients_filename, "XB1x", 1, "X") eq "");
		assert(xlate(clients_filename, "XB1x", 1, "C") eq "XB1x");

		// Return whole record
		assert(xlate(clients_filename, "XB1", "", "X") eq "Client XB1^10000"_var);

		// Return key (doesnt make much sense but could be useful in parameterised systems)
		assert(xlate(clients_filename, "XB1", 0, "X") eq "XB1");

		// var::xlate multivalues using VM
		keys = "XB1]XB2"_var;
		assert(keys.xlate(clients_filename, 1, "X").outputl() eq "Client XB1]Client XB2"_var);

		assert(deleterecord(myclients, "XB1"));
		assert(deleterecord(myclients, "XB2"));
	}

	{
		printl("Create a temporary file (ending _temp)");
		var filename = "xo_test_trans_temp";
		var file;
		assert(not open(filename to file));

		//deletefile(filename); //should never exist since temporary
		assert(createfile(filename));

		printl("Check open/read/write");
		assert(open(filename to file));
		assert(write(1 on file, 1));
		assert(read(RECORD from file, 1));
		assert(RECORD eq 1);

		printl("Start transaction");
		assert(begintrans());

		printl("Check write within transaction can be seen within transaction");
		assert(write(11 on file, 1));
		assert(read(RECORD from file, 1));
		assert(RECORD eq 11);

		printl("test Rollback");
		assert(rollbacktrans());

		printl("Check rollback reverts write");
		assert(read(RECORD from file, 1));
		assert(RECORD eq 1);

		printl("Check temporary files are deleted after closing connection");
		disconnect();
		//disconnectall();
		assert(connect());
		assert(not open(filename to file));
	}

	{

		var filename = "xo_naturals";
		if (not deletefile(filename))
			loglasterror();
		assert(createfile(filename));
		var naturals;
		assert(open(filename, naturals));

		var key = 0;

		var alphanum1 = "Flat 10a";
		var alphanum2 = "Flat 2b";
		//Flat 2b is before Flat 10a
		//assert(naturalorder(alphanum1.toString()) gt naturalorder(alphanum2.toString()));
		assert(write(++key on naturals, alphanum1));
		assert(write(++key on naturals, alphanum2));

		alphanum1 = "Part A-10";
		alphanum2 = "Part A-2";
		//Part A-2 is before Part A-10
		//assert(naturalorder(alphanum1.toString()) gt naturalorder(alphanum2.toString()));
		assert(write(++key on naturals, alphanum1));
		assert(write(++key on naturals, alphanum2));

		alphanum1 = "Xart -10";
		alphanum2 = "Xart -2";
		//Part -10 is before Part -2
		//assert(naturalorder(alphanum1.toString()) lt naturalorder(alphanum2.toString()));
		assert(write(++key on naturals, alphanum1));
		assert(write(++key on naturals, alphanum2));

		alphanum1 = "-10";
		alphanum2 = "-2";
		//-10 is before -2
		//assert(naturalorder(alphanum1.toString()) lt naturalorder(alphanum2.toString()));
		assert(write(++key on naturals, alphanum1));
		assert(write(++key on naturals, alphanum2));

		alphanum1 = "-1.11";
		alphanum2 = "-1.2";
		//-1.2 is before -1.11
		//assert(naturalorder(alphanum1.toString()) gt naturalorder(alphanum2.toString()));
		assert(write(++key on naturals, alphanum1));
		assert(write(++key on naturals, alphanum2));

		alphanum1 = "01.10";
		alphanum2 = "1.1";
		//01.10 is equal to 1.1
		//assert(naturalorder(alphanum1.toString()) eq naturalorder(alphanum2.toString()));
		assert(write(++key on naturals, alphanum1));
		assert(write(++key on naturals, alphanum2));

		alphanum1 = "A B C..C+";
		alphanum2 = "A B C.C";
		//A B C..C+ is before A B C.C
		//assert(naturalorder(alphanum1.toString()) lt naturalorder(alphanum2.toString()));
		assert(write(++key on naturals, alphanum1));
		assert(write(++key on naturals, alphanum2));

		//numbers in quotes?
		alphanum1 = "'2'";
		alphanum2 = "'10bbb22";
		//printl(naturalorder(alphanum1));
		//printl(naturalorder(alphanum2));
		//printl(oconv(naturalorder(alphanum1), "HEX"));
		//printl(oconv(naturalorder(alphanum2), "HEX"));
		//assert(naturalorder(alphanum1.toString()) gt naturalorder(alphanum2.toString()));
		assert(write(++key on naturals, alphanum1));
		assert(write(++key on naturals, alphanum2));

		alphanum1 = "10a";
		alphanum2 = "2b";
		//assert(naturalorder(alphanum1) gt naturalorder(alphanum2));
		assert(write(++key on naturals, alphanum1));
		assert(write(++key on naturals, alphanum2));

		//var sortable = naturalorder("-2 +2 -2.0 +2.0 -10 +10 -10.0 +10.0");
		//TRACE(sortable)
		//assert(sortable eq "4000000000000009c000000000000004000000000000009c000000000000003fdc000000000009c024000000000003fdc000000000009c02400000000000");

		//assert(naturalorder("") eq "");
		//assert(var(naturalorder(" ")).dump() eq " ");
		//assert(var(naturalorder(" abc ")).dump() eq " abc ");

		alphanum1 = " .";
		alphanum2 = "   .";
		assert(write(++key on naturals, alphanum1));
		assert(write(++key on naturals, alphanum2));

		alphanum1 = " abc ";
		alphanum2 = "abc";
		assert(write(++key on naturals, alphanum1));
		assert(write(++key on naturals, alphanum2));

		alphanum1 = " abc";
		alphanum2 = "abc ";
		assert(write(++key on naturals, alphanum1));
		assert(write(++key on naturals, alphanum2));

		alphanum1 = "a";
		alphanum2 = "aa";
		assert(write(++key on naturals, alphanum1));
		assert(write(++key on naturals, alphanum2));

		assert(select("select " ^ filename ^ " by id"));

		// postgres COLLATE exodus_natural does not do a good job of sorting negative numbers but does sort 2 before 10.
		var keys = "";
		while (readnext(ID)) {
			printl(ID.quote());
			keys ^= ID ^ FM;
		}
		keys.popper();
		TRACE(keys)
		keys.converter(FM, "^");
		assert(keys eq "   .^ .^ abc^ abc ^-1.2^-1.11^-2^-10^'2'^'10bbb22^1.1^01.10^2b^10a^a^A B C..C+^A B C.C^aa^abc^abc ^Flat 2b^Flat 10a^Part A-2^Part A-10^Xart -2^Xart -10");

		/*
		"   ."
		" ."
		" abc"
		" abc "
		"-1.2"
		"-1.11"
		"-2"
		"-10"
		"'2'"
		"'10bbb22"
		"1.1"
		"01.10"
		"2b"
		"10a"
		"a"
		"A B C..C+"
		"A B C.C"
		"aa"
		"abc"
		"abc "
		"Flat 2b"
		"Flat 10a"
		"Part A-2"
		"Part A-10"
		"Xart -2"
		"Xart -10"
		*/
		//deletefile(filename);
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

subroutine generate_xo_clients_dict_data(in filename) {

	let dictfilename = "dict." ^ filename;

	// Leave the test data files around for playing with after the program finishes
	//let cleanup = false;

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

	return;
}

programexit()
