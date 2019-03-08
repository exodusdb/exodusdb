#include <exodus/program.h>
programinit()


#include <gen.h>

function main() {
	//c
	//typically fewer keys/locks than number of users and numnber of tasks

	//get the configuration users and task
	var security;
	if (not(security.read(DEFINITIONS, "SECURITY"))) {
		call fsmsg();
		var().stop();
	}

	//decrypt it
	var tempkey = "SECURITY$";
	security.invert().write(DEFINITIONS, tempkey);

	//user edits the decrypted data
	osshell("edir DEFINITIONS " ^ tempkey);

	//get the edited version
	var security2;
	if (not(security2.read(DEFINITIONS, tempkey))) {
		call fsmsg();
		var().stop();
	}

	//re-encrypt and store
	if (security2 ne security) {
		security2.invert().write(DEFINITIONS, "SECURITY");
	}

	if (not(SENTENCE.field(" ", 2))) {
		DEFINITIONS.deleterecord(tempkey);

	}

	return "";
}


programexit()
