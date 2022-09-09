#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

//#include <sortarray.h>

function main() {
	printl("testsortarray says 'Hello World!'");
	var array="";

	array(3) = 	1  ^VM ^2  ^VM ^20  ^VM ^10;
	array(6) = 	61 ^VM ^62 ^VM ^620 ^VM ^610;
	array(7) = 	71 ^VM ^72 ^VM ^720 ^VM ^710;

	array.convert(VM^FM,"]\n").outputl();

	call sortarray(array,3 ^ VM ^ 7 ^ VM ^ 6,"DR");

	printl("-------------------");
	array.convert(VM^FM ,"]\n").outputl();

	converter(array,VM,"]");
	assert(array.f(3)=="20]10]2]1");
	assert(array.f(6)=="620]610]62]61");
	assert(array.f(7)=="720]710]72]71");

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
