#include <exodus/library.h>
libraryinit()

#include <esctoexit.h>
#include <delay.h>

function main() {
	//jbase linemark
	if (previous.unassigned()) {
		previous = 0;
	}

	var result = "";
	var keyx = "";

	var actfilename = mode.field(":", 1);
	var seqkey = mode.field(":", 2);
	var keyfilename = mode.field(":", 3);
	var prefix = mode.field(":", 4);

	if (actfilename == "") {
		var temp = seqkey.index(".SK", 1);
		if (temp) {
			actfilename = seqkey.substr(1, temp - 1);
		}else{
			actfilename = keyfilename;
		}
	}

	var keyfile;
	if (not(keyfile.open(keyfilename, ""))) {
		call fsmsg();
		goto exit;
	}

	var actfile;
	if (not(actfile.open(actfilename, ""))) {
		call fsmsg();
		goto exit;
	}

	//lock the source of numbers
	var buffer = "";
lockit:
	if (not(lockrecord("", keyfile, seqkey))) {
		var temp = keyfilename;
		if (temp.substr(-1, 1) == "S") {
			temp.splicer(1, 1, "");
		}
		if (not buffer) {
			call msg2("WAITING TO GET THE NEXT " ^ temp ^ " NUMBER|PRESS ESCAPE TO EXIT.", "UB", buffer, "");
		}
		if (esctoexit()) {
			call msg2("", "DB", buffer, "");
			return "";
		}
		call delay(1);
		goto lockit;
	}
	if (buffer) {
		call msg2("", "DB", buffer, "");
	}

	//get the initial next number
	var nextno;
	if (not(nextno.read(keyfile, seqkey))) {
		nextno = 1;
		var tt = seqkey;
		tt.swapper(".SK", "");
		tt.swapper(".", " ");
		if (seqkey.substr(1, 4) == "JOBS") {
inpnextno:
			call note2("PLEASE ENTER THE|FIRST NUMBER FOR " ^ tt, "RC", nextno, "");
			if (nextno == "") {
				var xx = unlockrecord("", keyfile, seqkey);
				ANS = "";
				return "";
			}
			if (not nextno.isnum()) {
				goto inpnextno;
			}
		}
		if (nextno == "") {
			nextno = 1;
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
			if (prefix.index("%", 1)) {
				keyx.swapper("%", result);
			}else{
				keyx ^= result;
			}
		}

		if (previous) {
			notok = 0;
		}else{
			var temp;
			if (temp.read(actfile, keyx)) {
				if (notok == 1) {
					currnextno += 1;
				}
			}else{
				if (notok == 1) {
					if (currnextno > nextno) {
						currnextno.writev(keyfile, seqkey, 1);
					}
					notok = 2;
				}
				if (lockrecord("", actfile, keyx)) {
					notok = 0;
				}
			}
		}

		if (notok) {
			result += 1;
			goto checkrec;
		}

		var xx = unlockrecord("", actfile, keyx);
	}

	var xx = unlockrecord("", keyfile, seqkey);

exit:
	ANS = keyx;
	return keyx;

}


libraryexit()