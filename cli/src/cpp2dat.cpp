#include <exodus/program.h>
programinit()

function main() {

	printl("cpp2dat says 'Hello World!'");

	//example code from dict_ads.cpp

	//libraryinit(brand_and_date)
	////-------------------------
	//function main() {
	//    ANS = RECORD.f(3) ^ "*" ^ RECORD.f(12);
	//    /*pgsql;
	//    ans:=exodus.extract_text(data,3,0,0) || '*' || exodus.extract_text(data,12,0,0);
	//    */
	//    return ANS;
	//}
	//libraryexit(brand_and_date)

	let update = OPTIONS.contains("U");

	let regen_cpp = false;

	let osfilenames = COMMAND.remove(1);
	for (in cppfilename : osfilenames) {

		cppfilename.outputl("file: ");

		var newcpptext = "#include <exodus/library.h>\n";

		var oldcpptext;
		if (not osread(oldcpptext from cppfilename)) {
			abort(cppfilename.quote() ^ " Cannot read file ");
			//cppfilename.errputl("Cannot read file ");
			//continue;
		}

		// Convert oldcpptext to FM separated for ease of processing
		oldcpptext.converter("\n", _FM);

		// Determine where the dat items are
		var datdirname = "dat/" ^ cppfilename.cut(4);
		datdirname.replacer("dict_", "dict.");
		datdirname.replacer(".cpp", "");

//		datdirname.outputl("dat: ");

		let dictfilename = cppfilename.cut(4).replace("dict_", "dict.").replace(".cpp", "");
		var dictfile;
		if (not open(dictfilename to dictfile)) {
			loglasterror();
			continue;
		}

		var newdictsrc;
		var dictid = "";
		var in_pgsql_block;
		for (in line : oldcpptext) {

			// We are interested in dict source between libraryinit and libraryexit statements
			if (line.starts("libraryinit")) {
				dictid = line.field("(",2).field(")",1).ucaser();

//				dictid.outputl("dict: ");

				newdictsrc = "";
				in_pgsql_block = false;

				if (regen_cpp) {
					newcpptext ^= "\n" ^ line ^ "\n";
					newcpptext ^= "/" "/" ^ str("-", len(line) - 2) ^ "\n";
				}

			}

			// If at libraryexit then process the dict item source code that we have accumulated
			else if (line.starts("libraryexit")) {

				// Remove "function main() {" and closing "}"
				// iif there are no #include statements
				// and there is only one function
				// and first line is function main()
				var functionmain = true;
				if (newdictsrc.f(1,1) == "function main() {" and not newdictsrc.contains("#include") and newdictsrc.count("function ") == 1) {
					functionmain = false;
					newdictsrc.replacer("function main() {" ^ VM, "");
					newdictsrc.cutter(-2);
				}

				newdictsrc.trimmerfirst(VM).trimmerlast(VM);

//				// Remove trailing RETURN ANS;
//				let nlines = fcount(newdictsrc, VM);
//				if (newdictsrc.f(1, nlines).trim().trim("\t") == "return ANS;") {
//					newdictsrc.remover(1,nlines);
//				}

				// Get the existing dat file text
				let datfilename = datdirname ^ "/" ^ dictid;
				var olddatrec = "";
				if (not osread(olddatrec from datfilename)) {
					abort(datfilename.quote() ^ " datfile is missing ... skipped. ");
					//datfilename.errputl("datfile is missing ... skipped. ");
					//continue;
				}

				// Get the old dict rec
				var olddictrec;
				if (not read(olddictrec from dictfile, dictid))
					abort("Cannot read " ^ dictid.quote() ^ " from " ^ dictfilename.quote());

				var oldsrc = olddictrec.f(8);

				// and append any pgsql function to our cpp source extracted from dict_xxxxx.cpp
				let oldpgsql_index = oldsrc.index("/" "*pgsql");
				var oldpgsql = "";
				if (oldpgsql_index) {
					oldpgsql = oldsrc.b(oldpgsql_index).trimlast(VM);
					oldsrc.firster(oldpgsql_index - 1);
					trimmerlast(oldpgsql, VM);
					trimmerlast(oldpgsql);
					trimmerlast(oldpgsql, VM);
					trimmerlast(oldpgsql);

					// Convert four space indents to tabs
					if (oldpgsql.index(_VM "        ")) {
						oldpgsql.replacer(_VM "                    ", _VM "\t\t\t\t\t");
						oldpgsql.replacer(_VM "                ", _VM "\t\t\t\t");
						oldpgsql.replacer(_VM "            ", _VM "\t\t\t");
						oldpgsql.replacer(_VM "        ", _VM "\t\t");
						oldpgsql.replacer(_VM "    ", _VM "\t");
					}

					// Convert single space indents to tabs
					oldpgsql.replacer(_VM " ", _VM "\t");
					oldpgsql.replacer("\t ", "\t\t");
					oldpgsql.replacer("\t ", "\t\t");
					oldpgsql.replacer("\t ", "\t\t");
					oldpgsql.replacer("\t ", "\t\t");
					oldpgsql.replacer("\t ", "\t\t");

					if (not oldpgsql.ends("*" "/")) {
						abort(oldpgsql ^ "\n" ^ dictid.quote() ^ " old pgsql must end with *" "/" " and not " ^ oldpgsql.last(2).quote());
						//oldpgsql.errputl("old pgsql must end with *" "/ ... skipped. ");
						//continue;
					}

					// Remove trailing spaces
					while (index(oldpgsql, " " _VM))
						oldpgsql.replacer(" " _VM, _VM);

					// Add one indent if none present
					if (oldpgsql.f(1, 2) != "DECLARE" and oldpgsql.f(1,2)[1] != "\t") {
						oldpgsql.replacer(_VM, _VM "\t");
						//unindent the closing comment
						oldpgsql.replacer(_VM "\t" "*" "/", _VM "*" "/");
					}

					newdictsrc ^= VM ^ VM ^ oldpgsql;
				}

				dictid = "";

				// Sanity check for max one pgsql section
				if (newdictsrc.count("/" "*pgsql") > 1)
					abort(dictid.quote() ^ "/" "*pgsql must only appear once ... skipped. ");

				// Generate the new dat rec with the new source
				var newdatrec = olddictrec;
				newdatrec.r(8, newdictsrc);

				// Double escape literal "\n" -> "\\n"
				newdatrec.replacer("\\n", "\\\\n");

				// Single escape LF -> "\n" ... should not actually occur since LF -> FM already
				newdatrec.replacer("\n", "\\n");

				// Convert back to text format
				newdatrec.converter(FM, "\n");

				// Escape VM to become text lines ending \ (backslash)
				newdatrec.replacer(VM, "\\" "\n");

				// Update the dat file iif changed
				if (newdatrec != olddatrec) {
					if (update) {
						if (oswrite(newdatrec on datfilename)) {
							datfilename.logputl("Updated: ");
						} else {
							abort(datfilename.quote() ^ " Could not update dat file.");
							//datfilename.errputl("Could not update dat file  ... skipped. ");
						}
					} else {
							datfilename.logputl("Requires update: ");
					}
				} else {
					//datfilename.logputl("No change");
				}

				// Append dict source to cpp file
				if (regen_cpp) {
					if (not functionmain)
						newcpptext ^= "function main() {\n";

					newcpptext ^= newdictsrc.convert(VM, "\n") ^ "\n";

					if (not functionmain)
						newcpptext ^= "}\n";

					newcpptext ^= line ^ "\n";
				}

			} else

			// Accumulate lines of source found in the .cpp file
			if (dictid and not line.starts("/" "/-")) {

				// Skip any pgsql function found in the .cpp file
				// since all updates have been done in dat/dict files
				if (not in_pgsql_block)
					in_pgsql_block = line.convert("\t ;", "") == "/*pgsql";

				if (not in_pgsql_block)
					newdictsrc ^= line ^ VM;

				if (in_pgsql_block and line.convert("\t ;", "") == "*" "/")
					in_pgsql_block = false;
			}

		}

		// Update dict cpp file
		if (regen_cpp and newcpptext != oldcpptext) {
			printl(newcpptext);
			if (update) {
				if (not oswrite(newcpptext on cppfilename))
					abort(cppfilename.quote() ^ " Could not write osfile");
				cppfilename.logputl("Updated: ");
			} else
				cppfilename.logputl("Needs Updating: ");
		}

	}

	return 0;
}

programexit()
