#include <exodus/library.h>
libraryinit()

var params;

function main(in msgno, io msg, in params0="") {
	//c sys in,io,""
	if (params0.unassigned()) {
		params = "";
	} else {
		params = params0;
	}

	switch (msgno.toInt()) {

	default:
	//should never get here
		msg = msgno.quote() ^ " unknown message no in LISTEN4";
		return 0;

	case 1:
		msg = "Error: Response not set in LISTEN";
		return 0;
	case 2:
		msg = "Error: Maximum record size of " ^ oconv(params.f(1), "[XBYTES]") ^ " exceeded in LISTEN";
		return 0;
	case 3:
		msg = "Error: LISTEN cannot open " ^ (params.quote());
		return 0;
	case 4:
		msg = "Error: No request";
		return 0;
	case 5:
		msg = "Error: EXODUS.NET Key missing and not READU";
		return 0;
	case 6:
		msg = "Error: Next number was not produced" ^ FM ^ params;
		return 0;
	case 7:
		msg = "Error: CANNOT LOCK RECORD";
		return 0;
	case 8:
		msg = "Error: NO RECORD";
		return 0;
	case 9:
	//same in 9 and 10
		msg = params.quote() ^ " file cannot be opened (9)";
		return 0;
	case 10:
	//same in 9 and 10 but 10 never used
		msg = params.quote() ^ " file cannot be opened (10)";
		return 0;
	case 11:
		msg = params.f(1) ^ " " ^ (params.f(2).quote()) ^ " is not allowed";
		return 0;
	case 12:
		msg = "Somebody has updated this record.|Your update cannot be applied." "|The session id does not agree " ^ (params.quote());
		return 0;
	case 13:
		msg = "You or somebody else has updated this record.|Your update cannot be applied." "|The time stamp does not agree||Perhaps it is already saved.|Open it in another window to check.";
		return 0;
	case 14:
		msg = "Write empty data record is disallowed.";
		return 0;
	case 15:
		msg = "Cannot delete because " ^ (params.f(1).quote()) ^ " does not exist in the " ^ (params.f(2).quote()) ^ " file";
		return 0;
	case 16:
		msg = "Error: " ^ (params.quote()) ^ " module is not available";
		return 0;
	case 17:
		msg = "Please select fewer records and/or simplify your request|(eg select by group instead of individually)";
		return 0;
	case 18:
		msg = "Error: No OK from request " ^ (params.quote());
		return 0;
	case 19:
		msg = "Error: Database already stopped/stopping";
		return 0;
	case 20:
		msg = "Error: Could not terminate " ^ params.f(1) ^ " processes|" ^ params.f(2);
		return 0;
	case 21:
		msg = "Error: Unknown request " ^ (params.quote());
		return 0;
	case 22:
		msg = "ERROR: LISTEN cannot create temporary file " ^ (params.quote());
		return 0;
	case 23:
		msg = "Error: " ^ (params.quote()) ^ " CANNOT BE WRITTEN BECAUSE IT IS LOCKED ELSEWHERE";
		return 0;
	case 24:
		msg = "Error: " ^ (params.quote()) ^ " file does not exist";
		return 0;
	case 25:
		msg = "Error: Somebody else has locked the record";
		return 0;
	case 26:
		msg = "Error: Cannot unlock - " ^ params ^ " session id";
		return 0;
	case 27:
		msg = "A new password has been emailed to you";
		return 0;
	case 28:
		msg = "Error: " ^ (params.f(1).quote()) ^ " new record keys must not contain spaces";
		return 0;
	case 29:
	//29 and 30 the same
		msg = "Unknown message " ^ (msgno.quote()) ^ " in LISTEN";
		return 0;
	case 30:
	//29 and 30 the same
		msg = "Unknown message " ^ (msgno.quote()) ^ " in LISTEN";
		return 0;
	case 31:
		msg(-1) = "INTERNAL ERROR Closing current EXODUS server process";
		return 0;
	case 32:
		msg = "LISTEN:EXECUTE: Module name is missing from request";
		return 0;

	//keyword to force adecom to insert a c++ }  to close the switch statement
	}

	return 0;
}

libraryexit()
