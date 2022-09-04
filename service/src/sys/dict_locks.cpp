#include <exodus/library.h>

libraryinit(key2)
//---------------
function main() {
	return ID.field("*", 2, 999);
}
libraryexit(key2)


libraryinit(lock_expired)
//-----------------------
var dostime;//num

function main() {
	dostime = ostime();
	//convert to Windows based date/time (ndays since 1/1/1900)
	//31/12/67 in rev date() format equals 24837 in windows date format
	dostime = 24837 + var().date() + dostime / 24 / 3600;
	return RECORD.f(1) le dostime;
}
libraryexit(lock_expired)

