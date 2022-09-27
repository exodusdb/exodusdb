#include <exodus/library.h>
libraryinit()

#include <system_common.h>

#include <sys_common.h>

var ntries;//num
var lockdesc;
var xx;
var allowduplicate;//num
//var lockitem;
var lockn;//num
var lockfile;

#define lockitemsep_ "*"

function main(in mode, in lockfilename, in lockkey, in lockdesc0, io locklist, in ntries0, out msg) {
	//c sys ,,,,io,in,out

	//global lockfile,lockitem

	if (locklist.unassigned()) {
		locklist = "";
	}
	if (ntries0.unassigned()) {
		ntries = 100;
	} else {
		ntries = ntries0;
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

	//NOTE: not(SYSTEM.f(33));
	var interactive = false;

	if (var(0) and USERNAME eq "EXODUS") {
		printl(mode, " ", lockfilename, " ", lockkey, " ", locklist);
		xx.input();
	}

	if (mode eq "LOCK") {
		allowduplicate = 0;
		goto lockit;

	} else if (mode eq "RELOCK") {
		allowduplicate = 1;
		goto lockit;

	} else if (mode eq "UNLOCK") {
		gosub unlockit(lockfilename, lockkey, locklist);
		return 1;

	} else if (mode eq "UNLOCKALL") {
		gosub unlockall(locklist, msg);
		return 1;

	} else {
		call note(mode.quote() ^ " is invalid in LOCKING");
	}

	return 0;

///////
lockit:
///////

	var lockitem = lockfilename ^ lockitemsep_ ^ lockkey;
	if (locklist.locateusing(FM, lockitem, lockn)) {
		return 1;
	}

	if (not(lockfile.open(lockfilename, ""))) {
		msg = lockfilename.quote() ^ " file cannot be opened in LOCKING";
		return 0;
	}

	var tryn = 1;
tryagain:
	if (not(lockrecord(lockfilename, lockfile, lockkey, "", 0, allowduplicate))) {
		var lockholder = (lockfilename ^ "*" ^ lockkey).xlate("LOCKS", 4, "X");
		if (lockholder eq "") {
			lockholder = "Someone, maybe you,";
		}
		gosub getlockdesc(lockdesc, lockfilename, lockkey);
		if (interactive) {
			call note(lockholder ^ " is using the " ^ lockdesc, "T1", xx, "");
			if (esctoexit()) {
				return 0;
			} else {
				goto tryagain;
			}
		} else {
			msg = lockholder ^ " is updating|" ^ lockdesc ^ "||PLEASE TRY LATER";
			if (tryn ge ntries) {
				return 0;
			} else {
				if (tryn eq 1) {
					printl(msg, " trying for ", ntries, " secs");
				} else if (ntries lt 1000) {
					print("*");
				}
				msg = "";
				call ossleep(1000*1);
				tryn += 1;
				goto tryagain;
			}
		}

		return 0;

	}

	locklist(-1) = lockitem;

	return 1;
}

subroutine getlockdesc(io lockdesc, in lockfilename, in lockkey) {

	if (not lockdesc) {
		lockdesc = lockfilename ^ " " ^ lockkey;
		lockdesc.converter(".", " ");
	}

	return;
}

subroutine unlockit(in lockfilename, in lockkey, io locklist) {

	if (lockfile.open(lockfilename, "")) {
		xx = unlockrecord("", lockfile, lockkey);
	}

	if (locklist.locateusing(FM, lockfilename ^ "*" ^ lockkey, lockn)) {
		locklist.remover(lockn);
	}

	return;
}

subroutine unlockall(io locklist, io msg) {

	var filename2 = "";
	var file = "";
//	for (lockn = 1; lockn <= 9999; ++lockn) {
//		lockitem = locklist.f(lockn);
//		///BREAK;
//		if (not lockitem) break;
	for (in lockitem : locklist) {
		var filename = lockitem.field(lockitemsep_, 1);
		var lockkeyx = lockitem.field(lockitemsep_, 2, 9999);
		if (filename ne filename2) {
			if (not(lockfile.open(filename, ""))) {
				msg(-1) = filename.quote() ^ " file cannot be opened in LOCKING,UNLOCKALL";
				lockfile = "";
			}
			filename2 = filename;
		}
		if (lockfile ne "") {
			xx = unlockrecord("", lockfile, lockkeyx);
		}
	} // lockitem

	locklist = "";

	return;
}

libraryexit()
