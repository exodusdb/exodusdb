#include <exodus/library.h>
libraryinit()

#include <esctoexit.h>
#include <delay.h>

#include <gen.h>

var xx;
var allowduplicate;//num
var lockn;//num

function main() {
	//jbase

	//declare function delay
	if (locklist.unassigned()) {
		locklist = "";
	}
	if (ntries.unassigned()) {
		ntries = 100;
	}
	if (lockdesc.unassigned()) {
		lockdesc = "";
	}
	if (not ntries) {
		ntries = 100;
	}
	var msg = "";

	var interactive = not system.system.a(33);

	if (0 and USERNAME == "NEOSYS") {
		cout << mode<< " "<< lockkey<< " "<< locklist << endl;
		cout << " ";
		cout << lockfilename;
		xx.input();
	}

	if (mode == "LOCK") {
		allowduplicate = 0;
		goto lockit;
		goto 248;
	}
	if (mode == "RELOCK") {
		allowduplicate = 1;
		goto lockit;
		goto 248;
	}
	if (mode == "UNLOCK") {
		goto unlockit;
		goto 248;
	}
	if (mode == "UNLOCKALL") {
		goto unlockall;
		goto 248;
	}
	if (1) {
		call mssg(DQ ^ (mode ^ DQ) ^ " is invalid in LOCKING");
	}
L248:
	return 0;

	///////
lockit:
	///////

	var lockitem = lockfilename ^ "*" ^ lockkey;
	if (locklist.locateUsing(lockitem, FM, lockn)) {
		//msg=quote(lockitem):' already locked'
		return 1;
	}

	var lockfile;
	if (not(lockfile.open(lockfilename, ""))) {
		//call fsmsg()
		msg = DQ ^ (lockfilename ^ DQ) ^ " file cannot be opened in LOCKING";
		return 0;
	}

	var tryn = 1;
tryagain:
	if (not(lockrecord("", lockfile, lockkey, "", 0, allowduplicate))) {
		var lockholder = lockfilename ^ "*" ^ lockkey.xlate("LOCKS", 4, "X");
		if (lockholder == "") {
			lockholder = "Someone, maybe you,";
		}
		//if lockdesc then
		gosub getlockdesc();
		if (interactive) {
			call mssg(lockholder ^ " is using the " ^ lockdesc, "T1", "", "");
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
					cout << msg<< "*";
					cout << " ";
				}else{
				}
				msg = "";
				call delay(1);
				tryn += 1;
				goto tryagain;
			}
		}
		// end

		return 0;

	}

	locklist.r(-1, lockitem);

	return 1;

}

subroutine getlockdesc() {
	if (not lockdesc) {
		lockdesc = lockfilename ^ " " ^ lockkey;
		lockdesc.converter(".", " ");
	}

	return;

	/////////
unlockit:
	/////////

	if (lockfile.open(lockfilename, "")) {
		xx = unlockrecord("", lockfile, lockkey);
	}

	if (locklist.locateUsing(lockfilename ^ "*" ^ lockkey, FM, lockn)) {
		locklist.eraser(lockn);
	}

	return 1;

	//////////
unlockall:
	//////////
	var filename2 = "";
	var file = "";
	for (var lockn = 1; lockn <= 9999; ++lockn) {
		lockitem = locklist.a(lockn);
	///BREAK;
	if (not lockitem) break;;
		var filename = lockitem.field("*", 1);
		lockkey = lockitem.field("*", 2, 9999);
		if (filename ne filename2) {
			if (not(file.open(filename, ""))) {
				msg.r(-1, DQ ^ (filename ^ DQ) ^ " file cannot be opened in LOCKING,UNLOCKALL");
				file = "";
			}
			filename2 = filename;
		}
		if (file ne "") {
			xx = unlockrecord("", file, lockkey);
		}
	};//lockn;

	locklist = "";

	if (msg) {
		return 0;
	}

	return 1;

}


libraryexit()