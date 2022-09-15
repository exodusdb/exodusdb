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

	U - Actually update the source file
)";

function main() {

	if (not OPTIONS)
		abort(syntax);

	COMMAND.remover(1);

	var r2a = OPTIONS.contains("R");
	var forrange = OPTIONS.contains("F");

	for (var filename : COMMAND) {

		if (not filename)
			continue;

		// Read a source file into a dimensioned array of vars
		dim txt;
		if (not txt.osread(filename))
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
				var part1 = line2.substr(pos).field(";", 1).trim();
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

		}

		// Update the source file
		if (replaced && OPTIONS.contains("U"))
			txt.oswrite(filename);
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
