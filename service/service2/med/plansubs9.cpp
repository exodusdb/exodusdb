#include <exodus/library.h>
libraryinit()

var tt;

function main(in msgno, out msg, in params="") {
	//c med in,out,""
	if (params.unassigned()) {
		params = "";
	}

	if ((msgno < 1) or (msgno > 21)) {
		goto badmsgno;
	}
	switch (msgno.toInt()) {

		//should never get here
badmsgno:
		msg = DQ ^ (msgno ^ DQ) ^ " unknown message no in plan.subs9";
		if (params) {
			msg.r(1, -1, params);
		}
		return 0;

	case 1:
		msg = "You cannot delete a schedule|after it has been invoiced||(You may delete all the dates|and then issue a credit note)";
		return 0;
	case 2:
		//MSG='YOU CANNOT DELETE A PARTIALLY BOOKED SCHEDULE|OR SCHEDULE PENDING CANCELLATIONS'
		msg = "You cannot delete a schedule|after it has been booked||(You may delete all the dates|and then issue a cancellation order.)";
		return 0;
	case 3:
		msg = "Brand code is required but missing";
		return 0;
	case 4:
		msg = "Currency code is required but missing";
		return 0;
	case 5:
		msg = "Period is required but missing";
		return 0;
	case 6:
		msg = "Company code is required but missing";
		return 0;
	case 7:
		msg = "Stop date must be within " ^ params ^ " days of the start date";
		return 0;
	case 8:
		msg = "SYSTEM ERROR - PLAN.SUBS LOCK FAILURE|SOMEBODY ELSE HAS UPDATED THIS RECORD!";
		return 0;
	case 9:
		msg = "\"DATES\" DO NOT AGREE WITH \"NUMBER\" COLUMN||Do you want to correct it now ?";
		return 0;
	case 10:
		msg = "NOT SUPPORTED IN DOS VERSION ANY MORE|BECAUSE TO NEED TO CONVERT TO SMDATES";
		return 0;
	case 11:
		msg = "Exchange rate for " ^ params.a(2) ^ " must be 1 in line " ^ params.a(1) ^ "|Please refresh exchange rates";
		return 0;
	case 12:
		msg = "Exchange rate for " ^ params.a(2) ^ " must not be 1 in line " ^ params.a(1) ^ "|Please refresh exchange rates";
		return 0;
	case 13:

		if (msgno == 13) {
			tt = "";
		}else{
			tt = "not ";
		}
		msg = "Cost exchange rate for " ^ params.a(2) ^ "/" ^ params.a(3) ^ " must " ^ tt ^ "be 1 in line " ^ params.a(1) ^ "|Please refresh exchange rates";
		return 0;
	case 15:
		msg = "Lower case letters are not allowed in plan/schedule no";
		return 0;
	case 16:
		msg = "You cannot delete a plan after it has been approved";
		return 0;
	case 17:
		msg = "Campaign description is required";
		return 0;
	case 18:
		msg = "Cannot approve because this client or brand is stopped";
		if (params) {
			msg.r(-1, "Reason:" ^ (DQ ^ (params ^ DQ)));
		}
		return 0;
	case 19:
		msg = "Non-numeric planned ads in line " ^ params.count(VM) + 1 ^ ". Please correct.";
		return 0;
	case 20:
		msg = DQ ^ (params ^ DQ) ^ " company is missing";
		return 0;
	case 21:
		msg = "You cannot delete a schedule which has any certified ads|or supplier invoices.||You may decertify/remove them first and try again.";
		return 0;

	}

	//add any more messages line numbers to line 6 max and 7 (on goto)

}


libraryexit()
