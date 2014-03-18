#include <exodus/library.h>
libraryinit()

#include <swap.h>

#include <gen.h>

function main() {
	//jbase
	if (not(fin.accparams.read(gen._definitions, "ACC.PARAMS"))) {
		if (not fin.accparams.osread("ACCOUNTS.CFG")) {
			fin.accparams = "";
		}
	}
	if (fin.accparams.a(11) == "") {
		fin.accparams.r(11, "Budget");
		fin.accparams.r(12, "F/cast");
		if ((gen.company.a(1)).index("Impact", 1)) {
			fin.accparams.r(11, 3, "Plan");
			fin.accparams.r(12, 2, "Apr F/c" _VM_ "Jul F/c" _VM_ "Oct F/c");
		}
		for (var ii = 2; ii <= 10; ++ii) {
			for (var fn = 11; fn <= 12; ++fn) {
				if (not(fin.accparams.a(fn, ii + 1))) {
					fin.accparams.r(fn, ii + 1, fin.accparams.a(fn, 1) ^ " " ^ ii);
				}
			};//fn;
		};//ii;
	}

	if (fin.accparams.a(16, 1) == "ALL") {
		var tt = "//Brand/Vehicle////Executive//////Market/////////Client/Supplier//////////Media Type";
		var tt2 = fin.accparams.a(16);
		tt2.r(1, 1, "");
		var nn = tt.count("/") + 1;
		for (var ii!!! = 1; ii!!! <= nn; ++ii!!!) {
			if (tt2.a(1, ii!!!) == "") {
				tt2.r(1, ii!!!, tt.field("/", ii!!!));
			}
		};//ii!!!;
		fin.accparams.r(16, tt2);
	}

	if (fin.accparams.a(16) == "" and ACCOUNT == "ADAGENCY") {

		fin.accparams.r(16, 3, "Brand");
		//acc.params<16,4>='Vehicle'
		fin.accparams.r(16, 8, "Executive");

		//14 means use mod(14,10) ie field 4 (vehicle code) to get market
		//acc.params<16,14>='Market'

		//23 means use mod(23,10) ie field 3 (brand code) to get client
		//acc.params<16,23>='Client'

		//24 means use mod(24,10) ie field 4 (vehicle code) to get supplier
		//acc.params<16,24>='Supplier'

		//34 means use mod(34,10) ie field 4 (vehicle code) to get media type
		//acc.params<16,34>='Media Type'

	}

	//save last time read so can be refreshed every so often in financeproxy etc
	fin.accparams.r(3, var().time());

	if (fin.accparams.a(15) == "") {
		fin.accparams.r(15, "30");
	}
	if (fin.accparams.a(1) == "") {
		fin.accparams.r(1, "2000");
	}

	if (fin.accparams.a(17) == "") {
		fin.accparams.r(17, "YYNNNN");
	}

	//18 free

	//default=2
	//0=None
	//1=Top
	//2=Bottom
	if (fin.accparams.a(19) == "") {
		fin.accparams.r(19, 2);
	}

	//debit/credit note
	if (fin.accparams.a(20) == "") {
		fin.accparams.r(20, "<u>Prepared by</u>");
	}
	if (fin.accparams.a(21) == "") {
		fin.accparams.r(21, "<u>Approved by</u>");
	}
	if (fin.accparams.a(22) == "") {
		fin.accparams.r(22, "<u>For Finance</u>");
	}
	//if acc.params<23>='' then acc.params<23>='???'

	//payment (no need for "for finance")
	if (fin.accparams.a(25) == "") {
		fin.accparams.r(25, "<u>Prepared by</u>");
	}
	if (fin.accparams.a(26) == "") {
		fin.accparams.r(26, "<u>Approved by</u>");
	}
	//if acc.params<27>='' then acc.params<27>='<u>For Finance</u>'
	//if acc.params<28>='' then acc.params<28>='???'

	//receipt (only needs "for xxx")
	//if acc.params<30>='' then acc.params<30>='<u>Prepared by</u>'
	//if acc.params<31>='' then acc.params<31>='<u>Approved by</u>'
	//if acc.params<32>='' then acc.params<32>='<u>For Finance</u>'
	if (fin.accparams.a(33) == "") {
		var tt = "Received in full or partial settlement of the above listed invoices unless we";
		tt.r(1, -1, "are notified otherwise within fifteen days. <i>All cheques are subject to clearing.</i>");
		fin.accparams.r(33, swap(VM, "<br/>", tt));
	}
	return 0;

}


libraryexit()