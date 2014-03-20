#include <exodus/library.h>
libraryinit()

#include <agencysubs.h>
#include <generalsubs.h>
#include <calcbill.h>
#include <singular.h>

#include <agy.h>
#include <gen.h>
#include <win.h>

#include <window.hpp>//after win

var msg;
var reply;//num
var tt;//num
var xx;
var clientcode;
var client;
var vcode;
var rcard;
var vehicle;
var t2;
var brandcode;
var brandn;
var discs;
var commfn;//num
var commng;
var feefn;//num
var tax;//num
var taxng;
var taxg;
var taxfn;//num
var prepfn;//num
var costrcard;
var agtfee;
var agtfeeng;
var lnx;//num
var rown;//num
var spec1;
var spec2;
var rate;//num
var costrown;
var costrate;//num
var discfn;//num
var rdates;
var daten;//num
var cubtemp;
var unused;
var rndg;
var multx;
var breakdown;
var currn;
var vn;
var wsmsg;
var silent;//num

var cgunitbillfn;
var cextrasfn;
var cunitbillfn;
var cunitbill2fn;

var mode;

function main(in modex) {
	mode=modex;

	//fix a bug without finding it
	if (not (RECORD.a(1)).isnum()) {
		RECORD.r(1, RECORD.a(1, 1, 1));
	}

	//actually 40 can be entered in the intranet system
	//but difficult to display so many in dos
	var nratecardcols = 20;

	if (mode == "CALCUBILL") {
		gosub calcubill();

	} else if (mode == "CALCUCOST") {
		gosub calcucost();

	} else if (mode == "RECALC_ALL_NETS") {
		var oldmv = win.mvx;

		var nmvs = (RECORD.a(20)).count(VM) + 1;
		for (win.mvx = 1; win.mvx <= nmvs; ++win.mvx) {
			gosub calcubill();
			gosub calcucost();
		};//win.mvx;

		win.mvx = oldmv;

	} else {
		msg = mode.quote() ^ " - INVALID MODE IGNORED";
		return invalid(msg);

	}

	return 0;

}

subroutine calcucost() {
	cgunitbillfn = 41;
	cextrasfn = 139;
	cunitbillfn = 42;
	cunitbill2fn = 63;

	//get original net unit cost
	nuc = RECORD.a(42, win.mvx);

	gosub calcubill2();

	//move down
	//fmt='MD':len(field(nuc,'.',2)):'0P'
	//if multx and nuc then nuc=(nuc/multx) fmt
	/*;
		//feedback net unit cost/size into gross unit bill
		if index(ucase(company),'Promopub',1) then;
			fmtx='MD':len(field(nuc,'.',2)):'0P';
			if multx and nuc then nuc=(nuc/multx) fmtx;
			if r<gunit.bill.fn,mv>=nuc and nuc<>'' then;
				//if multx then r<gunit.bill.fn,mv>=(r<42,mv>/multx) fmtx
				if multx then r<gunit.bill.fn,mv>=(nuc) fmtx;
				gosub calcubill;
				end;
			end;
	*/
	return;

}

subroutine calcubill() {
	cgunitbillfn = 37;
	cextrasfn = 140;
	cunitbillfn = 38;
	cunitbill2fn = 62;
}

var nuc;
var gunitbill;

subroutine calcubill2() {
	gunitbill = RECORD.a(cgunitbillfn, win.mvx);
	if (mode == "VAL.SPEC") {
		cubtemp = win.is;
	}else{
		cubtemp = RECORD.a(21, win.mvx);
	}
	var size = cubtemp.field("*", 2).field(",", 1);
	var extras = RECORD.a(cextrasfn, win.mvx);

	//do the calculation
	var unitbill = calcbill(gunitbill, size, extras, unused, msg, rndg, multx, breakdown);
	if (msg) {
		call note("IN LINE: " ^ win.mvx ^ "|" ^ msg);
	}

	//allow for manual entry of net unit bill alone
	if (not unitbill and not gunitbill) {
		unitbill = RECORD.a(cunitbillfn, win.mvx);
		//breakdown=''
		breakdown = unitbill;
	}

	//prevent negative result
	if (unitbill and unitbill < 0) {
		gunitbill = 0;
		unitbill = 0;
		breakdown = "";
		msg = "NET UNIT BILL CANNOT BE NEGATIVE - ZERO USED|(IN LINE " ^ win.mvx ^ ")";
		gosub note(msg);
	}

	//update net unit bill and redisplay
	RECORD.r(cunitbillfn, win.mvx, unitbill);
	if (mode ne "RECALC_ALL_NETS") {
		tt = win.registerx(1).a(cunitbillfn);
		//gosub r();
	}
	// IF cUNIT.BILL.FN=38 THEN T=TOT.BILL.WI ELSE T=TOT.cost.wi
	// gosub r

	//update breakdown
	breakdown.converter(FM, SVM);
	tt = gunitbill ^ SVM ^ breakdown;
	//call trimexcessmarks(t)
	//loop while t[-1,1]=sm do t[-1,1]='' repeat
	tt.trimmerb(SVM);
	RECORD.r(cunitbill2fn, win.mvx, tt);

	return;

}

subroutine getexchrates() {
	var vcodes = RECORD.a(20);
	var nvehicles = vcodes.count(VM) + 1;
	var storemv = win.mvx;
	for (win.mvx = 1; win.mvx <= nvehicles; ++win.mvx) {
		vcode = vcodes.a(1, win.mvx);
		if (vcode) {
			gosub getexchrate();
		}
	};//win.mvx;
	win.mvx = storemv;
	return;

}

subroutine getexchrate() {
	//get the vehicle currency, from the vehicle file if necessary
	var vehiclecurrcode = RECORD.a(45, win.mvx);
	if (not vehiclecurrcode) {
		if (not(vehicle.read(agy.vehicles, vcode))) {
			return;
		}
		vehiclecurrcode = vehicle.a(5);
		RECORD.r(45, win.mvx, vehiclecurrcode);
		tt = win.registerx(1).a(45);
		//gosub r();
	}

	//use existing rate if possible
	tt = RECORD.a(45).erase(1, win.mvx, 0);
	if (tt.locate(vehiclecurrcode, tt, 1)) {
		rate = RECORD.a(40).erase(1, win.mvx, 0).a(1, tt);
		costrate = RECORD.a(44).erase(1, win.mvx, 0).a(1, tt);
		if (not(rate and costrate)) {
			goto getrate;
		}

		//get rate from currency file
	}else{
getrate:
		rate = "";
		costrate = "";

		//get conversion to base
		if (gen.currency.read(gen.currencies, vehiclecurrcode)) {
			var datex = var("1/" ^ RECORD.a(12)).iconv("D/E");
			if (not(gen.currency.locateby(datex, "DR", tt, 4))) {
				{}
			}
			rate = gen.currency.a(5, tt);
			if (not rate) {
				rate = gen.currency.a(5, tt - 1);
			}
			if (not rate) {
				msg = "NOTE:|" ^ (DQ ^ (vehiclecurrcode ^ DQ)) ^ " - NO EXCHANGE RATE|(YOU WILL HAVE TO ENTER IT)";
				gosub note(msg);
			}
			costrate = rate;
		}else{
			msg = DQ ^ (vehiclecurrcode ^ DQ) ^ " - CURRENCY NOT ON FILE";
			gosub note(msg);
			rate = "";
			costrate = rate;
		}

		//if invoice currency not base currency then
		//convert base rate (but not cost rate) to be
		//rate from vehicle currency to invoice currency
		var invcurrcode = calculate("CURRENCY_CODE");
		if (invcurrcode ne agy.agp.a(2)) {
			if (vehiclecurrcode == invcurrcode) {
				rate = 1;
			}else{
				if (gen.currency.read(gen.currencies, invcurrcode)) {
					var datex = var("1/" ^ RECORD.a(12)).iconv("D/E");
					if (not(gen.currency.locateby(datex, "DR", tt, 4))) {
						{}
					}
					var backrate = gen.currency.a(5, tt);
					if (not backrate) {
						backrate = gen.currency.a(5, tt - 1);
					}
					if (backrate) {
						rate = ((rate / backrate).oconv("MD60P")) + 0;
					}else{
						msg = "NOTE:|" ^ (DQ ^ (invcurrcode ^ DQ)) ^ " - NO EXCHANGE RATE|(YOU MUST ENTER IT)";
						gosub note(msg);
						rate = "";
					}
				}else{
					msg = DQ ^ (invcurrcode ^ DQ) ^ " - NOT ON FILE";
					gosub note(msg);
					rate = "";
				}
			}
		}

		//look for rate on client file
		var client;
		if (client.read(agy.clients, calculate("BILLING_CLIENT_CODE"))) {
			if (invcurrcode == client.a(11)) {
				if (client.locate(vehiclecurrcode, currn, 13)) {
					rate = client.a(17, currn);
				}
			}
		}

	}
	//zzzcall note(rate)
	RECORD.r(40, win.mvx, rate);
	RECORD.r(44, win.mvx, costrate);
	tt = win.registerx(1).a(40);
	//gosub r();
	tt = win.registerx(1).a(44);
	//gosub r();

	return;

}

libraryexit()
