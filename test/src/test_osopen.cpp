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

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
