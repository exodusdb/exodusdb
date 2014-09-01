#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <getmark.h>
#include <gethtml.h>
#include <readcss.h>
#include <printtx.h>
#include <tag.h>
#include <quote2.h>

#include <gen.h>
#include <agy.h>

var msg;
//var r15;
//var l20;
//var l40;
var tdr;
var td;
var sep;
var tr;
var trx;
var todate;
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
var style;

function main() {
	//LIST TIMESHEETS BY JOB_NO BY ACTIVITY_CODE BREAK-ON JOB_NO BREAK-ON ACTIVITY_NAME TOTAL AMOUNT DET-SUPP ID-SUPP
	var byactivity = 0;
	var topmargin = 0;
	var tx = "";

	if (not(authorised("TIMESHEET ANALYSIS", msg, ""))) {
		call mssg(msg);
		return 0;
	}

	//var r5 = "[TAGHTML,TD]";
	//r15 = "[TAGHTML,TD ALIGN=RIGHT]";
	//l20 = "[TAGHTML,TD]";
	//l40 = "[TAGHTML,TD]";
	tdr="td aligh=right";
	td="td";
	sep = "";
	tr = "<tr>";
	var trr = "<tr align=right>";
	trx = "</tr>";


	var ifromdate = PSEUDO.a(30);
	var itodate = PSEUDO.a(31);
	//var fileformat = PSEUDO.a(1);
	//no html for now
	var fileformat = "xls";
	var reqcompcodes = PSEUDO.a(22);

	/*;
		YEAR.PERIODS=@PSEUDO<29>;
		companycodes=@pseudo<22>;
		CLIENTCODES=@PSEUDO<23>;
		BRANDCODES=@PSEUDO<24>;
		suppliercodes=@pseudo<26>;
		marketcodes=@pseudo<27>;
		EXECUTIVECODES=@PSEUDO<41>;
	*/
	var usercodes = PSEUDO.a(52);

	if (not(DICT.open("dict_TIMESHEETS"))) {
		return fsmsg();
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

	head ^= "<H2~style=\"margin:0px;text-align:center\">";

	head ^= "TIMESHEET ANALYSIS";
	head ^= "</H2>";

	head ^= FM;

	head ^= "<H4>";

	head ^= var(3).space() ^ "produced " ^ timedate();
	head ^= "</H4>";

	var cmd = "SELECT TIMESHEETS";

	if (ifromdate) {
		cmd ^= " AND WITH DATE BETWEEN " ^ (DQ ^ (ifromdate.oconv("[DATE,4*]") ^ DQ)) ^ " AND " ^ (DQ ^ (itodate.oconv("[DATE,4*]") ^ DQ));
		head ^= "<H3>";
		head ^= FM ^ "For the period " ^ ifromdate.oconv("[DATE,*4]") ^ " to " ^ itodate.oconv("[DATE,*4]") ^ FM;
		head ^= "</H3>";
	}

	if (usercodes) {
		cmd ^= " AND WITH PERSON_CODE " ^ quote2(usercodes);
	}

	//this should be filtered per line
	//develop exodus to explode sort if filter with on mv dictionary unless invent new WITH ANY type of syntax
	if (reqcompcodes) {
		cmd ^= " AND WITH COMPANY_CODE " ^ quote2(reqcompcodes);
		head ^= "<H3>";
		head ^= FM ^ "Company: " ^ quote(xlate("COMPANIES",reqcompcodes,1,"C")) ^ FM;
		head ^= "</H3>";
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

		if clientcodes then;
			head:='Client    : ':CLIENTCODES:"'L'";
			SWAP vm WITH '" "' IN CLIENTCODES;
			CMD:=' AND WITH CLIENT_CODE ':QUOTE2(CLIENTCODES);
			END;

	*/

	//always explode by job_no in order to check company code and authorisation per job
	cmd ^= " BY JOB_NO";

	//timesheet authorised should be mv depending on job company and brand
	//TODO! cmd^=" AND WITH AUTHORISED \"1\"";

	//remove the first AND
	temp = cmd.index("AND WITH", 1);
	if (temp) {
		cmd.splicer(temp, 4, "");
	}

	//cmd ^= " (S)";

	if (fileformat) {
		var sys2 = SYSTEM.a(2);
		sys2.splicer(-3, 3, fileformat);
		SYSTEM.r(2, sys2);
		perform("convcsv TIMESHEETS SELECT " ^ cmd.field(" ", 3, 9999));
		return 1;//ok
	}

/*
	//cmd ^= " (S)";

	perform(cmd);
	if (not LISTACTIVE) {
		call mssg("No timesheets found");
		return 0;
	}

	var key = "";
	var totalcost = "";

	var clientmark = SYSTEM.a(14);

	//col heading
	var xx = "";

	var clientwebsite = SYSTEM.a(8);
	var t2 = "<P STYLE=\"MARGIN-BOTTOM:0\" ALIGN=CENTER><SMALL>" ^ clientmark ^ "</SMALL>";
	if (clientwebsite) {
		t2 = "<A HREF=" ^ (DQ ^ (clientwebsite ^ DQ)) ^ ">" ^ t2 ^ "</A>";
	}
	xx ^= t2 ^ "<BR>";

	xx ^= "<TABLE BGCOLOR=#FFFFC0 BORDER=1 CELLSPACING=0 CELLPADDING=2";
	xx ^= " class=neosystable";
	xx ^= " ALIGN=CENTER";
	xx ^= " STYLE=\"{font-size:66%}\"";
	xx ^= "><THEAD>";
	head ^= xx;

	if (1) {
		temp = sep ^ tag(td, "Job");
	}else{
		temp = sep ^ tag(td, "Person");
	}
	temp ^= sep ^ tag(td, "Description");
	if (byactivity) {
		temp ^= sep ^ tag(td, "Activity");
	}
	temp ^= sep ^ tag(tdr, "Cost");
	temp.swapper("<td", "<th");
	temp.swapper("</td", "</th");
	var bar = var("-").str(temp.length());
	head ^= tr ^ temp ^ trx ^ FM;

	call printtx(tx, "head", head);

	gosub getrec();

//////////
//nextbreak:
//////////
	while (ID ne "") {

//////////
//breakinit:
//////////
		var cost = "";
		var jobno = newjobno;
		var job;
		if (not(job.read(agy.jobs, jobno))) {
			job = "";
		}

		var activitycodes = "";
		while (true) {

			if (esctoexit()) {
				return 0;
			}

			if (not(newjobno == jobno and ID)) {
				break;
			}

			var hours = RECORD.a(2, MV);
			var activity = RECORD.a(4);
			var username = ID.field("*", 1);
			if (not(timerates.locate(username, usern, 1))) {
				{}
			}
			var rate = timerates.a(5, usern);
			//if rate='' and timerates<5> then
			if (rate == "") {
				rate = 1;
				timerates.r(5, usern, rate);
				//tx<-1>='NO HOURLY RATE FOR USER ':quote(username):' - 1.00 USED'
				call printtx(tx);
			}
			cost += hours * rate;
			gosub getrec();
		}//loop;

//////////
//breakexit:
//////////
		if (cost) {
			tx = tr ^ sep ^ tag(td, jobno) ^ " " ^ tag(td, job.a(9, 1, 1))
				^ " " ^ tag(tdr,cost.oconv("MD20P")) ^ trx;
			call printtx(tx);
			totalcost += cost;
		}

	}//nextbreak

/////
//exit:
/////
	tx ^= FM;

	var tx2 = tr ^ (sep ^ tag(td, "TOTAL COST")) ^ sep ^ tag(td, "")
	 ^ sep ^ tag(tdr,totalcost.oconv("MD20P")) ^ FM;

	tx2.swapper("<TD", "<TH");
	tx2.swapper("</TD", "</TH");
	tx ^= tx2;
	tx ^= "</TBODY></TABLE>";

	tx ^= FM;

	var mark = "";
	call getmark("OWN", 1, mark);
	tx ^= mark;

	call printtx(tx);

*/

	return 1;

}

/*
subroutine getrec() {

getrec2:
	if (not ID.readnext(MV)) {
		ID = "";
		newjobno = "";
		return;
	}
	if (not(RECORD.read(gen.timesheets, ID))) {
		goto getrec2;
	}

	newjobno = RECORD.a(1, MV);
	return;

}
*/

libraryexit()
