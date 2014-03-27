#include <authorised.h>

function security(in mode)
{
	/////////
	//valid=1 move down
	var op = mv.singular(win.datafile);
	//if op[-3,3]='ies' then op[-3,3]='y'
	//if op-1,1]='s' then op[-1,1]=''
	//op:=' file'
	return security2(mode,op);
}

function security2(in mode, in op0)
{
//	win.valid = 1;
	var op=op0.convert(L"_.", L"  ");
	var op2 = L"";
	if (op[-1] == DQ) {
//		op2 = (op.substr(-2, L"B\"")).quote();
		op2=op.field2(DQ,-2);
		op.splicer(-op2.length(), op2.length(), L"");
		op2.splicer(1, 0, L" ");
	}
	if (mode.index(L"INIT", 1)) {
		var msg;
		if (!(authorised(op ^ L" ACCESS" ^ op2, msg, L"")))
			return invalid(msg);

	}else if (mode.index(L"READ", 1) || mode.index(L"WRITE", 1)) {
		if (!win.wlocked) {
			op ^= L" ACCESS";
		}else{
			if (win.orec) {
				op ^= L" UPDATE";
			}else{
				op ^= L" CREATE";
			}
		}
		var msg;
		if (!(authorised(op ^ op2, msg, L""))) {
			if (win.orec == L"" || !win.wlocked) {
				invalid();
				win.reset = 5;
			}
			if (win.wlocked) {
				unlockrecord(win.datafile, win.srcfile, ID);
				win.wlocked = 0;
			}
		}

	}else if (mode.index(L"DELETE", 1)) {
		var msg;
		if (!(authorised(op ^ L" DELETE" ^ op2, msg, L"")))
			return invalid(msg);

	}else if (1) {
		return security3(op,op2);
	}
	return true;
}

function security3(in op, in op2)
{
	var op2b=op2.assigned()?op2:L"";
	var msg;
	if (!(authorised(op ^ op2b, msg, L"")))
		return invalid(msg);

	return true;
}

function invalidq(in msg)
{
	return invalid(msg.splice(1, 0, (win.is.a(1, 1, 1)).quote() ^ L" "));
}

function invalid()
{
	return invalid(L"");
}

function invalid(in msg)
{
	win.valid = 0;
	//if win.is.orig then win.is=win.is.orig
	if (!win.isorig.unassigned()) {
		win.is = win.isorig;
		if (!win.reset)
			win.reset = 1;
	}
	if (!msg) return true;
	note(msg);
	return 0;
}

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

