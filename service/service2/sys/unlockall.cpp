#include <exodus/library.h>
libraryinit()

//#include <gen.h>

function main(io locklist) {
	//jbase

	//declare function delay
	if (locklist.unassigned()) {
		locklist = "";
	}

	var filename;
	var filename2="";
	var file;
	var nlocks = locklist.dcount(FM);
	var lockitem;
	var xx;
	var lockfilename;
	for (var lockn = 1; lockn <= nlocks; ++lockn) {
		lockitem = locklist.a(lockn);
		var filename = lockitem.field("*", 1);
		if (filename ne filename2) {
			if (not(file.open(filename, ""))) {
//				msg.r(-1, DQ ^ (filename ^ DQ) ^ " file cannot be opened in LOCKING,UNLOCKALL");
				file = "";
			}
			filename2 = filename;
		}
		if (file ne "") {
			var unlockkey = lockitem.field("*", 2, 9999);
			xx = unlockrecord(lockfilename, file, unlockkey);
		}
	};//lockn;

	locklist = "";
	return 1;
}

libraryexit()
