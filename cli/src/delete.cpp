#include <exodus/program.h>
programinit()

func main() {

	let filename = COMMAND.f(2);

	let itemids = COMMAND.remove(1).remove(1);

	if (not filename or not itemids)
		abort("Syntax is 'delete filename itemid ... (S=Silent)'");

	var file;
	if (not open(filename, file))
		abort(filename.quote() ^ " file does not exist.");

	var ndeleted = 0;

	for (var itemid : itemids) {

		if (itemid == "*") {
			//clearfile(file);
			if (not clearfile(file))
				abort(lasterror());
			//printl("All records deleted");
			//stop();
			ndeleted = "ALL";
			break;
		}

		if (deleterecord(file, itemid))
			++ndeleted;
		else
			printl(quote(itemid) ^ " does not exist.");
	}

	printl(ndeleted ^ " record(s) deleted.");

	return 0;
}

}; // programexit()
