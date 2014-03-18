#include <exodus/library.h>
libraryinit()

#include <agencysubs.h>
#include <getreccount.h>
#include <decide2.h>
#include <generalsubs.h>
#include <evaluatesubs.h>
#include <pop_up.h>
#include <calcbill.h>
#include <singular.h>

#include <agy.h>
#include <gen.h>
#include <win.h>

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

function main() {
	//garbagecollect;
	//y2k2 *jbase

	//fix a bug without finding it
	if (not (RECORD.a(1)).isnum()) {
		RECORD.r(1, RECORD.a(1, 1, 1));
	}

	//actually 40 can be entered in the intranet system
	//but difficult to display so many in dos
	var nratecardcols = 20;

	if (mode == "CALCUBILL") {
		gosub calcubill();

		goto 7083;
	}
	if (mode == "CALCUCOST") {
		gosub calcucost();

		goto 7083;
	}
	if (mode == "VAL.TYPE") {
		if (win.is == win.isorig) {
			return 0;
		}
		gosub chkinv();
		if (not win.valid) {
			return 0;
		}

		call agencysubs("VAL.MEDIAPROD.TYPE.WITHACNO");

		goto 7083;
	}
	if (mode == "VAL.APPROVED") {
		if (win.is == win.isorig) {
			return 0;
		}
		if (win.is and not win.isorig) {
			if (not(authorised("SCHEDULE APPROVAL", msg, ""))) {
				return invalid();
			}
		}
		if (win.is == "") {
			gosub chkinv();
			if (not win.valid) {
				return 0;
			}
		}

	if (mode == "F2.JOB.NO") {
		gosub chkupd();
	}else if (not win.valid) {
			return 0;
		}

		//CALL CATALYST('V',DATAFILE:',JOB_NO')
		call agencysubs("F2.JOB");

		goto 7083;
	}
	if (mode == "VAL.JOB.NO") {
		if (win.is == win.isorig or win.is == "") {
			return 0;
		}

		//only check if there are some jobs
		if (not(getreccount(agy.jobs, "", ""))) {
			return 0;
		}

		call agencysubs("VAL.JOB.OPEN");
		if (not win.valid) {
			return 0;
		}

		var job;
		if (not(job.read(agy.jobs, win.is))) {
			msg = DQ ^ (win.is ^ DQ) ^ " Job does not exist";
			return invalid();
		}
		if (job.a(2) ne var("BRAND_CODE").calculate()) {
			msg = "WRONG JOB?||JOB " ^ (DQ ^ (win.is ^ DQ)) ^ " IS FOR BRAND " ^ (DQ ^ (job.a(2) ^ DQ));
			var().chr(7).output();
			if (not(decide2(msg, "OK" _VM_ "Cancel", reply, 2))) {
				return 0;
			}
			if (reply == 2) {
				win.valid = 0;
				return 0;
			}
		}

		goto 7083;
	}
	if (mode == "VAL.CURRENCY") {
		if (win.is == win.isorig) {
			return 0;
		}
		gosub chkinv();
		if (not win.valid) {
			return 0;
		}

		call generalsubs("VAL.CURRENCY");
		if (not win.valid) {
			return 0;
		}

		if (win.registerx[10] and win.is ne win.registerx[10]) {
			msg = "NOTE: EXPECTED CURRENCY IS " ^ (DQ ^ (win.registerx[10] ^ DQ));
			gosub note();
		}

		//get exchange rates again
		if (win.isorig) {
			RECORD.r(13, win.is);
			RECORD.r(40, "");
			RECORD.r(44, "");
			gosub getexchrates();
		}

		//get ndecs
		var ndecs;
		if (ndecs.readv(gen.currencies, win.is, 3)) {
			if (RECORD.a(1) ne ndecs) {
				RECORD.r(1, ndecs);
				win.displayaction = 5;
				win.reset = 3;
			}
		}

		goto 7083;
	}
	if (mode == "VAL.GUNIT.BILL") {
		if (win.is == win.isorig) {
			return 0;
		}

		gosub chkupd();
		if (not win.valid) {
			return 0;
		}

		if (win.isorig) {
			if (not(decide("Are you sure that you want|to change the gross unit bill ?", "No" _VM_ "Yes", reply))) {
				reply = 1;
			}
			if (reply == 1) {
				win.is = win.isorig;
				return 0;
			}
		}

		if (not(gen.currency.reado(gen.currencies, RECORD.a(45, win.mvx)))) {
			gen.currency = "";
		}
		var ndecs = gen.currency.a(3);
		if (ndecs == "") {
			ndecs = "2";
		}

		win.is = win.is.oconv("MD" ^ ndecs ^ "0P");

		//force net unit cost to zero as well (because calcucost will not do zero)
		if (not win.is) {
			RECORD.r(38, win.mvx, win.is);
		}

		RECORD.r(37, win.mvx, win.is);
		gosub calcubill();

		//change gross unit cost if was same or blank
		tt = RECORD.a(41, win.mvx);
		//if is.orig='' or is.orig=t then
		if (tt == "" or win.isorig == tt) {

			//change gross unit cost
			RECORD.r(41, win.mvx, win.is);
			tt = win.registerx[1].a(41);
			gosub r();

			//change net unit cost as well if zero otherwise
			//net unit cost will not be recalculated in order to allow manual
			//entry of net unit cost without gross unit cost
			if (not win.is) {
				RECORD.r(42, win.mvx, win.is);
				tt = win.registerx[1].a(42);
				gosub r();
			}

			gosub calcucost();
			gosub chkbkg();
		}

		goto 7083;
	}
	if (mode == "VAL.GUNIT.COST") {
		if (win.is == win.isorig) {
			return 0;
		}

		gosub chkupd();
		if (not win.valid) {
			return 0;
		}

		if (win.isorig) {
			if (not(decide("Are you sure that you want|to change the gross unit cost ?", "No" _VM_ "Yes", reply))) {
				reply = 1;
			}
			if (reply == 1) {
				win.is = win.isorig;
				return 0;
			}
		}
		//same precision as gross unit bill
		win.is = win.is.oconv("MD" ^ (RECORD.a(37, win.mvx).field(".", 2)).length() ^ "0P");
		RECORD.r(41, win.mvx, win.is);
		gosub calcucost();
		gosub chkbkg();

		goto 7083;
	}
	if (mode == "VAL.UNIT.BILL") {
		if (win.is == win.isorig) {
			return 0;
		}

		gosub chkupd();
		if (not win.valid) {
			return 0;
		}

		if (win.isorig) {
			if (not(decide("Are you sure that you want|to change the unit bill ?", "No" _VM_ "Yes", reply))) {
				reply = 1;
			}
			if (reply == 1) {
				win.is = win.isorig;
				return 0;
			}
		}
		win.is = win.is.oconv("MD" ^ (RECORD.a(37, win.mvx).field(".", 2)).length() ^ "0P");

		goto 7083;
	}
	if (mode == "VAL.UNIT.COST") {
		if (win.is == win.isorig) {
			return 0;
		}

		gosub chkupd();
		if (not win.valid) {
			return 0;
		}

		if (win.isorig) {
			if (not(decide("Are you sure that you want|to change the unit cost ?", "No" _VM_ "Yes", reply))) {
				reply = 1;
			}
			if (reply == 1) {
				win.is = win.isorig;
				return 0;
			}
		}
		win.is = win.is.oconv("MD" ^ (RECORD.a(37, win.mvx).field(".", 2)).length() ^ "0P");
		gosub chkbkg();
		tt = win.registerx[3].a(2);
		gosub r();
		tt = win.registerx[3].a(1);
		gosub r();

		goto 7083;
	}
	if (mode == "FREEONOFF") {
		if (not win.mvx) {
			msg = "PLEASE GO ONTO THE LINE YOU WANT AND TRY AGAIN";
			return invalid();
		}

		if (win.is ne win.isorig) {
			msg = "PLEASE PRESS ENTER TO COMPLETE|YOUR ENTRY FIRST THEN TRY AGAIN";
			gosub note();
			return 0;
		}

		gosub chkupd();
		if (not win.valid) {
			return 0;
		}

		var extras = RECORD.a(190, win.mvx);
		var costextras = RECORD.a(191, win.mvx);
		if (extras == "") {
			extras = "FREE";
		}
		if (costextras == "") {
			costextras = "FREE";
		}
		RECORD.r(190, win.mvx, RECORD.a(140, win.mvx) ^ " ");
		RECORD.r(191, win.mvx, RECORD.a(139, win.mvx) ^ " ");
		RECORD.r(140, win.mvx, extras.trimb());
		RECORD.r(139, win.mvx, costextras.trimb());
		tt = win.registerx[1].a(140);
		gosub r();
		tt = win.registerx[1].a(139);
		gosub r();
		//locate si<4> in '140ý139'<1> setting x then data ''
		DATA ^= "" ^ MOVEKEYS.a(5) ^ "\r";

		gosub calcubill();
		gosub calcucost();

		gosub chkbkg();

		win.displayaction = 5;

		goto 7083;
	}
	if (mode == "DEF.EXTRAS") {
		//is.dflt=r<190,mv>

		goto 7083;
	}
	if (mode == "DEF.COST.EXTRAS") {
		//is.dflt=r<191,mv>

		goto 7083;
	}
	if (mode == "VAL.EXTRAS") {
		if (win.is == win.isorig) {
			return 0;
		}

		gosub chkupd();
		if (not win.valid) {
			return 0;
		}

		//warning if already invoiced
		if (RECORD.a(46, win.mvx) ne "") {
			msg = "NOTE: Some ads on this line have already been invoiced at the old rate and a credit/debit note will have to be raised if you continue.";
			msg ^= FM ^ "|Please consider splitting this line into two lines, one at the old rate and one at the new rate.|";
			if (not(decide2("!" ^ msg.oconv("T#50"), "OK" _VM_ "Cancel", reply, 2))) {
				reply = 2;
			}
			if (reply == 2) {
				win.valid = 0;
				return 0;
			}
		}

		var isorig2 = win.isorig;

		var lastextra = field2(win.is, ",", -1);

		//offer means zero gross unit bill
		if (lastextra.substr(1, 5) == "OFFER") {
			RECORD.r(37, win.mvx, 0);
			tt = win.registerx[1].a(37);
			gosub r();
		}

		//if no longer an offer, get the gross unit bill again
		if ((field2(win.isorig, ",", -1)).substr(1, 5) == "OFFER" and lastextra.substr(1, 5) ne "OFFER") {
			call plansubs2("VAL.SPEC2");
		}

		RECORD.r(140, win.mvx, win.is);
		gosub calcubill();

		//if use default then use cost default
		//if is and is=r<190,mv> then
		// r<139,mv>=r<191,mv>
		// goto calcucost
		// t=wis<139>;gosub r
		// end

		//various types of free ads
		//copy to the cost to agency side
		if (((lastextra.trim()).substr(1, 1)).match("1A0A")) {
updcostextras:
			var lastextran = win.is.count(",") + (win.is ne "");
			win.isorig = RECORD.a(139, win.mvx);
			win.is = win.isorig.fieldstore(",", lastextran, 1, lastextra);
			gosub valcostextras2();
			if (not win.valid) {
				return 0;
			}
			tt = win.registerx[1].a(139);
			gosub r();
			gosub chkbkg();
		}

		//r<190,mv>=is.orig2

		goto 7083;
	}
	if (mode == "VAL.COST.EXTRAS") {
		if (win.is == win.isorig) {
			return 0;
		}

		gosub chkupd();
		if (not win.valid) {
			return 0;
		}
}

subroutine valcostextras2() {
		var isorig2 = win.isorig;

		RECORD.r(139, win.mvx, win.is);
		gosub calcucost();

		gosub chkbkg();

		//r<191,mv>=is.orig2

		goto 7083;
	}
	if (mode == "VAL.BRAND") {
		if (win.is == "" or win.is == win.isorig) {
			return;
		}

		gosub chkupd();
		if (not win.valid) {
			return;
		}

		//check if allowed to change brand after invoicing
		if (RECORD.a(4)) {
			msg = "!THIS SCHEDULE HAS ALREADY BEEN INVOICED.";
			if (not(authorised("SCHEDULE UPDATE BRAND AFTER INVOICE", xx, ""))) {
				return invalid();
			}
			msg.r(-1, FM ^ "If you change the brand code after invoicing");
			msg.r(-1, "then the Media Analysis by brand and client will");
			msg.r(-1, "will not agree with the Billing Analysis.");
			msg.r(-1, "(The Media Analysis will be by the new brand only)");
			if (not(decide2(msg, "OK" _VM_ "Cancel", reply, 2))) {
				reply = "";
			}
			if (reply ne 1) {
				win.valid = 0;
				return;
			}
		}

		call agencysubs("VAL.BRAND");
		if (not win.valid) {
			return;
		}

		var brand;
		if (not(brand.read(agy.brands, win.is))) {
			msg = DQ ^ (win.is ^ DQ) ^ " - brand code not in brand file";
			return invalid();
		}
		clientcode = brand.a(1, 1);
		var client;
		if (not(client.read(agy.clients, clientcode))) {
			msg = DQ ^ (clientcode ^ DQ) ^ " - client not in client file";
			return invalid();
		}

		//convert to billing group
		if (RECORD.a(188) ne "none") {
			tt = client.a(25);
			if (tt) {
				if (client.read(agy.clients, tt)) {
					clientcode = tt;
				}
			}
		}

		//get default VAT if client.tax.ng is null
		var clienttaxng = client.a(21);
		if (RECORD.a(18) == "" and agy.agp.a(12) ne "" and clienttaxng == "") {
			RECORD.r(18, agy.agp.a(12));
			tt = win.registerx[1].a(18);
			gosub r();
		}

		//default the discount
		if (RECORD.a(17) == "") {
			var agencydiscng = client.a(23);
			if (agencydiscng == "") {
				var agencydisc = client.a(22);
				if (agencydisc) {
					RECORD.r(17, agencydisc);
					tt = win.registerx[1].a(17);
					gosub r();
				}
			}
		}

		goto 7083;
	}
	if (mode == "VAL.VEHICLE.CODE") {
		if (win.is == win.isorig) {
			return;
		}

		gosub chkupd();
		if (not win.valid) {
			return;
		}

		call agencysubs("VAL.VEHICLE");
		if (not win.valid) {
			return;
		}

		//get the VEHICLE
		vcode = win.is;
		rcard = "";

		//set some dates so that output conversion will operate and display columns
		if (RECORD.a(22, win.mvx) == "") {
			RECORD.r(22, win.mvx, " ");
		}

		var updbillext = 1;
		var updcostext = 1;

		var forcedotherfee = "";

		//also called from updateextras
valvehicle:
		var vehicle;
		if (not(vehicle.read(agy.vehicles, vcode))) {
			msg = DQ ^ (vcode ^ DQ) ^ " IS NOT ON VEHICLE FILE";
			return invalid();
		}

		//check market code
		if (mode == "VAL.VEHICLE.CODE") {
			if (var("MARKET_CODE").calculate()) {
				if (vehicle.a(4) ne var("MARKET_CODE").calculate()) {
					msg = "NOTE: THIS VEHICLE IS IN THE " ^ (DQ ^ (vehicle.a(4) ^ DQ)) ^ " MARKET";
					gosub note();
				}
			}
		}

		//check the vehicles currency
		//IF {CURRENCY_CODE} NE BASE.CURRENCY.CODE THEN
		// IF {CURRENCY_CODE} # VEHICLE<5> THEN
		// MSG='THIS VEHICLE USES ':VEHICLE<5>:' CURRENCY'
		// GOTO INVALID
		// END
		// END

		//check that they have not changed the vehicle currency because the invoicing
		//program cannot handle one vehicle with two currencies on one schedule

		if (vehicle.a(5)) {

			tt = "";
			t2 = "";

			//look in "as invoiced" record
			if (RECORD.a(7)) {
				var asinv;
				if (asinv.reado(agy.schedules, ID ^ "*I")) {
					var nn = tt.count(VM) + 2;
					tt.r(1, nn, asinv.a(20));
					t2.r(1, nn, asinv.a(45));
				}
			}

			//also look in orec
			if (win.orec) {
				var nn = tt.count(VM) + 2;
				tt.r(1, nn, win.orec.a(20));
				t2.r(1, nn, win.orec.a(45));
			}

			//look in current record except current line
			var nn = tt.count(VM) + 2;
			var t4 = RECORD.a(20);
			t4.r(1, win.mvx, "");
			tt.r(1, nn, t4);
			t2.r(1, nn, RECORD.a(45));

			if (tt.a(1).locateusing(vcode, VM, xx)) {
				var t3 = t2.a(1, xx);
				if (t3 and vehicle.a(5) ne t3) {
					msg = vehicle.a(1) ^ " ALSO USED " ^ (DQ ^ (t3 ^ DQ)) ^ "|A VEHICLE CAN ONLY USE ONE CURRENCY PER SCHEDULE";
					return invalid();
				}
			}

		}

		//get the client
		brandcode = RECORD.a(11);

		clientcode = var("BILLING_CLIENT_CODE").calculate();
		var client;
		if (not(client.read(agy.clients, clientcode))) {
			msg = DQ ^ (clientcode ^ DQ) ^ " - MISSING FROM CLIENT FILE";
			return invalid();
		}

		//update the currency code
		RECORD.r(45, win.mvx, vehicle.a(5));
		tt = win.registerx[1].a(45);
		gosub r();

		//clear the rate card dates
		//14/7/97
		//removed 2000/10/12 so that FINDPROG can select many programs
		//if mode='VAL.VEHICLE.CODE' then
		// R<24,MV>=''
		// R<27,MV>=''
		// end

		gosub getexchrate();

		//check if any rate cards exist
		if (vehicle.a(8) == "") {
			msg = "NOTE: NO RATE CARDS HAVE BEEN ENTERED FOR THIS VEHICLE|YOU WILL HAVE TO ENTER THE COSTS YOURSELF";
			gosub note();
			goto vvexit;
		}

		if (not rcard) {
			gosub getratecard();
		}

		if (not(client.a(2).locateusing(brandcode, VM, brandn))) {
			{}
		}
		var agencyfee = "";

		//discount brackets
		//also client commission
		//locate brand code if possible in vehicle rates
		//otherwise locate client code and blank brand code
		clientcode = var("BILLING_CLIENT_CODE").calculate();
		if (not(client.read(agy.clients, clientcode))) {
			{}
		}

		gosub getcommfee();

		//save the standard discounts
		var costdiscs = discs;

		//bills
		///////

		//client may not receive the discounts
		if (client.a(4) == "N") {
			discs = "";
		}

		//if standard fee for the client then deduct commission and add fee
		//13/8/97 fee could be "0" which is different from null
		//we subtract the commission but add no fee (client pays a monthly retainer)
		if (agencyfee == "") {
			agencyfee = client.a(5);
		}
		var agencyfeeng = client.a(12);
		if (agencyfee ne "") {
			if (agencyfee.substr(-3, 3) == ".00") {
				agencyfee.splicer(-3, 3, "");
			}

			//less commission
			//IF comm<>'' THEN
			if (win.registerx[6]) {
				tt = "-" ^ win.registerx[6];
				if (tt.substr(1, 2) == "--") {
					tt.splicer(1, 2, "");
				}
				discs.r(commfn, tt ^ "%" ^ commng);
			}

			//plus agency fee
			if (agencyfee) {
				tt = agencyfee ^ "%";
				if (not(var("+-").index(tt.substr(1, 1), 1))) {
					tt.splicer(1, 0, "+");
				}
				if (var("Gg").index(agencyfeeng, 1)) {
					tt ^= agencyfeeng;
				}
				discs.r(feefn, tt);
			}
		}

		//agency discount (agency gives to the client)
		var agencydiscng = client.a(23);
		if (agencydiscng ne "") {
			var agencydisc = client.a(22);
			if (agencydisc) {
				tt = agencydisc;
				if (not(var("+-").index(tt.substr(1, 1), 1))) {
					tt.splicer(1, 0, "-");
				}
				agencydiscng.converter("N", "");
				discs.r(feefn, tt ^ "%" ^ agencydiscng);
			}
		}

		//plus tax (either the standard tax or the vehicle tax)
		var clienttaxng = client.a(21);
		if (clienttaxng ne "" and agy.agp.a(12)) {
			tax = agy.agp.a(12);
			taxng = clienttaxng;
			win.registerx[6] = rcard.a(1);
			if (win.registerx[6]) {
				tax = (((100 - win.registerx[6]) / 100 * tax).oconv("MD40P")) + 0;
			}
		}else{
			tax = rcard.a(2);
			taxng = taxg;
		}
		if (tax) {
			if (not(var("+-").index(tax.substr(1, 1), 1))) {
				tax.splicer(1, 0, "+");
			}
			taxng.converter("N", "");
			discs.r(taxfn, tax ^ "%" ^ taxng);
		}

		//plus prepayment fee
		var otherfee = rcard.a(3);
		//prep.FEE=CLIENT<6>
		//prepayment fee is now obsolete and is now used as a numeric additional fee
		//IF prep.req='Y' AND prep.FEE THEN
		// DISCS<PREPfn>='+':prep.FEE:'%G'
		// END
		t2 = otherfee;
		t2.converter("%GNgn", "");
		if (t2 and t2.isnum()) {
			var tt!!! = otherfee;
			if ((tt!!!).substr(1, 1) ne "-") {
				(tt!!!).splicer(1, 0, "+");
			}
			discs.r(prepfn, tt!!!);
		}

		//update the discounts and charges
		if (updbillext) {
			discs.converter(FM, ",");
			RECORD.r(140, win.mvx, discs);
			tt = win.registerx[1].a(140);
			gosub r();
			gosub calcubill();
		}

		//costs
		//////

		//special disc for agency
		//14/8/97 if costrcard<11> then costdiscs<discfn>='-':costrcard<11>:'%'
		if ((costrcard.a(11)).length()) {
			tt = "-" ^ costrcard.a(11);
			if (tt.substr(1, 2) == "--") {
				tt.splicer(1, 2, "");
			}
			if (tt) {
				costdiscs = tt ^ "%";
			}else{
				costdiscs = "";
			}
		}

		//less cost commission or else commission
		var costcomm = costrcard.a(16);
		var costcommng = costrcard.a(6);

		if (costcomm == "") {
			costcomm = costrcard.a(1);
		}
		if (costcomm < win.registerx[6]) {
			costcomm = win.registerx[6];
		}

		if (costcomm) {
			tt = "-" ^ costcomm ^ "%";
			if (tt.substr(1, 2) == "--") {
				tt.splicer(1, 2, "");
			}
			if (var("Gg").index(costcommng, 1)) {
				tt ^= costcommng;
			}
			costdiscs.r(commfn, tt);
		}

		//plus agent fee zzz or look up in page 3
		if (agtfee == "") {
			agtfee = costrcard.a(14);
			agtfeeng = costrcard.a(15);
		}
		if (agtfee) {
			if (not(var("-+").index(agtfee.substr(1, 1), 1))) {
				agtfee.splicer(1, 0, "+");
			}
			tt = agtfee ^ "%";
			if (var("Gg").index(agtfeeng, 1)) {
				tt ^= agtfeeng;
			}
			costdiscs.r(feefn, tt);
		}

		//plus tax
		if (clienttaxng ne "N") {
			var costtax = costrcard.a(17);
			if (costtax == "") {
				costtax = costrcard.a(2);
			}
			//if costtax then costdiscs<taxfn>='+':costtax:'%':client.tax.ng
			if (costtax) {
				costdiscs.r(taxfn, "+" ^ costtax ^ "%" ^ taxng);
			}
		}

		//plus other fees
		if (otherfee and otherfee.isnum()) {
			costdiscs.r(prepfn, "+" ^ otherfee);
		}

		//update the discounts & charges
		if (updcostext) {
			costdiscs.converter(FM, ",");
			RECORD.r(139, win.mvx, costdiscs);
			tt = win.registerx[1].a(139);
			gosub r();
			gosub calcucost();
			gosub chkbkg();
		}

		//clear reconciliations if vehicle changed
		//IF vcode NE R<20,MV> THEN
		// R<68,MV>=''
		// R<67,MV>=''
		// GOSUB chkbkg
		// END

vvexit:

		//get the media weight
		var wgt = vehicle.a(12);
		if (not wgt) {
			wgt = 1;
		}
		RECORD.r(138, win.mvx, wgt);
		tt = win.registerx[1].a(138);
		gosub r();

		if (mode == "VAL.VEHICLE.CODE") {
			RECORD.r(win.si.a(4), win.mvx, win.is);
			call evaluatesubs("EVALUATE LINE");
		}

		return;

	// *reprocess the spec
	// SPEC=R<21,MV>
	// IF SPEC THEN GOSUB VAL.SPEC

		goto 7083;
	}
	if (mode == "F2.SPEC") {
		gosub chkupd();
		if (not win.valid) {
			return;
		}

		vcode = RECORD.a(20, win.mvx);
		var vehicle;
		if (not(vehicle.read(agy.vehicles, vcode))) {
			msg = DQ ^ (vcode ^ DQ) ^ " IS NOT ON VEHICLE FILE";
			return invalid();
		}
		gosub getratecard();

		//degressive rate
		//locate brand code if possible in vehicle rates
		//otherwise locate client code and blank brand code
		var degrrate = "";
		var nlines = (rcard.a(7)).count(VM) + 1;
		clientcode = var("BILLING_CLIENT_CODE").calculate();

		for (lnx = 1; lnx <= nlines; ++lnx) {
			if (rcard.a(7, lnx) == clientcode) {
				tt = rcard.a(10, lnx);
				if (not degrrate) {
					degrrate = tt;
				}
				if (rcard.a(8, lnx) == "" or rcard.a(8, lnx) == RECORD.a(11)) {
					if (tt) {
						degrrate = tt;
					}
				}
			}
		};//lnx;

		//extract the row and column titles
		var rowtitles = rcard.a(20);
		var nrows = rowtitles.count(VM) + 1;
		var coltitles = rcard.field(FM, 81, nratecardcols);
		//loop while coltitles[-1,1]=fm do coltitles[-1,1]='' repeat
		coltitles = trim2(coltitles, FM, "B");
		var ncols = coltitles.count(FM) + 1;

		//select the row
selrow:
		var params = "1:20:L::Description";
		for (var coln = 1; coln <= ncols; ++coln) {
			//PARAMS:='\':COLN+1:':6:R::':rcard<80+COLN>
			params ^= "\\" ^ coln + 1 ^ ":6:R::" ^ coltitles.a(coln);
		};//coln;
		var lines = "";
		for (rown = 1; rown <= nrows; ++rown) {
			var line = rowtitles.a(1, rown);
			for (var coln = 1; coln <= ncols; ++coln) {
				line.r(1, coln + 1, rcard.a(20 + coln, rown));
			};//coln;
			lines ^= FM ^ line;
		};//rown;
		lines.splicer(1, 1, "");

		rown = "1" ^ FM;
		if (rowtitles.ucase().a(1).locateusing(var("CLIENT_NAME").calculate().ucase(), VM, xx)) {
			rown ^= xx;
		}

		var qq = "Which line do you want ?";
		//ROWN=POP.UP(0,0,'',LINES,PARAMS,'R','',Q,'','','','P')
		rown = pop_up(0, 0, "", lines, params, "R", "", qq, "", "", rown, "P");
		if (not rown) {
			return;
		}
		var rowtitle = rowtitles.a(1, rown);

		//select the column
		var coltitle = "";
		if (coltitles.count(FM)) {
			params = "1:20:L::Description\\2:10:R::Rate";
			lines = "";
			for (var coln = 1; coln <= ncols; ++coln) {
				lines ^= FM ^ coltitles.a(coln) ^ VM ^ rcard.a(20 + coln, rown);
			};//coln;
			lines.splicer(1, 1, "");
			qq = "Which column do you want ?";
			var default = degrrate;
			if (default == "") {
				default = "30\"";
			}
			if (not(rcard.field(FM, 81, nratecardcols).locateusing(default, FM, default))) {
				default = "";
			}
			coln = pop_up(0, 0, "", lines, params, "R", "", qq, "", "", "" ^ FM ^ default, "P");
			if (not coln) {
				goto selrow;
			}
			coltitle = coltitles.a(coln);
	/*;
				//loading option
				if index(coltitle,'|',1) then;
					coltitle=decide('',coltitle,reply);
					if coltitle else goto selrow;
					end;
	*/

		}

		//get the size
		var size = "";
		if (rowtitle.substr(-1, 1) == "*") {
inpsize:
			call note2("PLEASE ENTER THE SIZE|(EG \"10X5\")", "RC", size, "");
			if (not size) {
				goto selrow;
			}
			var size2 = size;
			tt = UPPERCASE ^ LOWERCASE;
			//CONVERT 'Xx' TO '' IN T
			//CONVERT T:'.' TO '' IN SIZE2
			tt ^= "*/-,";
			size2.converter(tt, (tt.length()).space());
			size2.trimmer();
			size2.converter(" ", "X");
			if (not(size2.index("X", 1) and (size2.field("X", 1)).isnum() and (size2.field("X", 2)).isnum())) {
				msg = "PLEASE ENTER A SIZE - EG \"10X5\"";
				gosub note();
				goto inpsize;
			}
			size = size2;
		}

		ANS = rowtitle;
		if (size) {
			ANS ^= size;
		}
		if (coltitle ne "") {
			ANS ^= "," ^ coltitle;
		}

		DATA ^= "" "\r";

		goto 7083;
	}
	if (mode == "VAL.SPEC2") {
		var spec = RECORD.a(21, win.mvx);
		rcard = "";
		goto valspec;

		goto 7083;
	}
	if (mode == "VAL.SPEC") {
		if (win.is == win.isorig) {
			return;
		}

		gosub chkupd();
		if (not win.valid) {
			return;
		}

		var spec = win.is;
		rcard = "";

valspec:

		//check that same vehicle/spec has not been entered in any other lines
		vcode = RECORD.a(20, win.mvx);
		if (RECORD.a(21).locateusing(spec, VM, lnx)) {
			while (true) {
				tt = RECORD.a(20, lnx);
			///BREAK;
			if (tt == "" or lnx ne win.mvx and (tt == vcode and RECORD.a(21, lnx) == spec)) break;;
				lnx += 1;
			}//loop;
			if (tt) {
				//MSG='PLEASE COMBINE LINES WITH THE SAME SPEC|FOR THE SAME VEHICLE INTO ONE LINE|(OTHERWISE IT IS MORE DIFFICULT TO CHECK SUPPLIER INVOICES)'
				//GOSUB NOTE
			}
		}

		//split into two parts
		if (spec.index(",", 1)) {
			//SPEC2=SPEC[-1,'B,']
			//SPEC1=SPEC[1,COL1()-1]
			spec1 = spec.field(",", 1);
			spec2 = spec.field(",", 2, 9999);
		}else{
			spec1 = spec;
			spec2 = "";
		}

		//get the VEHICLE
		vcode = RECORD.a(20, win.mvx);
		var vehicle;
		if (not(vehicle.read(agy.vehicles, vcode))) {
			msg = DQ ^ (vcode ^ DQ) ^ " IS NOT ON VEHICLE FILE";
			return invalid();
		}

		if (not rcard) {
			gosub getratecard();
		}

		//degressive rate
		//locate brand code if possible in vehicle rates
		//otherwise locate client code and blank brand code
		var degrrate = "";
		var nlines = (rcard.a(7)).count(VM) + 1;
		clientcode = var("BILLING_CLIENT_CODE").calculate();
		var client;
		if (not(client.read(agy.clients, clientcode))) {
			client = "";
		}

		for (var lnx = 1; lnx <= nlines; ++lnx) {
			if (rcard.a(7, lnx) == clientcode) {
				tt = rcard.a(10, lnx);
				if (not degrrate) {
					degrrate = tt;
				}
				if (rcard.a(8, lnx) == "" or rcard.a(8, lnx) == RECORD.a(11)) {
					if (tt) {
						degrrate = tt;
					}
				}
			}
		};//lnx;

		//get the multiplier
		var mult = 1;
		var size = spec1.field("*", 2);
		if (size) {
			spec1.splicer(-size.length(), size.length(), "");
			if (size.index("X", 1) and (size.field("X", 1)).isnum() and (size.field("X", 2)).isnum()) {
				mult = size.field("X", 1) * size.field("X", 2);
			}else{
				msg = DQ ^ (size ^ DQ) ^ " - SIZE SHOULD BE ENTERED|AS numberXnumber (EG \"10X5\")";
				return invalid();
			}
		}

		//find the row number
		if (not(rcard.a(20).locateusing(spec1, VM, rown))) {
			if (rcard.a(20)) {
				msg = DQ ^ (spec1 ^ DQ) ^ " is not on the rate card.|";
				msg.r(-1, "YOU WILL HAVE TO ENTER THE|GROSS UNIT BILL YOURSELF");
				gosub note();
			}
			gosub calcubill();
			gosub calcucost();
			goto valspecexit;
		}

		//duration must be specified for broadcast vehicle
		if (spec2 == "") {
			//LOCATE VEHICLE<2> IN 'TVýTýSATýSýRýC'<1> SETTING T THEN
			//tv satellite radio cinema
			if (var("TSRC").index(vehicle.a(2)).substr(1, 1, 1)) {
				msg = "Broadcast vehicle spec should be Segment \",\" Duration|(30\" USED)";
				gosub note();
				goto valspecexit;
			}
		}

		//find the column number
		var coln = 1;
		tt = spec2;
		if (not tt) {
			tt = degrrate;
		}
		var loaded = 0;
		mult = 1;

		var colcodes = rcard.field(FM, 81, nratecardcols);
	// ncols=count(colcodes,vm)+(colcodes<>'')
		if (tt) {
			if (colcodes) {
				if (not(colcodes.locateusing(tt, FM, coln))) {
					//"per second" rates
					//if cannot find column, and is numeric then is a multiple of column 1
					//eg '30"' becomes 30x the rate in column 1
					if ((tt.substr(1, tt.length() - 1)).isnum()) {
						mult = tt.substr(1, tt.length() - 1);
						coln = 1;
					}else{

	// for coln=1 to ncols
	// locate t in colcodes<1,coln> using '|' setting loaded else null
	// until loaded
	// next coln
	// if loaded else

						msg = DQ ^ (tt ^ DQ) ^ " - NOT FOUND IN RATE CARD COLUMNS";
						gosub note();
						coln = 1000000;
	// end
					}
				}
			}
		}

		if (not(gen.currency.reado(gen.currencies, vehicle.a(5)))) {
			gen.currency = "";
		}
		var ndecs = gen.currency.a(3);
		if (ndecs == "") {
			ndecs = "2";
		}
		var currfmt = "MD" ^ ndecs ^ "0P";

		//bills
		///////
		//d ebug
		//get rate at column/rown
		rate = rcard.a(20 + coln, rown);
		if (rate == "") {
			msg = "YOU WILL HAVE TO ENTER|THE GROSS UNIT BILL YOURSELF";
			gosub note();
		}else{
			rate = (rate * mult).oconv(currfmt);
		}

		RECORD.r(37, win.mvx, rate);
		tt = win.registerx[1].a(37);
		gosub r();
		gosub calcubill();

	/*;
			//loading
			loadamt='';
			if loaded then;
				for loadcoln=1 to ncols;
					colcode=colcodes<1,loadcoln>;
					if colcode[1,len(spec2)]=spec2 then;
						loadamt2='';
						cellamt=rcard<20+loadcoln,rown>;
						if cellamt<>'' then;
							if colcode[-1,1]='%' then;
								loadamt2=rate*cellamt;
							end else;
								sizes=colcode[len(spec2)+1,9999];
								convert '/-,':@upper.case to space(100) in sizes;
								sizes=trim(sizes);
								if sizes then;
									minsize=field(sizes,' ',1);
									maxsize=field(sizes,' ',1);
									if maxsize else maxsize=10^10;
									if num(minsize) then;
										if mult>=minsize and mult<=maxsize then;
											loadamt2=cellamt;
											end;
										end;
									end;
								end;
							end;
						if loadamt2 then if loadamt='' or loadamt>t then loadamt=loadamt2;
						end;
					next loadcoln;
				end;
	*/

		//costs
		//////

		var costcoloffset = costrcard.a(12);
		var costcoln = coln + costcoloffset;
		if (costrcard.a(20).locateusing(rcard.a(20, rown), VM, costrown)) {
			costrate = costrcard.a(20 + costcoln, costrown);
		}else{
			costrate = rcard.a(20 + costcoln, rown);
		}
		if (costrate == "") {
			if (rate ne "") {
				msg = "YOU WILL HAVE TO ENTER THE|GROSS UNIT COST YOURSELF";
				gosub note();
			}
		}else{
			//IF MULT NE 1 THEN costRATE=costRATE/MULT
			costrate = (costrate * mult).oconv(currfmt);
		}

		RECORD.r(41, win.mvx, costrate);
		tt = win.registerx[1].a(41);
		gosub r();
		RECORD.r(21, win.mvx, spec);
		gosub calcucost();

		//promopub: negative commission means copy g/nunitcost to gunitbill
		brandcode = RECORD.a(11);

		gosub getcommfee();

		if (win.registerx[6] and win.registerx[6] < 0) {
			if (commng.ucase() ne "G") {
				//gunitbill=nunitcost
				var nuc = RECORD.a(42, win.mvx);
				if (mult ne 1) {
					nuc = (nuc / mult).oconv(currfmt);
				}
				RECORD.r(37, win.mvx, nuc);
			}else{
				//gunitbill=gunitcost
				RECORD.r(37, win.mvx, RECORD.a(41, win.mvx));
			}
			//gosub calcubill
			//this was commented out - but why?
			//it caused parmobel like clients not have the nup calculated correctly
			gosub calcubill();
		}

		//clear reconciliations if vehicle changed
		//R<68,MV>=''
		//R<67,MV>=''

valspecexit:
		if (win.wi == win.registerx[1].a(21)) {
			gosub chkbkg();
		}

	/* smdates here but not converted;
		CASE MODE='VAL.DAY';
			IF IS=IS.ORIG THEN RETURN;

			call plan.subs3('CHECKCOINCIDENCE');
			if valid else return;

			is=field(is,' ',1);

			IF IS=IS.ORIG THEN RETURN;
			IF IS THEN NUMBER=1 ELSE NUMBER='';
			R<39,MV>=NUMBER;
			T=WIS<39>;gosub r;
			R<43,MV>=NUMBER;
			T=WIS<43>;gosub r;
			IF IS THEN;
				T=SPACE(IS-1):'1';
			END ELSE;
				T='';
				END;
			R<22,MV>=T;

			r<si<4>,mv>=is;
			call evaluate.subs('EVALUATE LINE');
	*/

	/*;
		CASE MODE='GETRATECARD';
			vcode='';
			CALL msg2('WHICH VEHICLE DO YOU WANT ?|(PRESS ENTER TO SELECT)','RCE',vcode,'');
			IF vcode EQ CHAR(27) THEN RETURN;
			IF vcode ELSE;
				CALL CATALYST('P','ADPOPUPS*VEHICLE');
				IF @ANS ELSE RETURN;
				vcode=@ANS;
				END;
			READ VEHICLE FROM VEHICLES,vcode ELSE;
				MSG=QUOTE(vcode):' - VEHICLE CODE NOT ON VEHICLE FILE';
				GOTO INVALID;
				END;

			GOSUB GETRATECARD;

			//extract the row and column titles
			ROWTITLES=rcard<20>;
			NROWS=COUNT(ROWTITLES,vm)+1;
			COLTITLES=FIELD(rcard,fm,81,nratecardcols);
			//loop while coltitles[-1,1]=fm do coltitles[-1,1]='' repeat
			coltitles=trim2(coltitles,fm,'B');
			NCOLS=COUNT(COLTITLES,fm)+1;

			//select the row
	SELROW2:
			PARAMS='1:20:L::Description';
			FOR COLN=1 TO NCOLS;
				//PARAMS:='\':COLN+1:':6:R::':rcard<80+COLN>
				PARAMS:='\':COLN+1:':6:R::':coltitles<coln>;
				NEXT COLN;
			LINES='';
			FOR ROWN=1 TO NROWS;
				LINE=ROWTITLES<1,ROWN>;
				FOR COLN=1 TO NCOLS;
					LINE<1,COLN+1>=rcard<20+COLN,ROWN>;
					NEXT COLN;
				LINES:=fm:LINE;
				NEXT ROWN;
			LINES[1,1]='';
			//all rows or selected rows
			IF DECIDE('Do you want the whole rate card ?','',REPLY) ELSE RETURN;
			IF REPLY=2 THEN;
				Q='Which lines do you want ?';
				ROWNS=POP.UP(0,0,'',LINES,PARAMS,'R','O',Q,'','','','P');
				IF ROWNS ELSE RETURN;
			END ELSE;
				ROWNS='';
				END;

			//select the column
			COLTITLE='';
			IF COUNT(COLTITLES,fm) THEN;
				PARAMS='1:20:L::Description';
				LINES='';
				FOR COLN=1 TO NCOLS;
					LINES:=fm:COLTITLES<COLN>;
					NEXT COLN;
				LINES[1,1]='';
				Q='Which column do you want ?';
				COLN=POP.UP(0,0,'',LINES,PARAMS,'R','',Q,'','','','P');
				IF COLN ELSE GOTO SELROW2;
				COLTITLE=COLTITLES<COLN>;
				END;

			IF ROWNS THEN;
				NROWNS=COUNT(ROWNS,fm)+1;
			END ELSE;
				NROWNS=NROWS;
				END;
			updBILLext=1;
			updCOSText=1;
			FOR ROWNN=1 TO NROWNS;
				IF ROWNS THEN;
					ROWN=ROWNS<ROWNN>;
				END ELSE;
					ROWN=ROWNN;
					END;
				T=R<20>;
				SPEC=ROWTITLES<1,ROWN>;
				IF COLTITLE THEN SPEC:=',':COLTITLE;
				//zzz prevent duplicates
				MV=COUNT(T,vm)+(T NE '')+1;
				AMV.ACTION=4;
				R<20,MV>=vcode;
				R<21,MV>=SPEC;
				GOSUB VAL.VEHICLE;
				IF VALID ELSE RETURN;
				GOSUB VAL.SPEC;
				IF VALID ELSE RETURN;
				DISPLAY.ACTION=5;
				RESET=3;
				NEXT ROWNN;
	*/

		goto 7083;
	}
	if (mode.substr(1, 13) == "UPDATEEXTRAS2") {

		reply = 2;
		var forceotherfee = "";
		goto updateextras;

	if (mode == "UPDATEEXTRAS") {

		gosub chkupd();
	}else if (not win.valid) {
			return;
		}

		if (not(decide("Get standard discounts and charges ?", "Both" _VM_ "Client side" _VM_ "Agency side", reply))) {
			return;
		}

		var forceotherfee = "";

updateextras:
		var storemv = win.mvx;
		var updbillext = reply ne 3;
		var updcostext = reply ne 2;
		var nmvs = (RECORD.a(20)).count(VM) + 1;
		win.amvaction = 4;
		vcode = "";
		for (var win.mvx = 1; win.mvx <= nmvs; ++win.mvx) {
			tt = RECORD.a(20, win.mvx);
			if (tt) {
				if (tt ne vcode) {
					vcode = tt;
					//force reread of rate card
					rcard = "";
				}
				gosub valvehicle();
			}
		};//win.mvx;
		win.displayaction = 5;
		win.reset = 3;
		win.mvx = storemv;

	/*;

		CASE MODE='COPYCOSTEXTRAS';
			cEXTRAS.FN=139;
			GOTO COPYEXTRAS;

		CASE MODE='COPYEXTRAS';
			cEXTRAS.FN=140;

	COPYEXTRAS:
			NEW.EXTRA='';
			CALL msg2('What discount(s) and charge(s) do you want ?','RCE',NEW.EXTRA,'');
			IF NEW.EXTRA EQ CHAR(27) THEN RETURN;

			Q='Copy to which lines ?';
			GOSUB SELECTLINES;

			IF lnS ELSE RETURN;

			//copy the extras into the record
			STORE.MV=MV;
			FOR lnN=1 TO NlnS;
				MV=lnS<lnN>;
				old.extra=r<cextras.fn,mv>;
				R<cEXTRAS.FN,MV>=NEW.EXTRA;
				IF MODE='COPYEXTRAS' THEN;
					GOSUB calcubill;
				END ELSE;
					GOSUB calcucost;
					gosub chkbkg;
					END;
				NEXT lnN;
			MV=STORE.MV;
			T=WIS<cEXTRAS.FN>;gosub r;
	*/

		goto 7083;
	}
	if (mode == "RECALC") {
		win.displayaction = 5;
		win.reset = 3;

		goto 7083;
	}
	if (mode == "GETEXCHRATES") {
		gosub getexchrates();

		goto 7083;
	}
	if (mode == "RECALC_ALL_NETS") {
		var oldmv = win.mvx;

		var nmvs = (RECORD.a(20)).count(VM) + 1;
		for (var win.mvx = 1; win.mvx <= nmvs; ++win.mvx) {
			gosub calcubill();
			gosub calcucost();
		};//win.mvx;

		win.mvx = oldmv;

		goto 7083;
	}
	if (1) {
		msg = DQ ^ (mode ^ DQ) ^ " - INVALID MODE IGNORED";
		gosub note();

	}
L7083:
	return;

}

subroutine getratecard() {
	//these are all wrong now that loading column is inserted before disc
	if (var("SPLITEXTRAS").calculate()) {
		discfn = 1;
		commfn = 2;
		feefn = 3;
		taxfn = 4;
		taxg = "g";
		prepfn = 5;
	}else{
		discfn = -1;
		commfn = -1;
		feefn = -1;
		taxfn = -1;
		taxg = "G";
		prepfn = -1;
	}

	rcard = "";
	costrcard = "";

	var rdate = RECORD.a(24, win.mvx);
	var costdate = RECORD.a(27, win.mvx);

	if (not(rdate and costdate)) {

		//extract dates
		var ratekeys = vehicle.a(8);
		if (not ratekeys) {
			return;
		}
		rdates = "";
		for (var ii = 1; ii <= 999; ++ii) {
			rdate = ratekeys.a(1, ii).field("*", 2);
		///BREAK;
		if (not rdate) break;;
			rdates.r(-1, rdate);
		};//ii;

		//default to relevent rate card date
		var idate = ("1/" ^ var("PERIOD").calculate()).iconv("D/E");
		if (not(vehicle.a(8).locatebyusing(vcode ^ "*" ^ idate, "DR", daten, VM))) {
			{}
		}
		if (vehicle.a(8, daten) == "") {
			//MSG='THE FIRST RATES FOR THIS VEHICLE ONLY BECOME EFFECTIVE ON THE ':OCONV(FIELD(VEHICLE<8,DATEN-1>,'*',2),'D2')
			//GOSUB NOTE
			daten -= 1;
		}

		//user selects rate date
		if (rdates.count(FM)) {
			tt = pop_up(5, 5, "", rdates, "1:40:L:[DATE,4]:Effective Date", "F", "", "Which rate card do you want|to use FOR THE CLIENT ?", "", "", FM ^ daten, "P");
			if (tt) {
				daten = tt;
			}
		}
		rdate = rdates.a(daten);

		RECORD.r(24, win.mvx, rdate);
	}

	//get the rate card
	if (not(rcard.read(agy.ratecards, vcode ^ "*" ^ rdate))) {
		msg = DQ ^ (rdate.oconv("[DATE,4*]") ^ DQ) ^ "- CANNOT READ VEHICLE RATES RECORD";
		goto note;
	}

	//get the cost rate card
	costrcard = rcard;

	if (not costdate) {
		//default to special cost rate date if specified
		tt = rcard.a(13);
		if (tt) {
			if (not(rdates.locatebyusing(tt, "DR", daten, FM))) {
				{}
			}
		}

		//user selects date for costs
		if (rdates.count(FM)) {
			tt = pop_up(5, 5, "", rdates, "1:40:L:[DATE,4]:Effective Date", "F", "", "Which rate card do you want|to use FOR THE AGENCY ?", "", "", FM ^ daten, "P");
			if (tt) {
				daten = tt;
			}
		}
		costdate = rdates.a(daten);

		RECORD.r(27, win.mvx, costdate);
	}

	//get cost rate card
	if (costdate ne rdate) {
		if (not(costrcard.read(agy.ratecards, vcode ^ "*" ^ costdate))) {
			{}
		}
	}

	return;

}

subroutine calcucost() {
	var cgunitbillfn = 41;
	var cextrasfn = 139;
	var cunitbillfn = 42;
	var cunitbill2fn = 63;

	//get original net unit cost
	var nuc = RECORD.a(42, win.mvx);

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

subroutine calcubill2() {
	var gunitbill = RECORD.a(cgunitbillfn, win.mvx);
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
		gosub note();
	}

	//update net unit bill and redisplay
	RECORD.r(cunitbillfn, win.mvx, unitbill);
	if (mode ne "RECALC_ALL_NETS") {
		tt = win.registerx[1].a(cunitbillfn);
		gosub r();
	}
	// IF cUNIT.BILL.FN=38 THEN T=TOT.BILL.WI ELSE T=TOT.cost.wi
	// gosub r

	//update breakdown
	breakdown.converter(FM, SVM);
	tt = gunitbill ^ SVM ^ breakdown;
	//call trimexcessmarks(t)
	//loop while t[-1,1]=sm do t[-1,1]='' repeat
	call trim2(tt, SVM, "B");
	RECORD.r(cunitbill2fn, win.mvx, tt);

	return;

}

subroutine getexchrates() {
	var vcodes = RECORD.a(20);
	var nvehicles = vcodes.count(VM) + 1;
	var storemv = win.mvx;
	for (var win.mvx = 1; win.mvx <= nvehicles; ++win.mvx) {
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
		if (not(vehicle.reado(agy.vehicles, vcode))) {
			return;
		}
		vehiclecurrcode = vehicle.a(5);
		RECORD.r(45, win.mvx, vehiclecurrcode);
		tt = win.registerx[1].a(45);
		gosub r();
	}

	//use existing rate if possible
	tt = RECORD.a(45).erase(1, win.mvx, 0);
	if (tt.a(1).locateusing(vehiclecurrcode, VM, tt)) {
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
		if (gen.currency.reado(gen.currencies, vehiclecurrcode)) {
			var datex = var("1/" ^ RECORD.a(12)).iconv("D/E");
			if (not(gen.currency.a(4).locatebyusing(datex, "DR", tt, VM))) {
				{}
			}
			rate = gen.currency.a(5, tt);
			if (not rate) {
				rate = gen.currency.a(5, tt - 1);
			}
			if (not rate) {
				msg = "NOTE:|" ^ (DQ ^ (vehiclecurrcode ^ DQ)) ^ " - NO EXCHANGE RATE|(YOU WILL HAVE TO ENTER IT)";
				gosub note();
			}
			costrate = rate;
		}else{
			msg = DQ ^ (vehiclecurrcode ^ DQ) ^ " - CURRENCY NOT ON FILE";
			gosub note();
			rate = "";
			costrate = rate;
		}

		//if invoice currency not base currency then
		//convert base rate (but not cost rate) to be
		//rate from vehicle currency to invoice currency
		var invcurrcode = var("CURRENCY_CODE").calculate();
		if (invcurrcode ne agy.agp.a(2)) {
			if (vehiclecurrcode == invcurrcode) {
				rate = 1;
			}else{
				if (gen.currency.reado(gen.currencies, invcurrcode)) {
					var datex = var("1/" ^ RECORD.a(12)).iconv("D/E");
					if (not(gen.currency.a(4).locatebyusing(datex, "DR", tt, VM))) {
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
						gosub note();
						rate = "";
					}
				}else{
					msg = DQ ^ (invcurrcode ^ DQ) ^ " - NOT ON FILE";
					gosub note();
					rate = "";
				}
			}
		}

		//look for rate on client file
		var client;
		if (client.read(agy.clients, var("BILLING_CLIENT_CODE").calculate())) {
			if (invcurrcode == client.a(11)) {
				if (client.a(13).locateusing(vehiclecurrcode, VM, currn)) {
					rate = client.a(17, currn);
				}
			}
		}

	}
	//zzzcall note(rate)
	RECORD.r(40, win.mvx, rate);
	RECORD.r(44, win.mvx, costrate);
	tt = win.registerx[1].a(40);
	gosub r();
	tt = win.registerx[1].a(44);
	gosub r();

	return;

}

subroutine chkinv() {
	gosub chkupd();
	if (RECORD.a(4)) {
		msg = "THE SCHEDULE HAS ALREADY BEEN INVOICED";
		return invalid();
	}
	return;

}

subroutine getcommfee() {

	discs = "";
	var nlines = (rcard.a(7)).count(VM) + 1;
	agtfee = "";
	agtfeeng = "";
	win.registerx[6] = "";
	commng = "";

	//2001/3/13 only one discount allowed now
	if (not(rcard.a(8).locateusing(brandcode, VM, lnx))) {
		if (not(rcard.a(7).locateusing(clientcode, VM, lnx))) {
			if (not(rcard.a(7).locateusing("*", VM, lnx))) {
				lnx = 0;
			}
		}
	}

	//FOR lnx=1 TO NLINES
	if (lnx) {
		nlines = lnx;
	}else{
		nlines = -1;
	}
	for (var lnx = lnx; lnx <= nlines; ++lnx) {
		//14/8/97 IF rcard<7,lnx>=CLIENT.CODE THEN
		if (rcard.a(7, lnx) == clientcode or rcard.a(7, lnx) == "*") {
			tt = rcard.a(10, lnx);
			if (rcard.a(8, lnx) == "" or rcard.a(8, lnx) == brandcode) {

				//get the discount
				var discbrkt = rcard.a(9, lnx);

				//convert discount bracket to figure
				if (discbrkt) {
					var disc = "";
					if (rcard.a(4).locateusing(discbrkt, VM, vn)) {
						disc = rcard.a(5, vn);
					}else{
						if (ANS.isnum()) {
							disc = discbrkt;
						}else{
							msg = DQ ^ (discbrkt ^ DQ) ^ " - DISCOUNT BRACKET NOT FOUND IN VEHICLE RATES";
							gosub note();
						}
					}
					if (disc) {
						if (not(var("-+").index(disc.substr(1, 1), 1))) {
							disc.splicer(1, 0, "+");
						}
						var crnote = rcard.a(65, lnx);
						crnote.converter("YN", "X");
						discs.r(discfn, disc ^ "%" ^ crnote);
					}
				}

				//get the agent fee
				tt = rcard.a(18, lnx);
				if (tt.length()) {
					agtfee = tt;
					agtfeeng = rcard.a(19, lnx);
				}

				//get the commission
				tt = rcard.a(61, lnx);
				if (tt.length()) {
					win.registerx[6] = tt;
					commng = rcard.a(62, lnx);
				}
			}
		}
	};//lnx;

	//use the public commission if there no special commission for this client
	if (win.registerx[6] == "") {
		win.registerx[6] = rcard.a(1);
		commng = rcard.a(6);
	}
	commng.converter("Nn", "");

	//990729 allow for "net plus" clients
	if (client.a(12) == "NN" and costrcard.a(16)) {
		win.registerx[6] = costrcard.a(16);
	}

	return;

	//$INSERT GBP,WINDOW.SUBS
	//$insert bp,plan.subs6

	//jbase donotcompile
	//declare function security,unlockrecord,unassigned
	/////////
security:
	/////////
	//valid=1 move down
	var op = singular(win.datafile);
	//IF OP[-3,3]='IES' THEN OP[-3,3]='Y'
	//IF OP[-1,1]='S' THEN OP[-1,1]=''
	//OP:=' FILE'
security2:
	win.valid = 1;
	op.converter("_.", "  ");
	var op2 = "";
	if (op.substr(-1, 1) == DQ) {
		op2 = DQ ^ (op.substr(-2, "B\"") ^ DQ);
		op.splicer(-op2.length(), op2.length(), "");
		op2.splicer(1, 0, " ");
	}
	if (mode.index("INIT", 1)) {
		if (not(authorised(op ^ " ACCESS" ^ op2, msg, ""))) {
			gosub invalid();
			var().stop();
		}
		goto 9785;
	}
	if (mode.index("READ", 1) or mode.index("WRITE", 1)) {
		if (not win.wlocked) {
			op ^= " ACCESS";
		}else{
			if (win.orec) {
				op ^= " UPDATE";
			}else{
				op ^= " CREATE";
			}
		}
		var wspos = "";
wssec:
		if (not(authorised(op ^ op2, msg, ""))) {
			if (op2 == "") {
				op2 = DQ ^ (ID ^ DQ);
				wspos = "#";
				wsmsg = msg;
				goto wssec;
				goto 9647;
			}
			if (wspos) {
				msg = wsmsg;
			}
			if (win.orec == "" or not win.wlocked) {
				gosub invalid();
				win.reset = 5;
			}
			if (win.wlocked) {
				xx = unlockrecord(win.datafile, win.srcfile, ID);
				win.wlocked = 0;
			}
		}

	if (mode.index("DELETE", 1)) {
		if (not(authorised(op ^ " DELETE" ^ op2, msg, ""))) {
			return invalid();
		}
		goto 9785;
	}
	if (1) {
security3:
		if (op2.unassigned()) {
			op2 = "";
		}
	}else if (not(authorised(op ^ op2, msg, ""))) {
			return invalid();
		}
	}
	return;

	/////////
invalidq:
	/////////
	msg.splicer(1, 0, DQ ^ (win.is.a(1, 1, 1) ^ DQ) ^ " ");

}

subroutine invalid() {
	win.valid = 0;
	//IF IS.ORIG THEN IS=IS.ORIG
	if (not win.isorig.unassigned()) {
		win.is = win.isorig;
		if (not win.reset) {
			win.reset = 1;
		}
	}
	if (not msg) {
		return;
	}
	goto note2;

}

subroutine note() {
	// print char(7):
	call note(msg, "", "", "");
	return;

note2:
	// PRINT CHAR(7):
	call mssg(msg, "", "", "");
	return;

	//NB r is equated to newrecord not @record in planimport.subs
	//jbase donotcompile

	//also included in program.subs

}

subroutine chkupd() {
	if (not(authorised("SCHEDULE UPDATE PLAN", msg, ""))) {
		return invalid();
	}
	return;

}

subroutine chkbkg() {
	if (win.datafile ne "SCHEDULES") {
		return;
	}
	if (mode.substr(1, 4) ne "VAL.") {
		return;
	}

	//update the record
	RECORD.r(win.si.a(4), win.mvx, win.is);

chkbkg2:
	if (silent.unassigned()) {
		silent = 0;
	}

	//if not booked then exit
	if (RECORD.a(51, win.mvx) == "") {
		return;
	}

	//get pointer to line in the original record if any
	//(as there is a booking order then there must be an old line)
	var mvorec = RECORD.a(19, win.mvx);
	if (not mvorec) {
		call oswrite("MV=" ^ win.mvx ^ FM ^ RECORD, ID);
		var zzz = zzz;
	}

	//modify mode and already booked (booking number present)
	//then compare current with old (or old as booked if present)
	//orec as.booked : yes no
	//record as.booked : yes no yes no
	//same clear leave clear leave
	//different leave save save save

	//check against original "as booked" details if any
	//if different then save original "as booked" as "as booked"
	//if same then clear the "as booked"
	if (win.orec.a(52, mvorec) ne "") {

		//21/07/97
		var temp = RECORD.a(23, win.mvx);
		temp ^= TM ^ RECORD.a(41, win.mvx);
		temp ^= TM ^ RECORD.a(139, win.mvx);
		temp ^= TM ^ RECORD.a(42, win.mvx);
		while (true) {
		///BREAK;
		if (not(temp.substr(-1, 1) == TM)) break;;
			temp.splicer(-1, 1, "");
		}//loop;

		if (RECORD.a(20, win.mvx) ne win.orec.a(52, mvorec)) {
			var different = 1;
			goto 10371;
		}
		if (RECORD.a(21, win.mvx) ne win.orec.a(54, mvorec)) {
			var different = 1;
			goto 10371;
		}
		if (RECORD.a(22, win.mvx) ne win.orec.a(55, mvorec)) {
			var different = 1;

			//21/07/97
			//CASE R<23,MV> NE OREC<56,MV.OREC> ; DIFFERENT=1
			goto 10371;
		}
		if (temp ne win.orec.a(56, mvorec)) {
			var different = 1;

			goto 10371;
		}
		if (1) {
clearasbooked:
			if (not silent) {
				msg = "THIS LINE NO LONGER REQUIRES RE-BOOKING";
				gosub note();
			}
clearasbooked2:
			//clear "as booked"
			RECORD.r(52, win.mvx, "");
			RECORD.r(54, win.mvx, "");
			RECORD.r(55, win.mvx, "");
			RECORD.r(56, win.mvx, "");
			return;
		}
L10371:  *save "as booked";
		RECORD.r(52, win.mvx, win.orec.a(52, mvorec));
		RECORD.r(54, win.mvx, win.orec.a(54, mvorec));
		RECORD.r(55, win.mvx, win.orec.a(55, mvorec));
		RECORD.r(56, win.mvx, win.orec.a(56, mvorec));
		return;
	}

	//check against original details if any
	//if different then save original as "as booked"
	//if same then clear the "as booked"
	//21/7/97
	var temp = RECORD.a(23, win.mvx);
	temp ^= TM ^ RECORD.a(41, win.mvx);
	temp ^= TM ^ RECORD.a(139, win.mvx);
	temp ^= TM ^ RECORD.a(42, win.mvx);
	while (true) {
	///BREAK;
	if (not(temp.substr(-1, 1) == TM)) break;;
		temp.splicer(-1, 1, "");
	}//loop;
	var temp2 = win.orec.a(23, mvorec);
	temp2 ^= TM ^ win.orec.a(41, mvorec);
	temp2 ^= TM ^ win.orec.a(139, mvorec);
	temp2 ^= TM ^ win.orec.a(42, mvorec);
	while (true) {
	///BREAK;
	if (not(temp2.substr(-1, 1) == TM)) break;;
		temp2.splicer(-1, 1, "");
	}//loop;
	if (win.orec.a(20, mvorec) ne RECORD.a(20, win.mvx)) {
		var different = 1;
		goto 10752;
	}
	if (win.orec.a(21, mvorec) ne RECORD.a(21, win.mvx)) {
		var different = 1;
		goto 10752;
	}
	if (win.orec.a(22, mvorec) ne RECORD.a(22, win.mvx)) {
		var different = 1;
		//21/7/97
		//CASE OREC<23,MV.OREC> NE R<23,MV>;DIFFERENT=1

	if (temp ne temp2) {
		var different = 1;
		goto 10752;
	}
	if (1) {
		goto clearasbooked;
	}
L10752:
	//if changes are not to be re-booked then clear "as booked"
rebooked:

	if (not silent) {
		if (not(decide("Do you want the|changes to be re-booked ?", "", reply))) {
			goto rebooked;
		}
	}else if (reply == 2) {
			goto clearasbooked2;
		}
	}

	//save "as booked" (only if there are some dates to be saved)
	RECORD.r(52, win.mvx, win.orec.a(20, mvorec));
	RECORD.r(54, win.mvx, win.orec.a(21, mvorec));
	RECORD.r(55, win.mvx, win.orec.a(22, mvorec));

	//21/07/97
	//R<56,MV>=OREC<23,MV.OREC>
	temp = win.orec.a(23, mvorec);
	temp ^= TM ^ win.orec.a(41, mvorec);
	temp ^= TM ^ win.orec.a(139, mvorec);
	temp ^= TM ^ win.orec.a(42, mvorec);
	while (true) {
	///BREAK;
	if (not(temp.substr(-1, 1) == TM)) break;;
		temp.splicer(-1, 1, "");
	}//loop;
	RECORD.r(56, win.mvx, temp);

	if (RECORD.a(8)) {
		if (not silent) {
			msg = "Note: This schedule was fully booked,|but now needs rebooking";
			gosub note();
		}
		RECORD.r(8, "");
	}else{
		if (not silent) {
			msg = "SCHEDULE LINE " ^ win.mvx ^ " REQUIRES RE-BOOKING";
			msg.r(-1, RECORD.a(20, win.mvx));
			gosub note();
		}
	}

	return;

}

subroutine r() {
	//redisplay
	if (not tt) {
		return;
	}
	if (not(win.redisplaylist.locateusing(tt, FM, t2))) {
		win.redisplaylist.r(-1, tt);
	}
	win.displayaction = 6;
	win.reset = 3;
	return;

}


libraryexit()