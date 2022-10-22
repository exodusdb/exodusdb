#include <exodus/library.h>
libraryinit()

function main(in filename = "") {
	if (filename)
		flushindex(filename);
	return 0;
}

libraryexit()
