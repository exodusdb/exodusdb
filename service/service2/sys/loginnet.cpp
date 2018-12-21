#include <exodus/library.h>
libraryinit()

//#include <authorised.h>
#include <validcode2.h>
#include <initcompany.h>
#include <addcent.h>
#include <changelogsubs.h>
//#include <backupreminder.h>

#include <gen.h>
#include <fin.h>
//#include <agy.h>

var taskn;//num
var menun2;
var xx;
var msg;

function main(in dataset, in username, io cookie, io msg, io authcompcodes) {

	cookie = "";
	authcompcodes = "";

	//return allowable menus if legacy menu in users file
	//legacy menus from pre 03JUN10 code per user settings on Authorisation File
	//assignment of locks on menus and/or clearing userprivs<3>
	//removes legacy privileges
	var oldmenus = "";
	var users;
	if (users.open("USERS", "")) {
		var menuid;
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

	if (APPLICATION != "ACCOUNTS") {

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
		var menu = menus.a(1,menun).ucase();
		if (menu) {
			var menutask = "MENU " ^ menu;
			if (not(SECURITY.locate(menutask, taskn, 10))) {
				taskn = 0;
			}

			//specifically locked or no legacy menus - only allow if authorised
			if (oldmenus == "" or (menun and (SECURITY.a(11, taskn)).length())) {
				if (not authorised(menutask)) {
deleteit:
					if (menus.locate(menu, menun2, 1)) {
						menus.eraser(1, menun2);
						nmenus -= 1;
					}
				}

				//not specifically locked - only allow if in legacy menus
			}else{
				if (not(oldmenus.locate(menu, xx, 1))) {
					goto deleteit;
				}
			}

		}
	};//menun;

	//if they have MENU then ANALYSIS=MEDIAANALYSIS
	if (menus.index("MEDIA", 1)) {
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
		gen.companies.select();
		allcomps = "";
		var compcodex;
		while (gen.companies.readnext(compcodex)) {
			allcomps.r(-1, compcodex);
		}
	}

	//build a list of authorised companies
	var compcodes = "";
	if (APPLICATION ne "ACCOUNTS") {
		for (var ii = 1; ii <= 9999; ++ii) {
			compcode = allcomps.a(ii);

			///BREAK;
			if (not compcode)
				break;

			var brands="";//agy.brands may not be available and is not useful in this case
			if (validcode2(compcode, "", "", brands, msg)) {
				compcodes.r(-1, compcode);
			}
		};//ii;
	}else{
		compcodes = allcomps.a(1);
	}

	//check there is at least one authorised company
	if (allcomps and compcodes == "") {
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
	var defmarketcode = gen.company.a(30);
	//agy may not be available
	//if (not defmarketcode) {
	//	defmarketcode = agy.agp.a(37);
	//}
	//if unassigned(markets) then markets=''
	//TODO maybe use the market on the user file?
	//markets is not open in finance only module
	//readv maincurrcode from markets,defmarketcode,5 else maincurrcode=''
	var maincurrcode = "";
	//agy may not be available
	//if (agy.markets) {
	//	maincurrcode = defmarketcode.xlate("MARKETS", 5, "X");
	//}
	if (maincurrcode == "") {
		maincurrcode = fin.basecurrency;
	}

	//prepare session cookie

	cookie = "m=" ^ menus.convert(VM,",");
	cookie ^= "&cc=" ^ compcode;
	cookie ^= "&nc=" ^ ncompanies;
	cookie ^= "&pd=" ^ fin.currperiod ^ "/" ^ addcent(fin.curryear);
	cookie ^= "&bc=" ^ fin.basecurrency;
	cookie ^= "&bf=" ^ USER2;//base currency format
	cookie ^= "&mk=" ^ defmarketcode;
	cookie ^= "&mc=" ^ maincurrcode;
	cookie ^= "&tz=" ^ mv.SW;

	//current datasetname
	var temp = SYSTEM.a(23);
	temp.swapper("&", " and ");
	cookie ^= "&db=" ^ temp;

	//split extras
	//always split now
	//cookie ^= "&sp=" ^ agy.agp.a(5);

	//form color, font and fontsize
	cookie ^= "&fc=" ^ SYSTEM.a(46, 5);
	cookie ^= "&ff=" ^ SYSTEM.a(46, 6);
	cookie ^= "&fs=" ^ SYSTEM.a(46, 7);

	//date format
	cookie ^= "&df=" ^ gen.company.a(10);

	//first day of week
	//agy may not be available
	//var tt = agy.agp.a(13) + 1;
	//if (tt > 7) {
	//	tt = 1;
	//}
	var tt="";
	cookie ^= "&fd=" ^ tt;

	//whats new
	call changelogsubs("WHATSNEW" ^ FM ^ menus);
	var ans = ANS;
	cookie ^= "&wn=" ^ ans;

//TODO	call backupreminder(dataset, msg);
	if (false)
		dataset.output();

	return 0;

}


libraryexit()
