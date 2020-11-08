#include <exodus/library.h>
libraryinit()

#include <createjob.h>
#include <authorised.h>
#include <htmllib.h>
#include <sendmail.h>
#include <sysmsg.h>
#include <plansubs9.h>
#include <locking.h>
#include <agencysubs.h>
#include <singular.h>

#include <agy_common.h>
#include <gen_common.h>
#include <win_common.h>

#include <window.hpp>

var tx;
var anychanges;//num
var head;//num
var ln;//num
var oln;//num
var dictids;
var anydeleted;//num
var attachfilename;
var deletex;
var errormsg;
var replyto;
var params;
var msg;
var lockdesc;
var lockkey;
var otherln;
var compcode;
var newvalue;
var oldvalue;
var op;
var op2;
var wspos;
var wsmsg;
var xx;

function main(in mode) {
	//c med

	#include <agency_common.h>
	#include <general_common.h>
	#include <plans_common.h>

	#define locklist win.registerx(7)
	//equ tstore to register(10)
	#define interactive not(SYSTEM.a(33))

	//garbagecollect;

	if (mode == "CREATEJOB") {

		//skip if automatic creation of media jobs is not configured
		var jobjobtypecode = agy.agp.a(9999);
		//TEST only ATM
		if (not(jobjobtypecode) and (SYSTEM.a(17).substr(-4,4) ne "TEST")) {
			return 0;
		}

		//skip if job already exists
		//TODO resolve potential issue when job, plan or schedule numbers overlap
		//usually plan and schedule numbers are NOT numeric
		//whereas job number often are, or are prefixed
		//MAYBE configure one or both with different prefixes
		var job;
		if (job.read(agy.jobs, ID)) {
			return 0;
		}

		job = "";
		job.r(2, calculate("BRAND_CODE"));
		job.r(14, calculate("COMPANY_CODE"));
		job.r(8, calculate("EXECUTIVE_NAME"));
		var tt = "Media";
		var tt2 = calculate("CAMPAIGN_DESCRIPTION");
		if (tt2) {
			tt ^= ", " ^ tt2;
		}
		job.r(9, tt);
		job.r(1, calculate("PERIOD"));
		job.r(54, var().date());
		job.r(3, jobjobtypecode);
		job.r(12, calculate("MARKET_CODE"));

		call createjob(job, ID);

	} else if (mode == "EMAIL.CHANGES") {

		//changes to approved schedules get emailed, but not unapproved schedules
		if (not(calculate("APPROVAL_DATE"))) {
			return 0;
		}

		//test only ATM
		if (not(SYSTEM.a(17).substr(-4,4) == "TEST")) {
			return 0;
		}
		if (not(var("neosys.id").osfile())) {
			return 0;
		}

		tx = "";
		anychanges = "";

		//document header
		////////////////

		head = 1;
		ln = 0;
		oln = 0;
		dictids = "DOCUMENT_NO,CLIENT_AND_BRAND,DATE_RANGE,MARKET_NAME,CAMPAIGN_DESCRIPTION";
		gosub changes();

		//new or changed document lines
		//////////////////////////////

		head = 0;
		var nlns = RECORD.a(20).count(VM) + 1;
		var nolns = win.orec.a(20).count(VM) + 1;
		var lnids = RECORD.a(160);
		var olnids = win.orec.a(160);
		dictids = "VEHICLE_NAME,DATELIST,SPEC,LOADING,DETAILS,FLAGS,NUMBER";
		var tt2 = "";
		if (authorised("MEDIA INVOICE ACCESS")) {
			dictids ^= ",UNIT_BILL,FREE_NUMBER";
			tt2 ^= ",UNIT_BILL";
		}
		if (authorised("MEDIA COST ACCESS")) {
			dictids ^= ",UNIT_COST,COST_FREE_NUMBER";
			tt2 ^= ",UNIT_COST";
		}
		dictids ^= tt2;

		//search for matching lines
		for (ln = 1; ln <= nlns; ++ln) {
			var lnid = lnids.a(1, ln);
			if (not(olnids.locate(lnid,oln))) {
				oln = 0;
			}
			gosub changes();
		};//ln;

		//deleted lines
		//////////////

		anydeleted = 0;
		//search for deleted lines
		for (oln = 1; oln <= nolns; ++oln) {
			var olnid = olnids.a(1, ln);
			if (not(lnids.locate(olnid,ln))) {
				ln = 0;
				gosub changes();
			}
		};//oln;

		if (anychanges) {

			//DATELIST can return html tags
			call htmllib("STRIPTAGS", tx);
			call htmllib("DECODEHTML", tx);

			var toaddress = "sb2@neosys.com";
			var ccaddress = "";
			var subject = "NEOSYS: Changes in Schedule " ^ ID;
			var body = tx;
			body.swapper(FM, "\r\n");

			call sendmail(toaddress, ccaddress, subject, body, attachfilename, deletex, errormsg, replyto, params);

			if (errormsg) {
				call sysmsg(errormsg ^ FM ^ FM ^ tx);
			}

		}

	//called from PLAN.SUBS PREWRITE
	} else if (mode == "CHK.RATES") {

		//check rates
		var currcode = RECORD.a(13);
		var vehiclecurrcodes = RECORD.a(45);
		var nlines = vehiclecurrcodes.count(VM) + (vehiclecurrcodes ne "");
		var rates = RECORD.a(40);
		var costrates = RECORD.a(44);
		for (ln = 1; ln <= nlines; ++ln) {
			var vehiclecurrcode = vehiclecurrcodes.a(1, ln);

			var rate = rates.a(1, ln);
			if (vehiclecurrcode == currcode) {
				if (rate ne 1) {
					//msg='Exchange rate must be 1|Please refresh exchange rates'
					call plansubs9(11, msg, ln ^ FM ^ vehiclecurrcode);
					return invalid(msg);
				}
			}else{
				if (rate == 1) {
					//msg='Exchange rate must not be 1|Please refresh exchange rates'
					call plansubs9(12, msg, ln ^ FM ^ vehiclecurrcode);
					return invalid(msg);
				}
			}

			var costrate = costrates.a(1, ln);
			if (vehiclecurrcode == agy.basecurrencycode) {
				if (costrate ne 1) {
					//msg='Cost exchange rate for ... must be 1|Please refresh exchange rates'
					call plansubs9(13, msg, ln ^ FM ^ vehiclecurrcode ^ FM ^ agy.basecurrencycode);
					return invalid(msg);
				}
			}else{
				if (costrate == 1) {
					//msg='Cost exchange rate for ... must not be 1|Please refresh exchange rates'
					call plansubs9(14, msg, ln ^ FM ^ vehiclecurrcode ^ FM ^ agy.basecurrencycode);
					return invalid(msg);
				}
			}

		};//ln;

	//called from PLAN.SUBS PREWRITE and POSTWRITE
	} else if (mode.field(".", 1) == "DELETEMOVED") {

		//determine if any lines being moved and quit if not
		var lnids = RECORD.a(160);
		if (not lnids) {
			return 0;
		}
		if (not(lnids.index(":"))) {
			return 0;
		}

		var mode2 = mode.field(".", 2);

		var amvfns = amvfns1;
		amvfns ^= amvfns2;
		var nfns = amvfns.count(VM) + 1;

		//clear the list of locked keys
		if (mode2 == "PREWRITE") {
			locklist = "";
		}

		//c++ declare
		var ntries = "";
		var otherrec = "";
		var otherlnid = "";
		var otherkey = "";

		var nlns = lnids.count(VM) + (lnids ne "");
		for (ln = 1; ln <= nlns; ++ln) {

			var lnid = lnids.a(1, ln);
			if (not(lnid.index(":"))) {
				goto nextmoveln;
			}

			otherkey = lnid.field(":", 1);
			otherlnid = lnid.field(":", 2, 9999);

			//make sure that we update all other records or none
			if (mode2 == "PREWRITE") {
				ntries = 1;
				if (not(locking("LOCK", win.datafile, otherkey, lockdesc, locklist, ntries, msg))) {
cantmove:
					call locking("UNLOCKALL", win.datafile, lockkey, lockdesc, locklist);
					return invalid(msg);
				}
			}

			//get the other record or fail
			if (not(otherrec.read(win.srcfile, otherkey))) {
				msg = otherkey.quote() ^ " is missing from " ^ win.datafile;
				goto cantmove;
			}

			//zzz should really check if the line has been invoiced
			//but that may mean a more complicated update process
			//for a schedule
			if (otherrec.a(4)) {
				msg = otherkey.quote() ^ " cannot be split because it has been invoiced";
				goto cantmove;
			}

			//locate the lnid pointer in the other record
			if (not(otherrec.a(160).locate(otherlnid,otherln))) {
				msg = "lineid " ^ otherlnid ^ " is missing in " ^ win.datafile ^ " " ^ (otherkey.quote());
				goto cantmove;
			}

			//prevent moving booked lines
			//could theoretically moved unbooked parts of a line
			if (otherrec.a(51, otherln)) {
				msg = otherkey.quote() ^ " line " ^ otherln ^ " cannot be split because it has been booked";
				goto cantmove;
			}

			if (mode2 == "POSTWRITE") {

				//save pointer as copied pointer
				//retain filename, key and lnid
				var temp = win.datafile[1] ^ "~" ^ lnid;
				temp.converter(":", "~");
				RECORD.r(160, ln, temp);

				var othervehiclecode = otherrec.a(20, otherln);
				var otherdategrid = otherrec.a(22, otherln);
				var origotherdategrid = otherdategrid;

				//zzz remove dates from dategrid
				//zzz remove daterange
				//assume moving whole line for now
				otherdategrid = "";

				if (otherdategrid) {
					otherrec.r(22, otherln, otherdategrid);

				//delete the whole other line
				//safe to delete since not deleting booked or invoiced lines
				}else{
					for (var fnn = 1; fnn <= nfns; ++fnn) {
						otherrec.remover(amvfns.a(1, fnn), otherln);
					};//fnn;
				}

				//delete all/some of the lines ads
				var adid = otherkey ^ "*" ^ othervehiclecode ^ "*" ^ otherlnid;
				//smdates
				var ncols = otherdategrid.count(SVM) + 1;
				for (var gridcoln = 1; gridcoln <= ncols; ++gridcoln) {
					if (origotherdategrid.a(1, 1, gridcoln)) {
						if (otherdategrid.a(1, 1, gridcoln) == "") {
							for (var daten = 1; daten <= 9999; ++daten) {
								adid = adid.fieldstore("*", 4, 2, gridcoln ^ "*" ^ daten);
								var ad;
								if (not(ad.read(agy.ads, adid))) {
									ad = "";
								}
								///BREAK;
								if (not ad) break;
								agy.ads.deleterecord(adid);

							};//daten;
						}
					}
				};//gridcoln;

				//zzz should buffer the writes so multiple line moves from the same
				//record are faster
				otherrec.write(win.srcfile, otherkey);

			}

nextmoveln:;
		};//ln;

		if (mode2 == "POSTWRITE") {

			//update new pointers without : marks
			RECORD.write(win.srcfile, ID);

			call locking("UNLOCKALL", win.datafile, lockkey, lockdesc, locklist);

		}

	//PLAN.SUBS5,DEF.REF called from GENERAL.SUBS
	//
	//call chain to get a plan/schedule number
	//UI sends company code in readenvironment
	//0. LISTEN calls PLAN.SUBS PREREAD which adds * to all single letters
	//       * tells LISTEN to get a sequential key (which is does in step 1.)
	//1. LISTEN       call  general.subs('DEF.SK.':readenvironment)
	//2. GENERAL.SUBS call plan.subs5('DEF.REF.':field(mode,'.',3,999))
	//3. PLAN.SUBS5   call agency.subs('GETNEXTID.':compcode)
	//4. AGENCY.SUBS  call GENERAL.SUBS('DEF.SK2')  pattern NOT <NUMBER>
	//                @ans=nextkey(params,previous) pattern IS  <NUMBER>
	} else if (mode.field(".", 1, 2) == "DEF.REF") {
		//from call general.subs('DEF.SK') /sk2?

		if (win.wlocked or RECORD) {
			return 0;
		}

		var previous = 0;

		//try to use single letter key as sequential key IF IS COMPANY CODE
		//why was the following commented out?
		if (((ID.length() == 2) and (ID[-1] == "*")) and ((ID[1]).xlate("COMPANIES", 1, "X"))) {
			compcode = ID[1];
		//can this really happen (see step 0)
		} else if (ID.length() == 1) {
			compcode = ID;
		}else{
			//read environment (company code expected TODO should be checked)
			compcode = mode.field(".", 3);
		}

		//end else
		// compcode=field(mode,'.',3)
		// end
		//gosub getnextref

		call agencysubs("GETNEXTID." ^ compcode);

		win.isdflt = ANS;
		ANS = "";
		//tstore=is.dflt

		if (not(interactive)) {
			ID = win.isdflt;
		}

	} else {
		msg = mode.quote() ^ " is invalid in PLAN.SUBS5";
		return invalid(msg);
	}

	return 0;
}

subroutine changes() {
	var tx2 = "";
	var lnchanges = 0;

	var ndictids = dictids.count(",") + (dictids ne "");
	for (var dictidn = 1; dictidn <= ndictids; ++dictidn) {

		MV = ln;
		var dictid = dictids.field(",", dictidn);

		if (head or ln) {
			newvalue = calculate(dictid);
		}else{
			newvalue = "";
		}

		if (head or oln) {
			oldvalue = calculate(dictid, DICT, ID, win.orec, oln);
		}else{
			oldvalue = "";
		}

		if ((not(ln) or (dictidn <= 2)) or newvalue ne oldvalue) {

			var dictrec;
			if (not(dictrec.read(DICT, dictid))) {
				dictrec = FM ^ FM ^ dictid;
			}

			//oconv
			var tt = dictrec.a(7);
			if (tt) {
				if (newvalue) {
					newvalue = oconv(newvalue, tt);
				}
				if (oldvalue) {
					oldvalue = oconv(newvalue, tt);
				}
			}

			if (head or newvalue ne oldvalue) {

				//no "was:' line for delete lines
				if (oln and not(ln)) {
					newvalue = oldvalue;
					oldvalue = "";
					if (not anydeleted) {
						anydeleted = 1;
						tx2 ^= FM ^ FM ^ "*DELETED LINES*" ^ FM;
					}
				}

				var title = dictrec.a(3);
				title.converter(VM ^ SVM, "  ");
				tx2 ^= FM ^ title.oconv("L#20") ^ " : " ^ newvalue;

				if (newvalue ne oldvalue) {
					lnchanges += 1;
					//dont print "was:" line for new or deleted lines
					if (ln and oln) {
						tx2 ^= FM ^ var("was").oconv("R#20") ^ " : " ^ oldvalue;
					}
				}

			}

		}

	};//dictidn;

	if (not(ln) or lnchanges) {
		anychanges += lnchanges;
		tx.r(-1, tx2);
	}

	return;

	//$insert gbp,window.subs2

}

libraryexit()
