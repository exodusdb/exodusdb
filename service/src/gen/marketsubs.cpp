#include <exodus/library.h>
libraryinit()

#include <sortarray.h>
#include <generalsubs2.h>
#include <authorised.h>
#include <singular.h>

#include <gen_common.h>
#include <agy_common.h>
#include <win_common.h>

#include <window.hpp>

var msg;
var op;
var xx;
var temp;
var op2;
var wspos;
var wsmsg;

function main(in mode) {
	//c gen

	#include <general_common.h>
	#include <agency_common.h>
	#define interactive not(SYSTEM.a(33))

	//jbase
	if (mode == "PREWRITE") {

		//sort holidays in reverse order
		call sortarray(RECORD, 7 ^ VM ^ 8, "DR");

		//update version log
		call generalsubs2(mode);

	} else if (mode == "POSTWRITE") {
	} else if (mode == "POSTDELETE") {

	} else if ((mode == "POSTINIT") or (mode == "POSTREAD")) {

		//option to read previous versions
		call generalsubs2(mode);
		if (not(win.valid)) {
			return 0;
		}

		//GOSUB SECURITY
		if (not(win.valid)) {
			return 0;
		}
		//this is done to allow LISTEN to avoid special MARKET ACCESS PARTIAL logic
		var pass = 0;
		if (((mode == "POSTREAD") and not(interactive)) and win.orec) {
			//check allowed to access this ledger
			if (authorised("#MARKET ACCESS " ^ (ID.quote()), msg, "")) {
				pass = 1;
			}
		}

		//OP='COMPANY FILE'
		//GOSUB SECURITY2
		if (not pass) {
			gosub security(mode);
		}
		if (not(win.valid)) {
			return 0;
		}

	} else if (mode == "PREDELETE") {

		op = "MARKET";
		gosub security2(mode, op);
		if (not(win.valid)) {
			return 0;
		}

		var filenamesx = "CLIENTS,SUPPLIERS,COMPANIES,CURRENCIES,VEHICLES,JOBS";
		var nfiles = filenamesx.count(",") + 1;
		for (var filen = 1; filen <= nfiles; ++filen) {

			//prevent deletion if any records
			var filename = filenamesx.field(",", filen);
			if (xx.open(filename, "")) {
				perform("SELECT 1 " ^ filename ^ " WITH MARKET_CODE " ^ (ID.quote()) ^ " (S)");
				if (LISTACTIVE) {
					if (not(readnext(temp))) {
						temp = "";
					}
					clearselect();
					var keyfield = singular(filename).lcase();
					if (keyfield == "job") {
						keyfield ^= " no.";
					}else{
						keyfield ^= " code";
					}
					msg = "This market has been used for " ^ keyfield ^ " " ^ (temp.quote()) ^ " and maybe others.|It cannot be deleted unless they are deleted first.";
					return invalid(msg);
				}
			}

		};//filen;

		//update version log
		call generalsubs2(mode);

	} else {
		msg = mode.quote() ^ " - unknown mode skipped in MARKET.SUBS";
		return invalid(msg);

	}

	return 0;
}

libraryexit()
