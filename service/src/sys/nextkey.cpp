#include <exodus/library.h>
libraryinit()

var previous;//num

function main(in mode, in previous0="") {
	//c sys in,""
	//jbase linemark
	if (previous0.unassigned()) {
		previous = 0;
	} else {
		previous = previous0;
	}

	var result = "";
	ANS = "";
	var keyx = "";

	var actfilename = mode.field(":", 1);
	var seqkey = mode.field(":", 2);
	var keyfilename = mode.field(":", 3);
	var prefix = mode.field(":", 4);

	if (actfilename eq "") {
		var temp = seqkey.index(".SK");
		if (temp) {
			actfilename = seqkey.substr(1, temp - 1);
		} else {
			actfilename = keyfilename;
		}
	}

	var keyfile;
	if (not(keyfile.open(keyfilename, ""))) {
		call fsmsg();
		return ANS;
	}

	var actfile;
	if (not(actfile.open(actfilename, ""))) {
		call fsmsg();
		return ANS;
	}

	//lock the source of numbers
	var buffer = "";
lockit:
	if (not(lockrecord(keyfilename, keyfile, seqkey))) {
		var temp = keyfilename;
		if (temp[-1] eq "S") {
			temp.splicer(1, 1, "");
		}
		if (not buffer) {
			call mssg("WAITING TO GET THE NEXT " ^ temp ^ " NUMBER|PRESS ESCAPE TO EXIT.", "UB", buffer, "");
		}
		if (esctoexit()) {
			call mssg("", "DB", buffer, "");
			return "";
		}
		call ossleep(1000*1);
		goto lockit;
	}
	if (buffer) {
		call mssg("", "DB", buffer, "");
	}

	//get the initial next number
	var nextno;
	if (not(nextno.read(keyfile, seqkey))) {
		if (actfilename eq "VOUCHERS") {
			//actually voucher numbers are not generated by nextkey (see acc.params<17>)
			nextno = 1;
		} else {
			//better starting point than 1 (more sortable and less confusing)
			nextno = 1000;
		}

		nextno.write(keyfile, seqkey);
	}
	nextno = nextno.a(1);

	if (previous) {
		nextno -= 1;
	}

	keyx = "";
	if (nextno ne "") {
		result = nextno;
		var currnextno = result;
		var notok = 1;

checkrec:

		//make the keyx from RESULT and PREFIX
		keyx = result;
		if (prefix) {
			keyx = prefix;
			if (prefix.index("%")) {
				keyx.swapper("%", result);
			} else {
				keyx ^= result;
			}
		}

		if (previous) {
			notok = 0;
		} else {
			var temp;
			if (temp.read(actfile, keyx)) {
				if (notok eq 1) {
					currnextno += 1;
				}
			} else {
				if (notok eq 1) {
					if (currnextno gt nextno) {
						currnextno.writev(keyfile, seqkey, 1);

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

		var xx = unlockrecord(actfilename, actfile, keyx);
	}

	var xx = unlockrecord(keyfilename, keyfile, seqkey);

	ANS = keyx;
	return keyx;
}

libraryexit()
