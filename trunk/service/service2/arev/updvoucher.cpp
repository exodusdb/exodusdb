#include <exodus/library.h>
libraryinit()

#include <initcompany.h>
#include <addcent.h>
#include <updalloc.h>
#include <updvoucher2.h>
#include <nextkey.h>
#include <split.h>
#include <sysmsg.h>
#include <giveway.h>
#include <updindex.h>
#include <updbalances.h>

#include <fin.h>
#include <gen.h>

var ncompanies;//num
var mindigits;
var prefix;
var vtypen;
var vtypeserialno;//num
var internalvouchertype;
var balanceperiod;//num
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
var noncurrency;//num
var prevnoncurrency;
var netamount;
var taxcurr;
var controlamount;
var transferaccount;
var vn;//num
var curr;
var svn;
var rate;//num
var base;//num
var intercompanyacc;
var otherintercompanyacc;
var reqcurr;
var fmt;
var gainlossacno;

function main() {

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

	//detect if inter company voucher from MODE
	//inter company vouchers are stripped of other company lines
	//N.B. any new fields must be added into the source code for deleting lines
	//current multivalued fields are:
	// 3,5,8,10,11,17,19,20,23,24,30,34 in VOUCHER
	// and 1,2,3 in ALLOCS
	var intercompany = mode.index("INTERCOMPANY", 1);
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
	var voucherdate = (voucher.a(2)).oconv("HEX");
	var yearperiod = voucher.a(16);
	if (not yearperiod) {
		yearperiod = voucherdate.oconv(gen.company.a(6));
	}
	var period = yearperiod.substr(-2, 2);
	var year = yearperiod.substr(1, 2);

	//delete indexes and balances and mark as deleted
	//(ZZZ cancelled vouchers should be left in the indexes
	// and skipped by all reporting programs)
	////////////////////////////////////////
	if (mode.substr(1, 6) == "DELETE") {

		//if voucher not provided then get from file
		if (not voucher) {
			if (not(voucher.read(fin.vouchers, vouchercode))) {
				voucher = "";
			}
		}
		var doctype = voucher.a(29);

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
					var othercompanycode = othercompanies.a(1, companyn);
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
		if (addcent(year) >= 2011 and pattern) {

			if (pattern.index("Y", 1)) {
				seqkey ^= "*<YEAR>";
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
		seqkey = "%" ^ seqkey ^ "%";
		if (not(lockrecord("", fin.vouchers, seqkey))) {
			call mssg("WAITING TO GET NEXT VOUCHER NUMBER", "T", "", "");
			goto nextvoucherno;
		}
		var lastno;
		if (not(lastno.readv(fin.vouchers, seqkey, 1))) {
			lastno = 1;
		}
		lastno + 1.write(fin.vouchers, seqkey);
		var xx = unlockrecord("", fin.vouchers, seqkey);

		if (mindigits and lastno.length() < mindigits) {
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
	gosub othercompanies();

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
		if (oldvoucher.a(7, 1) == "D") {
			voucher.r(7, 1, "R");
			//remove any previous indexes
			call updvoucher2("DELETE", oldvoucher, vouchercode, "");
		}else{
			voucher.r(7, 1, "A");
			//remove any previous indexes and balances
			var temp = "DELETE FOR AMEND";
			if (origmode == "REALLOC") {
				temp ^= " REALLOC";
			}
			call updvoucher(temp, oldvoucher, vouchercode, "");
		}
	}

	//multi company/multi currency balancing (not for stock vouchers)
	////////////////////////////////////////////////////////////////
	//accumulate amounts by currency by company (in MVOUCHER)
	//and add base amount if missing (look up the exchange rate)
	gosub companyandcurrencytotals();

	if (ncompanies > 1) {
		gosub multicompany();
		goto exit;
	}

	//add lines for exchange gains and losses
	//except for stock and non accounts type vouchers eg orders
	if (voucher.a(4) ne "S" and doctype == "") {
		gosub addgainloss();
	}

	//post to index and balance files
	////////////////////////////////
	var deleting = 0;
	gosub process();

	//////////////
writevoucher:
	//////////////
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
	analyear = analyear.substr(-2, 2);

	var analysis;
	if (not(analysis.open("ANALYSIS", ""))) {
		analysis = "";
	}

	//add to last batch/make new batch
	if (not voucher.a(12)) {

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
			//add to last batch if same day/period and LIST type batch
			if (batch.a(2)) {
				goto newbatch;
			}
			if (batch.a(23) ne (period + 0 ^ "/" ^ year)) {
				goto newbatch;
			}
			if (gen.company.a(25) ne "PERIOD" and batch.a(26) ne voucherdate) {
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
			batch.r(26, voucherdate);
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
	var indexentrypart1 = var("   " ^ voucher.a(2)).substr(-fin.hexdatesize, fin.hexdatesize);
	indexentrypart1 ^= vouchertype ^ STM;

	//convert voucher number to special format
	//- if digits only then they are converted char(239+nchars) plus hex
	var convvoucherno = vouchercode.field("*", 2);
	if (convvoucherno.substr(1, 1) ne "0") {
		temp = convvoucherno;
		temp.converter("0123456789", "");
		if (not temp) {
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
	if (vouchertype.substr(1, 2) == "OB" or internalvouchertype.substr(1, 2) == "OB") {
		balanceperiod = 0;
	}else{
		balanceperiod = period + 0;
	}

lines:
	//////
	var accs = voucher.a(8);
	var nlines = accs.count(VM) + (accs ne "");

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

		if (not(acc or curramount or baseamount)) {
			goto postnextln;
		}
		account2flag = 0;
		gosub postacc();

		var analysiscode = voucher.a(37, ln);
		if (analysis and analysiscode) {

			var analfn = analysiscode.field("*", 1);
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
				goto 2524;
			}
			if (temp.substr(-accname.length(), accname.length()) == accname) {
				temp.splicer(-accname.length(), accname.length(), "~");
				voucher.r(3, ln, temp);
				goto 2524;
			}
			if (temp.substr(1, accname.length()) == accname) {
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

	if (not giveway("")) {
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
	if (not(fin.account.reado(fin.accounts, acc))) {
		if (fin.account.reado(fin.accounts, "." ^ acc)) {
			acc = fin.account.a(10);
			//VOUCHER<5?8,LN>=ACC
		}else{
			var().chr(7).output();
			var msg = "SYSTEM ERROR IN UPD.VOUCHER - GET TECHNICAL ASSISTANCE IMMEDIATELY|| POSTING VOUCHER " ^ vouchercode;
			msg ^= "||THE ACCOUNT " ^ (DQ ^ (acc ^ DQ)) ^ " IS MISSING";
			if (account2flag) {
				msg ^= "|*** POSTED TO ZZZ999 ACCOUNT ***";
				call mssg(msg);
				acc = "ZZZ999";
				fin.account = "";
			}else{
				msg ^= "|*** POSTED TO EXCHANGE GAIN/LOSSES ACCOUNT ***";
				call mssg(msg);
				acc = gen.company.a(4);
				if (not(fin.account.reado(fin.accounts, acc))) {
					var().chr(7).output();
					call mssg("NO EXCHANGE GAINS/LOSSES ACCOUNT|*** POSTED TO ZZZ999 ACCOUNT ***");
					acc = "ZZZ999";
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
	cout << var().cursor(1, 20);
	cout << var().cursor(-4);
	cout << " ";
	if (mode == "DELETE") {
		cout << "DELETING ";
		goto 3138;
	}
	if (mode.substr(1, 16) == "DELETE FOR AMEND") {
		cout << "AMENDING "<< "   "<< ln<< ". "<< accname.substr(1, 40)<< " ";
	}
	temp = vouchercode;
	temp.converter("*", "-");
	cout << temp;

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
		if (balancetype ne "O" and internalvouchertype.substr(1, 2) ne "OB" and vouchertype.substr(1, 2) ne "OB") {
	// IF BALANCE.TYPE NE 'O' THEN
			call updindex(fin.voucherindex, indexkey, indexentry, deleting);
		}
		//2) open item index if necessary
		if (balancetype ne "B") {
			call updindex(fin.voucherindex, "*" ^ indexkey.field("*", 2, 99), indexentry, deleting);
			//flag line as open item so easier to update documents payable elsewhere
			if (voucher.a(20, ln) == "" and not account2flag) {
				voucher.r(20, ln, SVM);
			}
		}
	}

	//check if units or currency
	if (not account2flag) {
		if (gen.currency.reado(gen.currencies, currencycode)) {
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
	call updbalances(fin.balancesfile, balanceskey, deleting, balanceperiod, amount, baseamount, "", fin.basecurrency, vouchertype);

	//2) VAT analysis
	if (taxcode and not doctype) {
		//vat amount
		var taxbalanceskey = year ^ "*" ^ taxcode ^ "*" ^ fin.currcompany ^ "*" ^ currencycode ^ "*TAX";
		var taxamount = amount;
		var taxbaseamount = baseamount;
		call updbalances(fin.balancesfile, taxbalanceskey, deleting, balanceperiod, taxamount, taxbaseamount, "", fin.basecurrency, vouchertype);

		//nett amount assumed to be previous line of voucher
		taxbalanceskey = taxbalanceskey.fieldstore("*", 5, 1, "NET");
		var netbaseamount = voucher.a(11, ln - 1) + 0;
		if (prevnoncurrency) {
			netamount = netbaseamount;
			taxbalanceskey = taxbalanceskey.fieldstore("*", 4, 1, fin.basecurrency);
		}else{
			netamount = split(voucher.a(10, ln - 1), taxcurr);
			taxbalanceskey = taxbalanceskey.fieldstore("*", 4, 1, taxcurr);
		}
		call updbalances(fin.balancesfile, taxbalanceskey, deleting, balanceperiod, netamount, netbaseamount, "", fin.basecurrency, vouchertype);
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
		call updbalances(fin.balancesfile, controlbalanceskey, deleting, balanceperiod, controlamount, baseamount, "", fin.basecurrency, vouchertype);
	}

	prevnoncurrency = noncurrency;
	//update balances of following year(s) if voucher year less than current year
	//check maximum year
	var maxyear = (gen.company.a(2)).substr(-2, 2);
	//990508 IF addcent(YEAR) LT addcent(CURRYEAR) THEN
	if (addcent(year) < addcent(maxyear)) {

		//for opening balances in prior years the user can choose to update
		// following years or not.
		if (updfollowing == "") {
			if (internalvouchertype.substr(1, 2) == "OB" or vouchertype.substr(1, 2) == "OB") {
updfollowing:
				var reply = 2;
				var msg = "DO YOU WANT TO UPDATE THE OPENING|";
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
				if (not(transferaccount.reado(fin.accounts, transferacc))) {
					//allow transfer a/c to be different from internal account
					if (transferaccount.reado(fin.accounts, "." ^ transferacc)) {
						transferacc = transferaccount.a(10);
					}else{
						//cant find transfer account, therefore don't carry forward
						goto postaccexit;
					}
				}
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
				call updbalances(fin.balancesfile, nextyearbalanceskey, deleting, 0, amount, baseamount, "", fin.basecurrency, vouchertype);
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
					call updbalances(fin.balancesfile, controlbalanceskey, deleting, 0, controlamount, baseamount, "", fin.basecurrency, vouchertype);
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

	cout << " ok";
	return;

}

subroutine othercompanies() {
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
				if (not intercompany) {
					//if not INTERCOMPANY then add into mv COMPANY.CODES
					if (not(companycodes.a(1).locateusing(acccompany, VM, vn))) {
						companycodes.r(1, vn, acccompany);
						ncompanies += 1;
					}
				}else{
					//XXX if INTERCOMPANY then remove the line from VOUCHER and ALLOCS
					accs.eraser(1, ln);
					var nn = ("3" _VM_ "5" _VM_ "8" _VM_ "10" _VM_ "11" _VM_ "17" _VM_ "19" _VM_ "20" _VM_ "23" _VM_ "24" _VM_ "30" _VM_ "34" _VM_ "37").count(VM) + 1;
					for (var ii = 1; ii <= nn; ++ii) {
						//VOUCHER=DELETE(VOUCHER,3,LN,0)
						//VOUCHER=DELETE(VOUCHER,5,LN,0)
						//VOUCHER=DELETE(VOUCHER,8,LN,0)
						//VOUCHER=DELETE(VOUCHER,10,LN,0)
						//VOUCHER=DELETE(VOUCHER,11,LN,0)
						//VOUCHER=DELETE(VOUCHER,17,LN,0)
						//VOUCHER=DELETE(VOUCHER,19,LN,0)
						//VOUCHER=DELETE(VOUCHER,20,LN,0)
						//VOUCHER=DELETE(VOUCHER,23,LN,0)
						//VOUCHER=DELETE(VOUCHER,24,LN,0)
						//VOUCHER=DELETE(VOUCHER,30,LN,0)
						//VOUCHER=DELETE(VOUCHER,34,LN,0)
						voucher.eraser(var("3" _VM_ "5" _VM_ "8" _VM_ "10" _VM_ "11" _VM_ "17" _VM_ "19" _VM_ "20" _VM_ "23" _VM_ "24" _VM_ "30" _VM_ "34" _VM_ "37").a(1, ii), ln);
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
	if (intercompany and voucher.a(4) ne "S") {
		if (maindesc.a(1, 1)) {
			voucher.r(3, 1, maindesc.a(1, 1));
		}else{
			if (maindesc.a(1, 2)) {
				voucher.r(3, 2, maindesc.a(1, 2));
			}
		}
	}
	return;

}

subroutine companyandcurrencytotals() {
	//MVOUCHER
	//field N is company N
	//multivalues 10 and 11 are currency and base amount respectively
	//subvalue N is currency N

	//CURRS (NCURRS)
	//subvalue N is currency N

	//RATES
	//subvalue N is exchange rate N

	var currs = "";
	var ncurrs = 0;
	var unitcodes = "";
	var rates = "";
	var mvoucher = "";

	//allocation may have added some accounts so get again
	accs = voucher.a(8);
	naccs = accs.count(VM) + (accs ne "");

	for (var ln = 1; ln <= naccs; ++ln) {
		acc = accs.a(1, ln, 1);
		var companyn = 1;
		var othercompanycode = acc.field(",", 2);
		if (othercompanycode) {
			if (not(intercompany and othercompanycode ne fin.currcompany)) {
				if (not(companycodes.a(1).locateusing(othercompanycode, VM, companyn))) {
					companycodes.r(1, companyn, othercompanycode);
					ncompanies += 1;
				}
			}
		}else{
			if (ncompanies > 1) {
				voucher.r(8, ln, 1, acc ^ "," ^ fin.currcompany);
			}
		}

		amount = split(voucher.a(10, ln), curr);
		baseamount = voucher.a(11, ln);
unitline:
		if (not(currs.a(1, 1).locateusing(curr, SVM, svn))) {

			//for stock vouchers, amount=base amount
			if (voucher.a(4) == "S") {
				if (curr ne fin.basecurrency) {
					if (unitcodes.a(1).locateusing(curr, VM, temp)) {
unitline2:
						amount = baseamount;
						curr = fin.basecurrency;
						goto unitline;
						//goto nextln
					}else{
						if (not(gen.currency.reado(gen.currencies, curr))) {
							unitcodes.r(1, -1, curr);
							//goto nextln
							goto unitline2;
						}
					}
				}
			}

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
					if (not(gen.currency.reado(gen.currencies, curr))) {
						gen.currency = "";
					}
					var voucherdate2 = (voucher.a(2)).oconv("HEX");
					if (not(gen.currency.a(4).locatebyusing(voucherdate2, "DR", vn, VM))) {
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
		//garbagecollect;
		mvoucher.r(companyn, 10, svn, 0 + ((mvoucher.a(companyn, 10, svn) + amount).oconv("MD40P")));
		mvoucher.r(companyn, 11, svn, (mvoucher.a(companyn, 11, svn) + baseamount).oconv(fin.basefmt));
nextln:
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
	var tototheramounts = mvoucher.a(2, 10);
	var tototherbases = mvoucher.a(2, 11);
	//add other companies if more than 2
	for (var companyn = 3; companyn <= ncompanies; ++companyn) {
		for (var currn = 1; currn <= ncurrs; ++currn) {
			curr = currs.a(1, 1, currn);
			//garbagecollect;
			tototheramounts.r(1, 1, currn, (tototheramounts.a(1, 1, currn) + mvoucher.a(companyn, 10, currn)).oconv("MD40P"));
			tototherbases.r(1, 1, currn, (tototherbases.a(1, 1, currn) + mvoucher.a(companyn, 11, currn)).oconv(fin.basefmt));
		};//currn;
	};//companyn;

	//for each other company for each currency two identical postings are made:
	//1) in this companies account in the other companies books
	//2) in the other companies account in this companies books
	for (var companyn = 2; companyn <= ncompanies; ++companyn) {
		var othercompanycode = companycodes.a(1, companyn);
		for (var currn = 1; currn <= ncurrs; ++currn) {
			curr = currs.a(1, 1, currn);

			if (ncompanies <= 2 or ncurrs > 1) {
				amount = -(mvoucher.a(companyn, 10, currn));
				base = -(mvoucher.a(companyn, 11, currn));

				//if there is more than one other company
				//then any exchange gains and losses are proportioned
			}else{

				amount = mvoucher.a(1, 10, currn);
				var tototheramount = tototheramounts.a(1, 1, currn);
				var otheramount = mvoucher.a(companyn, 10, currn);

				base = mvoucher.a(1, 11, currn);
				var tototherbase = tototherbases.a(1, 1, currn);
				var otherbase = mvoucher.a(companyn, 11, currn);

				//if not(amount) and not(tot.other.amount) and not(base) and not(tot.other.base) then
				if (not amount and not base) {

					//cater for debit company 2 credit company 3 and nothing in current company
					//other wise get no voucher in current company and dr/cr exchange in the other vouchers
					if (otheramount or otherbase) {
						amount = "-" ^ otheramount;
						if (amount.substr(1, 2) == "--") {
							amount.splicer(1, 2, "");
						}
						base = "-" ^ otherbase;
						if (base.substr(1, 2) == "--") {
							base.splicer(1, 2, "");
						}
					}

				}else{

					if (not amount) {
						//nothing in current company so no intercompany
						goto 5778;
					}
					if (not tototheramount) {
						//nothing in other companies so proportion the amount equally
						amount = amount / (ncompanies - 1);
						goto 5778;
					}
					if (not otheramount) {
						amount = "";

					if (1) {
						//proportion the amount according to the other company amounts
						if (otheramount - tototheramount) {
							amount = amount * otheramount / tototheramount;
						}
					}
L5778:
					}else if (not base) {
						//nothing in current company so use - amount in other company
						//BASE=-OTHER.BASE
						goto 5873;
					}
					if (not tototherbase) {
						//nothing in other companies so proportion the amount equally
						//garbagecollect;
						base = (base / (ncompanies - 1)).oconv(fin.basefmt);
						goto 5873;
					}
					if (not otherbase) {
						base = "";
						goto 5873;
					}
					if (1) {
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
					if (temp.reado(gen.currencies, curr)) {
						var ndecs = temp.a(3);
						goto 5920;
					}
					var ndecs = 2;
					amount = amount.oconv("MD" ^ ndecs ^ "0P");
				}

				//get other company's inter company account in this company's books
				if (gen.company.a(7).locateusing(othercompanycode, VM, temp)) {
					intercompanyacc = gen.company.a(8, temp, 2);
				}else{
					var().chr(7).output();
					var msg = "SYSTEM ERROR IN UPD.VOUCHER|THE INTER COMPANY ACCOUNT FOR COMPANY " ^ (DQ ^ (othercompanycode ^ DQ));
					msg ^= "|IN THE accounts OF " ^ gen.company.a(1) ^ " IS MISSING.||THE EXCHANGE GAINS/LOSSES ACCOUNT WILL BE USED";
					call mssg(msg);
					intercompanyacc = gen.company.a(4);
				}

				//get this company's inter company account in other company's books
				var othercompany;
				if (not(othercompany.reado(gen.companies, othercompanycode))) {
					call mssg(DQ ^ (othercompanycode ^ DQ) ^ " OTHER COMPANY IS MISSING");
					othercompany = "";
				}
				if (othercompany.a(7).locateusing(fin.currcompany, VM, temp)) {
					otherintercompanyacc = othercompany.a(8, temp, 2);
				}else{
					otherintercompanyacc = "";
				}
				if (not otherintercompanyacc) {
					var().chr(7).output();
					var msg = "SYSTEM ERROR IN UPD.VOUCHER|THE INTER COMPANY ACCOUNT FOR COMPANY " ^ (DQ ^ (fin.currcompany ^ DQ));
					msg ^= "|IN THE accounts OF " ^ (DQ ^ (othercompany.a(1) ^ DQ)) ^ " IS MISSING.||THE EXCHANGE GAINS/LOSSES ACCOUNT WILL BE USED";
					call mssg(msg);
					otherintercompanyacc = othercompany.a(4);
				}

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
				naccs += 1;
				voucher.r(8, naccs, otherintercompanyacc ^ "," ^ othercompanycode);
				voucher.r(10, naccs, othercompanyamount ^ othercompanycurr);
				voucher.r(11, naccs, base);

				naccs += 1;
				voucher.r(8, naccs, intercompanyacc ^ "," ^ fin.currcompany);
				voucher.r(10, naccs, -amount ^ curr);
				//garbagecollect;
				voucher.r(11, naccs, (-base).oconv(fin.basefmt));
			}

		};//currn;

		//post the combination voucher to the other company
		//(prevent the subroutine from changing parameters by :'')
		voucher.r(12, "");
		var vouchercode2 = vouchertype ^ "*" ^ origcompany ^ ":" ^ voucherno ^ "*" ^ othercompanycode;
		call updvoucher(mode ^ "INTERCOMPANY", voucher ^ "", vouchercode2, allocs ^ "");

	};//companyn;

	//save the combination voucher for de-bugging (VCHRNO=CURR.COMPANY:VOUCHER.NO)
	//WRITE VOUCHER ON VOUCHERS,FIELDSTORE(VOUCHER.CODE,'*',2,1,CURR.COMPANY:VOUCHER.NO)

	//post the voucher to the current company
	voucher.r(9, companycodes.field(VM, 2, 9999));
	voucher.r(12, currbatchno);
	call updvoucher(mode ^ "INTERCOMPANY", voucher, vouchercode, allocs);
	return;

}

subroutine addgainloss() {
	//for each currency that does not balance (currencywise) then
	//add posting to exchange gains/losses
	//oswrite mvoucher on 'xy'

	companyn = 1;

	//garbagecollect;
	var gainlossamt = -((mvoucher.a(companyn, 11)).sum()).oconv("MD40P");

	temp = mvoucher.a(companyn, 10);
	temp.converter(SVM ^ "0.", "");
	if (temp) {
revalit:

		if (gainlossamt < 0) {
			//exchange gain a/c
			gainlossacno = gen.company.a(4);
			goto 6659;
		}
		if (gainlossamt > 0) {
			//exchange loss a/c
			gainlossacno = gen.company.a(5);
		}else{
			//intercurrency conversion a/c
			gainlossacno = gen.company.a(12);
		}
L6659:

		if (gainlossacno) {
			if (intercompany) {
				gainlossacno ^= "," ^ fin.currcompany;
			}

			for (var currn = 1; currn <= ncurrs; ++currn) {
				amount = mvoucher.a(companyn, 10, currn);
				base = mvoucher.a(companyn, 11, currn);
				if (amount or base) {
					//IF SUM(MVOUCHER<COMPANYN,11>) THEN
					// *realised gain/losses
					// gainlossacno=COMPANY<4>
					//END ELSE
					// *no base balances so use intercurrency conversion
					// gainlossacno=COMPANY<12>
					// END
					//balance currencies only if there is an intercurrency account
					//IF gainlossacno THEN
					// IF INTERCOMPANY THEN gainlossacno:=',':CURR.COMPANY
					naccs += 1;
					voucher.r(8, naccs, gainlossacno);
					voucher.r(10, naccs, -amount ^ currs.a(1, 1, currn));
					voucher.r(11, naccs, -base);
					// END
				}
			};//currn;

			//no currency amounts
		}else{

			//if currency balances but base does not then
			//add postings to exchange gains/losses
			//unless there is only one currency and the difference
			//is no more than 2 cents
			//in which case adjust base on the first line
			temp = mvoucher.a(companyn, 11);
			temp.converter(SVM ^ "0.", "");
			if (temp) {
				//IF NCURRS GT 1 THEN GOTO revalit
				//TEMP=SUM(MVOUCHER<COMPANYN,11>)
				//IF ABS(TEMP) GT 0.02 THEN GOTO revalit
				if (ncurrs > 1 or gainlossamt.abs() > .02) {
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
			if (not(gen.currency.reado(gen.currencies, reqcurr))) {
				gen.currency = "";
			}
			if (not(gen.currency.a(4).locatebyusing(voucherdate, "DR", vn, VM))) {
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

}


libraryexit()