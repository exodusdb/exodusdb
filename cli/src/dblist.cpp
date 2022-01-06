#include <exodus/program.h>
programinit()

function main() {

	if (not connect())
		lasterror().errputl();

	printl(dblist().convert(FM, "\n"));

	return 0;
}

programexit()

