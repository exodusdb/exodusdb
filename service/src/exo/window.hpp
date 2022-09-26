#ifndef EXO_WINDOW_HPP_
#define EXO_WINDOW_HPP_

//#include <authorised.h>

function security(in mode)
{
	/////////
	//valid=1 move down
	var op = singular(win.datafile);
	//if op[-3,3]='ies' then op[-3,3]='y'
	//if op-1,1]='s' then op[-1,1]=''
	//op:=' file'
	return security2(mode,op);
}

function security2(in mode, in op0)
{
//	win.valid = 1;
	var op=op0.convert("_.", "  ");
	var op2 = "";
	if (op.ends(DQ)) {
//		op2 = (op.b(-2, "B\"")).quote();
		op2=op.field2(DQ, -2);
		op.cutter(-op2.len());
		op2.prefixer(" ");
	}
	if (mode.contains("INIT")) {
		var msg;
		if (!(authorised(op ^ " ACCESS" ^ op2, msg, "")))
			return invalid(msg);

	}else if (mode.contains("READ") || mode.contains("WRITE")) {
		if (!win.wlocked) {
			op ^= " ACCESS";
		} else {
			if (win.orec) {
				op ^= " UPDATE";
			} else {
				op ^= " CREATE";
			}
		}
		var msg;
		if (!(authorised(op ^ op2, msg, ""))) {
			if (win.orec == "" || !win.wlocked) {
				invalid();
				win.reset = 5;
			}
			if (win.wlocked) {
				unlockrecord(win.datafile, win.srcfile, ID);
				win.wlocked = 0;
			}
		}

	}else if (mode.contains("DELETE")) {
		var msg;
		if (!(authorised(op ^ " DELETE" ^ op2, msg, "")))
			return invalid(msg);

	}else if (1) {
		return security3(op,op2);
	}
	return true;
}

function security3(in op, in op2)
{
	var op2b=op2.assigned()?op2:"";
	var msg;
	if (!(authorised(op ^ op2b, msg, "")))
		return invalid(msg);

	return true;
}

function invalidq(in msg)
{
	return invalid(win.is.f(1, 1, 1).quote() ^ " " ^ msg);
}

function invalid()
{
	return invalid("");
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

function badchars(io msg) {
	//var msg = "";
	msg = "";
	if (not win.is) {
		return 0;
	}
	var tt = win.is;
	var bad = "\'" "~!@#$%^&*()_+|\\{}[]:\";,?";
	tt.converter(bad, RM.str(bad.len()));
	if (tt ne win.is) {
		var t2 = win.is;
		t2.converter(tt, "");
		msg = "SORRY, YOU CANNOT USE|" ^ t2 ^ " CHARACTERS IN KEY FIELDS";
		gosub invalid(msg);
	}
	return 0;
}

#endif // EXO_WINDOW_HPP_
