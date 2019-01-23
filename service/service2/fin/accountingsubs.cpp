#include <exodus/library.h>
libraryinit()

#include <getacc.h>
#include <valacc.h>
#include <singular.h>
#include <authorised.h>

#include <gen.h>
#include <fin.h>
#include <win.h>

#include <window.hpp>

var msg;
var wsmsg;

function main(in mode) {
	//c fin

	if (mode.substr(1,8) == "F2.ACCNO") {
		if (not(gen.accounting.a(1))) {
			msg = "THE ACCOUNTING MODULE IS NOT AVAILABLE";
			gosub invalid(msg);
			return 0;
		}
		call getacc("", "", fin.currcompany, "", "", "");
		ANS = ANS.a(1, 1, 1).oconv("[ACNO]");

	} else if (mode.substr(1,9) == "VAL.ACCNO") {
		if (not(gen.accounting.a(1))) {
			return 0;
		}
		if (not(valacc("", win.is, win.isorig, "", msg))) {
			gosub invalid(msg);
			return 0;
		}
		//IS=IS<1,1,1>
	}
//L205:

}


libraryexit()
