#include <exodus/library.h>
libraryinit()

#include <sysmsg.h>
#include <getlang.h>
#include <addcent.h>
#include <log2.h>
#include <fin.h>
#include <gen.h>

var weeksperperiod;//num
var companydesc;
var perioddesc;
var currencydesc;
var username;
var datax;
var logtime;

function main(in nextcompanycode) {

	call log2("-----initcompany init", logtime);

	call log2("default fin.currcompany", logtime);
	if (fin.currcompany.unassigned()) {
		fin.currcompany="";
		fin.currcurrency="";
		fin.converted="";
	}

	call log2("get first company", logtime);
	if (not nextcompanycode.unassigned()) {

		if (nextcompanycode) {
			var xx;
			if (not(xx.read(gen.companies, nextcompanycode))) {
				call sysmsg(DQ ^ (nextcompanycode ^ DQ) ^ " COMPANY IS MISSING IN INIT.COMPANY()");
				//TODO return abort code and change all callers to handle failure
				return 0;
			}
		}

		gen.gcurrcompany = nextcompanycode;
		fin.currcompany = nextcompanycode;
	}

	call log2("copy to globals", logtime);
	var oldcompany = gen.company;
	if (fin.currcompany) {
		if (not(gen.company.read(gen.companies, fin.currcompany))) {
			call mssg("COMPANY " ^ fin.currcompany.quote() ^ " IS MISSING - DO NOT PROCEED||USE \"SETTINGS\" TO|CHOOSE ANOTHER COMPANY|");
			gen.company = fin.currcompany;
		}
	}else{
		gen.company = "";
		gen.company.r(2, (var().date()).oconv("D2/E").field("/", 2, 2));
	}

	call log2("clientmark", logtime);
	if (gen.company.a(27)) {
		//SYSTEM.r(14, (gen.company.a(27)).invert());
		SYSTEM.r(8, "");
	}else{
		SYSTEM.r(14, SYSTEM.a(36));
	}

	log2("move the company to the front of the consolidated companies", logtime);
	if (fin.currcompanycodes.unassigned()) {
		fin.currcompanycodes = "";
	}
	var cn;
	if (fin.currcompanycodes.locateusing(fin.currcompany, VM, cn)) {
		fin.currcompanycodes.eraser(1, cn);
	}else{
		fin.currcompanycodes = "";
	}
	fin.currcompanycodes.inserter(1, 1, fin.currcompany);

	call log2("in init.company and init.general", logtime);

	if (gen.glang.unassigned() or gen.glang == "" or gen.company.a(14) ne oldcompany.a(14)) {
		call getlang("GENERAL", "", "", fin.alanguage, gen.glang);
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

	call log2("financial year dates", logtime);
	var financialyear = gen.company.a(6);
	var firstmonth = financialyear.field(",", 1);
	var temp;
	if (firstmonth.isnum()) {
		if (not(var("1,2,3,4,5,6,7,8,9,10,11,12").locateusing(firstmonth, ","))) {
			firstmonth = 1;
		}
		fin.maxperiod = financialyear.field(",", 2);
		if (not(fin.maxperiod.match("0N") and fin.maxperiod > 0 and fin.maxperiod <= 99)) {
			fin.maxperiod = 12;
		}
		gen.company.r(6, "[DATEPERIOD," ^ firstmonth ^ "," ^ fin.maxperiod ^ "]");
	}else{
		firstmonth = 1;
		if (addcent((gen.company.a(2)).substr(-2, 2)) >= 2004 or financialyear.field(",", 1) == "WEEKLY2") {
			fin.maxperiod = 12;
		}else{
			//"13X4WEEK,1/7,5" could be replaced by "WEEKLY,1/7,5,4"
			//"13X4WEEK" is in the old SABA data
			if (firstmonth == "13X4WEEK") {
				weeksperperiod = 4;
			}else{
				weeksperperiod = gen.company.a(6).field(",", 4);
				if (not weeksperperiod) {
					weeksperperiod = 1;
				}
			}
			fin.maxperiod = (52 / weeksperperiod).oconv("MD00");
		}
		gen.company.r(6, "[" ^ gen.company.a(6) ^ "]");
	}
	gen.company.r(45, firstmonth ^ "," ^ fin.maxperiod);

	call log2("date format", logtime);
	DATEFORMAT = "D2/E";
	var dateformat = gen.company.a(10);
	if (dateformat == "") {
		DATEFORMAT = "D2/E";
	} else if (dateformat.substr(1, 6) == "31/01/") {
		DATEFORMAT = "D2/E";
	} else if (dateformat.substr(1, 6) == "31-01-") {
		DATEFORMAT = "D2-E";
	} else if (dateformat == "31 JAN 90") {
		DATEFORMAT = "D2E";
	} else if (dateformat == "31 JAN 90.") {
		DATEFORMAT = "D2";
	} else if (dateformat.substr(1, 6) == "01/31/") {
		DATEFORMAT = "D2/";
	} else if (dateformat.substr(1, 6) == "01-31-") {
		DATEFORMAT = "D2-";
	} else if (dateformat.substr(-6, 6) == "90/01/31") {
		DATEFORMAT = "D2J";
	//CASE DATE.FORMAT='31/01/2000';@DATE.FORMAT='D2/E'
	//CASE DATE.FORMAT='31-01-2000';@DATE.FORMAT='D2-E'
	} else if (dateformat == "31 JAN 2000") {
		DATEFORMAT = "D2E";
	} else if (dateformat == "31 JAN 2000.") {
		DATEFORMAT = "D2";
	//CASE DATE.FORMAT='01/31/2000';@DATE.FORMAT='D2/'
	//CASE DATE.FORMAT='01-31-2000';@DATE.FORMAT='D2-'
	//CASE DATE.FORMAT='2000/01/31';@DATE.FORMAT='D2J'
	}

	call log2("get voucher type details", logtime);
	fin.definition = "";
	if (DEFINITIONS ne "") {
		//almost certainly by company is not implemented at any client or other code
		//TODO confirm and deleted
		if (not(fin.definition.read(DEFINITIONS, "ALL*" ^ fin.currcompany))) {
			if (not(fin.definition.read(DEFINITIONS, "ALL"))) {
				{}
			}
		}
	}
	var nvs = (fin.definition.a(1)).count(VM) + 1;
	for (var vn = 1; vn <= nvs; ++vn) {
		var temp = fin.definition.a(1, vn);
		if (temp) {
			//fix prefix
			if (fin.definition.a(6, vn) == "") {
				fin.definition.r(6, vn, temp);
			}
		}
	};//vn;
	fin.basecurrency = gen.company.a(3);

	if (not gen.company.a(4)) {
		gen.company.r(4, gen.company.a(5));
	}
	if (not gen.company.a(5)) {
		gen.company.r(5, gen.company.a(4));
	}
	call log2("if intercurrency conversion account is blank then", logtime);
	//trial balance balances in base currency but not for each currency separately
	//IF COMPANY<12> ELSE COMPANY<12>=COMPANY<4>

	call log2("convert currency gain/loss/conversion accounts and vat control to internal", logtime);
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
	var taxaccno = gen.company.a(19);
	if (gen.company.a(19, 1, 2)) {
		gen.company.r(19, gen.company.a(19, 1, 2));
	}

	call log2("initialise the vat rates", logtime);
	fin.taxes = "";
	if (DEFINITIONS ne "") {
		if (not(fin.taxes.read(DEFINITIONS, "TAXES"))) {
			fin.taxes = "Standard" _VM_ "Zero rated" _VM_ "Exempt";
			fin.taxes.r(2, "1" _VM_ "2" _VM_ "3");
			fin.taxes.r(3, "5" _VM_ "0" _VM_ "");
			fin.taxes.write(DEFINITIONS, "TAXES");
		}

		//onetime setup of tax account number per tax code
		//company file tax acc not used anywhere else
		if (taxaccno and fin.taxes.a(4) == "") {
			for (var ii = 1; ii <= 999; ++ii) {
			///BREAK;
			if (not(fin.taxes.a(1, ii))) break;;
				fin.taxes.r(4, ii, taxaccno);
			};//ii;
			fin.taxes.write(DEFINITIONS, "TAXES");
		}

	}

	call log2("initialise the period selection if the company period is different", logtime);
	if (fin.currperiod.unassigned()) {
		fin.currperiod = "";
	}
	if (fin.currperiod == "" or oldcompany == "" or gen.company.a(2) ne oldcompany.a(2)) {
		//current period is determined by system date
		var curryearperiod = (var().date()).oconv(gen.company.a(6));
		fin.currperiod = curryearperiod.substr(-2, 2) + 0;
		fin.curryear = curryearperiod.substr(1, 2);
		var maxyear = gen.company.a(2).field("/", 2);
		if (addcent(fin.curryear) > addcent(maxyear)) {
			fin.curryear = maxyear;
			fin.currperiod = fin.maxperiod;
		}
		//current period is determined by company record
		//CURRYEARPERIOD=COMPANY<2>
		//CURRPERIOD=FIELD(CURRYEARPERIOD,'/',1)+0
		//CURRYEAR= FIELD(CURRYEARPERIOD,'/',2)
		fin.currfromperiod = 1;
		fin.currtoperiod = fin.currperiod;
		fin.currfromyear = fin.curryear;
		fin.currtoyear = fin.curryear;
		fin.fromperiod = fin.currperiod;
		fin.toperiod = fin.currperiod;
		fin.fromyear = fin.curryear;

		fin.toyear = fin.curryear;
	}

	//initialise the currency selection if base or main currency changes
	//IF UNASSIGNED(CURR.CURRENCY) THEN CURR.CURRENCY=''
	//IF CURR.CURRENCY='' OR COMPANY<3> NE OLD.COMPANY<3> OR COMPANY<15> NE OLD.COMPANY<15> THEN
	if (gen.company.a(3) ne oldcompany.a(3) or gen.company.a(15) ne oldcompany.a(15)) {
		//if main currency then one currency unconverted
		fin.currcurrency = gen.company.a(15);
		if (fin.currcurrency) {
			fin.converted = "";
		}else{
			fin.converted = fin.basecurrency;
		}
	}

	//get base format
	//IF COMPANIES<>'' THEN
	var ndec;
	if (not(ndec.readv(gen.currencies, fin.basecurrency, 3))) {
		ndec = 2;
	}
	//END ELSE
	// NDEC='2'
	// END
	fin.basefmt = "MD" ^ ndec ^ "0PZ";

	//number format (@USER2)
	USER2 = "MC";
	if (var("1,000.00 1000.00").locateusing(gen.company.a(22), " ")) {
		USER2 = "MD";
	}
	USER2 ^= ndec ^ "0P";
	if (var("1,000.00 1.000,00").locateusing(gen.company.a(22), " ")) {
		USER2 ^= ",";
	}

	call log2("build up description", logtime);
	/////////////////////

	//company
	if (fin.currcompany) {
		companydesc = gen.company.a(1);
	}else{
		companydesc = "";
		gen.company = "";
	}

	//period
	if (fin.currcompany) {
		perioddesc = fin.fromperiod;
		if (fin.toyear == fin.fromyear) {
	// IF TOPERIOD NE FROMPERIOD THEN PERIODDESC:='-':TOPERIOD
			if (fin.toperiod ne fin.fromperiod) {
				perioddesc ^= "/" ^ addcent(fin.fromyear) ^ "-" ^ fin.toperiod;
			}
		}else{
			perioddesc ^= "/" ^ addcent(fin.fromyear) ^ "-" ^ fin.toperiod;
		}
		perioddesc ^= "/" ^ addcent(fin.toyear);
	}else{
		perioddesc = "";
	}

	//currency
	if (not fin.currcompany) {
		goto nocurrency;
	}
	if (gen.currencies == "") {
nocurrency:
		fin.currfmt = fin.basefmt;
		currencydesc = "";
		//goto currencydescexit;

	} else {

		var desc1 = "";
		var desc2 = "";
		if (fin.currcurrency) {
			var currencyrec;
			if (not(currencyrec.read(gen.currencies, fin.currcurrency))) {
				currencyrec = fin.currcurrency ^ FM ^ FM ^ 2;
			}
			desc1 = currencyrec.a(1);
			if (fin.currcurrency ne fin.basecurrency and fin.converted) {
		//? in ?
				currencydesc = gen.glang.a(4);
				goto basedesc;
			}
		//? only
			currencydesc = gen.glang.a(5);
			fin.currfmt = "MD" ^ currencyrec.a(3) ^ "0P";
		}else{
			if (fin.converted) {
				if (fin.converted == 1) {
					fin.converted = fin.basecurrency;
				}

			//All currencies in ?
				currencydesc = gen.glang.a(6);
basedesc:
				//get conversion currency
				var convcurrency;
				if (not(convcurrency.read(gen.currencies, fin.converted))) {
					convcurrency = fin.converted ^ FM ^ FM ^ 2;
				}
				desc2 = convcurrency.a(1);
				fin.currfmt = "MD" ^ convcurrency.a(3) ^ "0P";
			}else{
		//All currencies unconverted
				currencydesc = gen.glang.a(7);
			}
		}

		if (gen.company.a(4) == "") {
			currencydesc = desc2;
		}else{
			if (desc1) {
				var temp = currencydesc.index("?", 1);
				if (temp) {
					currencydesc.splicer(temp, 1, desc1);
				}
			}
			if (desc2) {
				temp = currencydesc.index("?", 1);
				if (temp) {
					currencydesc.splicer(temp, 1, desc2);
				}
			}
		}

	}//currencydescexit:

	call log2("combine and exit", logtime);
	if (gen.company.a(2)) {
		//ensure period,currency,company by removing any commas in period/curr/comp
		fin.gendesc = perioddesc.convert(",", "-");
		if (currencydesc) {
			fin.gendesc ^= currencydesc.convert(",","-");
		}
		if (companydesc) {
			fin.gendesc ^= ",  " ^ companydesc.convert(",","-");
		}
	}else{
		fin.gendesc = companydesc;
	}
	fin.gendesc.trimmer();

	if (not SYSTEM.a(33)) {

		username=USERNAME.trim();
		var s23 = SYSTEM.a(23);
		s23.converter(" ", FM);
		//locate 'TEST' in s23 setting x then username:='*' else
		if (s23.locateusing("TESTDATA", FM)) {
			username ^= "*";
		}else{
			if (s23.locateusing("TESTING", FM)) {
				username ^= "*";
			}else{
				if (s23.locateusing("TRAINING", FM)) {
					username ^= "*";
				}else{
					if ((SYSTEM.a(17)).index("TEST", 1)) {
						username ^= "*";
					}
				}
			}
		}
		// end

	}

	call log2("-----initcompany exit", logtime);
	return var("");

}

libraryexit()
