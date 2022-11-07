#include <exodus/program.h>
programinit()

function main() {

	COMMAND.remover(1);

	let find = COMMAND.f(1);
	COMMAND.remover(1);

	let repl = COMMAND.f(1);
	COMMAND.remover(1);

	if (not find) {
		var syntax = "\n"
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
	let rxopts = OPTIONS.convert("rU", "");
	const bool raw = rxopts.contains("r");

	for (in osfilename : COMMAND) {

		if (not RECORD.osread(osfilename))
			abort(lasterror());
		var origrec = RECORD;

		//RECORD.regex_replacer(R"___(\tcall fsmsg\(\);\n\t+stop\(\);)___", R"___(\tabort\(lasterror\);)___", "s");
		//RECORD.regex_replacer(R"___(\breado\b)___", R"___(readc)___", "s");
		if (raw)
			RECORD.replacer(find, repl);
		else
			RECORD.regex_replacer(find, repl, rxopts);

		if (RECORD ne origrec) {

			printl(osfilename);

			if (update) {

				// Update
				if (not oswrite(RECORD on osfilename))
					abort(lasterror());

			} else {

				// Diff
				var tmpfilename = ostempdirpath() ^ osfilename.convert(OSSLASH, "_");
				oswrite(RECORD on tmpfilename) or abort(lasterror());
				osshell("diff " ^ osfilename ^ " " ^ tmpfilename ^ " --color=always") or true;//abort(lasterror());
				osremove(tmpfilename) or abort(lasterror());
			}
		}
	}
	return 0;
}

programexit()
