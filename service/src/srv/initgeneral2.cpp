#include <exodus/library.h>
libraryinit()

#include <log2.h>
#include <menusubs.h>
#include <readhostsallow.h>
#include <shell2.h>
#include <sysmsg.h>

#include <service_common.h>

#include <srv_common.h>

//var tt;
var hosts;
var vn;
var conf;
var errors;
//var xx;
var userid;
var usern;
var lastdate;
var menutx;
var oldmenu;

function main(in mode, io logtime, in menu) {

	call log2("*init.general2 " ^ mode.f(1), logtime);

	if (mode == "INSTALLNEWPASS") {

		// not used anywhere 2017/05/17

		// change the current user (EXODUS) password to something new
		// this is in the SYSTEM file' in EXODUS/EXODUS
		perform("PASSWORD3");

		// get access to the SYSTEM file here (in EXODUS/EXODUS)
		// also gets access to SYSTEM file in EXODUS2/EXODUS (as SYSTEM2)
		perform("SETSO");

		// get access to the SYSTEM file in EXODUS2/EXODUS (the build directory)
		// perform 'SETFILE ../EXODUS2/EXODUS SYSPROG,SALADS SYSTEM'

		// copy the local user over to the build directory
		perform("COPY SYSTEM " ^ USERNAME ^ " (O) TO: (QFILE)");

	} else if (mode == "CREATEALERTS") {

		call log2("*createalert currusers", logtime);

		var lastrun;
		if (not lastrun.readf(DEFINITIONS, "INIT*CREATEALERT*CURRUSERS", 1)) {
			lastrun = "";
		}
		if (lastrun < 17203) {

			// to EXODUS only at the moment
			let cmd = "CREATEALERT CURRUSERS GENERAL CURRUSERS {} EXODUS (ROS)";

			// run once on first installation
			let cmd2 = cmd.replace("{}", "7:::::1");
			perform(cmd2);

			// run every 1st of the month
			let cmd3 = cmd.replace("{}", "7:1");
			perform(cmd3);

			date().write(DEFINITIONS, "INIT*CREATEALERT*CURRUSERS");
		}

	} else if (mode == "UPDATEIPNOS4EXODUS") {

		call log2("*update EXODUS-allowed ipnos from GBP file", logtime);

		// open 'GBP' to gbp else return
		// read hosts from gbp,'$HOSTS.ALLOW' else return
		call readhostsallow(hosts);

		// convert hosts.allow format to sv list of valid ip numbers or partial ip nos
		hosts.replacer("sshd:", "");
		hosts.converter(" ", "");
		var nn = hosts.fcount(FM);
		//for (var ln = nn; ln >= 1; --ln) {
		for (let ln : reverse_range(1, nn)) {
			hosts(ln) = hosts.f(ln).field("#", 1);
		}  // ln;

		// remove blank lines and convert fm to sm
		hosts.converter(FM, " ");
		hosts.trimmer();
		hosts.converter(" ", SM);

		// remove any trailing . from 10. etc which is valid syntax for hosts.allow
		hosts.replacer("." ^ SM, SM);
		if (hosts.ends(".")) {
			hosts.popper();
		}

		// exodus can login from
		// 1) 127 always
		// 2) EXODUS office/vpn static ips (hardcoded in each exodus version)

		// exodus can also login from System Configuration ip ranges and numbers
		// default system configuration ips is standard lan ips "192.168 10 172"
		// 3) standard LAN ips 192.168, 10, 172 RANGES BUT NOT FULLY FORMED IPS
		// 4) Config File Fully formed WAN ips BUT NOT RANGES
		// 5) NB the above 2 rules mean "*" for all is NOT allowed even if present

		// WARNING to disallow EXODUS login from outside the office via NAT router
		// (which makes the connection appear like a LAN access)
		// you MUST put the FULLY FORMED LAN IP of the router eg 192.168.1.1
		// in the System Configuration file (even if 192.168 is config for true LAN)
		// then EXODUS LOGINS FROM ANY *LISTED FULLY FORMED LAN IPS* WILL BE BLOCKED

		var configips = SYSTEM.f(39);
		if (configips == "") {
			configips = "192.168 10 172";
		}
		configips.converter(" ", SM);
		configips.replacer(".*", "");
		nn = configips.fcount(SM);
		for (const var ii : range(nn, 1)) {
			let ipno = configips.f(1, 1, ii);
			if (ipno.field(".", 1) == "10") {
				// OK
			} else if (ipno.field(".", 1) == "172") {
				// OK
			} else if (ipno.field(".", 1, 2) == "192.168") {
				// OK
			} else {
				// Delete all WAN (non-LAN) ranges and allow only fully specced WAN ips
				if (ipno.count(SM) != 3) {
					configips.remover(1, 1, ii);
				}
			}
		}  // ii;

		// allow exodus login from 127.*, LAN and any config default ips
		// although any full 4 part LAN ips in configips will be BLOCKED in LISTEN2
		// since they are deemed to be inwardly NATTING routers (see LISTEN2)
		hosts.inserter(1, 1, 1, "127.0.0.1");
		hosts.inserter(1, 1, 2, "127");
		hosts.inserter(1, 1, 3, "192.168");
		hosts.inserter(1, 1, 4, "172");
		hosts.inserter(1, 1, 5, configips);

		hosts.converter(SM, " ");
		hosts.trimmer();

		hosts.write(DEFINITIONS, "IPNOS*EXODUS");

	} else if (mode == "GETENV") {

		call log2("*get osenv", logtime);

		// nb will NOT overwrite any manual entries in SYSTEM.CFG
		let osenv = osgetenv().convert("\r\n", _FM _FM);
		// 		let nenv = osenv.fcount(FM);
		// 		for (const var ii : range(1, nenv)) {
		// 			let enventry = osenv.f(ii);
		for (let enventry : osenv) {
			if (not enventry)
				continue;
			// osenv being global is probably the same for all threads
			// unless they start after the process global env is changed
			if (THREADNO == 1)
				printl(THREADNO ^ ":", enventry);
			let envkey = enventry.field("=", 1);
			let envval = enventry.field("=", 2, 999999);
			if (not SYSTEM.f(12).locate(envkey, vn)) {
				SYSTEM(12, vn) = envkey;
				SYSTEM(13, vn) = envval;
			}
		}  // ii;

	} else if (mode == "FIXURLS") {

		call log2("*condition the http links", logtime);

		// remove any obvious page addresses
		// ensure ends in slash
		var baselinks	  = SYSTEM.f(114);
		var baselinkdescs = SYSTEM.f(115);
		if (not baselinks) {
			let baselink = "System Configuration File";
		}
		if (not baselinkdescs) {
			baselinkdescs = "Pending Configuration";
		}
		let nlinks = baselinks.fcount(VM);
		for (const var linkn : range(1, nlinks)) {
			var baselink = baselinks.f(1, linkn);
			if (baselink) {
				// TODO This code looks strange, perhaps buggy
				let lastpart = (field2(baselink, "/", -1)).lcase();
				if (lastpart.first(4).contains(".htm")) {
					baselink.cutter(-lastpart.len());
				}
				if (not(baselink.ends("/")) and not(baselink.ends("\\"))) {
					baselink ^= "/";
				}
				baselinks(1, linkn) = baselink;
			}
		}  // linkn;
		SYSTEM(114) = baselinks;
		SYSTEM(115) = baselinkdescs;

	} else if (mode == "COMPRESSLOGS") {

		// use WINDOWS COMPACT to save disk space
//		if (VOLUMES) {
//
//			call log2("*once off call to windows COMPACT command on ..LOGS", logtime);
//
//			if (not conf.osread("..\\LOGS\\exodus.ini")) {
//				conf = "";
//			}
//			if (not conf.contains("NOTREQ")) {
//
//				let cmd = "compact /C /S /F ..\\LOGS ..\\LOGS\\*.*";
//				var(cmd ^ " DONE,NOTREQ").oswrite("..\\LOGS\\exodus.ini");
//
//				printl(cmd);
//				printl(shell2(cmd, errors));
//				printl(errors);
//			}
//		}

		call log2("*compress logs with gzip", logtime);
		var	 dbcode	  = SYSTEM.f(17);
		var	 curryear = date().oconv("DY");
		var	 minyear  = 2000;
		//for (var year = curryear - 2; year >= minyear; --year) {
		for (let year : reverse_range(minyear, curryear - 2)) {

			// dir='..\logs\':dbcode:'\':year
			// filenames='..\logs\':dbcode:'\':year:'\*.xml'
			var filenamesx = "../logs/" ^ dbcode ^ "/" ^ year ^ "/*.xml";
			filenamesx.converter("/", OSSLASH);

			if (not oslistf(filenamesx))
				break;

			filenamesx.converter("\\", "/");

			let cygwinbin = SYSTEM.f(50);
			let cmd		  = cygwinbin ^ "gzip " ^ filenamesx;

			call log2("*compress logs with gzip to .gz" ^ filenamesx, logtime);
			printl(cmd);
			call shell2(cmd);

		}  // year;

	} else if (mode == "UPDATEUSERS") {

		call log2("*add keys and ipnos to users", logtime);

		var users;
		if (not users.open("USERS", "")) {
			return 0;
		}
		clearselect();
		select(users);
nextuser:
		if (readnext(userid)) {
			if (userid.starts("%")) {
				goto nextuser;
			}
			var userx;
			if (not userx.read(users, userid)) {
				goto nextuser;
			}
			var origuser = userx;
			if (not SECURITY.f(1).locate(userid, usern)) {
				goto nextuser;
			}
			userx(40)	 = SECURITY.f(6, usern);
			userx(41)	 = SECURITY.f(2, usern);
			origuser(40) = origuser.f(40);
			origuser(41) = origuser.f(41);
			if (userx != origuser) {
				userx.write(users, userid);
				// user<1>=userid
				// write user on users,'%':userid:'%'
			}
			goto nextuser;
		}

	} else if (mode == "TRIMREQUESTLOG") {

		call log2("*trim requestlog", logtime);

		// only run on saturdays and only run once per day per installation
		// if ((date() - 1).mod(7) + 1 != 6) {
		if (date().oconv("DW") != 6) {
			return 0;
		}

		// exclusive get and update the last date run otherwise skip
		if (not DEFINITIONS.lock("TRIMREQUESTLOG")) {
			return 0;
		}
		if (not lastdate.osread("reqlog.cfg")) {
			lastdate = "";
		}
		//var(date()).oswrite("reqlog.cfg");
		if (not var(date()).oswrite("reqlog.cfg")) {
			loglasterror();
		}
		DEFINITIONS.unlock("TRIMREQUESTLOG");

		// dont run twice on same day
		if (lastdate == date()) {
			return 0;
		}

		perform("TRIMREQUESTLOG");

	} else if (mode == "REORDERDBS") {

		call log2("*reorder databases", logtime);

		// only run once per installation
		if (osfile("reorder.cfg")) {
			return 0;
		}

		//var(date()).oswrite("reorder.cfg");
		if (not var(date()).oswrite("reorder.cfg")) {
			loglasterror();
		}

		perform("WINDOWSTUB DEFINITION.SUBS REORDERDBS");

	} else if (mode.f(1) == "LASTLOGWARNING") {

		var lastlog = mode.field(FM, 2, 999);
		lastlog		= trim(lastlog, FM);
		// first word is logfilename
		lastlog = lastlog.field(" ", 2, 999);
		if (not lastlog) {
			return 0;
		}

		if (lastlog.contains("UPGRADEVOC")) {
			return 0;
		}

		// email anything unexpected
		if (not lastlog.contains("Quitting.")) {
			if (not lastlog.contains("*chain to NET AUTO")) {
				call sysmsg("Unexpected last log entry||" ^ lastlog, "Unexpected Log", "EXODUS");
			}
		}

	} else if (mode == "OSCLEANUP") {
		// initdir '..\VDM*.TMP'
		// temps=dirlist()
//		if (VOLUMES) {
//			let temps  = oslistf("..\\vdm*.tmp");
//			let ntemps = temps.fcount(FM);
//			for (const var tempn : range(1, ntemps)) {
//				("..\\" ^ temps.f(tempn)).osremove();
//			}  // tempn;
//		}

	} else if (mode == "MAKEMENU") {

		call menusubs("INITMENUS", menutx);

		// MODULE MENUS FIRST ON RIGHT SIDE
		// ////////////////////////////////
		menutx ^= FM ^ menu;

		// SUPPORT MENU
		// ////////////

		call menusubs("ADDMENU", menutx, "_Support");

		var	 item = "_Authorisation File";
		var	 href = "../exodus/authorisation.htm";
		call menusubs("ADDITEM", menutx, item, href);

		item = "System Con_figuration File";
		href = "../exodus/systemconfiguration.htm";
		call menusubs("ADDITEM", menutx, item, href);

//		if (VOLUMES) {
//			item = "_Backup";
//			href = "../exodus/backup.htm";
//			call menusubs("ADDITEM", menutx, item, href);
//		}

		item = "L_og";
		href = "../exodus/log.htm";
		call menusubs("ADDITEM", menutx, item, href);

		item = "Re_questLog";
		// style="display: none"
		// id="exodussupportmenuitem1"
		href = "../exodus/requestlog.htm";
		call menusubs("ADDITEM", menutx, item, href);
		// <br style="display: none" id="exodussupportmenuitem2" />

		item = "_Usage Statistics";
		href = "../exodus/usagestatistics.htm";
		call menusubs("ADDITEM", menutx, item, href);

		item = "_Create Database";
		href = "../exodus/createdatabase.htm";
		call menusubs("ADDITEM", menutx, item, href);

		call menusubs("ADDSEP", menutx);

//		if (VOLUMES) {
//			item		= "List of Database _Processes";
//			let onclick = "javascript:openwindow_sync('EXECUTE|rGENERAL|rLISTPROCESSES');return false";
//			// backslash
//			onclick.converter("|", chr(92));
//			call menusubs("ADDITEM", menutx, item, href, onclick);
//		}

		item		= "_List of Documents in Use";
		var onclick = "javascript:openwindow_sync('EXECUTE|rGENERAL|rLISTLOCKS');return false";
		// backslash
		onclick.converter("|", chr(92));
		call menusubs("ADDITEM", menutx, item, href, onclick);

//		if (VOLUMES) {
//			item = "_Stop/Restart EXODUS Service";
//			href = "../exodus/stopservice.htm";
//			call menusubs("ADDITEM", menutx, item, href);
//		}

		call menusubs("ENDMENU", menutx);

		// HELP MENU
		// /////////

		call menusubs("ADDMENU", menutx, "_Help");

		item = "_What's new in EXODUS";
		href = "javascript:window.location.assign((typeof gusername!='undefined'&&gusername=='EXODUS'&&confirm('EXODUS only option|n|nChange Log File=OK|nWhats New Report=Cancel'))?'../exodus/changelog.htm':'../exodus/whatsnew.htm')";
		// backslash
		href.converter("|", chr(92));
		call menusubs("ADDITEM", menutx, item, href);

		item = "_User Details";
		href = "../exodus/users.htm";
		call menusubs("ADDITEM", menutx, item, href);

		item = "_Email </u>Users";
		href = "../exodus/emailusers.htm";
		call menusubs("ADDITEM", menutx, item, href);

		item = "_Keyboard Shortcuts";
		href = "../exodus/helpkeyboard.htm";
		call menusubs("ADDITEM", menutx, item, href);

		item = "Browser _Reset";
		href = "http://www.neosys.com/ie.htm";
		call menusubs("ADDITEM", menutx, item, href);

		item = "System Speed _Test";
		href = "../exodus/test.htm";
		call menusubs("ADDITEM", menutx, item, href);

		item = "EXODUS _Help Desk";
		href = "http://www.neosys.com/help";
		call menusubs("ADDITEM", menutx, item, href);

		item	= "_About";
		onclick = "javascript:displayresponsedata_sync('EXECUTE|rGENERAL|rABOUT');return false";
		// backslash
		onclick.converter("|", chr(92));
		call menusubs("ADDITEM", menutx, item, href, onclick);

		// this element at the end determines when the menu is fully loaded
		menutx ^= FM ^ "<div id=\"menucompleted\"></div>";

		call menusubs("ENDMENU", menutx);

		// <!-- </div> -->

		call menusubs("EXITMENUS", menutx);

		menutx.replacer(FM, _EOL);

		let menuosfilename = "../data/menu.htm";
		if (not oldmenu.osread(menuosfilename)) {
			oldmenu = "";
		}
		if (menutx != oldmenu) {
			call log2("Updating " ^ menuosfilename, logtime);
			//var(menutx).oswrite(menuosfilename);
			if (not var(menutx).oswrite(menuosfilename)) {
				abort(lasterror());
			}
		}

	} else {

		call log2(mode.quote() ^ " is invalid in init.general2", logtime);
	}

	return 0;
}

libraryexit()
