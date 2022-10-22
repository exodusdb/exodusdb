#include <exodus/library.h>
libraryinit()

function main(in instr, in sepchars, in stoplist, in mode) {

	//split into words by separators
	var outstr = instr.convert(sepchars, str(VM, sepchars.len()));

	//remove stop words
	if (mode) {
		var xx;
		for (var ii = outstr.fcount(VM); ii > 0; --ii) {
			if (stoplist.locate(outstr.f(1, ii), xx)) {
				outstr.remover(1, ii);
			}
		}
	}

	return 0;
}

libraryexit()
