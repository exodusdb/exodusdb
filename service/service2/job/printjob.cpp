#include <exodus/library.h>
libraryinit()

#include <getmark.h>
#include <authorised.h>
#include <agencysubs.h>
#include <ranges.h>
#include <quote2.h>
#include <safeselect.h>
#include <initcompany.h>
#include <validcode2.h>
#include <validcode3.h>
#include <timedate2.h>
#include <printtx.h>
#include <addcent.h>
#include <htmllib2.h>
#include <getreccount.h>
#include <select2.h>
#include <gethtml.h>
#include <getcss.h>
#include <docmods.h>

#include <fin.h>
#include <gen.h>
#include <agy.h>
#include <win.h>

var printptr;//num
var msg;
var compcode;
var xx;
var tt;
var reply;
var temp;
var styletx;
var classx;
var cols;
var coltx;
var style;
var basefmt0;
var clientmark;
var sep;
var tx;
var bases;
var statuses;
var totalbase;//num
var datax;
var txr;
var tag;
var rowattribs;
var avghourlyrate;
var newpage;//num
var aligned;
var topmargin;//num
var printfilename;
var html;
var printfile;
var letterhead;
var realpagen;//num
var printtxmark;
var rfmt;
var nbodylns;//num
var headx;
var newpagetag;
var css;

function main() {
	//
	//c job

	//global tx,cols,style,styletx,classx,sep,basefmt0,statuses,newpage,printptr
	//global bases

	//so we can call xxxxxx.SUBS
	//var().clearcommon();

	//pdf
	var repeatcolheads = PSEUDO.a(4).isnum();

	var interactive = not SYSTEM.a(33);
	var nbsp = "&nbsp;";

	var showcancelledlines = 0;

	var ownmark = "Production Management";
	call getmark("OWN", 1, ownmark);

	printptr = 0;
	var cssver = 2;

	if (not(authorised("JOB ACCESS", msg, ""))) {
		call mssg(msg);
		var().stop();
	}

	var timesheetparams;
	if (not(timesheetparams.read(DEFINITIONS, "TIMESHEET.PARAMS"))) {
		timesheetparams = "";
	}

	var reqsections = PSEUDO.a(3);
	//convert ':' to sm in reqsections

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
	if (not(reqsections.index("1"))) {
		accessbrief = 0;
	}
	if (not(reqsections.index("2"))) {
		accesscost = 0;
	}
	if (reqsections.index("3")) {
		if (not accesstime) {
			//personal time only
			accesstime = 2;
		}
	}else{
		accesstime = 0;
	}
	if (not(reqsections.index("4"))) {
		accessbill = 0;
	}

	/////
	//init:
	/////

	if (not(DICT.open("dict_JOBS"))) {
		call fsmsg();
		var().stop();
	}

	var jobversions;
	if (not(jobversions.open("JOB_VERSIONS", ""))) {
		call fsmsg();
		var().stop();
	}

	var jobdict = DICT;
	var timesheetdict;
	if (not(timesheetdict.open("dict_TIMESHEETS"))) {
		call fsmsg();
		var().stop();
	}
	win.datafile = "JOBS";

	//similar code in PRODINVS2 and PRINTJOB
	var jobnos = PSEUDO.a(2);
	if (not jobnos or jobnos.match("1A0A")) {
		//in case non-numeric job exist
		var job;
		if (not(job.read(agy.jobs, jobnos))) {
			if (jobnos.match("1A0A")) {
				compcode = jobnos;
			}else{
				compcode = gen.gcurrcompany;
			}
			//.1 means get previous ie last one
			call agencysubs("GETNEXTID." ^ compcode ^ ".1", xx);
			var tt;
			if (not(tt.read(agy.jobs, ID))) {
				call mssg("No documents have been created yet for company " ^ compcode ^ "| or company " ^ compcode ^ " doesnt exist");
				var().stop();
			}
			jobnos = ID;
		}
	}

	//jobnos=field(field(@sentence,'(',1),' ',2)
	if (jobnos) {
		call ranges(jobnos);
		if (not jobnos) {
			var().stop();
		}
	}
	if (not jobnos) {
		if (not LISTACTIVE) {

				/*;
				//jobnos=xlate('DEFINITIONS','JOBS.SK',1,'X')-1
				call job.subs('GETLASTJOBNO');
				jobnos=@ans;
				if jobnos else jobnos='';
	inpjob:
				if interactive then;
					call note2('Which job number(s) do you want ?||Eg "123" or "123 127" or "123 127 132"||(Press space then Enter to search)|','RCE',jobnos,'');
				end else;
					call msg('Job number is required but missing');
					stop;
					end;
				*/

			//similar code in PRODINVS2 and PRINTJOB
			if (not jobnos or jobnos.match("1A0A")) {
				if (jobnos.match("1A0A")) {
					compcode = jobnos;
				}else{
					compcode = gen.gcurrcompany;
				}
				//.1 means get previous ie last one
				call agencysubs("GETNEXTID." ^ compcode ^ ".1", xx);
				var tt;
				if (not(tt.read(agy.jobs, ID))) {
					call mssg("No documents have been created yet for company " ^ compcode);
					return 0;
				}
				jobnos = ID;
			}

			if (jobnos == 0x1B) {
				var().stop();
			}
			if (jobnos) {

				//convert range to list
				if (jobnos.match("1N0N\" \"1N0N")) {

					var fromno = jobnos.field(" ", 1);
					var tono = jobnos.field(" ", 2);

					if (fromno < tono - 1) {

						if (not(decide("", "All jobs from " ^ fromno ^ " to " ^ tono ^ "" _VM_ "Only jobs " ^ fromno ^ " and " ^ tono, reply))) {
							var().stop();
						}

						if (reply == 1) {

							//limit to 1000 max
							if (tono > fromno + 1000) {
								tono = fromno + 1000;
							}

							var jobnox = agy.agp.a(53);
							if (jobnox == "") {
								jobnox = "<NUMBER>";
							}
							temp = gen.company.a(28);
							if (not temp) {
								temp = gen.gcurrcompany;
							}
							jobnox.swapper("<COMPANY>", temp);

							jobnos = "";
							for (var jobno = fromno; jobno <= tono; ++jobno) {
							///BREAK;
							if (not(jobnos.length() < 32000)) break;;
								if (esctoexit()) {
									var().stop();
								}
								var jobnox2 = jobnox;
								jobnox2.swapper("<NUMBER>", jobno);
								jobnos ^= " " ^ jobnox2;
							};//jobno;

						}

					}

				}else{

					if (not(jobnos.index("-"))) {

						call safeselect("SSELECT JOBS WITH MASTER_JOB_NO " ^ quote2(jobnos) ^ " AND WITH AUTHORISED (S)");

						//error if cannot find any jobs
						if (not LISTACTIVE) {
							call mssg(DQ ^ (jobnos ^ DQ) ^ " - JOB NUMBER DOES NOT EXIST");
							var().stop();
						}

						var njobnos = jobnos.count(" ") + 1;
						if (RECCOUNT == njobnos) {
							jobnos = "";
						}else{
							call agencysubs("F2.JOBS", xx);
							jobnos = ANS;
						}

					}

				}

				jobnos.trimmer();

				//search by job index
			}else{
				call agencysubs("F2.JOBS", xx);
				if (not ANS) {
					var().stop();
				}
				jobnos = ANS;
			}

		}
	}

	var jobn = 0;
	jobnos.converter(", " ^ VM, FM ^ FM ^ FM);
	//call trim2(jobnos,fm)
	jobnos.converter(FM ^ " ", " " ^ FM);
	jobnos.trimmer();
	jobnos.converter(FM ^ " ", " " ^ FM);

	var njobs = jobnos.count(FM) + (jobnos ne "");

	styletx = "";

	classx = "HEAD1";
	cols = "L7,LB,L7,LB,L7,LB";
	gosub getcols();
	var head1cols = coltx;

	classx = "COST1";
	cols = "C,R,R,R,L,C,L,C";
	gosub getcols();
	var cost1cols = coltx;

	classx = "TIME1";
	cols = "C,C,R,C,L,R";
	if (accesstime < 2) {
		cols ^= ",R,R";
	}
	gosub getcols();
	var time1cols = coltx;

	classx = "BILL1";
	cols = "C,R,R,R,L,C,C,R,C";
	gosub getcols();
	var bill1cols = coltx;

	//add style for all table columns (using css nth-child method)
	style = "<style type=\"text/css\">";

	//tbody td version
	style.r(-1, styletx);

	//tbody th version
	styletx.swapper("> td:", "> th:");
	style.r(-1, FM ^ styletx);

	//thead th version
	styletx.swapper(" > tbody > ", " > thead > ");
	style.r(-1, FM ^ styletx);

	style.r(-1, "<style>");

	var personn = 1;

////////
nextjob:
////////
	//if interactive then call ossleep(1000*.2)
	if (esctoexit()) {
		gosub exit();
		var().stop();
	}
	if (jobnos) {
		jobn += 1;
		ID = jobnos.a(jobn);
		if (not ID) {
			gosub exit();
			var().stop();
		}
	}else{
		if (not(readnext(ID))) {
			gosub exit();
			var().stop();
		}
	}

	var isoldversion = 0;
	if (not(RECORD.read(agy.jobs, ID))) {
		if (not(RECORD.read(jobversions, ID))) {
			call mssg(DQ ^ (ID ^ DQ) ^ " - JOB NUMBER DOES NOT EXIST");
			goto nextjob;
		}
		isoldversion = 1;
	}
	MV = 0;

	if (RECORD.a(14) and RECORD.a(14) ne gen.gcurrcompany) {
		call initcompany(RECORD.a(14));
	}

	basefmt0 = fin.basefmt;
	basefmt0.converter("Z", "");

	call getmark("CLIENT", 1, clientmark);
	//clientmark='<table><tr><td>':clientmark:'</td></tr></table>'

	if (not(validcode2(calculate("COMPANY_CODE"), calculate("CLIENT_CODE"), calculate("BRAND_CODE"), agy.brands, msg))) {
unauth:
		call mssg("Job No:" ^ ID ^ "|" ^ msg);
		goto nextjob;
	}

	if (not(validcode3(calculate("MARKET_CODE"), "", "", xx, msg))) {
		goto unauth;
	}

	var htmltitle = "Job";
	var head = htmltitle;
	if (calculate("CLOSED") == "Y") {
		head ^= " (Closed)";
	} else if (calculate("CLOSED") == "N") {
		head ^= " (Reopened)";
	} else {
		head ^= " (Open)";
	}
	head = "<h2~style=\"margin:0px;text-align:center\">" ^ head ^ "</h2>";
	head ^= "<span style=\"acenter\">";
	head ^= "produced " ^ timedate2();
	head ^= FM ^ "</span>";
	head ^= FM ^ FM;

	if (njobs == 1) {
		htmltitle ^= " " ^ (DQ ^ (ID ^ DQ));
		htmltitle ^= " for " ^ calculate("CLIENT_NAME");
	}

	sep = VM;

	tx = head;
	tx.converter("~", " ");
	head = "";
	gosub printtx(tx);

	var tr = "<tr>";
	var trl = "<tr class=\"aleft\">";
	var trx = "</tr>";
	var td = "<td>";
	var tdx = "</td>";

	var nobr = "<span style=\"white-space:nowrap\">";
	var nobrx = "</span>";
	var bon = "<b>";
	var boff = "</b>";

	//super table used to reprint job header on continuation pages

	var trs = FM ^ "<TR>";
	var trxs = "</TR>" ^ FM;
	//TDS=FM:'<TD style="text-align:left;padding:0px">':FM
	var tds = "<TD style=\"text-align:left\">" ^ FM;
	var tdxs = FM ^ "</TD>";

	head.r(-1, "<table class=\"HEAD1\" style=\"width:100%\"");
	//head:=' cellspacing="0"';*ie7
	head ^= ">";

	head.r(-1, head1cols);
	head.r(-1, "<tbody>");

	var tab = "";

	//1st line

	tab.r(-1, tr ^ td ^ "Client:" ^ tdx ^ td ^ calculate("CLIENT_NAME") ^ tdx ^ td);

	var attention = calculate("ATTENTION");
	if (attention) {
		tab ^= nobr ^ "Attention:" ^ nobrx;
	}
	tab ^= tdx ^ td ^ bon ^ attention ^ boff ^ tdx;

	var jobnumber = calculate("NUMBER");
	if (not(RECORD.a(29))) {
		RECORD.r(29, "A");
	}
	if (isoldversion or (RECORD.a(29) > "A")) {
		jobnumber ^= " (rev " ^ RECORD.a(29) ^ ")";
	}
	if (isoldversion) {
		jobnumber ^= "*";
	}
	tab ^= td ^ nobr ^ "Job No:" ^ nobrx ^ tdx;
	tab ^= td ^ nobr ^ bon ^ jobnumber ^ boff ^ nobrx ^ tdx;
	tab ^= trx;

	//2nd line

	tab.r(-1, tr ^ td ^ "Brand:" ^ tdx);
	tab ^= td ^ calculate("FULL_BRAND_NAME") ^ tdx ^ td;

	//executive={EXECUTIVE_CODE}
	var executive = calculate("EXECUTIVE_NAME");
	if (executive) {
		tab ^= nobr ^ "Executive:" ^ nobrx;
	}
	tab ^= tdx ^ td ^ bon ^ executive ^ boff ^ tdx;

	//income=sum({QUOTE_AMOUNT_BASE})
	//cost=sum({ORDER_AMOUNT_BASE})

	MV = 0;

	bases = calculate("ORDER_AMOUNT_BASE");
	statuses = calculate("ORDER_STATUS2");
	gosub sumnotcancelled();
	var cost = totalbase;

	bases = calculate("QUOTE_AMOUNT_BASE");
	statuses = calculate("QUOTE_STATUS2");
	gosub sumnotcancelled();
	var income = totalbase;

	tab ^= td;

	if (accesscost and accessbill) {

		if (income) {
			tt = (income - cost) / income * 100;
			//garbagecollect;
			tt = tt.oconv("MD10P") ^ "%";
		}else{
			tt = "";
		}
		//tt={PERCENT_MARGIN2} 'MD10PZ'
		//garbagecollect
		//tt=margin 'MD10PZ'
		//if tt then tt:='%'
		if (income.length() or cost.length()) {
			//garbagecollect;
			tt ^= " " ^ nobr ^ bon ^ (income - cost).oconv(basefmt0) ^ agy.agp.a(2) ^ boff ^ nobrx;
		}

		tab ^= nobr ^ "Margin:" ^ nobrx;
		tab ^= tdx ^ td;
		tab ^= tt;

	}

	tab ^= tdx ^ trx;

	//3rd line

	tab.r(-1, tr ^ td ^ nobr ^ "Market-Period:" ^ nobrx ^ tdx);
	tab ^= td ^ calculate("MARKET_NAME") ^ " - " ^ addcent(calculate("PERIOD"), "", "", xx) ^ tdx ^ td;

	var prodtypedesc = calculate("PRODTYPE_DESC");
	if (prodtypedesc) {
		tab ^= nobr ^ "Type:" ^ nobrx;
	}
	tab ^= tdx ^ td ^ bon ^ prodtypedesc ^ boff ^ tdx ^ td;

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
		tab ^= nobr ^ "Markup:" ^ nobrx;
	}
	tab ^= tdx ^ td;
	if (accesscost and accessbill) {
		tab ^= bon ^ tt ^ boff;
	}

	tab ^= tdx ^ trx;

	//4th line

	var description = calculate("DESCRIPTION");

	//one line description row
	tab.r(-1, tr ^ td);
	var description1 = description.a(1, 1);
	//the first column is the title only if any description exists
	if (description1) {
		tab ^= nobr ^ "Description:" ^ nobrx;
	}
	//description1 fills the remaining 5 columns
	tab ^= tdx ^ "<td colspan=5>" ^ bon ^ description1 ^ boff ^ tdx;
	tab ^= trx;

	tab.swapper("&", "&amp;");
	tab.swapper("<tr>", "<tr style=\"vertical-align:top\">");

	tab.r(-1, "</tbody>");
	tab.r(-1, "</table>");

	tab.swapper("\'", "\'\'");
	head.r(-1, tab);

	//supertable heading
	tx.r(-1, "<TABLE>");
	if (repeatcolheads) {
		tx.r(-1, "<THEAD>");
	}
	tx.r(-1, trs ^ tds);
	tx.r(-1, head);
	head = "";

	tx.r(-1, tdxs ^ trxs);
	if (repeatcolheads) {
		tx.r(-1, "</THEAD>");
		tx.r(-1, "<TBODY>");
	}
	gosub printtx(tx);

	var foot = "";
	var bottomline = "";

	sep = VM;

	//5th line
	if (accessbrief) {
		var brief = description.field(VM, 2, 9999);
		if (brief) {

			//gap superrow
			tx.r(-1, trs ^ tds ^ tdxs ^ trxs);

			//open superrow
			tx.r(-1, trs ^ tds);

			//put brief in a table
			brief.swapper(VM, "<br/>" "\r\n");
			tx.r(-1, "<table class=\"aleft\">");
			tx.r(-1, tr ^ "<td style=\"vertical-align:top\">Brief:" ^ tdx ^ td ^ brief ^ tdx ^ trx);
			tx.r(-1, "</table>");

			//close superrow
			tx.r(-1, tdxs ^ trxs);

			gosub printtx(tx);

		}
	}

	if (accesscost and RECORD.a(4)) {

		//gap superrow
		tx.r(-1, trs ^ tds ^ tdxs ^ trxs);

		//open superrow
		tx.r(-1, trs ^ tds);

		//tx<-1>=clientmark
		tx ^= FM ^ clientmark;
		clientmark = "";

		datax = "";
		datax ^= "Order No." ^ sep;
		datax ^= "Date" ^ sep;
		datax ^= "Amount" ^ sep;
		datax ^= agy.agp.a(2) ^ " Equiv." ^ sep;
		datax ^= "Description" ^ sep;
		datax ^= "Suppl.Inv" ^ sep;
		datax ^= "Supplier" ^ sep;
		datax ^= "Status";

		datax ^= FM;

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
					datax ^= FM;
				}
				datax ^= calculate("ORDER_NO") ^ sep;
				datax ^= calculate("ORDER_DATE").oconv("[DATE,*4]" _VM_ "") ^ sep;
				datax ^= calculate("ORDER_AMOUNT") ^ calculate("ORDER_CURRENCY") ^ sep;
				var orderamountbase = calculate("ORDER_AMOUNT_BASE");
				datax ^= orderamountbase ^ sep;
				if (not cancelled) {
					totalbase += orderamountbase;
				}
				datax ^= calculate("ORDER_DESC") ^ sep;
				datax ^= orderinvno ^ sep;
				datax ^= calculate("SUPPLIER_NAME") ^ sep;
				datax ^= capitalise(status);
			}
		};//MV;

		if (totalbase) {
			datax ^= FM ^ "Total" ^ sep ^ sep ^ sep ^ bon ^ totalbase.oconv(basefmt0) ^ boff ^ sep ^ sep ^ sep ^ sep;
		}

		tt = "";
		tt ^= "<table class=\"neosystable COST1\"";
		//tt:=' cellspacing="0"';*ie7
		tt ^= " style=\"width:100%\"";
		tt ^= ">";

		tt.r(-1, cost1cols);

		datax.swapper("&", "&amp;");
		call htmllib2("TABLE.MAKE", datax, tt, 1);

		if (not repeatcolheads) {
			gosub removetheadtbody();
		}

		tx.r(-1, datax);

		//close superrow
		tx.r(-1, tdxs ^ trxs);

		gosub printtx(tx);

	}

	if (accesstime and getreccount(gen.timesheets, "", "")) {

		//get relevent timesheet lines by department person and date
		//better be under 64kb!!!
		var dictids = "DEPARTMENT2 PERSON_NAME DATE ACTIVITY_NAME DETAILS HOURS";
		//not personal
		if (accesstime == 1) {
			dictids ^= " RATE AMOUNT";
		}
		var filename = "TIMESHEETS";
		var linkfilename2 = "";
		var sortselect = "WITH JOB_NO " ^ (DQ ^ (ID ^ DQ)) ^ " BY JOB_NO BY DEPARTMENT BY PERSON_CODE BY DATE";
		if (timesheetparams.a(8)) {
			sortselect ^= " AND WITH DATE GE " ^ (DQ ^ (timesheetparams.a(8).oconv("[DATE,*]") ^ DQ));
		}
		sortselect ^= " AND WITH AUTHORISED1";
		//personal time only
		if (accesstime ne 1) {
			sortselect ^= " AND WITH PERSON_CODE " ^ (DQ ^ ("NABEEHA" ^ DQ));
		}
		var options = "";
		datax = "";
		var response = "";
		call select2(filename, linkfilename2, sortselect, dictids, options, datax, response, "JOB_NO" _VM_ "HOURS" _VM_ "HOURS", "EQ" _VM_ "NE" _VM_ "NE", ID ^ "" _VM_ "0" _VM_ "");

		if (datax) {

			//gap superrow
			tx.r(-1, trs ^ tds ^ tdxs ^ trxs);

			//open superrow
			tx.r(-1, trs ^ tds);

			tx.r(-1, clientmark);
			clientmark = "";

			tx.r(-1, "<table class=\"neosystable TIME1\"");
			//tx:=' cellspacing="0"';*ie7
			tx ^= " style=\"width:100%\">";

			tx.r(-1, time1cols);

			if (repeatcolheads) {
				tx.r(-1, "<thead>");
			}

			gosub printtx(tx);

			txr = "Department" ^ sep;
			txr ^= "Person" ^ sep;
			txr ^= "Date" ^ sep;
			//txr:='Code':sep
			txr ^= "Activity" ^ sep;
			txr ^= "Details" ^ sep;
			txr ^= "Hours" ^ sep;
			//not personal
			if (accesstime < 2) {
				txr ^= "Rate" ^ sep;
				txr ^= "Amount" ^ sep;
			}
			txr.splicer(-1, 1, "");

			tag = "th";
			rowattribs = "";
			gosub printtxr();

			if (repeatcolheads) {
				tx.r(-1, "</thead>");
				tx.r(-1, "<tbody>");
			}

			gosub printtx(tx);

			var personcoln = 2;
			var amountcoln = 8;
			var ratecoln = 7;
			var hourscoln = 6;

			//personal show lines immediately
			var styledisplay = "display:none";
			if (accesstime ne 1) {
				styledisplay = "";
			}

			var nlines = datax.count(FM) + 1;
			var totalamount = 0;
			var totalhours = 0;
			var personamount = 0;
			var personhours = 0;
			//move up so persons dont repeat if multiple jobs printed drill clicked
			//personn=1
			var personcode = datax.a(1, personcoln);
			for (var ln = 1; ln <= nlines; ++ln) {

				txr = datax.a(ln);
				totalamount += txr.a(1, amountcoln);
				personamount += txr.a(1, amountcoln);
				var hours = txr.a(1, hourscoln);
				if (hours) {
					//MT2 is a GBP PROGRAM!
					personhours += txr.a(1, hourscoln).iconv("MT2");
					totalhours += txr.a(1, hourscoln).iconv("MT2");
					txr.swapper(VM, sep);

					tag = "td";
					//id for pre ie8 compatibility without getElementsByClassName
					//rowattribs='id="person':personn:'" class="person':personn:'" style="cursor:pointer; ':styledisplay:'" onclick="toggle(':"'":'person':personn:"'":')"'
					rowattribs = "class=\"person" ^ personn ^ "\" style=\"cursor:pointer; " ^ styledisplay ^ "\" onclick=\"toggle(" "\'" "person" ^ personn ^ "\'" ")\"";
					gosub printtxr();

				}

				//person subtotal
				if (datax.a(ln + 1, personcoln) ne personcode) {
					if (personhours or personamount) {
						if (personhours) {
							avghourlyrate = (personamount / personhours).oconv("MD20PZ");
						}else{
							avghourlyrate = "";
						}
						txr = datax.a(ln, 1) ^ sep ^ capitalise(personcode) ^ sep ^ sep ^ sep ^ sep ^ personhours.oconv("MT2");
						//not personal
						if (accesstime < 2) {
							txr ^= sep ^ avghourlyrate ^ sep ^ bon ^ personamount.oconv(basefmt0) ^ boff;
						}

						tag = "th";
						//rowattribs='style="cursor:pointer" onclick="toggle(person':personn:')"'
						rowattribs = "style=\"cursor:pointer\" onclick=\"toggle(" "\'" "person" ^ personn ^ "\'" ")\"";
						gosub printtxr();

						personhours = "";
						personamount = "";
					}
					personcode = datax.a(ln + 1, personcoln);
					personn += 1;
				}

			};//ln;

			if (totalhours or totalamount) {
				if (totalhours) {
					avghourlyrate = (totalamount / totalhours).oconv("MD20PZ");
				}else{
					avghourlyrate = "";
				}
				txr = "Total" ^ sep ^ sep ^ sep ^ sep ^ sep ^ totalhours.oconv("MT2");
				//not personal
				if (accesstime < 2) {
					txr ^= sep ^ avghourlyrate ^ sep ^ bon ^ totalamount.oconv(basefmt0) ^ boff;
				}

				tag = "th";
				rowattribs = "";
				gosub printtxr();

			}

			tx.r(-1, "</tbody>" ^ FM ^ "</table>");

			//close superrow
			tx.r(-1, tdxs ^ trxs);

			gosub printtx(tx);

		}

	}

	if (accessbill and RECORD.a(10)) {

		//gap superrow
		tx.r(-1, trs ^ tds ^ tdxs ^ trxs);

		//open superrow
		tx.r(-1, trs ^ tds ^ clientmark);
		clientmark = "";

		gosub printtx(tx);

		datax = "";
		datax.r(-1, "Estimate No." ^ sep);
		datax ^= "Date" ^ sep;
		datax ^= "Amount" ^ sep;
		datax ^= agy.agp.a(2) ^ " Equiv." ^ sep;
		datax ^= "Description" ^ sep;
		datax ^= "Our Invoice No." ^ sep;
		datax ^= "Receipt No." ^ sep;
		datax ^= "Amount" ^ sep;
		datax ^= "Status";

		datax ^= FM;

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
					datax ^= FM;
				}
				datax ^= calculate("QUOTE_NO") ^ sep;
				//fix bug in datax datax:=oconv({QUOTE_DATE},'[DATE,*]') 'L#12':' '
				datax ^= calculate("QUOTE_DATE").a(1, 1).oconv("[DATE,*]") ^ sep;
				datax ^= calculate("QUOTE_AMOUNT") ^ calculate("QUOTE_CURRENCY") ^ sep;
				//datax:=({QUOTE_AMOUNT_BASE}):sep
				var quoteamountbase = calculate("QUOTE_AMOUNT_BASE");
				datax ^= quoteamountbase ^ sep;
				if (not cancelled) {
					totalbase += quoteamountbase;
				}

				datax ^= calculate("QUOTE_DESC") ^ sep;
				datax ^= calculate("QUOTE_INV_NO");

				var invno = invnos.a(1, MV);
				if (invno == "") {
noreceipt:
					datax ^= VM ^ VM;
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
							datax ^= FM ^ VM.str(6);
						}else{
							datax ^= sep;
						}
						datax ^= receipts.a(1, 1, receiptn).field("*", 1, 2) ^ sep;
						datax ^= allocamts.a(1, 1, receiptn).oconv("[NUMBER]");

					};//receiptn;
				}

				datax ^= sep ^ capitalise(status);

			}

		};//MV;

		if (totalbase) {
			datax ^= FM ^ "Total" ^ sep ^ sep ^ sep ^ bon ^ totalbase.oconv(basefmt0) ^ boff ^ sep ^ sep ^ sep ^ sep ^ sep;
		}

		tt = "";
		tt = FM ^ "<table class=\"neosystable BILL1\"";
		//tt:=' cellspacing="0"';*ie7
		tt ^= " style=\"width:100%\"";
		tt ^= ">";

		tt.r(-1, bill1cols);

		datax.swapper("&", "&amp;");
		if (datax[1] == FM) {
			datax.splicer(1, 1, "");
		}
		call htmllib2("TABLE.MAKE", datax, tt, 1);

		if (not repeatcolheads) {
			gosub removetheadtbody();
		}

		tx.r(-1, datax);

		//close superrow
		tx.r(-1, tdxs ^ trxs);

		gosub printtx(tx);

	}

	////////
	//jobexit:
	////////

	//open superrow
	tx.r(-1, trs ^ tds);

	tx.r(-1, ownmark);

	//close superrow
	tx.r(-1, tdxs ^ trxs);

	//close supertable
	tx.r(-1, "</TBODY>" ^ FM ^ "</TABLE>");

	gosub printtx(tx);

	gosub printtx(xx,"pagebreak");
	newpage = 1;

	goto nextjob;

	return "";
}

subroutine exit() {

	if (printptr) {

		//close whole document div
		tx.r(-1, "</div>");
		tx.r(-1, "</html>");

		newpage = 0;
		gosub printtx(tx);

	}

	var().stop();

}

subroutine sumnotcancelled() {
	var nn = bases.count(VM) + (bases ne "");
	totalbase = "";
	for (var ii = 1; ii <= nn; ++ii) {
		if (statuses.a(1, ii) ne "CANCELLED") {
			tt = bases.a(1, ii);
			if (tt.length()) {
				//garbagecollect;
				totalbase = (totalbase + bases.a(1, ii)).oconv(basefmt0);
			}
		}
	};//ii;
	return;

}

subroutine printtxr() {
	txr.swapper("&", "&amp;");
	txr.swapper(sep, "</" ^ tag ^ "><" ^ tag ^ ">");

	tx.r(-1, "<tr");
	if (rowattribs) {
		tx ^= " " ^ rowattribs;
		rowattribs = "";
	}
	tx ^= "><" ^ tag ^ ">" ^ txr ^ "</" ^ tag ^ "></tr>";
	rowattribs = "";

	gosub printtx(tx);

	return;

}

subroutine getcols() {
	coltx = "<colgroup>";

	var ncols = cols.count(",") + 1;
	for (var scoln = 1; scoln <= ncols; ++scoln) {
		var fmt = cols.field(",", scoln);

		if (fmt.index("L")) {
			aligned = "left";
		} else if (fmt.index("R")) {
			aligned = "right";
		} else {
			aligned = "center";
		}
		style = "text-align:" ^ aligned;
		if (fmt.index("7")) {
			style ^= ";width:7%";
		}
		if (fmt.index("B")) {
			style ^= ";font-weight:bold;";
		}
		coltx.r(-1, "<col style=\"" ^ style ^ "\">");

		//<col> on mozilla doesnt support text-align but does support nth-child
		if (scoln == 1) {
			styletx ^= FM;
		}
		//styletx<-1>='table#':id:' > tbody > tr > td:nth-child(':scoln:'){':style:'}'
		styletx.r(-1, "." ^ classx ^ " > tbody > tr > td:nth-child(" ^ scoln ^ "){" ^ style ^ "}");

	};//scoln;

	coltx ^= FM ^ "</colgroup>";
	return;

}

subroutine removetheadtbody() {
	datax.swapper("<thead>", "");
	datax.swapper("</thead>", "");
	datax.swapper("<tbody>", "");
	datax.swapper("</tbody>", "");
	return;

}


libraryexit()
