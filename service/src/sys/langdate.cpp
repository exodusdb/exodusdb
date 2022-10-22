#include <exodus/library.h>
libraryinit()

function main(in type, in in0, in mode0, out output, in glang) {

	// should really be sensitive to timezone in @SW

	var inx	 = in0;
	var mode = mode0;

	if (inx eq "") {
		output = "";
		return 0;
	}

	var nospaces = mode.contains("*");
	if (nospaces) {
		mode.converter("*", "");
	}

	if (mode) {

		if (mode eq "4") {
			mode = DATEFMT;
			mode.paster(2, 1, "4");
		}

	} else {

		if (DATEFMT) {
			mode = DATEFMT;
		} else {
			mode = "D2/E";
		}
	}

	// status=0
	if (type eq "OCONV") {

		// dont oconv 1 or 2 digits as they are probably day of month being converted
		// to proper dates
		// IF len(inx) gt 2 and inx MATCHES '0N' OR inx MATCHES '"-"0N' OR inx MATCHES '0N.0N' THEN
		if (inx.len() gt 2 and inx.match("^\\d*$")) {
			goto ok;
		}

		if (inx.match("^-\\d*$") or inx.match("^\\d*\\.\\d*$")) {
ok:
			// language specific (date format could be a pattern in lang?)
			if (mode eq "L") {

				var tt	 = inx.oconv("D4/E");
				var mth	 = glang.f(2).field("|", tt.field("/", 2));
				var day	 = tt.field("/", 1);
				var year = tt.field("/", 3);
				// if 1 then
				output = day ^ " " ^ mth ^ " " ^ year;
				// end else
				// output=mth:' ':day:' ':year
				// end

			} else {

				output = oconv(inx, mode);

				if (output.starts("0")) {
					output.paster(1, 1, " ");
				}

				if (output[4] eq "0") {
					output.paster(4, 1, " ");
				}

				if (output.b(4, 3).match("^[A-Za-z]{3}$")) {
					output.paster(7, 1, "");
					output.paster(3, 1, "");
				}

				if (nospaces) {
					output.converter(" ", "");
				}
			}

		} else {
			output = inx;
		}

	} else if (type eq "ICONV") {

		if (inx.match("^\\d*$") and inx le 31) {
			inx ^= date().oconv("D").b(4, 9);
		}

		output = iconv(inx, mode);
	}

	return 0;
}

libraryexit()
