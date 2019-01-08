/*
#include <exodus/dict.h>

dict(name_and_type) {
	ANS=RECORD(1)^"x";
	return ANS;
}
*/
#include <exodus/library.h>

libraryinit(name_and_type)
function main() {
	return RECORD(1) ^ " (" ^ RECORD(2) ^ ")";
}
libraryexit(name_and_type)

