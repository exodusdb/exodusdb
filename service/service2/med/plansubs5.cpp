#include <exodus/library.h>
libraryinit()

#include <createjob.h>
#include <authorised.h>
#include <htmllib.h>
#include <sendmail.h>
#include <sysmsg.h>
#include <plansubs9.h>
#include <locking.h>
#include <plansubs3.h>
#include <evaluatesubs.h>
#include <pop_up.h>
#include <agencysubs.h>
#include <generalsubs.h>
#include <accountingsubs.h>
#include <addcent.h>
#include <plansubs2.h>
#include <addacc.h>
#include <getreccount.h>
#include <singular.h>

#include <agy.h>
#include <gen.h>
#include <win.h>

#include <window.hpp>

var tt;
var attachfilename;
var deletex;
var errormsg;
var replyto;
var params;
var newvalue;
var oldvalue;
var msg;
var lockdesc;
var lockkey;
var otherln;
var rating;
var ratingn;//num
var xx;
var compcode;
var reply;//num
var defyear;//num
var stopdate;//num
var startdate;//num
var m1;//num
var m2;//num
var t2;
var lgrcd;
var accno;
var wsmsg;

function main(in mode) {
	//c med
	//garbagecollect;

	//y2k2 *jbase

	if (mode == "CREATEJOB") {

		//skip if automatic creation of media jobs is not configured
		var jobjobtypecode = agy.agp.a(9999);
		//TEST only ATM
		if (not jobjobtypecode and (SYSTEM.a(17).substr(-4,4) ne "TEST")) {
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
		tt = "Media";
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
		if (not(var("NEOSYS.ID").osfile())) {
			return 0;
		}

		var tx = "";
		var anychanges = "";

		//document header
		////////////////

		var head = 1;
		var ln = 0;
		var oln = 0;
		var dictids = "DOCUMENT_NO,CLIENT_AND_BRAND,DATE_RANGE,MARKET_NAME,CAMPAIGN_DESCRIPTION";
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
		for (var ln = 1; ln <= nlns; ++ln) {
			var lnid = lnids.a(1, ln);
			if (not(olnids.locateusing(lnid, VM, oln))) {
				oln = 0;
			}
			gosub changes();
		};//ln;

		//deleted lines
		//////////////

		var anydeleted = 0;
		//search for deleted lines
		for (var oln = 1; oln <= nolns; ++oln) {
			var olnid = olnids.a(1, ln);
			if (not(lnids.locateusing(olnid, VM, ln))) {
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

			if ((not ln or (dictidn <= 2)) or newvalue ne oldvalue) {

				var dictrec;
				if (not(dictrec.read(DICT, dictid))) {
					dictrec = FM ^ FM ^ dictid;
				}

				//oconv
				tt = dictrec.a(7);
				if (tt) {
					if (newvalue) {
						newvalue = newvalue.oconv(tt);
					}
					if (oldvalue) {
						oldvalue = newvalue.oconv(tt);
					}
				}

				if (head or newvalue ne oldvalue) {

					//no "was:' line for delete lines
					if (oln and not ln) {
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

		if (not ln or lnchanges) {
			anychanges += lnchanges;
			tx.r(-1, tx2);
		}

		return;

	} else if (mode == "CHK.RATES") {

		//check rates
		var currcode = RECORD.a(13);
		var vehiclecurrcodes = RECORD.a(45);
		var nlines = vehiclecurrcodes.count(VM) + (vehiclecurrcodes ne "");
		var rates = RECORD.a(40);
		var costrates = RECORD.a(44);
		for (var ln = 1; ln <= nlines; ++ln) {
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
			if (vehiclecurrcode == agy.agp.a(2)) {
				if (costrate ne 1) {
					//msg='Cost exchange rate for ... must be 1|Please refresh exchange rates'
					call plansubs9(13, msg, ln ^ FM ^ vehiclecurrcode ^ FM ^ agy.agp.a(2));
					return invalid(msg);
				}
			}else{
				if (costrate == 1) {
					//msg='Cost exchange rate for ... must not be 1|Please refresh exchange rates'
					call plansubs9(14, msg, ln ^ FM ^ vehiclecurrcode ^ FM ^ agy.agp.a(2));
					return invalid(msg);
				}
			}

		};//ln;

	} else if (mode.field(".", 1) == "DELETEMOVED") {

		//determine if any lines being moved and quit if not
		var lnids = RECORD.a(160);
		if (not lnids) {
			return;
		}
		if (not(lnids.index(":"))) {
			return;
		}

		var mode2 = mode.field(".", 2);

		var amvfns = "19" _VM_ "20" _VM_ "21" _VM_ "22" _VM_ "23" _VM_ "24" _VM_ "27" _VM_ "37" _VM_ "38" _VM_ "39" _VM_ "40" _VM_ "41" _VM_ "42" _VM_ "43" _VM_ "44" _VM_ "45" _VM_ "46" _VM_ "47" _VM_ "48" _VM_ "49" _VM_ "50" _VM_ "51" _VM_ "52" _VM_ "54" _VM_ "55" _VM_ "56" _VM_ "62" _VM_ "63" _VM_ "65" _VM_ "67" _VM_ "70" _VM_ "71" _VM_ "72" _VM_ "73" _VM_ "75" _VM_ "76" "" _VM_ "111" _VM_ "114" _VM_ "106" _VM_ "116" _VM_ "126" _VM_ "138" _VM_ "139" _VM_ "140" _VM_ "141" _VM_ "142" _VM_ "143" _VM_ "144" _VM_ "145" _VM_ "146" _VM_ "147" _VM_ "148" _VM_ "149" _VM_ "150" _VM_ "151" _VM_ "152" _VM_ "156" _VM_ "159" _VM_ "160" _VM_ "161" _VM_ "162" _VM_ "164" _VM_ "165" _VM_ "166" _VM_ "167" _VM_ "171" _VM_ "172" _VM_ "173" _VM_ "174" _VM_ "175" _VM_ "176" _VM_ "177" _VM_ "178" _VM_ "179" _VM_ "180" _VM_ "181" _VM_ "182" _VM_ "183" _VM_ "190" _VM_ "191" _VM_ "221" _VM_ "68";
		var nfns = amvfns.count(VM) + 1;

		//clear the list of locked keys
		if (mode2 == "PREWRITE") {
			win.registerx(7) = "";
		}

		var nlns = lnids.count(VM) + (lnids ne "");
		for (var ln = 1; ln <= nlns; ++ln) {

			var lnid = lnids.a(1, ln);
			if (not(lnid.index(":"))) {
				goto nextmoveln;
			}

			var otherkey = lnid.field(":", 1);
			var otherlnid = lnid.field(":", 2, 9999);

			//make sure that we update all other records or none
			if (mode2 == "PREWRITE") {
				var ntries = 1;
				if (not(locking("LOCK", win.datafile, otherkey, lockdesc, win.registerx(7), ntries, msg))) {
cantmove:
					call locking("UNLOCKALL", win.datafile, lockkey, lockdesc, win.registerx(7), "", "");
					return invalid(msg);
				}
			}

			//get the other record or fail
			var otherrec;
			if (not(otherrec.read(win.srcfile, otherkey))) {
				msg = DQ ^ (otherkey ^ DQ) ^ " is missing from " ^ win.datafile;
				goto cantmove;
			}

			//zzz should really check if the line has been invoiced
			//but that may mean a more complicated update process
			//for a schedule
			if (otherrec.a(4)) {
				msg = DQ ^ (otherkey ^ DQ) ^ " cannot be split because it has been invoiced";
				goto cantmove;
			}

			//locate the lnid pointer in the other record
			if (not(otherrec.a(160).locateusing(otherlnid, VM, otherln))) {
				msg = "lineid " ^ otherlnid ^ " is missing in " ^ win.datafile ^ " " ^ (DQ ^ (otherkey ^ DQ));
				goto cantmove;
			}

			//prevent moving booked lines
			//could theoretically moved unbooked parts of a line
			if (otherrec.a(51, otherln)) {
				msg = DQ ^ (otherkey ^ DQ) ^ " line " ^ otherln ^ " cannot be split because it has been booked";
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
						otherrec.eraser(amvfns.a(1, fnn), otherln);
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
							if (not ad) break;;
								agy.ads.deleterecord(adid);
								
							};//daten;
						}
					}
				};//gridcoln;

				//zzz should buffer the writes so multiple line moves from the same
				//record are faster
				otherrec.write(win.srcfile, otherkey);

			}

nextmoveln:
		};//ln;

		if (mode2 == "POSTWRITE") {

			//update new pointers without : marks
			RECORD.write(win.srcfile, ID);

			call locking("UNLOCKALL", win.datafile, lockkey, lockdesc, win.registerx(7), "", "");

		}

	} else if (mode == "VAL.TIME") {
		if (win.is == win.isorig) {
			return;
		}
		call plansubs3("CHECKCOINCIDENCE");
		RECORD.r(win.si.a(4), win.mvx, win.is);
		call evaluatesubs("EVALUATE LINE");
		DATA = "";

	} else if (mode == "F2.TIME") {
		if (not(agy.ratings.open("RATINGS", ""))) {
			agy.ratings = "";
		}
		var progcode = RECORD.a(110) ^ ".rev";
		var axis;
		if (not(axis.read(agy.ratings, progcode))) {
			if (not(axis.osread(progcode))) {
				if (not(axis.osread("x3"))) {
					msg = "PROGRAM LIST IS NOT AVAILABLE";
					return invalid(msg);
				}
			}
		}

		var lastdow = "";

		var vehiclecode = RECORD.a(20, win.mvx);
		var nn = axis.count(FM) + 1;
		var options = "";
		for (var ii = 1; ii <= nn; ++ii) {
			var line = axis.a(ii);
			var nsubs = line.count(VM) + (line ne "");
			for (var subn = 1; subn <= nsubs; ++subn) {
				var subline = line.a(1, subn);
				if (vehiclecode == subline.substr(1,9).trim()) {
					//idate=iconv(subline<1,1,2>,'DE')
					var idate = subline.a(1, 1, 2);
					var dayofweekn = (idate - 1) % 7 + 1;
					var dayofweek = var("Mon,Tue,Wed,Thu,Fri,Sat,Sun").field(",", dayofweekn);
					subline.r(1, 1, 2, dayofweek);
					subline.converter(SVM ^ VM, VM ^ FM);
					if (agy.ratings) {
						if (dayofweekn ne lastdow) {
							var ratingkey = calculate("SURVEY_CODE1") ^ "*" ^ calculate("TARGET1") ^ "*" ^ vehiclecode ^ "*" ^ dayofweekn;
							var rating;
							if (not(rating.read(agy.ratings, ratingkey))) {
								rating = "";
								agy.ratings = "";
							}
							lastdow = dayofweekn;
						}
						if (rating) {
							var itime = subline.substr(10,5).iconv("[TIME2]");
							if (not(rating.a(111).locateby(itime, "AR", ratingn))) {
								ratingn -= 1;
							}
							var ratingperc = rating.a(106, ratingn);
							subline.r(1, 4, ratingperc);
						}
					}
					options ^= FM ^ subline.substr(10,9999);
				}
			};//subn;
		};//ii;
		options.splicer(1, 1, "");

		params = "1:10:L:MTH:Time\\2:10:L::Day\\3:30:L::Program";
		var question = "Which time/program do you want ?";
		if (agy.ratings) {
			question ^= FM ^ "|(Ratings are for " ^ (DQ ^ (calculate("TARGET1") ^ DQ)) ^ " / " ^ (DQ ^ (calculate("SURVEY_CODE1") ^ DQ)) ^ ")";
			params ^= "\\4:7:R::Rating%";
		}else{
			if (calculate("TARGET1")) {
				question ^= "|(Ratings are not available because survey analysis";
				question ^= "|has not been done for \"" ^ calculate("TARGET1") ^ "\" / \"" ^ calculate("SURVEY_CODE1") ^ "\")";
			}
		}
		var optionn = pop_up(0, 0, "", options, params, "R", "", question, "", "", "", "P");
		if (not optionn) {
			ANS = win.is;
			return;
		}

		var starttime = options.a(optionn, 1).iconv("[TIME2]");
		var programx = options.a(optionn, 3);
		RECORD.r(23, win.mvx, RECORD.a(23, win.mvx).fieldstore("/", 1, 1, starttime.oconv("[TIME2]")));
		RECORD.r(23, win.mvx, RECORD.a(23, win.mvx).fieldstore("/", 3, 1, programx));
		tt = win.registerx(1).a(23);
		gosub r();
		ANS = ((starttime / (15 * 60)).floor() * 15 * 60).oconv("[TIME2]");

	} else if (mode == "DEF.CURRENCY") {
		win.isdflt = calculate("MARKET_CODE").xlate("MARKETS", 5, "X");

		//convert to invoice currency of the client
		var client;
		if (not(client.read(agy.clients, calculate("CLIENT_CODE")))) {
			return;
		}

		var invcurr = client.a(11);
		if (invcurr) {
			if (win.isdflt and client.a(13)) {
				if (client.a(13).locateusing(win.isdflt, VM, tt)) {
					win.isdflt = invcurr;
				}
			}else{
				win.isdflt = invcurr;
			}
		}
		if (win.isdflt == "") {
			win.isdflt = agy.agp.a(2);
		}
		win.registerx(10) = win.isdflt;

	} else if ((mode == "F2.PLANS") or (mode == "F2.SCHEDULES")) {
		call agencysubs(mode);

	/*;
		case mode='DEF.REF';
			call catalyst('S','GENERAL.SUBS,DEF.SK');
			tstore=is.dflt;
	*/

	} else if (mode == "VAL.REF") {
		if ((win.is == "") or (win.is == win.isorig)) {
			return;
		}

		//check no bad characters on new records
		var xx;
		if (not(xx.read(win.srcfile, win.is))) {

			//in case sombody else just used the next key
lockit:
			if (win.is == win.registerx(10)) {
				if (lockrecord(win.datafile, win.srcfile, win.is)) {
					xx = unlockrecord(win.datafile, win.srcfile, win.is);
				}else{
					if (win.is.isnum()) {
						win.is += 1;
						goto lockit;
					}
				}
			}

			win.is.trimmerf();
			if (win.is.index(" ")) {
				msg = "SORRY, YOU CANNOT USE SPACES HERE";
				gosub invalid(msg);
			}else{
				gosub badchars();
			}
		}

	//call chain to get a plan/schedule number
	//UI sends company code in readenvironment
	//0. LISTEN calls PLAN.SUBS PREREAD which adds * to all single letters
	// * tells LISTEN to get a sequential key (which is does in step 1.)
	//1. LISTEN call general.subs('DEF.SK.':readenvironment)
	//2. GENERAL.SUBS call plan.subs5('DEF.REF.':field(mode,'.',3,999))
	//3. PLAN.SUBS5 call agency.subs('GETNEXTID.':compcode)
	//4. AGENCY.SUBS call GENERAL.SUBS('DEF.SK2') pattern NOT <NUMBER>
	// @ans=nextkey(params,previous) pattern IS <NUMBER>
	} else if (mode.field(".", 1, 2) == "DEF.REF") {
		//from call general.subs('DEF.SK') /sk2?

		if (win.wlocked or RECORD) {
			return;
		}

		var previous = 0;

		//try to use single letter key as sequential key IF IS COMPANY CODE
		//why was the following commented out?
		if (((ID.length() == 2) and (ID[-1] == "*")) and ((ID[1]).xlate("COMPANIES", 1, "X"))) {
			compcode = ID[1];
			//can this really happen (see step 0)
			goto 3776;
		}
		if (ID.length() == 1) {
			compcode = ID;
		}else{
			//read environment (company code expected TODO should be checked)
			compcode = mode.field(".", 3);
		}
//L3776:

		//end else
		// compcode=field(mode,'.',3)
		// end
		//gosub getnextref

		call agencysubs("GETNEXTID." ^ compcode);

		win.isdflt = ANS;
		ANS = "";
		win.registerx(10) = win.isdflt;

		if (not(not SYSTEM.a(33))) {
			ID = win.isdflt;
		}

		return;
	/*;
	///////////
	getnextref:
	///////////
		if interactive else;
			if compcode and compcode<>gcurr.company then;
				call init.company(compcode);
				end;
			call readagp;
			end;
		//skip if no schedule number structure
		//keyformat=agp<63>
		if datafile='SCHEDULES' or agp<71>='' then keyformat=agp<63> else keyformat=agp<71>;

		if keyformat='' and @id matches '1A"*"' then;
			keyformat=@id[1,1]:'%<>';
			end;

		if index(keyformat,'<',1) else;
			//SK2 (not SK) to prevent endless loop when GENERAL.SUBS calls this routine
			call GENERAL.SUBS('DEF.SK2');
			@ans=is.dflt;
			return;
			end;

		//determine company prefix
		if compcode='' then compcode=gcurr.company;
		reado companyx from companies,compcode else;
			msg=quote(compcode):' IS NOT IN COMPANY FILE';
			gosub note;
			companyx='';
			end;
		if companyx<28>='' then companyx<28>=compcode;

		swap '<COMPANY>' with companyx<28> in keyformat;
		swap '<NUMBER>' with '%' in keyformat;
		swap '<>' with '' in keyformat;

		keyfilename=datafile;

		seqkeyfilename='DEFINITIONS';
		seqkeyformat='';
		if seqkeyformat='' then;
			seqkeyformat=keyformat;
			convert '%' to '' in seqkeyformat;
			end;
		if seqkeyformat<>'' then seqkeyformat[1,0]='.';
		seqkeyformat=keyfilename:'.SK':seqkeyformat;

		//get the next key
		params=keyfilename:':':seqkeyformat:':':seqkeyfilename:':':keyformat;
		@ans=nextkey(params,previous);

		return;
	*/

	/* replaced with call printplans8('CREATEACC');
		case mode='CREATEACC';

			//identical code is in printplans6 and plan.subs5 controlled by preferwip
			////////////////////////////////////////////////////////////////////////

			call printplans8('ADDACC.WIP');
	*/

	} else if (mode == "F2.PERIOD") {
		gosub checkchange();
		if (not(win.valid)) {
			return;
		}

		var period = var().date().oconv("D2/E").field("/", 2) + 0;
		var year = var().date().oconv("D2/E").field("/", 3);
		var options = "";
		for (var ii = 1; ii <= 12; ++ii) {
			options.r(-1, period ^ "/" ^ year);
			period += 1;
			if (period > 12) {
				period = 1;
				//year+=1
				year = (year + 1).oconv("R(0)#2");
			}
		};//ii;
		if (not(decide("", options, reply))) {
			return;
		}
		ANS = options.a(reply);
		DATA ^= "" "\r";

	} else if (mode == "DEF.PERIOD") {
		if (win.is) {
			return;
		}
		win.isdflt = var().date().oconv("D2/E").field("/", 2, 2);
		if (win.isdflt[1] == "0") {
			win.isdflt.splicer(1, 1, "");
		}

	} else if (mode.substr(1,10) == "VAL.PERIOD") {

		if ((win.is == "") or (win.is == win.isorig)) {
			return;
		}
		gosub checkchange();
		if (not(win.valid)) {
			return;
		}

		//default to 2 decimal places
		//zzz should this not be somewhere else ?
		if (RECORD.a(1) == "") {
			RECORD.r(1, 2);
			win.orec.r(1, 2);
		}

		//default year to the year of the period
		if (win.is.match("0N")) {
			if (mode == "VAL.PERIOD2") {
				defyear = calculate("PERIOD").field("/", 2);
				if (win.is < calculate("PERIOD").field("/", 1)) {
					defyear += 1;
				}
			}else{
				defyear = var().date().oconv("D2/").substr(7,2);
			}
			win.is ^= "/" ^ defyear;
		}

		//trim leading zeroes
		if (win.is[1] == "0") {
			win.is.splicer(1, 1, "");
		}

		//check month/year format
		var month = win.is.field("/", 1);
		if (not(var("1,2,3,4,5,6,7,8,9,10,11,12").locateusing(month, ",", tt))) {
badperiod:
			msg = "PLEASE ENTER PERIOD AS \"MONTH/YEAR\"|(EG \"1/92\" or \"1/1992\")";
			return invalid(msg);
		}
		var year = win.is.field("/", 2);
		if (year.match("4N")) {
			year = year.substr(-2,2);
			win.is = month ^ "/" ^ year;
		}
		if (not(year.match("2N"))) {
			goto badperiod;
		}

		gosub protectmonths();
		if (not(win.valid)) {
			return;
		}

		//T=WIS<22>
		//d ebug;gosub r

		if (win.datafile == "SCHEDULES") {
			var period = win.is;
			gosub getstartstop2();

			//check that there are not too many dates
			var ndays = stopdate - startdate + 1;
			var nlines = RECORD.a(22).count(VM) + 1;
			var first = 1;
			for (var lnx = 1; lnx <= nlines; ++lnx) {
				var dates = RECORD.a(22, lnx);
				//if date>0 then
				// msg='PLEASE REMOVE THE DATES AFTER ':stopdate '[DATE,4*]':' FIRST'
				// goto invalid
				// end
checkndates:
				//smdates
				goto smdatesunconverted;

				if (dates.length() > ndays) {

					//decide how to handle excess dates
					if (first) {
						first = 0;
						var options = "Remove them|Extend the schedule into the next month|Cancel";
						if (not(decide("Some ads will now fall on or after " ^ (stopdate + 1).oconv("[DATE,4*]") ^ "|What do you want to do ?", options, reply))) {
							reply = 3;
						}
						if (reply == 3) {
							win.valid = 0;
							return;
						}
					}

					//extend into next month by one day at a time
					if (reply == 2) {
						stopdate += 1;
						ndays += 1;
						goto checkndates;
					}

					//count number of ads
					//smdates unconverted
					tt = dates.substr(1,ndays).trimb();
					tt.converter(UPPERCASE, var("1").str(tt.length()));
					var nads = "";
					for (var ii = 1; ii <= 9; ++ii) {
						nads += tt.count(ii);
					};//ii;

					//put the trimmed dates back
					RECORD.r(22, lnx, dates.substr(1,ndays).trimb());

					//change the number of ads
					RECORD.r(39, lnx, nads);
					RECORD.r(43, lnx, nads);

				}
			};//lnx;

			//update the start and stop date for the schedule
			gosub setstartstop();

			win.displayaction = 5;
			win.reset = 3;

		}

		if (win.isorig) {
			gosub getexchrates();
		}

		win.displayaction = 5;

	} else if (mode == "PROTECTMONTHS") {
		gosub protectmonths();

	} else if (mode == "DEF.STARTDATE") {

	} else if (mode == "VAL.STARTDATE") {
		if (not(win.is.isnum())) {
			win.is = win.is.iconv("[DATE]");
		}
		if (win.is == win.isorig) {
			return;
		}
		gosub checkchange();
		if (not(win.valid)) {
			return;
		}

		//check not after stop date
		if (win.is > RECORD.a(25)) {
			msg = "\"START DATE\" CANNOT BE AFTER THE \"STOP DATE\"";
			return invalid(msg);
		}

		//check max 35 days
		var maxdate = win.is + 35 - 1;
		if (RECORD.a(25) > maxdate) {
			var qq = "Maximum of 35 days on one schedule";
			qq.r(-1, "Change \"Stop date\" to " ^ maxdate.oconv("[DATE,*]"));
			if (not(decide(qq, "", reply))) {
				reply = 2;
			}
			if (reply == 2) {
				win.valid = 0;
				return;
			}
			RECORD.r(25, maxdate);
			tt = win.registerx(1).a(25);
			gosub r();
		}

		//redisplay the dates
		tt = win.registerx(1).a(22);
		gosub r();

	} else if (mode == "DEF.STOPDATE") {

	} else if (mode == "VAL.STOPDATE") {
		if (not(win.is.isnum())) {
			win.is = win.is.iconv("[DATE]");
		}
		if (win.is == win.isorig) {
			return;
		}
		gosub checkchange();
		if (not(win.valid)) {
			return;
		}

		//check not before start date
		if (win.is < RECORD.a(10)) {
			msg = "\"STOP DATE\" CANNOT BE BEFORE THE \"START DATE\"";
			return invalid(msg);
		}

		//check max 35 days (5 weeks)
		var mindate = win.is - 34;
		if (RECORD.a(10) < mindate) {
			var qq = "Maximum of 35 days on one schedule";
			qq.r(-1, "Change \"Start date\" to " ^ mindate.oconv("[DATE,*]"));
			if (not(decide(qq, "", reply))) {
				reply = 2;
			}
			if (reply == 2) {
				win.isorig = "";
				win.valid = 0;
				return;
			}
			RECORD.r(10, mindate);
			tt = win.registerx(1).a(10);
			gosub r();
		}

		//redisplay the dates
		tt = win.registerx(1).a(22);
		gosub r();

	} else if (mode == "DEF.COMPANY") {
		var temp = "";
		if (temp == "") {
			temp = calculate("CLIENT_ACCNO").a(1, 1, 1).field(",", 2);
		}
		if (temp) {
			win.isdflt = temp;
		}else{
			if (not(agy.agp.a(1).ucase().index("PANGULF"))) {
				call generalsubs("DEF.COMPANY");
			}
		}

	} else if (mode == "VAL.COMPANY") {

		if (win.is == win.isorig) {
			return;
		}

		//prevent change of company after invoicing
		if (RECORD.a(4)) {

			var oldcompanycode = win.orec.a(187);
			var newcompanycode = win.is;

			//allow conversion of old data to company code "1" promopub
			//if (oldcompanycode='' and newcompanycode=1) then return

			if (oldcompanycode and newcompanycode ne oldcompanycode) {
				msg = "SORRY, YOU CANNOT CHANGE THE|COMPANY CODE AFTER INVOICING";
				return invalid(msg);
			}

		}

		call generalsubs("VAL.COMPANY");

	} else if (mode == "VAL.ACCNO") {

		if (win.is == win.isorig) {
			return;
		}

		//company code is no longer allowed in the account number
		if (win.is.index(",")) {
			msg = "Please enter the company code in the company code field";
			return invalid(msg);
		}

		call accountingsubs("VAL.ACCNO");

	}
//L5534:
	return;

}

subroutine protectmonths() {
	if (not(win.datafile == "PLANS")) {
		return;
	}

	//work out which months/columns are to be used
	if (win.wi == win.registerx(1).a(12)) {
		m1 = win.is;
	}else{
		m1 = RECORD.a(12);
	}
	if (win.wi == win.registerx(1).a(15)) {
		m2 = win.is;
	}else{
		m2 = RECORD.a(15);
	}
	if (not m2) {
		m2 = m1;
	}
	var y1 = addcent(m1.field("/", 2));
	var y2 = addcent(m2.field("/", 2));
	m1 = m1.field("/", 1);
	m2 = m2.field("/", 1);

	//check the number of months is 1-12
	if (mode ne "PROTECTMONTHS") {
		var nn = m2 - m1 + 12 * (y2 - y1) + 1;
		if (nn < 1) {
			msg = "THE STARTING MONTH CANNOT BE|AFTER THE ENDING MONTH";
			return invalid(msg);
		}
		if (nn > 12) {
			msg = "THE ENDING MONTH CANNOT BE MORE THAN|12 MONTHS AFTER THE STARTING MONTH";
			return invalid(msg);
		}
	}

	if (m2 < m1) {
		m2 += 12;
	}
	//month fns are different for date plans
//WARNING TODO: check trigraph following;
	var basefn = (win.registerx(1).a(140 + 1)) ? var(140) : var(170);

	//check that there is no data in the unused columns
	if (mode ne "PROTECTMONTHS") {
		for (var mm = m1; mm <= m1 + 11; ++mm) {
			var modmth = (mm - 1) % 12 + 1;
			var unused = mm > m2;
			if (unused) {
				tt = RECORD.a(140 + modmth) ^ RECORD.a(170 + modmth);
				tt.converter(VM, "");
				if (tt) {
					msg = "There are some ads in month " ^ (mm - 1) % 12 + 1 ^ "|Please clear them first";
					return invalid(msg);
				}
			}
		};//mm;
	}

	//hide/unhide the unused/used columns
	for (var mm = m1; mm <= m1 + 11; ++mm) {
		var modmth = (mm - 1) % 12 + 1;
		var unused = mm > m2;
		var fn = win.registerx(1).a(basefn + modmth);
		if (fn) {
//WARNING TODO: check trigraph following;
			win.ww(fn) = win.ww(fn) ? var(18) : (var(0).replace(0, if (unused) {, "P", "O"));
		}
	};//mm;

	return;

	/////////////
getstartstop:
	/////////////
	var period = RECORD.a(12);
}

subroutine getstartstop2() {
	var mth = period.field("/", 1);
	var year = period.field("/", 2);

	//schedules start and end on calendar months for the time being
	//if mth=1 then
	// startdate=iconv('1/':mth:'/':year,'D/E')
	//end else
	// startdate=iconv(mth-1:'/':year,agp<18>)+1
	// end
	//if mth=12 then
	// stopdate=iconv('31/12/':year,'D/E')
	//end else
	// stopdate=iconv(mth:'/':year,agp<18>)
	// end
	startdate = ("1/" ^ mth ^ "/" ^ year).iconv("D/E");
	stopdate = (mth ^ "/" ^ year).iconv("[DATEPERIOD,1,12]");
	return;

}

subroutine setstartstop() {
	RECORD.r(10, startdate);
	tt = win.registerx(1).a(10);
	gosub r();
	RECORD.r(25, stopdate);
	tt = win.registerx(1).a(25);
	gosub r();
	return;

}

subroutine checkchange() {
	if (not(authorised("SCHEDULE UPDATE PLAN", msg, ""))) {
		goto badchars;
	}

	if (RECORD.a(4)) {
		msg = "YOU CANNOT CHANGE THIS BECAUSE THE|SCHEDULE HAS ALREADY BEEN INVOICED";
		return invalid(msg);
	}

checkchangebooking:
	tt = RECORD.a(51);
	tt.converter(VM, "");
	if (tt) {
		msg = "YOU CANNOT CHANGE THIS BECAUSE BOOKING|ORDERS HAVE ALREADY BEEN ISSUED";
		return invalid(msg);
	}
	return;

}

subroutine getexchrates() {
	call plansubs2("GETEXCHRATES");
	return;

}

subroutine r() {
	//redisplay
	if (not tt) {
		return;
	}
	//call note(tt:' ':w.cnt:' ':w(tt)<5>)
	if (not(win.redisplaylist.locateusing(tt, FM, t2))) {
		win.redisplaylist.r(-1, tt);
	}
	win.displayaction = 6;
	win.reset = 3;
	return;

	///////
addacc:
	///////
	if (not lgrcd) {
		return;
	}
	var sortorder = "NAME";
	period = calculate("PERIOD");
	//accname='Sch ':@id:' ':trim({CLIENT_CODE}:' ':{BRAND_CODE}:' ':{PERIOD}:' ':{MARKET_CODE})
	var accname = (calculate("CLIENT_CODE") ^ " " ^ calculate("BRAND_CODE") ^ " " ^ calculate("PERIOD")).trim();

	//virtually identical in addjobacc plan.subs5 printplans6
	//year 99
	params = period.field("/", 2);
	//calendar period 01-12
	params.r(2, period.field("/", 1).oconv("R(0)#2"));
	//calendar quarter 1-4
	params.r(3, ((period.field("/", 1) - 1) / 3).floor() + 1);
	//company code
	params.r(4, gen.gcurrcompany);

	var addaccmode = "ADD";
	if (FILES(0).locateusing("ABP", FM, xx)) {
		if (FILES(0).locateusing("ACCOUNTS", FM, xx)) {
			call addacc(addaccmode, lgrcd, accno, accname, sortorder, params, msg);
			//if msg then
			//changed to prevent messages occuring while batch updating schedules
			if (msg and not SYSTEM.a(33)) {
				var tempfile;
				if (tempfile.open("BALANCES", "")) {
					if (getreccount(tempfile, "", "")) {
						call note(msg);
						win.valid = 0;
					}
				}
			}
		}
	}
	return;

	///////////////////
smdatesunconverted:
	///////////////////
	msg = "NOT SUPPORTED IN DOS VERSION ANYMORE";
	return invalid(msg);
	return;

}


libraryexit()
