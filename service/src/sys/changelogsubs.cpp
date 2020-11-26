#include <exodus/library.h>
libraryinit()

#include <openfile.h>
#include <initcompany.h>
#include <quote2.h>
#include <safeselect.h>
#include <daterangetext.h>
#include <singular.h>
#include <authorised.h>

#include <gen_common.h>
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

	#include <general_common.h>
	//global mode,text,ucline,rec,idate,date,line1,idates,errors,update,keywords,nkeywords,cmd,nlines

	#define data USER1
	keywords = "MEDIA" _VM_ "JOBS" _VM_ "FINANCE" _VM_ "TIMESHEETS" _VM_ "TECHNICAL" _VM_ "USER INTERFACE";
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

	if (mode.a(1) == "SELECTANDLIST") {

		//change to the user selected company
		//merely to get the right letterhead
		var compcode = mode.a(5);
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

			//backward compatible - can be deleted after all upgraded
			//leave in case reloading ancient data
			if (not(userrec.a(17))) {
				var changelogkey = "USER*" ^ USERNAME;
				if (changelog.read(DEFINITIONS, changelogkey)) {
					userrec.r(17, changelog.a(8));
					userrec.a(17).writev(users, USERNAME, 17);

					var("").writev(DEFINITIONS, changelogkey, 8);

				}
			}

			//mode<2>=changelog<7>
			//list all items in changelog since the EXODUS version in the user file
			mode.r(3, userrec.a(17));

			//fix a problem where people were missing most changes
			//due to sv being represented as : eg user:support:technical
			if (mode.a(3) and (mode.a(3) < 14773)) {
				mode.r(3, 14153);
			}

		}else{

			//show everything the first time they logon
			//mode<3>=iconv('1/1/2004','D/E')

			//show nothing the very first time they logon
			ANS = "";
			return 0;

		}

		gosub getcurrentversiondatetime();

		//nothing to see if seen matches (or after?) currentversiondate
		if (mode.a(3) >= currentversiondatetime) {
			ANS = "";
			return 0;
		}

		currentversiondatetime.writev(users, USERNAME, 17);

		//build preferences from menus if not specified
		if (not(mode.a(2))) {

			//tt=capitalise(menucodes)
			//swap 'Support' with 'Technical' in tt
			//mode<2>=tt
			if (menucodes.index("FINANCE")) {
				mode.r(2, -1, "Finance");
			}
			if (menucodes.index("MEDIA")) {
				mode.r(2, -1, "Media");
			}
			if (menucodes.index("JOBS")) {
				mode.r(2, -1, "Jobs");
			}
			if (menucodes.index("TIMESHEETS")) {
				mode.r(2, -1, "Timesheets");
			}
			if (menucodes.index("SUPPORT")) {
				mode.r(2, -1, "Technical");
			}

			//everybody gets User Interface changes
			mode.r(2, -1, "User Interface");

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
		var temp = PRIORITYINT.a(100);
		temp.splicer(-1, 1, "htm");
		SYSTEM.r(2, temp);

		//get new items in new filename and return the filename in @ans
		gosub list();
		ANS = SYSTEM.a(2);

	//actually this is GETINSTALLEDVERSION DATES!
	} else if (mode == "GETVERSIONDATES") {

		gosub getversiondates();

	} else if (mode.a(1) == "SELECT") {

		gosub select0();

	} else if (mode.a(1) == "LIST") {

		gosub list();

	}

	return 0;
}

subroutine select0() {

	//get the one date prior (so list is "as at")

	//get list of installed version dates
	gosub getversiondates();

	if (mode.a(3)) {
		//data contains a list of installed version dates
		//(not dates that upgrades were done!)
		//get the first installed version date equal to or prior to the selected
		//locate mode<3> in data by 'AR' using fm setting versionn else
		//locatebyusing() not available in c++
		tt = USER1;
		tt.converter(FM, VM);
		if (not(tt.locateby("AR",mode.a(3),versionn))) {
			if (versionn > 1) {
				mode.r(3, USER1.a(versionn - 1));
			}
		}
	}

	gosub select();
	return;
}

subroutine select() {
	USER1 = mode.field(FM, 2, 9999);

	//force all
	//data<2>=0

	cmd = "SELECT CHANGELOG";
	var andx = "";
	if (USER1.a(1)) {
		cmd ^= " WITH KEYWORD " ^ quote2(USER1.a(1));
		andx = " AND";
	}
	if (USER1.a(2)) {
		cmd ^= andx ^ " WITH DATE GE " ^ (USER1.a(2).oconv("D4").quote());
		andx = " AND";
	}
	if (USERNAME ne "EXODUS") {
		cmd ^= andx ^ " WITH DISTRIBUTION \"\"";
		andx = " AND";
		//if security('DOCUMENTATION ACCESS USER') then
		cmd ^= " \"User\"";
		// end
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

	var headingx = "What\'\'s New in EXODUS";
	gosub getcurrentversiondatetime();
	if (USER1.a(2)) {
		//heading:=' version ':data<2> '[DATE,4*]':' -'
		call daterangetext(USER1.a(2), currentversiondatetime, tt, gen.glang);
		headingx ^= " : " ^ tt;
	}else{
		headingx ^= " Version : " ^ oconv(currentversiondatetime, "[DATE,4*]");
	}

	if (USER1.a(1)) {
		headingx ^= "\'L\'" ^ USER1.a(1);
		headingx.swapper(SVM, ", ");
		headingx.swapper(VM, ", ");
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
	var temp = var("./general/version.dat").xlate("DOS", 1, "X");
	temp.converter("/", OSSLASH);
	currentversiondatetime = temp.trim().field(" ", 2, 999).iconv("D");
	currentversiondatetime ^= "." ^ temp.trim().field(" ", 1).iconv("MT").oconv("R(0)#5");
	return;
}

subroutine getversiondates() {
	//extract installed version dates from upgrade.cfg
	call osread(versionlog, "upgrade.cfg");
	//versionlog=trim(field(versionlog,\1A\,1))
	versionlog.converter("\r\n", FM);
	var nn = versionlog.count(FM) + (versionlog ne "");
	var versiondata = "";
	for (var ii = 1; ii <= nn; ++ii) {
		idate = versionlog.a(ii, 1).field(" ", 2, 3).iconv("D");
		//itime=iconv(field(versionlog,' ',1),'MT')
		if (not(versiondata.locateusing(FM,idate,xx))) {
			versiondata.r(-1, idate);
		}
	};//ii;

	//list of installed versions
	USER1 = versiondata;
	return;
}

libraryexit()