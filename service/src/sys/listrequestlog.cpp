#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <quote2.h>

#include <sys_common.h>

var msg;

function main() {
	//c sys

	#include <system_common.h>

	if (not(authorised("REQUESTLOG ACCESS", msg))) {
		call mssg(msg);
		stop();
	}

	var ifromdate = PSEUDO.a(5);
	var iuptodate = PSEUDO.a(6);
	var fromtime = PSEUDO.a(7);
	var uptotime = PSEUDO.a(8);
	var containing = PSEUDO.a(9);
	var duration = PSEUDO.a(10);
	var usercodes = PSEUDO.a(11);

	var cmd = "SORT REQUESTLOG BY SEQ";

	var title = "Request Log   'T'  Page 'PL'";

	if (not iuptodate) {
		iuptodate = ifromdate;
	}
	if (ifromdate) {
		if (iuptodate eq ifromdate) {
			cmd ^= " %AND% WITH DATE " ^ (ifromdate.oconv("D4").quote());
			title ^= "'L'Filter : " ^ oconv(ifromdate, "[DATE,4*]");
		} else {
			cmd ^= " %AND% WITH DATE BETWEEN " ^ (ifromdate.oconv("D4").quote()) ^ " AND " ^ (iuptodate.oconv("D4").quote());
			title ^= "'L'Filter : " ^ oconv(ifromdate, "[DATE,4*]") ^ " - " ^ oconv(iuptodate, "[DATE,4*]");
		}
	}

	//UI restricted to HH:MM not HH:MM:SS
	if (not uptotime) {
		uptotime = fromtime;
	}
	if (fromtime) {

		//allow 12:00:59.99 to match UPTO 12:00
		uptotime += 60;

		cmd ^= " %AND% WITH TIME BETWEEN " ^ (fromtime.oconv("MT").quote()) ^ " AND " ^ (uptotime.oconv("MT").quote());
		title ^= "'L'Filter : " ^ fromtime.oconv("MT") ^ " - " ^ uptotime.oconv("MT");
	}

	cmd ^= " %AND% WITH REQUEST1 NE \"RELOCK\"";

	if (containing) {

		var tt = containing.ucase();
		if (tt.index(DQ)) {
			if (tt.index("'")) {
				call mssg("Cannot find text containing single AND double quotes");
				stop();
			} else {
				tt = "'" ^ tt ^ "'";
			}
		} else {
			tt = tt.quote();
		}

		cmd ^= " %AND% WITH ALLUPPERCASE CONTAINING " ^ tt;
	}

	if (duration) {
		cmd ^= " %AND% WITH SECS GE " ^ duration;
	}

	if (usercodes) {
		cmd ^= " %AND% WITH USERCODE " ^ quote2(usercodes);
	}

	var tt = cmd.index("%AND% ");
	if (tt) {
		cmd.splicer(tt, 6, "");
	}
	cmd.swapper("%AND% ", "AND ");

	//cmd:=' HEADING ':quote(title)
	title = cmd.oconv("T#60");
	title.swapper(TM, " 'L' ");
	title.swapper(DQ, "''");
	cmd ^= " HEADING " ^ (title.quote());

	perform(cmd);

	return 0;
}

libraryexit()
