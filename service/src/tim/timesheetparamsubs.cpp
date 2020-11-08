#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <timesheetparamsubs.h>
#include <singular.h>

#include <gen_common.h>
#include <agy_common.h>
#include <win_common.h>

#include <window.hpp>

var msg;
var title;
var runhours;
var docid;
var reminderapproval;//num
var op;
var op2;
var wspos;
var wsmsg;
var xx;

function main(in mode) {
	//c tim
	#include <general_common.h>
	#include <agency_common.h>

	#include <timesheet_common.h>

	var interactive = not(SYSTEM.a(33));

	var isadmin = authorised("TIMESHEET ADMINISTRATION", msg, "TA");

	if (mode == "SETUPAUTORUN") {
		if (not(RECORD.read(DEFINITIONS, "TIMESHEET.PARAMS"))) {
			call fsmsg();
			return 0;
		}
		call timesheetparamsubs("POSTWRITE");

	} else if (mode == "PREREAD") {

	} else if (mode == "POSTREAD") {

		if (not(authorised("TIMESHEET CONFIGURATION UPDATE", msg, "NEOSYS"))) {
			//valid=0
			//if wlocked then
			// x=unlockrecord(datafile,src.file,@id)
			// wlocked=0
			// end
			//prevent even read
			return invalid(msg);
		}

	} else if (mode == "PREWRITE") {

	} else if (mode == "POSTWRITE") {

		//default reminders at 12:00
		//if @record<11>='' then
		// @record<11>='12'
		// end

		//default approvals at 15:00
		//if @record<12>='' then
		// @record<12>='15'
		// end

		//summary
		title = "Summary";
		runhours = RECORD.a(12);
		docid = "NEOS_TSSU";
		reminderapproval = 2;
		gosub updatedocuments();

		//reminders
		title = "Reminders";
		runhours = RECORD.a(11);
		docid = "NEOS_TSRE";
		reminderapproval = 3;
		gosub updatedocuments();

		//approvals
		title = "Approvals";
		runhours = RECORD.a(12);
		docid = "NEOS_TSAP";
		reminderapproval = 4;
		gosub updatedocuments();

	} else {
		msg = mode.quote() ^ " - unknown mode skipped in TIMESHEETPARAM.SUBS";
		return invalid(msg);
	}

	return 0;
}

subroutine updatedocuments() {
	//delete unless runable
	if (runhours == "") {
		gen.documents.deleterecord(docid);

		return;
	}

	if (not(gen.document.read(gen.documents, docid))) {
		gen.document = "";
	}

	var runasusercode = RECORD.a(14).a(1, 1);
	if (not runasusercode) {
		runasusercode = "NEOSYS";
	}
	gen.document.r(1, runasusercode);
	gen.document.r(2, "Timesheet " ^ title);
	gen.document.r(3, var().date());
	gen.document.r(4, var().time());
	gen.document.r(5, "JOBPROXY" _VM_ "TIMESHEETALERTS");
	gen.document.r(6, 2, reminderapproval);
	if (reminderapproval == 2) {
		gen.document.r(6, 4, 31);
	}
	gen.document.r(7, "ADAGENCY");
	gen.document.r(12, "1");
	gen.document.r(14, RECORD.a(14));
	gen.document.r(22, runhours);
	gen.document.r(31, "JOB");
	gen.document.r(32, "");
	gen.document.write(gen.documents, docid);

	return;
}

libraryexit()
