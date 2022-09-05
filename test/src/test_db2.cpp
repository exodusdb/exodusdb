#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>

programinit()

function main()
{

	//Pass if no default database connection
	if (not connect()) {
		printl("No default db connection to perform db testing.");
		printl("Test passed");
		return 0;
	}

//  //Skip if fast testing required
//	if (osgetenv("EXO_FAST_TEST")) {
//		printl("EXO_FAST_TEST - skipping test.");
//		printl("Test passed");
//		return 0;
//	}

	var FILE = "xo_users";
	DICT = "dict.xo_users";

	deletefile(FILE);
	deletefile(DICT);

	printl("Create " ^ FILE ^ " and " ^ DICT);
	assert(createfile(FILE));
	assert(createfile(DICT));

	printl(R"(Create some dictionary records (field descriptions)
	PERSON_NO    Type 'F', Key Field (0)
	BIRTHDAY     Type 'F', Data Field 1
	AGE IN DAYS  Type 'S', Source Code needs a dictionary subroutine library called dict_xo_users
	AGE IN YEARS Type 'S', Source Code ditto)");
	assert(write(convert( "F|0|Person No||||||R|10"   ,"|",FM),DICT, "PERSON_NO"   ));
	assert(write(convert( "F|1|Birthday||||D||R|12"   ,"|",FM),DICT, "BIRTHDAY"    ));
	assert(write(convert( "S||Age in Days||||||R|10"  ,"|",FM),DICT, "AGE_IN_DAYS" ));
	assert(write(convert( "S||Age in Years||||||R|10" ,"|",FM),DICT, "AGE_IN_YEARS"));

	printl("Create some users and their birthdays 11000=11 FEB 1998 .... 14000=30 APR 2006");
	assert(write("11000",FILE,"1"));
	assert(write("12000",FILE,"2"));
	assert(write("13000",FILE,"3"));
	assert(write("14000",FILE,"4"));

	printl("Check via dictionary giving a specific RECORD");
	assert(calculate("BIRTHDAY", "dict.xo_users", "1", "99000", 0).outputl() == 99000);

	printl("Same check using global");
	RECORD = "11000";
	ID = "2";
	assert(calculate("BIRTHDAY").outputl() == 11000);

//DBTRACE=true;
	//check can create and delete indexes
	//errmsg = {var_mvstr="ERROR:  function exodus_extract_date(bytea, integer, integer, integer) does not exist
	//use DBTRACE to see the error
	printl("CHECKING IF PGEXODUS POSTGRES PLUGIN IS INSTALLED");
	var pluginok=true;
	if (not createindex(FILE,"BIRTHDAY")) {
		pluginok=false;
		printl("Error: pgexodus, Exodus's plugin to PostgreSQL is not working. Run configexodus.");
	}

	if (pluginok) {
		assert(listindexes().oswrite("x")                       eq "xo_users]birthday"_var);
		assert(listindexes(FILE).oswrite("y")             eq "xo_users]birthday"_var);
		assert(listindexes(FILE, "birthday").oswrite("z") eq "xo_users]birthday"_var);
		assert(listindexes() ne "");
//ALN: do not delete to make subsequent select work::	assert(deleteindex(FILE,"BIRTHDAY"));
//		assert(listindexes(FILE) eq "");
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
		assert(not readnext(ID));//check no more
	}

	printl("Test dictionaries");
//	if (not select("SELECT xo_users WITH AGE_IN_DAYS GE 0 AND WITH AGE_IN_YEARS GE 0"))
//	if (not select("SELECT xo_users"))
	if (not select("SELECT xo_users (R)"))
		assert(false and var("Failed to Select xo_users!"));

	DICT="dict.xo_users";
	while (readnext(RECORD,ID,MV))
//	while (readnext(ID))
	{
		printl("ID=" ^ ID ^ " RECORD=" ^ RECORD);

		// test calling dictionary items (only type F so no dict_xxxxx lib calls are done)
		assert(calculate("BIRTHDAY") eq ("1" ^ ID ^ "000"));

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

	var filenames2="xo_clients";
	filenames2^=FM^"xo_currencies";
	filenames2^=FM^"xo_markets";
	filenames2^=FM^"xo_ads";

	var tempfile;
	printl();
	var nfiles=dcount(filenames2,FM);
	for (int ii=1;ii<=nfiles;++ii) {
		var filename=filenames2.f(ii);

		printl(filename);

		if (not open(filename, tempfile)) {
			printl("Creating "^filename);
			assert(createfile(filename));
		}

		if (not open("dict."^filename.lcase(), tempfile)) {
			printl("Creating dict."^filename);
			assert(createfile("dict."^filename));
		}

	}
//	var("x:y:z:").dcount(":").outputl();
//	var().stop();

	var ads;
	if (!ads.open("xo_ads"))
	{
		var().createfile("xo_ads");
		if (!ads.open("xo_ads"))
			printl("Cannot create xo_ads");
			//abort("Cannot create xo_ads");
	}

	printl("Create Currency dict recs");
	write("F"^FM^0^FM^"Currency Code"^FM^FM^FM^FM^FM^FM^""^"10","dict.xo_currencies","CURRENCY_CODE");
	write("F"^FM^1^FM^"Currency Name"^FM^FM^FM^FM^FM^FM^"T"^"20","dict.xo_currencies","CURRENCY_NAME");

	printl("Create Markets dict recs");
	write("F"^FM^1^FM^"Market Code"^FM^FM^FM^FM^FM^FM^""^"10","dict.xo_markets","CODE");
	write("F"^FM^1^FM^"Market Name"^FM^FM^FM^FM^FM^FM^"T"^"20","dict.xo_markets","NAME");

	printl("Create Ads dict recs");
	var dictrec="";
	dictrec(1) = "F";
	dictrec(2) = "3";
	dictrec(3) = "Brand Code";
	if (not dictrec.write("dict.xo_ads","BRAND_CODE"))
		printl("cannot write dict_ads, BRAND_CODE");

	//oo style
	assert(ads.createindex("BRAND_CODE"));
	assert(ads.deleteindex("BRAND_CODE"));

	//procedural one arg
	assert(createindex("xo_ads BRAND_CODE"));
	assert(deleteindex("xo_ads BRAND_CODE"));

	//procedural two args
	assert(createindex("xo_ads","BRAND_CODE"));
	assert(deleteindex("xo_ads","BRAND_CODE"));

	// Check cannot delete non-existent index
	assert(not deleteindex("xo_ads","BRAND_CODE"));

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
	int ii=0;
//	cin>>ii;
	var record;
	begintrans();
	if (ads.select("SELECT xo_ads")) {
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

//#ifdef FILE_IO_CACHED_HANDLES_EXCLUDED
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
//#endif

    var myclients;
	var clients_filename = "xo_clients";
    if (myclients.open(clients_filename)) {

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
        myclients.select();
        while(myclients.readnext(key)) {
                key.outputl("t1 key=");
        }

	printl();
        printl("2. test with default cursor (unassigned var) - select clause needs filename");
        myclients.select("select " ^ clients_filename ^ " with type 'B'");
        while(myclients.readnext(key)) {
                key.outputl("t2 key=");
        }

        printl();
        printl("3. tests with named cursor (assigned var) - if is file name then select clause can omit filename");
        myclients=clients_filename;
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

		assert(write("Client XB1^10000"_var on myclients, "XB1"));
		assert(write("Client XB2^20000"_var on myclients, "XB2"));

		// Check mvprogram's xlate using X and C mode9
		assert(xlate(clients_filename, "XB1", 1, "C") eq "Client XB1");
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
	    assert(write(1 on file,1));
	    assert(read(RECORD from file,1));
	    assert(RECORD eq 1);

	    printl("Start transaction");
	    assert(begintrans());

	    printl("Check write within transaction can be seen within transaction");
	    assert(write(11 on file,1));
	    assert(read(RECORD from file,1));
	    assert(RECORD eq 11);

	    printl("test Rollback");
	    assert(rollbacktrans());

	    printl("Check rollback reverts write");
	    assert(read(RECORD from file,1));
	    assert(RECORD eq 1);

	    printl("Check temporary files are deleted after closing connection");
	    disconnect();
	    //disconnectall();
	    connect();
	    assert(not open(filename to file));
	}

	printl("Test passed");

	return 0;
}

programexit()
