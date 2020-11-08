#include <exodus/library.h>
libraryinit()

#include <flushindex.h>
#include <singular.h>
#include <authorised.h>

#include <gen_common.h>
#include <win_common.h>

#include <window.hpp>

var op;
var op2;
var msg;
var wspos;
var wsmsg;
var xx;

function main(in mode) {
	//c gen

	#include <general_common.h>
	win.valid = 1;

	if (mode == "POSTINIT") {
		gosub security(mode);
		if (not(win.valid)) {
			return 0;
		}

	} else if (mode == "POSTREAD") {
		gosub security(mode);
		if (not(win.valid)) {
			return 0;
		}

	} else if (mode == "PREWRITE") {
		if (win.orec) {
			RECORD.r(22, var().date());
		}else{
			RECORD.r(21, var().date());
		}

	} else if ((mode == "POSTWRITE") or (mode == "POSTDELETE")) {
		call flushindex("ADDRESSES");

	} else if (mode == "PREDELETE") {
		gosub security(mode);
		if (not(win.valid)) {
			return 0;
		}

	}

	return 0;
}

libraryexit()
