#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <addcent.h>
#include <anydata.h>
#include <valacc.h>
#include <generalsubs2.h>
#include <companysubs.h>
#include <initcompany.h>
#include <getreccount.h>
#include <quote2.h>
#include <singular.h>

#include <fin_common.h>
#include <gen_common.h>
#include <win_common.h>

#include <window.hpp>

var msg;
var langcode;
var reply;//num
var temp;
var anybalancesfound;//num
var compcode;
var xx;
var ncompanies;
var companycodes;
var t2;
var tt;
var op;
var deleteall;
var deletemode;
var ii;//num
var oldtaxacc;
var newtaxacc;
var deleting;//num
var vn;
var op2;
var wspos;
var wsmsg;
var wis0;
var wis;

function main(in mode) {
	//c gen
	//global deleting

	#include <common.h>
	#include <general_common.h>

	#define prikey "1000" ^ "100"

	var interactive = not(SYSTEM.a(33));

	if (mode == "DEF.COMPANY.CODE") {
		win.isdflt = fin.currcompany;

		//fix problem with data from previous record
		ANS = "";

	} else if (mode == "VAL.COMPANY.CODE") {
		win.is.trimmerf();
		gosub badchars(msg);
		if (msg) {
			return invalid(msg);
		}

	} else if (mode == "F2.LANGUAGE") {
		select(fin.alanguage);
		var langcodes = "";
nextlang:
		if (readnext(langcode)) {
			if (langcode == "GENERAL") {
				langcode = "GENERAL*ENGLISH";
			}
			if (langcode.match("^GENERAL\\*.*$")) {
				langcodes.r(1, -1, langcode.substr(9,99));
			}
			goto nextlang;
		}
		if (not langcodes) {
			msg = "NO LANGUAGES ARE AVAILABLE|PLEASE LEAVE THIS LINE BLANK";
			ANS = "";
			return 0;
		}
		if (decide("", langcodes, reply)) {
			ANS = langcodes.a(1, reply);
		}else{
			ANS = "";
		}

	} else if (mode == "VAL.LANGUAGE") {
		if (not(win.is)) {
			return 0;
		}
		temp = "*" ^ win.is;
		if (temp == "*ENGLISH") {
			temp = "";
		}
		if (not(temp.read(fin.alanguage, "GENERAL" ^ temp))) {
			msg = win.is.quote() ^ " LANGUAGE IS NOT AVAILABLE|PRESS [F2] TO SEE AVAILABLE LANGUAGES";
			return invalid(msg);
		}

	} else if ((mode == "VAL.PERIOD") or (mode == "VAL.PERIOD2")) {
		if (not(win.is)) {
			return 0;
		}
		if (win.is == win.isorig) {
			return 0;
		}
		if (gen.companies == "") {
			win.is = win.isorig;
			msg = "You cannot change the period except while in accounts";
			return invalid(msg);
		}

		//no leading zero
		if (win.is[1] == "0") {
			win.is.splicer(1, 1, "");
		}

		//check maximum month
		temp = win.is.field("/",1);
		if (not(temp.match("^\\d*$"))) {
			goto badperiod;
		}
		if (temp < 1) {
badperiod:
			msg = "PERIOD OR PERIOD/YEAR PLEASE. EG 1/2000, 12/2000";
			return invalid(msg);
		}
		if (fin.maxperiod and (temp > fin.maxperiod)) {
			goto badperiod;
		}

		//check the year
		temp = win.is.field("/", 2).oconv("R(0)#2");
		if (temp == "") {
			temp = fin.curryear;
			if (temp == "") {
				temp = var().date().oconv("D2").substr(-2,2);
			}
		}
		win.is = win.is.fieldstore("/", 2, 1, temp);

		if (not(temp.match("^\\d{2}$"))) {
			goto badperiod;

		} else if (mode == "VAL.PERIOD2") {
			var currentclosedperiod = win.orec.a(16);
			if (currentclosedperiod) {

				var oldyear = currentclosedperiod.substr(-2,2);
				if (currentclosedperiod.field("/", 1) == fin.maxperiod) {
					oldyear += 1;
				}
				var newyear = win.is.substr(-2,2);
				if (win.is.field("/", 1) == fin.maxperiod) {
					newyear += 1;
				}
				if (newyear < oldyear) {
					if (not(authorised("LEDGER REOPEN CLOSED YEARS", msg, ""))) {
						return invalid(msg);
					}
				}
				if (addcent(iconv(win.is, gen.company.a(6))) < addcent(iconv(currentclosedperiod, gen.company.a(6)))) {
					if (not(authorised("LEDGER REOPEN CLOSED PERIODS", msg, ""))) {
						return invalid(msg);
					}
				}
			}

		//prevent change to future year
		} else if ((win.orec and win.isorig) and (addcent(temp) > addcent(win.isorig.field("/", 2)))) {
			gosub anybalances();
			if (anybalancesfound) {
				msg = "If you want to change the current year into the ";
				msg.r(-1, "next year, please use the \"Open New Year\" program.");
				if (interactive) {
					if (not(var("012").index(PRIVILEGE))) {
						return invalid(msg);
					}
					if (not(decide("Do you want to carry|forward the closing balances ?", "", reply))) {
						reply = 1;
					}
					if (reply == 1) {
						return invalid(msg);
					}
				}else{
					return invalid(msg);
				}
			}

		//prevent change to prior year
		} else if (win.orec and (addcent(temp) < addcent(win.isorig.field("/", 2)))) {
			gosub anybalances();
			if (anybalancesfound) {
				msg = "You cannot change the current year into the previous year";
				msg.r(-1, "|(If you want to post into a previous year,    ");
				msg.r(-1, " enter the period in the heading of the batch)");
				return invalid(msg);
			}

		}

	} else if (mode == "DEF.BASECURR") {
		if (not(win.is)) {
			ANS = fin.basecurrency;
		}

		} else if (mode == "VAL.BASECURR") {

		//prevent change of base currency
		if (win.orec and win.is ne win.isorig) {
			gosub anybalances();
			if (not anybalancesfound) {
				//if xlate('VOC','ANYDATA',1,'X') then call anydata('',anybalancesfound)
				call anydata("", anybalancesfound);
			}
			if (anybalancesfound) {
				msg = "You cannot change the base currency|after data has been entered.";
				return invalid(msg);
			}
		}

		//all companies must have the same base currency
		if (win.is ne win.isorig) {
			select(gen.companies);
checknextcompany:
			if (readnext(compcode)) {
				if (compcode ne ID) {
					if (temp.read(gen.companies, compcode)) {
						if (win.is ne temp.a(3)) {
							msg = "All companies must have the same base currency";
							msg.r(-1, "|but " ^ (temp.a(1).quote()) ^ "|has base currency " ^ (temp.a(3).quote()));
							return invalid(msg);
						}
					}
				}
				goto checknextcompany;
			}
		}

	// CASE MODE EQ 'F2.ACC'
	//  *must be in same company
	//  CALL GETACC('','',@ID,'','','')

	} else if (mode == "VAL.ACC") {
		//IF IS EQ IS.ORIG THEN RETURN
		if (not(win.is)) {
			return 0;
		}
		if (not(valacc(xx, win.is, win.isorig, "", msg))) {
			return invalid(msg);
		}

		//check not a control account
		if (fin.account.a(7)) {
			msg = "You cannot use the control accounts of other ledgers here";
			return invalid(msg);
		}

		//check not other company
		if (fin.account.a(5) and fin.account.a(5) ne ID) {
			msg = "Account " ^ win.is.a(1, 1, 1) ^ " belongs to company " ^ fin.account.a(5);
			return invalid(msg);
		}

	} else if (mode == "VAL.TAX") {
		//check and convert the nominal account
		if (not(valacc(xx, win.is, win.isorig, "", msg))) {
			win.valid = 0;
			return 0;
		}

		//the control cannot be a controlled account or be in this ledger
		if (win.is.a(1, 1, 2) and win.is.a(1, 1, 2) ne win.isorig.a(1, 1, 2)) {

			//check not a control account
			if (fin.account.a(7)) {
				msg = "You cannot use the control accounts of other ledgers here";
				return invalid(msg);
			}

			//check that the control is in a main ledger
			//IF ACCOUNT<6> THEN
			// MSG='The tax account cannot be in a sub ledger'
			// GOTO INVALID
			// END

			//check not other company
			if (fin.account.a(5) and fin.account.a(5) ne ID) {
				msg = "The tax account belongs to company " ^ fin.account.a(5);
				return invalid(msg);
			}

			gosub getcompanies();

			var buff = "";
			if (interactive) {
				msg = "SEARCHING FOR ANY POSTINGS TO THIS ACCOUNT";
				msg.r(-1, "|PLEASE WAIT:");
				call note(msg, "UB", buff, "");
			}
			for (var companyn = 1; companyn <= ncompanies; ++companyn) {
				compcode = companycodes.a(companyn);
				//FOR YEARN=addcent(CURRYEAR) TO addcent(CURRYEAR-2) STEP -1
				t2 = addcent(fin.curryear);

				for (var yearn = t2; yearn >= addcent(fin.curryear - 3); --yearn) {
					for (var periodn = 1; periodn <= fin.maxperiod; ++periodn) {
						if (esctoexit()) {
							win.valid = 0;
							if (interactive) {
								call mssg("", "DB", buff, "");
							}
							return 0;
						}
						var keyx = yearn.substr(-2,2) ^ ("00" ^ periodn).substr(-2,2) ^ "*" ^ win.is.a(1, 1, 2) ^ "*" ^ compcode;

						print(AW.a(30), var().at(CRTWIDE / 2 - 15, CRTHIGH / 2), ("Period: " ^ periodn ^ "/" ^ yearn ^ "  Company: " ^ compcode).oconv("C#30"));
						var rec;
						if (rec.read(fin.voucherindex, keyx)) {
							if (rec) {
								if (interactive) {
									call mssg("", "DB", buff, "");
								}
								msg = "You have already made postings to account " ^ win.is.a(1, 1, 1);
								msg.r(-1, "in period:" ^ periodn ^ "/" ^ yearn ^ "  company:" ^ (compcode.quote()));
								msg.r(-1, "so it cannot now be used as a tax control account");
								return invalid(msg);
							}
						}
					};//periodn;
				};//yearn;
			};//companyn;
			if (interactive) {
				call mssg("", "DB", buff, "");
			}

		}

	} else if (mode == "VAL.OTHER.COMPANY") {
		if (win.is == win.isorig) {
			return 0;
		}
		if (not(win.is)) {
			return 0;
		}
		if (not(temp.read(gen.companies, win.is))) {
			msg = win.is.quote() ^ " COMPANY CODE NOT ON FILE";
			return invalid(msg);
		}
		if (temp.a(3) ne RECORD.a(3)) {
			msg = "OTHER COMPANIES MUST HAVE THE SAME BASE";
			msg ^= "|CURRENCY TO ALLOW INTERCOMPANY POSTINGS.";
			return invalid(msg);
		}

	} else if ((mode == "POSTINIT") or (mode == "POSTREAD")) {

		//option to read prior versions
		call generalsubs2(mode);
		if (not(win.valid)) {
			return 0;
		}

		//this is done to allow LISTEN to avoid special COMPANY ACCESS PARTIAL logic
		var pass = 0;
		if (((mode == "POSTREAD") and not(interactive)) and win.orec) {
			//check allowed to access this ledger
			if (authorised("#COMPANY ACCESS " ^ (ID.field("~", 1).quote()), msg, "")) {
				pass = 1;
			}
		}

		//OP='COMPANY FILE'
		//GOSUB SECURITY2
		if (not pass) {
			gosub security(mode);
		}
		if (not(win.valid)) {
			return 0;
		}

		//remove obsolete period 13 from deloitte data
		if (RECORD.index("13X4WEEK,1/7,5")) {
			tt = RECORD.a(16);
			tt.swapper("13/", "12/");
			RECORD.r(16, tt);
		}

		//convert old letterhead to new
		if (((mode == "POSTREAD") and not(interactive)) and win.orec) {
			if (not(trim(RECORD.field(FM, 61, 6), FM))) {
				var headhtm;
				if (not(headhtm.read(DEFINITIONS, ID ^ "_HEAD.HTM"))) {
					if (headhtm.read(DEFINITIONS, "HEAD.HTM")) {
						headhtm.converter(FM ^ VM, "");
						//have to remove line marks since they become <br>
						//swap @record<1> with '%COMPANY_NAME%' in headhtm
						//swap @record<21> with '%TAX_REG_NO%' in headhtm
						//swap @record<59> with '%COMPANY_REG_NO%' in headhtm
						RECORD.r(63, headhtm);
					}
				}
			}
		}

	} else if (mode == "PREDELETE") {

		op = "COMPANY";
		gosub security2(mode, op);
		if (not(win.valid)) {
			return 0;
		}

		if (not(interactive) or USERNAME ne "NEOSYS") {
			msg = "Only NEOSYS Technical Support Team can|delete companies in maintenance mode";
			return invalid(msg);
		}

		var options = "";
		options.r(-1, "Delete transactions AND reference files");
		options.r(-1, "Delete transactions only and leave reference files");
		options.r(-1, "Dont delete any data except the " ^ ID ^ " company file record");
		options.r(-1, "No (Cancel)");
		var canceloptionn = options.count(FM) + 1;
		if (not(decide("!Are you SURE that you want|to delete this company???", options, reply, canceloptionn))) {
			reply = 0;
		}
		if ((reply == 0) or (reply == canceloptionn)) {
			win.valid = 0;
			return 0;
		}
		deleteall = reply == 1;

		if (reply ne 3) {

			if (not(decide("", "Generate script only|Archive records but dont actually delete|ARCHIVE AND DELETE records", reply))) {
				win.valid = 0;
				return 0;
			}
			deletemode = reply;
			var testing = deletemode ne 3;

			//zzz could delete media/production/finance only perhaps

			gosub deletecompanydata();

			if (testing or not(deleteall)) {
				win.valid = 0;
			}

		}

		//update version log
		call generalsubs2(mode);

	} else if (mode == "PREWRITE") {

		if (not RECORD) {
			return 0;
		}

		//ensure unrealised a/c tracks realized a/c - there is no user interface
		//if orec<4>=orec<5> then
		if (RECORD.a(5) == "") {
			RECORD.r(5, RECORD.a(4));
		}

		//double check some validations
		if (not interactive) {

			//check period
			win.is = RECORD.a(2);
			win.isorig = win.orec.a(2);
			call companysubs("VAL.PERIOD");
			if (not(win.valid)) {
				return 0;
			}

			//check closing period
			win.is = RECORD.a(16);
			win.isorig = win.orec.a(16);
			call companysubs("VAL.PERIOD2");
			if (not(win.valid)) {
				return 0;
			}

			//check the base currency
			win.is = RECORD.a(3);
			win.isorig = win.orec.a(3);
			call companysubs("VAL.BASECURR");
			if (not(win.valid)) {
				return 0;
			}

			//check the various account numbers except tax
			var fns = "4,5,8,12";
			for (ii = 1; ii <= fns.count(",") + 1; ++ii) {
				var fn = fns.field(",", ii);
				win.is = RECORD.a(fn);
				win.isorig = win.orec.a(fn);
				call companysubs("VAL.ACC");
				if (not(win.valid)) {
					return 0;
				}
			};//ii;

			//tax account
			win.is = RECORD.a(19);
			win.isorig = win.orec.a(19);
			call companysubs("VAL.TAX");
			if (not(win.valid)) {
				return 0;
			}

		}

		//update version log (unless same user changing only letterhead on the same day)
		tt = win.orec;
		for (var fn = 61; fn <= 69; ++fn) {
			tt.r(fn, RECORD.a(fn));
		};//fn;
		tt = trim(tt, FM, "B");
		var logreq = tt ne RECORD;
		if (not logreq) {
			var nlogs = RECORD.a(40).count(VM) + 1;
			if (RECORD.a(40, nlogs) ne USERNAME or RECORD.a(41, nlogs).field(".", 1) ne var().date()) {
				logreq = 1;
			}
		}
		if (logreq) {
			call generalsubs2(mode);
		}

	} else if (mode == "POSTWRITE") {
		//mark the control accounts
		//external
		oldtaxacc = win.orec.a(19);
		newtaxacc = RECORD.a(19);
		gosub marktaxacc(oldtaxacc, newtaxacc);

		if (ID ne fin.currcompany) {
			if (interactive) {
				msg = "YOU HAVE JUST CHANGED TO COMPANY|" ^ RECORD.a(1);
				call note(msg);
				//CURR.COMPANY=@ID
			}
		}
		call initcompany(ID);

		//change all other companies to the same period
		if ((interactive and RECORD.a(2) ne win.orec.a(2)) and win.orec) {
			var compcodes;
			if (compcodes.read(gen.companies, "%RECORDS%")) {
				if (compcodes.count(FM)) {
					if (not(decide("Do you want to update the current period|of other companies that had the same period ?", "", reply))) {
						reply = 2;
					}
					if (reply == 1) {
						select(gen.companies);
nextcomp:
						if (readnext(compcode)) {
							var tcompany;
							if (tcompany.read(gen.companies, compcode)) {
								if (tcompany.a(2) == win.orec.a(2)) {
									tcompany.r(2, RECORD.a(2));
									tcompany.write(gen.companies, compcode);
								}
							}
							goto nextcomp;
						}
					}
				}
			}
		}

	} else if (mode == "POSTDELETE") {
		//unmark control accounts
		//external
		oldtaxacc = win.orec.a(19);
		newtaxacc = "";
		gosub marktaxacc(oldtaxacc, newtaxacc);

	// case mode='REORDER.COMPANIES'
	//  gosub reorder.companies

	} else if (mode == "DELETECOMPANYDATA") {

		gosub deletecompanydata();

	} else {
		msg = mode.quote() ^ " invalid mode in COMPANY.SUBS";
		return invalid(msg);

	}

	return 0;
}

subroutine marktaxacc(in oldtaxacc, in newtaxacc) {

	if (newtaxacc == oldtaxacc) {
		return;
	}

	deleting = 1;
	var taxacc = "." ^ oldtaxacc.a(1, 1, 1);
	gosub marktaxacc2(taxacc, deleting);
	taxacc = oldtaxacc.a(1, 1, 2);
	gosub marktaxacc2(taxacc, deleting);

	deleting = 0;
	taxacc = "." ^ newtaxacc.a(1, 1, 1);
	gosub marktaxacc2(taxacc, deleting);
	taxacc = newtaxacc.a(1, 1, 2);
	gosub marktaxacc2(taxacc, deleting);

	return;
}

subroutine marktaxacc2(in taxacc, in deleting) {

	//mark the new control account if necessary
	if (taxacc) {
		var taxaccrec;
		if (taxaccrec.read(fin.accounts, taxacc)) {
			if (taxaccrec.a(13).locate(ID,vn)) {
				if (deleting) {
					taxaccrec.remove(13, vn, 0).write(fin.accounts, taxacc);
				}
			}else{
				if (not deleting) {
					taxaccrec.insert(13, vn, 0, ID).write(fin.accounts, taxacc);
				}
			}
		}
	}

	return;
}

subroutine getcompanies() {
	//get company code(s)
	if (not(companycodes.read(gen.companies, "%RECORDS%"))) {
		companycodes = "";
		select(gen.companies);
getnextcomp:
		if (readnext(temp)) {
			companycodes.r(-1, temp);
			goto getnextcomp;
		}
	}
	ncompanies = companycodes.count(FM) + 1;
	return;
}

subroutine anybalances() {
	anybalancesfound = getreccount(fin.balances, "", "");
	if (not anybalancesfound) {
		select(fin.balances);
		if (readnext(temp)) {
			anybalancesfound = 1;
		}
		clearselect();
	}
	return;
}

subroutine deletecompanydata() {

	var filesx = "";

	filesx.r(-1, "ANALYSIS");
	filesx.r(-1, "ANALYSIS2");

	filesx.r(-1, "INVOICE_TEXTS");
	filesx.r(-1, "INVOICES");

	filesx.r(-1, "PRODUCTION_INVOICES");
	filesx.r(-1, "PRODUCTION_ORDERS");

	if (FILES(0).locateusing(FM,"DICT.TIMESHEETS",xx)) {
		filesx.r(-1, "TIMESHEETS");
	}

		//why select booking orders when deleting booking texts?
	filesx.r(-1, "BOOKING_TEXTS|BOOKING_ORDERS");
	filesx.r(-1, "BOOKING_ORDERS");
	filesx.r(-1, "ADS");
	filesx.r(-1, "MATERIALS");

	filesx.r(-1, "SCHEDULES");
	filesx.r(-1, "PLANS");

	filesx.r(-1, "PROGRAMS");

	if (deleteall) {
		filesx.r(-1, "RATECARDS");
		filesx.r(-1, "VEHICLES");
		filesx.r(-1, "SUPPLIERS");
	}

	filesx.r(-1, "JOBS");

	if (deleteall) {
		filesx.r(-1, "BRANDS");
		filesx.r(-1, "CLIENTS");
	}

	filesx.r(-1, "JOB_TYPES");

	filesx.r(-1, "BALANCES");
	filesx.r(-1, "VOUCHER_INDEX");
	filesx.r(-1, "VOUCHERS");
	filesx.r(-1, "BATCHES");

	if (deleteall) {
		filesx.r(-1, "RECURRING");
		filesx.r(-1, "ACCOUNTS");
		filesx.r(-1, "LEDGERS");
		filesx.r(-1, "CHARTS");
		filesx.r(-1, "MARKETS");
		filesx.r(-1, "CURRENCIES");
	}

	filesx.r(-1, "COLLECTIONS");

		//files<-1>='COMPANIES'

	var nfiles = filesx.count(FM) + 1;
	var params = "";
	var filen2 = 0;
	var delete2filenames = "";
	for (var filen = 1; filen <= nfiles; ++filen) {
		var filename = filesx.a(filen).field("*", 1);
		filename.converter("_", ".");
		var dictfilename = "";
		filename = filename.field("|", 1);
		var selectfilename = filename.field("|", 2);
		if (not selectfilename) {
			selectfilename = filename;
		}
nextfile:
		if (FILES(0).locateusing(FM,filename,xx)) {
			var fieldname = filesx.a(filen).field("*", 2);
			if (not fieldname) {
				fieldname = "COMPANY_CODE";
			}
			filen2 += 1;
			params.r(3, filen2, filename);
			delete2filenames ^= "," ^ filename;
			var selectcmd = "SELECT " ^ selectfilename;
			if (dictfilename) {
				selectcmd ^= " USING " ^ dictfilename;
			}
			selectcmd ^= " WITH " ^ fieldname ^ " EQ " ^ quote2(ID);
			params.r(4, filen2, selectcmd);

				//also do versions if available
			if (filename.substr(-9,9) ne "_VERSIONS") {
				if (not dictfilename) {
					dictfilename = filename;
				}
				filename = singular(filename) ^ "_VERSIONS";
				goto nextfile;
			}

		}
	};//filen;

	var syss;
	if (not(syss.open("SYSTEMS", ""))) {
		call fsmsg();
		win.valid = 0;
		return;
	}

	var paramkey = "NNN" ^ ID;
	params.write(syss, paramkey);

		//dont run if generating script only
	if (deletemode > 1) {

	/*;
				location=drive()[1,2];

				//archiving
				options='A';
				//test option skips the deleting
				if testing then options='AT';

				perform 'INSTALL ':paramkey:' ':location:' (':options:')';
	*/

		var cmd = "DELETE2 " ^ delete2filenames.substr(2,999999) ^ " COMPANY_CODE " ^ ID ^ " (U)";
		perform(cmd);

	}

	return;
}

libraryexit()
