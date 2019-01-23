#include <exodus/library.h>
libraryinit()

#include <agencysubs.h>
#include <validcode2.h>
#include <validcode3.h>
#include <authorised.h>
#include <generalsubs2.h>
#include <validjob.h>
#include <chklic.h>
#include <sysmsg.h>
#include <initcompany.h>
#include <addjobacc.h>
#include <flushindex.h>
#include <locking.h>
#include <log.h>
#include <singular.h>

#include <gen.h>
#include <agy.h>
#include <fin.h>
#include <win.h>

#include <window.hpp>

var basecurrencyfmtz;
var mode;
var xx;
var msg;
var jobcode;
var job;
var reply;//num
var ndecs;
var tt;
var docname;
var copyfile;
var versionfilename;
var copynos;
var nn;//num
var vn;//num
var taxn;
var temp;
var linen2;
var wsmsg;

function main(in mode0) {
	//c job
	//global copynos,nn,basecurrencyfmtz,mode

	//garbagecollect;
	basecurrencyfmtz = agy.agp.a(3) ^ "Z";
	var interactive = not SYSTEM.a(33);
	mode = mode0;
	//for taxes

	var op = "PRODUCTION ESTIMATE";
	if (mode == "PERPETUAL") {

		//if copied an invoiced record then clear certain invoice info
		if (((win.orec == "") and ID) and ((((RECORD.a(11) == "INVOICED") or RECORD.a(12)) or RECORD.a(19)))) {
			gosub initrec();
			win.displayaction = 5;
			win.reset = 3;
		}
	/*;
		case mode='UPLIFT';
	uplift:
			uplift='';
			if decide('Uplift by ?','Percentage':vm:'Amount',reply) else return 0;
			perc=(reply=1);
			if perc then tt='percentage' else tt='amount';
			call note2('Uplift by what ':tt:' ?','RC',uplift,'');
			if uplift else return 0;
			if num(uplift) else;
				msg='Please enter a number';
				return invalid(msg);
				end;
			nlns=count(@record<3>,vm)+1;
			for ln=1 to nlns;
				amt=@record<3,ln>;
				fmt='MD':len(field(amt,'.',2)):'0PZ';
				if amt then;
					if perc then;
						amt=(amt*((100+uplift)/100));
					end else;
						amt+=uplift;
						end;
					garbagecollect;
					@record<3,ln>=amt fmt;
					end;
				next ln;
			display.action=5;
			reset=3;

			@record<11>='UPLIFTED';
	*/

	} else if (mode.field(".", 1, 3) == "DEF.QUOTE.NO") {

		//pretty much identical in prodorder.subs, prodinv.subs and even plan.subs
		//call general.subs('DEF.SK')
		//defquoteno=is.dflt

		if (win.wlocked or RECORD) {
			return 0;
		}

		var previous = 0;

		var compcode = mode.field(".", 4);
		//gosub getnextjobno
		call agencysubs("GETNEXTID." ^ compcode, xx, "", "");

		win.isdflt = ANS;
		ANS = "";

		var defaultjobno = win.isdflt;

		if (not interactive) {
			ID = win.isdflt;
		}

	} else if (mode == "VAL.QUOTE.NO") {

		if ((win.is == "") or (win.is == win.isorig)) {
			return 0;
		}

		if (RECORD.read(win.srcfile, win.is)) {

			if (not(validcode2(calculate("COMPANY_CODE"), calculate("CLIENT_CODE"), calculate("BRAND_CODE"), agy.brands, msg))) {
				return invalid(msg);
			}
			if (not(validcode3(calculate("MARKET_CODE"), "", "", xx, msg))) {
				return invalid(msg);
			}
			RECORD = "";

		}else{

			//if they enter orderno:versionletter
			if (not win.is.isnum()) {
				var versionfile;
				if (versionfile.open("PRODUCTION.INVOICE_VERSIONS", "")) {
					if (RECORD.read(versionfile, win.is)) {
						if (not(validcode2(calculate("COMPANY_CODE"), calculate("CLIENT_CODE"), calculate("BRAND_CODE"), agy.brands, msg))) {
							return invalid(msg);
						}
						if (not(validcode3(calculate("MARKET_CODE"), "", "", xx, msg))) {
							return invalid(msg);
						}
						//zzz should validate access better
						return 0;
					}
				}
			}

			//prevent certain users from creating their own quote numbers
			if (win.is ne win.registerx(10)) {
				if (not(authorised("PRODUCTION ESTIMATE CREATE OWN NO", msg))) {
					msg = DQ ^ (ID ^ DQ) ^ " does not exist and" ^ FM ^ FM ^ msg;
					return invalid(msg);
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
			if (win.is.index(" ")) {
				msg = "SORRY, YOU CANNOT USE SPACES HERE";
				gosub invalid(msg);
			}else{
				gosub badchars();
			}

		}
	/*;
		case mode='F2.PRODUCTION_INVOICES';
			call agency.subs(mode,xx,'','');

		case mode='DEF.STATUS';
			if is else @ans='ISSUED';

		case mode='F2.STATUS';
			options='';
			options<-1>='ISSUED    (Waiting for approval)';
			options<-1>='APPROVED  (Waiting to start)';
			options<-1>='STARTED   (Work commenced)';
			options<-1>='FINISHED  (Work completed)';
			options<-1>='CANCELLED (Permanent stop)';
			options<-1>='ON HOLD   (Temporary stop)';
			reply=trim(decide('',options,replyn)[1,10]);
			if reply then data reply;

		case mode='VAL.STATUS';
	*/

	} else if (mode == "VAL.JOB") {
		if (win.is == win.isorig) {
			return 0;
		}

		gosub checkjob();
		if (not(win.valid)) {
			return 0;
		}

		jobcode = win.is;
		gosub checkjobunlocked2();
		if (not(win.valid)) {
			return 0;
		}

		jobcode = win.isorig;
		gosub checkjobunlocked2();
		if (not(win.valid)) {
			return 0;
		}

		//warning of other estimates on the job
		var estimatenos = job.a(10);
		if (estimatenos.a(1).locateusing(win.isorig, VM, xx)) {
			estimatenos.eraser(1, xx);
		}
		if (estimatenos) {
			msg = "Note: Job " ^ win.is ^ " also has|the following estimates:";
			if (estimatenos.count(VM) > 4) {
				estimatenos = estimatenos.field(VM, 1, 5) ^ VM ^ "etc.";
			}
			msg.r(-1, estimatenos);
			call note(msg);
		}

		//copy the job description to description, and brief to details
		if (RECORD.a(3) == "") {

			//copy job description to quote description
			if (RECORD.a(6) and job.a(9, 1)) {
				//if decide('Get the "description" from the job ?','',reply) else return
				reply = 1;
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
				//if decide('Use the job "brief" as the details ?','',reply) else return
				reply = 1;
				if (reply == 1) {
					RECORD.r(8, brief);
				}
			}

			//update the internal line count for text section
			win.displayaction = 5;
			win.amvaction = 4;
			win.amvvars.r(1, 3, RECORD.a(8).count(VM) + (RECORD.a(8) ne ""));
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
		if ((win.is == "") or (win.is == win.isorig)) {
			return 0;
		}
		win.is.writev(agy.brands, calculate("BRAND_CODE"), 7);
		

	} else if (mode == "DEF.DESCRIPTION") {

		if (not(win.wlocked)) {
			return 0;
		}

		if (job.read(agy.jobs, calculate("JOB_NO"))) {
			win.isdflt = job.a(9, 1);
			// if is=is.dflt then is=''
		}
	/*;
		CASE MODE='DEF.CURRENCY';
			IF IS ELSE IS.DFLT=BASE.CURRENCY.CODE;

		case mode='VAL.CURRENCY';
			if is=is.orig then return 0;
			call GENERAL.SUBS('VAL.CURRENCY');
			if valid else return 0;

			//option to convert amounts
			if is ne is.orig and sum(@record<3>) then;
				rate='';
				call note2('Exchange rate ? 1 ':is.orig:' = ? ':is:'|(enter "1/rate" for inverse rate)','RC',rate,'');
				if rate then;
					if rate[1,2]='1/' then rate=1/rate[3,999];
					if rate[1,1]='/' then rate=1/rate[2,999];
					if num(rate) else;
						msg='PLEASE ENTER A NUMERIC EXCHANGE RATE';
						return invalid(msg);
						end;
					readv ndecs from currencies,is,3 else ndecs=2;
					fmt='MD':ndecs:'0PZ';
					nlines=count(@record<3>,vm)+(@record<3> ne '');
					if rate then;
						for ln=1 to nlines;
							garbagecollect;
							if @record<3,ln> then;
								amount=@record<3,ln>;
								@record<3,ln>=(amount*rate) fmt;
								end;
							next ln;
						end;
					end;
				end;

			//force recalc of exch rate
			//redisplay.list=wi+1
			display.action=5;
			reset=3;
			@record<5>='';

		CASE MODE='DEF.EXCH.RATE';
			if is then return 0;
			//also used by PRODINVS
			READ CURRENCY FROM CURRENCIES,{CURR_CODE} ELSE;
				MSG='CURRENCY ':QUOTE({CURR_CODE}):' IS MISSING FROM THE CURRENCIES FILE';
				GOTO INVALID;
				END;
			IDATE={DATE}
			LOCATE IDATE IN CURRENCY<4> BY 'DR' SETTING VN ELSE NULL;
			RATE=CURRENCY<5,VN>;
			IF RATE='' THEN RATE=CURRENCY<5,VN-1>;
			@ans=RATE;
	*/
	} else if (mode == "VAL.AMOUNT") {
		if ((win.is == "") or (win.is == win.isorig)) {
			return 0;
		}
		if (not(ndecs.readv(gen.currencies, calculate("CURR_CODE"), 3))) {
			ndecs = "2";
		}
		if (ndecs == "") {
			ndecs = "2";
		}
		win.is = win.is.oconv("MD" ^ ndecs ^ "0P");

	} else if (mode == "POSTINIT") {
		gosub security2(mode, op);
		if (not(win.valid)) {
			return 0;
		}

	} else if (mode.substr(1,8) == "POSTREAD") {

		//option to read previous versions
		call generalsubs2(mode);
		if (not(win.valid)) {
			return 0;
		}

		//check allowed to create (or prevent editing if not allowed to update)
		gosub security2(mode, op);
		if (not(win.valid)) {
			return 0;
		}

		var jobno = RECORD.a(2);
		job = "";

		//restrict access based on company and brand etc.
		if (jobno) {

			if (not(validcode2(calculate("COMPANY_CODE"), "", calculate("BRAND_CODE"), agy.brands, msg))) {
				win.reset = 5;
				return invalid(msg);
			}

			msg = "Job estimate " ^ (DQ ^ (ID ^ DQ));

			//JOB ACCESS OTHERS
			if (not(validjob("ACCESS", jobno, job, msg))) {
				win.reset = 5;
				return invalid(msg);
			}

			if (win.wlocked and win.orec) {
				//JOB UPDATE OTHERS
				if (not(validjob("UPDATE", jobno, job, msg))) {
					win.reset = -1;
					call note(msg);
					gosub unlockrec();
				}
			}

		}

		if (win.wlocked and RECORD) {
			call agencysubs("CHKCLOSEDPERIOD." ^ mode, msg, "", "");
			if (msg) {
				//comment to client
				win.reset = -1;
				call note(msg);
				gosub unlockrec();
			}
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
		if (win.wlocked and ((calculate("INVOICE_NO") or calculate("INVOICE_NO_FAILSAFE")))) {
preventediting:
			xx = unlockrecord(win.datafile, win.srcfile, ID);
			win.wlocked = 0;

			//////
			return 0;
			//////

		}

		//draft mode only
		if ((win.wlocked and win.orec) and RECORD.a(11) ne "DRAFT") {
			if (not(authorised("PRODUCTION ESTIMATE ISSUE", xx))) {
				win.wlocked = 0;
				xx = unlockrecord(win.datafile, win.srcfile, ID);
			}
		}

		//prevent editing if not allowed to update other executives estimates
		if (win.wlocked) {
			var executive = calculate("EXECUTIVE_CODE");
			if (executive and executive ne USERNAME) {
				if (not(authorised("PRODUCTION ESTIMATE UPDATE OTHERS", msg))) {
					if (interactive) {
						msg = "NOTE:|YOU CANNOT EDIT THIS ESTIMATE BECAUSE|IT BELONGS TO " ^ executive;
						call note(msg);
					}
					goto preventediting;
				}
			}
		}

		//prevent editing if approved and not allowed to approve
		if (win.wlocked and (RECORD.a(11) == "APPROVED")) {
			if (not(authorised("PRODUCTION ESTIMATE APPROVE", msg))) {
				goto preventediting;
			}
		}

		//prevent certain users from creating their own quote numbers
		if (((win.wlocked and (RECORD == "")) and not interactive) and not USER3.index("RECORDKEY")) {
			if (not(authorised("PRODUCTION ESTIMATE CREATE OWN NO", msg))) {
				msg = DQ ^ (ID ^ DQ) ^ " does not exist and" ^ FM ^ FM ^ msg;
				win.reset = 5;
				return invalid(msg);
			}
		}

		//backwards compatible with one amount and text lines separated by tm
		//garbagecollect;
		if (not(RECORD.a(3).count(VM))) {
			if (RECORD.a(8).count(TM)) {
				tt = RECORD.a(8);
				tt.converter(TM, VM);
				tt = tt.oconv("T#60");
				tt.converter(TM, VM);
				while (true) {
				///BREAK;
				if (not(tt.index(" " ^ VM))) break;;
					tt.swapper(" " ^ VM, VM);
				}//loop;
				RECORD.r(8, tt);
				if (interactive) {
					call note("Note: The text and amount section has been converted|to a new format, but it may need some editing|to put the amounts back into the amount column.");
				}
			}
		}

		//get the number of decimals
		if (RECORD and (RECORD.a(14) == "")) {
			RECORD.r(14, calculate("NDECS"));
			win.orec.r(14, RECORD.a(14));
		}

		//default signatory into old records
		if (win.orec and (RECORD.a(24) == "")) {
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

		if (RECORD or (mode == "POSTREAD2")) {
			return 0;
		}

		//if decide('','Enter a new estimate':vm:'Copy other estimate(s)':vm:'Copy from order(s)':vm:'Copy order(s) from job',reply) else
		if (0) {
canc:
			win.valid = 0;
			win.reset = 4;
			win.wlocked = "";
			xx = unlockrecord(win.datafile, win.srcfile, ID);
			return 0;
		}

		if (reply == 1) {
			return 0;
		}

		if (reply == 2) {
			docname = "estimate";
			copyfile = win.srcfile;
			versionfilename = "PRODUCTION.INVOICE_VERSIONS";
		}else{
			docname = "order";
			copyfile = agy.productionorders;
			versionfilename = "PRODUCTION.ORDER_VERSIONS";
		}

		//select orders from a job
		if (reply == 4) {
			jobno = "";
inpjob:
			call note("Job number?", "RC", jobno, "");
			if (not jobno) {
				if (RECORD) {
					gosub initrec();
					return 0;
				}
				goto canc;
			}
			if (not(job.read(agy.jobs, jobno))) {
				call mssg(DQ ^ (jobno ^ DQ) ^ " job number does not exist");
				goto inpjob;
			}
			if (not(validcode2(job.a(14), "", job.a(2), agy.brands, msg))) {
				call mssg(msg);
				goto inpjob;
			}
			var datax = job.a(4);
			if (not datax) {
				call mssg("There are no production orders/invoices|on job " ^ (DQ ^ (jobno ^ DQ)) ^ " to copy");
				goto inpjob;
			}

			datax.converter(VM, FM);
			var norders = datax.count(FM) + 1;
			for (var ii = 1; ii <= norders; ++ii) {
				var prodorder;
				if (prodorder.read(agy.productionorders, datax.a(ii))) {
					datax.r(ii, 2, prodorder.a(10));
					datax.r(ii, 3, prodorder.a(6));
				}
			};//ii;
			datax.swapper(VM, " / ");
			datax.converter(VM, " ");

			//if decide('~Which do you want to copy|and in what order?',datax:'',reply) else goto inpjob
			reply = "";
			copynos = "";
			var nreplies = reply.count(FM) + 1;
			for (var replyn = 1; replyn <= nreplies; ++replyn) {
				copynos ^= " " ^ datax.a(reply.a(replyn)).field(" ", 1);
			};//replyn;
			copynos.splicer(1, 1, "");

			goto copyno2;

		}

		copynos = "";
copyno:
		call note("What are the number(s) of the|" ^ docname ^ "(s) that you want to copy ?", "RC", copynos, "");
copyno2:
		if (not copynos) {
			if (RECORD) {
				gosub initrec();
				return 0;
			}
			goto canc;
		}
		copynos.converter(",", " ");
		copynos.trimmer();

nextcopyno:
		var copyno = copynos.field(" ", 1);
		copynos.splicer(1, copyno.length() + 1, "");

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
		gosub checkjobunlocked2();
		if (not(win.valid)) {
			goto copyno;
		}

		//check job closed
		//jobno=copydoc<2>
		win.is = copydoc.a(2);
		win.isorig = "";
		win.valid = 1;
		call agencysubs("VAL.JOB", xx, "", "");
		if (not(win.valid)) {
			win.valid = 1;
			goto nextcopyno2;
		}

gotdoc:
		nn = RECORD.a(8).count(VM) + (RECORD.a(8) ne "") + 2;
		if (copyfile == win.srcfile) {
			if (RECORD) {
				RECORD.r(8, nn + 1, copydoc.a(8));
				RECORD.r(3, nn + 1, copydoc.a(3));
				RECORD.r(17, nn + 1, copydoc.a(17));
				RECORD.r(20, nn + 1, copydoc.a(20));
			}else{
				RECORD = copydoc;
			}
		}else{
			if (RECORD) {
				RECORD.r(8, nn, copydoc.a(13));
				RECORD.r(3, nn, copydoc.a(3));
			}else{
				RECORD.r(2, copydoc.a(2));
				RECORD.r(3, copydoc.a(3));
				RECORD.r(4, copydoc.a(4));
				RECORD.r(5, copydoc.a(5));
				RECORD.r(6, copydoc.a(6));
				RECORD.r(8, copydoc.a(13));
				RECORD.r(14, copydoc.a(14));
				//@record<15>=copydoc<>
			}
		}

		//clear log
		for (var ii = 30; ii <= 34; ++ii) {
			RECORD.r(ii, "");
		};//ii;

nextcopyno2:
		if (copynos) {
			goto nextcopyno;
		}

		//if decide('Copy more from another document ?','No':vm:'Yes',reply) then
		// if reply=2 then
		// copynos=''
		// goto copyno
		// end
		// end

		gosub initrec();

	} else if (mode == "PREDELETE") {

		msg = "Estimated cannot be deleted. Change the status to cancelled.";
		return invalid(msg);

	/*  production estimates cannot be deleted;

			gosub checkjobunlocked;
			if valid else return 0;

			//prevent removal from closed jobs
			if security('PRODUCTION ESTIMATE UPDATE AFTER JOB CLOSED',msg,'UP2') else;
				if job<7>='Y' then;
					return invalid(msg);
					end;
				end;

			GOSUB SECURITY2;
			if valid else return 0;

			call agency.subs('CHKCLOSEDPERIOD.':mode,msg,'','');
			if msg then;
				return invalid(msg);
				end;

			//update version log
			call general.subs2(mode);
	*/
	} else if (mode == "PREWRITE") {

		//prevent creating new records if no lic
		call chklic(mode, msg);
		if (msg) {
			return invalid(msg);
		}

		//prevent f9 while locked
		if (not(win.wlocked)) {
			//msg='WLOCKED is false in PRODINV.SUBS,PREWRITE'
			msg = "";
			return invalid(msg);
		}

		call agencysubs("CHKCLOSEDPERIOD." ^ mode, msg, "", "");
		if (msg) {
			return invalid(msg);
		}

		if ((RECORD.a(11) == "INVOICED") or calculate("INVOICE_NO_FAILSAFE")) {
			if (interactive) {
				gosub printx();
				win.valid = 0;
				return 0;
			}
			msg = "Estimate " ^ (DQ ^ (ID ^ DQ)) ^ " system failure during previous invoicing attempt";
			call sysmsg(msg);
			return invalid(msg);
		}

		//non-draft mode prevented in client but lets make sure
		if (RECORD.a(11) ne "DRAFT") {
			if (not(authorised("PRODUCTION ESTIMATE ISSUE", msg))) {
				return invalid(msg);
			}
		}

		//check currency exists and is not stopped
		var currcode = RECORD.a(4);
		if (not(gen.currency.read(gen.currencies, currcode))) {
			msg = DQ ^ (currcode ^ DQ) ^ " currency does not exist";
			return invalid(msg);
		}
		if (currcode ne win.orec.a(4)) {
			tt = gen.currency.a(1).convert("<>", "()");
			if (gen.currency.a(25) or tt.ucase().index("(STOP)")) {
				msg = tt ^ FM ^ "currency is stopped" ^ FM ^ gen.currency.a(25);
				return invalid(msg);
			}
		}

		//refresh exchange rate
		if (currcode ne agy.agp.a(2)) {
			//accountingdate={DATE}
			var accountingdate = var().date();
			//cannot update after invoiced so cannot get here
			//accountingdate={INVOICE_DATE}
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
					if (not(fin.taxes.a(2).locateusing(taxcodex, VM, taxn))) {
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

		//prevent using INTERNAL with tax codes for now
		//otherwise might screw up VAT entries
		//perhaps allow ONLY if the lines have matching VAT codes
		if (alltaxcodes and RECORD.a(20)) {
			msg = "Sorry, \"Internal\" cannot be used with tax codes yet";
			return invalid(msg);
		}

		//get/check job
		gosub checkjobunlocked();
		if (not(win.valid)) {
			return 0;
		}

		var companycode = job.a(14);
		call initcompany(companycode);

		//ensure tax code on estimates from 1/1/2018 if registered for tax
		if (gen.company.a(21) and (calculate("DATE") >= 18264)) {
			if (not(RECORD.a(42) or RECORD.a(44))) {
				call mssg(DQ ^ (ID ^ DQ) ^ " Tax/VAT code is required on Estimates/Invoices|for " ^ gen.company.a(1) ^ ", " ^ gen.company.a(21));
				return invalid(msg);
			}
		}

		//prevent addition to closed jobs
		if (job.a(7) == "Y") {
			if (not(authorised("PRODUCTION ESTIMATE CREATE AFTER JOB CLOSED", msg, "UP2"))) {
jobclosed:
				msg = DQ ^ (jobcode ^ DQ) ^ " is closed" ^ FM ^ FM ^ msg;
				return invalid(msg);
			}
		}

		//prevent creating many estimates per job (also in UI to avoid late message)
		if (not win.orec and job.a(10)) {
			if (not(authorised("PRODUCTION ESTIMATE CREATE MANY PER JOB", msg))) {
				msg = DQ ^ (jobcode ^ DQ) ^ " already has an estimate " ^ job.a(10) ^ FM ^ FM ^ msg;
				return invalid(msg);
			}
		}

		//if changing job code then ensure old job is not closed
		jobcode = win.orec.a(2);
		if (jobcode and jobcode ne RECORD.a(2)) {
			gosub checkjobunlocked2();
			if (not(win.valid)) {
				return 0;
			}

			//prevent removal from closed jobs
			if (job.a(7) == "Y") {
				if (not(authorised("PRODUCTION ESTIMATE UPDATE AFTER JOB CLOSED", msg, "UP2"))) {
					goto jobclosed;
				}
			}

			//prevent changing job if any po linked to this estimate
			var oldordnos = job.a(4);
			nn = oldordnos.count(VM) + (oldordnos ne "");
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
						return invalid(msg);
					}
				}
			};//ii;

		}

		//restore jobcode and job
		gosub checkjobunlocked();
		if (not(win.valid)) {
			return invalid(msg);
		}

		//identical code in prodinv.subs
		if (RECORD.a(4) == agy.agp.a(2)) {
			//make sure exchange rate for base currency is 1
			if (RECORD.a(5) ne 1) {
				RECORD.r(5, 1);
			}
		}

		if (not(RECORD.a(5))) {
			msg = "Exchange rate cannot be blank or zero";
			return invalid(msg);
		}

		//check production type
		tt = RECORD.a(17);
		if (tt.a(1, 1) == "") {
			//loop while tt[1,1]=vm do tt[1,1]='' repeat
			tt = trim(tt, VM, "B");
			if (tt.a(1, 1)) {
				RECORD.r(17, 1, tt.a(1, 1));
			}else{
				tt = calculate("JOB_PRODUCTION_TYPE");
				if (tt) {
					var prodtype;
					if (not(prodtype.read(agy.jobtypes, tt))) {
						prodtype = "";
					}
					if (not(prodtype.a(5))) {
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

		//check can add an account for APPROVED
		if (not(var("DRAFT,ISSUED,CANCELLED,INVOICED").locateusing(RECORD.a(11), ",", xx))) {
			call addjobacc("ADD", jobcode, msg);
			if (msg) {
				return invalid(msg);
			}
		}

		if (RECORD.index(var().chr(0).str(4))) {
			msg = "INTERNAL ERROR IN PRODUCTION ESTIMATE/INVOICE|PLEASE GIVE THIS MESSAGE TO NEOSYS|PLEASE REDO THIS ESTIMATE/INVOICE";
			call oswrite(RECORD, "prodinv.$$$");
			gosub invalid(msg);
			xx = unlockrecord(win.datafile, win.srcfile, ID);
			var().stop();
		}

		//update log and version file
		call generalsubs2(mode);

	} else if ((mode == "POSTWRITE") or (mode == "POSTDELETE")) {

		gosub updanalysis();

		call flushindex("PRODUCTION_INVOICES");

		if (interactive and (mode == "POSTWRITE")) {
			//if decide('Do you want a printout ?','',reply) else reply=2
			reply = 1;
			xx = unlockrecord(win.datafile, win.srcfile, ID);
			if (reply == 1) {
				perform("GET NEW PRODINVS " ^ ID);
			}
		}

	} else if (mode == "UPD.ANALYSIS") {
		gosub updanalysis();

	} else if (mode == "PRINT") {
		if (not ID) {
			msg = "PLEASE GET AN ESTIMATE ON SCREEN FIRST";
			return invalid(msg);
		}
		if (RECORD ne win.orec) {
			msg = "PLEASE SAVE THE RECORD FIRST";
			return invalid(msg);
		}
		gosub printx();

	} else {
		msg = DQ ^ (mode ^ DQ) ^ " invalid mode in PRODINV.SUBS";
		return invalid(msg);

	}
//L5171:
	return 0;

}

subroutine printx() {
	perform("GET NEW PRODINVS " ^ ID);
	return;

}

subroutine initrec() {
	win.valid = 1;
	RECORD.r(10, "");
	RECORD.r(11, "DRAFT");
	RECORD.r(12, "");
	RECORD.r(19, "");

	gosub resetlines();

	//gosub uplift
	win.valid = 1;

	return;

}

subroutine resetlines() {
	//update the internal line count for text section
	win.displayaction = 5;
	win.amvaction = 4;
	win.amvvars.r(1, 3, RECORD.a(8).count(VM) + (RECORD.a(8) ne ""));
	win.reset = 4;
	return;

}

subroutine updanalysis() {

	//add to analysis
	var sign = 1;
	gosub updanalysis2( sign);

	//remove old analysis
	if (mode.index("DELETE") or win.orec) {
		sign = -1;

		var storerecord = RECORD;
		win.orec.transfer(RECORD);

		gosub updanalysis2( sign);

		RECORD.transfer(win.orec);
		storerecord.transfer(RECORD);

	}

	return;

}

subroutine updanalysis2(in sign) {
	//updanalysis2(in sign)
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
	var prodtypecode = calculate("JOB_PRODUCTION_TYPE");
	for (var linen = 1; linen <= nlines; ++linen) {
		temp = prodtypecodes.a(1, linen);
		if (temp) {
			prodtypecode = temp;
		}else{
			prodtypecodes.r(1, linen, prodtypecode);
		}
		var amountbase = amountbases.a(1, linen);
		if (amountbase) {
			if (prodtypecodes.a(1).locateusing(prodtypecode, VM, linen2)) {
				if (linen2 < linen) {
					//garbagecollect;
					amountbases.r(1, linen2, (amountbases.a(1, linen2) + amountbase).oconv(basecurrencyfmtz));
					amountbases.r(1, linen, "");
				}
			}
		}
	};//linen;

	//update the analysis file

	var suppliercode = calculate("SUPPLIER_CODE");
	var brandcode = calculate("BRAND_CODE");
	var marketcode = calculate("MARKET_CODE");
	var orderperiod = calculate("DATE").oconv("D2/E").substr(4,5);
	var ordermthno = orderperiod.field("/", 1);

	var analcompanycode = calculate("COMPANY_CODE");
	if (analcompanycode == "") {
		analcompanycode = gen.gcurrcompany;
	}

	for (var linen = 1; linen <= nlines; ++linen) {
		var amountbase = amountbases.a(1, linen);
		if (amountbase) {
			prodtypecode = prodtypecodes.a(1, linen);
			var analkey = orderperiod.field("/", 2) ^ "*" ^ brandcode ^ "**" ^ marketcode ^ "*" ^ suppliercode ^ "*" ^ prodtypecode;
			analkey = analkey.fieldstore("*", 8, 1, analcompanycode);

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

	return;

}

subroutine checkjob() {

	if (authorised("PRODUCTION ESTIMATE CREATE AFTER JOB CLOSED", msg, "UP2")) {
		temp = "";
	}else{
		temp = ".OPEN";
	}
	//call agency.subs(mode)
	call agencysubs("VAL.JOB" ^ temp, xx, "", "");
	if (not(win.valid)) {
		return;
	}

	//get the job
	if (not(job.read(agy.jobs, win.is))) {
		msg = DQ ^ (win.is ^ DQ) ^ " JOB DOES NOT EXIST";
		gosub invalid(msg);
		return;
	}

	//warning if job closed
	if (job.a(7) == "Y") {
		msg = "WARNING: Job " ^ (DQ ^ (win.is ^ DQ)) ^ " is closed.";
		call note(msg);
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
		gosub invalid(msg);
		return;
	}
	if (lockrecord("JOBS", agy.jobs, jobcode)) {
		xx = unlockrecord("JOBS", agy.jobs, jobcode);
	}else{
		msg = "Someone is updating job " ^ (DQ ^ (jobcode ^ DQ)) ^ FM ^ "Please wait and try again later";
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
