#include <exodus/library.h>
libraryinit()

#include <log2.h>
#include <indices2.h>
#include <quickdexer.h>

#include <gen.h>

var logtime;

function main() {

	call log2("*add client group index", logtime);
	if (not(indices2("CLIENTS", "GROUP_CODE"))) {
		var("MAKEINDEX CLIENTS GROUP_CODE").execute();
	}

	call log2("*add supplier group index", logtime);
	if (not(indices2("SUPPLIERS", "GROUP_CODE"))) {
		var("MAKEINDEX SUPPLIERS GROUP_CODE").execute();
	}

	call log2("*add clients name index", logtime);
	if (not(indices2("CLIENTS", "SEQUENCE.XREF"))) {
		var("MAKEINDEX CLIENTS SEQUENCE XREF").execute();
	}

	call log2("*add brands executive index", logtime);
	if (not(indices2("CLIENTS", "EXECUTIVE_CODE"))) {
		var("MAKEINDEX CLIENTS EXECUTIVE_CODE BTREE LOWERCASE").execute();
	}

	call log2("*add brand name index", logtime);
	if (not(indices2("BRANDS", "SEQUENCE.XREF"))) {
		var("MAKEINDEX BRANDS SEQUENCE XREF").execute();
	}

	call log2("*add brands executive index", logtime);
	if (not(indices2("BRANDS", "EXECUTIVE_CODE"))) {
		var("MAKEINDEX BRANDS EXECUTIVE_CODE BTREE LOWERCASE").execute();
	}

	call log2("*add jobs master job_no index", logtime);
	if (not(indices2("JOBS", "MASTER_JOB_NO"))) {
		var("MAKEINDEX JOBS MASTER_JOB_NO").execute();
	}

	call log2("*add jobs date created index", logtime);
	if (not(indices2("JOBS", "DATE_CREATED"))) {
		var("MAKEINDEX JOBS DATE_CREATED").execute();
	}

	call log2("*add jobs closed index", logtime);
	//nb doesnt seem to select closed "" for some reason despite
	//them appearing to be in the ! index file
	//however CLOSED NO (ie is open) has been " " (space) for a long time
	//maybe able to select all open jobs 'WITH CLOSE LT "Y"'
	if (not(indices2("JOBS", "CLOSED"))) {
		var("MAKEINDEX JOBS CLOSED").execute();
	}

	call log2("*add vehicles media_type_code index", logtime);
	if (not(indices2("VEHICLES", "MEDIA_TYPE_CODE"))) {
		var("MAKEINDEX VEHICLES MEDIA_TYPE_CODE").execute();
	}

	if (not(indices2("VEHICLES", "SEQUENCE.XREF"))) {
		var("MAKEINDEX VEHICLES SEQUENCE XREF").execute();
	}

	call log2("*add invoices date index", logtime);
	if (not(indices2("INVOICES", "DATE"))) {
		var("MAKEINDEX INVOICES DATE").execute();
	}

	call log2("*add invoices sch_or_job_no index", logtime);
	if (not(indices2("INVOICES", "SCH_OR_JOB_NO"))) {
		var("MAKEINDEX INVOICES SCH_OR_JOB_NO").execute();
	}

	call log2("*add production invoice status index", logtime);
	if (not(indices2("PRODUCTION.INVOICES", "STATUS"))) {
		var("MAKEINDEX PRODUCTION.INVOICES STATUS").execute();
	}

	call log2("*add schedule executive index", logtime);
	if (not(indices2("SCHEDULES", "EXECUTIVE_CODE"))) {
		var("MAKEINDEX SCHEDULES EXECUTIVE_CODE BTREE LOWERCASE").execute();
	}

	//to find all schedules that start, cover or end on a particular period
	//we index on YEAR_PERIODS which would be mv start period and stop period
	//based on periods of start and stop dates (not start period in schedule)
	//eg SELECT SCHEDULES WITH YEAR_PERIODS BETWEEN FROM_YEAR_PERIOD AND UPTO_YEAR_PERIOD
	//call log2('*add schedule year_period_to index',logtime)
	if (not(indices2("SCHEDULES", "YEAR_PERIODS"))) {
		var("MAKEINDEX SCHEDULES YEAR_PERIODS BTREE").execute();
	}
	//call log2('*add plan year_period_to index',logtime)
	if (not(indices2("PLANS", "YEAR_PERIODS"))) {
		var("MAKEINDEX PLANS YEAR_PERIODS BTREE").execute();
	}

	call log2("*add plans executive index", logtime);
	if (not(indices2("PLANS", "EXECUTIVE_CODE"))) {
		var("MAKEINDEX PLANS EXECUTIVE_CODE BTREE LOWERCASE").execute();
	}

	call log2("*delete ads vehicle index", logtime);
	if (indices2("ADS", "VEHICLE_CODE")) {
		var("DELETEINDEX ADS VEHICLE_CODE").execute();
	}

	call log2("*add ads vehicle and date index", logtime);
	if (not(indices2("ADS", "VEHICLE_AND_DATE"))) {
		var("MAKEINDEX ADS VEHICLE_AND_DATE").execute();
	}

	call log2("*add ads brand and date index", logtime);
	if (not(indices2("ADS", "BRAND_AND_DATE"))) {
		var("MAKEINDEX ADS BRAND_AND_DATE").execute();
	}

	call log2("*add brands client_code index", logtime);
	if (not(indices2("BRANDS", "CLIENT_CODE"))) {
		var("MAKEINDEX BRANDS CLIENT_CODE").execute();
	}

	call log2("*add jobs executive index", logtime);
	var xx;
	if (not(xx.read(gen._definitions, "JOBSEXECLOWERCASE"))) {
		var("MAKEINDEX JOBS EXECUTIVE_CODE BTREE LOWERCASE").execute();
		var().date().write(gen._definitions, "JOBSEXECLOWERCASE");
	}

	call log2("*add booking.orders schedule_no index", logtime);
	if (not(indices2("BOOKING.ORDERS", "SCHEDULE_NO"))) {
		var("MAKEINDEX BOOKING.ORDERS SCHEDULE_NO").execute();
	}

	//call log2('*delete booking.orders year_period index',logtime)
	//if indices2('BOOKING.ORDERS','YEAR_PERIOD') then
	// execute 'DELETEINDEX BOOKING.ORDERS YEAR_PERIOD'
	// end

	call log2("*create booking.orders year_period index", logtime);
	if (not(indices2("BOOKING.ORDERS", "YEAR_PERIOD"))) {
		var("MAKEINDEX BOOKING.ORDERS YEAR_PERIOD").execute();
	}

	call log2("*remove quickdex from MATERIALS file", logtime);
	var FILES;
	if (FILES.open("FILES", "")) {
		if ((var("MATERIALS").xlate("FILES", 4, "X")).index("QUICKDEX", 1)) {
			call quickdexer("MATERIALS", "L", 1, "");
		}
		if ((var("MATERIALS").xlate("FILES", 4, "X")).index("RIGHTDEX", 1)) {
			call quickdexer("MATERIALS", "R", 1, "");
		}
	}

	call log2("*remove production.invoices client_order_no index", logtime);
	if (indices2("PRODUCTION.INVOICES", "CLIENT_ORDER_NO")) {
		var("DELETEINDEX PRODUCTION.INVOICES CLIENT_ORDER_NO").execute();
	}

	call log2("*add production.invoices client_order_no_PARTS index", logtime);
	if (not(indices2("PRODUCTION.INVOICES", "CLIENT_ORDER_NO_PARTS"))) {
		var("MAKEINDEX PRODUCTION.INVOICES CLIENT_ORDER_NO_PARTS").execute();
	}

	call log2("*remove schedules client_order_no XREF index", logtime);
	if (indices2("SCHEDULES", "CLIENT_ORDER_NO.XREF")) {
		var("DELETEINDEX SCHEDULES CLIENT_ORDER_NO XREF").execute();
	}

	call log2("*add schedules client_order_no_PARTS index", logtime);
	if (not(indices2("SCHEDULES", "CLIENT_ORDER_NO_PARTS"))) {
		var("MAKEINDEX SCHEDULES CLIENT_ORDER_NO_PARTS").execute();
	}

}


libraryexit()