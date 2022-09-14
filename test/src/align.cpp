#include <exodus/program.h>
programinit()

function main() {

	var token = " eq ";

	var update_inplace = OPTIONS.contains("U");

	var comment_regex = "^\\s*/";

	var osfilenames = COMMAND.remove(1);
	for (in osfilename : osfilenames) {

		dim text;
		if (not text.osread(osfilename))
			abort(fsmsg());

		var maxprintcoln = 0;
		var block_startln = 0;
		var ln = 0;
		var updated = false;

		auto indenter = [&]() {
			for (var ln : range(block_startln, ln)) {
				var coln = text(ln).index(token);

				// Dont update // comment lines
				// Dont update lines not ending in ;
				// Dont update lines if the token is in double quotes
				// (there are an odd number of double quotes before the token
				if (not text(ln).match(comment_regex) and text(ln)[-1] == ";" and text(ln).substr(1,coln).count(DQ).mod(2) == 0) {
					var printcoln = textlen(text(ln).substr(1, coln - 1)) + 1;
					if (coln and printcoln < maxprintcoln) {
						text(ln).splicer(coln, 0, space(maxprintcoln-printcoln));
						updated = true;
						//printl(text(ln));
					}
				}
			}
			maxprintcoln = 0;
			block_startln = 0;
		};

		for (in line : text) {

			if (not line)
				indenter();

			ln ++;

			var coln = line.index(token);
			// Skip lines starting with white space and a slash
			if (coln and not line.match(comment_regex)) {
				var printcoln = textlen(line.substr(1, coln - 1)) + 1;
				if (printcoln > maxprintcoln)
					maxprintcoln = printcoln;
				if (not block_startln)
					block_startln = ln;
			}
		}

		indenter();

		if (update_inplace) {
			if (updated) {
				osfilename.outputl("Updated ");
				if (not text.oswrite(osfilename))
					abort(fsmsg());
			}
		} else {
			for (in line : text)
				printl(line);
		}

	}

	return 0;
}

programexit()

