#include <exodus/library.h>


libraryinit(secs)
//---------------
function main() {
	let responsetime = RECORD.f(5);
	if (responsetime.len()) {
		ANS = RECORD.f(5) - ID.field("*", 3);
		if (ANS < 0) {
			ANS += 86400;
		}
		ANS = (ANS.oconv("MD30P")) + 0;
	} else {
	// blank if no response time (crashed)
		ANS = "";
	}
	return ANS;
}
libraryexit(secs)


libraryinit(seq)
//--------------
function main() {
	return ID;
	return ANS;
}
/*pgsql
	return key;
*/
libraryexit(seq)
