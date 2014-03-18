#include <exodus/library.h>
libraryinit()

#include <getacc.h>
#include <valacc.h>
#include <singular.h>

#include <gen.h>
#include <fin.h>
#include <win.h>

#include <window.hpp>//after win

var msg;
var wsmsg;

function main(in mode) {

	printl("Not implemented yet - ACCOUNTINGSUBS,"^mode);
	/*
	//caserevised*
	//y2k
	if (mode.substr(1, 8) == "F2.ACCNO") {
		if (not gen.accounting.a(1)) {
			msg = "THE ACCOUNTING MODULE IS NOT AVAILABLE";
			return invalid(msg);
		}
 		//var getacc(io mode, io validaccs, io reqcompanies2, io acctype, io reqledgertype, io getacc2, io inpaccnos)
 		call getacc("", "", fin.currcompany, "", "", "","");
		ANS = (ANS.a(1, 1, 1)).oconv("[ACNO]");

	} else if (mode.substr(1, 9) == "VAL.ACCNO") {
		if (not gen.accounting.a(1)) {
			return 0;
		}
		if (not(valacc("", win.is, win.isorig, "", msg))) {
			return invalid(msg);
		}
		win.is = win.is.a(1, 1, 1);
	}

	*/

	return 0;

}


libraryexit()
