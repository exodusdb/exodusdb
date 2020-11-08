#include <exodus/library.h>

libraryinit(conversion)
//---------------------
function main() {
	if (calculate("EQUIVALENT").length()) {
		ANS = 1 / calculate("EQUIVALENT");
	}
	return ANS;
}
libraryexit(conversion)

libraryinit(equivalents)
//----------------------
function main() {
	var nunits = RECORD.a(3).count(VM) + (RECORD.a(3) ne "");
	ANS = "";
	for (var unitn = 1; unitn <= nunits; ++unitn) {
		ANS.r(1, unitn, "= " ^ oconv(RECORD.a(4, unitn), "[FP]") ^ " " ^ RECORD.a(3, unitn));
	};//unitn;
	return ANS;
}
libraryexit(equivalents)

libraryinit(ndecs)
//----------------
#include <fin_common.h>

function main() {
	#include <common.h>
	return fin.basefmtx[3];
}
libraryexit(ndecs)

libraryinit(other_description)
//----------------------------
function main() {
	return calculate("RELATED_ABBREVIATION").xlate("UNITS", 1, "X");
}
libraryexit(other_description)

libraryinit(to_1)
//---------------
function main() {
	return calculate("ABBREVIATION");
}
libraryexit(to_1)

libraryinit(unit_type_break)
//--------------------------
function main() {
	ANS = RECORD.a(2);
	if (ANS ne RECUR0) {
		printl();
		printl();
		var temp = oconv(ANS, "[UNIT.TYPE]") ^ " :";
		temp.ucaser();
		printl(temp);
		printl(var("-").str(temp.length()));
		printl();
		RECUR0 = ANS;
	}
	return ANS;
}
libraryexit(unit_type_break)
