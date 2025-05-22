#include <exodus/library.h>
libraryinit()

#include <hashpass.h>

func main() {

	if (not open("VOC") and not createfile("VOC"))
		abort(lasterror());

	if (not open("DICT.DEFINITIONS") and not createfile("DICT.DEFINITIONS"))
		abort(lasterror());

	if (not open("DEFINITIONS") and not createfile("DEFINITIONS"))
		abort(lasterror());

	if (not open("PROCESSES") and not createfile("PROCESSES"))
		abort(lasterror());

	if (not open("REQUESTLOG") and not createfile("REQUESTLOG"))
		abort(lasterror());

	if (not open("SYS_MESSAGES") and not createfile("SYS_MESSAGES"))
		abort(lasterror());

	if (not open("COMPANIES") and not createfile("COMPANIES"))
		abort(lasterror());

	if (not open("DOCUMENTS") and not createfile("DOCUMENTS"))
		abort(lasterror());

	if (not open("USERS") and not createfile("USERS"))
		abort(lasterror());

	if (not open("LOCKS") and not createfile("LOCKS"))
		abort(lasterror());

	if (not open("STATISTICS") and not createfile("STATISTICS"))
		abort(lasterror());

	if (not open("CHANGELOG") and not createfile("CHANGELOG"))
		abort(lasterror());

	SYSTEM(33) = "X";

	// Module
	APPLICATION = "EXODUS";

	// User
	USERNAME = "EXODUS";

	// Database
	var databasecode = osgetenv("EXO_DATA");
	if (not databasecode)
		databasecode = "exodus";
	SYSTEM(17) = databasecode;

	// Process No.
	SYSTEM(24) = THREADNO;

	// System mode
	SYSTEM(33, 1) = "1";
	SYSTEM(17)	  = "exodus";  //database code
	SYSTEM(23)	  = "EXODUS";  //database name
	SYSTEM(33, 2) = "EXODUS";  //connection
	SYSTEM(33, 3) = "EXODUS";

	// f5 key
	PRIORITYINT(2) = "x";

	// Create database dir
	let datadir = "../data/";
	if (not osdir(datadir))
		abort(datadir.quote() ^ " data directory does not exist");
	let databasedir = datadir ^ databasecode;
	if (osmkdir(databasedir)) {
		if (not osshell("chmod g+rws " ^ databasedir)) {
			abort(lasterror());
		}
		if (not osshell("setfacl -d -m g::rw " ^ databasedir)) {
			abort(lasterror());
		}
	}

	// Ensure MARKETS file exists and has at least an ALL markets record
	if (not open("MARKETS")) {
		//createfile("MARKETS");
		if (not createfile("MARKETS")) {
			abort(lasterror());
		}
	}
	select("MARKETS");
	if (hasnext())
		clearselect();
	else
		write("All Markets", "MARKETS", "ALL");

	// EXODUS pass
	var rec = "";
	rec(7) = hashpass("ALSOWINE");
	write(rec on "DEFINITIONS", "EXODUS");

	perform("initgeneral LOGIN");

	// Kick off with listen or some custom command
	let cmd = SENTENCE.field(" ", 2, 9999);
	if (cmd)
		perform(cmd);
	else
		perform("listen");

	return 0;
}

}; // libraryexit()
