#include <exodus/library.h>
libraryinit()

#include <sysmsg.h>
#include <getlang.h>
#include <initcompany2.h>

#include <system_common.h>

#include <sys_common.h>

var ndec;//num
var temp;

function main(in nextcompanycode) {


    //use app specific version of initcompany2
    if (APPLICATION ne "EXODUS") {
        initcompany2 = "initcompany2_app" ;
    }

	if (not(nextcompanycode.unassigned())) {

		if (nextcompanycode) {
			var xx;
			if (not(xx.read(sys.companies, nextcompanycode))) {
				call sysmsg(nextcompanycode.quote() ^ " COMPANY IS MISSING IN INIT.COMPANY()");
				//TODO return abort code and change all callers to handle failure
				return 0;
				}
			}

		sys.gcurrcompcode = nextcompanycode;
		//curr.company=nextcompanycode
	}

	var oldcompany = sys.company;
	if (sys.gcurrcompcode) {
		if (not(sys.company.read(sys.companies, sys.gcurrcompcode))) {
			call mssg("COMPANY " ^ (sys.gcurrcompcode.quote()) ^ " IS MISSING - DO NOT PROCEED||USE \"SETTINGS\" TO|CHOOSE ANOTHER COMPANY|");
			sys.company = sys.gcurrcompcode;
		}
	} else {
		sys.company = "";
		sys.company(2) = date().oconv("D2/E").field("/", 2, 2);
	}

	//in LISTEN2 and INIT.COMPANY
	var companystyle = sys.company.f(70);
	if (companystyle) {
		SYSTEM(46) = companystyle;
	}

	//clientmark
	if (sys.company.f(27)) {
		if (VOLUMES) {
			sys.company(27) = sys.company.f(27).invert();
		}
		SYSTEM(14) = sys.company.f(27);
		SYSTEM(8) = "";
	} else {
		SYSTEM(14) = SYSTEM.f(36);
	}

	//if company code2 is not specified then use company code IF alphabetic
	if (not(sys.company.f(28))) {
		if (sys.gcurrcompcode.match("^[A-Za-z]*$")) {
			sys.company(28) = sys.gcurrcompcode;
		}
	}

	//date format
	DATEFMT = "D2/E";
	var dateformat = sys.company.f(10);
	if (dateformat eq "") {
		DATEFMT = "D2/E";
	} else if (dateformat.starts("31/01/")) {
		DATEFMT = "D2/E";
	} else if (dateformat.starts("31-01-")) {
		DATEFMT = "D2-E";
	} else if (dateformat eq "31 JAN 90") {
		DATEFMT = "D2E";
	} else if (dateformat eq "31 JAN 90.") {
		DATEFMT = "D2";
	} else if (dateformat.starts("01/31/")) {
		DATEFMT = "D2/";
	} else if (dateformat.starts("01-31-")) {
		DATEFMT = "D2-";
	//CASE DATE.FORMAT[-6,6]='90/01/31';@DATE.FORMAT='D2J'

	//CASE DATE.FORMAT='31/01/2000';@DATE.FORMAT='D2/E'
	//CASE DATE.FORMAT='31-01-2000';@DATE.FORMAT='D2-E'
	} else if (dateformat eq "31 JAN 2000") {
		DATEFMT = "D2E";
	} else if (dateformat eq "31 JAN 2000.") {
		DATEFMT = "D2";
	//CASE DATE.FORMAT='01/31/2000';@DATE.FORMAT='D2/'
	//CASE DATE.FORMAT='01-31-2000';@DATE.FORMAT='D2-'
	//CASE DATE.FORMAT='2000/01/31';@DATE.FORMAT='D2J'
	}

	//in init.company and init.general

	if (sys.glang eq "" or sys.company.f(14) ne oldcompany.f(14)) {
		call getlang("GENERAL", "", "", sys.alanguage, sys.glang);
//		if (sys.glang.f(9)) {
//			UPPERCASE = sys.glang.f(9);
//		}
//		if (sys.glang.f(10)) {
//			LOWERCASE = sys.glang.f(10);
//		}
//		UPPERCASE.replacer("%FF", RM);
//		LOWERCASE.replacer("%FF", RM);
//		UPPERCASE.replacer("%FE", FM);
//		LOWERCASE.replacer("%FE", FM);
//		UPPERCASE.replacer("%FD", VM);
//		LOWERCASE.replacer("%FD", VM);
//		UPPERCASE.replacer("%25", "%");
//		LOWERCASE.replacer("%25", "%");
//		if (UPPERCASE.len() ne LOWERCASE.len()) {
//			LOWERCASE = "abcdefghijklmnopqrstuvwxyz";
//			UPPERCASE = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
//		}

		//sort.order
		//read sortorder from alanguage,'SORTORDER*':company<14> else
		// read sortorder from alanguage,'SORTORDER' else sortorder=''
		// end
		//if len(sortorder)=256 then
		// call sysvar('SET',109,147,sortorder)
		// end

	}

	if (not(sys.company.f(4))) {
		sys.company(4) = sys.company.f(5);
	}
	if (not(sys.company.f(5))) {
		sys.company(5) = sys.company.f(4);
	}
	//if intercurrency conversion account is blank then
	//trial balance balances in base currency but not for each currency separately
	//IF COMPANY<12> ELSE COMPANY<12>=COMPANY<4>

	//convert currency gain/loss/conversion accounts and vat control to internal
	//this was not a good idea but remains compatible with older code
	//TODO remove and change all other code
	if (sys.company.f(4, 1, 2)) {
		sys.company(4) = sys.company.f(4, 1, 2);
	}
	if (sys.company.f(5, 1, 2)) {
		sys.company(5) = sys.company.f(5, 1, 2);
	}
	if (sys.company.f(12, 1, 2)) {
		sys.company(12) = sys.company.f(12, 1, 2);
	}
	//taxaccno=company<19>
	if (sys.company.f(19, 1, 2)) {
		sys.company(19) = sys.company.f(19, 1, 2);
	}

	//save base currency for general use
	SYSTEM(134) = sys.company.f(3);

	//number format (@USER2)
	if (not(ndec.readv(sys.currencies, sys.company.f(3), 3))) {
		ndec = 2;
	}
	//default to dot for decimal point
	BASEFMT = "MD";
	//optional comma for decimal point
	if (var("1.000,00|1000,00").locateusing("|", sys.company.f(22), temp)) {
		BASEFMT = "MC";
	}
	BASEFMT ^= ndec ^ "0P";
	//optional comma to indicate delimiting of thousands (with comma MD OR dot MC)
	if (var("1,000.00|1.000,00").locateusing("|", sys.company.f(22), temp)) {
		BASEFMT ^= ",";
	}

	//financial initialisation
	call initcompany2(oldcompany);

	//dateperiod conversion in case not done in init.company2 above
	//financial year dates
	var financialyear = sys.company.f(6);
	var firstmonth = financialyear.field(",", 1);
	if (firstmonth.isnum()) {
		if (not(var("1,2,3,4,5,6,7,8,9,10,11,12").locateusing(",", firstmonth, temp))) {
			firstmonth = 1;
		}
		var maxperiod = financialyear.field(",", 2);
		if (not((maxperiod.match("^\\d*$") and maxperiod gt 0) and maxperiod le 99)) {
			maxperiod = 12;
		}
		sys.company(6) = "[DATEPERIOD," ^ firstmonth ^ "," ^ maxperiod ^ "]";
	}

	return 1;
}

libraryexit()
