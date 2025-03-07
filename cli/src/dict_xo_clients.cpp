/*
#include <exodus/dict.h>

dict(name_and_type) {
	ANS=RECORD(1)^"x";
	return ANS;
}
*/
#include <exodus/library.h>

libraryinit(name_and_type)
//--------------------
function main() {
	ANS = RECORD.f(1) ^ " (" ^ RECORD.f(2) ^ ")";
	return ANS;	 //.outputl("ANS=");
}
//--------------------
libraryexit(name_and_type)


libraryinit(name_and_code)
//--------------------
function main() {
	ANS = RECORD.f(1) ^ " (" ^ ID ^ ")";
	return ANS;	 //.outputl("ANS=");
}
//--------------------
libraryexit(name_and_code)

libraryinit(ndecs)
//--------------------
function main() {
	ANS = 2;
	return ANS;
}
//--------------------
libraryexit(ndecs)
