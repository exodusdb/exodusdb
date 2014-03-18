#include <exodus/library.h>
libraryinit()

var paramrec;

function main(in currdataset, in msg) {
	//jbase
	var msg = "";

	if (not(paramrec.osread("..\\data\\" ^ currdataset ^ "\\params2"))) {
		return 0;
	}
	var lastbackupdate = paramrec.a(2);

	//if lastbackupdate and lastbackupdate lt date()-1 then
	//assume backup on same day (ie after last midnight)
	if (lastbackupdate and lastbackupdate < var().date()) {
		msg = "The last backup was ";
		var ndays = var().date() - lastbackupdate;
		msg ^= ndays ^ " day" ^ var("s").substr(1, ndays ne 1) ^ " ago.";
		msg ^= "   (" ^ lastbackupdate.oconv("D") ^ ")";
		msg.swapper("(0", "(");
		msg.r(-1, "NEOSYS recommends that you \"BACKUP\" your data ");
		msg.r(-1, "regularly to prevent total loss of data due to");
		msg.r(-1, "power failure, disk damage or other accidents.");
	}

	return 0;

}


libraryexit()