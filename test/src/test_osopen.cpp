#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>

programinit()

function main() {

	var filename="t_hello.txt";
	assert(oswrite("xyz", filename));
	assert(osfile(filename).a(1) == 3);

	var filex;
	assert(osopen(filename,filex));

	var data;
	var offset=0;
	var length=4;
	assert(osbread(data,filex,offset,length));

	assert(data == "xyz");

	assert(osdelete(filename));
	assert(not osfile(filename));

	printl("Test passed");

	return 0;
}

programexit()

