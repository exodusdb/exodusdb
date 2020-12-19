#include <exodus/program.h>
programinit()

function main() {
	var filename=COMMAND.a(2);
	var fieldname=COMMAND.a(3);
	//no arguments - lists all file indexes
	//if (not filename)
	//	abort("Syntax is 'listindex {filename} {fieldname}'");
	printl(listindexes(filename,fieldname).convert(FM,"\n").swap(VM,"__"));
	return 0;
}

programexit()

