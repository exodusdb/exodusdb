#include <exodus/library.h>
libraryinit()

#include <log2.h>
#include <openfile.h>
#include <readagp.h>
#include <authorised.h>
//TODO #include <timesheetparamsubs.h>

#include <gen.h>
#include <agy.h>

var logtime;
var buffer;
var supplierinvoices;
var analysis2;
var certificates;
var programs;
var productcategories;
var tasks;
var reply;//num
var recordx;
var msg;
var ii2;

var xx;

function main() {
	var thisname="initagency";//TODO could be mv. name set in perform and other places

	call log2("-----initagency init", logtime);
	var interactive = not SYSTEM.a(33);
	var reloading = SENTENCE.field(" ", 2);

	//check general common is available
	if (not iscommon(gen)) {
		var().stop("gen common is not initialised in " ^ thisname);
	}

	//CREATE agency common
	mv.labelledcommon[4]=new agy_common;
	if (not iscommon(agy)) {
		var().stop("agy common is not initialised in " ^ thisname);
	}

	//what about creating dicts?

	call log2("checking for agency modules (clients file)", logtime);
	if (openfile("CLIENTS", agy.clients)) {

		call log2("open agency files", logtime);
		openfile("BRANDS"		, agy.brands		,"CLIENTS");
		openfile("PRODUCT_CATEGORIES"	, productcategories	,"CLIENTS");
		openfile("SUPPLIERS"		, agy.suppliers		,"CLIENTS");
		openfile("JOB_TYPES"		, agy.jobtypes	,"CLIENTS");
		openfile("MARKETS"		, agy.markets		,"CLIENTS");
		openfile("INVOICES"		, agy.invoices		,"CLIENTS");
		openfile("ANALYSIS"		, agy.analysis		,"CLIENTS");
		openfile("ANALYSIS2"		, analysis2		, "ANALYSIS");

		openfile("CURRENCY_VERSIONS"	, xx			, "CURRENCIES");
		openfile("MARKET_VERSIONS"	, xx			, "MARKETS");
		openfile("COMPANY_VERSIONS"	, xx			, "COMPANIES");
		openfile("CLIENT_VERSIONS"	, xx			, "CLIENTS");
		openfile("SUPPLIER_VERSIONS"	, xx			, "SUPPLIERS");
		openfile("JOB_TYPE_VERSIONS"	, xx			, "JOB_TYPES");

		call log2("Add agency indexes", logtime);
		gosub makeindex("clients"	, "group_code");
		gosub makeindex("suppliers"	, "group_code");
		gosub makeindex("clients"	, "sequence"		, "xref");
		gosub makeindex("clients"	, "executive_code"	, "btree", "lowercase");
		gosub makeindex("brands"	, "executive_code"	, "btree", "lowercase");
		gosub makeindex("brands"	, "sequence"		, "xref");
		gosub makeindex("brands"	, "client_code");
		gosub makeindex("invoices"	, "date");
		gosub makeindex("invoices"	, "sch_or_job_no");

		call log2("Add agency tasks", logtime);
		addtask("MENU ANALYSIS"			, "NEOSYS");
		addtask("CLIENT UPDATE TERMS"		, "UA");
		addtask("SUPPLIER UPDATE TERMS"		, "UA");
		addtask("CLIENT CREATE STOPPED");
		addtask("BRAND CHANGE CLIENT");
		addtask("SCHEDULE UPDATE \"BULK AD\"", "UM2");
		addtask("PRODUCTION ORDER CREATE MANY PER JOB");
		addtask("TIMESHEET ADMINISTRATION","TA");

	}

	call log2("check for media module (schedules file)", logtime);
	agy.schedules="";
	if ((openfile("SCHEDULES", agy.schedules))) {

		call log2("open media files", logtime);
		openfile("PLANS"		, agy.plans		, "SCHEDULES");
		openfile("ADS"			, agy.ads		, "SCHEDULES");
		openfile("SURVEYS"		, agy.surveys		, "SCHEDULES");
		openfile("SUPPLIER_INVOICES"	, supplierinvoices	, "SCHEDULES");
		openfile("CERTIFICATES"		, certificates		, "SCHEDULES");
		openfile("PROGRAMS"		, programs		, "SCHEDULES");
		openfile("RATINGS"		, agy.ratings		, "SCHEDULES");
		openfile("MATERIALS"		, agy.materials		, "SCHEDULES");
		openfile("RATECARDS"		, agy.ratecards		, "SCHEDULES");
		openfile("VEHICLES"		, agy.vehicles		, "SCHEDULES");
		openfile("BOOKING_ORDERS"	, agy.bookingorders	, "SCHEDULES");

		openfile("VEHICLE_VERSIONS"	, xx			, "VEHICLES");
		openfile("RATECARD_VERSIONS"	, xx			, "RATECARDS");
		openfile("PLAN_VERSIONS"	, xx			, "PLANS");
		openfile("SCHEDULE_VERSIONS"	, xx			, "SCHEDULES");

		call log2("Add media indexes", logtime);
		gosub makeindex("schedules"	, "executive_code"		, "btree"	, "lowercase");
		gosub makeindex("plans"		, "executive_code"		, "btree"	, "lowercase");
		gosub makeindex("schedules"	, "year_periods"		, "btree");
		gosub makeindex("schedules"	, "client_order_no_parts");
		gosub makeindex("plans"		, "year_periods"		, "btree");
		gosub makeindex("ads"		, "vehicle_and_date");
		gosub makeindex("ads"		, "brand_and-date");
		gosub makeindex("booking.orders", "schedule_no");
		gosub makeindex("booking.orders", "year_period");
		gosub makeindex("vehicles"	, "JOB_TYPE_code");
		gosub makeindex("vehicles"	, "sequence"			, "xref");
		gosub makeindex("materials"	, "first_appearance_date");
		gosub makeindex("booking.orders", "date");

		call log2("add index for ADS brand vehicle date time if needed", logtime);
		//import certification will be slow until the next day restart after createads
		//but generally clients will not have an additional index on ads unless required
		var unmatchedads;
		if (unmatchedads.open("UNMATCHED.ADS", "")) {
			unmatchedads.select();
			if (unmatchedads.readnext(xx)) {
				gosub makeindex("ads", "brand_vehicle_date_time");
				gosub makeindex("unmatched_ads", "brand_and_date");
				gosub makeindex("unmatched_ads", "vehicle_and_date");
				gosub makeindex("unmatched_ads", "date");
			}
		}

		call log2("Add media tasks", logtime);
		addtask("MENU MEDIA"				, "NEOSYS");
		addtask("PLAN UNPROFITABLE");
		addtask("PLAN SET NUMBER"			, "NEOSYS");
		addtask("PLAN CREATE OWN NO"			, "PLAN CREATE OWN NUMBER");
		addtask("SCHEDULE CREATE OWN NO"		, "SCHEDULE CREATE OWN NUMBER");
		addtask("SCHEDULE SET NUMBER"		, "NEOSYS");
		addtask("BOOKING ORDER SET NUMBER"		, "NEOSYS");
		addtask("MEDIA INVOICE SET NUMBER"		, "NEOSYS");
		addtask("MEDIA INVOICE CREATE CREDIT"	, "UM");
		addtask("BOOKING ORDER CREATE CHANGE");
		addtask("BOOKING ORDER CREATE REPLACEMENT");
		addtask("BOOKING ORDER CREATE COMBINATION");
		addtask("BOOKING ORDER CREATE REISSUE");
		addtask("BOOKING ORDER CREATE CANCELLATION");
		addtask("SCHEDULE COINCIDENT ADS");
		addtask("SCHEDULE UPDATE WITHOUT REBOOKING");
		addtask("BOOKING ORDER BACKDATE"		, "NEOSYS");
		addtask("SCHEDULE UPDATE WEEK");
		addtask("SCHEDULE UPDATE MATERIAL"		, "SCHEDULE UPDATE PLAN");
		addtask("SCHEDULE UPDATE CERTIFIED"		, "SCHEDULE UPDATE AFTER INVOICE");
		addtask("SCHEDULE UPDATE SUPPLIER INVOICED"	, "SCHEDULE UPDATE AFTER INVOICE");
		addtask("MEDIA PRICE ACCESS");
		addtask("SCHEDULE UPDATE BRAND AFTER BOOKING"	, "UM2");
		addtask("SCHEDULE UPDATE BRAND AFTER INVOICE"	, "UM2");
		addtask("SCHEDULE UPDATE COMPANY AFTER BOOKING"	, "NEOSYS");
		addtask("SCHEDULE UPDATE UNAPPROVED");
		addtask("SCHEDULE UPDATE BRAND AFTER BOOKING"	, "UM2");
		addtask("SCHEDULE ADVANCED");
		addtask("SCHEDULE ACCESS TOTALS");
		addtask("SCHEDULE WITHOUT DATES");
		addtask("VEHICLE UPDATE SUPPLIER");
		addtask("RATECARD ACCESS A");
		addtask("RATECARD ACCESS B");
		addtask("RATECARD ACCESS C");
		addtask("RATECARD ACCESS D");
		addtask("PLAN APPROVAL");
		addtask("MEDIA INVOICE ACCESS RECEIPT"		, "AM");
		addtask("MEDIA ACCESS COMMISSION"			, "AM2");
		addtask("MEDIA INVOICE DESPATCH");
		addtask("MEDIA INVOICE UNDESPATCH");
		addtask("MEDIA INVOICE APPROVE");
		addtask("MEDIA INVOICE UNAPPROVE");

	}

	call log2("check for jobs module (jobs file)", logtime);
	agy.jobs="";
	if (openfile("JOBS", agy.jobs)) {

		call log2("open jobs files", logtime);
		openfile("PRODUCTION_INVOICES"		, agy.productioninvoices	, "JOBS");
		openfile("PRODUCTION_ORDERS"		, agy.productionorders		, "JOBS");
		openfile("TASKS"			, tasks				, "JOBS");

		openfile("JOB_VERSIONS"			, xx				, "JOBS");
		openfile("TIMESHEET_VERSIONS"		, xx				, "TIMESHEETS");
		openfile("PRODUCTION_ORDER_VERSIONS"	, xx				, "PRODUCTION_ORDERS");
		openfile("PRODUCTION_INVOICE_VERSIONS"	, xx				, "PRODUCTION_INVOICES");

		call log2("create jobs indexes", logtime);
		gosub makeindex("jobs"			, "master_job_no");
		gosub makeindex("jobs"			, "closed");
		gosub makeindex("jobs"			, "executive_code"		, "btree"	, "lowercase");
		gosub makeindex("production_invoices"	, "client_order_no_parts");
		gosub makeindex("production_invoices"	, "status");
		gosub makeindex("tasks"			, "user_code");
		gosub makeindex("tasks"			, "parent_user_code");

		call log2("Add job tasks", logtime);
		addtask("MENU JOBS"				, "NEOSYS");
		addtask("MENU TIMESHEETS"			, "NEOSYS");
		addtask("JOB SET NUMBER"			, "NEOSYS");
		addtask("PRODUCTION ORDER SET NUMBER"		, "NEOSYS");
		addtask("PRODUCTION ESTIMATE SET NUMBER"	, "NEOSYS");
		addtask("PRODUCTION INVOICE SET NUMBER"		, "NEOSYS");
		addtask("PRODUCTION ORDER ISSUE"		, "PRODUCTION ORDER CREATE");
		addtask("PRODUCTION ESTIMATE ISSUE"		, "PRODUCTION ESTIMATE CREATE");
		addtask("PRODUCTION ESTIMATE APPROVE"		, "PRODUCTION ESTIMATE ISSUE");
		addtask("PRODUCTION ESTIMATE CANCEL"		, "PRODUCTION ESTIMATE APPROVE");
		addtask("PRODUCTION ESTIMATE DELIVER"		, "PRODUCTION ESTIMATE APPROVE");
		//NOTE: PRODUCTION ESTIMATE INVOICE is PRODUCTION SUPPLIER INVOICE CREATE
		addtask("PRODUCTION SUPPLIER INVOICE CREATE"	, "PRODUCTION SUPPLIER INVOICE UPDATE");
		addtask("PRODUCTION ORDER CANCEL"		, "PRODUCTION ORDER ISSUE");
		addtask("PRODUCTION ESTIMATE CREATE MANY PER JOB");
		addtask("PRODUCTION ORDER CREATE WITHOUT ESTIMATE");
		addtask("JOB REOPEN"				, "UA");
		addtask("JOB CREATE WITHOUT TYPE");
		addtask("PRODUCTION INVOICE ACCESS RECEIPT"	, "AP");
		addtask("PRODUCTION ACCESS COMMISSION"		, "AP2");
		addtask("TASK ACCESS"				, "AP");
		addtask("TASK ACCESS TEAM"			, "AP2");
		addtask("TASK ACCESS ALL"			, "AP2");
		addtask("TASK CREATE"				, "UP");
		addtask("TASK CREATE ALL"			, "UP2");
		addtask("TASK CREATE CROSSDEPT"			, "UP");
		addtask("TASK UPDATE"				, "UP");
		addtask("PRODUCTION INVOICE DESPATCH");
		addtask("PRODUCTION INVOICE UNDESPATCH");
		addtask("PRODUCTION INVOICE APPROVE");
		addtask("PRODUCTION INVOICE UNAPPROVE");

	}

	//get agp to do various updates to it
	if (not(agy.agp.read(DEFINITIONS, "AGENCY.PARAMS"))) {
		call note("AGENCY.PARAMS IS MISSING FROM DEFINITIONS");
		var().stop();
	}
	var origagp = agy.agp;

	//fix base currency code
	if (agy.agp.a(2) ne "CYP" and gen.company.a(3) and agy.agp.a(2) ne gen.company.a(3)) {
		call log2("*fix company base currency code in agency.params to " ^ gen.company.a(3), logtime);
		agy.agp.r(2, gen.company.a(3));
		//writev agp<2> on definitions,'AGENCY.PARAMS',2
	}

	call log2("fix base currency format", logtime);
	if (gen.company.a(3)) {
		if (gen.currency.read(gen.currencies, gen.company.a(3))) {
			agy.agp.r(3, "MD" ^ gen.currency.a(3) ^ "0P");
			call log2("*fix base currency format in agency.params to " ^ agy.agp.a(3), logtime);
			//writev agp<3> on definitions,'AGENCY.PARAMS',3
		}
	}

	call log2("make sure of currency format", logtime);
	if (not(agy.agp.a(3) == "MD20P" or agy.agp.a(3) == "MD30P" or agy.agp.a(3) == "MD00P")) {
		agy.agp.r(3, "MD20P");
		//writev agp<3> on definitions,'AGENCY.PARAMS',3
	}

	call log2("move booking options into new grouping", logtime);
	if ((agy.agp.a(77)).length()) {
		if (agy.agp.a(77)) {
			agy.agp.r(79, 1, -1, "4");
		}
		agy.agp.r(77, "");
	}
	if ((agy.agp.a(78)).length()) {
		if (agy.agp.a(78)) {
			agy.agp.r(79, 1, -1, "3");
		}
		agy.agp.r(78, "");
	}

	call log2("update agency.params", logtime);
	if (agy.agp ne origagp) {
		agy.agp.write(DEFINITIONS, "AGENCY.PARAMS");
	}

	call log2("get agency.params", logtime);
	call readagp();

	call log2("put web site and email into system parameters", logtime);
	if (SYSTEM.a(8) == "") {
		SYSTEM.r(8, agy.agp.a(16));
	}
	if (SYSTEM.a(14) == "") {
		SYSTEM.r(14, agy.agp.a(1));
		SYSTEM.r(36, agy.agp.a(1));
	}

	call log2("*create NONE supplier", logtime);
	var supplier;
	if (not(supplier.read(agy.suppliers, "NONE"))) {
		supplier = "No Cost";
		supplier.r(13, "MP");
		supplier.write(agy.suppliers, "NONE");
	}

	call log2("*clear unused brands", logtime);
	if (not(xx.read(DEFINITIONS, "ORPHANBRANDSFIXED"))) {
		agy.brands.select();
		var brandcode;
		while (agy.brands.readnext(brandcode)) {
			var brand;
			if (brand.read(agy.brands, brandcode)) {
				if (brand.a(1) == "") {
					agy.brands.deleterecord(brandcode);
				}
			}
		}
		var().date().write(DEFINITIONS, "ORPHANBRANDSFIXED");
	}

/*TODO
	call log2("*build client group members", logtime);
	var convkey = "BUILDCLIENTGROUPMEMBERS";
	if (lockrecord("DEFINITIONS", DEFINITIONS, convkey, recordx, 999999)) {
		if (not(xx.read(DEFINITIONS, convkey))) {
			perform("WINDOWSTUB CLIENT.SUBS " ^ convkey);
			var().date().write(DEFINITIONS, convkey);
		}
		call unlockrecord("DEFINITIONS", DEFINITIONS, convkey);
	}

	call log2("*build supplier group members", logtime);
	convkey = "BUILDSUPPLIERGROUPMEMBERS";
	if (lockrecord("DEFINITIONS", DEFINITIONS, convkey, recordx, 999999)) {
		if (not(xx.read(DEFINITIONS, convkey))) {
			perform("WINDOWSTUB SUPPLIER.SUBS " ^ convkey);
			var().date().write(DEFINITIONS, convkey);
		}
		call unlockrecord("DEFINITIONS", DEFINITIONS, convkey);
	}
*/

	var datasetcode = SYSTEM.a(17);

	//add missing/new tasks
	//since failure to find task in web ui
	//doesnt cause automatic addition into auth file since only disallowed
	//tasks are sent to client for speed
	//Failure to show all possible tasks would mean difficulty to know
	//what tasks are available to be locked
	//in init.xxx files per module
	var deletex = "%DELETE%";

	call log2("create default timesheet parameters", logtime);
	var rec;
	if (not(rec.read(DEFINITIONS, "TIMESHEET.PARAMS"))) {
		var("N" ^ FM ^ 0 ^ FM ^ 16 ^ FM ^ 3 ^ FM ^ 0 ^ FM ^ "DEFAULTED").write(DEFINITIONS, "TIMESHEET.PARAMS");
	}

	//TODO call log2("*setup timesheet reminder/approval autorun");
	//call timesheetparamsubs("SETUPAUTORUN");

/* needs createalert
	call log2("*autostop inactive vehicles", logtime);
	var tt;
	if (not(tt.readv(DEFINITIONS, "INIT*INACTVEH", 1))) {
		tt = "";
	}
	if (tt < 16207) {
		var dom = (var().date()).oconv("D/E").field("/", 1) + 0;
		var cmd = "CREATEALERT INACTVEH MEDIA INACTIVEVEHICLES:365:S {} NEOSYS (ROS)";
		//ensure it will run sometime up to 28th, monthly
		if (dom > 28) {
			//run once today
			tt = cmd;
			tt.swapper("{}", "7:::::1");
			perform(tt);
			dom = 28;
		}
		//run on the scheduled day of the month
		tt = cmd;
		tt.swapper("{}", "7:" ^ dom);
		perform(tt);
		var().date().write(DEFINITIONS, "INIT*INACTVEH");
	}

	call log2("*autoclose inactive jobs", logtime);
	if (not(tt.readv(DEFINITIONS, "INIT*INACTJOB", 1))) {
		tt = "";
	}
	if (tt < 16230) {
		var dom = (var().date()).oconv("D/E").field("/", 1) + 0;
		var cmd = "JOB INACTIVEJOBS:365:C {} NEOSYS (ROS)";
		//ensure it will run sometime up to 28th, monthly
		if (dom > 28) {
			//run once today
			tt = cmd;
			tt.swapper("{}", "7:::::1");
			perform("CREATEALERT INACTJOB0 " ^ tt);
			dom = 28;
		}
		//run on the scheduled day of the month
		tt = cmd;
		tt.swapper("{}", "7:" ^ dom);
		perform("CREATEALERT INACTJOB " ^ tt);
		var().date().write(DEFINITIONS, "INIT*INACTJOB");
	}
*/

/*needs CREATEADS

	call log2("*run CREATEADS if required and before 7am and not test data", logtime);
	var nyears = agy.agp.a(125);
	if (nyears.length() and not SYSTEM.a(61) and var().time() < var("07:00").iconv("MT") and not interactive and not reloading) {

		if (DEFINITIONS.lock( "LOCK*CREATEADS")) {

			//nyearsback/agp<125> will be cleared if CREATEADS starts
			//hopefully there will be no other processes running
			//if there are any same db processes running then it will leave it for next time
			//writev '' on definitions,'AGENCY.PARAMS',125

			//note that CREATEADS might be aborted by GLOBAL.END file
			//and therefore dual scheduled starts on windows 2008
			//might cause CREATEADS to start twice
			//... but on the second run it should complete ok

			var cmd = "CREATEADS CREATE " ^ nyears ^ " (S)";
			perform(cmd);

			DEFINITIONS.unlock( "LOCK*CREATEADS");
		}else{

		}
	}
*/
	call log2("-----initagency exit", logtime);

	return 0;
}

subroutine makeindex(in filename, in indexname, in mode="btree", in lowercase=""){
	
	if (mode ne "btree" or lowercase ne "") {
		call log2("*WARNING cant create index " ^ filename ^ " " ^
			indexname ^ " " ^ mode, logtime);
		return;
	}
	call log2("*check/create index " ^ filename ^ " " ^ indexname, logtime);
	if (not var().listindexes(filename).index(indexname)) {
		//TODO AFTER CREATING DICT FILES AND COLUMNS filename.createindex(indexname);
	}
	return;
}

subroutine addtask(in taskname, in defaultlock_or_taskname="") {
		var xx;
		authorised(taskname, xx, defaultlock_or_taskname);
}
///
libraryexit()
