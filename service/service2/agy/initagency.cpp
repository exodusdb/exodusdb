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

	//CREATE LABELLED COMMON
	mv.labelledcommon[4]=new agy_common;

	call log2("*INIT.AGENCY initialisation", logtime);
	var interactive = not SYSTEM.a(33);

	//y2k2

	var reloading = SENTENCE.field(" ", 2);

	call log2("*open various files", logtime);
	if (not(openfile("SCHEDULES", agy.schedules))) {
		var valid = "";
	}
	if (not(openfile("ADS", agy.ads, "SCHEDULES", ""))) {
		var valid = "";
	}
	if (not(openfile("SURVEYS", agy.surveys, "SCHEDULES", ""))) {
		var valid = "";
	}
	if (not(openfile("SUPPLIER.INVOICES", supplierinvoices, "SCHEDULES", 1))) {
		var valid = "";
	}
	if (not(openfile("CLIENTS", agy.clients))) {
		var valid = "";
	}
	if (not(openfile("BRANDS", agy.brands))) {
		var valid = "";
	}
	if (not(openfile("VEHICLES", agy.vehicles))) {
		var valid = "";
	}
	if (not(openfile("SUPPLIERS", agy.suppliers))) {
		var valid = "";
	}
	if (not(openfile("RATECARDS", agy.ratecards))) {
		var valid = "";
	}
	if (not(openfile("MEDIA.TYPES", agy.mediatypes))) {
		var valid = "";
	}
	if (not(openfile("MARKETS", agy.markets))) {
		var valid = "";
	}
	if (not(openfile("INVOICES", agy.invoices))) {
		var valid = "";
	}
	if (not(openfile("ANALYSIS", agy.analysis))) {
		var valid = "";
	}
	if (not(openfile("ANALYSIS2", analysis2, "ANALYSIS", ""))) {
		var valid = "";
	}
	if (not(openfile("BOOKING.ORDERS", agy.bookingorders))) {
		var valid = "";
	}
	if (not(openfile("JOBS", agy.jobs))) {
		var valid = "";
	}
	if (not(openfile("PRODUCTION.INVOICES", agy.productioninvoices))) {
		var valid = "";
	}
	if (not(openfile("PRODUCTION.ORDERS", agy.productionorders))) {
		var valid = "";
	}
	if (not(openfile("CERTIFICATES", certificates, "SCHEDULES", 1))) {
		var valid = "";
	}
	if (not(openfile("PROGRAMS", programs, "SCHEDULES", 1))) {
		var valid = "";
	}
	if (not(openfile("RATINGS", agy.ratings, "SCHEDULES", 1))) {
		var valid = "";
	}
	if (not(openfile("MATERIALS", agy.materials, "SCHEDULES", 1))) {
		var valid = "";
	}
	if (not(openfile("PRODUCT.CATEGORIES", productcategories, "BRANDS", 1))) {
		var valid = "";
	}
	if (not(openfile("TASKS", tasks, "JOBS", 1))) {
		var valid = "";
	}

	gosub makeindex("tasks", "user_code");
	gosub makeindex("tasks", "parent_user_code");

	gosub makeindex("materials", "first_appearance_date");
	gosub makeindex("booking.orders", "date");

	call log2("*add ADS brand vehicle date time index if needed", logtime);
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

	gosub makeindex("clients", "group_code");
	gosub makeindex("suppliers", "group_code");
	gosub makeindex("clients", "sequence", "xref");
	gosub makeindex("clients", "executive_code", "btree", "lowercase");
	gosub makeindex("brands", "executive_code", "btree", "lowercase");
	gosub makeindex("brands", "sequence", "xref");
	gosub makeindex("jobs", "master_job_no");
	gosub makeindex("jobs", "closed");
	gosub makeindex("vehicles", "media_type_code");
	gosub makeindex("vehicles", "sequence", "xref");
	gosub makeindex("invoices", "date");
	gosub makeindex("invoices", "sch_or_job_no");
	gosub makeindex("production.invoices", "status");
	gosub makeindex("schedules", "executive_code", "btree", "lowercase");
	gosub makeindex("plans", "executive_code", "btree", "lowercase");
	gosub makeindex("jobs", "executive_code", "btree", "lowercase");
	gosub makeindex("schedules", "year_periods", "btree");
	gosub makeindex("plans", "year_periods", "btree");
	gosub makeindex("ads", "vehicle_and_date");
	gosub makeindex("ads", "brand_and-date");
	gosub makeindex("brands", "client_code");
	gosub makeindex("booking.orders", "schedule_no");
	gosub makeindex("booking.orders", "year_period");
	gosub makeindex("production.invoices", "client_order_no_parts");
	gosub makeindex("schedules", "client_order_no_parts");

	//to find all schedules that start, cover or end on a particular period
	//we index on YEAR_PERIODS which would be mv start period and stop period
	//based on periods of start and stop dates (not start period in schedule)
	//eg SELECT SCHEDULES WITH YEAR_PERIODS BETWEEN FROM_YEAR_PERIOD AND UPTO_YEAR_PERIOD
	//call log2('*add schedule year_period_to index',logtime)

	if (not(openfile("JOB.VERSIONS", xx, "JOBS", 1))) {
		var valid = "";
	}
	if (not(openfile("TIMESHEET.VERSIONS", xx, "TIMESHEETS", 1))) {
		var valid = "";
	}
	if (not(openfile("PRODUCTION.ORDER.VERSIONS", xx, "PRODUCTION.ORDERS", 1))) {
		var valid = "";
	}
	if (not(openfile("PRODUCTION.INVOICE.VERSIONS", xx, "PRODUCTION.INVOICES", 1))) {
		var valid = "";
	}

	if (not(openfile("CURRENCY.VERSIONS", xx, "CURRENCIES", 1))) {
		var valid = "";
	}
	if (not(openfile("MARKET.VERSIONS", xx, "MARKETS", 1))) {
		var valid = "";
	}
	if (not(openfile("COMPANY.VERSIONS", xx, "COMPANIES", 1))) {
		var valid = "";
	}
	if (not(openfile("CLIENT.VERSIONS", xx, "CLIENTS", 1))) {
		var valid = "";
	}
	if (not(openfile("SUPPLIER.VERSIONS", xx, "SUPPLIERS", 1))) {
		var valid = "";
	}
	if (not(openfile("MEDIA.TYPE.VERSIONS", xx, "MEDIA.TYPES", 1))) {
		var valid = "";
	}
	if (not(openfile("VEHICLE.VERSIONS", xx, "VEHICLES", 1))) {
		var valid = "";
	}
	if (not(openfile("RATECARD.VERSIONS", xx, "RATECARDS", 1))) {
		var valid = "";
	}

	if (not(openfile("PLAN.VERSIONS", xx, "PLANS", 1))) {
		var valid = "";
	}
	if (not(openfile("SCHEDULE.VERSIONS", xx, "SCHEDULES", 1))) {
		var valid = "";
	}

	//get agp to do various updates to it
	if (not(agy.agp.read(gen._definitions, "AGENCY.PARAMS"))) {
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

	//fix base currency format
	if (gen.company.a(3)) {
		if (gen.currency.read(gen.currencies, gen.company.a(3))) {
			agy.agp.r(3, "MD" ^ gen.currency.a(3) ^ "0P");
			call log2("*fix base currency format in agency.params to " ^ agy.agp.a(3), logtime);
			//writev agp<3> on definitions,'AGENCY.PARAMS',3
		}
	}

	//make sure of currency format
	if (not(agy.agp.a(3) == "MD20P" or agy.agp.a(3) == "MD30P" or agy.agp.a(3) == "MD00P")) {
		agy.agp.r(3, "MD20P");
		//writev agp<3> on definitions,'AGENCY.PARAMS',3
	}

	//move booking options into new grouping
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

	if (agy.agp ne origagp) {
		agy.agp.write(gen._definitions, "AGENCY.PARAMS");
	}

	//get agency.params
	call readagp();

	call log2("*web site", logtime);
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
	if (not(xx.read(gen._definitions, "ORPHANBRANDSFIXED"))) {
		agy.brands.select();
nextbrandcode:
		var brandcode;
		if (agy.brands.readnext(brandcode)) {
			var brand;
			if (brand.read(agy.brands, brandcode)) {
				if (brand.a(1) == "") {
					agy.brands.deleterecord(brandcode);
				}
			}
			goto nextbrandcode;
		}
		var().date().write(gen._definitions, "ORPHANBRANDSFIXED");
	}

	call log2("*build client group members", logtime);
	var convkey = "BUILDCLIENTGROUPMEMBERS";
	if (lockrecord("DEFINITIONS", gen._definitions, convkey, recordx, 999999)) {
		if (not(xx.read(gen._definitions, convkey))) {
			perform("WINDOWSTUB CLIENT.SUBS " ^ convkey);
			var().date().write(gen._definitions, convkey);
		}
		call unlockrecord("DEFINITIONS", gen._definitions, convkey);
	}

	call log2("*build supplier group members", logtime);
	convkey = "BUILDSUPPLIERGROUPMEMBERS";
	if (lockrecord("DEFINITIONS", gen._definitions, convkey, recordx, 999999)) {
		if (not(xx.read(gen._definitions, convkey))) {
			perform("WINDOWSTUB SUPPLIER.SUBS " ^ convkey);
			var().date().write(gen._definitions, convkey);
		}
		call unlockrecord("DEFINITIONS", gen._definitions, convkey);
	}

	var datasetcode = SYSTEM.a(17);

	var deletex = "%DELETE%";

	//setup a few tasks in advance since failure to find task in client
	//doesnt cause automatic addition into auth file since only disallowed
	//tasks are sent to client for speed
	//Failure to show all possible tasks would mean difficulty to know
	//what tasks are available to be locked
	//in init.xxx files per module
	if (not(authorised("PLAN UNPROFITABLE", msg))) {
		{}
	}
	if (not(authorised("PLAN SET NUMBER", msg, "NEOSYS"))) {
		{}
	}

	if (not(authorised("PLAN CREATE OWN NO", msg, "PLAN CREATE OWN NUMBER"))) {
		{}
	}
	if (not(authorised(deletex ^ "PLAN CREATE OWN NUMBER", msg))) {
		{}
	}
	if (not(authorised("SCHEDULE CREATE OWN NO", msg, "SCHEDULE CREATE OWN NUMBER"))) {
		{}
	}
	if (not(authorised(deletex ^ "SCHEDULE CREATE OWN NUMBER", msg))) {
		{}
	}

	if (not(authorised("SCHEDULE SET NUMBER", msg, "NEOSYS"))) {
		{}
	}
	if (not(authorised("BOOKING ORDER SET NUMBER", msg, "NEOSYS"))) {
		{}
	}
	if (not(authorised("MEDIA INVOICE SET NUMBER", msg, "NEOSYS"))) {
		{}
	}
	if (not(authorised("MEDIA INVOICE CREATE CREDIT", msg, "UM"))) {
		{}
	}
	if (not(authorised("%RENAME%" "MEDIA INVOICE INVOICE UNCERTIFIED", msg, "MEDIA INVOICE CREATE UNCERTIFIED"))) {
		{}
	}

	if (not(authorised("JOB SET NUMBER", msg, "NEOSYS"))) {
		{}
	}
	if (not(authorised("PRODUCTION ORDER SET NUMBER", msg, "NEOSYS"))) {
		{}
	}
	if (not(authorised("PRODUCTION ESTIMATE SET NUMBER", msg, "NEOSYS"))) {
		{}
	}
	if (not(authorised("PRODUCTION INVOICE SET NUMBER", msg, "NEOSYS"))) {
		{}
	}
	if (not(authorised("CLIENT UPDATE TERMS", msg, "UA"))) {
		{}
	}
	if (not(authorised("SUPPLIER UPDATE TERMS", msg, "UA"))) {
		{}
	}
	//if security('RATECARD OVERRIDE',msg,'') else null
	if (not(authorised("BOOKING ORDER CREATE CHANGE", msg, ""))) {
		{}
	}
	if (not(authorised("BOOKING ORDER CREATE REPLACEMENT", msg, ""))) {
		{}
	}
	if (not(authorised("BOOKING ORDER CREATE COMBINATION", msg, ""))) {
		{}
	}
	if (not(authorised("BOOKING ORDER CREATE REISSUE", msg, ""))) {
		{}
	}
	if (not(authorised("BOOKING ORDER CREATE CANCELLATION", msg, ""))) {
		{}
	}
	if (not(authorised("SCHEDULE COINCIDENT ADS", msg, ""))) {
		{}
	}
	if (not(authorised("SCHEDULE UPDATE WITHOUT REBOOKING", msg, ""))) {
		{}
	}
	if (not(authorised("BOOKING ORDER BACKDATE", msg, "NEOSYS"))) {
		{}
	}
	if (not(authorised("CLIENT CREATE STOPPED", msg))) {
		{}
	}
	if (not(authorised("SCHEDULE UPDATE WEEK", msg))) {
		{}
	}
	if (not(authorised("SCHEDULE UPDATE MATERIAL", msg, "SCHEDULE UPDATE PLAN"))) {
		{}
	}
	if (not(authorised("SCHEDULE UPDATE CERTIFIED", msg, "SCHEDULE UPDATE AFTER INVOICE"))) {
		{}
	}
	if (not(authorised("SCHEDULE UPDATE SUPPLIER INVOICED", msg, "SCHEDULE UPDATE AFTER INVOICE"))) {
		{}
	}

	if (not(authorised("MENU MEDIA", msg))) {
		{}
	}
	if (not(authorised("MENU JOBS", msg))) {
		{}
	}
	if (not(authorised("MENU ANALYSIS", msg))) {
		{}
	}
	if (not(authorised("MENU TIMESHEETS", msg))) {
		{}
	}
	if (not(authorised("SCHEDULE UPDATE BRAND AFTER BOOKING", msg, "UM2"))) {
		{}
	}
	if (not(authorised("SCHEDULE UPDATE BRAND AFTER INVOICE", msg, "UM2"))) {
		{}
	}
	if (not(authorised("SCHEDULE UPDATE COMPANY AFTER BOOKING",msg, "NEOSYS"))) {
		{}
	}
	if (not(authorised("SCHEDULE UPDATE UNAPPROVED", msg))) {
		{}
	}
	if (not(authorised("SCHEDULE UPDATE BRAND AFTER BOOKING", msg, "UM2"))) {
		{}
	}
	if (not(authorised("SCHEDULE ADVANCED", msg, ""))) {
		{}
	}
	if (not(authorised("SCHEDULE ACCESS TOTALS", msg, ""))) {
		{}
	}
	if (not(authorised("SCHEDULE WITHOUT DATES", msg, ""))) {
		{}
	}
	if (not(authorised("BRAND CHANGE CLIENT", msg, ""))) {
		{}
	}
	if (not(authorised("VEHICLE UPDATE SUPPLIER", msg, ""))) {
		{}
	}
	if (not(authorised("PRODUCTION ORDER ISSUE", msg, "PRODUCTION ORDER CREATE"))) {
		{}
	}
	if (not(authorised("PRODUCTION ESTIMATE ISSUE", msg, "PRODUCTION ESTIMATE CREATE"))) {
		{}
	}
	if (not(authorised("PRODUCTION ESTIMATE APPROVE", msg, "PRODUCTION ESTIMATE ISSUE"))) {
		{}
	}
	if (not(authorised("PRODUCTION ESTIMATE CANCEL", msg, "PRODUCTION ESTIMATE APPROVE"))) {
		{}
	}
	if (not(authorised("PRODUCTION ESTIMATE DELIVER", msg, "PRODUCTION ESTIMATE APPROVE"))) {
		{}
	}
	//PRODUCTION ESTIMATE INVOICE is PRODUCTION SUPPLIER INVOICE CREATE
	if (not(authorised("PRODUCTION SUPPLIER INVOICE CREATE", msg, "PRODUCTION SUPPLIER INVOICE UPDATE"))) {
		{}
	}
	if (not(authorised("PRODUCTION ORDER CANCEL", msg, "PRODUCTION ORDER ISSUE"))) {
		{}
	}
	if (not(authorised("MEDIA PRICE ACCESS", msg, ""))) {
		{}
	}
	if (not(authorised("PRODUCTION ESTIMATE CREATE MANY PER JOB", msg, ""))) {
		{}
	}
	if (not(authorised("PRODUCTION ORDER CREATE WITHOUT ESTIMATE", msg, ""))) {
		{}
	}
	for (var ii = 1; ii <= 4; ++ii) {
		if (not(authorised("RATECARD ACCESS " ^ var().chr(64 + ii), msg, ""))) {
			{}
		}
	};//ii;
	if (not(authorised("PLAN APPROVAL", msg, ""))) {
		{}
	}
	if (not(authorised("JOB REOPEN", msg, "UA"))) {
		{}
	}
	if (not(authorised("JOB CREATE WITHOUT TYPE", msg))) {
		{}
	}
	if (not(authorised("MEDIA INVOICE ACCESS RECEIPT", msg, "AM"))) {
		{}
	}
	if (not(authorised("PRODUCTION INVOICE ACCESS RECEIPT", msg, "AP"))) {
		{}
	}
	if (not(authorised("MEDIA ACCESS COMMISSION", msg, "AM2"))) {
		{}
	}
	if (not(authorised("PRODUCTION ACCESS COMMISSION", msg, "AP2"))) {
		{}
	}

	if (not(authorised("TASK ACCESS", msg, "AP"))) {
		{}
	}
	if (not(authorised("TASK ACCESS TEAM", msg, "AP2"))) {
		{}
	}
	if (not(authorised("TASK ACCESS ALL", msg, "AP2"))) {
		{}
	}
	if (not(authorised("TASK CREATE", msg, "UP"))) {
		{}
	}
	if (not(authorised("TASK CREATE ALL", msg, "UP2"))) {
		{}
	}
	if (not(authorised("TASK CREATE CROSSDEPT", msg, "UP"))) {
		{}
	}
	if (not(authorised("TASK UPDATE", msg, "UP"))) {
		{}
	}
	for (var ii = 1; ii <= 2; ++ii) {
		var tt = var("MEDIA,PRODUCTION").field(",", ii);
		if (not(authorised(tt ^ " INVOICE DESPATCH", msg, ""))) {
			{}
		}
		if (not(authorised(tt ^ " INVOICE UNDESPATCH", msg, ""))) {
			{}
		}
		if (not(authorised(tt ^ " INVOICE APPROVE", msg, ""))) {
			{}
		}
		if (not(authorised(tt ^ " INVOICE UNAPPROVE", msg, ""))) {
			{}
		}
	};//ii;

	if (not(authorised(deletex ^ "SUPPLIER INVOICE ACCESS", msg))) {
		{}
	}
	if (not(authorised(deletex ^ "SUPPLIER INVOICE CREATE", msg))) {
		{}
	}
	if (not(authorised(deletex ^ "SUPPLIER INVOICE UPDATE", msg))) {
		{}
	}

	//split/rename RATECARD OVERRIDE
	if (not(authorised("SCHEDULE UPDATE PRICE", msg, "RATECARD OVERRIDE"))) {
		{}
	}
	if (not(authorised("SCHEDULE UPDATE BILL", msg, "RATECARD OVERRIDE"))) {
		{}
	}
	if (not(authorised("SCHEDULE UPDATE COST", msg, "RATECARD OVERRIDE"))) {
		{}
	}
	if (not(authorised(deletex ^ "RATECARD OVERRIDE", msg))) {
		{}
	}

/*	call log2("*delete wrong plural tasks", logtime);
	if (not((tt).readv(gen._definitions, "DELETEWRONGPLURALTASKS", 1))) {
		tt = "";
	}
	if (tt < 16033) {
		tt = "BRANDS,JOBS,PLANS,SCHEDULES,PRODUCTION ORDERS,PRODUCTION INVOICES,PRODUCTION ESTIMATES,TIMESHEETS";
		tt.converter(",", VM);
		var nn = (tt).count(VM) + 1;
		for (var ii = 1; ii <= nn; ++ii) {
			if (not(authorised(deletex ^ tt.a(1, ii) ^ " LIST"))) {
				{}
			}
		};//ii;
		var().date().write(gen._definitions, "DELETEWRONGPLURALTASKS");
	}
*/
	call log2("*create default timesheet parameters", logtime);
	var rec;
	if (not(rec.read(gen._definitions, "TIMESHEET.PARAMS"))) {
		var("N" ^ FM ^ 0 ^ FM ^ 16 ^ FM ^ 3 ^ FM ^ 0 ^ FM ^ "DEFAULTED").write(gen._definitions, "TIMESHEET.PARAMS");
	}

	//TODO call log2("*setup timesheet reminder/approval autorun");
	//call timesheetparamsubs("SETUPAUTORUN");

/* needs createalert
	call log2("*autostop inactive vehicles", logtime);
	var tt;
	if (not(tt.readv(gen._definitions, "INIT*INACTVEH", 1))) {
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
		var().date().write(gen._definitions, "INIT*INACTVEH");
	}

	call log2("*autoclose inactive jobs", logtime);
	if (not(tt.readv(gen._definitions, "INIT*INACTJOB", 1))) {
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
		var().date().write(gen._definitions, "INIT*INACTJOB");
	}
*/

/*needs CREATEADS

	call log2("*run CREATEADS if required and before 7am and not test data", logtime);
	var nyears = agy.agp.a(125);
	if (nyears.length() and not SYSTEM.a(61) and var().time() < var("07:00").iconv("MT") and not interactive and not reloading) {

		if (gen._definitions.lock( "LOCK*CREATEADS")) {

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

			gen._definitions.unlock( "LOCK*CREATEADS");
		}else{

		}
	}
*/
	call log2("*quit INIT.AGENCY", logtime);

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
		var().createindex(indexname, filename);
	}
	return;
}

libraryexit()
