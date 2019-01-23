#include <exodus/library.h>
libraryinit()

#include <agencysubs.h>
#include <validcode2.h>
#include <validcode3.h>
#include <authorised.h>
#include <btreeextract.h>
#include <prodordersubs2.h>
#include <generalsubs2.h>
#include <validjob.h>
#include <flushindex.h>
#include <singular.h>

#include <gen.h>
#include <agy.h>
#include <fin.h>
#include <win.h>

#include <window.hpp>

var xx;
var msg;
var temp;
var jobcode;
var tt;
var hits;
var reply;
var docname;
var copyfile;
var versionfilename;
var wsmsg;

function main(in mode) {
	//c job
	//global jobcode

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

	//EQU PO.DATE.FN TO 1
	//EQU PO.JOB.NO.FN TO 2
	//EQU PO.AMOUNT.FN TO 3
	//EQU PO.CURR.CODE TO 4
	//EQU PO.EXCH.RATE.FN TO 5
	//EQU PO.DESC.FN TO 6
	//EQU PO.INV.NO.FN TO 7
	//EQU PO.ACNO.FN TO 8

	if (mode == "F2.PRODUCTION_ORDERS") {
		call agencysubs(mode, xx);

	} else if (mode.field(".", 1, 3) == "DEF.ORDER.NO") {
		//pretty much identical in prodorder.subs, prodinv.subs and even plan.subs
		//call general.subs('DEF.SK')

		if (win.wlocked or RECORD) {
			return 0;
		}

		var previous = 0;

		var compcode = mode.field(".", 4);
		//gosub getnextjobno
		call agencysubs("GETNEXTID." ^ compcode, xx);

		win.isdflt = ANS;
		ANS = "";

		win.registerx(10) = win.isdflt;

		if (not interactive) {
			ID = win.isdflt;
		}

		return 0;

	} else if (mode == "VAL.ORDER.NO") {
		if ((win.is == "") or (win.is == win.isorig)) {
			return 0;
		}

		if (RECORD.read(win.srcfile, win.is)) {

			if (not(validcode2(calculate("COMPANY_CODE"), calculate("CLIENT_CODE"), calculate("BRAND_CODE"), agy.brands, msg))) {
				gosub invalid(msg);
				return 0;
			}
			if (not(validcode3(calculate("MARKET_CODE"), calculate("SUPPLIER_CODE"), "", xx, msg))) {
				gosub invalid(msg);
				return 0;
			}
			RECORD = "";

		}else{

			//prevent certain users from creating their own quote numbers
			if (win.is ne win.registerx(10)) {
				if (not(authorised("PRODUCTION ORDER CREATE OWN NO", msg, ""))) {
					msg = DQ ^ (win.is ^ DQ) ^ " does not exist and" ^ FM ^ FM ^ msg;
					return invalid(msg);
				}
			}

			//in case sombody else just used the next key
lockit:
			if (win.is == win.registerx(10)) {
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
			if (win.is.index(" ")) {
				msg = "SORRY, YOU CANNOT USE SPACES HERE";
				gosub invalid(msg);
			}else{
				gosub badchars();
			}

		}
	/*;
		case mode='CHOOSECOLS';

			prodorder.heads=xlate('DEFINITIONS','PRODORDER.COLHEADS',1,'X');
			@record<13>=prodorder.heads:vm:@record<13>;
			@record=insert(@record,3,1,0,'');
			display.action=5;
			reset=3;
	*/
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
		call agencysubs(mode ^ temp, xx);
		if (not(win.valid)) {
			return 0;
		}

		jobcode = win.is;
		gosub checkjobunlocked();
		if (not(win.valid)) {
			return 0;
		}

		jobcode = win.isorig;
		gosub checkjobunlocked();
		if (not(win.valid)) {
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
			call note(msg);
		}

		//copy the job description to description, and brief to details
		//or just copy the description and brief to the details
		if (not(RECORD.a(13))) {
			//if 1 then
			RECORD.r(6, job.a(9, 1));
			RECORD.r(13, job.a(9).field(VM, 2, 999));
			//end else
			// @record<13>=job<9>
			// end

			//update the internal line count for text section
			win.displayaction = 5;
			win.amvaction = 4;
			win.amvvars.r(1, 3, RECORD.a(13).count(VM) + (RECORD.a(13) ne ""));
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
				var inactivestatuses = "FINISHED,CANCELLED,ON HOLD,INVOICED";
				if (not(inactivestatuses.locateusing(estimate.a(11), ",", xx))) {
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
			if (authorised("PRODUCTION ORDER CREATE WITHOUT ESTIMATE", xx, "")) {
				msg.splicer(1, 0, "*** WARNING ***" ^ FM ^ FM);
				call note(msg);
			}else{
				return invalid(msg);
			}
		}

	//called from web interface
	} else if (mode == "VAL.SUPPINV") {
		if ((win.is == "") or (win.isorig == win.is)) {
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
		call btreeextract(tt, "PRODUCTION_ORDERS", DICT, hits);

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
			//if interactive then
			// if decide2('!WARNING|':tempmsg,'OK|Cancel',reply,2) else reply=2
			//end else
			//NB the WARNING word is required for a test in the html client.
			//call msg('WARNING: ':tempmsg)
			USER3 = "WARNING! " ^ tempmsg;
			return 0;
			// end
			if (reply == 2) {
				win.valid = 0;
				return 0;
			}
		}

	} else if (mode == "POSTINIT") {
		gosub security(mode);
		if (not(win.valid)) {
			return 0;
		}

		if (not(authorised("PRODUCTION COST ACCESS", msg, ""))) {
			gosub invalid(msg);
			var().stop();
		}

	} else if (mode == "PREWRITE") {
		call prodordersubs2(mode);

	} else if (mode == "PREDELETE") {
		call prodordersubs2(mode);
		if (not(win.valid)) {
			return 0;
		}

		//update version log
		//probably not since po cannot be deleted
		call generalsubs2(mode);

	} else if (mode.substr(1,8) == "POSTREAD") {

		//option to read previous versions
		call generalsubs2(mode);
		if (not(win.valid)) {
			return 0;
		}

		gosub security(mode);
		if (not(win.valid)) {
			return 0;
		}

		//general cost access restriction
		if (not(authorised("PRODUCTION COST ACCESS", msg, ""))) {
			win.reset = 5;
			return invalid(msg);
		}

		//restrict access based on company, brand, user etc
		var jobno = RECORD.a(2);
		var job = "";
		if (jobno) {

			if (not(validcode2(calculate("COMPANY_CODE"), "", calculate("BRAND_CODE"), agy.brands, msg))) {
				win.reset = 5;
				return invalid(msg);
			}

			if (not(validcode3(calculate("MARKET_CODE"), calculate("SUPPLIER_CODE"), "", xx, msg))) {
				win.reset = 5;
				return invalid(msg);
			}

			msg = "Purchase Order " ^ (DQ ^ (ID ^ DQ));

			//JOB ACCESS OTHERS
			if (not(validjob("ACCESS", jobno, job, msg))) {
				win.reset = 5;
				return invalid(msg);
			}

			if (win.wlocked and win.orec) {
				//JOB UPDATE OTHERS
				if (not(validjob("UPDATE", jobno, job, msg))) {
					//x=unlockrecord(datafile,src.file,@id)
					//wlocked=0
					win.reset = -1;
					call note(msg);
					gosub unlockrec();
				}
			}

		}

		//option to prevent adding costs after job has been billed
		//zzz but how to delete orders?
		if (win.wlocked and (calculate("JOB_CLOSED") == "Y")) {
			if (RECORD) {
				if (not(authorised("PRODUCTION ORDER UPDATE AFTER JOB CLOSED", msg, ""))) {
					win.wlocked = 0;
				}
			}else{
				//if security('PRODUCTION ORDER CREATE AFTER JOB CLOSED',msg,'') else
				// wlocked=0
				// end
			}
			if (not(win.wlocked)) {
				xx = unlockrecord(win.datafile, win.srcfile, ID);
				if (RECORD == "") {
					goto canc;
				}
			}
		}

		//get the number of decimals
		if (RECORD and (RECORD.a(14) == "")) {
			RECORD.r(14, calculate("NDECS"));
			win.orec.r(14, RECORD.a(14));
		}

		//prevent certain users from creating their own quote numbers
		if (((RECORD == "") and not interactive) and not USER3.index("RECORDKEY")) {
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
				gosub unlockrec();
			}
		}

		//default signatory into old records
		if (win.orec and (RECORD.a(24) == "")) {
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
		if ((win.wlocked and win.orec) and RECORD.a(11) ne "DRAFT") {
			if (not(authorised("PRODUCTION ORDER ISSUE", xx))) {
				//wlocked=0
				//xx=unlockrecord(datafile,src.file,@id)
				gosub unlockrec();
			}
		}

		if (win.wlocked and RECORD) {
			call agencysubs("CHKCLOSEDPERIOD." ^ mode, msg);
			if (msg) {
				//comment to client
				win.reset = -1;
				call note(msg);
				gosub unlockrec();
			}
		}

		if (RECORD or (mode == "POSTREAD2")) {
			return 0;
		}

		//if decide('','Enter a new purchase order|Copy another purchase order|Copy an estimate',reply) else
		if (0) {
canc:
			win.valid = 0;
			win.reset = 4;
			//wlocked=''
			//xx=unlockrecord(datafile,src.file,@id)
			gosub unlockrec();
			return 0;
		}

		if (reply == 1) {
			return 0;
		}

		if (reply == 2) {
			docname = "order";
			copyfile = win.srcfile;
			versionfilename = "PRODUCTION.ORDER_VERSIONS";
		}else{
			docname = "estimate";
			copyfile = agy.productioninvoices;
			versionfilename = "PRODUCTION.INVOICE_VERSIONS";
		}

		var copyno = "";
copyno:
		call note("What is the number of the|" ^ docname ^ " that you want to copy ?", "RC", copyno, "");
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
			gosub invalid(msg);
			goto copyno;
		}

		jobcode = copydoc.a(2);
		gosub checkjobunlocked();
		if (not(win.valid)) {
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

	//initrec:
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
		win.amvvars.r(1, 3, RECORD.a(13).count(VM) + (RECORD.a(13) ne ""));
		win.reset = 4;

		RECORD.r(14, calculate("NDECS"));

		//warn and force reentry of job number if job is closed
		if (calculate("JOB_CLOSED")) {
			//if security('PRODUCTION ORDER CREATE AFTER JOB CLOSED',msg,'') else
			call note(DQ ^ (calculate("JOB_NO") ^ DQ) ^ " Job has been closed.");
			RECORD.r(2, "");
		}

	} else if ((mode == "POSTWRITE") or (mode == "POSTDELETE")) {
		call flushindex("PRODUCTION_ORDERS");

	} else {
		msg = DQ ^ (mode ^ DQ) ^ " unrecognised mode in prodorder.subs";
		gosub invalid(msg);
	}
//L3074:
	return 0;

}

subroutine checkjobunlocked() {

	if (not jobcode) {
		return;
	}

	var job;
	if (not(job.read(agy.jobs, jobcode))) {
		msg = DQ ^ (jobcode ^ DQ) ^ " job does not exist";
		gosub invalid(msg);
		return;
	}

	if (lockrecord("JOBS", agy.jobs, jobcode)) {
		xx = unlockrecord("JOBS", agy.jobs, jobcode);
	}else{
		var lockholder = ("JOBS*" ^ jobcode).xlate("LOCKS", 4, "X");
		if (lockholder == "") {
			lockholder = "Someone";
		}
		msg = lockholder ^ " is updating job " ^ (DQ ^ (jobcode ^ DQ)) ^ FM ^ "Please wait and try again later";
		gosub invalid(msg);
		return;
	}
	return;

}

subroutine unlockrec() {
	xx = unlockrecord(win.datafile, win.srcfile, ID);
	win.wlocked = 0;
	return;

}


libraryexit()
