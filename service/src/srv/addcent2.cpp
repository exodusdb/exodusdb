#include <exodus/library.h>
libraryinit()

#include <service_common.h>

function main(in yymm) {

	// only used in accounting to convert YYMM->YYYYMM

	// only convert 4 digit YYMM to YYYYMM
	if (yymm.len() != 4) {
		return yymm;
	}

	// year 00-50 -> 2000-2050
	if (yymm.first(2) <= centuryyear_) {
		return "20" ^ yymm;
	}

	// year 51-99 -> 1951-1999
	return "19" ^ yymm;
}

libraryexit()
