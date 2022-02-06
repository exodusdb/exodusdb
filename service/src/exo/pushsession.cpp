#include <exodus/library.h>
libraryinit()

function main(out v1, out v2, out v3, out v4, out v5, out v6) {
	v1 = getcursor();
	SENTENCE.transfer(v2);
	RECORD.transfer(v3);
	ID.transfer(v4);
	DICT.transfer(v5);
	MV.transfer(v6);
	//v6 ^= RM ^ INSERT ^ RM ^ CURATR();
	return 0;

}

libraryexit()

