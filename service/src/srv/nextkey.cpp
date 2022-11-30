#include <exodus/library.h>
libraryinit()

var previous;  // num

function main(in mode, in previous0 = "") {
	// jbase linemark
	if (previous0.unassigned()) {
		previous = 0;
	} else {
		previous = previous0;
	}

	var result = "";
	ANS		   = "";
	var keyx   = "";

	var actfilename = mode.field(":", 1);
	let seqkey		= mode.field(":", 2);
	let keyfilename = mode.field(":", 3);
	let prefix		= mode.field(":", 4);

	if (actfilename == "") {
		let temp = seqkey.index(".SK");
		if (temp) {
			actfilename = seqkey.first(temp - 1);
		} else {
			actfilename = keyfilename;
		}
	}

	var keyfile;
	if (not(keyfile.open(keyfilename, ""))) {
		call mssg(lasterror());
		return ANS;
	}

	var actfile;
	if (not(actfile.open(actfilename, ""))) {
		call mssg(lasterror());
		return ANS;
	}

	// lock the source of numbers
	var buffer = "";
lockit:
	if (not(lockrecord(keyfilename, keyfile, seqkey))) {
		var temp = keyfilename;
		if (temp.ends("S")) {
			temp.cutter(1);
		}
		if (not buffer) {
			call mssg("WAITING TO GET THE NEXT " ^ temp ^ " NUMBER|PRESS ESCAPE TO EXIT.", "UB", buffer, "");
		}
		if (esctoexit()) {
			call mssg("", "DB", buffer, "");
			return "";
		}
		call ossleep(1000 * 1);
		goto lockit;
	}
	if (buffer) {
		call mssg("", "DB", buffer, "");
	}

	// get the initial next number
	var nextno;
	if (not nextno.read(keyfile, seqkey)) {
		if (actfilename == "VOUCHERS") {
			// actually voucher numbers are not generated by nextkey (see acc.params<17>)
			nextno = 1;
		} else {
			// better starting point than 1 (more sortable and less confusing)
			nextno = 1000;
		}

		nextno.write(keyfile, seqkey);
	}
	nextno = nextno.f(1);

	if (previous) {
		nextno -= 1;
	}

	keyx = "";
	if (nextno != "") {
		result		   = nextno;
		var currnextno = result;
		var notok	   = 1;

checkrec:

		// make the keyx from RESULT and PREFIX
		keyx = result;
		if (prefix) {
			keyx = prefix;
			if (prefix.contains("%")) {
				keyx.replacer("%", result);
			} else {
				keyx ^= result;
			}
		}

		if (previous) {
			notok = 0;
		} else {
			var temp;
			if (temp.read(actfile, keyx)) {
				if (notok == 1) {
					currnextno += 1;
				}
			} else {
				if (notok == 1) {
					if (currnextno > nextno) {
						currnextno.writef(keyfile, seqkey, 1);
					}
					notok = 2;
				}
				if (lockrecord(actfilename, actfile, keyx)) {
					notok = 0;
				}
			}
		}

		if (notok) {
			result += 1;
			goto checkrec;
		}

		let xx = unlockrecord(actfilename, actfile, keyx);
	}

	let xx = unlockrecord(keyfilename, keyfile, seqkey);

	ANS = keyx;
	return keyx;
}

libraryexit()
