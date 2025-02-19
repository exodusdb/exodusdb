#include <exodus/library.h>
libraryinit()

#include <cid.h>
#include <diskfreespace.h>
#include <getbackpars.h>
#include <monitor2b.h>
#include <sendmail.h>
#include <shell2.h>
#include <sysmsg.h>

#include <service_common.h>

#include <srv_common.h>

var checkinterval;	// num
var forced;
//var upgradeready; // num
var anyusers;  // num
//var upgradefilename;
var processes;
var request;
var tempfilename;
var monitorkey;
var monitordata;
var currenttime;  // num
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
var hourn;	// num
//var ii;		// num
//var tt;
var processcount;
var backuprequired;
var backupdrives;
var dbasesystems;
var dbasecodes;
var dbasecode;
//var dbasen;	 // num
var status;
var statusn;
var bakpars;
var descriptions;
var status0123;	 // num
var ndbases;
var nok;
var temp;
var secs;
var description;
var minreq;
var nhung;
var first;	  // num
var startit;  // num
var voc;
//var xx;
var dbasesystem;
var nmaint;
var backupdrive;
var tpath;
var paramrec;
var home;
var lastbackupsize;		 // num
var lastbackupdatetime;	 // num
var currentdatetime;	 // num
//var backupdriven;		 // num
var freespace;			 // num
var testdata;
var testfile;
var nextbackupdate;	 // num
var dow;
var nextbackupfilename;
var nextbackupfileinfo;
var reminderhours;	// num
var localdate;
var localtime;	// num
var lastnote;
var toaddresses;
var remindern;
var subject;
var body;
//var nbackupdrives;
var present;
var deletingsize;  // num
//var hostdescriptions;
var versionnote;
var versiondate;
//var upgradefilename83;
//var upgradefiledir;
//var longupgradefilename;
var errors;
//var upgradex;
//var upgradefiledatetime;
var wgetoutput;
//var jj;	 // num
var osver;
var cpudesc;
var nprocs;
var result;
var nn;
var ips;
var line;
var nips;	 // num
var maxips;	 // num
//var upgflagfile;

function main() {

	// NB the PROCESSES file is central to all databases in one installation
	// so this update of the monitor covers all the installations running processes

	// monitor every three minutes so there are 3 checks within the monitor 10 min
	checkinterval = 180;

	// command mode forces, call mode only every x minutes
	forced = SENTENCE.field(" ", 1).ucase() == "MONITOR2";

	// quit if disabled and not forced
	if (not(forced) and var("../../disabled.cfg").osfile()) {
		return 0;
	}

	// upgradeready = 0;
	// only counts web users
	anyusers = 0;
	// upgradefilename = "upgrade.php";

	if (not processes.open("PROCESSES", "")) {
		call mssg(lasterror());
		return 0;
	}

	if (not DICT.open("DICT.PROCESSES", "")) {
		// call mssg(lasterror());
		return 0;
	}

	// the monitor command to record exodus process status
	// TODO get reply with new exodus auth and other info
	request = "UPDATE";

	// assume cannot have more than one update pending per installation
	tempfilename = request;

	// surrounded in % to evade being selected except by direct read
	monitorkey = "%" ^ request ^ "%";

	// attempt lock and keep it locked while we check (async so very quick)
	// so that we can update monitor status on the way out
	if (not lockrecord("PROCESSES", processes, monitorkey)) {
		// print 'MONITOR2 is busy. Skipping Nagios update.'
		return 0;
	}

	// get the current monitor info
	if (not monitordata.read(processes, monitorkey)) {
		monitordata = "";
	}

	// work out the current and nextchecktime
	currenttime = (date() + time() / 86400).oconv("MD50P");
	// nextchecktime=monitordata<1>+checkinterval/86400

	// only process if current time is > last checktime plus check interval
	// change to if difference > check interval in case clock is turned back
	// if forced or currenttime>nextchecktime then
	if (not(forced or ((currenttime - monitordata.f(1)).abs() > checkinterval / 86400))) {

		// ///////
		// goto exit
		call unlockrecord("PROCESSES", processes, monitorkey);
		return 0;
		// ///////
	}

	// get the unique computer installation number
	cidx = cid();

	// get the host name
	hostid = SYSTEM.f(57);
	if (not hostid) {
		hostid = SYSTEM.f(44);
	}
	hostid.trimmer();
	hostid.lcaser();

	installid = SYSTEM.f(139) ^ hostid ^ "_" ^ cidx;

	// READ
	// ///

	// TODO call READ before write to get any results
	msg = "";
	if (cidx) {
		call monitor2b("READ", request, tempfilename, datax, msg);
	}

	// WRITE
	// ////

	// count users (unique sessionids) in the last hour
	usertab.redim(25, 3);
	usertab = "";  //dim
	// last 24 hours or back to midnight
	minusagetime = currenttime - 1;
	// minusagetime=int(currenttime)
	// 31 days ago
	delusagetime = date() - 31;

	if (statistics.open("STATISTICS", "")) {
		select(statistics);
nextstatistic:
		if (readnext(ID)) {
			if (not RECORD.read(statistics, ID)) {
				goto nextstatistic;
			}

			// skip/delete usage records older than x
			if (RECORD.f(1) < minusagetime) {
				// trim out usage records older than one week
				if (RECORD.f(1) < delusagetime) {
					statistics.deleterecord(ID);
				}
				goto nextstatistic;
			}

			// count the "user"
			netid.redim(3);
			netid[1] = ID.field("*", 2);
			netid[2] = ID.field("*", 3);
			netid[3] = ID.field("*", 4);
			hourn	 = ((currenttime - RECORD.f(1)) * 24).floor() + 1;
			hourn	 = (hourn - 1).mod(24) + 1;
			for (const int ii : range(1, 3)) {
				if (not usertab(hourn, ii).locate(netid[ii])) {
					usertab(hourn, ii)(1, -1) = netid[ii];
				}
			}  // ii;

			goto nextstatistic;
		}
	}

	// get the number of processes by database and status
	select(processes);
	processcount   = "";
	backuprequired = "";
	// checkeddrives=''
	backupdrives = "";
	dbasesystems = "";
	dbasecodes	 = SYSTEM.f(58);

nextprocess:
	if (readnext(ID)) {
		if (not RECORD.read(processes, ID)) {
			goto nextprocess;
		}

		// handle special records in processes
		if (ID.starts("%")) {
			goto nextprocess;
		}

		dbasecode = RECORD.f(17);
		if (not dbasecode) {
			goto nextprocess;
		}
		var dbasen;
		if (not dbasecodes.f(1).locate(dbasecode, dbasen)) {
			dbasecodes(1, dbasen) = dbasecode;
		}
		dbasesystems(1, dbasen) = RECORD.f(51);
		status					= calculate("STATUS");
		if (not var("OK,Hung,Maintenance,Closed,Crashed").locateusing(",", status.field(" ", 1), statusn)) {
			// statusn will be 6
			processcount(20, dbasen) = status;
		}
		processcount(statusn, dbasen) = processcount.f(statusn, dbasen) + 1;

		// works out if backup is required from ok processes by preference
		// use hung and maintenance processes if no ok processes
		// dont use closed and crashed processes because maybe old and obsolete
		// TODO make sensitive to definitions BACKUP records
		if (statusn == 1 or ((backuprequired.f(1, dbasen) == "" and statusn <= 3))) {

			// get the bakpars for a specific process
			call getbackpars(bakpars, RECORD);

			// not suppressed and not test (non-live)
			backuprequired(1, dbasen) = not(bakpars.f(9)) and not(bakpars.f(11));

			// no backups required if database processes are all automatically started
			// except BASIC which we presume is used to startup and backup data.bak
			// if (SYSTEM.f(58) == "" and dbasecode != "BASIC") {
			if (SYSTEM.f(58) == "") {
				backuprequired(1, dbasen) = 0;
			}

			// work out backup targets (data and uploads)
			// get datatarget sm uploadstarget sm backuptimefrom
			if (backuprequired.f(1, dbasen)) {
				var tt = bakpars.f(7);
				// backpars 12 (upload backup target) can be 0 to suppress
				if (bakpars.f(12) and bakpars.f(12) != bakpars.f(7)) {
					tt(1, 1, 2) = bakpars.f(12);
				}
				// backuptime
				if (tt) {
					tt(1, 1, 3) = bakpars.f(3);
				}
				backuprequired(1, dbasen) = tt;
			}
		}

		goto nextprocess;
	}

	// add self into list of ok (should already be in OK list)
	// locate system<17> in dbasecodes<1> setting dbasen else
	// dbasecodes<1,dbasen>=system<17>
	// end
	// processcount<1,dbasen>=processcount<1,dbasen>+1

	// prepare the data to be sent to the monitor
	// checking minumum number of processes required
	descriptions = "";
	status0123	 = 0;
	ndbases		 = dbasecodes.fcount(VM);
	for (const int dbasen : range(1, ndbases)) {
		dbasecode = dbasecodes.f(1, dbasen);
		if (not dbasecode) {
			goto nextdbasen;
		}

		// show nok (number ok)
		nok = processcount.f(1, dbasen);
//		if (not(nok) and not(VOLUMES)) {
//			temp = ("../data/" ^ dbasecode ^ "/" ^ dbasecode ^ ".svr").osfile();
//			secs = date() * 86400 + time() - (temp.f(2) * 86400 + temp.f(3));
//			nok	 = secs < 600;
//			// otherwise flag hung
//			if (not nok) {
//				processcount(2, dbasen) = 1;
//			}
//		}
		description = dbasecode;
		// if nok then description:=' ':nok:':Ok'
		if (nok > 1) {
			description ^= " " ^ nok;
		}

		// warning if one less than required and critical if even less
		// ! for warning, !! for critical
		minreq = SYSTEM.f(59, dbasen);
		// if nok=minreq-1 then
		// if status0123<1 then status0123=1
		// description:=' but ':minreq:' req.!'
		// end else if nok<minreq then
		// if status0123<2 then status0123=2
		// description:=' but ':minreq:' req!!'
		// end else
		// end

		// show and critical if any hung
		nhung = processcount.f(2, dbasen);
		if (nhung) {
			description ^= " " ^ nhung ^ ":Hung!!";
		}

		// start up new required processes unless a lot (currently 5) hung processes
		// not if this is a test database (only those with codes ending in TEST')
		// unless configured to all it
		first = 1;
		if (((not SYSTEM.f(17, 1).ends("_test") or SYSTEM.f(126)) and nok < minreq) and nhung < 5) {
			// if locksystem('LOCK',dbasecode) then
			// unlock immediately to enable startup - which will fail if anyone locks
			// call locksystem('UNLOCK',dbasecode)

			// dont bother trying to start it if another process is backing up
			// or it has been stopped or is the database is missing
			startit = not((dbasecode.lcase() ^ ".end").osfile());
			if (startit) {
				var tt = "../data/" ^ dbasecode.lcase() ^ "/general/revmedia.lk";
				tt.converter("/", OSSLASH);
				if (not tt.osfile()) {
					startit = 0;
				}
			}
			if (startit) {
				if (voc.open("VOC", "")) {
					if (lockrecord("", voc, "BACKUP*" ^ dbasecode)) {
						unlockrecord("", voc, "BACKUP*" ^ dbasecode);
					} else {
						startit = 0;
					}
				}
			}

			// dont start anything if another process is still starting up
			// this lock is in INIT.GENERAL and MONITOR2
			// dont
			if (startit) {
				if (lockrecord("VOC", voc, "INIT.GENERAL.LOGIN")) {
					call unlockrecord("VOC", voc, "INIT.GENERAL.LOGIN");
				} else {
					startit = 0;
				}
			}

			if (startit) {
				dbasesystem = dbasesystems.f(1, dbasen);
				if (not dbasesystem) {
					dbasesystem = APPLICATION;
				}
				var cmd = "start exodus.js /system " ^ dbasesystem ^ " /database " ^ dbasecode;
				// pid:=' /pid ':cmd<6>
				// print @(0):@(-4):time() 'MTS':' ':cmd
				if (first) {
					printl();
					first = 0;
				}
				printx(cmd, " ...");
				// pcperform cmd
				printx("monitor2 calling shell2 ", cmd, " ...");
				call shell2(cmd);
				call ossleep(1000 * 5);
				printl("done.");
			}

			// end
		}

		// show maintenance
		nmaint = processcount.f(3, dbasen);
		if (nmaint) {
			if ((description.last()).isnum()) {
				description ^= "+";
			} else if (not description.ends(" ")) {
				description ^= " ";
			}
			description ^= nmaint ^ "M";
		}

		// ignore closed

		// ignore crashed
		// TODO flag crashed ONCE and perhaps delete or mark as warning sent

		// show any unknown statuses (shouldnt be any really since all in {STATUS})
		// but catch them here anyway and critical
		if (processcount.f(6, dbasen)) {
			description ^= " " ^ processcount.f(20, dbasen) ^ "?!!";
		}

		// warning if backup required but not done.
		// Warning for 1 day or never
		// Critical for more
		backupdrive = backuprequired.f(1, dbasen, 1).ucase();
//		if (not VOLUMES) {
//			backupdrive = not dbasecode.ends("_test");
//		}
		if (backupdrive) {

			// uncomment to test non-existent drive
			// backupdrive='1'
			description ^= " Backup->" ^ backupdrive;
			tpath = "../data/" ^ dbasecode.lcase() ^ "/params2";
			tpath.converter("/", OSSLASH);

			let fileinfo  = tpath.osfile();
			paramrec	  = fileinfo.f(3) ^ FM ^ fileinfo.f(2);
			// size
			//call osgetenv("HOME", home);
			if (not osgetenv("HOME", home)) {
				//null
			}
			tpath ^= "/backups/sql/" ^ dbasecode.lcase() ^ ".sql.gz";
			tpath.converter("/", OSSLASH);

			paramrec(3) = tpath.osfile().f(1);

			lastbackupsize = paramrec.f(3);
			if (lastbackupsize) {
				description ^= " " ^ oconv(lastbackupsize, "[XBYTES,1]");
			}
			// uncomment to test insufficient backup space
			// lastbackupsize=999999999
			lastbackupdatetime = (paramrec.f(2) + paramrec.f(1) / 86400).oconv("MD50P");
			// if integer datetime then old format missing time so add 2 hours
			// if lastbackupdatetime and int(lastbackupdatetime)=lastbackupdatetime then lastbackupdatetime+=2/24
			// assume backup on same day (ie after last midnight)
			currentdatetime		= (date() + time() / 86400).oconv("MD50P");
			let days_since_backup	= currentdatetime - lastbackupdatetime;
			// allow one day and one hour
			if (lastbackupdatetime and (days_since_backup > 1 + 1 / 24.0)) {
				// warning if one day missed and critical if more than one
				description ^= " not done " ^ days_since_backup.oconv("MD10P") ^ " days!";
				if (paramrec and days_since_backup > 2) {
					description ^= "!";
				}
			}

			var backupdriven;
			if (not backupdrives.f(1).locate(backupdrive, backupdriven)) {
				backupdrives(1, backupdriven) = backupdrive;

				// ensure something is on the target
				// otherwise diskfreespace fails
				var oldbackuppath = backupdrive ^ "/data.bak";
				oldbackuppath.converter("/", OSSLASH);
				if (osdir(oldbackuppath) and not osrmdir(oldbackuppath)) {
					abort(lasterror());
				}

				// check target exists
				freespace = diskfreespace(backupdrive);
				if (freespace == 999999999) {
					freespace = 0;
				}
				backupdrives(2, backupdriven) = freespace;

				// Check Presence of external device (possibly in container host server)
				// Can be a link (deref not required) or simply exists
				// lxc containers can see but often not dereference links due to permissions issues)
				let backupdir = "/backups/usb";
				if (not osshell("test -L " ^ backupdir ^ " -o -e " ^ backupdir)) {
					description ^= " impossible!!";
				} else {
					// present
					backupdrives(3, backupdriven) = 1;

					// determine next backup filename
					// similar in MONITOR2 and FILEMAN
					nextbackupdate = date();
					// add 1 if next backup is tomorrow
					if (time() > backuprequired.f(1, dbasen, 3)) {
						nextbackupdate += 1;
					}
					// dow = ((srv.glang.f(22).field("|", (nextbackupdate - 1).mod(7) + 1)).first(8)).ucase();
					dow = srv.glang.f(22).field("|", nextbackupdate.oconv("DW")).first(8).ucase();
					// eg 1/data.bak/adlined/wednesda/backup.zip
					// nextbackupfilename = backupdrive ^ "/data.bak/" ^ (dbasecode ^ "/" ^ dow).lcase() ^ "/backup.zip";
					// nextbackupfilename = "../../backup." ^ ((nextbackupdate - 1).mod(7) + 1) ^ ".txt";
					nextbackupfilename = "../../backup." ^ nextbackupdate.oconv("DW") ^ ".txt";
					nextbackupfilename.converter("/", OSSLASH);
					nextbackupfileinfo = nextbackupfilename.osfile();

					// if the next backup file exists (going to be overwritten)
					// TODO this should be checked for all databases not just the first
					if (nextbackupfileinfo) {

						// add its space to the freespace
						freespace += nextbackupfileinfo.f(2);

						// print a warning if havent changed the backup media and not suppressed
						if (not(bakpars.f(13)) and (nextbackupfileinfo.f(2) == nextbackupdate - 7)) {

							// email a request to change usb
							// from 0600 to 1800 and not sent in the last 5.5 hours
							// usually 0600 1130 1700
							reminderhours = 5.5;
							var dummy3;
							var dummy4;
							var dummy5;
							var dummy6;
							call getdatetime(localdate, localtime, dummy3, dummy4, dummy5, dummy6);
							// if (localtime >= 21600 and localtime <= 64800) {
							if (localtime >= iconv("06:00", "MT") and localtime <= iconv("18:00", "MT")) {
								// only one email per installation
								//call osread(lastnote, "lastnote.cfg");
								if (not osread(lastnote, "lastnote.cfg")) {
									//null
								}
								if (lastnote.f(1) != localdate or (lastnote.f(2) < localtime - 3600 * reminderhours)) {
									//call oswrite(localdate ^ FM ^ localtime, "lastnote.cfg");
									if (not oswrite(localdate ^ FM ^ localtime, "lastnote.cfg")) {
										abort(lasterror());
									}

									// sendmail - if it fails, there will be an entry in the log
									toaddresses = bakpars.f(6);
									// never send reminders to exodus since we will get nagios warnings at 12:00
									toaddresses.replacer("backups@neosys.com", "");
									toaddresses = trim(toaddresses, ";");
									if (toaddresses) {
										remindern = (localtime - 21600).mod(3600 * reminderhours) + 1;
										subject	  = "EXODUS Backup Reminder";
										if (remindern > 1) {
											subject ^= " (" ^ remindern ^ ")";
										}
										body = "It is time to change the EXODUS backup media (e.g. USB Flash Drive)";
										if (localtime < 43200) {
											body(-1) = FM ^
													   "Please change it "
													   "before 12:00 midday today.";
										} else {
											body(-1) = FM ^
													   "Please change it "
													   "before 00:00 midnight tonight.";
										}
										printl(body);
										//body.replacer(FM, chr(13));
										// chr(13) no longer compatible with Postfix 3.6.4-1ubuntu1.2
										// sendmail.cpp converts FM to newline

										call sendmail(toaddresses, "", subject, body, "", "");
									}
								}
							}

							// warning if they have not changed the usb by noon
							if (localtime >= var("12:00").iconv("MT")) {
								description ^= " Change Backup!";
							}
						}
					}
				}
			}

			// accumulate size of existing backups that will be deleted and overwritten
			backupdrives(4, backupdriven) = backupdrives.f(4, backupdriven) + lastbackupsize;
		}

		// if index(description,'!',1) or nok or nhung or nmaint then
		if ((nok or nhung) or nmaint) {
			if (descriptions) {
				descriptions ^= ", ";
			}
			descriptions ^= description;
		}

nextdbasen:;
	}  // dbasen;

	// check for free space on backup drive(s)
	let nbackupdrives = backupdrives.f(1).fcount(VM);
	for (const int backupdriven : range(1, nbackupdrives)) {

		present = backupdrives.f(3, backupdriven);
		if (present) {

			backupdrive	   = backupdrives.f(1, backupdriven);
			freespace	   = backupdrives.f(2, backupdriven);
			lastbackupsize = backupdrives.f(4, backupdriven);
			deletingsize   = backupdrives.f(5, backupdriven);
			freespace += deletingsize;

			description = " Drive " ^ backupdrive ^ " Free:" ^ oconv(freespace, "[XBYTES,1]");

			// ensure 10% free space over last backup size
			// if we know the last backup size (this could fail if a NEW db is added)
			if (lastbackupsize > 0 and (freespace < lastbackupsize * 11 / 10)) {
				description ^= " only!!";
			}

			if (lastbackupsize) {
				description ^= " Req:" ^ oconv(lastbackupsize, "[XBYTES,1]");
			}

			if (description) {
				descriptions ^= " " ^ description;
			}
		}

	}  // backupdriven;
	// oswrite descriptions on 'DESCRIPS'
	if (descriptions.contains("!!") and status0123 < 2) {
		status0123 = 2;
	}
	if (descriptions.contains("!") and status0123 < 1) {
		status0123 = 1;
	}

	var hostdescriptions = "EXODUS ";

	// add exodus version for info
	{
		var versionpath = "general/version.dat";
		versionpath.converter("/", OSSLASH);
		//call osread(versionnote, tt);
		if (not osread(versionnote, versionpath)) {
			//null
		}
		versiondate = versionnote.trim().field(" ", 2, 3).iconv("D");
		versiondate = versiondate.oconv("D2/");
		versiondate = versiondate.last(2) ^ "/" ^ versiondate.first(5);
		hostdescriptions ^= "Ver" ^ versiondate ^ "-" ^ versionnote.field(" ", 1).field(":", 1, 2);

		// show local time
		hostdescriptions ^= " - At:" ^ time().oconv("MT");
	}

	// find max nusersperhour by type
	for (const int ii : range(1, 24)) {
		for (const int jj : range(1, 3)) {
			usertab(ii, jj) = usertab(ii, jj).fcount(VM);
			if (usertab(ii, jj) > usertab(25, jj)) {
				usertab(25, jj) = usertab(ii, jj);
			}
		}  // jj;
	}	   // ii;

	// list number of users (in last hour)
	hostdescriptions ^= " - Users:";
	hourn = 1;
	for (const int ii : range(1, 3)) {
		let hourlyusers = usertab(hourn, ii);
		if (hourlyusers) {
			anyusers = 1;
		}
		if (ii > 1) {
			hostdescriptions ^= "/";
		}
		hostdescriptions ^= hourlyusers;
	}  // ii;

	// list max number of users
	hostdescriptions ^= " - Max:";
	hourn = 25;
	for (const int ii : range(1, 3)) {
		//let tt = usertab(1, ii);
		if (ii > 1) {
			hostdescriptions ^= "/";
		}
		hostdescriptions ^= usertab(hourn, ii);
	}  // ii;

	// os description
	//call osgetenv("VER", osver);
	//hostdescriptions ^= " - " ^ osver;

	// cpu description
	//call osgetenv("CPU", cpudesc);
	nprocs = SYSTEM.f(9);
	//hostdescriptions ^= " - " ^ cpudesc ^ " x " ^ nprocs;
	hostdescriptions ^= " - nprocs:" ^ nprocs;

	ips = shell2("printf $(hostname -I | cut -d' ' -f 1)");
// gotip:
	if (not ips) {
		ips = "0.0.0.0";
	}
	if (ips) {
		ips.converter(FM, ",");
		nips = ips.fcount(",");
		// limit to 5 ips, replace middle ones with - to indicate suppressed
		maxips = 5;
		if (nips > maxips) {
			ips = ips.fieldstore(",", (maxips / 2).floor(), maxips - nips, "...");
		}
		hostdescriptions ^= " - " ^ ips;
		// hostdescriptions:=' - ':ips:' - '
	}

	// package the data for the monitor
	datax = "";

	// host passive check line
	// currently any exodus update indicates that the host is ok
	if (datax) {
		datax ^= chr(10);
	}
	datax ^= "PROCESS_HOST_CHECK_RESULT;" ^ installid ^ ";0;" ^ hostdescriptions;

	// service passive check line
	if (datax) {
		datax ^= chr(10);
	}
	datax ^= "PROCESS_SERVICE_CHECK_RESULT;" ^ installid ^ ";exodus;" ^ status0123 ^ ";" ^ descriptions;

	// request info to be sent to the monitor asynchronously - doesnt wait

	// request it to be done
	// currently just uses wget to http post the info in the background
	msg = "";
	if (cidx) {
		call monitor2b("WRITE", request, tempfilename, datax, msg);
	}

	// any error is going to be up front like error in parameters or missing wget
	// errors in hostnames and connectivity must be obtained
	// with a mode='READ' and the same tempfilename
	// report and prevent further checking/reporting for an hour
	// unless somehow forced
	if (msg) {
		monitordata(1) = currenttime.f(1) + 1 / 24.0;
		call sysmsg(msg);
	} else {
		// register checked at current time
		monitordata(1) = currenttime;
	}

	// NOTE and errors in hostnames and connectivity timeouts/response etc
	// can be obtained after timeout etc with a mode='READ' and same tempfilename
	// but how to know when ready (or timedout)?

	// update the monitor status
	monitordata.write(processes, monitorkey);

	// ///
	// exit:
	// ///
	call unlockrecord("PROCESSES", processes, monitorkey);

	return 0;
}

libraryexit()
