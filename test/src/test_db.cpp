#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>
#include <sstream>

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

function main()
{

	//Pass if no default database connection
	if (not connect()) {
		printl("No default db connection to perform db testing. Test passed");
		return 0;
	}

	var response;
	var().sqlexec("select exodus_extract_date_array(''||chr(29)||'1',0,0,0);",response);
	assert(response.field(RM,2)=="{NULL,1968-01-01}");
	var().sqlexec("select exodus_extract_time_array(''||chr(29)||'1'||chr(29)||'86400'||chr(29)||'86401'||chr(29)||'43200',0,0,0);",response);
	assert(response.field(RM,2)=="{NULL,00:00:01,00:00:00,00:00:01,12:00:00}");

	var filename="test_main_temp1";

	var trec;
	deletefile(filename);
	assert(createfile(filename));

    //check writeo creates a cached record readable by reado but not read
	{
		var k1="tempkey";

		//write to cache
		var xyz="xyz";
		assert(xyz.writeo(filename,k1));

		//ensure wasnt written to real file
		assert(not xyz.read(filename,k1));
		//and ensure failure to read results in unassigned variable
		//assert(unassigned(xyz));
		//ensure failure to read results in no change to record
		TRACE(xyz)
		assert(xyz == "xyz");

		//read from cache ok
		assert(xyz.reado(filename,k1));

		//writing empty record to cache is like a deletion
		xyz="";
		assert(xyz.writeo(filename,k1));
		assert(not xyz.reado(filename,k1));
		//and ensure failure to reado results in unassigned variable
		//assert(unassigned(xyz));
		//ensure failure to read results in no change to record
		TRACE(xyz)
		assert(xyz == "");

		//writing to real file also clears cache forcing a real reread
		xyz="abc";
		assert(xyz.writeo(filename,k1));
		assert(xyz.reado(filename,k1));
		xyz="123";
		assert(xyz.write(filename,k1));
		assert(xyz.reado(filename,k1));
		assert(xyz == "123");

		//deleting a record deletes it from both real and cache file
		assert(deleterecord(filename,k1));
		assert(not xyz.read(filename,k1));
		assert(not xyz.reado(filename,k1));

		//writing empty record to cache is like a deletion
		xyz="";
		assert(xyz.writeo(filename,k1));
		//deletion from cache
		assert(not xyz.reado(filename,k1));

		assert(not xyz.read(filename,k1));
	}

	{
		printl("Create a temp file");
		var tempfilename="test_deleterecord";
		var tempfile=tempfilename;
		deletefile(tempfile);
		if (createfile(tempfile))
			outputl(tempfile);
		for (int i=1;i<=10;++i)
			write(i on tempfile, i);
		tempfile.reccount().outputl("nrecs");

		printl("Check deleterecord works on a select list");
//		select(tempfile ^ " with ID ge 5");
		select(tempfile ^ " with ID ge '5'");
		deleterecord(tempfile);
		assert(tempfile.reccount() == 5);

		printl("Check deleterecord works command style list of keys");
		deleterecord(tempfilename ^ " '1' 2 \"3\"");
		assert(tempfile.reccount() == 2);

		printl("Check deleterecord one key using 2nd parameter works");
		deleterecord(tempfilename, "10");
		assert(tempfile.reccount() == 1);
	}

    //NFD - Decomposed Normal Form                                                                     │
    //NFC - Compact Normal Form                                                                        │
    var decomp_a="\x61\xCC\x81";//"á";                                                                 │
    var compact_a="\xC3\xA1";//"á";

    //test write decomp can be read by compact and deleted by compact
	write("temp",filename,decomp_a);
	assert(read(trec,filename,compact_a));
	assert(trec=="temp");
	deleterecord(filename,compact_a);
	assert(not read(trec,filename,compact_a));

	//test write compact can be read by decomp and deleted by decomp
	write("temp",filename,compact_a);
	trec="";
	assert(read(trec,filename,decomp_a));

    //check failure to read a record turns the record variable into unassigned and unusable
    var rec="abc";
    assert( not rec.read("dict_voc","LJLKJLKJLKJLKJLKJw"));
    try
    {
        printl(rec.quote());
    }
    catch (MVError e) {
        printl("OK failure to read a record makes the record variable unassigned. For programming safety");
    }

#define MULTIPLE_CONNECTION_CODE_EXCLUDED
#ifdef MULTIPLE_CONNECTION_CODE_EXCLUDED
	{
		var conn1;
		//connect normally doesnt touch default connection
		//!!! EXCEPT if default connection is empty then it sets it
		//default connection is empty to start with
		//OR if you disconnect a connection with the same number (ie the same connection)
		assert(conn1.connect( ""));
	}		// one connection is lost here (hangs)

	{
		var conn1;
		conn1.connect( "");
		conn1.disconnect();
	}
	var dbname2="exodus2b";
	var dbname3="exodus3b";
	var dbname4="exodus4b";
	{
		var conn1;
		conn1.connect( "");			// creates new connection with default parameters (connection string)

		//remove any existing test databases
		conn1.deletedb( dbname2);
		conn1.deletedb( dbname3);
		conn1.deletedb( dbname4);

		printl("verify CANNOT connect to non-existent deleted database2");
		printl("=======================================================");
		assert(not conn1.connect(dbname2));

		printl("verify CANNOT connect to non-existent deleted database3");
		printl("=======================================================");
		assert(not conn1.connect(dbname3));

		conn1.disconnect();

	}

	{	// Lets test"
		//	global connect/disconnect
		//	locks/unlocks
		//	createfile/deletefile
		connect();			// global connection
		var file = "NANOTABLE";
		deletefile(file);
		assert( createfile( file));
		file.lock( "1");
		file.lock( "2");
		file.unlock( "2");
		file.unlock( "1");
		assert( deletefile( file));
		disconnect();		// global connection
	}

	{
		printl("create dbs exodus2 and exodus3");
		var conn1;
		assert(conn1.connect(""));
		assert(conn1.createdb(dbname2));
		assert(conn1.createdb(dbname3));

		printl("create table2 on exodus2 and table3 on exodus3 - interleaved");
		var conn2,conn3;
		var table2="TABLE2";
		var table3="TABLE3";
		assert(conn2.connect(dbname2));
		assert(conn3.connect(dbname3));
		assert(not table2.open("TABLE2",conn2));
		assert(not table3.open("TABLE3",conn3));
		printl(table2);
		printl(table3);
		assert(conn2.createfile(table2));
		assert(conn3.createfile(table3));
		assert(not table2.open(table2,conn3));
		assert(not table3.open(table3,conn2));

		assert(table2.open(table2,conn2));
		assert(table3.open(table3,conn3));
		assert(write( "2.1111", table2, "2.111"));
		assert(write( "3.1111", table3, "3.111"));
		assert(write( "2.2222", table2, "2.222"));
		assert(write( "3.2222", table3, "3.222"));
		assert(write( "2.3333", table2, "2.333"));
		assert(write( "3.3333", table3, "3.333"));

		var tt;
		assert(! read( tt, table3, "2.111"));
		assert(! read( tt, table2, "3.111"));

		assert(conn2.disconnect());
		assert(conn3.disconnect());
	}
	{
		printl("Go through table2 in exodus2 db and through table3 in exodus3 db");
		var conn2, conn3;
		assert(conn2.connect(dbname2));
		assert(conn3.connect(dbname3));
		var table2,table3;
		assert( table2.open("TABLE2",conn2));
		assert( table3.open("TABLE3",conn3));

		conn2.begintrans();
		printl("select table2");
		table2.select();

		conn3.begintrans();
		printl("select table3");
		table3.select();
		var record2, id2, record3, id3;

		//if (!table2.readnext( record2, id2,MV))
		//	printl("couldnt readnext table2");
		//if (!table3.readnext( record3, id3,MV))
		//	printl("couldnt readnext table23");

		assert(table2.readnext( record2, id2, MV) and table3.readnext( record3, id3, MV));
		assert(record2 eq "2.1111" and id2 eq "2.111" and record3 eq "3.1111" and id3 eq "3.111");

		assert(table2.readnext( record2, id2, MV) and table3.readnext( record3, id3, MV));
		assert(record2 eq "2.2222" and id2 eq "2.222" and record3 eq "3.2222" and id3 eq "3.222");

		assert(table2.readnext( record2, id2, MV) and table3.readnext( record3, id3, MV));
		assert(record2 eq "2.3333" and id2 eq "2.333" and record3 eq "3.3333" and id3 eq "3.333");

		assert(not table2.readnext( record2, id2, MV) and not table3.readnext( record3, id3, MV));

/* rather slow to check so skip
		printl("check CANNOT delete databases while a connection is open");
		//NB try to delete db2 from conn3 and vice versa
		assert(not conn3.deletedb(dbname2));
		assert(not conn2.deletedb(dbname3));
*/
		conn2.committrans();
		conn3.committrans();

		conn2.disconnect();
		conn3.disconnect();

		printl("check copy db exodus2b to exodus4b");
		var conn4;
		assert(conn4.copydb(dbname2,dbname4));
		//printl(conn4.getlasterror());
		assert(conn4.connect( dbname4));
		printl("check can open table2 on copied database exodus4");
		var table2b,table3b;
		assert( table2b.open( "TABLE2",conn4));

		printl("remove any test databases");
		//connect to exodus first cant delete db if connected to it.
		var conn1;
		assert(conn1.connect("exodus"));
		assert(conn1.deletedb(dbname2));
		assert(conn1.deletedb(dbname3));
		conn4.disconnect();
		assert(conn1.deletedb(dbname4));

		conn1.disconnect();
	}
#endif

//#if 0

	deletefile("XUSERS");
	deletefile("dict_XUSERS");

	createfile("XUSERS");
	createfile("DICT_XUSERS");

	//create some dictionary records (field descriptions)
	//PERSON_NO    Type "F", Key Field (0)
	//BIRTHDAY     Type "F", Data Field 1
	//AGE IN DAYS  Type "S", Source Code needs a dictionary subroutine library called dict_XUSERS
	//AGE IN YEARS Type "S", Source Code ditto
	assert(write(convert( "F|0|Person No||||||R|10"   ,"|",FM),"DICT_XUSERS", "PERSON_NO"   ));
	assert(write(convert( "F|1|Birthday||||D||R|12"   ,"|",FM),"DICT_XUSERS", "BIRTHDAY"    ));
	assert(write(convert( "S||Age in Days||||||R|10"  ,"|",FM),"DICT_XUSERS", "AGE_IN_DAYS" ));
	assert(write(convert( "S||Age in Years||||||R|10" ,"|",FM),"DICT_XUSERS", "AGE_IN_YEARS"));

	//create some users and their birthdays 11000=11 FEB 1998 .... 14000=30 APR 2006
	assert(write("11000","XUSERS","1"));
	assert(write("12000","XUSERS","2"));
	assert(write("13000","XUSERS","3"));
	assert(write("14000","XUSERS","4"));

//DBTRACE=true;
	//check can create and delete indexes
	//errmsg = {var_mvstr="ERROR:  function exodus_extract_date(bytea, integer, integer, integer) does not exist
	//use DBTRACE to see the error
	printl("CHECKING IF PGEXODUS POSTGRES PLUGIN IS INSTALLED");
	var pluginok=true;
	if (not createindex("XUSERS","BIRTHDAY")) {
		pluginok=false;
		printl("Error: pgexodus, Exodus's plugin to PostgreSQL is not working. Run configexodus.");
	}

	if (pluginok) {
		assert(listindexes("XUSERS") eq ("xusers" _VM_ "birthday"));
		assert(listindexes() ne "");
//ALN: do not delete to make subsequent select work::	assert(deleteindex("XUSERS","BIRTHDAY"));
//		assert(listindexes("XUSERS") eq "");
	}
	//check can select and readnext through the records
	printl("Beginning transaction");
	assert(begintrans());
	printl("Begun transaction");
	if (pluginok) {
		assert(select("select XUSERS with BIRTHDAY between '1 JAN 2000' and '31 DEC 2003'"));
		assert(readnext(ID));
		assert(ID eq 2);
		assert(readnext(ID));
		assert(ID eq 3);
		assert(not readnext(ID));//check no more
	}
//test function dictionaries
//	if (not select("SELECT XUSERS WITH AGE_IN_DAYS GE 0 AND WITH AGE_IN_YEARS GE 0"))
//	if (not select("SELECT XUSERS"))
	if (not select("SELECT XUSERS (R)"))
		assert(false and var("Failed to Select XUSERS!"));

	DICT="dict_XUSERS";
	while (readnext(RECORD,ID,MV))
//	while (readnext(ID))
	{
		printl("ID=" ^ ID ^ " RECORD=" ^ RECORD);

//		continue;
//following requires dict_XUSERS to be a dictionary library something like
//edic dict_XUSERS
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

	committrans();

	var filenames=listfiles();
	var indexnames=listindexes("test_people");

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

	var filenames2="MY_JOBS";
	filenames2^=FM^"MY_PRODUCTION_ORDERS";
	filenames2^=FM^"MY_PRODUCTION_INVOICES";
	filenames2^=FM^"MY_COMPANIES";
	filenames2^=FM^"MY_BRANDS";
	filenames2^=FM^"MY_CLIENTS";
	filenames2^=FM^"MY_VEHICLES";
	filenames2^=FM^"MY_SUPPLIERS";
	filenames2^=FM^"MY_CURRENCIES";
	filenames2^=FM^"MY_MARKETS";
	filenames2^=FM^"MY_ADS";

	var tempfile;
	printl();
	var nfiles=dcount(filenames2,FM);
	for (int ii=1;ii<=nfiles;++ii) {
		var filename=filenames2.a(ii);

		printl(filename);

		if (not open(filename, tempfile)) {
			printl("creating "^filename);
			assert(createfile(filename));
		}

		if (not open("dict_"^filename.lcase(), tempfile)) {
			printl("creating dict_"^filename);
			assert(createfile("dict_"^filename));
		}

	}
//	var("x:y:z:").dcount(":").outputl();
//	var().stop();

	var ads;
	if (!ads.open("MY_ADS"))
	{
		var().createfile("MY_ADS");
		if (!ads.open("MY_ADS"))
			printl("Cannot create MY_ADS");
			//abort("Cannot create MY_ADS");
	}

	write("F"^FM^0^FM^"Currency Code"^FM^FM^FM^FM^FM^FM^""^"10","DICT_MY_CURRENCIES","CURRENCY_CODE");
	write("F"^FM^1^FM^"Currency Name"^FM^FM^FM^FM^FM^FM^"T"^"20","DICT_MY_CURRENCIES","CURRENCY_NAME");
	write("F"^FM^1^FM^"Market Code"^FM^FM^FM^FM^FM^FM^""^"10","DICT_MY_MARKETS","CODE");
	write("F"^FM^1^FM^"Market Name"^FM^FM^FM^FM^FM^FM^"T"^"20","DICT_MY_MARKETS","NAME");

	var dictrec="";
	dictrec.r(1,"F");
	dictrec.r(2,"3");
	dictrec.r(3,"Brand Code");
	if (not dictrec.write("DICT_MY_ADS","BRAND_CODE"))
		printl("cannot write dict_ads, BRAND_CODE");
	//oo style
	assert(ads.createindex("BRAND_CODE"));
	assert(ads.deleteindex("BRAND_CODE"));
	//procedural
	assert(createindex("MY_ADS BRAND_CODE"));
	assert(deleteindex("MY_ADS BRAND_CODE"));

//	var("").select("MARKETS","WITH CURRENCY_NAME = '' AND WITH AUTHORISED");
//	var("").select("MARKETS","WITH AUTHORISED");
//	var("").select("MY_ADS","WITH AUTHORISED");
//	ads.select("MY_ADS","BY MARKET_CODE WITH MARKET_CODE 'BAH'");
//	ads.select("MY_ADS","BY MARKET_CODE");
//	var().select("MY_ADS");
//	var("").select("SCHEDULES","WITH AUTHORISED");
//	var("").select("SCHEDULES","");
	//MvLibs mvlibs;
	var key;
	int ii=0;
//	cin>>ii;
	var record;
	begintrans();
	if (ads.select("SELECT MY_ADS")) {
		while (ii<3&&ads.readnext(record,key,MV))
		{
			++ii;
			if (!(ii%10000))
				printl(" " ^ key);
			if (record.lcase().index("QWEQWE"))
				print("?");

		}
	}
	clearselect();
	committrans();
#ifdef FILE_IO_CACHED_HANDLES_EXCLUDED
	{	// test to reproduce cached_handles error
		var file1( "t_FILE1.txt");
		oswrite( "", file1);
		var off1 = 0;
		osbwrite( "This text is written to the file 't_FILE1.txt'", file1, off1);

		var file2( "t_FILE2.txt");
		oswrite( "", file2);
		var off2 = 0;
		osbwrite( "This text is written to the file 't_FILE2.txt'", file2, off2);

		var file1x = file1;		// wicked copy of file handle
		file1x.osclose();		// we could even do: var( file1).osclose();

		var file3( "t_FILE3.txt");
		oswrite( "", file3);
		var off3 = 0;
		osbwrite( "This text is written to the file 't_FILE3.txt'", file3, off3);

		osbwrite( "THIS TEXT INTENDED FOR FILE 't_FILE1.txt' BUT IT GOES TO 't_FILE3.txt'", file1, off1);
	}
#endif

    var myclients;
    if (myclients.open("myclients")) {

	    printl();
    	printl("The following section requires data created by testsort.cpp");

        var key;

        //begintrans();

        if (var().open("myclients"))
                printl("Could open myclients");
        else
                printl("Could NOT open myclients");

        printl();
        printl("1. test full output with no selection clause or preselect");
        myclients.select();
        while(myclients.readnext(key)) {
                key.outputl("t1 key=");
        }

	printl();
        printl("2. test with default cursor (unassigned var) - select clause needs filename");
        myclients.select("select myclients with type 'B'");
        while(myclients.readnext(key)) {
                key.outputl("t2 key=");
        }

        printl();
        printl("3. tests with named cursor (assigned var) - if is file name then select clause can omit filename");
        myclients="myclients";
        printl("test preselect affects following select");
        myclients.select("with type 'B'");
        printl("Normally the following select would output all records but it only shows the preselected ones (2)");
        myclients.select();
        while(myclients.readnext(key)) {
                key.outputl("t3 key=");
        }

        printl();
        printl("4. Normally the following select would output all records but it only shows 3 keys from makelist");
        var keys="SB1" _FM_ "JB2" _FM_ "SB001";
        myclients.makelist("",keys);
        myclients.outputl();
//      myclients.select();
	//while(myclients.readnext(key)) {
	//	key.outputl("t4 key=");
	//}
	myclients.readnext(key);
	assert(key=="SB1");
	myclients.readnext(key);
	assert(key=="JB2");
	myclients.readnext(key);
	assert(key=="SB001");
	myclients.readnext(key);
	assert(key=="SB001");
	//committrans();
	//rollbacktrans();

    }

	printl("Test passed");

	return 0;
}

programexit()
