#include <exodus/program.h>
programinit()

func main() {

	let filenames = COMMAND.remove(1);

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

}; // programexit()
