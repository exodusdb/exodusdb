#include <exodus/library.h>

libraryinit(keywords)
//-------------------
function main() {
	ANS = RECORD.a(2);
	ANS.swapper(VM, ", ");
	ANS.swapper(SVM, ", ");
	return ANS;
}
libraryexit(keywords)

libraryinit(text2)
//----------------
function main() {
	ANS = RECORD.a(3);
	var distrib = RECORD.a(5);
	distrib.converter(SVM, ":");
	if (distrib and not(distrib.index("User"))) {
		ANS.splicer(1, 0, distrib.field(":", 1) ^ ": ");
	}
	return ANS;
}
libraryexit(text2)
