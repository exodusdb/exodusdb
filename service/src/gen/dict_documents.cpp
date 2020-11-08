#include <exodus/library.h>

libraryinit(command)
//------------------
function main() {
	return calculate("INSTRUCTIONS").field(" ", 1);
}
libraryexit(command)

libraryinit(created)
//------------------
function main() {
	return RECORD.a(3) ^ "." ^ RECORD.a(4).oconv("R(0)#5");
}
libraryexit(created)

libraryinit(instructions2)
//------------------------
function main() {
	ANS = RECORD.a(5);
	ANS.swapper(VM, "%FD");
	return ANS;
}
libraryexit(instructions2)

libraryinit(param1)
//-----------------
function main() {
	return RECORD.a(6, 1);
}
libraryexit(param1)

libraryinit(param10)
//------------------
function main() {
	return RECORD.a(6, 10);
}
libraryexit(param10)

libraryinit(param2)
//-----------------
function main() {
	return RECORD.a(6, 2);
}
libraryexit(param2)

libraryinit(param3)
//-----------------
function main() {
	return RECORD.a(6, 3);
}
libraryexit(param3)

libraryinit(param4)
//-----------------
function main() {
	return RECORD.a(6, 4);
}
libraryexit(param4)

libraryinit(param5)
//-----------------
function main() {
	return RECORD.a(6, 5);
}
libraryexit(param5)

libraryinit(param6)
//-----------------
function main() {
	return RECORD.a(6, 6);
}
libraryexit(param6)

libraryinit(param7)
//-----------------
function main() {
	return RECORD.a(6, 7);
}
libraryexit(param7)

libraryinit(param8)
//-----------------
function main() {
	return RECORD.a(6, 8);
}
libraryexit(param8)

libraryinit(param9)
//-----------------
function main() {
	return RECORD.a(6, 9);
}
libraryexit(param9)

libraryinit(params)
//-----------------

function main() {
	ANS = raise(RECORD.a(6));
	var nfs = ANS.count(FM) + 1;
	for (var fn = nfs; fn >= 1; --fn) {
		var tt = ANS.a(fn);
		if (tt.length()) {
			ANS.r(fn, fn ^ "=" ^ tt);
		}
	};//fn;
	ANS = trim(ANS, FM);
	ANS.converter(" " _FM_ _VM_ _SM_ _TM_ _STM_, "~ :;,^");
	return ANS;
}
libraryexit(params)

libraryinit(program)
//------------------
var programn;

function main() {
	var program = RECORD.a(5).field(" ", 1);
	if (var("ANAL,ANALSCH,BALANCES").locateusing(",",program,programn)) {
		program = var("Billing Analysis,Media Analysis,Financial Statements").field(",", programn);
	}
	return program;
}
libraryexit(program)

libraryinit(program2)
//-------------------
function main() {
	return RECORD.a(5).field(VM, 2, 9999);
}
libraryexit(program2)

libraryinit(recipients)
//---------------------
function main() {
	ANS = RECORD.a(14);
	ANS.swapper(VM, ", ");
	return ANS;
}
libraryexit(recipients)

libraryinit(running_now)
//----------------------
var ans;//num

function main() {
	ANS = "";
	var documents;
	if (documents.open("DOCUMENTS", "")) {
		if (documents.lock( ID)) {
			documents.unlock( ID);
			ans = "";
		}else{
			ans = 1;
		}
		ANS = ans;
	}
	return ANS;
}
libraryexit(running_now)
