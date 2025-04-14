#include <req_common.h>
#include <exodus/library.h>

#include <srv_common.h>

libraryinit()

#include <authorised.h>
#include <locking.h>
#include <service_common.h>


#include <request.hpp>

//var msg;
//var deleting;	 // num
//var validating;	 // num
//var mastercode;
//var vn;
//var op;
//var op2;
//var wspos;
//var wsmsg;
//var xx;

func main(in mode, in subfn, in masterfilename, io masterfile, in masterfn, io locklist) {

	// also needs
	// datafile,src.file

	// and (except build)
	// @id,@record,orec

	if (masterfile.unassigned()) {
		if (not masterfile.open(masterfilename, "")) {
			let msg = masterfilename.quote() ^ " file does not exist";
			return invalid(msg);
		}
	}

	if (mode == "BUILD") {

		// zzz build updates masterfile without locking
		// so should be run exclusively

		clearselect();
		perform("SELECT " ^ req.datafile ^ " (S)");
		if (not LISTACTIVE) {
			return 0;
		}

nextbuild:
		if (readnext(ID)) {
			if (not RECORD.read(req.srcfile, ID)) {
				goto nextbuild;
			}
			if (RECORD.f(subfn) == "") {
				goto nextbuild;
			}
			let deleting   = 0;
			let validating = 0;
			gosub validateupdate(deleting, validating, subfn, masterfilename, masterfile, masterfn, locklist);
			goto nextbuild;
		}

	} else if (mode == "PREWRITE") {
		if (req.orec.f(subfn) == RECORD.f(subfn)) {
			return 0;
		}
		let validating = 1;

		var deleting = 1;
		gosub validateupdate(deleting, validating, subfn, masterfilename, masterfile, masterfn, locklist);
		if (not req.valid) {
			return 0;
		}

		deleting = 0;
		gosub validateupdate(deleting, validating, subfn, masterfilename, masterfile, masterfn, locklist);

	} else if (mode == "PREDELETE") {
		let validating = 1;
		let deleting   = 1;
		gosub validateupdate(deleting, validating, subfn, masterfilename, masterfile, masterfn, locklist);

	} else if (mode == "WRITE") {

		var validating = 1;

		var deleting = 1;
		gosub validateupdate(deleting, validating, subfn, masterfilename, masterfile, masterfn, locklist);
		if (not req.valid) {
			return 0;
		}

		deleting = 0;
		gosub validateupdate(deleting, validating, subfn, masterfilename, masterfile, masterfn, locklist);
		if (not req.valid) {
			return 0;
		}

		validating = 0;

		deleting = 1;
		gosub validateupdate(deleting, validating, subfn, masterfilename, masterfile, masterfn, locklist);
		if (not req.valid) {
			return 0;
		}

		deleting = 0;
		gosub validateupdate(deleting, validating, subfn, masterfilename, masterfile, masterfn, locklist);
		if (not req.valid) {
			return 0;
		}

		gosub unlockall(locklist);

	} else if (mode == "DELETE") {
		let deleting = 1;

		var validating = 1;
		gosub validateupdate(deleting, validating, subfn, masterfilename, masterfile, masterfn, locklist);

		validating = 0;
		gosub validateupdate(deleting, validating, subfn, masterfilename, masterfile, masterfn, locklist);

		gosub unlockall(locklist);

	} else if (mode == "POSTWRITE") {
		if (req.orec.f(subfn) == RECORD.f(subfn)) {
			return 0;
		}
		let validating = 0;

		var deleting = 1;
		gosub validateupdate(deleting, validating, subfn, masterfilename, masterfile, masterfn, locklist);

		deleting = 0;
		gosub validateupdate(deleting, validating, subfn, masterfilename, masterfile, masterfn, locklist);

	} else if (mode == "POSTDELETE") {
		let deleting   = 1;
		let validating = 0;
		gosub validateupdate(deleting, validating, subfn, masterfilename, masterfile, masterfn, locklist);

	} else {
		let msg = mode.quote() ^ " invalid mode in xref.subs";
		return invalid(msg);
	}

	return 0;
}

subr validateupdate(in deleting, in validating, in subfn, in masterfilename, in masterfile, in masterfn, io locklist) {

	let mastercode = deleting ? req.orec.f(subfn) : RECORD.f(subfn);
	if (mastercode == "") {
		return;
	}

	if (validating) {

		var msg;
		if (not locking("LOCK", masterfilename, mastercode, "", locklist, 1, msg)) {
			gosub unlockall(locklist);
			msg = mastercode.quote() ^ " master record is in use elsewhere|Try again later";
			gosub invalid(msg);
			return;
		}

	} else {

		var masterrecord;
		if (masterrecord.read(masterfile, mastercode)) {
			let origmasterrecord = masterrecord;
			var vn;
			if (masterrecord.f(masterfn).locateby("AL", ID, vn)) {
				if (deleting) {
					masterrecord.remover(masterfn, vn);
				}
			} else {
				if (not deleting) {
					masterrecord.inserter(masterfn, vn, ID);
				}
			}
			if (masterrecord != origmasterrecord) {
				masterrecord.write(masterfile, mastercode);
			}
		} else {
		}
	}

	return;
}

subr unlockall(io locklist) {
	call locking("UNLOCKALL", "", "", "", locklist, 0);
	return;
}

}; // libraryexit()
