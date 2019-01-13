#include <exodus/library.h>
libraryinit()

var params;

function main(in msgno, io msg, in params0="") {
	//c sys in,io,""
	if (params0.unassigned()) {
		params = "";
	}else{
		params = "";
	}

	switch (msgno.toInt()) {

default:
		//should never get here
		msg = DQ ^ (msgno ^ DQ) ^ " unknown message no in LISTEN4";
		return 0;

	case 1:
		msg = "Error: Response not set in LISTEN";
		return 0;
	case 2:
		msg = "Error: Maximum record size of 64Kb exceeded in LISTEN";
		return 0;
	case 3:
		msg = "Error: LISTEN cannot open " ^ (DQ ^ (params ^ DQ));
		return 0;
	case 4:
		msg = "Error: No request";
		return 0;
	case 5:
		msg = "Error: NEOSYS.NET Key missing and not READU";
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
		msg = DQ ^ (params ^ DQ) ^ " file cannot be opened";
		return 0;
	case 10:
		//same in 9 and 10
		msg = DQ ^ (params ^ DQ) ^ " file cannot be opened";
		return 0;
	case 11:
		msg = params.a(1) ^ " " ^ (DQ ^ (params.a(2) ^ DQ)) ^ " is not allowed";
		return 0;
	case 12:
		msg = "Somebody has updated this record." _VM_ "Your update cannot be applied." "" _VM_ "The session id does not agree " ^ (DQ ^ (params ^ DQ));
		return 0;
	case 13:
		msg = "You or somebody else has updated this record." _VM_ "Your update cannot be applied." "" _VM_ "The time stamp does not agree" _VM_ "" _VM_ "Perhaps it is already saved." _VM_ "Open it in another window to check.";
		return 0;
	case 14:
		msg = "Write empty data record is disallowed.";
		return 0;
	case 15:
		msg = "Cannot delete because " ^ (DQ ^ (params.a(1) ^ DQ)) ^ " does not exist in the " ^ (DQ ^ (params.a(2) ^ DQ)) ^ " file";
		return 0;
	case 16:
		msg = "Error: " ^ (DQ ^ (params ^ DQ)) ^ " module is not available";
		return 0;
	case 17:
		msg = "Please select fewer records and/or simplify your request|(eg select by group instead of individually)";
		return 0;
	case 18:
		msg = "Error: No OK from request " ^ (DQ ^ (params ^ DQ));
		return 0;
	case 19:
		msg = "Error: Database already stopped/stopping";
		return 0;
	case 20:
		msg = "Error: Could not terminate " ^ params.a(1) ^ " processes|" ^ params.a(2);
		return 0;
	case 21:
		msg = "Error: Unknown request " ^ (DQ ^ (params ^ DQ));
		return 0;
	case 22:
		msg = "ERROR: LISTEN cannot create temporary file " ^ (DQ ^ (params ^ DQ));
		return 0;
	case 23:
		msg = "Error: " ^ (DQ ^ (params ^ DQ)) ^ " CANNOT BE WRITTEN BECAUSE IT IS LOCKED ELSEWHERE";
		return 0;
	case 24:
		msg = "Error: " ^ (DQ ^ (params ^ DQ)) ^ " file does not exist";
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
		msg = "Error: " ^ (DQ ^ (params.a(1) ^ DQ)) ^ " new record keys must not contain spaces";
		return 0;
	case 29:
		//29 and 30 the same
		msg = "Unknown message " ^ (DQ ^ (msgno ^ DQ)) ^ " in LISTEN";
		return 0;
	case 30:
		//29 and 30 the same
		msg = "Unknown message " ^ (DQ ^ (msgno ^ DQ)) ^ " in LISTEN";
		return 0;
	case 31:
		msg.r(-1, "INTERNAL ERROR Closing current NEOSYS server process");
		return 0;

			}

		return 0;

}


libraryexit()
