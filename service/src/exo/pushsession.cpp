#include <exodus/library.h>
libraryinit()

function main(out v1, out v2, out v3, out v4, out v5, out v6) {
	v1 = getcursor();
	SENTENCE.move(v2);
	RECORD.move(v3);
	ID.move(v4);
	DICT.move(v5);
	MV.move(v6);
	//v6 ^= RM ^ INSERT ^ RM ^ CURATR();
	return 0;

}

libraryexit()

