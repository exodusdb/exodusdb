#include <exodus/program.h>
programinit()

/*

 convr2a  Convert EXODUS cpp "r()" style replacement syntax to assign

 Syntax:  convr2a SOURCEFILENAME ... {OPTION}

 Example: conv_r2a ~/neosys/src/ASTERISK/ASTERISK.cpp {U}

 OPTION   U - Update the source file

 Converts  x.r(fn, replacement);         to x(fn)         = replacement;
 Converts  x.r(fn, vn, replacement);     to x(fn, vn)     = replacement;
 Converts  x.r(fn, vn, sn, replacement); to x(fn, vn, sn) = replacement;

*/

function main() {

	COMMAND.remover(1);

	for (var filename : COMMAND) {

		if (not filename)
			continue;

		// Read a source file into a dimensioned array of vars
		dim txt;
		if (not txt.osread(filename))
			abort(lasterror());

		bool replaced = false;

		// For each text line in the source file
		for (var& line : txt) {

			var pos = line.index(".r(");
			if (pos) {

				// Hide commas inside brackets and quotes so we can use the remaining 1, 2 or 3 commas to discover fn, vn, sn and replacement
				var line2 = line;
				pos += 3; //skip over first paren
				pos -=1;
				var depth = 0;
				bool inquotes = false;
				while (++pos <= line.len()) {

					char c = line[pos].toChar();

					if (c == '(' and not inquotes)
						depth++;

					else if (c == ')' and not inquotes)
						depth--;

					else if (c == '\\')
						pos++;

					else if (c == '"')
						inquotes = not(inquotes);

					else if (c == ',' and (inquotes or depth))
						line2.splicer(pos, 1, FM_);
				}

				//x(fn) = y;
				line2.replacer(
					"([\\w()]+)"	// (1) A variable or variable with brackets containing a variable
					"\\.r\\("		// The .r( syntax
					"([^,]+)"		// (2) Everything up to the only comma is the FN
					", ?"			// ,
					"([^,]+)"		// (3) Everything else if no comma is the replacement
					"\\);"			// The trailing );
				, R"(\1(\2) = \3;)");

				//x(fn, vn) = y;
				line2.replacer(
					"([\\w()]+)"	// (1) A variable or variable with brackets containing a variable
					"\\.r\\("		// The .r( syntax
					"([^,]+)"		// (2) Everything up to the first comma is the FN
					", ?"			// ,
					"([^,]+)"		// (3) Everything up to the second comma is the VN
					", ?"			// ,
					"([^,]+)"		// (4) Everything else if no comma is the replacement
					"\\);"			// The trailing );
				, R"(\1(\2, \3) = \4;)");

				//x(fn, vm , sn) = y;
				line2.replacer(
					"([\\w()]+)"	// (1) A variable or variable with brackets containing a variable
					"\\.r\\("		// The .r( syntax
					"([^,]+)"		// (2) Everything up to the first comma is the FN
					", ?"			// ,
					"([^,]+)"		// (3) Everything up to the second comma is the VN
					", ?"			// ,
					"([^,]+)"		// (4) Everything up to the third comma is the SN
					", ?"			// ,
					"([^,]+)"		// (5) Everything else if no comma is the replacement
					"\\);"			// The trailing );
				, R"(\1(\2, \3, \4) = \5;)");


				if (line2.index(".r(")) {

					// Failed to convert
					printl(line2);

				} else {

					//printl("-", line);

					// Restore the hidden commas
					line = line2.convert(FM_, ",");

					replaced = true;
					printl("+", line);
				}
			}

		}

		// Update the source file
		if (replaced && OPTIONS.index("U"))
			txt.oswrite(filename);
	}

	return 0;
}

programexit()
