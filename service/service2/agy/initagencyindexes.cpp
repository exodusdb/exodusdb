#include <exodus/library.h>
libraryinit()

#include <log2.h>
#include <quickdexer.h>

#include <gen.h>
#include <agy.h>

var logtime;

function main() {
	//
	//c agy

	call log2("*add client group index", logtime);
	if (not(listindexes("CLIENTS", "GROUP_CODE"))) {
		execute("MAKEINDEX CLIENTS GROUP_CODE");
	}

	//call log2('*add supplier group index',logtime)
	//if indices2('SUPPLIERS','GROUP_CODE') else
	// execute 'MAKEINDEX SUPPLIERS GROUP_CODE'
	// end

	call log2("*add/rebuild suppliers full text index", logtime);
	var tt;
	if (not(tt.read(DEFINITIONS, "REINDEX*SUPPLIERS*SEQUENCE*XREF"))) {
		tt = "";
	}
	if (not listindexes("SUPPLIERS", "SEQUENCE.XREF") or (tt.a(1) < 17532)) {
		execute("REINDEX SUPPLIERS SEQUENCE.XREF (S)");
		var().date().write(DEFINITIONS, "REINDEX*SUPPLIERS*SEQUENCE*XREF");
	}

	call log2("*add clients name index", logtime);
	if (not(tt.read(DEFINITIONS, "REINDEX*CLIENTS*SEQUENCE*XREF"))) {
		tt = "";
	}
	if (not listindexes("CLIENTS", "SEQUENCE.XREF") or (tt.a(1) < 17532)) {
		execute("MAKEINDEX CLIENTS SEQUENCE XREF");
		var().date().write(DEFINITIONS, "REINDEX*CLIENTS*SEQUENCE*XREF");
	}

	call log2("*add brands executive index", logtime);
	if (not(listindexes("CLIENTS", "EXECUTIVE_CODE"))) {
		execute("MAKEINDEX CLIENTS EXECUTIVE_CODE BTREE LOWERCASE");
	}

	call log2("*add brand name index", logtime);
	if (not(tt.read(DEFINITIONS, "REINDEX*BRANDS*SEQUENCE*XREF"))) {
		tt = "";
	}
	if (not listindexes("BRANDS", "SEQUENCE.XREF") or (tt.a(1) < 17532)) {
		execute("MAKEINDEX BRANDS SEQUENCE XREF");
		var().date().write(DEFINITIONS, "REINDEX*BRANDS*SEQUENCE*XREF");
	}

	call log2("*add brands executive index", logtime);
	if (not(listindexes("BRANDS", "EXECUTIVE_CODE"))) {
		execute("MAKEINDEX BRANDS EXECUTIVE_CODE BTREE LOWERCASE");
	}

	call log2("*add jobs master job_no index", logtime);
	if (not(listindexes("JOBS", "MASTER_JOB_NO"))) {
		execute("MAKEINDEX JOBS MASTER_JOB_NO");
	}

	call log2("*add jobs date created index", logtime);
	if (not(tt.read(DEFINITIONS, "REINDEX*JOBS*DATE_CREATED"))) {
		tt = "";
	}
	if (not listindexes("JOBS", "DATE_CREATED") or (tt.a(1) <= 17250)) {
		execute("REINDEX JOBS DATE_CREATED (S)");
		var().date().write(DEFINITIONS, "REINDEX*JOBS*DATE_CREATED");
	}

	call log2("*add jobs closed index", logtime);
	//nb doesnt seem to select closed "" for some reason despite
	//them appearing to be in the ! index file
	//however CLOSED NO (ie is open) has been " " (space) for a long time
	//maybe able to select all open jobs 'WITH CLOSE LT "Y"'
	if (not(listindexes("JOBS", "CLOSED"))) {
		execute("MAKEINDEX JOBS CLOSED");
	}

	call log2("*add vehicles media_type_code index", logtime);
	if (not(listindexes("VEHICLES", "MEDIA_TYPE_CODE"))) {
		execute("MAKEINDEX VEHICLES MEDIA_TYPE_CODE");
	}

	call log2("*add/rebuild vehicles sequence full text index", logtime);
	if (not(tt.read(DEFINITIONS, "REINDEX*VEHICLES*SEQUENCE*XREF"))) {
		tt = "";
	}
	if (not listindexes("VEHICLES", "SEQUENCE.XREF") or (tt.a(1) < 17532)) {
		execute("REINDEX VEHICLES SEQUENCE.XREF (S)");
		var().date().write(DEFINITIONS, "REINDEX*VEHICLES*SEQUENCE*XREF");
	}

	//if indices2('VEHICLES','SEQUENCE.XREF') else
	// execute 'MAKEINDEX VEHICLES SEQUENCE XREF'
	// end

	call log2("*add invoices date index", logtime);
	if (not(listindexes("INVOICES", "DATE"))) {
		execute("MAKEINDEX INVOICES DATE");
	}

	call log2("*add invoices sch_or_job_no index", logtime);
	if (not(listindexes("INVOICES", "SCH_OR_JOB_NO"))) {
		execute("MAKEINDEX INVOICES SCH_OR_JOB_NO");
	}

	call log2("*add production invoice status index", logtime);
	if (not(listindexes("PRODUCTION_INVOICES", "STATUS"))) {
		execute("MAKEINDEX PRODUCTION_INVOICES STATUS");
	}

	call log2("*add production invoice date index", logtime);
	if (not(listindexes("PRODUCTION_INVOICES", "INVOICE_DATE"))) {
		execute("MAKEINDEX PRODUCTION_INVOICES INVOICE_DATE");
	}

	call log2("*add schedule executive index", logtime);
	if (not(listindexes("SCHEDULES", "EXECUTIVE_CODE"))) {
		execute("MAKEINDEX SCHEDULES EXECUTIVE_CODE BTREE LOWERCASE");
	}

	//to find all schedules that start, cover or end on a particular period
	//we index on YEAR_PERIODS which would be mv start period and stop period
	//based on periods of start and stop dates (not start period in schedule)
	//eg SELECT SCHEDULES WITH YEAR_PERIODS BETWEEN FROM_YEAR_PERIOD AND UPTO_YEAR_PERIOD
	//call log2('*add schedule year_period_to index',logtime)
	if (not(listindexes("SCHEDULES", "YEAR_PERIODS"))) {
		execute("MAKEINDEX SCHEDULES YEAR_PERIODS BTREE");
	}
	//call log2('*add plan year_period_to index',logtime)
	if (not(listindexes("PLANS", "YEAR_PERIODS"))) {
		execute("MAKEINDEX PLANS YEAR_PERIODS BTREE");
	}

	call log2("*add plans executive index", logtime);
	if (not(listindexes("PLANS", "EXECUTIVE_CODE"))) {
		execute("MAKEINDEX PLANS EXECUTIVE_CODE BTREE LOWERCASE");
	}

	call log2("*delete ads vehicle index", logtime);
	if (listindexes("ADS", "VEHICLE_CODE")) {
		execute("DELETEINDEX ADS VEHICLE_CODE");
	}

	/* assume done by REINDEXADS;
		//dont do in case they are being rebuilt on another database
		//although this means they will not be updated WHICH COULD BE A PROBLEM
		call log2('*add ads vehicle and date index',logtime);
		if indices2('ADS','VEHICLE_AND_DATE') else;
			execute 'MAKEINDEX ADS VEHICLE_AND_DATE';
			end;

		call log2('*add ads brand and date index',logtime);
		if indices2('ADS','BRAND_AND_DATE') else;
			execute 'MAKEINDEX ADS BRAND_AND_DATE';
			end;
	*/

	call log2("*add brands client_code index", logtime);
	if (not(listindexes("BRANDS", "CLIENT_CODE"))) {
		execute("MAKEINDEX BRANDS CLIENT_CODE");
	}

	call log2("*add jobs executive index", logtime);
	var xx;
	if (not(xx.read(DEFINITIONS, "JOBSEXECLOWERCASE"))) {
		execute("MAKEINDEX JOBS EXECUTIVE_CODE BTREE LOWERCASE");
		var().date().write(DEFINITIONS, "JOBSEXECLOWERCASE");
	}

	call log2("*add booking.orders schedule_no index", logtime);
	if (not(listindexes("BOOKING_ORDERS", "SCHEDULE_NO"))) {
		execute("MAKEINDEX BOOKING_ORDERS SCHEDULE_NO");
	}

	//call log2('*delete booking.orders year_period index',logtime)
	//if indices2('BOOKING_ORDERS','YEAR_PERIOD') then
	// execute 'DELETEINDEX BOOKING_ORDERS YEAR_PERIOD'
	// end

	call log2("*create booking.orders year_period index", logtime);
	if (not(listindexes("BOOKING_ORDERS", "YEAR_PERIOD"))) {
		execute("MAKEINDEX BOOKING_ORDERS YEAR_PERIOD");
	}

	call log2("*remove quickdex from MATERIALS file", logtime);
	var filesx;
	if (filesx.open("FILES", "")) {
		if (var("MATERIALS").xlate("FILES", 4, "X").index("QUICKDEX")) {
			call quickdexer("MATERIALS", "L", 1, "");
		}
		if (var("MATERIALS").xlate("FILES", 4, "X").index("RIGHTDEX")) {
			call quickdexer("MATERIALS", "R", 1, "");
		}
	}

	call log2("*remove production.invoices client_order_no index", logtime);
	if (listindexes("PRODUCTION_INVOICES", "CLIENT_ORDER_NO")) {
		execute("DELETEINDEX PRODUCTION_INVOICES CLIENT_ORDER_NO");
	}

	call log2("*remove schedules client_order_no XREF index", logtime);
	if (listindexes("SCHEDULES", "CLIENT_ORDER_NO.XREF")) {
		execute("DELETEINDEX SCHEDULES CLIENT_ORDER_NO XREF");
	}

	call log2("*add production.invoices client_order_no_PARTS index", logtime);
	if (not(tt.read(DEFINITIONS, "REINDEX*PRODUCTION_INVOICES*CLIENT_ORDER_NO_PARTS"))) {
		tt = "";
	}
	if ((tt.a(1) < 17520) or not listindexes("PRODUCTION_INVOICES", "CLIENT_ORDER_NO_PARTS")) {
		execute("MAKEINDEX PRODUCTION_INVOICES CLIENT_ORDER_NO_PARTS");
		var().date().write(DEFINITIONS, "REINDEX*PRODUCTION_INVOICES*CLIENT_ORDER_NO_PARTS");
	}

	call log2("*add schedules client_order_no_PARTS index", logtime);
	if (not(tt.read(DEFINITIONS, "REINDEX*SCHEDULES*CLIENT_ORDER_NO_PARTS"))) {
		tt = "";
	}
	if ((tt.a(1) < 17520) or not listindexes("SCHEDULES", "CLIENT_ORDER_NO_PARTS")) {
		execute("MAKEINDEX SCHEDULES CLIENT_ORDER_NO_PARTS");
		var().date().write(DEFINITIONS, "REINDEX*SCHEDULES*CLIENT_ORDER_NO_PARTS");
	}

	call log2("*add timesheets date index", logtime);
	if (not(listindexes("TIMESHEETS", "DATE"))) {
		execute("MAKEINDEX TIMESHEETS DATE");
	}

	call log2("*add tasks user_code index", logtime);
	if (not(listindexes("TASKS", "USER_CODE"))) {
		execute("MAKEINDEX TASKS USER_CODE");
	}

	call log2("*add tasks parent_user_code index", logtime);
	if (not(listindexes("TASKS", "PARENT_USER_CODE"))) {
		execute("MAKEINDEX TASKS PARENT_USER_CODE");
	}

	call log2("*add material first_appearance_date index", logtime);
	if (not(listindexes("MATERIALS", "FIRST_APPEARANCE_DATE"))) {
		execute("MAKEINDEX MATERIALS FIRST_APPEARANCE_DATE");
	}

	var filenamesx = "PLANS,SCHEDULES,JOBS,PRODUCTION_ORDERS,PRODUCTION_INVOICES";
	for (var filen = 1; filen <= filenamesx.count(",") + 1; ++filen) {
		var filename = filenamesx.field(",", filen);

		call log2("*add text index to " ^ filename, logtime);
		//warning this could be skipped if the file is large
		//and/or multiple databases in one installation
		//and done manually (maybe sysmsg to instruct support)
		if (not(tt.read(DEFINITIONS, "REINDEX*" ^ filename ^ "*TEXT*XREF"))) {
			tt = "";
		}
		if (not listindexes(filename, "TEXT.XREF") or (tt.a(1) < 18352)) {
			execute("MAKEINDEX " ^ filename ^ " TEXT XREF");
			var().date().write(DEFINITIONS, "REINDEX*" ^ filename ^ "*TEXT*XREF");
		}

	};//filen;

	return "";
}


libraryexit()
