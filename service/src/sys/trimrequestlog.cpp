#include <exodus/library.h>
libraryinit()

#include <log.h>

#include <sys_common.h>

var reqdate;
var tt;

function main() {
	//c sys
	//global reqdate

	#include <system_common.h>

	var update = 1;

	//keep approx nn Mb
	var maxsize = 20 * 1024 * 1024;

	//estimate based on size in last n days
	var perioddays = 7;

	var requestlog;
	if (not(requestlog.open("REQUESTLOG", ""))) {
		stop();
	}

	//osfilename=field2(file,vm,-1)[14,9999]
	//osfilename[-3,3]=''
	//osopen osfilename:'.lk' to lkfile else call fsmsg();stop
	//osopen osfilename:'.ov' to ovfile else call fsmsg();stop

	//stage1:
	////////

	//calculate the size of the last month of requestlog entries

	var periodago = var().date() - perioddays;
	select(requestlog);
	var periodsize = 0;
	var recn = 0;
	var mindate = 999999;

nextrec1:
/////////
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
	if (reqdate lt mindate) {
		mindate = reqdate;
	}
	if (reqdate lt periodago) {
		goto nextrec1;
	}

	if (not(RECORD.read(requestlog, ID))) {
		goto nextrec1;
	}

	periodsize += RECORD.length() + ID.length();

	goto nextrec1;

///////
stage2:
///////

	//calculate approx how many periods to keep
	//and delete all records prior to that

	//if no data in last period then quit
	if (not periodsize) {
		stop();
	}

	var nperiodskept = (maxsize / periodsize).oconv("MD10P");
	var ndayskept = (nperiodskept * perioddays).floor();
	if (ndayskept gt 9999) {
		var datekept = var("1/1/2000").iconv("D");
		ndayskept = var().date() - datekept + 1;
	}
	var datekept = var().date() - ndayskept;

	var tx = "TRIM REQUESTLOG";
	tx(-1) = "";
	tx(-1) = "Max size desired      " ^ oconv(maxsize, "[XBYTES]");
	tx(-1) = ("Last " ^ perioddays ^ " days").oconv("L#22") ^ oconv(periodsize, "[XBYTES]");
	tx(-1) = "";
	tx(-1) = "No. of days to keep   " ^ ndayskept;
	tx(-1) = "Delete upto           " ^ oconv(datekept - 1, "[DATE,4*]");
	tx(-1) = "";
	tx(-1) = "Minimum date found    " ^ oconv(mindate, "[DATE,4*]");

	if (mindate ge datekept) {
		tx(-1) = "";
		tx(-1) = "No need to delete anything";
		tx.swapper(FM, "\r\n");
		printl();
		printl(tx);
		stop();
	}

	//delete older recs
	var sizedeleted = 0;

	select(requestlog);
	recn = 0;
	var ndeleted = 0;

nextrec2:
/////////
	if (esctoexit()) {
		stop();
	}

	if (not(readnext(ID))) {

		//email results of deletions
		if (update) {
			tt = "Deleted";
		} else {
			tt = "Deletable";
		}
		tx(-1) = "";
		tx(-1) = tt.oconv("L#22") ^ ndeleted ^ " records - " ^ oconv(sizedeleted, "[XBYTES]");

		//call sysmsg(tx)
		call log("TRIMREQUESTLOG", tx);

		tx.swapper(FM, "\r\n");
		printl();
		printl(tx);

		stop();
	}

	recn += 1;
	if (TERMINAL)
		output(AT(-40), recn, ". ", ID);

	reqdate = ID.field("*", 2);

	if (reqdate ge datekept) {
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
		sizedeleted += ID.length() + RECORD.length();
	} else {
		printl(" deletable");
	}

	goto nextrec2;

	//for c++
	return 0;
}

libraryexit()
