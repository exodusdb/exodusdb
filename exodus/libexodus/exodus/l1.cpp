#include <exodus/library.h>
libraryinit()

var temp;
function main() {
	if (temp.unassigned())
		temp = 0;
	temp++;
	TRACE(temp);
	return 0;
}

libraryexit()

