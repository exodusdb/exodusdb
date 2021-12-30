#include <exodus/library.h>
libraryinit()

function main(in type, in inx, in mode, out outx) {
	//c sys in,in,in,out

	var status = 0;
	if (type eq "OCONV") {
		outx = "" ^ inx;
		var exp = outx.field("E", 2);
		if (exp.length()) {
			outx = outx.field("E",1);

			if (exp lt 0) {
				outx.splicer(2, 1, "");
				outx = ("." ^ var("0").str(-(exp + 1)) ^ outx).substr(1, 15);
				while (true) {
					///BREAK;
					if (not(outx.length() gt 2 and (outx[-1] eq "0"))) break;
					outx.popper();
				}//loop;

			} else if (exp gt 0) {
				outx.splicer(2, 1, "");

				if (outx.length() eq 1 + exp) {
				} else if (outx.length() gt 1 + exp) {
					outx.splicer(2 + exp, 1, ".");
				} else {
					outx ^= var("0").str(exp - outx.length());
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
