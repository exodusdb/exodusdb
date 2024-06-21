#include <exodus/program.h>
programinit()

var syntax = R"--(
NAME
	convsyntax2  Convert various EXODUS cpp syntax
SYNTAX
	convsyntax2 SOURCEFILENAME ... {OPTIONS}
EXAMPLE
	conv_syntax2  *.cpp {dU}
OPTIONS
	d - Change single dimensioned array access syntax from (n) to [i]
	dd - Change single and multi dimensioned array access syntax from (i) and (i, j) to [i] and [i, j]
	U - Actually update the source file
	H - Display this help
)--";

function main() {

	// Report if [i, j] supported by the current compiler
	printx("Note: Multidimension array access using [i, j]");
#if __cpp_multidimensional_subscript >= 202110L
		printx(" IS ");
#else
		printx(" IS NOT ");
#endif
	printl("supported on the current compiler", _COMPILER, _COMPILER_VERSION);

	// Check syntax
	let filenames = COMMAND.remove(1);
	if (OPTIONS.contains("H") or not(OPTIONS.ucase() ne OPTIONS) or OPTIONS.convert("dHU", "") or not filenames) {
		abort(syntax);
	}

	// Determine options
	let update = OPTIONS.contains("U");
	let multidim = OPTIONS.contains("dd");
	if (multidim)
		printl("dd - Changing single and multi dimensioned array access syntax from (i) and (i, j) to [i] and [i, j]");
	else
		printl("d - Changing single dimensioned array access syntax from (n) to [i]");

	// Init
	var nfiles = 0;
	var nfiles_updateable = 0;

	// Work on each file in turn
	for (var filename : filenames) {

		var rec = osread(filename);
		let oldrec = rec;

		if (not rec)
			continue;

		nfiles++;

		// Find original dim lines to restore them after dumb conversion
		// dim aaa(bbb);
		var dimlines = rec.match(R"-(\bdim[ \t]+[a-z0-9_]+[ \t]*\([^;\n]+)-");

		// Find dim statements
		// dim aaa (
		// dim aaa =
		// dim aaa ;
		var dimnames = rec.match(R"-(\bdim[ \t]+[a-z0-9_]+[ \t]*[;(=])-");
//		TRACE(dimnames)
		dimnames = dimnames.match(R"-(\b[A-Za-z0-9_]+)-");
		dimnames = dimnames.regex_replacer("\\bdim\\b", "").sort().unique();

		for (var dimname : dimnames) {

			// Convert things like aaa(bbb) to aaa[bbb] where
			// aaa is the dim variable name and
			// bbb can be any expression variable name (a-aA-Z0-0_), digits, +, -, *, /, or comma
			// but NO parens
			// aaa(bbb * (ccc + 1)) is NOT converted.

			rec.regex_replacer(

				// replace aaa(bbb)
				// but not dim aaa(bbb);
				//R"# (([^d][^i][^m]\s+)\\b) #"
				"\\b"
				^ dimname

				// INCLUDE comma, or EXCLUDE comma to avoid converting multidimension access not supported
				^ (
					multidim ?
					R"-(\(([a-z0-9_\-+*/ ,.]+)\))-"
					:
					R"-(\(([a-z0-9_\-+*/ .]+)\))-"
				)

				//^ R"-(\[([a-z0-9_\-+*/ ,.]+)\])-"
				,

				// with aaa[bbb]
				dimname ^ R"-(\[\1\])-"
				//dimname ^ R"-(\(\1\))-"
				,
				"g"
			);
		}

		// Restore incorrect dumb conversion
		//  dim aaa[bbb]
		// back to original
		//  dim aaa(bbb)
		if (rec != oldrec) {
			//TRACE(dimlines)
			for (var dimline : dimlines) {
				rec.replacer(dimline.convert("()", "[]"), dimline);
			}
		}

		// Changes exist
		if (rec != oldrec ) {

			nfiles_updateable++;

			output(filename);
			if (update) {

				// Update the file
				if (oswrite(rec, filename))
					printl(" updated");

				else
					lasterror().errputl();
			} else {
				printl(" not updated. Use option {U}");
			}
		}
	}

	// Summary
	printl(nfiles, "files,", nfiles_updateable, "updated/updateable.");

	return 0;
}

programexit()
