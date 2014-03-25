#include <exodus/library.h>
libraryinit()

#include <agencysubs.h>
#include <validcode2.h>
#include <validcode3.h>
#include <authorised.h>
#include <btreeextract.h>
#include <generalsubs.h>
//#include <chklic.h>
#include <generalsubs2.h>
#include <sysmsg.h>
#include <addjobacc.h>
#include <initcompany.h>
#include <lockposting.h>
#include <getagp.h>
#include <updvoucher2.h>
#include <split.h>
#include <singular.h>

#include <gen.h>
#include <agy.h>
#include <fin.h>
#include <win.h>

var msg;
var xx;
var temp;
var reply;//num
var tt;
var hits;
var vn;//num
var locklist;
var taxn;
var newrec;
var deleting;//num
var docname;
var copyfile;
var versionfilename;
var orderourinvno;
var isinvoiced;//num
var taxperc;//num
var taxamount;//num
var taxamountbase;//num
var analperiod;
var pendinganalysis;
var curranalysis;
var ourinvno;
var invoicekey;
var bycompany;//num
var accountingdate;
var costanalysis;
var jobperiodanalysis;
var voucher;
var reference;
var currencycode;
var details;
var minconsolidatelinen;//num
var analysiscodex;
var analfn;//num
var voucherln;//num
var linecurrencycode;
var analn;//num
var wsmsg;

function main(in mode) {

	var jobaccnoprefix = "JOB";
	var interactive = not SYSTEM.a(33);
	//for taxes

	//synchronise backdateamendments in agency.subs and prodorder.subs
	var backdateamendments = 0;

	var invnobycompany = agy.agp.a(48);

	//curranalysis<1>=mv analkeys
	//curranalysis<2>=analperiod
	//curranalysis<3>=mv billbase (not applicable in production orders/invoices)
	//curranalysis<4>=mv costbase

	if (mode == "F2.PRODUCTION.ORDERS") {
		call agencysubs(mode);

		//case mode='DEF.ORDER.NO'
		// call general.subs('DEF.SK')
		// defaultorderno=is.dflt

	} else if (mode.field(".", 1, 3) == "DEF.ORDER.NO") {
		//pretty much identical in prodorder.subs, prodinv.subs and even plan.subs
		//call general.subs('DEF.SK')

		if (win.wlocked or RECORD) {
			return 0;
		}

		var previous = 0;

		var compcode = mode.field(".", 4);
		//gosub getnextjobno
		call agencysubs("GETNEXTID." ^ compcode);

		win.isdflt = ANS;
		ANS = "";

		win.registerx[10] = win.isdflt;

		if (not interactive) {
			ID = win.isdflt;
		}

		return 0;

	} else if (mode == "VAL.ORDER.NO") {
		if (win.is == "" or win.is == win.isorig) {
			return 0;
		}

		if (RECORD.read(win.srcfile, win.is)) {

			//x={BRAND_CODE}
			//if validcode('BRAND',{BRAND_CODE},msg) else goto invalid
			if (not(validcode2(calculate("COMPANY_CODE"), calculate("CLIENT_CODE"), calculate("BRAND_CODE"), agy.brands, msg))) {
				return invalid(msg);
			}
			if (not(validcode3(calculate("MARKET_CODE"), calculate("SUPPLIER_CODE"), "", "", msg))) {
				return invalid(msg);
			}
			RECORD = "";

		}else{

			//prevent certain users from creating their own quote numbers
			if (win.is ne win.registerx[10]) {
				if (not(authorised("PRODUCTION ORDER CREATE OWN NO", msg, ""))) {
					msg = DQ ^ (win.is ^ DQ) ^ " does not exist and" ^ FM ^ FM ^ msg;
					return invalid(msg);
				}
			}

			//in case sombody else just used the next key
lockit:
			if (win.is == win.registerx[10]) {
				if (lockrecord(win.datafile, win.srcfile, win.is)) {
					xx = unlockrecord("", win.srcfile, win.is);
				}else{
					if (win.is.isnum()) {
						win.is += 1;
						goto lockit;
					}
				}
			}

			win.is.trimmerf();
			if (win.is.index(" ", 1)) {
				msg = "SORRY, YOU CANNOT USE SPACES HERE";
				return invalid(msg);
			}else{
				gosub EOF_523();
			}

		}

	} else if (mode == "CHOOSECOLS") {

		var prodorderheads = var("PRODORDER.COLHEADS").xlate("DEFINITIONS", 1, "X");
		RECORD.r(13, prodorderheads ^ VM ^ RECORD.a(13));
		RECORD.inserter(3, 1, "");
		win.displayaction = 5;
		win.reset = 3;

	} else if (mode == "VAL.JOB") {
		if (win.is == win.isorig) {
			return 0;
		}

		if (authorised("PRODUCTION ORDER CREATE AFTER JOB CLOSED", msg, "UP2")) {
			temp = "";
		}else{
			temp = ".OPEN";
		}
		//call agency.subs(mode)
		call agencysubs(mode ^ temp);
		if (not win.valid) {
			return 0;
		}

		var jobcode = win.is;
		gosub checkjobunlocked2();
		if (not win.valid) {
			return 0;
		}

		jobcode = win.isorig;
		gosub checkjobunlocked2();
		if (not win.valid) {
			return 0;
		}

		//get the job
		var job;
		if (not(job.read(agy.jobs, win.is))) {
			msg = DQ ^ (win.is ^ DQ) ^ " JOB DOES NOT EXIST";
			return invalid(msg);
		}

		//warning if job closed
		if (job.a(7) == "Y") {
			msg = "WARNING: Job " ^ (DQ ^ (win.is ^ DQ)) ^ " is closed.";
			gosub EOF_487();
		}

		//copy the job description to description, and brief to details
		//or just copy the description and brief to the details
		if (not RECORD.a(13)) {
			if (1) {
				RECORD.r(6, job.a(9, 1));
				RECORD.r(13, job.a(9).field(VM, 2, 999));
			}else{
				RECORD.r(13, job.a(9));
			}

			//update the internal line count for text section
			win.displayaction = 5;
			win.amvaction = 4;
			win.amvvars.r(1, 3, (RECORD.a(13)).count(VM) + (RECORD.a(13) ne ""));
			win.reset = 3;

		}

		//check that there at least one pending approved estimate
		//also in job.subs and prodorder.subs
		var estimatenos = job.a(10);
		var nestimates = estimatenos.count(VM) + (estimatenos ne "");
		var ok = 0;
		var estimatetext = "";
		for (var estimaten = 1; estimaten <= nestimates; ++estimaten) {
			var estimate;
			if (estimate.read(agy.productioninvoices, estimatenos.a(1, estimaten))) {
				var inactivestatuses = "FINISHED" _VM_ "CANCELLED" _VM_ "ON HOLD" _VM_ "INVOICED";
				if (not(inactivestatuses.locate(estimate.a(11), xx, 1))) {
					ok = 1;
				}
			}
		///BREAK;
		if (ok) break;;
			estimatetext ^= FM ^ "Estimate No: " ^ estimatenos.a(1, estimaten);
			estimatetext ^= "  is " ^ estimate.a(11);
		};//estimaten;

		//put warning/error if no pending estimates
		if (not ok) {
			msg = "THERE ARE NO PENDING ESTIMATES FOR JOB " ^ (DQ ^ (win.is ^ DQ));
			msg ^= FM ^ estimatetext ^ FM;
			if (authorised("PRODUCTION ORDER CREATE WITHOUT ESTIMATE", "", "")) {
				msg.splicer(1, 0, "*** WARNING ***" ^ FM ^ FM);
				gosub EOF_487();
			}else{
				return invalid(msg);
			}
		}

	/*;
			READV INVS FROM JOBS,IS,5 ELSE;
				MSG='JOB ':QUOTE(IS):' IS MISSING';
				GOTO INVALID;
				END;
			IF INVS THEN;
				CALL MSG('THIS JOB HAS ALREADY BEEN BILLED TO THE CLIENT');
				IF IS.ORIG AND IS NE IS.ORIG THEN;
					VALID=0;
					RETURN;
					END;
				END;
	*/

	} else if (mode == "VAL.SUPPLIER") {

		call agencysubs("VAL.SUPPLIER.PRODUCTION");

		//check supplier currency
		var supplier;
		if (supplier.read(agy.suppliers, win.is)) {
			if (supplier.a(9)) {
				if (fin.accounts.open("ACCOUNTS", "")) {
					if (fin.account.read(fin.accounts, supplier.a(9, 1, 2))) {
						if (fin.account.a(4)) {
							if (calculate("CURR_CODE") ne fin.account.a(4)) {
//								if (not(decide2("WARNING:|This purchase order currency is " ^ (DQ ^ (calculate("CURR_CODE") ^ DQ)) ^ "|but the suppliers account is currency " ^ (DQ ^ (fin.account.a(4) ^ DQ)), "OK" _VM_ "Cancel", reply, 2))) {
//									reply = 2;
//								}
								reply=2;
								if (reply ne 1) {
									win.valid = 0;
									return 0;
								}
							}
						}
					}
				}
			}
		}

		//called from web interface
	} else if (mode == "VAL.SUPPINV") {
		if (win.is == "" or win.isorig == win.is) {
			return 0;
		}

		if (not win.isorig and not authorised("PRODUCTION SUPPLIER INVOICE CREATE", msg, "")) {
			return invalid(msg);
		}

		if (not win.is and not authorised("PRODUCTION SUPPLIER INVOICE DELETE", msg, "")) {
			return invalid(msg);
		}

		//check supplier has not sent us the same invoice number *zzz intranet?
		tt = "SUPP_INV_NO" ^ VM ^ win.is ^ FM;
		call btreeextract(tt, "PRODUCTION.ORDERS", DICT, hits);

		hits.converter(FM, VM);
		var nhits = hits.count(VM) + 1;
		var maxhits = 100;
		var supplierhits = "";
		var nsupplierhits = 0;
		for (var hitn = 1; hitn <= nhits; ++hitn) {
			var prodorderid = hits.a(1, hitn);

			//skip current if changing suppinvno
			if (prodorderid ne ID) {

				var prodorder;
				if (prodorder.read(win.srcfile, prodorderid)) {
					if (prodorder.a(10) == calculate("SUPPLIER_CODE")) {
						nsupplierhits += 1;
						if (nsupplierhits > maxhits) {
							supplierhits ^= VM ^ "and others";
							hitn = nhits + 1;
						}else{
							supplierhits ^= VM ^ prodorderid;
						}
						//tempmsg=quote(is):' This invoice number has already|been used on production order number ':quote(prodorderid):'|for this supplier'
						//if interactive then
						// if decide2('!WARNING|':tempmsg,'OK|Cancel',reply,2) else reply=2
						//end else
						// *NB the WARNING word is required for a test in the html client.
						// call msg('WARNING: ':tempmsg)
						// reply=2
						// end
						//if reply=2 then
						// valid=0
						// return 0
						// end
					}
				}
			}
		};//hitn;

		if (nsupplierhits) {

			//trim excess leading vm
			supplierhits.splicer(1, 1, "");

			var tempmsg = "Supplier invoice number " ^ (DQ ^ (win.is ^ DQ));
			tempmsg.r(-1, "has already been entered for supplier");
			tempmsg.r(-1, calculate("SUPPLIER_NAME"));
			tempmsg.r(-1, "on order number(s)");
			tempmsg.r(-1, swap(VM, " ", supplierhits));
			if (interactive) {
				if (not(decide2("!WARNING|" ^ tempmsg, "OK" _VM_ "Cancel", reply, 2))) {
					reply = 2;
				}
			}else{
				//NB the WARNING word is required for a test in the html client.
				//call msg('WARNING: ':tempmsg)
				USER3 = "WARNING! " ^ tempmsg;
				return 0;
			}
			if (reply == 2) {
				win.valid = 0;
				return 0;
			}
		}

	if (mode == "VAL.INVDATE") {

	}else if (win.winext > win.wi) {
			if (win.is == "" and RECORD.a(7)) {
				msg = "INVOICE DATE IS REQUIRED";
				return invalid(msg);
			}
		}

		if (win.is == "" or win.is == win.isorig) {
			return 0;
		}
		//consider revising the exchange rate
		if (RECORD.a(5) == "") {
			return 0;
		}
		RECORD.r(9, win.is);
		gosub defexchrate();
		var exchrate = ANS;
		ANS = "";
		if (exchrate) {
			if (RECORD.a(5) ne exchrate) {
				msg = "Note: The exchange rate has been|revised from " ^ RECORD.a(5) ^ " to " ^ exchrate;
				RECORD.r(5, exchrate);
				win.displayaction = 5;
				win.reset = 3;
				gosub EOF_487();
			}
		}

	// CASE MODE='DEF.CURRENCY'
	// IF IS ELSE IS.DFLT=BASE.CURRENCY.CODE

	} else if (mode == "VAL.CURRENCY") {
		if (win.is == win.isorig) {
			return 0;
		}
		call generalsubs("VAL.CURRENCY");
		if (not win.valid) {
			return 0;
		}

		//force recalc of exch rate
		//redisplay.list=wi+1
		win.displayaction = 5;
		win.reset = 3;
		RECORD.r(5, "");

		//get the number of decimals
		var ndecs;
		if (not(ndecs.readv(gen.currencies, win.is, 3))) {
			ndecs = "2";
		}
		RECORD.r(14, ndecs);

	} else if (mode == "VAL.AMOUNT") {
		win.is = win.is.oconv("MD" ^ calculate("NDECS") ^ "0P");

	} else if (mode == "DEF.EXCH.RATE") {
}

subroutine defexchrate() {
		ANS = "";
		var currcode = calculate("CURR_CODE");
		if (not currcode) {
			return;
		}
		//also used by PRODINVS
		if (not(gen.currency.read(gen.currencies, currcode))) {
			msg = "CURRENCY " ^ (DQ ^ (currcode ^ DQ)) ^ " IS MISSING FROM THE CURRENCIES FILE";
			return invalid(msg);
		}
		//use the supplier invoice date by preference
		var idate = calculate("INV_DATE");
		if (idate == "") {
			idate = calculate("DATE");
		}
		if (not(gen.currency.locateby(idate, "DR", vn, 4))) {
			{}
		}
		var rate = gen.currency.a(5, vn);
		if (rate == "") {
			rate = gen.currency.a(5, vn - 1);
		}
		ANS = rate;

	} else if (mode == "POSTINIT") {
		gosub security();
		if (not(authorised("PRODUCTION COST ACCESS", msg, ""))) {
			return invalid(msg);
			var().stop();
		}

	} else if (mode == "PREWRITE") {

		//prevent creating new records if no lic
//TODO		call chklic(mode, msg);
		if (msg) {
			return invalid(msg);
		}

		//non-draft mode prevented in client but lets make sure
		if (RECORD.a(11) ne "DRAFT") {
			if (not(authorised("PRODUCTION ORDER ISSUE", msg))) {
				return invalid(msg);
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
			if (not(tt.substr(1, 1) == VM)) break;;
				tt.splicer(1, 1, "");
			}//loop;
			if (tt.a(1, 1)) {
				RECORD.r(17, 1, tt.a(1, 1));
			}else{
				tt = calculate("JOB_PRODUCTION_TYPE");
				if (tt) {
					var prodtype;
					if (not(prodtype.read(agy.jobtypes, tt))) {
						prodtype = "";
					}
					if (not prodtype.a(5)) {
						goto badtype;
					}
				}else{
					//if 1 then
badtype:
					msg = "Production type is required because it|is not specified on the job record OR|it does not have an income account no";
					return invalid(msg);
				}
			}
		}

		//check currency
		var currcode = RECORD.a(4);
		if (currcode ne win.orec.a(4)) {
			if (not(gen.currency.read(gen.currencies, currcode))) {
				msg = DQ ^ (currcode ^ DQ) ^ " currency does not exist";
				return invalid(msg);
			}
			tt = convert("<>", "()", gen.currency.a(1));
			if (gen.currency.a(25) or (tt.ucase()).index("(STOP)", 1)) {
				msg = tt ^ FM ^ "currency is stopped" ^ FM ^ gen.currency.a(25);
				return invalid(msg);
			}
		}

		//prevent tax if not configured
		var taxcode = RECORD.a(40);
		if (RECORD.a(22) or taxcode.length()) {
			if (taxcode == "") {
				taxcode = agy.agp.a(64);
			}
			if (taxcode == "") {
				taxcode = 1;
			}
			if (not(fin.taxes.locate(taxcode, taxn, 2))) {
				{}
			}
			if (not(fin.taxes.a(4, taxn))) {
				msg = "Tax should be entered as a line item,|not in the footing|because Tax A/c has not been setup";
				return invalid(msg);
			}
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
				goto unlockall;
			}
		}

		//validate if can update new record
		deleting = 0;
		newrec = RECORD;
		gosub validate();
		RECORD = saverecord;
		win.orec = saveorec;
		if (not win.valid) {
			goto unlockall;
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
				goto unlockall;
			}

			gosub update();
			RECORD = saverecord;
			win.orec = saveorec;
			if (not win.valid) {
				goto unlockall;
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
			goto unlockall;
		}

		gosub update();
		if (not win.valid) {
			RECORD = saverecord;
			win.orec = saveorec;
			goto unlockall;
		}

		gosub unlockall();

		//update log and version file
		call generalsubs2(mode);

	} else if (mode == "PREDELETE") {

		msg = "Orders cannot be deleted. Change the status to cancelled.";
		return invalid(msg);

		gosub security();

		if (win.orec.a(7)) {
			if (not(authorised("PRODUCTION SUPPLIER INVOICE DELETE", msg, ""))) {
				return invalid(msg);
			}
		}

		call agencysubs("CHKCLOSEDPERIOD." ^ mode, msg);
		if (msg) {
			return invalid(msg);
		}

		locklist = "";

		//delete record
		deleting = 1;
		newrec = RECORD;
		RECORD = win.orec;
		gosub validate();
		if (not win.valid) {
			gosub unlockall();
			newrec.transfer(RECORD);
			return;
		}
		gosub update();

		gosub unlockall();

	if (mode.substr(1, 8) == "POSTREAD") {

		//option to read previous versions
		call generalsubs2(mode);
	}else if (not win.valid) {
			return;
		}

		gosub security();
		if (not win.valid) {
			return;
		}

		//general cost access restriction
		if (not(authorised("PRODUCTION COST ACCESS", msg, ""))) {
			win.reset = 5;
			return invalid(msg);
		}

		//restrict access based on company and brand
		if (RECORD.a(2)) {
			if (not(validcode2(calculate("COMPANY_CODE"), "", calculate("BRAND_CODE"), agy.brands, msg))) {
				win.reset = 5;
				return invalid(msg);
			}
			if (not(validcode3(calculate("MARKET_CODE"), calculate("SUPPLIER_CODE"), "", "", msg))) {
				win.reset = 5;
				return invalid(msg);
			}
		}

		//option to prevent adding costs after job has been billed
		//zzz but how to delete orders?
		if (win.wlocked and calculate("JOB_CLOSED") == "Y") {
			if (RECORD) {
				if (not(authorised("PRODUCTION ORDER UPDATE AFTER JOB CLOSED", msg, ""))) {
					win.wlocked = 0;
				}
			}else{
				//if security('PRODUCTION ORDER CREATE AFTER JOB CLOSED',msg,'') else
				// wlocked=0
				// end
			}
			if (not win.wlocked) {
				xx = unlockrecord(win.datafile, win.srcfile, ID);
				if (RECORD == "") {
					goto canc;
				}
			}
		}

			/*prevent editing if prepaid and not allowed to update prepaid;
			if wlocked then;
				prepaid={PREPAID}
				if prepaid then;

					if security('PRODUCTION ORDER UPDATE AFTER JOB CLOSED',msg,'') else;
						//if interactive then
							msg[1,0]='NOTE:|YOU CANNOT EDIT THIS ESTIMATE BECAUSE|IT IS PREPAID AND':fm;
							gosub note;
						// end
						xx=unlockrecord(datafile,src.file,@id);
						wlocked=0;

						//////
						return;
						//////

						end;
					end;
				end*/;

			//get the number of decimals
			if @record and @record<14>='' then;
		if (RECORD and RECORD.a(14) == "") {
				@record<14>={NDECS}
			RECORD.r(14, calculate("NDECS"));
				orec<14>=@record<14>;
			win.orec.r(14, RECORD.a(14));
				end;
		}

	/* moved to before validations;
			//if they enter orderno:versionletter
			if @record='' and not(num(@id)) then;
				open 'PRODUCTION.ORDER.VERSIONS' to versionfile then;
					read @record from versionfile,@id then;
						orec=@record;
						xx=unlockrecord('',src.file,@id);

						wlocked=0;
						gosub resetlines;
						return;
						end;
					end;
				end;
	*/

		//prevent certain users from creating their own quote numbers
		if (RECORD == "" and not interactive and not USER3.index("RECORDKEY", 1)) {
			if (not(authorised("PRODUCTION ORDER CREATE OWN NO", msg, ""))) {
				msg = DQ ^ (ID ^ DQ) ^ " does not exist and" ^ FM ^ FM ^ msg;
				win.reset = 5;
				return invalid(msg);
			}
		}

		//check/prevent updating production supplier invoices
		if (win.wlocked and win.orec.a(7)) {
			if (not(authorised("PRODUCTION SUPPLIER INVOICE UPDATE", msg, ""))) {
				//xx=UNLOCKrecord(datafile,SRC.FILE,@ID)
				//WLOCKED=0
				gosub unlockrecord();
			}
		}

		//default signatory into old records
		if (win.orec and RECORD.a(24) == "") {
			if (agy.agp.a(51) and agy.agp.a(51) ne "NEVER") {
				tt = agy.agp.a(61);
				if (tt == "") {
					tt = USERNAME;
				}
			}else{
				tt = "None";
			}
			if (tt == "USER") {
				tt = USERNAME;
			}
			if (tt == "EXECUTIVE") {
				tt = calculate("EXECUTIVE_CODE");
			}
			RECORD.r(24, tt);
		}

		//draft mode only
		if (win.wlocked and win.orec and RECORD.a(11) ne "DRAFT") {
			if (not(authorised("PRODUCTION ORDER ISSUE"))) {
				//wlocked=0
				//xx=unlockrecord(datafile,src.file,@id)
				gosub unlockrecord();
			}
		}

		call agencysubs("CHKCLOSEDPERIOD." ^ mode, msg);
		if (msg) {
			//comment to client
			win.reset = -1;
			gosub EOF_487();
			gosub unlockrecord();
		}

		if (RECORD or mode == "POSTREAD2") {
			return;
		}

		if (not(decide("", "Enter a new purchase order" _VM_ "Copy another purchase order" _VM_ "Copy an estimate", reply))) {
canc:
			win.valid = 0;
			win.reset = 4;
			//wlocked=''
			//xx=unlockrecord(datafile,src.file,@id)
			gosub unlockrecord();
			return;
		}

		if (reply == 1) {
			return;
		}

		if (reply == 2) {
			docname = "order";
			copyfile = win.srcfile;
			versionfilename = "PRODUCTION.ORDER.VERSIONS";
		}else{
			docname = "estimate";
			copyfile = agy.productioninvoices;
			versionfilename = "PRODUCTION.INVOICE.VERSIONS";
		}

		var copyno = "";
copyno:
		call note2("What is the number of the|" ^ docname ^ " that you want to copy ?", "RC", copyno, "");
		if (not copyno) {
			goto canc;
		}
		var copydoc;
		if (not(copydoc.read(copyfile, copyno))) {
			//if docname='estimate' then
			var versionfile;
			if (versionfile.open(versionfilename, "")) {
				if (copydoc.read(versionfile, copyno)) {
					goto gotdoc;
				}
			}
			// end
			msg = DQ ^ (copyno ^ DQ) ^ " - " ^ docname ^ " does not exist";
			return invalid(msg);
			goto copyno;
		}

		var jobcode = copydoc.a(2);
		gosub checkjobunlocked2();
		if (not win.valid) {
			goto copyno;
		}

gotdoc:
		RECORD = copydoc;

		if (copyfile == win.srcfile) {
			RECORD = copydoc;
		}else{
			RECORD = "";
			RECORD.r(2, copydoc.a(2));
			RECORD.r(3, copydoc.a(3));
			RECORD.r(4, copydoc.a(4));
			RECORD.r(5, copydoc.a(5));
			RECORD.r(6, copydoc.a(6));
			RECORD.r(13, copydoc.a(8));
			RECORD.r(14, copydoc.a(14));
			//@record<15>=copydoc<>
		}

initrec:
		//clear supp inv no, date, our invoice number
		RECORD.r(7, "");
		RECORD.r(9, "");
		RECORD.r(16, "");

		//clear log
		for (var ii = 30; ii <= 34; ++ii) {
			RECORD.r(ii, "");
		};//ii;

		//update the internal line count for text section
		win.displayaction = 5;
		win.amvaction = 4;
		win.amvvars.r(1, 3, (RECORD.a(13)).count(VM) + (RECORD.a(13) ne ""));
		win.reset = 4;

		RECORD.r(14, calculate("NDECS"));

		//warn and force reentry of job number if job is closed
		if (calculate("JOB_CLOSED")) {
			//if security('PRODUCTION ORDER CREATE AFTER JOB CLOSED',msg,'') else
			call note(DQ ^ (calculate("JOB_NO") ^ DQ) ^ " Job has been closed.");
			RECORD.r(2, "");
		}

	} else if (mode == "POSTWRITE" or mode == "POSTDELETE") {
//		call flushindex("PRODUCTION.ORDERS");

	} else {
		msg = DQ ^ (mode ^ DQ) ^ " unrecognised mode in prodorder.subs";
		return invalid(msg);
	}

	return 0;

}

subroutine validate() {

	//raise additional costs if job billed
	//keep list of outstanding purchase orders for the job

	var analysisfnx = 29;
	var brandcode = calculate("BRAND_CODE");
	var suppliercode = calculate("SUPPLIER_CODE");
	var marketcode = calculate("MARKET_CODE");

	var jobno = calculate("JOB_NO");
	if (not(locking("LOCK", "JOBS", jobno, "", locklist, 3, msg))) {
errexit:
		gosub EOF_487();
		gosub unlockall();
		win.valid = 0;
		return;
	}

	var job;
	if (not(job.read(agy.jobs, jobno))) {
		msg = "JOB NUMBER " ^ jobno ^ " IS MISSING";
		goto errexit;
	}
	var brand;
	if (not(brand.read(agy.brands, job.a(2)))) {
		msg = DQ ^ (job.a(2) ^ " - BRAND IS MISSING" ^ DQ);
		goto errexit;
	}

	//prevent addition/deletion from/to closed jobs
	//if deleting else
	if (not(authorised("PRODUCTION ORDER CREATE AFTER JOB CLOSED", msg, "UP2"))) {
		if (job.a(7) == "Y") {
			goto errexit;
		}
	}
	// end

	//make sure inv no and date are entered ... or neither
	if (RECORD.a(7) == "" and RECORD.a(9) ne "" or RECORD.a(7) ne "" and RECORD.a(9) == "") {
		msg = "BOTH INVOICE NUMBER AND DATE MUST BE ENTERED OR NEITHER";
		goto errexit;
	}

	//make sure exchange rate for base currency is 1
	if (RECORD.a(4) == agy.agp.a(2) and RECORD.a(5) ne 1) {
		RECORD.r(5, 1);
	}

	//check production type exists
	//if @record<15>='' then
	if (trim2(calculate("OLD_PRODUCTION_TYPE"), VM) == "") {
		msg = "Production type is required";
		goto errexit;
	}

	//basic checks done
	//no further processing for cancelled records
	if (var("DRAFT" _VM_ "CANCELLED").locate(RECORD.a(11), xx, 1)) {
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
		//isinvoiced=(xlate('PRODUCTION.INVOICES',orderestno,10,'X') ne '')
		var prodinv;
		if (not(prodinv.read(agy.productioninvoices, orderestno))) {
			msg = DQ ^ (orderestno ^ DQ) ^ " estimate is missing in PRODORDER.SUBS";
			call sysmsg(msg);
			goto errexit;
		}
		orderourinvno = prodinv.a(10);
		RECORD.r(16, orderourinvno);
		isinvoiced = orderourinvno ne "";
	}else{
		orderourinvno = RECORD.a(16);
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
			if (interactive) {
				if (not(decide2(msg, "Continue (without creating account)" _VM_ "Cancel", reply, 2))) {
					reply = 2;
				}
			}else{
				//reply=1
				goto errexit;
			}
			if (reply == 2) {
				win.valid = 0;
				return;
			}
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
	var prodtypecodes = calculate("OLD_PRODUCTION_TYPE");
	var amounts = calculate("AMOUNT");
	var amountbases = calculate("AMOUNT_BASE");
	var namounts = amounts.count(VM) + (amounts ne "");

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

	//garbagecollect;

	var amount = (amounts.sum()).oconv(fin.currfmt);
	var amountbase = (amountbases.sum()).oconv(agy.agp.a(3));

	//calculate and overall tax percentage if exists
	var taxcode = agy.agp.a(64);
	//dont calculate tax on cost accruals (or reversal of accruals during po2pi)
	if (calculate("SUPP_INV_NO") {
		taxperc = calculate("TAX_PERC");
	}else{
		taxperc = "";
	}
	if (taxperc.length()) {

		//cant do this while tax code config is same for media
		//same in PRODORDER.SUBS and PRODINVS2
		//eg need NO tax code for media and tax code for jobs etc
		//if len(fixedtaxcode) else
		// call msg('Please setup "Invoice tax code" in Jobs Configuration File|before invoicing with tax percent.')
		// stop
		// end
		//if there is a tax perc and no taxcode for now use taxcode 1
		if (not taxcode.length()) {
			taxcode = 1;
		}

		//garbagecollect;
		taxamount = (amount * taxperc / 100).oconv(fin.currfmt);
		taxamountbase = (amountbase * taxperc / 100).oconv(agy.agp.a(3));
	}else{
		taxamount = "";
		taxamountbase = "";
	}

	//garbagecollect;
	var invamount = amount;
	if (taxamount) {
		invamount.r(1, 1, 6, taxamount);
	}
	var invamountbase = amountbase;
	if (taxamountbase) {
		invamountbase.r(1, 1, 6, taxamountbase);
	}

	//work out linewise tax amounts and base
	var taxamounts = "";
	var taxamountbases = "";
	if (taxperc) {
		var lastamountn = 0;

		for (var amountn = 1; amountn <= namounts; ++amountn) {
			if (amounts.a(1, amountn)) {
				lastamountn = amountn;
				//garbagecollect;
				var linetaxamount = (amounts.a(1, amountn) * taxperc / 100).oconv(fin.currfmt);
				taxamounts.r(1, amountn, linetaxamount);
				//garbagecollect;
				var linetaxamountbase = (amountbases.a(1, amountn) * taxperc / 100).oconv(agy.agp.a(3));
				taxamountbases.r(1, amountn, linetaxamountbase);
			}
		};//amountn;

		//add rounding errors to last calculated tax
		//garbagecollect;

		var taxrounding = (taxamount - taxamounts.sum()).oconv(fin.currfmt);
		var taxbaserounding = (taxamountbase - taxamountbases.sum()).oconv(agy.agp.a(3));
		if ((taxrounding or taxbaserounding) and lastamountn) {
			//garbagecollect;
			taxamounts.r(1, lastamountn, (taxamounts.a(1, lastamountn) + taxrounding).oconv(fin.currfmt));
			taxamountbases.r(1, lastamountn, (taxamountbases.a(1, lastamountn) + taxbaserounding).oconv(agy.agp.a(3)));
		}

	}

	//get the production type record
	var firstprodtypecode = trim2(calculate("OLD_PRODUCTION_TYPE"), VM, "F").a(1, 1);
	//multitype
	var firstprodtype;
	if (not(firstprodtype.read(agy.jobtypes, firstprodtypecode))) {
		msg = DQ ^ (firstprodtypecode ^ DQ) ^ " production type is missing";
		goto errexit;
	}

	//get client or brand account number
	var accno = calculate("CLIENT_ACCNO");
	//client.accno=field(accno<1,1,1>,',',1)

	//decide the company code
	//from the client or brand account number
	var companycode = job.a(14);
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
		var().chr(7).output();
		call mssg("COMPANY CODE HAS NOT BEEN|SETUP YET - CANNOT INVOICE");
		goto errexit;
	}

	call initcompany(companycode);

	//if outstanding order then update relevent analysis column
	if (calculate("SUPP_INV_NO") == "") {

		var orderperiod = ((calculate("DATE").oconv("D2/E")).substr(4, 5);

		//multitype
		analperiod = orderperiod;
		gosub lockandaccumulateanalysis();
		if (msg) {
			goto errexit;
		}
		pendinganalysis = curranalysis;

	}

	//no postings for purchase orders if the job has not been closed
	///////////////////////////////////////////////////////////////
	if (calculate("SUPP_INV_NO") == "" and not isinvoiced) {
		return;
	}

	if (isinvoiced) {

		//*990707 SJB costs are to be analysed at the date of the last invoice
		//not any more
		ourinvno = RECORD.a(16);
		if (ourinvno == "") {
			ourinvno = field2(job.a(5), VM, -1).field(".", 1);
		}

		invoicekey = ourinvno;
		var invoice;
		if (invoice.read(agy.invoices, invoicekey ^ "**" ^ companycode)) {
			bycompany = 1;
		}else{
			bycompany = 0;
		}

	}else{
		invoicekey = "J" ^ jobno;
		var invoice;
		if (invoice.read(agy.invoices, invoicekey ^ ".1" "**" ^ companycode)) {
			bycompany = 1;
		}else{
			bycompany = invnobycompany;
		}
	}

	//cost adjustments go into the invoices file as
	// last invoice number.1 .2 .3 etc
	//or, if job as never been invoiced then as job number.1 .2 .3 etc
	//find the last invoice revision number
	for (var ii = 1; ii <= 9999; ++ii) {
		temp = invoicekey ^ "." ^ ii;
		if (bycompany) {
			temp ^= "**" ^ companycode;
		}
		var invoice;
		if (not(invoice.read(agy.invoices, temp))) {
			invoicekey ^= "." ^ ii;
			if (bycompany) {
				invoicekey ^= "**" ^ companycode;
			}
			ii = 9999;
		}
	};//ii;

	//determine and lock the audit.key
	var auditkey = "%PRODUCTION%";
	if (invnobycompany) {
		if (not(tt.read(agy.invoices, auditkey))) {
			auditkey.splicer(-1, 0, "*" ^ gen.gcurrcompany);
		}
		tt = "";
	}
	if (not(locking("LOCK", "INVOICES", auditkey, "", locklist, 9, msg))) {
		goto errexit;
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
	if (not accountingdate) {
		accountingdate = calculate("DATE"));
	}

	if (isinvoiced) {

		//readv our.inv.date from invoices,our.inv.no:'**':company.code,2 else
		// readv our.inv.date from invoices,our.inv.no,2 else
		// msg='ERROR - OUR INVOICE NUMBER ':quote(our.inv.no):' IS MISSING - CANNOT CONTINUE'
		// goto errexit
		// end
		// end
		//analdate=our.inv.date

		//multitype
		analperiod = (accountingdate.oconv("D2/E")).substr(4, 5);
		if (analperiod.substr(1, 1) == "0") {
			analperiod.splicer(1, 1, "");
		}
		gosub lockandaccumulateanalysis();
		if (msg) {
			goto errexit;
		}
		costanalysis = curranalysis;

		//need to analyse by job period as well
		analperiod = job.a(1);
		gosub lockandaccumulateanalysis();
		if (msg) {
			goto errexit;
		}
		jobperiodanalysis = curranalysis;

		//save our invoice number
		//not any more ... so that we can know the analysis period later in case of amendments
		//and other reasons like knowing if accounting needs to be done
		RECORD.r(16, ourinvno);

	}

	if (isinvoiced or calculate("SUPP_INV_NO")) {
		if (not(lockposting("LOCK", locklist, 9, msg))) {
			goto errexit;
		}
	}

	if (agy.agp.a(102)) {
		if (calculate("SUPP_INV_NO")) {
			if (not calculate("SUPP_ACNO")) {
				msg = "You must setup the supplier A/c No. on the Supplier File|before entering supplier invoices for";
				msg ^= FM ^ FM ^ (DQ ^ (calculate("SUPPLIER_NAME") ^ DQ)) ^ " (" ^ calculate("SUPPLIER_CODE") ^ ")" ^ FM;
				msg ^= FM ^ "(Finance journals are to be posted directly)";
				goto errexit;
			}
		}
	}

	return;

}

subroutine update() {

	if (var("DRAFT" _VM_ "CANCELLED").locate(calculate("STATUS")) xx, 1)) {
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

updaccounts:
	/////////////

	// IF JOB OPEN IF JOB CLOSED
	// and our inv='' or our inv<>''
	// ----------- -------------
	// PRODUCTION ORDER - DR COSTS
	// - CR ACCRUED COSTS
	//
	// PURCHASE INVOICE DR WIP (BY JOB) DR COSTS
	// CR SUPPLIER CR SUPPLIER
	//

	//IF COST OR cost.base THEN
	if (1) {

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
		taxamount = (taxamounts.sum()).oconv(fin.currfmt);
		taxamountbase = (taxamountbases.sum()).oconv(agy.agp.a(3));
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
			taxamount = "";
			taxamountbase = "";

			//credit supplier
			//add first voucher line
			accno = var("SUPP_ACNO")();
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
						if (not(lineprodtype.read(agy.jobtypes, lineprodtypecode))) {
							//should never get here since all are checked in validation
							lineprodtype = "";
						}
						currlineprodtypecode = lineprodtypecode;
					}

					amount = amounts.a(1, amountn);
					if (amount) {

						amountbase = amountbases.a(1, amountn);
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
				accno = getagp(agy.agp.a(43), calculate("PERIOD"));
				accno.swapper("%SUPPLIER%", suppliercode);
				if (accno == "") {
					accno = jobno;
					if (jobno.isnum()) {
						accno.splicer(1, 0, jobaccnoprefix);
					}
				}

				amount = (amounts.sum()).oconv(fin.currfmt);
				amountbase = (amountbases.sum()).oconv(agy.agp.a(3));
				taxamount = (taxamounts.sum()).oconv(fin.currfmt);
				taxamountbase = (taxamountbases.sum()).oconv(agy.agp.a(3));
				analysiscodex = "";
				gosub addline();

			}

			//GOSUB POSTVOUCHER

			var().osflush();
			tt = "";
			if (agy.agp.a(102)) {
				tt = "POST";
			}
			call updvoucher2(tt, voucher, vouchercode, "", agy.agp.a(102));

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
					if (not(accno.a(1, 1, 2))) {
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

	var invindex;
	if (not(invindex.read(agy.invoices, auditkey))) {
		invindex = "";
	}
	//if len(inv.index)>65000 then
	// inv.index=field(inv.index,fm,11,99999)
	// end
	while (true) {
	///BREAK;
	if (not(invindex.length() > 32000)) break;;
		invindex = invindex.field(FM, 100, 99999);
	}//loop;
	invindex.r(-1, invoicekey);
	invindex.write(agy.invoices, auditkey);

	//update invoice
	///////////////

	var invoice = brandcode ^ "*" ^ calculate("PERIOD");
	invoice.r(2, accountingdate);
	invoice.r(6, job.a(5, 1));

	//changes to accrued cost only
	if (isinvoiced) {
		invoice.r(7, invamount);
		invoice.r(8, calculate("CURR_CODE"));
		invoice.r(4, invamountbase);
	}else{
		invoice.r(12, invamount);
		invoice.r(13, calculate("CURR_CODE"));
		invoice.r(14, invamountbase);
	}

	invoice.r(9, calculate("DESC"));
	invoice.r(5, var().date());
	invoice.r(11, jobno);
	invoice.r(18, calculate("TAX_PERC"));
	invoice.r(19, brandcode);
	invoice.r(21, "P");
	if (isinvoiced) {

	/*;
		//invoice<26>=prodtypecode
			invoice<22>=suppliercode;
			invoice<23>=0;
			invoice<24>=cost.base;
			invoice<26>=prodtypecode;
	*/

		//multitype
		//strip off some of the analkeys to suit the invoice analcode format
		var analcodes = "";
		var nanalkeys = (costanalysis.a(1)).count(VM) + 1;
		for (var analkeyn = 1; analkeyn <= nanalkeys; ++analkeyn) {
			analcodes.r(1, analkeyn, costanalysis.a(1, analkeyn).field("*", 5, 2));
		};//analkeyn;
		invoice.r(22, analcodes);
		invoice.r(24, costanalysis.a(4));

		//costanalperiod
		invoice.r(25, costanalysis.a(2));

		invoice.r(38, ID);

	}

	invoice.write(agy.invoices, invoicekey);

	call flushindex("INVOICES");

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
	if (job.locate(ID, vn, 6)) {
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
exit:
	/////
	return;

}

subroutine unlockall() {
	call locking("UNLOCKALL", "", "", "", locklist, 0, msg);
	return;

	/////////////////
checkjobunlocked:
	/////////////////
	var jobcode = RECORD.a(2);
}

subroutine checkjobunlocked2() {

	if (not jobcode) {
		return;
	}

	if (not(job.read(agy.jobs, jobcode))) {
		msg = DQ ^ (jobcode ^ DQ) ^ " job does not exist";
		return invalid(msg);
	}

	if (lockrecord("JOBS", agy.jobs, jobcode)) {
		xx = unlockrecord("JOBS", agy.jobs, jobcode);
	}else{
		var lockholder = "JOBS*" ^ jobcode.xlate("LOCKS", 4, "X");
		if (lockholder == "") {
			lockholder = "Someone";
		}
		msg = lockholder ^ " is updating job " ^ (DQ ^ (jobcode ^ DQ)) ^ FM ^ "Please wait and try again later";
		return invalid(msg);
	}
	return;

	///////////
resetlines:
	///////////
	//update the internal line count for text section
	win.displayaction = 5;
	win.amvaction = 4;
	win.amvvars.r(1, 3, (RECORD.a(8)).count(VM) + (RECORD.a(8) ne ""));
	win.reset = 4;
	return;

}

subroutine unlockrecord() {
	xx = unlockrecord(win.datafile, win.srcfile, ID);
	win.wlocked = 0;
	return;

}

subroutine reverse() {
	var nn = tt.count(VM) + 1;
	for (var ii = 1; ii <= nn; ++ii) {
		var tt2 = tt.a(1, ii);
		if (tt2.length()) {
			tt2 = "-" ^ tt2;
			if (tt2.substr(1, 2) == "--") {
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

		//2 is jobs 30/12/2009
		//analysiscodex=analysisfnx:'*1*':analysiscodex
		analysiscodex = analysisfnx ^ "*2*" ^ analysiscodex;

		//1 28/29 income/cost
		//2 1/2 media/jobs
		//3 brand code
		//4 vehicle code (media)
		//5 market code (jobs)
		//6 supplier code (jobs)
		//7 job type code (jobs)
		//8 executive code
		analysiscodex = analysiscodex.fieldstore("*", 8, 1, calculate("EXECUTIVE_CODE").ucase());
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

	if (voucher.locate(accno, voucherln, 8)) {
		while (true) {
			xx = split(voucher.a(10, voucherln), linecurrencycode);
		///BREAK;
		if (voucher.a(8, voucherln) == "" or voucher.a(8, voucherln) == accno and linecurrencycode == currencycode and voucherln > minconsolidatelinen) break;;
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
		//garbagecollect;
		voucher.r(10, voucherln, (amount + split(voucher.a(10, voucherln), currencycode)).oconv(fin.currfmt) ^ currencycode);
		voucher.r(11, voucherln, (voucher.a(11, voucherln) + amountbase).oconv(agy.agp.a(3)));
	}else{
newvoucherline:
		while (true) {
		///BREAK;
		if (not(voucher.a(10, voucherln))) break;;
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

	if (taxamount or taxamountbase) {
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
	taxamount = "";
	taxamountbase = "";
	analysiscodex = "";

	return;

}

subroutine addrevline() {

	//reverse the amounts
	if (amount) {
		amount.splicer(1, 0, "-");
		if (amount.substr(1, 2) == "--") {
			amount.splicer(1, 2, "");
		}
	}
	if (amountbase) {
		amountbase.splicer(1, 0, "-");
		if (amountbase.substr(1, 2) == "--") {
			amountbase.splicer(1, 2, "");
		}
	}
	if (taxamount) {
		taxamount.splicer(1, 0, "-");
		if (taxamount.substr(1, 2) == "--") {
			taxamount.splicer(1, 2, "");
		}
	}
	if (taxamountbase) {
		taxamountbase.splicer(1, 0, "-");
		if (taxamountbase.substr(1, 2) == "--") {
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
			if (not(currprodtype.read(agy.jobtypes, tt))) {
				msg = DQ ^ (tt ^ DQ) ^ " production type is missing";
				//goto errexit
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
			if (not(curranalysis.locate(analkey, analn, 1))) {
				curranalysis.r(1, analn, analkey);
			}

			//lock
			if (not(locking("LOCK", "ANALYSIS", analkey, "", locklist, 9, msg))) {
				//goto errexit
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
	for (var analn = 1; analn <= nanals; ++analn) {

		var analkey = curranalysis.a(1, analn);

		var analrec;
		if (not(analrec.read(agy.analysis, analkey))) {
			analrec = "";
		}

		analrec.r(analfn, analmth, (analrec.a(analfn, analmth) +++ curranalysis.a(4, analn)).oconv(agy.agp.a(3)));

		analrec.write(agy.analysis, analkey);

	};//analn;

	return;

}


libraryexit()
