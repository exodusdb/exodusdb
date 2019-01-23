#include <exodus/library.h>
libraryinit()

#include <autorun2.h>
#include <singular.h>
#include <authorised.h>

#include <agy.h>
#include <gen.h>
#include <win.h>

#include <window.hpp>

var mode;
var versionfile;
var msg;
var versionfilename;
var versionsepchar;
var xx;
var version;//num
var currentversion;
var status;
var statusfn;//num
var defaultstatus;
var versionfn;//num
var firstversion;
var statuslogfn;//num
var ologn;//num
var useversioneconomy;//num
var datetimelogfn;
var userlogfn;
var stationlogfn;
var oldversion;
var appending;//num
var logn;//num
var versionlogfn;//num
var execn;
var status2;
var subject;
var module;
var request;
var datax;
var task;
var taskn;
var yy;
var zz;
var ii;//num
var wsmsg;

function main(in mode0) {
	//c gen
	//global mode

	var interactive = not SYSTEM.a(33);

	//was agency.subs2 but has company and currency in it so moved here
	//so that accounts only module would not work

	//because valid doesnt seem to be set in vehicle.subs postread!
	//maybe the problem extends elsewhere
	win.valid = 1;
	mode = mode0;

	//begin case
	//case mode='POSTINIT'

	if (mode.index("POSTREAD")) {

		//if in main file then nothing to do
		if (RECORD ne "") {
			return 0;
		}

		//following is only when not in main file
		////////////////////////////////////////

		gosub getfiledefaults();
		if (not(win.valid)) {
			return 0;
		}

		//if opening documents then look in versions file
		//if just reading then do not
		//if wlocked then
		if (var(1) or win.wlocked) {

			//if not in version file and no ~ character in key then
			//assume is an ordinary new record
			if (not(RECORD.read(versionfile, ID))) {

				//not in versions file and no ~ in key -> ordinary new record
				if (not(ID.index("~"))) {
					return 0;
				}

				//~ in key but not in versions file
				msg = DQ ^ (ID ^ DQ) ^ " missing from " ^ versionfilename ^ " file";
				return invalid(msg);
			}

			//if found a deleted record, return it (still locked so they can save it)
			if (versionsepchar and not ID.index(versionsepchar)) {
				win.orec = RECORD;
				return 0;
			}

		}

		//found in version file or not looked because not wlocked

		win.orec = RECORD;

		//need to unlock BEFORE changing ~ to * in @id
		xx = unlockrecord("", win.srcfile, ID);
		win.wlocked = 0;

		//change ~ to * to try and prevent ~ appearing in multipart key date fields
		//to allow dates to be presented correctly
		if (ID.index("*")) {
			ID.converter("~", "*");
		}

		//allow to pass usual security checks

	} else if (mode.index("PREDELETE")) {

		if (not(win.wlocked)) {
			return 0;
		}

		if (not(win.orec)) {
			return 0;
		}

		RECORD = win.orec;

		gosub getfiledefaults();
		if (not(win.valid)) {
			return 0;
		}

		version = currentversion;
		status = "DELETED";
		gosub addlog();

		RECORD.write(versionfile, ID);

		RECORD = "";

	} else if (mode.index("PREWRITE")) {

		if (not(win.wlocked)) {
			return 0;
		}

		//standard code to log version and updates

		gosub getfiledefaults();
		if (not(win.valid)) {
			return 0;
		}

		//skip out if no changes
		if (RECORD == win.orec) {

			//force write not to skip
			win.orec ^= FM ^ FM ^ FM;

			//////
			return 0;
			//////

		}

		if (statusfn > 0) {
			status = RECORD.a(statusfn).trim();
			if (win.datafile == "JOBS") {
				//'OPEN' is coded in general.subs2 and job.subs postread, client.subs,postwrite
				if (status == "") {
					status = "OPEN";
				}
				if (status == "Y") {
					status = "CLOSED";
				}
				if (status == "N") {
					status = "REOPENED";
				}

			//VOUCHERS also updated in UPD.VOUCHER2
			} else if (win.datafile == "VOUCHERS") {
				if (status.a(1, 1) == "D") {
					status = "UNPOSTED";
				}else{
					//status='POSTED'
					status = "AMENDED";
				}
			}
//L643:
			//if status='' then status='ISSUED'

			if (status == "") {
				status = defaultstatus;
			}
		}

		if (versionfn) {
			version = RECORD.a(versionfn);
			//if version='' then version='A'
			if (version == "") {
				version = firstversion;
				RECORD.r(versionfn, firstversion);
			}
		}

		var anythingchanged = 1;

		//no orec means creating (or restoring)
		if (win.orec == "") {

			//restoring a deleted record
			if (RECORD.a(statuslogfn, ologn) == "DELETED") {

				//save the deleted version
				//save the previous version

				var tt;
				if (tt.read(versionfile, ID)) {
					tt.write(versionfile, ID ^ versionsepchar ^ version);
					versionfile.deleterecord(ID);
					
				}

				//increment version
				if (statusfn) {
					gosub incrementversion();
				}

			}

			//orec means updating
		}else{

			//update the version if anything changed except status
			anythingchanged = 0;
			if (statusfn > 0) {
				var temporec = win.orec.replace(statusfn, 0, 0, RECORD.a(statusfn));

				//special case amending order no AFTER invoicing
				//if not considered changing anything (so that the version remains)
				if ((win.datafile == "PRODUCTION_INVOICES") and RECORD.a(34).index("INVOICED")) {
					temporec.r(7, RECORD.a(7));
				}

				if (temporec ne RECORD) {
					anythingchanged = 1;
				}
				temporec = "";
			}else{
				anythingchanged = 1;
			}

			//option not to save version if status/user/date/workstation same
			if (anythingchanged and useversioneconomy) {
				var lastdate = win.orec.a(datetimelogfn, ologn).field(".", 1);
				if (lastdate == var().date()) {
					var lastuser = win.orec.a(userlogfn, ologn);
					if (lastuser == USERNAME) {
						var laststation = win.orec.a(stationlogfn, ologn);
						if (laststation == STATION.trim()) {
							if (statusfn > 0) {
								var laststatus = win.orec.a(statuslogfn, ologn);
								if (laststatus == status) {
									anythingchanged = 0;
								}
							}else{
								anythingchanged = 0;
							}
						}
					}
				}
			}

			oldversion = version;

			if (anythingchanged) {

				//save the previous version
				win.orec.write(versionfile, ID ^ versionsepchar ^ version);

				gosub incrementversion();

			}

			//orec<>''
		}

		//update log fields

		//could update last log instead of inserting if doing versioneconomy?
		//would lose complete datetime record but avoid confusing same versions

		//this condition will suppress recording every change in the document log
		//for timesheets
		if ((win.datafile == "TIMESHEETS") and RECORD.a(8) ne win.orec.a(8)) {
			//here it means add an entry in the log
			anythingchanged = 1;
		}

		if (anythingchanged or win.datafile ne "TIMESHEETS") {

			//backward compatible with old records without versioning
			//save version 1 line even if record doesnt exist
			if (((win.orec and appending) and (logn == 1)) and versionfn) {
				RECORD.inserter(versionlogfn, logn, oldversion);
				logn += 1;
			}

			gosub addlog();

		}

		//update
		//add new executives/reactivate executives if reused
		//TODO obtain temporary lock
		if ((win.datafile == "JOBS") or (win.datafile == "SCHEDULES")) {
			var execcode = calculate("EXECUTIVE_CODE");
			var indexvalueskey = "INDEXVALUES*" ^ win.datafile ^ "*EXECUTIVE_CODE";
			var indexvalues;
			if (indexvalues.read(DEFINITIONS, indexvalueskey)) {
				if (indexvalues.a(1).locateby(execcode, "AL", execn)) {
					//reactivate so seen when creating new documents again
					if (indexvalues.a(1, execn)) {
						indexvalues.r(2, execn, "");
						goto updindexvalues;
					}

				}else{
					//add so all are seen if updating "Executive File"
					//NB no "Executive File" on UI as at 2014/06/26 and probably not needed
					//can adjust frequency and age of clearing (default one year) in autorun task
					indexvalues.inserter(1, execn, execcode);
					indexvalues.inserter(2, execn, "");
updindexvalues:
					indexvalues.write(DEFINITIONS, indexvalueskey);
				}
			}
		}

		//create a onetime autorun task to print and email the document to the group

		if (win.datafile == "JOBS") {
			status2 = calculate("STATUS2");
			subject = "Job " ^ ID ^ calculate("VERSION") ^ " " ^ status2 ^ " (" ^ calculate("EXECUTIVE_NAME") ^ ") " ^ calculate("DESCRIPTION1");
			module = "PRODUCTION";
			request = "JOBPRINT";
			datax = ID;
			task = "JOB";

		} else if (win.datafile == "PRODUCTION_ORDERS") {
			status2 = calculate("STATUS2");
			subject = "Cost " ^ ID ^ calculate("VERSION") ^ " " ^ status2 ^ " (" ^ calculate("EXECUTIVE_NAME") ^ ") " ^ calculate("DESCRIPTION");
			module = "PRODUCTION";
			request = "COSTESTIMATEPRINT";
			datax = "COST" ^ FM ^ ID;
			if (status2.ucase() == "REOPENED") {
				status2 = "OPEN";
			}
			task = "PURCHASE ORDER";

		} else if (win.datafile == "PRODUCTION_INVOICES") {
			status2 = calculate("STATUS2");
			subject = "Estimate " ^ ID ^ calculate("VERSION") ^ " " ^ status2 ^ " (" ^ calculate("EXECUTIVE_NAME") ^ ") " ^ calculate("DESCRIPTION");

			status = status2.ucase();
			if (status.index("INVOICE") or status.index("PROFORMA")) {

				//reprint invoice or proforma (risk of confusion due to no option autodetect)
				module = "AGENCY";
				request = "REPRINTINVS";

				if (status.index("PROFORMA")) {
					datax = calculate("PROFORMA_INVOICE_NO").a(1, 1);
				}else{
					datax = calculate("INVOICE_NO");
				}
				datax.r(3, "PRODUCTION");
				datax.r(43, calculate("COMPANY_CODE"));

			}else{
				module = "PRODUCTION";
				request = "COSTESTIMATEPRINT";
				datax = "ESTIMATE" ^ FM ^ ID;
				status2 = status;
			}
			task = "ESTIMATE";

		} else {
			module = "";
			request = "";
			subject = "";
		}
//L1849:
		//create a onetime autorun task
		if (module and request) {

			task ^= " MONITOR " ^ status2.ucase();

			var monitorusers = "";

			//find key otherwise no monitors
			if (not(SECURITY.a(10).locateusing(task, VM, taskn))) {
				return 0;
			}
			var locks = SECURITY.a(11, taskn);
			if (not locks) {
				return 0;
			}

			//find all users with the key (no hierarchy!)
			var keys = SECURITY.a(2);
			var nusers = keys.count(VM) + 1;
			for (var usern = 1; usern <= nusers; ++usern) {
				var usercode = SECURITY.a(1, usern);
				//locate key in keys<1,user> using ',' setting xx then
				if (usercode ne "---" and var(2).rnd()) {
					monitorusers ^= SVM ^ usercode;
				}else{
					//break between groups so we can run with the lowest user per group
					if ((usercode == "") or (usercode == "---")) {
						monitorusers ^= VM;
					}
				}
			};//usern;

			//run for each group with the last (bottom) user as the runas user
			//in order to ensure lowest common denominator authorisations
			var ngroups = monitorusers.count(VM) + 1;
			for (var groupn = 1; groupn <= ngroups; ++groupn) {
				var groupusers = trim(monitorusers.a(1, groupn), SVM);
				if (groupusers) {
					groupusers.converter(SVM, VM);
					call autorun2("ASAP", subject, module, request, datax, groupusers.field(VM, -1), groupusers, xx, yy, zz);
					//ignore any failures to create for now in order not to bother updaters
				}
			};//groupn;

		}

	}
//L2143:
	return 0;

}

subroutine getfiledefaults() {
	versionfn = "";
	firstversion = "1";
	statusfn = "";
	defaultstatus = "";
	versionsepchar = "~";
	var logfn = 30;
	versionlogfn = "";
	statuslogfn = "";
	//add on end by default
	appending = 1;
	//option to save only one version per user per day per status
	useversioneconomy = 1;

	versionfilename = singular(win.datafile) ^ "_VERSIONS";
	if (not(versionfile.open(versionfilename, ""))) {
		if (not(ID.index("~"))) {
			return;
		}
		msg = versionfilename ^ " file is missing";
		gosub invalid(msg);
		return;
	}

	if (win.datafile == "TIMESHEETS") {
		versionfn = 7;
		statusfn = 8;
		//firstversion='1'
		//defaultstatus=''
		//useversioneconomy=1

	} else if (win.datafile == "PRODUCTION_ORDERS") {
		versionfn = 12;
		statusfn = 11;
		firstversion = "A";
		defaultstatus = "ISSUED";
		versionsepchar = "";

	} else if (win.datafile == "BOOKING_ORDERS") {
		statusfn = 39;
		logfn = 40;
		statuslogfn = logfn + 3;

	} else if (win.datafile == "PRODUCTION_INVOICES") {
		versionfn = 12;
		statusfn = 11;
		firstversion = "A";
		defaultstatus = "ISSUED";
		versionsepchar = "";

	} else if (win.datafile == "JOBS") {
		//in general.subs2 and createjob
		versionfn = 29;
		statusfn = 7;
		firstversion = "A";
		defaultstatus = "OPEN";
		versionsepchar = "";

	} else if (win.datafile == "CLIENTS") {
		versionfn = 41;
		statusfn = 35;
		//firstversion='1'
		//defaultstatus=''
		logfn = 50;

	} else if (win.datafile == "SUPPLIERS") {
		versionfn = 16;
		statusfn = 15;
		//firstversion='1'
		//defaultstatus=''

	} else if (win.datafile == "VEHICLES") {
		versionfn = 26;
		statusfn = 24;
		//firstversion='1'
		//defaultstatus=''

	} else if (win.datafile == "RATECARDS") {
		versionfn = 67;
		statusfn = 68;
		//firstversion='1'
		//defaultstatus=''
		logfn = 70;

	} else if (win.datafile == "CURRENCIES") {
		statusfn = 25;
		versionfn = 26;
		//firstversion='1'
		//defaultstatus=''

	} else if (win.datafile == "MARKETS") {
		statusfn = 25;
		versionfn = 26;
		//firstversion='1'
		//defaultstatus=''

	} else if (win.datafile == "COMPANIES") {
		statusfn = 35;
		versionfn = 36;
		//firstversion='1'
		//defaultstatus=''
		logfn = 40;

	} else if (win.datafile == "JOB_TYPES") {
		statusfn = 25;
		versionfn = 26;
		//firstversion='1'
		//defaultstatus=''

	} else if ((win.datafile == "PLANS") or (win.datafile == "SCHEDULES")) {
		statusfn = 157;
		versionfn = 222;
		//firstversion='1'
		//defaultstatus=''
		logfn = 194;
		versionlogfn = 223;
		statuslogfn = 224;
		appending = 0;
		//if invoiced/booked/certified or otherwise touched
		//after prior user update then always update version number
		if (RECORD.a(193) > RECORD.a(195, 1)) {
			useversioneconomy = 0;
		}else{
			useversioneconomy = 1;
		}

	} else if (win.datafile == "VOUCHERS") {
		statusfn = 7;
		versionfn = 59;
		logfn = 60;
		//edits should not be frequent and finance is sensitive so log all changes
		useversioneconomy = 0;

	//PREWRITE done in DAYBOOK.SUBS3 see trace:
	//only POSTREAD AND PREDELETE done here
	} else if (win.datafile == "BATCHES") {
		//indicate no statusfn but statuslogfn still present
		statusfn = -1;
		versionfn = 41;
		logfn = 25;
		//edits should not be frequent and finance is sensitive so log all changes
		appending = 0;
		useversioneconomy = 0;

	} else {
		msg = DQ ^ (win.datafile ^ DQ) ^ " unknown file in GENERAL.SUBS2," ^ mode;
		gosub invalid(msg);
		return;
	}
//L2864:
	userlogfn = logfn + 0;
	datetimelogfn = logfn + 1;
	stationlogfn = logfn + 2;
	if (versionfn) {
		if (not versionlogfn) {
			versionlogfn = logfn + 3;
		}
		currentversion = RECORD.a(versionfn);
	}else{
		currentversion = "";
	}
	if (statusfn) {
		if (not statuslogfn) {
			statuslogfn = logfn + 4;
		}
	}

	if (appending) {
		var tt = RECORD.a(logfn + 0);
		logn = tt.count(VM) + (tt ne "") + 1;
		ologn = logn - 1;
	}else{
		logn = 1;
		ologn = 1;
	}

	return;

}

subroutine addlog() {
	//copied in general.subs2 and similar in productionproxy and upd.voucher

	RECORD.inserter(userlogfn, logn, USERNAME);

	var datetime = var().date() ^ "." ^ var().time().oconv("R(0)#5");
	RECORD.inserter(datetimelogfn, logn, datetime);

	RECORD.inserter(stationlogfn, logn, STATION.trim());

	if (versionfn) {
		RECORD.inserter(versionlogfn, logn, version);
	}

	if (statusfn) {
		RECORD.inserter(statuslogfn, logn, status);
	}

	//on large records with more than 20 log entries, trim the log to 10 entries
	//can look at previous versions to get prior versions
	if ((RECORD.length() > 0) and (RECORD.a(userlogfn).count(VM) >= 20)) {
		if (logn == 1) {
			ii = 1;
		}else{
			ii = logn - 9;
			if (ii < 1) {
				ii = 1;
			}
		}
		RECORD.r(userlogfn, RECORD.a(userlogfn).field(VM, ii, 10));
		RECORD.r(datetimelogfn, RECORD.a(datetimelogfn).field(VM, ii, 10));
		RECORD.r(stationlogfn, RECORD.a(stationlogfn).field(VM, ii, 10));
		if (versionfn) {
			RECORD.r(versionlogfn, RECORD.a(versionlogfn).field(VM, ii, 10));
		}
		if (statusfn) {
			RECORD.r(statuslogfn, RECORD.a(statuslogfn).field(VM, ii, 10));
		}
	}

	return;

}

subroutine incrementversion() {
	if (versionfn) {
		if (version.match("\"V\"0N")) {
			var tt = version.substr(2,999) + 1;
			version = "V" ^ tt;
		} else if (version == "") {
			version = "B";
		} else if (version == "Z") {
			version = "V27";
		} else if (version.isnum()) {
			version += 1;
		} else if (var("ABCDEFGHIJKLMNOPQRSTUVWXY").index(version)) {
			version = var().chr(version.seq() + 1);
		}
//L3416:
		RECORD.r(versionfn, version);
		return;

	}

}


libraryexit()
