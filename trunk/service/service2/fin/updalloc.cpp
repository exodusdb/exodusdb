#include <exodus/library.h>
libraryinit()

#include <split.h>
#include <addcent.h>
#include <updvoucher.h>

#include <fin.h>
#include <gen.h>

var currcode;
var tt;//num
var selfalloc;//num
var invoice;
var allocbase;//num
var gainbase;//num
var allocyearperiod;
var revalvoucherpointer;
var revaldate;
var revalpayln;

var origmode;
var invoicepointer;
var paymentpointer;
var payn;
var invln;
var acc;
var invoicecode;

function main(io mode, io payment, in paymentcode, io allocs) {
	//y2k

	//mode
	//blank (work out allocations and update voucher and write invoices
	//REALLOC
	//UNPOST (like blank but doesnt write invoices or complain if invoices are unposted

//init:

	origmode = mode;
	//CURR.YEARPERIOD=CURRYEAR:('00':CURRPERIOD)[-2,2]
	var currtoyearperiod = fin.currtoyear ^ var("00" ^ fin.currtoperiod).substr(-2, 2);

	//get the payment voucher if necessary
	if (not payment) {
		if (not(payment.read(fin.vouchers, paymentcode))) {
			if (mode ne "UNPOSTED") {
				call mssg("SYSTEM ERROR IN UPD.ALLOC|PAYMENT VOUCHER " ^ (DQ ^ (paymentcode ^ DQ)) ^ " IS MISSING|(ALLOCATION NOT DONE)");
			}
			return 0;
		}
		if (payment.a(7, 1) == "D") {
			if (mode ne "UNPOSTED") {
				call mssg("SYSTEM ERROR IN UPD.ALLOC|PAYMENT VOUCHER " ^ (DQ ^ (paymentcode ^ DQ)) ^ " IS NOT POSTED/DELETED|(ALLOCATION NOT DONE)");
			}
			return exit();
		}
	}

	//payment voucher period/year (from voucher date if not specified)
	var paymentyearperiod = payment.a(16);
	if (not paymentyearperiod) {
		paymentyearperiod = ((payment.a(2)).oconv("HEX")).oconv(gen.company.a(6));
	}

//initpay:
	var npays = (allocs.a(1)).count(VM) + 1;

//nextpay:
	for (var payn=1; payn<=npays; ++payn) {

		if (not(allocs.a(1, payn))) {
			continue;//next pay
		}

		//skip lines of other companies (not necessary?)
		acc = payment.a(8, payn, 2);
		if (not acc) {
			acc = payment.a(8, payn, 1);
		}
		var acccompany = acc.field(",", 2);
		if (acccompany and mode ne "UNPOSTED") {
			if (acccompany ne fin.currcompany) {
				continue; //nextpay;
			}
		}

//payinit:

		paymentpointer = paymentcode.field("*", 1, 2);
		if (payn > 1) {
			paymentpointer ^= "*" ^ payn;
		}

		var ninvs = (allocs.a(1, payn)).count(SVM) + 1;

		//see if foreign currency
		var amount = split(payment.a(10, payn), currcode);
		var temp = payment.a(19, payn);
		//IF TEMP THEN AMOUNT-=SUM(TEMP)
		if (temp) {
			amount = (amount - temp.sum()).oconv("MD40P");
		}
		var foreign = currcode ne fin.basecurrency;

		//if foreign currency the calculate allocations in base amount
		if (foreign) {
			var baseamount = payment.a(11, payn);
			temp = payment.a(20, payn);
			//IF TEMP THEN BASE.AMOUNT-=SUM(TEMP)
			//garbagecollect;
			if (temp) {
				baseamount = (baseamount - temp.sum()).oconv(fin.basefmt);
			}
			for (var invn = 1; invn <= ninvs; ++invn) {
				var allocamount = allocs.a(2, payn, invn);
				if (amount) {
					tt = (baseamount * (allocamount / amount)).oconv(fin.basefmt);
				}else{
					tt = 0;
				}
				allocs.r(3, payn, invn, tt);
			};//invn;

			//if currency completely allocated then ensure that base is
			//by adding any rounding errors to the first allocation
			if (not(amount - (allocs.a(2, payn)).sum())) {
				temp = baseamount - (allocs.a(3, payn)).sum();
				//garbagecollect;
				//IF TEMP THEN ALLOCS<3,PAYN,1>=ALLOCS<3,PAYN,1>+TEMP
				//sjb971111
				if (temp) {
					allocs.r(3, payn, 1, (allocs.a(3, payn, 1) + temp).oconv(fin.basefmt));
				}
			}
		}


//nextinv:
		for (var invn=1; invn<= ninvs; ++invn) {

			//get the invoice and line number
			invoicepointer = allocs.a(1, payn, invn);
			invoicecode = invoicepointer.field("*", 1, 2) ^ "*" ^ fin.currcompany;
			if (not invoicecode) {
				continue;//nextinv//nextinv
			}
			//IF field(INVOICE.CODE,'*',1) EQ 'RV' THEN GOTO NEXTINV
			//skip unallocated mass revaluations in open item accounts
			if (invoicecode == "RV") {
				continue;//nextinv
			}
			invln = invoicepointer.field("*", 3);
			if (not invln) {
				invln = 1;
			}
			if (invoicecode == paymentcode) {
				//N.B. make sure all changes to PAYMENT are copied to INVOICE and vice versa
				selfalloc = 1;
				invoice = payment;
			}else{
				selfalloc = 0;
				var invoice;
				if (not(invoice.read(fin.vouchers, invoicecode))) {
					if (mode ne "UNPOSTED") {
						call mssg("SYSTEM ERROR IN UPD.ALLOC|INVOICE VOUCHER " ^ (DQ ^ (invoicecode ^ DQ)) ^ " IS MISSING|(ALLOCATION NOT DONE)");
					}
					continue;//nextinv
				}
				//if invoice<7,1>='D' and mode<>'UNPOSTED' then
				if (invoice.a(7, 1) == "D") {
					if (mode ne "UNPOSTED") {
						call mssg("SYSTEM ERROR IN UPD.ALLOC|INVOICE VOUCHER " ^ (DQ ^ (invoicecode ^ DQ)) ^ " IS NOT POSTED/DELETED|(ALLOCATION NOT DONE)");
					}
					continue;//nextinv
				}
			}

//invinit:

			var allocamount = allocs.a(2, payn, invn);

			//get the base allocation and the gain/loss in base
			if (not foreign) {
				allocbase = allocamount;
				gainbase = "";
			}else{
				//calculate base allocation as a proportion of the unallocated base
				//get currency outstanding
				allocbase = allocs.a(3, payn, invn);
				var unallocamount = split(invoice.a(10, invln), temp);
				temp = invoice.a(19, invln);
				if (temp) {
					unallocamount -= temp.sum();
				}
				//get the base amount outstanding
				var unallocbase = invoice.a(11, invln);
				temp = invoice.a(20, invln);
				if (temp) {
					unallocbase -= temp.sum();
				}

				//calculate the proportion
				//garbagecollect;
				unallocamount = -unallocamount;
				if (allocamount - unallocamount) {
					if (unallocamount) {
						tt = (-(unallocbase * allocamount / unallocamount)).oconv(fin.basefmt);
					}else{
						tt = 0;
					}
					var invallocbase = tt;
					gainbase = (invallocbase - allocbase).oconv(fin.basefmt);

					//adjust inv alloc base if rounding difference
					if (gainbase.abs() <= .01) {
						invallocbase = (invallocbase - gainbase).oconv(fin.basefmt);
						gainbase = 0;
					}

				}else{
					var invallocbase = -unallocbase;
					gainbase = (invallocbase - allocbase).oconv(fin.basefmt);
				}
			}

			//determine the allocation period as the higher of the two voucher periods
			// and not before the closed period
			//**removed 23/2/93 (and, if reallocating, not less than the current period)
			var invoiceyearperiod = invoice.a(16);
			if (not invoiceyearperiod) {
				invoiceyearperiod = ((invoice.a(2)).oconv("HEX")).oconv(gen.company.a(6));
			}
			if (addcent(invoiceyearperiod) > addcent(paymentyearperiod)) {
				allocyearperiod = invoiceyearperiod;
			}else{
				allocyearperiod = paymentyearperiod;
			}

			//reinstated 20070616 to prevent way back allocations esp. to avoid revaluation
			if (origmode == "REALLOC") {
				//IF addcent(ALLOC.YEARPERIOD) LT addcent(CURR.YEARPERIOD) THEN
				// ALLOC.YEARPERIOD=CURR.YEARPERIOD
				if (addcent(allocyearperiod) < addcent(currtoyearperiod)) {
					allocyearperiod = currtoyearperiod;
				}
			}

			//5 feb 97 alloc period cannot be before closed period
			var minperiod = gen.company.a(16);
			if (minperiod) {
				var minyear = minperiod.substr(-2, 2);
				minperiod = minperiod.field("/", 1) + 1;
				if (minperiod > fin.maxperiod) {
					minperiod = 1;
					minyear = (minyear + 1).oconv("R(0)#2");
				}
				var minyearperiod = minyear ^ var("00" ^ minperiod).substr(-2, 2);
				if (addcent(allocyearperiod) < addcent(minyearperiod)) {
					allocyearperiod = minyearperiod;
				}
			}

			//cross allocate the invoice and payment

			//a) the invoice
			//garbagecollect;
			temp = invoice.a(17, invln);
			var invaln = temp.count(SVM) + (temp ne "") + 1;
			invoice.r(17, invln, invaln, paymentpointer);
			invoice.r(19, invln, invaln, -allocamount);
			invoice.r(20, invln, invaln, (-allocbase).oconv(fin.basefmt));
			if (invoiceyearperiod ne allocyearperiod) {
				invoice.r(23, invln, invaln, allocyearperiod);
			}
			if (selfalloc) {
				payment = invoice;
			}

			//b) the payment
			var existingalloc = payment.a(17, payn);
			var invn2 = existingalloc.count(SVM) + (existingalloc ne "") + 1;
			payment.r(17, payn, invn2, allocs.a(1, payn, invn));
			payment.r(19, payn, invn2, allocamount);
			payment.r(20, payn, invn2, allocbase);
			if (paymentyearperiod ne allocyearperiod) {
				payment.r(23, payn, invn2, allocyearperiod);
			}
			if (selfalloc) {
				invoice = payment;
			}

			//revalue the invoice if necessary
			if (gainbase) {

				//the additional posting may be done in two ways

				//A) if the allocation period is the same as the payment period
				//either 1) the payment base amount is adjusted and allocated
				//or 2) an extra line is posted to the account and allocated
				//in either case an extra line is added to the voucher
				// of base currency only to the exchange gain/loss account

				//(this method avoids a base currency entry on the open item account
				//but if already posted will require reposting)

				//B) if the allocation period is NOT the same as the allocated voucher
				//a base currency only posting is made between this account and the
				//gain/loss account to balance the allocation made
				//this is similar to the method used in revaluation)

				if (paymentyearperiod == allocyearperiod and gen.company.a(11) < 2) {
					gosub sameperiod(mode,payment);
				}else{
					gosub diffperiod(payment);
				}

			//8) adjust the invoice base outstanding by allocation to the above item
				//garbagecollect;
				invaln += 1;
				invoice.r(17, invln, invaln, revalvoucherpointer);
				invoice.r(19, invln, invaln, "0");
				invoice.r(20, invln, invaln, (-gainbase).oconv(fin.basefmt));
				if (invoiceyearperiod ne allocyearperiod) {
					invoice.r(23, invln, invaln, allocyearperiod);
				}
				if (selfalloc) {
					payment = invoice;
				}
			}

//invexit:
			//write the invoice back to file
			if (not selfalloc and mode ne "UNPOSTED") {
				invoice.write(fin.vouchers, invoicecode);
			}

		}

//payexit:

	}//next pay

	return exit();

}

function exit(){
	return 0;
}

subroutine diffperiod(in payment) {
	//post a base currency only journal
	// 1. DB ACCNO BASE 0XXX
	// 2. CR REALISED GAIN/LOSS " "
	var revalvoucher = "";

	if (origmode == "REALLOC") {
		//reval date is last date of alloc period
		//or today if today if less but still in same period
		revaldate = allocyearperiod.iconv(gen.company.a(6));
		if (revaldate > var().date()) {
			if ((var().date()).oconv(gen.company.a(6)) == allocyearperiod) {
				revaldate = var().date();
			}
		}
	}else{
		revalvoucher.r(16, allocyearperiod);
		revaldate = (payment.a(2)).oconv("HEX");
	}

	revalvoucher.r(2, revaldate.iconv("HEX"));
	revalvoucher.r(8, acc ^ VM ^ gen.company.a(4));
	var temp = "0" ^ currcode;
	revalvoucher.r(10, temp ^ VM ^ temp);
	//garbagecollect;
	revalvoucher.r(11, gainbase ^ VM ^ (-gainbase).oconv(fin.basefmt));
	temp = invoicecode.field("*", 1, 2);
	if (invln ne 1) {
		temp ^= "*" ^ invln;
	}
	revalvoucher.r(17, temp);
	temp.converter("*", "-");
	revalvoucher.r(13, temp);
	revalvoucher.r(19, "0");
	revalvoucher.r(20, gainbase);
	var revalvouchercode = "RV**" ^ fin.currcompany;
	var xx="";
	call updvoucher("", revalvoucher, revalvouchercode,xx);

	//the invoice will point back to the revaluation voucher
	revalvoucherpointer = revalvouchercode.field("*", 1, 2);
	return;

}

subroutine sameperiod(io mode, io payment) {

	//indicate to the calling program that the voucher is adjusted
	if (origmode == "REALLOC") {
		mode = "";
	}

	var npaylns = payment.a(8).dcount(VM);

	//post the gain to the account and allocate the gain to the invoice
	//(either adjust the line directly or add a new line)
	if (gen.company.a(11)) {
		npaylns += 1;
		revalpayln = npaylns;
		payment.r(8, revalpayln, acc);
		payment.r(10, revalpayln, 0 ^ currcode);
		payment.r(11, revalpayln, gainbase);
	}else{
		revalpayln = payn;
		payment.r(11, revalpayln, (payment.a(11, revalpayln) + gainbase).oconv(fin.basefmt));
	}
	//allocation
	var temp = payment.a(17, revalpayln);
	temp = temp.count(SVM) + (temp ne "") + 1;
	payment.r(17, revalpayln, temp, invoicepointer);
	payment.r(19, revalpayln, temp, "0");
	payment.r(20, revalpayln, temp, gainbase);

	//post the gain to realised exchange gains/losses
	//(add a line to the payment)
	//garbagecollect;
	npaylns += 1;
	payment.r(8, npaylns, gen.company.a(4));
	payment.r(10, npaylns, "0" ^ currcode);
	payment.r(11, npaylns, (-gainbase).oconv(fin.basefmt));

	//invoice will point back to this payment line
	revalvoucherpointer = paymentpointer.field("*", 1, 2);
	if (revalpayln ne 1) {
		revalvoucherpointer ^= "*" ^ revalpayln;
	}

	if (selfalloc) {
		invoice = payment;
	}
	return;

}


libraryexit()
