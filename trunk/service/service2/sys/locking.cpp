#include <exodus/library.h>
libraryinit()

#include <gen.h>

var xx;
var allowduplicate;//num
var lockn;//num
var lockdesc;

function main(in mode, in lockfilename, in lockkey, in lockdesc0, io locklist, int ntries, out msg) {
	//jbase

	//declare function delay
	if (locklist.unassigned()) {
		locklist = "";
	}
	if (not ntries) {
		ntries = 100;
	}
	if (lockdesc0.unassigned()) {
		lockdesc = "";
	} else {
		lockdesc = lockdesc0;
	}
	if (not ntries) {
		ntries = 100;
	}
	msg = "";

	var interactive = not SYSTEM.a(33);

	if (0 and USERNAME == "NEOSYS") {
		printl(mode, " ", lockkey, " ", locklist);
		xx.input(" "^lockfilename);
	}

	if (mode == "LOCK") {
		allowduplicate = 0;
		goto lockit;

	} else if (mode == "RELOCK") {
		allowduplicate = 1;

lockit:
		var lockitem = lockfilename ^ "*" ^ lockkey;
		if (locklist.locate(lockitem, lockn)) {
			//msg=quote(lockitem):' already locked'
			return 1;
		}

		var lockfile;
		if (not(lockfile.open(lockfilename, ""))) {
			//call fsmsg()
			msg = lockfilename.quote() ^ " file cannot be opened in LOCKING";
			return 0;
		}

		var tryn = 1;
		var locksfile;
		var lockholder;
tryagain:
		//TODO if (not(lockrecord(lockfilename, lockfile, lockkey, "", 0, allowduplicate))) {
		if (not(lockrecord(lockfilename, lockfile, lockkey, "", 0))) {

			//try to get lockholder
			if (locksfile.open("LOCKS")) {
				lockholder = lockfilename ^ "*" ^ lockkey.xlate("LOCKS", 4, "X");
			}
			if (!lockholder)
				lockholder = "Someone, maybe you,";

			//if lockdesc then
			gosub getlockdesc(lockfilename,lockkey);
			if (interactive) {
				call mssg(lockholder ^ " is using the " ^ lockdesc, "T1");
				if (esctoexit()) {
					return 0;
				}else{
					goto tryagain;
				}
			}else{
				msg = lockholder ^ " is updating|" ^ lockdesc ^ "||PLEASE TRY LATER";
				if (tryn >= ntries) {
					return 0;
				}else{
					if (tryn == 1) {
						print( msg, "*", " ");
					}
					msg = "";
					call ossleep(1000);
					tryn += 1;
					goto tryagain;
				}
			}

			return 0;

		}

		//locklist.r(-1, lockitem);
		if (locklist.length())
			locklist^=FM;
		locklist^=lockitem;

		return 1;

	} else if (mode == "UNLOCK") {

		var lockfile;
		if (lockfile.open(lockfilename, "")) {
			xx = unlockrecord(lockfilename, lockfile, lockkey);
		}

		var lockn;
		if (locklist.locate(lockfilename ^ "*" ^ lockkey, lockn)) {
			locklist.eraser(lockn);
		}

		return 1;

	} else if (mode == "UNLOCKALL") {

		var filename2 = "";
		var file = "";
		var nlocks = locklist.dcount(FM);
		var lockitem;
		var lockkey2;
		for (var lockn = 1; lockn <= nlocks; ++lockn) {
			lockitem = locklist.a(lockn);
			var filename = lockitem.field("*", 1);
			if (filename ne filename2) {
				if (not(file.open(filename, ""))) {
					msg.r(-1, DQ ^ (filename ^ DQ) ^ " file cannot be opened in LOCKING,UNLOCKALL");
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

		if (msg) {
			return 0;
		}

		return 1;


	} else {
		call mssg(DQ ^ (mode ^ DQ) ^ " is invalid in LOCKING");
	}

	return 0;

}

subroutine getlockdesc(in lockfilename, in lockkey) {
	if (not lockdesc) {
		lockdesc = lockfilename ^ " " ^ lockkey;
		lockdesc.converter(".", " ");
	}

	return;
}

libraryexit()
