#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <daterangetext.h>
#include <initcompany.h>
#include <openfile.h>
#include <quote2.h>
#include <safeselect.h>

#include <service_common.h>

#include <srv_common.h>
#include <req_common.h>

#include <request.hpp>

//var keywords;
//var mode;
//var nkeywords;
//var changelog;
//var currentversiondatetime;
//var tt;
//var versionn;  // num
//var cmd;
//var versionlog;
//var idate;
//var xx;
//var op;
//var op2;
//var msg;
//var wspos;
//var wsmsg;

function main(in mode0) {

	var mode = mode0;

	let keywords  = "MEDIA" _VM "JOBS" _VM "FINANCE" _VM "TIMESHEETS" _VM "TECHNICAL" _VM "USER INTERFACE";
	let nkeywords = keywords.fcount(VM);

	var changelog;
	if (not openfile("CHANGELOG", changelog)) {
		call mssg(lasterror());
		return 0;
	}
	if (not openfile("DICT.CHANGELOG", DICT)) {
		call mssg(lasterror());
		return 0;
	}

	if (mode.f(1) == "SELECTANDLIST") {

		// change to the user selected company
		// merely to get the right letterhead
		let compcode = mode.f(5);
		if (compcode) {
			call initcompany(compcode);
		}

		// call changelog.subs('SELECT':fm:data)
		gosub select0(mode);
		if (not LISTACTIVE) {
			call mssg("Error: No records found");
			return 0;
		}

		// call changelog.subs('LIST':fm:data)
		gosub list(mode);

		// called from LOGIN.NET. not UI. UI calls SELECTANDLIST
	} else if (mode.f(1) == "WHATSNEW") {

		let menucodes = mode.f(2);
		mode		  = mode.f(1);
		ANS			  = "";

		var users;
		if (not users.open("USERS", "")) {
			return 0;
		}

		var userrec;
		if (userrec.read(users, USERNAME)) {

			// backward compatible - can be deleted after all upgraded
			// leave in case reloading ancient data
			if (not userrec.f(17)) {
				let changelogkey = "USER*" ^ USERNAME;
				if (changelog.read(DEFINITIONS, changelogkey)) {
					userrec(17) = changelog.f(8);
					userrec.f(17).writef(users, USERNAME, 17);

					var("").writef(DEFINITIONS, changelogkey, 8);
				}
			}

			// mode<2>=changelog<7>
			// list all items in changelog since the EXODUS version in the user file
			mode(3) = userrec.f(17);

			// fix a problem where people were missing most changes
			// due to sv being represented as : eg user:support:technical
			if (mode.f(3) and mode.f(3) < 14773) {
				mode(3) = 14153;
			}

		} else {

			// show everything the first time they logon
			// mode<3>=iconv('1/1/2004','D/E')

			// show nothing the very first time they logon
			ANS = "";
			return 0;
		}

		var currentversiondatetime = gosub getcurrentversiondatetime();

		// nothing to see if seen matches (or after?) currentversiondate
		if (mode.f(3) >= currentversiondatetime) {
			ANS = "";
			return 0;
		}

		currentversiondatetime.writef(users, USERNAME, 17);

		// build preferences from menus if not specified
		if (not mode.f(2)) {

			// tt=capitalise(menucodes)
			// swap 'Support' with 'Technical' in tt
			// mode<2>=tt
			if (menucodes.contains("FINANCE")) {
				mode(2, -1) = "Finance";
			}
			if (menucodes.contains("MEDIA")) {
				mode(2, -1) = "Media";
			}
			if (menucodes.contains("JOBS")) {
				mode(2, -1) = "Jobs";
			}
			if (menucodes.contains("TIMESHEETS")) {
				mode(2, -1) = "Timesheets";
			}
			if (menucodes.contains("SUPPORT")) {
				mode(2, -1) = "Technical";
			}

			// everybody gets User Interface changes
			mode(2, -1) = "User Interface";

			// if no preferences then no whats new
			// if mode<2> else
			// @ans=''
			// return
			// end
		}

		// indicate need to select whats new preferences (by numeric whatsnew)
		// if mode<2> else
		// if currentversiondate else currentversiondate='0'
		// @ans=currentversiondate
		// return
		// end

		// find and new items else quit
		gosub select(mode);
		if (not LISTACTIVE) {
			ANS = "";
			return 0;
		}

		// make a suitable output filename based on the responsefilename
		var temp = PRIORITYINT.f(100);
		temp.paster(-1, 1, "htm");
		SYSTEM(2) = temp;

		// get new items in new filename and return the filename in @ans
		gosub list(mode);
		ANS = SYSTEM.f(2);

		// actually this is GETINSTALLEDVERSION DATES!
	} else if (mode == "GETVERSIONDATES") {

		gosub getversiondates();

	} else if (mode.f(1) == "SELECT") {

		gosub select0(mode);

	} else if (mode.f(1) == "LIST") {

		gosub list(mode);

	} else if (mode.f(1) == "POSTWRITE") {

		// After writing new entry to changelog database file
		// also write entry to new dat osfile can be distributed
		// to other installations by adding to git

		let path = "/root/neosys/src/dat/changelog/";
		if (not osdir(path)) {
			call mssg("ERROR in CHANGELOGSUBS | " ^ path ^ " is missing");
			return 0;
		}

		let osrecord = RECORD.convert(FM, "\n");
		if (not osrecord.oswrite(path ^ ID)) {
			call mssg(lasterror());
			return 0;
		}
	}

	return 0;
}

subroutine select0(io mode) {

	// get the one date prior (so list is "as at")

	// get list of installed version dates
	gosub getversiondates();

	if (mode.f(3)) {
		// data contains a list of installed version dates
		// (not dates that upgrades were done!)
		// get the first installed version date equal to or prior to the selected
		// locate mode<3> in data by 'AR' using fm setting versionn else
		// locatebyusing() not available in c++
		let tt = data_.convert(FM, VM);
		var versionn;
		if (not tt.locateby("AR", mode.f(3), versionn)) {
			if (versionn > 1) {
				mode(3) = data_.f(versionn - 1);
			}
		}
	}

	gosub select(mode);
	return;
}

subroutine select(in mode) {
	data_ = mode.field(FM, 2, 9999);

	// force all
	// data<2>=0

	var cmd	 = "SELECT CHANGELOG";
	var andx = "";
	if (data_.f(1)) {
		cmd ^= " WITH KEYWORD " ^ quote2(data_.f(1));
		andx = " AND";
	}
	if (data_.f(2)) {
		cmd ^= andx ^ " WITH DATE GE " ^ (data_.f(2).oconv("D4").quote());
		andx = " AND";
	}
	if (USERNAME != "EXODUS") {
		// cmd:=andx:' WITH DISTRIBUTION ""'
		// andx=' AND'
		// cmd:=' "User"'
		cmd ^= andx ^ " ( WITH NO DISTRIBUTION OR WITH DISTRIBUTION = \"User\" )";
		andx = " AND";
	}
	call safeselect(cmd ^ " (S)");

	return;
}

subroutine list(in mode) {
	// input
	// data<1>=mv list of topics
	// data<2>=date from which interested
	// mode<2>

	data_ = mode.field(FM, 2, 9999);

	var cmd = "LIST CHANGELOG ID-SUPP";
	// cmd:=' KEYWORD TEXT'
	cmd ^= " DATE";
	cmd ^= " KEYWORDS TEXT2";

	// cmd:=' BY KEYWORD'
	cmd ^= " BY KEYWORDS";
	cmd ^= " BY NUMBER";

	var	  headingx = "What''s New in EXODUS";
	var currentversiondatetime = gosub getcurrentversiondatetime();
	if (data_.f(2)) {
		// heading:=' version ':data<2> '[DATE,4*]':' -'
		var rangetext;
		call daterangetext(data_.f(2), currentversiondatetime, rangetext, srv.glang);
		headingx ^= " : " ^ rangetext;
	} else {
		headingx ^= " Version : " ^ oconv(currentversiondatetime, "[DATE,4*]");
	}

	if (data_.f(1)) {
		headingx ^= "'L'" ^ data_.f(1);
		headingx.replacer(SM, ", ");
		headingx.replacer(VM, ", ");
	}

	cmd ^= " HEADING " ^ (headingx.quote());

	perform(cmd);
	return;
}

function getcurrentversiondatetime() {
	// get currently installed version date
	// temp=xlate('DOS','.\general\version.dat',1,'x')
	var temp = "./general/version.dat";
	temp.converter("/", OSSLASH);
	// temp = temp.xlate("DOS", 1, "X");
	temp = osread(temp);
	if (not VOLUMES) {
		if (temp == "") {
			temp = EXECPATH.osfile();
			// 19:05:55  18 NOV 2020
			temp = temp.f(3).oconv("MTS") ^ " " ^ temp.f(2).oconv("D");
		}
	}
	var currentversiondatetime = temp.trim().field(" ", 2, 999).iconv("D");
	currentversiondatetime ^= "." ^ temp.trim().field(" ", 1).iconv("MT").oconv("R(0)#5");
	return currentversiondatetime;
}

subroutine getversiondates() {
	// extract installed version dates from upgrade.cfg
	//call osread(versionlog, "upgrade.cfg");
	var versionlog;
	if (not osread(versionlog, "upgrade.cfg"))
		loglasterror();

	// versionlog=trim(field(versionlog,\1A\,1))
	versionlog.converter("\r\n", _FM _FM);
	let nn			= versionlog.fcount(FM);
	var versiondata = "";
	for (const var ii : range(1, nn)) {
		let idate = versionlog.f(ii, 1).field(" ", 2, 3).iconv("D");
		if (not idate)
			continue;
		// itime=iconv(field(versionlog,' ',1),'MT')
		if (not versiondata.locateusing(FM, idate)) {
			versiondata(-1) = idate;
		}
	}  // ii;

	// list of installed versions
	data_ = versiondata;
	return;
}

libraryexit()
