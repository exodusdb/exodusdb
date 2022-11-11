#include <exodus/program.h>
programinit()

function main() {

	var filenames = COMMAND.field(FM, 2, 999999);

	if (not filenames)
		abort("Syntax is createfile filename ... {OPTIONS}");

	var result = 0;

	for (let filename : filenames) {

		if (open(filename)) {
			errputl("Cannot create ", filename, " - it already exists");
			result = 1;
			continue;
		}

		if (not createfile(filename)) {
			result = 1;
			errputl("Cannot create ", filename);
			continue;
		}

		if (not OPTIONS.contains("S"))
			printl("Created", filename);

	}

	return result;
}

programexit()
