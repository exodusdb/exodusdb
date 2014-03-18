#include <exodus/library.h>
libraryinit()

#include <sysvar_109_134.h>
#include <revrelease.h>
#include <rtp57.h>

var value;
var lockmode;//num
var unlockmode;//num

function main(in databasecode) {

	//returns the number of other users of NEOSYS

	if (SENTENCE.field(" ", 1) == "OTHERUSERS") {
		databasecode = SENTENCE.field(" ", 2);
	}
	if (databasecode.unassigned()) {
		databasecode = "";
	}
	var processes = "";
	//userlockid.sys134=sysvar('GET',109,134)
	var userlockidsys134 = sysvar_109_134("GET", value);

	////
	////

	var returndata = 0;
	var otherusercodes = "";
	if (STATION ne "") {

		if (revrelease() >= 2.1) {
			lockmode = 36;
			unlockmode = 37;
		}else{
			lockmode = 23;
			unlockmode = 24;
		}

		var lockprefix = "U" ^ (var().serial()).substr(-4, 4);

		//FOR lockno = 1 TO RUNTIME();*SYSE3_NUSERS

		for (var lockno = 1; lockno <= 100; ++lockno) {
			//skip 10,20,100 etc because they appear to be equivalent to
			// their equivalents without trailing zeroes
			if (lockno.substr(-1, 1) ne "0") {

				var lockid = lockprefix ^ lockno;
				//IF lockid # SYS134 THEN

				//attempt to lock
				var result = "";
				call rtp57(lockmode, "", "", lockid, "", "", result);
				if (result) {
					//if successful, then unlock
					call rtp57(unlockmode, "", "", lockid, "", "", "");
				}else{

					if (lockid == userlockidsys134) {

						//why was this here?
						//otherusercodes<1,-1>=lockid

					}else{

						//skip processes in wrong database
						if (databasecode) {
							if (processes == "") {
								if (not(processes.open("PROCESSES", ""))) {
									processes = 0;
								}
							}
							if (processes) {
								var processno = lockno - (lockno / 10).floor();
								var process;
								if (process.read(processes, processno)) {
									if (process.a(17) ne databasecode) {
										goto nextlock;
									}
								}else{
									//if no process record then assume no process
									//and failed lock because another OTHERUSERS is testing the same lock
									//could really skip further checking since should not be
									//any higher processes but lock fail on missing process is infrequent
									//but does happen when with many eg 10+ processes on diff dbs
									goto nextlock;
								}
							}
						}

						otherusercodes.r(1, -1, lockid);

					}

					returndata += 1;
				}

			}
nextlock:
		};//lockno;

		returndata -= 1;
		if (returndata < 0) {
			returndata = 0;
		}
	}
	if (returndata) {
		returndata.r(2, otherusercodes);
		returndata.r(3, userlockidsys134);
	}

	for (var ii = 1; ii <= 9999; ++ii) {
		var usercode = returndata.a(2, ii);
	///BREAK;
	if (not usercode) break;;
		usercode = usercode.substr(6, 9999);
		usercode -= (usercode / 10).floor();
		returndata.r(2, ii, "PROCESS" ^ usercode);
	};//ii;

	if (SENTENCE.field(" ", 1) == "OTHERUSERS") {
		call note(returndata.a(1) ^ " other users");
	}
	//if field(@sentence,' ',1)='TEST' then call note(returndata:' other users|':otherlocks)

	return returndata;

}


libraryexit()