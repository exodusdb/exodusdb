#include <exodus/program.h>
programinit()

function main() {

	var syntax =
		"NAME\n"
		"	\n"
		"	copyrec - copies records between database files or os dirs\n"
		"	\n"
		"SYNOPSIS\n"
		"	\n"
		"	copyrec FROMFILE KEY ... to: TOFILE [KEY2 ...] {OPTIONS}\n"
		"	\n"
		"EXAMPLES\n"
		"	\n"
		"	copyrec fromfilename x y z to: tofilename {OCDS}\n"
		"	\n"
		"	copyrec fromfilename x y z to: . a b c {OCDS}\n"
		"	\n"
		"OPTIONS\n"
		"	\n"
		"	O = Overwrite target (if target exists)\n"
		"	C = Create target (if target doesnt exist)\n"
		"	D = Delete source (i.e. move)\n"
		"	S = Silent\n"
		"	\n"
		"NOTES\n"
		"	\n"
		"	TOFILE can be '.' to mean the same as the FROMFILE\n"
		"	\n"
		"	FROMFILE/TOFILE can be 'DOS' to copy from and to OS files.\n";

	// Parse arguments

	if (not COMMAND.f(2))
		abort(syntax);

	var delete_opt = OPTIONS.contains("D");
	var overwrite_opt = OPTIONS.contains("O");
	var create_opt = OPTIONS.contains("C");
	var silent_opt = OPTIONS.contains("S");

	var pos1;
	if (not COMMAND.locateusing(FM, "to:", pos1))
		abort("to: is required");

	// copyrec file1 a b c to: file2 x y z
	// pos1 = 6 (to:)
	// pos1 -3 = 6 - 3 = 3 (a)
	var fromfilename = COMMAND.f(2);
	var fromkeys = field(COMMAND, FM, 3, pos1 - 3);

	// copyrec file1 a b c to: file2 x y z
	// pos1 = 6 (to:)
	// pos1 + 1 = 6 + 1 = 7 (file2)
	// pos1 + 2 = 6 + 2 = 8 (x)
	var tofilename = COMMAND.f(pos1 + 1);
	var tokeys = field(COMMAND, FM, pos1 + 2, 999'999'999);

	if (tofilename eq ".")
		tofilename = fromfilename;

	var fromfile;
	if (not open(fromfilename to fromfile))
		abort(lasterror());

	var tofile;
	if (not open(tofilename to tofile))
		abort(lasterror());

	var itemn = 0;

	for (var fromkey : fromkeys) {

/////////////
// nextrecord
/////////////

		itemn++;
		var tokey = tokeys.f(itemn);
		if (not tokey.len())
			tokey = fromkey;

		if (not read(RECORD from fromfile, fromkey)) {
			if (decide(fromkey.quote() ^ " Cannot be read from " ^ fromfilename, "Skip|Cancel") != "Skip")
				abort("");
			continue;
		}

		var xx;
		if (read(xx from tofile, tokey)) {

			// Case: Target does exist

			// No (O)verwrite
			if (not overwrite_opt) {
				if (decide(tokey.quote() ^ " Already exists in " ^ tofilename
					^ " and you have not provided the (O)verwrite option", "Skip|Cancel") != "Skip") {
					abort("");
				}
				continue;
			}

		} else {

			// Case: target doesnt exist

			// No (C)reate
			if (not create_opt) {
				if (decide(tokey.quote() ^ " Does not exist in " ^ tofilename
					^ " and you have not provided the (C)reate option", "Skip|Cancel") != "Skip") {
					abort("");
				}
				continue;
			}

		}

/////////////
// recordinit
/////////////

/////////////
// recordexit
/////////////

		// actually write should crash if not successful, but program defensively
		if (not write(RECORD on tofile, tokey))
			abort(lasterror());

		if (not silent_opt)
			printl(tokey.quote() ^ " written");

		if (delete_opt) {
			// actually delete should crash if not successful, but program defensively
			if (not deleterecord(fromfile, fromkey))
				abort(lasterror());

			if (not silent_opt)
				printl(fromkey.quote() ^ " deleted");
		}

		// goto nextrecord
		continue;
	}

	return 0;
}

programexit()
