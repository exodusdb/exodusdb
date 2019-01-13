#include <exodus/library.h>
libraryinit()

#include <chklic.h>
#include <authorised.h>
#include <agencysubs.h>
#include <generalsubs2.h>
#include <locking.h>
#include <sysmsg.h>
#include <addjobacc.h>
#include <trim2.h>
#include <initcompany.h>
#include <lockposting.h>
#include <getagp.h>
#include <updvoucher2.h>
#include <flushindex.h>
#include <split.h>
#include <singular.h>

#include <gen.h>
#include <agy.h>
#include <fin.h>
#include <win.h>

#include <window.hpp>

var jobaccnoprefix;
var backdateamendments;//num
var invnobycompany;
var msg;
var locklist;
var tt;
var prodtype;
var currcode;
var taxn;//num
var newrec;
var deleting;//num
var analysisfnx;//num
var brandcode;
var suppliercode;
var marketcode;
var supplier;
var jobno;
var job;
var xx;
var prodinv;
var orderourinvno;
var isinvoiced;//num
var tinv;
var prodtypecodes;
var amounts;
var amountbases;
var namounts;
var taxamounts;
var taxamountbases;
var utaxcodes;
var utaxableamounts;
var utaxableamountbases;
var utaxamounts;
var utaxamountbases;
var taxcodes;
var taxperc;//num
var taxamount;//num
var taxamountbase;//num
var amount;//num
var amountbase;//num
var invamount;
var invamountbase;
var firstprodtypecode;
var firstprodtype;
var accno;
var companycode;
var analperiod;
var pendinganalysis;
var curranalysis;
var ourinvno;
var invoicekey;
var auditkey;
var accountingdate;
var vn;//num
var costanalysis;
var jobperiodanalysis;
var prepvoucher;//num
var voucher;
var reference;
var currencycode;
var taxcode;
var details;
var minconsolidatelinen;//num
var analysiscodex;
var invindex;
var inv;
var analfn;//num
var voucherln;//num
var linecurrencycode;
var analn;//num
var analrec;
var brand;
var wsmsg;

function main(in mode) {
	//c job in

	//global invnobycompany,backdateamendments,companycode,amount,amounts,amountbase,amountbases,taxamountbase,taxamountbases
	//global accno,supplier,firstprodtypecode,firstprodtype,namounts,prodtypecodes,taxcodes
	//global brandcode,suppliercode,marketcode,jobaccnoprefix,utaxcodes,utaxableamounts
	//global utaxableamountbases,utaxamounts,utaxamountbases,auditkey,invamount,invamountbase
	//global inv,analysisfnx,taxamounts,invindex,prepvoucher

	jobaccnoprefix = "JOB";
	var interactive = not SYSTEM.a(33);
	//for taxes

	//synchronise backdateamendments in agency.subs and prodorder.subs
	backdateamendments = 0;

	invnobycompany = agy.agp.a(48);

	//curranalysis<1>=mv analkeys
	//curranalysis<2>=analperiod
	//curranalysis<3>=mv billbase (not applicable in production orders/invoices)
	//curranalysis<4>=mv costbase

	//EQU PO.DATE.FN TO 1
	//EQU PO.JOB.NO.FN TO 2
	//EQU PO.AMOUNT.FN TO 3
	//EQU PO.CURR.CODE TO 4
	//EQU PO.EXCH.RATE.FN TO 5
	//EQU PO.DESC.FN TO 6
	//EQU PO.INV.NO.FN TO 7
	//EQU PO.ACNO.FN TO 8

	if (mode == "VALIDATEORDERESTIMATE") {
		//common code for production orders and estimates
		gosub validateorderestimate();

	} else if (mode == "PREWRITE") {

		//prevent creating new records if no lic
		call chklic(mode, msg);
		if (msg) {
			return invalid(msg);
		}

		//non-draft mode prevented in client but lets make sure
		if (RECORD.a(11) ne "DRAFT") {
			if (not(authorised("PRODUCTION ORDER ISSUE", msg))) {
				return invalid(msg);
			}
			if (agy.agp.a(139) and calculate("TOTAL_BASE") >= agy.agp.a(139)) {
				if (not(authorised("PRODUCTION ORDER ISSUE LARGE", msg))) {
					return invalid(msg);
				}
			}
		}

		locklist = "";

		//fix a bug in ui which allows readonly fields to be entered sometime
		//our invoice number
		RECORD.r(16, win.orec.a(16));

		//ensure any internal column copied from estimates is cleared.
		RECORD.r(20, "");

		//check production type
		tt = RECORD.a(15);
		if (tt.a(1, 1) == "") {
			while (true) {
			///BREAK;
			if (not(tt[1] == VM)) break;;
				tt.splicer(1, 1, "");
			}//loop;
			if (tt.a(1, 1)) {
				RECORD.r(17, 1, tt.a(1, 1));
			}else{
				tt = calculate("JOB_PRODUCTION_TYPE");
				if (tt) {
					if (not prodtype.read(agy.jobtypes, tt)) {
						prodtype = "";
					}
					if (not prodtype.a(5)) {
						goto badtype;
					}
				}else{
badtype:
					msg = "Production type is required because it|is not specified on the job record OR|it does not have an income account no";
					return invalid(msg);
				}
			}
		}

		//check currency exists and is not stopped
		currcode = RECORD.a(4);
		if (currcode ne win.orec.a(4)) {
			if (not gen.currency.read(gen.currencies, currcode)) {
				msg = DQ ^ (currcode ^ DQ) ^ " currency does not exist";
				return invalid(msg);
			}
			tt = gen.currency.a(1);
			tt.converter("<>", "()");
			if (gen.currency.a(25) or (tt.ucase()).index("(STOP)", 1)) {
				msg = tt ^ FM ^ "currency is stopped" ^ FM ^ gen.currency.a(25);
				return invalid(msg);
			}
		}

		//check taxcodes setup properly with a/c nos
		//identical code in PRODINV.SUBS and PRODORDER.SUBS2
		var alltaxcodes = RECORD.a(42);
		if (alltaxcodes) {
			tt = calculate("AMOUNT");
			var nlns = tt.count(VM) + (tt ne "");
			var defaulttaxcode = RECORD.a(44);
			if (defaulttaxcode) {
				alltaxcodes ^= VM ^ defaulttaxcode;
				nlns += 1;
			}
			for (var ln = 1; ln <= nlns; ++ln) {
				var taxcodex = alltaxcodes.a(1, ln);
				if (taxcodex.length()) {
					if (not fin.taxes.a(2).locateusing(taxcodex, VM, taxn)) {
						{}
					}
					if (fin.taxes.a(4, taxn) == "") {
						msg = "Tax A/c has not been setup for tax code " ^ (DQ ^ (taxcodex ^ DQ));
						return invalid(msg);
					}
				}else{
					if (not defaulttaxcode) {
						msg = "Tax/VAT Code is required in footing or line " ^ ln;
						return invalid(msg);
					}
				}
			};//ln;
		}

		//save for restoration
		var saverecord = RECORD;
		var saveorec = win.orec;

		call agencysubs("CHKCLOSEDPERIOD." ^ mode, msg);
		if (msg) {
			return invalid(msg);
		}

		//VALIDATE

		//validate if can delete previous record
		if (win.orec) {
			RECORD.transfer(newrec);
			win.orec.transfer(RECORD);
			deleting = 1;
			gosub validate();
			RECORD = saverecord;
			win.orec = saveorec;
			if (not win.valid) {
				gosub unlockall();
				return 0;
			}
		}

		//validate if can update new record
		deleting = 0;
		newrec = RECORD;
		gosub validate();
		RECORD = saverecord;
		win.orec = saveorec;
		if (not win.valid) {
			gosub unlockall();
			return 0;
		}

		//DELETE

		//delete previous record
		if (win.orec) {

			RECORD.transfer(newrec);
			win.orec.transfer(RECORD);
			deleting = 1;

			//call validate again to setup ... should not fail
			gosub validate();
			if (not win.valid) {
				RECORD = saverecord;
				win.orec = saveorec;
				gosub unlockall();
				return 0;
			}

			gosub update();
			RECORD = saverecord;
			win.orec = saveorec;
			if (not win.valid) {
				gosub unlockall();
				return 0;
			}

		}

		//UPDATE

		//update new record
		deleting = 0;
		newrec = RECORD;

		//call validate again to setup - should never fail
		gosub validate();
		if (not win.valid) {
			RECORD = saverecord;
			win.orec = saveorec;
			gosub unlockall();
			return 0;
		}

		gosub update();
		if (not win.valid) {
			RECORD = saverecord;
			win.orec = saveorec;
			gosub unlockall();
			return 0;
		}

		gosub unlockall();

		//update log and version file
		call generalsubs2(mode);

	} else if (mode == "PREDELETE") {

		msg = "Orders cannot be deleted. Change the status to cancelled.";
		return invalid(msg);

	}
//L1160:
	return 0;

}

subroutine validate() {

	//raise additional costs if job billed
	//keep list of outstanding purchase orders for the job

	analysisfnx = 29;
	brandcode = calculate("BRAND_CODE");
	suppliercode = calculate("SUPPLIER_CODE");
	marketcode = calculate("MARKET_CODE");

	//needed for account number
	if (not supplier.read(agy.suppliers, suppliercode)) {
		msg = DQ ^ (suppliercode ^ DQ) ^ " supplier does not exist";
		gosub errexit();
		return;
	}

	gosub validateorderestimate();
	if (not win.valid) {
		return;
	}

	if (not(locking("LOCK", "JOBS", jobno, "", locklist, 3, msg))) {
		gosub errexit();
		return;
	}

	//prevent addition/deletion from/to closed jobs
	//if deleting else
	if (not(authorised("PRODUCTION ORDER CREATE AFTER JOB CLOSED", msg, "UP2"))) {
		if (job.a(7) == "Y") {
			gosub errexit();
			return;
		}
	}
	// end

	//make sure inv no and date are entered ... or neither
	//if (@record<7>='' and @record<9><>'') or (@record<7><>'' and @record<9>='') then
	var t1 = RECORD.a(7) == "" and RECORD.a(9) ne "";
	var t2 = RECORD.a(7) ne "" and RECORD.a(9) == "";
	if (t1 or t2) {
		msg = "BOTH INVOICE NUMBER AND DATE MUST BE ENTERED OR NEITHER";
		gosub errexit();
		return;
	}

	//basic checks done
	//no further processing for cancelled records
	if (var("DRAFT,CANCELLED").locateusing(RECORD.a(11), ",", xx)) {
		return;
	}

	//people can close and reopen jobs manually now
	//so ONLY presence of an invoice indicates whether costs have been posted or not
	//jobisclosed=(job<7>='Y')
	//orderourinvno=@record<16> *moved down
	var orderestno = RECORD.a(35);
	var jobourinvnos = job.a(5);

	if (orderestno) {
		//get invoice number from estimate (in case they changed estimate number)
		//isinvoiced=(xlate('PRODUCTION_INVOICES',orderestno,10,'X') ne '')
		if (not prodinv.read(agy.productioninvoices, orderestno)) {
			msg = DQ ^ (orderestno ^ DQ) ^ " estimate is missing in PRODORDER.SUBS";
			call sysmsg(msg);
			gosub errexit();
			return;
		}
		orderourinvno = prodinv.a(10);
		RECORD.r(16, orderourinvno);
		isinvoiced = orderourinvno ne "";

		//if job was changed then order estimate number is cleared
		//TODO if they change and before saving, restore job no in UI - what happens?
	}else{

		orderourinvno = RECORD.a(16);

		//make sure the invoice belongs to the job number otherwise remove
		//since user must have changed the job number in UI and not entered a new Est No
		//NB we dont want this production order looking like it has been invoiced
		//and skipped from cost generation if and when the new job is invoiced
		//as it was in NEOSYS up to 2014/07/01
		if (orderourinvno and not deleting) {
			var invkey = orderourinvno;
			if (agy.agp.a(48)) {
				invkey = invkey.fieldstore("*", 3, 1, calculate("COMPANY_CODE"));
			}
			if (not tinv.read(agy.invoices, invkey)) {
				msg = DQ ^ (invkey ^ DQ) ^ " missing from invoices";
				call sysmsg(msg);
				gosub errexit();
				return;
			}
			if (tinv.a(11) ne jobno) {
				RECORD.r(16, "");
				orderourinvno = "";
			}
		}

		isinvoiced = jobourinvnos ne "";
	}
	//2007/07/16 changing po on manually closed job generated accounting entries
	//but certainly should not
	//reqaccounting=(jobisclosed or isinvoiced or orderourinvno)
	//reqaccounting=(isinvoiced or orderourinvno)
	if (orderourinvno) {
		isinvoiced = 1;
	}

	//check can add an account
	if (not deleting and (isinvoiced or calculate("SUPP_INV_NO"))) {
		call addjobacc("VALIDATE", jobno, msg);
		if (msg) {
			gosub errexit();
			return;
		}
	}

	var analid = "";

	fin.currfmt = "MD" ^ calculate("NDECS") ^ "0P";

	//multitype
	//COST={TOTAL_AMOUNT}
	//cost.base={AMOUNT_BASE}
	//IF DELETING THEN
	// COST=-COST
	// cost.base=-cost.base
	// END
	MV = 0;
	prodtypecodes = calculate("OLD_PRODUCTION_TYPE");
	amounts = calculate("AMOUNT");
	amountbases = calculate("AMOUNT_BASE");
	namounts = amounts.count(VM) + (amounts ne "");

	if (deleting) {

		//reverse amounts
		amounts.transfer(tt);
		gosub reverse();
		tt.transfer(amounts);

		//reverse bases
		amountbases.transfer(tt);
		gosub reverse();
		tt.transfer(amountbases);

	}

	//calculate and save tax amounts

	taxamounts = "";
	taxamountbases = "";

	utaxcodes = "";
	var utaxpercs = "";
	utaxableamounts = "";
	utaxableamountbases = "";
	utaxamounts = "";
	utaxamountbases = "";

	taxcodes = calculate("LINE_TAX_CODE");
	var defaulttaxcode = calculate("TAX_CODE");
	if (taxcodes or defaulttaxcode) {

		//use previous tax codes and percents if any
		utaxcodes = calculate("UTAX_CODES");
		utaxpercs = calculate("UTAX_PERC");

		//work out linewise tax amounts and base
		for (var amountn = 1; amountn <= namounts; ++amountn) {

			var taxableamount = amounts.a(1, amountn);
			if (taxableamount) {

				var taxableamountbase = amountbases.a(1, amountn);

				//get taxcode
				var taxcodex = taxcodes.a(1, amountn);
				if (taxcodex == "") {
					taxcodex = defaulttaxcode;
					taxcodes.r(1, amountn, taxcodex);
				}

				//get taxperc
				if (utaxcodes.locateusing(taxcodex, VM, taxn)) {
					taxperc = utaxpercs.a(1, taxn);
				}else{
					utaxcodes.r(1, taxn, taxcodex);
					if (fin.taxes.a(2).locateusing(taxcodex, VM, xx)) {
						taxperc = fin.taxes.a(3, xx);
					}else{
						taxperc = 0;
						call mssg(DQ ^ (taxcodex ^ DQ) ^ " missing from Tax File");
						gosub errexit();
						return;
					}
					utaxpercs.r(1, taxn, taxperc);
				}

				//calculate taxamount
				//garbagecollect;
				taxamount = (amounts.a(1, amountn) * taxperc / 100).oconv(fin.currfmt);
				taxamounts.r(1, amountn, taxamount);

				//calculate taxamountbase
				//garbagecollect;
				taxamountbase = (amountbases.a(1, amountn) * taxperc / 100).oconv(agy.agp.a(3));
				taxamountbases.r(1, amountn, taxamountbase);

				//save per taxcode (for wip if split)
				//garbagecollect;
				utaxableamounts.r(1, taxn, (utaxableamounts.a(1, taxn) + taxableamount).oconv(fin.currfmt));
				utaxableamountbases.r(1, taxn, (utaxableamountbases.a(1, taxn) + taxableamountbase).oconv(fin.currfmt));
				utaxamounts.r(1, taxn, (utaxamounts.a(1, taxn) + taxamount).oconv(fin.currfmt));
				utaxamountbases.r(1, taxn, (utaxamountbases.a(1, taxn) + taxamountbase).oconv(agy.agp.a(3)));

			}
		};//amountn;

	}

	//this INCLUDES blank tax code if any!
	RECORD.r(22, utaxpercs);
	RECORD.r(40, utaxcodes);
	RECORD.r(41, utaxableamounts);
	RECORD.r(43, utaxamounts);

	//garbagecollect;
	amount = (amounts.sum()).oconv(fin.currfmt);
	amountbase = (amountbases.sum()).oconv(agy.agp.a(3));

	//calculate tax on supplier invoices
	if (calculate("SUPP_INV_NO") and utaxcodes) {

		//calculate total taxamount and taxamountbase
		//garbagecollect;
		taxamount = (taxamounts.sum()).oconv(fin.currfmt);
		taxamountbase = (taxamountbases.sum()).oconv(agy.agp.a(3));

		//dont calculate tax on cost accruals (or reversal of accruals during po2pi)
	}else{

		taxamount = "";
		taxamountbase = "";

		taxcodes = "";
		taxamounts = "";
		taxamountbases = "";

	}

	//garbagecollect;
	invamount = amount;
	if (taxamount.length()) {
		invamount.r(1, 1, 6, taxamount);
	}
	invamountbase = amountbase;
	if (taxamountbase.length()) {
		invamountbase.r(1, 1, 6, taxamountbase);
	}

	//get the production type record
	firstprodtypecode = (trim2(calculate("OLD_PRODUCTION_TYPE"), VM, "F")).a(1, 1);
	//multitype
	if (not firstprodtype.read(agy.jobtypes, firstprodtypecode)) {
		msg = DQ ^ (firstprodtypecode ^ DQ) ^ " production type is missing";
		gosub errexit();
		return;
	}

	//get client or brand account number
	accno = calculate("CLIENT_ACCNO");
	//client.accno=field(accno<1,1,1>,',',1)

	//decide the company code
	//from the client or brand account number
	companycode = job.a(14);
	if (companycode == "") {
		companycode = accno.a(1, 1, 1).field(",", 2);
	}
	//otherwise from the production type (no longer since compcode is req on job)
	//multitype
	if (companycode == "") {
		companycode = firstprodtype.a(9);
	}
	//otherwise from the default for production
	if (companycode == "") {
		companycode = agy.agp.a(19);
	}
	//otherwise the same as for media
	if (companycode == "") {
		companycode = agy.agp.a(39);
	}
	if (companycode == "") {
		call mssg("COMPANY CODE HAS NOT BEEN|SETUP YET - CANNOT INVOICE");
		gosub errexit();
		return;
	}

	call initcompany(companycode);

	//ensure tax code on purchase invoices from 1/1/2018 if registered for tax
	if (gen.company.a(21) and calculate("INV_DATE") >= 18264 and not deleting) {
		if (not RECORD.a(42) or RECORD.a(44)) {
			msg = DQ ^ (ID ^ DQ) ^ " Tax/VAT code is required because|" ^ gen.company.a(1) ^ " has Tax Reg.No." ^ gen.company.a(21);
			gosub errexit();
			return;
		}
	}

	//if outstanding order then update relevant analysis column
	if (calculate("SUPP_INV_NO") == "") {

		var orderperiod = ((calculate("DATE")).oconv("D2/E")).substr(4,5);

		//multitype
		analperiod = orderperiod;
		gosub lockandaccumulateanalysis();
		if (msg) {
			gosub errexit();
			return;
		}
		pendinganalysis = curranalysis;

	}

	///////////////////////////////////////////////////////////////
	//no postings for purchase orders if the job has not been closed
	///////////////////////////////////////////////////////////////
	if (calculate("SUPP_INV_NO") == "" and not isinvoiced) {
		return;
	}

	if (isinvoiced) {

		//!990707 SJB costs are to be analysed at the date of the last invoice
		//not any more
		ourinvno = RECORD.a(16);
		if (ourinvno == "") {
			ourinvno = (field2(job.a(5), VM, -1)).field(".", 1);
		}

		invoicekey = ourinvno;

		//read invoice from invoices,invoice.key:'**':company.code then
		// bycompany=1
		//end else
		// bycompany=0
		// end

	}else{
		invoicekey = "J" ^ jobno;

		//read invoice from invoices,invoice.key:'.1':'**':company.code then
		// bycompany=1
		//end else
		// bycompany=invnobycompany
		// end

	}

	//cost adjustments go into the invoices file as
	// last invoice number.1 .2 .3 etc
	//or, if job as never been invoiced then as job number.1 .2 .3 etc
	//find the last invoice revision number
	for (var ii = 1; ii <= 9999; ++ii) {

		var temp = invoicekey ^ "." ^ ii;
		//if bycompany then temp:='**':company.code
		if (invnobycompany) {
			temp ^= "**" ^ companycode;
		}

		if (not tt.read(agy.invoices, temp)) {
			invoicekey ^= "." ^ ii;
			//if bycompany then invoice.key:='**':company.code
			if (invnobycompany) {
				invoicekey ^= "**" ^ companycode;
			}
			ii = 9999;
		}

	};//ii;

	//determine and lock the audit.key
	auditkey = "%PRODUCTION%";
	if (invnobycompany) {
		if (not tt.read(agy.invoices, auditkey)) {
			auditkey.splicer(-1, 0, "*" ^ gen.gcurrcompany);
		}
		tt = "";
	}
	if (not(locking("LOCK", "INVOICES", auditkey, "", locklist, 9, msg))) {
		gosub errexit();
		return;
	}

	//use the supplier invoice date if at all possible
	//otherwise use the order date
	//IF DELETING THEN
	// *this has no effect if truely deleting but
	// *causes reversal in new supp inv date period if amending the supp inv date
	// accountingdate=NEWREC<PO.INV.DATE.FN>
	//END ELSE
	// *this causes reinsertion in original purchase invoice date
	// *ie amendments get inserted back dated (but this is in conflict with the above)
	// accountingdate=OREC<PO.INV.DATE.FN>
	// END
	//check method agrees with CHKCLOSEDPERIOD in agency.subs
	if (backdateamendments) {
		accountingdate = win.orec.a(9);
	}else{
		//NB not @record (because during reversal stage of amendment @record is orc)
		accountingdate = newrec.a(9);
	}
	if (not accountingdate) {
		accountingdate = calculate("INV_DATE");
	}
	//IF accountingdate ELSE accountingdate={DATE}
	if (not accountingdate) {
		accountingdate = var().date();
	}

	//refresh exchange rate
	if (currcode ne agy.agp.a(2)) {
		//call currexch(accountingdate,currcode,base.currency.code,1,'',xrate,msg)
		if (not(gen.currency.a(4).locateby(accountingdate, "DR", vn))) {
			{}
		}
		var xrate = gen.currency.a(5, vn);
		if (xrate == "") {
			xrate = gen.currency.a(5, vn - 1);
		}
		RECORD.r(5, xrate);
	}

	if (isinvoiced) {

		//readv our.inv.date from invoices,our.inv.no:'**':company.code,2 else
		// readv our.inv.date from invoices,our.inv.no,2 else
		// msg='ERROR - OUR INVOICE NUMBER ':quote(our.inv.no):' IS MISSING - CANNOT CONTINUE'
		// gosub errexit
		// return
		// end
		// end
		//analdate=our.inv.date

		//multitype
		analperiod = (accountingdate.oconv("D2/E")).substr(4,5);
		if (analperiod[1] == "0") {
			analperiod.splicer(1, 1, "");
		}
		gosub lockandaccumulateanalysis();
		if (msg) {
			gosub errexit();
			return;
		}
		costanalysis = curranalysis;

		//need to analyse by job period as well
		analperiod = job.a(1);
		gosub lockandaccumulateanalysis();
		if (msg) {
			gosub errexit();
			return;
		}
		jobperiodanalysis = curranalysis;

		//save our invoice number
		//not any more ... so that we can know the analysis period later in case of amendments
		//and other reasons like knowing if accounting needs to be done
		RECORD.r(16, ourinvno);

	}

	if (isinvoiced or calculate("SUPP_INV_NO")) {
		if (not(lockposting("LOCK", locklist, 9, msg))) {
			gosub errexit();
			return;
		}
	}

	if (agy.agp.a(102) and calculate("SUPP_INV_NO") and not calculate("SUPP_ACNO")) {
		msg = "You must setup the supplier A/c No. on the Supplier File|before entering supplier invoices for";
		msg ^= FM ^ FM ^ (DQ ^ (calculate("SUPPLIER_NAME") ^ DQ)) ^ " (" ^ calculate("SUPPLIER_CODE") ^ ")" ^ FM;
		msg ^= FM ^ "(Finance journals are to be posted directly)";
		gosub errexit();
		return;
	}

	return;

}

subroutine update() {

	if (var("DRAFT,CANCELLED").locateusing(calculate("STATUS"), ",", xx)) {
		return;
	}

	var origjob = job;

	//create a new job account
	//always do this if entering pi directly into finance journals
	//hopefully will not create too many unused job a/c
	//if isinvoiced or {SUPP_INV_NO} then
	call addjobacc("ADD", jobno, msg);
	// end

	//no postings for purchase orders if the job has not been closed
	///////////////////////////////////////////////////////////////
	if (calculate("SUPP_INV_NO") == "" and not isinvoiced) {
		goto updatenoinvoice;
	}

	//UPD.ACCOUNTS:
	/////////////

	// IF JOB OPEN IF JOB CLOSED
	// and our inv='' or our inv<>''
	// ----------- -------------
	// PRODUCTION ORDER - DR COSTS - TOT EX VAT
	// - CR ACCRUED COSTS - TOT EX VAT
	//
	// PURCHASE INVOICE DR WIP/JOB PER LINE DR COSTS PER LINE
	// VAT IN PARALLEL VAT IN PARALLEL
	// CR SUPPLIER INC VAT CR SUPPLIER
	//

	//IF COST OR cost.base THEN
	prepvoucher = 1;
	if (prepvoucher) {

		//prepare voucher

		//determine the voucher type
		var vouchertype = agy.agp.a(85);

		var vouchercode = vouchertype ^ "**" ^ companycode;
		voucher = "";
		voucher.r(2, accountingdate.iconv("HEX"));
		if (calculate("SUPP_INV_NO")) {
			reference = "INV" ^ calculate("SUPP_INV_NO");
		}else{
			reference = "ORD" ^ ID;
		}
		voucher.r(13, reference);

		currencycode = calculate("CURR_CODE");

		//prepare first line to supplier/wip
		//garbagecollect;
		amount = (amounts.sum()).oconv(fin.currfmt);
		amountbase = (amountbases.sum()).oconv(agy.agp.a(3));
		taxcode = "";
		taxamount = "";
		taxamountbase = "";
		//taxamount=sum(taxamounts) CURRFMT
		//taxamountbase=sum(taxamountbases) BASE.CURRENCY.FMT

		details = "~ Job:" ^ jobno ^ ", Order:" ^ ID ^ ", " ^ calculate("DESC");
		if (deleting) {
			details.splicer(1, 0, "Reversal ");
		}

		//do not consolidate costs into income line
		minconsolidatelinen = 1;

		//invoice
		if (calculate("SUPP_INV_NO")) {

			//prepare first line to supplier/wip
			//garbagecollect;
			amount = (amounts.sum() + taxamounts.sum()).oconv(fin.currfmt);
			amountbase = (amountbases.sum() + taxamountbases.sum()).oconv(agy.agp.a(3));
			taxcode = "";
			taxamount = "";
			taxamountbase = "";

			//credit supplier
			//add first voucher line
			//accno={SUPP_ACNO}
			accno = supplier.a(9);
			//if accno='' then accno=suppliercode
			//if accno='' then swap '~' with {SUPPLIER_NAME} in details
			if (accno == "") {
				accno = "SUPPLIER " ^ calculate("SUPPLIER_NAME");
			}
			analysiscodex = "";
			gosub addrevline();

			if (isinvoiced) {

creditcostspertype:

				//debit cost a/c per type
				//multitype
				var currlineprodtypecode = "";
				var lineprodtypecode = firstprodtypecode;
				var lineprodtype = firstprodtype;
				for (var amountn = 1; amountn <= namounts; ++amountn) {

					tt = prodtypecodes.a(1, amountn);
					if (tt) {
						lineprodtypecode = tt;
					}
					if (lineprodtypecode ne currlineprodtypecode) {
						if (not lineprodtype.read(agy.jobtypes, lineprodtypecode)) {
							//should never get here since all are checked in validation
							lineprodtype = "";
						}
						currlineprodtypecode = lineprodtypecode;
					}

					amount = amounts.a(1, amountn);
					if (amount) {

						amountbase = amountbases.a(1, amountn);
						taxcode = taxcodes.a(1, amountn);
						taxamount = taxamounts.a(1, amountn);
						taxamountbase = taxamountbases.a(1, amountn);

						//should be similar in prodorder.subs and prodinvs3 in two places
						accno = lineprodtype.a(6);
						if (accno and lineprodtype.a(9)) {
							//add company code
							accno.r(1, 1, 1, (accno.a(1, 1, 1)).fieldstore(",", 2, 1, lineprodtype.a(9)));
						}
						//if accno='' then ACCNO=lineprodtype<5>
						if (accno == "") {
							accno = agy.agp.a(42);
						}
						if (accno == "") {
							accno = "TYPE " ^ lineprodtypecode;
						}

						analysiscodex = brandcode ^ "**" ^ marketcode ^ "*" ^ suppliercode ^ "*" ^ lineprodtypecode;

						gosub addline();

					}

				};//amountn;

			}else{

				//debit total cost to work in progress a/c

				//should be similar in prodorder.subs and prodinvs3

				var wipaccno = getagp(agy.agp.a(43), calculate("PERIOD"));
				wipaccno.swapper("%SUPPLIER%", suppliercode);
				if (wipaccno == "") {
					wipaccno = jobno;
					if (jobno.isnum()) {
						wipaccno.splicer(1, 0, jobaccnoprefix);
					}
				}

				//work in progress HAS vat entries WITHOUT analysis

				//if no tax codes then one entry in wip
				if (utaxcodes == "") {
					accno = wipaccno;
					//garbagecollect;
					amount = (amounts.sum()).oconv(fin.currfmt);
					amountbase = (amountbases.sum()).oconv(agy.agp.a(3));
					taxcode = "";
					taxamount = "";
					taxamountbase = "";
					//taxamount=sum(taxamounts) CURRFMT
					//taxamountbase=sum(taxamountbases) BASE.CURRENCY.FMT
					analysiscodex = "";
					gosub addline();

					//if tax codes then one entry in wip per tax code
				}else{
					var ntaxes = utaxcodes.count(VM) + 1;
					for (taxn = 1; taxn <= ntaxes; ++taxn) {

						accno = wipaccno;
						amount = utaxableamounts.a(1, taxn);
						amountbase = utaxableamountbases.a(1, taxn);

						if (amount.length() or amountbase.length()) {

							taxcode = utaxcodes.a(1, taxn);
							taxamount = utaxamounts.a(1, taxn);
							taxamountbase = utaxamountbases.a(1, taxn);

							analysiscodex = "";
							gosub addline();

						}

					};//taxn;
				}
			}

			//GOSUB POSTVOUCHER

			var().osflush();
			tt = "";
			if (agy.agp.a(102)) {
				tt = "POST";
			}

			call updvoucher2(tt, voucher, vouchercode, xx, agy.agp.a(102));

			if (job.a(14) ne "") {
				job.r(14, companycode);
			}

			//order after invoiced to client
		}else{

			if (isinvoiced) {

				//credit the total cost to the wip/accrual account
				accno = getagp(agy.agp.a(14), calculate("PERIOD"));
				accno.swapper("%SUPPLIER%", suppliercode);
				if (accno == "") {
					accno = jobno;
					if (jobno.isnum()) {
						accno.splicer(1, 0, jobaccnoprefix);
					}
					if (not accno.a(1, 1, 2)) {
						accno.r(1, 1, 2, accno.a(1, 1, 1));
					}
				}
				analysiscodex = "";
				gosub addrevline();

				//rest credits cost per type just like supplier invoice with r
				goto creditcostspertype;

			}
		}
	}

	//update invoice index
	/////////////////////

	if (not invindex.read(agy.invoices, auditkey)) {
		invindex = "";
	}
	//discard the first/oldest 100 entries if inv index too large
	while (true) {
	///BREAK;
	if (not(invindex.length() > 32000)) break;;
		invindex = invindex.field(FM, 100, 99999);
	}//loop;
	invindex.r(-1, invoicekey);
	invindex.write(agy.invoices, auditkey);

	//update invoice
	///////////////

	inv = "";
	inv.r(1, brandcode ^ "*" ^ calculate("PERIOD"));
	inv.r(2, accountingdate);
	inv.r(6, job.a(5, 1));

	//changes to accrued cost only
	if (isinvoiced) {
		inv.r(7, invamount);
		inv.r(8, calculate("CURR_CODE"));
		inv.r(4, invamountbase);
	}else{
		inv.r(12, invamount);
		inv.r(13, calculate("CURR_CODE"));
		inv.r(14, invamountbase);
	}

	inv.r(9, calculate("DESC"));
	inv.r(5, var().date());
	inv.r(11, jobno);
	//inv<18>={TAX_PERC}
	inv.r(46, utaxableamounts);
	inv.r(47, utaxcodes);
	inv.r(48, utaxamounts);
	inv.r(49, utaxamountbases);
	inv.r(19, brandcode);
	inv.r(21, "P");
	if (isinvoiced) {

	/*;
		//inv<26>=prodtypecode
			inv<22>=suppliercode;
			inv<23>=0;
			inv<24>=cost.base;
			inv<26>=prodtypecode;
	*/

		//multitype
		//strip off some of the analkeys to suit the invoice analcode format
		var analcodes = "";
		var nanalkeys = (costanalysis.a(1)).count(VM) + 1;
		for (var analkeyn = 1; analkeyn <= nanalkeys; ++analkeyn) {
			analcodes.r(1, analkeyn, costanalysis.a(1, analkeyn).field("*", 5, 2));
		};//analkeyn;
		inv.r(22, analcodes);
		inv.r(24, costanalysis.a(4));

		//costanalperiod
		inv.r(25, costanalysis.a(2));

		inv.r(38, ID);

	}

	inv.write(agy.invoices, invoicekey);

	call flushindex("INVOICES");

	//SIMILAR CODE IN PRODORDER.SUBS and CREATEANALYSIS

	//update cost analysis
	/////////////////////

	//multitype
	//no analysis for orders or invoices unless job has been billed
	if (isinvoiced) {

		curranalysis = costanalysis;
		analfn = 2;
		gosub updateanalysis();

		curranalysis = jobperiodanalysis;
		analfn = 27;
		gosub updateanalysis();

	}

updatenoinvoice:
	////////////////

	//update pending analysis
	////////////////////////

	//if outstanding order then update relevent analysis column
	//regardless of whether invoiced to client or not
	if (calculate("SUPP_INV_NO") == "") {

		curranalysis = pendinganalysis;
		analfn = 12;
		gosub updateanalysis();

	}

	//update job
	///////////

	//JOB<5,-1>=field(invoice.key,'*',1)

	//add order to or remove, from the list of outstanding PO on the billed job
	// the list of outstanding PO on the billed job
	if (job.a(6).locateusing(ID, VM, vn)) {
		if (calculate("SUPP_INV_NO") or deleting) {
			job.eraser(6, vn);
		}
	}else{
		if (not(calculate("SUPP_INV_NO") and not deleting)) {
			job.r(6, -1, ID);
		}
	}

	if (job ne origjob) {
		job.write(agy.jobs, jobno);
	}

	/////
	//EXIT:
	/////
	return;

}

subroutine unlockall() {
	call locking("UNLOCKALL", "", "", "", locklist, 0, msg);
	return;

}

subroutine reverse() {
	var nn = tt.count(VM) + 1;
	for (var ii = 1; ii <= nn; ++ii) {
		var tt2 = tt.a(1, ii);
		if (tt2.length()) {
			tt2 = "-" ^ tt2;
			if (tt2.substr(1,2) == "--") {
				tt2.splicer(1, 2, "");
			}
			tt.r(1, ii, tt2);
		}
	};//ii;
	return;

}

subroutine addline() {

	if (analysiscodex) {
		//analysisfnx*1*brandcode**marketcode*suppliercode*typecode

		//for the time being sent over total amount not split
		//into gross,loading,discount,commission,fee,tax,other
		//later on can perhaps put another column in voucher ensuring total=amount

		//2015 always 1, or 9 for rebate - similar code in PRODINVS3 and PRODORDER.SUBS
		//2 was jobs 30/12/2009
		analysiscodex = analysisfnx ^ "*1*" ^ analysiscodex;
		//analysiscodex=analysisfnx:'*2*':analysiscodex

		//1 28/29 income/cost
		//2 1/2 media/jobs
		//3 brand code
		//4 vehicle code (media)
		//5 market code (jobs)
		//6 supplier code (jobs)
		//7 job type code (jobs)
		//8 executive code
		analysiscodex = analysiscodex.fieldstore("*", 8, 1, (calculate("EXECUTIVE_CODE")).ucase());
	}

	if (accno and accno.a(1, 1, 2) == "") {
		accno.r(1, 1, 2, accno.a(1, 1, 1).field(",", 1));
	}

	if (accno and details) {
		if (fin.accounts.open("ACCOUNTS", "")) {
			tt = accno.a(1, 1, 2);
			if (not tt) {
				tt = accno.field(",", 1);
			}
			if (fin.account.read(fin.accounts, tt)) {
				details.swapper("~", fin.account.a(1));
			}
		}
	}

	if (voucher.a(8).locateusing(accno, VM, voucherln)) {
		while (true) {
			xx = split(voucher.a(10, voucherln), linecurrencycode);
			//until voucher<8,voucherln>='' or (voucher<8,voucherln>=accno and line.currency.code=currency.code and voucherln>minconsolidatelinen) do
			tt = voucher.a(8, voucherln) == accno and linecurrencycode == currencycode and voucherln > minconsolidatelinen;
		///BREAK;
		if (voucher.a(8, voucherln) == "" or tt) break;;
nextvline:
			voucherln += 1;
		}//loop;

		//dont consolidate if base amount are of different signs
		//to ensure different voucher lines even if income and cost a/cs are the same
		if (voucher.a(11, voucherln) * amountbase < 0) {
			goto nextvline;
		}
		if (voucher.a(8, voucherln) == "") {
			goto newvoucherline;
		}
		if (voucher.a(3, voucherln) ne details) {
			goto nextvline;
		}
		if (voucher.a(37, voucherln) ne analysiscodex) {
			goto nextvline;
		}
		if (voucher.a(24, voucherln) ne taxcode) {
			goto nextvline;
		}

		//garbagecollect;
		voucher.r(10, voucherln, (amount + split(voucher.a(10, voucherln), currencycode)).oconv(fin.currfmt) ^ currencycode);
		voucher.r(11, voucherln, (voucher.a(11, voucherln) + amountbase).oconv(agy.agp.a(3)));
	}else{
newvoucherline:
		while (true) {
		///BREAK;
		if (not voucher.a(10, voucherln)) break;;
			voucherln += 1;
		}//loop;
		if (details) {
			voucher.r(3, voucherln, details);
		}
		voucher.r(8, voucherln, accno);
		voucher.r(10, voucherln, amount ^ currencycode);
		//garbagecollect;
		voucher.r(11, voucherln, amountbase.oconv(agy.agp.a(3)));
	}

	//if taxamount or taxamountbase then
	if (taxcode) {
		voucher.r(24, voucherln, taxcode);

		if (taxamount == "") {
			taxamount = "0";
		}
		if (taxamount.abs() < .0001) {
			taxamount = "0";
		}
		if (taxamount.index("E-", 1)) {
			taxamount = "0";
		}
		//voucher<26,voucherln>=taxamount
		//garbagecollect;
		voucher.r(26, voucherln, (voucher.a(26, voucherln) + taxamount).oconv(fin.currfmt));

		if (taxamountbase == "") {
			taxamountbase = "0";
		}
		if (taxamountbase.abs() < .0001) {
			taxamountbase = "0";
		}
		if (taxamountbase.index("E-", 1)) {
			taxamountbase = "0";
		}
		//voucher<27,voucherln>=taxamountbase
		//garbagecollect;
		voucher.r(27, voucherln, (voucher.a(27, voucherln) + taxamountbase).oconv(agy.agp.a(3)));

	}

	if (analysiscodex) {
		voucher.r(37, voucherln, analysiscodex);
	}

	details = "";
	amount = "";
	amountbase = "";
	taxcode = "";
	taxamount = "";
	taxamountbase = "";
	analysiscodex = "";

	return;

}

subroutine addrevline() {

	//reverse the amounts
	if (amount) {
		amount.splicer(1, 0, "-");
		if (amount.substr(1,2) == "--") {
			amount.splicer(1, 2, "");
		}
	}
	if (amountbase) {
		amountbase.splicer(1, 0, "-");
		if (amountbase.substr(1,2) == "--") {
			amountbase.splicer(1, 2, "");
		}
	}
	if (taxamount) {
		taxamount.splicer(1, 0, "-");
		if (taxamount.substr(1,2) == "--") {
			taxamount.splicer(1, 2, "");
		}
	}
	if (taxamountbase) {
		taxamountbase.splicer(1, 0, "-");
		if (taxamountbase.substr(1,2) == "--") {
			taxamountbase.splicer(1, 2, "");
		}
	}

	gosub addline();

	return;

}

subroutine lockandaccumulateanalysis() {

	//prepares to update analysis. lock and accumulate base amount

	//requires analperiod
	//returns curranalysis - or msg if error

	curranalysis = "";
	msg = "";

	//save period to know which month to update
	curranalysis.r(2, analperiod);

	var currprodtypecode = firstprodtypecode;
	var currprodtype = firstprodtype;
	for (var amountn = 1; amountn <= namounts; ++amountn) {

		//determine typecode
		tt = prodtypecodes.a(1, amountn);
		if (tt and tt ne currprodtypecode) {
			//check exists
			if (not currprodtype.read(agy.jobtypes, tt)) {
				msg = DQ ^ (tt ^ DQ) ^ " production type is missing";
				//gosub errexit
				return;
			}
			currprodtypecode = tt;
		}

		amountbase = amountbases.a(1, amountn);
		taxamountbase = taxamountbases.a(1, amountn);
		if (amountbase or taxamountbase) {

			//determine analysis key
			//TODO ensure job brand/market code changes cannot happen or are handled
			var analkey = analperiod.field("/", 2) ^ "*" ^ brandcode ^ "**" ^ marketcode ^ "*" ^ suppliercode ^ "*" ^ currprodtypecode;
			analkey = analkey.fieldstore("*", 8, 1, companycode);
			if (not curranalysis.a(1).locateusing(analkey, VM, analn)) {
				curranalysis.r(1, analn, analkey);
			}

			//lock
			if (not(locking("LOCK", "ANALYSIS", analkey, "", locklist, 9, msg))) {
				//gosub errexit
				return;
			}

			//merge tax and accumulate
			if (taxamountbase) {
				amountbase.r(1, 1, 6, taxamountbase);
			}
			curranalysis.r(4, analn, (curranalysis.a(4, analn) +++ amountbase).oconv(agy.agp.a(3)));

		}

	};//amountn;

	return;

}

subroutine updateanalysis() {

	//needs curranalysis and analfn

	if (not curranalysis.a(1)) {
		return;
	}

	var analmth = curranalysis.a(2).field("/", 1);

	var nanals = (curranalysis.a(1)).count(VM) + 1;
	for (analn = 1; analn <= nanals; ++analn) {

		var analkey = curranalysis.a(1, analn);

		if (not analrec.read(agy.analysis, analkey)) {
			analrec = "";
		}

		analrec.r(analfn, analmth, (analrec.a(analfn, analmth) +++ curranalysis.a(4, analn)).oconv(agy.agp.a(3)));

		analrec.write(agy.analysis, analkey);

	};//analn;

	return;

}

subroutine validateorderestimate() {

	//get currency for exch rate update
	currcode = RECORD.a(4);
	if (not gen.currency.read(gen.currencies, currcode)) {
		msg = DQ ^ (currcode ^ DQ) ^ " currency does not exist";
		gosub errexit();
		return;
	}

	jobno = calculate("JOB_NO");
	if (not job.read(agy.jobs, jobno)) {
		msg = "JOB NUMBER " ^ jobno ^ " IS MISSING";
		gosub errexit();
		return;
	}
	if (not brand.read(agy.brands, job.a(2))) {
		msg = DQ ^ (job.a(2) ^ " - BRAND IS MISSING" ^ DQ);
		gosub errexit();
		return;
	}

	//identical code in prodinv.subs
	if (RECORD.a(4) == agy.agp.a(2)) {
		//make sure exchange rate for base currency is 1
		if (RECORD.a(5) ne 1) {
			RECORD.r(5, 1);
		}
	}
	if (not RECORD.a(5)) {
		msg = "Exchange rate cannot be blank or zero";
		gosub errexit();
		return;
	}

	//check production type exists
	//if @record<15>='' then
	if (trim2(calculate("OLD_PRODUCTION_TYPE"), VM) == "") {
		msg = "Production type is required";
		gosub errexit();
		return;
	}

	return;

}

subroutine errexit() {
	gosub invalid(msg);
	gosub unlockall();
	win.valid = 0;
	return;

}


libraryexit()
