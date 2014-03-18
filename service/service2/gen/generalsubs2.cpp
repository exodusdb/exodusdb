#include <exodus/library.h>
libraryinit()

#include <singular.h>
#include <autorun2.h>
#include <authorised.h>

#include <agy.h>
#include <gen.h>
#include <win.h>

#include <window.hpp>

var msg;
var statusfn;//num
var status;
var defaultstatus;
var versionfn;//num
var version;//num
var firstversion;
var useversioneconomy;//num
var datetimelogfn;
var logn;//num
var userlogfn;
var stationlogfn;
var statuslogfn;//num
var oldversion;
var versionsepchar;
var ascending;//num
var versionlogfn;//num
var status2;
var subject;
var module;
var request;
var datax;
var task;
var taskn;
var ii;//num
var wsmsg;

function main(in mode) {
	//jbase
	var interactive = not SYSTEM.a(33);

	//was agency.subs2 but has company and currency in it so moved here
	//so that accounts only module would not work

	//because valid doesnt seem to be set in vehicle.subs postread!
	//maybe the problem extends elsewhere
	win.valid = 1;

	//y2k2
	if (mode == "POSTINIT") {

	} else if (mode == "PREDELETE") {

	} else if (mode.index("POSTREAD", 1)) {

		if (RECORD ne "") {
			return 0;
		}

		var versionfilename = singular(win.datafile) ^ ".VERSIONS";
		var versionfile;
		if (not(versionfile.open(versionfilename, ""))) {
			if (not(ID.index("~", 1))) {
				return 0;
			}
			msg = versionfilename ^ " file is missing";
			return invalid(msg);
		}

		//if not in version file and no ~ character in key then
		//assume is an ordinary new record
		if (not(RECORD.read(versionfile, ID))) {
			if (not(ID.index("~", 1))) {
				return 0;
			}
			msg = DQ ^ (ID ^ DQ) ^ " missing from " ^ versionfilename ^ " file";
			return invalid(msg);
		}

		win.orec = RECORD;

		//need to unlock BEFORE changing ~ to * in @id
		var xx = unlockrecord("", win.srcfile, ID);
		win.wlocked = 0;

		//change ~ to * to try and prevent ~ appearing in multipart key date fields
		//to allow dates to be presented correctly
		if (ID.index("*", 1)) {
			ID.converter("~", "*");
		}

		//allow to pass usual security checks

	/*;
		case index(mode,'ADDLOG',1);

			gosub getfiledefaults;
			if valid else return 0;
			version=;
			status=;
			gosub addlog;
	*/

	} else if (mode.index("PREWRITE", 1)) {

		if (not win.wlocked) {
			return 0;
		}

		//standard code to log version and updates

		gosub getfiledefaults();
		if (not win.valid) {
			return 0;
		}

		//skip out if no changes
		if (RECORD == win.orec) {

			//force write not to skip
			win.orec.r(1000, not RECORD.a(1000));

			//////
			return 0;
			//////

		}

		if (statusfn) {
			status = RECORD.a(statusfn).trim();
			if (win.datafile == "JOBS") {
				//'OPEN' is coded in general.subs2 and job.subs postread, client.subs,postwrite
				if (status == "") {
					status = "OPEN";
				}
				else if (status == "Y") {
					status = "CLOSED";
				}
				else if (status == "N") {
					status = "REOPENED";
				}
			}
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
			}
		}

		if (win.orec) {

			//update the version if anything changed except status
			var anythingchanged = 0;
			if (statusfn) {
				var temporec = win.orec.replace(statusfn, 0, 0, RECORD.a(statusfn));

				//special case amending order no AFTER invoicing
				//if not considered changing anything (so that the version remains)
				if (win.datafile == "PRODUCTION.INVOICES" and (RECORD.a(34)).index("INVOICED", 1)) {
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
				var lastdate = win.orec.a(datetimelogfn, logn).field(".", 1);
				if (lastdate == var().date()) {
					var lastuser = win.orec.a(userlogfn, logn);
					if (lastuser == USERNAME) {
						var laststation = win.orec.a(stationlogfn, logn);
						if (laststation == mv.STATION.trim()) {
							if (statusfn) {
								var laststatus = win.orec.a(statuslogfn, logn);
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
				var versionfile;
				if (versionfile.open(singular(win.datafile) ^ ".VERSIONS", "")) {
					//write orec on versionfile,fieldstore(@id,'*',5,1,version)
					win.orec.write(versionfile, ID ^ versionsepchar ^ version);
				}

				if (versionfn) {
					if (version.match("\"V\"0N")) {
						version = "V" ^ (version.substr(2, 999) + 1);
						
					} else if (version == "") {
						version = "B";
						
					} else if (version == "Z") {
						version = "V27";
						
					} else if (version.isnum()) {
						version += 1;
						
					} else if (var("ABCDEFGHIJKLMNOPQRSTUVWXY").index(version, 1)) {
						version = var().chr(version.seq() + 1);
					}

					RECORD.r(versionfn, version);
				}

			}

		}

		//update log fields

		//could update last log instead of inserting if doing versioneconomy?
		//would lose complete datetime record but avoid confusing same versions

		//backward compatible with old records without versioning
		//save version 1 line even if record doesnt exist
		if (win.orec and ascending and logn == 1 and versionfn) {
			RECORD.inserter(versionlogfn, logn, oldversion);
			logn += 1;
		}

		gosub addlog();

		if (win.datafile == "JOBS") {
			status2 = calculate("STATUS2");
			subject = "Job " ^ ID ^ calculate("VERSION") ^ " " ^ status2 ^ " (" ^ calculate("EXECUTIVE_NAME") ^ ") " ^ calculate("DESCRIPTION1");
			module = "PRODUCTION";
			request = "JOBPRINT";
			datax = ID;
			task = "JOB";

			
		} else if (win.datafile == "PRODUCTION.ORDERS") {
			status2 = calculate("STATUS2");
			subject = "Cost " ^ ID ^ calculate("VERSION") ^ " " ^ status2 ^ " (" ^ calculate("EXECUTIVE_NAME") ^ ") " ^ calculate("DESCRIPTION");
			module = "PRODUCTION";
			request = "COSTESTIMATEPRINT";
			datax = "COST" ^ FM ^ ID;
			if (status2.ucase() == "REOPENED") {
				status2 = "OPEN";
			}
			task = "PURCHASE ORDER";

			
		} else if (win.datafile == "PRODUCTION.INVOICES") {
			status2 = calculate("STATUS2");
			subject = "Estimate " ^ ID ^ calculate("VERSION") ^ " " ^ status2 ^ " (" ^ calculate("EXECUTIVE_NAME") ^ ") " ^ calculate("DESCRIPTION");

			status = status2.ucase();
			if (status.index("INVOICE", 1) or status.index("PROFORMA", 1)) {

				//reprint invoice or proforma (risk of confusion due to no option autodetect)
				module = "AGENCY";
				request = "REPRINTINVS";

				if (status.index("PROFORMA", 1)) {
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

		//create a onetime autorun task to email the required people
		if (module and request) {

			task ^= " MONITOR " ^ status2.ucase();

			var monitorusers = "";

			//find key otherwise no monitors
			if (not(gen._security.a(10).locateusing(task, VM, taskn))) {
				return 0;
			}
			var locks = gen._security.a(11, taskn);
			if (not locks) {
				return 0;
			}

			//find all users with the key (no hierarchy!)
			var keys = gen._security.a(2);
			var nusers = keys.count(VM) + 1;
			for (var usern = 1; usern <= nusers; ++usern) {
				var usercode = gen._security.a(1, usern);
				//locate key in keys<1,user> using ',' setting xx then
				if (usercode ne "---" and var(2).rnd()) {
					monitorusers ^= SM ^ usercode;
				}else{
					//break between groups so we can run with the lowest user per group
					if (usercode == "" or usercode == "---") {
						monitorusers ^= VM;
					}
				}
			};//usern;

			//run for each group with the last (bottom) user as the runas user
			//in order to ensure lowest common denominator authorisations
			var ngroups = monitorusers.count(VM) + 1;
			for (var groupn = 1; groupn <= ngroups; ++groupn) {
				var groupusers = trim(monitorusers.a(1, groupn), SM);
				if (groupusers) {
					groupusers.converter(SM, VM);
					var document;
					var docid;
					call autorun2("ASAP", subject, module, request, datax, groupusers.field(VM, -1), groupusers, document, docid, msg);
					//ignore any failures to create for now in order not to bother updaters
				}
			};//groupn;

		}

	}

	return 0;

}

subroutine getfiledefaults() {
	versionfn = "";
	statusfn = "";
	versionsepchar = "~";
	var logfn = 30;
	versionlogfn = "";
	statuslogfn = "";
	ascending = 1;
	//option to save only one version per user per day per status
	useversioneconomy = 1;

	if (win.datafile == "TIMESHEETS") {
		versionfn = 7;
		statusfn = 8;
		firstversion = "1";
		defaultstatus = "";

	} else if (win.datafile == "PRODUCTION.ORDERS") {
		versionfn = 12;
		statusfn = 11;
		firstversion = "A";
		defaultstatus = "ISSUED";
		versionsepchar = "";

	} else if (win.datafile == "BOOKING.ORDERS") {
		statusfn = 39;
		logfn = 40;
		statuslogfn = logfn + 3;

	} else if (win.datafile == "PRODUCTION.INVOICES") {
		versionfn = 12;
		statusfn = 11;
		firstversion = "A";
		defaultstatus = "ISSUED";
		versionsepchar = "";

	} else if (win.datafile == "JOBS") {
		//in general.subs2 and client.subs,postwrite
		versionfn = 29;
		statusfn = 7;
		firstversion = "A";
		defaultstatus = "OPEN";
		versionsepchar = "";

	} else if (win.datafile == "CLIENTS") {
		versionfn = 41;
		statusfn = 35;
		firstversion = "1";
		defaultstatus = "";
		logfn = 50;

	} else if (win.datafile == "SUPPLIERS") {
		versionfn = 16;
		statusfn = 15;
		firstversion = "1";
		defaultstatus = "";

	} else if (win.datafile == "VEHICLES") {
		versionfn = 26;
		statusfn = 24;
		firstversion = "1";
		defaultstatus = "";

	} else if (win.datafile == "RATECARDS") {
		versionfn = 67;
		statusfn = 68;
		firstversion = "1";
		defaultstatus = "";
		logfn = 70;

	} else if (win.datafile == "CURRENCIES") {
		statusfn = 25;
		versionfn = 26;
		firstversion = "1";
		defaultstatus = "";

	} else if (win.datafile == "MARKETS") {
		statusfn = 25;
		versionfn = 26;
		firstversion = "1";
		defaultstatus = "";

	} else if (win.datafile == "COMPANIES") {
		statusfn = 35;
		versionfn = 36;
		firstversion = "1";
		defaultstatus = "";
		logfn = 40;

	} else if (win.datafile == "MEDIA.TYPES") {
		statusfn = 25;
		versionfn = 26;
		firstversion = "1";
		defaultstatus = "";

	} else if (win.datafile == "PLANS" or win.datafile == "SCHEDULES") {
		statusfn = 157;
		versionfn = 222;
		firstversion = "1";
		defaultstatus = "";
		logfn = 194;
		versionlogfn = 223;
		statuslogfn = 224;
		ascending = 0;
		//if invoiced/booked/certified or otherwise touched
		//after prior user update then always update version number
		if (RECORD.a(193) > RECORD.a(195, 1)) {
			useversioneconomy = 0;
		}else{
			useversioneconomy = 1;
		}

	} else {
		msg = DQ ^ (win.datafile ^ DQ) ^ " unknown file in AGENCY.SUBS2";
		invalid(msg);
		return;
	}

	userlogfn = logfn + 0;
	datetimelogfn = logfn + 1;
	stationlogfn = logfn + 2;
	if (versionfn) {
		if (not versionlogfn) {
			versionlogfn = logfn + 3;
		}
	}
	if (statusfn) {
		if (not statuslogfn) {
			statuslogfn = logfn + 4;
		}
	}

	if (ascending) {
		logn = RECORD.a(logfn + 0);
		logn = logn.count(VM) + (logn ne "") + 1;
	}else{
		logn = 1;
	}

	return;

}

subroutine addlog() {
	//copied in general.subs2 and productionproxy

	RECORD.inserter(userlogfn, logn, USERNAME);

	var datetime = var().date() ^ "." ^ (var().time()).oconv("R(0)#5");
	RECORD.inserter(datetimelogfn, logn, datetime);

	RECORD.inserter(stationlogfn, logn, mv.STATION.trim());

	if (versionfn) {
		RECORD.inserter(versionlogfn, logn, version);
	}

	if (statusfn) {
		RECORD.inserter(statuslogfn, logn, status);
	}

	//on large records with more than 20 log entries, trim the log to 10 entries
	//can look at previous versions to get prior versions
	if (RECORD.length() > 0 and (RECORD.a(userlogfn)).count(VM) >= 20) {
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


libraryexit()
