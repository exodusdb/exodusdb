#include <exodus/library.h>
libraryinit()

function main(in yymm) {
	//c sys in,in,in,out

	#define centuryyear 90

	//linemark

	//only used in accounting to convert YYMM->YYYYMM

	//only convert 4 digit YYMM to YYYYMM
	if (yymm.length() ne 4) {
		return yymm;
	}

	//year 00-90 -> 2000-2090
	if (yymm.substr(1,2) <= centuryyear) {
		return "20" ^ yymm;
	}

	//year 91-99 -> 1991-1999
	return "19" ^ yymm;
}

libraryexit()
