#include <exodus/library.h>
libraryinit()

#include <openfile.h>
#include <quote2.h>
#include <singular.h>
#include <authorised.h>

#include <gen.h>
#include <win.h>

var changelog;
var upgradelog;
var xx;
var upgraden;//num
var doc;
var idaten;
var msg;

function main(in mode0) {

	//WHATSNEW returns in ANS
	ANS="";

	var mode=mode0;

	var keywords = "MEDIA" _VM_ "JOBS" _VM_ "FINANCE" _VM_ "TIMESHEETS" _VM_ "TECHNICAL" _VM_ "USER INTERFACE";

	var nkeywords = keywords.count(VM) + 1;

	if (not(openfile("CHANGELOG", changelog))) {
		//suppress openfiles errormsg
		if (mode.a(1) == "WHATSNEW")
			USER4="";
		return 0;
	}
	if (not(openfile("DICT_CHANGELOG", DICT))) {
		call fsmsg();
		return 0;
	}

	if (mode.a(1) == "SELECTANDLIST") {

		//call changelog.subs('SELECT':fm:data)
		gosub select0(mode);
		if (not LISTACTIVE) {
			call mssg("Error: No records found");
			return 0;
		}

		//call changelog.subs('LIST':fm:data)
		gosub list(mode);

	//returns outputfilename in ANS
	} else if (mode.a(1) == "WHATSNEW") {

		var menucodes = mode.a(2);
		mode = mode.a(1);
		ANS = "";

		var users;
		if (not(users.open("USERS", ""))) {
			return 0;
		}

		var userrec;
		if (userrec.read(users, USERNAME)) {

			//backward compatible ... can be deleted after all upgraded
			//leave in case reloading ancient data
			if (not userrec.a(17)) {
				var changelogkey = "USER*" ^ USERNAME;
				if (changelog.read(DEFINITIONS, changelogkey)) {
					userrec.r(17, changelog.a(8));
					(userrec.a(17)).writev(users, USERNAME, 17);
					var("").writev(DEFINITIONS, changelogkey, 8);
				}
			}

			//mode<2>=changelog<7>
			mode.r(3, userrec.a(17));

			//fix a problem where people were missing most changes
			//due to sv being represented as : eg user:support:technical
			if (mode.a(3) and mode.a(3) < 14773) {
				mode.r(3, 14153);
			}

		}else{

			//show everything the first time they logon
			//mode<3>=iconv('1/1/2004','D/E')

			//show nothing the very first time they logon
			ANS = "";
			return 0;

		}

		//get last upgradedate

		var temp = var(".\\GENERAL\\VERSION.DAT").xlate("DOS", 1, "X");
		var lastupgradedatetime = (temp.trim().field(" ", 2, 999)).iconv("D");
		lastupgradedatetime ^= "." ^ ((temp.trim().field(" ", 1)).iconv("MT")).oconv("R(0)#5");

		//nothing to see if seen after lastupgradedate
		if (mode.a(3) >= lastupgradedatetime) {
			ANS = "";
			return 0;
		}

		lastupgradedatetime.writev(users, USERNAME, 17);

		//build preferences from menus if not specified
		if (not mode.a(2)) {

			//tt=capitalise(menucodes)
			//swap 'Support' with 'Technical' in tt
			//mode<2>=tt
			if (menucodes.index("FINANCE", 1)) {
				mode.r(2, -1, "Finance");
			}
			if (menucodes.index("MEDIA", 1)) {
				mode.r(2, -1, "Media");
			}
			if (menucodes.index("JOBS", 1)) {
				mode.r(2, -1, "Jobs");
			}
			if (menucodes.index("TIMESHEETS", 1)) {
				mode.r(2, -1, "Timesheets");
			}
			if (menucodes.index("SUPPORT", 1)) {
				mode.r(2, -1, "Technical");
			}

			//everybody gets User Interface changes
			mode.r(2, -1, "User Interface");

			//if no preferences then no whats new
			//if mode<2> else
			// @ans=''
			// return 0
			// end

		}

		//indicate need to select whats new preferences (by numeric whatsnew)
		//if mode<2> else
		// if lastupgradedate else lastupgradedate='0'
		// @ans=lastupgradedate
		// return 0
		// end

		//find and new items else quit
		gosub select(mode);
		if (not LISTACTIVE) {
			ANS = "";
			return 0;
		}

		//make a suitable output filename based on the responsefilename
		temp = PRIORITYINT.a(100);
		temp.splicer(-1, 1, "HTM");
		SYSTEM.r(2, temp);

		//get new items in new filename and return the filename in @ans
		gosub list(mode);
		ANS = SYSTEM.a(2);

	} else if (mode == "GETUPGRADEDATES") {
		gosub getupgradedates();

	} else if (mode.a(1) == "SELECT") {
		gosub select0(mode);

	} else if (mode.a(1) == "LIST") {
		gosub list(mode);

	}

	return 0;

}

subroutine getupgradedates() {

	call osread(upgradelog, "UPGRADE.CFG");
	upgradelog = upgradelog.field(0x1A, 1).trim();
	upgradelog.converter("\r\n", FM);
	var nn = upgradelog.count(FM) + (upgradelog ne "");
	var upgradedata = "";
	for (var ii = 1; ii <= nn; ++ii) {
		var idate = (upgradelog.a(ii, 1).field(" ", 2, 3)).iconv("D");
		//itime=iconv(field(upgradelog,' ',1),'MT')
		if (not(upgradedata.locate(idate, xx))) {
			upgradedata.r(-1, idate);
		}
	};//ii;

	USER1 = upgradedata;
	return;
}

subroutine list(in mode) {
	//input
	//data<1>=mv list of topics
	//data<2>=date from which interested
	//mode<2>

	USER1 = mode.field(FM, 2, 9999);

	var heading = "What\'\'s New in NEOSYS";
	var cmd = "LIST CHANGELOG ID-SUPP";
	//cmd:=' KEYWORD TEXT'
	cmd ^= " DATE";
	cmd ^= " KEYWORDS TEXT2";

	//cmd:=' BY KEYWORD'
	cmd ^= " BY KEYWORDS";
	cmd ^= " BY NUMBER";

	if (USER1.a(2)) {
		heading ^= " Since " ^ (USER1.a(2)).oconv("[DATE,4*]");
	}

	if (USER1.a(1)) {
		heading ^= "\'L\'" ^ USER1.a(1);
		heading.swapper(SVM, ", ");
		heading.swapper(VM, ", ");
	}

	cmd ^= " HEADING " ^ heading.quote();

	perform(cmd);

	return;
}

subroutine select0(in modex) {

	//get the one date prior (so list is "as at")
	gosub getupgradedates();

	var mode=modex;
	if (USER1.locateby(mode.a(3), "AR", upgraden)) {
		if (upgraden > 1) {
			mode.r(3, USER1.a(upgraden - 1));
		}else{
			mode.r(3, "");
		}
	}else{
		mode.r(3, "");
	}

	gosub select(mode);

	return;
}

subroutine select(in mode) {

	USER1 = mode.field(FM, 2, 9999);

	//force all
	//data<2>=0

	var cmd = "SELECT CHANGELOG";
	var andword = "";
	if (USER1.a(1)) {
		cmd ^= " WITH KEYWORD " ^ quote2(USER1.a(1));
		andword = " AND";
	}
	if (USER1.a(2)) {
		cmd ^= andword ^ " WITH DATE GE " ^ (DQ ^ ((USER1.a(2)).oconv("[DATE,4*]") ^ DQ));
		andword = " AND";
	}
	if (USERNAME ne "NEOSYS") {
		cmd ^= andword ^ " WITH DISTRIBUTION \"\"";
		andword = " AND";
		//if security('DOCUMENTATION ACCESS USER') then
		cmd ^= " \"User\"";
		// end
	}
	//call safeselect(cmd ^ " (S)");
	var().select(cmd ^ " (S)");

}

libraryexit()
