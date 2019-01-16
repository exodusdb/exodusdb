#include <exodus/library.h>
libraryinit()


var inp;
var crlf;
var align;
var ncols;
var coln;//num
var tt;
var otherstyle;

function main(in mode, in tableid, in in0, out outx) {
	//c sys in,in,in,out

	//convert <COL ALIGN=RIGHT> to nth-child style

	//evade c++ variable not used warning
	if (not(mode.unassigned())) {
		{}
	}

	inp = in0.trim().lcase();

	crlf = "\r\n";

	outx = crlf ^ "<style type=\"text/css\">" ^ crlf;
	align = "left";

	ncols = inp.count(VM) + 1;
	for (coln = 1; coln <= ncols; ++coln) {
		tt = inp.a(1, coln);

		if (tt.index("left", 1)) {
			align = "left";
		}
		if (tt.index("right", 1)) {
			align = "right";
		}
		if (tt.index("center", 1)) {
			align = "center";
		}

		otherstyle = tt.field(DQ, 2);
		otherstyle.converter("{}", "");
		otherstyle.trimmer();
		if (otherstyle[1] ne ";") {
			otherstyle.splicer(1, 0, ";");
		}
		if (otherstyle[-1] ne ";") {
			otherstyle ^= ";";
		}

		//> means dont inherit to subtables
		//td
		outx ^= "table#" ^ tableid ^ " > tbody > tr > td:nth-child(" ^ coln ^ "){text-align:" ^ align ^ otherstyle ^ "}" ^ crlf;
		//th
		outx ^= "table#" ^ tableid ^ " > tbody > tr > th:nth-child(" ^ coln ^ "){text-align:" ^ align ^ otherstyle ^ "}" ^ crlf;
	};//coln;

	outx ^= "</style>" ^ crlf;

	return 0;

}


libraryexit()
