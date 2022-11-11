#include <exodus/library.h>
libraryinit()

var xx;

function main(in /*type*/, in in0, in mode, out output) {

	output = in0;
	if (output == "") {
		var temp = mode.first(2);
		temp.ucaser();
		if (temp == "TD" or mode == "TH") {
			output = "&nbsp;";
		}
	}
	if (output == "") {
		return 0;
	}

	if (output.starts("-")) {
		if (mode == "TD") {
			if (var("1234567890").contains(output[2])) {
				output = "<NOBR>" ^ output ^ "</NOBR>";
			}
		}
	}

	let precoded = _VM "nbsp" _VM "amp" _VM "lt" _VM "gt" _VM "infin" _VM;
	for (var pos = 1;; pos++) {
		pos = output.index("&", pos);
		if (not pos)
			break;
		let ss = _VM ^ output.b(pos + 1, 10).field(";", 1) ^ _VM;
		if (not precoded.contains(ss)) {
			output.paster(pos + 1, "amp;");
			pos += 4;
		}
	}

	// swap '<' with '&lt;' in output
	// swap '>' with '&gt;' in output

	if (mode != "") {
		output = "<" ^ mode ^ ">" ^ output ^ "</" ^ mode.field(" ", 1) ^ ">";
	}

	return 0;
}

// subroutine test() {
//	// test oconv [TAGHTML]
//	let text = "&nbsp" _VM "&amp" _VM "&lt" _VM "&gt" _VM "&infin" _VM;
//	assert(oconv(text,"[TAGHTML,XYZ]").convert(VM,"]") == "<XYZ>&nbsp</XYZ>]<XYZ>&amp</XYZ>]<XYZ>&lt</XYZ>]<XYZ>&gt</XYZ>]<XYZ>&infin</XYZ>]");
//
//	let text2 =  "&&___&amp;___&lt;___&gt&gt___&&";
//	assert(oconv(text2,"[TAGHTML,XYZ]").convert(VM,"]").outputl() == "<XYZ>&amp;&amp;___&amp;___&lt;___&amp;gt&amp;gt___&amp;&amp;</XYZ>");
//}

libraryexit()
