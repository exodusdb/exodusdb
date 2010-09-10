#include <exodus/exodus.h>

program() {
	var filename=_COMMAND.extract(2);
	if (not filename)
		abort("Syntax is 'listindexes filename'");
	println(listindexes(filename).convert(FM,"\n"));
}

