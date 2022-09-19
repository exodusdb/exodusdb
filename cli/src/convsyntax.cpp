#include <exodus/program.h>
programinit()

var syntax = R"(
NAME
 convsyntax  Convert various EXODUS cpp syntax
SYNTAX
	convsyntax SOURCEFILENAME ... {OPTION}
EXAMPLE
	conv_syntax ~/neosys/src/ASTERISK/ASTERISK.cpp {RF}
OPTIONS
	R - Change replacer syntax
		   x.r(a,b,c,y)
		-> x(a,b,c) = y
		Examples:
		   x.r(fn, replacement);
		-> x(fn) = replacement;
		   x.r(fn, vn, replacement);
		-> x(fn, vn) = replacement;
		   x.r(fn, vn, sn, replacement);
		-> x(fn, vn, sn) = replacement;
	F - Change for loop syntax to use range()
		   for (const var x = ...
		-> for (var x : range(...
		Example:
		for (const var x = a; x <= b; ++x)
		-> for (var x : range(a to b))
	B - Change substr to b
		   .substr(
		-> .b(
	S - Change .b(1, n) eq/== to .starts(
	    and    '.b(n, n)' to .first(n)
		   .b(1, n) eq "XXX"
		-> .starts("XXX")
		   .b(3, 3)
		-> .first(3)
	L - Change '.b(-n, n) eq' to .ends(n)
	    and    '.b(-n, n)' to .last(n)
		   .b(1, n) eq "XXX"
		-> .starts("XXX")
		   .b(-3, 3)
		-> .last(3)
	C - Cut
		   .b(2)
		-> .cut(1)
		...
		   .b(10)
		-> .cut(9)
	[] - 

	G - General
	v - Remove empty var().

	A - All conversions except R and F

	U - Actually update the source file
	V - Verbose
)";

function main() {

	if (not OPTIONS)
		abort(syntax);

	COMMAND.remover(1);

	if (OPTIONS.contains("A"))
		OPTIONS ^= "BSLCGv[]";
	var r2a = OPTIONS.contains("R") ;
	var forrange = OPTIONS.contains("F");
	var substr2b = OPTIONS.contains("B");
	var b1eq2starts = OPTIONS.contains("S");
	var b2last = OPTIONS.contains("L");
	var general = OPTIONS.contains("G");
	var emptyvar = OPTIONS.contains("v");
	var cut = OPTIONS.contains("C");
	var onechar = OPTIONS.contains("[]");

	var verbose = OPTIONS.contains("V");

	for (var osfilename : COMMAND) {

		if (not osfilename
			or osfilename.contains("convsyntax")
			or osfilename.ends(".so")
			or osdir(osfilename)
			or osfilename.starts(".")
		)
			continue;

		if (verbose)
			logputl(osfilename);

		// Read a source file into a dimensioned array of vars
		dim txt;
		if (not txt.osread(osfilename))
			abort(lasterror());

		bool replaced = false;

		// For each text line in the source file
		// Using var& instead of var allows us to update lines directly into dim txt()
		for (var& line : txt) {

			var pos = 0;

			// r2a - xxx.r(a,b,c,d) -> =xxx(1,b,c) = d;
			///////////////////////////////////////////
			if (r2a)
				pos = line.index(".r(");
			if (r2a and pos) {

				var line2 = hide_subsyntax(line, pos, ',');

				//x(fn) = y;
				line2.regex_replacer(
					"([\\w()]+)"	// (1) A variable or variable with brackets containing a variable
					"\\.r\\("		// The .r( syntax
					"([^,]+)"		// (2) Everything up to the only comma is the FN
					", ?"			// ,
					"([^,]+)"		// (3) Everything else if no comma is the replacement
					"\\);"			// The trailing );
				, R"(\1(\2) = \3;)");

				//x(fn, vn) = y;
				line2.regex_replacer(
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
				line2.regex_replacer(
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


				if (line2.contains(".r(")) {

					// Failed to convert
					errputl('?', line2);

				} else {

					//printl("-", line);

					line = restore_subsyntax(line2, ',');

					replaced = true;
					printl('+', line);
				}

				continue;

			}// r2a

			// r2a - xxx.r(a,b,c,d) -> =xxx(1,b,c) = d;
			var pattern = "for (var ";
			if (forrange)
				pos = line.index(pattern);
			if (forrange and pos) {

				//var line2 = hide_subsyntax(line, pos, ';');
				var line2 = line;
				bool fail = false;

				//for (const var dictidn : range(1, ndictids)) {

				// for (var dictidn = 1
				var part1 = line2.b(pos).field(";", 1).trim();
				var varname = part1.field(" ", 3);
				var starting = part1.field("=", 2, 999).trim();

				// dictidn <= ndictids + 20
				var part2 = line2.field(";", 2).trim();
				var varname2 = part2.field(" ", 1).trim();
				if (varname2 ne varname)
					fail = true;
				var compare = part2.field(" ", 2);
				if (compare ne "<=" and compare ne "le")
					fail = true;
				var ending = part2.field(" ", 3, 999);

				// ++dictidn) {
				var part3 = line2.field(";", 3).trim();
				if (part3 ne ("++" ^ varname ^ ") {") and part3 ne (varname ^ "++) {"))
					fail = true;

				// Cannot really occur since part3 is checked for ++varname
				if (starting.isnum() and ending.isnum() and starting > ending) {
//					TRACE(line2)
//					TRACE(starting)
//					TRACE(ending)
					fail = true;
				}

				//printl("part1 :", part1.quote());
				//printl("part2 :", part2.quote());
				//printl("part3 :", part3.quote());
				//printl("varname :", varname.quote(), ",", varname2.quote(), "starting :", starting.quote(), "compare :" ^ compare.quote(), "ending :", ending.quote());
				if (not fail) {
					line2.splicer(pos, 99999, "for (const var " ^ varname ^ " : range(" ^ starting ^ ", " ^ ending ^ ")) {");
				}

				if (not line2.contains(" : range(")) {

					// Failed to convert
					if (line2.contains(";"))
						errputl('?', line2.replace("\t", "    "));

				} else {

					//printl("-", line);

					//line = restore_subsyntax(line2, ',');
					if (line2 ne line) {
						line = line2;
						replaced = true;
						printl('+', line.replace("\t", "    "));
					}
				}

				continue;

			}// forrange

			// the following conversions are applied progressively

			var line2 = line;

			// B - substr() to b()
			if (substr2b) {
				line2.regex_replacer("\\.substr\\(", ".b\\(");
			}

			// S - .b(1, ..) eq -> .starts(..
			//var newrec = RECORD.regex_replace(R"(\.b\(1, \d+\) eq (".*"))", R"(.starts(\1)");
			if (b1eq2starts) {
				line2.regex_replacer(
					R"__(\.b\(1, ?\d+\) ?(eq|==) ?(".*?"))__",
					R"__(.starts\(\2\))__", "g"
				);

				// first
				line.regex_replacer(
					R"__(\.b\(1, ?(\d+)\))__",
					R"__(.first\(\1\))__", "g"
				);
			}

			// L - last()/ends()
			if (b2last) {

				// ends
				line2.regex_replacer(
					R"__(\.b\(-(\d+), ?\1\) ?(eq|==) ?(".*?"))__",
					R"__(.ends\(\3\))__", "g"
				);

				// last
				line2.regex_replacer(
					R"__(\.b\(-(\d+), ?\1\))__",
					R"__(.last\(\1\))__", "g"
				);
			}

			// C - Cut
			if (cut) {
				line2.regex_replacer(
					R"__(\.b\(2\))__",
					R"__(\.cut\(1\))__"
				);
				line2.regex_replacer(
					R"__(\.b\(3\))__",
					R"__(\.cut\(2\))__"
				);
				line2.regex_replacer(
					R"__(\.b\(4\))__",
					R"__(\.cut\(3\))__"
				);
				line2.regex_replacer(
					R"__(\.b\(5\))__",
					R"__(\.cut\(4\))__"
				);
				line2.regex_replacer(
					R"__(\.b\(6\))__",
					R"__(\.cut\(5\))__"
				);
				line2.regex_replacer(
					R"__(\.b\(7\))__",
					R"__(\.cut\(6\))__"
				);
				line2.regex_replacer(
					R"__(\.b\(8\))__",
					R"__(\.cut\(7\))__"
				);
				line2.regex_replacer(
					R"__(\.b\(9\))__",
					R"__(\.cut\(8\))__"
				);
				line2.regex_replacer(
					R"__(\.b\(10\))__",
					R"__(\.cut\(9\))__"
				);
			}

			// G - General
			if (general) {

				line2.regex_replacer(
					R"__(\.oconv\("D2/E"\)\.first\(2\))__",
					R"__(\.oconv\("DD"\))__"
				);

				// .b(9, 99) -> .b(9)
				line2.regex_replacer(
					R"__(\.b\(([a-zA-Z_0-9"]+), 99+\))__",
					//R"__(\.b\((\d), 99+\))__",
					R"__(.b\(\1\))__"
				);

				// .b(1, -> .starts(
				line2.regex_replacer(
					R"__(\.b\(1,\s*)__",
					R"__(.first\()__"
				);

			}
			// var().xxx -> xxx
			if (emptyvar) {

				// var().date/time/chr( -> date/time/chr(
				line2.regex_replacer(
					R"__(var\(\)\.(chr|date|time)\()__",
					R"__(\1\()__"
				);

			}

			// [1/-1] eq/ne "x"/XX -> .starts/ends(xx)
			if (onechar) {

				line2.regex_replacer(
					R"__(\[1\]\s(eq|==)\s(".{1,2}"|[A-Z0-9a-z_.]+\b))__",
					R"__(.starts\(\1\))__"
				);

				line2.regex_replacer(
					R"__(\[-1\]\s(eq|==)\s(".{1,2}"|[A-Z0-9a-z_.]+\b))__",
					R"__(.ends\(\1\))__"
				);

			}

			//line = restore_subsyntax(line2, ',');
			if (line2 ne line) {
				printl('-', osfilename, line.replace("\t", "    "));
				printl('+', osfilename, line2.replace("\t", "    "));
				line = line2;
				replaced = true;
			}

		}

		// Update the source file
		if (replaced && OPTIONS.contains("U"))
			txt.oswrite(osfilename);
	}

	return 0;
}

// Restore the hidden commas|semicolons etc
function restore_subsyntax(in line2, in hidden_char) {
	//var line = line2.convert(FM_, hidden_char);
	var line = line2.convert(_FM, hidden_char);
	return line;
}
// Hide commas|semicolons inside brackets and quotes
// so we can use the remaining 1, 2 or 3 commas to discover fn, vn, sn and replacement
// or semicolons etc.
function hide_subsyntax(in line, io pos, in hidden_char) {

	var line2 = line;
	pos += 3; //skip over first paren
	pos -=1;
	var depth = 0;
	bool inquotes = false;
	while (++pos <= line.len()) {

		char c = line.at(pos).toChar();

		if (c == '(' and not inquotes)
			depth++;

		else if (c == ')' and not inquotes)
			depth--;

		else if (c == '\\')
			pos++;

		else if (c == '"')
			inquotes = not(inquotes);

		//else if (c == ',' and (inquotes or depth))
		else if (c == hidden_char and (inquotes or depth))
			line2.splicer(pos, 1, _FM);
	}
	return line2;
}
programexit()
