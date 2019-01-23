#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <timedate2.h>
#include <quote2.h>
#include <xselect.h>
#include <convcsv.h>
#include <getmark.h>
#include <printtx.h>
#include <gethtml.h>
#include <getcss.h>
#include <docmods.h>

#include <gen.h>

var tx;
var msg;
var html;//num
var r15;
var l20;
var l40;
var sep;
var tr;
var trx;
var todate;
var totalcost;//num
var bar;
var newjobno;
var usern;
var printptr;//num
var printfilename;
var printfile;
var letterhead;
var realpagen;//num
var newpage;//num
var bottomline;
var printtxmark;
var rfmt;
var foot;
var nbodylns;//num
var headx;
var newpagetag;
var css;
var cssver;
var style;
var htmltitle;

function main() {
	//LIST TIMESHEETS BY JOB_NO BY ACTIVITY_CODE BREAK-ON JOB_NO BREAK-ON ACTIVITY_NAME TOTAL AMOUNT DET-SUPP ID-SUPP
	//c tim
	var byactivity = 0;
	//global html,tx,bar,totalcost

	//printer off
	var topmargin = 0;
	tx = "";
	var interactive = not SYSTEM.a(33);

	if (not(authorised("TIMESHEET ANALYSIS", msg, ""))) {
		call mssg(msg);
		var().stop();
	}

	var jobs;
	if (not(jobs.open("JOBS", ""))) {
		call fsmsg();
		var().stop();
	}

	var timesheetparams;
	if (not(timesheetparams.read(DEFINITIONS, "TIMESHEET.PARAMS"))) {
		timesheetparams = "";
	}

	html = 1;

	if (html) {
		var r5 = "[TAGHTML,TD]";
		r15 = "[TAGHTML,TD ALIGN=RIGHT]";
		l20 = "[TAGHTML,TD]";
		l40 = "[TAGHTML,TD]";
		sep = "";
		tr = "<TR>";
		var trr = "<TR ALIGN=RIGHT>";
		trx = "</TR>";
	}else{
		var r5 = "R#5";
		r15 = "R#15";
		l20 = "L#20";
		l40 = "L#40";
		sep = " ";
		tr = "";
		var trr = "";
		trx = "";
	}

	if (interactive) {
		PSEUDO = "";
	}
	var ifromdate = PSEUDO.a(30);
	var itodate = PSEUDO.a(31);
	var fileformat = PSEUDO.a(1);
	fileformat = "xls";
	var reqcompanycodes = PSEUDO.a(22);

	/*;
		YEAR.PERIODS=@PSEUDO<29>;
		companycodes=@pseudo<22>;
		CLIENTCODES=@PSEUDO<23>;
		BRANDCODES=@PSEUDO<24>;
		suppliercodes=@pseudo<26>;
		marketcodes=@pseudo<27>;
		EXECUTIVECODES=@PSEUDO<41>;

	*/

	var reqclientcodes = PSEUDO.a(23);
	var reqactivitycodes = PSEUDO.a(45);
	var requsercodes = PSEUDO.a(52);

	if (interactive) {

		var fromdate = (("1/" ^ var().date().oconv("D2/E").substr(4,9)).iconv("D2/E")).oconv("[DATE,*]");
inpfromdate:
		call note("Starting at what date ?", "RC", fromdate, "");
		if (not fromdate) {
			var().stop();
		}
		ifromdate = fromdate.iconv("[DATE,*4]");
		if (not ifromdate) {
			call mssg(fromdate ^ "  is not a valid date");
			goto inpfromdate;
		}

		for (var dom = 31; dom >= 28; --dom) {
			todate = dom ^ "/" ^ ifromdate.oconv("D2/E").substr(4,9);
			todate = todate.iconv("D2/E");
			todate = todate.oconv("[DATE,*4]");
		///BREAK;
		if (todate) break;;
		};//dom;
inptodate:
		call note("Ending at what date ?", "RC", todate, "");
		if (not todate) {
			var().stop();
		}
		itodate = todate.iconv("[DATE,*4]");
		if (not itodate) {
			call mssg(todate ^ "  is not a valid date");
			goto inptodate;
		}

		//call msg(todate 'D2/E')

	}

	if (not(DICT.open("dict_TIMESHEETS"))) {
		call fsmsg();
		var().stop();
	}

	var timerates = SECURITY;
	var temp = timerates.a(5);
	temp.converter(VM, "");
	if (temp == "") {
		timerates.r(5, "");
	}

	//////////
	//initbreak:
	//////////
	//page heading
	var head = "";

	if (html) {
		head ^= "<H2~style=\"margin:0px;text-align:center\">";
	}
	head ^= "TIMESHEET ANALYSIS";
	if (html) {
		head ^= "</H2>";
	}

	head ^= FM;

	if (not html) {
		head ^= FM;
	}

	if (html) {
		head ^= "<H4>";
	}
	head ^= var(3).space() ^ "produced " ^ timedate2();
	if (html) {
		head ^= "</H4>";
	}

	var cmd = "SELECT TIMESHEETS";

	if (ifromdate) {
		if (timesheetparams.a(8) and (ifromdate < timesheetparams.a(8))) {
			call mssg("Timesheet analysis is only available from " ^ timesheetparams.a(8).oconv("[DATE,*4]"));
			var().stop();
		}
		cmd ^= " AND WITH DATE BETWEEN " ^ (DQ ^ (ifromdate.oconv("[DATE,4*]") ^ DQ)) ^ " AND " ^ (DQ ^ (itodate.oconv("[DATE,4*]") ^ DQ));
		if (html) {
			head ^= "<H3>";
		}
		head ^= FM ^ "For the period " ^ ifromdate.oconv("[DATE,*4]") ^ " to " ^ itodate.oconv("[DATE,*4]") ^ FM;
		if (html) {
			head ^= "</H3>";
		}
	}

	if (requsercodes) {
		cmd ^= " AND WITH PERSON_CODE " ^ quote2(requsercodes);
	}

	if (reqcompanycodes) {
		if (html) {
			head ^= "<H3>";
		}
		head ^= FM ^ "Company " ^ swap(VM, ", ", reqcompanycodes.xlate("COMPANIES", 1, "C"));
		if (html) {
			head ^= "</H3>";
		}
	}

	/*;
		IF BRANDCODES THEN;
			head:='Brand     : ':BRANDCODES:"'L'";
			CMD:=' AND WITH BRAND_CODE ':QUOTE2(BRANDCODES);
			END;

		IF executivecodes THEN;
			head:='Executive     : ':executivecodes:"'L'";
			CMD:=' AND WITH EXECUTIVE_CODE ':QUOTE2(executivecodes);
			END;

		if companycodes then;
			head:='Company    : ':companycodes:"'L'";
			CMD:=' AND WITH COMPANY_CODE ':QUOTE2(companycodes);
			end;

		if suppliercodes then;
			head:='Supplier  : ':suppliercodes:"'L'";
			CMD:=' AND WITH SUPPLIER_CODE ':QUOTE2(suppliercodes);
			end;

		if marketcodes then;
			head:='Market    : ':marketcodes:"'L'";
			CMD:=' AND WITH MARKET_CODE ':QUOTE2(marketcodes);
			end;

		if reqclientcodes then;
			head:='Client    : ':reqclientcodes:"'L'";
			SWAP vm WITH '" "' IN reqclientcodes;
			CMD:=' AND WITH CLIENT_CODE ':QUOTE(reqclientcodes);
			END;

		cmd:=' AND WITH AUTHORISED';

	*/

	//remove the first AND
	temp = cmd.index("AND WITH");
	if (temp) {
		cmd.splicer(temp, 4, "");
	}

	//cause multivalued sort because each job can have a different company
	//and authorisation is per company too
	cmd ^= " BY JOB_NO";

	if (not interactive) {
		cmd ^= " (S)";
	}

	if (fileformat) {

		call xselect(cmd);
		if (not LISTACTIVE) {
			call mssg("No records found");
			var().stop();
		}
		cmd = "";

		//annoyingly cannot seem to filter multivalues in arev select
		//so do it per multivalue (similar to LIMIT clause in NLIST)

		var filters = "";

		var nfilters = 1;
		filters.r(1, nfilters, "HOURS");
		filters.r(3, nfilters, "");

		if (reqactivitycodes) {
			nfilters += 1;
			filters.r(1, nfilters, "ACTIVITY_CODE");
			filters.r(3, nfilters, reqactivitycodes.convert(VM, SVM));
		}

		if (reqcompanycodes) {
			nfilters += 1;
			//look up on jobs
			filters.r(1, nfilters, "COMPANY_CODE");
			filters.r(3, nfilters, reqcompanycodes.convert(VM, SVM));
		}

		if (reqclientcodes) {
			nfilters += 1;
			//look up brand on jobs, then client on brands
			filters.r(1, nfilters, "CLIENT_CODE");
			filters.r(3, nfilters, reqclientcodes.convert(VM, SVM));
		}

		//change output file to desired extension
		var sys2 = SYSTEM.a(2);
		sys2.splicer(-3, 3, fileformat);
		SYSTEM.r(2, sys2);

		call convcsv("SELECT TIMESHEETS", "", filters);

		var().stop();
	}

	if (not interactive) {
		cmd ^= " (S)";
	}
	perform(cmd);
	if (not LISTACTIVE) {
		call mssg("No timesheets found");
		var().stop();
	}

	var key = "";
	totalcost = "";

	if (not html) {
		head ^= FM;
	}

	var clientmark = SYSTEM.a(14);

	//col heading
	var xx = "";
	if (html) {

		//clientwebsite=system<8>
		//t2='<P STYLE="MARGIN-BOTTOM:0" ALIGN=CENTER><small>':clientmark:'</small>'
		//if clientwebsite then t2='<A HREF=':quote(clientwebsite):'>':t2:'</A>'
		//x:=t2:'<BR>'
		call getmark("CLIENT", html, clientmark);
		xx ^= "<table><tr><td>" ^ clientmark ^ "</td></tr></table>";

		xx ^= "<TABLE BGCOLOR=#FFFFC0 BORDER=1 CELLSPACING=0 CELLPADDING=2";
		xx ^= " class=neosystable";
		xx ^= " ALIGN=CENTER";
		xx ^= " STYLE=\"{font-size:66%}\"";
		xx ^= "><THEAD>";
		head ^= xx;
	}

	//if 1 then
	temp = sep ^ var("Job").oconv(l20);
	//end else
	// temp=sep:'Person' l20
	// end
	temp ^= sep ^ var("Description").oconv(l40);
	if (byactivity) {
		temp ^= sep ^ var("Activity").oconv(l40);
	}
	temp ^= " " ^ var("Cost").oconv(r15);
	temp.swapper("<TD", "<TH");
	temp.swapper("</TD", "</TH");
	bar = var("-").str(temp.length());
	if (not html) {
		head ^= bar ^ FM;
	}
	head ^= tr ^ temp ^ trx ^ FM;
	if (not html) {
		head ^= bar ^ FM;
	}

	gosub getrec();

//////////
nextbreak:
//////////
	if (ID == "") {
		gosub exit();
		var().stop();
	}

	//////////
	//breakinit:
	//////////
	var cost = "";
	var jobno = newjobno;
	var job;
	if (not(job.read(jobs, jobno))) {
		job = "";
	}

	var activitycodes = "";
	while (true) {
	///BREAK;
	if (not((newjobno == jobno) and ID)) break;;
		var hours = RECORD.a(2, MV);
		var activity = RECORD.a(4);
		var username = ID.field("*", 1);
		if (not(timerates.a(1).locateusing(username, VM, usern))) {
			{}
		}
		var rate = timerates.a(5, usern);
		//if rate='' and timerates<5> then
		if (rate == "") {
			rate = 1;
			timerates.r(5, usern, rate);
			//tx<-1>='NO HOURLY RATE FOR USER ':quote(username):' - 1.00 USED'
			gosub printtx(tx);
		}
		cost += hours * rate;
		gosub getrec();
	}//loop;

	//////////
	//breakexit:
	//////////
	if (cost) {
		tx = tr ^ (sep ^ jobno).oconv(l20) ^ " " ^ job.a(9, 1, 1).oconv(l40) ^ " " ^ cost.oconv("MD20P").oconv(r15) ^ trx;
		gosub printtx(tx);
		totalcost += cost;
	}

	goto nextbreak;

	return 0;

	return "";
}

subroutine exit() {
	if (not html) {
		tx.r(-1, bar);
	}
	tx ^= FM;

	var tx2 = tr ^ (sep ^ "TOTAL COST").oconv(l20) ^ sep ^ var("").oconv(l40) ^ sep ^ totalcost.oconv("MD20P").oconv(r15) ^ FM;

	if (html) {
		tx2.swapper("<TD", "<TH");
		tx2.swapper("</TD", "</TH");
		tx ^= tx2;
		tx ^= "</TBODY></TABLE>";
	}else{
		tx ^= tx2;
		tx ^= bar;
	}

	tx ^= FM;

	var mark = "Timesheet";
	call getmark("OWN", html, mark);
	if (not html) {
		tx ^= FM;
	}
	tx ^= "<table><tr><td>" ^ mark ^ "</td></tr></table>";

	gosub printtx(tx);
	return;

}

subroutine getrec() {

	//null to help c++ decompiler
	{}

nextrec:
////////
	if (esctoexit()) {
		var().stop();
	}
	if (not(readnext(ID, MV))) {
		ID = "";
		newjobno = "";
		return;
	}
	if (not(RECORD.read(gen.timesheets, ID))) {
		goto nextrec;
	}

	newjobno = RECORD.a(1, MV);
	return;

}


libraryexit()
