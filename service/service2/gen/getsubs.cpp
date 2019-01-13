#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <singular.h>

#include <gen.h>
#include <win.h>

#include <window.hpp>

var xx;
var nextno;//num
var tt;
var temp2;
var taskprefix;
var msg;
var wsmsg;

function main(in mode) {
	//c gen
	//declare function lockposting

	//"where" is not really functional in the intranet system
	//since all document text is temp placed in the ..\DATA\datasetcode\ folder
	//but it is still used to store the last document number though
	var where = "DOCS\\PUBLIC\\";

	if (not(gen.documents.open("DOCUMENTS", ""))) {
		call fsmsg();
		win.valid = 0;
		return 0;
		}

	if (mode == "DEF.DOCUMENT.NO") {

		var verbs;
		if (not(verbs.open("VERBS", ""))) {
			call fsmsg();
			win.valid = 0;
			return 0;
		}

nextdoc:

		//lock source of document numbers
		printl("*lock source of document numbers");
		if (not(lockrecord("VERBS", verbs, "0", xx, 10))) {
			win.valid = 0;
			return 0;
		}

		//get/update next document number
		if (not(nextno.osread(where ^ "0"))) {
			if (nextno.read(gen.documents, "0")) {
				gen.documents.deleterecord("0");
				
			}else{
				nextno = "";
			}
		}
		nextno += 1;
		call oswrite(nextno, where ^ "0");

		//build new file name
		if (USERNAME == "NEOSYS") {
			tt = "NEO";
		}else{
			tt = "DOC";
		}
		var temp = tt ^ var("00000" ^ nextno).oconv("R#5");

		//skip to next file if already exists
		if (temp2.osopen(where ^ temp)) {
			temp2.osclose();
			goto nextdoc;
		}

		//skip if document already exists
		if (temp.read(gen.documents, temp)) {
			goto nextdoc;
		}
		if (temp.read(gen.documents, where ^ temp)) {
			goto nextdoc;
		}

		ID = temp;

		xx = unlockrecord("VERBS", verbs, "0");

	} else if (mode == "POSTREAD") {

		if (not RECORD) {
			return 0;
		}

		var task = win.orec.a(5);
		gosub gettaskprefix();

		//dont know task at this point
		//if taskprefix then
		// if security(taskprefix:' CREATE',msg,'') else
		// call msg(msg)
		// reset=5
		// end
		// end

		//user can always modify own reports
		if (win.wlocked and RECORD.a(1) and RECORD.a(1) ne USERNAME) {

			//check if allowed to modify
			if (taskprefix) {
				if (not(authorised(taskprefix ^ " UPDATE", msg, ""))) {
					call mssg(msg);
					xx = unlockrecord(win.datafile, win.srcfile, ID);
					win.wlocked = 0;
				}
			}

			//always prevent users from editing documents designed by NEOSYS
			if ((RECORD.a(1)).index("NEOSYS", 1) and not USERNAME.index("NEOSYS", 1)) {
				call mssg("You cannot modify report designs created by NEOSYS|Use the Copy button to copy them and modify the copy");
				xx = unlockrecord(win.datafile, win.srcfile, ID);
				win.wlocked = 0;
			}

		}

		RECORD.r(101, raise(RECORD.a(6)));

		if (RECORD.a(8) == "") {
			RECORD.r(8, RECORD.a(3) ^ "." ^ (RECORD.a(4)).oconv("R(0)#5"));
		}

	} else if (mode == "PREWRITE") {

		//check if allowed to create
		if (win.orec == "") {
			var task = RECORD.a(5);
			gosub gettaskprefix();
			if (taskprefix) {
				if (not(authorised(taskprefix ^ " CREATE", msg, ""))) {
					return invalid(msg);
				}
			}

		}

		//move fields 101 onwards into field 6 (after lowering)
		//and remove fields 101 onwards
		RECORD.r(6, lower(RECORD.field(FM, 101, 9999)));
		RECORD = RECORD.field(FM, 1, 100);

		if (RECORD.a(1) == "") {
			RECORD.r(1, USERNAME);
		}
		RECORD.r(3, var().date());
		RECORD.r(4, var().time());
		RECORD.r(7, APPLICATION);

		//update neosys standard (in case doing this on the programming system)
		//the programming standard is installed into all clients
		//on first login after upgrade
		//this is also done in copygbp perhaps could be removed from there
		//almost identical code in definition.subs and get.subs (for documents)
		//field 10 in documents and definitions xxx*analdesign means the same
		if (USERNAME == "NEOSYS" and RECORD.a(10)) {
			var reports;
			if (reports.open("REPORTS", "")) {
				var key = ID;
				key.swapper("*", "%2A");
				RECORD.write(reports, "DOCUMENTS*" ^ key);
			}
		}

	} else if (mode == "POSTWRITE") {

	} else if (mode == "PREDELETE") {

		var task = win.orec.a(5);
		gosub gettaskprefix();

		//user can always delete their own reports
		if (RECORD.a(1) and RECORD.a(1) ne USERNAME) {

			//check if allowed to delete
			if (taskprefix) {
				if (not(authorised(taskprefix ^ " DELETE", msg, ""))) {
					return invalid(msg);
				}
			}

			//always prevent users from deleting documents designed by NEOSYS
			if ((RECORD.a(1)).index("NEOSYS", 1) and not USERNAME.index("NEOSYS", 1)) {
				msg = "You cannot delete report designs created by NEOSYS";
				return invalid(msg);
			}

		}

		//update neosys standard (in case doing this on the programming system)
		//%DELETED% ensures that deleted NEOSYS documents get deleted
		//on upgrading clients
		if (USERNAME == "NEOSYS" and RECORD.a(10)) {
			var reports;
			if (reports.open("REPORTS", "")) {
				var key = ID;
				key.swapper("*", "%2A");
				key = "DOCUMENTS*" ^ key;
				if (xx.read(reports, key)) {
					var("%DELETED%").write(reports, key);
				}
				//delete reports,key
			}
		}

		} else if (mode == "POSTDELETE") {

	} else {
		call mssg(DQ ^ (mode ^ DQ) ^ " is invalid in GET.SUBS");
	}
//L1041:
	return 0;

	//in get.subs and generalproxy
}

subroutine gettaskprefix() {
	taskprefix = "";
	var task = task.field(" ", 1);
	if (task == "ANAL") {
		taskprefix = "BILLING REPORT";
	} else if (task == "BALANCES") {
		taskprefix = "FINANCIAL REPORT";
	} else if (task == "ANALSCH") {
		taskprefix = "BILLING REPORT";
	} else {
		taskprefix = "";
	}
	return;

}


libraryexit()
