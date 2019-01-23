#include <exodus/library.h>
libraryinit()

#include <readagp.h>
#include <generalsubs2.h>
#include <validcode2.h>
#include <validcode3.h>
#include <singular.h>
#include <authorised.h>
#include <sch_join_invoices.h>
#include <plansubs4.h>
#include <plansubs5.h>
#include <agencysubs.h>
#include <plansubs9.h>
#include <chklic.h>
#include <initcompany.h>
#include <sysmsg.h>
#include <getreccount.h>
#include <updanalysis.h>
#include <plansubs2.h>
#include <locking.h>
#include <updads.h>
#include <flushindex.h>
#include <updads2.h>
#include <addcent.h>

#include <agy.h>
#include <gen.h>
#include <fin.h>
#include <win.h>

#include <window.hpp>

var msg;
var ndecs;
var xx;
var ln;
var taxn;
var ii2;
var booked;//num
var USER4;
var dategrid;
var nspots;//num
var spotmat;
var massad;//num
var oldnspots;
var period;
var yearx;
var startx;
var stopx;//num
var nweeksx;//num
var overlapx;//num
var underlapx;
var wsmsg;

function main(in mode) {
	//c med
	//garbagecollect;
	//for taxes

	//y2k2
	//see also plan.subsX and evaluate.subs
	var doubletilde = var().chr(247);

	//see also clear.inv

	var joinfns = 215 ^ VM ^ 216 ^ VM ^ 217 ^ VM ^ 226 ^ VM ^ 227 ^ VM ^ 228;
	var njoinfns = joinfns.count(VM) + 1;

	call readagp();

	if (mode.field(".", 1) == "PREAPP") {
		win.registerx(10) = mode.field(".", 2);

	} else if (mode == "READ") {

		if (RECORD.read(win.srcfile, ID)) {

			//done in postread now
			//declare function validcode2
			//if validcode2({COMPANY_CODE},{CLIENT_CODE},{BRAND_CODE},brands,msg) else
			// gosub invalid
			// goto reset.read
			// end

			return 0;
		}else{
			RECORD = "";
		}

	} else if (((mode == "POSTREAD") or (mode == "POSTREAD2")) or (mode == "POSTREAD3")) {

		//postread=dos interactive version
		//postread2=intranet non interactive
		//postread3=system level none-validated version (dos or intranet) eg import
		if (mode ne "POSTREAD3") {

			//option to read previous versions
			call generalsubs2(mode);
			if (not(win.valid)) {
				return 0;
			}

			gosub security(mode);
			if (not(win.valid)) {
				win.reset = 5;
				return 0;
			}

			//restrict access based on company and brand
			if (not(validcode2(calculate("COMPANY_CODE"), "", RECORD.a(11), agy.brands, msg))) {
				win.reset = 5;
				return invalid(msg);
			}

			//restrict access based on market and FIRST vehicle only!
			if (not(validcode3(RECORD.a(14), "", RECORD.a(20, 1), agy.vehicles, msg))) {
				win.reset = 5;
				return invalid(msg);
			}

			//restrict access to own plans/schedules
			if (RECORD.a(26) and RECORD.a(26) ne USERNAME) {
				if (RECORD.a(26) ne USERNAME.xlate("USERS", 1, "X")) {
					if (not(authorised(singular(win.datafile) ^ " ACCESS OTHERS", msg, "AM"))) {
						msg = ID ^ " belongs to " ^ RECORD.a(26) ^ FM ^ FM ^ msg;
						win.reset = 5;
						return invalid(msg);
					}
				}
			}

			//prevent working on unapproved schedules
			if (not(RECORD.a(9))) {
				if (not(authorised("SCHEDULE UPDATE UNAPPROVED", msg))) {
					//call msg(msg)
					gosub unlockrecord();
				}
			}

			if ((ID.substr(-2,2) == "*I") or (ID[1] == "%")) {
				//xx=UNLOCKrecord(datafile,SRC.FILE,@ID)
				//WLOCKED=0
				gosub unlockrecord();
			}

		}

		//prevent certain users from creating their own quote numbers
		if (((win.wlocked and (RECORD == "")) and not not SYSTEM.a(33)) and not USER3.index("RECORDKEY")) {
			if (not(authorised(singular(win.datafile) ^ " CREATE OWN NO", msg, ""))) {
				msg = DQ ^ (ID ^ DQ) ^ " does not exist and" ^ FM ^ FM ^ msg;
				win.reset = 5;
				return invalid(msg);
			}
		}

		if (RECORD == "") {

			if (win.wlocked) {

				//insist that schedule codes have some numbers in them
				//to prevent accidental use of accrual a/cs that happen to have
				//the same account number
				//schedule codes should always refer to some numerical period
				//otherwise impossible to enter another one for a different period
				var tt = ID;
				tt.converter("0123456789", "");
				if (tt == ID) {
					msg = DQ ^ (ID ^ DQ) ^ " should include some numerical part to indicate starting period." ^ FM ^ FM ^ "E.g. " ^ ID ^ 1001 ^ " for January 2010";
badnew:
					win.reset = 5;
					return invalid(msg);
				}

				//limit schedule key size
				tt = agy.agp.a(124);
				if (tt and (ID.length() > tt)) {
					msg = DQ ^ (ID ^ DQ) ^ " maximum " ^ tt ^ " characters";
					goto badnew;
				}

				//new records are marked with split extras
				RECORD.r(28, agy.agp.a(5));
				win.orec.r(28, RECORD.a(28));

				//technical version
				//but beware copying old versions would result in blank in UI
				//so be consistent and new schedules have it blank until updated in prewrite
				//r<253>=technicalversion
				//orec<253>=r<253>

				//force companycode if new record and companycode in key
				//if index(agp<63>,'<COMPANY>',1) then
				if ((win.datafile == "SCHEDULES") or (agy.agp.a(71) == "")) {
					tt = agy.agp.a(63);
					}else{
					tt = agy.agp.a(71);
				}
				if (ID == win.isdflt) {
					tt = "";
					if (tt.index("<COMPANY>")) {
						tt = gen.gcurrcompany;
					}
					if (not tt) {
						var compcodex = ID.convert("0123456789/-", var(12).space()).trimb();
						if (compcodex.match("1A0A")) {
							var tcompany;
							if (tcompany.read(gen.companies, compcodex)) {
								tt = compcodex;
							}
						}
					}
					if (tt) {
						RECORD.r(187, tt);
						win.orec.r(187, RECORD.a(187));
					}
				}

				//!indicate smdates
				//r<220>=1
				//orec<220>=1

				//indicate schedule format 2 (smdates and 6 extras columns)
				RECORD.r(220, 2);
				win.orec.r(220, 2);

			}

			//if interactive and datafile='PLANS' and wlocked then data ''

			return 0;

		}

		//all records are marked if splitextras is "1"
		//in order to help DOS data entry and update logic
		if (agy.agp.a(5) == 1) {
			RECORD.r(28, agy.agp.a(5));
			win.orec.r(28, RECORD.a(28));
		}

		//skip heading
		if (mode == "POSTREAD") {
			win.winext = win.registerx(1).a(20);
		}

		//backward compatible with prices (copy from bills)
		//35/137/36/75=37/140/38/62
		if (not(RECORD.a(35).length())) {
			if (not(RECORD.a(140).length().oconv(RECORD.a(36).length() + RECORD.a(75).length()))) {
				var fn1s = "37,140,38,62";
				var fn2s = "35,137,36,75";
				for (var fnn = 1; fnn <= 4; ++fnn) {
					var tt = RECORD.a(fn1s.field(",", fnn));
					RECORD.r(fn2s.field(",", fnn), tt);
				};//fnn;
			}
		}

postreadprep:
/////////////

		//always allow read only access/copy of large schedules (avoid adding data)
		if (not win.wlocked and (RECORD.length() > 48000)) {
			return 0;
		}

		if (win.wlocked) {

			//refresh currency decimal places
			if (ndecs.readv(gen.currencies, RECORD.a(13), 3)) {
				RECORD.r(1, ndecs);
			}

			//setup pointers to OREC lines (to track deletions and insertions)
			var nlines = RECORD.a(20).count(VM) + (RECORD.a(20) ne "");
			var tt = "";
			for (var linen = 1; linen <= nlines; ++linen) {
				tt.r(1, linen, linen);
			};//linen;
			win.orec.r(19, tt);
			RECORD.r(19, tt);

			//backward compatible with no vehicle currency
			var temp = RECORD.a(45);
			if (not temp) {
				for (var linen = 1; linen <= nlines; ++linen) {
					if (RECORD.a(20, linen)) {
						//r<40,LINEN>=1
						//r<44,LINEN>=1
						//r<45,LINEN>=BASE.CURRENCY.CODE
						win.reset = 5;
						msg = "Vehicle currency code is missing in line " ^ linen;
						return invalid(msg);
					}
				};//linen;
			}

			//backward compatible with no gross unit costs
			//TEMP=r<41>
			//CONVERT VM TO '' IN TEMP
			//IF TEMP ELSE
			// r<41>=r<37>
			// r<42>=r<37>
			// END

		}

		//backward compatible with no currency code
		if (not(RECORD.a(13))) {
			RECORD.r(13, agy.agp.a(2));
			win.orec.r(13, agy.agp.a(2));
		}

			/*move company code to its own field;
			accno=@record<186>;
			companycode=field(accno<1,1,1>,',',2);
			if companycode then;
				swap ',':companycode with '' in accno;
				@record<186>=accno;
				orec<186>=accno;
				@record<187>=companycode;
				orec<187>=companycode;
				end;
			*/

		if (win.datafile == "SCHEDULES") {

			//get invoice amounts

			//get these for speed to avoid further lookup from the client program
			MV = 0;
				/*done in JOIN_INVOICES for loop efficiency now;
				tt={INV_AMOUNT}
				r<215>=tt;
				@mv=0;
				r<216>={INV_TOT_COST_BASE}
				*/
			if (RECORD.length() < 60000) {
				RECORD.r(217, calculate("PROFORMA_DATE"));
			}
			//tt={JOIN_INVOICES}
			call sch_join_invoices();

			//copy joined fields to orec
			for (var ii = 1; ii <= njoinfns; ++ii) {
				var fn = joinfns.a(1, ii);
				win.orec.r(fn, RECORD.a(fn));
			};//ii;

			//add all material letters to material table
			if (RECORD.length() < 60000) {
				var dategrids = RECORD.a(22).ucase();
				//smdates ... this works ok while materials are single letters
				dategrids.converter(VM ^ SVM ^ " 1234567890", "");
				for (var ii = 1; ii <= dategrids.length(); ++ii) {
					var temp = dategrids[ii];
					if (UPPERCASE.index(temp)) {
						if (not(RECORD.a(184).locateusing(temp, VM, xx))) {
							RECORD.r(184, -1, temp);
						}
					}
				};//ii;
				win.orec.r(184, RECORD.a(184));
				//if mode='POSTREAD' then amv.vars<2,3>=count(r<184>,vm)+(r<184> ne '')
			}

		}

		//must be done after setting up fn19 pointers for chk.bkg
		if (win.datafile == "SCHEDULES") {
			call plansubs4("GETPENDINGUPDATES");
		}

		//get any new material codes (if not already on plan/schedule)
		//perhaps this should be forced when not original materials
		//also flag files uploaded
		var matkeys = calculate("MATERIAL_NO");
		var tt = calculate("MATERIAL_LETTER");
		var nmats = tt.count(VM) + (tt ne "");
		for (var matn = 1; matn <= nmats; ++matn) {

			//if repeat then get original schedule + letter
			var matkey = matkeys.a(1, matn);

			//or if not a repeat
			if (not matkey) {
				matkey = ID ^ "." ^ RECORD.a(184, matn);
			}

			//doesnt work for plans because plans dont update the materials file
			var material;
			if (material.read(agy.materials, matkey)) {

				//update the schedules material code from the material file
				if (RECORD.a(199, matn) == "") {
					var matcode = material.a(1);
					//if matcode then
					RECORD.r(199, matn, matcode);
					// end
				}

			}else{
				material = "";
			}

			//if uploaded files are not in the material file, look in uploads folder
			if (not(material.a(16))) {

				//uploads=dirlist2(system<49>:@id:'.':letter:'.*')
				var uploads = oslistf(SYSTEM.a(49) ^ matkey ^ ".*");
				uploads.converter(FM, VM);
				material.r(16, uploads);

				//update the uploads on the material file to avoid dir next time
				//this is also done in GBP UPLOAD.SUBS OPENLOAD called when viewing uploads
				//this could invalidate anybody with material file open for editing
				//if material<13> then
				// write material on materials,matkey
				// end

			}

			//refresh uploaded_filenames (doesnt work for plans since no material file)
			RECORD.r(200, matn, var("*").substr(1,material.a(16) ne ""));
		};//matn;

		if (mode == "POSTREAD") {
			call plansubs5("PROTECTMONTHS");
		}

		if (mode ne "POSTREAD3" and win.wlocked) {

			call agencysubs("CHKCLOSEDPERIOD." ^ mode, msg);
			if (msg) {
				//comment to client
				if (msg.index("WARNING")) {
					//reset=1
					win.reset = -1;
					gosub unlockrecord();
				}else{
					win.reset = -1;
					gosub unlockrecord();
				}
				call note(msg);
			}

			//may prevent editing if (completely) approved
			//there is also locking in the user interface per approved month
			if (RECORD.a(9)) {
				if (not(authorised(singular(win.datafile) ^ " UPDATE AFTER APPROVAL", tt))) {
					//xx=unlockrecord(datafile,src.file,@id)
					//wlocked=0
					msg = tt;
					gosub unlockrecord();
				}
			}

		}

	} else if (mode == "PREDELETE") {

		var op = singular(win.datafile);
		if (USERNAME ne "NEOSYS") {
			op.splicer(1, 0, "#");
		}
		gosub security2(mode, op);
		if (not(win.valid)) {
			return 0;
		}

		if (win.datafile == "SCHEDULES") {

			//prevent deletion if invoiced
			if (RECORD.a(4)) {
				//MSG='YOU CANNOT DELETE A SCHEDULE|AFTER IT HAS BEEN INVOICED||(YOU MAY DELETE ALL THE LINES|AND THEN ISSUE A CREDIT NOTE)'
				call plansubs9(1, msg);
				goto invalid2;
			}

			//prevent deletion if partially booked
			if (RECORD.a(51) ne "" or RECORD.a(57) ne "") {
				//MSG='YOU CANNOT DELETE A PARTIALLY BOOKED SCHEDULE|OR SCHEDULE PENDING CANCELLATIONS'
				call plansubs9(2, msg);
				goto invalid2;
			}

			//prevent deletion if any certification or supplier invoices
			var tt = RECORD.a(67) ^ RECORD.a(159);
			tt.converter(VM ^ SVM ^ TM, "");
			if (tt) {
				//msg='You cannot delete a schedule with certified ads|or with any supplier invoices.|You may remove them first and try again.'
				call plansubs9(21, msg);
				goto invalid2;
			}

			//this is irrelevent, the as invoiced record can only exist if sch has been invoiced
			//DELETE SRC.FILE,@ID:'*I'

		}else{

			//prevent deletion if fully or partially approved plans
			var tt = RECORD.field(FM, 231, 13);
			tt.converter(FM, "");
			if (RECORD.a(9) or tt) {
				//msg='You cannot delete a plan after it has been approved'
				call plansubs9(16, msg);
				goto invalid2;
			}

		}

			/*NOINTERACTIVE;
			if interactive then;
				if decide('!Are you SURE that you want to|completely delete this document ?||(You will not be able to recover|this document if you continue)|','No|Yes',reply) else reply=1;
				if reply=1 then valid=0;
				end;
			*/

		//update version log
		call generalsubs2(mode);

	} else if ((mode == "PREWRITE") or (mode == "PREWRITE2")) {

		//prevent creating new records if no lic
		call chklic(mode, msg);
		if (msg) {
			goto EOF_756;
		}

		//check company code present and exists and switch to right company
		var compcode = calculate("COMPANY_CODE");
		if (not compcode) {
			//msg='Company code is required but missing'
			call plansubs9(6, msg);
			return invalid(msg);
		}
		if (not(xx.read(gen.companies, compcode))) {
			//company is missing
			call plansubs9(20, msg, compcode);
			return invalid(msg);
		}
		if (compcode ne gen.gcurrcompany) {
			call initcompany(compcode);
		}

		//default date
		if (not(RECORD.a(16))) {
			RECORD.r(16, var().date());
		}

		//default version 1
		if (not(RECORD.a(157))) {
			RECORD.r(157, 1);
		}
		//incver=0

		//check brand code
		if (not(RECORD.a(11))) {
			//msg='Brand code is required but missing'
			call plansubs9(3, msg);
			return invalid(msg);
		}

		//check any new currencies
		var currcodes = RECORD.a(13);
		if (currcodes == win.orec.a(13)) {
			currcodes = "";
		}
		if (not(win.orec)) {
			currcodes.r(1, -1, RECORD.a(45));
		}
		var ncurrs = currcodes.count(VM) + (currcodes ne "");
		for (var currn = 1; currn <= ncurrs; ++currn) {
			var currcode = currcodes.a(1, currn);
			if (currcode) {
				if (not(currcodes.a(1).locateusing(currcode, VM, ln))) {
					{}
				}
				if (ln == currn) {
					if (not(gen.currency.read(gen.currencies, currcode))) {
						msg = DQ ^ (currcode ^ DQ) ^ " currency does not exist";
						return invalid(msg);
					}
					var currname = gen.currency.a(1).convert("<>", "()");
					if (gen.currency.a(25) or currname.ucase().index("(STOP)")) {
						msg = currname ^ FM ^ "currency is stopped" ^ FM ^ gen.currency.a(25);
						return invalid(msg);
					}
				}
			}
		};//currn;

		//check campaign
		if (agy.agp.a(92)) {
			if (RECORD.a(251) == "") {
				//msg='Campaign description is required'
				call plansubs9(17, msg);
				return invalid(msg);
			}
		}

		//check no lowercase
		if (ID.ucase() ne ID) {
			call plansubs9(15, msg);
			return invalid(msg);
		}

		//check numeric planned ads
		var tt = RECORD.a(141);
		tt.converter("/+0123456789", "");
		tt = trim(tt, VM, "B");
		if (tt) {
			call plansubs9(19, msg, tt);
			return invalid(msg);
		}

		//check currency code
		if (not(RECORD.a(13))) {
			//msg='Currency code is required but missing'
			call plansubs9(4, msg);
			return invalid(msg);
		}

		//prevent vat/tax code if not configured or no tax reg on company file
		//230=taxcode
		tt = RECORD.a(230);
		if (tt.length()) {
			if (not(fin.taxes.read(DEFINITIONS, "TAXES"))) {
				fin.taxes = "";
			}
			if (not(fin.taxes.a(2).locateusing(tt, VM, taxn))) {
				{}
			}
			if (not fin.taxes.a(4, taxn) or not gen.company.a(21)) {
				msg = "Tax code " ^ tt ^ " cannot be entered in the footing because|Tax A/c has not been setup";
				msg.r(-1, "or company " ^ (DQ ^ (gen.company.a(1) ^ DQ)) ^ " has no Tax Reg. No. setup");
				msg.r(-1, "|You may enter tax in the tax column of the lines");
				return invalid(msg);
			}
		}

		//r<21>=swap('&lt;','<',r<21>)
		//r<23>=swap('&lt;','<',r<23>)
		//r<68>=swap('&lt;','<',r<68>)

		//check period
		if (not(RECORD.a(12))) {
			//msg='Period is required but missing'
			call plansubs9(5, msg);
			return invalid(msg);
		}

		//prevent approval if fully stopped or not authorised to override
		//if authorised to override then dont check client<35>
		//this duplicates the theoretically identical restriction in the UI
		if (RECORD.a(9) and not win.orec.a(9)) {
			var brandcode = RECORD.a(11);
			var brand = brandcode.xlate("BRANDS", "", "X");
			var clientcode = brand.a(1, 1);
			var client = clientcode.xlate("CLIENTS", "", "X");
			tt = (brand.a(2) ^ brand.a(3) ^ client.a(35)).ucase();
			if (tt.index("(STOP)") or tt.index("<STOP>")) {
				if (brand.a(2).ucase().index("(STOP)")) {
					client.r(35, -1, brand.a(2));
				}
				call plansubs9(18, msg, client.a(35));
				return invalid(msg);
			}
			if (client.a(35)) {
				if (not authorised("CLIENT CREATE STOPPED", msg)) {
					call plansubs9(18, msg, client.a(35) ^ FM ^ FM ^ msg);
					return invalid(msg);
				}
			}
		}

		call agencysubs("CHKCLOSEDPERIOD." ^ mode, msg);
		if (msg) {
			goto EOF_756;
		}

		//check exchange rates
		call plansubs5("CHK.RATES");
		if (not(win.valid)) {
			return 0;
		}

		//check dates
		if (win.datafile == "SCHEDULES") {
			tt = RECORD.a(25) - RECORD.a(10);
			if ((tt < 0) or (tt > 366)) {
				//msg='Stop date must be within ':gmaxndays:' days of the start date'
				call plansubs9(7, msg, 366);
				return invalid(msg);
			}
		}

		//fail safe in case a lock has changed
		var temp;
		if (not(temp.read(win.srcfile, ID))) {
			temp = "";
		}
		if (temp.a(193) ne win.orec.a(193)) {
			//msg='SYSTEM ERROR: LOCK FAILURE|SOMEBODY ELSE HAS UPDATED THIS RECORD!'
			call plansubs9(8, msg);
			return invalid(msg);
		}

		var interactive2 = not SYSTEM.a(33);
		//PREWRITE2 is from schedule/booking import/program import
		if (mode == "PREWRITE2") {
			interactive2 = 0;
		}

		//if no changes and the record is already on file then
		//do not do any prewrite updates
		if (RECORD == win.orec) {
			if (temp.read(win.srcfile, ID)) {
				return 0;
			}
		}

		if (win.datafile == "SCHEDULES") {

			//detect/attempt fix of defective vehicle code
			//this is supposedly fixed in user interface around dec 2011 see whats new
			//eg happened in email sent 23 February 2012 10:59
			if (RECORD.a(20).index(SVM)) {
				var(RECORD).oswrite(ID.substr(1,8) ^ "." ^ var().time().substr(-3,3));
				var nn = RECORD.a(20).count(VM) + 1;
				for (var ii = 1; ii <= nn; ++ii) {
					var vcode = RECORD.a(20, ii);
					if (vcode.index(SVM)) {
						tt = RECORD.a(160, ii);
						if (win.orec.a(160).locateusing(tt, VM, ii2)) {
							RECORD.r(20, ii, win.orec.a(20, ii2));
						}
					}
				};//ii;
				tt = DQ ^ (ID ^ DQ) ^ "system error in vehicle code - ";
				if (RECORD.a(20).index(SVM)) {
					tt ^= "couldnt be fixed";
				}else{
					tt ^= "recovered ok";
				}
				call sysmsg(tt);
			}

			//check if exceeded plan and not authorised to do it
			if ((not not SYSTEM.a(33) and not RECORD.a(218)) and mode ne "PREWRITE2") {
				//declare function get.reccount
				var plans;
				if (plans.open("PLANS", "")) {
					if (getreccount(plans, "", "")) {
						call updanalysis("VALIDATE", win.datafile, win.orec);
						msg = ANS;
						if (msg) {
							//xx=unlockrecord('',src.file,@id)
							var msg2 = "YTD PLANNED AMOUNT EXCEEDED (" ^ agy.agp.a(2) ^ ")|";
							//msg[1,0]='Vehicle/Market' 'L#15':'Period' 'R#15':'YTD-Planned' 'R#15':'YTD-Scheduled' 'R#15':'|'
							//msg:='|'
							msg = msg2 ^ "If authorised, you may check the \"Allow Over Plan\" checkbox in the schedule heading||" ^ msg;
							return invalid(msg);
						}
					}
				}
			}

			//update new split unit bill/cost if not invoiced
			//if {SPLITEXTRAS} and not({SPLIT}) then
			//if {SPLITEXTRAS} and not({SPLIT}) and not(@record<4>) then
			if (calculate("SPLITEXTRAS") and not RECORD.a(4)) {
				RECORD.r(64, 1);
				call plansubs2("RECALC_ALL_NETS");
				if (not(win.valid)) {
					return 0;
				}
			}

			//NB after this then one needs to call unlock all
			//otherwise all the other schedules remain locked
			if (RECORD.a(160).index(":")) {
				call plansubs5("DELETEMOVED.PREWRITE");
				if (not(win.valid)) {
					return 0;
				}
			}

			//from here on we are committed and cannot cancel the write
			//or we can but we must call unlock all?
			//////////////////////////////////////////////////////////

			//fix missing schedule version
			if (win.datafile == "SCHEDULES") {
				if (RECORD.a(253) == "") {
					if (RECORD.a(4)) {
						RECORD.r(253, 1);
					}else{
						RECORD.r(253, 2);
					}
				}
			}

			//detect if need to save "as invoiced" schedule
			//(deleting invoiced lines would have already set the flag to do this)
			//deleting invoice lines is not longer allowed in the client
			//TODO check no invoiced lines have been deleted
			//since invoice creation will fail and probably irreversibly lose track/hang
			var nlines = RECORD.a(20).count(VM) + (RECORD.a(20) ne "");
			var storemv = MV;

			if (RECORD.a(4) ne "" and (RECORD.a(7) == "")) {

				//check for any change in dates requiring invoicing
				tt = RECORD.a(167);
				tt.converter(TM ^ SVM ^ VM, "");
				if (tt) {
					RECORD.r(7, "X");
					nlines = 0;
				}

				for (MV = 1; MV <= nlines; ++MV) {

					var ok = 0;

					//detect bill and cost amounts pending invoice
					//like {HAS_AMOUNT_PENDING_BILLING}
					if (calculate("COST") - calculate("INV_COST").sum()) {
					} else if (calculate("COST_BASE") - calculate("INV_COST_BASE").sum()) {
					} else if (calculate("BILL") - calculate("INV_BILL").sum()) {
					} else if (calculate("BILL_BASE") - calculate("INV_BILL_BASE").sum()) {

						//catch new uninvoiced free ads? what about added free to a line though?
						//zzz should check all ads for invoice numbers
						//case {NUMBER} and not(r<50,@mv>)

					//2. ? detect free ads pending invoicing (which dont have pending cost)
					} else {

						ok = 1;

						//following detect changed number of free ads which has no effect on cost
						var oln = RECORD.a(19, MV);
						if (oln) {
							//2002/7/13 locate oln in r<19> setting orecln then
							//locate oln in orec<19> setting orecln then
							//orec no longer has the prep since it is read from file in listen
							var orecln = oln;
							//if changed number of spots ("free" otherwise would change costs above)
							if (RECORD.a(39, MV) ne win.orec.a(39, orecln)) {
								ok = 0;
							}
							if (RECORD.a(43, MV) ne win.orec.a(43, orecln)) {
								ok = 0;
							}
							//moving dates could cause re-invoicing even if no cost implications
							//we need to apply the new invoice number on the new ads
							//now done by detecting any r<167> above
							//if r<22,@mv> ne orec<22,orecln> then ok=0
							// end
						}

					}
//L4549:
					if (not ok) {
						RECORD.r(7, "X");
					}

					if (not ok) {
						MV = nlines;
					}

				};//MV;

			}
			MV = storemv;

			//save "as invoiced" schedule
			if (RECORD.a(7) == "X") {
				RECORD.r(7, "I");

				//condition the "as invoiced" record
				//add pointers to the schedule or delete uninvoiced lines
				//add pointers to the "as invoiced" record to the schedule
				RECORD.r(50, "");
				//no longer remove this as it will be replaced/deleted
				//and is the most reliable indicator that the line has been invoiced
				//orec<inv.ln.fn>=''
				var amvfns = "19" _VM_ "20" _VM_ "21" _VM_ "22" _VM_ "23" _VM_ "24" _VM_ "27" _VM_ "37" _VM_ "38" _VM_ "39" _VM_ "40" _VM_ "41" _VM_ "42" _VM_ "43" _VM_ "44" _VM_ "45" _VM_ "46" _VM_ "47" _VM_ "48" _VM_ "49" _VM_ "50" _VM_ "51" _VM_ "52" _VM_ "54" _VM_ "55" _VM_ "56" _VM_ "62" _VM_ "63" _VM_ "65" _VM_ "67" _VM_ "70" _VM_ "71" _VM_ "72" _VM_ "73" _VM_ "75" _VM_ "76" "" _VM_ "111" _VM_ "114" _VM_ "106" _VM_ "116" _VM_ "126" _VM_ "138" _VM_ "139" _VM_ "140" _VM_ "141" _VM_ "142" _VM_ "143" _VM_ "144" _VM_ "145" _VM_ "146" _VM_ "147" _VM_ "148" _VM_ "149" _VM_ "150" _VM_ "151" _VM_ "152" _VM_ "156" _VM_ "159" _VM_ "160" _VM_ "161" _VM_ "162" _VM_ "164" _VM_ "165" _VM_ "166" _VM_ "167" _VM_ "171" _VM_ "172" _VM_ "173" _VM_ "174" _VM_ "175" _VM_ "176" _VM_ "177" _VM_ "178" _VM_ "179" _VM_ "180" _VM_ "181" _VM_ "182" _VM_ "183" _VM_ "190" _VM_ "191" _VM_ "221" _VM_ "68";
				var namvs = amvfns.count(VM) + 1;

				//build it up on write instead on read for remote operation
				//oldlns=orec<19>
				//noldlns=count(oldlns,vm)+(oldlns<>'')
				var noldlns = win.orec.a(20).count(VM) + (win.orec.a(20) ne "");
				var oldlns = "";
				for (var oldln = 1; oldln <= noldlns; ++oldln) {
					oldlns.r(1, oldln, oldln);
				};//oldln;

				for (var oldln = 1; oldln <= noldlns; ++oldln) {

					//if the line has been invoiced then add a pair of pointers
					if (win.orec.a(46, oldln).length()) {
inved:
						win.orec.r(50, oldln, oldln);
						if (RECORD.a(19).locateusing(oldlns.a(1, oldln), VM, tt)) {
							RECORD.r(50, tt, oldln);
						}

						goto 4900;
					}
					if (win.orec.a(50, oldln)) {
						goto inved;

						//otherwise, if it has not been invoiced yet
						//delete the line from the "as invoiced" orec
						//zzz should never happen!
					}else{

						for (var amvn = 1; amvn <= namvs; ++amvn) {
							var fn = amvfns.a(1, amvn);
							win.orec.eraser(fn, oldln);
						};//amvn;

						oldlns.eraser(1, oldln);

						oldln -= 1;
						noldlns -= 1;

					}
				};//oldln;

				var oldkey = ID ^ "*I";

				//failsafe bugtrap
				if (xx.read(win.srcfile, oldkey)) {
					call locking("UNLOCKALL", "", "", "", win.registerx(7));
					var(RECORD).oswrite("PSE");
					var(win.orec).oswrite("PSEI");
					msg = "SYSTEM ERROR: in PLAN.SUBS " ^ (DQ ^ (oldkey ^ DQ)) ^ " already exists";
					return invalid(msg);
				}

				win.orec.write(win.srcfile, oldkey);

				if (interactive2) {
					var().chr(7).output();
					call mssg("A DEBIT/CREDIT NOTE MUST|BE ISSUED FOR THIS SCHEDULE");
				}

			}

			//make sure any tm that appear in the booking details are stored as
			//char(13) because as.booked.details uses tm as a separator for various info
			temp = RECORD.a(23);
			if (temp.index(TM)) {
				temp.converter(TM, var().chr(13));
				RECORD.r(23, temp);
			}

			//not booked if there are booking cancellations
			if (RECORD.a(57)) {
				booked = 0;

				//not booked if there are any lines no booked or with "as booked"
			}else{
				booked = 1;
				var vehiclecodes = RECORD.a(20);
				nlines = vehiclecodes.count(VM) + (vehiclecodes ne "");
				for (var vehicleno = 1; vehicleno <= nlines; ++vehicleno) {
					var vehiclecode = vehiclecodes.a(1, vehicleno);
					if (vehiclecode) {
						if (RECORD.a(51, vehicleno) == "") {
							booked = 0;
						}
						if (RECORD.a(52, vehicleno) ne "") {
							booked = 0;
						}
					}
				///BREAK;
				if (not booked) break;;
				};//vehicleno;
			}

			//mark as fully booked or not
			if (booked) {
				if (RECORD.a(8) == "") {
					RECORD.r(8, var().date());
				}
			}else{
				if (RECORD.a(8) ne "") {
					RECORD.r(8, "");
				}
			}

			//remove the original multivalue markers
			RECORD.r(19, "");

		}

		//clear joined fns
		for (var ii = 1; ii <= njoinfns; ++ii) {
			var fn = joinfns.a(1, ii);
			RECORD.r(fn, "");
		};//ii;

		//cut off excess field and value marks
		call cropper(RECORD);

		//forced WRITE
		if (win.orec == RECORD) {
			win.orec.r(500, not win.orec.a(500));
		}

		//if incver='Y' then
		// r<157>=r<157>+1
		// r<16>=date()
		// end

		//update version log
		//call general.subs2(mode)

		//missing username etc indicates adding file archive entry
		ii = RECORD.a(263).count(VM) + (RECORD.a(263) ne "");
		if (ii and (RECORD.a(260, ii) == "")) {
			RECORD.r(260, ii, USERNAME);
			RECORD.r(261, ii, var().date() ^ "." ^ var().time().oconv("R(0)#5"));
			RECORD.r(262, ii, STATION);

		}else{

			//update version log
			call generalsubs2(mode);

		}

	} else if ((mode == "POSTWRITE") or (mode == "POSTDELETE")) {

		//delete any lines moved from other records
		if (RECORD.a(160).index(":")) {
			call plansubs5("DELETEMOVED.POSTWRITE");
		}

		//ideally this should be after upd.analysis and flush.index
		//in case of failure eg too many ads etc
		if (win.datafile == "SCHEDULES") {
			USER4.transfer(USER4);
			call updads(mode.substr(5,999));
			win.valid = 1;
			var tt = USER4;
			USER4.transfer(USER4);
			if (tt) {
				call sysmsg(tt, "PLAN.SUBS UPD.ADS " ^ ID);
			}
		}

		call updanalysis(mode.substr(5,999), win.datafile, win.orec);
		msg = ANS;

		call flushindex(win.datafile);

		//if msg and interactive then
		// xx=unlockrecord(datafile,src.file,@id)
		// *declare function get.reccount
		// open 'PLANS' to plans then
		// if get.reccount(plans,'','') then
		// msg[1,0]='|':'Vehicle/Market' 'L#15':'Period' 'R#15':'YTD Budget' 'R#15':'YTD Actual' 'R#15':'||'
		// msg[1,0]='NOTE: YTD BUDGET EXCEEDED||':base.currency.code:'|'
		// msg:='|'
		// gosub note
		// end
		// end
		// end

		call plansubs5("CREATEJOB");

		if (win.datafile == "SCHEDULES") {
			var pendingkey = "PENDINGUPDATES*SCHEDULES*" ^ ID;
			DEFINITIONS.deleterecord(pendingkey);
			

			call plansubs5("EMAIL.CHANGES");

		}

		if (mode == "POSTWRITE") {
			goto postreadprep;
		}

	} else if (mode == "SHOWINV") {
		var t2 = "INV ";
showinv:
		if (not(win.mvx)) {
			msg = "PLEASE SELECT AN INVOICE FIRST";
			goto invalid2;
		}
		var tt = RECORD.a(4, win.mvx);
		execute("GET NEW REPRINTINVS MEDIA " ^ t2 ^ tt);

	} else if (mode == "SHOWCOSTINV") {
		var t2 = "COST ";
		goto showinv;

	} else if (mode == "POSTSCHSTATUS") {

		if (RECORD.a(9) ne win.orec.a(9)) {
			call updads2(ID, RECORD.a(9));

			if (RECORD.a(9) == "") {
				call updanalysis("DELETE", win.datafile, win.orec);
			}else{
				call updanalysis("", win.datafile, win.orec);
			}

		}

		call flushindex(win.datafile);

	} else if (mode == "RECALC") {
		win.displayaction = 5;
		//AMV.ACTION=4
		win.reset = 3;

	} else if (mode == "REORDER") {

		if (not RECORD) {
			return 0;
		}

		if (RECORD.a(4)) {
			msg = "CANNOT REORDER AFTER INVOICING";
			return invalid(msg);
		}

		var vehiclecodes = RECORD.a(20);
		var alldates = RECORD.a(22);

		//start with no lines
		var doc = RECORD;
		var amvfns = "19" _VM_ "20" _VM_ "21" _VM_ "22" _VM_ "23" _VM_ "24" _VM_ "27" _VM_ "37" _VM_ "38" _VM_ "39" _VM_ "40" _VM_ "41" _VM_ "42" _VM_ "43" _VM_ "44" _VM_ "45" _VM_ "46" _VM_ "47" _VM_ "48" _VM_ "49" _VM_ "50" _VM_ "51" _VM_ "52" _VM_ "54" _VM_ "55" _VM_ "56" _VM_ "62" _VM_ "63" _VM_ "65" _VM_ "67" _VM_ "70" _VM_ "71" _VM_ "72" _VM_ "73" _VM_ "75" _VM_ "76" "" _VM_ "111" _VM_ "114" _VM_ "106" _VM_ "116" _VM_ "126" _VM_ "138" _VM_ "139" _VM_ "140" _VM_ "141" _VM_ "142" _VM_ "143" _VM_ "144" _VM_ "145" _VM_ "146" _VM_ "147" _VM_ "148" _VM_ "149" _VM_ "150" _VM_ "151" _VM_ "152" _VM_ "156" _VM_ "159" _VM_ "160" _VM_ "161" _VM_ "162" _VM_ "164" _VM_ "165" _VM_ "166" _VM_ "167" _VM_ "171" _VM_ "172" _VM_ "173" _VM_ "174" _VM_ "175" _VM_ "176" _VM_ "177" _VM_ "178" _VM_ "179" _VM_ "180" _VM_ "181" _VM_ "182" _VM_ "183" _VM_ "190" _VM_ "191" _VM_ "221" _VM_ "68";
		var namv = amvfns.count(VM) + 1;
		for (var ii = 1; ii <= namv; ++ii) {
			doc.r(amvfns.a(1, ii), "");
		};//ii;

		//add or insert each line
		var nlns = RECORD.a(20).count(VM) + (RECORD.a(20) ne "");
		for (var lnx = 1; lnx <= nlns; ++lnx) {
			var newmv = lnx;
			var vehiclecode = vehiclecodes.a(1, lnx);
			dategrid = alldates.a(1, lnx);

			//see if it should be inserted in the middle
			if (doc.a(20).locateusing(vehiclecode, VM, newmv)) {
checkdate:
				if (dategrid <= doc.a(22, newmv)) {
					newmv += 1;
					if (doc.a(20, newmv) == vehiclecode) {
						goto checkdate;
					}
				}
			}

			//insert the line
			//NCOLS=COUNT(AMV.FNS,VM)+1
			for (var coln = 1; coln <= namv; ++coln) {
				var fn = amvfns.a(1, coln);
				var temp = RECORD.a(fn, lnx);
				doc.inserter(fn, newmv, temp);
			};//coln;

		};//lnx;

		RECORD = doc;
		win.amvaction = 4;
		win.reset = 4;
		DATA ^= "" "\r";

	} else {
		msg = DQ ^ (mode ^ DQ) ^ " - INVALID MODE IGNORED";
		call note(msg);
	}
//L6299:
	return 0;

	//selectlines:
	//copyschedule:
	//copydocs:
	return 0;

	/*;
////////////
	SELECTLINES:
////////////
		IF QUESTION ELSE QUESTION='Which line(s) do you want ?';
		VEHICLE.CODES=r<20>;
		SPECS=r<21>;
		IF COPY.EXTRAS.FN THEN;
			EXTRAS=r<COPY.EXTRAS.FN>;
			END;
		NLINES=COUNT(VEHICLE.CODES,@VM)+(VEHICLE.CODES NE '');
		POPUP.DATA='';
		FOR LINEN=1 TO NLINES;
			TEMP=VEHICLE.CODES<1,LINEN>:@VM:SPECS<1,LINEN>;
			IF COPY.EXTRAS.FN THEN TEMP:=@VM:EXTRAS<1,LINEN>;
			POPUP.DATA<LINEN>=TEMP;
			NEXT LINEN;
		PARAMS='1:10:L::VEHICLE\2:20:L::Specification';
		IF COPY.EXTRAS.FN THEN PARAMS:='\3:15:L::Disc/Charges';
		LINENS=POP.UP(2,2,'',POPUP.DATA,PARAMS,'R','1',QUESTION,'','','','P');
		NLINENS=COUNT(LINENS,@FM)+1;
		RETURN;

/////////////
	COPYSCHEDULE:
/////////////
		if datafile='PLANS' then;
			doctype='plan';
			docfile='PLANS';
		end else;
			doctype='schedule';
			docfile='SCHEDULES';
			end;
		goto copydoc2;

/////////
	COPYDOCS:
/////////
		IF @ID ELSE;
			MSG='PLEASE START A DOCUMENT FIRST';
			goto invalid2;
			END;
		doctype='plan';
		docfile='PLANS';

	copydoc2:
/////////
		IF MV THEN;
			NEWMV=MV;
		END ELSE;
			TEMP=r<VEHICLE.CODE.FN>;
			IF TEMP THEN;
				NEWMV=COUNT(TEMP,@VM)+2;
			END ELSE;
				NEWMV=1;
				END;
			END;

	INP.COPYREF:
		COPYDOCID='';
		CALL note2('What is the number of the|':doctype:' that you want to copy ?||(Press [Enter] to see a list of ':doctype:'s)','RCE',COPYDOCID,'');
		IF COPYDOCID EQ CHAR(27) THEN RETURN;
		IF COPYDOCID EQ '' THEN;
			temp=docfile;
			temp[-1,1]='';
			//CALL PLAN.SUBS2('F2.':temp)
			CALL AGENCY.SUBS('F2.':temp);
			if @ans='' then @ans=browse.next;
			IF @ANS ELSE GOTO COPY.ANOTHER;
			COPYDOCID=@ANS;
			@ANS='';
			END;

		//get the document to be copied
		OPEN docfile TO file ELSE;
			CALL FSMSG();
			RETURN;
			END;
		READ DOC FROM file,COPYDOCID ELSE;
			MSG='There is no ':doctype:' number ':QUOTE(COPYDOCID);
			GOSUB NOTE;
			GOTO INP.COPYREF;
			END;

		//do not copy status info of schedules OR plans
		//if doctype='schedule' then
			nfns=count(initfns,',')+1;
			for fn=1 to nfns;
				doc<field(initfns,',',fn)>='';
				next fn;
		// end

		//build list of months in the plan
		if doctype='plan' and datafile='SCHEDULES' then;
			MONTHS=DOC<12>;
			IF DOC<15> THEN;
				MONTH=FIELD(DOC<12>,'/',1);
				YEAR=FIELD(DOC<12>,'/',2);
				STOPMONTH=FIELD(DOC<15>,'/',1);
				STOPYEAR=FIELD(DOC<15>,'/',2);
				LOOP;
					MONTH+=1;
					IF MONTH GT 12 THEN;
						IF addcent(YEAR) LT addcent(STOPYEAR) THEN;
							MONTH-=12;

							year=(year+1) 'R(0)#2';
							END;
						END;
				WHILE MONTH LE STOPMONTH OR addcent(YEAR) LT addcent(STOPYEAR);
					MONTHS<-1>=MONTH:'/':YEAR;
					REPEAT;

				END;

			//select month
			IF DECIDE('Which month do you want to copy ?',MONTHS:'',REPLY) THEN;

				PERIOD=field(MONTHS<REPLY>,'/',1);

				//convert the weekly period to a calendar month
				fromperiod=doc<12>;
				toperiod=doc<15>;
				call wks2mths(period:'',fromperiod,toperiod,field(doc,fm,141,12),mthnumbers);

				doc<39>=mthnumbers<field(period,'/',1)>;

				//cost ads=bill ads
				DOC<43>=DOC<39>;

				//get period details
				gosub getperiod;

				//clear the monthly data
				DOC=FIELDSTORE(DOC,@FM,141,12,'');

				//set the period to the period selected
				DOC<12>=PERIOD;
				mth=field(period,'/',1);
				year=field(period,'/',2);
				//for now schedules are calendar months
				//doc<10>=iconv(mth-1:'/':year,agp<18>)+1
				//doc<25>=iconv(period,agp<18>)
				doc<10>=iconv(mth-1:'/':year,'[DATEPERIOD,1,12]')+1;
				doc<25>=iconv(period,'[DATEPERIOD,1,12]');

				//clear out the "TO" period
				DOC<15>='';*period to;

				//set the date to today's date
				DOC<16>=DATE();

				//clear out the rating data
				//DOC=FIELDSTORE(DOC,@FM,105,8,'')
				//DOC=FIELDSTORE(DOC,@FM,115,8,'')
				//DOC=FIELDSTORE(DOC,@FM,125,8,'')

					skip=1;
				//only copy lines which are used
				IF DECIDE('Do you want to skip lines that are not used ?','Yes|No',REPLY) ELSE REPLY=2;

				IF REPLY=1 THEN;

					TEMP=DOC<39>;
					CONVERT @VM:'0' TO '' IN TEMP;
					if temp='' then;
						msg='There are no ads for period ':period;
						gosub note;
						goto inp.copyref;
						end;

					IF 1 THEN;
						amv.fns=amv.fns1:amv.fns2;
						N=COUNT(AMV.FNS,@VM)+1;
						NLINES=COUNT(DOC<20>,@VM)+(DOC<20> NE '');
						FOR LINEN=1 TO NLINES;
							IF DOC<20,LINEN>='' AND DOC<20,LINEN-1>='' THEN GOTO DELETELINE;
							IF DOC<20,LINEN> AND NOT(DOC<39,LINEN>) THEN;
	DELETELINE:
								FOR ii=1 TO N;
									TEMP=AMV.FNS<1,ii>;
									IF DOC<TEMP> THEN DOC=DELETE(DOC,TEMP,LINEN,0);
									NEXT ii;
								LINEN-=1;
								NLINES-=1;
								END;
							NEXT LINEN;
						END;

					END;
				END;
			end;

		if r='' then;

			r=DOC;

			//clear the existing lines if necessary
			amv.fns=amv.fns1:amv.fns2;
			N=COUNT(AMV.FNS,@VM)+1;
			FOR ii=1 TO N;
				r<AMV.FNS<1,ii>>='';
				NEXT ii;

			END;* of r='';

		IF DECIDE('Do you want to copy all of it ?','',REPLY) ELSE GOTO COPY.ANOTHER;

		//select some lines
		IF REPLY EQ 2 THEN;

			//select lines
			QUESTION='';
			COPY.EXTRAS.FN='';
			TRANSFER r TO RECORD;
			TRANSFER DOC TO r;
			COPY.EXTRAS.FN='';
			GOSUB SELECTLINES;
			TRANSFER r TO DOC;
			TRANSFER RECORD TO r;

			MVS=LINENS;
			IF MVS ELSE GOTO COPY.ANOTHER;
			CONVERT @FM TO @VM IN MVS;
			NMVS=COUNT(MVS,@VM)+1;
		END ELSE;
			MVS='';
			NMVS=COUNT(DOC<VEHICLE.CODE.FN>,@VM)+1;
			END;

		//copy lines over
		copyfns=amv.fns1:amv.fns2;
		//do not copy plan only fns to schedule
		if doctype='plan' and datafile='SCHEDULES' then;
			planfns='171,172,173,174,175,176,177,178,179,180,181,182,183';
			planfns:=vm:'141,142,143,144,145,146,147,148,149,150,151,152,153';
			convert ',' to vm in planfns;
			n=count(planfns,vm)+1;
			for ii=1 to n;
				locate planfns<1,ii> in copyfns<1> setting vn then;
					copyfns=delete(copyfns,1,vn,0);
					end;
				next ii;
			end;
		FOR mvN=1 TO NMVS;
			IF MVS THEN MV=MVS<1,mvN> ELSE MV=mvN;
			EXPLODELINE=0;
			NCOLS=COUNT(copyfns,VM)+1;
			FOR COLN=1 TO NCOLS;
				FN=copyfns<1,COLN>;
				cell=DOC<FN,MV>;

				if doctype='plan' and datafile='SCHEDULES' and (fn=number.fn or fn=cost.number.fn) then;

					//copy over dates if date plan
					convert '/' to sm in cell;
					if fn=number.fn then;
						mthn=field(period,'/',1);
						datelist=doc<170+mthn,mv>;

						//if no datelist then assume nads by week (sep by '/')
						if datelist else;
							//if index(agp<18>,'WEEK',1) then

								//msg='NOT SUPPORTED IN DOS VERSION ANY MORE|BECAUSE NEED TO CONVERT TO SMDATES'
								call plan.subs9(10,msg);
								goto invalid2;

								ndates=doc<25>-doc<10>+1;
								dategrid=space(ndates);

								for weekn=1 to nweeks+1;

									dgptr=weekn*7-6;
									if startd<>realstartx then;
										dgptr+=(realstartx 'D2/E')[1,2]-1;
									end else;
										if underlap and weekn>1 then;
											dgptr+=(realstartx 'D2/E')[1,2]-1-7;
											end;
										end;

									if dgptr>ndates then dgptr=ndates;
									tt=cell<1,1,weekn>+trim(dategrid[dgptr,1]);
									loop;
									while tt gt 9 do;
										dategrid[dgptr,1]='9';
										tt-=9;
										dgptr+=1;
										repeat;
									if tt then dategrid[dgptr,1]=tt;
									next weekn;
								dategrid=trimb(dategrid);
								goto setdategrid;
							// end
							end;

						if datelist then;
							dategrid='';
							call datelist(datelist,dategrid,freegrid,r<10>,r<25>,msg,nads);
	setdategrid:
							//smdates
							//r<dates.fn,newmv>=trimb(dategrid)
							r<dates.fn,newmv>=dategrid;
							end;
						end;

					cell=sum(cell);

					//if not date grid
					//reduce the number by one and indicate that this line must be inserted again
					if wis<dates.fn> else;
						IF cell GT 1 THEN;
							cell-=1;
							DOC<FN,MV>=cell;
							EXPLODELINE=1;
							cell=1;
							END;
						//clear the numbers
						cell='';
						end;

					end;

				r=insert(r,fn,newmv,0,cell);
	copynextcoln:
				next coln;

			IF EXPLODELINE THEN mvN-=1;
			NEWMV+=1;
			NEXT mvN;

	COPY.ANOTHER:
		MV=0;
		IF r<VEHICLE.CODE.FN> THEN;
			TEMP='more';
			TEMP2='No|Yes';
		END ELSE;
			TEMP='';
			TEMP2='';
			END;
		REPLY=DECIDE('Copy ':TEMP:' from another ':doctype:' ?',TEMP2,REPLY);
		IF REPLY EQ 'Yes' THEN GOTO INP.COPYREF;

		//initialise the version date and number
		r<16>=date();
		r<157>=1;
		if doctype='plan' and datafile='SCHEDULES' then r<189>=copydocid;

		call trimexcessmarks(r);

		IF MODE NE 'READ' THEN;
			AMV.ACTION=4;
			RESET=4;
			DATA '';
			END;

		return 0;
	*/

/////////
invalid2:
/////////
	if (not SYSTEM.a(33)) {
		goto EOF_756;
	}
	//@user4:=fm:msg
	USER4.r(-1, "Error: " ^ msg);
	win.valid = 0;
	return 0;

}

subroutine unlockrecord() {
	xx = unlockrecord(win.datafile, win.srcfile, ID);
	win.wlocked = 0;
	return;

}


libraryexit()
