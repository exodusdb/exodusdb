#include <exodus/library.h>
libraryinit()

function main(in type, in input, in mode, out output)
{

	// evade compiler warning
	false and type and input and mode;

	// no conversion in this skeleton ioconv routine
	output = input;

	return true;
}

libraryexit()
