#include <exodus/library.h>
libraryinit()

#include <log2.h>
#include <openfile.h>
#include <authorised.h>
#include <readaccparams.h>

#include <gen.h>
#include <fin.h>

var logtime;

function main() {

	var thisname="initacc";

	call log2("-----initacc init", logtime);

	if (not iscommon(gen)) {
		var().stop("gen common is not initialised in " ^ thisname);
	}

	//unfortunately general needs finance for now until some finance commons
	//get moved into general common (and this moved into initacc)
	//mv.labelledcommon[3]=new fin_common;
	//if (not iscommon(fin)) {
	//	var().stop("fin common is not initialised in " ^ thisname);
	//}

        //CREATE LABELED COMMON fin
        mv.labelledcommon[3]=new fin_common;
        if (not iscommon(fin)) {
              var().stop("fin common is not initialised in " ^ thisname);
        }

	readaccparams();

	gen.accounting = SYSTEM.a(31);
	gen.accounting.converter(VM, FM);
	gen.accounting.r(1, 1);

	//initialise statement type and hexdates (v.critical and important)
	//TODO move to declare static in fin.h
	fin.statmtypes = "13";
	fin.hexdatesize = 3;
/*TODO
	gosub makeindex("BATCHES", "COMPANY_JOURNAL_YEAR_PERIOD");
	gosub makeindex("VOUCHERS", "REFERENCE_AND_ACNO");
	gosub makeindex("VOUCHERS", "TEXT", "XREF");
	gosub makeindex("VOUCHERS", "AMOUNT_NUMBER");
*/
	call log2("*open the accounts files", logtime);
	openfile("ACCOUNTS",      fin.accounts,     "ACCOUNTS");
	openfile("BALANCES",      fin.balances,     "ACCOUNTS");
	openfile("BATCHES",       fin.batches,      "ACCOUNTS");
	openfile("LEDGERS",       fin.ledgers,      "ACCOUNTS");
	openfile("CHARTS",        fin.charts,       "ACCOUNTS");
	openfile("VOUCHERS",      fin.vouchers,     "ACCOUNTS");
	openfile("VOUCHER_INDEX", fin.voucherindex, "ACCOUNTS");
	openfile("COLLECTIONS",   fin.collections,  "ACCOUNTS");
	//{
	//	call mssg("DO NOT CONTINUE UNLESS YOU KNOW WHAT YOU ARE DOING");
	//}

	log2("insert new or missing tasks", logtime);
	var xx="";
	call authorised("JOURNAL POST FUTURE PERIOD",    xx, "UA");
	call authorised("JOURNAL POST REVENUE",          xx, "UA2");
	call authorised("FINANCIAL REPORT AGING ACCESS", xx, "FINANCIAL REPORT ACCESS");
	call authorised("FINANCIAL REPORT AGING ACCESS PARTIAL", xx, "FINANCIAL REPORT ACCESS PARTIAL");
	call authorised("MENU FINANCE",                  xx);
	call authorised("JOURNAL ACCESS POSTED",         xx);//not working while creation always starts as posted
	call authorised("JOURNAL ACCESS UNPOSTED",       xx);
	call authorised("JOURNAL POST UNALLOCATED", xx);
	call authorised("JOURNAL POST TAX", xx, "NEOSYS");

	//ensure REVALUATION (BASE CURRENCY ONLY) ENTRIES ARE LOCKED
	//if security('JOURNAL POST REVALUATION',xx,'NEOSYS') else null
	var tt;
	if (APPLICATION ne "ACCOUNTS") {
		tt = "NEOSYS";
	}else{
		tt = "";
	}
	call authorised("JOURNAL POST REVALUATION", xx, tt);

	if (not(authorised("JOURNAL POST UNALLOCATED", xx))) {
		{}
	}

	call log2("*initialise statement type and hexdates", logtime);
	fin.statmtypes = "13";
	fin.hexdatesize = 3;

	call log2("*add analysis code column to journals if not pure finance module", logtime);
	if (APPLICATION EQ "ADAGENCY" and fin.definition.a(60) == "") {
		if (lockrecord("DEFINITIONS", DEFINITIONS, "ALL", "", 3)) {
			if (fin.definition.read(DEFINITIONS, "ALL")) {
				var nn = (fin.definition.a(5)).count(VM) + 1;
				for (var ii = 1; ii <= nn; ++ii) {
					if (fin.definition.a(5, ii) and fin.definition.a(60, ii) == "") {
						fin.definition.r(60, ii, "Analysis Code");
					}
				};//ii;
				fin.definition.write(DEFINITIONS, "ALL");
				call unlockrecord("DEFINITIONS", DEFINITIONS, "ALL");
			}
		}

		call log2("*setup default budget names etc", logtime);
		call readaccparams();
	}
	
	call log2("-----initacc exit", logtime);

	return 1;

}

subroutine makeindex(in filename, in indexname, in mode="btree", in lowercase=""){

	if (mode ne "btree" or lowercase ne "") {
		call log2("*WARNING cant create index " ^ filename ^ " " ^
			indexname ^ " " ^ mode, logtime);
		return;
	}
	call log2("check index " ^ filename ^ " " ^ indexname, logtime);
	if (not var().listindexes(filename).index(indexname)) {

		call log2("*create* index " ^ filename ^ " " ^ indexname, logtime);
		if (not createindex(filename, indexname)) {
			printl("WARNING could not create index ", indexname , " for ", filename);
		}
	}
	return;
}

libraryexit()
