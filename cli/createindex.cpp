#include <exodus/program.h>
programinit()

func main() {

	let force = OPTIONS.contains("F");
	var indexnames = COMMAND.remove(1);

	if (not indexnames)
		abort("Syntax is 'createindex [filename] [filename__]fieldname... {F=Force}'");

	// "createindex xref" means redo all _xref indexes
	if (indexnames == "xref") {
		indexnames = var().listindex();
		var indexnames2 = "";
		for (var index : indexnames) {
			if (index.ends("_xref"))
				indexnames2 ^= index ^ FM;
		}
		indexnames = indexnames2.pop();
		indexnames.replacer(VM, "__");
		printl(indexnames);
	}

	let nindexes = fcount(indexnames, FM);

	var result = 0;

//	for (var indexn : range(1, nindexes)) {
	for (var indexn = 1; indexn le nindexes; ++indexn) {

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

		let indexname = filename ^ "__" ^ fieldname;
		if (force && listindex(filename, fieldname)) {
			printl("deleteindex", filename, fieldname);
			if (not filename.deleteindex(fieldname)) {
				errputl(lasterror());
				result = 1;
			}
		}
		printl("createindex", filename, fieldname);
		if (not filename.createindex(fieldname)) {
			errputl(lasterror());
			result = 1;
		}
	}

	return result;
}

}; // programexit()
