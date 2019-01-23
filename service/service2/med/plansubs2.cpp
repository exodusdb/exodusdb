#include <exodus/library.h>
libraryinit()

#include <agencysubs.h>
#include <authorised.h>
#include <getreccount.h>
#include <generalsubs.h>
#include <plansubs2.h>
#include <evaluatesubs.h>
#include <pop_up.h>
#include <calcbill.h>
#include <singular.h>

#include <agy.h>
#include <gen.h>
#include <win.h>

#include <window.hpp>

var msg;
var reply;//num
var ndecs;
var tt;//num
var vcode;
var rcard;
var vehicle;
var t2;
var xx;
var brandcode;
var clientcode;
var client;
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
var tspec;
var unused;
var rndg;
var multx;
var breakdown;
var currn;
var vn;
var wsmsg;
var silent;//num

function main(in mode) {
	//c med
	//garbagecollect;
	//y2k2 *jbase

	//fix a bug without finding it
	if (not(RECORD.a(1).isnum())) {
		RECORD.r(1, RECORD.a(1, 1, 1));
	}

	//actually 40 can be entered in the intranet system
	//but difficult to display so many in dos
	var nratecardcols = 20;

	if (mode == "CALCUBILL") {
		gosub calcubill();

	} else if (mode == "CALCUCOST") {
		gosub calcucost();

	} else if (mode == "VAL.TYPE") {
		if (win.is == win.isorig) {
			return 0;
		}
		gosub chkinv();
		if (not(win.valid)) {
			return 0;
		}

		call agencysubs("VAL.MEDIAPROD.TYPE.WITHACNO");

	} else if (mode == "VAL.APPROVED") {
		if (win.is == win.isorig) {
			return 0;
		}
		if (win.is and not win.isorig) {
			if (not(authorised("SCHEDULE APPROVAL", msg, ""))) {
				goto EOF_442;
			}
		}
		if (win.is == "") {
			gosub chkinv();
			if (not(win.valid)) {
				return 0;
			}
		}

	} else if (mode == "F2.JOB.NO") {
		gosub chkupd();
		if (not(win.valid)) {
			return 0;
		}

		//CALL CATALYST('V',DATAFILE:',JOB_NO')
		call agencysubs("F2.JOB");

	} else if (mode == "VAL.JOB.NO") {
		if ((win.is == win.isorig) or (win.is == "")) {
			return 0;
		}

		//only check if there are some jobs
		if (not(getreccount(agy.jobs, "", ""))) {
			return 0;
		}

		call agencysubs("VAL.JOB.OPEN");
		if (not(win.valid)) {
			return 0;
		}

		var job;
		if (not(job.read(agy.jobs, win.is))) {
			msg = DQ ^ (win.is ^ DQ) ^ " Job does not exist";
			return invalid(msg);
		}
		if (job.a(2) ne calculate("BRAND_CODE")) {
			msg = "WRONG JOB?||JOB " ^ (DQ ^ (win.is ^ DQ)) ^ " IS FOR BRAND " ^ (DQ ^ (job.a(2) ^ DQ));
			var().chr(7).output();
			if (not(decide(msg, "OK|Cancel", reply, 2))) {
				return 0;
			}
			if (reply == 2) {
				win.valid = 0;
				return 0;
			}
		}

	} else if (mode == "VAL.CURRENCY") {
		if (win.is == win.isorig) {
			return 0;
		}
		gosub chkinv();
		if (not(win.valid)) {
			return 0;
		}

		call generalsubs("VAL.CURRENCY");
		if (not(win.valid)) {
			return 0;
		}

		if (win.registerx(10) and win.is ne win.registerx(10)) {
			msg = "NOTE: EXPECTED CURRENCY IS " ^ (DQ ^ (win.registerx(10) ^ DQ));
			call note(msg);
		}

		//get exchange rates again
		if (win.isorig) {
			RECORD.r(13, win.is);
			RECORD.r(40, "");
			RECORD.r(44, "");
			gosub getexchrates();
		}

		//get ndecs
		if (ndecs.readv(gen.currencies, win.is, 3)) {
			if (RECORD.a(1) ne ndecs) {
				RECORD.r(1, ndecs);
				win.displayaction = 5;
				win.reset = 3;
			}
		}

	} else if (mode == "VAL.GUNIT.BILL") {
		if (win.is == win.isorig) {
			return 0;
		}

		gosub chkupd();
		if (not(win.valid)) {
			return 0;
		}

		if (win.isorig) {
			if (not(decide("Are you sure that you want|to change the gross unit bill ?", "No|Yes", reply))) {
				reply = 1;
			}
			if (reply == 1) {
				win.is = win.isorig;
				return 0;
			}
		}

		if (not(gen.currency.read(gen.currencies, RECORD.a(45, win.mvx)))) {
			gen.currency = "";
		}
		ndecs = gen.currency.a(3);
		if (ndecs == "") {
			ndecs = "2";
		}

		win.is = win.is.oconv("MD" ^ ndecs ^ "0P");

		//force net unit cost to zero as well (because calcucost will not do zero)
		if (not(win.is)) {
			RECORD.r(38, win.mvx, win.is);
		}

		RECORD.r(37, win.mvx, win.is);
		gosub calcubill();

		//change gross unit cost if was same or blank
		tt = RECORD.a(41, win.mvx);
		//if is.orig='' or is.orig=tt then
		if ((tt == "") or (win.isorig == tt)) {

			//change gross unit cost
			RECORD.r(41, win.mvx, win.is);
			tt = win.registerx(1).a(41);
			gosub EOF_1615();

			//change net unit cost as well if zero otherwise
			//net unit cost will not be recalculated in order to allow manual
			//entry of net unit cost without gross unit cost
			if (not(win.is)) {
				RECORD.r(42, win.mvx, win.is);
				tt = win.registerx(1).a(42);
				gosub EOF_1615();
			}

			gosub calcucost();
			gosub chkbkg();
		}

	} else if (mode == "VAL.GUNIT.COST") {
		if (win.is == win.isorig) {
			return 0;
		}

		gosub chkupd();
		if (not(win.valid)) {
			return 0;
		}

		if (win.isorig) {
			if (not(decide("Are you sure that you want|to change the gross unit cost ?", "No|Yes", reply))) {
				reply = 1;
			}
			if (reply == 1) {
				win.is = win.isorig;
				return 0;
			}
		}
		//same precision as gross unit bill
		win.is = win.is.oconv("MD" ^ RECORD.a(37, win.mvx).field(".", 2).length() ^ "0P");
		RECORD.r(41, win.mvx, win.is);
		gosub calcucost();
		gosub chkbkg();

	} else if (mode == "VAL.UNIT.BILL") {
		if (win.is == win.isorig) {
			return 0;
		}

		gosub chkupd();
		if (not(win.valid)) {
			return 0;
		}

		if (win.isorig) {
			if (not(decide("Are you sure that you want|to change the unit bill ?", "No|Yes", reply))) {
				reply = 1;
			}
			if (reply == 1) {
				win.is = win.isorig;
				return 0;
			}
		}
		win.is = win.is.oconv("MD" ^ RECORD.a(37, win.mvx).field(".", 2).length() ^ "0P");

	} else if (mode == "VAL.UNIT.COST") {
		if (win.is == win.isorig) {
			return 0;
		}

		gosub chkupd();
		if (not(win.valid)) {
			return 0;
		}

		if (win.isorig) {
			if (not(decide("Are you sure that you want|to change the unit cost ?", "No|Yes", reply))) {
				reply = 1;
			}
			if (reply == 1) {
				win.is = win.isorig;
				return 0;
			}
		}
		win.is = win.is.oconv("MD" ^ RECORD.a(37, win.mvx).field(".", 2).length() ^ "0P");
		gosub chkbkg();
		tt = win.registerx(3).a(2);
		gosub EOF_1615();
		tt = win.registerx(3).a(1);
		gosub EOF_1615();

	} else if (mode == "FREEONOFF") {
		if (not(win.mvx)) {
			msg = "PLEASE GO ONTO THE LINE YOU WANT AND TRY AGAIN";
			return invalid(msg);
		}

		if (win.is ne win.isorig) {
			msg = "PLEASE PRESS ENTER TO COMPLETE|YOUR ENTRY FIRST THEN TRY AGAIN";
			call note(msg);
			return 0;
		}

		gosub chkupd();
		if (not(win.valid)) {
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
		tt = win.registerx(1).a(140);
		gosub EOF_1615();
		tt = win.registerx(1).a(139);
		gosub EOF_1615();
		DATA ^= "" ^ MOVEKEYS.a(5) ^ "\r";

		gosub calcubill();
		gosub calcucost();

		gosub chkbkg();

		win.displayaction = 5;

	} else if (mode == "DEF.EXTRAS") {
		//is.dflt=r<190,mv>

	} else if (mode == "DEF.COST.EXTRAS") {
		//is.dflt=r<191,mv>

	} else if (mode == "VAL.EXTRAS") {
		if (win.is == win.isorig) {
			return 0;
		}

		gosub chkupd();
		if (not(win.valid)) {
			return 0;
		}

		//warning if already invoiced
		if (RECORD.a(46, win.mvx) ne "") {
			msg = "NOTE: Some ads on this line have already been invoiced at the old rate and a credit/debit note will have to be raised if you continue.";
			msg ^= FM ^ "|Please consider splitting this line into two lines, one at the old rate and one at the new rate.|";
			if (not(decide("!" ^ msg.oconv("T#50"), "OK|Cancel", reply, 2))) {
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
		if (lastextra.substr(1,5) == "OFFER") {
			RECORD.r(37, win.mvx, 0);
			tt = win.registerx(1).a(37);
			gosub EOF_1615();
		}

		//if no longer an offer, get the gross unit bill again
		if (((field2(win.isorig, ",", -1)).substr(1,5) == "OFFER") and lastextra.substr(1,5) ne "OFFER") {
			call plansubs2("VAL.SPEC2");
		}

		RECORD.r(140, win.mvx, win.is);
		gosub calcubill();

		//if use default then use cost default
		//if is and is=r<190,mv> then
		// r<139,mv>=r<191,mv>
		// goto calcucost
		// tt=wis<139>;gosub r
		// end

		//various types of free ads
		//copy to the cost to agency side
		if ((lastextra.trim()[1]).match("1A0A")) {
updcostextras:
			var lastextran = win.is.count(",") + (win.is ne "");
			win.isorig = RECORD.a(139, win.mvx);
			win.is = win.isorig.fieldstore(",", lastextran, 1, lastextra);
			gosub val.costextras2();
			if (not(win.valid)) {
				return 0;
			}
			tt = win.registerx(1).a(139);
			gosub EOF_1615();
			gosub chkbkg();
		}

		//r<190,mv>=is.orig2

	} else if (mode == "VAL.COST.EXTRAS") {
		if (win.is == win.isorig) {
			return 0;
		}

		gosub chkupd();
		if (not(win.valid)) {
			return 0;
		}

valcostextras2:

		var isorig2 = win.isorig;

		RECORD.r(139, win.mvx, win.is);
		gosub calcucost();

		gosub chkbkg();

		//r<191,mv>=is.orig2

	/*;
		CASE MODE='VAL.BRAND';
			IF IS='' OR IS=IS.ORIG THEN RETURN;

			gosub chkupd;
			if valid else return 0;

			//check if allowed to change brand after invoicing
			if r<inv.no.fn> then;
				msg='!THIS SCHEDULE HAS ALREADY BEEN INVOICED.';
				if security('SCHEDULE UPDATE BRAND AFTER INVOICE',xx,'') else return invalid(msg);
				msg<-1>=fm:'If you change the brand code after invoicing';
				msg<-1>='then the Media Analysis by brand and client will';
				msg<-1>='will not agree with the Billing Analysis.';
				msg<-1>='(The Media Analysis will be by the new brand only)';
				if decide2(msg,'OK|Cancel',reply,2) else reply='';
				if reply ne 1 then;
					valid=0;
					return 0;
					end;
				end;

			call agency.subs('VAL.BRAND');
			if valid else return 0;

			read brand from brands,is else;
				msg=quote(is):' - brand code not in brand file';
				return invalid(msg);
				end;
			client.code=brand<1,1>;
			read client from clients,client.code else;
				msg=quote(client.code):' - client not in client file';
				return invalid(msg);
				end;

			//convert to billing group
			if r<188> ne 'none' then;
				tt=client<25>;if tt then read client from clients,tt then client.code=tt;
				end;

			//get default VAT if client.tax.ng is null
			client.tax.ng=client<21>;
			if r<18>='' and agp<12> ne '' and client.tax.ng='' then;
				r<18>=agp<12>;
				tt=wis<18>;gosub r;
				end;

			//default the discount
			if r<17>='' then;
				agency.disc.ng=client<23>;
				if agency.disc.ng='' then;
					agency.disc=client<22>;
					if agency.disc then;
						r<17>=agency.disc;
						tt=wis<17>;gosub r;
						end;
					end;
				end;
	*/
	} else if (mode == "VAL.VEHICLE.CODE") {
		if (win.is == win.isorig) {
			return 0;
		}

		gosub chkupd();
		if (not(win.valid)) {
			return 0;
		}

		call agencysubs("VAL.VEHICLE");
		if (not(win.valid)) {
			return 0;
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
			return invalid(msg);
		}

		//check market code
		if (mode == "VAL.VEHICLE.CODE") {
			if (calculate("MARKET_CODE")) {
				if (vehicle.a(4) ne calculate("MARKET_CODE")) {
					msg = "NOTE: THIS VEHICLE IS IN THE " ^ (DQ ^ (vehicle.a(4) ^ DQ)) ^ " MARKET";
					call note(msg);
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
				var asinved;
				if (asinved.read(agy.schedules, ID ^ "*I")) {
					var nn = tt.count(VM) + 2;
					tt.r(1, nn, asinved.a(20));
					t2.r(1, nn, asinved.a(45));
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
					return invalid(msg);
				}
			}

		}

		//get the client
		brandcode = RECORD.a(11);

		clientcode = calculate("BILLING_CLIENT_CODE");
		var client;
		if (not(client.read(agy.clients, clientcode))) {
			msg = DQ ^ (clientcode ^ DQ) ^ " - MISSING FROM CLIENT FILE";
			return invalid(msg);
		}

		//update the currency code
		RECORD.r(45, win.mvx, vehicle.a(5));
		tt = win.registerx(1).a(45);
		gosub EOF_1615();

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
			call note(msg);
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
		clientcode = calculate("BILLING_CLIENT_CODE");
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
			if (agencyfee.substr(-3,3) == ".00") {
				agencyfee.splicer(-3, 3, "");
			}

			//less commission
			//IF comm<>'' THEN
			if (win.registerx(6)) {
				tt = "-" ^ win.registerx(6);
				if (tt.substr(1,2) == "--") {
					tt.splicer(1, 2, "");
				}
				discs.r(commfn, tt ^ "%" ^ commng);
			}

			//plus agency fee
			if (agencyfee) {
				tt = agencyfee ^ "%";
				if (not(var("+-").index(tt[1]))) {
					tt.splicer(1, 0, "+");
				}
				if (var("Gg").index(agencyfeeng)) {
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
				if (not(var("+-").index(tt[1]))) {
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
			win.registerx(6) = rcard.a(1);
			if (win.registerx(6)) {
				tax = (((100 - win.registerx(6)) / 100 * tax).oconv("MD40P")) + 0;
			}
		}else{
			tax = rcard.a(2);
			taxng = taxg;
		}
		if (tax) {
			if (not(var("+-").index(tax[1]))) {
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
			tt = otherfee;
			if (tt[1] ne "-") {
				tt.splicer(1, 0, "+");
			}
			discs.r(prepfn, tt);
		}

		//update the discounts and charges
		if (updbillext) {
			discs.converter(FM, ",");
			RECORD.r(140, win.mvx, discs);
			tt = win.registerx(1).a(140);
			gosub EOF_1615();
			gosub calcubill();
		}

		//costs
		//////

		//special disc for agency
		//14/8/97 if costrcard<11> then costdiscs<discfn>='-':costrcard<11>:'%'
		if (costrcard.a(11).length()) {
			tt = "-" ^ costrcard.a(11);
			if (tt.substr(1,2) == "--") {
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
		if (costcomm < win.registerx(6)) {
			costcomm = win.registerx(6);
		}

		if (costcomm) {
			tt = "-" ^ costcomm ^ "%";
			if (tt.substr(1,2) == "--") {
				tt.splicer(1, 2, "");
			}
			if (var("Gg").index(costcommng)) {
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
			if (not(var("-+").index(agtfee[1]))) {
				agtfee.splicer(1, 0, "+");
			}
			tt = agtfee ^ "%";
			if (var("Gg").index(agtfeeng)) {
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
			tt = win.registerx(1).a(139);
			gosub EOF_1615();
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
		tt = win.registerx(1).a(138);
		gosub EOF_1615();

		if (mode == "VAL.VEHICLE.CODE") {
			RECORD.r(win.si.a(4), win.mvx, win.is);
			call evaluatesubs("EVALUATE LINE");
		}

		return 0;

	// *reprocess the spec
	// SPEC=R<21,MV>
	// IF SPEC THEN GOSUB VAL.SPEC

	} else if (mode == "F2.SPEC") {
		gosub chkupd();
		if (not(win.valid)) {
			return 0;
		}

		vcode = RECORD.a(20, win.mvx);
		var vehicle;
		if (not(vehicle.read(agy.vehicles, vcode))) {
			msg = DQ ^ (vcode ^ DQ) ^ " IS NOT ON VEHICLE FILE";
			return invalid(msg);
		}
		gosub getratecard();

		//degressive rate
		//locate brand code if possible in vehicle rates
		//otherwise locate client code and blank brand code
		var degrrate = "";
		var nlines = rcard.a(7).count(VM) + 1;
		clientcode = calculate("BILLING_CLIENT_CODE");

		for (lnx = 1; lnx <= nlines; ++lnx) {
			if (rcard.a(7, lnx) == clientcode) {
				tt = rcard.a(10, lnx);
				if (not degrrate) {
					degrrate = tt;
				}
				if ((rcard.a(8, lnx) == "") or (rcard.a(8, lnx) == RECORD.a(11))) {
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
		coltitles = trim(coltitles, FM, "B");
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
		if (rowtitles.ucase().a(1).locateusing(calculate("CLIENT_NAME").ucase(), VM, xx)) {
			rown ^= xx;
		}

		var qq = "Which line do you want ?";
		//ROWN=POP.UP(0,0,'',LINES,PARAMS,'R','',Q,'','','','P')
		rown = pop_up(0, 0, "", lines, params, "R", "", qq, "", "", rown, "P");
		if (not rown) {
			return 0;
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
		if (rowtitle[-1] == "*") {
inpsize:
			call note("PLEASE ENTER THE SIZE|(EG \"10X5\")", "RC", size, "");
			if (not size) {
				goto selrow;
			}
			var size2 = size;
			tt = UPPERCASE ^ LOWERCASE;
			//CONVERT 'Xx' TO '' IN tt
			//CONVERT tt:'.' TO '' IN SIZE2
			tt ^= "*/-,";
			size2.converter(tt, tt.length().space());
			size2.trimmer();
			size2.converter(" ", "X");
			//obsolete code never used - force error
			xx = 3 / 0;
			if (not((size2.index("X") and size2.field("X", 1).isnum()) and size2.field("X", 2).isnum())) {
				msg = "PLEASE ENTER A SIZE - EG \"10X5\"";
				call note(msg);
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

	} else if (mode == "VAL.SPEC2") {
		var spec = RECORD.a(21, win.mvx);
		rcard = "";
		goto valspec;

	} else if (mode == "VAL.SPEC") {
		if (win.is == win.isorig) {
			return 0;
		}

		gosub chkupd();
		if (not(win.valid)) {
			return 0;
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
			if ((tt == "") or ((lnx ne win.mvx and (((tt == vcode) and (RECORD.a(21, lnx) == spec)))))) break;;
				lnx += 1;
			}//loop;
			if (tt) {
				//MSG='PLEASE COMBINE LINES WITH THE SAME SPEC|FOR THE SAME VEHICLE INTO ONE LINE|(OTHERWISE IT IS MORE DIFFICULT TO CHECK SUPPLIER INVOICES)'
				//GOSUB NOTE
			}
		}

		//split into two parts
		if (spec.index(",")) {
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
			return invalid(msg);
		}

		if (not rcard) {
			gosub getratecard();
		}

		//degressive rate
		//locate brand code if possible in vehicle rates
		//otherwise locate client code and blank brand code
		var degrrate = "";
		var nlines = rcard.a(7).count(VM) + 1;
		clientcode = calculate("BILLING_CLIENT_CODE");
		var client;
		if (not(client.read(agy.clients, clientcode))) {
			client = "";
		}

		for (lnx = 1; lnx <= nlines; ++lnx) {
			if (rcard.a(7, lnx) == clientcode) {
				tt = rcard.a(10, lnx);
				if (not degrrate) {
					degrrate = tt;
				}
				if ((rcard.a(8, lnx) == "") or (rcard.a(8, lnx) == RECORD.a(11))) {
					if (tt) {
						degrrate = tt;
					}
				}
			}
		};//lnx;

		//sizemult: x size OBSOLETE CODE
		var mult = 1;
		var size = spec1.field("*", 2);
		if (size) {
			spec1.splicer(-size.length(), size.length(), "");
			if ((size.index("X") and size.field("X", 1).isnum()) and size.field("X", 2).isnum()) {
				mult = size.field("X", 1) * size.field("X", 2);
			}else{
				msg = DQ ^ (size ^ DQ) ^ " - SIZE SHOULD BE ENTERED|AS numberXnumber (EG \"10X5\")";
				return invalid(msg);
			}
		}

		//find the row number
		if (not(rcard.a(20).locateusing(spec1, VM, rown))) {
			if (rcard.a(20)) {
				msg = DQ ^ (spec1 ^ DQ) ^ " is not on the rate card.|";
				msg.r(-1, "YOU WILL HAVE TO ENTER THE|GROSS UNIT BILL YOURSELF");
				call note(msg);
			}
			gosub calcubill();
			gosub calcucost();
			goto valspecexit;
		}

		//duration must be specified for broadcast vehicle
		if (spec2 == "") {
			//tv satellite radio cinema
			if (var("TSRC").index(vehicle.a(2)[1])) {
				msg = "Broadcast vehicle spec should be Segment \",\" Duration|(30\" USED)";
				call note(msg);
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
					if ((tt.substr(1,tt.length() - 1)).isnum()) {
						mult = tt.substr(1,tt.length() - 1);
						coln = 1;
					}else{

	// for coln=1 to ncols
	// locate tt in colcodes<1,coln> using '|' setting loaded else null
	// until loaded
	// next coln
	// if loaded else

						msg = DQ ^ (tt ^ DQ) ^ " - NOT FOUND IN RATE CARD COLUMNS";
						call note(msg);
						coln = 1000000;
	// end
					}
				}
			}
		}

		if (not(gen.currency.read(gen.currencies, vehicle.a(5)))) {
			gen.currency = "";
		}
		ndecs = gen.currency.a(3);
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
			call note(msg);
		}else{
			rate = (rate * mult).oconv(currfmt);
		}

		RECORD.r(37, win.mvx, rate);
		tt = win.registerx(1).a(37);
		gosub EOF_1615();
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
						if loadamt2 then if loadamt='' or loadamt>tt then loadamt=loadamt2;
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
				call note(msg);
			}
		}else{
			//IF MULT NE 1 THEN costRATE=costRATE/MULT
			costrate = (costrate * mult).oconv(currfmt);
		}

		RECORD.r(41, win.mvx, costrate);
		tt = win.registerx(1).a(41);
		gosub EOF_1615();
		RECORD.r(21, win.mvx, spec);
		gosub calcucost();

		//promopub: negative commission means copy g/nunitcost to gunitbill
		brandcode = RECORD.a(11);

		gosub getcommfee();

		if (win.registerx(6) and (win.registerx(6) < 0)) {
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
		if (win.wi == win.registerx(1).a(21)) {
			gosub EOF_544();
		}

	} else if (mode.substr(1,13) == "UPDATEEXTRAS2") {

		reply = 2;
		var forceotherfee = "";
		goto updateextras;

	} else if (mode == "UPDATEEXTRAS") {

		gosub chkupd();
		if (not(win.valid)) {
			return 0;
		}

		if (not(decide("Get standard discounts and charges ?", "Both|Client side|Agency side", reply))) {
			return 0;
		}

		var forceotherfee = "";

updateextras:
		var storemv = win.mvx;
		var updbillext = reply ne 3;
		var updcostext = reply ne 2;
		var nmvs = RECORD.a(20).count(VM) + 1;
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
				gosub val.vehicle();
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
			tt=WIS<cEXTRAS.FN>;gosub r;
	*/

	} else if (mode == "RECALC") {
		win.displayaction = 5;
		win.reset = 3;

	} else if (mode == "GETEXCHRATES") {
		gosub getexchrates();

	} else if (mode == "RECALC_ALL_NETS") {
		var oldmv = win.mvx;

		var nmvs = RECORD.a(20).count(VM) + 1;
		for (var win.mvx = 1; win.mvx <= nmvs; ++win.mvx) {
			gosub calcubill();
			gosub calcucost();
		};//win.mvx;

		win.mvx = oldmv;

	} else {
		msg = DQ ^ (mode ^ DQ) ^ " - INVALID MODE IGNORED";
		call note(msg);

	}
//L6592:
	return 0;

}

subroutine getratecard() {
	//these are all wrong now that loading column is inserted before disc
	if (calculate("SPLITEXTRAS")) {
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
		var idate = ("1/" ^ calculate("PERIOD")).iconv("D/E");
		if (not(vehicle.a(8).locateby(vcode ^ "*" ^ idate, "DR", daten))) {
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
	//called in PREWRITE
	var gunitbill = RECORD.a(cgunitbillfn, win.mvx);
	if (mode == "VAL.SPEC") {
		tspec = win.is;
	}else{
		tspec = RECORD.a(21, win.mvx);
	}
	//size=field(field(tspec,'*',2),',',1)
	var extras = RECORD.a(cextrasfn, win.mvx);

	//do the calculation
	var unitbill = calcbill(gunitbill, tspec, extras, unused, msg, rndg, multx, breakdown);
	if (msg) {
		call note("IN LINE: " ^ win.mvx ^ "|" ^ msg);
	}

	//allow for manual entry of net unit bill alone
	if (not unitbill and not gunitbill) {
		unitbill = RECORD.a(cunitbillfn, win.mvx);
		//breakdown=''
		breakdown = unitbill;
	}

	//prevent negative result for unit BILL
	//but allow for unit COST to enable credit notes from suppliers
	if ((cgunitbillfn ne 41 and unitbill) and (unitbill < 0)) {
		if (not(RECORD.ucase().index("NEGATIVE"))) {
			msg = "NET UNIT AMOUNT CANNOT BE NEGATIVE, WITHOUT COMMENT|(" ^ (DQ ^ (unitbill ^ DQ)) ^ " IN LINE " ^ win.mvx ^ ")";
			gosub invalid(msg);
			gunitbill = 0;
			unitbill = 0;
			breakdown = "";
		}
	}

	//update net unit bill and redisplay
	RECORD.r(cunitbillfn, win.mvx, unitbill);
	if (mode ne "RECALC_ALL_NETS") {
		tt = win.registerx(1).a(cunitbillfn);
		gosub EOF_1615();
	}
	// IF cUNIT.BILL.FN=38 THEN tt=TOT.BILL.WI ELSE tt=TOT.cost.wi
	// gosub r

	//update breakdown
	breakdown.converter(FM, SVM);
	tt = gunitbill ^ SVM ^ breakdown;
	//call trimexcessmarks(t)
	//loop while tt[-1,1]=sm do tt[-1,1]='' repeat
	call trim(tt, SVM, "B");
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
		if (not(vehicle.read(agy.vehicles, vcode))) {
			return;
		}
		vehiclecurrcode = vehicle.a(5);
		RECORD.r(45, win.mvx, vehiclecurrcode);
		tt = win.registerx(1).a(45);
		gosub EOF_1615();
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
		if (gen.currency.read(gen.currencies, vehiclecurrcode)) {
			var datex = ("1/" ^ RECORD.a(12)).iconv("D/E");
			if (not(gen.currency.a(4).locateby(datex, "DR", tt))) {
				{}
			}
			rate = gen.currency.a(5, tt);
			if (not rate) {
				rate = gen.currency.a(5, tt - 1);
			}
			if (not rate) {
				msg = "NOTE:|" ^ (DQ ^ (vehiclecurrcode ^ DQ)) ^ " - NO EXCHANGE RATE|(YOU WILL HAVE TO ENTER IT)";
				call note(msg);
			}
			costrate = rate;
		}else{
			msg = DQ ^ (vehiclecurrcode ^ DQ) ^ " - CURRENCY NOT ON FILE";
			call note(msg);
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
					var datex = ("1/" ^ RECORD.a(12)).iconv("D/E");
					if (not(gen.currency.a(4).locateby(datex, "DR", tt))) {
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
						call note(msg);
						rate = "";
					}
				}else{
					msg = DQ ^ (invcurrcode ^ DQ) ^ " - NOT ON FILE";
					call note(msg);
					rate = "";
				}
			}
		}

		//look for rate on client file
		var client;
		if (client.read(agy.clients, calculate("BILLING_CLIENT_CODE"))) {
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
	tt = win.registerx(1).a(40);
	gosub EOF_1615();
	tt = win.registerx(1).a(44);
	gosub EOF_1615();

	return;

}

subroutine chkinv() {
	gosub chkupd();
	if (RECORD.a(4)) {
		msg = "THE SCHEDULE HAS ALREADY BEEN INVOICED";
		return invalid(msg);
	}
	return;

}

subroutine getcommfee() {

	discs = "";
	var nlines = rcard.a(7).count(VM) + 1;
	agtfee = "";
	agtfeeng = "";
	win.registerx(6) = "";
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
	for (lnx = lnx; lnx <= nlines; ++lnx) {
		//14/8/97 IF rcard<7,lnx>=CLIENT.CODE THEN
		if ((rcard.a(7, lnx) == clientcode) or (rcard.a(7, lnx) == "*")) {
			tt = rcard.a(10, lnx);
			if ((rcard.a(8, lnx) == "") or (rcard.a(8, lnx) == brandcode)) {

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
							call note(msg);
						}
					}
					if (disc) {
						if (not(var("-+").index(disc[1]))) {
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
					win.registerx(6) = tt;
					commng = rcard.a(62, lnx);
				}
			}
		}
	};//lnx;

	//use the public commission if there no special commission for this client
	if (win.registerx(6) == "") {
		win.registerx(6) = rcard.a(1);
		commng = rcard.a(6);
	}
	commng.converter("Nn", "");

	//990729 allow for "net plus" clients
	if ((client.a(12) == "NN") and costrcard.a(16)) {
		win.registerx(6) = costrcard.a(16);
	}

	return;

}


libraryexit()
