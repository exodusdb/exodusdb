#include <exodus/library.h>
libraryinit()

#include <btreeextract.h>

#include <agy.h>
#include <gen.h>

var notinteractive;
var basefmtx;
var mode;
var base;//num
var type;
var fn;//num
var tskeys;

function main() {
	//c job
	//called in jobs.subs,postread (and dict.jobs,analyse used in listjobs)
	//updates fn 17-25

	//global mode,basefmtx,notinteractive

	notinteractive = SYSTEM.a(33);

	basefmtx = "MD" ^ USER2[3] ^ "0P";

	mode = "COST";
	var costs = "";

	var ordernos = RECORD.a(4);
	var norders = ordernos.count(VM) + (ordernos ne "");
	for (var ordern = 1; ordern <= norders; ++ordern) {

		var orderno = ordernos.a(1, ordern);
		var prodorder;
		if (prodorder.read(agy.productionorders, orderno)) {

			if (prodorder.a(11) ne "CANCELLED") {
				if (not(prodorder.a(5))) {
					call mssg("Exchange rate is missing on Order No. " ^ orderno);
					var().stop();
				}
				//garbagecollect;
				base = (prodorder.a(3).sum() / prodorder.a(5)).oconv(basefmtx);
				costs.r(1, ordern, base);

				//decide where it should go
				var typecode = prodorder.a(15);
				var type;
				if (not(type.read(agy.jobtypes, typecode))) {
					type = "";
				}
				base.splicer(1, 0, "-");
				if (base.substr(1,2) == "--") {
					base.splicer(1, 2, "");
				}
				gosub analyse();

			}

		}

	};//ordern;

	RECORD.r(25, costs);

	mode = "INCOME";
	var incomes = "";
	var pendingincomes = "";
	var invoicedincomes = "";

	var quotenos = RECORD.a(10);

	var nquotes = quotenos.count(VM) + (quotenos ne "");

	for (var quoten = 1; quoten <= nquotes; ++quoten) {
		var quoteno = quotenos.a(1, quoten);
		var prodquote;
		if (prodquote.read(agy.productioninvoices, quoteno)) {

			if (prodquote.a(11) ne "CANCELLED") {

				if (not(prodquote.a(5))) {
					call mssg("Exchange rate is missing on Estimate No. " ^ quoteno);
					var().stop();
				}

				//garbagecollect;
				base = (prodquote.a(3).sum() / prodquote.a(5)).oconv(basefmtx);

				incomes.r(1, quoten, base);
				if (prodquote.a(10) == "") {
					pendingincomes.r(1, quoten, base);
				}else{
					invoicedincomes.r(1, quoten, base);
				}

				var nlines = prodquote.a(3).count(VM) + 1;
				var typecode = "";
				for (var ln = 1; ln <= nlines; ++ln) {
					//garbagecollect;
					base = (prodquote.a(3, ln) / prodquote.a(5)).oconv(basefmtx);
					var temp = prodquote.a(17, ln);
					if (temp) {
						typecode = temp;
						var type;
						if (not(type.read(agy.jobtypes, typecode))) {
							type = "";
						}
					}
					if (not typecode) {
						typecode = RECORD.a(3);
						var type;
						if (not(type.read(agy.jobtypes, typecode))) {
							type = "";
						}
					}
					gosub analyse();
				};//ln;

			}

		}
	};//quoten;

	RECORD.r(22, incomes);
	RECORD.r(23, pendingincomes);
	RECORD.r(24, invoicedincomes);

	gosub gettime();

	return 0;

	return "";
}

subroutine analyse() {
	//mode=from cost or income side
	//type record
	//base (amount to be added) costs are negative

	if (not base) {
		return;
	}

	/*;
	rebillable: has cost a/c and cost a/c ne income a/c;
	internal: income a/c eq cost a/c;
	commission: no cost a/c;

	PROFIT_AND_COMMISSION_BASE;
		REBILLABLE_PROFIT_BASE;
			17 REBILLABLE_COST_BASE;
		18 REBILLABLE_INCOME_BASE;
		19 COMMISSION_BASE;
	20 INTERNAL_COST_BASE;
	21 INTERNAL_INCOME_BASE;
	*/

	//default to rebillable cost or income
	if (mode == "COST") {
		fn = 17;
	}else{
		fn = 18;
	}

	if (type.a(5)) {

		//income and no cost account means commission
		if (type.a(6) == "") {
			fn = 19;

			//income and cost account same means studio
		}else{
			if (type.a(5) == type.a(6)) {
				fn = 21;
			}
		}

	}

	//show cost as positive
	if ((fn == 17) or (fn == 20)) {
		base = -base;
	}

	//garbagecollect;
	RECORD.r(fn, (RECORD.a(fn) + base).oconv(basefmtx));

	return;

}

subroutine gettime() {

	//work out timesheet cost

	var timecost = "";
	var btreeparams = "JOB_NO" ^ VM ^ ID ^ FM;

	var dicttimesheets;
	if (not(dicttimesheets.open("DICT.TIMESHEETS", ""))) {
		call fsmsg();
		var().stop();
	}

	if (not notinteractive) {
		SYSTEM.r(33, 1);
	}
	call btreeextract(btreeparams, "TIMESHEETS", dicttimesheets, tskeys);
	if (not notinteractive) {
		SYSTEM.r(33, notinteractive);
	}

	//ZZZ really should find some way to read jobs without getting all ts
	USER4 = "";

	var latesttsdate = "";
	if (tskeys) {

		//ntskeys=count(tskeys,fm)+1
		var ntskeys = tskeys.count(VM) + 1;
		for (var tskeyn = 1; tskeyn <= ntskeys; ++tskeyn) {
			var tskey = tskeys.a(1, tskeyn);

			var tsdate = tskey.field("*", 2);
			if (tsdate > latesttsdate) {
				latesttsdate = tsdate;
			}

			var tsrec;
			if (tsrec.read(gen.timesheets, tskey)) {
				var ntslines = tsrec.a(2).count(VM) + 1;
				if (tsrec.a(1).locateusing(ID, VM, MV)) {
onetsline:
					var hours = tsrec.a(2, MV);
					if (hours) {
						var amount = calculate("AMOUNT", dicttimesheets, tskey, tsrec, MV);
						timecost += amount;
					}
					while (true) {
						MV += 1;
						if (tsrec.a(1, MV) == ID) {
							goto onetsline;
						}
					///BREAK;
					if (not(MV < ntslines)) break;;

					}//loop;
				}
			}
		};//tskeyn;
		//garbagecollect;
	}

	//garbagecollect;
	RECORD.r(55, timecost.oconv(basefmtx));

	if (tskeys.length() and (tskeys.length() < 65000)) {
		RECORD.r(56, latesttsdate);
	}

	return;

}


libraryexit()
