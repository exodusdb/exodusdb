#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>

programinit()

	var testfilename;
	var dictfilename;
	var testfile;

function main() {

	//Pass if no default database connection
	if (not connect()) {
		printl("No default db connection to perform db testing. Test passed");
		return 0;
	}

	//ensure dict_voc has f1m
	var dictvoc;
	if (not(dictvoc.open("dict.voc", ""))) {
		call fsmsg();
		stop();
	}
	if (not read(RECORD,dictvoc,"F1M")) {
		var rr = "F";
		for (var ii = 1; ii <= 12; ++ii) {

			ID = "F" ^ ii;
			rr.r(2, ii);
			rr.r(3, ID);
			rr.r(9, "L");
			rr.r(10, "10");

			printl(ID);
			rr.write(dictvoc, ID);

			rr.r(4, "M");
			ID ^= "M";
			printl(ID);
			rr.write(dictvoc, ID);

		} //ii;
	}

	testfilename="xo_test_select";
	dictfilename="dict." ^ testfilename;

	deletefile(testfilename);
	deletefile(dictfilename);

	assert(createfile(testfilename));
	assert(createfile(dictfilename));

	assert(open(testfilename,testfile));
	assert(open(dictfilename,DICT));

	assert(write(convert("AA BB CC DD"," ",VM),testfile,"A"));
	assert(write(convert("BB CC DD EE"," ",VM),testfile,"B"));
	assert(write(convert("CC DD EE FF"," ",VM),testfile,"C"));

	test();

	assert(createindex(testfilename,"f1m"));

	test();

	printl("Test passed");

	return 0;
}

function test() {

	assert(select(testfilename ^ " with f1m 'E]' 'XX' (SR)"));
	assert(readnext(RECORD,ID,MV));
	assert(ID.outputl("E] FF 1=") == "B");
	assert(readnext(RECORD,ID,MV) && ID.outputl("E] FF 2=") == "C");
	assert(!readnext(RECORD,ID,MV));

	assert(select(testfilename ^ " with f1m 'AA' (SR)"));
	assert(readnext(RECORD,ID,MV) && ID.outputl("AA 1=") == "A");
	assert(!readnext(RECORD,ID,MV));

	assert(select(testfilename ^ " with f1m 'BB' (SR)"));
	assert(readnext(RECORD,ID,MV) && ID.outputl("BB 1=") == "A");
	assert(readnext(RECORD,ID,MV) && ID.outputl("BB 2=") == "B");
	assert(!readnext(RECORD,ID,MV));

	assert(select(testfilename ^ " with f1m 'CC' (SR)"));
	assert(readnext(RECORD,ID,MV) && ID.outputl("CC 1=") == "A");
	assert(readnext(RECORD,ID,MV) && ID.outputl("CC 2=") == "B");
	assert(readnext(RECORD,ID,MV) && ID.outputl("CC 3=") == "C");
	assert(!readnext(RECORD,ID,MV));

	assert(select(testfilename ^ " with f1m 'DD' (SR)"));
	assert(readnext(RECORD,ID,MV) && ID.outputl("DD 1=") == "A");
	assert(readnext(RECORD,ID,MV) && ID.outputl("DD 2=") == "B");
	assert(readnext(RECORD,ID,MV) && ID.outputl("DD 3=") == "C");
	assert(!readnext(RECORD,ID,MV));

	assert(select(testfilename ^ " with f1m 'EE' (SR)"));
	assert(readnext(RECORD,ID,MV) && ID.outputl("EE 1=") == "B");
	assert(readnext(RECORD,ID,MV) && ID.outputl("EE 2=") == "C");
	assert(!readnext(RECORD,ID,MV));

	assert(select(testfilename ^ " with f1m 'FF' (SR)"));
	assert(readnext(RECORD,ID,MV) && ID.outputl("EE 1=") == "C");
	assert(!readnext(RECORD,ID,MV));

	assert(select(testfilename ^ " with f1m 'AA' 'FF' (SR)"));
	assert(readnext(RECORD,ID,MV) && ID.outputl("AA FF 1=") == "A");
	assert(readnext(RECORD,ID,MV) && ID.outputl("AA FF 2=") == "C");
	assert(!readnext(RECORD,ID,MV));

	assert(select(testfilename ^ " with f1m 'BB' 'EE' (SR)"));
	assert(readnext(RECORD,ID,MV) && ID.outputl("BB EE 1=") == "A");
	assert(readnext(RECORD,ID,MV) && ID.outputl("BB EE 2=") == "B");
	assert(readnext(RECORD,ID,MV) && ID.outputl("BB EE 3=") == "C");
	assert(!readnext(RECORD,ID,MV));

	assert(select(testfilename ^ " with f1m 'EE' 'FF' (SR)"));
	assert(readnext(RECORD,ID,MV) && ID.outputl("EE FF 1=") == "B");
	assert(readnext(RECORD,ID,MV) && ID.outputl("EE FF 2=") == "C");
	assert(!readnext(RECORD,ID,MV));

	//E doesnt exist
	assert(select(testfilename ^ " with f1m 'E' 'FF' (SR)"));
	assert(readnext(RECORD,ID,MV) && ID.outputl("E FF 1=") == "C");
	assert(!readnext(RECORD,ID,MV));

	//E and F dont exist
	assert(select(testfilename ^ " with f1m 'E' 'F' (SR)"));
	assert(!readnext(RECORD,ID,MV));

	//test STARTING filter on multivalues
	if (true) {

		//E] and XX finds EE
		assert(select(testfilename ^ " with f1m 'E]' 'XX' (SR)"));
		assert(readnext(RECORD,ID,MV) && ID.outputl("E] XX 1=") == "B");
		assert(readnext(RECORD,ID,MV) && ID.outputl("E] XX 2=") == "C");
		assert(!readnext(RECORD,ID,MV));

		//A] and F] finds AA FF
		assert(select(testfilename ^ " with f1m 'A]' 'F]' (SR)"));
		assert(readnext(RECORD,ID,MV) && ID.outputl("A] F] 1=") == "A");
		assert(readnext(RECORD,ID,MV) && ID.outputl("A] F] 2=") == "C");
		assert(!readnext(RECORD,ID,MV));

		//without A] and F] finds B only
		assert(select(testfilename ^ " without f1m 'A]' 'F]' (SR)"));
		assert(readnext(RECORD,ID,MV) && ID.outputl("without A] F] 1=") == "B");
		assert(!readnext(RECORD,ID,MV));
	}

	return 0;
}

programexit()
