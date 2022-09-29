#include <exodus/program.h>
programinit()

function main() {

	var filename = COMMAND.f(2);
	var fieldname = COMMAND.f(3);

	//no arguments - lists all file indexes
	//if (not filename)
	//	abort("Syntax is 'listindex {filename} {fieldname}'");

	printl(listindexes(filename, fieldname).convert(FM, "\n").replace(VM, "__"));

	return 0;
}

programexit()
