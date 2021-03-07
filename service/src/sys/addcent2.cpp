#include <exodus/library.h>
libraryinit()

function main(in yymm) {
	//c sys in,in,in,out

	//only used in accounting to convert YYMM->YYYYMM

	#define centuryyear 50

	//linemark

	//only convert 4 digit YYMM to YYYYMM
	if (yymm.length() ne 4) {
		return yymm;
	}

	//year 00-50 -> 2000-2050
	if (yymm.substr(1, 2) le centuryyear) {
		return "20" ^ yymm;
	}

	//year 51-99 -> 1951-1999
	return "19" ^ yymm;
}

libraryexit()
