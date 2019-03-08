#include <exodus/library.h>
libraryinit()

function main(io v1, io v2, io v3, io v4, io v5, io v6) {
	v1.setcursor();
	v1 = "";
	v2.transfer(SENTENCE);
	v3.transfer(RECORD);
	v4.transfer(ID);
	v5.transfer(DICT);
	//MV = v6.substr(1,RM);
	//INSERT = v6.substr(_COL2 + 1,RM);
	//print(v6.substr(_COL2 + 1,v6.length()));
	MV=v6.field(RM,1);
	//INSERT=v6.field(RM,2);
	print(v6.substr(RM,3));
	v6 = "";
	return 0;

}

libraryexit()

