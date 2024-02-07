#include <exodus/library.h>


libraryinit(department)
//---------------------
function main() {
	ANS = RECORD.f(3);
	// may be missing try to get from user file
	if (not ANS) {
		ANS = ID.field("*", 3).xlate("USERS", 5, "X");
	}
	// use database as better than blank/unknown
	if (not ANS) {
		if (ID.field("*", 3) == "EXODUS") {
			ANS = "EXODUS";
		} else {
			ANS = ID.field("*", 1);
		}
	}
	return ANS;
}
libraryexit(department)


libraryinit(last_access_time)
//---------------------------
function main() {
	return (RECORD.f(1) * 86400 + SW.f(1)).mod(86400);
}
libraryexit(last_access_time)


libraryinit(user_browser)
//-----------------------
function main() {
	let usercode = ID.field("*", 3);
	return usercode.xlate("USERS", 39, "X").f(1, 6);
}
libraryexit(user_browser)


libraryinit(user_code)
//--------------------
function main() {
	ANS = ID.field("*", 3);
	ANS.replacer(" ", "_");
	return ANS;
}
libraryexit(user_code)


libraryinit(user_name)
//--------------------
function main() {
	let usercode = ID.field("*", 3);
	ANS = usercode.xlate("USERS", 1, "X");
	if (ANS == "") {
		ANS = usercode;
		ANS.replacer(" ", "_");
	} else if (ANS != usercode) {
		ANS ^= " (" ^ usercode ^ ")";
	}
	return ANS;
}
libraryexit(user_name)
