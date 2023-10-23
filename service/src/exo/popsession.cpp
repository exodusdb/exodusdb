#include <exodus/library.h>
libraryinit()

function main(io v1, io v2, io v3, io v4, io v5, io v6) {
	/* pushsession
        v1=var().getcursor();
        SENTENCE.move(v2);
        RECORD.move(v3);
        ID.move(v4);
        DICT.move(v5);
        MV.move(v6);
        //v6 ^= RM ^ INSERT ^ RM ^ CURATR();
*/
	setcursor(v1);
	v1 = "";
	v2.move(SENTENCE);
	v3.move(RECORD);
	v4.move(ID);
	v5.move(DICT);
	v6.move(MV);
	MV = v6.field(RM, 1);
	// INSERT=v6.field(RM,2);
	// printx(v6.field(RM,3));
	v6 = "";
	return 0;
}

libraryexit()
