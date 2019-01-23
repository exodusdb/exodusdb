#include <exodus/library.h>
libraryinit()

function main(in type, io in0, io mode, out output, in glang="") {
	//c sys in,io,io,out,""
	//should really be sensitive to timezone in @SW
	//jbase
	if (in0 == "") {
		output = "";
		return 0;
	}
	var nospaces = mode.index("*");
	if (nospaces) {
		mode.converter("*", "");
	}
	if (mode) {
		if (mode == "4") {
			mode = DATEFORMAT;
			mode.splicer(2, 1, "4");
		}
	}else{
		if (DATEFORMAT) {
			mode = DATEFORMAT;
		}else{
			mode = "D2/E";
		}
	}
	//STATUS=0
	if (type == "OCONV") {
		//dont oconv 1 or 2 digits as they are probably day of month being converted
		// to proper dates
		//IF len(IN0) gt 2 and IN0 MATCHES '0N' OR IN0 MATCHES '"-"0N' OR IN0 MATCHES '0N.0N' THEN
		if ((in0.length() > 2) and in0.match("0N")) {
			goto ok;
		}
		if ((in0.match("\"-\"0N")) or in0.match("0N.0N")) {
ok:
			//language specific (date format could be a pattern in lang?)
			if (mode == "L") {
				var tt = in0.oconv("D4/E");
				var mth = glang.a(2).field("|", tt.field("/", 2));
				var day = tt.field("/", 1);
				var year = tt.field("/", 3);
				//if 1 then
				output = day ^ " " ^ mth ^ " " ^ year;
				//end else
				// output=mth:' ':day:' ':year
				// end
			}else{
				output = in0.oconv(mode);
				if (output[1] == "0") {
					output.splicer(1, 1, " ");
				}
				if (output[4] == "0") {
					output.splicer(4, 1, " ");
				}
				if (output.substr(4,3).match("3A")) {
					output.splicer(7, 1, "");
					output.splicer(3, 1, "");
				}
				if (nospaces) {
					output.converter(" ", "");
				}
			}

		}else{
			output = in0;
		}
	} else if (type == "ICONV") {
		if (in0.match("0N") and (in0 <= 31)) {
			in0 ^= var().date().oconv("D").substr(4,9);
		}
		output = in0.iconv(mode);
	}
	return 0;

}


libraryexit()
