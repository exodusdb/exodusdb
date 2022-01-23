#include <exodus/library.h>

libraryinit(alines)
//-----------------
function main() {
	ANS = RECORD.count("\n") + 1;
	if (RECORD eq "") {
		ANS = 0;
	}
	return ANS;
}
libraryexit(alines)

libraryinit(all)
//--------------
function main() {
	var ans = ID ^ FM ^ RECORD;
	return ans;
}
libraryexit(all)

libraryinit(alluppercase)
//-----------------------
function main() {
	RECORD.ucaser();
	return RECORD;
	//@ANS=@RECORD
	//convert @lower.case to @upper.case in @ans
	var ans = RECORD;
	ans.ucaser();
	//can cause out of memory
	//1. doesnt help
	//declare function memspace
	//mem=memspace(999999)
	//2. dont pass back in @ans variable seems to solve it!
	//transfer @ans to ans
	//if you return something then it uses it, otherwise it uses what is in @ans
	return RECORD;
}
libraryexit(alluppercase)

libraryinit(alpha)
//----------------
function main() {
	ANS = ID;
	ANS.converter("0123456789", "");
	return ANS;
}
libraryexit(alpha)

libraryinit(anydebug)
//-------------------
function main() {
	if (var("*$").index(ID[1])) {
		return 0;
	}
	if (ID eq "DEBUG") {
		return 0;
	}
	if (ID eq "MSG") {
		return 0;
	}
	if (ID eq "RTP25") {
		return 0;
	}
	if (ID eq "TEST") {
		return 0;
	}
	if (ID eq "TESTBASIC") {
		return 0;
	}

	//upper=@record
	//convert @lower.case to @upper.case in upper

	if (RECORD.index("debug")) {
		return 1;
	}
	if (ID eq "SENDMAIL") {
		RECORD.converter("'EXODUS.ID'", "");
	}
	if (RECORD.index("'EXODUS.ID'") and ID ne "INSTALLALLOWHOSTS") {
		return 1;
	}
	return 0;
}
libraryexit(anydebug)

libraryinit(caseratio)
//--------------------
function main() {
	var xx = RECORD;
	xx.converter(LOWERCASE, "");
	var yy = RECORD;
	yy.converter(UPPERCASE, "");
	var uu = RECORD.length() - yy.length();
	var ll = RECORD.length() - xx.length();
	if (uu gt ll) {
		ANS = uu / (ll + 1);
	} else {
		ANS = -(ll / (uu + 1));
	}
	return ANS;
}
libraryexit(caseratio)

libraryinit(conv2utf8)
//--------------------
#include <conv2utf8.h>

var msg;

function main() {
	var origrecord = RECORD;
	var origid = ID;
	call conv2utf8(msg, RECUR0, "");
	RECUR1 += 1;
	if (TERMINAL)
		output(at(-40), RECUR1, ". ", RECCOUNT);
	ANS = msg;
	if (not ANS) {
		ANS = RECORD ne origrecord or ID ne origid;
	}
	//restore otherwise select will not find it again
	ID = origid;
	return ANS;
}
libraryexit(conv2utf8)

libraryinit(count)
//----------------
function main() {
	return 1;
}
libraryexit(count)

libraryinit(c_args)
//-----------------
function main() {
	ANS = RECORD.a(2);
	if (ANS.substr(1, 3) ne "*c ") {
		ANS = "";
	}
	return ANS;
}
libraryexit(c_args)

libraryinit(data)
//---------------
function main() {
	ANS = RECORD;
	ANS.converter(_RM_ _FM_ _VM_ _SM_ _TM_ _STM_ "\u07f9\u07f8", "");
	return ANS;
}
libraryexit(data)

libraryinit(datetime_created)
//---------------------------
function main() {
	return calculate("DATETIME_UPDATED").a(1, 1);
}
libraryexit(datetime_created)

libraryinit(different)
//--------------------

#include <sys_common.h>

var rec;

function main() {
	#include <system_common.h>

	//COMMON /DIFFERENT/ LAST.RECCOUNT, COMPARE.FILE
	//IF COMPARE.FILE EQ '' THEN LAST.RECCOUNT = 9999
	//!IF @RECCOUNT LT LAST.RECCOUNT THEN
	//!  FN=''
	//!  CALL MSG('DIFFERENT FROM WHAT FILE','RC',FN,'')
	//

	var fn = "QFILE";
	var comparefile;
	if (not(comparefile.open(fn))) {
		printl("CANNOT OPEN " ^ fn);
		stop();
	}

	//END
	var lastreccount = RECCOUNT;
	if (rec.reado(comparefile, ID)) {
		if (RECORD eq rec) {
			ANS = "";
		} else {
			if (ID[1] eq "$") {
				var recdatetime = field2(RECORD, FM, -1);
				recdatetime = recdatetime.field(" ", 2, 9).trim() ^ " " ^ recdatetime.field(" ", 1);
				recdatetime = recdatetime.iconv("DT");
				var cmpdatetime = field2(rec, FM, -1);
				cmpdatetime = cmpdatetime.field(" ", 2, 9).trim() ^ " " ^ cmpdatetime.field(" ", 1);
				cmpdatetime = cmpdatetime.iconv("DT");
				if (not(recdatetime and cmpdatetime)) {
					goto changed;
				}
				if (recdatetime eq cmpdatetime) {
					goto changed;
				}
				if (recdatetime gt cmpdatetime) {
					ANS = "REPLACES";
				} else {
					ANS = "REPLACED";
				}
			} else {
changed:
				ANS = "CHANGED";
			}
		}
	} else {
		ANS = "NEW REC";
	}
	return ANS;
}
libraryexit(different)

libraryinit(display2)
//-------------------
function main() {
	ANS = RECORD.a(3);
	ANS.converter(VM, " ");
	return ANS;
}
libraryexit(display2)

libraryinit(dt)
//-------------
function main() {
	return var().date();
}
libraryexit(dt)

libraryinit(executive_email)
//--------------------------
function main() {
	var executivecode = calculate("EXECUTIVE_CODE");
	executivecode.ucaser();

	//@ in executive name assume is an email email
	if (executivecode.index("@")) {
		ANS = executivecode;
		ANS.converter(" ,", ";;");
	} else {

	//1) look for user code directly
		var userx = xlate("USERS", executivecode, "", "X");

	//2) look for user name
		if (not userx) {
			userx = xlate("USERS", "%" ^ executivecode ^ "%", "", "X");
		}

	//3) try to use the first word of the executive code as the username
	//first name only
		if (not userx) {
			userx = xlate("USERS", executivecode.field(" ", 1), "", "X");
		}

		if (userx.a(35) and var().date() ge userx.a(35)) {
		//expired
			ANS = "";
		} else {
		//not expired
			ANS = userx.a(7);
		}

	//runtime users email
		if (not ANS) {
			ANS = USERNAME.xlate("USERS", 7, "X");
		}

	}
	return ANS;
}
libraryexit(executive_email)

libraryinit(executive_name)
//-------------------------

function main() {
	//ans={EXECUTIVE_CODE}
	//ans2=ans
	//convert @lower.case to @upper.case in ans2
	//@ans=xlate('USERS',ans2,1,'X')
	//if @ans else @ans=ans
	var ans = calculate("EXECUTIVE_CODE");
	var ans2 = ans;
	ans2.ucaser();
	if (ans2 ne ans) {
		ans.transfer(ANS);
	} else {
	//@ans=xlate('USERS','%':ans2:'%',1,'X')
		ANS = ans2.xlate("USERS", 1, "X");
		if (ANS) {
			ans2 = ANS;
			ans2.ucaser();
			if (ans2 eq ANS) {
				ANS = capitalise(ANS);
				}
			} else {
			ans.transfer(ANS);
			ANS = capitalise(ANS);
		}
	}
	return ANS;
}
libraryexit(executive_name)

libraryinit(field)
//----------------

function main() {
	if (not RECUR0) {
		call mssg("WHICH FIELD NUMBER ?", "R", RECUR0, "");
	}
	return RECORD.a(RECUR0);
}
libraryexit(field)

libraryinit(findall)
//------------------
var datax;

function main() {
	if (RECORD.a(1)) {
		datax = RECORD.a(8);
		datax.converter(VM, FM);
	} else {
		datax = RECORD;
	}
	var nn = datax.count(FM) + 1;
	datax.ucaser();
	datax.converter(DQ, "'");
	ANS = "";
	for (var fn = 1; fn <= nn; ++fn) {
		var tx = datax.a(fn).trim();

		if (tx.index("xlate(") and ((tx.index("','C')") or tx.index("','X')")))) {
		//gosub change2
			ANS(1, -1) = tx;
		}

	//if index(tx,'MSG(',1) and ( index(tx,",'U",1) or index(tx,",'D",1) or index(tx,",'R",1) ) then
	// gosub change
	// @ans<1,-1>=tx
	// end

	//if index(tx,'NOTE(',1) and ( index(tx,",'U",1) or index(tx,",'D",1) or index(tx,",'R",1) or index(tx,",'T",1)) then
	// gosub change
	// @ans<1,-1>=tx
	// end

	} //fn;
	return ANS;
	/*;
	change:
		tx=data<fn>;
		swap 'MSG(' with 'msg2(' in tx;
		declare function decide2;
		if decide2(@id:@fm:tx:'','',reply,2) else stop;
		if reply=1 then;
			open 'TEMP' to file else call fsmsg();stop;
			//writev tx on file,@id,fn
			if @record<1>='S' then;
				@record<8,fn>=tx;
			end else;
				@record<fn>=tx;
				end;
			write @record on file,@id;
			end;
		return 0;
	*/
	return ANS;
}
libraryexit(findall)

libraryinit(iscpp)
//----------------
function main() {
	return RECORD.a(2).substr(1, 3) eq "*c ";
}
libraryexit(iscpp)

libraryinit(key1)
//---------------
function main() {
	return ID.field("*", 1);
}
libraryexit(key1)

libraryinit(key2)
//---------------
function main() {
	return ID.field("*", 2);
}
libraryexit(key2)

libraryinit(key3)
//---------------
function main() {
	return ID.field("*", 3);
}
libraryexit(key3)

libraryinit(key4)
//---------------
function main() {
	return ID.field("*", 4);
}
libraryexit(key4)

libraryinit(keylen)
//-----------------
function main() {
	return ID.length();
}
libraryexit(keylen)

libraryinit(keysize)
//------------------
function main() {
	return ID.length();
}
libraryexit(keysize)

libraryinit(linemarks)
//--------------------
function main() {
	if (var("$*").index(ID[1])) {
		return "";
	} else if (RECORD.index("linemark")) {
		var tt = RECORD;
		tt.converter(" ", FM);
		return tt.index(FM ^ "linemark");
	} else {
		ANS = 0;
	}
	return ANS;
}
libraryexit(linemarks)

libraryinit(lines)
//----------------
function main() {
	ANS = RECORD;
	ANS.converter(FM, VM);
	return ANS;
}
libraryexit(lines)

libraryinit(log_message2)
//-----------------------

function main() {
	return trim(RECORD.a(2), VM);
}
libraryexit(log_message2)

libraryinit(log_source1)
//----------------------
function main() {
	return RECORD.a(1).field(" ", 1);
}
libraryexit(log_source1)

libraryinit(mv)
//-------------
function main() {
	return MV;
}
libraryexit(mv)

libraryinit(nfields)
//------------------
function main() {
	return RECORD.count(FM) + 1;
}
libraryexit(nfields)

libraryinit(nfields0)
//-------------------
function main() {
	RECORD.converter(_FM_ " ", " " _FM_);
	return RECORD.trim().count(" ") + 1;
}
libraryexit(nfields0)

libraryinit(nfields00)
//--------------------
function main() {
	//remove comment blocks
	while (true) {
		var pos1 = RECORD.index("/*");
		///BREAK;
		if (not pos1) break;
		var pos2 = RECORD.index("*/");
		if (not pos2) {
			pos2 = RECORD.length() + 1;
		}
		RECORD.splicer(pos1, pos2 - pos1 + 2, "");
	}//loop;
	//call msg('x')
	//remove comment lines
	var nfields = RECORD.count(FM) + 1;

	//dim x(nfields)
	//matparse @record into x
	for (var ii = 1; ii <= nfields; ++ii) {
	//if trim(x(i))[1,1]='*' then x(i)=''
		if (RECORD.a(ii).trim()[1] eq "*") {
			RECORD(ii) = "";
		}
	} //ii;
	//@record=matunparse(x)
	//             call msg('y')
	//remove blank lines
	RECORD.converter(_FM_ " ", " " _FM_);

	nfields = RECORD.trim().count(" ") + 1;
	return nfields;
}
libraryexit(nfields00)

libraryinit(nlines)
//-----------------
function main() {
	ANS = RECORD.count(FM) + 1;
	if (RECORD eq "") {
		ANS = 0;
	}
	return ANS;
}
libraryexit(nlines)

libraryinit(num)
//--------------
function main() {
	ANS = ID;
	var hold = ID;
	hold.converter("0123456789", "");
	ANS.converter(hold, "");
	return ANS;
}
libraryexit(num)

libraryinit(objectcodetype)
//-------------------------
function main() {
	return RECORD.a(1).seq();
}
libraryexit(objectcodetype)

libraryinit(program_date)
//-----------------------
function main() {
	var ans = calculate("TIMEDATE");
	return ans.substr(11, 99).iconv("D");
}
libraryexit(program_date)

libraryinit(recnum)
//-----------------
function main() {
	//@reccount is the same thing
	return RECCOUNT;
}
libraryexit(recnum)

libraryinit(record)
//-----------------
function main() {
	return RECORD;
}
libraryexit(record)

libraryinit(size)
//---------------
function main() {
	return RECORD.length() + ID.length() + 5;
}
libraryexit(size)

libraryinit(spacer)
//-----------------
function main() {
	return "";
}
libraryexit(spacer)

libraryinit(temp)
//---------------
function main() {
	return RECORD.count("xlate(");
}
libraryexit(temp)

libraryinit(time)
//---------------
function main() {
	return ID.osfile().field(FM, 3);
}
libraryexit(time)

libraryinit(timedate)
//-------------------

#include <sys_common.h>

function main() {
	#include <system_common.h>
	var ans = field2(RECORD, FM, -1);
	if (ans[1] eq "V") {
		ans = field2(RECORD, FM, -2);
	}
	if (not(ans.substr(1, 2).match("^\\d{2}$"))) {
		ANS = "";
	} else {
		ANS = ans;
		}
	return ANS;
}
libraryexit(timedate)

libraryinit(username)
//-------------------
function main() {
	return USERNAME;
}
libraryexit(username)

libraryinit(username_created)
//---------------------------
function main() {
	return calculate("USERNAME_UPDATED").a(1, 1);
}
libraryexit(username_created)

libraryinit(version)
//------------------

function main() {
	if (DICT.index("VOC")) {
		ANS = xlate(RECORD.a(3), "$" ^ RECORD.a(4), "VERSION", "X");
	} else {
		ANS = field2(RECORD, FM, -1);
	//IF @ANS[1,1] = 'V' THEN
	//  @ANS=@ANS[2,99]
	//END ELSE
	//  @ANS = ''
	}
	return ANS;
}
libraryexit(version)

libraryinit(version_date)
//-----------------------
function main() {
	return calculate("VERSION").trim().field(" ", 2, 3).iconv("D");
}
libraryexit(version_date)
