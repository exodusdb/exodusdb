#include <exodus/program.h>
programinit()

function main() {

	var filename = COMMAND.f(2);

	var itemids = remove(COMMAND, 1, 0, 0);
	remover(itemids, 1, 0, 0);

	if (not filename or not itemids)
		abort("Syntax is 'delete filename itemid ... (S=Silent)'");

	var file;
	if (not open(filename, file))
		abort(filename.quote() ^ " file does not exist.");

	var sep = 0;
	var posn = 1;
	var ndeleted = 0;
	do {
		var itemid = substr2(itemids, posn, sep);

		if (itemid == "*") {
			clearfile(file);
			printl("All records deleted");
			stop();
		}

		if (deleterecord(file, itemid))
			++ndeleted;
		else
			printl(quote(itemid) ^ " does not exist.");
	} while (sep);

	printl(ndeleted ^ " record(s) deleted.");

	return 0;
}

programexit()
