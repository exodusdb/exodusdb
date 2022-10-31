#include <exodus/library.h>
libraryinit()

var list1n;
var xx;

function main(in /*mode*/, io list1, io list2, in seps) {

	if (not(list1.len() and list2.len())) {
		return 0;
	}

	list1.converter(seps, _FM _FM _FM _FM _FM _FM _FM _FM _FM _FM);
	list2.converter(seps, _FM _FM _FM _FM _FM _FM _FM _FM _FM _FM);
	var alllist2 = list2;

	// remove matches from both
	var nlist2 = list2.fcount(FM);
	for (var list2n = nlist2; list2n >= 1; --list2n) {
		if (list1.locateusing(FM, list2.f(list2n), list1n)) {
			list1.remover(list1n);
			list2.remover(list2n);
		}
	}  // list2n;

	// any list1 that are in origlist2 are duplicates - consider as not removed
	var nlist1 = list1.fcount(FM);
	for (list1n = nlist1; list1n >= 1; --list1n) {
		if (alllist2.locateusing(FM, list1.f(list1n), xx)) {
			list1.remover(list1n);
		}
	}  // list1n;

	list1.converter(FM, seps[1]);
	list2.converter(FM, seps[1]);

	return 0;
}

libraryexit()
