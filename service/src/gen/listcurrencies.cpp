#include <exodus/library.h>
libraryinit()

#include <authorised.h>

#include <gen_common.h>

var msg;

function main() {
	//c gen

	#include <general_common.h>

	var interactive = not(SYSTEM.a(33));

	if (not(authorised("CURRENCY ACCESS", msg, ""))) {
		var().chr(7).output();
		call mssg(msg);
		stop();
	}

	//COMMON /EXCHANGE/ EXCHANGE.DATE
	#define exchangedate RECUR0
	var temp = SENTENCE.field(" ", 2);
	if (temp == "") {
		temp = var().date().oconv("D");
	}
	RECUR0 = iconv(temp, "[DATE]");
	if (not RECUR0) {
		call mssg(temp.quote() ^ " cannot be understood as a date");
		stop();
	}

	var cmd = "NLIST CURRENCIES";
	cmd ^= " CURRENCY_NAME SUBUNIT_NAME ISO_CODE CURRENCY_CODE COLHEAD \"Code\" RELEVENT_RATE CONVERSION RELEVENT_DATE";
	cmd ^= " WITH STOPPED EQ \"\"";
	cmd ^= " ID-SUPP DBL-SPC";
	var head = "List of Currencies & Exchange Rates \'S10\' Page \'PL\'";
	head ^= var(2).space() ^ "(Effective " ^ oconv(RECUR0, "[DATE,4]") ^ ")";
	cmd ^= " HEADING " ^ (head.quote());

	perform(cmd);
	return 0;
}

libraryexit()
