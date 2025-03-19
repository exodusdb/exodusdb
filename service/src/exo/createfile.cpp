#include <exodus/library.h>
libraryinit()

// OBSOLETE. No callers. replaced by var::createfile

func main() {

	// syntax is:
	// createfile filename dictfilename
	// or
	// createfile filename

	// volumename is ignored

	var command = COMMAND.convert(FM, " ").lcase();

	command.replacer(" dict.", " dict ");
	command.replacer(" dict_", " dict ");
	let dataonly = command.contains(" data ");
	let dictonly = command.contains(" dict ");
	command.replacer(" data ", " ");
	command.replacer(" dict ", " ");

	// printl(command);

	// something like DATAVOL\NEOS0001 or REVBOOT will be ignored
	var volname = field(command, " ", 2);

	var filename = field(command, " ", 3);
	if (not filename) {
		filename = volname;
		volname	 = "";
	}

	// TODO support volume in createfile

	// dict file
	if (not dataonly)
		create("DICT." ^ filename);

	// data file
	if (not dictonly)
		create(filename);

	return 0;
}

subr create(in filename) {
	var msg = filename;
	if (createfile(filename))
		msg ^= " created";
	else
		msg ^= " not created";
	if (not index(OPTIONS, "S"))
		printl(msg);
}
libraryexit()
