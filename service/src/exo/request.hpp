#ifndef EXO_REQUEST_HPP_
#define EXO_REQUEST_HPP_

#include <singular.h>

function security(in mode)
{
	var op = singular(req.datafile);
	return security2(mode,op);
}

function security2(in mode, in op0)
{
	var op=op0.convert("_.", "  ");
	var op2 = "";
	if (op.ends(DQ)) {
		op2=op.field2(DQ, -2);
		op.cutter(-op2.len());
		op2.prefixer(" ");
	}
	if (mode.contains("INIT")) {
		var msg;
		if (!(authorised(op ^ " ACCESS" ^ op2, msg, "")))
			return invalid(msg);

	}else if (mode.contains("READ") || mode.contains("WRITE")) {
		if (!req.wlocked) {
			op ^= " ACCESS";
		} else {
			if (req.orec) {
				op ^= " UPDATE";
			} else {
				op ^= " CREATE";
			}
		}
		var msg;
		if (!(authorised(op ^ op2, msg, ""))) {
			if (req.orec == "" || !req.wlocked) {
				invalid();
				req.reset = 5;
			}
			if (req.wlocked) {
				unlockrecord(req.datafile, req.srcfile, ID);
				req.wlocked = 0;
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
	return invalid(req.is.f(1, 1, 1).quote() ^ " " ^ msg);
}

function invalid()
{
	return invalid("");
}

function invalid(in msg)
{
	req.valid = 0;
	if (!req.isorig.unassigned()) {
		req.is = req.isorig;
		if (!req.reset)
			req.reset = 1;
	}
	if (!msg) return true;
	note(msg);
	return 0;
}

function invalid2(in msg) {
	if (not req.registerx(3)) {
		invalid(msg);
	}
	if (req.isorig) {
		req.is = req.isorig;
	}
	req.valid = 0;
	req.reset = 1;
	return 0;
}

function note3(in msg) {
	if (not msg) {
		return 0;
	}
	if (req.registerx(3)) {
		return 0;
 	}
	note(msg);
	return 0;
}

function badchars(io msg) {

	msg = "";
	if (not req.is) {
		return 0;
	}
	var tt = req.is;
	var bad = "\'" "~!@#$%^&*()_+|\\{}[]:\";,?";
	tt.converter(bad, RM.str(bad.len()));
	if (tt ne req.is) {
		var t2 = req.is;
		t2.converter(tt, "");
		msg = "SORRY, YOU CANNOT USE|" ^ t2 ^ " CHARACTERS IN KEY FIELDS";
		gosub invalid(msg);
	}
	return 0;
}

#endif // EXO_REQUEST_HPP_
