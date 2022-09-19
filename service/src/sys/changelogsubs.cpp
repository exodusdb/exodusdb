#include <exodus/library.h>
libraryinit()

#include <openfile.h>
#include <initcompany.h>
#include <quote2.h>
#include <safeselect.h>
#include <daterangetext.h>
#include <singular.h>
#include <authorised.h>

#include <sys_common.h>
#include <win_common.h>

#include <window.hpp>

var keywords;
var mode;
var nkeywords;
var changelog;
var currentversiondatetime;
var tt;
var versionn;//num
var cmd;
var versionlog;
var idate;
var xx;
var op;
var op2;
var msg;
var wspos;
var wsmsg;

function main(in mode0) {
	//c sys

	#include <system_common.h>
	//global mode,text,ucline,rec,idate,date,line1,idates,errors,update,keywords,nkeywords,cmd,nlines

	#define data_ USER1
	keywords = "MEDIA" _VM "JOBS" _VM "FINANCE" _VM "TIMESHEETS" _VM "TECHNICAL" _VM "USER INTERFACE";
	mode = mode0;
	nkeywords = keywords.count(VM) + 1;

	if (not(openfile("CHANGELOG", changelog))) {
		call fsmsg();
		return 0;
	}
	if (not(openfile("DICT.CHANGELOG", DICT))) {
		call fsmsg();
		return 0;
	}

	if (mode.f(1) eq "SELECTANDLIST") {

		//change to the user selected company
		//merely to get the right letterhead
		var compcode = mode.f(5);
		if (compcode) {
			call initcompany(compcode);
		}

		//call changelog.subs('SELECT':fm:data)
		gosub select0();
		if (not LISTACTIVE) {
			call mssg("Error: No records found");
			return 0;
		}

		//call changelog.subs('LIST':fm:data)
		gosub list();

	//called from LOGIN.NET. not UI. UI calls SELECTANDLIST
	} else if (mode.f(1) eq "WHATSNEW") {

		var menucodes = mode.f(2);
		mode = mode.f(1);
		ANS = "";

		var users;
		if (not(users.open("USERS", ""))) {
			return 0;
		}

		var userrec;
		if (userrec.read(users, USERNAME)) {

			//backward compatible - can be deleted after all upgraded
			//leave in case reloading ancient data
			if (not(userrec.f(17))) {
				var changelogkey = "USER*" ^ USERNAME;
				if (changelog.read(DEFINITIONS, changelogkey)) {
					userrec(17) = changelog.f(8);
					userrec.f(17).writev(users, USERNAME, 17);

					var("").writev(DEFINITIONS, changelogkey, 8);

				}
			}

			//mode<2>=changelog<7>
			//list all items in changelog since the EXODUS version in the user file
			mode(3) = userrec.f(17);

			//fix a problem where people were missing most changes
			//due to sv being represented as : eg user:support:technical
			if (mode.f(3) and mode.f(3) lt 14773) {
				mode(3) = 14153;
			}

		} else {

			//show everything the first time they logon
			//mode<3>=iconv('1/1/2004','D/E')

			//show nothing the very first time they logon
			ANS = "";
			return 0;

		}

		gosub getcurrentversiondatetime();

		//nothing to see if seen matches (or after?) currentversiondate
		if (mode.f(3) ge currentversiondatetime) {
			ANS = "";
			return 0;
		}

		currentversiondatetime.writev(users, USERNAME, 17);

		//build preferences from menus if not specified
		if (not(mode.f(2))) {

			//tt=capitalise(menucodes)
			//swap 'Support' with 'Technical' in tt
			//mode<2>=tt
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

			//everybody gets User Interface changes
			mode(2, -1) = "User Interface";

			//if no preferences then no whats new
			//if mode<2> else
			// @ans=''
			// return
			// end

		}

		//indicate need to select whats new preferences (by numeric whatsnew)
		//if mode<2> else
		// if currentversiondate else currentversiondate='0'
		// @ans=currentversiondate
		// return
		// end

		//find and new items else quit
		gosub select();
		if (not LISTACTIVE) {
			ANS = "";
			return 0;
		}

		//make a suitable output filename based on the responsefilename
		var temp = PRIORITYINT.f(100);
		temp.paster(-1, 1, "htm");
		SYSTEM(2) = temp;

		//get new items in new filename and return the filename in @ans
		gosub list();
		ANS = SYSTEM.f(2);

	//actually this is GETINSTALLEDVERSION DATES!
	} else if (mode eq "GETVERSIONDATES") {

		gosub getversiondates();

	} else if (mode.f(1) eq "SELECT") {

		gosub select0();

	} else if (mode.f(1) eq "LIST") {

		gosub list();

	}

	return 0;
}

subroutine select0() {

	//get the one date prior (so list is "as at")

	//get list of installed version dates
	gosub getversiondates();

	if (mode.f(3)) {
		//data contains a list of installed version dates
		//(not dates that upgrades were done!)
		//get the first installed version date equal to or prior to the selected
		//locate mode<3> in data by 'AR' using fm setting versionn else
		//locatebyusing() not available in c++
		tt = data_;
		tt.converter(FM, VM);
		if (not(tt.locateby("AR", mode.f(3), versionn))) {
			if (versionn gt 1) {
				mode(3) = USER1.f(versionn - 1);
			}
		}
	}

	gosub select();
	return;
}

subroutine select() {
	data_ = mode.field(FM, 2, 9999);

	//force all
	//data<2>=0

	cmd = "SELECT CHANGELOG";
	var andx = "";
	if (USER1.f(1)) {
		cmd ^= " WITH KEYWORD " ^ quote2(data_.f(1));
		andx = " AND";
	}
	if (USER1.f(2)) {
		cmd ^= andx ^ " WITH DATE GE " ^ (data_.f(2).oconv("D4").quote());
		andx = " AND";
	}
	if (USERNAME ne "EXODUS") {
		//cmd:=andx:' WITH DISTRIBUTION ""'
		//andx=' AND'
		//cmd:=' "User"'
		cmd ^= andx ^ " ( WITH NO DISTRIBUTION OR WITH DISTRIBUTION = \"User\" )";
		andx = " AND";
	}
	call safeselect(cmd ^ " (S)");

	return;
}

subroutine list() {
	//input
	//data<1>=mv list of topics
	//data<2>=date from which interested
	//mode<2>

	USER1 = mode.field(FM, 2, 9999);

	cmd = "LIST CHANGELOG ID-SUPP";
	//cmd:=' KEYWORD TEXT'
	cmd ^= " DATE";
	cmd ^= " KEYWORDS TEXT2";

	//cmd:=' BY KEYWORD'
	cmd ^= " BY KEYWORDS";
	cmd ^= " BY NUMBER";

	var headingx = "What''s New in EXODUS";
	gosub getcurrentversiondatetime();
	if (data_.f(2)) {
		//heading:=' version ':data<2> '[DATE,4*]':' -'
		call daterangetext(USER1.f(2), currentversiondatetime, tt, sys.glang);
		headingx ^= " : " ^ tt;
	} else {
		headingx ^= " Version : " ^ oconv(currentversiondatetime, "[DATE,4*]");
	}

	if (data_.f(1)) {
		headingx ^= "'L'" ^ USER1.f(1);
		headingx.replacer(SM, ", ");
		headingx.replacer(VM, ", ");
	}

	cmd ^= " HEADING " ^ (headingx.quote());

	perform(cmd);
	return;

	/*;
	///////
	import:
	///////

		importfilename='exodus.txt';

		osread doc from importfilename else call fsmsg();stop;

		convert "\r\n" to fm in doc;

		swap '_':fm with '\r' in doc;

		doc=trim2(doc,fm);
		nlines=count(doc,fm)+(doc<>'');

		//validate
		update=0;
		errors=0;
		gosub process;
		if errors then stop;

		//load
		update=1;
		idates='';
		gosub process;

		stop;

	////////
	process:
	////////
		for linen=1 to nlines;

			line=trimf(trimb(doc<linen>));

			//skip comments
			if line[1,1]='#' then goto nextline;

			line1=ucase(field(line,':',1));

			date=field(line1,' ',1);
			idate=iconv(date,@date.format);
			if idate else;
				call msg('wrong date in ':line);
				errors+=1;
				goto nextline;
				end;

			rec=idate;

			ucline=ucase(line);
			line1=field(line1,' ',2,9999);

			//look for keywords
			for keywordn=1 to nkeywords;
				keyword=keywords<1,keywordn>;
				if index(line1,keyword,1) then;
					//rec<keywordn>=1
					rec<2,-1>=keyword;
					swap keyword with '' in line1;
				end else;
					if keyword='BUG' and index(ucline,' BUG',1) then rec<keywordn>=1;
					end;
				next keyword;

			convert '/,' to '' in line1;

			if trim(line1) then;
				call msg(line1:' is what?');
				errors+=1;
				end;

			text=field(line,':',2,9999);
			swap '\r' with vm in text;

			rec<3>=text;

			if update then;
				locate idate in idates<1> setting idaten else idates<1,idaten>=idate;
				idatekey=(idate:'.':idates<2,idaten>) 'MD20PZ';
				idates<2,idaten>=idates<2,idaten>+1;
				write rec on changelog,idatekey;
				end;

	nextline:
			next linen;

		return;
	*/

}

subroutine getcurrentversiondatetime() {
	//get currently installed version date
	//temp=xlate('DOS','.\general\version.dat',1,'x')
	var temp = "./general/version.dat";
	temp.converter("/", OSSLASH);
	//temp = temp.xlate("DOS", 1, "X");
	temp = osread(temp);
	if (not(VOLUMES)) {
		if (temp eq "") {
			temp = EXECPATH.osfile();
			//19:05:55  18 NOV 2020
			temp = temp.f(3).oconv("MTS") ^ " " ^ temp.f(2).oconv("D");
		}
	}
	currentversiondatetime = temp.trim().field(" ", 2, 999).iconv("D");
	currentversiondatetime ^= "." ^ temp.trim().field(" ", 1).iconv("MT").oconv("R(0)#5");
	return;
}

subroutine getversiondates() {
	//extract installed version dates from upgrade.cfg
	call osread(versionlog, "upgrade.cfg");
	//versionlog=trim(field(versionlog,\1A\,1))
	versionlog.converter("\r\n", FM);
	let nn = versionlog.count(FM) + (versionlog ne "");
	var versiondata = "";
	for (const var ii : range(1, nn)) {
		idate = versionlog.f(ii, 1).field(" ", 2, 3).iconv("D");
		//itime=iconv(field(versionlog,' ',1),'MT')
		if (not(versiondata.locateusing(FM, idate, xx))) {
			versiondata(-1) = idate;
		}
	} //ii;

	//list of installed versions
	data_ = versiondata;
	return;
}

libraryexit()
