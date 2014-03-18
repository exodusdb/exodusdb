#include <exodus/library.h>
libraryinit()

#include <openfile.h>
#include <quote2.h>
#include <safeselect.h>
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

function main(io mode) {

	var keywords = "MEDIA" _VM_ "JOBS" _VM_ "FINANCE" _VM_ "TIMESHEETS" _VM_ "TECHNICAL" _VM_ "USER INTERFACE";

	var nkeywords = keywords.count(VM) + 1;

	if (not(openfile("CHANGELOG", changelog))) {
		var().stop();
	}
	if (not(openfile("DICT.CHANGELOG", DICT))) {
		var().stop();
	}

	if (mode.a(1) == "SELECTANDLIST") {

		//call changelog.subs('SELECT':fm:data)
		gosub select0();
		if (not LISTACTIVE) {
			call mssg("Error: No records found");
			return 0;
		}

		//call changelog.subs('LIST':fm:data)
		gosub list();

		goto 1213;
	}
	if (mode.a(1) == "WHATSNEW") {

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
				if (changelog.read(gen._definitions, changelogkey)) {
					userrec.r(17, changelog.a(8));
					(userrec.a(17)).writev(users, USERNAME, 17);
					var("").writev(gen._definitions, changelogkey, 8);
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
		gosub select();
		if (not LISTACTIVE) {
			ANS = "";
			return 0;
		}

		//make a suitable output filename based on the responsefilename
		temp = PRIORITYINT.a(100);
		temp.splicer(-1, 1, "HTM");
		SYSTEM.r(2, temp);

		//get new items in new filename and return the filename in @ans
		gosub list();
		ANS = SYSTEM.a(2);

		goto 1213;
	}
	if (mode == "GETUPGRADEDATES") {

getupgradedates:
	////////////////
		call osread(upgradelog, "UPGRADE.CFG");
		upgradelog = upgradelog.field(0x1A, 1).trim();
		upgradelog.converter("\r\n", FM);
		var nn = upgradelog.count(FM) + (upgradelog ne "");
		var upgradedata = "";
		for (var ii = 1; ii <= nn; ++ii) {
			var idate = (upgradelog.a(ii, 1).field(" ", 2, 3)).iconv("D");
			//itime=iconv(field(upgradelog,' ',1),'MT')
			if (not(upgradedata.locateusing(idate, FM, xx))) {
				upgradedata.r(-1, idate);
			}
		};//ii;

		USER1 = upgradedata;

		goto 1213;
	}
	if (mode.a(1) == "SELECT") {
}

subroutine select0() {
		//get the one date prior (so list is "as at")
		gosub getupgradedates();
		if (USER1.locatebyusing(mode.a(3), "AR", upgraden, FM)) {
			if (upgraden > 1) {
				mode.r(3, USER1.a(upgraden - 1));
			}else{
				mode.r(3, "");
			}
		}else{
			mode.r(3, "");
		}
}

subroutine select() {
		USER1 = mode.field(FM, 2, 9999);

	//force all
	//data<2>=0

		var cmd = "SELECT CHANGELOG";
		var and = "";
		if (USER1.a(1)) {
			cmd ^= " WITH KEYWORD " ^ quote2(USER1.a(1));
			and = " AND";
		}
		if (USER1.a(2)) {
			cmd ^= and ^ " WITH DATE GE " ^ (DQ ^ ((USER1.a(2)).oconv("[DATE,4*]") ^ DQ));
			and = " AND";
		}
		if (USERNAME ne "NEOSYS") {
			cmd ^= and ^ " WITH DISTRIBUTION \"\"";
			and = " AND";
			//if security('DOCUMENTATION ACCESS USER') then
			cmd ^= " \"User\"";
			// end
		}
		call safeselect(cmd ^ " (S)");

		goto 1213;
	}
	if (mode.a(1) == "LIST") {
}

subroutine list() {
		//input
		//data<1>=mv list of topics
		//data<2>=date from which interested
		//mode<2>

		USER1 = mode.field(FM, 2, 9999);

		var HEADING = "What\'\'s New in NEOSYS";
		var cmd = "LIST CHANGELOG ID-SUPP";
		//cmd:=' KEYWORD TEXT'
		cmd ^= " DATE";
		cmd ^= " KEYWORDS TEXT2";

		//cmd:=' BY KEYWORD'
		cmd ^= " BY KEYWORDS";
		cmd ^= " BY NUMBER";

		if (USER1.a(2)) {
			HEADING ^= " Since " ^ (USER1.a(2)).oconv("[DATE,4*]");
		}

		if (USER1.a(1)) {
			HEADING ^= "\'L\'" ^ USER1.a(1);
			HEADING.swapper(SVM, ", ");
			HEADING.swapper(VM, ", ");
		}

		cmd ^= " HEADING " ^ (DQ ^ (HEADING ^ DQ));

		perform(cmd);

	}
L1213:
	return;

	///////
import:
	///////

	var importfilename = "neosys.txt";

	if (not doc.osread(importfilename)) {
		call fsmsg();
		var().stop();
	}

	doc.converter("\r\n", FM);

	doc.swapper("_" ^ FM, "\\r");

	doc = trim2(doc, FM);
	var nlines = doc.count(FM) + (doc ne "");

	//validate
	var update = 0;
	var errors = 0;
	gosub process();
	if (errors) {
		var().stop();
	}

	//load
	update = 1;
	var idates = "";
	gosub process();

	var().stop();

}

subroutine process() {
	for (var linen = 1; linen <= nlines; ++linen) {

		var line = doc.a(linen).trimb().trimf();

		//skip comments
		if (line.substr(1, 1) == "#") {
			goto nextline;
		}

		var line1 = line.field(":".ucase(1));

		var date = line1.field(" ", 1);
		var idate = date.iconv(DATEFORMAT);
		if (not idate) {
			call mssg("wrong date in " ^ line);
			errors += 1;
			goto nextline;
		}

		var rec = idate;

		var ucline = line.ucase();
		line1 = line1.field(" ", 2, 9999);

		//look for keywords
		for (var keywordn = 1; keywordn <= nkeywords; ++keywordn) {
			var keyword = keywords.a(1, keywordn);
			if (line1.index(keyword, 1)) {
				//rec<keywordn>=1
				rec.r(2, -1, keyword);
				line1.swapper(keyword, "");
			}else{
				if (keyword == "BUG" and ucline.index(" BUG", 1)) {
					rec.r(keywordn, 1);
				}
			}
		};//keywordn;

		line1.converter("/,", "");

		if (line1.trim()) {
			call mssg(line1 ^ " is what?");
			errors += 1;
		}

		var text = line.field(":", 2, 9999);
		text.swapper("\\r", VM);

		rec.r(3, text);

		if (update) {
			if (not(idates.a(1).locateusing(idate, VM, idaten))) {
				idates.r(1, idaten, idate);
			}
			var idatekey = (idate ^ "." ^ idates.a(2, idaten)).oconv("MD20PZ");
			idates.r(2, idaten, idates.a(2, idaten) + 1);
			rec.write(changelog, idatekey);
		}

nextline:
	};//linen;

	return;

}


libraryexit()