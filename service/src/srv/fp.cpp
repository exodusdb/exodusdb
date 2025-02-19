#include <exodus/library.h>
libraryinit()

function main(in type, in inx, in /*mode*/, out outx) {

	let status = 0;
	if (type == "OCONV") {
		outx	= "" ^ inx;
		let exp = outx.field("E", 2);
		if (exp.len()) {
			outx = outx.field("E", 1);

			if (exp < 0) {
				outx.paster(2, 1, "");
				outx = ("." ^ var("0").str(-(exp + 1)) ^ outx).first(15);
				// 				while (true) {
				// 					// /BREAK;
				// 					if (not(outx.len() > 2 and (outx.ends("0")))) break;
				while (outx.len() > 2 and outx.ends("0")) {
					outx.popper();
				}

			} else if (exp > 0) {
				outx.paster(2, 1, "");

				if (outx.len() == 1 + exp) {
				} else if (outx.len() > 1 + exp) {
					outx.paster(2 + exp, 1, ".");
				} else {
					outx ^= var("0").str(exp - outx.len());
				}
			}
		}
		//  out=out '[number]'

	} else if (type == "ICONV") {
		outx = inx;
	}

	return 0;
}

libraryexit()
