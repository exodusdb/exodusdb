#include <exodus/library.h>

libraryinit(acno)
function main() {
	return RECORD.a(5) ^ VM ^ RECORD.a(6);
}
libraryexit(acno)


libraryinit(authorised)
#include <authorised.h>
#include <validcode2.h>
var positive;
var msg;
function main() {
	//ALLOWPARTIALACCESS
	//market code is not currently data entered in suppliers but validate it anyway
	ANS = 0;

	if (authorised("MEDIA/PRODUCTION TYPE ACCESS", msg)) {
		positive = "";
	}else{
		positive = "#";
	}

	if (authorised(positive ^ "MEDIA/PRODUCTION TYPE ACCESS " ^ ID.quote(), msg )) {
		var compcode = calculate("COMPANY_CODE");
		if (compcode) {
			var xx;
			if (validcode2(calculate("COMPANY_CODE"), "", "", xx, msg)) {
				ANS = 1;
			}
		}else{
			ANS = 1;
		}
	}
	//also check access to group (perhaps this should be checked first for speed)
	if (ANS) {
		if (not(authorised(positive ^ "MEDIA/PRODUCTION TYPE ACCESS " ^ RECORD.a(10).quote(), msg ))) {
			ANS = 0;
		}
	}
	return ANS;
}
libraryexit(authorised)


libraryinit(cost_accname)
function main() {
	return RECORD.a(6, 1, 2).xlate("ACCOUNTS", 1, "X");
}
libraryexit(cost_accname)


libraryinit(cost_acno2)
function main() {
	return RECORD.a(6, 1, 1);
}
libraryexit(cost_acno2)


libraryinit(cost_tax_acno2)
function main() {
	return RECORD.a(14, 1, 1);
}
libraryexit(cost_tax_acno2)


libraryinit(group_code)
function main() {
	ANS = RECORD.a(10);
	if (not ANS) {
		ANS = ID;
	}
	return ANS;
}
libraryexit(group_code)


libraryinit(group_name)
function main() {
	if (RECORD.a(10) == "") {
		return RECORD.a(1);
	}else{
		return RECORD.a(10).xlate("JOB_TYPES", 1, "X");
	}
}
libraryexit(group_name)


libraryinit(has_accs)
function main() {
	return (RECORD.a(5) ^ RECORD.a(6)) ne "";
}
libraryexit(has_accs)


libraryinit(income_accname)
function main() {
	return RECORD.a(5, 1, 2).xlate("ACCOUNTS", 1, "X");
}
libraryexit(income_accname)


libraryinit(income_account_seq)
function main() {
	return RECORD.a(5, 1, 2).xlate("ACCOUNTS", 26, "X");
}
libraryexit(income_account_seq)


libraryinit(income_acno2)
function main() {
	return RECORD.a(5, 1, 1);
}
libraryexit(income_acno2)


libraryinit(mjt)
function main() {
	if (RECORD.a(12)) {
		return "T";
	}
	if (RECORD.a(3) == "Y") {
		return "J";
	}else{
		return "M";
	}
}
libraryexit(mjt)


libraryinit(rawname)
function main() {
	return RECORD.a(1).ucase().convert("\'" " \".,/-", "");
}
libraryexit(rawname)


libraryinit(tax_acno2)
function main() {
	return RECORD.a(13, 1, 1);
}
libraryexit(tax_acno2)


libraryinit(year_period)
function main() {
	return "";
}
libraryexit(year_period)
