#include <exodus/program.h>
programinit()

var syntax = R"-(
var2let [cppfilename ...] [{OPTIONS}]

Converts '{tab}var xyz ' to '{tab}let xyz ' where possible by testing if it compiles.

Option U - Update
       V - Verbose
       W - Ignore warnings
)-";

func main() {

	let osfilenames = COMMAND.remove(1);
	let update = OPTIONS.contains("U");
	let verbose = OPTIONS.contains("V");
	let ignore_warnings = OPTIONS.contains("W");

	if (not osfilenames or OPTIONS.convert("UVW", ""))
		abort(syntax);

	for (let osfilename : osfilenames) {

		printx(osfilename, "");
		osflush();

		//if (osfilename.contains("/dict_"))
		//	continue;
		var text = "";
		if (not osread(text from osfilename)) {
			loglasterror();
			continue;
		}

		var nvars = text.count("\tvar ");
		var nchanges = 0;
		for (int varn = 1; varn <= nvars; ++varn) {

			if (isterminal(0) and esctoexit())
				break;

			// Change a var to let
			int pos = text.indexn("\tvar ", varn);
			if (not pos)
				continue;
			text.paster(pos, 5, "\tlet ");

			// Write to /tmp
			let tmpfilename = ostempdir() ^ "/" ^ osfilename.field("/", -1);
			if (not oswrite(text on tmpfilename)) {
				loglasterror();
				continue;
			}

			var cmd = "compile " ^ tmpfilename;
			if (not verbose)
				cmd ^= " 2>/dev/null";

			var cxx_options = " -Wno-sign-conversion -Wno-shorten-64-to-32";
			if (not ignore_warnings)
				cxx_options ^= " -fmax-errors=1";
			cmd.prefixer("CXX_OPTIONS=" ^ quote(cxx_options));

			if (not DATA.osshellread(cmd)) {
				// Restore "\tlet " to "\tvar " if will not compile
				text.paster(pos, 5, "\tvar ");
				printx("-");
			} else {
				++nchanges;
				--varn;
				--nvars;
				printx("+");
			}
			osflush();

			// Remove the tempfile
			if (not osremove(tmpfilename)) {
				loglasterror();
				continue;
			}

			// Restore the proper header file by compiling with {h} in the proper place
			let cmd2 = "compile " ^ osfilename ^ " {h}";
			if (not DATA.osshellread(cmd2)) {
				loglasterror();
			}

		}

		// Report changes
		printx("",nchanges, "changed/changeable");

		// Update the source file
		if (nchanges) {
			if (update) {
				if (not oswrite(text on osfilename))
					abort(lasterror());
				printx(" Updated");
			} else {
				printx(" Not updated. Use option {U}.");
			}
		}

		printl();
	}

	return 0;
}

programexit()
