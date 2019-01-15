#include <exodus/library.h>
libraryinit()

var list1n;
var xx;

function main(in mode, io list1, io list2, in seps) {
	//c sys in,io,io,in

	if (mode.unassigned()) {
		{}
	}

	if (not(list1.length() and list2.length())) {
		return 0;
	}

	list1.converter(seps, "\xFE\xFE\xFE\xFE\xFE\xFE\xFE\xFE\xFE\xFE");
	list2.converter(seps, "\xFE\xFE\xFE\xFE\xFE\xFE\xFE\xFE\xFE\xFE");
	var alllist2 = list2;

	//remove matches from both
	var nlist2 = list2.count(FM) + 1;
	for (var list2n = nlist2; list2n >= 1; --list2n) {
		if (list1.locateusing(list2.a(list2n), FM, list1n)) {
			list1.eraser(list1n);
			list2.eraser(list2n);
		}
	};//list2n;

	//any list1 that are in origlist2 are duplicates - consider as not removed
	var nlist1 = list1.count(FM) + 1;
	for (list1n = nlist1; list1n >= 1; --list1n) {
		if (alllist2.locateusing(list1.a(list1n), FM, xx)) {
			list1.eraser(list1n);
		}
	};//list1n;

	list1.converter(FM, seps[1]);
	list2.converter(FM, seps[1]);

	return 0;

}


libraryexit()
