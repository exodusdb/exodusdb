#include <exodus/program.h>
programinit()

	function main() {

	var filenames = COMMAND.field(FM, 2, 999999);

	var result = 0;

	if (not filenames)
		stop("Syntax is deletefile filename filename ...\ndict files are named dict_filename");

	var nfiles = dcount(filenames, FM);
	for (var filen = 1; filen <= nfiles; ++filen) {

		var filename = filenames.a(filen);
		if (deletefile(filename)) {
			print("Deleted ");
		} else {
			errput("Cannot delete ");
			result = 1;
		}
		printl(filename);
	}
	return result;
}

programexit()
