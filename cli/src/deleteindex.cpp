#include <exodus/program.h>
programinit()

function main() {

	var force = OPTIONS.contains("F");
	var indexnames = COMMAND.field(FM, 2, 999999);
	var nindexes = fcount(indexnames, FM);

	if (not nindexes)
		abort("Syntax is 'deleteindex filename__fieldname ...'");

	var result = 0;

	var indexn;
	for (indexn = 1; indexn <= nindexes; ++indexn) {

		var filename = indexnames.f(indexn);
		if (not filename)
			break;
		var fieldname;
		var tt = filename.index("__");
		if (tt) {
			fieldname = filename.b(tt + 2);
			filename = filename.first(tt - 1);
		} else {
			fieldname = indexnames.f(indexn + 1);
			indexn += 1;
		}

		printl("Deleting index " ^ filename ^ "__" ^ fieldname);
		if (not filename.deleteindex(fieldname)) {
			errputl("Cannot delete index " ^ filename ^ "__" ^ fieldname);
			result = 1;
		}
	}

	return result;
}

programexit()
