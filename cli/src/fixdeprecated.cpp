#include <exodus/program.h>
programinit()

function main() {

	let option_d = OPTIONS.count("d");
	let option_s = OPTIONS.count("s");
	let recompile = OPTIONS.contains("C");
	let update = OPTIONS.contains("U") or recompile;
	let verbose = OPTIONS.contains("V");

	if (not OPTIONS or OPTIONS.convert("dsUCXV", "")) {
		abort("Invalid options");
	}

	// If any files provided on command line
	// then setup a pipe from the compiler to another copy of this program in another process
	/////////////////////////////////////////////////////////////////////////////////////////
	if (COMMAND.count(FM))  {

		// Compile all requested files to generate deprecation warnings
		// Parallel compilation will speed matters up.
		let compilecmd = COMMAND.pickreplace(1, "compile").convert(FM, " ");

		// Generate all headers and install in ~/inc first
		let headercmd = compilecmd ^ " {hS}";
		if (verbose)
			TRACE(headercmd)
		if (not osshell(headercmd))
			abort(lasterror());

		// Pipe the compiler output into another process of this command
		// Force recompilation to generate all warning messages
		let pipedcmd = compilecmd ^ " {F} |& convsyntax3 {X" ^ OPTIONS ^ "}";
		if (verbose)
			TRACE(pipedcmd);
		if (not osshell("bash -c " ^ pipedcmd.squote()))
			abort(lasterror());

		// Optionally recompile all requested files after updating in order to gain parallelism
		if (recompile) {
			let recompilecmd = compilecmd ^ " {S}";
			if (verbose)
				TRACE(recompilecmd);
			if (not osshell(recompilecmd))
				abort(lasterror());
		}

		// and quit
		return 0;
	}

///////
// Init
///////

	println("Reading from stdin");

	var src;
	var srcfile = "";
	var updatereq = 0;
	var srcfiles = "";
	var nlinesupdated = 0;

	// Duplicated from cli/compile
	//let exo_HOME = osgetenv("EXO_HOME") ?: osgetenv("HOME");
	//let incdir = exo_HOME ^ "/inc";

////////////////////////////////////////
// Lambda function to update source when
// source file changes or at the end
////////////////////////////////////////

	auto update_src = [&]() {

		// Quit if no update required
		if (not updatereq)
			return;
		updatereq = 0;

		// Optionally update srcfile
		if (update) {
			if (not oswrite(src on srcfile))
				abort(lasterror());
			if (verbose)
				TRACE(srcfile)

			// If a header in ~/inc also write to original dir as per line 1 of the header
			// //copied by exodus "compile /root/neosys/src/agy/ab_common.h"
			// TODO think about possible race condition with piped parallel compiler input to this program
			if (srcfile.ends(".h")) {
				let origfile = src.field(EOL, 1).field("\"", 2).field(" ", 2);
				if (not oswrite(src on origfile))
					abort(lasterror());
				if (verbose)
					 TRACE(origfile)
			}
		}

		// Save all the srcfilenames in case recompilation has been requested
		srcfiles(-1) = srcfile;
	};

//////////////////
// Main input loop
//////////////////

	// Process all the warnings in the input sequentially
	var compline;
	while (compline.input()) {

		if (not compline.contains("deprecated"))
			continue;

		compline.trimmerfirst("*");

		// Example compile warning
		//fin/ledger4.cpp:170:27: warning: ‘exodus::var& exodus::dim::operator()(int)’ is deprecated: EXODUS: Replace single dimensioned array accessors like () with [] e.g. dimarray(n) -> dimarray[n] [-Wdeprecated-declarations]
		//  170 |   let taxcodes = lg.vch(24);
		//      |                           ^

		var ok = 0;
		var old_open_char;
		var old_close_char;
		var new_open;
		var new_close;

		// Deprecation of single and multiple dimensioned access at the moment
		// ddd(i) -> ddd[i]
		// ddd(i, j) -> ddd[i, j]
		if (option_d) {

			// {d} or {dd}
			if (compline.contains("EXODUS: Replace single dimensioned array")) {
				ok = 1;
			}
			// {dd}
			else if (option_d > 1 and compline.contains("EXODUS: Replace multiple dimensioned")) {
				ok = 1;
			}
			if (ok) {
				old_open_char = "(";
				old_close_char = ")";
				new_open = "[";
				new_close = "]";
			}
		}

		// s - deprecation of single char access using []
		// xxx[n] -> xxx.at(n)
		if (not ok and option_s) {

			if (compline.contains("EXODUS: Replace single character accessors")) {
				ok = 1;
				old_open_char = "[";
				old_close_char = "]";
				new_open = ".at(";
				new_close = ")";
			}
		}
		if (not ok)
			continue;

		// Identify where in the source the warning occurs
		let srcloc = compline.field(" ",1);
		let filename = srcloc.field(":", 1);
		let lineno = srcloc.field(":", 2);
		let charno = srcloc.field(":", 3);

		// Switch to a new srcfile
		if (filename != srcfile) {

			// Update the previous srcfile before loading the next one
			update_src();

			// Get the new src
			if (not src.osread(filename))
				abort(lasterror());

			srcfile = filename;
		}

		// Extract the src line
		var srcline = src.field(EOL, lineno);

		// Verify the target character is indeed ")"
		let char2 = srcline.at(charno);
		if (char2 != old_close_char) {
			if (char2 == new_close)
				// Already converted. Maybe some race condition
				continue;
			// Skip on error
			if (verbose)
				errputl(srcloc, " char should be '" ^ old_close_char ^ "' but is actually " ^ srcline.at(charno).squote() ^ ". Already converted?\n" ^ srcline.convert("\t", " ") ^ "\n" ^ space(charno - 1) ^ "^");
			continue;
		}

		// Find opening "("
		var charno2 = charno;
		var depth = 0;
		var ch;
		while (--charno2) {
			ch = srcline.at(charno2);
			if (ch == old_open_char) {
				if (depth == 0)
					break;
				depth--;
			} else if (ch == old_close_char) {
				depth++;
			}
		}

		// If found opening "(" then replace both ( and ) with [ and ]
		if (depth == 0 and charno2 > 0 and ch == old_open_char) {

			nlinesupdated++;

			// Change them together or not at all
			srcline.paster(charno, 1, new_close);
			// MUST replace opener 2nd since it may be more than one character
			srcline.paster(charno2, 1, new_open);

			if (verbose)
				TRACE(compline);
			printl(srcloc, srcline.trimfirst("\t "));

			// Flag update required
			updatereq = 1;
			src.fieldstorer(EOL, lineno, 1, srcline);

		}

	} // end of one compline

	// Deal with changes to the last srcfile if any
	update_src();

	printl(nlinesupdated, "lines", update ? "updated." : "updateable. Option {U} to update.");

	return 0;
}

programexit()
