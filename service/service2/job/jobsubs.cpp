#include <exodus/library.h>
libraryinit()

#include <agencysubs.h>
#include <initcompany.h>
#include <authorised.h>
#include <generalsubs.h>
#include <chklic.h>
#include <updsecindex.h>
#include <flushindex.h>
#include <generalsubs2.h>
#include <validcode2.h>
#include <validjob.h>
#include <jobanalysis.h>
#include <updtasks.h>
#include <jobsubs.h>
#include <singular.h>

#include <gen.h>
#include <agy.h>
#include <win.h>

#include <window.hpp>

var xx;
var msg;
var pikeys;
var yy;
var otherusercode;
var wsmsg;

function main(in mode) {
	//c job

	//global pikeys

	var interactive = not SYSTEM.a(33);

	if (mode == "PERPETUAL") {

		if (win.orec == "" and (RECORD.a(4) or RECORD.a(5))) {
			//gosub initrec
			RECORD.r(4, "");
			RECORD.r(5, "");
			RECORD.r(6, "");
			RECORD.r(7, "");
			RECORD.r(10, "");
			win.displayaction = 5;
			win.reset = 4;
			DATA ^= "\r";
		}

	} else if (mode.field("*", 1, 3) == "GETLASTJOBNO") {
		var previous = 1;
		var compcode = mode.field(".", 2);
		//goto getnextjobno
		var storedatafile = win.datafile;
		var storesrcfile = win.srcfile;
		win.datafile = "JOBS";
		if (not(win.srcfile.open(win.datafile, ""))) {
			call fsmsg();
		}
		call agencysubs("GETNEXTID." ^ compcode ^ "." ^ previous, xx);
		win.datafile = storedatafile;
		win.srcfile = storesrcfile;

	} else if (mode.field(".", 1, 3) == "DEF.JOB.NO") {
		//pretty much identical in prodorder.subs, prodinv.subs and even plan.subs
		//call general.subs('DEF.SK')

		if (win.wlocked or RECORD) {
			return 0;
		}

		var previous = 0;

		var compcode = mode.field(".", 4);
		//gosub getnextjobno

		if (compcode ne gen.gcurrcompany) {
			call initcompany(compcode);
		}

		call agencysubs("GETNEXTID." ^ compcode, xx);
		win.isdflt = ANS;

		if (not interactive) {
			ID = win.isdflt;
		}

	} else if (mode == "VAL.JOB.NO") {

		if (win.is == "" or win.is == win.isorig) {
			return 0;
		}

		var job;
		if (job.read(win.srcfile, win.is)) {

			//validcode2( validcode3( done in postread now

		}else{

			//if not a multilevel job then check if allowed to create own job numbers
			if (win.is ne win.registerx(1)) {
				var nlevels = win.is.count("-");
				if (nlevels == 0) {
					if (not(authorised("JOB CREATE OWN NO", msg, "UP"))) {
						msg = DQ ^ (ID ^ DQ) ^ " does not exist and" ^ FM ^ FM ^ msg;
						return invalid(msg);
					}
				}
			}

			//in case somebody else just used the next key
			//zzz only works for simple numeric job nos at the moment
			if (win.is.isnum()) {
lockit:
				if (win.is == win.registerx(1)) {
					if (lockrecord("JOBS", win.srcfile, win.is)) {
						xx = unlockrecord("JOBS", win.srcfile, win.is);
					}else{
						win.is += 1;
						goto lockit;
					}
				}
			}

			call generalsubs("VAL.ALPHANUMERIC./-");
		}

	} else if (mode == "F2.JOBS") {
		call agencysubs(mode, xx);

	} else if (mode == "F2.BRANDS") {
		//from the jobq template
		var clientcodes = RECORD.a(23);
		clientcodes.converter(VM, "*");
		if (clientcodes) {
			clientcodes.splicer(1, 0, ",");
		}
		call agencysubs("F2.BRANDS" ^ clientcodes, xx);

	} else if (mode == "DEF.COMPANY") {
		var temp = agy.agp.a(19);
		if (temp) {
			win.isdflt = temp;
		}else{
			call generalsubs("DEF.COMPANY");
		}

	} else if (mode == "VAL.COMPANY") {
		if (win.isorig and win.is ne win.isorig) {
			if (win.registerx(1)) {
				msg = "YOU CANNOT CHANGE THE COMPANY AFTER|RECEIVING SUPPLIER INVOICES OR ISSUING INVOICES";
				return invalid(msg);
			}
		}
		call generalsubs("VAL.COMPANY");

	} else if (mode == "VAL.CLOSED") {

		//check if allowed to reopen the job
		if (win.is ne win.isorig and win.isorig == "Y") {
			if (not(authorised("JOB REOPEN", msg, "UP"))) {
				return invalid(msg);
			}
		}

	} else if (mode == "DEF.EXECUTIVE") {
		if (authorised("JOB UPDATE EXECUTIVE", msg, "UP")) {
			ANS = calculate("BRAND_CODE").xlate("BRANDS", 11, "X");
		}else{
			if (not win.is) {
				if (ANS == "") {
					ANS = USERNAME;
				}
			}
			win.si.r(18, "P");
		}

	} else if (mode == "PREWRITE") {

		//prevent creating new records if no lic
		call chklic(mode, msg);
		if (msg) {
			return invalid(msg);
		}

		//if {INVOICE_NUMBER} then
		// if {INV_DATE}='' then
		// msg='INVOICE DATE IS REQUIRED'
		// goto invalid
		// end
		// end
		//@record<9>=''

		//check if allowed without type
		if (not RECORD.a(3)) {
			if (not(authorised("JOB CREATE WITHOUT TYPE", msg))) {
				return invalid(msg);
			}
		}

		var brand;
		if (not(brand.read(agy.brands, calculate("BRAND_CODE")))) {
			msg = DQ ^ (calculate("BRAND_CODE") ^ DQ) ^ " brand does not exist";
			return invalid(msg);
		}

		call agencysubs("CHKCLOSEDPERIOD." ^ mode, msg);
		if (msg) {
			return invalid(msg);
		}

		//check if allowed to reopen the job
		if (RECORD.a(7) ne win.orec.a(7) and win.orec.a(7) == "Y") {
			if (not(authorised("JOB REOPEN", msg))) {
				return invalid(msg);
			}
		}

		//ensure production order and invoices index on job brand and executive
		//are updated by moving their keys elsewhere
		//and reinstating them in postwrite
		if (win.orec) {

			if (RECORD.a(2) ne win.orec.a(2) or RECORD.a(8) ne win.orec.a(8)) {

				//the sym indexes happen to be the same for both files
				var symindexfields = "BRAND_CODE" ^ FM ^ "EXECUTIVE_CODE";
				var oldfields = win.orec.a(2) ^ FM ^ win.orec.a(8);
				var newfields = RECORD.a(2) ^ FM ^ RECORD.a(8);

				//lock all associated production orders and invoices
				//otherwise unlock, fail and tell user to unlock assoc records and try again
				//nb if lock fails then only the locked keys are returned to be unlocked
				var pokeys = RECORD.a(4);
				call updsecindex("LOCK", "PRODUCTION_ORDERS", pokeys, "", "", "", win.valid, msg);
				if (win.valid) {
					pikeys = RECORD.a(10);
					call updsecindex("LOCK", "PRODUCTION_INVOICES", pikeys, "", "", "", win.valid, msg);

					if (not win.valid) {
unlockprodinvs:
						call updsecindex("UNLOCK", "PRODUCTION_INVOICES", pikeys, "", "", "", xx, yy);
						goto unlockprodorders;
					}

				}else{

unlockprodorders:
					call updsecindex("UNLOCK", "PRODUCTION_ORDERS", pokeys, "", "", "", xx, yy);
					msg = "You cannot change brand or executive while|" ^ msg;
					return invalid(msg);
					return 0;
				}

				//lock the !indexing record as late as possible
				call updsecindex("INDEXINGLOCK", "", xx, "", "", "", win.valid, msg);
				if (not win.valid) {
					goto unlockprodinvs;
				}

				//updates from here on so cannot abort
				/////////////////////////////////////
				if (pokeys) {
					call updsecindex("UPDATE", "PRODUCTION_ORDERS", pokeys, symindexfields, oldfields, newfields, xx, yy);
				}
				if (pikeys) {
					call updsecindex("UPDATE", "PRODUCTION_INVOICES", pikeys, symindexfields, oldfields, newfields, xx, yy);
				}

				//unlock everything
				//(not needed now that update unlocks as it goes
				//call upd.secindex('UNLOCK','PRODUCTION_ORDERS',pokeys)
				//call upd.secindex('UNLOCK','PRODUCTION_INVOICES',pikeys)

				//unlock the indexing record
				call updsecindex("INDEXINGUNLOCK", "", xx, "", "", "", xx, yy);

				if (pokeys) {
					call flushindex("PRODUCTION_ORDERS");
				}
				if (pikeys) {
					call flushindex("PRODUCTION_INVOICES");
				}

			}

		}

		//restore hidden cost and income info
		//not really needed in AREV as cross referenced fields cannot be updated
		if (win.orec) {
			//have to retrieve original orec
			if (win.orec.read(agy.jobs, ID)) {
				RECORD.r(4, win.orec.a(4));
				RECORD.r(10, win.orec.a(10));
			}
		}

		//record date created
		if (not RECORD.a(54)) {
			RECORD.r(54, var().date() ^ "." ^ (var().time()).oconv("R(0)#5"));
		}

		var brief = RECORD.a(9);
		if (brief) {
			while (true) {
			///BREAK;
			if (not(brief.substr(-6,6) == "<br />")) break;;
				brief.splicer(-6, 6, "");
			}//loop;
			brief.converter(TM, VM);
			RECORD.r(9, brief);
		}

		//closed is now 0 for open
		if (not RECORD.a(7).trim()) {
			RECORD.r(7, 0);
		}

		//if only added files then dont add log
		var tt = RECORD;

		//remove duplicate income and cost info
		RECORD.r(26, "");
		RECORD.r(27, "");

		//missing username etc indicates adding file archive entry
		var ii = (RECORD.a(43)).count(VM) + (RECORD.a(43) ne "");
		if (ii and RECORD.a(40, ii) == "") {
			RECORD.r(40, ii, USERNAME);
			RECORD.r(41, ii, var().date() ^ "." ^ (var().time()).oconv("R(0)#5"));
			RECORD.r(42, ii, STATION);

		}else{

			//update version log
			call generalsubs2(mode);

		}
	/*;
		case mode='PREINIT';

			//if security('PRODUCTION COST ACCESS',msg,'UP') else
			// open 'ADTEMPLATES' to adtemplates else return 0
			// read @record from adtemplates,'JOBS2' else
			//  call fsmsg()
			//  stop
			//  end
			// end

		CASE MODE='POSTINIT';
			GOSUB SECURITY;
			if valid else return 0;

			gosub getwis;

			//protect the job number
			if index(agp<53>,'<COMPANY>',1) then;
				company<14>=gcurr.company;
				w(wis<14>)<18>='FP';
				end;
	*/

	} else if (mode == "POSTREAD" or mode == "POSTREAD2") {

		//option to read previous versions
		call generalsubs2(mode);
		if (not win.valid) {
			return 0;
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

		//hide costs and/or income
		win.registerx(1) = "";
		if (win.orec) {
			var apc = authorised("PRODUCTION COST ACCESS", msg, "");
			if (apc) {
				apc = authorised("PRODUCTION ORDER ACCESS", msg, "");
			}
			var api = authorised("PRODUCTION ESTIMATE ACCESS", msg, "");

			//flag if has orders or quotes before clearing them
			//hasordorinv=(@record<4> or @record<10>)
			MV = 0;
			var tt = calculate("ORDER_INV_NO") ^ calculate("QUOTE_INV_NO");
			tt.converter(VM ^ SVM, "");
			win.registerx(1) = tt ne "";
			RECORD.r(15, win.registerx(1));

			if (not apc) {
				RECORD.r(4, "");
			}
			if (not api) {
				RECORD.r(10, "");
			}
			win.orec = RECORD;
		}

		//check if allowed to create/update/access in general
		gosub security(mode);
		if (not win.valid) {
			win.reset = 5;
			return 0;
		}

		//checks on new records
		if (win.orec == "") {

			if (win.wlocked) {

				//needs work on the call to check @ID not is
				call generalsubs("VAL.ALPHANUMERIC./-");

				//check if allowed to create main jobs
				//and if allowed to create own job numbers
				var nlevels = ID.count("-");
				if (nlevels == 0) {

					//no levels so check if can create master jobs
					if (not(authorised("JOB CREATE MASTER JOBS", msg, "UP"))) {
						msg = DQ ^ (ID ^ DQ) ^ " does not exist and" ^ FM ^ FM ^ msg;
						win.reset = 5;
						return invalid(msg);
					}

					//prevent certain users from creating their own job numbers
					//uncomment when job number is not generated in the client
					//uncommented to prevent own job number creation
					if (RECORD == "" and not interactive and not USER3.index("RECORDKEY", 1)) {
						if (not(authorised("JOB CREATE OWN NO", msg, ""))) {
							msg = DQ ^ (ID ^ DQ) ^ " does not exist and" ^ FM ^ FM ^ msg;
							win.reset = 5;
							return invalid(msg);
						}
					}

				}else{

					//check that all higher jobs exist, are authorised, and are not closed
					for (var leveln = 1; leveln <= nlevels; ++leveln) {

						var topjobno = ID.field("-", 1, leveln);

						//check higher job exists
						var topjob;
						if (not(topjob.read(agy.jobs, topjobno))) {
							msg = DQ ^ (topjobno ^ DQ) ^ " Main Job does not exist.|You must create it first";
							win.reset = 5;
							return invalid(msg);
						}

						//check higher job access authorised
						if (not(validcode2(topjob.a(14), "", topjob.a(2), agy.brands, msg))) {
							win.reset = 5;
							return invalid(msg);
						}

						//check higher job is open
						//if topjob<7>='Y' then
						// msg=quote(topjobno):' is closed'
						// reset=5
						// goto invalid
						// end

					};//leveln;

				}

			}

			//force companycode if new record and key prefixed with company code
			//similar code in JOB.SUBS and PLAN.SUBS
			//PO/PI should be tied to jobs of the same company if prefixed
			if (ID == win.isdflt and win.wlocked) {
				var ccode2 = gen.company.a(28);
				if (ccode2) {
					var tt = ID;
					tt.converter("0123456789-/", "            ");
					if (tt.field(" ", 1) == ccode2) {

						//job file company codes
						RECORD.r(14, gen.gcurrcompany);
						RECORD.r(28, RECORD.a(14));
						//removed so orec is ''
						//orec<14>=r<14>
						//orec<28>=r<14>

					}
				}
			}

			//checks on existing records
		}else{

			//JOB ACCESS OTHERS
			if (not(validjob("ACCESS", ID, RECORD, msg))) {
				win.reset = 5;
				return invalid(msg);
			}

			//restrict access based on company and brand
			if (not(validcode2(RECORD.a(14), "", RECORD.a(2), agy.brands, msg))) {
				win.reset = 5;
				return invalid(msg);
			}

			if (win.wlocked and win.orec) {
				//JOB UPDATE OTHERS
				msg = "";
				if (not(validjob("UPDATE", ID, RECORD, msg))) {
					win.reset = -1;
					gosub unlockrec();
					call note(msg);
				}
			}

			if (win.wlocked and RECORD.a(5)) {
				msg = "";
				if (not(authorised("JOB UPDATE AFTER INVOICING", msg, "UP"))) {
					call note(msg);
					win.reset = -1;
					gosub unlockrec();
				}
			}

			call jobanalysis();

		}

		if (RECORD and not interactive) {

			//fix a old bug where file details where updated instead of log on first write
			if (RECORD.a(30) == "" and RECORD.a(43) == "" and RECORD.a(40)) {
				for (var ii = 30; ii <= 32; ++ii) {
					RECORD.r(ii, RECORD.a(ii + 10));
					RECORD.r(ii + 10, "");
				};//ii;
				RECORD.r(33, "A");
				//'OPEN' is coded in general.subs2 and job.subs postread
				RECORD.r(34, "OPEN");
			}

			var brief = RECORD.a(9);
			if (brief) {
				brief.converter(VM, TM);

				//convert old text to html format for rich text edit
				var tt = brief.field(TM, 2, 9999);
				if (tt) {
					//if index(tt,'<p>',1) else
					// swap tm with '</p><p>' in tt
					// brief=field(brief,tm,1,1):tm:'<p>':tt:'</p>'
					// end
					if (not(tt.index("<br />", 1))) {
						tt.swapper(TM, "<br />");
						brief = brief.field(TM, 1) ^ TM ^ tt;
					}
					while (true) {
					///BREAK;
					if (not(brief.substr(-6,6) == "<br />")) break;;
						brief.splicer(-6, 6, "");
					}//loop;
				}

				RECORD.r(9, brief);
			}

			//get the quote statuses (to check if can add po's)
			if (not win.wlocked) {
				MV = 0;
				RECORD.r(16, calculate("QUOTEmv.STATUS2"));
			}

			var costids = RECORD.a(4);
			var ncosts = costids.count(VM) + (costids ne "");
			for (MV = 1; MV <= ncosts; ++MV) {
				var cost = "";
				cost.r(1, 1, 1, calculate("ORDER_DATE"));
				cost.r(1, 1, 2, calculate("ORDER_AMOUNT") ^ calculate("ORDER_CURRENCY"));
				cost.r(1, 1, 3, calculate("ORDER_AMOUNT_BASE"));
				cost.r(1, 1, 4, calculate("ORDER_DESC"));
				var orderinvno = calculate("ORDER_INV_NO");
				cost.r(1, 1, 5, orderinvno);
				cost.r(1, 1, 6, calculate("ORDER_INV_DATE"));
				cost.r(1, 1, 7, calculate("SUPPLIER_NAME"));

				var status = calculate("ORDERmv.STATUS2");
				//similar logic in printjob and jobs.subs/postread
				if (orderinvno) {
					status = "INVOICE";
				}
				if (not status) {
					status = "ISSUED";
				}
				cost.r(1, 1, 8, status);
				cost.r(1, 1, 9, calculate("ORDER_ESTIMATE_NO"));

				RECORD.r(26, MV, cost);

			};//MV;

			var billids = RECORD.a(10);
			var nbills = billids.count(VM) + (billids ne "");
			for (MV = 1; MV <= nbills; ++MV) {
				var bill = "";
				bill.r(1, 1, 1, calculate("QUOTE_DATE"));
				bill.r(1, 1, 2, calculate("QUOTE_AMOUNT") ^ calculate("QUOTE_CURRENCY"));
				bill.r(1, 1, 3, calculate("QUOTE_AMOUNT_BASE"));
				bill.r(1, 1, 4, calculate("QUOTE_DESC"));
				bill.r(1, 1, 5, calculate("QUOTE_INV_NO"));
				bill.r(1, 1, 6, calculate("QUOTE_INV_DATE"));
				bill.r(1, 1, 8, calculate("QUOTEmv.STATUS2"));
				bill.r(1, 1, 9, calculate("QUOTE_PROFORMA_NO"));
				RECORD.r(27, MV, bill);

			};//MV;

		}

	} else if (mode == "PREDELETE") {

		msg = "Jobs cannot be deleted. Change the status of all orders and estimates to cancelled.";
		return invalid(msg);

	} else if (mode == "POSTWRITE") {
		call flushindex("JOBS");

		if (win.orec == "") {

			//allocate task to user in order of preference
			//1. executive if they are in the users file
			//2. job creator
			var taskusercode = RECORD.a(8);
			if (not(taskusercode.xlate("USERS", 1, "X"))) {
				taskusercode = USERNAME;
			}

			var taskid = ID ^ "*1";

			var task = "";
			task.r(1, ID);
			task.r(2, taskusercode);
			task.r(3, "Allocated");
			task.r(4, var().date() ^ "." ^ (var().time()).oconv("R(0)#5"));

			//parent user for job closure
			task.r(8, agy.agp.a(106));

			//add a task for the exectutive or user
			call updtasks("ADD", taskid, task, msg);
			if (msg) {
				call note(msg);
			}

			//add automatic child task(s) for other departments/users
			var otherusercodes = "";
			if (otherusercodes) {
				var notherusers = otherusercodes.count(VM);

				//connect subtask to main task
				task.r(7, taskid);
				task.r(2, otherusercode);

				for (var otherusern = 1; otherusern <= notherusers; ++otherusern) {
					otherusercode = otherusercodes.a(1, otherusern);
					if (xlate("USERS", otherusercode, "LIVE_USER_WITH_EMAIL", "X")) {

						call updtasks("ADD", xx, task, msg);
						if (msg) {
							call note(msg);
						}

					}
				};//otherusern;

			}

		}

		win.orec = RECORD;
		call jobsubs("POSTREAD2");

	} else if (mode == "POSTDELETE") {
		call flushindex("JOBS");

	} else if (mode == "PRINTJOB") {
		if (RECORD ne win.orec) {
			msg = "Please save your changes to this job first";
			return invalid(msg);
		}

	//printjob:
		perform("GET NEW PRINTJOB " ^ ID);

	} else {
		msg = DQ ^ (mode ^ DQ) ^ " invalid mode in job.subs";
		return invalid(msg);
	}
//L4012:
	return 0;

	/*;
	///////////
	setcompany:
	///////////
		if r<14> else return 0;
		gosub setcompany2;
		return 0;

	////////////
	setcompany2:
	////////////
		if index(agp<53>,'<COMPANY>',1) then;
			r<14>=gcurr.company;
			r<28>=r<14>;
			end;

		return 0;
	*/

}

subroutine unlockrec() {
	xx = unlockrecord(win.datafile, win.srcfile, ID);
	win.wlocked = 0;
	return;

}


libraryexit()