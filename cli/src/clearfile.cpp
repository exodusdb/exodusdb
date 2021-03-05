#include <exodus/program.h>
programinit()

	function main() {

	var filenames = COMMAND.field(FM, 2, 999999);

	if (not filenames)
		stop("Syntax is clearfile filename filename ...\ndict files are named dict_filename");

	var nfiles = dcount(filenames, FM);
	for (var filen = 1; filen <= nfiles; ++filen) {

		var filename = filenames.a(filen);
		if (clearfile(filename)) {
			print("Cleared ");
		} else {
			print("Cannot clear ");
		}
		printl(filename);
	}
	return 0;
}

programexit()
