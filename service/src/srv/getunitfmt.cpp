#include <exodus/library.h>
libraryinit()

#include <srv_common.h>

function main(in code, out fmtx) {
	// jbase

	if (not code.len()) {
		fmtx = "";
		return 0;
	}

	// Locate cached currfmt by currency code
	// or get the next free cache fmtn
	var fmtn;  // num
	if (srv.currency_codes.locate(code, fmtn)) {
		fmtx = srv.currency_convs.f(1, fmtn);
		return 0;
	}

	// Read currency ndecs from cache for speed
	var ndecs;
	if (var currency; currency.readc(srv.currencies, code)) {
		ndecs = currency.f(3);
	} else {
		ndecs = "";
	}

	// Default to base currency ndecs or 2
	if (not ndecs.len()) {
		ndecs = BASEFMT.at(3);
		if (not ndecs.match("^\\d$")) {
			ndecs = 2;
		}
	}

	// Format is plain numeric.
	// D = Decimal point = .
	// No commas for thousands.
	// 0P = No shifting of actual or implicit decimal point.
	// e.g. oconv(1234.567, "MD20P") -> "1234.56"
	fmtx = "MD" ^ ndecs ^ "0P";

	// Cache the currency format
	srv.currency_codes(1, fmtn) = code;
	srv.currency_convs(1, fmtn) = fmtx;

	return 0;
}

libraryexit()
