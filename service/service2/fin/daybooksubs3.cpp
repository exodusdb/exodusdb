#include <exodus/library.h>
libraryinit()

#include <nextkey.h>
#include <authorised.h>
#include <getbatchcodes.h>
#include <pop_up.h>
#include <addcent.h>
#include <singular.h>

#include <fin.h>
#include <gen.h>
#include <win.h>

#include <window.hpp>

var raisebatchtype;
var vn;
var reply;//num
var suffix;
var temp;
var status;
var ln1;//num
var addcurr;
var addamt;//num
var addbase;//num
var baseamtfn;
var add;//num
var currn;
var wsmsg;

function main(in mode) {
	//c fin

	//seems to be used only for VAL.PERIOD and RAISEUNPOSTEDMINIMUM
	//garbagecollect;
	//y2k2
	// daybook batch header validation and interface routines

////////
declare:
////////
	var interactive = not SYSTEM.a(33);

	//equ printtypes to register(10)<3>
	//equ tstore2 to register(10)<4>

	//removed so we can call raise.unposted.minimum without setting @id
	//t=field(@id,'*',1)
	//IF t NE CURR.COMPANY THEN
	// IF t THEN
	// *CURR.COMPANY=t
	// CALL INIT.COMPANY(t)
	// if interactive then
	// MSG='YOU HAVE JUST CHANGED BACK TO ':QUOTE(CURR.COMPANY):' COMPANY'
	// GOSUB NOTE3
	// end
	// END
	// END

	win.redisplaylist = "";
	var tt = "";
	//do not set valid because valid used in prewrite to indicate failed
	//valid=1

	////////////
	//select mode
	////////////

	if (mode.field(".", 1) == "ADDVERSION") {
		goto addversion;
	}

	if (mode.field("*", 1) == "RAISE.UNPOSTED.MINIMUM") {
		raisebatchtype = mode.field("*", 2);
		if (not raisebatchtype) {
			raisebatchtype = win.registerx(10).a(1);
		}
		goto raiseunpostedminimum;
	}

	//none used from web
	if ("DEF.BATCH.REF" _VM_ "F2.BATCH.REF" _VM_ "VAL.BATCH.REF".a(1).locateusing(mode, VM, vn)) {
		win.valid = 1;
		switch (vn.toInt()) {
		}

		var modea = mode.substr(1,4);
		var modeb = mode.substr(5,99);

		//preprompt routines
		if (modea == "DEF.") {
			if (not(win.wlocked)) {
				return 0;
			}
			win.valid = 1;
			if (var("PERIOD").a(1).locateusing(modeb, VM, vn)) {
				switch (vn.toInt()) {
				}
			}

			//postprompt routines
			if (modea == "VAL.") {
				if (win.amvaction == 2) {
					return 0;
				}
				if (var("PERIOD").a(1).locateusing(modeb, VM, vn)) {
					switch (vn.toInt()) {
					}
				}

				//F2 options routines
				//LOCATE MODE IN 'F2.PERIOD' SETTING VN THEN
				// IF WLOCKED ELSE
				// CALL MSG('W810')
				// RETURN
				// END
				// valid=1
				// ON VN GOTO F2.PERIOD
				// END

				//unknown mode
				var msg = "SYSTEM ERROR IN DAYBOOK.SUBS3|INVALID MODE " ^ (DQ ^ (mode ^ DQ)) ^ " IGNORED";
				goto note3;

	//!********* SUBROUTINES ***********

//////////////
				case defbatchref:
//////////////
				if (not(win.wlocked or RECORD)) {
					if (not(ID.field("*", 1))) {
						ID = ID.fieldstore("*", 1, 1, fin.currcompany);
					}
					if (not(ID.field("*", 2))) {
						ID = ID.fieldstore("*", 2, 1, win.registerx(10).a(1));
					}

					tt = ":%" ^ ID.field("*", 1, 2) ^ "%:BATCHES:" ^ ID.field("*", 1, 2) ^ "*";
					//eg t=":%H*INV%:BATCHES:H*INV*"

					tt = nextkey(tt, "");

					ID = tt;
					win.isdflt = tt.field("*", 3);
					ANS = "";
				}
				win.registerx(4) = win.isdflt;
				return 0;

/////////////
				case f2batchref:
/////////////
				gosub keychange();
				if (not(win.valid)) {
					return 0;
				}

				if (authorised("JOURNAL ACCESS POSTED", msg, "")) {
					if (not(decide("", "Posted batches" _VM_ "Unposted batches", reply))) {
						return 0;
					}
				}else{
					reply = 2;
				}
				if (reply == 1) {
					suffix = "";
				}else{
					suffix = "*U";
				}

				var refs = getbatchcodes(fin.currcompany, win.registerx(10).a(1), suffix, fin.batches);
				if (not refs) {
					msg = "No batches found";
					goto invalid2;
				}

				//select a batch
				if (suffix) {
					temp = "Entered by";
				}else{
					temp = "Posted By";
				}
				var params = "BATCH_REFERENCE:5:R::Batch\\FIRST_VOUCHER:10:R::1st Vouch.\\CONTRA_ACCOUNT_NAME:30:::First account\\WHO:10:::" ^ temp ^ "\\23:7:R:[ADDCENT]:Period\\ACTION:1:::";
				tt = "Which batch(es) do you want ?";
				if (suffix) {
					tt ^= "|(unposted batches)";
				}
				ANS = pop_up(0, 0, "@BATCHES", refs, params, "T", "1", tt, "", "", "", "K");
				if (not ANS) {
					return 0;
				}
				ANS.converter(FM, VM);
				if (ANS.index(VM)) {
					while (true) {
					///BREAK;
					if (not(ANS.index(VM ^ VM))) break;;
						ANS.swapper(VM ^ VM, VM);
					}//loop;
					win.newbrowse = 1;
					ANS.transfer(win.browsenext);
					win.reset = 4;
				}
				ANS = ANS.field("*", 3);

				//force prefix into @ID
				if (suffix) {
					ID = ID.fieldstore("*", 4, 1, suffix[2]);
				}

				DATA ^= "" "\r";
				return 0;
}

subroutine keychange() {
				if (win.wlocked or RECORD) {
					msg = "PLEASE CLEAR THIS BATCH FIRST";
					goto invalid2;
				}
				return;

//////////////
				case valbatchref:
//////////////
				gosub badchars();
				if (not(win.valid)) {
					return;
				}
				if (win.is ne win.registerx(4) and win.is ne win.isorig) {

					gosub keychange();
					if (not(win.valid)) {
						return;
					}

					//query by voucher number
					if (win.is[1] == "V") {
						if (not(win.is.readv(fin.vouchers, ID.field("*", 2) ^ "*" ^ win.is.substr(2,99) ^ "*" ^ fin.currcompany, 12))) {
							{}
						}
					}

					suffix = "";
					if (var(".U").index(win.is[1])) {
						suffix = "*U";
						win.is.splicer(1, 1, "");
						}
					if (not(tt.read(win.srcfile, ID.fieldstore("*", 3, 1, win.is) ^ suffix))) {
						msg = DQ ^ (win.is ^ DQ) ^ " " "NO SUCH BATCH - IF YOU WANT TO START A|NEW BATCH JUST PRESS [Enter]";
						win.is = "";
						gosub invalid2();
						win.reset = 4;
						return;
					}
					if (suffix) {
						ID = ID.fieldstore("*", 4, 1, "U");
					}

					//check allowed access to posted batches
					if (win.orec and not ID.field("*", 4)) {
						if (not(authorised("JOURNAL ACCESS POSTED", msg, ""))) {
							goto invalid2;
						}
					}

	//be careful! t is used below

					//check if own batch (any user in history not just latest!)
					//locate @username in t<25> setting temp else
					if (tt.a(25, 1) ne USERNAME) {
						if (not(authorised("JOURNAL ACCESS OTHERS", msg, ""))) {
							//done in BATCHES.DICT now for webui
							msg.r(-1, FM ^ "Batch " ^ (DQ ^ (ID ^ DQ)) ^ " belongs to " ^ (DQ ^ (tt.a(25, 1) ^ DQ)));
							win.is = "";
							goto invalid2;
						}
					}

				}
				return;

///////////
				case defperiod:
///////////
				win.isdflt = fin.currperiod ^ "/" ^ addcent(fin.curryear);
				return;

///////////
				case valperiod:
///////////
				//also used by REVERSALS and REVAL
				win.isdflt = "";
				if (not(win.is)) {
					return;
				}
				if (win.is == win.isorig) {
					return;
				}

				//no leading zero
				if (win.is[1] == "0") {
					win.is.splicer(1, 1, "");
				}

				//add current year
				win.is.converter(" ", "/");
				tt = win.is.field("/", 2);
				if (tt == "") {
					if (win.is > fin.currperiod) {
						tt = (addcent(fin.curryear - 1)).oconv("R(0)#2");
					}else{
						tt = fin.curryear;
					}
					win.is = win.is.fieldstore("/", 2, 1, tt);
				}else{
					win.is = win.is.fieldstore("/", 2, 1, ("00" ^ tt).substr(-2,2));
				}

				//check period
				var isperiod = win.is;
				gosub chk.period();
				if (not(win.valid)) {
					return;
				}

				return;

///////////
addversion:
///////////
				var action = mode.field(".", 2);

				//skip recording if reposting and authorised to repost without record and not dev
				if ((action == "A") and authorised("#JOURNAL REPOST WITHOUT RECORD", msg, "NEOSYS")) {
					return;
				}

				//similar code in DAYBOOK.SUBS IMPORTDAYBOOK.SUBS RECURRING.SUBS

				var version = RECORD.a(41) + 1;
				RECORD.r(41, version);

				if ((action == "A") or (ID.field("*", 4) == "")) {
					if ((RECORD.a(24, 1) == "A") or (RECORD.a(29, 1) == "POSTED")) {
						status = "REPOSTED";
					}else{
						status = "POSTED";
					}
				}else{
					status = "UNPOSTED";
				}

				RECORD.inserter(24, 1, action);
				RECORD.inserter(25, 1, USERNAME);
				//garbagecollect;
				var datetime = var().date() ^ "." ^ var().time().oconv("R(0)#5");
				RECORD.inserter(26, 1, datetime);
				RECORD.inserter(27, 1, STATION.trim());
				RECORD.inserter(28, 1, version);
				RECORD.inserter(29, 1, status);

				return;

///////////////////////
raiseunpostedminimum:
///////////////////////
				//if this is the lowest batch number then update the minimum batch number
				var seqkey = "%" ^ fin.currcompany ^ "*" ^ raisebatchtype ^ "*U%";
				var min;
				if (min.read(fin.batches, seqkey ^ "*")) {
					min = min.a(1);
					//IF MIN EQ FIELD(@ID,'*',3) THEN
					//mar2014
					if (min >= ID.field("*", 3)) {
						var max;
						if (max.read(fin.batches, seqkey)) {
							max = max.a(1);
							goto 1731;
						}
						max = min;
L1731:      *go from minimum+1 to maximum but stopping at any existing batch;

						for (var min = 1; min <= max; ++min) {
							if (not(tt.read(fin.batches, fin.currcompany ^ "*" ^ raisebatchtype ^ "*" ^ min ^ "*U"))) {
								tt = "";
							}
						///BREAK;
						if (tt) break;;
						};//min;
						//end;*bug missing? added then removed Mar2014
						if (min > max) {
							min = max;
						}
						min.writev(fin.batches, seqkey ^ "*", 1);
						
					}
				}
				return;

}


libraryexit()
