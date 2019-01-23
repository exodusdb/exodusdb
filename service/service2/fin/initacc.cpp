#include <exodus/library.h>
libraryinit()

#include <log2.h>
#include <openfile.h>
#include <authorised.h>
#include <convledgertype.h>
#include <vouchertypesubs.h>
#include <ledgersubs.h>
#include <chartsubs2.h>
#include <readaccparams.h>

#include <gen.h>
#include <fin.h>
#include <trading.h>

var logtime;
var xx;
var collections;
var msg;
var chartid;
var voucherid;
var realprogfilename;

function main() {
	//
	//c fin

	call log2("*INIT.ACC initialisation", logtime);
	//y2k
	gen.accounting = SYSTEM.a(31);
	gen.accounting.converter(VM, FM);
	gen.accounting.r(1, 1);

	call log2("*create VOUCHER_VERSIONS if missing", logtime);
	if (not(openfile("VOUCHER_VERSIONS", xx, "VOUCHERS", 1))) {
		var valid = "";
	}

	call log2("*create BATCH_VERSIONS (journals) if missing", logtime);
	if (not(openfile("BATCH_VERSIONS", xx, "BATCHES", 1))) {
		var valid = "";
	}

	call log2("*detach BOM", logtime);
	if (xx.open("DICT.BOM", "")) {
		perform("DETACH DICT.BOM (S)");
	}

	call log2("*equ onlystockledgers to accounting<2>", logtime);
	//equ allocorder to accounting<3>
	// blank or 0 =by date
	// 1=by voucher
	// 2=by reference

	call log2("*convert batch history to versions style", logtime);
	var tt;
	if (not(tt.read(DEFINITIONS, "CONVERTED*BATCHES"))) {
		tt = "";
	}
	if (tt <= 17635) {
		execute("UPDATE BATCHES CONVBATCHES (U)");
		var().date().write(DEFINITIONS, "CONVERTED*BATCHES");
	}

	call log2("*add missing voucher version history", logtime);
	if (not(tt.read(DEFINITIONS, "CONVERTED*VOUCHERS"))) {
		tt = "";
	}
	if (tt <= 17648) {
		//batches is needed to be opened in CONVVOUCHERS
		if (not(openfile("BATCHES", fin.batches))) {
			var valid = 0;
		}
		execute("UPDATE VOUCHERS CONVVOUCHERS (U)");
		var().date().write(DEFINITIONS, "CONVERTED*VOUCHERS");
	}

	call log2("*add index to batches", logtime);
	if (not(listindexes("BATCHES", "COMPANY_JOURNAL_YEAR_PERIOD"))) {
		execute("MAKEINDEX BATCHES COMPANY_JOURNAL_YEAR_PERIOD");
	}

	call log2("*add reference index to vouchers", logtime);
	if (not(listindexes("VOUCHERS", "REFERENCE_AND_ACNO"))) {
		execute("MAKEINDEX VOUCHERS REFERENCE_AND_ACNO");
	}

	call log2("*add text index to vouchers", logtime);
	//warning this could be skipped if vouchers file is large
	//and/or multiple databases in one installation
	//and done manually (maybe sysmsg to instruct support)
	if (not(tt.read(DEFINITIONS, "REINDEX*VOUCHERS*TEXT*XREF"))) {
		tt = "";
	}
	if (not listindexes("VOUCHERS", "TEXT.XREF") or (tt.a(1) < 17532)) {
		execute("MAKEINDEX VOUCHERS TEXT XREF");
		var().date().write(DEFINITIONS, "REINDEX*VOUCHERS*TEXT*XREF");
	}

	call log2("*add number index to vouchers", logtime);
	if (not(listindexes("VOUCHERS", "AMOUNT_NUMBER"))) {
		execute("MAKEINDEX VOUCHERS AMOUNT_NUMBER");
	}

	call log2("*open the accounts files", logtime);
	var valid = 1;

	if (not(openfile("ACCOUNTS", fin.accounts))) {
		valid = 0;
	}
	if (not(openfile("BALANCES", fin.balances))) {
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
	if (not(openfile("DEFINITIONS", DEFINITIONS))) {
		valid = 0;
	}
	if (not(openfile("VOUCHERS", fin.vouchers))) {
		valid = 0;
	}
	if (not(openfile("VOUCHER_INDEX", fin.voucherindex))) {
		valid = 0;
	}
	if (not(trading.sales.open("SALES", ""))) {
		{}
	}
	if (not(trading.purchases.open("PURCHASES", ""))) {
		{}
	}
	if (not(openfile("COLLECTIONS", collections, "ACCOUNTS", 1))) {
		{}
	}
	if (not valid) {
		var().chr(7).output()<< var().chr(7)<< var().chr(7);
		call mssg("DO NOT CONTINUE UNLESS YOU KNOW WHAT YOU ARE DOING");
		var().stop();
	}

	call log2("*delete some program files that were accidentally put in the data directory", logtime);
	var dictdeleted = 0;
	var filename = "ACCOUNTS";
	gosub deldict();
	filename = "BALANCES";
	gosub deldict();
	filename = "BATCHES";
	gosub deldict();
	filename = "CHARTS";
	gosub deldict();
	filename = "LEDGERS";
	gosub deldict();
	filename = "VOUCHER_INDEX";
	gosub deldict();
	filename = "VOUCHERS";
	gosub deldict();
	if (dictdeleted) {
		call mssg("THE NEW PROGRAMS HAVE CONVERTED YOUR OLD DATA|YOU MUST NOW EXIT AND RE-ENTER|THE NEOSYS PROGRAM AGAIN");
		perform("OFF");
	}
	filename = "COMPANIES";
	gosub deldict();

	//force new tasks
	if (not(authorised("JOURNAL POST FUTURE PERIOD", msg, "UA"))) {
		{}
	}
	if (not(authorised("%RENAME%" "JOURNAL POST REVENUE ACCOUNTS", msg, "JOURNAL POST REVENUE"))) {
		{}
	}
	//if security('%RENAME%':'JOURNAL ACCESS OTHER PEOPLES BATCHES',msg,'JOURNAL ACCESS OTHER PEOPLES') else null
	if (not(authorised("%RENAME%" "JOURNAL ACCESS OTHER PEOPLES", msg, "JOURNAL ACCESS OTHERS"))) {
		{}
	}
	if (not(authorised("%RENAME%" "JOURNAL ACCESS POSTED BATCHES", msg, "JOURNAL ACCESS POSTED"))) {
		{}
	}
	if (not(authorised("%RENAME%" "VAT STATEMENT", msg, "TAX/VAT RETURN"))) {
		{}
	}
	if (not(authorised("JOURNAL POST REVENUE", msg, "UA2"))) {
		{}
	}
	if (not(authorised("JOURNAL POST REBATE", msg, "JOURNAL POST REVENUE"))) {
		{}
	}
	if (not(authorised("FINANCIAL REPORT AGING ACCESS", msg, "FINANCIAL REPORT ACCESS"))) {
		{}
	}
	if (not(authorised("%DELETE%FINANCIAL REPORT AGING ACCESS PARTIAL", msg))) {
		{}
	}
	if (not(authorised("%DELETE%FINANCIAL REPORT ACCESS PARTIAL", msg, ""))) {
		{}
	}
	if (not(authorised("%DELETE%LEDGER ACCESS PARTIAL"))) {
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
	if (not(authorised("JOURNAL POST TAX", "", "NEOSYS"))) {
		{}
	}

	call log2("*convert ledger types", logtime);
	call convledgertype();

	call log2("*convert daybook setup to include column titles", logtime);
	call vouchertypesubs("DEF.COLUMNS");
	//build list of ledgers for each type of ledger in the definitions file
	var temp;
	if (not(temp.read(DEFINITIONS, "LEDGER.TYPE."))) {
		call ledgersubs("MAKE.LEDGER.TYPE.LISTS");
	}

	call log2("*add nlines to ledgers", logtime);
	if (not(xx.read(DEFINITIONS, "LEDGERS_NLINES"))) {
		var().clearselect();
		fin.charts.select();
nextchart:
		if (readnext(chartid)) {

			var chart;
			if (not(chart.read(fin.charts, chartid))) {
				goto nextchart;
			}

			tt = chart.a(3);
			var nlines = tt.count(VM) + (tt ne "");
			tt = chart.a(4);
			tt = tt.count(VM) + (tt ne "");
			if (nlines < tt) {
				nlines = tt;
			}

			//update ledger only to avoid unnecessary conflict with user updates
			//popup info only uses ledger file and nlines on chart will be updated
			//during user updates
			nlines.writev(fin.ledgers, chartid, 34);
			

			goto nextchart;
		}
		var().date().write(DEFINITIONS, "LEDGERS_NLINES");
	}

	call log2("*register all closing accounts");
	if (not(temp.read(DEFINITIONS, "CONVERTED*CLOSINGACC"))) {
		perform("WINDOWSTUB CHART.SUBS2 " "REGISTERALLCLOSINGACCS");
		var(15781).write(DEFINITIONS, "CONVERTED*CLOSINGACC");
	}

	var doubletilde = var().chr(247);
	call log2("*convert double-tilde to tilde (~) account name place holder", logtime);
	if (not(temp.read(DEFINITIONS, "CONVERTED*ACCOUNTNAMEPLACEHOLDER"))) {

		//double check/look for ~ to indicate already converted
		var().clearselect();
		fin.vouchers.select();
		var vouchern = 0;
nextvoucher:
		if (readnext(voucherid)) {
			var voucher;
			if (not(voucher.read(fin.vouchers, voucherid))) {
				goto nextvoucher;
			}
			if (voucher.a(3).index("~")) {
				goto placemarkerconverted;
			}
			vouchern += 1;
			if (vouchern < 1000) {
				goto nextvoucher;
			}
		}

		perform("REPLACE VOUCHERS " ^ (DQ ^ (doubletilde ^ DQ)) ^ " ~ 3 (U)");
		perform("REPLACE BATCHES " ^ (DQ ^ (doubletilde ^ DQ)) ^ " ~ 3,9 (U)");
placemarkerconverted:
		var().date().write(DEFINITIONS, "CONVERTED*ACCOUNTNAMEPLACEHOLDER");
	}

	call log2("*split report design", logtime);
	if (not(temp.read(DEFINITIONS, "CONVERTED*SPLITREPORTDESIGN"))) {

		var().clearselect();
		fin.charts.select();
nextchart2:
		if (readnext(chartid)) {
			var chart;
			if (not(chart.read(fin.charts, chartid))) {
				goto nextchart2;
			}
			RECORD = chart;

			if (RECORD.a(3).index(SVM)) {
				call chartsubs2("SPLITREPORTDESIGN");
			}

			//remove old ibm graphics underline (ONLY if many since they are also latin)
			var names = RECORD.a(3);
			var singleline = var().chr(196);
			var doubleline = var().chr(205);
			if (names.index(singleline.str(3))) {
				names.converter(singleline, "");
			}
			if (names.index(doubleline.str(3))) {
				names.converter(doubleline, "");
			}
			while (true) {
			///BREAK;
			if (not(names.index(" " ^ VM))) break;;
				names.swapper(" " ^ VM, VM);
			}//loop;
			RECORD.r(3, names);

			if (RECORD ne chart) {
				RECORD.write(fin.charts, chartid);
			}

			goto nextchart2;
		}

		var().date().write(DEFINITIONS, "CONVERTED*SPLITREPORTDESIGN");
	}

	call log2("*check if all stock ledgers", logtime);
	gen.accounting.r(2, "");
	if (temp.read(DEFINITIONS, "LEDGER.TYPE.STOCK")) {
		var temp2;
		if (temp2.read(fin.ledgers, "%RECORDS%")) {
			if (temp and (temp.count(VM) == temp2.count(FM))) {
				gen.accounting.r(2, 1);
			}
		}
	}

	//ensure REVALUATION (BASE CURRENCY ONLY) ENTRIES ARE LOCKED
	//if security('JOURNAL POST REVALUATION',msg,'NEOSYS') else null
	if (APPLICATION == "ADAGENCY") {
		tt = "NEOSYS";
	}else{
		tt = "";
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

	call log2("*add analysis code column to journals if not pure finance module");
	if ((APPLICATION == "ADAGENCY") and (fin.definition.a(60) == "")) {
		if (lockrecord("DEFINITIONS", DEFINITIONS, "ALL", "", 3)) {
			if (fin.definition.read(DEFINITIONS, "ALL")) {
				var nn = fin.definition.a(5).count(VM) + 1;
				for (var ii = 1; ii <= nn; ++ii) {
					if (fin.definition.a(5, ii) and (fin.definition.a(60, ii) == "")) {
						fin.definition.r(60, ii, "Analysis Code");
					}
				};//ii;
				fin.definition.write(DEFINITIONS, "ALL");
				call unlockrecord("DEFINITIONS", DEFINITIONS, "ALL");
			}
		}

		call log2("*setup default budget names etc", logtime);
		call readaccparams();

		goto exit;

	return "";
}

subroutine deldict() {
		var FILES;
		if (not(FILES.open("FILES", ""))) {
			return;
		}
		var dictfile;
		if (not(dictfile.open("DICT", filename))) {
			return;
		}
		if (not(dictfile.index("\\data\\"))) {
			return;
		}
		//perform 'DELETEFILE DICT.':filename:' (S)'
		//perform 'attach .\accounts dict.':filename:' (s)'
		var file;
		if (not(file.read(FILES, "DICT." ^ filename))) {
			return;
		}
		var datavol = file.a(1);
		fin.account = file.a(3);
		if (dictfile.index("GENERAL")) {
			realprogfilename = "GBP";
		}else{
			realprogfilename = "ABP";
		}
		if (not(file.read(FILES, realprogfilename))) {
			return;
		}
		var progvol = file.a(1);
		perform("ATTACH " ^ progvol ^ " DICT." ^ filename ^ " (S)");
		if (xx.open("XFILE", "")) {
			perform("DETACH XFILE (S)");
		}
		perform("SETFILE " ^ datavol ^ " " ^ fin.account ^ " DICT." ^ filename ^ " XFILE" " (S)");
		perform("DELETEFILE XFILE" " (S)");
		dictdeleted = 1;

		return;

exit:
		call log2("*quit INIT.ACC", logtime);

	}

}


libraryexit()
