#include <exodus/library.h>
libraryinit()


#include <gen.h>

var mode;
var outputformat;
var toexec;
var byexec;
var orderby;
var summary;
var jobtypecodes;
var fromdate;
var uptodate;
var whichcosts;//num
var billcolumns;
var costcolumns;
var currencybase;//num
var showstatus;//num
var showclient;//num
var showsupplier;//num
var approved;
var booked;
var certified;
var supplierinvoice;
var invoiced;

function main(in alerttype0, in runasuser, out tasks, out title, out request, out datax) {
	//c med in,in,out,out,out,out

	//given alerttype0 which includes parameters separated by :
	// and runasuser
	//return tasks,title,request (and datax if used)

	//TOEXEC option if available for all media diary type reports

	//0) stop all vehicles with no ads appearing in the last 365 days
	//at 7am on the 14th of each month (ie regularly but not everyday)
	//CREATEALERT INACTVEH MEDIA INACTIVEVEHICLES:365:S 7:14 NEOSYS (ROS)

	//0) stop all executives with no schedules starting in the last 365 days
	//at 7am on the 14th of each month (ie regularly but not everyday)
	//CREATEALERT INACTMEDIAEXEC MEDIA INACTIVEMEDIAEXECS:365:S 7:14 NEOSYS (ROS)

	//media diary like reports
	//option 2 = media type codes eg M,N
	//option 3 = orderby. eg CLIENT_NAME,SCHEDULE_NO
	//option 4 = summary. 1=summary 2=summary without drilldown

	//1a) totally unapproved schedules to STEVE and JOE at 8pm run as NEOSYS
	//1b) approved then amended schedule ... ditto
	//1c) either of the above
	//CREATEALERT UNAPPSCH MEDIA UNAPPSCH 20 NEOSYS STEVE,JOE (ROS)
	//CREATEALERT UNAPPSCH MEDIA UNAPPSCH1 20 NEOSYS STEVE,JOE (ROS)
	//CREATEALERT UNAPPSCH MEDIA UNAPPSCH2 20 NEOSYS STEVE,JOE (ROS)

	//1) unapproved magazine and newspaper ads to execs at 4pm to NEOSYS
	//CREATEALERT UNAPP01 MEDIA UNAPPROVED:M,N:TOEXEC 16 NEOSYS (ROS)
	//CREATEALERT UNAPP01 MEDIA UNAPPROVED:M,N:BYEXEC 16 NEOSYS (ROS)

	//1) unbooked magazine and newspaper ads to execs at 4pm to NEOSYS
	//CREATEALERT UNBOOK MEDIA UNBOOKED:M,N:TOEXEC 16 NEOSYS (ROS)

	//1) uncertified ads at 4pm to NEOSYS
	//2) ditto only magazine and newspapers (M,N)
	//CREATEALERT UNCERT MEDIA UNCERTIFIED:M,N 16 NEOSYS (ROS)
	//CREATEALERT UNCERT MEDIA UNCERTIFIED:M,N 16 NEOSYS (ROS)

	//certified mag,nespaper ads pending invoicing to execs at 8am runas JOE
	//appearing more than 3 days ago
	//CREATEALERT UNINV01 MEDIA UNINVOICED:M,N:TOEXEC: 8 JOE (ROS)

	//1) seven days before material deadline warning to execs at 8am 12noon 5pm
	// shows ads without materials upto 30 days after appearance date
	//2) ditto but last day warning to manager
	//3) testing - manager to see the what executives are getting
	//CREATEALERT MATPEN01 MEDIA PENDINGMATERIAL:7:TOEXEC:30 8,12,17 manager (ROS)
	//CREATEALERT MATPEN02 MEDIA PENDINGMATERIAL:-1::30 8,12,17 manager (ROS)
	//CREATEALERT MATPEN03 MEDIA PENDINGMATERIAL:7:BYEXEC:30 8,12,17 manager (ROS)

	//YESTERDAYS XLS MEDIA DIARY
	//CREATEALERT ADSXLS01 MEDIA ADSXLS 7 NEOSYS (ROS)

	//1) media progress report to execs at 4pm to NEOSYS
	//CREATEALERT PRGRSS01 MEDIA PROGRESS:M,N:TOEXEC 16 NEOSYS (ROS)
	//CREATEALERT PRGRSS02 MEDIA PROGRESS:M,N:BYEXEC 16 NEOSYS (ROS)

	//global toexec,byexec,summary,outputformat,showsupplier,showclient,showstatus
	//global costcolumns,billcolumns,whichcosts,currencybase,mode,orderby

	var alerttype = alerttype0.field(":", 1);
	var alertoptions = alerttype0.field(":", 2, 9999).convert(":", FM);

	if (runasuser.unassigned()) {
		{}
	}

	mode = "";
	outputformat = "";

	toexec = "";
	byexec = "";

	toexec = alertoptions.index("TOEXEC");
	byexec = alertoptions.index("BYEXEC");
	if (toexec) {
		alertoptions.swapper("TOEXEC", "");
	}
	if (byexec) {
		alertoptions.swapper("BYEXEC", "");
	}

	orderby = alertoptions.a(2).convert(",", VM);
	summary = alertoptions.a(3);

	if (alerttype == "INACTIVEVEHICLES") {
		tasks = "VEHICLE UPDATE";
		title = "Inactive Vehicles";
		request = "INACTIVEVEHICLES";
		var ageindays = alertoptions.a(1);
		if (not(ageindays.match("1N0N"))) {
			call mssg(DQ ^ (ageindays ^ DQ) ^ " age in days should be numeric");
			var().stop();
		}
		datax = "{TODAY-" ^ ageindays ^ "}";
		var stopping = alertoptions.a(2);
		if (not((stopping == "S") or (stopping == ""))) {
			call mssg(DQ ^ (stopping ^ DQ) ^ " parameter 2 must be S for Stop or blank");
			var().stop();
		}
		datax.r(2, stopping);

	} else if (alerttype == "INACTIVEMEDIAEXECS") {
		tasks = "SCHEDULE UPDATE";
		title = "Inactive Media Executives";
		request = "INACTIVEMEDIAEXECS";
		var ageindays = alertoptions.a(1);
		if (not(ageindays.match("1N0N"))) {
			call mssg(DQ ^ (ageindays ^ DQ) ^ " age in days should be numeric");
			var().stop();
		}
		datax = "{TODAY-" ^ ageindays ^ "}";
		var stopping = alertoptions.a(2);
		if (not((stopping == "S") or (stopping == ""))) {
			call mssg(DQ ^ (stopping ^ DQ) ^ " parameter 2 must be S for Stop or blank");
			var().stop();
		}
		datax.r(2, stopping);

	} else if (alerttype == "ADSXLS") {
		title = "Approved Ads Appearing Yesterday";

		tasks = "MEDIA DIARY LIST";
		request = "MEDIADIARY";

		jobtypecodes = alertoptions.a(1);

		mode = "";
		outputformat = "xls";
		fromdate = "{YESTERDAY}";
		uptodate = fromdate;

		//gosub standardnocosts
		gosub mediadiary( title,  datax);

	} else if (alerttype == "UNAPPSCH") {
		title = "Pending Approval/Reapproval";
		tasks = "SCHEDULE LIST";
		request = "MEDIALIST";
		datax = "UNAPPROVED";

	} else if (alerttype == "UNAPPSCH1") {
		title = "Pending Approval";
		tasks = "SCHEDULE LIST";
		request = "MEDIALIST";
		datax = "UNAPPROVED1";

	} else if (alerttype == "UNAPPSCH2") {
		title = "Pending Reapproval";
		tasks = "SCHEDULE LIST";
		request = "MEDIALIST";
		datax = "UNAPPROVED2";

	} else if (alerttype == "UNAPPROVED") {

		title = "Pending Approval";

		tasks = "MEDIA DIARY LIST";
		request = "MEDIADIARY";

		jobtypecodes = alertoptions.a(1);

		//preset options by mode
		mode = "UNAPPROVED";

		//dates
		fromdate = "{30DAYSAGO}";
		uptodate = "";

		//gosub standardnocosts
		gosub mediadiary( title,  datax);

	} else if (alerttype == "PROGRESS") {

		title = "";

		tasks = "MEDIA DIARY LIST";
		request = "MEDIADIARY";

		jobtypecodes = alertoptions.a(1);

		//preset options by mode
		mode = "PROGRESS";

		//dates
		fromdate = "{60DAYSAGO}";
		uptodate = "{TODAY}";

		//gosub standardnocosts
		gosub mediadiary( title,  datax);

		//default order by client
		if (not(datax.a(16))) {
			datax.r(16, "CLIENT_NAME");
			datax.r(42, 2);
		}

	} else if (alerttype == "UNBOOKED") {

		title = "Pending Bookings";

		tasks = "MEDIA DIARY LIST";
		request = "MEDIADIARY";

		jobtypecodes = alertoptions.a(1);

		//preset options by mode
		mode = "UNBOOKED";

		//dates
		fromdate = "{30DAYSAGO}";
		uptodate = "";

		gosub mediadiary( title,  datax);

	} else if (alerttype == "PENDINGMATERIAL") {

		title = "Pending Materials";

		tasks = "MATERIAL ACCESS";
		request = "LISTMATERIALS";

		//before material due date
		var daysbefore = alertoptions.a(1);

		//after appearance date
		var daysafter = alertoptions.a(3);

		orderby = "";
		var emailtarget = "";
		if (toexec) {
			emailtarget = "EXECUTIVE";
		} else if (byexec) {
			orderby = "EXECUTIVE";
		}
//L964:
		datax = "";
		datax.r(16, orderby);
		datax.r(59, emailtarget);
		datax.r(60, daysbefore);
		datax.r(61, daysafter);

	} else if (alerttype == "UNCERTIFIED") {

		title = "Pending Certifications";

		tasks = "MEDIA DIARY LIST";
		request = "MEDIADIARY";

		jobtypecodes = alertoptions.a(1);

		//preset options by mode
		mode = "UNCERTIFIED";

		//dates
		fromdate = "{OPERATIONS_OPEN_DATE}";
		uptodate = "{2WORKINGDAYSAGO}";

		//gosub standardnocosts
		gosub mediadiary( title,  datax);

	} else if (alerttype == "UNINVOICED") {

		title = "Pending Invoices";

		tasks = "MEDIA DIARY LIST";
		request = "MEDIADIARY";

		jobtypecodes = alertoptions.a(1);
		var ageindays = alertoptions.a(2);

		//preset options by mode
		mode = "UNINVOICED";

		//dates
		fromdate = "{OPERATIONS_OPEN_DATE}";
		//uptodate='{TODAY}'
		//uptodate=''
		if (ageindays == "") {
			uptodate = var("31 DEC 2099").iconv("D");
		}else{
			uptodate = var().date() - ageindays;
		}

		//gosub standardnocosts
		gosub mediadiary( title,  datax);

	} else {
		call mssg(DQ ^ (alerttype ^ DQ) ^ " unrecognised alert type in MEDIA.ALERTS");
		var().stop();
	}
//L1211:
	return 0;

}

subroutine mediadiary(io title, io datax) {
	//mediadiary(io title, io datax)
	gosub standardnocosts( title,  datax);

	if (toexec) {
		datax.r(59, "EXECUTIVE");
	} else if (byexec) {
		datax.r(16, "EXECUTIVE_CODE");
	//pagebreak
	//datax<?>='EXECUTIVE_CODE'
	} else {
		datax.r(16, orderby);
		datax.r(42, summary);
	}
//L1296:
	return;

}

subroutine standardnocosts(io title, io datax) {
	//standardnocosts(io title, io datax)

	//costs (none)
	whichcosts = 4;
	billcolumns = "Client" _VM_ "1" _VM_ "" _VM_ "" _VM_ "" _VM_ "" _VM_ "" _VM_ "" _VM_ "1" _VM_ "1";
	costcolumns = "Agency" _VM_ "1" _VM_ "" _VM_ "" _VM_ "" _VM_ "" _VM_ "" _VM_ "" _VM_ "1" _VM_ "1";
	currencybase = 3;

	//columns
	showstatus = 1;
	showclient = 1;
	showsupplier = 1;

	//rows
	//summary=0;*normal

	gosub mediadiarydata( title,  datax);

	return;

}

subroutine mediadiarydata(io title, io datax) {
	//mediadiarydata(io title, io datax)
	var yes = 1;
	var no = 2;
	var all = 3;

	if ((mode == "") or (mode == "PROGRESS")) {
		approved = yes;

		booked = all;
		certified = all;
		supplierinvoice = all;
		invoiced = all;

	} else if (mode == "UNAPPROVED") {
		approved = no;

		booked = all;
		certified = all;
		supplierinvoice = all;
		invoiced = all;

	} else if (mode == "UNBOOKED") {
		approved = yes;
		booked = no;

		//dont consider as unbooked if it has any postbooking status
		//certified=no
		//supplierinvoice=no
		//invoiced=no
		certified = all;
		supplierinvoice = all;
		invoiced = all;

	//perhaps this is defaulted in MEDIADIARY but do it here for safety
	} else if (mode == "UNCERTIFIED") {
		approved = yes;
		certified = no;

		supplierinvoice = all;
		booked = all;
		invoiced = all;

	} else if (mode == "UNINVOICED") {

		certified = yes;

		approved = all;
		booked = all;
		supplierinvoice = all;
		invoiced = all;

	} else if (mode == "ADSXLS") {
		approved = yes;
	}
//L1628:
	datax = "";

	//general mode and filters

	//title for report
	datax.r(14, title);

	datax.r(1, mode);

	//status filters

	//approved 1/2/3=Yes/No/All
	datax.r(9, approved);

	//certified 1/2/3 Yes/No/All
	datax.r(10, certified);

	//supplier invoice 1/2/3 Yes/No/All
	datax.r(11, supplierinvoice);

	//booked 1/2/3 Yes/No/All
	datax.r(12, booked);

	//invoiced 1/2/3=Yes/No/All
	datax.r(102, invoiced);

	//data filters

	//filter
	datax.r(25, jobtypecodes.convert(",", VM));

	//costs

	//costs 4=no
	datax.r(3, whichcosts);

	//currency 1 local and base 2 local 3 base
	datax.r(4, currencybase);

	//bill/cost columns
	datax.r(17, 1, billcolumns);
	datax.r(17, 2, costcolumns);

	//agency fields 21-99

	//media fields 100-

	//show status columns
	//datax<13>=showstatus
	if (showstatus) {
		datax.r(101, 4, 4);
	}

	//show client name
	//datax<101>=showclient
	if (showclient) {
		datax.r(101, 1, 1);
	}

	//show supplier name
	if (showsupplier) {
		datax.r(101, 2, 2);
	}

	//common agency fields 21-99
	///////////////////////////

	//fromdate
	datax.r(30, fromdate);

	//uptodate
	datax.r(31, uptodate);

	//htm/xls/txt
	datax.r(37, outputformat);

	//summary 0/1/2 details+subtotals/subtotals with drilldown/subtotals only
	datax.r(42, summary);

	//calendar format option
	//1/2 Vertical/Horizontal
	//datax<46>=1

	return;

}


libraryexit()
