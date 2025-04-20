#include <exodus/program.h>
programinit()

func main() {

//	TRACE(COMMAND)
//	TRACE(OPTIONS)
//	TRACE(SENTENCE)
	COMMAND.remover(1);

	let find = COMMAND.f(1);
	COMMAND.remover(1);

	let repl = COMMAND.f(1);
	COMMAND.remover(1);

	if (not find or OPTIONS.convert("rsiUV", "")) {
		let syntax = "\n"
			"SYNOPSIS\n"
			"	fire FIND REPL filepath ... {OPTIONS}\n"
			"\n"
			"OPTIONS\n"
			"\n"
			"	r - raw find (not regex nor case insensitive)\n"
			"	s - whole file as single line\n"
			"	i - case insensitive\n"
			"	U - Update files\n"
			"	V - Verbose\n";
		abort(syntax);
	}

	let update = OPTIONS.contains("U");

	const bool raw = OPTIONS.contains("r");
	const bool verbose = OPTIONS.contains("V");

	// Remove all non-regex options
	let rxopts = OPTIONS.convert("rUV", "");

	// TODO Enable fire to replace only outside/only inside quotes/literals/comments.
	rex find_rex = rex(find,rxopts);

	for (in osfilename : COMMAND) {

		if (verbose)
			TRACE(osfilename);

		// Get text or skip
		if (not RECORD.osread(osfilename)) {
//			abort(lasterror());
			continue;
		}

		// Ignore .swp files else ::replace throws
		// "Invalid data or replacement during regex replace of "aaa" with "bbb"."
		// "Invalid UTF-8 sequence encountered while trying to encode UTF-32 character"
		if (osfilename.ends(".swp")) {
			logputl("Skipping " ^ osfilename);
			continue;
		}

		let origrec = RECORD;

		if (raw)
			RECORD.replacer(find, repl);
		else
			RECORD.replacer(find_rex, repl);

		if (RECORD != origrec) {

			printl(osfilename);

			if (update) {

				// Update
				if (not oswrite(RECORD on osfilename))
					abort(lasterror());

			} else {

				// Diff
				let tmpfilename = ostempdir() ^ osfilename.convert(OSSLASH, "_");
				if (not oswrite(RECORD on tmpfilename))
					abort(lasterror());
				// diff returns an error if there are differences
				//if (not osshell("diff " ^ osfilename ^ " " ^ tmpfilename ^ " --color=always")) {
				var cmd = "diff " ^ osfilename ^ " " ^ tmpfilename;
				if (TERMINAL) {
					var palette=osgetenv("DIFF_COLORS");
//					if (not palette) {
//						var palette=osgetenv("GREP_COLORS");
//						// https://www.linux.org/docs/man1/grep.html
//						GREP_COLORS=ms=1;31:mc=1;31:sl=:cx=:fn=1;35:ln=1;32:bn=1;32:se=1;36
//					}
					if (not palette)
						// ad = additions
						// de = deletions
						palette = "ad=01;31;38;5;154:de=01;31;38;5;9";
					cmd ^= " --color=always --palette=" ^ squote(palette);
				}
				if (not osshell(cmd)) {
					//lasterror().outputl();
				}
				if (not osremove(tmpfilename))
					abort(lasterror());
			}
		}
	}
	return 0;
}

}; // programexit()
