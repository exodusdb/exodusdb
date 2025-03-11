#include <exodus/library.h>
#include <srv_common.h>
#include <req_common.h>

#include <req_common.h>

libraryinit()

#include <authorised.h>

#include <service_common.h>

#include <request.hpp>

//var xx;
//var tt;
//var task;
//var taskprefix;
var msg;
//var op;
//var op2;
//var wspos;
//var wsmsg;

function main(in mode) {

	// declare function lockposting

	// "where" is not really functional in the intranet system
	// since all document text is temp placed in the ..\data\datasetcode\ folder
	// but it is still used to store the last document number though
	// where='docs\public\'
	var where = "docs/public/";
	where.converter("/", OSSLASH);

	if (srv.documents.unassigned()) {
		if (not srv.documents.open("DOCUMENTS", "")) {
			req.valid = 0;
			note(lasterror());
			return 0;
		}
	}

	if (mode == "DEF.DOCUMENT.NO") {

		// lock source of document numbers
		printl("*lock source of document numbers");
		var dummy;
		if (not lockrecord("DOCUMENTS", srv.documents, "0", dummy, 10)) {
			req.valid = 0;
			return 0;
		}

		// get/update next document number
		var nextno;
		if (not nextno.read(srv.documents, "0")) {
			if (not nextno.osread(where ^ "0")) {
				nextno = "";
			}
		}

nextdoc:
		nextno += 1;
		// call oswrite(nextno,where:'0')

		// build new file name
		let tt = (USERNAME == "EXODUS") ? "NEO" : "DOC";
		ID = tt ^ ("00000" ^ nextno).oconv("R#5");

		// skip to next file if already exists
		// osopen lcase(where:temp) to temp2 then
		// osclose temp2
		// goto nextdoc
		// end

		// skip if document already exists
		var doc;
		if (doc.read(srv.documents, ID)) {
			goto nextdoc;
		}
		if (doc.read(srv.documents, where ^ ID)) {
			goto nextdoc;
		}

		unlockrecord("DOCUMENTS", srv.documents, "0");

	} else if (mode == "POSTREAD") {

		if (not RECORD) {
			return 0;
		}

		let task = req.orec.f(5);
		let taskprefix = gosub gettaskprefix(task);

		// dont know task at this point
		// if taskprefix then
		// if security(taskprefix:' CREATE',msg,'') else
		//  call msg(msg)
		//  reset=5
		//  end
		// end

		// user can always modify own reports
		if ((req.wlocked and RECORD.f(1)) and RECORD.f(1) != USERNAME) {

			// check if allowed to modify
			if (taskprefix) {
				if (not authorised(taskprefix ^ " UPDATE", msg, "")) {
					call note(msg);
					unlockrecord(req.datafile, req.srcfile, ID);
					req.wlocked = 0;
				}
			}

			// always prevent users from editing documents designed by EXODUS
			if (RECORD.f(1).contains("EXODUS") and not(USERNAME.contains("EXODUS"))) {
				call note("You cannot modify report designs created by EXODUS|Use the Copy button to copy them and modify the copy");
				unlockrecord(req.datafile, req.srcfile, ID);
				req.wlocked = 0;
			}
		}

		RECORD(101) = raise(RECORD.f(6));

		if (RECORD.f(8) == "") {
			RECORD(8) = RECORD.f(3) ^ "." ^ RECORD.f(4).oconv("R(0)#5");
		}

	} else if (mode == "PREWRITE") {

		// check if allowed to create
		if (req.orec == "") {
			let task = RECORD.f(5);
			let taskprefix = gosub gettaskprefix(task);
			if (taskprefix) {
				if (not authorised(taskprefix ^ " CREATE", msg, "")) {
					return invalid(msg);
				}
			}
		}

		// move fields 101 onwards into field 6 (after lowering)
		// and remove fields 101 onwards
		RECORD(6) = lower(RECORD.field(FM, 101, 9999));
		RECORD	  = RECORD.field(FM, 1, 100);

		if (RECORD.f(1) == "") {
			RECORD(1) = USERNAME;
		}
		RECORD(3) = date();
		RECORD(4) = time();
		RECORD(7) = APPLICATION;

		// update exodus standard (in case doing this on the programming system)
		// the programming standard is installed into all clients
		// on first login after upgrade
		// this is also done in copygbp perhaps could be removed from there
		// almost identical code in definition.subs and get.subs (for documents)
		// field 10 in documents and definitions xxx*analdesign means the same
		if (USERNAME == "EXODUS" and RECORD.f(10)) {
			var reports;
			if (reports.open("REPORTS", "")) {
				var key = ID;
				key.replacer("*", "%2A");
				RECORD.write(reports, "DOCUMENTS*" ^ key);
			}
		}

	} else if (mode == "POSTWRITE") {

	} else if (mode == "PREDELETE") {

		let task = req.orec.f(5);
		let taskprefix = gosub gettaskprefix(task);

		// user can always delete their own reports
		if (RECORD.f(1) and RECORD.f(1) != USERNAME) {

			// check if allowed to delete
			if (taskprefix) {
				if (not authorised(taskprefix ^ " DELETE", msg, "")) {
					return invalid(msg);
				}
			}

			// always prevent users from deleting documents designed by EXODUS
			if (RECORD.f(1).contains("EXODUS") and not(USERNAME.contains("EXODUS"))) {
				msg = "You cannot delete report designs created by EXODUS";
				return invalid(msg);
			}
		}

		// update exodus standard (in case doing this on the programming system)
		// %DELETED% ensures that deleted EXODUS documents get deleted
		// on upgrading clients
		if (USERNAME == "EXODUS" and RECORD.f(10)) {
			var reports;
			if (reports.open("REPORTS", "")) {
				var key = ID;
				key.replacer("*", "%2A");
				key = "DOCUMENTS*" ^ key;
				if (var().read(reports, key)) {
					var("%DELETED%").write(reports, key);
				}
				// delete reports,key
			}
		}

	} else if (mode == "POSTDELETE") {

	} else {
		call note(mode.quote() ^ " is invalid in GET.SUBS");
	}

	return 0;

	// in get.subs and generalproxy
}

function gettaskprefix(in task) {
	var taskprefix = "";
	var task1	   = task.field(" ", 1);
	if (task1 == "ANAL") {
		taskprefix = "BILLING REPORT";
	} else if (task1 == "BALANCES") {
		taskprefix = "FINANCIAL REPORT";
	} else if (task1 == "ANALSCH") {
		taskprefix = "BILLING REPORT";
	} else {
		taskprefix = "";
	}
	return taskprefix;
}

libraryexit()
