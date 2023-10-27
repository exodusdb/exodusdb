#include <exodus/library.h>
libraryinit()

function main(in /*x*/, in /*y*/, in /*x2*/, in /*y2*/, in /*readwrite*/, io /*buffer*/) {

	// TODO implement if screen handling required

	// VIDEO.RW(0,0,@CRTWIDE-1,@CRTHIGH-1,'R',startBUFFER)
	// eg to copy a whole screen sized 80x25, use 0,0,79,24
	// R=Read, W=Write

	// evade warning "usused"
	//false and x and y and x2 and y2 and readwrite;

//	if (readwrite == "R") {
//		buffer = "";
//	} else if (readwrite == "W") {
//	}

	return 0;
}

libraryexit()
