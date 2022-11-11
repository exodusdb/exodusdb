#include <exodus/program.h>
programinit()

function main() {

	let filenames = COMMAND.field(FM, 2, 999999);

	if (not filenames)
		abort("Syntax is clearfile filename ...");

	var result = 0;

	for (let filename : filenames) {

		if (not open(filename) or not clearfile(filename)) {
			errputl("Cannot clear", filename);
			result = 1;
			continue;
		}

		printl("Cleared", filename);

	}

	return result;
}

programexit()
