#include <exodus/library.h>
libraryinit()

function main(in type, in inx, in mode, out outx) {
	//c fin in,in,in,out

	STATUS = 0;
	outx = inx;
	if (not inx) {
		return inx;
	}

	//evade warning
	if (0) {
		outx = mode;
	}

	//sadly print x '[ACNO2]' this subroutine gets called once per bit
	//not once for the whole lot
	//unless you CALL it
	var nn = inx.count(VM) + 1;
	var ii = 0;
	for (var ii = 1; ii <= nn; ++ii) {
		var temp = inx.a(1, ii);
		if (type == "OCONV") {
			temp = temp.a(1, 1, 1);
		} else if (type == "ICONV") {
			temp = temp.a(1, 1, 2);
		}
		if (temp) {
			outx.r(1, ii, temp);
		}
	};//ii;

	return outx;
}

libraryexit()
