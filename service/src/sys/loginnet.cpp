#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <initcompany.h>
#include <addcent4.h>
#include <changelogsubs.h>

#include <sys_common.h>

var menuid;
var taskn;//num
var xx;
var menun2;
var compcodex;
var paramrec;

function main(in dataset, in username, io cookie, io msg, io authcompcodes) {
	//c sys in,in,io,io,io

	//this is a special login routine called from LISTEN2
	//declare function validcode1
	#include <system_common.h>
	//$insert abp,common
	//$insert bp,agency.common
	cookie = "";
	authcompcodes = "";

	//evade c++ unused variable warning
	if (not(dataset.unassigned())) {
		{}
	}

	//return allowable menus if legacy menu in users file
	//legacy menus from pre 03JUN10 code per user settings on Authorisation File
	//assignment of locks on menus and/or clearing userprivs<3>
	//removes legacy privileges
	var oldmenus = "";
	var users;
	if (users.open("USERS", "")) {
		if (menuid.readv(users, username, 34)) {
			if (menuid) {
				var menufile;
				if (menufile.open("ADMENUS", "")) {
readmenu:
					if (not(oldmenus.readv(menufile, menuid, 6))) {
						{}
					}
				} else {
					if (menufile.open("MENUS", "")) {
						goto readmenu;
					}
				}
			}
		}
	}
	//convert to new menu codes
	oldmenus.swapper("ACCS", "FINANCE");
	oldmenus.swapper("ANALMENU", "ANALYSIS");
	oldmenus.swapper("ANALYSIS.INVOICES", "ANALYSIS");
	oldmenus.swapper("ADJOB", "JOBS");
	oldmenus.swapper("GENERAL", "SUPPORT");
	//convert '.' to '_' in oldmenus

	//all menus - not in order
	var menus = "SUPPORT";
	if (APPLICATION eq "ACCOUNTS" or APPLICATION eq "ADAGENCY") {
		menus(1, -1) = "FINANCE";
	}

	if (APPLICATION eq "ADAGENCY") {

		menus(1, -1) = "ANALYSIS";

		//MEDIA is always a candidate since it is difficult
		//to work out exc
		menus(1, -1) = "MEDIA";

		//JOBS menu is only a candidate if they can access jobs
		//unfortunately media probably have job access to attach jobs
		if (authorised("JOB ACCESS", msg, "")) {
			menus(1, -1) = "JOBS";
		}

		//TIMESHEETS menu is only a candidate if then can access timesheets
		if (authorised("TIMESHEET ACCESS", msg, "")) {
			menus(1, -1) = "TIMESHEETS";
		}

	}

	//reduce the menus to a list of authorised menus
	var nmenus = menus.count(VM) + 1;
	//work backwards because we are deleting
	for (var menun = nmenus; menun >= 1; --menun) {
		var menu = menus.a(1, menun).ucase();
		if (menu) {
			var menutask = "MENU " ^ menu;
			if (not(SECURITY.a(10).locate(menutask, taskn))) {
				taskn = 0;
			}

			//specifically locked or no legacy menus - only allow if authorised
			if (oldmenus eq "" or ((menun and SECURITY.a(11, taskn).length()))) {
				if (not(authorised(menutask, xx))) {
deleteit:
					if (menus.locate(menu, menun2)) {
						menus.remover(1, menun2);
						nmenus -= 1;
					}
				}

			//not specifically locked - only allow if in legacy menus
			} else {
				if (not(oldmenus.locate(menu, xx))) {
					goto deleteit;
				}
			}

		}
	} //menun;

	//if they have MENU then ANALYSIS=MEDIAANALYSIS
	if (menus.index("MEDIA")) {
		menus.swapper("ANALYSIS", "MEDIAANALYSIS");
	}

	//check there is at least one menu
	if (not menus) {
		msg = "Error: You are not authorised to access any menus";
		return 0;
	}

	//everybody gets the HELP menu
	menus(1, -1) = "HELP";

	//determine the company

	var compcode = "";

	var allcomps;
	if (not(allcomps.read(sys.companies, "%RECORDS%"))) {
		select(sys.companies);
		allcomps = "";
nextcomp:
		if (readnext(compcodex)) {
			allcomps(-1) = compcodex;
			goto nextcomp;
		}
	}

	//build a list of authorised companies
	var compcodes = "";
	if (APPLICATION eq "ADAGENCY") {
		for (const var ii : range(1, 9999)) {
			compcode = allcomps.a(ii);
			///BREAK;
			if (not compcode) break;
			//if validcode2(compcode,'','',xx,yy) then compcodes<-1>=compcode
			//if validcode1(compcode,'','',xx,yy) then compcodes<-1>=compcode
			//dont use general subroutine in system module
			var companypositive = "";
			if (not(authorised("COMPANY ACCESS", xx, ""))) {
				companypositive = "#";
			}
			if (authorised(companypositive ^ "COMPANY ACCESS " ^ (compcode.quote()), xx, "")) {
				compcodes(-1) = compcode;
			}

		} //ii;
	} else {
		compcodes = allcomps.a(1);
	}

	//check there is at least one authorised company
	if (allcomps and compcodes eq "") {
		msg = "Error: You are not authorised to access any companies";
		return 0;
	}

	//initialise to the first authorised company
	compcode = compcodes.a(1);
	if (compcode) {
		var tempcompany;
		if (not(tempcompany.read(sys.companies, compcode))) {
			msg = "Error: " ^ (compcode.quote()) ^ " company code is missing";
			return 0;
		}
	}

	sys.company = "";
	call initcompany(compcode);

	var ncompanies = compcodes.count(FM) + 1;
	authcompcodes = compcodes;
	authcompcodes.converter(FM, VM);

	//market
	//defmarketcode=if company<30> then company<30> else agp<37>;*market
	//WARNING TODO: check ternary op following;
	var defmarketcode = sys.company.a(30) ? sys.company.a(30) : SYSTEM.a(137);
	//if unassigned(markets) then markets=''
	//TODO maybe use the market on the user file?
	//markets is not open in finance only module
	//readv maincurrcode from markets,defmarketcode,5 else maincurrcode=''
	var maincurrcode = "";
	if (xx.open("MARKETS", "")) {
		//defmarketcode=agp<37>
		defmarketcode = SYSTEM.a(137);
		maincurrcode = defmarketcode.xlate("MARKETS", 5, "X");
	}

	//main currencycode
	if (maincurrcode.unassigned()) {
		maincurrcode = "";
	}
	//if maincurrcode='' then maincurrcode=base.currency
	if (maincurrcode eq "") {
		maincurrcode = SYSTEM.a(134);
	}

	//system<134> financial base.currency
	//system<135> financial curr.period
	//system<136> financial curr.year
	//system<137> agency default market code
	//system<138> agency last day of week mon-sun 1-7

	//prepare session cookie

	cookie = "m=" ^ menus.convert(VM, ",");
	cookie ^= "&cc=" ^ compcode;
	cookie ^= "&nc=" ^ ncompanies;

	//period/year
	//cookie:='&pd=':currperiod:'/':addcent4(curryear)
	cookie ^= "&pd=" ^ SYSTEM.a(135) ^ "/" ^ addcent4(SYSTEM.a(136));

	//cookie:='&bc=':base.currency
	cookie ^= "&bc=" ^ SYSTEM.a(134);

	//base
	cookie ^= "&bf=" ^ BASEFMT;

	cookie ^= "&mk=" ^ defmarketcode;
	cookie ^= "&mc=" ^ maincurrcode;
	cookie ^= "&tz=" ^ SW;
	//cookie ^= "&ms=60000";
	cookie ^= "&ms=1000000";

	//current datasetname
	var temp = SYSTEM.a(23);
	if (not temp) {
		temp = SYSTEM.a(17);
	}
	temp.swapper("&", " and ");
	cookie ^= "&db=" ^ temp;

	//split extras (does gui use this? it must always be 1)
	//cookie:='&sp=':agp<5>
	cookie ^= "&sp=1";

	//form color, font and fontsize
	cookie ^= "&fc=" ^ SYSTEM.a(46, 5);
	cookie ^= "&ff=" ^ SYSTEM.a(46, 6);
	cookie ^= "&fs=" ^ SYSTEM.a(46, 7);

	//date format
	cookie ^= "&df=" ^ sys.company.a(10);

	//first day of week
	//tt=agp<13>+1
	var tt = SYSTEM.a(138) + 1;
	if (tt gt 7) {
		tt = 1;
	}
	cookie ^= "&fd=" ^ tt;

	//whats new

	call changelogsubs("WHATSNEW" ^ FM ^ menus);
	var ans = ANS;
	cookie ^= "&wn=" ^ ans;

    cookie ^= "&ap=" ^ APPLICATION;

	//call backupreminder(dataset,msg)

	//osread paramrec from '..\data\':lcase(dataset):'\params2' else return
	var paramfilename = "../data/" ^ dataset.lcase() ^ "/params2";
	paramfilename.converter("/", OSSLASH);
	if (not(paramrec.osread(paramfilename))) {
		return 0;
	}
	//var lastbackupdate = paramrec.a(2);
	//backup_db script updates params2 date/time
	var lastbackupdate = paramfilename.osfile().a(2);

	//if lastbackupdate and lastbackupdate lt date()-1 then
	//assume backup on same day (ie after last midnight)
	if (lastbackupdate and lastbackupdate lt var().date()) {
		msg = "The last backup was ";
		var ndays = var().date() - lastbackupdate;
		msg ^= ndays ^ " day" ^ var("s").substr(1, ndays ne 1) ^ " ago.";
		msg ^= "   (" ^ lastbackupdate.oconv("D") ^ ")";
		msg.swapper("(0", "(");
		msg(-1) = "EXODUS recommends that you \"BACKUP\" your data ";
		msg(-1) = "regularly to prevent total loss of data due to";
		msg(-1) = "power failure, disk damage or other accidents.";
	}

	return 0;
}

libraryexit()
