#include <exodus/library.h>
libraryinit()

#include <cid.h>
#include <secid.h>

#include <gen.h>
#include <win.h>

var xx;
var lictext;
var licn;
var datedict;
var chkmodulename;
var docdate;
var dateorperiod;//num

function main() {

	//users can always create documents for periods they have lic for
	//IF they hold a lic that has not expired (with some grace days allowed)
	//they can also create documents for other periods

		/* example ADDLIC command (but wrong verificationcode);
		ADDLIC MEDIA,FINANCE 1/10/2009 31/12/2009 7 542684 1D63A3AC 999999;
		*/

	var maxdaysgrace = 30;
	var maxlicencedays = 366;
	var secidcode = 1090909;
	var allowneosys = 1;

	var lickey = "SYSLICX";

	//LICX in definitions
	//all parallel mv
	//1 uptodate (must be sorted in ascending order)
	//2 fromdate
	//3 modulenames (or * for all)
	//4 datasetid
	//5 daysgrace
	//6 licencetext
	//7 verificationcode

	//mode can be ADD - uses @SENTENCE for parameters
	//otherwise lic is checked using datafile and {YEAR_PERIOD}

	var licxpresent = 1;
	var licx;
	if (not(licx.read(gen._definitions, lickey))) {
		licxpresent = 0;
		licx = "";
	}

	var datasetid;
	if (not(datasetid.read(gen._definitions, "GLOBALDATASETID"))) {
		msg = "GLOBALDATASETID is missing from DEFINITIONS";
		return 0;
	}
	datasetid = datasetid.a(1);

	if (SENTENCE.field(" ", 1) == "CHKLIC") {
		mode = SENTENCE.field(" ", 2);
	}

	if (mode == "") {

		var cmd = "LIST DEFINITIONS " ^ (DQ ^ (lickey ^ DQ)) ^ " LIC_FIELDS";
		cmd ^= " HEADING " ^ (DQ ^ ("NEOSYS LICENCES as at \'T\'" ^ DQ));
		perform(cmd);
		var().stop();

		goto 326;
	}
	if (mode == "TEST") {
		//clear common
		SENTENCE = "";
		win.datafile = "SCHEDULES";
		if (not(DICT.open("DICT", "SCHEDULES"))) {
			call fsmsg();
			var().stop();
		}
		RECORD.r(12, "1/09");
		call chklic("PREWRITE", msg);
		if (msg) {
			call mssg();
		}
		var().stop();
	}
L326:

	//this option is not documented at the moment
	//since no licence record = no licence restrictions
	if (mode == "DELETE") {
		gen._definitions.deleterecord(lickey);
		msg = "";
		return 0;

		goto 1274;
	}
	if (mode == "ADD") {

		var SENTENCE = SENTENCE.field("(", 1);
		var options = SENTENCE.field("(", 2);

		var modulenames = SENTENCE.field(" ", 2);
		var fromdate = SENTENCE.field(" ", 3);
		var uptodate = SENTENCE.field(" ", 4);
		var daysgrace = SENTENCE.field(" ", 5);

		var computerid = SENTENCE.field(" ", 6);
		var cmddatasetid = SENTENCE.field(" ", 7);
		var verification = SENTENCE.field(" ", 8);

		if (modulenames == "" or fromdate == "" or uptodate == "" or daysgrace == "") {
			msg = "";
syntax:
			msg.r(-1, "Syntax is:" ^ FM ^ FM ^ "ADDLIC modulename fromdate uptodate daygrace");
			msg.r(-1, FM ^ "or " ^ FM ^ FM ^ "ADDLIC modulename fromdate uptodate daygrace compid databaseid verification");
			msg.r(-1, FM);
			return 0;
		}

		var allmodulenames = "MEDIA";
		allmodulenames.r(1, -1, "JOBS");
		allmodulenames.r(1, -1, "FINANCE");
		allmodulenames.r(1, -1, "TIMESHEETS");
		allmodulenames.r(1, -1, "*");

		//check module names
		for (var modulen = 1; modulen <= 9999; ++modulen) {
			var modulename = modulenames.field(",", modulen);
		///BREAK;
		if (not modulename) break;;
			if (not(allmodulenames.locateusing(modulename, VM, xx))) {
				msg = "modulename must be one of the following";
				msg.r(-1, allmodulenames);
				return 0;
			}
		};//modulen;

		//check fromdate
		var ifromdate = fromdate.iconv("[DATE,4*]");
		if (not ifromdate.match("5N")) {
			msg = DQ ^ (fromdate ^ DQ) ^ " cannot be recognised as a date";
			goto syntax;
		}

		//check upto date
		var iuptodate = uptodate.iconv("[DATE,4*]");
		if (not iuptodate.match("5N")) {
			msg = DQ ^ (uptodate ^ DQ) ^ " cannot be recognised as a date";
			goto syntax;
		}

		//check fromdate<uptodate
		if (ifromdate >= iuptodate) {
			msg = "fromdate must be before uptodate";
			goto syntax;
		}

		//check daysgrace numeric
		if (not daysgrace.match("1N0N")) {
			msg = DQ ^ (daysgrace ^ DQ) ^ " days grace must be numeric";
			goto syntax;
		}

		//limit licence period
		if (iuptodate - ifromdate > maxlicencedays) {
			msg = "the maximum licence period is " ^ maxlicencedays ^ " days";
			return 0;
		}

		//limit future licence
		if (iuptodate >= var().date() + maxlicencedays + maxdaysgrace) {
			msg = "the maximum licence date is " ^ maxlicencedays ^ "+" ^ maxdaysgrace ^ " days in the future";
			return 0;
		}

		//limit days grace
		if (daysgrace > maxdaysgrace) {
			msg = "the number of days grace is limited to " ^ maxdaysgrace;
			return 0;
		}

		//if incomplete command then prompt for verificationcode
		if (computerid == "" or cmddatasetid == "" or verification == "") {
			if (computerid == "") {
				computerid = cid();
			}
			if (cmddatasetid == "") {
				cmddatasetid = datasetid;
			}
			lictext = modulenames;
			lictext ^= " " ^ fromdate;
			lictext ^= " " ^ uptodate;
			lictext ^= " " ^ daysgrace;
			lictext ^= " " ^ computerid;
			lictext ^= " " ^ cmddatasetid;
			call note2("PLEASE GIVE THE FOLLOWING TEXT TO YOUR NEOSYS SUPPORT STAFF" ^ FM ^ FM ^ (DQ ^ (lictext ^ DQ)) ^ FM ^ FM ^ "What is the verification code?", "R", verification, "");
			if (not verification) {
				msg = "Verification code not entered";
				return 0;
			}
		}else{
			lictext = SENTENCE.field(" ", 2, 6);
		}

	//example
	//ADDLIC MEDIA,FINANCE 1/10/2009 31/12/2009 7 542684 1D63A3AC 915036

		var reqverification = secid(lictext, secidcode);
		if (verification ne reqverification) {
			msg = DQ ^ (verification ^ DQ) ^ " is not the correct verification code";
			//if dir('NEOSYS.ID') then
			// msg<-1>=reqverification
			// end
			return 0;
		}

		if (options.index("O", 1)) {
			licx = "";
		}

		for (var modulen = 1; modulen <= 9999; ++modulen) {

			var modulename = modulenames.field(",", modulen);
		///BREAK;
		if (not modulename) break;;

			//must be in numerical order by licence "upto" date
			if (not(licx.a(1).locateusing(iuptodate, VM, licn))) {
				{}
			}

			licx.inserter(1, licn, iuptodate);
			licx.inserter(2, licn, ifromdate);
			licx.inserter(3, licn, modulename);
			licx.inserter(4, licn, cmddatasetid);
			licx.inserter(5, licn, daysgrace);
			licx.inserter(6, licn, lictext);
			licx.inserter(7, licn, reqverification);

			licx.write(gen._definitions, lickey);

		};//modulen;

		call note("Licence added");
		msg = "";
		return 0;

	}
L1274:

	/////////////////////////////////////////
	//from here on we are checking if licenced
	/////////////////////////////////////////

	//ok if not creating a new record
	if (win.orec ne "") {
		goto ok;
	}

	//ok if no lic info (for now)
	if (not licxpresent) {
		goto ok;
	}

	//ok if the neosys user
	if (allowneosys and USERNAME == "NEOSYS") {
		goto ok;
	}

	//no lic check on neosys development machines
	if (allowneosys and var("NEOSYS.ID").osfile()) {
		goto ok;
	}

	msg = "NEOSYS Lic" "ence: ";

	if (win.datafile == "SCHEDULES") {
		datedict = "STOPDATE";
		chkmodulename = "MEDIA";

		goto 1548;
	}
	if (win.datafile == "PLANS") {
		datedict = "PERIOD_TO";
		chkmodulename = "MEDIA";

		goto 1548;
	}
	if (win.datafile == "JOBS") {
		datedict = "YEAR_PERIOD";
		chkmodulename = "JOBS";

		goto 1548;
	}
	if (win.datafile == "PRODUCTION.INVOICES" or win.datafile == "PRODUCTION.ORDERS") {
		datedict = "DATE";
		chkmodulename = "JOBS";

		goto 1548;
	}
	if (win.datafile == "BATCHES") {
		datedict = "YEAR_PERIOD";
		chkmodulename = "FINANCE";

		goto 1548;
	}
	if (win.datafile == "TIMESHEETS") {
		datedict = "DATE";
		chkmodulename = "TIMESHEETS";

	}else{
		msg = DQ ^ (win.datafile ^ DQ) ^ " datafile is invalid in chklic";
		return 0;
	}
L1548:

	//determine document date or use last date of documents period
	var tt = var(datedict).calculate();
	if (tt.match("5N")) {
		docdate = tt;
		dateorperiod = 1;
		goto 1759;
	}
	if (tt.index(".", 1)) {
		docdate = (tt ^ ".31").iconv("DJ");
		if (not docdate) {
			docdate = (tt ^ ".30").iconv("DJ");
		}
		if (not docdate) {
			docdate = (tt ^ ".28").iconv("DJ");
		}
		dateorperiod = 2;
		goto 1759;
	}
	if (tt.index("/", 1)) {
		docdate = var("31/" ^ tt).iconv("D/E");
		if (not docdate) {
			docdate = var("30/" ^ tt).iconv("D/E");
		}
		if (not docdate) {
			docdate = var("28/" ^ tt).iconv("D/E");
		}
		dateorperiod = 2;
	}else{
		msg = DQ ^ (tt ^ DQ) ^ " unrecognisable period or date in " ^ win.datafile ^ " " ^ ID;
		return 0;
	}
L1759:

	//ok if document 1/period/year is in a specifically authorised lic period
	//skip expired licences
	if (not(licx.a(1).locatebyusing(docdate, "AR", licn, VM))) {
		{}
	}
	//check remaining licences which havent expired
	for (var licn = licn; licn <= 9999; ++licn) {
		var uptodate = licx.a(1, licn);
	///BREAK;
	if (not uptodate) break;;
		var did = licx.a(4, licn);
		if (did == datasetid and docdate >= licx.a(2, licn)) {
			var modulename = licx.a(3, licn);
			if (modulename == "*") {
ok:
				msg = "";
				return 0;
			}
			if (modulename == chkmodulename) {
				goto ok;
			}
		}
	};//licn;

	//ok if current date is not after last expiry date plus grace days
	var nlics = (licx.a(1)).count(VM) + (licx ne "");
	//find last expiry date for current module
	for (var licn = nlics; licn >= 1; --licn) {
		var modulename = licx.a(3, licn);
		var did = licx.a(4, licn);
		if (did == datasetid and (modulename == "*" or modulename == chkmodulename)) {

			var expirydate = licx.a(1, licn);
			var daysgrace = licx.a(5, licn);
			var finalexpirydate = expirydate + daysgrace;

			if (var().date() > finalexpirydate) {
				var commencedate = licx.a(2, licn);
				if (dateorperiod == 1) {
					tt = "date";
				}else{
					tt = "period closing";
				}
				msg = "The document " ^ tt ^ " " ^ docdate.oconv("[DATE,4*]") ^ " is not within";
				msg.r(-1, "the NEOSYS licence period ");
				msg ^= commencedate.oconv("[DATE,4*]") ^ " - ";
				msg ^= expirydate.oconv("[DATE,4*]");
				msg.r(-1, " and it is now " ^ (var().date()).oconv("[DATE,4*]") ^ " - " ^ var().date() - expirydate ^ " days after the licence expiry date");
				//if daysgrace then
				// msg:=' plus ':daysgrace:' days grace'
				// end
				return 0;

			}else{
				goto ok;
			}

		}
	};//licn;

	//cannot find any lic (either * or specific)
	msg = "There is no licence to use the NEOSYS " ^ chkmodulename ^ " module";
	return 0;

}


libraryexit()