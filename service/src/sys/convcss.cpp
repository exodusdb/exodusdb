#include <exodus/library.h>
libraryinit()

function main(in mode, in tableid, in in0, out outx) {
	//c sys in,in,in,out

	//convert <COL ALIGN=RIGHT> to nth-child style

	//evade c++ variable not used warning
	if (not(mode.unassigned())) {
		{}
	}

	var inp = in0.trim().lcase();

	var crlf = "\r\n";

	outx = "\n<style type=\"text/css\">\n";
	var align = "left";

	let ncols = inp.count(VM) + 1;
	for (const var coln : range(1, ncols)) {
		var tt = inp.f(1, coln);

		if (tt.contains("left")) {
			align = "left";
		}
		if (tt.contains("right")) {
			align = "right";
		}
		if (tt.contains("center")) {
			align = "center";
		}

		var otherstyle = tt.field(DQ, 2);
		otherstyle.converter("{}", "");
		otherstyle.trimmer();
		if (not otherstyle.starts(";")) {
			otherstyle.paster(1, 0, ";");
		}
		if (not otherstyle.ends(";")) {
			otherstyle ^= ";";
		}

		//> means dont inherit to subtables
		//td
		outx ^= "table#" ^ tableid ^ " > tbody > tr > td:nth-child(" ^ coln ^ "){text-align:" ^ align ^ otherstyle ^ "}\n";
		//th
		outx ^= "table#" ^ tableid ^ " > tbody > tr > th:nth-child(" ^ coln ^ "){text-align:" ^ align ^ otherstyle ^ "}\n";
	} //coln;

	outx ^= "</style>\n";

	return 0;
}

libraryexit()
