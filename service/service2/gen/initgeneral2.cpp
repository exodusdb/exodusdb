#include <exodus/library.h>
libraryinit()

#include <log2.h>
#include <shell2.h>
#include <sysmsg.h>

#include <gen.h>

var tt;
var vn;
var conf;
var errors;
var xx;
var userid;
var usern;
var lastdate;

function main(in mode, io logtime) {
	//c gen in,io

	call log2("*init.general2 " ^ mode.a(1), logtime);

	if (mode == "INSTALLNEWPASS") {

		//not used anywhere 2017/05/17

		//change the current user (NEOSYS) password to something new
		//this is in the SYSTEM file' in NEOSYS\NEOSYS
		perform("PASSWORD3");

		//get access to the SYSTEM file here (in NEOSYS\NEOSYS)
		//also gets acceds to SYSTEM file in NEOSYS2\NEOSYS (as SYSTEM2)
		perform("SETSO");

		//get access to the SYSTEM file in NEOSYS2\NEOSYS (the build directory)
		//perform 'SETFILE ../NEOSYS2/NEOSYS SYSPROG,SALADS SYSTEM'

		//copy the local user over to the build directory
		perform("COPY SYSTEM " ^ USERNAME ^ " (O) TO: (QFILE)");

	} else if (mode == "CREATEALERTS") {

		call log2("*createalert currusers", logtime);

		var tt;
		if (not(tt.readv(DEFINITIONS, "INIT*CREATEALERT*CURRUSERS", 1))) {
			tt = "";
		}
		if (tt < 17203) {

			//to NEOSYS only at the moment
			var cmd = "CREATEALERT CURRUSERS GENERAL CURRUSERS {} NEOSYS (ROS)";

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

	} else if (mode == "UPDATEIPNOS4NEOSYS") {

		call log2("*update NEOSYS-allowed ipnos from GBP file", logtime);

		var gbp;
		if (not(gbp.open("GBP", ""))) {
			return 0;
		}
		var hosts;
		if (not(hosts.read(gbp, "$HOSTS.ALLOW"))) {
			return 0;
		}

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

		//neosys can login from
		//1) 127 always
		//2) NEOSYS office/vpn static ips (hardcoded in each neosys version)

		//neosys can also login from System Configuration ip ranges and numbers
		//default system configuration ips is standard lan ips "192.168 10 172"
		//3) standard LAN ips 192.168, 10, 172 RANGES BUT NOT FULLY FORMED IPS
		//4) Config File Fully formed WAN ips BUT NOT RANGES
		//5) NB the above 2 rules mean "*" for all is NOT allowed even if present

		//WARNING to disallow NEOSYS login from outside the office via NAT router
		//(which makes the connection appear like a LAN access)
		//you MUST put the FULLY FORMED LAN IP of the router eg 192.168.1.1
		//in the System Configuration file (even if 192.168 is config for true LAN)
		//then NEOSYS LOGINS FROM ANY *LISTED FULLY FORMED LAN IPS* WILL BE BLOCKED

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
					configips.eraser(1, 1, ii);
				}
			}
		};//ii;

		//allow neosys login from 127.*, LAN and any config default ips
		//although any full 4 part LAN ips in configips will be BLOCKED in LISTEN2
		//since they are deemed to be inwardly NATTING routers (see LISTEN2)
		hosts.inserter(1, 1, 1, "127.0.0.1");
		hosts.inserter(1, 1, 2, "127");
		hosts.inserter(1, 1, 3, "192.168");
		hosts.inserter(1, 1, 4, "172");
		hosts.inserter(1, 1, 5, configips);

		hosts.converter(SVM, " ");
		hosts.trimmer();

		hosts.write(DEFINITIONS, "IPNOS*NEOSYS");

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
				if (not(SYSTEM.a(12).locateusing(tt, VM, vn))) {
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
				var tt2 = (field2(tt, "/", -1)).ucase();
				if (tt2.index(".HTM", 1)) {
					tt.splicer(-tt2.length(), tt2.length(), "");
				}
				if (not(var("\\/").index(tt[-1], 1))) {
					tt ^= "/";
				}
				baselinks.r(1, linkn, tt);
			}
		};//linkn;
		SYSTEM.r(114, baselinks);
		SYSTEM.r(115, baselinkdescs);

		} else if (mode == "COMPRESSLOGS") {

		call log2("*once off call to windows COMPACT command on ..LOGS", logtime);

		if (not(conf.osread("..\\LOGS\\neosys.ini"))) {
			conf = "";
		}
		if (not(conf.index("NOTREQ", 1))) {

			var cmd = "compact /C /S /F ..\\LOGS ..\\LOGS\\*.*";
			var(cmd ^ " DONE,NOTREQ").oswrite("..\\LOGS\\neosys.ini");

			printl(cmd);
			printl(shell2(cmd, errors));
			printl(errors);

		}

		call log2("*compress logs with gzip", logtime);
		var dbcode = SYSTEM.a(17);
		var curryear = var().date().oconv("D").substr(-4,4);
		var minyear = 2000;
		for (var year = curryear - 2; year >= minyear; --year) {
			var dir = "..\\LOGS\\" ^ dbcode ^ "\\" ^ year;
			var filenamesx = "..\\LOGS\\" ^ dbcode ^ "\\" ^ year ^ "\\*.XML";

			// initdir filenames
			//while dirlist()
			tt = oslistf(filenamesx);
		///BREAK;
		if (not tt) break;;
			filenamesx.converter("\\", "/");
			var cygwinbin = SYSTEM.a(50);

			var cmd = cygwinbin ^ "gzip " ^ filenamesx;
			call log2("*compress logs with gzip " ^ filenamesx, logtime);
			printl(cmd);
			call shell2(cmd, xx);

		};//year;

	} else if (mode == "UPDATEUSERS") {

		call log2("*add keys and ipnos to users", logtime);

		var users;
		if (not(users.open("USERS", ""))) {
			return 0;
		}
		var().clearselect();
		users.select();
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
			if (not(SECURITY.a(1).locateusing(userid, VM, usern))) {
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

	/*now done in neosys.net/web.config;
		case mode='CONFIGURE IIS';

			//iis cache default to be 1 hour
			cmd='%systemroot%\system32\inetsrv\APPCMD set config /section:staticContent /clientCache.cacheControlMode:UseMaxAge /clientCache.cacheControlMaxAge:';
			cmd:='0:01:00:00';
	*/

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
		if (not(lastdate.osread("REQLOG.CFG"))) {
			lastdate = "";
		}
		var(var().date()).oswrite("REQLOG.CFG");
		DEFINITIONS.unlock( "TRIMREQUESTLOG");

		//dont run twice on same day
		if (lastdate == var().date()) {
			return 0;
		}

		perform("TRIMREQUESTLOG");

	} else if (mode == "REORDERDBS") {

		call log2("*reorder databases", logtime);

		//only run once per installation
		if (tt.osread("REORDER.CFG")) {
			return 0;
		}
		var(var().date()).oswrite("REORDER.CFG");

		perform("WINDOWSTUB DEFINITION.SUBS REORDERDBS");

	} else if (mode.a(1) == "LASTLOGWARNING") {

		var lastlog = mode.field(FM, 2, 999);
		lastlog = trim(lastlog, FM);
		//first word is logfilename
		lastlog = lastlog.field(" ", 2, 999);
		if (not lastlog) {
			return 0;
		}

		if (lastlog.index("UPGRADEVERBS", 1)) {
			return 0;
		}

		//email anything unexpected
		if (not(lastlog.index("Quitting.", 1))) {
			if (not(lastlog.index("*chain to NET AUTO", 1))) {
				call sysmsg("Unexpected last log entry||" ^ lastlog, "Unexpected Log", "NEOSYS");
			}
		}

	} else if (mode == "OSCLEANUP") {
		//initdir '..\VDM*.TMP'
		//temps=dirlist()
		var temps = oslistf("..VDM*.TMP");
		var ntemps = temps.count(FM) + (temps ne "");
		for (var tempn = 1; tempn <= ntemps; ++tempn) {
			var("..\\" ^ temps.a(tempn)).osdelete();
		};//tempn;

	} else {

		call log2(DQ ^ (mode ^ DQ) ^ " is invalid in init.general2", logtime);

	}
//L1865:
	return 0;

}


libraryexit()
