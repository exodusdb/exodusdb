#include <exodus/library.h>
libraryinit()

var vn;
var sn;//num

function main(in balancesfile, in balanceskey, in deleting, in balanceperiod, in amount, in baseamount, in xxxxxx, in basecurrency, in vouchertype) {
	//c fin
	//y2k
	var currencycode = balanceskey.field("*", 4);

	////////////////
updatebalances:
	////////////////
	//TRANSACTION CURRENCY RECORD
	//A) PART1 - CURRENCY
	//LOCK BALANCES.FILE,BALANCES.KEY ELSE
	// CALL MSG('TRYING TO LOCK BALANCES FILE ':BALANCES.KEY,'T1','','')
	// GOTO UPDATE.BALANCES
	// END
	var balances;
	if (not(balances.read(balancesfile, balanceskey))) {
		balances = "";
	}

	//backward compatibility
	//calculate average unit cost (relevent for noncurrency only)
	if (currencycode ne basecurrency and (balances.a(18) == "")) {
		var quantity = (balances.a(1) - balances.a(2)).oconv("MD40PZ");
		var cost = (balances.a(11) - balances.a(12)).oconv("MD40PZ");
		if (quantity and cost) {
			var avgunitcost = (cost / quantity).oconv("MD40PZ");
			balances.r(18, avgunitcost + 0);
		}
	}

	var fn = 1;
	var addamount = amount;
	gosub add();

	///////////////
	//if this is base currency then we have finished
	///////////////
	if (currencycode == basecurrency) {

		gosub clearconversionflags();

		balances.write(balancesfile, balanceskey);
		//UNLOCK BALANCES.FILE,BALANCES.KEY
		return 0;
	}

	//////
	//PART3 - BASE EQUIVALENT STORED IN THE SAME RECORD
	//////

	fn = 11;
	addamount = baseamount;
	gosub add();

	//recalculate average unit cost (relevent for noncurrency only)
	var quantity = (balances.a(1) - balances.a(2)).oconv("MD40PZ");
	var cost = (balances.a(11) - balances.a(12)).oconv("MD40PZ");
	if (quantity and cost) {
		var avgunitcost = (cost / quantity).oconv("MD40PZ");
		balances.r(18, avgunitcost + 0);
	}

	gosub clearconversionflags();

	balances.r(6, "");
	balances.r(7, "");
	balances.r(8, "");
	balances.r(9, "");

	balances.write(balancesfile, balanceskey);
	//UNLOCK BALANCES.FILE,BALANCES.KEY

	//////
	//PART2 - TOTAL OTHER CURRENCIES CONVERTED STORED IN BASE CURRENCY RECORD
	//////
	var foreignbalanceskey = balanceskey.fieldstore("*", 4, 1, basecurrency);
lockbase:
	// LOCK BALANCES.FILE,FOREIGN.BALANCES.KEY ELSE
	// CALL MSG('TRYING TO LOCK BALANCES FILE ':FOREIGN.BALANCES.KEY,'T1','','')
	// GOTO LOCK.BASE
	// END
	if (not(balances.read(balancesfile, foreignbalanceskey))) {
		balances = "";
	}

	fn = 6;
	addamount = baseamount;
	gosub add();

	//list of other currencies stored in base currency record
	if (not deleting) {
		if (not(balances.a(16).locateby(currencycode, "AL", vn))) {
			balances.inserter(16, vn, currencycode);
		}
	}

	gosub clearconversionflags();

	balances.write(balancesfile, foreignbalanceskey);
	//UNLOCK BALANCES.FILE,FOREIGN.BALANCES.KEY
	return 0;

}

subroutine add() {
	if (vouchertype) {
		if (not(balances.a(17, 1).locateusing(vouchertype, var().chr(252), sn))) {
			if (balanceperiod) {
				balances.r(17, 1, -1, vouchertype);
			}
		}
		goto 453;
	}
	sn = 1;
//L453:

	//decide debit or credit side independently for amount and base
	//(no negative numbers in the balances record)
	if (addamount < 0) {
		addamount = -addamount;
		fn += 1;
	}

	//if deleting then invert the amount
	if (deleting) {
		addamount = -addamount;
	}

	balances.r(fn, 0 + ((balances.a(fn) + addamount).oconv("MD40P")));
	if (balanceperiod) {
		balances.r(fn + 2, balanceperiod, sn, 0 + ((balances.a(fn + 2, balanceperiod, sn) + addamount).oconv("MD40P")));
	}

	return;

}

subroutine clearconversionflags() {
	//22/5/2001 clear the flags indicating that conversion is available
	// in f6/7/8/9 of the currency records
	balances.r(20, "");
	return;

}


libraryexit()
