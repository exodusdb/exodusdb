#include <exodus/program.h>
programinit()

function main() {

//	TRACE(COMMAND)
//	TRACE(OPTIONS)
//	TRACE(SENTENCE)
	COMMAND.remover(1);

	let find = COMMAND.f(1);
	COMMAND.remover(1);

	let repl = COMMAND.f(1);
	COMMAND.remover(1);

	if (not find) {
		let syntax = "\n"
			"SYNOPSIS\n"
			"	fire FIND REPL filepath ... {OPTIONS}\n"
			"\n"
			"OPTIONS\n"
			"\n"
			"	r - raw find (not regex nor case insensitive)\n"
			"	s - whole file as single line\n"
			"	i - case insensitive\n"
			"	U - Update files\n";
		abort(syntax);
	}

	let update = OPTIONS.contains("U");
//	const bool raw = rxopts.contains("r");
	const bool raw = OPTIONS.contains("r");
	let rxopts = OPTIONS.convert("rU", "");

	for (in osfilename : COMMAND) {

		// Get text or skip
		if (not RECORD.osread(osfilename)) {
//			abort(lasterror());
			continue;
		}

		// Ignore .swp files else ::regex_replace throws
		// "Invalid data or replacement during regex replace of "aaa" with "bbb"."
		// "Invalid UTF-8 sequence encountered while trying to encode UTF-32 character"
		if (osfilename.ends(".swp")) {
			logputl("Skipping " ^ osfilename);
			continue;
		}

		let origrec = RECORD;

		//RECORD.regex_replacer(R"___(\tcall fsmsg\(\);\n\t+stop\(\);)___", R"___(\tabort\(lasterror\);)___", "s");
		//RECORD.regex_replacer(R"___(\breado\b)___", R"___(readc)___", "s");
		if (raw)
			RECORD.replacer(find, repl);
		else
			RECORD.regex_replacer(find, repl, rxopts);

		if (RECORD != origrec) {

			printl(osfilename);

			if (update) {

				// Update
				if (not oswrite(RECORD on osfilename))
					abort(lasterror());

			} else {

				// Diff
				let tmpfilename = ostempdirpath() ^ osfilename.convert(OSSLASH, "_");
				oswrite(RECORD on tmpfilename) or abort(lasterror());
				// diff returns an error if there are differences
				//if (not osshell("diff " ^ osfilename ^ " " ^ tmpfilename ^ " --color=always")) {
				var cmd = "diff " ^ osfilename ^ " " ^ tmpfilename;
				if (TERMINAL)
					cmd ^= " --color=always";
				if (not osshell(cmd)) {
					//lasterror().outputl();
				}
				osremove(tmpfilename) or abort(lasterror());
			}
		}
	}
	return 0;
}

programexit()

