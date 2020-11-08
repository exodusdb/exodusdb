#include <exodus/library.h>
libraryinit()

#include <nextkey.h>
#include <addcent.h>
#include <authorised.h>
#include <singular.h>

#include <fin_common.h>
#include <gen_common.h>
#include <win_common.h>

#include <window.hpp>

var interactive;
var tt;
var raisebatchtype;
var modea;
var modeb;
var vn;
var msg;
var isperiod;
var action;
var version;
var status;
var datetime;
var seqkey;
var min;//num
var max;
var minperiod;
var t2;
var op;
var op2;
var wspos;
var wsmsg;
var xx;

function main(in mode) {
	//c fin
	//global all

	//seems to be used only for VAL.PERIOD and RAISEUNPOSTEDMINIMUM
	//garbagecollect;
	//y2k2
	// daybook batch header validation and interface routines

	#include <common.h>
	#include <general_common.h>
	interactive = not(SYSTEM.a(33));

	#define validating win.registerx(3)
	#define tstore win.registerx(4)
	#define otheraccs win.registerx(5)
	#define contraaccs win.registerx(6)
	#define sign win.registerx(7)
	#define continuewi win.registerx(8)
	#define contraaccount win.registerx(9)
	#define batchtype win.registerx(10).a(1)
	#define ibatchtype win.registerx(10).a(2)
	//equ printtypes to register(10)<3>
	//equ tstore2 to register(10)<4>

	//removed so we can call raise.unposted.minimum without setting @id
	//tt=field(@id,'*',1)
	//IF tt NE CURR.COMPANY THEN
	// IF tt THEN
	//  *CURR.COMPANY=tt
	//  CALL INIT.COMPANY(tt)
	//  if interactive then
	//   MSG='YOU HAVE JUST CHANGED BACK TO ':QUOTE(CURR.COMPANY):' COMPANY'
	//   GOSUB NOTE3
	//   end
	//  END
	// END

	win.redisplaylist = "";
	tt = "";
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
			raisebatchtype = batchtype;
		}
		goto raiseunpostedminimum;
	}

	if (mode == "DEF.BATCH.REF") {
		goto defbatchref;
	}

	//none used from web
	//LOCATE MODE IN 'F2.BATCH.REF,VAL.BATCH.REF' using ',' SETTING VN THEN
	// valid=1
	// ON VN GOTO F2.BATCH.REF,VAL.BATCH.REF
	// END

	modea = mode.substr(1,4);
	modeb = mode.substr(5,99);

	//preprompt routines
	if (modea == "DEF.") {
		if (not(win.wlocked)) {
			return 0;
		}
		win.valid = 1;
		if (var("PERIOD").locate(modeb,vn)) {
			goto defperiod;
		}
	}

	//postprompt routines
	if (modea == "VAL.") {
		if (win.amvaction == 2) {
			return 0;
		}
		if (var("PERIOD").locate(modeb,vn)) {
			goto valperiod;
		}
	}

	//F2 options routines
	//LOCATE MODE IN 'F2.PERIOD' SETTING VN THEN
	// IF WLOCKED ELSE
	//  CALL MSG('W810')
	//  RETURN
	//  END
	// valid=1
	// ON VN GOTO F2.PERIOD
	// END

	//unknown mode
	msg = "SYSTEM ERROR IN DAYBOOK.SUBS3|INVALID MODE " ^ (mode.quote()) ^ " IGNORED";
	gosub note3();
	return 0;

	//!********* SUBROUTINES ***********

//////////////
defbatchref:
//////////////
	if (not(win.wlocked or RECORD)) {
		if (not(ID.field("*", 1))) {
			ID = ID.fieldstore("*", 1, 1, fin.currcompany);
		}
		if (not(ID.field("*", 2))) {
			ID = ID.fieldstore("*", 2, 1, batchtype);
		}

		tt = ":%" ^ ID.field("*", 1, 2) ^ "%:BATCHES:" ^ ID.field("*", 1, 2) ^ "*";
		//eg tt=":%H*INV%:BATCHES:H*INV*"

		tt = nextkey(tt, "");

		ID = tt;
		win.isdflt = tt.field("*", 3);
		ANS = "";
	}
	tstore = win.isdflt;
	return 0;

	/*;
	/////////////
	F2.BATCH.REF:
	/////////////
		GOSUB KEYCHANGE;
		IF VALID ELSE RETURN;

		if security('JOURNAL ACCESS POSTED',msg,'') then;
			IF DECIDE('','Posted batches|Unposted batches',REPLY) ELSE RETURN;
		end else;
			reply=2;
			end;
		IF REPLY EQ 1 THEN SUFFIX='' ELSE SUFFIX='*U';

		refs=getbatchcodes(curr.company,batch.type,suffix,batches);
		if refs else;
			msg='No batches found';
			goto invalidx;
			end;

		//select a batch
		if suffix then temp='Entered by' else temp='Posted By';
		PARAMS='BATCH_REFERENCE:5:R::Batch\FIRST_VOUCHER:10:R::1st Vouch.\CONTRA_ACCOUNT_NAME:30:::First account\WHO:10:::':temp:'\23:7:R:[ADDCENT]:Period\ACTION:1:::';
		tt='Which batch(es) do you want ?';
		if suffix then tt:='|(unposted batches)';
		@ANS=POP.UP(0,0,'@BATCHES',REFS,PARAMS,'T','1',tt,'','','','K');
		IF @ANS ELSE RETURN;
		convert fm to vm in @ans;
		if index(@ans,vm,1) then;
			loop;
			while index(@ans,vm:vm,1) do swap vm:vm with vm in @ans;
				repeat;
			new.browse=1;
			transfer @ans to browse.next;
			reset=4;
			end;
		@ANS=FIELD(@ANS,'*',3);

		//force prefix into @ID
		IF SUFFIX THEN;
			@ID=FIELDSTORE(@ID,'*',4,1,SUFFIX[2,1]);
			END;

		data '';
		RETURN;

	KEYCHANGE:
		IF WLOCKED OR R THEN;
			MSG='PLEASE CLEAR THIS BATCH FIRST';
			GOTO invalidx;
			END;
		RETURN;

	//////////////
	VAL.BATCH.REF:
	//////////////
		GOSUB BADCHARS;
		IF VALID ELSE RETURN;
		IF IS NE TSTORE AND IS NE IS.ORIG THEN;

			GOSUB KEYCHANGE;
			IF VALID ELSE RETURN;

			//query by voucher number
			if is[1,1]='V' then;
				readv is from vouchers,field(@id,'*',2):'*':is[2,99]:'*':curr.company,12 else null;
				end;

			SUFFIX='';
			IF INDEX('.U',IS[1,1],1) THEN;
				SUFFIX='*U';
				IS[1,1]='';
				END;
			READ tt FROM SRC.FILE,FIELDSTORE(@ID,'*',3,1,IS):SUFFIX ELSE;
				MSG=QUOTE(IS):' ':'NO SUCH BATCH - IF YOU WANT TO START A|NEW BATCH JUST PRESS [Enter]';
				IS='';
				GOsub invalidx;
				reset=4;
				return 0;
				END;
			IF SUFFIX THEN;
				@ID=FIELDSTORE(@ID,'*',4,1,'U');
				end;

			//check allowed access to posted batches
			if orec and not(field(@id,'*',4)) then;
				if security('JOURNAL ACCESS POSTED',msg,'') else;
					goto invalidx;
					end;
				end;

	//be careful! tt is used below

			//check if own batch (any user in history not just latest!)
			//locate @username in tt<25> setting temp else
			if tt<25,1> ne @username then;
				if security('JOURNAL ACCESS OTHERS',msg,'') else;
					//done in BATCHES.DICT now for webui
					msg<-1>=fm:'Batch ':quote(@id):' belongs to ':quote(tt<25,1>);
					is='';
					goto invalidx;
					end;
				end;

			END;
		RETURN;
	*/
///////////
defperiod:
///////////
	win.isdflt = fin.currperiod ^ "/" ^ addcent(fin.curryear);
	return 0;

///////////
valperiod:
///////////
	//also used by REVERSALS and REVAL
	win.isdflt = "";
	if (not(win.is)) {
		return 0;
	}
	if (win.is == win.isorig) {
		return 0;
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
	isperiod = win.is;
	gosub chkperiod();
	//call daybook.subs6('CHK.PERIOD',isperiod,msg)
	if (not(win.valid)) {
		return 0;
	}

	return 0;

///////////
addversion:
///////////
	action = mode.field(".", 2);

	//skip recording if reposting and authorised to repost without record and not dev
	if ((action == "A") and authorised("#JOURNAL REPOST WITHOUT RECORD", msg, "NEOSYS")) {
		return 0;
	}

	//similar code in DAYBOOK.SUBS IMPORTDAYBOOK.SUBS RECURRING.SUBS

	version = RECORD.a(41) + 1;
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
	datetime = var().date() ^ "." ^ var().time().oconv("R(0)#5");
	RECORD.inserter(26, 1, datetime);
	RECORD.inserter(27, 1, STATION.trim());
	RECORD.inserter(28, 1, version);
	RECORD.inserter(29, 1, status);

	return 0;

///////////////////////
raiseunpostedminimum:
///////////////////////
	//if this is the lowest batch number then update the minimum batch number
	seqkey = "%" ^ fin.currcompany ^ "*" ^ raisebatchtype ^ "*U%";
	if (min.read(fin.batches, seqkey ^ "*")) {
		min = min.a(1);
		//IF MIN EQ FIELD(@ID,'*',3) THEN
		//mar2014
		if (min >= ID.field("*", 3)) {

			if (max.read(fin.batches, seqkey)) {
				max = max.a(1);
			}else{
				max = min;
			}

			//go from minimum+1 to maximum but stopping at any existing batch
			for (min = 1; min <= max; ++min) {
				if (not(tt.read(fin.batches, fin.currcompany ^ "*" ^ raisebatchtype ^ "*" ^ min ^ "*U"))) {
					tt = "";
				}
				///BREAK;
				if (tt) break;
			};//min;

			//end;*bug missing? added then removed Mar2014
			if (min > max) {
				min = max;
			}

			min.writev(fin.batches, seqkey ^ "*", 1);

		}
	}
	return 0;
}

subroutine chkperiod() {

	//check if allowed to post into other periods
	if (isperiod ne (fin.currperiod ^ "/" ^ fin.curryear)) {
		if (not(authorised("JOURNAL POST OUTSIDE CURRENT PERIOD", msg, ""))) {
			gosub invalidx(msg);
			return;
		}
	}

	//check minimum period
	minperiod = gen.company.a(16);
	if (minperiod) {
		//tt=MIN.PERIOD[-2,2]:('00':FIELD(MIN.PERIOD,'/',1))[-2,2]
		tt = addcent(minperiod.substr(-2,2)) ^ ("00" ^ minperiod.field("/", 1)).substr(-2,2);
		//IF ADDCENT(tt) GE ADDCENT(isperiod[-2,2]:('00':FIELD(isperiod,'/',1))[-2,2]) THEN
		t2 = addcent(isperiod.field("/", 2)) ^ ("00" ^ isperiod.field("/", 1)).substr(-2,2);
		//call msg(tt:' ':t2:' ':isperiod)
		if (tt >= t2) {
			msg = "Sorry, the ledgers are closed up to " ^ addcent(minperiod) ^ "|for company " ^ (gen.company.a(1).quote()) ^ " (" ^ fin.currcompany ^ ")";
			gosub invalidx(msg);
			return;
		}
	}

	//check maximum month
	tt = isperiod.field("/", 1);
	if (((tt < 1) or (tt > fin.maxperiod)) or not(tt.match("^\\d*$"))) {
badperiod:
		msg = "Please enter a period, or period/year|eg. " ^ (fin.currperiod.quote()) ^ " or " ^ ((fin.currperiod ^ "/" ^ fin.curryear).quote());
		gosub invalidx(msg);
		return;
	}

	//check year
	tt = isperiod.field("/", 2).substr(-2,2);
	//IF tt matches '1N' or tt MATCHES '2N' else goto badperiod

	if (not(tt.match("^\\d{2}$"))) {
		goto badperiod;
	}

	//check maximum year
	if (addcent(tt) > addcent(gen.company.a(2).substr(-2,2))) {
		msg = "The year " ^ (addcent(tt).quote()) ^ " must be opened for|" ^ gen.company.a(1) ^ "|before you can post to it";
		gosub invalidx(msg);
		return;
	}

	//note if not current year
	//IF interactive and tt NE CURRYEAR THEN
	if (tt ne fin.curryear) {
		tt = addcent(fin.curryear) - addcent(tt);
		if (tt > 5) {
			msg = "WARNING: THAT IS " ^ tt ^ " YEARS BEFORE THE CURRENT YEAR !!!";
			//gosub note3
			if (not(validating)) {
				call note(msg);
			}
			return;
		}
		//not needed by reval.subs
		//MSG='NOTE. THE CURRENT YEAR IS ':CURRYEAR
		//GOSUB NOTE3
	}

	return;
}

subroutine note3() {
	if (not msg) {
		return;
	}
	if (validating) {
		return;
	}
	call note(msg);
	return;
}

subroutine invalidx(in msg) {

	//duplicated in daybook.subs2/3/4/6
	if (not(validating)) {
		gosub invalid(msg);
		return;
	}
	if (win.isorig) {
		win.is = win.isorig;
	}
	win.valid = 0;
	win.reset = 1;

	//TODO ensure that this is not needed in POSTBATCH etc
	//since it never gets back to caller via mode0
	//mode=msg

	return;
}

libraryexit()
