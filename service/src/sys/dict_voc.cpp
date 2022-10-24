#include <exodus/library.h>

libraryinit(alines)
//-----------------
function main() {
	ANS = RECORD.fcount("\n");
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
	return RECORD.ucaser();
}
/*pgsql
    return upper(data);
*/
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
	if (var("*$").contains(ID[1])) {
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

	// upper=@record
	// convert @lower.case to @upper.case in upper

	if (RECORD.contains("debug")) {
		return 1;
	}
	if (ID eq "SENDMAIL") {
		RECORD.converter("'EXODUS.ID'", "");
	}
	if (RECORD.contains("'EXODUS.ID'") and ID ne "INSTALLALLOWHOSTS") {
		return 1;
	}
	return 0;
}
libraryexit(anydebug)


libraryinit(c_args)
//-----------------
function main() {
	ANS = RECORD.f(2);
	if (ANS.first(3) ne "*c ") {
		ANS = "";
	}
	return ANS;
}
libraryexit(c_args)


libraryinit(caseratio)
//--------------------
function main() {
	var xx = RECORD;
	xx.converter("abcdefghijklmnopqrstuvwxyz", "");
	var yy = RECORD;
	yy.converter("ABCDEFGHIJKLMNOPQRSTUVWXYZ", "");
	var uu = RECORD.len() - yy.len();
	var ll = RECORD.len() - xx.len();
	if (uu gt ll) {
		ANS = uu / (ll + 1);
	} else {
		ANS = -(ll / (uu + 1));
	}
	return ANS;
}
libraryexit(caseratio)


libraryinit(count)
//----------------
function main() {
	return 1;
}
libraryexit(count)


libraryinit(cpp_text)
//-------------------
function main() {
	return RECORD.f(8).field(
		"/"
		"*pgsql",
		1);
}
libraryexit(cpp_text)


libraryinit(data)
//---------------
function main() {
	ANS = RECORD;
	ANS.converter(_RM _FM _VM _SM _TM _ST "\u07f9\u07f8", "");
	return ANS;
}
libraryexit(data)


libraryinit(datetime_created)
//---------------------------
function main() {
	return calculate("DATETIME_UPDATED").f(1, 1);
}
libraryexit(datetime_created)


libraryinit(different)
//--------------------
#include <sys_common.h>

var rec;

function main() {
	#include <system_common.h>

	// COMMON /DIFFERENT/ LAST.RECCOUNT, COMPARE.FILE
	// IF COMPARE.FILE EQ '' THEN LAST.RECCOUNT = 9999
	// !IF @RECCOUNT LT LAST.RECCOUNT THEN
	// !  FN=''
	// !  CALL MSG('DIFFERENT FROM WHAT FILE','RC',FN,'')
	// 

	var fn = "QFILE";
	var comparefile;
	if (not(comparefile.open(fn))) {
		printl("CANNOT OPEN " ^ fn);
		stop();
	}

	// END
	var lastreccount = RECCOUNT;
	if (rec.reado(comparefile, ID)) {
		if (RECORD eq rec) {
			ANS = "";
		} else {
			if (ID.starts("$")) {
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
	ANS = RECORD.f(3);
	ANS.converter(VM, " ");
	return ANS;
}
libraryexit(display2)


libraryinit(dt)
//-------------
function main() {
	return date();
}
libraryexit(dt)


libraryinit(executive_email)
//--------------------------
	// @ans=xlate('USERS',{EXECUTIVE_CODE},7,'C')
function main() {
	var executivecode = calculate("EXECUTIVE_CODE");
	executivecode.ucaser();

	// @ in executive name assume is an email email
	if (executivecode.contains("@")) {
		ANS = executivecode;
		ANS.converter(" ,", ";;");
	} else {

	// 1) look for user code directly
		var userx = xlate("USERS", executivecode, "", "X");

	// 2) look for user name
		if (not userx) {
			userx = xlate("USERS", "%" ^ executivecode ^ "%", "", "X");
		}

	// 3) try to use the first word of the executive code as the username
	// first name only
		if (not userx) {
			userx = xlate("USERS", executivecode.field(" ", 1), "", "X");
		}

		if (userx.f(35) and date() ge userx.f(35)) {
		// expired
			ANS = "";
		} else {
		// not expired
			ANS = userx.f(7);
		}

	// runtime users email
		if (not ANS) {
			ANS = USERNAME.xlate("USERS", 7, "X");
		}

	}
	return ANS;
}
libraryexit(executive_email)


libraryinit(executive_name)
//-------------------------
	// @ans=xlate('USERS',{EXECUTIVE_CODE},1,'C')
function main() {
	// ans={EXECUTIVE_CODE}
	// ans2=ans
	// convert @lower.case to @upper.case in ans2
	// @ans=xlate('USERS',ans2,1,'X')
	// if @ans else @ans=ans
	var ans = calculate("EXECUTIVE_CODE");
	var ans2 = ans;
	ans2.ucaser();
	if (ans2 ne ans) {
		ans.move(ANS);
	} else {
	// @ans=xlate('USERS','%':ans2:'%',1,'X')
		ANS = ans2.xlate("USERS", 1, "X");
		if (ANS) {
			ans2 = ANS;
			ans2.ucaser();
			if (ans2 eq ANS) {
				ANS = capitalise(ANS);
				}
			} else {
			ans.move(ANS);
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
	return RECORD.f(RECUR0);
}
libraryexit(field)


libraryinit(findall)
//------------------
var datax;

function main() {
	if (RECORD.f(1)) {
		datax = RECORD.f(8);
		datax.converter(VM, FM);
	} else {
		datax = RECORD;
	}
	let nn = datax.fcount(FM);
	datax.ucaser();
	datax.converter(DQ, "'");
	ANS = "";
	for (const var fn : range(1, nn)) {
		var tx = datax.f(fn).trim();

		if (tx.contains("xlate(") and ((tx.contains("','C')") or tx.contains("','X')")))) {
		// gosub change2
			ANS(1, -1) = tx;
		}

	// if index(tx,'MSG(',1) and ( index(tx,",'U",1) or index(tx,",'D",1) or index(tx,",'R",1) ) then
	// gosub change
	// @ans<1,-1>=tx
	// end

	// if index(tx,'NOTE(',1) and ( index(tx,",'U",1) or index(tx,",'D",1) or index(tx,",'R",1) or index(tx,",'T",1)) then
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
			open 'TEMP' to file else call abort(lasterror());
			// writef tx on file,@id,fn
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


libraryinit(full_brand_name)
//--------------------------
function main() {
	var brandcode = calculate("BRAND_CODE");
	var brand = brandcode.xlate("BRANDS", "", "X");
	var ans = brand.f(2);
	var brandcode1 = brand.f(8);
	brandcode1.converter(SM, VM);
	brandcode1 = brandcode1.f(1, 1);
	if (brandcode1 and brandcode1 ne brandcode) {
		ans.prefixer(brandcode1.xlate("BRANDS", 2, "X") ^ " ");
	}
	return ans;
}
/*pgsql BRAND_CODE
DECLARE
	brandcode text;
	brand text;
	brandcode2 text;
	brandname2 text;
BEGIN
	brandcode := key;
	brand := xlate BRANDS brandcode ''
	ans := split_part(brand,FM,2);
	brandcode2 := exodus_extract_text(brand,8,1,1);
	if brandcode2 != '' and brandcode2 != brandcode then
		brandname2 := xlate BRANDS brandcode2 2
		ans := brandname2 || ' ' || ans;
	end if;
END;
*/
libraryexit(full_brand_name)


libraryinit(iscpp)
//----------------
function main() {
	return RECORD.f(2).starts("*c ");
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
	return ID.len();
}
libraryexit(keylen)


libraryinit(keysize)
//------------------
function main() {
	return ID.len();
}
libraryexit(keysize)


libraryinit(linemarks)
//--------------------
function main() {
	if (var("$*").contains(ID[1])) {
		return "";
	} else if (RECORD.contains("linemark")) {
		var tt = RECORD;
		tt.converter(" ", FM);
		return tt.contains(FM ^ "linemark");
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
	return trim(RECORD.f(2), VM);
}
libraryexit(log_message2)


libraryinit(log_source1)
//----------------------
function main() {
	return RECORD.f(1).field(" ", 1);
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
	return RECORD.fcount(FM);
}
libraryexit(nfields)


libraryinit(nfields0)
//-------------------
function main() {
	RECORD.converter(_FM " ", " " _FM);
	return RECORD.trim().fcount(" ");
}
libraryexit(nfields0)


libraryinit(nfields00)
//--------------------
function main() {
	// remove comment blocks
	while (true) {
		var pos1 = RECORD.index("/*");
		// /BREAK;
		if (not pos1)
			break;
		var pos2 = RECORD.index("*/");
		if (not pos2) {
			pos2 = RECORD.len() + 1;
		}
		RECORD.paster(pos1, pos2 - pos1 + 2, "");
	}//loop;
	// call msg('x')
	// remove comment lines
	let nfields = RECORD.fcount(FM);

	// dim x(nfields)
	// matparse @record into x
	for (const var ii : range(1, nfields)) {
	// if trim(x(i))[1,1]='*' then x(i)=''
		if (RECORD.f(ii).trim().starts("*")) {
			RECORD(ii) = "";
		}
	} //ii;
	// @record=matunparse(x)
	//             call msg('y')
	// remove blank lines
	RECORD.converter(_FM " ", " " _FM);

	var nfields2 = RECORD.trim().fcount(" ");
	return nfields2;
}
libraryexit(nfields00)


libraryinit(nlines)
//-----------------
function main() {
	ANS = RECORD.fcount(FM);
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
	return RECORD.f(1).seq();
}
libraryexit(objectcodetype)


libraryinit(program_date)
//-----------------------
function main() {
	var ans = calculate("TIMEDATE");
	return ans.cut(10).iconv("D");
}
libraryexit(program_date)


libraryinit(recnum)
//-----------------
function main() {
	// @reccount is the same thing
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
	return RECORD.len() + ID.len() + 5;
}
libraryexit(size)


libraryinit(spacer)
//-----------------
function main() {
	return "";
}
libraryexit(spacer)


libraryinit(sql_text)
//-------------------
function main() {
	return RECORD.f(8).field("/" "*pgsql", 2).field("*" "/", 1);
}
libraryexit(sql_text)


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
	if (ans.starts("V")) {
		ans = field2(RECORD, FM, -2);
	}
	if (not(ans.first(2).match("^\\d{2}$"))) {
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
	return calculate("USERNAME_UPDATED").f(1, 1);
}
libraryexit(username_created)


libraryinit(version)
//------------------
function main() {
	if (DICT.contains("VOC")) {
		ANS = xlate(RECORD.f(3), "$" ^ RECORD.f(4), "VERSION", "X");
	} else {
		ANS = field2(RECORD, FM, -1);
	// IF @ANS[1,1] = 'V' THEN
	//  @ANS=@ANS[2,99]
	// END ELSE
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
