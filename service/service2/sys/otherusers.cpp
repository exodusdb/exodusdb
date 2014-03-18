#include <exodus/library.h>
libraryinit()

var value;
var lockmode;//num
var unlockmode;//num

function main(in databasecode="") {

	//returns the number of other users of NEOSYS
	//by attempting to lock process numbers in the PROCESSES file

	//open PROCESSES file or indicate no other users
	var processes = "";
	if (processes == "") {
		if (not(processes.open("PROCESSES", ""))) {
			processes = 0;
			return "";
		}
	}

	var lockprefix="processlock-";

	var ownprocessno = SYSTEM.a(24);
	var ownlockid = lockprefix^ownprocessno;
	var returndata = FM ^ FM ^ ownprocessno;

	var notherprocesses = 0;
	var otherusercodes = "";

	for (var processno = 1; processno <= 100; ++processno) {

		var lockid = lockprefix ^ processno;

		//if can lock and no process then become that processno
		//assume no other process on this number
		if (processes.lock(lockid)) {

			//become that processno and leave it locked
			if (not ownprocessno) {
				ownprocessno=processno;
				SYSTEM.r(24, processno);
				ownlockid = lockprefix^ownprocessno;
printl("Became process ", ownprocessno);
			} else
				//unlock - unused processno
				processes.unlock(lockid);

		//if cant lock
		}else{

			if (lockid == ownlockid)
				continue;

			//skip processes in wrong database
			if (databasecode) {
				if (processes) {
					var process;
					if (process.read(processes, processno)) {
						if (process.a(17) ne databasecode) {
							continue;
						}
					}else{
						//if no process record then assume no process
						//and failed lock because another OTHERUSERS is testing the same lock
						//could really skip further checking since should not be
						//any higher processes but lock fail on missing process is infrequent
						//but does happen when with many eg 10+ processes on diff dbs
						continue;
					}
				}
			}
			returndata.r(1, returndata.a(1)+1);
			returndata.inserter(2,-1,processno);
		}

	}

	returndata.r(3, ownprocessno);
	return returndata;

}

libraryexit()
