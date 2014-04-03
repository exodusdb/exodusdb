#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <nextkey.h>
#include <addcent.h>
#include <singular.h>

#include <fin.h>
#include <gen.h>
#include <win.h>

#include <window.hpp>//after win

var raisebatchtype;
var vn;
var reply;//num
var suffix;
var temp;
var ln1;//num
var addcurr;
var addamt;//num
var addbase;//num
var baseamtfn;
var add;//num
var currn;
var wsmsg;
var msg;
var interactive;

function main(in mode) {
	//garbagecollect;
	//y2k2
	// daybook batch header validation and interface routines

	interactive = not SYSTEM.a(33);

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


	if (mode eq "DEF.BATCH.REF") {

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

/*
	/////////////
} else if (mode eq "F2.BATCH.REF") {
	/////////////
	gosub keychange();
	if (not win.valid) {
		return 0;
	}

	if (authorised("JOURNAL ACCESS POSTED BATCHES", msg, "")) {
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
	if (ANS.index(VM, 1)) {
		while (true) {
		///BREAK;
		if (not(ANS.index(VM ^ VM, 1))) break;;
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

	//////////////
valbatchref:
	//////////////
	gosub EOF_516();
	if (not win.valid) {
		return;
	}
	if (win.is ne win.registerx(4) and win.is ne win.isorig) {

		gosub keychange();
		if (not win.valid) {
			return;
		}

		//query by voucher number
		if (win.is[1] == "V") {
			if (not(win.is.readv(fin.vouchers, ID.field("*", 2) ^ "*" ^ win.is.substr(2, 99) ^ "*" ^ fin.currcompany, 12))) {
				{}
			}
		}

		suffix = "";
		if (var(".U").index(win.is[1], 1)) {
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
		if (not(ID.field("*", 4))) {
			if (not(authorised("JOURNAL ACCESS POSTED BATCHES", msg, ""))) {
				goto invalid2;
			}
		}

		//check if own batch
		if (not(tt.locate(USERNAME, temp, 25))) {
			if (not(authorised("JOURNAL ACCESS OTHER PEOPLES", msg, ""))) {
				win.is = "";
				goto invalid2;
			}
		}

	}
	return;

} else if (mode eq "DEF.PERIOD") {

	win.isdflt = fin.currperiod ^ "/" ^ addcent(fin.curryear);

*/


	} else if (mode eq "VAL.PERIOD") {

		//also used by REVERSALS and REVAL
		win.isdflt = "";
		if (not win.is) {
			return 0 ;
		}
		if (win.is == win.isorig) {
			return 0 ;
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
			win.is = win.is.fieldstore("/", 2, 1, var("00" ^ tt).substr(-2, 2));
		}

		//check period
		//var isperiod = win.is;
		gosub chkperiod(win.is);
		if (not win.valid)
			return 0;


	} else if (mode.field(".", 1) eq "TRACE") {

		var action = mode.field(".", 2);
		if (action == "A" and authorised("JOURNAL REPOST WITHOUT RECORD", msg, "NEOSYS")) {
			return 0;
		}
		RECORD.inserter(24, 1, action);
		RECORD.inserter(25, 1, USERNAME);
		//R=INSERT(R,26,1,0,ICONV(DATE(),'HEX'))
		RECORD.inserter(26, 1, var().date());
		RECORD.inserter(27, 1, var().time());


	} else if (mode.field("*", 1) == "RAISE.UNPOSTED.MINIMUM") {
		raisebatchtype = mode.field("*", 2);
		if (not raisebatchtype) {
			raisebatchtype = win.registerx(10).a(1);
		}

		//if this is the lowest batch number then update the minimum batch number
		var seqkey = "%" ^ fin.currcompany ^ "*" ^ raisebatchtype ^ "*U%";
		var min,max;
		if (min.read(fin.batches, seqkey ^ "*")) {
			min = min.a(1);
			if (min == ID.field("*", 3)) {
				if (max.read(fin.batches, seqkey)) {
					max = max.a(1);
				} else {
					max = min;
				}
				//go from minimum+1 to maximum but stopping at any existing batch;
				for (var min = 1; min <= max; ++min) {
					if (not(tt.read(fin.batches, fin.currcompany ^ "*" ^ raisebatchtype ^ "*" ^ min ^ "*U"))) {
						tt = "";
					}
					///BREAK;
					if (tt)
						break;;
				};//min;
			}
			if (min > max) {
				min = max;
			}
			min.writev(fin.batches, seqkey ^ "*", 1);
		}

	} else {
		//unknown mode
		msg = "SYSTEM ERROR IN DAYBOOK.SUBS3|INVALID MODE " ^ (DQ ^ (mode ^ DQ)) ^ " IGNORED";
		//goto note3;
		return invalid(msg);
	}

	return 0;

}

function chkperiod(in isperiod) {

	//check if allowed to post into other periods
	if (isperiod ne (fin.currperiod ^ "/" ^ fin.curryear)) {
		if (not(authorised("JOURNAL POST OUTSIDE CURRENT PERIOD", msg, ""))) {
			return invalid2(msg);
		}
	}

	//check minimum period
	var minperiod = gen.company.a(16);
	if (minperiod) {
		//T=MIN.PERIOD[-2,2]:('00':FIELD(MIN.PERIOD,'/',1))[-2,2]
		var tt = addcent(minperiod.substr(-2, 2)) ^ ("00" ^ minperiod.field("/", 1)).substr(-2, 2);
		//IF ADDCENT(T) GE ADDCENT(IS.PERIOD[-2,2]:('00':FIELD(IS.PERIOD,'/',1))[-2,2]) THEN
		var t2 = addcent(isperiod.field("/", 2)) ^ ("00" ^ isperiod.field("/", 1)).substr(-2, 2);
		//call msg(t:' ':t2:' ':is.period)
		if (tt >= t2) {
			msg = "Sorry, the ledgers are closed up to " ^ addcent(minperiod) ^ "|for company " ^ (DQ ^ (gen.company.a(1) ^ DQ)) ^ " (" ^ fin.currcompany ^ ")";
			return invalid2(msg);
		}
	}

	//check maximum month
	var tt = isperiod.field("/", 1);
	if (tt < 1 or tt > fin.maxperiod or not tt.match("0N")) {
badperiod:
		msg = "Please enter a period, or period/year|eg. " ^ (DQ ^ (fin.currperiod ^ DQ)) ^ " or " ^ (DQ ^ (fin.currperiod ^ "/" ^ fin.curryear ^ DQ));
		return invalid2(msg);
	}

	//check year
	tt = (isperiod.field("/", 2)).substr(-2, 2);
	//IF t matches '1N' or T MATCHES '2N' ELSE GOTO BADPERIOD
	if (not tt.match("2N")) {
		goto badperiod;
	}

	//check maximum year
	if (addcent(tt) > addcent((gen.company.a(2)).substr(-2, 2))) {
		msg = "The year " ^ (DQ ^ (addcent(tt) ^ DQ)) ^ " must be opened for|" ^ gen.company.a(1) ^ "|before you can post to it";
		return invalid2(msg);
	}

	//note if not current year
	if (interactive and tt ne fin.curryear) {
		tt = addcent(fin.curryear) - addcent(tt);
		if (tt > 5) {
			msg = "WARNING: THAT IS " ^ tt ^ " YEARS BEFORE THE CURRENT YEAR !!!";
			gosub note3(msg);
		}
		//not needed by reval.subs
		//MSG='NOTE. THE CURRENT YEAR IS ':CURRYEAR
		//GOSUB NOTE3
	}

	return 1;

}

/*
function invalid2(in msg) {
	if (not(win.registerx(3))) {
		invalid(msg);
	}
	if (win.isorig) {
		win.is = win.isorig;
	}
	win.valid = 0;
	win.reset = 1;
	//mode = msg;
	return 0;
}

function note3(in msg) {
	if (not msg) {
		return 0;
	}
	if (win.registerx(3)) {
		return 0;
	}
	note(msg);
	return 0;

}
*/

libraryexit()
