#include <exodus/library.h>

libraryinit(company_code)
//-----------------------
function main() {
	//currencies currently dont belong to specific companies
	return "";
}
libraryexit(company_code)

libraryinit(conversion)
//---------------------
var vn;

function main() {
	//COMMON /EXCHANGE/ EXCHANGE.DATE
	#define exchangedate RECUR0

	if (not(RECORD.a(4).locateby("DR",RECUR0,vn))) {
		{}
	}
	var ans = RECORD.a(5, vn);
	if (ans) {
		ANS = 1 / ans;
		ANS = ANS.oconv("MD60P");
	}else{
		ANS = "";
	}
	return ANS;
}
libraryexit(conversion)

libraryinit(conversion2)
//----------------------
function main() {
	var rates = calculate("EXCHANGE_RATE");
	ANS = "";
	var nrates = rates.count(VM) + (rates ne "");
	for (var raten = 1; raten <= nrates; ++raten) {
		var rate = rates.a(1, raten);
		if (rate) {
			ANS.r(1, raten, (1 / rate).oconv("MD90P"));
		}
	};//raten;
	return ANS;
}
libraryexit(conversion2)

libraryinit(currency_code_symbolic)
//---------------------------------
function main() {
	return ID;
}
libraryexit(currency_code_symbolic)

libraryinit(current_date)
//-----------------------
var vn;

function main() {
	if (RECORD.a(4).locateby("DR",var().date(),vn)) {
		{}
	}
	return RECORD.a(4, vn);
}
libraryexit(current_date)

libraryinit(current_rate)
//-----------------------
var vn;

function main() {
	if (not(RECORD.a(4).locateby("DR",var().date(),vn))) {
		{}
	}
	return RECORD.a(5, vn).substr(1,10);
}
libraryexit(current_rate)

libraryinit(iso_code2)
//--------------------
function main() {
	ANS = RECORD.a(9);
	if (ANS == "") {
		ANS = ID;
	}
	return ANS;
}
libraryexit(iso_code2)

libraryinit(ndecs)
//----------------
function main() {
	return "";
}
libraryexit(ndecs)

libraryinit(rawname)
//------------------
function main() {
	ANS = RECORD.a(1);
	ANS.ucaser();
	ANS.converter("\'" " \".,/-", "");
	if (ANS[-1] == "S") {
		ANS.splicer(-1, 1, "");
	}
	return ANS;
}
libraryexit(rawname)

libraryinit(relevent_date)
//------------------------
var vn;

function main() {
	//COMMON /EXCHANGE/ EXCHANGE.DATE
	#define exchangedate RECUR0

	if (not(RECORD.a(4).locateby("DR",RECUR0,vn))) {
		{}
	}
	return RECORD.a(4, vn);
}
libraryexit(relevent_date)

libraryinit(relevent_rate)
//------------------------
var vn;

function main() {
	//COMMON /EXCHANGE/ EXCHANGE.DATE
	#define exchangedate RECUR0

	if (not(RECORD.a(4).locateby("DR",RECUR0,vn))) {
		{}
	}
	return RECORD.a(5, vn).substr(1,15);
}
libraryexit(relevent_rate)

libraryinit(stopped2)
//-------------------

function main() {
	var tt = ((RECORD.a(1) ^ RECORD.a(25)).ucase()).convert("<>", "()");
	ANS = RECORD.a(25) or tt.index("(STOP)");
	/*pgsql;
	DECLARE;
		tt text;;
		--tt2 text;;
	BEGIN;
		tt:= translate(;
																	--lower( split_part(data,FM,1)||split_part(data,FM,25) );
																	upper(split_part(data,FM,1)||split_part(data,FM,25));
																	,'<>' ,'()');;
		--tt2:=position('(STOP)' in tt);;
		--if tt2::bool then;
		-- ans:=tt2;;
		--else;
		-- ans:=split_part(data,FM,25);;
		--end if;;
		if split_part(data,FM,25)<>'' or position('(STOP)' in tt)<>0 then;
			ans=1;;
		else;
			ans='';;
		end if;;
	END;;
	*/
	return ANS;
}
libraryexit(stopped2)

libraryinit(to_1)
//---------------
#include <fin_common.h>

function main() {
	#include <common.h>
	ANS = fin.basecurrency;
	if (not ANS) {
		ANS = "base currency unit";
	}
	return ANS;
}
libraryexit(to_1)

libraryinit(year_period)
//----------------------
function main() {
	return "";
}
libraryexit(year_period)
