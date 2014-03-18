#include <exodus/library.h>
libraryinit()

#include <log2.h>
#include <renamefile2.h>
#include <initagencyindexes.h>
#include <openfile.h>
#include <openfile.h>
#include <indices2.h>
#include <getreccount.h>
#include <perf.h>
#include <convextras.h>
#include <readagp.h>
#include <authorised.h>
#include <timesheetparamsubs.h>
#include <mediatypesubs.h>
#include <convert.h>
#include <initagency2.h>

#include <gen.h>

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

function main() {
	//jbase

	call log2("*INIT.AGENCY initialisation", logtime);
	var interactive = not SYSTEM.a(33);

	//y2k2

	var reloading = SENTENCE.field(" ", 2);

	call log2("*catalog ad programs is not already done", logtime);
	var progfilename = "ADBP";
	gosub catalog();
	progfilename = "BP";
	gosub catalog();

	//move to init.general
	//call log2('*put the admenus program as the system menu file',logtime)
	//*as there is no way to have multiple menus files
	//perform 'setfile .\adagency global admenus sys.menus'

	//call log2('*convert rate cards to new format 20 columns',logtime)
	call log2("*convert rate cards 1", logtime);
	if (agy.ratecards.open("VEHICLE.RATES", "")) {
		call renamefile2("VEHICLE.RATES", "RATECARDS");
		call note2("CONVERTING RATE CARD FILE TO NEW FORMAT||PLEASE WAIT ...", "UB", buffer, "");
		if (agy.ratecards.open("RATECARDS", "")) {
			agy.ratecards.select();
			var recn = "";
nextrcard:
			var keyx;
			if (keyx.readnext()) {
				var rcard;
				if (rcard.read(agy.ratecards, keyx)) {
					recn += 1;
					cout << var().cursor(32, _CRTHIGH / 2)<< keyx.oconv("L#15");
					cout << recn;
					cout << ". ";
					rcard.r(200, 1);
					for (var fn = 31; fn <= 40; ++fn) {
						var tt = rcard.a(fn);
						rcard.r(fn, rcard.a(fn + 10));
						rcard.r(fn + 10, tt);
					};//fn;
					rcard.write(agy.ratecards, keyx);
				}
				goto nextrcard;
			}
		}
		call msg2("", "DB", buffer, "");
	}

	//call log2('*convert rate cards to new format 40 columns',logtime)
	call log2("*convert rate cards 2", logtime);
	var xx;
	if (not(xx.read(gen._definitions, "RATECARD40COLS"))) {
		var temp = "CONVERTING RATE CARD FILE TO NEW FORMAT||PLEASE WAIT ...";
		if (interactive) {
			call note2(temp, "UB", buffer, "");
		}else{
			cout << temp << endl;
		}
		if (agy.ratecards.open("RATECARDS", "")) {
			var().date().write(gen._definitions, "RATECARD40COLS");
			agy.ratecards.select();
			var recn = "";
nextrcard2:
			var keyx;
			if (keyx.readnext()) {
				var rcard;
				if (rcard.read(agy.ratecards, keyx)) {
					recn += 1;
					cout << var().cursor(32, _CRTHIGH / 2)<< keyx.oconv("L#15");
					cout << recn;
					cout << ". ";
					if (rcard.a(200) == "") {
						rcard.r(200, 1);
						temp = rcard.field(FM, 41, 20);
						rcard = rcard.fieldstore(FM, 81, 20, temp);
						rcard = rcard.fieldstore(FM, 41, 20, "");
						rcard.write(agy.ratecards, keyx);
					}
				}
				goto nextrcard2;
			}
		}
		if (interactive) {
			call msg2("", "DB", buffer, "");
		}
	}

	call log2("*rename QUOTATIONS to PRODUCTION.INVOICE.VERSIONS", logtime);
	if (xx.open("QUOTATIONS", "")) {
		call renamefile2("QUOTATIONS", "PRODUCTION.INVOICE.VERSIONS");
	}

	call log2("*rename ULXML TO IFJOURNALS", logtime);
	if (xx.open("ULXML", "")) {
		call renamefile2("ULXML", "IFJOURNALS");
	}

	call initagencyindexes();

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
	//done below to presize
	//IF OPENFILE2('BOOKING.TEXTS',BOOKING.TEXTS,'BOOKING.ORDERS',1) ELSE VALID=''
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
	//IF OPENFILE2('QUOTATIONS',quotations,'PRODUCTION.INVOICES',1) ELSE VALID=''
	if (not(openfile("PRODUCT.CATEGORIES", productcategories, "BRANDS", 1))) {
		var valid = "";
	}
	if (not(openfile("TASKS", tasks, "JOBS", 1))) {
		var valid = "";
	}

	call log2("*add tasks user_code index", logtime);
	if (not(indices2("TASKS", "USER_CODE"))) {
		var("MAKEINDEX TASKS USER_CODE").execute();
	}

	call log2("*add tasks parent_user_code index", logtime);
	if (not(indices2("TASKS", "PARENT_USER_CODE"))) {
		var("MAKEINDEX TASKS PARENT_USER_CODE").execute();
	}

	call log2("*add material first_appearance_date index", logtime);
	if (not(indices2("MATERIALS", "FIRST_APPEARANCE_DATE"))) {
		var("MAKEINDEX MATERIALS FIRST_APPEARANCE_DATE").execute();
	}

	//reattach in case just been created
	perform("ATTACH ..\\DATA\\" ^ SYSTEM.a(17) ^ "\\ADAGENCY BOOKING.TEXTS (S)");
	var bookingtexts;
	if (not(bookingtexts.open("BOOKING.TEXTS", ""))) {
		var FILES;
		if (FILES.open("FILES", "")) {
			var fileloc;
			if (fileloc.readv(FILES, "BOOKING.ORDERS", 1)) {
				//avgbosize=5000
				//no point to more than 1000 unless make frame size >1000
				//since records will be in overflow anyway
				var avgbosize = 1000;
				var nrecs = getreccount(agy.bookingorders, "", "");
				("MAKEFILE " ^ fileloc ^ " DATA BOOKINGTEXTS " ^ avgbosize ^ " " ^ nrecs ^ " (S)").execute();
				var("RENAMEFILE BOOKINGTEXTS BOOKING.TEXTS").execute();
				perform("CONV.BOOKINGS");
			}
		}
	}

	call log2("*add booking.orders date index", logtime);
	//check after booking.text creation because date index will be rebuilt
	//during creation
	if (not(indices2("BOOKING.ORDERS", "DATE"))) {
		var("MAKEINDEX BOOKING.ORDERS DATE").execute();
	}

	call log2("*add ADS brand vehicle date time index if needed", logtime);
	//import certification will be slow until the next day restart after createads
	//but generally clients will not have an additional index on ads unless required
	var unmatchedads;
	if (unmatchedads.open("UNMATCHED.ADS", "")) {
		if (getreccount(unmatchedads, "", "")) {
			if (not(indices2("ADS", "BRAND_VEHICLE_DATE_TIME"))) {
				var("MAKEINDEX ADS BRAND_VEHICLE_DATE_TIME").execute();
			}
			var tt!!! = "BRAND_AND_DATE,VEHICLE_AND_DATE,DATE";
			for (var ii = 1; ii <= 3; ++ii) {
				var tt2 = tt!!!.field(",", ii);
				call log2("*add UNMATCHED.ADS index " ^ tt2, logtime);
				if (not(indices2("UNMATCHED.ADS", tt2))) {
					var("MAKEINDEX UNMATCHED.ADS " ^ tt2).execute();
				}
			};//ii;
		}
	}

	//clear obsolete vehicle codes from the markets file to speed up
	//lookup of market names over slow links
	if (not(xx.read(gen._definitions, "MARKET_VEHICLE_CODE_CLEARED"))) {
		perform("CLEARFIELD MARKETS VEHICLE_CODE");
		var().date().write(gen._definitions, "MARKET_VEHICLE_CODE_CLEARED");
	}

	call log2("*convert ads 50/51 X to 1");
	var convkey = "CONVERTED*ADS";
	if (not(fin.converted.read(gen._definitions, convkey))) {
		fin.converted = "";
	}
	if (fin.converted.a(1) < var("16 JUN 2010").iconv("D")) {

		//starting maintenance mode for large databases nschedules>nnn
		//provide option to not or not fully convert the ads files
		var nschedules = getreccount(agy.schedules, "", "");
		if (nschedules > 10000) {
			//defaults to option 1 except in interactive mode
decideupg:
			var qq = "Upgrade ADS file?";
			var oo = "Recreate - Slow for large databases";
			oo.r(1, -1, "Patch - Quick to avoid errors but do CREATEADS later.");
			oo.r(1, -1, "No - MUST do CREATEADS later to avoid errors!");
			if (not(decide(qq, oo, reply))) {
				goto decideupg;
			}
		}else{
			reply = 1;
		}

		if (reply == 1) {
			call perf("CREATEADS CREATE (S)");
			goto 1878;
		}
		if (reply == 2) {
			call perf("UPDATE ADS CONV.ADS (U)");
		}else{
			//dont convert
		}
L1878:

		var().date().write(gen._definitions, convkey);
	}

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

	//open 'SUPPLIER.INVOICES' to x then

	// perform 'DELETEFILE SUPPLIER.INVOICES (S)'
	// end

	//!!! ADDMFS SHADOW.MFS newfilenames

	call log2("*add dates to schedules", logtime);
	var nrecs = getreccount(agy.schedules, "", "");
	agy.schedules.select();
	var schn = 0;
	buffer = "";
nextsch:
	var schid;
	if (schid.readnext()) {
		var schedule;
		if (schedule.read(agy.schedules, schid)) {
			schn += 1;
			if (schn < 4) {
				if (schedule.a(10) and schedule.a(25)) {
					goto adddateexit;
				}
				call note2("Updating schedules file.||Please wait ...", "UB", buffer, "");
			}
			cout << var().cursor(35, _CRTHIGH / 2)<< nrecs;
			cout << schn;
			cout << " of ";
			var mth = schedule.a(12).field("/", 1);
			var year = schedule.a(12).field("/", 2);
			var startdate = ("1/" ^ mth ^ "/" ^ year).iconv("D/E");
			mth += 1;
			if (mth > 12) {
				mth = 1;
				year = (year + 1).oconv("R(0)#2");
			}
			var stopdate = ("1/" ^ mth ^ "/" ^ year).iconv("D/E") - 1;
			schedule.r(10, startdate);
			schedule.r(25, stopdate);
			schedule.write(agy.schedules, schid);
		}

		goto nextsch;
	}
adddateexit:
	var().clearselect();
	if (buffer) {
		call note2("", "DB", buffer, "");
	}

	//smdates
	call log2("*convert schedule dates to sm list", logtime);
	var paramkey = "SCHEDULES_DATELIST_CONVERTED";
convertdatelist:
	if (not(lockrecord("DEFINITIONS", gen._definitions, paramkey, recordx, 999999))) {
		cout << "Waiting to lock DEFINITIONS, ";
		cout << paramkey << endl;
		call sleep(1);
		goto convertdatelist;
	}
	if (not(xx.read(gen._definitions, paramkey))) {

		call note2("Converting schedules file date format||Please wait ...", "UB", buffer, "");
		nrecs = getreccount(agy.schedules, "", "");
		agy.schedules.select();
		schn = 0;
		var fns = 22 ^ VM ^ 55 ^ VM ^ 60;
nextsch2:
		if (schid.readnext()) {
			var schedule;
			if (schedule.read(agy.schedules, schid)) {

				var changes = 0;

				schn += 1;
				cout << var().cursor(0)<< " of "<< nrecs;
				cout << var().cursor(-4);
				cout << schn;

				//failsafe in case definitions file is cleared dont double convert
				//also set for new schedules in plans.subs prewrite
				//and checked exists in plan.subs, postread
				if (schedule.a(220)) {
					goto nextsch2;
				}

				schedule.r(220, 1);
				changes = 1;

				for (var fnn = 1; fnn <= 3; ++fnn) {
					var fn = fns.a(1, fnn);
					if ((schedule.a(fn)).index(SVM, 1)) {
						goto nextsch2;
					}
				};//fnn;

				for (var fnn = 1; fnn <= 3; ++fnn) {
					var fn = fns.a(1, fnn);
					var dategrid = schedule.a(fn);
					if (not(dategrid.index(SVM, 1))) {
						var olddategrid = dategrid;

						//make the one character one day string into a one subvalue one day string
						//make spaces data
						dategrid.converter(" ", "~");
						//insert a tm character between each character
						dategrid = dategrid.oconv("T#1");
						//turn the tm characters into sm characters and remove the orig spaces
						dategrid.converter(TM ^ "~ ", SVM);

						if (dategrid ne olddategrid) {
							schedule.r(fn, dategrid);
							changes = 1;
							//print @id:'~':fn:'~':len(olddategrid):'~':olddategrid
							//print @id:'~':fn:'~':len(dategrid):'~':dategrid
						}
					}
				};//fnn;

				if (changes) {
					call cropper(schedule);
					schedule.write(agy.schedules, schid);
				}

			}
			goto nextsch2;
		}
		var().clearselect();
		var().date().write(gen._definitions, paramkey);
		if (buffer) {
			call note2("", "DB", buffer, "");
		}
	}
	call unlockrecord("DEFINITIONS", gen._definitions, paramkey);

	call log2("*convert plans/schedules/invoices/analysis to have 6 extras column", logtime);
	call convextras(logtime);

	call log2("*add internal account numbers", logtime);
	if (xx.open("ACCOUNTS", "")) {
		if (not(xx.read(gen._definitions, "FIXACCNOS"))) {
			perform("FIXACCNOS");
			var().date().write(gen._definitions, "FIXACCNOS");
		}
	}

	//get agp to do various updates to it
	if (not(agy.agp.read(gen._definitions, "AGENCY.PARAMS"))) {
		call note("AGENCY.PARAMS IS MISSING FROM DEFINITIONS");
		var().stop();
	}
	var origagp = agy.agp;

	if (agy.agp.a(1) == "ºª­°ß­¬¼¸ß¯­°²°¯ª½ß®¾«¾­" or agy.agp.a(1) == "ºª­°ß­¬¼¸ß¯­°²°¯ª½ß½¾·­¾¶±") {
		if (var().date() > 13163) {
			call log2("*limit ba hrain and q atar user", logtime);
			if (not(xx.read(gen._definitions, "SU"))) {
				1.write(gen._definitions, "SU");
			}
		}
	}

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

	call log2("*convert invoice audit parameters", logtime);
	var auditno;
	if (auditno.read(agy.invoices, "%SK%")) {
		auditno.write(agy.invoices, "%MEDIA.NO%");
		agy.invoices.deleterecord("%SK%");
	}
	if (auditno.read(agy.invoices, "%SK2%")) {
		auditno.write(agy.invoices, "%MEDIA.NO2%");
		agy.invoices.deleterecord("%SK2%");
	}
	var auditindex;
	if (auditindex.read(agy.invoices, "%AUDIT%")) {
		auditindex.write(agy.invoices, "%MEDIA%");
		agy.invoices.deleterecord("%AUDIT%");
	}
	if (auditindex.read(agy.invoices, "%AUDIT.NO%")) {
		auditindex.write(agy.invoices, "%MEDIA.AUDITNO%");
		agy.invoices.deleterecord("%AUDIT.NO%");
	}

	call log2("*create NONE supplier");
	var supplier;
	if (not(supplier.read(agy.suppliers, "NONE"))) {
		supplier = "No Cost";
		supplier.r(13, "MP");
		supplier.write(agy.suppliers, "NONE");
	}

	call log2("*convert supplier file", logtime);
	agy.suppliers.select();
nextsupplier:
	var supplierkey;
	if (supplierkey.readnext()) {
		if (supplier.read(agy.suppliers, supplierkey)) {
			if (supplier.a(13)) {
				var().clearselect();
			}else{
force error here TODO: check trigraph following;
				supplier = supplier ? (13) : (0.replace(0, if (supplier.a(8)) {, "M", "P"));
				supplier.write(agy.suppliers, supplierkey);
			}
		}
		goto nextsupplier;
	}
	var().clearselect();

	call log2("*clear unused brands", logtime);
	if (not(xx.read(gen._definitions, "ORPHANBRANDSFIXED"))) {
		agy.brands.select();
nextbrandcode:
		var brandcode;
		if (brandcode.readnext()) {
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
	convkey = "BUILDCLIENTGROUPMEMBERS";
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

	call log2("*fix client 29 in 19 corruption", logtime);
	convkey = "FIXCLIENT29IN19";
	if (lockrecord("DEFINITIONS", gen._definitions, convkey, recordx, 999999)) {
		if (not(xx.read(gen._definitions, convkey))) {
			perform("WINDOWSTUB CLIENT.SUBS " ^ convkey);
			var().date().write(gen._definitions, convkey);
		}
		call unlockrecord("DEFINITIONS", gen._definitions, convkey);
	}

	var datasetcode = SYSTEM.a(17);

	call log2("*detach sales and purchases dictionaries*", logtime);
	if (xx.open("DICT", "SALES")) {
		perform("DETACH DICT.SALES (S)");
	}
	if (xx.open("DICT", "PURCHASES")) {
		perform("DETACH DICT.PURCHASES (S)");
	}

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

	if (not(authorised("PLAN CREATE OWN NO", "", "PLAN CREATE OWN NUMBER"))) {
		{}
	}
	if (not(authorised(deletex ^ "PLAN CREATE OWN NUMBER", msg))) {
		{}
	}
	if (not(authorised("SCHEDULE CREATE OWN NO", "", "SCHEDULE CREATE OWN NUMBER"))) {
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
	if (not(authorised("CLIENT CREATE STOPPED"))) {
		{}
	}
	if (not(authorised("SCHEDULE UPDATE WEEK"))) {
		{}
	}
	if (not(authorised("SCHEDULE UPDATE MATERIAL", "", "SCHEDULE UPDATE PLAN"))) {
		{}
	}
	if (not(authorised("SCHEDULE UPDATE CERTIFIED", "", "SCHEDULE UPDATE AFTER INVOICE"))) {
		{}
	}
	if (not(authorised("SCHEDULE UPDATE SUPPLIER INVOICED", "", "SCHEDULE UPDATE AFTER INVOICE"))) {
		{}
	}

	if (not(authorised("MENU MEDIA"))) {
		{}
	}
	if (not(authorised("MENU JOBS"))) {
		{}
	}
	if (not(authorised("MENU ANALYSIS"))) {
		{}
	}
	if (not(authorised("MENU TIMESHEETS"))) {
		{}
	}

	//change blank to UM2
	var tt!!! = "SCHEDULE UPDATE BRAND AFTER BOOKING";
	tt!!!.r(-1, "SCHEDULE UPDATE BRAND AFTER INVOICE");
	for (var ii = 1; ii <= 2; ++ii) {
		if (gen._security.a(10).locateusing(tt!!!.a(ii), VM, ii2)) {
			if (gen._security.a(11, ii2) == "") {
				call authorised(deletex ^ tt!!!.a(ii));
			}
		}
		if (not(authorised(tt!!!.a(ii), msg, "UM2"))) {
			{}
		}
	};//ii;

	if (not(authorised("SCHEDULE UPDATE COMPANY AFTER BOOKING", "", "NEOSYS"))) {
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
		tt!!! = var("MEDIA,PRODUCTION").field(",", ii);
		if (not(authorised(tt!!! ^ " INVOICE DESPATCH", msg, ""))) {
			{}
		}
		if (not(authorised(tt!!! ^ " INVOICE UNDESPATCH", msg, ""))) {
			{}
		}
		if (not(authorised(tt!!! ^ " INVOICE APPROVE", msg, ""))) {
			{}
		}
		if (not(authorised(tt!!! ^ " INVOICE UNAPPROVE", msg, ""))) {
			{}
		}
	};//ii;

	if (not(authorised(deletex ^ "SUPPLIER INVOICE ACCESS"))) {
		{}
	}
	if (not(authorised(deletex ^ "SUPPLIER INVOICE CREATE"))) {
		{}
	}
	if (not(authorised(deletex ^ "SUPPLIER INVOICE UPDATE"))) {
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
	if (not(authorised(deletex ^ "RATECARD OVERRIDE"))) {
		{}
	}

	//these can be deleted after a handful of database are fixed
	if (not(authorised(deletex ^ "JOB MONITOR OPEN"))) {
		{}
	}
	if (not(authorised(deletex ^ "JOB MONITOR CLOSED"))) {
		{}
	}
	if (not(authorised(deletex ^ "PURCHASE ORDER MONITOR DRAFT"))) {
		{}
	}
	if (not(authorised(deletex ^ "PURCHASE ORDER MONITOR ISSUED"))) {
		{}
	}
	if (not(authorised(deletex ^ "PURCHASE ORDER MONITOR CANCELLED"))) {
		{}
	}
	if (not(authorised(deletex ^ "PURCHASE ORDER MONITOR INVOICE"))) {
		{}
	}
	if (not(authorised(deletex ^ "ESTIMATE MONITOR DRAFT"))) {
		{}
	}
	if (not(authorised(deletex ^ "ESTIMATE MONITOR ISSUED"))) {
		{}
	}
	if (not(authorised(deletex ^ "ESTIMATE MONITOR APPROVED"))) {
		{}
	}
	if (not(authorised(deletex ^ "ESTIMATE MONITOR CANCELLED"))) {
		{}
	}
	if (not(authorised(deletex ^ "ESTIMATE MONITOR DELIVERED"))) {
		{}
	}
	if (not(authorised(deletex ^ "ESTIMATE MONITOR INVOICED"))) {
		{}
	}

	//use TIMESHEET ADMINISTRATION
	//if security('TIMESHEET CONFIGURATION UPDATE','','TIMESHEET ADMINISTRATION') else null

	call log2("*delete wrong plural tasks", logtime);
	if (not((tt!!!).readv(gen._definitions, "DELETEWRONGPLURALTASKS", 1))) {
		tt!!! = "";
	}
	if (tt!!! < 16033) {
		tt!!! = "BRANDS,JOBS,PLANS,SCHEDULES,PRODUCTION ORDERS,PRODUCTION INVOICES,PRODUCTION ESTIMATES,TIMESHEETS";
		tt!!!.converter(",", VM);
		var nn = (tt!!!).count(VM) + 1;
		for (var ii = 1; ii <= nn; ++ii) {
			if (not(authorised(deletex ^ tt!!!.a(1, ii) ^ " LIST"))) {
				{}
			}
		};//ii;
		var().date().write(gen._definitions, "DELETEWRONGPLURALTASKS");
	}

	call log2("*delete obsolete tasks", logtime);
	//remove "QUOTE"
	if (not(authorised(deletex ^ "PRODUCTION ESTIMATE CREATE OWN QUOTE NO"))) {
		{}
	}
	//replaced with "AMEND" with "UPDATE"
	if (not(authorised(deletex ^ "PRODUCTION ORDER AMEND AFTER JOB CLOSED", msg, ""))) {
		{}
	}

	call log2("*create default timesheet parameters", logtime);
	var rec;
	if (not(rec.read(gen._definitions, "TIMESHEET.PARAMS"))) {
		var("N" ^ FM ^ 0 ^ FM ^ 16 ^ FM ^ 3 ^ FM ^ 0 ^ FM ^ "DEFAULTED").write(gen._definitions, "TIMESHEET.PARAMS");
	}

	call log2("*setup timesheet reminder/approval autorun");
	call timesheetparamsubs("SETUPAUTORUN");

	call log2("*update list of bill/cost account numbers*", logtime);
	//once a day update the list of income and cost accounts
	//used to warn/prevent manual journal entries on these accounts
	//so that the operational analysis reports agree with the financial statements
	//also updated whenever any type file record is updated or deleted
	//should also be updated whenever file is loaded/cleared during implementation
	if (not((tt!!!).read(gen._definitions, "BILLCOSTACCS"))) {
		tt!!! = "";
	}
	if (tt!!!.a(6) <= 16416) {
		call mediatypesubs("UPDATEBILLCOSTACCS");
	}

	call log2("*create the initial client account inverted index");
	if (not(rec.read(gen._definitions, "CLIENTACNOINDEX"))) {
		perform("LISTCOLLECTIONS UPDATEINDEX");
		var().date().write(gen._definitions, "CLIENTACNOINDEX");
	}

	call log2("*add languages to media type booking text");
	perform("ADDMEDIATYPELANGS");

	call log2("*add executive to vouchers analysis codes");
	if (not(rec.read(gen._definitions, "CONVERSION*ADDEXEC"))) {
		perform("ADDEXEC (U)");
		var().date().write(gen._definitions, "CONVERSION*ADDEXEC");
	}

	call log2("*autoconfigure TV certification codes before 9 DEC 2010");
	var tv;
	if (tv.read(agy.mediatypes, "TV")) {
		if (tv.a(20) == "" and (tv.a(11)).floor() <= 15685) {
			tv.r(20, .convert(", VM, "OK,(DNA),(ERROR),(DURATION),(COPY),(PLACEMENT)"));
			tv.r(21, .convert(", VM, "Aired/TIME/comment,Did not Air//comment,Error/time/COMMENT,Wrong Duration/TIME/COMMENT,Wrong Copy/TIME/COMMENT,Wrong Placement/TIME/COMMENT"));
			tv.write(agy.mediatypes, "TV");
		}
	}

	call log2("*join certain broken lines of old booking notes", logtime);
	var t2 = "forwarded to us with your invoice.";
	agy.mediatypes.select();
nextmediatype:
	var ID;
	if (ID.readnext()) {
		if (rec.read(agy.mediatypes, ID)) {
			if (rec.a(3) ne "Y") {
				if (rec.a(11) <= 16014) {
					rec.swapper(VM ^ var(9).space() ^ t2, " " ^ t2);
					rec.swapper(VM ^ t2, " " ^ t2);
					rec.write(agy.mediatypes, ID);
				}else{
					//skip out if any rec is recent
					var().clearselect();
				}
			}
		}
		goto nextmediatype;
	}

	call log2("*autoconfigure \"ratecards may have loading\"", logtime);
	if (agy.agp.a(112) == "") {
		var().clearselect();
		perform("SELECT RATECARDS WITH LOADING_DESCRIPTION (S)");
		if (LISTACTIVE) {
			agy.agp.r(112, 1);
		}else{
			agy.agp.r(112, 0);
		}
		(agy.agp.a(112)).writev(gen._definitions, "AGENCY.PARAMS", 112);
		var().clearselect();
	}

	call log2("*fix booking orders", logtime);
	if (not((tt!!!).readv(gen._definitions, "FIX*BOOKINGS", 1))) {
		tt!!! = "";
	}
	if (tt!!! < 16195) {
		perform("UPDATE BOOKING.ORDERS FIX.BOOKINGS (U)");
		var().date().write(gen._definitions, "FIX*BOOKINGS");
	}

	call log2("*do any pending schedules and program updates*", logtime);
	perform("PENDINGUPDATES");

	call initagency2(logtime, reloading);

	goto exit;

}

subroutine catalog() {
	var progfile;
	if (not(progfile.open(progfilename, ""))) {
		return;
	}

	var voc;
	if (not(voc.open("VOC", ""))) {
		return;
	}

	var records;
	if (not(records.read(progfile, "%RECORDS%"))) {
		return;
	}
	var lentail = (field2(records, "$", -1)).length();
	while (true) {
		if (lentail and records.substr(-lentail, 1) ne FM) {
			lentail -= 1;
		}//loop;
	}
	records.splicer(-lentail, lentail, "");
	records.converter(FM ^ "$", " ");
	var records2;
	if (not(records2.read(voc, "%" ^ progfilename ^ ".CATALOG%"))) {
		records2 = "";
	}
	if (records ne records2) {
		records.write(voc, "%" ^ progfilename ^ ".CATALOG%");
		perform("MAKEVOC " ^ progfilename ^ " " ^ records ^ " (SO)");
	}

	return;

exit:
	call log2("*quit INIT.AGENCY", logtime);

}


libraryexit()