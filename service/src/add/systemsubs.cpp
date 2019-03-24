#include <exodus/library.h>
libraryinit()

#include <win_common.h>

#include <window.hpp>

var mode;

function main(in mode0) {

	//this is a template used for calling all filexxxsubs() routines
	//they must have ONE in arg
	return 0;

	mode = mode0;

	if (mode.field(",", 1) == "PREREAD") {
	}
	else if (mode.field(",", 1) == "POSTREAD") {
	}
	else if (mode.field(",", 1) == "PREWRITE") {
	}
	else if (mode.field(",", 1) == "POSTWRITE") {
	}
	else if (mode.field(",", 1) == "PREDELETE") {
	}
	else if (mode.field(",", 1) == "POSTDELETE") {
	} else {
		var().chr(7).output();
		call mssg(DQ ^ (mode ^ DQ) ^ " - invalid mode ignored");
		//valid=0
	}
	return 0;

}

libraryexit()
