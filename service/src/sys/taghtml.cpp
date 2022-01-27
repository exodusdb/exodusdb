#include <exodus/library.h>
libraryinit()

var xx;

function main(in type, in in0, in mode, out output) {
	//c sys in,in,in,out

	//prevent c++ var not used
	if (var(0) or type) {
		{}
	}

	output = in0;
	if (output eq "") {
		var temp = mode.substr(1, 2);
		temp.ucaser();
		if (temp eq "TD" or mode eq "TH") {
			output = "&nbsp;";
		}
	}
	if (output eq "") {
		return 0;
	}

	if (output[1] eq "-") {
		if (mode eq "TD") {
			if (var("1234567890").index(output[2])) {
				output = "<NOBR>" ^ output ^ "</NOBR>";
			}
		}
	}

	//convert "&" to &amp;
	for (const var ii : range(1, 99999)) {
		var charn = output.index("&", ii);

		///BREAK;
		if (not charn) break;

		if (not(var("nbsp,amp,lt,gt,infin").locateusing(",", (output.substr(charn + 1, 99999)).field(";", 1), xx))) {
			output.splicer(charn, 1, "&amp;");
		}

	} //ii;

	//swap '<' with '&lt;' in output
	//swap '>' with '&gt;' in output

	if (mode ne "") {
		output = "<" ^ mode ^ ">" ^ output ^ "</" ^ mode.field(" ", 1) ^ ">";
	}

	return 0;
}

libraryexit()
