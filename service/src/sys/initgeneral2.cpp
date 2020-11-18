#include <exodus/library.h>
libraryinit()

#include <log2.h>
#include <readhostsallow.h>
#include <shell2.h>
#include <sysmsg.h>

#include <gen_common.h>

var tt;
var hosts;
var vn;
var conf;
var errors;
var xx;
var userid;
var usern;
var lastdate;

function main(in mode, io logtime) {
	//c sys in,io

	#include <general_common.h>

	call log2("*init.general2 " ^ mode.a(1), logtime);

	if (mode == "INSTALLNEWPASS") {

		//not used anywhere 2017/05/17

		//change the current user (EXODUS) password to something new
		//this is in the SYSTEM file' in EXODUS/EXODUS
		perform("PASSWORD3");

		//get access to the SYSTEM file here (in EXODUS/EXODUS)
		//also gets acceds to SYSTEM file in EXODUS2/EXODUS (as SYSTEM2)
		perform("SETSO");

		//get access to the SYSTEM file in EXODUS2/EXODUS (the build directory)
		//perform 'SETFILE ../EXODUS2/EXODUS SYSPROG,SALADS SYSTEM'

		//copy the local user over to the build directory
		perform("COPY SYSTEM " ^ USERNAME ^ " (O) TO: (QFILE)");

	} else if (mode == "CREATEALERTS") {

		call log2("*createalert currusers", logtime);

		if (not(tt.readv(DEFINITIONS, "INIT*CREATEALERT*CURRUSERS", 1))) {
			tt = "";
		}
		if (tt < 17203) {

			//to EXODUS only at the moment
			var cmd = "CREATEALERT CURRUSERS GENERAL CURRUSERS {} EXODUS (ROS)";

			//run once on first installation
			tt = cmd;
			tt.swapper("{}", "7:::::1");
			perform(tt);

			//run every 1st of the month
			tt = cmd;
			tt.swapper("{}", "7:1");
			perform(tt);

			var().date().write(DEFINITIONS, "INIT*CREATEALERT*CURRUSERS");
		}

	} else if (mode == "UPDATEIPNOS4EXODUS") {

		call log2("*update EXODUS-allowed ipnos from GBP file", logtime);

		//open 'GBP' to gbp else return
		//read hosts from gbp,'$HOSTS.ALLOW' else return
		call readhostsallow(hosts);

		//convert hosts.allow format to sv list of valid ip numbers or partial ip nos
		hosts.swapper("sshd:", "");
		hosts.converter(" ", "");
		var nn = hosts.count(FM) + 1;
		for (var ln = nn; ln >= 1; --ln) {
			hosts.r(ln, hosts.a(ln).field("#", 1));
		};//ln;

		//remove blank lines and convert fm to sm
		hosts.converter(FM, " ");
		hosts.trimmer();
		hosts.converter(" ", SVM);

		//remove any trailing . from 10. etc which is valid syntax for hosts.allow
		hosts.swapper("." ^ SVM, SVM);
		if (hosts[-1] == ".") {
			hosts.splicer(-1, 1, "");
		}

		//exodus can login from
		//1) 127 always
		//2) EXODUS office/vpn static ips (hardcoded in each exodus version)

		//exodus can also login from System Configuration ip ranges and numbers
		//default system configuration ips is standard lan ips "192.168 10 172"
		//3) standard LAN ips 192.168, 10, 172 RANGES BUT NOT FULLY FORMED IPS
		//4) Config File Fully formed WAN ips BUT NOT RANGES
		//5) NB the above 2 rules mean "*" for all is NOT allowed even if present

		//WARNING to disallow EXODUS login from outside the office via NAT router
		//(which makes the connection appear like a LAN access)
		//you MUST put the FULLY FORMED LAN IP of the router eg 192.168.1.1
		//in the System Configuration file (even if 192.168 is config for true LAN)
		//then EXODUS LOGINS FROM ANY *LISTED FULLY FORMED LAN IPS* WILL BE BLOCKED

		var configips = SYSTEM.a(39);
		if (configips == "") {
			configips = "192.168 10 172";
		}
		configips.converter(" ", SVM);
		configips.swapper(".*", "");
		nn = configips.count(SVM) + (configips ne "");
		for (var ii = nn; ii <= 1; ++ii) {
			var ipno = configips.a(1, 1, ii);
			if (ipno.field(".", 1) == "10") {
				{}
			} else if (ipno.field(".", 1) == "172") {
			} else if (ipno.field(".", 1, 2) == "192.168") {
			} else {
				//delete all WAN (non-LAN) ranges and allow only fully specced WAN ips
				if (ipno.count(SVM) ne 3) {
					configips.remover(1, 1, ii);
				}
			}
		};//ii;

		//allow exodus login from 127.*, LAN and any config default ips
		//although any full 4 part LAN ips in configips will be BLOCKED in LISTEN2
		//since they are deemed to be inwardly NATTING routers (see LISTEN2)
		hosts.inserter(1, 1, 1, "127.0.0.1");
		hosts.inserter(1, 1, 2, "127");
		hosts.inserter(1, 1, 3, "192.168");
		hosts.inserter(1, 1, 4, "172");
		hosts.inserter(1, 1, 5, configips);

		hosts.converter(SVM, " ");
		hosts.trimmer();

		hosts.write(DEFINITIONS, "IPNOS*EXODUS");

	} else if (mode == "GETENV") {

		call log2("*get environment", logtime);

		//nb will NOT overwrite any manual entries in SYSTEM.CFG
		var environment = osgetenv();
		environment.converter("\r\n", FM);
		var nenv = environment.count(FM) + 1;
		for (var ii = 1; ii <= nenv; ++ii) {
			var enventry = environment.a(ii);
			if (enventry) {
				tt = enventry.field("=", 1);
				if (not(SYSTEM.a(12).locate(tt,vn))) {
					SYSTEM.r(12, vn, tt);
					SYSTEM.r(13, vn, enventry.field("=", 2, 99999));
				}
			}
		};//ii;

	} else if (mode == "FIXURLS") {

		call log2("*condition the http links", logtime);

		//remove any obvious page addresses
		//ensure ends in slash
		var baselinks = SYSTEM.a(114);
		var baselinkdescs = SYSTEM.a(115);
		if (not baselinks) {
			var baselink = "System Configuration File";
		}
		if (not baselinkdescs) {
			baselinkdescs = "Pending Configuration";
		}
		var nlinks = baselinks.count(VM) + (baselinks ne "");
		for (var linkn = 1; linkn <= nlinks; ++linkn) {
			tt = baselinks.a(1, linkn);
			if (tt) {
				var tt2 = (field2(tt, "/", -1)).lcase();
				if (tt2.substr(1,4).index(".htm")) {
					tt.splicer(-tt2.length(), tt2.length(), "");
				}
				if (not(var("\\/").index(tt[-1]))) {
					tt ^= "/";
				}
				baselinks.r(1, linkn, tt);
			}
		};//linkn;
		SYSTEM.r(114, baselinks);
		SYSTEM.r(115, baselinkdescs);

	} else if (mode == "COMPRESSLOGS") {

		//use WINDOWS COMPACT to save disk space
		if (VOLUMES) {

			call log2("*once off call to windows COMPACT command on ..LOGS", logtime);

			if (not(conf.osread("..\\LOGS\\exodus.ini"))) {
				conf = "";
			}
			if (not(conf.index("NOTREQ"))) {

				var cmd = "compact /C /S /F ..\\LOGS ..\\LOGS\\*.*";
				var(cmd ^ " DONE,NOTREQ").oswrite("..\\LOGS\\exodus.ini");

				printl(cmd);
				printl(shell2(cmd, errors));
				printl(errors);

			}

		}

		call log2("*compress logs with gzip", logtime);
		var dbcode = SYSTEM.a(17);
		var curryear = var().date().oconv("D").substr(-4,4);
		var minyear = 2000;
		for (var year = curryear - 2; year >= minyear; --year) {

			//dir='..\logs\':dbcode:'\':year
			//filenames='..\logs\':dbcode:'\':year:'\*.xml'
			var filenamesx = "../logs/" ^ dbcode ^ "/" ^ year ^ "/*.xml";
			filenamesx.converter("/", OSSLASH);

			tt = oslistf(filenamesx);
			///BREAK;
			if (not tt) break;

			filenamesx.converter("\\", "/");

			var cygwinbin = SYSTEM.a(50);
			var cmd = cygwinbin ^ "gzip " ^ filenamesx;

			call log2("*compress logs with gzip to .gz" ^ filenamesx, logtime);
			printl(cmd);
			call shell2(cmd, xx);

		};//year;

	} else if (mode == "UPDATEUSERS") {

		call log2("*add keys and ipnos to users", logtime);

		var users;
		if (not(users.open("USERS", ""))) {
			return 0;
		}
		clearselect();
		select(users);
nextuser:
		if (readnext(userid)) {
			if (userid[1] == "%") {
				goto nextuser;
			}
			var userx;
			if (not(userx.read(users, userid))) {
				goto nextuser;
			}
			var origuser = userx;
			if (not(SECURITY.a(1).locate(userid,usern))) {
				goto nextuser;
			}
			userx.r(40, SECURITY.a(6, usern));
			userx.r(41, SECURITY.a(2, usern));
			origuser.r(40, origuser.a(40));
			origuser.r(41, origuser.a(41));
			if (userx ne origuser) {
				userx.write(users, userid);
				//user<1>=userid
				//write user on users,'%':userid:'%'
			}
			goto nextuser;
		}

	} else if (mode == "TRIMREQUESTLOG") {

		call log2("*trim requestlog", logtime);

		//only run on saturdays and only run once per day per installation
		if ((var().date() - 1) % 7 + 1 ne 6) {
			return 0;
		}

		//exclusive get and update the last date run otherwise skip
		if (not(DEFINITIONS.lock( "TRIMREQUESTLOG"))) {
			return 0;
		}
		if (not(lastdate.osread("reqlog.cfg"))) {
			lastdate = "";
		}
		var(var().date()).oswrite("reqlog.cfg");
		DEFINITIONS.unlock( "TRIMREQUESTLOG");

		//dont run twice on same day
		if (lastdate == var().date()) {
			return 0;
		}

		perform("TRIMREQUESTLOG");

	} else if (mode == "REORDERDBS") {

		call log2("*reorder databases", logtime);

		//only run once per installation
		if (tt.osread("reorder.cfg")) {
			return 0;
		}
		var(var().date()).oswrite("reorder.cfg");

		perform("WINDOWSTUB DEFINITION.SUBS REORDERDBS");

	} else if (mode.a(1) == "LASTLOGWARNING") {

		var lastlog = mode.field(FM, 2, 999);
		lastlog = trim(lastlog, FM);
		//first word is logfilename
		lastlog = lastlog.field(" ", 2, 999);
		if (not lastlog) {
			return 0;
		}

		if (lastlog.index("UPGRADEVERBS")) {
			return 0;
		}

		//email anything unexpected
		if (not(lastlog.index("Quitting."))) {
			if (not(lastlog.index("*chain to NET AUTO"))) {
				call sysmsg("Unexpected last log entry||" ^ lastlog, "Unexpected Log", "EXODUS");
			}
		}

	} else if (mode == "OSCLEANUP") {
		//initdir '..\VDM*.TMP'
		//temps=dirlist()
		if (VOLUMES) {
			var temps = oslistf("..\\vdm*.tmp");
			var ntemps = temps.count(FM) + (temps ne "");
			for (var tempn = 1; tempn <= ntemps; ++tempn) {
				("..\\" ^ temps.a(tempn)).osdelete();
			};//tempn;
		}

	} else {

		call log2(mode.quote() ^ " is invalid in init.general2", logtime);

	}

	return 0;
}

libraryexit()
