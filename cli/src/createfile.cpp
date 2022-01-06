#include <exodus/program.h>
programinit()

function main() {

	var filenames = COMMAND.field(FM, 2, 999999);

	if (not filenames) {
		stop("Syntax is createfile filename filename ...\ndict files are named dict.filename");
	}

	var result = 0;
	var nfiles = dcount(filenames, FM);
	for (var filen = 1; filen <= nfiles; ++filen) {

		var filename = filenames.a(filen);

		if (createfile(filename)) {
			print("Created ");
		} else {
			result = 1;
			print("Cannot create ");
		}
		printl(filename);
	}

	//why 1??
	return 1;
}

programexit()
