#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

func main() {

	var filename = "t_hello.txt";
	assert(oswrite("xyz", filename));
	assert(osfile(filename).f(1) eq 3);

	var filex;
	assert(osopen(filename, filex));

	var data;
	var offset = 0;
	var length = 4;
	assert(osbread(data, filex, offset, length));

	assert(data.outputl() eq "xyz");

	assert(osremove(filename));
	assert(not osfile(filename));

	// Test automatic osopen
	{
		let tname = "t_testx.txt";
		assert(oswrite(str("abcdefghij", 10) on tname));
		var pos1 = 15;
		// Use tname without opening it.
		assert(osbread(RECORD from tname, pos1, 10) and RECORD == "fghijabcde");

		// Check tname now is an osfilevar.
		// var:0x7ffe859e7160 typ:25 osfile: int:1 dbl:1 nan: str: "t_testx.txt"0.631 ms
		assert(tname.dump().errputl().match(" typ:25 osfile: int:\\d+ dbl:\\d+ nan: str: "));

		// Check removal autocloses
		assert(tname.osremove());
		// var:0x7ffcd31bab28 typ:1 str: "t_testx.txt"
		assert(tname.dump().errputl().match(" typ:1 str: "));
	}

	{
		// Check osbread autoopen can fail
		let tname = "t_testx.txt";
		var pos1 = 0;
		assert(not osbread(RECORD from tname, pos1, 10));

		// Check osbwrite autoopen CANNOT autocreate if offset ne 0
		pos1 = 10;
		assert(not osbwrite("xyz" on tname, pos1));

		// Check osbwrite autoopen CAN autocreate if offset 0
		pos1 = 0;
		assert(osbwrite("abc" on tname, pos1));

		// Check was written correctly
		assert(osread(tname) == "abc");
	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
