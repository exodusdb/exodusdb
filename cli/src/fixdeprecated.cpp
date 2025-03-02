#include <exodus/program.h>
programinit()

let syntax = R"(
SYNTAX
	fixdeprecated [file|dir...] {OPTIONS}
	Will read compiler output from standard input if available file|dir are not provided.
OPTIONS
	U - Actually update
	C - Recompile after updated
	V - Verbose
	H - Help
)";

function main() {
//
	let option_d = 2;//OPTIONS.count("d");
//	let option_s = 1;//OPTIONS.count("s");
	let update = OPTIONS.contains("U");
	let recompile = OPTIONS.contains("C");
	let verbose = OPTIONS.count("V");

	if (OPTIONS.convert("UCV", "")) {
		abort(syntax);
	}

	// Use files/dirs otherwise stdin
	let stdinput = not COMMAND.f(2);

	// If any files provided on command line
	// then recompile them to get any deprecation messages and pipe
	// them into a another copy of this program in another process
	///////////////////////////////////////////////////////////////
	if (not stdinput) {

		// Compile all requested files to generate deprecation warnings
		// Parallel compilation will speed matters up.
		let compilecmd = COMMAND.update(1, "compile").convert(FM, " ");

		// h = Generate all headers and install in ~/inc first
		let headercmd = compilecmd ^ " {hS}";
		TRACE(headercmd)
		if (not osshell(headercmd))
			abort(lasterror());

		// Pipe the compiler output into another process of this command
		// F = Force recompilation to generate any deprecation warning messages
//		let pipedcmd = compilecmd ^ " {F} |& " ^ COMMAND.f(1) ^ " {" ^ OPTIONS ^ "}";
		let pipedcmd = compilecmd ^ " {F} |& " ^ EXECPATH ^ " {" ^ OPTIONS ^ "}";
		if (verbose)
			TRACE(pipedcmd)
		if (not osshell("bash -c " ^ pipedcmd.squote()))
//			abort(lasterror());
			abort(1);

		// Optionally recompile all requested files after updating in order to gain parallelism
		if (recompile) {
			printl("Recompiling");
			let recompilecmd = compilecmd ^ " {S}";
			if (verbose)
				TRACE(recompilecmd)
			if (not osshell(recompilecmd))
				abort(lasterror());
		}

		// and quit
		return 0;
	}

///////
// Init
///////

	printl("fixdeprecated: Reading from stdin");

	var src;
	var srcfile = "";
	var updatereq = 0;
	var srcfiles = "";
	var nlinesupdated = 0;
	var datfilenames = "";

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
//var allinp = "";
	// Process all the warnings in the input sequentially
	var compline;
	while (not eof()) {
		if (not compline.input()) {}
//allinp.appender(compline, "\n");
//oswrite(allinp on "allinp");
		//warning: 'operator[]' is deprecated: EXODUS: Replace single character accessors like xxx[n]
//		if (not compline.contains("warning") and not compline.contains("deprecated"))
		if (not compline.contains("warning") and not compline.contains("error:"))
			continue;

		compline.trimmerfirst("*");

		// Example compile warning
		//fin/ledger4.cpp:170:27: warning: ‘exodus::var& exodus::dim::operator()(int)’ is deprecated: EXODUS: Replace single dimensioned array accessors like () with [] e.g. dimarray(n) -> dimarray[n] [-Wdeprecated-declarations]
		//  170 |   let taxcodes = lg.vch(24);
		//      |                           ^
		// Identify where in the source the warning occurs
		let srcloc = compline.match("^([a-zA-Z0-9_./]+):([0-9]+):([0-9]+):");

		if (not srcloc)
			continue;

		let filename = srcloc.f(1, 2);
		let lineno = srcloc.f(1, 3);
		let charno = srcloc.f(1, 4);

		if (verbose)
			TRACE(compline)
//		bool option_d = 2;
//		bool option_s = false;

		var fixable = false;
		var old_open_char;
		var old_close_char;
		var new_open;
		var new_close;
		var new_prefix = "";
		if (option_d) {
//			var fixable = false;
//			var old_open_char;
//			var old_close_char;
//			var new_open;
//			var new_close;
//			var new_prefix = "";

			// Deprecation of single and multiple dimensioned access at the moment
			// ddd(i) -> ddd[i]
			// ddd(i, j) -> ddd[i, j]
			if (option_d) {

//				// {d} or {dd}
//				if (compline.contains("EXODUS: Replace single dimensioned array")) {
//					fixable = 1;
//				}
//				// {dd}
//				else
				if (option_d > 1 and compline.contains("EXODUS: Replace multiple dimensioned")) {
					fixable = 1;
				}
				if (fixable) {
					old_open_char = "(";
					old_close_char = ")";
					new_open = "[";
					new_close = "]";
				}
			}

//			// s - deprecation of single char access using []
//			// xxx[n] -> xxx.at(n)
//			if (not fixable and option_s) {
//
//				if (compline.contains("EXODUS: Replace single character accessors")) {
//					fixable = 1;
//					old_open_char = "[";
//					old_close_char = "]";
//					new_open = "(";
//					new_close = ")";
//
//					new_prefix = ".at";
//				}
//			}

//			// No fixable deprecations found
//			if (not fixable) {
//				logputl("fixdeprecated: Not fixable.");
//				continue;
//			}

		} // fixing brackets

		// Switch to a new srcfile
		if (filename != srcfile) {

			// Update the previous srcfile before loading the next one
			update_src();

			// Get the new src
			if (filename and not src.osread(filename)) {
				abort(lasterror() ^ " srcfile=" ^ srcfile.quote());
//				abort(lasterror());
			}

			srcfile = filename;

			datfilenames = "";
		}

		// Extract the src line
		var srcline = src.field(EOL, lineno);
		let orig_srcline = srcline;

		if (verbose)
			TRACE(srcline);

		//////////////////////////////////////////////////////////
		// Lambda to update the source line.
		// Will also be used to update a dat file for dict_xyz.cpp
		//////////////////////////////////////////////////////////
		auto source_fixer = [](io srcline) {

			// NOT doing move until brackets are fixed first
////			srcline.replacer(R"__(\bmove\()__"_rex, "move_to\\(");
//			// pp.dtext[pp.dn, pnx].move(tq);
//			// response_.move(RECORD);
//			srcline.replacer(R"__(([_\w.\[\]]+)\.move\(([_\w.\[\]]+)\);)__"_rex, "$2 = $1\\.move\\(\\);");

			srcline.replacer(R"__(makelist("", )__", "selectkeys(");
			srcline.replacer(R"__(field2()__", "field(");

			// if (not lkfile.osopen(lkfilename, "C")) {
			// if (not ovfile.osopen(osfile ^ ".OV", "C")) {
//			srcline.replacer(R"__("C")__", R"__(false)__");
			srcline.replacer(R"__((osopen\([^,]+,[^"]*)"C")__"_rex, R"__($1false)__");

			// reply.input();
			// if (not reply.input()) {}
			srcline.replacer(R"__(^(\s*)([a-zA-Z0-9_.]+\.input\([^)]*\));)__"_rex, R"__($1if \(not $2\) {})__");

			//call pushselect(0, v69, v70, v71);
			//call popselect(0, v69, v70, v71);
			srcline.replacer(R"__(\b(push|pop)select\([^,]+,\s*([a-zA-Z0-9_]+).*)__"_rex, "$1select\\($2\\);");

			//MVLogoff -> ExoLogoff
			srcline.replacer(R"__(\bMVLogoff\b)__"_rex, "ExoLogoff");

			//tcase(mediacodes, "QUOTE");
			//mediacode.replace(_VM, "\" \"").quote();
			srcline.replacer(R"__(\bcapitalise\(([a-zA-Z0-9_]+), "QUOTE"\);)__"_rex, R"__($1.replace\(_VM, "\\" \\""\).quote\(\);)__");

			//capitalise( -> tcase(
			srcline.replacer(R"__(\bcapitalise\()__"_rex, "tcase\\(");
		};

		source_fixer(srcline);

		if (verbose)
			TRACE(srcline);

		// code related to changing use of brackets
		if (fixable) {

			// Lambda function to search forwards for closing bracket or backwards for opening bracket
			//////////////////////////////////////////////////////////////////////////////////////////
			// Find charno2 of opening "(" or ")"
			// or return 0
			auto find_char = [&srcline, charno](in char_to_find, in direction) -> var {
				var opposite_char = srcline.at(charno);
				var charno2 = charno;
				var depth = 0;
				var ch;
				int nchars = srcline.len();

				// break out if gone down to zero
				while (true) {

					charno2 += direction;

					// Break out if out of range
					if (not charno2 or charno > nchars) {
						logputl("error: fixdeprecated: Could not find " ^ char_to_find.quote());
						return 0;
					}

					ch = srcline.at(charno2);
					if (ch == char_to_find) {

						// break out if found
						if (depth == 0)
							return charno2;

						depth--;
					} else if (ch == opposite_char) {
						depth++;
					}
				}
	//			throw VarError("fixdeprecated: Should not get here");
	//			return 0;
			};

			// Note:
			// gcc warning points to closing ")" or "]"
			// clang warning points to opening "(" or "["


			// Verify the target character is indeed ")" "]" (for gcc) or "(" "[" (for clang)
			let char2 = srcline.at(charno);
			var charno2;
			if (char2 == old_open_char) {

				// Clang - Find closing ")" or "]" to the right
				charno2 = find_char(old_close_char, 1);

				// Skip on error
				if (not charno2) {
					if (verbose)
						errputl(srcloc, " could not find '" ^ old_close_char ^ "'. Already converted clang?\n" ^ srcline.convert("\t", " ") ^ "\n" ^ space(charno - 1) ^ "^");
					continue;
				}

				// Expected below in reverse order, so swap them
				// charno2 is opening and charno is closing
				charno.swap(charno2);

			} else {

				// gcc
				if (char2 != old_close_char) {
					if (char2 == new_close)
						// Already converted. Maybe some race condition
						continue;
					// Skip on error
					if (verbose) {
						var msg = srcloc ^ " (1) char should be '" ^ old_open_char ^ "' or '" ^ old_close_char ^ "' but is actually " ^ srcline.at(charno).squote() ^ ".";
						if (char2 == new_open or char2 == new_close)
							msg ^= "Already converted?";
						else
							msg ^= "Maybe #define hides it. Find and change the #define manually.";
						msg ^= "\n" ^ srcline.convert("\t", " ") ^ "\n" ^ space(charno - 1) ^ "^\n";
						logputl(msg);
					}
					continue;
				}

				// gcc - Find opening "(" to the left
				// TODO use above lambda function to deduplicate code
				charno2 = charno;
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

				// Skip if not found
				if (depth or not charno2 or ch != old_open_char) {
					if (verbose)
						errputl(srcloc, " (2) char should be '" ^ old_close_char ^ "' but is actually " ^ srcline.at(charno).squote() ^ ". Already converted?\n" ^ srcline.convert("\t", " ") ^ "\n" ^ space(charno - 1) ^ "^");
				}

			}

			// If found matching "(" or ")" then replace both ( and ) with [ and ]
			// charno2 points to opening (
			// charno points to closing )
			nlinesupdated++;

			// Change them together or not at all
			srcline.paster(charno, 1, new_close);
			// MUST replace opener 2nd since it may be more than one character
			srcline.paster(charno2, 1, new_open);

			// e.g. "xyz[n] xyz.at(n)
			if (new_prefix)
				srcline.paster(charno2, new_prefix);
		}

		if (srcline == orig_srcline)
			continue;

		if (verbose)
			TRACE(compline)

		printl(srcloc.f(1,1), srcline.trimfirst("\t "));

		// Try to update dat file as well
		if (filename.contains("dict_")) {

			// get dat file dir
			// dic/dict_jobs.cpp
			var datfiledir = filename.replace("dic/", "dat/").replace("/dict_", "/dict.").cut(-4);

			// find dict id backwards
			// libraryinit(date_created)
			var lineno2 = lineno - 1;
			var datfilename = "";
			while (lineno2) {
				var srcline2 = src.field(EOL, lineno2);
				if (srcline2.match("libraryinit")) {
					datfilename = datfiledir ^ "/" ^ srcline2.field("(", 2).field(")", 1).ucase();
					break;
				}
				lineno2--;
			}

			bool dat_update = false;
			if (datfilename) {
				var datrec;
				if (osread(datrec from datfilename)) {
					let orig_datrec = datrec;

					source_fixer(datrec);

					if (datrec ne orig_datrec) {
						printx(datfilename, datrec);
						if (update) {
							if (oswrite(datrec on datfilename)) {
								datfilenames ^= datfilename ^ FM;
								printl(" Updated.");
								dat_update = true;
							} else
								loglasterror();
						} else {
							dat_update = true;
							printl(" Updatable.");
						}
					} else {
						// Probably updated already.
						dat_update = locate(datfilename, datfilenames);
					}
				} else {
					// Assume someone else owns the dict element
					dat_update = true;
				}
			}

			if (not dat_update)
				printl("MANUAL update required in ", filename, datfilename);
		}

		// Flag update required
		nlinesupdated++;
		updatereq = 1;
		src.fieldstorer(EOL, lineno, 1, srcline);

	} // end of one compline

	// Deal with changes to the last srcfile if any
	update_src();

	if (nlinesupdated)
		printl(nlinesupdated, "lines", update ? "updated." : "updateable. 'fixdeprecated {U}' to update.");
	else
		printl(nlinesupdated, "lines updated.");

	return 0;
}

programexit()
