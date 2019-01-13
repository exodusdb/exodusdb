#include <exodus/library.h>
libraryinit()

#include <sysmsg.h>
#include <quote2.h>
#include <initcompany.h>
#include <authorised.h>
#include <safeselect.h>
#include <agencysubs.h>
#include <readagp.h>
#include <nextkey.h>
#include <singular.h>
#include <btreeextract.h>
#include <validcode3.h>
#include <validcode2.h>
#include <generalsubs.h>
#include <giveway.h>
#include <suppliersubs.h>
#include <getreccount.h>

#include <agy.h>
#include <gen.h>
#include <win.h>

#include <window.hpp>

var allownew;
var iaccno;
var billcostn;//num
var xx;
var brandcode;
var vehicle;
var doccompanycode;
var docdate;
var docname;
var doccompany;
var filen;
var companyx;
var filetitle;
var temp;
var foundkeys;
var module;
var username;
var reqcompcode;
var versionfile;
var reply;//num
var saver;
var saveid;
var savedict;
var type;
var mode3;
var saverecord;
var jobnos;
var clientcode;
var reqtype;
var title;
var typerec;
var hasaccounts;
var wsmsg;

function main(in mode, io msg, in param3="", in param4="") {
	//c agy in,io,"",""

	if (param4.unassigned()) {
		allownew = "";
	}else{
		allownew = param4;
	}

	//y2k2
	if (mode.field("*", 1) == "CHECKANALYSISCODE") {

		//validates an analysis code - used by finance module daybook.subs2
		//return msg if wrong or no msg if ok

		var accno = mode.field("*", 2);

		//analysiscode=field(mode,'*',3,9999)
		var analysiscode = param3;

		if (accno.a(1, 1, 2)) {
			iaccno = accno.a(1, 1, 2);
		}else{
			iaccno = accno.a(1, 1, 1).field(",", 1);
		}

		//can only check versus account number
		if (accno == "") {
			msg = "Please enter the account number before entering the analysis code";
			return 0;
		}

		msg = "Analysis code " ^ (DQ ^ (analysiscode ^ DQ)) ^ " is invalid for account " ^ (DQ ^ (iaccno.oconv("[ACNO]") ^ DQ)) ^ ".";

		//open 'DEFINITIONS' to definitions else
		// call fsmsg()
		// stop
		// end
		var billcostaccs;
		if (not(billcostaccs.read(DEFINITIONS, "BILLCOSTACCS"))) {
			msg = "System Error: BILLCOSTACCS is missing in AGENCY.SUBS";
			call sysmsg(msg);
			return 0;
		}

		//check if the account requires analysis
		if (not billcostaccs.a(2).locateusing(iaccno, VM, billcostn)) {
			billcostn = 0;
			var accounts;
			if (not(accounts.open("ACCOUNTS", ""))) {
				call fsmsg();
				var().stop();
			}
			var icontrolaccno;
			if (icontrolaccno.readv(accounts, iaccno, 6)) {
				if (icontrolaccno) {
					if (not billcostaccs.a(2).locateusing(icontrolaccno, VM, billcostn)) {
						billcostn = 0;
					}
				}
			}
			if (not billcostn) {
				if (analysiscode == "") {
					//ok
					msg = "";
					return 0;
				}
				msg.r(-1, "Account " ^ (DQ ^ (iaccno.oconv("[ACNO]") ^ DQ)) ^ " cannot have an analysis code");
				return 0;
			}
		}
		if (not analysiscode) {
			msg = "Analysis code is required for account " ^ (DQ ^ (iaccno.oconv("[ACNO]") ^ DQ));
			return 0;
		}

		//msg:=fm:fm:'Analysis code field '
		msg ^= FM ^ FM ^ "Analysis ";

		//analysis field number
		var tt = billcostaccs.a(3, billcostn);
		if (not(tt.locateusing(analysiscode.field("*", 1), SVM, xx))) {
			//could be BILL and COST for some accounts
			if (tt.index(SVM, 1)) {
				tt.swapper(28, "28 for Bill");
				tt.swapper(29, "29 for Cost");
				tt.swapper(30, "30 for Accrued Cost");
				tt.swapper(31, "31 for WIP");
				tt.swapper(SVM, " or ");
				msg ^= "1 must be " ^ tt;
				return 0;
			}else{
				//force it if only one
				analysiscode = analysiscode.fieldstore("*", 1, 1, tt);
			}
		}

		//column (discount/charge) number
		var colno = analysiscode.field("*", 2);
		if (not colno) {
			goto field2err;
		}
		tt = billcostaccs.a(7, billcostn);
		//income sharing seems to be missing column number in BILLCOSTACCS
		//probably should be 1 (or 9 like rebate)
		if (tt) {
			if (not tt.locateusing(colno, SVM, xx)) {
field2err:
				msg ^= " column is required and must be one of ";
				msg ^= DQ ^ (tt ^ DQ);
				//msg<-1>='1=Gross'
				//msg<-1>='2=Loading'
				//msg<-1>='3=Discount'
				//msg<-1>='4=Commission'
				//msg<-1>='5=Fee'
				//msg<-1>='6=Tax'
				//msg<-1>='7=Other'
				//msg<-1>='8=Net'
				//msg<-1>='9=AVR'
				return 0;
			}
		}

		//brandcode - analysis field 3
		brandcode = analysiscode.field("*", 3);
		if (not brandcode) {
			msg ^= "brand is required";
			return 0;
		}
		if (not xx.read(agy.brands, brandcode)) {
			msg ^= DQ ^ (brandcode ^ DQ) ^ " brand code does not exist";
			return 0;
		}

		//media verification - analysis field 4
		var vehiclecode = analysiscode.field("*", 4);
		if (vehiclecode) {

			//check account allows media
			if (not billcostaccs.a(5, billcostn).locateusing(1, SVM, xx)) {
				//could force jobs here but best to indicate some error since unexpected
				msg ^= "media/vehicle code must not be entered";
				return 0;
			}

			//vehiclecode
			if (not vehicle.read(agy.vehicles, vehiclecode)) {
				msg ^= DQ ^ (vehiclecode ^ DQ) ^ " vehicle code does not exist";
				return 0;
			}

			//check vehicle media type
			var accjobtypes = billcostaccs.a(4, billcostn);
			//allow for unlikely case that we have an account that is allowed
			//for all types like income sharing? not a great idea to have that
			if (accjobtypes) {
				if (not accjobtypes.locateusing(vehicle.a(2), SVM, xx)) {
					msg ^= DQ ^ (vehiclecode ^ DQ) ^ " vehicle media type " ^ (DQ ^ (vehicle.a(2) ^ DQ));
					msg ^= " does not match account media type(s) " ^ quote2(accjobtypes);
					return 0;
				}
			}

			//verify no job analysis fields if media vehicle is present
			tt = analysiscode.field("*", 5, 3);
			tt.converter("*", "");
			if (tt.length()) {
				//could force no fields here but best to indicate some error since unexpected
				msg ^= "fields 5-7 must be empty";
				return 0;
			}

			//nonmedia verification
		}else{

			//check account allows jobs - analysis field 4
			if (not billcostaccs.a(5, billcostn).locateusing(2, SVM, xx)) {
				//TODO we should try and avoid an account being used for both media and jobs
				msg ^= "media/vehicle is required";
				return 0;
			}

			//marketcode - analysis field 5
			var marketcode = analysiscode.field("*", 5);
			if (not marketcode) {
				msg ^= "market is required";
				return 0;
			}
			if (not xx.read(agy.markets, marketcode)) {
				msg ^= DQ ^ (marketcode ^ DQ) ^ " market code does not exist";
				return 0;
			}

			//suppliercode - analysis field 6
			var suppliercode = analysiscode.field("*", 6);
			if (not suppliercode) {
				msg ^= "supplier code is required";
				return 0;
			}
			if (not xx.read(agy.suppliers, suppliercode)) {
				msg ^= DQ ^ (suppliercode ^ DQ) ^ " supplier code does not exist";
				return 0;
			}

			//type exists - analysis field 7
			var typecode = analysiscode.field("*", 7);
			if (not typecode) {
				msg ^= "job type is required";
				return 0;
			}
			if (not xx.read(agy.jobtypes, typecode)) {
				msg ^= DQ ^ (typecode ^ DQ) ^ " job type code does not exist";
				return 0;
			}

			//type code limitation
			tt = billcostaccs.a(4, billcostn);
			//is it possible that there are no types associated with an account?
			if (tt) {
				if (not tt.locateusing(typecode, SVM, xx)) {
					msg ^= " type (" ^ typecode ^ ") must be ";
					if (tt.index(SVM, 1)) {
						tt.swapper(SVM, ", ");
						msg ^= "one of " ^ tt;
					}else{
						msg ^= DQ ^ (tt ^ DQ);
					}
					return 0;
				}
			}

		}

		//indicate analysis code is all ok
		msg = "";

	} else if (mode.field(".", 1) == "CHKCLOSEDPERIOD") {

		//prevent updating in closed period

		//similar code in booking.subs, certify, printplans7 and prodinvs2

		//called in postread/2 and prewrite/2 and predelete
		//of plan.subs, job.subs, prodorder.subs and prodinv.subs
		//a similar check is done in CERTIFY for ads
		//postread will make any old documents readonly
		//prewrite will prevent any new documents or recent documents getting old dates
		//predelete is checked too but actually all docs are prevented from being
		//deleted now in order to preserve history
		//zzz maybe add authorisation to work before closed period

		msg = "";

		//in postread if new record then nothing to check
		if (not RECORD) {
			return 0;
		}

		var mode2 = mode.field(".", 2);

		if (win.datafile == "TIMESHEETS") {
			doccompanycode = (calculate("COMPANY_CODE")).a(1, 1);
			docdate = ID.field("*", 2);
			docname = "timesheet";

		} else if (win.datafile == "PLANS") {
			doccompanycode = RECORD.a(187);
			//check oldest period on the plan
			//docdate=iconv('1/':@record<12>,'D2/E')
			//check LATEST date on plan (so they can work on multimonth plans)
			//TODO closed periods should be locked in the plan data entry screen
			docdate = var("28/" ^ RECORD.a(15)).iconv("D2/E");
			docname = "plan";

		} else if (win.datafile == "SCHEDULES") {
			doccompanycode = RECORD.a(187);
			//check oldest date on the schedule
			//docdate=@record<10>
			//check LATEST date on schedule (so they can work on multimonth schedules)
			//closed dates are locked in the schedule calendar popup
			docdate = RECORD.a(25);
			docname = "schedule";

		} else if (win.datafile == "JOBS") {
			doccompanycode = RECORD.a(14);
			docdate = var("1/" ^ RECORD.a(1)).iconv("D2/E");
			docname = "job";

		} else if (win.datafile == "PRODUCTION_ORDERS") {
			doccompanycode = RECORD.a(2).xlate("JOBS", 14, "X");

			//synchronise backdateamendments in agency.subs and prodorder.subs
			var backdateamendments = 0;
			if (backdateamendments) {
				docdate = win.orec.a(9);
			}else{
				docdate = RECORD.a(9);
			}
			if (docdate) {
				docname = "production supplier invoice";
			}else{
				docname = "production order";

				//prevent even adding invoice numbers to old production orders
				//for safety
				docdate = RECORD.a(1);
				// end
			}
			goto getolderjobdate;

		} else if (win.datafile == "PRODUCTION_INVOICES") {
			doccompanycode = RECORD.a(2).xlate("JOBS", 14, "X");
			call initcompany(doccompanycode);
			docdate = RECORD.a(1);
			docname = "production estimate";

			//prevent adding/changing to jobs with old periods
			//maybe move the job period forward in this case or unclose temporarily
getolderjobdate:
			var jobdate = ("1/" ^ RECORD.a(2).xlate("JOBS", 1, "X")).iconv("D2/E");
			if (jobdate < docdate) {
				docdate = jobdate;
				docname = "job";
			}

		} else {
			msg = DQ ^ (win.datafile ^ DQ) ^ " is invalid in AGENCY.SUBS " ^ mode;
			gosub invalid(msg);
			return 0;
		}
//L1732:
		if (doccompanycode and docdate) {
			if (not doccompany.read(gen.companies, doccompanycode)) {
				msg = DQ ^ (doccompanycode ^ DQ) ^ " company code is missing";
				win.reset = 5;
				return invalid(msg);
			}
			var accops = "operations";
			var closedperiod = doccompany.a(37);
			if (not closedperiod) {
				closedperiod = doccompany.a(16);
				accops = "accounts";
			}
			var closedate = closedperiod.iconv("[DATEPERIOD," ^ doccompany.a(6) ^ "]");
			if (docdate <= closedate) {

				var tt = "You cannot update this " ^ docname ^ " dated " ^ docdate.oconv("[DATE,*4]") ^ " because";
				tt.r(-1, doccompany.a(1) ^ " " ^ accops ^ " are closed up to " ^ closedate.oconv("[DATE,*4]"));
				tt.r(-1, "(See Company File)");

				if (authorised(docname ^ " UPDATE AFTER CLOSING", msg, "NEOSYS")) {
					tt.swapper("cannot", "should NOT");
					msg = "WARNING! " ^ tt;
				}else{
					msg = tt ^ FM ^ FM ^ msg;
				}

			}
		}

	} else if (mode == "PREREAD") {

		var compcode = "";

		if (ID == "") {
			compcode = win.templatex.a(1);

		} else if (ID.match("1A0A")) {

			//find with specific company code2 first (ignore blank company code2)
			var().clearselect();
			call safeselect("SELECT COMPANIES WITH COMPANY_CODE2 " ^ (DQ ^ (ID ^ DQ)) ^ " AND WITH AUTHORISED (S)");

			if (readnext(compcode)) {
				var().clearselect();

			}else{

				//default with ANY starting letter(s)
				compcode = ID;

			}

		}
//L2085:
		if (compcode) {
			call agencysubs("GETNEXTID." ^ compcode, xx);
			win.isdflt = ID;
		}

	//from plan.subs5 def.ref
	} else if (mode.substr(1,10) == "GETNEXTID.") {

		var compcode = mode.field(".", 2);
		var previous = mode.field(".", 3);

		if (compcode and compcode ne gen.gcurrcompany) {
			//they might provide a prefix and not a real company code
			var comp;
			if (comp.read(gen.companies, compcode)) {
				call initcompany(compcode);
			}
		}

		call readagp();

		if (not((var("PLANS,SCHEDULES,JOBS,PRODUCTION_ORDERS,PRODUCTION_INVOICES").a(1)).locateusing(win.datafile, ",", filen))) {
			msg = "File " ^ (DQ ^ (win.datafile ^ DQ)) ^ " is invalid in AGENCY.SUBS,GETNEXTID";
			return invalid(msg);
		}
		//agp<71>
		//agp<63>
		//agp<53>
		//agp<69>
		//agp<70>
		var keyformat = agy.agp.a(var("71,63,53,69,70").field(",", filen));

		//if single letter and no complex pattern then ALLOW ANY X99999
		if ((keyformat == "" or keyformat == "<NUMBER>") and ID.match("1A\"*\"")) {
			keyformat = ID[1] ^ "%<>";
		}

		//if index(keyformat,'<',1) else
		// *SK2 (not SK) to prevent endless loop when GENERAL.SUBS calls this routine
		// call GENERAL.SUBS('DEF.SK2')
		// @ans=is.dflt
		// return
		// end

		//determine company prefix
		if (compcode == "") {
			compcode = gen.gcurrcompany;
		}
		if (not companyx.read(gen.companies, compcode)) {
			//a company prefix might be provided and not a real company code
			//msg=quote(compcode):' IS NOT IN COMPANY FILE'
			//gosub note
			companyx = "";
		}
		if (companyx.a(28) == "") {
			companyx.r(28, compcode);
		}

		//prevent numeric company numbers being used as <COMPANY>
		//to avoid <COMPANY><NUMBER> being merged numbers
		if (keyformat.index("<COMPANY>", 1)) {
			if ((companyx.a(28)).isnum()) {
				msg = "Please setup Company Prefix for company " ^ (DQ ^ (compcode ^ DQ)) ^ " in Company File first";
				//goto invalid
				//too deep to call msg()
				USER4 = msg;
				win.valid = 0;
				return 0;
			}
		}

		//similar code in AGENCY.SUBS,PRINTPLANS2,PRODINVS2,REPRINTINVS
		keyformat.swapper("<YEAR>", ((var().date()).oconv("D")).field(" ", 3));
		keyformat.swapper("<YEAR2>", (((var().date()).oconv("D")).field(" ", 3)).substr(-2,2));
		keyformat.swapper("<COMPANY>", companyx.a(28));
		keyformat.swapper("<NUMBER>", "%");
		keyformat.swapper("<>", "");

		var keyfilename = win.datafile;

		var seqkeyfilename = "DEFINITIONS";
		var seqkeyformat = "";
		if (seqkeyformat == "") {
			seqkeyformat = keyformat;
			seqkeyformat.converter("%", "");
		}
		if (seqkeyformat ne "") {
			seqkeyformat.splicer(1, 0, ".");
		}
		seqkeyformat = keyfilename ^ ".SK" ^ seqkeyformat;

		//get the next key
		var params = keyfilename ^ ":" ^ seqkeyformat ^ ":" ^ seqkeyfilename ^ ":" ^ keyformat;
		ID = nextkey(params, previous);

	} else if (mode == "DEF.CURRENCY") {
		if (win.is == "") {
			ANS = (calculate("MARKET_CODE")).xlate("MARKETS", 5, "X");
		}

	} else if (mode == "VAL.SCHEDULE" or mode == "VAL.PLAN" or mode == "VAL.JOB" or mode == "VAL.PRODUCTION.ORDER" or mode == "VAL.PRODUCTION.INVOICE") {

		if (mode == "VAL.PRODUCTION.ORDER") {
			filetitle = "Purchase Orders";

		} else if (mode == "VAL.PRODUCTION.INVOICE") {
			filetitle = "Estimates";

		} else {
			filetitle = mode.field(".", 2) ^ "s";
		}
//L2767:
		//no search if version key
		if (win.is.index("~", 1)) {
			return 0;
		}

		win.datafile = mode.field(".", 2, 9) ^ "S";

		if (not(win.srcfile.open(win.datafile, ""))) {
			call fsmsg();
			var().stop();
		}

		//no search if exists
		var tt;
		if (tt.read(win.srcfile, win.is)) {
			return 0;
		}

		//no search if in versions file
		//eg production.orders/invoices MO25027A
		if (allownew) {
			var versionsfile;
			if (versionsfile.open(singular(win.datafile) ^ "_VERSIONS", "")) {
				if (xx.read(versionsfile, win.is)) {
					//change src.file so that AGENCYPROXY will find it
					win.srcfile = versionsfile;
					return 0;
				}
			}
		}

		if (not(win.srcdict.open("DICT." ^ win.datafile, ""))) {
			call fsmsg();
			var().stop();
		}

		//allowing mv to indicate alternative searches

		//analogous code whereever allpunctuation is used
		//ED BP LISTSCHED AGENCY.SUBS SCHEDULES.DICT
		//ED GBP BTREE.EXTRACT2
		temp = win.is;
		var allpunctuation = SYSTEM.a(130);
		temp.converter(allpunctuation, var(50).space());
		temp.trimmer();

		temp.swapper(" ", "&");
		//if 1 then
		temp.swapper("&", "]&");
		temp.swapper(VM, "]" ^ VM);
		temp ^= "]";
		// end

		temp = "TEXT.XREF" ^ VM ^ temp ^ FM;
		//call btree.extract2(temp,datafile,src.dict,foundkeys,msg)

		call btreeextract(temp, win.datafile, win.srcdict, foundkeys);

		//The key list has exceeded the maximum allowed length.
		//The list will be truncated.
		//Do you want to continue? [Y/N]
		//if index(@user4,'maximum',1) then @user4=''
		//if msg then
		// goto invalid
		// end
		//suppress maximum message
		//similar code in LISTSCHED and AGENCY.SUBS
		if (USER4.index("maximum", 1)) {
			USER4 = "Too many records found. Some may have been excluded.";
		}

		if (not foundkeys) {

			//maybe a new key
			//if not(index(is,' ',1)) and len(is)<3 then
			if (not win.is.index(" ", 1)) {
				ANS = win.is;

				//trigger in the client
				msg = "cannot be found - create new";
				return invalid(msg);

			}

			//"cannot be found" is a trigger in the client

			if (win.datafile == "PLANS" or win.datafile == "SCHEDULES") {
				module = "MEDIA_MODULE";
			}else{
				module = "JOBS_MODULE";
			}
			msg = "";
			msg.r(-1, "<div align=left>");
			msg.r(-1, "<h3>No " ^ filetitle.lcase() ^ " found for " ^ win.is ^ "</h3>");

			msg.r(-1, FM ^ "The following fields have been searched:");

			msg.r(-1, "Brand name, Client name, Brand group, Client group, Brand code, Client code");

			if (module == "MEDIA_MODULE") {
				msg.r(-1, "Campaign, Schedule/Plan No., Client Order No.");
			}

			msg.r(-1, "Executive, Attention");
			msg.r(-1, "Start month, Start year, Market name and code");

			if (module == "JOBS_MODULE") {
				msg.r(-1, "Description, Job No., Estimate No.");
			}

			if (win.datafile == "JOBS") {
				msg ^= ", Invoice No., Brief, Purchase Order No.";

			} else if (win.datafile == "PRODUCTION_ORDERS") {
				msg.r(-1, "Purchase Order No., Our Invoice No.");
				msg.r(-1, "Supplier Invoice No., Supplier Name, Supplier Code");
				msg.r(-1, "Status: Draft, Issued, Cancelled, Invoice");

			} else if (win.datafile == "PRODUCTION_INVOICES") {
				msg.r(-1, "Client Order No., Invoice No.");
				msg.r(-1, "Status: Draft, Issued, Approved, Cancelled, Delivered, Invoiced");

			} else if (module == "MEDIA_MODULE") {
				msg.r(-1, "Media vehicle names and codes");
				msg.r(-1, "Notes, Private comments, Invoice notes");
				msg.r(-1, "Material code, Material description/Special instructions");
				msg.r(-1, "Status: NOTAPPROVED, NOTBOOKED, NOTINVOICED, OPEN, CLOSED");
			}
//L3464:
			//jobs, production orders and invoices
			if (module == "JOBS_MODULE") {
				msg.r(-1, "Status: Open, Closed");
			}

			msg.r(-1, FM ^ "1. To search for SOMETHING SPECIAL in period 1/2018, enter SOME SPEC 1801");
			if (USERNAME == "NEOSYS") {
				username = "NEOSYS";
			}else{
				username = (USERNAME.xlate("USERS", 1, "C")).field(" ", 1);
				username.ucaser();
			}

			if (win.datafile == "JOBS") {
				msg.r(-1, "2. Searching for OPEN " ^ username.substr(1,3) ^ " will open jobs for executive " ^ username);

			} else if (module == "MEDIA_MODULE") {
				msg.r(-1, "2. Searching for NOTAPP 1801 " ^ username.substr(1,3) ^ " will find documents pending approval" ^ FM ^ "&nbsp;&nbsp;&nbsp;&nbsp;from 1/2018 for executive/attention of " ^ username);

			} else {
				msg.r(-1, "2. Searching for OPEN " ^ username.substr(1,3) ^ " will find open documents for executive/attention of " ^ username);
			}

			msg.r(-1, "3. For quicker searching, use rarer words.");

			msg.r(-1, "</div>");
			return invalid(msg);
		}

		foundkeys.converter(VM, FM);
		foundkeys.transfer(ANS);
		win.valid = 0;

		return 0;

	} else if (mode == "VAL.VEHICLE" or mode == "VAL.VEHICLE2") {

		if (param3.unassigned()) {
			reqcompcode = "";
		}else{
			reqcompcode = param3;
		}

		if (win.is == win.isorig or win.is == "") {
			return 0;
		}
		if (vehicle.read(agy.vehicles, win.is)) {

gotvehicle:
			//in vehicle.subs and agency.subs mode val.vehicle
			var ans = 0;
			if (validcode3(vehicle.a(4), vehicle.a(3), win.is, agy.vehicles, msg)) {
				var dictvehicles;
				if (dictvehicles.open("DICT.VEHICLES", "")) {
					var compcode = calculate("COMPANY_CODE", dictvehicles, win.is, vehicle, 0);
					if (validcode2(compcode, "", "", xx, msg)) {
						if (compcode and reqcompcode and reqcompcode ne compcode) {
							msg = "belongs to company " ^ (DQ ^ (compcode ^ DQ)) ^ " but the required company is " ^ (DQ ^ (reqcompcode ^ DQ));
							goto badvehiclemsg;
						}
						if (vehicle.a(24)) {
							ans = authorised("VEHICLE ACCESS STOPPED", msg);
							if (not ans) {
badvehiclemsg:
								msg = "Media vehicle " ^ (DQ ^ (vehicle.a(1) ^ DQ)) ^ " (" ^ win.is ^ ") " ^ vehicle.a(24) ^ FM ^ FM ^ msg;
							}
						}else{
							ans = 1;
						}
					}
				}
			}
			if (not ans) {
				win.srcfile.unlock( ID);
				win.wlocked = 0;
				RECORD = "";
				return invalid(msg);
			}

			//required to prevent search list popping up if exact vehicle code entered
			//TODO but should be removed to allow ... (?verification?)
			return 0;

		}else{
			if (allownew) {
				gosub openversionfile( msg);
				if (vehicle.read(versionfile, win.is)) {
					goto gotvehicle;
				}
			}
		}

		if (win.templatex == "VEHICLES") {
			//if mode='VAL.VEHICLE2' then
			// reply=1
			// mode='VAL.VEHICLE'
			//end else
			// q=quote(is):'|Do you want to create a new vehicle ?'
			// if decide(q,'No - search for existing vehicle|Yes|No',reply) else reply=3
			// end
			//if reply=3 then valid=0
			reply = 1;
			if (reply > 1) {
				call generalsubs("VAL.ALPHANUMERIC");
				//t=is
				//convert @lower.case:@upper.case:'0123456789' to '' in t
				//if t<>'' then
				// msg='PLEASE USE ONLY LETTERS AND NUMBERS FOR VEHICLE CODES'
				// goto invalid
				// end
				return 0;
			}
		}

		var vehiclecodes = "";
		var vehicledict;
		if (not(vehicledict.open("dict_VEHICLES"))) {
			msg = "CANNOT OPEN DICT.VEHICLES IN AGENCY.SUBS";
			return invalid(msg);
		}

		//convert ' ' to '&' in is
		//temp=trim(is):' '
		//swap ' ' with ']':vm in temp
		//temp[-1,1]=fm
		temp = win.is.trim();
		temp.swapper(" ", "]&");
		temp ^= "]";
		temp = "SEQUENCE.XREF" ^ VM ^ temp ^ FM;
		//call btree.extract(temp,'VEHICLES',@dict,vehicle.codes)
		call btreeextract(temp, "VEHICLES", vehicledict, vehiclecodes);
		//call btree.extract2(temp,'VEHICLES',vehicledict,vehicle.codes)

		if (not vehiclecodes) {
novehiclecodes:
			win.is.converter("&", " ");
			//cannot be found is a trigger in the client
			msg = DQ ^ (win.is ^ DQ) ^ " vehicle cannot be found";
			if (reqcompcode) {
				msg.r(-1, "for company code " ^ (DQ ^ (reqcompcode ^ DQ)));
			}
			return invalid(msg);
		}

		var lists;
		if (not(lists.open("LISTS", ""))) {
			call fsmsg();
			var().stop();
		}
		vehiclecodes.converter(VM, FM);

		//suppress obsolete vehicles and ones without ratecards
		//if not allowed to override ratecards
		var ratecardoverride = authorised("SCHEDULE UPDATE AMOUNTS", xx, "");
		var accessstopped = authorised("VEHICLE ACCESS STOPPED", xx, "");

		if (not ratecardoverride or not accessstopped or reqcompcode) {

			RECORD.transfer(saver);
			ID.transfer(saveid);
			DICT.transfer(savedict);

			var nn = vehiclecodes.count(FM) + 1;
			for (var ii = 1; ii <= nn; ++ii) {

				if (ii[-1] == "0") {
					if (giveway()) {
						var().stop();
					}
				}

				ID = vehiclecodes.a(ii);
				var badvehicle = 1;
				if (RECORD.read(agy.vehicles, ID)) {

					//code is complicated, but safely converted from goto logic
					badvehicle = 0;

					//only vehicles with ratecards if override not authorised
					//if ratecardoverride else if @record<8> else badvehicle=1
					if (not ratecardoverride and not RECORD.a(8)) {
						badvehicle = 1;

					//skip stopped records completely
					} else if (not accessstopped and RECORD.a(24)) {
						badvehicle = 1;

					//skip vehicles belonging to wrong company code
					} else if (reqcompcode) {
						var compcode = calculate("COMPANY_CODE", vehicledict, ID, RECORD, 0);
						if (compcode) {
							if (compcode ne reqcompcode) {
								badvehicle = 1;
							}
						}
					}

				}

				if (badvehicle) {
					vehiclecodes.eraser(ii);
					ii -= 1;
					nn -= 1;
				}

			};//ii;

			if (not vehiclecodes) {
				goto novehiclecodes;
			}

			saver.transfer(RECORD);
			saveid.transfer(ID);
			savedict.transfer(DICT);

		}

		ANS = vehiclecodes;
		win.valid = 0;
		return 0;

	} else if (mode.substr(1,12) == "VAL.SUPPLIER") {
		if (win.is == win.isorig or win.is == "") {
			return 0;
		}

		gosub getmode3( mode,  msg,  mode3,  type);

		var supplier;
		if (supplier.read(agy.suppliers, win.is)) {
gotsupplier:
			{}
		}else{

			if (allownew) {
				gosub openversionfile( msg);
				if (supplier.read(versionfile, win.is)) {
					goto gotsupplier;
				}
			}

			gosub getmode3( mode,  msg,  mode3,  type);

			var suppliercodes = "";
			var storedict = DICT;
			if (DICT.open("dict_SUPPLIERS")) {
				//convert ' ' to '&' in is
				//temp=trim(is):' '
				//swap ' ' with ']':vm in temp
				//temp[-1,1]=fm
				temp = win.is.trim();
				temp.swapper(" ", "]&");
				temp ^= "]";
				temp = "SEQUENCE.XREF" ^ VM ^ temp ^ FM;
				call btreeextract(temp, "SUPPLIERS", DICT, suppliercodes);
				//call btree.extract2(temp,'SUPPLIERS',@dict,supplier.codes)
				DICT = storedict;
			}

			if (suppliercodes) {
				suppliercodes.converter(VM, FM);

				//remove media or production suppliers
				if (type.length() == 1) {
					var nsuppliers = suppliercodes.count(FM) + 1;
					for (var suppliern = 1; suppliern <= nsuppliers; ++suppliern) {
						if (suppliern[-1] == "0") {
							call giveway();
						}
						if (not supplier.read(agy.suppliers, suppliercodes.a(suppliern))) {
							supplier = "";
						}
						if (not (supplier.a(13)).index(type, 1)) {
							suppliercodes.eraser(suppliern);
							nsuppliers -= 1;
							suppliern -= 1;
						}
					};//suppliern;
				}

			}

			if (suppliercodes) {
				ANS = suppliercodes;
				win.valid = 0;
				return 0;
			}

			win.is.converter("&", " ");
			//cannot be found is a trigger in the client
			msg = DQ ^ (win.is ^ DQ) ^ " - " ^ mode3.lcase() ^ " supplier cannot be found";
			return invalid(msg);

		}

		//check right type of supplier
		type = (mode.field(".", 3))[1];
		if (type == "N") {
			type = "P";
		} else if (type == "1") {
			type = "";
		} else if (type == "2") {
			type = "M";
		} else if (type == "3") {
			type = "P";
		}
		if (type and not (supplier.a(13)).index(type, 1)) {
			if (supplier.a(13) == "P") {
				temp = "Production";
			}else{
				temp = "Media";
			}
			msg = supplier.a(1) ^ " " ^ (DQ ^ (win.is ^ DQ)) ^ "||This supplier is a " ^ temp ^ "|supplier and cannot be used here.|";
			return invalid(msg);
		}

		//get any supplementary fields
		RECORD.transfer(saverecord);
		supplier.transfer(RECORD);
		call suppliersubs("POSTREAD2");
		RECORD.transfer(supplier);
		saverecord.transfer(RECORD);

		return 0;

	} else if (mode == "VAL.EXECUTIVE") {

		var indexfile = "JOBS";
		if (win.datafile == "CLIENTS") {
			indexfile = "BRANDS";
		}

		if (win.is == "" or win.is == win.isorig) {
			return 0;
		}
		var dict = DICT;
		if (DICT.open("DICT", indexfile)) {
			temp = win.is.trim();
			temp.swapper(" ", "]&");
			temp ^= "]";
			temp = "EXECUTIVE_CODE" ^ VM ^ win.is ^ FM;
			call btreeextract(temp, indexfile, DICT, jobnos);
			//call btree.extract2(temp,indexfile,@dict,jobnos)
			if (jobnos == "" and win.is ne USERNAME) {
				if (win.templatex ne "JOBS" and win.templatex ne "CLIENTS") {
					msg = DQ ^ (win.is ^ DQ) ^ " - INVALID EXECUTIVE OR HAS NOT BEEN USED";
					return invalid(msg);
					return invalid(msg);
				}
				if (not(decide(DQ ^ (win.is ^ DQ) ^ "|Executive has not been used before.|Is " ^ (DQ ^ (win.is ^ DQ)) ^ " a new executive ?", "No - Search for an executive|Yes|Cancel", reply))) {
					reply = 3;
				}
				if (reply == 3) {
					win.valid = 0;
					return 0;
				}
				//if reply=1 then goto f2.executive
			}
		}
		DICT = dict;

	} else if (mode.substr(1,10) == "VAL.CLIENT") {
		if (win.is == "") {
			return 0;
		}

		if (win.is == win.isorig or win.is == "") {
			return 0;
		}

		win.datafile = "CLIENTS";

		for (var isn = 1; isn <= win.is.count(VM) + (win.is ne ""); ++isn) {

			clientcode = win.is.a(1, isn);

			//check client exists
			var client;
			if (client.read(agy.clients, clientcode)) {
gotclient:

				//check allowed to access this client
				if (not(validcode2("", clientcode, "", xx, msg))) {
					return invalid(msg);
				}

				//search for clients by name
			}else{

				if (allownew) {
					gosub openversionfile( msg);
					if (client.read(versionfile, clientcode)) {
						goto gotclient;
					}
				}

				gosub findclient( msg);
				if (not win.valid) {
					return 0;
				}
				//nb clientcode can be mv'ed if mode ends in 'S'
				win.is.r(1, isn, clientcode);
				ANS = "";
			}

		};//isn;

	} else if (mode.substr(1,9) == "VAL.BRAND") {

		if (win.is == win.isorig or win.is == "") {
			return 0;
		}

		for (var isn = 1; isn <= win.is.count(VM) + (win.is ne ""); ++isn) {

			brandcode = win.is.a(1, isn);

			//check brand exists
			var brand;
			if (brand.read(agy.brands, brandcode)) {

				//check allowed to access this brand
				if (not(validcode2("", "", brandcode, agy.brands, msg))) {
					return invalid(msg);
				}

				//search for brands by name
			}else{
				gosub findbrand( msg);
				if (not win.valid) {
					return 0;
				}
				//nb brandcode can be mv'ed if mode ends in 'S'
				win.is.r(1, isn, brandcode);
				ANS = "";
			}

		};//isn;

	} else if (mode == "DEF.PERIOD") {
		if (win.is) {
			return 0;
		}
		win.isdflt = ((var().date()).oconv("D2/E")).field("/", 2, 2);
		if (win.isdflt[1] == "0") {
			win.isdflt.splicer(1, 1, "");
		}

	} else if (mode == "VAL.PERIOD") {
		if (win.is == "" or win.is == win.isorig) {
			return 0;
		}

		//default year to the year of the period
		if (win.is.match("0N")) {
			win.is ^= ((var().date()).oconv("D2/")).substr(6,3);
		}

		//trim leading zeroes
		if (win.is[1] == "0") {
			win.is.splicer(1, 1, "");
		}

		//check month/year format
		var month = win.is.field("/", 1);
		if (not((var("1,2,3,4,5,6,7,8,9,10,11,12").a(1)).locateusing(month, ",", temp))) {
badperiod:
			msg = "PLEASE ENTER PERIOD AS \"MONTH/YEAR\"|(EG \"1/92\")";
			return invalid(msg);
		}
		var year = (win.is.field("/", 2)).substr(-2,2);
		if (not(year.match("2N"))) {
			goto badperiod;
		}

		win.is = month ^ "/" ^ year;

	} else if (mode == "VAL.YEARPERIOD") {
		if (win.is == "") {
			return 0;
		}
		var nn = win.is.count(VM) + 1;
		for (var ii = 1; ii <= nn; ++ii) {
			if (not((win.is.a(1, ii)).match("2N\".\"2N"))) {
				msg = DQ ^ (win.is.a(1, ii) ^ DQ) ^ " - INCORRECT FORMAT|PLEASE ENTER YEAR.PERIOD (EG 97.03)";
				return invalid(msg);
			}
		};//ii;

	} else if (mode.substr(1,19) == "VAL.PRODUCTION.TYPE" or mode.substr(1,14) == "VAL.MEDIA.TYPE" or mode.substr(1,18) == "VAL.MEDIAPROD.TYPE") {

		if (win.is == "" or win.is == win.isorig) {
			return 0;
		}

		var mode2 = mode.field(".", 2);
		if (mode2 == "MEDIAPROD") {
			reqtype = "*";
			title = "";

		} else if (mode2 == "MEDIA") {
			reqtype = "";
			title = "MEDIA";

		} else {
			reqtype = "Y";
			title = "PRODUCTION";
		}

		if (not typerec.read(agy.jobtypes, win.is)) {
			msg = DQ ^ (win.is ^ DQ) ^ " " ^ title ^ " TYPE DOES NOT EXIST";
			return invalid(msg);
		}
		if (reqtype ne "*") {
			if (typerec.a(3) ne reqtype) {
				msg = DQ ^ (win.is ^ DQ) ^ " - IS NOT A " ^ title ^ " TYPE";
				return invalid(msg);
			}
		}

		mode2 = mode.field(".", 4);
		msg = capitalise(title) ^ " Type :" ^ typerec.a(1) ^ " " ^ (DQ ^ (win.is ^ DQ));

		//check group
		if (mode2 == "NOGROUP") {
			if (typerec.a(10)) {
				msg ^= "|has a group code itself and cannot be used here.";
				return invalid(msg);
			}
		}

		//check account numbers
		gosub gethasaccounts( hasaccounts);
		if (hasaccounts) {
			temp = "";
			if (mode2 == "WITHACNO" and typerec.a(5) == "" and typerec.a(6) == "") {
badtype:
				msg ^= "|does not have " ^ temp ^ "account numbers specified|and cannot be used until they are.";
				return invalid(msg);
			}
			if (mode2 == "WITHINCOMEACNO" and typerec.a(5) == "") {
				temp = "an income ";
				goto badtype;
			}
			if (mode2 == "WITHCOSTACNO" and typerec.a(6) == "") {
				temp = "a cost ";
				goto badtype;
			}
		}

	} else {
		msg = DQ ^ (mode ^ DQ) ^ " MODE NOT RECOGNISED IN AGENCY.SUBS";
		gosub invalid(msg);
		return 0;
	}

	return 0;

}

subroutine getmode3(in mode, io msg, io mode3, out type) {
	//getmode3(in mode, io msg, io mode3, out type)

	//required types Media/Production
	mode3 = mode.field(".", 3);
	if (not mode3) {
		if (authorised("SCHEDULE ACCESS", msg, "")) {
			mode3 = "MEDIA";
		}
		if (authorised("JOB ACCESS", msg, "")) {
			if (mode3) {
				//both are allowed, users can choose either or both
				mode3 = "";
				reply = 3;
				if (reply == 1) {
					mode3 = "MEDIA";
				} else if (reply == 2) {
					mode3 = "PRODUCTION";
				}
			}
			mode3 = "PRODUCTION";
		}
	}

	if (mode3 == "M" or mode3 == 2) {
		mode3 = "MEDIA";
	}
	if (mode3 == "N" or mode3 == 3) {
		mode3 = "PRODUCTION";
	}
	type = mode3[1];
	//mode=fieldstore(mode,'.',3,1,mode3)

	return;

}

subroutine findbrand(io msg) {
	//findbrand(io msg)
	var brandcodes = "";
	var storedict2 = DICT;
	if (DICT.open("dict_BRANDS")) {

		//convert ' ' to '&' in brandcode
		//temp=trim(brandcode):' '
		//swap ' ' with ']':vm in temp
		//temp[-1,1]=fm
		//temp='SEQUENCE.XREF':vm:temp

		temp = brandcode.trim();
		temp.swapper(" ", "]&");
		temp ^= "]";
		temp = "SEQUENCE.XREF" ^ VM ^ temp ^ FM;

		call btreeextract(temp, "BRANDS", DICT, brandcodes);
		//call btree.extract2(temp,'BRANDS',@dict,brandcodes)
		DICT = storedict2;
	}

	brandcodes.converter(VM, FM);

	//suppress invalid brands
	var nb = brandcodes.count(FM) + 1;
	for (var ii = nb; ii >= 1; --ii) {
		if (not(validcode2("", "", brandcodes.a(ii), agy.brands, msg))) {
			brandcodes.eraser(ii);
			ii -= 1;
			nb -= 1;
		}
	};//ii;

	if (not brandcodes) {
		//cannot be found is a trigger in the client
		msg = DQ ^ (brandcode ^ DQ) ^ " brand cannot be found";
		brandcode = "";
		gosub invalid(msg);
		return;
	}

	ANS = brandcodes;
	win.valid = 0;
	return;

}

subroutine findclient(io msg) {
	//findclient(io msg)

	var clientcodes = "";
	var storedict = DICT;
	if (DICT.open("dict_CLIENTS")) {

		temp = clientcode.trim();
		temp.swapper(" ", "]&");
		temp ^= "]";
		temp = "SEQUENCE.XREF" ^ VM ^ temp ^ FM;

		call btreeextract(temp, "CLIENTS", DICT, clientcodes);
		//call btree.extract2(temp,'CLIENTS',@dict,clientcodes)
		DICT = storedict;
	}

	clientcodes.converter(VM, FM);

	//suppress invalid clients
	var nc = clientcodes.count(FM) + 1;
	for (var ii = nc; ii >= 1; --ii) {
		if (not(validcode2("", clientcodes.a(ii), "", xx, msg))) {
			clientcodes.eraser(ii);
		}
	};//ii;

	if (not clientcodes) {
		//cannot be found is a trigger in the client
		msg = DQ ^ (clientcode ^ DQ) ^ " client cannot be found";
		clientcode = "";
		gosub invalid(msg);
		return;
	}

	ANS = clientcodes;
	win.valid = 0;
	return;

}

subroutine gethasaccounts(io hasaccounts) {
	//gethasaccounts(io hasaccounts)

	var balancesfile;
	if (balancesfile.open("BALANCES", "")) {
		hasaccounts = getreccount(balancesfile, "", "");
	}else{
		hasaccounts = "";
	}
	return;

}

subroutine openversionfile(io msg) {
	//openversionfile(io msg)

	var versionfilename = singular(win.datafile) ^ "_VERSIONS";
	if (not(versionfile.open(versionfilename, ""))) {
		if (not(ID.index("~", 1))) {
			return;
		}
		msg = versionfilename ^ " file is missing";
		gosub invalid(msg);
		return;
	}
	return;

}


libraryexit()
