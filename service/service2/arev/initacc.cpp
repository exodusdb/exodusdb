#include <exodus/library.h>
libraryinit()

#include <log2.h>
#include <indices2.h>
#include <openfile.h>
#include <authorised.h>
#include <convledgertype.h>
#include <readaccparams.h>

#include <gen.h>
#include <fin.h>

var logtime;
var collections;
var msg;

function main() {

	call log2("*INIT.ACC initialisation", logtime);
	//y2k
	gen.accounting = SYSTEM.a(31);
	gen.accounting.converter(VM, FM);
	gen.accounting.r(1, 1);

	call log2("*add index to batches", logtime);
	if (not(indices2("BATCHES", "COMPANY_JOURNAL_YEAR_PERIOD"))) {
		var("MAKEINDEX BATCHES COMPANY_JOURNAL_YEAR_PERIOD").execute();
	}

	call log2("*add reference index to vouchers", logtime);
	if (not(indices2("VOUCHERS", "REFERENCE_AND_ACNO"))) {
		var("MAKEINDEX VOUCHERS REFERENCE_AND_ACNO").execute();
	}

	call log2("*add text index to vouchers", logtime);
	if (not(indices2("VOUCHERS", "TEXT.XREF"))) {
		var("MAKEINDEX VOUCHERS TEXT XREF").execute();
	}

	call log2("*add number index to vouchers", logtime);
	if (not(indices2("VOUCHERS", "AMOUNT_NUMBER"))) {
		var("MAKEINDEX VOUCHERS AMOUNT_NUMBER").execute();
	}

	call log2("*open the accounts files", logtime);
	var valid = 1;

	if (not(openfile("ACCOUNTS", fin.accounts))) {
		valid = 0;
	}
	if (not(openfile("BALANCES", fin.balancesfile))) {
		valid = 0;
	}
	if (not(openfile("BATCHES", fin.batches))) {
		valid = 0;
	}
	if (not(openfile("LEDGERS", fin.ledgers))) {
		valid = 0;
	}
	if (not(openfile("CHARTS", fin.charts))) {
		valid = 0;
	}
	if (not(openfile("DEFINITIONS", gen._definitions))) {
		valid = 0;
	}
	if (not(openfile("VOUCHERS", fin.vouchers))) {
		valid = 0;
	}
	if (not(openfile("VOUCHER.INDEX", fin.voucherindex))) {
		valid = 0;
	}
	if (not(openfile("COLLECTIONS", collections, "ACCOUNTS", 1))) {
		{}
	}
	if (not valid) {
		call mssg("DO NOT CONTINUE UNLESS YOU KNOW WHAT YOU ARE DOING");
	}

	//force new tasks
	if (not(authorised("JOURNAL POST FUTURE PERIOD", msg, "UA"))) {
		{}
	}
	if (not(authorised("JOURNAL POST REVENUE", msg, "UA2"))) {
		{}
	}
	if (not(authorised("FINANCIAL REPORT AGING ACCESS", msg, "FINANCIAL REPORT ACCESS"))) {
		{}
	}
	if (not(authorised("FINANCIAL REPORT AGING ACCESS PARTIAL", msg, "FINANCIAL REPORT ACCESS PARTIAL"))) {
		{}
	}
	if (not(authorised("MENU FINANCE"))) {
		{}
	}
	if (not(authorised("JOURNAL ACCESS POSTED"))) {
		{}
	}
	if (not(authorised("JOURNAL ACCESS UNPOSTED"))) {
		{}
	}

	call log2("*convert ledger types", logtime);
	call convledgertype();

	//ensure REVALUATION (BASE CURRENCY ONLY) ENTRIES ARE LOCKED
	//if security('JOURNAL POST REVALUATION',msg,'NEOSYS') else null
	if (ACCOUNT == "ADAGENCY") {
		var tt = "NEOSYS";
	}else{
		var tt = "";
	}
	if (not(authorised("JOURNAL POST REVALUATION"))) {
		{}
	}
	if (not(authorised("JOURNAL POST UNALLOCATED"))) {
		{}
	}

	call log2("*initialise statement type and hexdates", logtime);
	fin.statmtypes = "13";
	fin.hexdatesize = 3;

		call log2("*quit INIT.ACC", logtime);

	return;

}


libraryexit()