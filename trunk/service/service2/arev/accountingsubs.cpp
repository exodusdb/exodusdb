#include <exodus/library.h>
libraryinit()

#include <getacc.h>
#include <valacc.h>
#include <singular.h>

#include <gen.h>
#include <fin.h>
#include <win.h>

var msg;
var wsmsg;

function main() {
	//caserevised*
	//y2k
	if (mode.substr(1, 8) == "F2.ACCNO") {
		if (not gen.accounting.a(1)) {
			msg = "THE ACCOUNTING MODULE IS NOT AVAILABLE";
			goto EOF_386;
		}
		call getacc("", "", fin.currcompany, "", "", "");
		ANS = (ANS.a(1, 1, 1)).oconv("[ACNO]");

		goto 208;
	}
	if (mode.substr(1, 9) == "VAL.ACCNO") {
		if (not gen.accounting.a(1)) {
			return 0;
		}
		if (not(valacc("", win.is, win.isorig, "", msg))) {
			goto EOF_386;
		}
		win.is = win.is.a(1, 1, 1);
	}
L208:
	///////////
validaccs:
	///////////

	return 0;

}


libraryexit()