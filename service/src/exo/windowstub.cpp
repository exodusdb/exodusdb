#include <exodus/library.h>
libraryinit()

#include "systemsubs.h"

function main() {
	//printl("windowstub says 'Hello World!'");
	//SENTENCE.outputl("SENTENCE=");
	var progname=SENTENCE.field(" ",2).lcase().convert(".","");
	var arg1=SENTENCE.field(" ",3,999);
	systemsubs=progname;
	call systemsubs(arg1);
	return 0;
}

libraryexit()

