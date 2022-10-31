#include <exodus/library.h>
libraryinit()

#include <service_common.h>

var inx;  // num

function main(in type, in in0, in /*mode*/, out outx) {

	// CALLED
	// /////
	if (in0.unassigned()) {

		inx = type;

oconvx:
		// /////
		// nothing in - no conv
		if (inx.len() eq 0) {
noconv:
			outx = inx;
			return outx;
		}

		// non-numeric input - no conv
		if (not(inx.isnum())) {
			goto noconv;
		}

		// negative years - backwards from 1999
		// -1    1999
		// -2    1998
		// -3    1997
		if (inx.isnum() and inx le 0) {
			inx += 2000;
		}

		// not pure digits - no conv
		if (not(inx.match("^\\d*$"))) {
			goto noconv;
		}

		// four digit year already - no conv
		if (inx.len() eq 4) {
			goto noconv;
		}

		// two digit year expected
		if (inx.len() eq 2) {
			// ok

			// pad to two digits 1 -> 01 -> 2001
		} else if (inx.len() eq 1) {
			inx = "0" ^ inx;

			// more than four digit - no conv
		} else if (inx.len() gt 4) {
			goto noconv;

			// three digits - use right most two digits
		} else {
			inx.laster(2);
		}

		// 00-49 -> 2000 ... 2049
		// 50-99 -> 2050 ... 2099
		if (inx le centuryyear_) {
			outx = "20" ^ inx;
		} else {
			outx = "19" ^ inx;
		}

		return outx;

		// OCONV
		// /////
	} else if (type eq "OCONV") {
		inx = in0;
		goto oconvx;
	}

	// ICONV
	// ////

	// nothing in - no conv
	if (in0.len() eq 0) {
		outx = in0;
		return outx;
	}

	// return right hand two chars (presumably digits)
	// pad to at least two zeros
	if (in0.len() lt 2) {
		outx = ("00" ^ in0).last(2);
	} else {
		outx = in0.last(2);
	}

	return outx;

	// ""      ""
	// 2050     2050
	// 50      1950
	// 49      2049
	// 2010    2010
	// 2000    2000
	// 21      2021
	// 20      2020
	// 19      2019
	// 10      2010
	// 2       2002
	// 1       2001
	// 0       2000
	// -1      1999
	// -2      1998
}

libraryexit()
