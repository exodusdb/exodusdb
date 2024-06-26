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
	r - Change replacer syntax
		   x.r(a,b,c,y)
		-> x(a,b,c) = y
		Examples:
		   x.r(fn, replacement);
		-> x(fn) = replacement;
		   x.r(fn, vn, replacement);
		-> x(fn, vn) = replacement;
		   x.r(fn, vn, sn, replacement);
		-> x(fn, vn, sn) = replacement;
	f - Change for loop syntax to use range (1)
		Example:
		   for (var x = a; x <= b; ++x)
		-> for (var x : range(a to b))
   ff - Change for loop syntax to use range (2)
		Example:
		   for (x = a; x <= b; ++x)
		-> for (x : range(a to b))
	b - Change substr to b
		   .substr(
		-> .b(
	s - Change .b(1, n) eq/== to .starts(
	    and    '.b(n, n)' to .first(n)
		   .b(1, n) == "XXX"
		-> .starts("XXX")
		   .b(3, 3)
		-> .first(3)
	l - Change '.b(-n, n) eq' to .ends(n)
	    and    '.b(-n, n)' to .last(n)
		   .b(1, n) == "XXX"
		-> .starts("XXX")
		   .b(-3, 3)
		-> .last(3)
	c - Cut
		   .b(2)
		-> .cut(1)
		...
		   .b(10)
		-> .cut(9)
	p - paster
		   .paster(1, n, ""))
		-> .cutter(n)
	[] - 

	g - General
	v - Remove empty var().

	a - All conversions except R and F

	u - Actually update the source file
	V - Verbose
)";

function main() {

	if (not OPTIONS)
		abort(syntax);

	COMMAND.remover(1);

	if (OPTIONS.contains("a"))
		OPTIONS ^= "bslcpgv[]";
	let r2a = OPTIONS.contains("r");
	let forrange = OPTIONS.count("f");
	let substr2b = OPTIONS.contains("b");
	let b1eq2starts = OPTIONS.contains("s");
	let b2last = OPTIONS.contains("l");
	let general = OPTIONS.contains("g");
	let emptyvar = OPTIONS.contains("v");
	let cutting = OPTIONS.contains("c");
	let splicing = OPTIONS.contains("p");
	let onechar = OPTIONS.contains("[]");

	let verbose = OPTIONS.contains("V");

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
		int linen = 0;
		for (var& line : txt) {

			++linen;

			var pos = 0;

			// r2a - xxx.r(a,b,c,d) -> =xxx(1,b,c) = d;
			///////////////////////////////////////////
			if (r2a)
				pos = line.index(".r(");
			if (r2a and pos) {

				var line2 = hide_subsyntax(line, pos, ',');

				//x(fn) = y;
				line2.replacer(
					"([\\w()]+)"	// (1) A variable or variable with brackets containing a variable
					"\\.r\\("		// The .r( syntax
					"([^,]+)"		// (2) Everything up to the only comma is the FN
					", ?"			// ,
					"([^,]+)"		// (3) Everything else if no comma is the replacement
					"\\);"_rex		// The trailing );
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
					"\\);"_rex		// The trailing );
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
					"\\);"_rex		// The trailing );
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
			if (forrange and line.contains("\tfor (") and not line.contains("; --")) {

				//var line2 = hide_subsyntax(line, pos, ';');
				var line2 = line;
				bool fail = false;

				//for (const var dictidn : range(1, ndictids)) {
				let pattern = "for (var ";
				pos = line.index(pattern);
				if (pos) {
					if (forrange != 2) {
						// for (var dictidn = 1
						let part1 = line2.b(pos).field(";", 1).trim();
						let varname = part1.field(" ", 3);
						let starting = part1.field("=", 2, 999).trim();

						// dictidn <= ndictids + 20
						let part2 = line2.field(";", 2).trim();
						let varname2 = part2.field(" ", 1).trim();
						if (varname2 != varname)
							fail = true;
						let compare = part2.field(" ", 2);
						if (compare != "<=" and compare != "le")
							fail = true;
						let ending = part2.field(" ", 3, 999);

						// ++dictidn) {
						let part3 = line2.field(";", 3).trim();
						if (part3 != ("++" ^ varname ^ ") {") and part3 != (varname ^ "++) {"))
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
							line2.paster(pos, 99999, "for (const var " ^ varname ^ " : range(" ^ starting ^ ", " ^ ending ^ ")) {");
						}
					}
				} else if (forrange == 2) {
					line2.replacer(
						R"___(\tfor \(([a-z0-9_]+?) = (.+?); \1 (<=|le) (.+?); (\+\+\1|\1\+\+)\))___"_rex,
						R"___(\tfor \(let \1 : range\(\2, \4\)\))___");
						//R"___(\tfor \(([a-zA-Z_.0-9]+) =)___",
						//R"___(QQQ : range\()___");
	//				if (line.contains("QQQ"))
	//					TRACE(line);

				}

				if (not line2.contains(" : range(")) {

					// Failed to convert
					if (line2.contains(";")) {
						errput(osfilename, ":", linen, " ");
						errputl('?', line2.replace("\t", "    "));
					}

				} else {

					//printl("-", line);

					//line = restore_subsyntax(line2, ',');
					if (line2 != line) {
						line = line2;
						replaced = true;
						output(osfilename, ":", linen, " ");
						printl('+', line.replace("\t", "    "));
					}
				}

				continue;

			}// forrange

			// the following conversions are applied progressively

			var line2 = line;

			// B - substr() to b()
			if (substr2b) {
				line2.replacer(R"__(\.substr\()__"_rex, R"__(.b\()__");
			}

			// S - .b(1, ..) == -> .starts(..
			//var newrec = RECORD.replace(R"(\.b\(1, \d+\) == (".*"))"_rex, R"(.starts(\1)");
			if (b1eq2starts) {
				line2.replacer(
					rex(R"__(\.b\(1, ?\d+\) ?(eq|==) ?(".*?"))__", "g"),
					R"__(.starts\(\2\))__"
				);

				// first
				line.replacer(
					rex(R"__(\.b\(1, ?(\d+)\))__", "g"),
					R"__(.first\(\1\))__"
				);
			}

			// L - last()/ends()
			if (b2last) {

				// ends
				line2.replacer(
					rex(R"__(\.b\(-(\d+), ?\1\) ?(eq|==) ?(".*?"))__", "g"),
					R"__(.ends\(\3\))__"
				);

				// last
				line2.replacer(
					rex(R"__(\.b\(-(\d+), ?\1\))__", "g"),
					R"__(.last\(\1\))__"
				);
			}

			// C - Cut
			if (cutting) {
				line2.replacer(
					R"__(\.b\(2\))__"_rex,
					R"__(\.cut\(1\))__"
				);
				line2.replacer(
					R"__(\.b\(3\))__"_rex,
					R"__(\.cut\(2\))__"
				);
				line2.replacer(
					R"__(\.b\(4\))__"_rex,
					R"__(\.cut\(3\))__"
				);
				line2.replacer(
					R"__(\.b\(5\))__"_rex,
					R"__(\.cut\(4\))__"
				);
				line2.replacer(
					R"__(\.b\(6\))__"_rex,
					R"__(\.cut\(5\))__"
				);
				line2.replacer(
					R"__(\.b\(7\))__"_rex,
					R"__(\.cut\(6\))__"
				);
				line2.replacer(
					R"__(\.b\(8\))__"_rex,
					R"__(\.cut\(7\))__"
				);
				line2.replacer(
					R"__(\.b\(9\))__"_rex,
					R"__(\.cut\(8\))__"
				);
				line2.replacer(
					R"__(\.b\(10\))__"_rex,
					R"__(\.cut\(9\))__"
				);
			}

			// paster "" -> cutter
			if (splicing) {

				line2.replacer(
					//R"__(.paster\(1,\s*([A-Z0-9a-z_. + -]+),\s*""\))__"_rex,
					R"__(.paster\(1,\s*([^,]+),\s*""\))__"_rex,
					R"__(.cutter\(\1\))__"
				);

			}

			// G - General
			if (general) {

				line2.replacer(
					R"__(\.oconv\("D2/E"\)\.first\(2\))__"_rex,
					R"__(\.oconv\("DD"\))__"
				);

				// .b(9, 99) -> .b(9)
				line2.replacer(
					R"__(\.b\(([a-zA-Z_0-9"]+), 99+\))__"_rex,
					//R"__(\.b\((\d), 99+\))__",
					R"__(.b\(\1\))__"
				);

				// .b(1, -> .starts(
				line2.replacer(
					R"__(\.b\(1,\s*)__"_rex,
					R"__(.first\()__"
				);

			}
			// var().xxx -> xxx
			if (emptyvar) {

				// var().date/time/chr( -> date/time/chr(
				line2.replacer(
					R"__(var\(\)\.(chr|date|time)\()__"_rex,
					R"__(\1\()__"
				);

			}

			// [1/-1] eq/ne "x"/XX -> .starts/ends(xx) []
			if (onechar) {

				line2.replacer(
					R"__(\[1\]\s(eq|==)\s(".{1,2}"|[A-Z0-9a-z_.]+\b))__"_rex,
					R"__(.starts\(\2\))__"
				);

				line2.replacer(
					R"__(\[-1\]\s(eq|==)\s(".{1,2}"|[A-Z0-9a-z_.]+\b))__"_rex,
					R"__(.ends\(\2\))__"
				);

			}

			// Finally

			//line = restore_subsyntax(line2, ',');
			if (line2 != line) {
				printl('-', osfilename, line.replace("\t", "    "));
				printl('+', osfilename, line2.replace("\t", "    "));
				line = line2;
				replaced = true;
			}

		}

		// Update the source file
		if (replaced && OPTIONS.contains("U"))
			//txt.oswrite(osfilename);
			if (not txt.oswrite(osfilename))
				abort(lasterror());
	}

	return 0;
}

// Restore the hidden commas|semicolons etc
function restore_subsyntax(in line2, in hidden_char) {
	//var line = line2.convert(FM_, hidden_char);
	let line = line2.convert(_FM, hidden_char);
	return line;
}
// Hide commas|semicolons inside brackets and quotes
// so we can use the remaining 1, 2 or 3 commas to discover fn, vn, sn and replacement
// or semicolons etc.
function hide_subsyntax(in line, io pos, in hidden_char) {

	var line2 = line;
	pos += 3; //skip over first paren
	pos -= 1;
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
			line2.paster(pos, 1, _FM);
	}
	return line2;
}
programexit()
