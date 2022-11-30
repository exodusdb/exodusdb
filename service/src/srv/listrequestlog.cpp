#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <quote2.h>

#include <service_common.h>

#include <srv_common.h>

var msg;

function main() {

	if (not authorised("REQUESTLOG ACCESS", msg)) {
		call mssg(msg);
		stop();
	}

	let ifromdate  = PSEUDO.f(5);
	var iuptodate  = PSEUDO.f(6);
	let fromtime   = PSEUDO.f(7);
	var uptotime   = PSEUDO.f(8);
	let containing = PSEUDO.f(9);
	let duration   = PSEUDO.f(10);
	let usercodes  = PSEUDO.f(11);
	let relocks	   = PSEUDO.f(12);

	var cmd = "SORT REQUESTLOG BY SEQ";

	var title = "Request Log   'T'  Page 'PL'";

	if (not iuptodate) {
		iuptodate = ifromdate;
	}
	if (ifromdate) {
		if (iuptodate == ifromdate) {
			cmd ^= " %AND% WITH DATE " ^ (ifromdate.oconv("D4").quote());
			title ^= "'L'Filter : " ^ oconv(ifromdate, "[DATE,4*]");
		} else {
			cmd ^= " %AND% WITH DATE BETWEEN " ^ (ifromdate.oconv("D4").quote()) ^ " AND " ^ (iuptodate.oconv("D4").quote());
			title ^= "'L'Filter : " ^ oconv(ifromdate, "[DATE,4*]") ^ " - " ^ oconv(iuptodate, "[DATE,4*]");
		}
	}

	// UI restricted to HH:MM not HH:MM:SS
	if (not uptotime) {
		uptotime = fromtime;
	}
	if (fromtime) {

		// allow 12:00:59.99 to match UPTO 12:00
		uptotime += 60;

		cmd ^= " %AND% WITH TIME BETWEEN " ^ (fromtime.oconv("MT").quote()) ^ " AND " ^ (uptotime.oconv("MT").quote());
		title ^= "'L'Filter : " ^ fromtime.oconv("MT") ^ " - " ^ uptotime.oconv("MT");
	}

	if (not relocks) {
		cmd ^= " %AND% WITH REQUEST1 NE \"RELOCK\"";
	}

	if (containing) {

		var tt = containing.ucase();
		if (tt.contains(DQ)) {
			if (tt.contains("'")) {
				call mssg("Cannot find text containing single AND double quotes");
				stop();
			} else {
				tt = "'" ^ tt ^ "'";
			}
		} else {
			tt.quoter();
		}

		cmd ^= " %AND% WITH ALLUPPERCASE CONTAINING " ^ tt;
	}

	if (duration) {
		cmd ^= " %AND% WITH SECS GE " ^ duration;
	}

	if (usercodes) {
		cmd ^= " %AND% WITH USERCODE " ^ quote2(usercodes);
	}

	let tt = cmd.index("%AND% ");
	if (tt) {
		cmd.paster(tt, 6, "");
	}
	cmd.replacer("%AND% ", "AND ");

	// cmd:=' HEADING ':quote(title)
	title = cmd.oconv("T#60");
	title.replacer(TM, " 'L' ");
	title.replacer(DQ, "''");
	cmd ^= " HEADING " ^ (title.quote());

	perform(cmd);

	return 0;
}

libraryexit()
