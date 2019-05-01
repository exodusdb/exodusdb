#include <exodus/library.h>
libraryinit()

function main(in mode, in processno, out result) {

	//returns 1 if processno is busy/cannot be locked, otherwise 0
	false and mode;//evade warning

	result=not mv.processno_islocked(processno);

	return result;
}

libraryexit()

