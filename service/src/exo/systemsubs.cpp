#include <exodus/library.h>
libraryinit()


#include <req_common.h>

#include <request.hpp>

var mode;

function main(in mode0) {

	/*
		-Wcast-function-type - magic word - do not remove or split from previous line

		THE HEADER .H FILE OF THS SUBROUTINE IS USED AS A DUMMY
		FOR CALLING FUNCTIONS BY VARIABLE FUNCTON NAME

		PICK:
		FUNCTIONNAME = "SOMEFUNCTION"
		CALL @FUNCTIONVAR(V1,V2,V3)

		C++:
		systemsubs = "somefunction";
		call systemsubs(v1,v2,v3);

		THE CALL IS WRAPPED WITH A G++ DIAGNOSTIC SUPPRESSED
		TO EVADE COMPILER WARNINGS
		BECAUSE SYSTEMSUBS ACTUALLY HAS ONLY ONE ARGUMENT
		WHEREAS THE ACTUAL FUNCTION MAY HAVE ANY NUMBER OF ARGUMENTS
		RUNTIME HAS NO ISSUE AS LONG AS THE ACTUAL FUNCTION
		AND CALL HAVE THE SAME FUNCTION SIGNATURE

	*/

	// this is a template used for calling all filexxxsubs() routines
	// they must have ONE in arg
	return 0;

	mode = mode0;

	if (mode.field(",", 1) == "PREREAD")
		;
	else if (mode.field(",", 1) == "POSTREAD")
		;
	else if (mode.field(",", 1) == "PREWRITE")
		;
	else if (mode.field(",", 1) == "POSTWRITE")
		;
	else if (mode.field(",", 1) == "PREDELETE")
		;
	else if (mode.field(",", 1) == "POSTDELETE") {
	} else {
		call mssg(DQ ^ (mode ^ DQ) ^ " - invalid mode ignored");
		// valid=0
	}
	return 0;
}

libraryexit()
