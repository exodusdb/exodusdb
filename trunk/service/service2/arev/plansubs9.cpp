#include <exodus/library.h>
libraryinit()

var tt;

function main() {

	if (params.unassigned()) {
		params = "";
	}
	if (msgno < 1 or msgno > 18) {
		goto badmsgno;
	}

	ON msgno GOTO 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 15, 16, 17, 18;

	//should never get here
badmsgno:
	var msg = DQ ^ (msgno ^ DQ) ^ " unknown message no in plan.subs9";
	if (params) {
		msg.r(1, -1, params);
	}
	return 0;

1:
	msg = "You cannot delete a schedule|after it has been invoiced||(You may delete all the dates|and then issue a credit note)";
	return 0;
2:
	//MSG='YOU CANNOT DELETE A PARTIALLY BOOKED SCHEDULE|OR SCHEDULE PENDING CANCELLATIONS'
	msg = "You cannot delete a schedule|after it has been booked||(You may delete all the dates|and then issue a cancellation order.)";
	return 0;
3:
	msg = "Brand code is required but missing";
	return 0;
4:
	msg = "Currency code is required but missing";
	return 0;
5:
	msg = "Period is required but missing";
	return 0;
6:
	msg = "Company code is required but missing";
	return 0;
7:
	msg = "Stop date must be within " ^ params ^ " days of the start date";
	return 0;
8:
	msg = "SYSTEM ERROR - PLAN.SUBS LOCK FAILURE|SOMEBODY ELSE HAS UPDATED THIS RECORD!";
	return 0;
9:
	msg = "\"DATES\" DO NOT AGREE WITH \"NUMBER\" COLUMN||Do you want to correct it now ?";
	return 0;
10:
	msg = "NOT SUPPORTED IN DOS VERSION ANY MORE|BECAUSE TO NEED TO CONVERT TO SMDATES";
	return 0;
11:
	msg = "Exchange rate for " ^ params.a(2) ^ " must be 1 in line " ^ params.a(1) ^ "|Please refresh exchange rates";
	return 0;
12:
	msg = "Exchange rate for " ^ params.a(2) ^ " must not be 1 in line " ^ params.a(1) ^ "|Please refresh exchange rates";
	return 0;
13:

	if (msgno == 13) {
		tt = "";
	}else{
		tt = "not ";
	}
	msg = "Cost exchange rate for " ^ params.a(2) ^ "/" ^ params.a(3) ^ " must " ^ tt ^ "be 1 in line " ^ params.a(1) ^ "|Please refresh exchange rates";
	return 0;
15:
	msg = "Lower case letters are not allowed in plan/schedule no";
	return 0;
16:
	msg = "You cannot delete a plan after it has been approved";
	return 0;
17:
	msg = "Campaign description is required";
	return 0;
18:
	msg = "Cannot approve because this client or brand is stopped";
	if (params) {
		msg.r(-1, "Reason:" ^ (DQ ^ (params ^ DQ)));
	}
	return 0;
19:
	msg = "Non-numeric planned ads in line " ^ params.count(VM) + 1 ^ ". Please correct.";
	return 0;

}


libraryexit()