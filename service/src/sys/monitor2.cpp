#include <exodus/library.h>
libraryinit()

#include <cid.h>
#include <monitor2b.h>
#include <getbackpars.h>
#include <shell2.h>
#include <diskfreespace.h>
#include <getdatetime.h>
#include <sendmail.h>
#include <sysmsg.h>
#include <upgrade.h>

#include <sys_common.h>

var checkinterval;//num
var forced;
var upgradeready;//num
var anyusers;//num
var upgradefilename;
var processes;
var request;
var tempfilename;
var monitorkey;
var monitordata;
var currenttime;//num
var cidx;
var hostid;
var installid;
var msg;
var datax;
dim usertab;
var minusagetime;
var delusagetime;
var statistics;
dim netid;
var hourn;//num
var ii;//num
var tt;
var processcount;
var backuprequired;
var backupdrives;
var dbasesystems;
var dbasecodes;
var dbasecode;
var dbasen;//num
var status;
var statusn;
var bakpars;
var descriptions;
var status0123;//num
var ndbases;
var nok;
var temp;
var secs;
var description;
var minreq;
var nhung;
var first;//num
var startit;//num
var voc;
var xx;
var dbasesystem;
var nmaint;
var backupdrive;
var tpath;
var paramrec;
var home;
var lastbackupsize;//num
var lastbackupdatetime;//num
var currentdatetime;//num
var backupdriven;//num
var freespace;//num
var testdata;
var testfile;
var nextbackupdate;//num
var dow;
var nextbackupfilename;
var nextbackupfileinfo;
var reminderhours;//num
var localdate;
var localtime;//num
var xx2;
var xx3;
var xx4;
var lastnote;
var toaddresses;
var remindern;
var subject;
var body;
var nbackupdrives;
var present;
var deletingsize;//num
var hostdescriptions;
var versionnote;
var versiondate;
var upgradefilename83;
var upgradefiledir;
var longupgradefilename;
var errors;
var upgradex;
var upgradefiledatetime;
var wgetoutput;
var jj;//num
var osver;
var cpudesc;
var nprocs;
var result;
var nn;
var ips;
var line;
var nips;//num
var maxips;//num
var upgflagfile;

function main() {
	//c sys

	#include <system_common.h>
	//global all

	//NB the PROCESSES file is central to all databases in one installation
	//so this update of the monitor covers all the installations running processes

	//monitor every three minutes so there are 3 checks within the monitor 10 min
	checkinterval = 180;

	//command mode forces, call mode only every x minutes
	forced = SENTENCE.field(" ", 1).ucase() eq "MONITOR2";

	//quit if disabled and not forced
	if (not(forced) and var("../../disabled.cfg").osfile()) {
		return 0;
		}

	upgradeready = 0;
	//only counts web users
	anyusers = 0;
	upgradefilename = "upgrade.php";

	if (not(processes.open("PROCESSES", ""))) {
		call fsmsg();
		return 0;
	}

	if (not(DICT.open("DICT.PROCESSES", ""))) {
		//call fsmsg();
		return 0;
	}

	//the monitor command to record exodus process status
	//TODO get reply with new exodus auth and other info
	request = "UPDATE";

	//assume cannot have more than one update pending per installation
	tempfilename = request;

	//surrounded in % to evade being selected except by direct read
	monitorkey = "%" ^ request ^ "%";

	//attempt lock and keep it locked while we check (async so very quick)
	//so that we can update monitor status on the way out
	if (not(lockrecord("PROCESSES", processes, monitorkey))) {
		//print 'MONITOR2 is busy. Skipping Nagios update.'
		return 0;
	}

	//get the current monitor info
	if (not(monitordata.read(processes, monitorkey))) {
		monitordata = "";
	}

	//work out the current and nextchecktime
	currenttime = (var().date() + var().time() / 86400).oconv("MD50P");
	//nextchecktime=monitordata<1>+checkinterval/86400

	//only process if current time is > last checktime plus check interval
	//change to if difference > check interval in case clock is turned back
	//if forced or currenttime>nextchecktime then
	if (not(forced or ((currenttime - monitordata.a(1)).abs() gt checkinterval / 86400))) {

		/////////
		//goto exit
		call unlockrecord("PROCESSES", processes, monitorkey);
		return 0;
		/////////
	}

	//get the unique computer installation number
	cidx = cid();

	//get the host name
	hostid = SYSTEM.a(57);
	if (not hostid) {
		hostid = SYSTEM.a(44);
	}
	hostid = hostid.trim().lcase();

	installid = SYSTEM.a(139) ^ hostid ^ "_" ^ cidx;

	//READ
	/////

	//TODO call READ before write to get any results
	msg = "";
	if (cidx) {
		call monitor2b("READ", request, tempfilename, datax, msg);
	}

	if (msg) {
		//ignore errors which can include wget not being available
		//print msg
	} else {
		//print data
		//swap crlf with fm in data
		//convert crlf to fm:fm in data
		//if data<1>=system<17> then
		// perform cmd
		// end
	}

	//WRITE
	//////

	//count users (unique sessionids) in the last hour
	usertab.redim(25, 3);
	usertab = "";//dim
	//last 24 hours or back to midnight
	minusagetime = currenttime - 1;
	//minusagetime=int(currenttime)
	//31 days ago
	delusagetime = var().date() - 31;

	if (statistics.open("STATISTICS", "")) {
		select(statistics);
nextstatistic:
		if (readnext(ID)) {
			if (not(RECORD.read(statistics, ID))) {
				goto nextstatistic;
			}

			//skip/delete usage records older than x
			if (RECORD.a(1) lt minusagetime) {
				//trim out usage records older than one week
				if (RECORD.a(1) lt delusagetime) {
					statistics.deleterecord(ID);

				}
				goto nextstatistic;
			}

			//count the "user"
			netid.redim(3);
			netid(1) = ID.field("*", 2);
			netid(2) = ID.field("*", 3);
			netid(3) = ID.field("*", 4);
			hourn = ((currenttime - RECORD.a(1)) * 24).floor() + 1;
			hourn = (hourn - 1).mod(24) + 1;
			for (ii = 1; ii <= 3; ++ii) {
				if (not(usertab(hourn, ii).locate(netid(ii), tt))) {
					usertab(hourn, ii).r(1, -1, netid(ii));
				}
			} //ii;

			goto nextstatistic;

		}
	}

	//get the number of processes by database and status
	select(processes);
	processcount = "";
	backuprequired = "";
	//checkeddrives=''
	backupdrives = "";
	dbasesystems = "";
	dbasecodes = SYSTEM.a(58);

nextprocess:
	if (readnext(ID)) {
		if (not(RECORD.read(processes, ID))) {
			goto nextprocess;
		}

		//handle special records in processes
		if (ID[1] eq "%") {
			goto nextprocess;
		}

		dbasecode = RECORD.a(17);
		if (not dbasecode) {
			goto nextprocess;
		}
		if (not(dbasecodes.a(1).locate(dbasecode, dbasen))) {
			dbasecodes.r(1, dbasen, dbasecode);
		}
		dbasesystems.r(1, dbasen, RECORD.a(51));
		status = calculate("STATUS");
		if (not(var("OK,Hung,Maintenance,Closed,Crashed").locateusing(",", status.field(" ", 1), statusn))) {
			//statusn will be 6
			processcount.r(20, dbasen, status);
		}
		processcount.r(statusn, dbasen, processcount.a(statusn, dbasen) + 1);

		//works out if backup is required from ok processes by preference
		//use hung and maintenance processes if no ok processes
		//dont use closed and crashed processes because maybe old and obsolete
		//TODO make sensitive to definitions BACKUP records
		if (statusn eq 1 or ((backuprequired.a(1, dbasen) eq "" and statusn le 3))) {

			//get the bakpars for a specific process
			call getbackpars(bakpars, RECORD);

			//not suppressed and not test (non-live)
			backuprequired.r(1, dbasen, not(bakpars.a(9)) and not(bakpars.a(11)));

			//no backups required if database processes are all automatically started
			//except BASIC which we presume is used to startup and backup data.bak
			if (SYSTEM.a(58) eq "" and dbasecode ne "BASIC") {
				backuprequired.r(1, dbasen, 0);
			}

			//work out backup targets (data and uploads)
			//get datatarget sm uploadstarget sm backuptimefrom
			if (backuprequired.a(1, dbasen)) {
				tt = bakpars.a(7);
				//backpars 12 (upload backup target) can be 0 to suppress
				if (bakpars.a(12) and bakpars.a(12) ne bakpars.a(7)) {
					tt.r(1, 1, 2, bakpars.a(12));
				}
				//backuptime
				if (tt) {
					tt.r(1, 1, 3, bakpars.a(3));
				}
				backuprequired.r(1, dbasen, tt);
			}

		}

		goto nextprocess;
	}

	//add self into list of ok (should already be in OK list)
	//locate system<17> in dbasecodes<1> setting dbasen else
	// dbasecodes<1,dbasen>=system<17>
	// end
	//processcount<1,dbasen>=processcount<1,dbasen>+1

	//prepare the data to be sent to the monitor
	//checking minumum number of processes required
	descriptions = "";
	status0123 = 0;
	ndbases = dbasecodes.count(VM) + (dbasecodes ne "");
	for (dbasen = 1; dbasen <= ndbases; ++dbasen) {
		dbasecode = dbasecodes.a(1, dbasen);
		if (not dbasecode) {
			goto nextdbasen;
		}

		//show nok (number ok)
		nok = processcount.a(1, dbasen);
		if (not(nok) and not(VOLUMES)) {
			temp = ("../data/" ^ dbasecode ^ "/" ^ dbasecode ^ ".svr").osfile();
			secs = var().date() * 86400 + var().time() - (temp.a(2) * 86400 + temp.a(3));
			nok = secs lt 600;
			//otherwise flag hung
			if (not nok) {
				processcount.r(2, dbasen, 1);
			}
		}
		description = dbasecode;
		//if nok then description:=' ':nok:':Ok'
		if (nok gt 1) {
			description ^= " " ^ nok;
		}

		//warning if one less than required and critical if even less
		//! for warning, !! for critical
		minreq = SYSTEM.a(59, dbasen);
		//if nok=minreq-1 then
		// if status0123<1 then status0123=1
		// description:=' but ':minreq:' req.!'
		//end else if nok<minreq then
		// if status0123<2 then status0123=2
		// description:=' but ':minreq:' req!!'
		//end else
		// end

		//show and critical if any hung
		nhung = processcount.a(2, dbasen);
		if (nhung) {
			description ^= " " ^ nhung ^ ":Hung!!";
		}

		//start up new required processes unless a lot (currently 5) hung processes
		//not if this is a test database (only those with codes ending in TEST')
		//unless configured to all it
		first = 1;
		if ((((SYSTEM.a(17, 1).substr(-4, 4) ne "TEST") or SYSTEM.a(126)) and nok lt minreq) and nhung lt 5) {
			//if locksystem('LOCK',dbasecode) then
				//unlock immediately to enable startup - which will fail if anyone locks
				//call locksystem('UNLOCK',dbasecode)

				//dont bother trying to start it if another process is backing up
				//or it has been stopped or is the database is missing
			startit = not((dbasecode.lcase() ^ ".end").osfile());
			if (startit) {
				tt = "../data/" ^ dbasecode.lcase() ^ "/general/revmedia.lk";
				tt.converter("/", OSSLASH);
				if (not(tt.osfile())) {
					startit = 0;
				}
			}
			if (startit) {
				if (voc.open("VOC", "")) {
					if (lockrecord("", voc, "BACKUP*" ^ dbasecode)) {
						xx = unlockrecord("", voc, "BACKUP*" ^ dbasecode);
					} else {
						startit = 0;
					}
				}
			}

				//dont start anything if another process is still starting up
				//this lock is in INIT.GENERAL and MONITOR2
				//dont
			if (startit) {
				if (lockrecord("VOC", voc, "INIT.GENERAL.LOGIN")) {
					call unlockrecord("VOC", voc, "INIT.GENERAL.LOGIN");
				} else {
					startit = 0;
				}
			}

			if (startit) {
				//garbagecollect;
				dbasesystem = dbasesystems.a(1, dbasen);
				if (not dbasesystem) {
					dbasesystem = APPLICATION;
				}
				tt = "start exodus.js /system " ^ dbasesystem ^ " /database " ^ dbasecode;
					//pid:=' /pid ':tt<6>
					//print @(0):@(-4):time() 'MTS':' ':tt
				if (first) {
					printl();
					first = 0;
				}
				print(tt, " ...");
					//pcperform tt
				print("monitor2 calling shell2 ", tt, " ...");
				call shell2(tt, xx);
				call ossleep(1000*5);
				printl("done.");
			}

			// end
		}

		//show maintenance
		nmaint = processcount.a(3, dbasen);
		if (nmaint) {
			if ((description[-1]).isnum()) {
				description ^= "+";
			} else if (description[-1] ne " ") {
				description ^= " ";
			}
			description ^= nmaint ^ "M";
		}

		//ignore closed

		//ignore crashed
		//TODO flag crashed ONCE and perhaps delete or mark as warning sent

		//show any unknown statuses (shouldnt be any really since all in {STATUS})
		//but catch them here anyway and critical
		if (processcount.a(6, dbasen)) {
			description ^= " " ^ processcount.a(20, dbasen) ^ "?!!";
		}

		//warning if backup required but not done.
		//Warning for 1 day or never
		//Critical for more
		backupdrive = backuprequired.a(1, dbasen, 1).ucase();
		if (not(VOLUMES)) {
			backupdrive = dbasecode.substr(-5, 5) ne "_test";
		}
		if (backupdrive) {

	//uncomment to test non-existent drive
	//backupdrive='1'
			description ^= " Backup->" ^ backupdrive;
			tpath = "../data/" ^ dbasecode.lcase() ^ "/params2";
			tpath.converter("/", OSSLASH);
			if (VOLUMES) {
				//time fm date fm size
				if (not(paramrec.osread(tpath))) {
					paramrec = "";
				}
			} else {
				//time fm date
				tt = tpath.osfile();
				paramrec = tt.a(3) ^ FM ^ tt.a(2);
				//size
				call osgetenv("HOME", home);
				tpath ^= "/backups/sql/" ^ dbasecode.lcase() ^ ".sql.gz";
				tpath.converter("/", OSSLASH);

				paramrec.r(3, tpath.osfile().a(1));
			}
			lastbackupsize = paramrec.a(3);
			if (lastbackupsize) {
				description ^= " " ^ oconv(lastbackupsize, "[XBYTES,1]");
			}
	//uncomment to test insufficient backup space
	//lastbackupsize=999999999
			//garbagecollect;
			lastbackupdatetime = (paramrec.a(2) + paramrec.a(1) / 86400).oconv("MD50P");
			//if integer datetime then old format missing time so add 2 hours
			//if lastbackupdatetime and int(lastbackupdatetime)=lastbackupdatetime then lastbackupdatetime+=2/24
			//assume backup on same day (ie after last midnight)
			currentdatetime = (var().date() + var().time() / 86400).oconv("MD50P");
			tt = currentdatetime - lastbackupdatetime;
			//allow one day and one hour
			if (lastbackupdatetime and (tt gt 1 + 1 / 24.0)) {
				//warning if one day missed and critical if more than one
				description ^= " not done " ^ tt.oconv("MD10P") ^ " days!";
				if (paramrec and tt gt 2) {
					description ^= "!";
				}
			}

			if (not(backupdrives.a(1).locate(backupdrive, backupdriven))) {
				backupdrives.r(1, backupdriven, backupdrive);

				//ensure something is on the target
				//otherwise diskfreespace fails
				tt = backupdrive ^ "/data.bak";
				tt.converter("/", OSSLASH);
				call osmkdir(tt);

				//check target exists
				freespace = diskfreespace(backupdrive);
				if (freespace eq 999999999) {
					freespace = 0;
				}
				backupdrives.r(2, backupdriven, freespace);

				testdata = var().date() ^ FM ^ var().time();
				if (freespace) {
					testfile = backupdrive.a(1, 1, 1) ^ "/MONITOR.$$$";
					testfile.converter("/", OSSLASH);
					var(testdata).oswrite(testfile);
					if (not(tt.osread(testfile))) {
						tt = "";
					}
					testfile.osdelete();
				} else {
					tt = "";
				}

				if (tt ne testdata) {
					description ^= " impossible!!";

				} else {
					//present
					backupdrives.r(3, backupdriven, 1);

					//determine next backup filename
					//similar in MONITOR2 and FILEMAN
					nextbackupdate = var().date();
					//add 1 if next backup is tomorrow
					if (var().time() gt backuprequired.a(1, dbasen, 3)) {
						nextbackupdate += 1;
					}
					dow = ((sys.glang.a(22).field("|", (nextbackupdate - 1).mod(7) + 1)).substr(1, 8)).ucase();
					// eg 1/data.bak/adlined/wednesda/backup.zip
					//nextbackupfilename = backupdrive ^ "/data.bak/" ^ (dbasecode ^ "/" ^ dow).lcase() ^ "/backup.zip";
					nextbackupfilename = "../../backup." ^ ((nextbackupdate - 1).mod(7) + 1) ^ ".txt";
					nextbackupfilename.converter("/", OSSLASH);
					nextbackupfileinfo = nextbackupfilename.osfile();

					//if the next backup file exists (going to be overwritten)
					//TODO this should be checked for all databases not just the first
					if (nextbackupfileinfo) {

						//add its space to the freespace
						freespace += nextbackupfileinfo.a(2);

						//print a warning if havent changed the backup media and not suppressed
						if (not(bakpars.a(13)) and (nextbackupfileinfo.a(2) eq nextbackupdate - 7)) {

							//email a request to change usb
							//from 0600 to 1800 and not sent in the last 5.5 hours
							//usually 0600 1130 1700
							reminderhours = 5.5;
							call getdatetime(localdate, localtime, xx, xx2, xx3, xx4);
							//if (localtime ge 21600 and localtime le 64800) {
							if (localtime ge iconv("06:00", "MT") and localtime le iconv("18:00", "MT")){
								//only one email per installation
								call osread(lastnote, "lastnote.cfg");
								if (lastnote.a(1) ne localdate or (lastnote.a(2) lt localtime - 3600 * reminderhours)) {
									call oswrite(localdate ^ FM ^ localtime, "lastnote.cfg");

									//sendmail - if it fails, there will be an entry in the log
									toaddresses = bakpars.a(6);
									//never send reminders to exodus since we will get nagios warnings at 12:00
									toaddresses.swapper("backups@neosys.com", "");
									toaddresses = trim(toaddresses, ";");
									if (toaddresses) {
										remindern = (localtime - 21600).mod(3600 * reminderhours) + 1;
										subject = "EXODUS Backup Reminder";
										if (remindern gt 1) {
											subject ^= " (" ^ remindern ^ ")";
										}
										body = "It is time to change the EXODUS backup media (e.g. USB Flash Drive)";
										if (localtime lt 43200) {
											body.r(-1, FM ^ "Please change it " "before 12:00 midday today.");
										} else {
											body.r(-1, FM ^ "Please change it " "before 00:00 midnight tonight.");
										}
										printl(body);
										body.swapper(FM, var().chr(13));
										call sendmail(toaddresses, "", subject, body, "", "", xx);
									}

								}
							}

							//warning if they have not changed the usb by noon
							if (localtime ge var("12:00").iconv("MT")) {
								description ^= " Change Backup!";
							}

						}

					}

				}

			}

			//accumulate size of existing backups that will be deleted and overwritten
			backupdrives.r(4, backupdriven, backupdrives.a(4, backupdriven) + lastbackupsize);

		}

		//if index(description,'!',1) or nok or nhung or nmaint then
		if ((nok or nhung) or nmaint) {
			if (descriptions) {
				descriptions ^= ", ";
			}
			descriptions ^= description;
		}

nextdbasen:;
	} //dbasen;

	//check for free space on backup drive(s)
	nbackupdrives = backupdrives.a(1).count(VM) + (backupdrives.a(1) ne "");
	for (backupdriven = 1; backupdriven <= nbackupdrives; ++backupdriven) {

		present = backupdrives.a(3, backupdriven);
		if (present) {

			backupdrive = backupdrives.a(1, backupdriven);
			freespace = backupdrives.a(2, backupdriven);
			lastbackupsize = backupdrives.a(4, backupdriven);
			deletingsize = backupdrives.a(5, backupdriven);
			freespace += deletingsize;

			description = " Drive " ^ backupdrive ^ " Free:" ^ oconv(freespace, "[XBYTES,1]");

			//ensure 10% free space over last backup size
			//if we know the last backup size (this could fail if a NEW db is added)
			if (lastbackupsize gt 0 and (freespace lt lastbackupsize * 11 / 10)) {
				description ^= " only!!";
			}

			if (lastbackupsize) {
				description ^= " Req:" ^ oconv(lastbackupsize, "[XBYTES,1]");
			}

			if (description) {
				descriptions ^= " " ^ description;
			}

		}

	} //backupdriven;
	//oswrite descriptions on 'DESCRIPS'
	if (descriptions.index("!!") and status0123 lt 2) {
		status0123 = 2;
	}
	if (descriptions.index("!") and status0123 lt 1) {
		status0123 = 1;
	}

	hostdescriptions = "EXODUS ";

	//add exodus version for info
	tt = "general/version.dat";
	tt.converter("/", OSSLASH);
	call osread(versionnote, tt);
	versiondate = versionnote.trim().field(" ", 2, 3).iconv("D");
	tt = versiondate.oconv("D2/");
	tt = tt.substr(-2, 2) ^ "/" ^ tt.substr(1, 5);
	hostdescriptions ^= "Ver" ^ tt ^ "-" ^ versionnote.field(" ", 1).field(":", 1, 2);

	//dont allow upgrades by test databases
	if ((var(0) and SYSTEM.a(124)) and (SYSTEM.a(17).substr(-4, 4) ne "TEST")) {

		//get upgrade file details
		upgradefilename83 = SYSTEM.a(112);
		upgradefiledir = upgradefilename83.osfile();
		//get and cache upgradefilename83
		//ie the dos 8.3 version of upgrade.php@data=xxxxxxxx_999999
		if (not upgradefiledir) {
			//get the 8.3 filename version
			longupgradefilename = upgradefilename ^ "@data=" ^ installid;
			upgradefilename83 = shell2("dir " ^ (longupgradefilename.quote()) ^ " /x /l", errors);
			if (not errors) {
				upgradefilename83.converter("\r\n", FM);
				upgradefilename83 = upgradefilename83.a(6).substr(22, 999).trim().field(" ", 2);
				upgradefiledir = upgradefilename83.osfile();
				if (upgradefiledir) {
					SYSTEM.r(112, upgradefilename83);
				}
			}
		}

		//note if upgrade ready to be be installed (currently after nightly backup)
		if (upgradefiledir) {
			if (not(upgradex.read(processes, "%UPGRADE%"))) {
				upgradex = "";
			}
			upgradefiledatetime = (upgradefiledir.a(2) + upgradefiledir.a(3) / 86400).oconv("MD50P");
			//TODO make this only work for newer files? currently allows downgrading
			if (upgradex.a(2) ne upgradefiledatetime) {
				//check wget output file to see if is fully downloaded (100%) or "NO NEWER"
				tt = upgradefilename;
				tt.splicer(-3, 3, "$wg");
				call osread(wgetoutput, tt);
				if (wgetoutput.ucase().index(" NO NEWER ") or wgetoutput.index("100%")) {
					upgradefiledir = upgradefilename83.osfile();
					//hostdescriptions:=' - Upg':upgradefiledir<2> 'D2/J':'-':upgradefiledir<3> 'MT'
					//tt=upgradefiledir<2> 'D2/J'
					tt = upgradefiledir.a(2).oconv("D2/E");
					tt = tt.substr(-2, 2) ^ "/" ^ tt.substr(1, 5);
					hostdescriptions ^= " - Upg" ^ tt ^ "-" ^ upgradefiledir.a(3).oconv("MT");
					upgradeready = 1;
				} else if (wgetoutput.ucase().index(" ERROR 404")) {
				} else if (wgetoutput.ucase().index(" failed: Unknown host.")) {
					printl("DNS cant resolve upgrade host name");
				} else if (2) {
					if (not(var("exodus.id").osfile())) {
						print("upgrade downloading");
					}
				}
				{}
			}
		}

	}

	//show local time
	hostdescriptions ^= " - At:" ^ var().time().oconv("MT");

	//find max nusersperhour by type
	for (ii = 1; ii <= 24; ++ii) {
		for (jj = 1; jj <= 3; ++jj) {
			usertab(ii, jj) = usertab(ii, jj).count(VM) + (usertab(ii, jj) ne "");
			if (usertab(ii, jj) gt usertab(25, jj)) {
				usertab(25, jj) = usertab(ii, jj);
			}
		} //jj;
	} //ii;

	//list number of users (in last hour)
	hostdescriptions ^= " - Users:";
	hourn = 1;
	for (ii = 1; ii <= 3; ++ii) {
		tt = usertab(hourn, ii);
		if (tt) {
			anyusers = 1;
		}
		if (ii gt 1) {
			hostdescriptions ^= "/";
		}
		hostdescriptions ^= tt;
	} //ii;

	//list max number of users
	hostdescriptions ^= " - Max:";
	hourn = 25;
	for (ii = 1; ii <= 3; ++ii) {
		tt = usertab(1, ii);
		if (ii gt 1) {
			hostdescriptions ^= "/";
		}
		hostdescriptions ^= usertab(hourn, ii);
	} //ii;

	//os description
	call osgetenv("VER", osver);
	hostdescriptions ^= " - " ^ osver;

	//cpu description
	call osgetenv("CPU", cpudesc);
	nprocs = SYSTEM.a(9);
	hostdescriptions ^= " - " ^ cpudesc ^ " x " ^ nprocs;

	//list ipnos
	if (VOLUMES) {
		result = shell2("ipconfig /all", errors).ucase();
		result.converter("\r\n", FM);
		nn = result.count(FM) + 1;
		ips = "";
		for (ii = 1; ii <= nn; ++ii) {
			line = result.a(ii).trim();
			line.swapper("IPV4 ADDRESS", "IP ADDRESS");
			if (line.substr(1, 10) eq "IP ADDRESS") {
				ips.r(-1, line.field(":", 2).trim().field("(", 1));
			//only display the first
				goto gotip;
			}
		} //ii;
	} else {
		ips = shell2("printf $(hostname -I | cut -d' ' -f 1)");
	}
gotip:
	if (not ips) {
		ips = "0.0.0.0";
	}
	if (ips) {
		ips.converter(FM, ",");
		nips = ips.count(",") + 1;
		//limit to 5 ips, replace middle ones with - to indicate suppressed
		maxips = 5;
		if (nips gt maxips) {
			ips = ips.fieldstore(",", (maxips / 2).floor(), maxips - nips, "...");
		}
		hostdescriptions ^= " - " ^ ips;
		//hostdescriptions:=' - ':ips:' - '
	}

	//oswrite hostdescriptions on 'xx'

	//package the data for the monitor
	datax = "";

	//host passive check line
	//currently any exodus update indicates that the host is ok
	if (datax) {
		datax ^= var().chr(10);
	}
	datax ^= "PROCESS_HOST_CHECK_RESULT;" ^ installid ^ ";0;" ^ hostdescriptions;

	//service passive check line
	if (datax) {
		datax ^= var().chr(10);
	}
	datax ^= "PROCESS_SERVICE_CHECK_RESULT;" ^ installid ^ ";exodus;" ^ status0123 ^ ";" ^ descriptions;

	//request info to be sent to the monitor asynchronously - doesnt wait

	//request it to be done
	//currently just uses wget to http post the info in the background
	msg = "";
	if (cidx) {
		call monitor2b("WRITE", request, tempfilename, datax, msg);
	}

	//also look to get any upgrade file
	//TODO put this on a less frequent check since it will only be
	//updated after the nightly backup usually *unless last hour users are 0/0/0
	//dont allow upgrades by test databases
	if ((var(0) and SYSTEM.a(124)) and (SYSTEM.a(17).substr(-4, 4) ne "TEST")) {
		if (msg) {
		} else {
			msg = "";
			if (cidx) {
				call monitor2b("WRITE", "UPGRADE", "UPGRADE", installid, msg);
			}
		}
	}

	//any error is going to be up front like error in parameters or missing wget
	//errors in hostnames and connectivity must be obtained
	//with a mode='READ' and the same tempfilename
	//report and prevent further checking/reporting for an hour
	//unless somehow forced
	if (msg) {
		monitordata.r(1, currenttime.a(1) + 1 / 24.0);
		call sysmsg(msg);
	} else {
		//register checked at current time
		monitordata.r(1, currenttime);
	}

	//NOTE and errors in hostnames and connectivity timeouts/response etc
	//can be obtained after timeout etc with a mode='READ' and same tempfilename
	//but how to know when ready (or timedout)?

	//update the monitor status
	monitordata.write(processes, monitorkey);

	//optional upgrade immediate if no web users in the last hour
	//and no other parallel installation upgrades started in last 1 mins
	//parallel means installations next to each other in the same folder
	upgflagfile = "../../UPGRADE.$$$";
	upgflagfile.converter("/", OSSLASH);
	if ((upgradeready and not(anyusers)) and ((var().time() - upgflagfile.osfile().a(3)).abs() gt 60)) {
		call oswrite("", upgflagfile);
		//close all other processes, upgrade, close and restart
		call upgrade("R");
		perform("OFF");
		var().logoff();
	}

	/////
	//exit:
	/////
	call unlockrecord("PROCESSES", processes, monitorkey);

	return 0;
}

libraryexit()
