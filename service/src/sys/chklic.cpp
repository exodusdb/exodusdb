#include <exodus/library.h>
libraryinit()

#include <cid.h>
#include <secid.h>
#include <sendmail.h>
#include <sysmsg.h>

#include <gen_common.h>
#include <win_common.h>

var mode;
var lictext;
var subject;
var body;
var uptodate;
var modulename;
var xx;
var licn;//num
var datedict;
var chkmodulename;
var tt;
var docdate;
var dateorperiod;//num
var did;
var maxexpirydate;//num
var errormsg;

function main(in mode0, out msg) {
	//c sys in,out
	#include <general_common.h>

	//global mode,modulename,did,uptodate,licn,tt,msg

	//users can always create documents for periods they have lic for
	//IF they hold a lic that has not expired (with some grace days allowed)
	//they can also create documents for other periods

	//mode=mode0
	if (mode0.unassigned()) {
		mode = "";
		} else {
		mode = mode0;
	}

		/* REQUESTING AND ENTERING A VERIFICATION CODE with 7 days grace;
		ADDLIC MEDIA,FINANCE 1/10/2009 31/12/2009 7;
		*/

		/* or entering a verification code 999999 (get other 2 numbers from request);
		ADDLIC MEDIA,FINANCE 1/10/2009 31/12/2009 7 542684 1D63A3AC 999999;
		*/

	//CHKLIC by itself lists the licences installed if any

	//CHKLIC TEST tests an ancient document date

	//DELLIC by itself requests a verification code
	//to remove licencing restrictions (and all licences)

	//DELLIC 999999 (where 999999 is the verification code)
	//removes licencing restrictions (and all licences)

	var maxdaysgrace = 30;
	var maxlicencedays = 366;
	var secidcode = 1090909;
	var allowexodus = 0;

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
	if (not(licx.read(DEFINITIONS, lickey))) {
		licxpresent = 0;
		licx = "";
	}

	var datasetid;
	if (not(datasetid.read(DEFINITIONS, "GLOBALDATASETID"))) {
		msg = "GLOBALDATASETID is missing from DEFINITIONS";
		return 0;
	}
	datasetid = datasetid.a(1);

	if (SENTENCE.field(" ", 1) eq "CHKLIC") {
		mode = SENTENCE.field(" ", 2);
	}

	if (mode eq "") {

		var cmd = "LIST DEFINITIONS " ^ (lickey.quote()) ^ " LIC_FIELDS";
		cmd ^= " HEADING " ^ (var("EXODUS LIC':'ENCES as at 'T'").quote());
		perform(cmd);
		stop();
	}

	if (mode eq "TEST") {
		perform("CHKLIC2");
		stop();
	}

	if (mode eq "DELETE") {

		if (not licxpresent) {
			call mssg("NO LICENCES ARE INSTALLED - CANNOT DELETE");
			stop();
		}

		var sentencex = SENTENCE.field("(", 1);
		var options = SENTENCE.field("(", 2);
		var computerid = cid();
		lictext = "DELLIC " ^ computerid ^ " " ^ datasetid;

		var verification = sentencex.field(" ", 2);

		//if incomplete command then prompt for verificationcode
		if (verification eq "") {
			call note("PLEASE GIVE THE FOLLOWING TEXT TO YOUR EXODUS SUPPORT STAFF" ^ FM ^ FM ^ (lictext.quote()) ^ FM ^ FM ^ "What is the verification code?", "R", verification, "");
			if (not verification) {
				msg = "Verification code not entered";
				return 0;
			}
		}

	//example
	//DELLIC 915036

		var reqverification = secid(lictext, secidcode);
		if (verification ne reqverification) {
			call ossleep(1000*3);
			msg = verification.quote() ^ " is not the correct verification code";
			return 0;
		}

		if (licxpresent) {
			licx.write(DEFINITIONS, lickey ^ ".PRIOR");
		}

		DEFINITIONS.deleterecord(lickey);

		msg = "";

		subject = "EXODUS: Licencing restrictions removed " ^ SYSTEM.a(17) ^ " - " ^ SYSTEM.a(23);
		body = lictext;
		body.r(-1, "Verification: " ^ verification);
		gosub emailadmin(msg);

		return 0;

	} else if (mode eq "ADD") {

		var sentencex = SENTENCE.field("(", 1);
		var options = SENTENCE.field("(", 2);

		var modulenames = sentencex.field(" ", 2);
		var fromdate = sentencex.field(" ", 3);
		uptodate = sentencex.field(" ", 4);
		var daysgrace = sentencex.field(" ", 5);

		var computerid = sentencex.field(" ", 6);
		var cmddatasetid = sentencex.field(" ", 7);
		var verification = sentencex.field(" ", 8);

		if (cmddatasetid and cmddatasetid ne datasetid) {
			msg = sentencex ^ "||" ^ (cmddatasetid.quote()) ^ " is not the id of the current database (" ^ datasetid ^ ")";
			return 0;
		}

		if (((modulenames eq "" or fromdate eq "") or uptodate eq "") or daysgrace eq "") {
			msg = "";
			gosub syntax(msg);
			return 0;
		}

		var allmodulenames = "MEDIA";
		allmodulenames.r(1, -1, "JOBS");
		allmodulenames.r(1, -1, "FINANCE");
		allmodulenames.r(1, -1, "TIMESHEETS");
		allmodulenames.r(1, -1, "*");

		//check module names
		for (var modulen = 1; modulen <= 9999; ++modulen) {
			modulename = modulenames.field(",", modulen);
			///BREAK;
			if (not modulename) break;
			if (not(allmodulenames.locate(modulename, xx))) {
				msg = "modulename must be one of the following";
				msg.r(-1, allmodulenames);
				return 0;
			}
		} //modulen;

		//check fromdate
		var ifromdate = iconv(fromdate, "[DATE,4*]");
		if (not(ifromdate.match("^\\d{5}$"))) {
			msg = fromdate.quote() ^ " cannot be recognised as a date";
			gosub syntax(msg);
			return 0;
		}

		//check upto date
		var iuptodate = iconv(uptodate, "[DATE,4*]");
		if (not(iuptodate.match("^\\d{5}$"))) {
			msg = uptodate.quote() ^ " cannot be recognised as a date";
			gosub syntax(msg);
			return 0;
		}

		//check fromdate<uptodate
		if (ifromdate ge iuptodate) {
			msg = "fromdate must be before uptodate";
			gosub syntax(msg);
			return 0;
		}

		//check daysgrace numeric
		if (not(daysgrace.match("^\\d+$"))) {
			msg = daysgrace.quote() ^ " days grace must be numeric";
			gosub syntax(msg);
			return 0;
		}

		//limit days grace
		if (daysgrace gt maxdaysgrace) {
			msg = "the number of days grace is limited to " ^ maxdaysgrace;
			return 0;
		}

		//limit the size of future licences, but not historical
		var nfuturedays = iuptodate - var().date();
		if (ifromdate gt var().date()) {
			nfuturedays -= ifromdate - var().date();
		}
		if (nfuturedays gt maxlicencedays) {
			msg = "the maximum licence period in the future is " ^ maxlicencedays ^ " days";
			return 0;
		}

		//if incomplete command then prompt for verificationcode
		if ((computerid eq "" or cmddatasetid eq "") or verification eq "") {
			if (computerid eq "") {
				computerid = cid();
			}
			if (cmddatasetid eq "") {
				cmddatasetid = datasetid;
			}
			lictext = modulenames;
			lictext ^= " " ^ fromdate;
			lictext ^= " " ^ uptodate;
			lictext ^= " " ^ daysgrace;
			lictext ^= " " ^ computerid;
			lictext ^= " " ^ cmddatasetid;
			call note("PLEASE GIVE THE FOLLOWING TEXT TO YOUR EXODUS SUPPORT STAFF" ^ FM ^ FM ^ (lictext.quote()) ^ FM ^ FM ^ "What is the verification code?", "R", verification, "");
			if (not verification) {
				msg = "Verification code not entered";
				return 0;
			}
		} else {
			lictext = sentencex.field(" ", 2, 6);
		}

	//example
	//ADDLIC MEDIA,FINANCE 1/10/2009 31/12/2009 7 542684 1D63A3AC 915036

		var reqverification = secid(lictext, secidcode);
		if (verification ne reqverification) {
			call ossleep(1000*3);
			msg = verification.quote() ^ " is not the correct verification code";
			return 0;
		}

		if (options.index("O")) {
			licx = "";
		}

		//skip for now since can be duplicated by chance (secid/hash function)
		//following licences produce the same authorisation requirement/code
		// 1/4/2015 30/6/2015 7
		// 1/4/2016 30/6/2016 7
		//locate reqverification in licx<7> setting licn then
		// msg=quote(lictext):' already added'
		// return
		// end

		for (var modulen = 1; modulen <= 9999; ++modulen) {

			modulename = modulenames.field(",", modulen);
			///BREAK;
			if (not modulename) break;

			//ideally in numerical order by licence "upto" date
			//checking code does not need it though
			if (not(licx.a(1).locateby("AR", iuptodate, licn))) {
				{}
			}

			licx.inserter(1, licn, iuptodate);
			licx.inserter(2, licn, ifromdate);
			licx.inserter(3, licn, modulename);
			licx.inserter(4, licn, cmddatasetid);
			licx.inserter(5, licn, daysgrace);
			licx.inserter(6, licn, lictext);
			licx.inserter(7, licn, reqverification);

			licx.write(DEFINITIONS, lickey);

		} //modulen;

		subject = "EXODUS: Licence Added " ^ SYSTEM.a(17) ^ " - " ^ SYSTEM.a(23);
		body = "Licence:     : " ^ SENTENCE.field(" ", 2, 999);
		body.r(-1, "Verification : " ^ verification);
		var nlics = licx.a(1).count(VM) + 1;
		body ^= FM ^ FM;
		body ^= "From          Upto          Module          Days Grace";
		body.r(-1, "----------    ----------    --------------- ----------");
		for (licn = 1; licn <= nlics; ++licn) {
			body ^= FM;
			body ^= oconv(licx.a(2, licn), "[DATE,4]") ^ "   ";
			body ^= " " ^ oconv(licx.a(1, licn), "[DATE,4]") ^ "   ";
			body ^= " " ^ licx.a(3, licn).oconv("L#15") ^ " ";
			body ^= "    " ^ licx.a(5, licn);
		} //licn;
		body ^= FM;
		body.swapper("/ ", "/0");

		gosub emailadmin(msg);
		return 0;

	}

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

	//ok if the exodus user
	if (allowexodus and USERNAME eq "EXODUS") {
		goto ok;
	}

	//no lic check on exodus development machines
	if (allowexodus and var("exodus.id").osfile()) {
		goto ok;
	}

	msg = "EXODUS Lic" "ence: ";

	if (win.datafile eq "SCHEDULES") {
		datedict = "STOPDATE";
		chkmodulename = "MEDIA";

	} else if (win.datafile eq "PLANS") {
		datedict = "PERIOD_TO";
		chkmodulename = "MEDIA";

	} else if (win.datafile eq "JOBS") {
		datedict = "YEAR_PERIOD";
		chkmodulename = "JOBS";

	} else if (win.datafile eq "PRODUCTION_INVOICES" or win.datafile eq "PRODUCTION_ORDERS") {
		datedict = "DATE";
		chkmodulename = "JOBS";

	} else if (win.datafile eq "JOURNALS") {
		datedict = "YEAR_PERIOD";
		chkmodulename = "FINANCE";

	} else if (win.datafile eq "TIMESHEETS") {
		datedict = "DATE";
		chkmodulename = "TIMESHEETS";

	} else {
		msg = win.datafile.quote() ^ " datafile is invalid in chklic";
		return 0;
	}

	//determine document date or use last date of documents period
	tt = calculate(datedict);
	if (tt.match("^\\d{5}$")) {
		docdate = tt;
		dateorperiod = 1;
	} else if (tt.index(".")) {
		//docdate=iconv(tt:'.31','DJ')
		//if docdate else docdate=iconv(tt:'.30','DJ')
		//if docdate else docdate=iconv(tt:'.28','DJ')
		tt = tt.field(".", 2) ^ "." ^ tt.field(".", 1);
		docdate = ("31." ^ tt).iconv("DE");
		if (not docdate) {
			docdate = ("30." ^ tt).iconv("D.E");
		}
		if (not docdate) {
			docdate = ("28." ^ tt).iconv("D.E");
		}
		dateorperiod = 2;
	} else if (tt.index("/")) {
		docdate = ("31/" ^ tt).iconv("D/E");
		if (not docdate) {
			docdate = ("30/" ^ tt).iconv("D/E");
		}
		if (not docdate) {
			docdate = ("28/" ^ tt).iconv("D/E");
		}
		dateorperiod = 2;
	} else {
		msg = tt.quote() ^ " unrecognisable period or date in " ^ win.datafile ^ " " ^ ID;
		return 0;
	}

	//ok if document 1/period/year is in a specifically authorised lic period
	//skip expired licences
	if (not(licx.a(1).locateby("AR", docdate, licn))) {
		{}
	}
	//check remaining licences which havent expired
	for (licn = licn; licn <= 9999; ++licn) {
		uptodate = licx.a(1, licn);
		///BREAK;
		if (not uptodate) break;
		did = licx.a(4, licn);
		if (did eq datasetid and docdate ge licx.a(2, licn)) {
			modulename = licx.a(3, licn);
			if (modulename eq "*") {
ok:
				msg = "";
				return 0;
			}
			if (modulename eq chkmodulename) {
				goto ok;
			}
		}
	} //licn;

	//ok if current date is not after last expiry date plus grace days
	var nlics = licx.a(1).count(VM) + (licx ne "");
	//find last expiry date for current module
	var maxfinalexpirydate = "";
	var mincommencedate = "";
	for (licn = 1; licn <= nlics; ++licn) {
		modulename = licx.a(3, licn);
		did = licx.a(4, licn);
		if (did eq datasetid and ((modulename eq "*" or modulename eq chkmodulename))) {

			var expirydate = licx.a(1, licn);
			var daysgrace = licx.a(5, licn);
			var finalexpirydate = expirydate + daysgrace;

			if (finalexpirydate gt maxfinalexpirydate) {
				maxexpirydate = expirydate;
				maxfinalexpirydate = finalexpirydate;
			}

			var commencedate = licx.a(2, licn);
			if (commencedate lt mincommencedate or not(mincommencedate)) {
				mincommencedate = commencedate;
			}

		}
	} //licn;

	if (not maxfinalexpirydate) {
		//cannot find any lic (either * or specific)
		msg = "There is no licence to use the EXODUS " ^ chkmodulename ^ " module";
		return 0;
	}

	if (var().date() le maxfinalexpirydate and var().date() ge mincommencedate) {
		msg = "";
		return 0;
	}

	if (dateorperiod eq 1) {
		tt = "date";
	} else {
		tt = "period closing";
	}
	msg = "The document " ^ tt ^ " " ^ oconv(docdate, "[DATE,4*]") ^ " is not within";
	msg.r(-1, "the EXODUS licence period ");
	msg ^= oconv(mincommencedate, "[DATE,4*]") ^ " - ";
	msg ^= oconv(maxexpirydate, "[DATE,4*]");
	tt = var().date() - maxexpirydate;
	msg.r(-1, " and it is now " ^ oconv(var().date(), "[DATE,4*]") ^ " - " ^ tt ^ " days after the licence expiry date");

	return 0;
}

subroutine emailadmin(io msg) {

	call sendmail("admin@neosys.com", "", subject, body ^ "", "", "", errormsg);
	call sysmsg(body);

	if (mode eq "DELETE") {
		msg = "Licencing restrictions removed; and all licences";
	} else {
		msg = "Licence added OK.";
	}
	if (errormsg) {
		msg.r(-1, " but could NOT email admin@neosys.com" ^ FM ^ FM ^ errormsg);
		call mssg(msg);
	} else {
		call note(msg ^ FM ^ FM ^ "admin@neosys.com has been emailed" ^ FM);
	}
	msg = "";

	return;
}

subroutine syntax(io msg) {

	msg.r(-1, "Syntax is:" ^ FM ^ FM ^ "ADDLIC modulename fromdate uptodate daygrace");
	msg.r(-1, FM ^ "or " ^ FM ^ FM ^ "ADDLIC modulename fromdate uptodate daygrace compid databaseid verification");
	msg.r(-1, FM);
	return;
}

libraryexit()
