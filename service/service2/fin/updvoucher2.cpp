#include <exodus/library.h>
libraryinit()

#include <addcent.h>
#include <updvoucher.h>
#include <updvoucher2.h>
#include <daybooksubs3.h>
#include <sysmsg.h>
#include <updalloc.h>
#include <giveway.h>
#include <split.h>
#include <updindex.h>
#include <nextkey.h>

#include <gen.h>
#include <agy.h>
#include <fin.h>

var temp;
var internalvouchertype;
var xx;
var vtypereverse;//num
var typen;
var type;
var daybookstyle;//num
var amount;//num
var voucherunreversed;
var batchid;
var currencycode;
var currencycompanytags;
var vn;
var amountbase;
var accno1;
var accno2;
var details;
var outputfile;

var interactive;
var vouchertype;
var voucherno;
var origvoucherno;
var vouchercompanycode;
var voucherdate;
var reference;
var yearperiod;
var period;
var year;
var fullyear;
var fullperiod;
var username;

function main(io mode, io voucher, io vouchercode, io allocs, in username0="") {

	//this program takes a voucher from a batch or adds it to an unposted batch
	//and ensures that it is entering in the voucher index file
	//but marked as deleted so that standard programs skip them

	//it is designed to be used by:
	//a) non financial applications installed on the same
	//computer as the accounting system to post accounting vouchers
	//into the accounting system directly
	//b) by the standard posting routine to post unposted vouchers
	//in this case the account numbers are purely internal

	//single accounts are assumed to be internal accounts
	//external/internal combinations are converted to pure
	//internal accounts before saving the voucher

	//(since it now post the voucher index and has much identical code
	//it could be provided as a modification of UPD.VOUCHER)

	//voucher should not already exist in the voucher file
	//otherwise an error msg will be returned

	//mode
	//POST - create a new batch and try to post it
	// otherwise leave it unposted and send a system message
	//DELETE - remove an unposted voucher basically
	//anything else - add to an existing unposted batch or create a new one if too big

	interactive = not SYSTEM.a(33);
	//garbagecollect;

	//extract various info from the voucher
	vouchertype = vouchercode.field("*", 1);
	voucherno = vouchercode.field("*", 2);
	origvoucherno = voucherno;
	vouchercompanycode = vouchercode.field("*", 3);
	voucherdate = ((voucher.a(2)).oconv("HEX")) + 0;
	reference = voucher.a(13);

	//opening balance type voucher ?
	if (fin.definition.locate(vouchertype, temp, 6)) {
		internalvouchertype = fin.definition.a(1, temp);
		//DR means that voucher is to be reversed if posted instead of saved in batch
		if (var("JO,OB,TR,RV").locateusing(internalvouchertype, ",", xx)) {
			vtypereverse = 0;
		}else{
			vtypereverse = var("D+").index(fin.definition.a(4, temp)[1]);
			if (not vtypereverse) {
				vtypereverse = internalvouchertype == "PI";
			}
		}
	}else{
		internalvouchertype = vouchertype;
		vtypereverse = "";
	}

	//voucher period/year (from voucher date if not specified)
	yearperiod = voucher.a(16);
	if (not yearperiod) {
		if (gen.company.a(6)) {
			yearperiod = voucherdate.oconv(gen.company.a(6));
		}else{
			temp = voucherdate.oconv("D2/E").substr(-5,5);
			yearperiod = temp.substr(-2,2) ^ temp.substr(1,2);
		}
		voucher.r(16, yearperiod);
	}
	period = yearperiod.substr(-2, 2);
	year = yearperiod.substr(1, 2);
	fullperiod = (period + 0) ^ "/" ^ year;

	//prevent period older than closed period
	var closedperiod = gen.company.a(16);
	if (closedperiod) {
		var tt = addcent(closedperiod.substr(-2, 2)) ^ ("00" ^ closedperiod.field("/", 1)).substr(-2, 2);
		if (tt >= (addcent(fullperiod.substr(-2, 2)) ^ ("00" ^ fullperiod.field("/", 1)).substr(-2, 2))) {
			period = closedperiod.field("/", 1) + 1;
			year = closedperiod.substr(-2, 2);
			if (period > fin.maxperiod) {
				period = (period - fin.maxperiod).oconv("R(0)#2");
				year = (year + 1).oconv("R(0)#2");
			}
			yearperiod = year ^ period.oconv("R(0)#2");
			voucher.r(16, yearperiod);
		}
	}

	if (username0.unassigned()) {
		username = "";
	} else
		username = username0;
	if (not username)
		username=USERNAME;

	//garbagecollect;
	if (fin.batches.open("BATCHES")) {
		gosub postdirect(mode, voucher, vouchercode, allocs);
	}else{
	// GOSUB POSTINDIRECT
	}

	return 0;

}

subroutine postdirect(io mode, io voucher, io vouchercode, io allocs) {
	//daybook styles are
	//1=journal (one account per line)
	//2=invoice (main account per line)
	//3=cashbook (main account per batch)

	if (fin.definition.locate(vouchertype, typen, 6)) {
		type = fin.definition.a(1, typen);
	}else{
		type = "";
	}
	if (type == "") {
		type = vouchertype;
	}

	if (type == "RE" or type == "PA" or type == "PP" or type == "PC") {
		daybookstyle = 3;

	} else if (type == "SI" or type == "CN" or type == "PI" or type == "PC" or type == "INV" or type == "PIN") {
		daybookstyle = 2;

	} else {
		daybookstyle = 1;
	}

	var deleting = mode == "DELETE";

	var voucherx;
	if (voucherx.read(fin.vouchers, vouchercode)) {

		if (mode == "DELETE") {

			if (voucherx.a(7, 1) ne "D") {
				//this could happen when saving an unposted batch
				//where the same voucher number was in another unposted batch now posted
				//call msg(quote(voucher.code):' cannot delete posted vouchers in UPD.VOUCHER2')
				return;
			}

			//delete voucher as in file in preference to any voucher provided
			//(even if the batch number is not the same)
			//if voucher<12>=voucherx<12> then
			voucher = voucherx;
			// end

		}else{

			//should never get here unless saving an invalid batch with duplicate voucher
			if (voucherx.a(7, 1) ne "D") {
				//call msg(quote(voucher.code):' cannot overwrite posted vouchers in UPD.VOUCHER2')
				return;
			}

			//two vouchers with the same number could be in two unposted batches
			//call msg(quote(voucher.code):' already exists in UPD.VOUCHER2')

			//delete the existing voucher first
			//in case resaving an unposted batch
			//or the voucher is *also* in another unposted batch
			var deletemode="DELETE";
			call updvoucher2(deletemode, voucherx, vouchercode, allocs);

		}

	}else{

		//if deleting and not on file then delete the voucher provided
		if (mode == "DELETE") {
		}

	}

	if (mode ne "DELETE") {

		//make sure voucher is marked as deleted
		//so that unposted vouchers are skipped by default in most other programs
		voucher.r(7, "D");

		//add the contra amount and base if missing
		if (voucher.a(10, 2) == "") {
			amount = voucher.a(10, 1);
			var base = voucher.a(11, 1);

			//reverse it
			if (amount) {
				amount.splicer(1, 0, "-");
			}
			if (base) {
				base.splicer(1, 0, "-");
			}
			if (amount.substr(1, 2) == "--") {
				amount.splicer(1, 2, "");
			}
			if (base.substr(1, 2) == "--") {
				base.splicer(1, 2, "");
			}

			voucher.r(10, 2, amount);
			voucher.r(11, 2, base);

		}

	}

	//add to unposted batch/get fake voucher number if none present
	//voucher.no comes back with fake voucher number
	//voucher.code comes back with fake voucher number
	////////////////////////////////////
	if (mode ne "DELETE" and voucher.a(12) == "") {
		var origvouchercode = vouchercode;

		if (vtypereverse) {
			voucherunreversed = voucher;
			gosub reversevoucher(voucher);
		}

		gosub addtobatch(mode, voucher, vouchercode, allocs);

		if (vtypereverse) {
			voucher = voucherunreversed;
		}

		if (mode == "POST") {

			//validate
			//U=leave unposted if invalid
			//X=dont do lockposting/unlockposting (assume done by caller of upd.voucher2)
			var postcmd = "POSTBATCH " ^ batchid ^ " " " " "UX";
			var postasvoucher = 1;
			if (postasvoucher) {
				//V=validate
				postcmd ^= "V";
			}
			
			USER4 = "";

			//call executeas(postcmd, USERNAME);
			USERNAME.exchange(username);
			perform(postcmd);
			username.exchange(USERNAME);

			//and post if validated ok
			if ((USER4.ucase()).index("OK TO POST", 1)) {
				vouchercode = origvouchercode;
				//mark not unposted and force addition to existing or new batch
				voucher.r(7, "");
				voucher.r(12, "");

				//voucher comes correct now and reversed IF required for batch
				//if vtype.reverse then
				// gosub reversevoucher
				// end

				var xx="";
				call updvoucher(xx, voucher, vouchercode, allocs);
//				call flushindex("VOUCHERS");

				fin.batches.deleterecord(batchid);
				call daybooksubs3("RAISE.UNPOSTED.MINIMUM*" ^ batchid.field("*", 2));
//				call flushindex("BATCHES");

			} else if ((USER4.ucase()).index("POSTED OK", 1)) {
				//execute postcmd

			}else{
				//call msg(@user4)
				//for now, return any failures to post as system errors
				//since we expect most postings should be fine
				//still expect errors like "year must be opened before posting" in new year
				call sysmsg(USER4);
			}

			//dont leave good or bad messages around in case it gets returned to the UI
			USER4 = "";

		}

	}
	////////////////////////////////////

	//add into/delete from the index (as unposted/deleted)

	//voucher period/year (from voucher date if not specified)
	yearperiod = voucher.a(16);
	if (not yearperiod) {
		//garbagecollect;
		yearperiod = ((voucher.a(2)).oconv("HEX")).oconv(gen.company.a(6));
	}

	if (allocs) {
		var unpostedmode="UNPOSTED";
		call updalloc(unpostedmode, voucher, vouchercode, allocs);
	}

	//by this time they are only internal account numbers
	//possibly with ,company code appended
	var accnos = voucher.a(8);
	var naccs = accnos.count(VM) + 1;
	for (var accn = 1; accn <= naccs; ++accn) {

		if (not giveway()) {
			{}
		}

		var origaccno = accnos.a(1, accn).field(",", 1);
		var linecompanycode = accnos.a(1, accn).field(",", 2);
		if (not linecompanycode) {
			linecompanycode = vouchercompanycode;
		}

		if (fin.account.read(fin.accounts, origaccno)) {

			if (fin.account.a(5)) {
				linecompanycode = fin.account.a(5);
			}

			//doesnt handle job accounts from voucher<5>
			var account2flag = 0;

			var accname = fin.account.a(1);
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

			if (not summaryacc) {

				//voucher index entry starts with date*voucher type
				var indexentrypart1 = var("\00\00\00" ^ voucher.a(2)).substr(-fin.hexdatesize, fin.hexdatesize);
				indexentrypart1 ^= vouchertype ^ STM;

				//convert voucher number to special format
				//- if digits only then they are converted char(239+nchars) plus hex
				var convvoucherno = vouchercode.field("*", 2);
				if (convvoucherno[1] ne "0") {
					temp = convvoucherno;
					temp.converter("0123456789", "");
					if (not temp) {
						temp = convvoucherno.iconv("HEX");
						convvoucherno = var().chr(239 + temp.length()) ^ temp;
					}
				}

				//voucher index entry continues with voucher number in special format
				indexentrypart1 ^= convvoucherno ^ STM;

				//add currency tag
				var curramount = voucher.a(10, accn);
				if (curramount) {
					amount = split(curramount, currencycode);
				}else{
					amount = 0;
					currencycode = fin.basecurrency;
				}
				if (currencycode ne fin.basecurrency) {
					currencycompanytags = STM ^ currencycode;
				}else{
					//BASE.AMOUNT=CURR.AMOUNT
					currencycompanytags = "";
				}

				//unposted index entries in other companies need to point back to main voucher
				//since we do not split the voucher by company until posted
				if (linecompanycode ne vouchercompanycode) {
					if (currencycompanytags == "") {
						currencycompanytags = STM;
					}
					currencycompanytags ^= STM ^ vouchercompanycode;
				}

				var indexentry = indexentrypart1 ^ accn ^ currencycompanytags;

				var indexkey = yearperiod ^ "*" ^ origaccno ^ "*" ^ linecompanycode;

				//1) balance forward index if necessary
				if (balancetype ne "O" and internalvouchertype.substr(1, 2) ne "OB" and vouchertype.substr(1, 2) ne "OB") {
					call updindex(fin.voucherindex, indexkey, indexentry, deleting);
				}

				//2) open item index if necessary
				if (balancetype ne "B") {
					call updindex(fin.voucherindex, "*" ^ indexkey.field("*", 2, 99), indexentry, deleting);
					//flag line as open item so easier to update documents payable elsewhere
					if (voucher.a(20, accn) == "" and not account2flag) {
						voucher.r(20, accn, SVM);
					}
				}

				//add currency to balance record so "all currencies unconverted" ledger works
				//or just an empty balance record if base currency
				if (not deleting) {
					var balkey = yearperiod.substr(1, 2) ^ "*" ^ origaccno ^ "*" ^ linecompanycode ^ "*" ^ fin.basecurrency;
					var balances;
					if (not(balances.read(fin.balances, balkey))) {
						balances = "";
					}
					if (not(balances.locateby(currencycode, "AL", vn, 16))) {
						if (currencycode ne fin.basecurrency) {
							balances.inserter(16, vn, currencycode);
							//put a record there too
							var("").write(fin.balances, balkey.fieldstore("*", 4, 1, currencycode));
						}
						balances.write(fin.balances, balkey);
					}
				}

			}
		}
	};//accn;

	//delete voucher from file
	if (deleting) {

		fin.vouchers.deleterecord(vouchercode);

		//if deleting then assume deleting/posting a batch
		//so no need to remove it from the batch below
		return;

	}

	//write before, and after if updated
	voucher.write(fin.vouchers, vouchercode);

	return;

}

subroutine addtobatch(io mode, io voucher, io vouchercode, io allocs) {

	//this is all to do with adding/updating the batch
	//which does not apply if the voucher has come from an unposted batch
	//look at last unposted batch
	var nextkeyparam = ":%" ^ vouchercompanycode ^ "*" ^ vouchertype ^ "*U%:BATCHES:" ^ vouchercompanycode ^ "*" ^ vouchertype ^ "*%*U";
	batchid = nextkey(nextkeyparam, "");
	batchid = batchid.fieldstore("*", 3, 1, batchid.field("*", 3) - 1);
	var newbatch = 0;
	var batch;
	var vnos;
	if (mode == "POST") {
		goto newbatch;
	}
	if (not(lockrecord("BATCHES", fin.batches, batchid))) {
		goto newbatch;
	}
	if (batch.read(fin.batches, batchid)) {
		//add to last batch if same period
		if (batch.a(23) ne ((period + 0) ^ "/" ^ year)) {
			goto newbatch;
		}
		if (batch.a(25)) {
			goto newbatch;
		}
		//7/11/98 if len(batch)>16000 then goto newbatch
		if (batch.length() > 10000) {
			goto newbatch;
		}
		if (not interactive and batch.length() > 3000) {
			goto newbatch;
		}

		//dont add to batch if allocating to an unposted voucher in the same batch
		//otherwise will not be able to post the batch without removing the allocation
		vnos = batch.a(1);
		if (vnos) {
			allocs = voucher.a(17);
			if (allocs) {
				var vtype = vouchercode.field("*", 1);
				allocs.converter(SVM, VM);
				var nallocs = allocs.count(VM) + 1;
				for (var allocn = 1; allocn <= nallocs; ++allocn) {
					var alloc = allocs.a(1, allocn);
					if (alloc) {
						if (alloc.field("*", 1) == vtype) {
							//skip out if found
							if (vnos.locate(alloc.field("*", 2), xx, 1)) {
								goto newbatch;
							}
						}
					}
				};//allocn;
			}
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
		batch.r(23, (period + 0) ^ "/" ^ year);
	}
	var batchno = batchid.field("*", 3);

	//add the voucher into the batch

	//get fake voucher no if needed
	if (voucherno == "") {
		var faken = 1;
		for (var ii = 1; ii <= 9999; ++ii) {
			temp = batch.a(2, ii);
		///BREAK;
		if (not temp) break;;
			if (temp.match("0N")) {
				if (batch.a(1, ii) == "") {
					faken += 1;
				}
			}
		};//ii;
		voucherno = batchno ^ ":" ^ faken;
		vouchercode = vouchercode.fieldstore("*", 2, 1, voucherno);
	}

	//voucher header
	var batchln = (batch.a(2)).count(VM) + (batch.a(2) ne "") + 1;
	batch.r(1, batchln, origvoucherno);
	batch.r(2, batchln, voucherdate);
	batch.r(10, batchln, reference);

	//get full internal and external accounts for the batch
	//and make voucher internal account numbers only
	var accnos = voucher.a(8);
	var naccs = accnos.count(VM) + 1;
	for (var accn = 1; accn <= naccs; ++accn) {

		var accno = accnos.a(1, accn);
		if (accno) {
			var origaccno = accno.a(1, 1, 2);
			var newaccno = accno.a(1, 1, 1).field(",", 1);
			var linecompanycode = accno.a(1, 1, 1).field(",", 2);
			if (origaccno) {
				if (not(fin.account.read(fin.accounts, origaccno))) {
					fin.account = "";
				}
				if (fin.account.a(10)) {
					newaccno = fin.account.a(10);
				}
			}else{

				//first check if the sole account is an original account
				if (fin.account.read(fin.accounts, newaccno)) {
					origaccno = newaccno;
					newaccno = fin.account.a(10);

					//otherwise check if it is an external account number
				}else{
					if (not(fin.account.read(fin.accounts, "." ^ newaccno))) {
						fin.account = "";
					}
					origaccno = fin.account.a(10);
					if (not origaccno) {
						origaccno = newaccno;
					}
				}

				accno.r(1, 1, 2, origaccno);
			}

			//standardise the company code
			if (fin.account.a(5)) {
				linecompanycode = fin.account.a(5);
			}
			if (linecompanycode == vouchercompanycode) {
				linecompanycode = "";
			}
			if (linecompanycode) {
				linecompanycode.splicer(1, 0, ",");
			}

			//if open item account and currency amount is zero and base not zero
			//change the account to the exchange gain/loss account
			//and remove any allocations, analysis code etc
			//why? because cant post base amount only lines to open item accounts
			//why? because 0 currency amounts cannot be allocated and must be revalued to zero
			if (fin.account.a(8) ne "B") {
				var xx;
				if (not split(voucher.a(10, accn),xx)) {
					if (voucher.a(11, accn)) {

						//get the gain/loss account number
						origaccno = gen.company.a(4);
						newaccno = origaccno.xlate("ACCOUNTS", 10, "C");
						linecompanycode = "";

						//clear any details
						if (voucher.a(3, accn)) {
							voucher.r(3, accn, "");
						}

						//clear any allocations and analysis code
						voucher.r(17, accn, "");
						voucher.r(19, accn, "");
						voucher.r(20, accn, "");
						voucher.r(37, accn, "");

					}
				}
			}

			//get the full account number for the batch
			accnos.r(1, accn, newaccno ^ linecompanycode ^ SVM ^ origaccno);

			//force the voucher to be only the internal account number (,company code)
			voucher.r(8, accn, origaccno ^ linecompanycode);

		}

	};//accn;

	//voucher lines
	var voucherln = 1;
	//put the first voucher line in the main account
	if (daybookstyle ne 1) {
		var desc = voucher.a(3, 1);
		if (desc == "") {
			if (accnos.a(1, 1, 2)) {
				if (fin.account.read(fin.accounts, accnos.a(1, 1, 2))) {
					desc = desc ^ " " ^ fin.account.a(1).trim();
				}
			}
		}
		//garbagecollect;
		batch.r(3, batchln, desc);
		batch.r(5, batchln, voucher.a(10, 1));
		batch.r(6, batchln, voucher.a(11, 1));
		batch.r(7, batchln, accnos.a(1, 1));
		voucherln += 1;
	}

	while (true) {
		var accno = accnos.a(1, voucherln);
		amount = voucher.a(10, voucherln);
		var base = voucher.a(11, voucherln);
		var tax = voucher.a(26, voucherln);
		var taxbase = voucher.a(27, voucherln);
		var taxcode = voucher.a(24, voucherln);
		if (tax.length() or taxbase.length()) {
			if (taxcode == "") {
				taxcode = "1";
			}
		}
		var allocvouchers = voucher.a(17, voucherln);
		allocvouchers.converter(SVM, ",");
		var allocamounts = voucher.a(19, voucherln);
	///BREAK;
	if (not(accno or amount or base or tax or taxbase)) break;;
		//garbagecollect;
		//reverse the amounts if necessary
		if (daybookstyle ne 1) {
			if (amount) {
				amount.splicer(1, 0, "-");
			}
			if (base) {
				base.splicer(1, 0, "-");
			}
			if (tax.length()) {
				tax.splicer(1, 0, "-");
			}
			if (taxbase.length()) {
				taxbase.splicer(1, 0, "-");
			}
			if (amount.substr(1, 2) == "--") {
				amount.splicer(1, 2, "");
			}
			if (base.substr(1, 2) == "--") {
				base.splicer(1, 2, "");
			}
			if (tax.substr(1, 2) == "--") {
				tax.splicer(1, 2, "");
			}
			if (taxbase.substr(1, 2) == "--") {
				taxbase.splicer(1, 2, "");
			}
		}
		if (batch.a(2, batchln) == "") {
			batch.r(2, batchln, DQ);
		}
		batch.r(17, batchln, amount);
		batch.r(18, batchln, base);
		if (tax.length() or taxbase.length()) {
			batch.r(13, batchln, taxcode);
			batch.r(14, batchln, tax);
			batch.r(16, batchln, taxbase);
		}
		batch.r(19, batchln, accno);

		if (allocvouchers and allocamounts) {
			if (daybookstyle ne 1) {
				var nn = allocamounts.count(SVM) + 1;
				for (var ii = 1; ii <= nn; ++ii) {
					var allocamount = allocamounts.a(1, 1, ii);
					if (allocamount[1] == "-") {
						allocamount.splicer(1, 1, "");
					}else{
						if (allocamount) {
							allocamount.splicer(1, 0, "-");
						}
					}
					allocamounts.r(1, 1, ii, allocamount);
				};//ii;
			}
			batch.r(21, batchln, allocvouchers);
			allocamounts.converter(SVM, ",");
			batch.r(22, batchln, allocamounts);
		}

		temp = voucher.a(3, voucherln);
		if (temp == "") {
			var acno = accno.a(1, 1, 2);
			if (not acno) {
				acno = accno.a(1, 1, 1).field(",", 1);
			}
			if (not(fin.account.read(fin.accounts, acno))) {
				fin.account = "";
			}
			temp = temp ^ " " ^ fin.account.a(1).trim();
		}
		if (temp) {
			batch.r(9, batchln, temp);
		}

		var analysiscode = voucher.a(37, voucherln);
		if (analysiscode) {
			batch.r(40, batchln, analysiscode);
		}

		//zzz add in the VAT element

		voucherln += 1;
		batchln += 1;
	}//loop;

	//update the daybook file
	batch.write(fin.batches, batchid);
	//make sure points to next batch

	if (newbatch) {
		temp = nextkey(nextkeyparam, "");
	}
	var xx = unlockrecord("BATCHES", fin.batches, batchid);

//	call flushindex("BATCHES");

	//send the batch number back to the calling program
	voucher.r(12, batchid.field("*", 3));

	return;

}

/*
	/////////////
postindirect:
	/////////////
	var line = origvoucherno.oconv("L#15");
	line ^= (voucherdate.oconv("D2/E")).oconv("L#15");
	line ^= reference.oconv("L#15");
	line ^= amount.oconv("R#15");
	line ^= currencycode.oconv("R#15");
	line ^= amountbase.oconv("R#15");
	line ^= accno1.oconv("L#15");
	line ^= accno2.oconv("L#15");
	line ^= details;
	line ^= "\r\n";

	//journal file
	//LOCK FILE ZZZZ
	var outputfilename = vouchercompanycode ^ "." ^ vouchertype;
	if (not outputfile.osopen(outputfilename)) {
		call oswrite("", outputfilename);
		if (not outputfile.osopen(outputfilename)) {
			var msg = DQ ^ (outputfilename ^ DQ) ^ " - CANNOT OPEN THE OUTPUT FILE";
			call mssg(msg);
		}
	}
	call osbwrite(line, outputfile, outputfilename, outputfilename.osfile().a(1));
	outputfile.osclose();

	return;

}*/

subroutine reversevoucher(io voucher) {
	gosub reverse(voucher, 10);
	gosub reverse(voucher, 11);
	gosub reverse(voucher, 19);
	gosub reverse(voucher, 20);
	return;

}

subroutine reverse(io voucher, in fn) {
	var amounts = voucher.a(fn);
	if (not amounts) {
		return;
	}
	var nlns = amounts.count(VM) + 1;
	for (var ln = 1; ln <= nlns; ++ln) {
		amount = amounts.a(1, ln);
		if (amount) {
			gosub reverse2(amount);
			amounts.r(1, ln, amount);
		}
	};//ln;
	voucher.r(fn, amounts);
	return;

}

subroutine reverse2(io amount) {
	if (not amount) {
		return;
	}
	var nsublns = amount.count(SVM) + 1;
	for (var subln = 1; subln <= nsublns; ++subln) {
		var subamount = amount.a(1, 1, subln);
		if (subamount) {
			if (subamount[1] == "-") {
				subamount.splicer(1, 1, "");
			}else{
				subamount.splicer(1, 0, "-");
			}
			amount.r(1, 1, subln, subamount);
		}
	};//subln;
	return;

}

libraryexit()
