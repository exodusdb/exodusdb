#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <singular.h>

#include <fin_common.h>
#include <gen_common.h>
#include <win_common.h>

#include <window.hpp>

var nmvfns;//num
var notmv;
var journalcode;
var msg;
var reply;//num
var vatrequired;
var daybookn;//num
var daybooktype;
var fn;//num
var xx;
var tt;
var copydaybookn;
var op;
var op2;
var wspos;
var wsmsg;

function main(in mode) {
	//c fin
	//global tt,notmv,nmvfns,fn

	#include <common.h>
	#include <general_common.h>

	nmvfns = 99;
	notmv = "15,20";

	if (mode == "ANYPOSTINGS") {

		journalcode = win.is;
		gosub anypostings();

	} else if (mode == "PREREAD") {

	} else if (mode == "READ") {
		if (not(RECORD.read(DEFINITIONS, "ALL"))) {
			return 0;
		}

	} else if (mode == "POSTREAD") {
		if (not(authorised("JOURNAL SETUP", msg, ""))) {
			win.srcfile.unlock( ID);
			win.wlocked = 0;
		}

		//remove unauthorised journal types if only reading
		if (not(win.wlocked)) {
			//WARNING TODO: check ternary op following;
			var prefix = authorised("JOURNAL ACCESS") ? "" : "#";
			var vtypes = RECORD.a(6);
			var nvtypes = vtypes.count(VM) + 1;
			for (var vtypen = nvtypes; vtypen >= 1; --vtypen) {
				var vtype = vtypes.a(1, vtypen);
				if (not(authorised(prefix ^ "JOURNAL ACCESS " ^ (vtype.quote())))) {
					win.mvx = vtypen;
					gosub deletemv();
				}
			};//vtypen;

			if (not(RECORD.a(6))) {
				msg = "You are not authorised to access any journal types";
				gosub invalid(msg);
				return 0;
			}

		}

	} else if (mode == "PREWRITE") {
		if (not(authorised("JOURNAL SETUP", msg, ""))) {
			gosub invalid(msg);
			return 0;
		}

		var editing = 0;
		gosub defaultcolwidths();

	} else if (mode == "PREDELETE") {
		msg = "Journal setup cannot be deleted";
		gosub invalid(msg);
		return 0;

	} else if (mode == "WRITE") {

		if (not(authorised("JOURNAL SETUP", msg, ""))) {
			gosub invalid(msg);
			return 0;
		}

		var editing = 0;
		gosub defaultcolwidths();

		if (RECORD) {
			RECORD.write(DEFINITIONS, "ALL");
		}

		fin.definition = RECORD;
		var nvs = fin.definition.a(1).count(VM) + 1;
		for (var vn = 1; vn <= nvs; ++vn) {
			var temp = fin.definition.a(1, vn);
			if (temp and (fin.definition.a(6, vn) == "")) {
				fin.definition.r(6, vn, temp);
			}
		};//vn;

	} else if (mode == "POSTWRITE") {

	} else if (mode == "POSTDELETE") {

	} else if (mode == "PREWRITE") {

	} else if (mode == "PREDELETE") {
		msg = "Journal Setup cannot be deleted";
		gosub invalid(msg);
		return 0;

	} else if (mode == "ANYPOSTINGS") {

	} else if (mode.substr(1,11) == "DEF.COLUMNS") {

		if (not(RECORD.read(DEFINITIONS, "ALL"))) {
			return 0;
		}
		if (RECORD.a(21)) {
			return 0;
		}

		if (not(decide("Are you (or will you be)|registered for VAT ?", "", reply))) {
			reply = 2;
		}
		RECORD.r(21, VM);
		vatrequired = reply == 1;
		if (vatrequired and (gen.company.a(19) == "")) {
			msg = "NOTE:|";
			msg.r(-1, "TO GET VAT COLUMNS ON THE DAYBOOKS|");
			msg.r(-1, "1) SET UP A VAT CONTROL ACCOUNT   ");
			msg.r(-1, "   (IN THE CURRENT LIABILITIES)   |");
			msg.r(-1, "2) ENTER THE VAT ACCOUNT NUMBER ON");
			msg.r(-1, "   THE COMPANY FILE               |");
			call note(msg);
		}
		var ndaybooks = RECORD.a(1).count(VM) + 1;
		for (daybookn = 1; daybookn <= ndaybooks; ++daybookn) {
			daybooktype = RECORD.a(1, daybookn);
			gosub defaultcolumns();
		};//daybookn;
		RECORD.write(DEFINITIONS, "ALL");

	} else {
		msg = mode.quote() ^ " invalid mode in VOUCHER.TYPE.SUBS";
		gosub invalid(msg);
		return 0;
	}

	return 0;
}

subroutine anypostings() {

	var compcodes;
	if (not(compcodes.read(gen.companies, "%RECORDS%"))) {
		msg = "Please QUICKDEX the COMPANIES file first";
		gosub invalid(msg);
		return;
	}

	for (var ii = 1; ii <= 999; ++ii) {
		var compcode = compcodes.a(ii);
		///BREAK;
		if (not compcode) break;

		//check batches
		var temp;
		if (temp.read(fin.batches, "%" ^ compcode ^ "*" ^ journalcode ^ "%")) {
			if (temp.a(1) > 1) {
				msg = "Journal " ^ (journalcode.quote()) ^ " has entries for company " ^ (compcode.quote());
				gosub invalid(msg);
				return;
			}
		}

		//check unposted batches
		//read temp from batches,'%':compcode:'*':journalcode:'%' then
		// if temp<1> gt 1 then
		//  msg='Journal ':quote(journalcode):' has entries for company ':quote(compcode)
		//  gosub invalid
		//  return
		//  end
		// end

		//check vouchers to be sure (only works for autonumbered journals)
		if (temp.read(fin.vouchers, "%" ^ journalcode ^ "*" ^ compcode ^ "%")) {
			msg = "Journal " ^ (journalcode.quote()) ^ " has entries for company " ^ (compcode.quote());
			gosub invalid(msg);
			return;
		}

	};//ii;

	return;
}

subroutine deletemv() {
	for (fn = 1; fn <= nmvfns; ++fn) {
		//if fn ne 20 and fn ne 15 then
		if (not(notmv.locateusing(",",fn,xx))) {
			if (RECORD.a(fn)) {
				RECORD.remover(fn, win.mvx);
			}
		}
	};//fn;
	return;
}

subroutine defaultcolumns() {

	//try to copy setup of existing similar journal
	tt = RECORD.a(1);
	tt.r(1, daybookn, "");
	if (tt.a(1).locate(daybooktype,copydaybookn)) {
		for (fn = 1; fn <= nmvfns - 20; ++fn) {
			tt = RECORD.a(20 + fn, copydaybookn);
			gosub addtitle();
		};//fn;
		return;
	}

	if ((((daybooktype == "SI") or (daybooktype == "CN")) or (daybooktype == "PI")) or (daybooktype == "PC")) {
		if ((daybooktype == "SI") or (daybooktype == "CN")) {
			if (daybooktype == "SI") {
				tt = "Inv.No\\10";
			}else{
				tt = "Cred.No\\10";
			}
			fn = 1;
			gosub addtitle();
			fn = 10;
			tt = "Ref.";
			gosub addtitle();
			fn = 7;
			tt = "Client A/c";
			gosub addtitle();
		}else{
			fn = 1;
			tt = "Vch.No";
			gosub addtitle();
			if (daybooktype == "PI") {
				tt = "Inv.No";
			}else{
				tt = "Cred.No";
			}
			fn = 10;
			gosub addtitle();
			fn = 7;
			tt = "Suppl.A/c";
			gosub addtitle();
		}
		fn = 3;
		tt = "Details";
		gosub addtitle();
		fn = 5;
		tt = "Tot.Amt";
		gosub addtitle();
		fn = 6;
		tt = "Base Amt";
		gosub addtitle();
		fn = 37;
		tt = "Due Date";
		gosub addtitle();
		if ((daybooktype == "SI") or (daybooktype == "CN")) {
			fn = 19;
			tt = "Sales A/c";
			gosub addtitle();
			if (not vatrequired) {
				fn = 20;
				tt = "Sales A/c Name";
				gosub addtitle();
			}
		}else{
			fn = 19;
			tt = "Purch A/c";
			gosub addtitle();
			if (not vatrequired) {
				fn = 20;
				tt = "Purch A/c Name";
				gosub addtitle();
			}
		}
		if (vatrequired) {
			tt = "Net Amt";
		}else{
			tt = "Amount";
		}
		fn = 17;
		gosub addtitle();

		//tax columns
		if (vatrequired) {
	//  FN=18;T='Base Amt'     ;GOSUB ADDTITLE
			fn = 13;
			tt = "C";
			gosub addtitle();
			fn = 14;
			tt = "VAT Amt";
			gosub addtitle();
		}

		fn = 15;
		tt = "Job No";
		gosub addtitle();

	} else if (daybooktype == "SIC") {
		fn = 1;
		tt = "Inv.No";
		gosub addtitle();
		fn = 10;
		tt = "Ref.";
		gosub addtitle();
		fn = 7;
		tt = "Cost A/c";
		gosub addtitle();
		fn = 3;
		tt = "Details";
		gosub addtitle();
		fn = 5;
		tt = "Tot.Cost";
		gosub addtitle();
		fn = 19;
		tt = "Stock A/c";
		gosub addtitle();
		fn = 20;
		tt = "Stock A/c Name";
		gosub addtitle();
		fn = 17;
		tt = "Qty/Amt";
		gosub addtitle();
		fn = 17;
		tt = "Cost";
		gosub addtitle();
		fn = 15;
		tt = "Job No";
		gosub addtitle();

	} else if ((daybooktype == "RE") or (daybooktype == "PA")) {
		fn = 7;
		tt = "Bank/Cash A/c:";
		gosub addtitle();
		fn = 8;
		tt = "-";
		gosub addtitle();
		fn = 1;
		tt = "Vch.No";
		gosub addtitle();
		fn = 10;
		tt = "Ref.";
		gosub addtitle();
		fn = 5;
		tt = "Tot.Amt";
		gosub addtitle();
		fn = 6;
		tt = "Base Amt";
		gosub addtitle();
		fn = 19;
		tt = "A/c No";
		gosub addtitle();
		fn = 9;
		tt = "Details";
		gosub addtitle();
		if (vatrequired) {
			tt = "Net Amt";
		}else{
			tt = "Amount";
		}
		fn = 17;
		gosub addtitle();

		//tax columns
		if (vatrequired) {
		// FN=18;T='Base Amt'     ;GOSUB ADDTITLE
			fn = 13;
			tt = "C";
			gosub addtitle();
			fn = 14;
			tt = "VAT Amt";
			gosub addtitle();
		}

		//allocations
		fn = 21;
		tt = "Inv(s)Paid";
		gosub addtitle();
		fn = 22;
		tt = "Amt(s)Paid";
		gosub addtitle();

		fn = 15;
		tt = "Job No";
		gosub addtitle();

	} else if (daybooktype == "PP") {
		fn = 7;
		tt = "Cash A/c";
		gosub addtitle();
		fn = 8;
		tt = "-";
		gosub addtitle();
		fn = 1;
		tt = "Vch.No";
		gosub addtitle();
		fn = 10;
		tt = "Ref.";
		gosub addtitle();
		fn = 19;
		tt = "A/c No";
		gosub addtitle();
		fn = 9;
		tt = "Details";
		gosub addtitle();
		if (vatrequired) {
			tt = "Net Amt";
		}else{
			tt = "Amount";
		}
		fn = 17;
		gosub addtitle();
		//tax columns
		if (vatrequired) {
			fn = 13;
			tt = "C";
			gosub addtitle();
			fn = 14;
			tt = "VAT Amt";
			gosub addtitle();
		}
		fn = 15;
		tt = "Job No";
		gosub addtitle();

	} else if ((daybooktype == "JO") or (daybooktype == "RV")) {
		fn = 1;
		tt = "Journal.No";
		gosub addtitle();
		fn = 10;
		tt = "Ref.";
		gosub addtitle();
		fn = 19;
		tt = "A/c No";
		gosub addtitle();
		fn = 9;
		tt = "Details";
		gosub addtitle();
		fn = 17;
		tt = "Amount";
		gosub addtitle();
		fn = 18;
		tt = "Base Amt";
		gosub addtitle();
		if (daybooktype == "JO") {
			//tax columns
			if (vatrequired) {
				fn = 13;
				tt = "C";
				gosub addtitle();
				fn = 14;
				tt = "VAT Amt";
				gosub addtitle();
			}
			fn = 15;
			tt = "Job No";
			gosub addtitle();
		}

	} else if (daybooktype == "TR") {
		fn = 1;
		tt = "Transfer No";
		gosub addtitle();
		fn = 10;
		tt = "Ref.";
		gosub addtitle();
		fn = 3;
		tt = "Details";
		gosub addtitle();
		fn = 19;
		tt = "A/c No";
		gosub addtitle();
		fn = 17;
		tt = "Amount";
		gosub addtitle();
		fn = 18;
		tt = "Base Amt";
		gosub addtitle();
		//allocations
		fn = 21;
		tt = "Alloc Vchs";
		gosub addtitle();
		fn = 22;
		tt = "Alloc Amts";
		gosub addtitle();
		fn = 15;
		tt = "Job No";
		gosub addtitle();

	} else if (daybooktype == "OB") {
		fn = 1;
		tt = "O/B No";
		gosub addtitle();
		fn = 19;
		tt = "A/c No";
		gosub addtitle();
		fn = 20;
		tt = "A/c Name";
		gosub addtitle();
		fn = 17;
		tt = "Amount";
		gosub addtitle();
		fn = 18;
		tt = "Base Amt";
		gosub addtitle();

	} else if (daybooktype.match("^ST\\d$")) {
		fn = 1;
		tt = "Vch.No";
		gosub addtitle();
		fn = 10;
		tt = "Ref.";
		gosub addtitle();
		fn = 3;
		tt = "Details";
		gosub addtitle();
		fn = 19;
		tt = "Stock code";
		gosub addtitle();
		fn = 20;
		tt = "Item name";
		gosub addtitle();
		fn = 17;
		tt = "Quantity";
		gosub addtitle();
		fn = 18;
		tt = "Amount";
		gosub addtitle();

	}

	return;
}

subroutine addtitle() {
	RECORD.r(20 + fn, daybookn, tt);
	return;
}

subroutine defaultcolwidths() {

	PSEUDO = RECORD.a(20);
	PSEUDO.converter(VM, FM);
	PSEUDO.splicer(1, 0, FM.str(20));
	PSEUDO.r(15, RECORD.a(15));

	//also in print daybook setup
	var temp = "";
	temp.r(-1, "6" _VM_ "21" _VM_ "Voucher Number");
	temp.r(-1, "6" _VM_ "30" _VM_ "Reference");
	temp.r(-1, "10" _VM_ "27" _VM_ "Main Account");
	temp.r(-1, "19" _VM_ "28" _VM_ "Main A/c Name");
	temp.r(-1, "19" _VM_ "23" _VM_ "Main Details");
	temp.r(-1, "15" _VM_ "25" _VM_ "Main Amount");
	temp.r(-1, "10" _VM_ "26" _VM_ "Main Base Amt");
	temp.r(-1, "9" _VM_ "57" _VM_ "Due Date");
	temp.r(-1, "10" _VM_ "39" _VM_ "Contra Account");
	temp.r(-1, "19" _VM_ "40" _VM_ "Contra A/c Name");
	temp.r(-1, "19" _VM_ "29" _VM_ "Contra Details");
	temp.r(-1, "15" _VM_ "37" _VM_ "Contra Amount");
	temp.r(-1, "10" _VM_ "38" _VM_ "Contra Base Amt");
	temp.r(-1, "4" _VM_ "33" _VM_ "Tax Code");
	temp.r(-1, "10" _VM_ "34" _VM_ "Tax Amount");
	temp.r(-1, "10" _VM_ "41" _VM_ "Alloc.Vouchers");
	temp.r(-1, "10" _VM_ "42" _VM_ "Alloc.Amounts");
	temp.r(-1, "10" _VM_ "35" _VM_ "Job No");
	var ncols = temp.count(FM) + 1;
	for (var coln = 1; coln <= ncols; ++coln) {
		fn = temp.a(coln, 2);
		if (PSEUDO.a(fn) == "") {
			PSEUDO.r(fn, temp.a(coln, 1));
		}
	};//coln;
	if (PSEUDO) {
		PSEUDO.converter(FM, VM);
		RECORD.r(20, PSEUDO.field(VM, 21, 9999));
		RECORD.r(15, PSEUDO.a(1, 15));
	}

	return;
}

libraryexit()
