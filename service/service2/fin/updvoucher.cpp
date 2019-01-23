#include <exodus/library.h>
libraryinit()

#include <sysmsg.h>
#include <initcompany.h>
#include <updvoucher.h>
#include <addcent.h>
#include <updalloc.h>
#include <updvoucher2.h>
#include <nextkey.h>
#include <split.h>
#include <giveway.h>
#include <updindex.h>
#include <updbalances.h>
#include <authorised.h>

#include <fin.h>
#include <gen.h>

var ncompanies;//num
var othercompanycode;
var mindigits;
var prefix;
var vtypen;
var vtypeserialno;//num
var lastno;//num
var internalvouchertype;
var balanceperiod;//num
var cursor;
var baseamount;//num
var amount;//num
var currencycode;
var currencytag;
var acc;
var taxcode;
var account2flag;//num
var analrec;
var tt;//num
var accname;
var msg;
var noncurrency;//num
var netbaseamount;//num
var taxcurr;
var controlamount;
var vn;//num
var otherintercompanyacc;
var intercompanyacc;
var curr;
var svn;
var rate;//num
var base;//num
var reqcurr;
var othercompany;
var fmt;
var gainlossacno;
var status;

function main(in mode, io voucher, io vouchercode, io allocs) {
	//c fin in,io,io,io
	//garbagecollect;
	//y2k *NB if add multivalue fields don't forget to delete them at XXX

	//MODE can be:
	//blank post or repost the voucher after deleting existing
	//DELETE remove the voucher from indexes/balances and mark 'D'
	//DELETE FOR AMEND same as above
	//REALLOC the existing voucher may or may not need to be reposted
	//DELETE FOR AMEND REALLOC any intercompany vouchers are not unposted/deleted
	var origmode = mode;
	var updfollowing = "";

	//extract voucher type
	var vouchertype = vouchercode.field("*", 1);

	var locks;
	if (not(locks.open("LOCKS", ""))) {
		call sysmsg("CANNOT OPEN LOCKS IN UPD.VOUCHER");
		locks = "";
	}

	//detect if inter company voucher from MODE
	//inter company vouchers are stripped of other company lines
	//N.B. any new fields must be added into the source code for deleting lines
	//current multivalued fields are:
	// 3,5,8,10,11,17,19,20,23,24,30,34 in VOUCHER
	// and 1,2,3 in ALLOCS
	var intercompany = mode.index("INTERCOMPANY");
	if (intercompany) {
		mode.splicer(intercompany, 12, "");
	}

	//change to voucher company if different and save CURR.COMPANY
	var origcompany = fin.currcompany;
	var vouchercompany = vouchercode.field("*", 3);
	if (fin.currcompany) {
		if (vouchercompany) {
			if (not(vouchercompany == fin.currcompany)) {
				//CURR.COMPANY=VOUCHER.COMPANY
				call initcompany(vouchercompany);
			}
		}else{
			//if company not specified in VOUCHER.CODE then use CURR.COMPANY
			vouchercompany = fin.currcompany;
			vouchercode = vouchercode.fieldstore("*", 3, 1, fin.currcompany);
		}
	}

	//if voucher not provided then get from file
	if (not voucher) {
		if (not(voucher.read(fin.vouchers, vouchercode))) {
			voucher = "";
		}
	}

	//voucher period/year (from voucher date if not specified)
	//garbagecollect;
	var voucherdate = voucher.a(2).oconv("HEX");
	var yearperiod = voucher.a(16);
	if (not yearperiod) {
		yearperiod = voucherdate.oconv(gen.company.a(6));
	}
	var period = yearperiod.substr(-2,2);
	var year = yearperiod.substr(1,2);

	//delete indexes and balances and mark as deleted
	//(ZZZ cancelled vouchers should be left in the indexes
	// and skipped by all reporting programs)
	////////////////////////////////////////
	if (mode.substr(1,6) == "DELETE") {

		//if voucher not provided then get from file
		//IF VOUCHER ELSE READ VOUCHER FROM VOUCHERS,VOUCHER.CODE ELSE VOUCHER=''
		if (not voucher) {
			if (not(voucher.read(fin.vouchers, vouchercode))) {
				goto exit;
			}
		}
		var doctype = voucher.a(29);

		//this should probably be after deleting any intercompany vouchers
		//but since unposted voucher are not split into intercompany vouchers ATM
		//then it doesnt matter ATM
		if (voucher.a(7, 1) == "D") {
			//call upd.voucher2('DELETE',voucher,voucher.code,allocs)
			goto exit;
		}

		voucher.r(7, 1, "D");

		//delete any intercompany vouchers created by this voucher
		//(not if we are reposting because of allocation)
		if (mode ne "DELETE FOR AMEND REALLOC") {
			var othercompanies = voucher.a(9);
			if (othercompanies) {
				ncompanies = othercompanies.count(VM) + 1;
				for (var companyn = 1; companyn <= ncompanies; ++companyn) {
					othercompanycode = othercompanies.a(1, companyn);
					var vouchercode2 = vouchertype ^ "*" ^ fin.currcompany ^ ":" ^ vouchercode.field("*", 2) ^ "*" ^ othercompanycode;
					//allow for old system XX999 voucher number construction
					var xx;
					if (not(xx.read(fin.vouchers, vouchercode2))) {
						vouchercode2 = vouchertype ^ "*" ^ fin.currcompany ^ vouchercode.field("*", 2) ^ "*" ^ othercompanycode;
					}
					call updvoucher(mode, "", vouchercode2, "");
					//actually delete the voucher from the file
					//otherwise change of inter company and reposting can leave
					//vouchers in the file (marked "D")
					fin.vouchers.deleterecord(vouchercode2);
					
				};//companyn;
			}
		}

		//delete the voucher
		var deleting = 1;
		gosub process();
		goto writevoucher;
		//DELETE VOUCHERS,VOUCHER.CODE
		//GOTO EXIT
	}

	//if voucher number blank then get next one automatically
	///////////////////////////////////////////////////////
	var voucherno = vouchercode.field("*", 2);
	if (voucherno == "") {
nextvoucherno:
		var seqkey = "<TYPE>*<COMPANY>";

		//eg YYCVNNNN
		//default is YYNNNN
		//to get old put just N
		var pattern = fin.accparams.a(17);
		if ((addcent(year) >= 2011) and pattern) {

			//TODO ensure that PP is prefixed by YY ALWAYS in format
			if (pattern.index("Y")) {
				seqkey ^= "*<YEAR>";
			}
			if (pattern.index("P")) {
				seqkey ^= "*<PERIOD>";
			}
			mindigits = pattern.count("N");
			prefix = pattern;

			var companyserialno = gen.company.a(46);
			if (companyserialno == "") {
				companyserialno = 0;
			}

			if (fin.definition.a(6).locateusing(vouchertype, VM, vtypen)) {
				vtypeserialno = fin.definition.a(11, vtypen);
				if (vtypeserialno == "") {
					vtypeserialno = 0;
				}
			}else{
				vtypeserialno = 0;
			}

			prefix.swapper("N", "");
			prefix.swapper("YY", year);
			prefix.swapper("CC", companyserialno.oconv("R(0)#2"));
			prefix.swapper("PP", period.oconv("R(0)#2"));
			prefix.swapper("C", companyserialno);
			prefix.swapper("VV", vtypeserialno.oconv("R(0)#2"));
			prefix.swapper("V", vtypeserialno);

		}else{
			mindigits = "";
			prefix = "";
		}

		seqkey.swapper("<TYPE>", vouchercode.field("*", 1));
		seqkey.swapper("<COMPANY>", fin.currcompany);
		seqkey.swapper("<YEAR>", year);
		seqkey.swapper("<PERIOD>", period);
		seqkey = "%" ^ seqkey ^ "%";
		if (not(lockrecord("", fin.vouchers, seqkey))) {
			call mssg("WAITING TO GET NEXT VOUCHER NUMBER", "T", "", "");
			goto nextvoucherno;
		}
		if (not(lastno.readv(fin.vouchers, seqkey, 1))) {
			lastno = 1;
		}
		(lastno + 1).write(fin.vouchers, seqkey);
		var xx = unlockrecord("", fin.vouchers, seqkey);

		if (mindigits and (lastno.length() < mindigits)) {
			lastno = lastno.oconv("R(0)#" ^ mindigits);
		}

		var temp = prefix ^ lastno;

		vouchercode = vouchercode.fieldstore("*", 2, 1, temp);
	}
	var oldvoucher;
	if (oldvoucher.read(fin.vouchers, vouchercode)) {
		if (not voucherno) {
			goto nextvoucherno;
		}
	}else{
		oldvoucher = "";
	}
	voucherno = vouchercode.field("*", 2);
	var doctype = voucher.a(29);

	//check each line for other companies
	//remove ,CURR.COMPANY from account number if found
	//if intercompany then delete other companies
	//otherwise build up mv COMPANY.CODES and NCOMPANIES
	gosub other.companies();

	//if one company (or others stripped) then perform any allocations
	/////////////////////////////////////////////////////////////////
	if (ncompanies == 1) {
		if (allocs) {
			call updalloc(mode, voucher, vouchercode, allocs);
			//if mode no longer REALLOC then the voucher needs reposting
			if (mode == "REALLOC") {
				goto writevoucher;
			}
		}
	}

	//if reposting then unpost the old voucher
	//before multi company postings
	/////////////////////////////////////////
	if (oldvoucher) {
		//nb old.voucher MUST be preserved for saving in voucher.versions
		if (oldvoucher.a(7, 1) == "D") {
			voucher.r(7, 1, "R");
			//remove any previous indexes
			call updvoucher2("DELETE", oldvoucher ^ "", vouchercode, "");
		}else{
			voucher.r(7, 1, "A");
			//remove any previous indexes and balances
			var temp = "DELETE FOR AMEND";
			if (origmode == "REALLOC") {
				temp ^= " REALLOC";
			}
			call updvoucher(temp, oldvoucher ^ "", vouchercode, "");
		}
	}

	//multi currency and multi company balancing
	///////////////////////////////////////////
	//accumulate amounts by currency by company (in compcurrtots)
	//and add base amount if missing (look up the exchange rate)
	gosub company.and.currency.totals();

	//multi company vouchers
	///////////////////////
	if (ncompanies > 1) {
		gosub multicompany();
		goto exit;
	}

	//add lines for exchange gains and losses
	////////////////////////////////////////
	gosub add.gainloss();

	//post to index and balance files
	////////////////////////////////
	var deleting = 0;
	gosub process();

	//////////////
writevoucher:
	//////////////

	//dont add log unless deleting for amending or allocating
	//could addlog if REALLOC if stop adding log in voucherx.subs/general.subs2
	if (not mode) {
		gosub addlog();
	}

	//cant call upd.voucher3 here since we need to update indexes
	//so if reposting any vouchers then we have to ensure
	//no reposted vouchers are locked for editing anywhere
	//therefore simply DELETE any record in lock file!
	if (locks) {
		locks.deleterecord("VOUCHERS*" ^ vouchercode);
		
	}
	voucher.write(fin.vouchers, vouchercode);

	/////
exit:
	/////
	if (fin.currcompany ne origcompany) {
		//CURR.COMPANY=ORIG.COMPANY
		call initcompany(origcompany);
	}
	return 0;

	//////////////////////////////////////////////////////////////////////////////

}

subroutine process() {

	//check if open new year run on another workstation
	var temp;
	if (temp.read(gen.companies, fin.currcompany)) {
		if (temp.a(2) ne gen.company.a(2)) {
			call initcompany(fin.currcompany);
		}
	}

	//moved up to allow year to be part of automatic voucher no
	//voucher period/year (from voucher date if not specified)
	//garbagecollect;
	//voucherdate=oconv(voucher<2>,'HEX')
	//YEARPERIOD=VOUCHER<16>
	//IF YEARPERIOD ELSE
	// garbagecollect
	// YEARPERIOD=oconv(voucherdate,COMPANY<6>)
	// end
	//PERIOD=YEARPERIOD[-2,2]
	//YEAR= YEARPERIOD[1,2]

	//analyse by the calendar year and month because the analysis is that way
	//in upd.voucher and upd.analysis
	//tt=voucherdate 'D2/E'
	//analyear=tt[-2,2]
	//analmth=field(tt,'/',2)+0
	var analyear = addcent(year);
	var analmth = period;
	var firstmth = gen.company.a(45).field(",", 1);
	if (firstmth) {
		analmth += firstmth - 1;
	}
	if (analmth > 12) {
		analmth -= 12;
		analyear += 1;
	}
	if (analmth < 1) {
		analmth += 12;
		analyear -= 1;
	}
	analyear = analyear.substr(-2,2);

	var analysis;
	if (not(analysis.open("ANALYSIS", ""))) {
		analysis = "";
	}

	//add to last batch/make new batch
	if (not deleting and not voucher.a(12)) {

		//look at last batch
		var nextkeyparam = ":%" ^ fin.currcompany ^ "*" ^ vouchertype ^ "%:BATCHES:" ^ fin.currcompany ^ "*" ^ vouchertype ^ "*%";
		var batchid = nextkey(nextkeyparam, "");
		batchid = batchid.fieldstore("*", 3, 1, batchid.field("*", 3) - 1);
		var newbatch = 0;
		if (not(lockrecord("BATCHES", fin.batches, batchid))) {
			goto newbatch;
		}
		var batch;
		if (batch.read(fin.batches, batchid)) {

			//consider adding to an autobatch (list of voucher numbers)

			//only add to the last batch if it is an autobatch (no dates)
			if (batch.a(2)) {
				goto newbatch;
			}

			//only add to the last batch if batch and voucher periods match
			if (batch.a(23) ne (period + 0 ^ "/" ^ year)) {
				goto newbatch;
			}

			//only add to the last batch if it was created today
			//no longer supporting company<25> configuration 'PERIOD'
			//if company<25>#'PERIOD' and field(batch<26,1>,'.',1) ne date() then goto newbatch
			if (batch.a(26, 1).field(".", 1) ne var().date()) {
				goto newbatch;
			}

			//make a new batch
		}else{
newbatch:
			var xx = unlockrecord("BATCHES", fin.batches, batchid);
			newbatch = 1;
			batchid = nextkey(nextkeyparam, "");
			if (not(lockrecord("BATCHES", fin.batches, batchid))) {
				goto newbatch;
			}
			batch = "";
			batch.r(23, period + 0 ^ "/" ^ year);
			//batch<26>=voucherdate

			//create or insert version history
			//dont increment versions when adding to existing generate batches
			//to avoid wasting space in versions file for no purpose
			//since amended batches are no longer considered generated
			var version = batch.a(41);
			if (not version) {
				version = 1;
			}
			batch.r(41, version);
			batch.inserter(24, 1, "");
			batch.inserter(25, 1, "GENERATED");
			batch.inserter(26, 1, var().date() ^ "." ^ var().time().oconv("R(0)#5"));
			batch.inserter(27, 1, STATION.trim());
			batch.inserter(28, 1, version);
			batch.inserter(29, 1, "POSTED");

		}

		//update the batch
		voucherno = vouchercode.field("*", 2);
		if (not(batch.a(1).locateusing(voucherno, VM, temp))) {
			batch.r(1, -1, voucherno);
		}
		batch.write(fin.batches, batchid);
		if (newbatch) {
			temp = nextkey(nextkeyparam, "");
		}
		var xx = unlockrecord("BATCHES", fin.batches, batchid);

		voucher.r(12, batchid.field("*", 3));
	}

	//voucher index entry starts with date*voucher type
	var indexentrypart1 = (var().chr(0) ^ var().chr(0) ^ var().chr(0) ^ voucher.a(2)).substr(-fin.hexdatesize,fin.hexdatesize);
	indexentrypart1 ^= vouchertype ^ STM;

	//convert voucher number to special format
	//- if digits only then they are converted char(239+nchars) plus hex
	var convvoucherno = vouchercode.field("*", 2);
	if (convvoucherno[1] ne "0") {
		temp = convvoucherno;
		temp.converter("0123456789", "");
		if (not temp) {
			//garbagecollect;
			temp = convvoucherno.iconv("HEX");
			convvoucherno = var().chr(239 + temp.length()) ^ temp;
		}
	}

	//voucher index entry continues with voucher number in special format
	indexentrypart1 ^= convvoucherno ^ STM;

	//opening balance type voucher ?
	if (fin.definition.a(6).locateusing(vouchertype, VM, temp)) {
		internalvouchertype = fin.definition.a(1, temp);
	}else{
		internalvouchertype = vouchertype;
	}
	if ((vouchertype.substr(1,2) == "OB") or (internalvouchertype.substr(1,2) == "OB")) {
		balanceperiod = 0;
	}else{
		balanceperiod = period + 0;
	}

lines:
	//////
	var accs = voucher.a(8);
	var nlines = accs.count(VM) + (accs ne "");

	printl();
	var().getcursor();

	//postings
	/////////

	for (var ln = 1; ln <= nlines; ++ln) {

		//get amount/base/currency
		var curramount = voucher.a(10, ln);
		baseamount = voucher.a(11, ln) + 0;
		if (curramount) {
			amount = split(curramount, currencycode);
		}else{
			amount = 0;
			currencycode = fin.basecurrency;
		}
		if (currencycode ne fin.basecurrency) {
			currencytag = STM ^ currencycode;
		}else{
			//BASE.AMOUNT=CURR.AMOUNT
			currencytag = "";
		}

		//post to the account (use the internal account if present)
		acc = accs.a(1, ln);
		taxcode = voucher.a(24, ln);

		if (not((acc or curramount) or baseamount)) {
			goto postnextln;
		}
		account2flag = 0;
		gosub postacc();

		var analysiscode = voucher.a(37, ln);
		if (analysis and analysiscode) {

			//eg 28/29 bill/cost xx/yy wip/acc
			var analfn = analysiscode.field("*", 1);

			//eg 1/2/3/4/5/6/7 gross/load/disc/comm/fee/tax/other/net/avr
			//currently always 1 or 9 in manual entries
			//might have been 1/2 for media/jobs in automated entries by mistake
			var analcoln = analysiscode.field("*", 2);

			var analkey = analyear ^ "*" ^ analysiscode.field("*", 3, 5);
			analkey = analkey.fieldstore("*", 8, 1, fin.currcompany);

			//single record lock/unlock will avoid deadly embrace lock hanging
			//without trying to do a complete lock of all updatable records in prewrite
			//(which would be difficult due to lack of easy knowledge of company code
			//in multicompany vouchers maybe - and also could be excessive numb locks)
			//NB all other programs updating analysis should not hold any locks
			//longer than 99 seconds - otherwise update will be skipped
			//and you will have to CREATEANALYSIS (for vouchers only)
			//allow duplicate lock to avoid some strange double locking issue in po update
			if (lockrecord("ANALYSIS", analysis, analkey, analrec, 99, 1)) {

				if (not(analrec.read(analysis, analkey))) {
					analrec = "";
				}

				//billings are credits to income and are negative (unless credit note)
				if (analfn == 28) {
					tt = -baseamount;
				}else{
					tt = baseamount;
				}
				//and deletion (or delete for amendment) should subtract
				if (deleting) {
					tt = -tt;
				}
				//garbagecollect;
				analrec.r(analfn, analmth, analcoln, (analrec.a(analfn, analmth, analcoln) + tt).oconv(fin.basefmt));

				analrec.write(analysis, analkey);

				if (not(unlockrecord("ANALYSIS", analysis, analkey))) {
					{}
				}

			}else{
				call sysmsg(DQ ^ (analkey ^ DQ) ^ " UPD.VOUCHER CANNOT LOCK - ANALYSIS UPDATE SKIPPED");
			}
		}

		//remove account name from the description (whole/starting/ending)
		//or rather substitute with the ~ account name place holder
		temp = voucher.a(3, ln);
		if (temp) {
			if (temp == accname) {
				voucher.r(3, ln, "");
			} else if (temp.substr(-accname.length(),accname.length()) == accname) {
				temp.splicer(-accname.length(), accname.length(), "~");
				voucher.r(3, ln, temp);
			} else if (temp.substr(1,accname.length()) == accname) {
				temp.splicer(1, accname.length(), "~");
				voucher.r(3, ln, temp);
			}
		}

		//post to the job account
		//ACC=VOUCHER<5,LN>
		//IF ACC AND (CURR.AMOUNT OR BASE.AMOUNT) THEN
		// if noncurrency then
		// amount=base.amount
		// currency.code=base.currency
		// currency.tag=''
		// end
		// ACCOUNT2.FLAG=1
		// TAX.CODE=''
		// GOSUB POSTACC
		// END

		//post to WIP system (FRINICS)
		//jobinfo=field(voucher<3,ln>,' ',1)
		//convert @lower.case to @upper.case in jobinfo
		//if jobinfo[1,4]='JOB=' then
		// call upd.voucher.wip(jobinfo,deleting,voucher.code,ln,voucherdate,base.amount)
		// end

postnextln:
	};//ln;

	return;

}

subroutine postacc() {

	if (not(giveway(""))) {
		{}
	}

	//skip any lines not of this company
	var acc2 = acc.a(1, 1, 1);
	var acccompany = acc2.field(",", 2);
	if (acccompany) {
		if (acccompany ne fin.currcompany) {
			return;
		}
		acc2.r(1, 1, 1, acc2.field(",", 1));
		if (not acc2) {
			return;
		}
	}

	//convert to internal account
	if (acc.a(1, 1, 2)) {
		acc2 = acc.a(1, 1, 2);
	}

	//update voucher with internal account, no company
	if (acc2 ne acc) {
		acc = acc2;
		if (account2flag) {
			voucher.r(5, ln, acc);
		}else{
			voucher.r(8, ln, acc);
		}
	}

	//get account details
	if (not(fin.account.read(fin.accounts, acc))) {
		if (fin.account.read(fin.accounts, "." ^ acc)) {
			acc = fin.account.a(10);
			//VOUCHER<5?8,LN>=ACC
		}else{
			var().chr(7).output();
			msg = "SYSTEM ERROR IN UPD.VOUCHER - GET TECHNICAL ASSISTANCE IMMEDIATELY|| POSTING VOUCHER " ^ vouchercode;
			msg ^= "||THE ACCOUNT " ^ (DQ ^ (acc ^ DQ)) ^ " IS MISSING";
			if (account2flag) {
				msg ^= "|*** POSTED TO " ^ ("ZZZ999" ^ SVM ^ "ZZZ999").a(1, 1, 1) ^ " ACCOUNT ***";
				call mssg(msg);
				acc = "ZZZ999" ^ SVM ^ "ZZZ999";
				fin.account = "";
			}else{
				msg ^= "|*** POSTED TO EXCHANGE GAIN/LOSSES ACCOUNT ***";
				call mssg(msg);
				acc = gen.company.a(4);
				if (not(fin.account.read(fin.accounts, acc))) {
					var().chr(7).output();
					call mssg("NO EXCHANGE GAINS/LOSSES ACCOUNT|*** POSTED TO " ^ ("ZZZ999" ^ SVM ^ "ZZZ999").a(1, 1, 1) ^ " ACCOUNT ***");
					acc = "ZZZ999" ^ SVM ^ "ZZZ999";
					fin.account = "";
				}
			}
		}
	}

	accname = fin.account.a(1);
	var acctype = fin.account.a(2);
	var summdet = fin.account.a(3);
	var controlacc = fin.account.a(6);
	var balancetype = fin.account.a(8);
	var summaryacc = fin.account.a(13);
	if (summaryacc and fin.accparams.a(1)) {
		if (addcent(yearperiod) >= addcent(fin.accparams.a(1))) {
			summaryacc = 0;
		}
	}

	//blank lines of stock vouchers do not get posted at all
	if (voucher.a(4) == "S") {
		if (not(amount or baseamount)) {
			return;
		}
	}

	//switch to company of the account (job accounts only)
	var lineorigcompany = fin.currcompany;
	if (account2flag) {
		var linecompany = fin.account.a(5);
		if (linecompany) {
			if (linecompany ne fin.currcompany) {
				//CURR.COMPANY=LINE.COMPANY
				call initcompany(linecompany);
			}
		}
	}

	//show details on screen
	//PRINT @(1,20):@(-4):' ':
	cursor.setcursor();
	if (mode == "DELETE") {
		print("DELETING ");
	} else if (mode.substr(1,16) == "DELETE FOR AMEND") {
		print("AMENDING ");
	}
	temp = vouchercode;
	temp.converter("*", "-");
	print(temp, "   ", ln, ". ", accname.substr(1,40), " ");

	//!!! also in loadreplication and upd.voucher2

	//update voucher index(es)
	if (not summaryacc) {
		var indexentry = indexentrypart1 ^ ln ^ currencytag;

		//point to original company if necessary
		if (fin.currcompany ne lineorigcompany) {
			if (not currencytag) {
				indexentry ^= STM;
			}
			indexentry ^= STM ^ lineorigcompany;
		}

		var indexkey = yearperiod ^ "*" ^ acc ^ "*" ^ fin.currcompany;
		//1) balance forward index if necessary
		if ((balancetype ne "O" and internalvouchertype.substr(1,2) ne "OB") and vouchertype.substr(1,2) ne "OB") {
	// IF BALANCE.TYPE NE 'O' THEN
			call updindex(fin.voucherindex, indexkey, indexentry, deleting);
		}
		//2) open item index if necessary
		if (balancetype ne "B") {
			call updindex(fin.voucherindex, "*" ^ indexkey.field("*", 2, 99), indexentry, deleting);
			//flag line as open item so easier to update documents payable elsewhere
			if ((voucher.a(20, ln) == "") and not account2flag) {
				voucher.r(20, ln, SVM);
			}
		}
	}

	//check if units or currency
	if (not account2flag) {
		if (gen.currency.read(gen.currencies, currencycode)) {
			noncurrency = 0;
		}else{
			noncurrency = 1;
		}
	}

	//update balances
	//1) account
	var balanceskey = year ^ "*" ^ acc ^ "*" ^ fin.currcompany ^ "*" ^ currencycode;
	if (doctype) {
		balanceskey = balanceskey.fieldstore("*", 5, 1, doctype);
	}
	call updbalances(fin.balances, balanceskey, deleting, balanceperiod, amount, baseamount, "", fin.basecurrency, vouchertype);

	//2) VAT analysis
	if (taxcode and not doctype) {
		//vat amount
		var taxbalanceskey = year ^ "*" ^ taxcode ^ "*" ^ fin.currcompany ^ "*" ^ currencycode ^ "*TAX";
		var taxamount = amount;
		var taxbaseamount = baseamount;
		call updbalances(fin.balances, taxbalanceskey, deleting, balanceperiod, taxamount, taxbaseamount, "", fin.basecurrency, vouchertype);

		//net amount in fn 70, 71 otherwise assumed to be previous line of voucher
		//also in CHK.POST
		taxbalanceskey = taxbalanceskey.fieldstore("*", 5, 1, "NET");
		var netamount = voucher.a(70, ln);
		if (netamount.length()) {
			netbaseamount = voucher.a(71, ln);
		}else{
			netamount = voucher.a(10, ln - 1);
			netbaseamount = voucher.a(11, ln - 1);
		}
		netamount = split(netamount, taxcurr);
		netbaseamount += 0;
		taxbalanceskey = taxbalanceskey.fieldstore("*", 4, 1, taxcurr);
		call updbalances(fin.balances, taxbalanceskey, deleting, balanceperiod, netamount, netbaseamount, "", fin.basecurrency, vouchertype);
	}

	//3) control
	if (controlacc and not doctype) {
		var controlbalanceskey = year ^ "*" ^ controlacc ^ "*" ^ fin.currcompany ^ "*";
		if (noncurrency) {
			controlbalanceskey ^= fin.basecurrency;
			controlamount = baseamount;
		}else{
			controlbalanceskey ^= currencycode;
			controlamount = amount;
		}
		call updbalances(fin.balances, controlbalanceskey, deleting, balanceperiod, controlamount, baseamount, "", fin.basecurrency, vouchertype);
	}

	var prevnoncurrency = noncurrency;
	//update balances of following year(s) if voucher year less than current year
	//check maximum year
	var maxyear = gen.company.a(2).substr(-2,2);
	//990508 IF addcent(YEAR) LT addcent(CURRYEAR) THEN
	if (addcent(year) < addcent(maxyear)) {

		//for opening balances in prior years the user can choose to update
		// following years or not.
		if (updfollowing == "") {
			if ((internalvouchertype.substr(1,2) == "OB") or (vouchertype.substr(1,2) == "OB")) {
updfollowing:
				var reply = 2;
				msg = "DO YOU WANT TO UPDATE THE OPENING|";
				msg ^= "BALANCES OF THE FOLLOWING YEAR(S)";
				if (not(decide(msg, "", reply))) {
					var().chr(7).output();
					call mssg("YOU MUST ANSWER \"YES\" OR \"NO\"");
					goto updfollowing;
				}
				updfollowing = reply == 1;
			}else{
				updfollowing = 1;
			}
		}

		if (updfollowing) {
			var nextyear = (year + 1).oconv("R(0)#2");

			//get forward account and it's control account (if any)
			var transferacc = fin.account.a(12);
			//if no transfer a/c then check control a/c if any
			if (transferacc == "") {
				if (controlacc) {
					if (not(transferacc.readv(fin.accounts, controlacc, 12))) {
						{}
					}
				}
			}
			if (transferacc) {
				var transferaccount;
				if (not(transferaccount.read(fin.accounts, transferacc))) {
					//allow transfer a/c to be different from internal account
					if (transferaccount.read(fin.accounts, "." ^ transferacc)) {
						transferacc = transferaccount.a(10);
					}else{
						//cant find transfer account, therefore don't carry forward
						goto postaccexit;

								}else{
								}else{
					transferacc = acc;
					transferaccount = fin.account;
				}
				var transfercontrolacc = transferaccount.a(6);

				//loop through the years
				//990508 LOOP WHILE addcent(NEXTYEAR) LE addcent(curryear) DO
				while (true) {
				///BREAK;
				if (not(addcent(nextyear) <= addcent(maxyear))) break;;
					//1) update balances
					var nextyearbalanceskey = nextyear ^ "*" ^ transferacc ^ "*" ^ fin.currcompany ^ "*" ^ currencycode;
					if (doctype) {
						nextyearbalanceskey = nextyearbalanceskey.fieldstore("*", 5, 1, doctype);
					}
					call updbalances(fin.balances, nextyearbalanceskey, deleting, 0, amount, baseamount, "", fin.basecurrency, vouchertype);
					//2) update control
					if (transfercontrolacc and not doctype) {
						var controlbalanceskey = nextyear ^ "*" ^ transfercontrolacc ^ "*" ^ fin.currcompany ^ "*";
						if (noncurrency) {
							controlbalanceskey ^= fin.basecurrency;
							controlamount = baseamount;
						}else{
							controlbalanceskey ^= currencycode;
							controlamount = amount;
						}
						call updbalances(fin.balances, controlbalanceskey, deleting, 0, controlamount, baseamount, "", fin.basecurrency, vouchertype);
					}
					nextyear = (nextyear + 1).oconv("R(0)#2");
				}//loop;

			}
		}

postaccexit:
		//restore orig company if necessary
		if (fin.currcompany ne lineorigcompany) {
			//CURR.COMPANY=LINE.ORIG.COMPANY
			call initcompany(lineorigcompany);
		}

		print(" ok");
		return;

	////////////////
othercompanies:
	////////////////
		accs = voucher.a(8);
		var naccs = accs.count(VM) + (accs ne "");

		var companycodes = fin.currcompany;
		ncompanies = 1;

		var maindesc = voucher.a(3);
		for (var ln = 1; ln <= naccs; ++ln) {

			acc = accs.a(1, ln, 1);
			acccompany = acc.field(",", 2);
			if (acccompany) {

				//current company
				if (acccompany == fin.currcompany) {
					//remove it from the account number
					if (intercompany) {
						voucher.r(8, ln, 1, acc.field(",", 1));
					}

					//other company
				}else{

					//build a list of company codes starting with curr.company
					if (not intercompany) {
						//if not INTERCOMPANY then add into mv COMPANY.CODES
						if (not(companycodes.a(1).locateusing(acccompany, VM, vn))) {
							companycodes.r(1, vn, acccompany);
							ncompanies += 1;
						}

						//if INTERCOMPANY then remove the line from VOUCHER and ALLOCS
					}else{
						accs.eraser(1, ln);
						var nn = "3" _VM_ "5" _VM_ "8" _VM_ "10" _VM_ "11" _VM_ "17" _VM_ "19" _VM_ "20" _VM_ "23" _VM_ "24" _VM_ "26" _VM_ "27" _VM_ "30" _VM_ "34" _VM_ "37" _VM_ "70" _VM_ "71".count(VM) + 1;
						for (var ii = 1; ii <= nn; ++ii) {
							//3 5 8 10 11 17 19 20 23 24 26 27 30 34?
							voucher.eraser("3" _VM_ "5" _VM_ "8" _VM_ "10" _VM_ "11" _VM_ "17" _VM_ "19" _VM_ "20" _VM_ "23" _VM_ "24" _VM_ "26" _VM_ "27" _VM_ "30" _VM_ "34" _VM_ "37" _VM_ "70" _VM_ "71".a(1, ii), ln);
						};//ii;
						if (allocs) {
							allocs.eraser(1, ln);
							allocs.eraser(2, ln);
							allocs.eraser(3, ln);
						}
						naccs -= 1;
						ln -= 1;
					}

				}
			}
		};//ln;

		//fix description on intercompany vouchers
		//TODO looks like this will not work in all cases
		if (intercompany) {
			if (maindesc.a(1, 1)) {
				voucher.r(3, 1, maindesc.a(1, 1));
			}else{
				if (maindesc.a(1, 2)) {
					voucher.r(3, 2, maindesc.a(1, 2));
				}
			}
		}

		return;

	////////////////////////////
companyandcurrencytotals:
	////////////////////////////
		//compcurrtots
		//field N is company N
		//multivalues 10 and 11 are currency and base amount respectively
		//subvalue N is currency N

		//CURRS (NCURRS)
		//subvalue N is currency N

		//RATES
		//subvalue N is exchange rate N

		var currs = "";
		var ncurrs = 0;
		var rates = "";
		var compcurrtots = "";

		//allocation may have added some accounts so get again
		accs = voucher.a(8);
		naccs = accs.count(VM) + (accs ne "");

		for (var ln = 1; ln <= naccs; ++ln) {
			acc = accs.a(1, ln, 1);
			var companyn = 1;
			othercompanycode = acc.field(",", 2);
			if (othercompanycode) {

				//if a company code is specified, and not posting ic voucher
				//build the list of company codes

	//? IF INTERCOMPANY AND OTHER.COMPANY.CODE NE CURR.COMPANY ELSE
	//it is a bit weird adding curr.company code to company.codes at this point
	//but left as is in case company.codes isnt initialised to curr.company
	//eg what happens if ALL postings are to other companies in one voucher?
				if (not intercompany or (othercompanycode == fin.currcompany)) {

					if (not(companycodes.a(1).locateusing(othercompanycode, VM, companyn))) {
						companycodes.r(1, companyn, othercompanycode);
						ncompanies += 1;
					}

					//NEW FOR TAX/VAT ENTRIES TO GET NET REPORTED CORRECTLY
					// TODO could/should this done for ALL I/C postings?
					//
					//IF posting to another company and there is a posting to
					// CURRENT company tax/vat on the FOLLOWING line
					// then INSERT A DUPLICATE of the CURRENT line but to intercompany a/c
					//this ensures that VAT postings are always preceded by a posting
					// of the net amount - since the the current line is goint to be deleted
					//also insert a reversal of the inserted line but into the other company
					//
					//NB any NET/NET_BASE existing in field 70/71 will be used by preference
					//in UPD.VOUCHER tax/net, CHK.VOUCHERS, LISTTAX etc
					//field 70/71 currently is only filled for bugs where PRIOR LINE NET missing
					//
					//the pair of inserted lines will reduce the amounts in the compcurrtots
					//maybe to zero so probably there are no additional I/C lines added
					//in the multicompany routine

					if (othercompanycode ne fin.currcompany) {

						var nextln = ln + 1;
						var taxcodex = voucher.a(24, nextln);
						if (taxcodex.length()) {

							//taxcompcode should always be the same as the net line before it
							//since there is no way for user to enter the taxaccno
							var taxcompcode = accs.a(1, nextln, 1).field(",", 2);
							if ((taxcompcode == "") or (taxcompcode == fin.currcompany)) {

								//insert a pair of voucher lines after the current line and before the tax line
								//to act as the NET AMOUNT line for the tax line
								for (var pairn = 1; pairn <= 2; ++pairn) {
									accs.inserter(1, nextln, "");
									var nn = "3" _VM_ "5" _VM_ "8" _VM_ "10" _VM_ "11" _VM_ "17" _VM_ "19" _VM_ "20" _VM_ "23" _VM_ "24" _VM_ "26" _VM_ "27" _VM_ "30" _VM_ "34" _VM_ "37" _VM_ "70" _VM_ "71".count(VM) + 1;
									for (var ii = 1; ii <= nn; ++ii) {
										//3 5 8 10 11 17 19 20 23 24 26 27 30 34?
										voucher.inserter("3" _VM_ "5" _VM_ "8" _VM_ "10" _VM_ "11" _VM_ "17" _VM_ "19" _VM_ "20" _VM_ "23" _VM_ "24" _VM_ "26" _VM_ "27" _VM_ "30" _VM_ "34" _VM_ "37" _VM_ "70" _VM_ "71".a(1, ii), nextln, "");
									};//ii;
									if (allocs) {
										allocs.inserter(1, nextln, "");
										allocs.inserter(2, nextln, "");
										allocs.inserter(3, nextln, "");
									}
									naccs += 1;
								};//pairn;

								//copy the current line into the FIRST inserted voucher line
								// except post to the other intercompany account and REVERSED
								gosub getotherintercompanyacc();
								voucher.r(8, nextln, otherintercompanyacc ^ "," ^ othercompanycode);
								accs.r(1, nextln, voucher.a(8, nextln));
								tt = "-" ^ voucher.a(10, ln);
								if (tt.substr(1,2) == "--") {
									tt.splicer(1, 2, "");
								}
								voucher.r(10, nextln, tt);
								tt = "-" ^ voucher.a(11, ln);
								if (tt.substr(1,2) == "--") {
									tt.splicer(1, 2, "");
								}
								voucher.r(11, nextln, tt);

								//copy the current line into the SECOND inserted voucher line
								// except post to the intercompany account
								//this line will provide the NET for tax reporting
								nextln += 1;
								gosub getintercompanyacc();
								voucher.r(8, nextln, intercompanyacc ^ "," ^ fin.currcompany);
								accs.r(1, nextln, voucher.a(8, nextln));
								voucher.r(10, nextln, voucher.a(10, ln));
								voucher.r(11, nextln, voucher.a(11, ln));

							}
						}
					}

				}

				//add this company code to every account that doesnt have one
			}else{
				if (ncompanies > 1) {
					voucher.r(8, ln, 1, acc ^ "," ^ fin.currcompany);
				}
			}

			amount = split(voucher.a(10, ln), curr);
			baseamount = voucher.a(11, ln);

			//build a list of currencies found
			if (not(currs.a(1, 1).locateusing(curr, SVM, svn))) {
				currs.r(1, 1, svn, curr);
				ncurrs += 1;
			}

			//calculate base amount if not specified
			//(SJB 23/8/93 ALLOW 0 BASE) IF BASE.AMOUNT ELSE
			if (baseamount == "") {
				if (curr == fin.basecurrency) {
					baseamount = amount;
				}else{
					rate = rates.a(1, 1, svn);
					if (not rate) {
						if (not(gen.currency.read(gen.currencies, curr))) {
							gen.currency = "";
						}
						//garbagecollect;
						var voucherdate2 = voucher.a(2).oconv("HEX");
						if (not(gen.currency.a(4).locateby(voucherdate2, "DR", vn))) {
							{}
						}
						rate = gen.currency.a(5, vn);
						if (not rate) {
							rate = gen.currency.a(5, vn - 1);
						}
						rates.r(1, 1, svn, rate);
					}
					if (rate) {
						//garbagecollect;
						baseamount = (amount / rate).oconv(fin.basefmt);
					}else{
						baseamount = "";
					}
				}
				voucher.r(11, ln, baseamount);
			}

			//save the amount and base amount per company and currency
			//garbagecollect;
			compcurrtots.r(companyn, 10, svn, 0 + ((compcurrtots.a(companyn, 10, svn) + amount).oconv("MD40P")));
			compcurrtots.r(companyn, 11, svn, (compcurrtots.a(companyn, 11, svn) + baseamount).oconv(fin.basefmt));

		};//ln;

		return;

}

subroutine multicompany() {

		//post separate vouchers for each company if necessary
		//
		// VOUCHER VOUCHER IN A VOUCHER IN B VOUCHER IN C
		// ------- ------------ ------------ ------------
		// COMPANY A 100 100 84 16
		// COMPANY B -80 -84 -80
		// COMPANY C -15 -16 -15
		// GAINLOSS 0 -4 -1
		//
		//If voucher does'nt balance then gains and losses are distributed
		// to the other companies 100 x 80/95 = 84.
		//Note that B in A (-84) is the same as A in B,
		// and C in A (-16) is the same as A in C,
		// in other words the inter company accounts match

		var intercompanyaccs = "";
		var otherintercompanyaccs = "";
		var currbatchno = voucher.a(12);

		//get the total amount posted to OTHER companies for each currency
		//get second company total
		var tototheramounts = compcurrtots.a(2, 10);
		var tototherbases = compcurrtots.a(2, 11);
		//add other companies if more than 2
		for (var companyn = 3; companyn <= ncompanies; ++companyn) {
			for (var currn = 1; currn <= ncurrs; ++currn) {
				curr = currs.a(1, 1, currn);
				//garbagecollect;
				tototheramounts.r(1, 1, currn, (tototheramounts.a(1, 1, currn) + compcurrtots.a(companyn, 10, currn)).oconv("MD40P"));
				tototherbases.r(1, 1, currn, (tototherbases.a(1, 1, currn) + compcurrtots.a(companyn, 11, currn)).oconv(fin.basefmt));
			};//currn;
		};//companyn;

		//for each other company for each currency two identical postings are made:
		//1) in this companies account in the other companies books
		//2) in the other companies account in this companies books
		for (var companyn = 2; companyn <= ncompanies; ++companyn) {
			othercompanycode = companycodes.a(1, companyn);
			for (var currn = 1; currn <= ncurrs; ++currn) {
				curr = currs.a(1, 1, currn);

				if ((ncompanies <= 2) or (ncurrs > 1)) {
					amount = -(compcurrtots.a(companyn, 10, currn));
					base = -(compcurrtots.a(companyn, 11, currn));

					//if there is more than one other company
					//then any exchange gains and losses are proportioned
				}else{

					amount = compcurrtots.a(1, 10, currn);
					var tototheramount = tototheramounts.a(1, 1, currn);
					var otheramount = compcurrtots.a(companyn, 10, currn);

					base = compcurrtots.a(1, 11, currn);
					var tototherbase = tototherbases.a(1, 1, currn);
					var otherbase = compcurrtots.a(companyn, 11, currn);

					//if not(amount) and not(tot.other.amount) and not(base) and not(tot.other.base) then
					if (not amount and not base) {

						//cater for debit company 2 credit company 3 and nothing in current company
						//other wise get no voucher in current company and dr/cr exchange in the other vouchers
						if (otheramount or otherbase) {
							amount = "-" ^ otheramount;
							if (amount.substr(1,2) == "--") {
								amount.splicer(1, 2, "");
							}
							base = "-" ^ otherbase;
							if (base.substr(1,2) == "--") {
								base.splicer(1, 2, "");
							}
						}

					}else{

						if (not amount) {
						//nothing in current company so no intercompany
						} else if (not tototheramount) {
							//nothing in other companies so proportion the amount equally
							amount = amount / (ncompanies - 1);
						} else if (not otheramount) {
							amount = "";
						} else {
							//proportion the amount according to the other company amounts
							if (otheramount - tototheramount) {
								amount = amount * otheramount / tototheramount;
							}
						}
//L6467:
						if (not base) {
						//nothing in current company so use - amount in other company
						//BASE=-OTHER.BASE
						} else if (not tototherbase) {
							//nothing in other companies so proportion the amount equally
							//garbagecollect;
							base = (base / (ncompanies - 1)).oconv(fin.basefmt);
						} else if (not otherbase) {
							base = "";
						} else {
							//proportion the amount according to the other company amounts
							if (otherbase - tototherbase) {
								//garbagecollect;
								base = (base * otherbase / tototherbase).oconv(fin.basefmt);
							}
						}
					}

				}

				if (amount or base) {

					if (amount) {
						if (temp.read(gen.currencies, curr)) {
							var ndecs = temp.a(3);
							goto 6609;
						}
						var ndecs = 2;
						amount = amount.oconv("MD" ^ ndecs ^ "0P");
					}

					gosub getintercompanyacc();

					gosub getotherintercompanyacc();

					//if other company is mainly a different currency then convert
					//if a rate can be found
					var othercompanyamount = amount;
					var othercompanycurr = curr;
					reqcurr = othercompany.a(15);
					if (reqcurr and curr ne reqcurr) {
						gosub getrate();
						if (rate) {
							//garbagecollect;
							othercompanyamount = (base * rate).oconv(fmt);
							othercompanycurr = reqcurr;
						}
					}

					//if this company uses mainly another currency then convert
					reqcurr = gen.company.a(15);
					if (reqcurr and curr ne reqcurr) {
						gosub getrate();
						if (rate) {
							//garbagecollect;
							amount = (base * rate).oconv(fmt);
							curr = reqcurr;
						}
					}

					//add two lines to the voucher

					//posting in other company to its account for this company
					naccs += 1;
					voucher.r(8, naccs, otherintercompanyacc ^ "," ^ othercompanycode);
					voucher.r(10, naccs, othercompanyamount ^ othercompanycurr);
					voucher.r(11, naccs, base);

					//posting in this company to its account for the other company
					//will be posted when we post the main voucher
					//Maybe dont do this if we post the original other company lines
					//directly to the intercompany account instead of deleting them
					naccs += 1;
					voucher.r(8, naccs, intercompanyacc ^ "," ^ fin.currcompany);
					voucher.r(10, naccs, -amount ^ curr);
					//garbagecollect;
					voucher.r(11, naccs, (-base).oconv(fin.basefmt));

				}

			};//currn;

			/////////////////////////////////////////////////////////
			//post the combination voucher to the other company
			/////////////////////////////////////////////////////////

			//force addition to/creation of a batch in the other company
			voucher.r(12, "");

			//create a unique voucher no for the other company posting - in the format C:NNNNNN
			//based on the current company code C and voucher no NNNNNN
			var vouchercode2 = vouchertype ^ "*" ^ origcompany ^ ":" ^ voucherno ^ "*" ^ othercompanycode;

			//!!! PREVENT THE SUBROUTINE CHANGING PARAMETERS - PASS VALUES NOT VARIABLES (use :'')

			call updvoucher(mode ^ "INTERCOMPANY", voucher ^ "", vouchercode2, allocs ^ "");

		};//companyn;

		//save the combination voucher for de-bugging (VCHRNO=CURR.COMPANY:VOUCHER.NO)
		//WRITE VOUCHER ON VOUCHERS,FIELDSTORE(VOUCHER.CODE,'*',2,1,CURR.COMPANY:VOUCHER.NO)

		////////////////////////////////////////
		//post the voucher to the current company
		////////////////////////////////////////

		//save a list of other company codes posted
		voucher.r(9, companycodes.field(VM, 2, 9999));

		//restore our batch no
		voucher.r(12, currbatchno);

		call updvoucher(mode ^ "INTERCOMPANY", voucher, vouchercode, allocs);

		return;

}

subroutine getintercompanyacc() {
		//get this company's intercompany account for the other company
		if (gen.company.a(7).locateusing(othercompanycode, VM, temp)) {
			intercompanyacc = gen.company.a(8, temp, 2);
		}else{
			msg = "SYSTEM ERROR IN UPD.VOUCHER|THE INTER COMPANY ACCOUNT FOR COMPANY " ^ (DQ ^ (othercompanycode ^ DQ));
			msg ^= "|IN THE accounts OF " ^ gen.company.a(1) ^ " IS MISSING.||THE " ^ ("ZZZ999" ^ SVM ^ "ZZZ999").a(1, 1, 1) ^ " ACCOUNT WILL BE USED";
			call mssg(msg);
			//INTER.COMPANY.ACC=COMPANY<4>
			intercompanyacc = "ZZZ999" ^ SVM ^ "ZZZ999";
		}
		return;

}

subroutine getotherintercompanyacc() {
		//get the other company's intercompany account for this company
		if (not(othercompany.read(gen.companies, othercompanycode))) {
			call mssg(DQ ^ (othercompanycode ^ DQ) ^ " OTHER COMPANY IS MISSING");
			othercompany = "";
		}
		if (othercompany.a(7).locateusing(fin.currcompany, VM, temp)) {
			otherintercompanyacc = othercompany.a(8, temp, 2);
		}else{
			otherintercompanyacc = "";
		}
		if (not otherintercompanyacc) {
			msg = "SYSTEM ERROR IN UPD.VOUCHER|THE INTER COMPANY ACCOUNT FOR COMPANY " ^ (DQ ^ (fin.currcompany ^ DQ));
			msg ^= "|IN THE accounts OF " ^ (DQ ^ (othercompany.a(1) ^ DQ)) ^ " IS MISSING.||THE " ^ ("ZZZ999" ^ SVM ^ "ZZZ999").a(1, 1, 1) ^ " ACCOUNT WILL BE USED";
			call mssg(msg);
			//OTHER.INTER.COMPANY.ACC=OTHER.COMPANY<4>
			otherintercompanyacc = "ZZZ999" ^ SVM ^ "ZZZ999";
		}
		return;

	/////////////
addgainloss:
	/////////////
		//for each currency that does not balance (currencywise) then
		//add posting to exchange gains/losses
		//oswrite compcurrtots on 'xy'

		companyn = 1;

		//garbagecollect;
		var gainlossamt = -(compcurrtots.a(companyn, 11).sum().oconv("MD40P"));

		temp = compcurrtots.a(companyn, 10);
		temp.converter(SVM ^ "0.", "");
		if (temp) {
revalit:

			//exchange gain a/c
			if (gainlossamt < 0) {
				gainlossacno = gen.company.a(4);

				//exchange loss a/c
				goto 7428;
			}
			if (gainlossamt > 0) {
				gainlossacno = gen.company.a(5);

				//intercurrency conversion a/c
			}else{
				gainlossacno = gen.company.a(12);
			}
//L7428:

			if (not gainlossacno) {
				gainlossacno = "ZZZ999" ^ SVM ^ "ZZZ999";
			}

			if (intercompany) {
				gainlossacno ^= "," ^ fin.currcompany;
			}

			for (var currn = 1; currn <= ncurrs; ++currn) {
				amount = compcurrtots.a(companyn, 10, currn);
				base = compcurrtots.a(companyn, 11, currn);
				if (amount or base) {
					naccs += 1;
					voucher.r(8, naccs, gainlossacno);
					voucher.r(10, naccs, -amount ^ currs.a(1, 1, currn));
					voucher.r(11, naccs, -base);
				}
			};//currn;

			//no currency amounts so must be a base imbalance
		}else{

			//if currency balances but base does not then
			//add postings to exchange gains/losses
			//unless there is only one currency and the difference
			//is no more than 2 cents
			//in which case adjust base on the first line
			temp = compcurrtots.a(companyn, 11);
			temp.converter(SVM ^ "0.", "");
			if (temp) {
				//IF NCURRS GT 1 THEN GOTO revalit
				//TEMP=SUM(compcurrtots<COMPANYN,11>)
				//IF ABS(TEMP) GT 0.02 THEN GOTO revalit

				if ((ncurrs > 1) or (gainlossamt.abs() > .02)) {
					goto revalit;
				}

				//garbagecollect;
				//VOUCHER<11,1>=((VOUCHER<11,1>-TEMP) 'MD40P')+0
				voucher.r(11, 1, ((voucher.a(11, 1) + gainlossamt).oconv("MD40P")) + 0);
			}

		}

		return;

}

subroutine getrate() {
		//get exchange rate and format given required currency
		if (reqcurr == fin.basecurrency) {
			rate = 1;
			fmt = fin.basefmt;
		}else{
			if (not(gen.currency.read(gen.currencies, reqcurr))) {
				gen.currency = "";
			}
			if (not(gen.currency.a(4).locateby(voucherdate, "DR", vn))) {
				{}
			}
			rate = gen.currency.a(5, vn);
			if (not rate) {
				rate = gen.currency.a(5, vn - 1);
			}
			var ndecs = gen.currency.a(3);
			if (not ndecs) {
				ndecs = 2;
			}
			fmt = "MD" ^ ndecs ^ "0P";
		}

		return;

}

subroutine addlog() {
		var versionfn = 59;
		var statusfn = 7;

		var logfn = 60;
		var userlogfn = 60;
		var datetimelogfn = 61;
		var stationlogfn = 62;
		var versionlogfn = 63;
		var statuslogfn = 64;

		//first 16 fields include all main details excluding allocations
		tt = oldvoucher.field(FM, 1, 16);
		tt.r(9, voucher.a(9));
		var changed = tt ne voucher.field(FM, 1, 16);

		var oldversion = voucher.a(versionfn);
		var version = oldversion + changed;

		if (version == 1) {
			status = "POSTED";
		}else{
			//NEOSYS and special user can repost without record
			if (authorised("#JOURNAL REPOST WITHOUT RECORD", msg, "NEOSYS")) {

				return;

			}
			status = "REPOSTED";
		}

		voucher.r(versionfn, version);

		//logn=r<logfn+0>
		//logn=count(logn,vm)+(logn<>'')+1
		//to be consistent with journal versions in reverse order
		var logn = 1;
		//ologn=logn-1

	//copied in general.subs2 and productionproxy and upd.voucher

		voucher.inserter(userlogfn, logn, USERNAME);

		var datetime = var().date() ^ "." ^ var().time().oconv("R(0)#5");
		voucher.inserter(datetimelogfn, logn, datetime);

		voucher.inserter(stationlogfn, logn, STATION.trim());

		if (versionfn) {
			voucher.inserter(versionlogfn, logn, version);
		}

		if (statusfn) {
			voucher.inserter(statuslogfn, logn, status);
		}

		//save old version in VOUCHER_VERSIONS
		if ((version > 1) and version ne oldversion) {
			var voucherversions;
			if (voucherversions.open("VOUCHER_VERSIONS", "")) {
				tt = vouchercode ^ "~" ^ oldversion;
				oldvoucher.write(voucherversions, tt);
			}
		}

		return;

}


libraryexit()
