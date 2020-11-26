#include <exodus/program.h>
programinit()

#include <sortarray.h>

function main() {
	printl("testsortarray says 'Hello World!'");
	var array="";

	array.r(3,	1  ^VM ^2  ^VM ^20  ^VM ^10);
	array.r(6,	61 ^VM ^62 ^VM ^620 ^VM ^610);
	array.r(7,	71 ^VM ^72 ^VM ^720 ^VM ^710);

	array.convert(VM^FM,"]\n").outputl();

	call sortarray(array,3 ^ VM ^ 7 ^ VM ^ 6,"DR");

	printl("-------------------");
	array.convert(VM^FM ,"]\n").outputl();

	return 0;
}

programexit()

