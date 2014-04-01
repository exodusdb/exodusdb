#include <exodus/library.h>
libraryinit()

#include <getmark.h>
#include <authorised.h>
#include <ranges.h>
#include <jobsubs.h>
#include <agencysubs.h>
#include <validcode2.h>
#include <validcode3.h>
#include <initcompany.h>
#include <addcent.h>
#include <htmllib.h>
#include <gethtml.h>
#include <readcss.h>

#include <select2.h>
#include <printtx.h>

#include <gen.h>
#include <fin.h>
#include <agy.h>
#include <win.h>

var msg;
var xx;
var reply;
var temp;
var totalbase;//num
var tt;
var tag;
var rowattribs;
var avghourlyrate;

var sep;
var nbsp;
var showcancelledlines;
var ownmark;
var tx;
var mark;

//gets parameters from PSEUDO (or SENTENCE if PSEUDO="" for testing)
function main() {
 
	var html = 1;
	var interactive = not SYSTEM.a(33);
	nbsp = "&nbsp;";
	sep = VM;

	showcancelledlines = 0;

	ownmark = "Production Management";
	call getmark("OWN", html, ownmark);

	if (not(authorised("JOB ACCESS", msg, ""))) {
		call mssg(msg);
		return 0;
	}

	if (PSEUDO.unassigned() or PSEUDO eq "") {
		PSEUDO=SENTENCE.convert(" ",FM);
	}

	var reqsections = PSEUDO.a(3);

	//check securities
	var accessbrief = 1;
	var accesscost = authorised("PRODUCTION COST ACCESS", msg, "");
	if (accesscost) {
		accesscost = authorised("PRODUCTION ORDER ACCESS", msg, "");
	}
	//accesstime=accesscost
	var accesstime = authorised("PRODUCTION TIME ACCESS", msg, "PRODUCTION COST ACCESS");
	var accessbill = authorised("PRODUCTION ESTIMATE ACCESS", msg, "");

	//suppress unwanted sections
	//only check if some sections required because mac doesnt specify sections
	//1=brief, 2=costs, 3=time, 4=bills
	if (reqsections) {
		if (not reqsections.index(1)) {
			accessbrief = 0;
		}
		if (not reqsections.index(2)) {
			accesscost = 0;
		}
		if (not reqsections.index(3)) {
			accesstime = 0;
		}
		if (not reqsections.index(4)) {
			accessbill = 0;
		}
	}

//init:
	if (not(DICT.open("DICT_JOBS"))) {
		call fsmsg();
		return 0;
	}

	var jobversions;
	if (not(jobversions.open("JOB_VERSIONS", ""))) {
		call fsmsg();
		return 0;
	}

	var jobdict = DICT;
	var timesheetdict;
	if (not(timesheetdict.open("DICT_TIMESHEETS"))) {
		call fsmsg();
		return 0;
	}

	var jobnos = PSEUDO.a(2);
	if (jobnos) {
		call ranges(jobnos, xx);
		if (not jobnos) {
			return 0;
		}
	}
	if (not jobnos) {
		call mssg("Job number is required but missing");
		return 0;
	}

	var jobn = 0;
	jobnos.converter(", " ^ VM, FM ^ FM ^ FM);
	//call trim2(jobnos,fm)
	jobnos.converter(FM ^ " ", " " ^ FM);
	jobnos.trimmer();
	jobnos.converter(FM ^ " ", " " ^ FM);

//nextjob:
	while (not esctoexit()) {
		if (jobnos) {
			jobn += 1;
			ID = jobnos.a(jobn);
			if (not ID) {
				break;
			}
		}else{
			if (not var().readnext(ID)) {
				break;
			}
		}

		var isoldversion = 0;
		if (not(RECORD.read(agy.jobs, ID))) {
			if (not(RECORD.read(jobversions, ID))) {
				call mssg(DQ ^ (ID ^ DQ) ^ " - JOB NUMBER DOES NOT EXIST");
				continue;
			}
			isoldversion = 1;
		}
		MV = 0;

		var mark = "";
		call getmark("CLIENT", html, mark);

		if (not(validcode2(calculate("COMPANY_CODE"), calculate("CLIENT_CODE"), calculate("BRAND_CODE"), agy.brands, msg))) {
unauth:
			call mssg("Job No:" ^ ID ^ "|" ^ msg);
			continue;
		}

		if (not(validcode3(calculate("MARKET_CODE"), "", "", xx="", msg))) {
			goto unauth;
		}

	//jobinit:

		if (RECORD.a(14) and RECORD.a(14) ne gen.gcurrcompany) {
			call initcompany(RECORD.a(14));
		}

		var head = "";
		head ^= "JOB FILE";
		if (calculate("CLOSED") == "Y") {
			head ^= " (Closed)";
		} else if (calculate("CLOSED") == "N") {
			head ^= " (Reopened)";
		} else {
			head ^= " (Open)";
		}
		head = "<H2~style=\"margin:0px;text-align:center\">" ^ head ^ "</H2>";
		head ^= "<CENTER>";
		head ^= "produced " ^ timedate();
		head ^= FM ^ "</CENTER>";
		head ^= FM ^ FM;

//		call printtx(xx,"head",head);
//		call printtx(xx="");

		sep = VM;

		tx = head;
		tx.converter("~", " ");
		head = "";
		call printtx(tx);

		var l18 = "";
		var tr = "<TR>";
		//trl='<TR ALIGN=LEFT>'
		var trx = "</TR>";
		var td = "<TD>";
		var tdx = "</TD>";

		head ^= "<TABLE border=0 ID=\"HEADER\" ALIGN=CENTER";
		head ^= " WIDTH=100%";
		head ^= ">";
		head ^= var("<COL WIDTH=7% ALIGN=LEFT><COL STYLE=\"Font-Weight:Bold\">").str(3);
		//head:=str('<COL ALIGN=LEFT><COL STYLE="Font-Weight:Bold">',3)

		//tab:='<P>'

		var tab = "";

		//1st line

		tab ^= tr ^ td ^ "Client:" ^ tdx ^ td ^ calculate("CLIENT_NAME") ^ tdx ^ td;

		var attention = calculate("ATTENTION");
		if (attention) {
			tab ^= "Attention:";
		}
		tab ^= tdx ^ td ^ attention ^ tdx;

		var jobnumber = calculate("NUMBER");
		if (not RECORD.a(29)) {
			RECORD.r(29, "A");
		}
		if (isoldversion or RECORD.a(29) > "A") {
			jobnumber ^= " (rev " ^ RECORD.a(29) ^ ")";
		}
		if (isoldversion) {
			jobnumber ^= "*";
		}
		tab ^= td ^ "Job No:" ^ tdx ^ td ^ jobnumber ^ tdx;
		tab ^= trx;

		//2nd line

		tab ^= tr ^ td ^ "Brand:" ^ tdx ^ td ^ calculate("FULL_BRAND_NAME") ^ tdx ^ td;

		//executive={EXECUTIVE_CODE}
		var executive = calculate("EXECUTIVE_NAME");
		if (executive) {
			tab ^= "Executive:";
		}
		tab ^= tdx ^ td ^ executive ^ tdx;

		//income=sum({QUOTE_AMOUNT_BASE})
		//cost=sum({ORDER_AMOUNT_BASE})

		MV = 0;

		var bases = calculate("ORDER_AMOUNT_BASE");
		var statuses = calculate("ORDER_STATUS2");
		gosub sumnotcancelled(bases, statuses, totalbase);
		var cost = totalbase;

		bases = calculate("QUOTE_AMOUNT_BASE");
		statuses = calculate("QUOTE_STATUS2");
		gosub sumnotcancelled(bases, statuses, totalbase);
		var income = totalbase;

		tab ^= td;

		if (accesscost and accessbill) {

			if (income) {
				tt = (income - cost) / income * 100;
				//garbagecollect;
				tt = tt.oconv("MD10PZ") ^ "%";
			}else{
				tt = "";
			}
			//t={PERCENT_MARGIN2} 'MD10PZ'
			//garbagecollect
			//t=margin 'MD10PZ'
			//if t then t:='%'
			if (income.length() or cost.length()) {
				//garbagecollect;
				tt ^= " " ^ (income - cost).oconv(fin.basefmt) ^ agy.agp.a(2);
			}

			tab ^= "Margin:";
			tab ^= tdx ^ td;
			tab ^= tt;

		}

		tab ^= tdx ^ trx;

		//3rd line

		tab ^= tr ^ td ^ "<nobr>Market-Period:</nobr>" ^ tdx ^ td ^ calculate("MARKET_NAME") ^ " - " ^ addcent(calculate("PERIOD")) ^ tdx ^ td;

		var prodtypedesc = calculate("PRODTYPE_DESC");
		if (prodtypedesc) {
			tab ^= "Type:";
		}
		tab ^= tdx ^ td ^ prodtypedesc ^ tdx ^ td;

		//t={PERCENT_MARKUP2} 'MD10PZ'
		//if t then t:='%'
		if (cost) {
			tt = (income - cost) / cost * 100;
			//garbagecollect;
			tt = tt.oconv("MD10P") ^ "%";
		}else{
			tt = "";
		}

		if (accesscost and accessbill) {
			tab ^= "Markup:";
		}
		tab ^= tdx ^ td;
		if (accesscost and accessbill) {
			tab ^= tt;
		}

		tab ^= tdx ^ trx;

		//4th line

		//tab:=tr:td:'Market:':tdx:td:{MARKET_NAME}:tdx:td
		tab ^= tr ^ td;

		var description = calculate("DESCRIPTION").a(1, 1);
		if (description) {
			tab ^= "Description:";
		}
		tab ^= tdx ^ "<TD COLSPAN=5>" ^ description ^ tdx;

		tab ^= trx;

		//5th line
		//brief=field({DESCRIPTION},vm,2,9999)
		//if brief then
		// swap vm with '<BR>':crlf in brief
		// tab:=tr:td:'Brief:':tdx
		// tab:='<TD COLSPAN=6>':brief:tdx:trx
		// end

		tab.swapper("&", "&amp;");
		//call htmllib('TABLE.MAKE',tab,t,'')
		tab.swapper("<TR>", "<TR ALIGN=LEFT VALIGN=TOP>");

		tab ^= "</TABLE>";

		tab.swapper("\'", "\'\'");
		head ^= tab;

		tx = "";

		//supertable heading
		tx ^= "<TABLE border=0 WIDTH=100%><THEAD><TR><TD>";
		tx ^= head;
		head = "";
		//tx:='<P><P>':mark

		tx ^= "</TD></TR></THEAD>";

		tx ^= "<TBODY>";

		call printtx(tx);

		call printtx(xx,"foot","");
		call printtx(xx,"bottomline","");

		//brief
		description = calculate("DESCRIPTION").field(VM, 2, 9999);

		var l6 = "";
		var l10 = "";
		var l12 = "";
		var r15 = "";
		var r10 = "";
		var r12 = "";
		var l20 = "";
		sep = VM;

		tx = "";

		//5th line
		if (accessbrief) {
			var brief = calculate("DESCRIPTION").field(VM, 2, 9999);
			if (brief) {

				//open superrow
				tx = "<TR><TD><xP><xP></TD></TR>";
				tx ^= "<TR><TD ALIGN=LEFT>";
				call printtx(tx);

				brief.swapper(VM, "<BR>" "\r\n");
				tx = "<TABLE><TR><TD VALIGN=TOP>Brief:</TD><TD>" ^ brief ^ "</TD></TR></TABLE>";

				//close superrow
				tx ^= "</TD></TR>";
				call printtx(tx);

			}
		}

		if (accesscost and RECORD.a(4)) {

			//open superrow
			tx = "<TR><TD><xP><xP></TD></TR>";
			tx ^= "<TR><TD>" ^ mark;
			mark = "";
			call printtx(tx);

			tx = "";
			tx ^= var("Order No.").oconv(l6) ^ sep;
			tx ^= var("Date").oconv(l12) ^ sep;
			tx ^= var("Amount").oconv(r15) ^ sep;
			tx ^= (agy.agp.a(2) ^ " Equiv.").oconv(r12) ^ sep;
			tx ^= var("Description").oconv(l20) ^ sep;
			tx ^= var("Suppl.Inv").oconv(l12) ^ sep;
			tx ^= var("Supplier").oconv(l20) ^ sep;
			tx ^= var("Status").oconv(l12);
			var bar = var("-").str(tx.length());

			tx ^= FM;

			MV = 0;
			temp = calculate("ORDER_NO");
			var nlines = temp.count(VM) + (temp ne "");
			totalbase = 0;
			for (MV = 1; MV <= nlines; ++MV) {

				var status = calculate("ORDER_STATUS2");
				var orderinvno = calculate("ORDER_INV_NO");
				//similar logic in printjob and jobs.subs/postread
				if (orderinvno) {
					status = "INVOICE";
				}
				if (not status) {
					status = "ISSUED";
				}
				var cancelled = status == "CANCELLED";

				if (showcancelledlines or not cancelled) {
					if (MV > 1) {
						tx ^= FM;
					}
					tx ^= (calculate("ORDER_NO")).oconv(l6) ^ sep;
					tx ^= (calculate("ORDER_DATE")).oconv("[DATE,*]" _VM_ "" ^ l12) ^ sep;
					tx ^= (calculate("ORDER_AMOUNT") ^ calculate("ORDER_CURRENCY")).oconv(r15) ^ sep;
					var orderamountbase = calculate("ORDER_AMOUNT_BASE");
					tx ^= orderamountbase.oconv(r12) ^ sep;
					if (not cancelled) {
						totalbase += orderamountbase;
					}
					tx ^= (calculate("ORDER_DESC")).oconv(l20) ^ sep;
					tx ^= orderinvno.oconv(l12) ^ sep;
					tx ^= (calculate("SUPPLIER_NAME")).oconv(l20) ^ sep;
					tx ^= capitalise(status);
				}
			};//MV;

			if (totalbase) {
				tx ^= FM ^ "Total" ^ sep ^ sep ^ sep ^ "<B>" ^ totalbase.oconv(fin.basefmt) ^ "</B>" ^ sep ^ sep ^ sep;
			}

			tt = "";
			tt ^= "<table id=" "\"costs\" align=center class=neosystable";
			tt ^= " cellpadding=1 cellspacing=0 border=1";
			tt ^= " width=100%";
			tt ^= ">";
			//t:=str('<COL WIDTH=7% ALIGN=LEFT><COL STYLE="Font-Weight:Bold">',2)
			tt ^= var("<COL ALIGN=RIGHT>").str(4);
			tx.swapper("&", "&amp;");
			call htmllib("TABLE.MAKE", tx, tt, 1);
			//swap '<TR>' with '<TR VALIGN=TOP>' in tx

			//close superrow
			tx ^= FM ^ "</TR></TR>";

			//tx:='<P>'
			call printtx(tx);

		}

	//	if (accesstime and getreccount(gen.timesheets, "", "")) {
		if (accesstime) {

			//get relevent timesheet lines by department person and date
			//better be under 64kb!!!
			var dictids = "DEPARTMENT2 PERSON_NAME DATE ACTIVITY_NAME DETAILS HOURS RATE AMOUNT";
			var filename = "TIMESHEETS";
			var linkfilename2 = "";
			var sortselect = "WITH JOB_NO " ^ (DQ ^ (ID ^ DQ)) ^ " BY JOB_NO BY DEPARTMENT BY PERSON_CODE BY DATE";
			sortselect ^= " AND WITH AUTHORISED1";
			var options = "";
			var selecteddata = "";
			var response = "";
			call select2(filename, linkfilename2, sortselect, dictids, options, selecteddata, response, "JOB_NO" _VM_ "HOURS" _VM_ "HOURS", "EQ" _VM_ "NE" _VM_ "NE", ID ^ "" _VM_ "0" _VM_ "");

			if (selecteddata) {

				//open superrow
				tx = "<TR><TD><xP><xP></TD></TR>";
				tx ^= "<TR><TD>" ^ mark ^ FM;
				mark = "";

				tx ^= "<table id=\"time\" align=center class=neosystable cellpadding=1 cellspacing=0 border=1 width=100%><thead><col><col><col align=right><col><col><col><col align=right><col align=right><col align=right>";
				call printtx(tx);

				tx ^= "Department" ^ sep;
				tx ^= "Person" ^ sep;
				tx ^= "Date" ^ sep;
				//tx:='Code':sep
				tx ^= "Activity" ^ sep;
				tx ^= "Details" ^ sep;
				tx ^= "Hours" ^ sep;
				tx ^= "Rate" ^ sep;
				tx ^= "Amount";
				tag = "TH";

				rowattribs = "";
				gosub printtxrow(tx);

				tx = "</THEAD>";
				call printtx(tx);

				var personcoln = 2;
				var amountcoln = 8;
				var ratecoln = 7;
				var hourscoln = 6;

				var nlines = selecteddata.count(FM) + 1;
				var totalamount = 0;
				var totalhours = 0;
				var personamount = 0;
				var personhours = 0;
				var personn = 1;
				var personcode = selecteddata.a(1, personcoln);
				for (var ln = 1; ln <= nlines; ++ln) {

					tx = selecteddata.a(ln);
					totalamount += tx.a(1, amountcoln);
					personamount += tx.a(1, amountcoln);
					var hours = tx.a(1, hourscoln);
					if (hours) {
						//personhours += (tx.a(1, hourscoln)).iconv("[MT2U]");
						//totalhours += (tx.a(1, hourscoln)).iconv("[MT2U]");
						personhours += tx.a(1, hourscoln);
						totalhours += tx.a(1, hourscoln);
						tx.swapper(VM, sep);
						tag = "TD";
						rowattribs = "ID=person" ^ personn ^ " style={cursor:hand} style=\"{display:none}\" onclick=\"toggle(" "\'" "person" ^ personn ^ "\'" ")\"";
						gosub printtxrow(tx);
					}

					//person subtotal
					if (selecteddata.a(ln + 1, personcoln) ne personcode) {
						if (personhours or personamount) {
							rowattribs = "style={cursor:hand} onclick=toggle(person" ^ personn ^ ")";
							if (personhours) {
								avghourlyrate = (personamount / personhours).oconv("MD20PZ");
							}else{
								avghourlyrate = "";
							}
							tx = selecteddata.a(ln, 1) ^ sep ^ capitalise(personcode) ^ sep ^ sep ^ sep ^ sep
							 ^ personhours.oconv("MT2U") ^ sep ^ avghourlyrate ^ sep ^ "<B>"
							 ^ personamount.oconv(fin.basefmt) ^ "</B>";
							tag = "TH";
							gosub printtxrow(tx);
							personhours = "";
							personamount = "";
						}
						personcode = selecteddata.a(ln + 1, personcoln);
						personn += 1;
					}

				};//ln;

				if (totalhours or totalamount) {
					if (totalhours) {
						avghourlyrate = (totalamount / totalhours).oconv("MD20PZ");
					}else{
						avghourlyrate = "";
					}
					tx = "Total" ^ sep ^ sep ^ sep ^ sep ^ sep ^ totalhours.oconv("MT2U")
					 ^ sep ^ avghourlyrate ^ sep ^ "<B>" ^ totalamount.oconv(fin.basefmt) ^ "</B>";
					tag = "TH";
					gosub printtxrow(tx);
				}

				tx ^= "</TBODY></TABLE>";

				//close superrow
				tx ^= "</TD></TR>";

				//tx:='<P>'
				call printtx(tx);

			}

		}

		if (accessbill and RECORD.a(10)) {

			//open superrow
			tx = "<TR><TD><xP><xP></TD></TR>";
			tx ^= "<TR><TD>" ^ mark;
			mark = "";
			call printtx(tx);

			tx = "";
			tx ^= var("Estimate No.").oconv(l6) ^ sep;
			tx ^= var("Date").oconv(l12) ^ sep;
			tx ^= var("Amount").oconv(r15) ^ sep;
			tx ^= (agy.agp.a(2) ^ " Equiv.").oconv(r12) ^ sep;
			tx ^= var("Description").oconv(l20) ^ sep;
			tx ^= var("Our Invoice No.").oconv(l12) ^ sep;
			tx ^= var("Receipt No.").oconv(l10) ^ sep;
			tx ^= var("Amount").oconv(r12) ^ sep;
			tx ^= var("Status").oconv(l12);

			var bar = var("-").str(tx.length());

			tx ^= FM;

			MV = 0;
			temp = calculate("QUOTE_NO");
			var nlines = temp.count(VM) + (temp ne "");
			var invnos = calculate("QUOTE_INV_NO");
			totalbase = 0;
			for (MV = 1; MV <= nlines; ++MV) {
				var status = calculate("QUOTE_STATUS2");
				var cancelled = status == "CANCELLED";
				if (showcancelledlines or not cancelled) {
					if (MV > 1) {
						tx ^= FM;
					}
					tx ^= (calculate("QUOTE_NO")).oconv(l6) ^ sep;
					//fix bug in selecteddata tx:=oconv({QUOTE_DATE},'[DATE,*]') 'L#12':' '
					tx ^= ((calculate("QUOTE_DATE").a(1, 1)).oconv("[DATE,*]")).oconv(l12) ^ sep;
					tx ^= (calculate("QUOTE_AMOUNT") ^ calculate("QUOTE_CURRENCY")).oconv(r15) ^ sep;
					//tx:=({QUOTE_AMOUNT_BASE}) r12:sep
					var quoteamountbase = calculate("QUOTE_AMOUNT_BASE");
					tx ^= quoteamountbase.oconv(r12) ^ sep;
					if (not cancelled) {
						totalbase += quoteamountbase;
					}

					tx ^= (calculate("QUOTE_DESC")).oconv(l20) ^ sep;
					tx ^= (calculate("QUOTE_INV_NO")).oconv(l12);

					var invno = invnos.a(1, MV);
					if (invno == "") {
	noreceipt:
						tx ^= VM ^ VM;
					}else{
						var voucher;
						if (not(voucher.read(fin.vouchers, "INV*" ^ invno ^ "*" ^ fin.currcompany))) {
							goto noreceipt;
						}
						var receipts = voucher.a(17, 1);
						var allocamts = voucher.a(20, 1);
						var nreceipts = receipts.count(SVM) + 1;
						for (var receiptn = 1; receiptn <= nreceipts; ++receiptn) {
							if (receiptn > 1) {
								tx ^= FM ^ VM.str(6);
							}else{
								tx ^= sep;
							}
							tx ^= (receipts.a(1, 1, receiptn).field("*", 1, 2)).oconv(r10) ^ sep;
							tx ^= ((allocamts.a(1, 1, receiptn)).oconv("[NUMBER]")).oconv(r12);

						};//receiptn;
					}

					tx ^= sep ^ capitalise(status);

				}

			};//MV;

			if (totalbase) {
				tx ^= FM ^ "Total" ^ sep ^ sep ^ sep ^ "<B>" ^ totalbase.oconv(fin.basefmt) ^ "</B>" ^ sep ^ sep ^ sep;
			}

			tt = "";
			tt ^= "<table id=" "\"income\" align=center class=neosystable";
			tt ^= " cellpadding=1 cellspacing=0 border=1";
			tt ^= " width=100%";
			tt ^= ">";
			//t:=str('<COL WIDTH=7% ALIGN=LEFT><COL STYLE="Font-Weight:Bold">',2)
			tt ^= var("<col align=right>").str(4);
			tx.swapper("&", "&amp;");
			if (tx[1] == FM) {
				tx.splicer(1, 1, "");
			}
			call htmllib("TABLE.MAKE", tx, tt, 1);
			//swap '<TR>' with '<TR VALIGN=TOP>' in tx

			//close superrow
			tx ^= "</TD></TR>";

			//tx:='<P>'
			call printtx(tx);

		}

	//jobexit:

		//close supertable?

		tx ^= "</TD></TR></TABLE>";
		tx ^= ownmark;

		call printtx(tx);

		call printtx(xx,"pagebreak");
		var newpage = 1;

	}//nextjob;

//exit:
	return 1;

}

subroutine sumnotcancelled(in bases, in statuses, out totalbase) {
	var nn = bases.count(VM) + (bases ne "");
	totalbase = "";
	for (var ii = 1; ii <= nn; ++ii) {
		if (statuses.a(1, ii) ne "CANCELLED") {
			var tt = bases.a(1, ii);
			if ((tt).length()) {
				//garbagecollect;
				totalbase = (totalbase + bases.a(1, ii)).oconv(fin.basefmt);
			}
		}
	};//ii;
	return;
}

subroutine printtxrow(io tx) {
	tx.swapper("&", "&amp;");
	tx.swapper(sep ^ sep, sep ^ nbsp ^ sep);
	tx.swapper(sep ^ sep, sep ^ nbsp ^ sep);
	tx.swapper(sep, "</" ^ tag ^ "><" ^ tag ^ ">");
	tx = "<TR " ^ rowattribs ^ "><" ^ tag ^ ">" ^ tx ^ "</" ^ tag ^ "></TR>";
	rowattribs = "";
	call printtx(tx);
	return;

}

libraryexit()
