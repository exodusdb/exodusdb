#include <exodus/library.h>
libraryinit()

function main(in msg, in /*maxlen*/, in /*show*/, in /*allowablechars*/, io data, in /*escx*/) {

	var prompt = msg;
	if (!data.assigned())
		data = "";

	data.input(prompt);

	return 0;
}

libraryexit()
