#include <exodus/program.h>
programinit()

function main() {
	var filename=COMMAND.a(2);
	var fieldname=COMMAND.a(3);
	//if (not filename)
	//	abort("Syntax is 'listindexes {filename} {fieldname}'");
	printl(listindexes(filename,fieldname).convert(FM^VM,"\n "));
	return 0;
}

programexit()

