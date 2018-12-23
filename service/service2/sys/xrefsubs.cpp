#include <exodus/library.h>
libraryinit()

#include <locking.h>
#include <singular.h>
#include <authorised.h>

#include <gen.h>
#include <win.h>

#include <window.hpp>

var msg;
var deleting;//num
var validating;//num
var mastercode;
var vn;
var wsmsg;

function main(in mode, in subfn, in masterfilename, io masterfile, in masterfn, io locklist) {
	//c sys in,in,in,io,in,io

	//also needs
	//datafile,src.file

	//and (except build)
	//@id,@record,orec

	if (masterfile.unassigned()) {
		if (not(masterfile.open(masterfilename, ""))) {
			msg = DQ ^ (masterfilename ^ DQ) ^ " file does not exist";
			return invalid();
		}
	}

	if (mode == "BUILD") {

		//zzz build updates masterfile without locking
		//so should be run exclusively

		var().clearselect();
		perform("SELECT " ^ win.datafile ^ " (S)");
		if (not LISTACTIVE) {
			return 0;
		}

nextbuild:
		if (readnext(ID)) {
			if (not(RECORD.read(win.srcfile, ID))) {
				goto nextbuild;
			}
			if (RECORD.a(subfn) == "") {
				goto nextbuild;
			}
			deleting = 0;
			validating = 0;
			gosub validateupdate( subfn,  masterfilename,  masterfile,  masterfn,  locklist);
			goto nextbuild;
		}

	} else if (mode == "PREWRITE") {
		if (win.orec.a(subfn) == RECORD.a(subfn)) {
			return 0;
		}
		validating = 1;

		deleting = 1;
		gosub validateupdate( subfn,  masterfilename,  masterfile,  masterfn,  locklist);
		if (not win.valid) {
			return 0;
		}

		deleting = 0;
		gosub validateupdate( subfn,  masterfilename,  masterfile,  masterfn,  locklist);

	} else if (mode == "PREDELETE") {
		validating = 1;
		deleting = 1;
		gosub validateupdate( subfn,  masterfilename,  masterfile,  masterfn,  locklist);

	} else if (mode == "WRITE") {

		validating = 1;

		deleting = 1;
		gosub validateupdate( subfn,  masterfilename,  masterfile,  masterfn,  locklist);
		if (not win.valid) {
			return 0;
		}

		deleting = 0;
		gosub validateupdate( subfn,  masterfilename,  masterfile,  masterfn,  locklist);
		if (not win.valid) {
			return 0;
		}

		validating = 0;

		deleting = 1;
		gosub validateupdate( subfn,  masterfilename,  masterfile,  masterfn,  locklist);
		if (not win.valid) {
			return 0;
		}

		deleting = 0;
		gosub validateupdate( subfn,  masterfilename,  masterfile,  masterfn,  locklist);
		if (not win.valid) {
			return 0;
		}

		gosub unlockall( locklist);

	} else if (mode == "DELETE") {
		deleting = 1;

		validating = 1;
		gosub validateupdate( subfn,  masterfilename,  masterfile,  masterfn,  locklist);

		validating = 0;
		gosub validateupdate( subfn,  masterfilename,  masterfile,  masterfn,  locklist);

		gosub unlockall( locklist);

	} else if (mode == "POSTWRITE") {
		if (win.orec.a(subfn) == RECORD.a(subfn)) {
			return 0;
		}
		validating = 0;

		deleting = 1;
		gosub validateupdate( subfn,  masterfilename,  masterfile,  masterfn,  locklist);

		deleting = 0;
		gosub validateupdate( subfn,  masterfilename,  masterfile,  masterfn,  locklist);

	} else if (mode == "POSTDELETE") {
		deleting = 1;
		validating = 0;
		gosub validateupdate( subfn,  masterfilename,  masterfile,  masterfn,  locklist);

	} else if (1) {
		msg = DQ ^ (mode ^ DQ) ^ " invalid mode in xref.subs";
		return invalid();
	}
//L524:
	return 0;

}

subroutine validateupdate(in subfn, in masterfilename, in masterfile, in masterfn, io locklist) {
	//validateupdate(in subfn, in masterfilename, in masterfile, in masterfn, io locklist)
	if (deleting) {
		mastercode = win.orec.a(subfn);
	}else{
		mastercode = RECORD.a(subfn);
	}
	if (mastercode == "") {
		return;
	}

	if (validating) {

		if (not(locking("LOCK", masterfilename, mastercode, "", locklist, 1, msg))) {
			gosub unlockall( locklist);
			msg = DQ ^ (mastercode ^ DQ) ^ " master record is in use elsewhere|Try again later";
			gosub invalid();
			return;
		}

	}else{

		var masterrecord;
		if (masterrecord.read(masterfile, mastercode)) {
			var origmasterrecord = masterrecord;
			if (masterrecord.a(masterfn).locateby(ID, "AL", vn)) {
				if (deleting) {
					masterrecord.eraser(masterfn, vn);
				}
			}else{
				if (not deleting) {
					masterrecord.inserter(masterfn, vn, ID);
				}
			}
			if (masterrecord ne origmasterrecord) {
				masterrecord.write(masterfile, mastercode);
			}
		}else{
		}

	}

	return;

}

subroutine unlockall(io locklist) {
	//unlockall(io locklist)
	call locking("UNLOCKALL", "", "", "", locklist, 0, msg);
	return;

}


libraryexit()