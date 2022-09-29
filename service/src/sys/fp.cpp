#include <exodus/library.h>
libraryinit()

function main(in type, in inx, in mode, out outx) {

	var status = 0;
	if (type eq "OCONV") {
		outx = "" ^ inx;
		var exp = outx.field("E", 2);
		if (exp.len()) {
			outx = outx.field("E",1);

			if (exp lt 0) {
				outx.paster(2, 1, "");
				outx = ("." ^ var("0").str(-(exp + 1)) ^ outx).first(15);
//				while (true) {
//					///BREAK;
//					if (not(outx.len() gt 2 and (outx.ends("0")))) break;
				while (outx.len() gt 2 and outx.ends("0")) {
					outx.popper();
				}

			} else if (exp gt 0) {
				outx.paster(2, 1, "");

				if (outx.len() eq 1 + exp) {
				} else if (outx.len() gt 1 + exp) {
					outx.paster(2 + exp, 1, ".");
				} else {
					outx ^= var("0").str(exp - outx.len());
				}

			}
		}
	//  out=out '[number]'

	} else if (type eq "ICONV") {
		outx = inx;

	} else if (0) {
		//evade c++ compiler warning
		if (mode) {
			{}
		}
	}

	return 0;
}

libraryexit()
