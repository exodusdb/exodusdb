#include <exodus/library.h>
libraryinit()

var xx;

function main(in /*type*/, in in0, in mode, out output) {

	output = in0;
	if (output eq "") {
		var temp = mode.first(2);
		temp.ucaser();
		if (temp eq "TD" or mode eq "TH") {
			output = "&nbsp;";
		}
	}
	if (output eq "") {
		return 0;
	}

	if (output.starts("-")) {
		if (mode eq "TD") {
			if (var("1234567890").contains(output[2])) {
				output = "<NOBR>" ^ output ^ "</NOBR>";
			}
		}
	}

	var precoded = _VM "nbsp" _VM "amp" _VM "lt" _VM "gt" _VM "infin" _VM;
	for (var pos = 1; ; pos++) {
		pos = output.index("&", pos);
		if (not pos)
			break;
		var ss = _VM ^ output.b(pos + 1, 10).field(";", 1) ^ _VM;
		if (not precoded.contains(ss)) {
			output.paster(pos + 1, "amp;");
			pos += 4;
		}
	}

	//swap '<' with '&lt;' in output
	//swap '>' with '&gt;' in output

	if (mode ne "") {
		output = "<" ^ mode ^ ">" ^ output ^ "</" ^ mode.field(" ", 1) ^ ">";
	}

	return 0;
}

//subroutine test() {
//	// test oconv [TAGHTML]
//	var text = "&nbsp" _VM "&amp" _VM "&lt" _VM "&gt" _VM "&infin" _VM;
//	assert(oconv(text,"[TAGHTML,XYZ]").convert(VM,"]") eq "<XYZ>&nbsp</XYZ>]<XYZ>&amp</XYZ>]<XYZ>&lt</XYZ>]<XYZ>&gt</XYZ>]<XYZ>&infin</XYZ>]");
//
//	var text2 =  "&&___&amp;___&lt;___&gt&gt___&&";
//	assert(oconv(text2,"[TAGHTML,XYZ]").convert(VM,"]").outputl() eq "<XYZ>&amp;&amp;___&amp;___&lt;___&amp;gt&amp;gt___&amp;&amp;</XYZ>");
//}

libraryexit()
