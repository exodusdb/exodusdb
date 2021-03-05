#include <exodus/program.h>
programinit()

	function main() {

	var force = OPTIONS.index("F");
	var indexnames = COMMAND.field(FM, 2, 999999);
	var nindexes = dcount(indexnames, FM);

	if (not nindexes)
		abort("Syntax is 'createindex filename__fieldname ... {F=Force}'");

	var result = 0;

	var indexn;
	for (indexn = 1; indexn <= nindexes; ++indexn) {

		var filename = indexnames.a(indexn);
		if (not filename)
			break;
		var fieldname;
		var tt = filename.index("__");
		if (tt) {
			fieldname = filename.substr(tt + 2);
			filename = filename.substr(1, tt - 1);
		} else {
			fieldname = indexnames.a(indexn + 1);
			indexn += 1;
		}

		if (force && listindexes(filename, fieldname)) {
			printl("Deleting index ", filename, "__", fieldname);
			if (not filename.deleteindex(fieldname)) {
				errputl("Cannot delete index ", filename, "__", fieldname);
				result = 1;
			}
		}
		printl("Creating index " ^ filename ^ "__" ^ fieldname);
		if (not filename.createindex(fieldname)) {
			errputl("Cannot create index " ^ filename ^ "__" ^ fieldname);
			result = 1;
		}
	}

	return result;
}

programexit()
