#include <exodus/library.h>
libraryinit()

#include <getlang.h>
#include <initcompany2.h>
#include <sysmsg.h>

#include <service_common.h>

#include <srv_common.h>

var ndec;  // num
var temp;

function main(in nextcompanycode) {

	// use app specific version of initcompany2
	if (APPLICATION != "EXODUS") {
		initcompany2 = "initcompany2_app";
	}

	if (not nextcompanycode.unassigned()) {

		if (nextcompanycode) {
			if (not var().read(srv.companies, nextcompanycode)) {
				call sysmsg(nextcompanycode.quote() ^ " COMPANY IS MISSING IN INIT.COMPANY()");
				// TODO return abort code and change all callers to handle failure
				return 0;
			}
		}

		srv.gcurrcompcode = nextcompanycode;
		// curr.company=nextcompanycode
	}

	let oldcompany = srv.company;
	if (srv.gcurrcompcode) {
		if (not srv.company.read(srv.companies, srv.gcurrcompcode)) {
			call mssg("COMPANY " ^ (srv.gcurrcompcode.quote()) ^ " IS MISSING - DO NOT PROCEED||USE \"SETTINGS\" TO|CHOOSE ANOTHER COMPANY|");
			srv.company = srv.gcurrcompcode;
		}
	} else {
		srv.company	   = "";
		srv.company(2) = date().oconv("D2/E").field("/", 2, 2);
	}

	// in LISTEN2 and INIT.COMPANY
	let companystyle = srv.company.f(70);
	if (companystyle) {
		SYSTEM(46) = companystyle;
	}

	// clientmark
	if (srv.company.f(27)) {
//		if (VOLUMES) {
//			srv.company(27) = srv.company.f(27).invert();
//		}
		SYSTEM(14) = srv.company.f(27);
		SYSTEM(8)  = "";
	} else {
		SYSTEM(14) = SYSTEM.f(36);
	}

	// if company code2 is not specified then use company code IF alphabetic
	if (not srv.company.f(28)) {
		if (srv.gcurrcompcode.match("^[A-Za-z]*$")) {
			srv.company(28) = srv.gcurrcompcode;
		}
	}

	// date format
	DATEFMT		   = "D2/E";
	let dateformat = srv.company.f(10);
	if (dateformat == "") {
		DATEFMT = "D2/E";
	} else if (dateformat.starts("31/01/")) {
		DATEFMT = "D2/E";
	} else if (dateformat.starts("31-01-")) {
		DATEFMT = "D2-E";
	} else if (dateformat == "31 JAN 90") {
		DATEFMT = "D2E";
	} else if (dateformat == "31 JAN 90.") {
		DATEFMT = "D2";
	} else if (dateformat.starts("01/31/")) {
		DATEFMT = "D2/";
	} else if (dateformat.starts("01-31-")) {
		DATEFMT = "D2-";
		// CASE DATE.FORMAT[-6,6]='90/01/31';@DATE.FORMAT='D2J'

		// CASE DATE.FORMAT='31/01/2000';@DATE.FORMAT='D2/E'
		// CASE DATE.FORMAT='31-01-2000';@DATE.FORMAT='D2-E'
	} else if (dateformat == "31 JAN 2000") {
		DATEFMT = "D2E";
	} else if (dateformat == "31 JAN 2000.") {
		DATEFMT = "D2";
		// CASE DATE.FORMAT='01/31/2000';@DATE.FORMAT='D2/'
		// CASE DATE.FORMAT='01-31-2000';@DATE.FORMAT='D2-'
		// CASE DATE.FORMAT='2000/01/31';@DATE.FORMAT='D2J'
	}

	// in init.company and init.general

	if (srv.glang == "" or srv.company.f(14) != oldcompany.f(14)) {
		call getlang("GENERAL", "", "", srv.alanguage, srv.glang);
		// 		if (srv.glang.f(9)) {
		// 			UPPERCASE = srv.glang.f(9);
		// 		}
		// 		if (srv.glang.f(10)) {
		// 			LOWERCASE = srv.glang.f(10);
		// 		}
		// 		UPPERCASE.replacer("%FF", RM);
		// 		LOWERCASE.replacer("%FF", RM);
		// 		UPPERCASE.replacer("%FE", FM);
		// 		LOWERCASE.replacer("%FE", FM);
		// 		UPPERCASE.replacer("%FD", VM);
		// 		LOWERCASE.replacer("%FD", VM);
		// 		UPPERCASE.replacer("%25", "%");
		// 		LOWERCASE.replacer("%25", "%");
		// 		if (UPPERCASE.len() != LOWERCASE.len()) {
		// 			LOWERCASE = "abcdefghijklmnopqrstuvwxyz";
		// 			UPPERCASE = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		// 		}

		// sort.order
		// read sortorder from alanguage,'SORTORDER*':company<14> else
		// read sortorder from alanguage,'SORTORDER' else sortorder=''
		// end
		// if len(sortorder)=256 then
		// call sysvar('SET',109,147,sortorder)
		// end
	}

	if (not srv.company.f(4)) {
		srv.company(4) = srv.company.f(5);
	}
	if (not srv.company.f(5)) {
		srv.company(5) = srv.company.f(4);
	}
	// if intercurrency conversion account is blank then
	// trial balance balances in base currency but not for each currency separately
	// IF COMPANY<12> ELSE COMPANY<12>=COMPANY<4>

	// convert currency gain/loss/conversion accounts and vat control to internal
	// this was not a good idea but remains compatible with older code
	// TODO remove and change all other code
	if (srv.company.f(4, 1, 2)) {
		srv.company(4) = srv.company.f(4, 1, 2);
	}
	if (srv.company.f(5, 1, 2)) {
		srv.company(5) = srv.company.f(5, 1, 2);
	}
	if (srv.company.f(12, 1, 2)) {
		srv.company(12) = srv.company.f(12, 1, 2);
	}
	// taxaccno=company<19>
	if (srv.company.f(19, 1, 2)) {
		srv.company(19) = srv.company.f(19, 1, 2);
	}

	// save base currency for general use
	SYSTEM(134) = srv.company.f(3);

	// number format (@USER2)
	if (not ndec.readf(srv.currencies, srv.company.f(3), 3)) {
		ndec = 2;
	}
	// default to dot for decimal point
	BASEFMT = "MD";
	// optional comma for decimal point
	if (var("1.000,00|1000,00").locateusing("|", srv.company.f(22), temp)) {
		BASEFMT = "MC";
	}
	BASEFMT ^= ndec ^ "0P";
	// optional comma to indicate delimiting of thousands (with comma MD OR dot MC)
	if (var("1,000.00|1.000,00").locateusing("|", srv.company.f(22), temp)) {
		BASEFMT ^= ",";
	}

	// financial initialisation
	call initcompany2(oldcompany);

	// dateperiod conversion in case not done in init.company2 above
	// financial year dates
	let financialyear = srv.company.f(6);
	var firstmonth	  = financialyear.field(",", 1);
	if (firstmonth.isnum()) {
		if (not var("1,2,3,4,5,6,7,8,9,10,11,12").locateusing(",", firstmonth, temp)) {
			firstmonth = 1;
		}
		var maxperiod = financialyear.field(",", 2);
		if (not((maxperiod.match("^\\d*$") and maxperiod > 0) and maxperiod <= 99)) {
			maxperiod = 12;
		}
		srv.company(6) = "[DATEPERIOD," ^ firstmonth ^ "," ^ maxperiod ^ "]";
	}

	return 1;
}

libraryexit()
