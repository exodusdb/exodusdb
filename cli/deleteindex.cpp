#include <exodus/program.h>
programinit()

func main() {

	let silent = OPTIONS.count("S");

	let indexnames = COMMAND.remove(1);
	let nindexes = fcount(indexnames, FM);

	if (not nindexes)
		abort("Syntax is 'deleteindex filename__fieldname ... {S=Silent, SS=No error messages}'");

	let exo_data = osgetenv("EXO_DATA");
	var result = 0;

	var indexn;
	for (indexn = 1; indexn <= nindexes; ++indexn) {

		var filename = indexnames.f(indexn);
		if (not filename)
			break;
		var fieldname;
		let tt = filename.index("__");
		if (tt) {
			fieldname = filename.b(tt + 2);
			filename = filename.first(tt - 1);
		} else {
			fieldname = indexnames.f(indexn + 1);
			indexn += 1;
		}

		if (not silent)
			printl(exo_data, "deleteindex", filename, fieldname);
		if (not filename.deleteindex(fieldname)) {
			if (silent < 2)
				errputl(lasterror());
			result = 1;
		}
	}

	return result;
}

}; // programexit()
