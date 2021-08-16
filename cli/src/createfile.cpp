#include <exodus/program.h>
programinit()

	function main() {

	var filenames = COMMAND.field(FM, 2, 999999);

	if (not filenames) {
		stop("Syntax is createfile filename filename ...\ndict files are named dict.filename");
	}
	var nfiles = dcount(filenames, FM);
	for (var filen = 1; filen <= nfiles; ++filen) {

		var filename = filenames.a(filen);

		if (createfile(filename)) {
			print("Created ");
		} else {
			print("Cannot create ");
		}
		printl(filename);
	}
	return 0;
}

programexit()
