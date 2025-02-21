#ifndef EXO_REQUEST_HPP_
#define EXO_REQUEST_HPP_

#include <singular.h>

function security(in modex)
{
	var op = singular(req.datafile);
	return security2(modex,op);
}

function security2(in modex, in op0)
{
	var op=op0.convert("_.", "  ");
	var op2 = "";
	if (op.ends(DQ)) {
		op2=op.field(DQ, -2);
		op.cutter(-op2.len());
		op2.prefixer(" ");
	}
	if (modex.contains("INIT")) {
		var msgx;
		if (!(authorised(op ^ " ACCESS" ^ op2, msgx, "")))
			return invalid(msgx);

	}else if (modex.contains("READ") || modex.contains("WRITE")) {
		if (!req.wlocked) {
			op ^= " ACCESS";
		} else {
			if (req.orec) {
				op ^= " UPDATE";
			} else {
				op ^= " CREATE";
			}
		}
		var msgx;
		if (!(authorised(op ^ op2, msgx, ""))) {
			if (req.orec == "" || !req.wlocked) {
				invalid();
				req.reset = 5;
			}
			if (req.wlocked) {
				unlockrecord(req.datafile, req.srcfile, ID);
				req.wlocked = 0;
			}
		}

	}else if (modex.contains("DELETE")) {
		var msgx;
		if (!(authorised(op ^ " DELETE" ^ op2, msgx, "")))
			return invalid(msgx);

	}else if (1) {
		return security3(op,op2);
	}
	return true;
}

function security3(in op, in op2)
{
	var op2b=op2.assigned()?op2:"";
	var msgx;
	if (!(authorised(op ^ op2b, msgx, "")))
		return invalid(msgx);

	return true;
}

function invalidq(in msgx)
{
	return invalid(req.is.f(1, 1, 1).quote() ^ " " ^ msgx);
}

function invalid()
{
	return invalid("");
}

function invalid(in msgx)
{
	req.valid = 0;
	if (!req.isorig.unassigned()) {
		req.is = req.isorig;
		if (!req.reset)
			req.reset = 1;
	}
	if (!msgx) return true;
	note(msgx);
	return 0;
}

function invalid2(in msgx) {
	if (not req.registerx[3]) {
		invalid(msgx);
	}
	if (req.isorig) {
		req.is = req.isorig;
	}
	req.valid = 0;
	req.reset = 1;
	return 0;
}

function note3(in msgx) {
	if (not msgx) {
		return 0;
	}
	if (req.registerx[3]) {
		return 0;
 	}
	note(msgx);
	return 0;
}

function badchars(io msgx) {

	msgx = "";
	if (not req.is) {
		return 0;
	}
	var tt = req.is;
	var bad = "\'" "~!@#$%^&*()_+|\\{}[]:\";,?";
	tt.converter(bad, RM.str(bad.len()));
	if (tt ne req.is) {
		var t2 = req.is;
		t2.converter(tt, "");
		msgx = "SORRY, YOU CANNOT USE|" ^ t2 ^ " CHARACTERS IN KEY FIELDS";
		gosub invalid(msgx);
	}
	return 0;
}

#endif // EXO_REQUEST_HPP_
