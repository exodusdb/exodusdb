#include <exodus/library.h>
libraryinit()

function main(io v1, io v2, io v3, io v4, io v5, io v6) {
/* pushsession
        v1=var().getcursor();
        SENTENCE.transfer(v2);
        RECORD.transfer(v3);
        ID.transfer(v4);
        DICT.transfer(v5);
        MV.transfer(v6);
        //v6 ^= RM ^ INSERT ^ RM ^ CURATR();
*/
	v1.setcursor();
	v1 = "";
	v2.transfer(SENTENCE);
	v3.transfer(RECORD);
	v4.transfer(ID);
	v5.transfer(DICT);
	v6.transfer(MV);
	MV=v6.field(RM,1);
	//INSERT=v6.field(RM,2);
	//print(v6.field(RM,3));
	v6 = "";
	return 0;

}

libraryexit()

