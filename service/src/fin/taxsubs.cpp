#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <initcompany.h>
#include <singular.h>

#include <gen_common.h>
#include <fin_common.h>
#include <win_common.h>

#include <window.hpp>

var msg;
var op;
var op2;
var wspos;
var wsmsg;
var xx;

function main(in mode) {
	//c fin

	#include <general_common.h>
	#include <common.h>

	win.valid = 1;

	if (mode == "READ") {
		if (not(authorised("TAX RATE ACCESS", msg, ""))) {
			return invalid(msg);
		}
		if (not(RECORD.read(DEFINITIONS, "TAXES"))) {
			fin.taxes = "";
		}

	} else if (mode == "POSTREAD") {
		op = "TAX RATE";
		gosub security2(mode, op);
		if (not(win.valid)) {
			return 0;
		}

	} else if (mode == "PREWRITE") {
		if (not(authorised("TAX RATE UPDATE", msg, ""))) {
			return invalid(msg);
		}

		var nn = fin.taxes.a(4).count(VM) + 1;
		for (var ii = 1; ii <= nn; ++ii) {
			var acno = RECORD.a(4, ii);
			var iacno = acno.a(1, 1, 2);
			var oacno = acno.a(1, 1, 1);
			var iacc = "";
			var oacc = "";

			if (iacno) {
				if (not(iacc.read(fin.accounts, iacno))) {
					{}
				}
			}
			if (oacno) {
				if (not(oacc.read(fin.accounts, "." ^ oacno))) {
					{}
				}
			}

			//get iaccno if missing
			if (iacno == "") {
				acno.r(1, 1, 2, oacc.a(10));
			}
			//update oaccno
			if (iacc) {
				acno.r(1, 1, 1, iacc.a(10));
			}

			RECORD.r(4, ii, acno);

		};//ii;

	} else if (mode == "PREDELETE") {
		msg = "TAXES cannot be deleted";
		return invalid(msg);

	} else if (mode == "WRITE") {

		if (not(authorised("TAX RATE UPDATE", msg, ""))) {
			return invalid(msg);
		}

		RECORD.write(DEFINITIONS, "TAXES");
		call initcompany();

	} else if (mode == "POSTWRITE") {
		fin.taxes = RECORD;

	} else if (mode == "POSTDELETE") {

	}

	return 0;
}

libraryexit()
