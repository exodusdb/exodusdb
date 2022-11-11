#include <exodus/library.h>
libraryinit()

#include <log.h>

#include <service_common.h>

#include <srv_common.h>

var reqdate;
var tt;

function main() {

	let update = 1;

	// keep approx nn Mb
	let maxsize = 20 * 1024 * 1024;

	// estimate based on size in last n days
	let perioddays = 7;

	var requestlog;
	if (not(requestlog.open("REQUESTLOG", ""))) {
		stop();
	}

	// stage1:
	// //////

	// calculate the size of the last month of requestlog entries

	let periodago = date() - perioddays;
	select(requestlog);
	var periodsize = 0;
	var recn	   = 0;
	var mindate	   = 999999;

nextrec1:
	// ///////
	if (esctoexit()) {
		stop();
	}

	if (not(readnext(ID))) {
		goto stage2;
	}

	recn += 1;
	if (TERMINAL)
		output(AT(-40), recn, ". ", ID);

	reqdate = ID.field("*", 2);
	if (reqdate < mindate) {
		mindate = reqdate;
	}
	if (reqdate < periodago) {
		goto nextrec1;
	}

	if (not(RECORD.read(requestlog, ID))) {
		goto nextrec1;
	}

	periodsize += RECORD.len() + ID.len();

	goto nextrec1;

///////
stage2:
	// /////

	// calculate approx how many periods to keep
	// and delete all records prior to that

	// if no data in last period then quit
	if (not periodsize) {
		stop();
	}

	let nperiodskept = (maxsize / periodsize).oconv("MD10P");
	var ndayskept	 = (nperiodskept * perioddays).floor();
	if (ndayskept > 9999) {
		let datekept = var("1/1/2000").iconv("D");
		ndayskept	 = date() - datekept + 1;
	}
	let datekept = date() - ndayskept;

	var tx = "TRIM REQUESTLOG";
	tx(-1) = "";
	tx(-1) = "Max size desired      " ^ oconv(maxsize, "[XBYTES]");
	tx(-1) = ("Last " ^ perioddays ^ " days").oconv("L#22") ^ oconv(periodsize, "[XBYTES]");
	tx(-1) = "";
	tx(-1) = "No. of days to keep   " ^ ndayskept;
	tx(-1) = "Delete upto           " ^ oconv(datekept - 1, "[DATE,4*]");
	tx(-1) = "";
	tx(-1) = "Minimum date found    " ^ oconv(mindate, "[DATE,4*]");

	if (mindate >= datekept) {
		tx(-1) = "";
		tx(-1) = "No need to delete anything";
		tx.replacer(_FM, _EOL);
		printl();
		printl(tx);
		stop();
	}

	// delete older recs
	var sizedeleted = 0;

	select(requestlog);
	recn		 = 0;
	var ndeleted = 0;

nextrec2:
	// ///////
	if (esctoexit()) {
		stop();
	}

	if (not(readnext(ID))) {

		// email results of deletions
		if (update) {
			tt = "Deleted";
		} else {
			tt = "Deletable";
		}
		tx(-1) = "";
		tx(-1) = tt.oconv("L#22") ^ ndeleted ^ " records - " ^ oconv(sizedeleted, "[XBYTES]");

		// call sysmsg(tx)
		call log("TRIMREQUESTLOG", tx);

		tx.replacer(_FM, _EOL);
		printl();
		printl(tx);

		stop();
	}

	recn += 1;
	if (TERMINAL)
		output(AT(-40), recn, ". ", ID);

	reqdate = ID.field("*", 2);

	if (reqdate >= datekept) {
		print(" kept");
		goto nextrec2;
	}

	if (update) {
		if (not(RECORD.read(requestlog, ID))) {
			goto nextrec2;
		}
		requestlog.deleterecord(ID);

		printl(" deleted");
		ndeleted += 1;
		sizedeleted += ID.len() + RECORD.len();
	} else {
		printl(" deletable");
	}

	goto nextrec2;

	// for c++
	return 0;
}

libraryexit()
