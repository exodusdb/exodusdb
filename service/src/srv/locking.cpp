#include <exodus/library.h>
libraryinit()

#include <service_common.h>

#include <srv_common.h>

var ntries;	 // num
var lockdesc;
//var xx;
var allowduplicate;	 // num
//var lockitem;
var lockn;	// num
var lockfile;

#define lockitemsep_ "*"

function main(in mode, in lockfilename, in lockkey, in lockdesc0, io locklist, in ntries0, out msg) {

//	if (locklist.unassigned()) {
//		locklist = "";
//	}
//	if (ntries0.unassigned()) {
//		ntries = 100;
//	} else {
//		ntries = ntries0;
//	}
//	if (lockdesc0.unassigned()) {
//		lockdesc = "";
//	} else {
//		lockdesc = lockdesc0;
//	}
	locklist.defaulter("");
	ntries = ntries0.or_default(100);
	lockdesc = lockdesc0.or_default("");
	if (not ntries) {
		ntries = 100;
	}
	msg = "";

	// NOTE: not(SYSTEM.f(33));
	let interactive = false;

	if (var(0) and USERNAME == "EXODUS") {
		printl(mode, " ", lockfilename, " ", lockkey, " ", locklist);
		var reply;
		if (not reply.input()) {};
	}

	if (mode == "LOCK") {
		allowduplicate = 0;
		goto lockit;

	} else if (mode == "RELOCK") {
		allowduplicate = 1;
		goto lockit;

	} else if (mode == "UNLOCK") {
		gosub unlockit(lockfilename, lockkey, locklist);
		return 1;

	} else if (mode == "UNLOCKALL") {
		gosub unlockall(locklist, msg);
		return 1;

	} else {
		call note(mode.quote() ^ " is invalid in LOCKING");
	}

	return 0;

///////
lockit:
	// /////

	let lockitem = lockfilename ^ lockitemsep_ ^ lockkey;
	if (locklist.locateusing(FM, lockitem, lockn)) {
		return 1;
	}

	if (not lockfile.open(lockfilename, "")) {
		msg = lockfilename.quote() ^ " file cannot be opened in LOCKING";
		return 0;
	}

	var tryn = 1;
tryagain:
	if (not lockrecord(lockfilename, lockfile, lockkey, "", 0, allowduplicate)) {
		var lockholder = (lockfilename ^ "*" ^ lockkey).xlate("LOCKS", 4, "X");
		if (lockholder == "") {
			lockholder = "Someone, maybe you,";
		}
		gosub getlockdesc(lockdesc, lockfilename, lockkey);
		if (interactive) {
			var dummy;
			call note(lockholder ^ " is using the " ^ lockdesc, "T1", dummy);
			if (esctoexit()) {
				return 0;
			} else {
				goto tryagain;
			}
		} else {
			msg = lockholder ^ " is updating|" ^ lockdesc ^ "||PLEASE TRY LATER";
			if (tryn >= ntries) {
				return 0;
			} else {
				if (tryn == 1) {
					printl(msg, " trying for ", ntries, " secs");
				} else if (ntries < 1000) {
					printx("*");
				}
				msg = "";
				call ossleep(1000 * 1);
				tryn += 1;
				goto tryagain;
			}
		}

		//std::unreachable();
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunreachable-code-return"
		// Here for safety in case the above code is broken by maintenance programming
		return 0;
#pragma clang diagnostic pop
	}

	locklist(-1) = lockitem;

	return 1;
}

subroutine getlockdesc(io lockdescx, in lockfilename, in lockkey) {

	if (not lockdescx) {
		lockdescx = lockfilename ^ " " ^ lockkey;
		lockdescx.converter(".", " ");
	}

	return;
}

subroutine unlockit(in lockfilename, in lockkey, io locklist) {

	if (lockfile.open(lockfilename, "")) {
		unlockrecord("", lockfile, lockkey);
	}

	if (locklist.locateusing(FM, lockfilename ^ "*" ^ lockkey, lockn)) {
		locklist.remover(lockn);
	}

	return;
}

subroutine unlockall(io locklist, io msg) {

	var filename2 = "";
	let file	  = "";
	// 	for (lockn : range(1, 9999)) {
	// 		lockitem = locklist.f(lockn);
	// 		// /BREAK;
	// 		if (not lockitem) break;
	for (in lockitem : locklist) {
		let filename = lockitem.field(lockitemsep_, 1);
		let lockkeyx = lockitem.field(lockitemsep_, 2, 9999);
		if (filename != filename2) {
			if (not lockfile.open(filename, "")) {
				msg(-1)	 = filename.quote() ^ " file cannot be opened in LOCKING,UNLOCKALL";
				lockfile = "";
			}
			filename2 = filename;
		}
		if (lockfile != "") {
			unlockrecord("", lockfile, lockkeyx);
		}
	}  // lockitem

	locklist = "";

	return;
}

libraryexit()
