#include <exodus/library.h>

libraryinit(keywords)
//-------------------
function main() {
	ANS = RECORD.f(2);
	ANS.swapper(VM, ", ");
	ANS.swapper(SVM, ", ");
	return ANS;
}
/*pgsql
	ans := exodus_extract_text(DATA, 2, 0, 0);
	ans := replace(ans, VM, ', ');
	ans := replace(ans, SM, ', ');
*/
libraryexit(keywords)


libraryinit(text2)
//----------------
function main() {
	ANS = RECORD.f(3);
	var distrib = RECORD.f(5);
	distrib.converter(SVM, ":");
	if (distrib and not(distrib.index("User"))) {
		ANS.splicer(1, 0, distrib.field(":", 1) ^ ": ");
	}
	return ANS;
}
libraryexit(text2)

