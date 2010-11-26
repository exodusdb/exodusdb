//
// Testing dict_TESTLIB
//
// NOTE 1. Assumed that project dict_TESTLIB exist and builds dict_TESTLIB.lib
//
// NOTE 2. Multivalue-mimicking "everything is a global function" syntax is used
//		   instead of exodus's OO-style syntax "xxx.yyy().zzz()"
//
//NOTE 3. The code mainly copied from service.sln, .\service\main.cpp (or main2.cpp)
//
#include <exodus/program.h>
//#include <cassert>
#include <cassert-exodus>

programinit()

function main()
{
    if (not connect())
        abort("Cannot connect to database. Please check configuration");

	var filename="TESTLIB";
    var dictfilename="dict_"^ filename;

	std::string narrow = filename.tostring();

    // leave the test data files around for playing with
    var cleanup=true;
    if (cleanup) {
		assert_warn( false);
		assert_pause( ! filename);
        deletefile(filename);
        deletefile(dictfilename);
	}

	createfile(filename);
	createfile(dictfilename);

	write("S"^FM^FM^"Age in Days"^FM^FM^FM^FM^FM^FM^"R"^FM^"10",dictfilename,"AGE_IN_DAYS");
	write("S"^FM^FM^"Age in Years"^FM^FM^FM^FM^FM^FM^"R"^FM^"10",dictfilename,"AGE_IN_YEARS");
	write("F"^FM^1^FM^"Number"^FM^FM^FM^FM^FM^FM^"R"^FM^"10",dictfilename,"NUMBER");

	write("1",filename,"10");
	write("2",filename,"20");
	write("3",filename,"30");
	write("4",filename,"40");

//	if (not select("SELECT " ^ filename))	<-- this does NOT fill RECORD
	if (not selectrecord("SELECT " ^ filename))
		printl("Failed to Select");

	DICT=dictfilename;

//	while (readnext(ID))				<-- does not initialize/fill RECORD
//	while (readnextrecord( ID, RECORD))	<-- wrong parameters usage
	while ( readnextrecord( RECORD, ID))
	{
		print("ID=",ID, " RECORD=",RECORD);
		print(" AGE_IN_DAYS=",calculate("AGE_IN_DAYS"));	// calls to dict_TESTLIB.DLL->AGE_IN_DAYS() function
		printl(" AGE_IN_YEARS=",calculate("AGE_IN_YEARS"));	// calls to dict_TESTLIB.DLL->AGE_IN_YEARS() function
	}

	return 0;
}

programexit()
