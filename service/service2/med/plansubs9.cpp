#include <exodus/library.h>
libraryinit()

var tt;

function main(in msgno, out msg, in params="") {

switch (msgno.toInt()) {

case 1:
	msg = "You cannot delete a schedule|after it has been invoiced||(You may delete all the dates|and then issue a credit note)";
	break;
case 2:
	//MSG='YOU CANNOT DELETE A PARTIALLY BOOKED SCHEDULE|OR SCHEDULE PENDING CANCELLATIONS'
	msg = "You cannot delete a schedule|after it has been booked||(You may delete all the dates|and then issue a cancellation order.)";
	break;
case 3:
	msg = "Brand code is required but missing";
	break;
case 4:
	msg = "Currency code is required but missing";
	break;
case 5:
	msg = "Period is required but missing";
	break;
case 6:
	msg = "Company code is required but missing";
	break;
case 7:
	msg = "Stop date must be within " ^ params ^ " days of the start date";
	break;
case 8:
	msg = "SYSTEM ERROR - PLAN.SUBS LOCK FAILURE|SOMEBODY ELSE HAS UPDATED THIS RECORD!";
	break;
case 9:
	msg = "\"DATES\" DO NOT AGREE WITH \"NUMBER\" COLUMN||Do you want to correct it now ?";
	break;
case 10:
	msg = "NOT SUPPORTED IN DOS VERSION ANY MORE|BECAUSE TO NEED TO CONVERT TO SMDATES";
	break;
case 11:
	msg = "Exchange rate for " ^ params.a(2) ^ " must be 1 in line " ^ params.a(1) ^ "|Please refresh exchange rates";
	break;
case 12:
	msg = "Exchange rate for " ^ params.a(2) ^ " must not be 1 in line " ^ params.a(1) ^ "|Please refresh exchange rates";
	break;
case 13:

	if (msgno == 13) {
		tt = "";
	}else{
		tt = "not ";
	}
	msg = "Cost exchange rate for " ^ params.a(2) ^ "/" ^ params.a(3) ^ " must " ^ tt ^ "be 1 in line " ^ params.a(1) ^ "|Please refresh exchange rates";
	break;
case 15:
	msg = "Lower case letters are not allowed in plan/schedule no";
	break;
case 16:
	msg = "You cannot delete a plan after it has been approved";
	break;
case 17:
	msg = "Campaign description is required";
	break;
case 18:
	msg = "Cannot approve because this client or brand is stopped";
	if (params) {
		msg.r(-1, "Reason:" ^ (DQ ^ (params ^ DQ)));
	}
	break;
case 19:
	msg = "Non-numeric planned ads in line " ^ (params.count(VM) + 1) ^ ". Please correct.";
	break;
case 20:
	msg = DQ ^ (params ^ DQ) ^ " company is missing";
	break;
	//see line 6

default:
	var msg = DQ ^ (msgno ^ DQ) ^ " unknown message no in plansubs9";
	if (params) {
		msg.r(1, -1, params);
	}
	break;
}

return 0;

}

libraryexit()
