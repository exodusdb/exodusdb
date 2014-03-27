#include <exodus/library.h>
libraryinit()

function main(in instr, in sepchars, in stoplist, in mode) {

	//split into words by separators
	var outstr=instr.convert(sepchars,str(VM,sepchars.length()));

	//remove stop words
	if (mode) {
		var xx;
		for (var ii=outstr.dcount(VM);ii>0;--ii) {
			if (stoplist.locate(outstr.a(1,ii),xx,1)) {
				outstr.eraser(1,ii);
			}
		}
	}

	return 0;
}

libraryexit()
