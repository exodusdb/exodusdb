#include <exodus/library.h>
libraryinit()

#include <sysmsg.h>
#include <getlang.h>
#include <initcompany2.h>

#include <gen_common.h>

var ndec;//num
var temp;

function main(in nextcompanycode) {
	//c sys

	#include <general_common.h>
	//global desc1,desc2

	#define interactive not(SYSTEM.a(33))

	if (not(nextcompanycode.unassigned())) {

		if (nextcompanycode) {
			var xx;
			if (not(xx.read(gen.companies, nextcompanycode))) {
				call sysmsg(nextcompanycode.quote() ^ " COMPANY IS MISSING IN INIT.COMPANY()");
				//TODO return abort code and change all callers to handle failure
				return 0;
				}
			}

		gen.gcurrcompany = nextcompanycode;
		//curr.company=nextcompanycode
	}

	var oldcompany = gen.company;
	if (gen.gcurrcompany) {
		if (not(gen.company.read(gen.companies, gen.gcurrcompany))) {
			call mssg("COMPANY " ^ (gen.gcurrcompany.quote()) ^ " IS MISSING - DO NOT PROCEED||USE \"SETTINGS\" TO|CHOOSE ANOTHER COMPANY|");
			gen.company = gen.gcurrcompany;
		}
	} else {
		gen.company = "";
		gen.company.r(2, var().date().oconv("D2/E").field("/", 2, 2));
	}

	//in LISTEN2 and INIT.COMPANY
	var companystyle = gen.company.a(70);
	if (companystyle) {
		SYSTEM.r(46, companystyle);
	}

	//clientmark
	if (gen.company.a(27)) {
		if (VOLUMES) {
			gen.company.r(27, gen.company.a(27).invert());
		}
		SYSTEM.r(14, gen.company.a(27));
		SYSTEM.r(8, "");
	} else {
		SYSTEM.r(14, SYSTEM.a(36));
	}

	//if company code2 is not specified then use company code IF alphabetic
	if (not(gen.company.a(28))) {
		if (gen.gcurrcompany.match("^[A-Za-z]*$")) {
			gen.company.r(28, gen.gcurrcompany);
		}
	}

	//date format
	DATEFMT = "D2/E";
	var dateformat = gen.company.a(10);
	if (dateformat eq "") {
		DATEFMT = "D2/E";
	} else if (dateformat.substr(1, 6) eq "31/01/") {
		DATEFMT = "D2/E";
	} else if (dateformat.substr(1, 6) eq "31-01-") {
		DATEFMT = "D2-E";
	} else if (dateformat eq "31 JAN 90") {
		DATEFMT = "D2E";
	} else if (dateformat eq "31 JAN 90.") {
		DATEFMT = "D2";
	} else if (dateformat.substr(1, 6) eq "01/31/") {
		DATEFMT = "D2/";
	} else if (dateformat.substr(1, 6) eq "01-31-") {
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

	if (gen.glang eq "" or gen.company.a(14) ne oldcompany.a(14)) {
		call getlang("GENERAL", "", "", gen.alanguage, gen.glang);
		if (gen.glang.a(9)) {
			UPPERCASE = gen.glang.a(9);
		}
		if (gen.glang.a(10)) {
			LOWERCASE = gen.glang.a(10);
		}
		UPPERCASE.swapper("%FF", RM);
		LOWERCASE.swapper("%FF", RM);
		UPPERCASE.swapper("%FE", FM);
		LOWERCASE.swapper("%FE", FM);
		UPPERCASE.swapper("%FD", VM);
		LOWERCASE.swapper("%FD", VM);
		UPPERCASE.swapper("%25", "%");
		LOWERCASE.swapper("%25", "%");
		if (UPPERCASE.length() ne LOWERCASE.length()) {
			LOWERCASE = "abcdefghijklmnopqrstuvwxyz";
			UPPERCASE = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		}

		//sort.order
		//read sortorder from alanguage,'SORTORDER*':company<14> else
		// read sortorder from alanguage,'SORTORDER' else sortorder=''
		// end
		//if len(sortorder)=256 then
		// call sysvar('SET',109,147,sortorder)
		// end

	}

	if (not(gen.company.a(4))) {
		gen.company.r(4, gen.company.a(5));
	}
	if (not(gen.company.a(5))) {
		gen.company.r(5, gen.company.a(4));
	}
	//if intercurrency conversion account is blank then
	//trial balance balances in base currency but not for each currency separately
	//IF COMPANY<12> ELSE COMPANY<12>=COMPANY<4>

	//convert currency gain/loss/conversion accounts and vat control to internal
	//this was not a good idea but remains compatible with older code
	//TODO remove and change all other code
	if (gen.company.a(4, 1, 2)) {
		gen.company.r(4, gen.company.a(4, 1, 2));
	}
	if (gen.company.a(5, 1, 2)) {
		gen.company.r(5, gen.company.a(5, 1, 2));
	}
	if (gen.company.a(12, 1, 2)) {
		gen.company.r(12, gen.company.a(12, 1, 2));
	}
	//taxaccno=company<19>
	if (gen.company.a(19, 1, 2)) {
		gen.company.r(19, gen.company.a(19, 1, 2));
	}

	//save base currency for general use
	SYSTEM.r(134, gen.company.a(3));

	//number format (@USER2)
	if (not(ndec.readv(gen.currencies, gen.company.a(3), 3))) {
		ndec = 2;
	}
	//default to dot for decimal point
	BASEFMT = "MD";
	//optional comma for decimal point
	if (var("1.000,00|1000,00").locateusing("|", gen.company.a(22), temp)) {
		BASEFMT = "MC";
	}
	BASEFMT ^= ndec ^ "0P";
	//optional comma to indicate delimiting of thousands (with comma MD OR dot MC)
	if (var("1,000.00|1.000,00").locateusing("|", gen.company.a(22), temp)) {
		BASEFMT ^= ",";
	}

	//financial initialisation
	call initcompany2(oldcompany);

	//dateperiod conversion in case not done in init.company2 above
	//financial year dates
	var financialyear = gen.company.a(6);
	var firstmonth = financialyear.field(",", 1);
	if (firstmonth.isnum()) {
		if (not(var("1,2,3,4,5,6,7,8,9,10,11,12").locateusing(",", firstmonth, temp))) {
			firstmonth = 1;
		}
		var maxperiod = financialyear.field(",", 2);
		if (not((maxperiod.match("^\\d*$") and maxperiod gt 0) and maxperiod le 99)) {
			maxperiod = 12;
		}
		gen.company.r(6, "[DATEPERIOD," ^ firstmonth ^ "," ^ maxperiod ^ "]");
	}

	return 1;
}

libraryexit()
