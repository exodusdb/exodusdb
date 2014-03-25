#include <exodus/library.h>
libraryinit()

#include <agencysubs.h>
#include <validcode2.h>
#include <validcode3.h>
#include <authorised.h>
#include <generalsubs.h>
#include <generalsubs2.h>
#include <sysmsg.h>
#include <addjobacc.h>
#include <log.h>
#include <singular.h>
#include <locking.h>

#include <win.h>
#include <gen.h>
#include <fin.h>//for taxes
#include <agy.h>

#include <window.hpp>

var reply;//num
var tt;
var msg;
var xx;
var replyn;//num
var jobcode;
var job;
var vn;//num
var docname;
var copyfile;
var versionfilename;
var taxn;
var temp;
var wsmsg;
var basecurrencyfmtz;

function main(in mode) {

	//garbagecollect;
	basecurrencyfmtz = agy.agp.a(3) ^ "Z";
	var interactive = not SYSTEM.a(33);
	//for taxes

	var op = "PRODUCTION ESTIMATE";
	if (mode.field(".", 1, 3) == "DEF.QUOTE.NO") {

		//pretty much identical in prodorder.subs, prodinv.subs and even plan.subs
		//call general.subs('DEF.SK')
		//defquoteno=is.dflt

		if (win.wlocked or RECORD) {
			return 0;
		}

		var previous = 0;

		var compcode = mode.field(".", 4);
		//gosub getnextjobno
		call agencysubs("GETNEXTID." ^ compcode, xx);

		win.isdflt = ANS;
		ANS = "";

		var defaultjobno = win.isdflt;

		if (not interactive) {
			ID = win.isdflt;
		}

	} else if (mode == "VAL.QUOTE.NO") {

		if (win.is == "" or win.is == win.isorig) {
			return 0;
		}

		if (RECORD.read(win.srcfile, win.is)) {

			if (not(validcode2(calculate("COMPANY_CODE"), calculate("CLIENT_CODE"), calculate("BRAND_CODE"), agy.brands, msg))) {
				return invalid();
			}
			if (not(validcode3(calculate("MARKET_CODE"), "", "", agy.vehicles, msg))) {
				return invalid();
			}
			RECORD = "";

		}else{

			//if they enter orderno:versionletter
			if (not win.is.isnum()) {
				var versionfile;
				if (versionfile.open("PRODUCTION.INVOICE.VERSIONS", "")) {
					if (RECORD.read(versionfile, win.is)) {
						//if validcode('BRAND',{BRAND_CODE},msg) else goto invalid
						if (not(validcode2(calculate("COMPANY_CODE"), calculate("CLIENT_CODE"), calculate("BRAND_CODE"), agy.brands, msg))) {
							return invalid();
						}
						if (not(validcode3(calculate("MARKET_CODE"), "", "", agy.vehicles, msg))) {
							return invalid();
						}
						//zzz should validate access better
						return 0;
					}
				}
			}

			//prevent certain users from creating their own quote numbers
			if (win.is ne win.registerx(10)) {
				if (not(authorised("PRODUCTION ESTIMATE CREATE OWN NO", msg, ""))) {
					msg = DQ ^ (ID ^ DQ) ^ " does not exist and" ^ FM ^ FM ^ msg;
					return invalid();
				}
			}

			//in case sombody else just used the next key
lockit:
			if (win.is == win.registerx(10)) {
				if (lockrecord(win.datafile, win.srcfile, win.is)) {
					xx = unlockrecord(win.datafile, win.srcfile, win.is);
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
				gosub invalid();
			}else{
				gosub note(msg);
			}

		}

	} else if (mode == "F2.PRODUCTION.INVOICES") {
		call agencysubs(mode, xx);

	} else if (mode == "DEF.STATUS") {
		if (not win.is) {
			ANS = "ISSUED";
		}

	} else if (mode == "F2.STATUS") {
		var options = "";
		options.r(-1, "ISSUED    (Waiting for approval)");
		options.r(-1, "APPROVED  (Waiting to start)");
		options.r(-1, "STARTED   (Work commenced)");
		options.r(-1, "FINISHED  (Work completed)");
		options.r(-1, "CANCELLED (Permanent stop)");
		options.r(-1, "ON HOLD   (Temporary stop)");
		reply = (decide("", options, replyn)).substr(1, 10).trim();
		if (reply) {
			mv.DATA ^= reply ^ "\r";
		}

	} else if (mode == "VAL.JOB") {
		if (win.is == win.isorig) {
			return 0;
		}

		gosub checkjob();
		if (not win.valid) {
			return 0;
		}

		jobcode = win.is;
		gosub checkjobunlocked2();
		if (not win.valid) {
			return 0;
		}

		jobcode = win.isorig;
		gosub checkjobunlocked2();
		if (not win.valid) {
			return 0;
		}

		//warning of other estimates on the job
		var estimatenos = job.a(10);
		if (estimatenos.locate(win.isorig, xx, 1)) {
			estimatenos.eraser(1, xx);
		}
		if (estimatenos) {
			msg = "Note: Job " ^ win.is ^ " also has|the following estimates:";
			if (estimatenos.count(VM) > 4) {
				estimatenos = estimatenos.field(VM, 1, 5) ^ VM ^ "etc.";
			}
			msg.r(-1, estimatenos);
			gosub note(msg);
		}

		//copy the job description to description, and brief to details
		if (RECORD.a(3) == "") {

			//copy job description to quote description
			if (RECORD.a(6) and job.a(9, 1)) {
				if (not(decide("Get the \"description\" from the job ?", "", reply))) {
					return 0;
				}
				if (reply == 1) {
					RECORD.r(6, "");
				}
			}
			if (RECORD.a(6) == "") {
				RECORD.r(6, job.a(9, 1));
			}

			//copy job brief to job details
			var brief = job.a(9).field(VM, 2, 9999);
			if (brief) {
				if (not(decide("Use the job \"brief\" as the details ?", "", reply))) {
					return 0;
				}
				if (reply == 1) {
					RECORD.r(8, brief);
				}
			}

			//update the internal line count for text section
			win.displayaction = 5;
			win.amvaction = 4;
			win.amvvars.r(1, 3, (RECORD.a(8)).count(VM) + (RECORD.a(8) ne ""));
			win.reset = 3;

		}

	} else if (mode == "DEF.ATTENTION") {
		if (win.is == "") {
			//readv @ans from brands,{BRAND_CODE},7 else @ans=''
			if (not(ANS.readv(agy.jobs, calculate("JOB_NO"), 11))) {
				ANS = "";
			}
		}

	} else if (mode == "VAL.ATTENTION") {
		if (win.is == "" or win.is == win.isorig) {
			return 0;
		}
		win.is.writev(agy.brands, calculate("BRAND_CODE"), 7);

	} else if (mode == "DEF.DESCRIPTION") {

		if (not win.wlocked) {
			return 0;
		}

		if (job.read(agy.jobs, calculate("JOB_NO"))) {
			win.isdflt = job.a(9, 1);
			// if is=is.dflt then is=''
		}

	} else if (mode == "DEF.CURRENCY") {
		if (not win.is) {
			win.isdflt = agy.agp.a(2);
		}

	} else if (mode == "VAL.CURRENCY") {
		if (win.is == win.isorig) {
			return 0;
		}
		call generalsubs("VAL.CURRENCY");
		if (not win.valid) {
			return 0;
		}

		//option to convert amounts
		if (win.is ne win.isorig and (RECORD.a(3)).sum()) {
			var rate = "";
			call note2("Exchange rate ? 1 " ^ win.isorig ^ " = ? " ^ win.is ^ "|(enter \"1/rate\" for inverse rate)", "RC", rate, "");
			if (rate) {
				if (rate.substr(1, 2) == "1/") {
					rate = 1 / rate.substr(3, 999);
				}
				if (rate.substr(1, 1) == "/") {
					rate = 1 / rate.substr(2, 999);
				}
				if (not rate.isnum()) {
					msg = "PLEASE ENTER A NUMERIC EXCHANGE RATE";
					return invalid();
				}
				var ndecs;
				if (not(ndecs.readv(gen.currencies, win.is, 3))) {
					ndecs = 2;
				}
				var fmt = "MD" ^ ndecs ^ "0PZ";
				var nlines = (RECORD.a(3)).count(VM) + (RECORD.a(3) ne "");
				if (rate) {
					for (var ln = 1; ln <= nlines; ++ln) {
						//garbagecollect;
						if (RECORD.a(3, ln)) {
							var amount = RECORD.a(3, ln);
							RECORD.r(3, ln, (amount * rate).oconv(fmt));
						}
					};//ln;
				}
			}
		}

		//force recalc of exch rate
		//redisplay.list=wi+1
		win.displayaction = 5;
		win.reset = 3;
		RECORD.r(5, "");

	} else if (mode == "DEF.EXCH.RATE") {
		if (win.is) {
			return 0;
		}
		//also used by PRODINVS
		if (not(gen.currency.read(gen.currencies, calculate("CURR_CODE")))) {
			msg = "CURRENCY " ^ (DQ ^ (calculate("CURR_CODE") ^ DQ)) ^ " IS MISSING FROM THE CURRENCIES FILE";
			return invalid();
		}
		var idate = calculate("DATE");
		if (not(gen.currency.locateby(idate, "DR", vn, 4))) {
			{}
		}
		var rate = gen.currency.a(5, vn);
		if (rate == "") {
			rate = gen.currency.a(5, vn - 1);
		}
		ANS = rate;

	} else if (mode == "VAL.AMOUNT") {
		if (win.is == "" or win.is == win.isorig) {
			return 0;
		}
		var ndecs;
		if (not(ndecs.readv(gen.currencies, calculate("CURR_CODE"), 3))) {
			ndecs = "2";
		}
		if (ndecs == "") {
			ndecs = "2";
		}
		win.is = win.is.oconv("MD" ^ ndecs ^ "0P");

	} else if (mode == "POSTINIT") {
		gosub security2(mode, op);

	} else if (mode.substr(1, 8) == "POSTREAD") {

		//option to read previous versions
		call generalsubs2(mode);
		if (not win.valid) {
			return 0;
		}

		//check allowed to create (or prevent editing if not allowed to update)
		gosub security2(mode, op);
		if (not win.valid) {
			return 0;
		}

		//restrict access based on company and brand
		if (RECORD.a(2)) {
			if (not(validcode2(calculate("COMPANY_CODE"), "", calculate("BRAND_CODE"), agy.brands, msg))) {
				win.reset = 5;
				return invalid();
			}
		}

		call agencysubs("CHKCLOSEDPERIOD." ^ mode, msg);
		if (msg) {
			//comment to client
			win.reset = -1;
			gosub note(msg);
			gosub unlockrecordx();
		}

		//backwards compatible with old estimates with no update log
		//only works up to letter Z
		if (win.orec) {
			var version = RECORD.a(12);
			if (not version) {
				version = "A";
			}
			if (win.wlocked and not RECORD.a(33)) {
				for (var ii = 1; ii <= 26; ++ii) {
					var letter = var().chr(64 + ii);
					RECORD.r(33, ii, letter);
				///BREAK;
				if (letter == version) break;;
				};//ii;
				win.orec.r(33, RECORD.a(33));
			}
		}

		//prevent editing if already invoiced
		if (win.wlocked and (calculate("INVOICE_NO") or calculate("INVOICE_NO_FAILSAFE"))) {
preventediting:
			xx = unlockrecord(win.datafile, win.srcfile, ID);
			win.wlocked = 0;

			//////
			return 0;
			//////

		}

		//draft mode only
		if (win.wlocked and win.orec and RECORD.a(11) ne "DRAFT") {
			if (not(authorised("PRODUCTION ESTIMATE ISSUE", xx))) {
				win.wlocked = 0;
				xx = unlockrecord(win.datafile, win.srcfile, ID);
			}
		}

		//prevent editing if not allowed to update other executives estimates
		if (win.wlocked) {
			var executive = calculate("EXECUTIVE_CODE");
			if (executive and executive ne USERNAME) {
				if (not(authorised("PRODUCTION ESTIMATE UPDATE OTHERS", msg, ""))) {
					if (interactive) {
						msg = "NOTE:|YOU CANNOT EDIT THIS ESTIMATE BECAUSE|IT BELONGS TO " ^ executive;
						gosub note(msg);
					}
					goto preventediting;
				}
			}
		}

		//prevent editing if approved and not allowed to approve
		if (win.wlocked and RECORD.a(11) == "APPROVED") {
			if (not(authorised("PRODUCTION ESTIMATE APPROVE", msg, ""))) {
				goto preventediting;
			}
		}

		//prevent editing if prepaid and not allowed to update prepaid
		//if wlocked then
		// prepaid={PREPAID}
		// if prepaid then
		// if security('PRODUCTION ESTIMATE UPDATE AFTER JOB CLOSED',msg,'') else
		// *if interactive then
		// msg[1,0]='NOTE:|YOU CANNOT EDIT THIS ESTIMATE BECAUSE|IT IS PREPAID AND':fm
		// gosub note
		// * end
		// goto preventediting
		// end
		// end
		// end

	/* moved up to before security checks;
			//if they enter orderno:versionletter
			if @record='' and not(num(@id)) then;
				open 'PRODUCTION.INVOICE.VERSIONS' to versionfile then;
					read @record from versionfile,@id then;
						orec=@record;
						x=unlockrecord('',src.file,@id);

						wlocked=0;
						gosub resetlines;
						return 0;
						end;
					end;
				end;
	*/

		//prevent certain users from creating their own quote numbers
		if (win.wlocked and RECORD == "" and not interactive and not USER3.index("RECORDKEY", 1)) {
			if (not(authorised("PRODUCTION ESTIMATE CREATE OWN NO", msg, ""))) {
				msg = DQ ^ (ID ^ DQ) ^ " does not exist and" ^ FM ^ FM ^ msg;
				win.reset = 5;
				return invalid();
			}
		}

		//backwards compatible with one amount and text lines separated by tm
		//garbagecollect;
		if (not (RECORD.a(3)).count(VM)) {
			if ((RECORD.a(8)).count(TM)) {
				tt = RECORD.a(8);
				tt.converter(TM, VM);
				tt = tt.oconv("T#60");
				tt.converter(TM, VM);
				while (true) {
				///BREAK;
				if (not(tt.index(" " ^ VM, 1))) break;;
					tt.swapper(" " ^ VM, VM);
				}//loop;
				RECORD.r(8, tt);
				if (interactive) {
					call note("Note: The text and amount section has been converted|to a new format, but it may need some editing|to put the amounts back into the amount column.");
				}
			}
		}

		//get the number of decimals
		if (RECORD and RECORD.a(14) == "") {
			RECORD.r(14, calculate("NDECS"));
			win.orec.r(14, RECORD.a(14));
		}

		//default signatory into old records
		if (win.orec and RECORD.a(24) == "") {
			if (agy.agp.a(51) and agy.agp.a(51) ne "NEVER") {
				tt = agy.agp.a(62);
				if (tt == "") {
					tt = calculate("EXECUTIVE_CODE");
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


	} else if (mode == "PREDELETE") {

		msg = "Estimated cannot be deleted. Change the status to cancelled.";
		return invalid();

		gosub checkjobunlocked();
		if (not win.valid) {
			return invalid();
		}

		//prevent removal from closed jobs
		if (not(authorised("PRODUCTION ESTIMATE UPDATE AFTER JOB CLOSED", msg, "UP2"))) {
			if (job.a(7) == "Y") {
				return invalid();
			}
		}

		gosub security2(mode, op);

		call agencysubs("CHKCLOSEDPERIOD." ^ mode, msg);
		if (msg) {
			return invalid();
		}

	} else if (mode == "PREWRITE") {

		//prevent creating new records if no lic
//		call (mode, msg);
//		if (msg) {
//			return invalid();
//		}

		//prevent f9 while locked
		if (not win.wlocked) {
			//msg='WLOCKED is false in PRODINV.SUBS,PREWRITE'
			msg = "";
			return invalid();
		}

		call agencysubs("CHKCLOSEDPERIOD." ^ mode, msg);
		if (msg) {
			return invalid();
		}

		if (RECORD.a(11) == "INVOICED" or calculate("INVOICE_NO_FAILSAFE")) {
			if (interactive) {
//				gosub print();
				win.valid = 0;
				return invalid();
			}
			msg = "Estimate " ^ (DQ ^ (ID ^ DQ)) ^ " system failure during previous invoicing attempt";
			call sysmsg(msg);
			return invalid();
		}

		//non-draft mode prevented in client but lets make sure
		if (RECORD.a(11) ne "DRAFT") {
			if (not(authorised("PRODUCTION ESTIMATE ISSUE", msg))) {
				return invalid();
			}
		}

		//check currency
		var currcode = RECORD.a(4);
		if (currcode ne win.orec.a(4)) {
			if (not(gen.currency.read(gen.currencies, currcode))) {
				msg = DQ ^ (currcode ^ DQ) ^ " currency does not exist";
				return invalid();
			}
			var tt = convert("<>", "()", gen.currency.a(1));
			if (gen.currency.a(25) or (tt.ucase()).index("(STOP)", 1)) {
				msg = tt ^ FM ^ "currency is stopped" ^ FM ^ gen.currency.a(25);
				return invalid();
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
				return invalid();
			}
		}

		//get/check job
		gosub checkjobunlocked();
		if (not win.valid) {
			return invalid();
		}

		//prevent addition to closed jobs
		if (job.a(7) == "Y") {
			if (not(authorised("PRODUCTION ESTIMATE CREATE AFTER JOB CLOSED", msg, "UP2"))) {
jobclosed:
				msg = DQ ^ (jobcode ^ DQ) ^ " is closed" ^ FM ^ FM ^ msg;
				return invalid();
			}
		}

		//prevent creating many estimates per job (also in UI to avoid late message)
		if (not win.orec and job.a(10)) {
			if (not(authorised("PRODUCTION ESTIMATE CREATE MANY PER JOB", xx))) {
				msg = DQ ^ (jobcode ^ DQ) ^ " already has an estimate " ^ job.a(10) ^ FM ^ FM ^ msg;
				return invalid();
			}
		}

		//if changing job code then ensure old job is not closed
		jobcode = win.orec.a(2);
		if (jobcode and jobcode ne RECORD.a(2)) {
			gosub checkjobunlocked2();
			if (not win.valid) {
				return invalid();
			}

			//prevent removal from closed jobs
			if (job.a(7) == "Y") {
				if (not(authorised("PRODUCTION ESTIMATE UPDATE AFTER JOB CLOSED", msg, "UP2"))) {
					goto jobclosed;
				}
			}

			//prevent changing job if any po linked to this estimate
			var oldordnos = job.a(4);
			var nn = oldordnos.count(VM) + (oldordnos ne "");
			for (var ii = 1; ii <= nn; ++ii) {
				var prodordno = oldordnos.a(1, ii);
				var prodorder;
				if (prodorder.read(agy.productionorders, prodordno)) {
					if (prodorder.a(35) == ID) {
						msg = "You cannot change the job on this estimate " ^ (DQ ^ (ID ^ DQ));
						msg.r(-1, "because purchase order " ^ (DQ ^ (prodordno ^ DQ)) ^ " is linked to it");
						msg.r(-1, "");
						msg.r(-1, "You could unlink the PO " ^ (DQ ^ (prodordno ^ DQ)) ^ " from this estimate");
						msg.r(-1, "  by removing the estimate number on the PO  ");
						return invalid();
					}
				}
			};//ii;

		}

		//restore jobcode and job
		gosub checkjobunlocked();
		if (not win.valid) {
			return invalid();
		}

		//make sure exchange rate for base currency is 1
		if (RECORD.a(4) == agy.agp.a(2) and RECORD.a(5) ne 1) {
			RECORD.r(5, 1);
		}

		//check production type
		tt = RECORD.a(17);
		if (tt.a(1, 1) == "") {
			//loop while t[1,1]=vm do t[1,1]='' repeat
			tt.trimmerb(VM);
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
					return invalid();
				}
			}
		}

		//check can add an account for APPROVED
		if (not(var("DRAFT,ISSUED,CANCELLED,INVOICED").locateusing(RECORD.a(11), ",", xx))) {
			call addjobacc("ADD", jobcode, msg);
			if (msg) {
				if (interactive) {
//					if (not(decide2(msg, "Continue (without creating account)" _VM_ "Cancel", reply, 2))) {
//						reply = 2;
//					}
				}else{
					//reply=1
					//goto errexit
					return invalid();
				}
				if (reply == 2) {
					win.valid = 0;
					return invalid();
				}
			}
		}

		//update log and version file
		call generalsubs2(mode);

	} else if (mode == "POSTWRITE" or mode == "POSTDELETE") {

		gosub updanalysis(mode);

//		call flushindex("PRODUCTION.INVOICES");

		if (interactive and mode == "POSTWRITE") {
			if (not(decide("Do you want a printout ?", "", reply))) {
				reply = 2;
			}
			xx = unlockrecord(win.datafile, win.srcfile, ID);
			if (reply == 1) {
				perform("GET NEW PRODINVS " ^ ID);
			}
		}

	} else if (mode == "UPD.ANALYSIS") {
		gosub updanalysis(mode);
	}

	return 0;
}

subroutine updanalysis(in mode) {

	//add to analysis
	gosub updanalysis2(+1);

	//remove old analysis
	if (mode.index("DELETE", 1) or win.orec) {

		var storerecord = RECORD;
		win.orec.transfer(RECORD);

		gosub updanalysis2(-1);

		RECORD.transfer(win.orec);
		storerecord.transfer(RECORD);

	}

	return;

}

subroutine updanalysis2(in sign) {

	//only want approved estimates pending billing
	var status = calculate("STATUS");
	if (not status) {
		return;
	}
	if (var("DRAFT,ISSUED,CANCELLED,ON HOLD,INVOICED").locateusing(status, ",", xx)) {
		return;
	}

	//get the production type record
	MV = 0;
	var amountbases = calculate("AMOUNT_BASE");
	var prodtypecodes = calculate("PRODUCTION_TYPE");

	//consolidate lines with same production type
	var nlines = amountbases.count(VM) + 1;
	var prodtypecode = "";
	for (var linen = 1; linen <= nlines; ++linen) {
		var amountbase = amountbases.a(1, linen);
		if (amountbase) {
			temp = prodtypecodes.a(1, linen);
			if (temp) {
				prodtypecode = temp;
			}else{
				prodtypecodes.r(1, linen, prodtypecode);
			}
			if (prodtypecodes.locate(prodtypecode, xx, 1)) {
				if (xx < linen) {
					amountbases.r(1, xx, (amountbases.a(1, xx) + amountbase).oconv(basecurrencyfmtz));
					amountbases.r(1, linen, "");
				}
			}
		}
	};//linen;

	//update the analysis file

	var suppliercode = calculate("SUPPLIER_CODE");
	var brandcode = calculate("BRAND_CODE");
	var marketcode = calculate("MARKET_CODE");
	var orderperiod = ((calculate("DATE")).oconv("D2/E")).substr(4, 5);
	var ordermthno = orderperiod.field("/", 1);

	var companycode = calculate("COMPANY_CODE");
	if (companycode == "") {
		companycode = gen.gcurrcompany;
	}

	for (var linen = 1; linen <= nlines; ++linen) {
		var amountbase = amountbases.a(1, linen);
		if (amountbase) {
			prodtypecode = prodtypecodes.a(1, linen);
			var analkey = orderperiod.field("/", 2) ^ "*" ^ brandcode ^ "**" ^ marketcode ^ "*" ^ suppliercode ^ "*" ^ prodtypecode;
			analkey = analkey.fieldstore("*", 8, 1, companycode);

			//try to lock one analysis record at a time to avoid "deadly embrace"
			//if cannot lock then update anyway
			if (not(locking("LOCK", "ANALYSIS", analkey, "", win.registerx(5), 99, xx))) {
				call log("PRODINV.SUBS", DQ ^ (analkey ^ DQ) ^ " could not lock analysis record while invoicing " ^ (DQ ^ (ID ^ DQ)));
			}

			var analrec;
			if (not(analrec.read(agy.analysis, analkey))) {
				analrec = "";
			}
			//garbagecollect;
			analrec.r(11, ordermthno, (analrec.a(11, ordermthno) + sign * amountbase).oconv(basecurrencyfmtz));

			call cropper(analrec);
			if (analrec) {
				analrec.write(agy.analysis, analkey);
			}else{
				agy.analysis.deleterecord(analkey);
			}

			call locking("UNLOCK", "ANALYSIS", analkey, "", win.registerx(5), 0, xx);

		}

	};//linen;

}

subroutine checkjob() {

	if (authorised("PRODUCTION ESTIMATE CREATE AFTER JOB CLOSED", msg, "UP2")) {
		temp = "";
	}else{
		temp = ".OPEN";
	}
	//call agency.subs(mode)
	call agencysubs("VAL.JOB" ^ temp, xx);
	if (not win.valid) {
		return;
	}

	//get the job
	if (not(job.read(agy.jobs, win.is))) {
		msg = DQ ^ (win.is ^ DQ) ^ " JOB DOES NOT EXIST";
		gosub invalid();
		return;
	}

	//warning if job closed
	if (job.a(7) == "Y") {
		msg = "WARNING: Job " ^ (DQ ^ (win.is ^ DQ)) ^ " is closed.";
		gosub note(msg);
	}

	return;

}

subroutine checkjobunlocked() {
	jobcode = RECORD.a(2);
}

subroutine checkjobunlocked2() {
	if (not jobcode) {
		return;
	}
	if (not(job.read(agy.jobs, jobcode))) {
		msg = DQ ^ (jobcode ^ DQ) ^ " job does not exist";
		gosub invalid();
		return;
	}
	if (lockrecord("JOBS", agy.jobs, jobcode)) {
		xx = unlockrecord("JOBS", agy.jobs, jobcode);
	}else{
		msg = "Someone is updating job " ^ (DQ ^ (jobcode ^ DQ)) ^ FM ^ "Please wait and try again later";
		gosub invalid();
		return;
	}

	return;

}

subroutine unlockrecordx() {
	xx = unlockrecord(win.datafile, win.srcfile, ID);
	win.wlocked = 0;
	return;

}


libraryexit()
