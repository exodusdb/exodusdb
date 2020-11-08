#include <exodus/library.h>
libraryinit()

function main(io filename, in mode, out filetitle, out triggers) {
	//c gen io,in,out,out

	//purpose:
	//1. given a filename and mode (READ/READO/READU/WRITE/WRITEU/DELETE)
	//return the filetitle or '' if filename not valid or not allowed
	//and a list of pre and post routines
	//2. also called for SELECT to fix up the filename (_->. etc)

	filetitle = "";
	triggers = "";

	//determine pre and post delete routines
	var preread = "";
	var prereadmode = "PREREAD";
	var postread = "";
	var postreadmode = "POSTREAD";
	var updatesubs = "";
	var replaceread = "";
	var replacewrite = "";
	var replacedelete = "";

	//fix filenames from AREV to EXODUS style
	if (not(VOLUMES)) {
		filename.swapper("MEDIA" ".TYPE", "JOB_TYPE");
		filename.converter(".", "_");
		}

	if ((filename == "SCHEDULES") or (filename == "PLANS")) {
		//preread='PLAN.SUBS5'
		preread = "AGENCY.SUBS";
		postread = "PLAN.SUBS";
		//force splitextras and company code plus lots of others
		postreadmode = "POSTREAD2";
		updatesubs = "PLAN.SUBS";

	} else if (filename == "TIMESHEETS") {
		preread = "TIMESHEET.SUBS";
		postread = "TIMESHEET.SUBS";
		//get previous timesheet,get job descriptions/brandnames
		postreadmode = "POSTREAD2";
		updatesubs = "TIMESHEET.SUBS";

	} else if (filename == "JOBS") {
		preread = "AGENCY.SUBS";
		postread = "JOB.SUBS";
		//hide costs and/or income, add company code maybe, job analysis 22/23/24,
		//cost and income records, brief vm to tm
		postreadmode = "POSTREAD2";
		updatesubs = "JOB.SUBS";

	} else if (filename == "RATECARDS") {
		postread = "RATECARD.SUBS";
		updatesubs = "RATECARD.SUBS";

	} else if (filename == "JOB_TYPES") {
		postread = "JOBTYPE.SUBS";
		//production ''/Y become M/N
		updatesubs = "JOBTYPE.SUBS";

	//this file is never read or written directly, only via MEDIAPROXY
	//case filename='BOOKING_ORDERS'

	} else if (filename == "SUPPLIERS") {
		postread = "SUPPLIER.SUBS";
		//get currency from account if poss
		updatesubs = "SUPPLIER.SUBS";

	} else if (filename == "VEHICLES") {
		postread = "VEHICLE.SUBS";
		//convert dows to sm list
		updatesubs = "VEHICLE.SUBS";

	} else if (filename == "MATERIALS") {
		postread = "MATERIAL.SUBS";
		updatesubs = "MATERIAL.SUBS";

	} else if (filename == "PRODUCT_CATEGORIES") {
		postread = "";
		updatesubs = "";

	} else if (filename == "CLIENTS") {
		postread = "CLIENT.SUBS";
		updatesubs = "CLIENT.SUBS";

	} else if (filename == "MARKETS") {
		postread = "MARKET.SUBS";
		updatesubs = "MARKET.SUBS";

	} else if (filename == "CURRENCIES") {
		postread = "CURRENCY.SUBS";
		//adds conversions
		updatesubs = "CURRENCY.SUBS";

	} else if (filename == "PRODUCTION_ORDERS") {
		preread = "AGENCY.SUBS";
		postread = "PRODORDER.SUBS";
		//get ndecs
		postreadmode = "POSTREAD2";
		updatesubs = "PRODORDER.SUBS";

	} else if (filename == "PRODUCTION_INVOICES") {
		preread = "AGENCY.SUBS";
		postread = "PRODINV.SUBS";
		//get saved prior estimates, add currency ndecs
		postreadmode = "POSTREAD2";
		updatesubs = "PRODINV.SUBS";
		filetitle = "PRODUCTION ESTIMATES";

	} else if (filename == "BATCHES") {
		postread = "DAYBOOK.SUBS4";
		updatesubs = "DAYBOOK.SUBS4";
		filetitle = "JOURNALS";

	} else if (filename == "VOUCHERS") {
		postread = "VOUCHERX.SUBS";
		//VOUCHER.SUBS would be *date from hex, period like 1/99, switch sign if - or CR, full accountcodes x:sm:x
		updatesubs = "VOUCHERX.SUBS";
		replacewrite = "WRITE";

	} else if (filename == "RECURRING") {
		postread = "RECURRING.SUBS";
		//clear the postmode field
		updatesubs = "RECURRING.SUBS";

	} else if (filename == "DEFINITIONS") {
		preread = "DEFINITION.SUBS";
		postread = "DEFINITION.SUBS";
		//security remove passwords,sort tasks,remove unauth tasks and higher/lower users/groups
		//chequedesign get default
		updatesubs = "DEFINITION.SUBS";

	} else if (filename == "USERS") {
		postread = "USER.SUBS";
		updatesubs = "USER.SUBS";

	} else if ((filename == "CHARTS") or (filename == "LEDGERS")) {
		postread = "CHART.SUBS";
		//fix transfer accounts, split off report design, change open item '' to '0'
		updatesubs = "CHART.SUBS";
		filetitle = "LEDGERS";

	} else if (filename == "ADDRESSES") {
		postread = "ADDRESS.SUBS";
		updatesubs = "ADDRESS.SUBS";

	} else if (filename == "COMPANIES") {
		postread = "COMPANY.SUBS";
		updatesubs = "COMPANY.SUBS";

	} else if (filename == "DOCUMENTS") {
		postread = "GET.SUBS";
		//move instructions to fn 101 plus, default time/date to DATE_TIME
		updatesubs = "GET.SUBS";

	} else if (filename == "PROGRAMS") {
		postread = "PROGRAM.SUBS";
		//get ads for each program
		updatesubs = "PROGRAM.SUBS";

	} else if (filename == "CHANGELOG") {
		postread = "CHANGELOG.SUBS";
		updatesubs = "CHANGELOG.SUBS";

	} else if (filename == "SUPPLIER_INVOICES") {
		postread = "SUPPINV.SUBS";
		updatesubs = "SUPPINV.SUBS";

	} else if (filename == "ACCOUNTS") {
		postread = "ACCOUNT.SUBS";

	} else if (filename == "COLLECTIONS") {
		postread = "COLLECTION.SUBS";
		updatesubs = "COLLECTION.SUBS";

	} else {
		//unknown files return with filetitle=''
		//which probably causes the calling program to reject the request
		return 0;
	}

	//////
	//exit:
	//////

	if (mode.index("READ")) {
		//1/2
		if (preread) {
			triggers.r(1, preread ^ FM ^ prereadmode);
		}
		//3/4
		if (postread) {
			triggers.r(3, postread ^ FM ^ postreadmode);
		}
		//5/6
		if (replaceread) {
			triggers.r(5, replaceread ^ FM ^ "READ");
		}
	} else if (mode.index("WRITE")) {
		if (updatesubs) {
			triggers.r(1, updatesubs ^ FM ^ "PREWRITE");
			triggers.r(3, updatesubs ^ FM ^ "POSTWRITE");
			if (replacewrite) {
				triggers.r(5, updatesubs ^ FM ^ "WRITE");
			}
		}
	} else if (mode.index("DELETE")) {
		if (updatesubs) {
			triggers.r(1, updatesubs ^ FM ^ "PREDELETE");
			triggers.r(3, updatesubs ^ FM ^ "POSTDELETE");
			if (replacedelete) {
				triggers.r(5, updatesubs ^ FM ^ "DELETE");
			}
		}
	} else if (mode == "LOCK") {
	} else if (mode == "RELOCK") {
	} else if (mode == "UNLOCK") {
	} else if (mode == "SELECT") {
	} else if (mode == "GETINDEXVALUES") {
	} else {
		call mssg(mode.quote() ^ " is invalid in LISTEN3");
		return 0;
	}

	//allow all files for the time being
	if (filetitle == "") {
		filetitle = filename;
	}

	//c++ variation
	if (not(VOLUMES)) {
		for (var ii = 1; ii <= 5; ii+=2) {
			var tt = triggers.a(ii);
			tt.lcaser();
			tt.converter(".", "");
			triggers.r(ii, tt);
		};//ii;
	}

	return 0;
}

libraryexit()
