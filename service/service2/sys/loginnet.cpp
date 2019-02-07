#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <validcode2.h>
#include <initcompany.h>
#include <addcent.h>
#include <changelogsubs.h>

#include <gen.h>
#include <fin.h>
#include <agy.h>

var menuid;
var taskn;//num
var xx;
var menun2;
var compcodex;
var yy;
var paramrec;

function main(in dataset, in username, io cookie, io msg, io authcompcodes) {
	//c sys in,in,io,io,io

	//this is a special login routine called from LISTEN2
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
				}else{
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
	oldmenus.swapper("ADPRODUCTION", "JOBS");
	oldmenus.swapper("GENERAL", "SUPPORT");
	//convert '.' to '_' in oldmenus

	//all menus - not in order
	var menus = "FINANCE" _VM_ "SUPPORT";

	if (APPLICATION == "ADAGENCY") {

		menus.r(1, -1, "ANALYSIS");

		//MEDIA is always a candidate since it is difficult
		//to work out exc
		menus.r(1, -1, "MEDIA");

		//JOBS menu is only a candidate if they can access jobs
		//unfortunately media probably have job access to attach jobs
		if (authorised("JOB ACCESS", msg, "")) {
			menus.r(1, -1, "JOBS");
		}

		//TIMESHEETS menu is only a candidate if then can access timesheets
		if (authorised("TIMESHEET ACCESS", msg, "")) {
			menus.r(1, -1, "TIMESHEETS");
		}

	}

	//reduce the menus to a list of authorised menus
	var nmenus = menus.count(VM) + 1;
	//work backwards because we are deleting
	for (var menun = nmenus; menun >= 1; --menun) {
		var menu = menus.a(1, menun).ucase();
		if (menu) {
			var menutask = "MENU " ^ menu;
			if (not(SECURITY.a(10).locateusing(menutask, VM, taskn))) {
				taskn = 0;
			}

			//specifically locked or no legacy menus - only allow if authorised
			if ((oldmenus == "") or ((menun and SECURITY.a(11, taskn).length()))) {
				if (not(authorised(menutask, xx))) {
deleteit:
					if (menus.locateusing(menu, VM, menun2)) {
						menus.eraser(1, menun2);
						nmenus -= 1;
					}
				}

				//not specifically locked - only allow if in legacy menus
			}else{
				if (not(oldmenus.locateusing(menu, VM, xx))) {
					goto deleteit;
				}
			}

		}
	};//menun;

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
	menus.r(1, -1, "HELP");

	//determine the company

	var compcode = "";

	var allcomps;
	if (not(allcomps.read(gen.companies, "%RECORDS%"))) {
		select(gen.companies);
		allcomps = "";
nextcomp:
		if (readnext(compcodex)) {
			allcomps.r(-1, compcodex);
			goto nextcomp;
		}
	}

	//build a list of authorised companies
	var compcodes = "";
	if (APPLICATION == "ADAGENCY") {
		for (var ii = 1; ii <= 9999; ++ii) {
			compcode = allcomps.a(ii);
		///BREAK;
		if (not compcode) break;;
			if (validcode2(compcode, "", "", xx, yy)) {
				compcodes.r(-1, compcode);
			}
		};//ii;
	}else{
		compcodes = allcomps.a(1);
	}

	//check there is at least one authorised company
	if (allcomps and (compcodes == "")) {
		msg = "Error: You are not authorised to access any companies";
		return 0;
	}

	//initialise to the first authorised company
	compcode = compcodes.a(1);
	if (compcode) {
		var tempcompany;
		if (not(tempcompany.read(gen.companies, compcode))) {
			msg = "Error: " ^ (DQ ^ (compcode ^ DQ)) ^ " company code is missing";
			return 0;
		}
	}

	gen.company = "";
	call initcompany(compcode);

	var ncompanies = compcodes.count(FM) + 1;
	authcompcodes = compcodes;
	authcompcodes.converter(FM, VM);

	//market
//WARNING TODO: check trigraph following;
	var defmarketcode = gen.company.a(30) ? gen.company.a(30) : agy.agp.a(37);
	//if unassigned(markets) then markets=''
	//TODO maybe use the market on the user file?
	//markets is not open in finance only module
	//readv maincurrcode from markets,defmarketcode,5 else maincurrcode=''
	var maincurrcode = "";
	if (FILES(0).locateusing("MARKETS", FM, xx)) {
		defmarketcode = agy.agp.a(37);
		maincurrcode = defmarketcode.xlate("MARKETS", 5, "X");
	}

	//main currencycode
	if (maincurrcode.unassigned()) {
		maincurrcode = "";
	}
	if (maincurrcode == "") {
		maincurrcode = fin.basecurrency;
	}

	//prepare session cookie

	cookie = "m=" ^ menus.convert(VM, ",");
	cookie ^= "&cc=" ^ compcode;
	cookie ^= "&nc=" ^ ncompanies;
	cookie ^= "&pd=" ^ fin.currperiod ^ "/" ^ addcent(fin.curryear, "", "", xx);
	cookie ^= "&bc=" ^ fin.basecurrency;
	cookie ^= "&bf=" ^ USER2;
	cookie ^= "&mk=" ^ defmarketcode;
	cookie ^= "&mc=" ^ maincurrcode;
	cookie ^= "&tz=" ^ SW;

	//current datasetname
	var temp = SYSTEM.a(23);
	temp.swapper("&", " and ");
	cookie ^= "&db=" ^ temp;

	//split extras
	cookie ^= "&sp=" ^ agy.agp.a(5);

	//form color, font and fontsize
	cookie ^= "&fc=" ^ SYSTEM.a(46, 5);
	cookie ^= "&ff=" ^ SYSTEM.a(46, 6);
	cookie ^= "&fs=" ^ SYSTEM.a(46, 7);

	//date format
	cookie ^= "&df=" ^ gen.company.a(10);

	//first day of week
	var tt = agy.agp.a(13) + 1;
	if (tt > 7) {
		tt = 1;
	}
	cookie ^= "&fd=" ^ tt;

	//whats new

	call changelogsubs("WHATSNEW" ^ FM ^ menus);
	var ans = ANS;
	cookie ^= "&wn=" ^ ans;

	//call backupreminder(dataset,msg)

	if (not(paramrec.osread("..\\data\\" ^ dataset ^ "\\params2"))) {
		return 0;
	}
	var lastbackupdate = paramrec.a(2);

	//if lastbackupdate and lastbackupdate lt date()-1 then
	//assume backup on same day (ie after last midnight)
	if (lastbackupdate and (lastbackupdate < var().date())) {
		msg = "The last backup was ";
		var ndays = var().date() - lastbackupdate;
		msg ^= ndays ^ " day" ^ var("s").substr(1,ndays ne 1) ^ " ago.";
		msg ^= "   (" ^ lastbackupdate.oconv("D") ^ ")";
		msg.swapper("(0", "(");
		msg.r(-1, "NEOSYS recommends that you \"BACKUP\" your data ");
		msg.r(-1, "regularly to prevent total loss of data due to");
		msg.r(-1, "power failure, disk damage or other accidents.");
	}

	return 0;

}


libraryexit()
