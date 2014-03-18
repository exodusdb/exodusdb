#include <exodus/library.h>
libraryinit()

#include <sysmsg.h>
#include <initcompany.h>
#include <readagp.h>
#include <generalsubs.h>
#include <nextkey.h>
#include <validcode3.h>
#include <calculatex.h>
#include <validcode2.h>
#include <btreeextract.h>
#include <giveway.h>
#include <makelist.h>
#include <catalyst.h>
#include <jobsubs.h>
#include <getreccount.h>
#include <suppliersubs.h>
#include <singular.h>

#include <agy.h>
#include <gen.h>
#include <win.h>

var iaccno;
var billcostn;//num
var xx;
var brandcode;
var doccompanycode;
var docdate;
var docname;
var filen;
var reqcompcode;
var reply;//num
var temp;
var saver;
var saveid;
var savedict;
var type;
var mode3;
var saverecord;
var jobnos;
var clientcode;
var hasaccounts;
var wsmsg;

function main() {
	//jbase
	var interactive = not SYSTEM.a(33);

	//y2k2
	if (mode.field("*", 1) == "CHECKANALYSISCODE") {

		//validates an analysis code - used by finance module daybook.subs2
		//return msg if wrong or no msg if ok

		var accno = mode.field("*", 2);

		//analysiscode=field(mode,'*',3,9999)

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

		msg = "Analysis code " ^ (DQ ^ (param3 ^ DQ)) ^ " is invalid for account " ^ (DQ ^ (iaccno.oconv("[ACNO]") ^ DQ)) ^ ".";

		//open 'DEFINITIONS' to definitions else
		// call fsmsg()
		// stop
		// end
		var billcostaccs;
		if (not(billcostaccs.read(gen._definitions, "BILLCOSTACCS"))) {
			msg = "System Error: BILLCOSTACCS is missing in AGENCY.SUBS";
			call sysmsg(msg);
			return 0;
		}

		//check if the account requires analysis
		if (not(billcostaccs.a(2).locateusing(iaccno, VM, billcostn))) {
			billcostn = 0;
			var accounts;
			if (not(accounts.open("ACCOUNTS", ""))) {
				call fsmsg();
				var().stop();
			}
			var icontrolaccno;
			if (icontrolaccno.readv(accounts, iaccno, 6)) {
				if (icontrolaccno) {
					if (not(billcostaccs.a(2).locateusing(icontrolaccno, VM, billcostn))) {
						billcostn = 0;
					}
				}
			}
			if (not billcostn) {
				if (param3 == "") {
					//ok
					msg = "";
					return 0;
				}
				msg.r(-1, "Account " ^ (DQ ^ (iaccno.oconv("[ACNO]") ^ DQ)) ^ " cannot have an analysis code");
				return 0;
			}
		}
		if (not param3) {
			msg = "Analysis code is required for account " ^ (DQ ^ (iaccno.oconv("[ACNO]") ^ DQ));
			return 0;
		}

		msg ^= FM ^ FM ^ "Analysis code field ";

		//analysis field number
		var tt = billcostaccs.a(3, billcostn);
		if (not(tt.locateusing(param3.field("*", 1), SVM, xx))) {
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
				param3 = param3.fieldstore("*", 1, 1, tt);
			}
		}

		//discount/charge number
		var colno = param3.field("*", 2);
		if (not colno) {
			goto field2err;
		}
		if (not(var("1" _VM_ "2" _VM_ "3" _VM_ "4" _VM_ "5" _VM_ "6" _VM_ "7").a(1).locateusing(colno, VM, xx))) {
field2err:
			msg ^= "2 is required and must be one of";
			msg.r(-1, "1=Gross");
			msg.r(-1, "2=Loading");
			msg.r(-1, "3=Discount");
			msg.r(-1, "4=Commission");
			msg.r(-1, "5=Fee");
			msg.r(-1, "6=Tax");
			msg.r(-1, "7=Other");
			return 0;
		}

		//brandcode
		brandcode = param3.field("*", 3);
		if (not brandcode) {
			msg ^= "3 - brand code is required";
			return 0;
		}
		if (not(xx.read(agy.brands, brandcode))) {
			msg ^= "3 - " ^ (DQ ^ (brandcode ^ DQ)) ^ " brand code does not exist";
			return 0;
		}

		//media verification
		var vehiclecode = param3.field("*", 4);
		if (vehiclecode) {

			//check account allows media
			if (not(billcostaccs.a(5, billcostn).locateusing(1, SVM, xx))) {
				//could force jobs here but best to indicate some error since unexpected
				msg ^= "4 - media/vehicle code cannot be entered";
				return 0;
			}

			//vehiclecode
			if (not(xx.read(agy.vehicles, vehiclecode))) {
				msg ^= "4 - " ^ (DQ ^ (vehiclecode ^ DQ)) ^ " vehicle code does not exist";
				return 0;
			}

			//verify no job analysis fields if media vehicle is present
			tt = param3.field("*", 5, 3);
			tt.converter("*", "");
			if (tt.length()) {
				//could force no fields here but best to indicate some error since unexpected
				msg ^= "5-7 must be empty";
				return 0;
			}

			//nonmedia verification
		}else{

			//check account allows jobs
			if (not(billcostaccs.a(5, billcostn).locateusing(2, SVM, xx))) {
				//TODO we should try and avoid an account being used for both media and jobs
				msg ^= "4 - media/vehicle code must be entered";
				return 0;
			}

			//marketcode
			var marketcode = param3.field("*", 5);
			if (not marketcode) {
				msg ^= "5 - market code is required";
				return 0;
			}
			if (not(xx.read(agy.markets, marketcode))) {
				msg ^= "5 - " ^ (DQ ^ (marketcode ^ DQ)) ^ " market code does not exist";
				return 0;
			}

			//suppliercode
			var suppliercode = param3.field("*", 6);
			if (not suppliercode) {
				msg ^= "6 - supplier code is required";
				return 0;
			}
			if (not(xx.read(agy.suppliers, suppliercode))) {
				msg ^= "6 - " ^ (DQ ^ (suppliercode ^ DQ)) ^ " supplier code does not exist";
				return 0;
			}

			//type exists
			var typecode = param3.field("*", 7);
			if (not typecode) {
				msg ^= "7 - job type code is required";
				return 0;
			}
			if (not(xx.read(agy.mediatypes, typecode))) {
				msg ^= "7 - " ^ (DQ ^ (typecode ^ DQ)) ^ " job type code does not exist";
				return 0;
			}

			//type code limitation
			tt = billcostaccs.a(4, billcostn);
			if (tt) {
				if (not(tt.locateusing(typecode, SVM, xx))) {
					tt.swapper(SVM, ", ");
					msg ^= "7 job type code must be one of " ^ tt;
					return 0;
				}
			}

		}

		//indicate analysis code is all ok
		msg = "";

		goto 6505;
	}
	if (mode.field(".", 1) == "CHKCLOSEDPERIOD") {

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
			doccompanycode = var("COMPANY_CODE").calculate().a(1, 1);
			docdate = ID.field("*", 2);
			docname = "timesheet";

			goto 1680;
		}
		if (win.datafile == "PLANS") {
			doccompanycode = RECORD.a(187);
			//check oldest period on the plan
			//docdate=iconv('1/':@record<12>,'D2/E')
			//check LATEST date on plan (so they can work on multimonth plans)
			//TODO closed periods should be locked in the plan data entry screen
			docdate = var("28/" ^ RECORD.a(15)).iconv("D2/E");
			docname = "plan";

			goto 1680;
		}
		if (win.datafile == "SCHEDULES") {
			doccompanycode = RECORD.a(187);
			//check oldest date on the schedule
			//docdate=@record<10>
			//check LATEST date on schedule (so they can work on multimonth schedules)
			//closed dates are locked in the schedule calendar popup
			docdate = RECORD.a(25);
			docname = "schedule";

			goto 1680;
		}
		if (win.datafile == "JOBS") {
			doccompanycode = RECORD.a(14);
			docdate = var("1/" ^ RECORD.a(1)).iconv("D2/E");
			docname = "job";

	/* job period is no longer used - you can work on old jobs if not closed;
			case datafile='PRODUCTION.ORDERS' or datafile='PRODUCTION.INVOICES';
				doccompanycode=xlate('JOBS',@record<2>,14,'X');
				docdate=iconv('1/':xlate('JOBS',@record<2>,1,'X'),'D2/E');
				docname='job';

				//order and estimate date should not really be before job period
				//but I think no restriction in neosys so go back if necessary
				recdate=@record<1>;
				if recdate<docdate then;
					docdate=recdate;
					if datafile='PRODUCTION.ORDERS' then;
						docname='order';
					end else;
						docname='estimate';
						end;
					end;

				if datafile='PRODUCTION.ORDERS' then;
					suppinvdate=orec<9>;
					if suppinvdate else suppinvdate=@record<9>;
					if suppinvdate and suppinvdate<docdate then;
						docdate=suppinvdate;
						docname='supplier invoice';
						end;
					end;
	*/

			goto 1680;
		}
		if (win.datafile == "PRODUCTION.ORDERS") {
			doccompanycode = RECORD.a(2).xlate("JOBS", 14, "X");

			//synchronise backdateamendments in agency.subs and prodorder.subs
			var backdateamendments = 0;
			if (backdateamendments) {
				docdate = win.orec.a(9);
			}else{
				docdate = RECORD.a(9);
			}
			if (docdate) {
				docname = "supplier invoice";
			}else{
				docname = "purchase order";
				//if index(mode,'POSTREAD',1) then
				// *always allow amendments to purchase orders in order to add invoice details
				// *in PREWRITE the inv date will be checked
				// *if no inv date then the order date will have to be moved after closed
				// docdate=''
				//end else

				//prevent even adding invoice numbers to old production orders
				//for safety
				docdate = RECORD.a(1);
				// end
			}
			goto getolderjobdate;

			goto 1680;
		}
		if (win.datafile == "PRODUCTION.INVOICES") {
			doccompanycode = RECORD.a(2).xlate("JOBS", 14, "X");
			call initcompany(doccompanycode);
			docdate = RECORD.a(1);
			docname = "estimate";

			//prevent adding/changing to jobs with old periods
			//maybe move the job period forward in this case or unclose temporarily
getolderjobdate:
			var jobdate = ("1/" ^ RECORD.a(2).xlate("JOBS", 1, "X")).iconv("D2/E");
			if (jobdate < docdate) {
				docdate = jobdate;
				docname = "job";
			}

			/* done in certify for speed;
			case datafile='ADS';
				doccompanycode=@record<40>;
				docdate=@record<12>;
				docname='ad';
			*/

			goto 1680;
		}
		if (1) {
			msg = DQ ^ (win.datafile ^ DQ) ^ " is invalid in AGENCY.SUBS " ^ mode;
			goto EOF_439;
		}
L1680:
		if (doccompanycode and docdate) {
			var doccompany;
			if (not(doccompany.read(gen.companies, doccompanycode))) {
				msg = DQ ^ (doccompanycode ^ DQ) ^ " company code is missing";
				win.reset = 5;
				goto EOF_439;
			}
			var accops = "operations";
			var closedperiod = doccompany.a(37);
			if (not closedperiod) {
				closedperiod = doccompany.a(16);
				accops = "accounts";
			}
			var closedate = closedperiod.iconv("[DATEPERIOD," ^ doccompany.a(6) ^ "]");
			if (docdate <= closedate) {
				msg = "You cannot update this " ^ docname ^ " dated " ^ docdate.oconv("[DATE,*4]") ^ " because";
				msg.r(-1, doccompany.a(1) ^ " " ^ accops ^ " are closed up to " ^ closedate.oconv("[DATE,*4]"));
			}
		}

		goto 6505;
	}
	if (mode.substr(1, 10) == "GETNEXTID.") {

		var compcode = mode.field(".", 2);
		var previous = mode.field(".", 3);

		if (not interactive) {
			if (compcode and compcode ne gen.gcurrcompany) {
				call initcompany(compcode);
			}
			call readagp();
		}

		if (not(var("PLANS" _VM_ "SCHEDULES" _VM_ "JOBS" _VM_ "PRODUCTION.ORDERS" _VM_ "PRODUCTION.INVOICES").a(1).locateusing(win.datafile, VM, filen))) {
			msg = "File " ^ (DQ ^ (win.datafile ^ DQ)) ^ " is invalid in AGENCY.SUBS,GETNEXTID";
			goto EOF_439;
		}
		//agp<71>
		//agp<63>
		//agp<53>
		//agp<69>
		//agp<70>
		var keyformat = agy.agp.a(var("71" _VM_ "63" _VM_ "53" _VM_ "69" _VM_ "70").a(1, filen));

		//if no pattern then allow own patterns like X99999
		//if they enter a single letter
		if ((keyformat == "" or keyformat == "<NUMBER>") and ID.match("1A\"*\"")) {
			keyformat = ID.substr(1, 1) ^ "%<>";
		}

		if (not(keyformat.index("<", 1))) {
			//SK2 (not SK) to prevent endless loop when GENERAL.SUBS calls this routine
			call generalsubs("DEF.SK2");
			ANS = win.isdflt;
			return 0;
		}

		//determine company prefix
		if (compcode == "") {
			compcode = gen.gcurrcompany;
		}
		var companyx;
		if (not(companyx.reado(gen.companies, compcode))) {
			msg = DQ ^ (compcode ^ DQ) ^ " IS NOT IN COMPANY FILE";
			gosub EOF_484();
			companyx = "";
		}
		if (companyx.a(28) == "") {
			companyx.r(28, compcode);
		}

		//prevent numeric company numbers being used as <COMPANY>
		//to avoid <COMPANY><NUMBER> being merged numbers
		if (keyformat.index("<COMPANY>", 1)) {
			if ((companyx.a(28)).isnum()) {
				msg = "Please setup alphabetic company code for company " ^ (DQ ^ (compcode ^ DQ)) ^ " in Company File first";
				goto EOF_439;
			}
		}

		//similar code in AGENCY.SUBS,PRINTPLANS2,PRODINVS2,REPRINTINVS
		keyformat.swapper("<YEAR>", (var().date()).oconv("D").field(" ", 3));
		keyformat.swapper("<YEAR2>", ((var().date()).oconv("D").field(" ", 3)).substr(-2, 2));
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
		ANS = nextkey(params, previous);
	/*;
		case mode[1,12]='F2.CATALYST.';
			storepseudo=@pseudo;
			convert '.' to ',' in mode;
			convert ':' to '.' in mode;
			call catalyst(field(mode,',',3),field(mode,',',4,999));
			if @ans then data '';
			@pseudo=storepseudo;
	*/

		goto 6505;
	}
	if (mode == "DEF.CURRENCY") {
		if (win.is == "") {
			ANS = var("MARKET_CODE").calculate().xlate("MARKETS", 5, "X");
		}

	/*;
		CASE MODE[1,7]='F2.PLAN' OR MODE[1,11]='F2.SCHEDULE';
			TEMP=FIELD(MODE,'.',2);
			if temp[-1,1]='S' else temp:='S';
			DISPLAY.FIELDS='BRAND_NAMEýPERIOD';
			IF TEMP='PLANS' THEN DISPLAY.FIELDS:='ýPERIOD_TO';
			DISPLAY.FIELDS:='ýMARKET_CODEýVEHICLE_CODE';
			IF TEMP EQ 'SCHEDULES' THEN;
				DISPLAY.FIELDS:='ýAPPROVAL_DATEýFULLY_BOOKEDýINVOICED';
			end else;
				display.fields:='ýREF_NO';
				end;
			//CALL USEINDEX(MODE,TEMP,DISPLAY.FIELDS,'')
			CALL USEINDEX((if mode[-1,1]='S' then 'M' else ''),TEMP,DISPLAY.FIELDS,'');

		CASE MODE[1,10]='F2.VEHICLE';

			if @list.active then;
				mediatypecode='';
				marketcode='';
			end else;
				mediatypecode=field(mode,'.',3);
				if mediatypecode else;
					//call agency.subs('F2.MEDIA.TYPE')
					call agency.subs('F2.MEDIA.TYPE.WITHACNO');
					@data='';
					if @ans='' then return 0;
					mediatypecode=@ans;
					end;
				marketcode='';
				//call agency.subs('F2.MARKET')
				//marketcode=@ans
				end;

			call msg2('Getting vehicles,|please wait.','UB',buffer,'');
			cmd='SELECT VEHICLES BY SEQUENCE (S)';
			andx='';
			if mediatypecode then;
				cmd:=andx:' WITH MEDIA_TYPE_CODE ':quote(mediatypecode);
				andx=' AND';
				end;

			if marketcode then;
				cmd:=andx:' WITH MARKET_CODE ':quote(marketcode);
				andx=' AND';
				end;
			cmd:=andx:' WITH VEHICLE_CODE2 = ""';
			call safeselect(cmd);
			call msg2('','DB',buffer,'');
			if @list.active else;
				msg='NO VEHICLES FOUND';
				return invalid();
				end;
			goto selectvehicle;
	*/
		goto 6505;
	}
	if (mode == "VAL.VEHICLE" or mode == "VAL.VEHICLE2") {
		if (param3.unassigned()) {
			reqcompcode = "";
		}else{
			reqcompcode = param3;
		}
		if (win.is == win.isorig or win.is == "") {
			return 0;
		}
		var vehicle;
		if (vehicle.read(agy.vehicles, win.is)) {

			//in vehicle.subs and agency.subs mode val.vehicle
			var ANS = 0;
			if (validcode3(vehicle.a(4), vehicle.a(3), win.is, agy.vehicles, msg)) {
				var dictvehicles;
				if (dictvehicles.open("DICT.VEHICLES", "")) {
					var compcode = calculatex("COMPANY_CODE", dictvehicles, win.is, vehicle, 0);
					if (validcode2(compcode, "", "", "", msg)) {
						if (compcode and reqcompcode and reqcompcode ne compcode) {
							msg = "belongs to company " ^ (DQ ^ (compcode ^ DQ)) ^ " but the required company is " ^ (DQ ^ (reqcompcode ^ DQ));
							goto badvehiclemsg;
						}
						if (vehicle.a(24)) {
							ANS = authorised("VEHICLE ACCESS STOPPED", msg);
							if (not ANS) {
badvehiclemsg:
								msg = "Media vehicle " ^ (DQ ^ (vehicle.a(1) ^ DQ)) ^ " (" ^ win.is ^ ") " ^ vehicle.a(24) ^ FM ^ FM ^ msg;
							}
						}else{
							ANS = 1;
						}
					}
				}
			}
			if (not ANS) {
				win.srcfile.unlock( ID);
				win.wlocked = 0;
				RECORD = "";
				goto EOF_439;
			}

			//required to prevent search list popping up if exact vehicle code entered
			//TODO but should be removed to allow ... (?verification?)
			return 0;

		}

		if (win.templatex == "VEHICLES") {
			if (mode == "VAL.VEHICLE2") {
				reply = 1;
				mode = "VAL.VEHICLE";
			}else{
				var qq = DQ ^ (win.is ^ DQ) ^ "|Do you want to create a new vehicle ?";
				if (not(decide(qq, "No - search for existing vehicle" _VM_ "Yes" _VM_ "No", reply))) {
					reply = 3;
				}
			}
			if (reply == 3) {
				win.valid = 0;
			}
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
		if (not(vehicledict.open("DICT", "VEHICLES"))) {
			msg = "CANNOT OPEN DICT.VEHICLES IN AGENCY.SUBS";
			goto EOF_439;
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

		if (not vehiclecodes) {
novehiclecodes:
			win.is.converter("&", " ");
			//cannot be found is a trigger in the client
			msg = DQ ^ (win.is ^ DQ) ^ " vehicle cannot be found";
			if (reqcompcode) {
				msg.r(-1, "for company code " ^ (DQ ^ (reqcompcode ^ DQ)));
			}
			goto EOF_439;
		}

		var lists;
		if (not(lists.open("LISTS", ""))) {
			call fsmsg();
			var().stop();
		}
		vehiclecodes.converter(VM, FM);

		//suppress obsolete vehicles and ones without ratecards
		//if not allowed to override ratecards
		var ratecardoverride = authorised("SCHEDULE UPDATE AMOUNTS");
		var accessstopped = authorised("VEHICLE ACCESS STOPPED");

		if (not ratecardoverride or not accessstopped or reqcompcode) {

			RECORD.transfer(saver);
			ID.transfer(saveid);
			DICT.transfer(savedict);

			var nn = vehiclecodes.count(FM) + 1;
			for (var ii = 1; ii <= nn; ++ii) {
				if (ii.substr(-1, 1) == "0") {
					call giveway("");
				}
				ID = vehiclecodes.a(ii);
				if (RECORD.reado(agy.vehicles, ID)) {

					if (not ratecardoverride) {
						if (not RECORD.a(8)) {
							goto badvehicle;
						}
					}

					//skip stopped records completely
					if (not accessstopped and RECORD.a(24)) {
badvehicle:
						vehiclecodes.eraser(ii);
						ii -= 1;
						nn -= 1;

						//skip vehicles belonging to wrong company code
						goto 3355;
					}
					if (reqcompcode) {
						var compcode = calculatex("COMPANY_CODE", vehicledict, ID, RECORD, 0);
						if (compcode) {
							if (compcode ne reqcompcode) {
								goto badvehicle;
							}
						}
					}
L3355:

								}else{
					goto badvehicle;
				}
			};//ii;

			if (not vehiclecodes) {
				goto novehiclecodes;
			}

			saver.transfer(RECORD);
			saveid.transfer(ID);
			savedict.transfer(DICT);

		}

		if (not interactive) {
			ANS = vehiclecodes;
			win.valid = 0;
			return 0;
		}

		call makelist("", vehiclecodes, "", "");
		//goto selectvehicle

selectvehicle:
		call catalyst("P", "ADPOPUPS*" ^ mode.field(".", 2));
		win.is = ANS;
		if (not win.is) {

			win.valid = 0;
			return 0;
		}

		if (mode.substr(1, 2) == "F2") {
			DATA ^= "" "\r";
		}
		win.is = ANS;

		goto 6505;
	}
	if (mode == "DEF.JOB") {
		if (win.is) {
			return 0;
		}
		//read is.dflt from definitions,'JOBS.SK' then is.dflt-=1 else is.dflt=''
		call jobsubs("GETLASTJOBNO");
		ANS.transfer(win.isdflt);
	/*;
		CASE MODE[1,6]='F2.JOB';
			DISPLAY.FIELDS='NUMBERLýCLIENT_NAME15ýBRAND_NAME15ýPERIODýPRODUCTION_TYPEýDESCRIPTION15ýCLOSED';
			multi=if mode[-1,1]='S' then 'M' else '';
			CALL USEINDEX(multi,'JOBS',DISPLAY.FIELDS,'');
			if @ans then data '';
	*/
		goto 6505;
	}
	if (mode.substr(1, 7) == "VAL.JOB") {
		if (win.is == "" or win.is == win.isorig) {
			return 0;
		}

		if (mode == "VAL.JOBS") {
			if (getreccount(agy.jobs, "", "") == 0) {
				return 0;
			}
		}

		//cannot remove records from closed jobs
		if (win.isorig and win.orec and mode.field(".", 3) == "OPEN") {
			var oldjob;
			if (not(oldjob.read(agy.jobs, win.isorig))) {
				msg = DQ ^ (win.isorig ^ DQ) ^ " - JOB IS MISSING";
				goto EOF_439;
			}
			if (oldjob.a(7) == "Y") {
				msg = DQ ^ (win.isorig ^ DQ) ^ " JOB HAS BEEN CLOSED|YOU CANNOT REMOVE THIS DOCUMENT";
				goto EOF_439;
			}
		}

		var nis = win.is.count(VM) + 1;
		for (var isn = 1; isn <= nis; ++isn) {

			var is2 = win.is.a(1, isn);

			var job;
			if (not(job.read(agy.jobs, is2))) {
				msg = DQ ^ (is2 ^ DQ) ^ " - JOB DOES NOT EXIST";
				goto EOF_439;
			}

			//restrict access to own jobs only
			if (job.a(8) and job.a(8) ne USERNAME) {
				if (not(authorised("JOB ACCESS OTHERS", msg, ""))) {
					goto EOF_439;
				}
			}

			//if validcode('BRAND',job<2>,msg) else goto invalid
			//if job<14> then if validcode('COMPANY',job<14>,msg) else goto invalid
			if (not(validcode2(job.a(14), "", job.a(2), agy.brands, msg))) {
				goto EOF_439;
			}
			if (not(validcode3(job.a(12), "", "", agy.vehicles, msg))) {
				goto EOF_439;
			}

			//check job not closed
			if (mode.field(".", 3) == "OPEN") {
				if (job.a(7) == "Y") {
					msg = DQ ^ (win.is ^ DQ) ^ " JOB IS CLOSED";
					goto EOF_439;
				}
			}

		};//isn;
	/*;
		case mode[1,11]='F2.SUPPLIER';

			gosub getmode3;

			call msg2('Getting suppliers,|please wait.','UB',buffer,'');
			cmd='SELECT SUPPLIERS BY SEQUENCE (S)';
			if type then cmd:=' WITH TYPE CONTAINING ':quote(type[1,1]);
			call safeselect(cmd);
			call msg2('','DB',buffer,'');

			goto selectsupplie;
	*/
		goto 6505;
	}
	if (mode.substr(1, 12) == "VAL.SUPPLIER") {
		if (win.is == win.isorig or win.is == "") {
			return 0;
		}

		if (not interactive) {
			gosub getmode3();
		}

		var supplier;
		if (not(supplier.read(agy.suppliers, win.is))) {

			if (interactive and win.templatex == "SUPPLIERS") {
				var qq = "Do you want to create a new supplier?";
				if (not(decide(qq, "No - search for existing supplier" _VM_ "Yes" _VM_ "No", reply))) {
					reply = 3;
				}
				if (reply == 3) {
					win.valid = 0;
				}
				if (reply > 1) {
					call generalsubs("VAL.ALPHANUMERIC");
					//t=is
					//convert @lower.case:@upper.case:'0123456789' to '' in t
					//if t<>'' then
					// msg='PLEASE USE ONLY LETTERS AND NUMBERS FOR SUPPLIER CODES'
					// goto invalid
					// end
					return 0;
				}
			}

			gosub getmode3();

			var suppliercodes = "";
			var storedict = DICT;
			if (DICT.open("DICT", "SUPPLIERS")) {
				//convert ' ' to '&' in is
				//temp=trim(is):' '
				//swap ' ' with ']':vm in temp
				//temp[-1,1]=fm
				temp = win.is.trim();
				temp.swapper(" ", "]&");
				temp ^= "]";
				temp = "SEQUENCE.XREF" ^ VM ^ temp ^ FM;
				call btreeextract(temp, "SUPPLIERS", DICT, suppliercodes);
				DICT = storedict;
			}

			if (suppliercodes) {
				suppliercodes.converter(VM, FM);

				//remove media or production suppliers
				if (type.length() == 1) {
					var nsuppliers = suppliercodes.count(FM) + 1;
					for (var suppliern = 1; suppliern <= nsuppliers; ++suppliern) {
						if (suppliern.substr(-1, 1) == "0") {
							call giveway("");
						}
						if (not(supplier.reado(agy.suppliers, suppliercodes.a(suppliern)))) {
							supplier = "";
						}
						if (not((supplier.a(13)).index(type, 1))) {
							suppliercodes.eraser(suppliern);
							nsuppliers -= 1;
							suppliern -= 1;
						}
					};//suppliern;
				}

			}

			if (suppliercodes) {

				if (not interactive) {
					ANS = suppliercodes;
					win.valid = 0;
					return 0;
				}

				//convert into a select list and goto select
				if (suppliercodes) {
					var lists;
					if (not(lists.open("LISTS", ""))) {
						call fsmsg();
						var().stop();
					}
					temp = var().time() ^ "X" ^ STATION;
					temp.converter(" ", "");
					call makelist("", suppliercodes, "", "");
					//write supplier.codes on lists,temp
					//perform 'GET-LIST ':temp:' (S)'
					//delete lists,temp
					//call note(temp)
					goto selectsupplier;
				}

			}

			win.is.converter("&", " ");
			//cannot be found is a trigger in the client
			msg = DQ ^ (win.is ^ DQ) ^ " - " ^ mode3.lcase() ^ " supplier cannot be found";
			goto EOF_439;

		}

		//check right type of supplier
		type = (mode.field(".", 3)).substr(1, 1);
		if (type == "N") {
			type = "P";
			goto 4556;
		}
		if (type == "1") {
			type = "";
			goto 4556;
		}
		if (type == "2") {
			type = "M";
			goto 4556;
		}
		if (type == "3") {
			type = "P";
		}
		if (type and not (supplier.a(13)).index(type, 1)) {
force error here TODO: check trigraph following;
			temp = (supplier.a(13) == "P") ? "Production" : "Media";
			msg = supplier.a(1) ^ " " ^ (DQ ^ (win.is ^ DQ)) ^ "||This supplier is a " ^ temp ^ "|supplier and cannot be used here.|";
			goto EOF_439;
		}

		//get any supplementary fields
		if (not interactive) {
			RECORD.transfer(saverecord);
			supplier.transfer(RECORD);
			call suppliersubs("POSTREAD2");
			RECORD.transfer(supplier);
			saverecord.transfer(RECORD);
		}

		return 0;
selectsupplier:

		if (not LISTACTIVE) {
			msg = "PLEASE SETUP SOME SUPPLIERS FIRST";
			//print char(7):
			call mssg(msg);
			return 0;
		}

		//user selects
		var tt!!! = "SUPPLIER";
		if (mode3) {
			tt!!! ^= "." ^ mode3;
		}
		call catalyst("P", "ADPOPUPS*" ^ tt!!!);

		win.is = ANS;
		if (not win.is) {
			win.valid = 0;
			return 0;
		}
		if (mode.substr(1, 2) == "F2") {
			DATA ^= "" "\r";
		}

		win.is = ANS;

	/*;
		case mode='F2.EXECUTIVE';
	f2.executive:
			indexfile='JOBS';
			if datafile='CLIENTS' then indexfile='BRANDS';
			call catalyst('V',indexfile:',EXECUTIVE_CODE,S');
			if @ans else return 0;

			//stop trying to lowercase executives
			//temp=@ans
			//convert @lower.case to '' in temp
			//if temp ne @ans else @ans=capitalise(@ans)
	*/
		goto 6505;
	}
	if (mode == "VAL.EXECUTIVE") {

		var indexfile = "JOBS";
		if (win.datafile == "CLIENTS") {
			indexfile = "BRANDS";
		}

		if (win.is == "" or win.is == win.isorig) {
			return 0;
		}
		var DICT = DICT;
		if (DICT.open("DICT", indexfile)) {
			temp = win.is.trim();
			temp.swapper(" ", "]&");
			temp ^= "]";
			temp = "EXECUTIVE_CODE" ^ VM ^ win.is ^ FM;
			call btreeextract(temp, indexfile, DICT, jobnos);
			if (jobnos == "" and win.is ne USERNAME) {
				if (win.templatex ne "JOBS" and win.templatex ne "CLIENTS") {
					msg = DQ ^ (win.is ^ DQ) ^ " - INVALID EXECUTIVE OR HAS NOT BEEN USED";
					goto EOF_439;
				}else{
					if (not(decide(DQ ^ (win.is ^ DQ) ^ "|Executive has not been used before.|Is " ^ (DQ ^ (win.is ^ DQ)) ^ " a new executive ?", "No - Search for an executive" _VM_ "Yes" _VM_ "Cancel", reply))) {
						reply = 3;
					}
					if (reply == 3) {
						win.valid = 0;
						return 0;
					}
					//if reply=1 then goto f2.executive
				}
			}
		}
		DICT = DICT;
	/*;
		CASE MODE[1,16]='F2.BOOKING.ORDER';
			DISPLAY.FIELDS='BRAND_NAMEýPERIODýVEHICLE_CODEýREýORDER_NO';
			multi=mode[-1,1]='S';
			CALL USEINDEX(multi,'BOOKING.ORDERS',DISPLAY.FIELDS,'');
			if @ans then data '';

		CASE MODE[1,19]='F2.PRODUCTION.ORDER';
			DISPLAY.FIELDS='BRAND_NAMEýPERIODýPRODUCTION_TYPEýSUPPLIER_NAMEýSUPP_INV_NO';
			multi=mode[-1,1]='S';
			CALL USEINDEX(multi,'PRODUCTION.ORDERS',DISPLAY.FIELDS,'');
			if @ans then data '';

		CASE MODE[1,21]='F2.PRODUCTION.INVOICE';
			DISPLAY.FIELDS='BRAND_NAMEýPERIODýEXECUTIVE_CODEýDESC_ABBR2ýSTATUS';
			multi=mode[-1,1]='S';
			CALL USEINDEX(multi,'PRODUCTION.INVOICES',DISPLAY.FIELDS,'');
			if @ans then data '';

		case mode[1,9]='F2.CLIENT';
			storedict=@dict;
			call safeselect('SELECT CLIENTS BY NAME WITH AUTHORISED (S)');
			CALL CATALYST('P','ADPOPUPS*':field(mode,'.',2));
			if @ans then data '';
			@dict=storedict;
	*/
		goto 6505;
	}
	if (mode.substr(1, 10) == "VAL.CLIENT") {
		if (win.is == "") {
			return 0;
		}

		if (win.is == win.isorig or win.is == "") {
			return 0;
		}

		for (var isn = 1; isn <= win.is.count(VM) + (win.is ne ""); ++isn) {

			clientcode = win.is.a(1, isn);

			//check client exists
			var client;
			if (client.read(agy.clients, clientcode)) {

				//check allowed to access this client
				if (not(validcode2("", clientcode, "", "", msg))) {
					goto EOF_439;
				}

				//search for clients by name
			}else{
				gosub findclient();
				if (not win.valid) {
					return 0;
				}
				//nb clientcode can be mv'ed if mode ends in 'S'
				win.is.r(1, isn, clientcode);
				ANS = "";
			}

		};//isn;
	/*;
		case mode[1,8]='F2.BRAND';

			//determine clients
			client.codes=field(mode,',',2);
			if client.codes='' then;
				call agency.subs('F2.CLIENT');
				if @ans else return 0;
				@data='';
				client.codes=@ans;
				convert fm to '*' in client.codes;
				end;
			swap '*' with vm in client.codes;

			//find brands for selected clients
			brandcodes='';
			nclients=count(client.codes,vm)+1;
			for clientn=1 to nclients;
				read client from clients,client.codes<1,clientn> else client='';
				brandcodes<1,-1>=client<2>;
				next clientn;

			//select brands which are authorised
			//cmd='SELECT BRANDS BY SEQUENCE'
			//to get them sorted by group
			cmd='SELECT BRANDS';
			andx='';
			//if brandcodes then
			// temp=brandcodes
			// swap vm with '" "' in temp
			// cmd:=quote(temp)
			// end
			if client.codes then;
				temp=client.codes;
				swap vm with '" "' in temp;
				cmd:=' WITH CLIENT_CODE ':quote(temp);
				andx=' AND';
				end;
			cmd:=' BY BRAND_LEVELS_S';
			cmd:=andx:' WITH AUTHORISED';
			call safeselect(cmd:' (S)');
			if @list.active else;
				msg='NO BRANDS FOUND';
				return invalid();
				end;

			//user selects brands
			//call msg2('','DB',buffer,'')
			CALL CATALYST('P','ADPOPUPS*':FIELD(field(mode,'.',2),',',1));
			if @ans then @data=crlf;
	*/

	if (mode.substr(1, 9) == "VAL.BRAND") {

	}else if (win.is == win.isorig or win.is == "") {
			return 0;
		}

		for (var isn = 1; isn <= win.is.count(VM) + (win.is ne ""); ++isn) {

			brandcode = win.is.a(1, isn);

			//check brand exists
			var brand;
			if (brand.read(agy.brands, brandcode)) {

				//check allowed to access this brand
				//if validcode('BRAND',brandcode,msg) else goto invalid
				if (not(validcode2("", "", brandcode, agy.brands, msg))) {
					goto EOF_439;
				}

				//search for brands by name
			}else{
				gosub findbrand();
				if (not win.valid) {
					return 0;
				}
				//nb brandcode can be mv'ed if mode ends in 'S'
				win.is.r(1, isn, brandcode);
				ANS = "";
			}

		};//isn;

		goto 6505;
	}
	if (mode == "DEF.MARKET") {
		if (not win.wlocked) {
			return 0;
		}
		win.isdflt = agy.agp.a(37);
		if (win.isdflt) {
			return 0;
		}

		if (agy.agp.a(2) == "CYP") {
			ANS = "CYP";
			//case base.currency.code='DHM';@ans='UAE'
			//case 1;@ans='UAE'
		}
		if (ANS and win.is == "") {
			DATA ^= "" "\r";
		}
	/*;
		case mode='F2.MARKET';
			call catalyst('P','ADPOPUPS*MARKET');
			if @ans then data '';
	*/
		goto 6505;
	}
	if (mode == "VAL.MARKET") {
		if (win.is == win.isorig or win.is == "") {
			return 0;
		}

		call generalsubs("VAL.ALPHANUMERIC");

		var market;
		if (not(market.read(agy.markets, win.is))) {
			msg = DQ ^ (win.is ^ DQ) ^ " - market code does not exist";
			goto EOF_439;
		}
	/*;
		case mode='F2.PERIOD';
			period=field(date() 'D2/E','/',2)+0;
			year=field(date() 'D2/E','/',3);
			options='';
			for i=1 to 12;
				options<-1>=period:'/':year;
				period+=1;
				if period>12 then;
					period=1;

					year=(year+1) 'R(0)#2';
					end;
				next i;
			if decide('',options,reply) else return 0;
			@ans=options<reply>;
			data '';
	*/
		goto 6505;
	}
	if (mode == "DEF.PERIOD") {
		if (win.is) {
			return 0;
		}
		win.isdflt = (var().date()).oconv("D2/E").field("/", 2, 2);
		if (win.isdflt.substr(1, 1) == "0") {
			win.isdflt.splicer(1, 1, "");
		}

		goto 6505;
	}
	if (mode == "VAL.PERIOD") {
		if (win.is == "" or win.is == win.isorig) {
			return 0;
		}

		//default year to the year of the period
		if (win.is.match("0N")) {
			win.is ^= ((var().date()).oconv("D2/")).substr(6, 3);
		}

		//trim leading zeroes
		if (win.is.substr(1, 1) == "0") {
			win.is.splicer(1, 1, "");
		}

		//check month/year format
		var month = win.is.field("/", 1);
		if (not(var("1" _VM_ "2" _VM_ "3" _VM_ "4" _VM_ "5" _VM_ "6" _VM_ "7" _VM_ "8" _VM_ "9" _VM_ "10" _VM_ "11" _VM_ "12").a(1).locateusing(month, VM, temp))) {
badperiod:
			msg = "PLEASE ENTER PERIOD AS \"MONTH/YEAR\"|(EG \"1/92\")";
			goto EOF_439;
		}
		var year = (win.is.field("/", 2)).substr(-2, 2);
		if (not year.match("2N")) {
			goto badperiod;
		}

		win.is = month ^ "/" ^ year;

		goto 6505;
	}
	if (mode == "VAL.YEARPERIOD") {
		if (win.is == "") {
			return 0;
		}
		var nn = win.is.count(VM) + 1;
		for (var ii = 1; ii <= nn; ++ii) {
			if (not((win.is.a(1, ii)).match("2N\".\"2N"))) {
				msg = DQ ^ (win.is.a(1, ii) ^ DQ) ^ " - INCORRECT FORMAT|PLEASE ENTER YEAR.PERIOD (EG 97.03)";
				goto EOF_439;
			}
		};//ii;
	/*;
		case mode[1,7]='F2.TYPE';
			req.type='';
			title='';
			goto f2.type;

		CASE MODE[1,13]='F2.MEDIA.TYPE';
			REQ.TYPE='M';
			TITLE='Media';
			GOTO F2.TYPE;

		CASE MODE[1,18]='F2.PRODUCTION.TYPE';
			REQ.TYPE='P';
			TITLE='Production';

	F2.TYPE:
			reqdeptcode=field(mode,',',2);
			mode2=field(mode,'.',4);

			//get prevous row number
			if len(popup.row.buffer)>4000 then popup.row.buffer='';
			//LOCATE 'F2.MEDIA.':req.type IN POPUP.ROW.BUFFER<2> SETTING SENTENCEN ELSE
			LOCATE mode IN POPUP.ROW.BUFFER<2> SETTING SENTENCEN ELSE;
				POPUP.ROW.BUFFER<2,sentencen>=mode;
				END;
			median=POPUP.ROW.BUFFER<1,SENTENCEN>;

			//SELECT MEDIA.TYPES
			call safeselect('SSELECT MEDIA.TYPES BY PRODUCTION BY DEPARTMENT BY GROUP BY SEQ BY NAME (S)');

			gosub gethasaccounts;

			datax='';
			mediatypecodes='';
			LOOP;
		while (true) {
				READNEXT keyx ELSE keyx='';
			WHILE keyx;
				READ REC FROM MEDIA.TYPES,keyx THEN;
					if mode2='NOGROUP' and rec<10>='' and (rec<5>='' and rec<6>='') then goto notwanted;
					if hasaccounts then;
						if mode2='WITHACNO' and rec<5>='' and rec<6>='' then goto notwanted;
						if mode2='WITHINCOMEACNO' and rec<5>='' then goto notwanted;
						if mode2='WITHCOSTACNO' and rec<6>='' then goto notwanted;
						end;
					if req.type='M' and rec<3> ne '' then goto notwanted;
					if req.type='P' and rec<3> ne 'Y' then goto notwanted;
					//if reqdeptcode and rec<12> ne reqdeptcode then goto notwanted
					if rec<12> ne reqdeptcode then goto notwanted;
					//LOCATE REC<1> IN datax BY 'AL' USING FM SETTING FN ELSE NULL
					LOCATE REC<1> IN datax USING FM SETTING FN ELSE NULL;
					datax=INSERT(datax,FN,0,0,REC<1>:VM:keyx:vm:rec<10>:vm:rec<2>:vm:rec<5,1,1>:vm:rec<6,1,1>);
					mediatypecodes=insert(mediatypecodes,1,fn,0,keyx);
	notwanted:
					END;
				REPEAT;
			OPTIONS='1:30:L::':TITLE:' Type\2:5:L::Code\3:5:L::Group\4:3:::Seq\5:10:::Income A/c\6:10:::Cost A/c';
			if req.type='P' then options:='\3:5:L::Group';
			if field(mode,',',1)[-1,1]='S' then;
				multi=1;
				t='(s)';
			end else;
				multi=0;
				t='';
				end;
			@ANS=POP.UP(0,5,'',datax,OPTIONS,'R',multi,'Which type':t:' do you want ?','','','2':fm:median,'');
			if @ans then;
				data '';
				locate @ans in mediatypecodes<1> setting median then popup.row.buffer<1,sentencen>=median;
				end;

		case mode[1,17]='F2.MEDIAPROD.TYPE';
			REQ.TYPE='';
			TITLE='';
			GOTO F2.TYPE;

			//perform 'SSELECT MEDIA.TYPES BY PRODUCTION BY SEQ (S)'
			//call catalyst('P','ADPOPUPS*MEDIA.TYPE')
			//if @ans then data ''
	*/
			goto 6505;
		}
		if (mode.substr(1, 18) == "VAL.MEDIAPROD.TYPE") {
			if (win.is == "" or win.is == win.isorig) {
				return 0;
			}

			//read temp from media.types,is else
			// msg=quote(is):' - NO SUCH MEDIA OR PRODUCTION TYPE'
			// goto invalid
			// end

			var reqtype = "*";
			var title = "";
			gosub valtype();

			//call general.subs('VAL.ALPHANUMERIC')

			goto 6505;
		}
		if (mode.substr(1, 14) == "VAL.MEDIA.TYPE") {
			if (win.is == "" or win.is == win.isorig) {
				return 0;
			}

			var reqtype = "";
			var title = "MEDIA";
			goto valtype;

			goto 6505;
		}
		if (mode.substr(1, 19) == "VAL.PRODUCTION.TYPE") {
			if (win.is == "" or win.is == win.isorig) {
				return 0;
			}

			var reqtype = "Y";
			var title = "PRODUCTION";
}

subroutine valtype() {
			var typerec;
			if (not(typerec.read(agy.mediatypes, win.is))) {
				msg = DQ ^ (win.is ^ DQ) ^ " " ^ title ^ " TYPE DOES NOT EXIST";
				goto EOF_439;
			}
			if (reqtype ne "*") {
				if (typerec.a(3) ne reqtype) {
					msg = DQ ^ (win.is ^ DQ) ^ " - IS NOT A " ^ title ^ " TYPE";
					goto EOF_439;
				}
			}

			var mode2 = mode.field(".", 4);
			msg = capitalise(title) ^ " Type :" ^ typerec.a(1) ^ " " ^ (DQ ^ (win.is ^ DQ));

			//check group
			if (mode2 == "NOGROUP") {
				if (typerec.a(10)) {
					msg ^= "|has a group code itself and cannot be used here.";
					goto EOF_439;
				}
			}

			//check account numbers
			gosub gethasaccounts();
			if (hasaccounts) {
				temp = "";
				if (mode2 == "WITHACNO" and typerec.a(5) == "" and typerec.a(6) == "") {
badtype:
					msg ^= "|does not have " ^ temp ^ "account numbers specified|and cannot be used until they are.";
					goto EOF_439;
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
	/*;
		case mode[1,9]='F2.MARKET';
			call catalyst('P','ADPOPUPS*':field(mode,'.',2));
	*/
			goto 6505;
		}
		if (1) {
			msg = DQ ^ (mode ^ DQ) ^ " MODE NOT RECOGNISED IN AGENCY.SUBS";
			goto EOF_484;
		}
L6505:
		return;

}

subroutine getmode3() {

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
					if (interactive) {
						if (not(decide("", "Media suppliers" _VM_ "Production suppliers" _VM_ "Both", reply))) {
							RETURN TO getmode3exit;
						}
					}else{
						reply = 3;
					}
					if (reply == 1) {
						mode3 = "MEDIA";
						goto 6654;
					}
					if (reply == 2) {
						mode3 = "PRODUCTION";
					}
				}else{
					mode3 = "PRODUCTION";
				}
			}
		}

		if (mode3 == "M" or mode3 == 2) {
			mode3 = "MEDIA";
		}
		if (mode3 == "N" or mode3 == 3) {
			mode3 = "PRODUCTION";
		}
		type = mode3.substr(1, 1);
		mode = mode.fieldstore(".", 3, 1, mode3);

getmode3exit:
		return;

}

subroutine findbrand() {

		var brandcodes = "";
		var storedict = DICT;
		if (DICT.open("DICT", "BRANDS")) {

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
			DICT = storedict;
		}

		brandcodes.converter(VM, FM);

		//suppress invalid brands
		nn = brandcodes.count(FM) + 1;
		for (var ii = nn; ii >= 1; --ii) {
			//if validcode('BRANDS',brandcode) else
			if (not(validcode2("", "", brandcodes.a(ii), agy.brands, msg))) {
				brandcodes.eraser(ii);
				ii -= 1;
				nn -= 1;
			}
		};//ii;

		if (not brandcodes) {
			//cannot be found is a trigger in the client
			msg = DQ ^ (brandcode ^ DQ) ^ " brand cannot be found";
			brandcode = "";
			goto EOF_439;
		}

		if (not interactive) {
			ANS = brandcodes;
			win.valid = 0;
			return;
		}

		//turn into an active select list
		var lists;
		if (not(lists.open("LISTS", ""))) {
			call fsmsg();
			var().stop();
		}
		call makelist("", brandcodes, "", "");
		lists.deleterecord(temp);
		//call note(temp)

		if ((mode.field(",", 1)).substr(-1, 1) == "S") {
			temp = "S";
		}else{
			temp = "";
		}
		call catalyst("P", "ADPOPUPS*BRAND" ^ temp);
		brandcode = ANS;
		if (not brandcode) {
			win.valid = 0;
		}

		return;

}

subroutine findclient() {

		var clientcodes = "";
		storedict = DICT;
		if (DICT.open("DICT", "CLIENTS")) {

			temp = clientcode.trim();
			temp.swapper(" ", "]&");
			temp ^= "]";
			temp = "SEQUENCE.XREF" ^ VM ^ temp ^ FM;

			call btreeextract(temp, "CLIENTS", DICT, clientcodes);
			DICT = storedict;
		}

		clientcodes.converter(VM, FM);

		//suppress invalid clients
		nn = clientcodes.count(FM) + 1;
		for (var ii = nn; ii >= 1; --ii) {
			if (not(validcode2("", clientcodes.a(ii), "", "", msg))) {
				clientcodes.eraser(ii);
			}
		};//ii;

		if (not clientcodes) {
			//cannot be found is a trigger in the client
			msg = DQ ^ (clientcode ^ DQ) ^ " client cannot be found";
			clientcode = "";
			goto EOF_439;
		}

		if (not interactive) {
			ANS = clientcodes;
			win.valid = 0;
			return;
		}

		//turn into an active select list
		if (not(lists.open("LISTS", ""))) {
			call fsmsg();
			var().stop();
		}
		call makelist("", clientcodes, "", "");

		if ((mode.field(",", 1)).substr(-1, 1) == "S") {
			temp = "S";
		}else{
			temp = "";
		}
		call catalyst("P", "ADPOPUPS*CLIENT" ^ temp);
		clientcode = ANS;
		if (not clientcode) {
			win.valid = 0;
		}

		return;

}

subroutine gethasaccounts() {

		var balancesfile;
		if (balancesfile.open("BALANCES", "")) {
			hasaccounts = getreccount(balancesfile, "", "");
		}else{
			hasaccounts = "";
		}
		return;

}


libraryexit()