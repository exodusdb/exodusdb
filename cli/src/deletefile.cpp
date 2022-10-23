#include <exodus/program.h>
programinit()

function main() {

	var filenames = COMMAND.field(FM, 2, 999999);

	if (not filenames)
		abort("Syntax is deletefile filename ...");

	var result = 0;

	for (let filename : filenames) {

		if (not open(filename)) {
			errputl(lasterror());
			result = 1;
			continue;
		}

		if (not deletefile(filename)) {
			errputl(lasterror());
			result = 1;
			continue;
		}

		printl("Deleted", filename);
	}

	return result;
}

programexit()
