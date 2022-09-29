#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <singular.h>

#include <system_common.h>

#include <sys_common.h>
#include <win_common.h>

#include <window.hpp>

var xx;
var tt;
var task;
var taskprefix;
var msg;
var op;
var op2;
var wspos;
var wsmsg;

function main(in mode) {

	//declare function lockposting

	//"where" is not really functional in the intranet system
	//since all document text is temp placed in the ..\data\datasetcode\ folder
	//but it is still used to store the last document number though
	//where='docs\public\'
	var where = "docs/public/";
	where.converter("/", OSSLASH);

	if (sys.documents.unassigned()) {
		if (not(sys.documents.open("DOCUMENTS", ""))) {
			call fsmsg();
			win.valid = 0;
			return 0;
		}
	}

	if (mode eq "DEF.DOCUMENT.NO") {

		//lock source of document numbers
		printl("*lock source of document numbers");
		if (not(lockrecord("DOCUMENTS", sys.documents, "0", xx, 10))) {
			win.valid = 0;
			return 0;
		}

		//get/update next document number
		var nextno;
		if (not(nextno.read(sys.documents, "0"))) {
			if (not(nextno.osread(where ^ "0"))) {
				nextno = "";
			}
		}

nextdoc:
		nextno += 1;
		//call oswrite(nextno,where:'0')

		//build new file name
		if (USERNAME eq "EXODUS") {
			tt = "NEO";
		} else {
			tt = "DOC";
		}
		ID = tt ^ ("00000" ^ nextno).oconv("R#5");

		//skip to next file if already exists
		//osopen lcase(where:temp) to temp2 then
		// osclose temp2
		// goto nextdoc
		// end

		//skip if document already exists
		if (tt.read(sys.documents, ID)) {
			goto nextdoc;
		}
		if (tt.read(sys.documents, where ^ ID)) {
			goto nextdoc;
		}

		xx = unlockrecord("DOCUMENTS", sys.documents, "0");

	} else if (mode eq "POSTREAD") {

		if (not RECORD) {
			return 0;
		}

		task = win.orec.f(5);
		gosub gettaskprefix();

		//dont know task at this point
		//if taskprefix then
		// if security(taskprefix:' CREATE',msg,'') else
		//  call msg(msg)
		//  reset=5
		//  end
		// end

		//user can always modify own reports
		if ((win.wlocked and RECORD.f(1)) and RECORD.f(1) ne USERNAME) {

			//check if allowed to modify
			if (taskprefix) {
				if (not(authorised(taskprefix ^ " UPDATE", msg, ""))) {
					call mssg(msg);
					xx = unlockrecord(win.datafile, win.srcfile, ID);
					win.wlocked = 0;
				}
			}

			//always prevent users from editing documents designed by EXODUS
			if (RECORD.f(1).contains("EXODUS") and not(USERNAME.contains("EXODUS"))) {
				call mssg("You cannot modify report designs created by EXODUS|Use the Copy button to copy them and modify the copy");
				xx = unlockrecord(win.datafile, win.srcfile, ID);
				win.wlocked = 0;
			}

		}

		RECORD(101) = raise(RECORD.f(6));

		if (RECORD.f(8) eq "") {
			RECORD(8) = RECORD.f(3) ^ "." ^ RECORD.f(4).oconv("R(0)#5");
		}

	} else if (mode eq "PREWRITE") {

		//check if allowed to create
		if (win.orec eq "") {
			task = RECORD.f(5);
			gosub gettaskprefix();
			if (taskprefix) {
				if (not(authorised(taskprefix ^ " CREATE", msg, ""))) {
					return invalid(msg);
				}
			}

		}

		//move fields 101 onwards into field 6 (after lowering)
		//and remove fields 101 onwards
		RECORD(6) = lower(RECORD.field(FM, 101, 9999));
		RECORD = RECORD.field(FM, 1, 100);

		if (RECORD.f(1) eq "") {
			RECORD(1) = USERNAME;
		}
		RECORD(3) = date();
		RECORD(4) = time();
		RECORD(7) = APPLICATION;

		//update exodus standard (in case doing this on the programming system)
		//the programming standard is installed into all clients
		//on first login after upgrade
		//this is also done in copygbp perhaps could be removed from there
		//almost identical code in definition.subs and get.subs (for documents)
		//field 10 in documents and definitions xxx*analdesign means the same
		if (USERNAME eq "EXODUS" and RECORD.f(10)) {
			var reports;
			if (reports.open("REPORTS", "")) {
				var key = ID;
				key.replacer("*", "%2A");
				RECORD.write(reports, "DOCUMENTS*" ^ key);
			}
		}

	} else if (mode eq "POSTWRITE") {

	} else if (mode eq "PREDELETE") {

		task = win.orec.f(5);
		gosub gettaskprefix();

		//user can always delete their own reports
		if (RECORD.f(1) and RECORD.f(1) ne USERNAME) {

			//check if allowed to delete
			if (taskprefix) {
				if (not(authorised(taskprefix ^ " DELETE", msg, ""))) {
					return invalid(msg);
				}
			}

			//always prevent users from deleting documents designed by EXODUS
			if (RECORD.f(1).contains("EXODUS") and not(USERNAME.contains("EXODUS"))) {
				msg = "You cannot delete report designs created by EXODUS";
				return invalid(msg);
			}

		}

		//update exodus standard (in case doing this on the programming system)
		//%DELETED% ensures that deleted EXODUS documents get deleted
		//on upgrading clients
		if (USERNAME eq "EXODUS" and RECORD.f(10)) {
			var reports;
			if (reports.open("REPORTS", "")) {
				var key = ID;
				key.replacer("*", "%2A");
				key = "DOCUMENTS*" ^ key;
				if (xx.read(reports, key)) {
					var("%DELETED%").write(reports, key);
				}
				//delete reports,key
			}
		}

	} else if (mode eq "POSTDELETE") {

	} else {
		call mssg(mode.quote() ^ " is invalid in GET.SUBS");
		}

	return 0;

	//in get.subs and generalproxy
}

subroutine gettaskprefix() {
	taskprefix = "";
	task = task.field(" ", 1);
	if (task eq "ANAL") {
		taskprefix = "BILLING REPORT";
	} else if (task eq "BALANCES") {
		taskprefix = "FINANCIAL REPORT";
	} else if (task eq "ANALSCH") {
		taskprefix = "BILLING REPORT";
	} else {
		taskprefix = "";
	}
	return;
}

libraryexit()
