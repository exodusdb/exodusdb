#include <exodus/library.h>
libraryinit()

#include <log2.h>
#include <initagency.h>
#include <initgeneral2.h>//alternatively it could be performed
#include <otherusers.h>
#include <authorised.h>
#include <logprocess.h>
#include <openfile.h>
#include <sysmsg.h>
#include <initcompany.h>
#include <initacc.h>
#include <readbakpars.h>
#include <usersubs.h>
#include <addcent.h>

#include <gen.h>
#include <fin.h>

var logtime;
var neosysid;
var pidrec;
var dbdate;
var dbtime;
var dbdatetimerequired;
var msg;
var reply;//num
var reply2;
var temp;
var colors;
var tt;//num
var smtp;
var vn;
var os;
var ver;
var errors;
var shadow;
var log;
var dostime;
var bakpars;
var version;
var upgradelog;
var codepaging;
var systemx;

var xx;

function main() {
	//return true or false to caller to abort
	var thisname="initgeneral";

	call log2("-----initgeneral init", logtime);

	//CREATE LABELLED COMMON
	mv.labelledcommon[2]=new gen_common;
	if (not iscommon(gen)) {
		var().stop("gen common is not initialised in " ^ thisname);
	}

	//unfortunately general needs finance for now until some finance commons
	//get moved into general common (and this moved into initacc)
	mv.labelledcommon[3]=new fin_common;
	if (not iscommon(fin)) {
		var().stop("fin common is not initialised in " ^ thisname);
	}

gen.gcurrcompany="";
gen.company="";
USERNAME="";
if (not gen._definitions.open("DEFINITIONS")) {
	var().stop("cant open DEFINITIONS");
}

	var resetting = mv.PSEUDO == "RESET";

	var initmode = SENTENCE.field(" ", 2);
	mv.PSEUDO = "";
	//equ request to @user0
	//equ data to @user1
	//equ response to @user3
	//equ msg to @user4
	//@user2 is used to hold the current base currency format
	//@user4 is used to hold validation/error messages for non-interactive progs

	//equ dosenv to system<1> command line parameters set in logon.old
	//following can be blanked to indicate that the proxy has already responded to the user
	//(used to run detached processes)
	//equ getfilename to system<2>;*TRANSACTION printfile name while performing print program
	//equ getflag to system<3>;*1=printing program has reset setptr to prn and done its own printing
	//equ getprintern to system<4>;*OBSOLETE SESSION default printer number
	//equ sessionid to system<4>;*SESSION WAS system<32>
	var sessionid = SYSTEM.a(4);
	//equ printptr to system<5>;*used in BP and GBP/MSG3
	//equ servermode to system<6>;1=yes OBSOLETE
	//equ html to system<7>;OBSOLETE 1=output html if possible;2=output html
	//equ clientwebsite to system<8>;*eg www.neosys.com
	//equ nprocessors to system<9>
	//equ email to system<10>;* eg reception@neosys.com
	//equ fromname to system<10,1>;*username
	//equ fromemail to system<10,2>;*useremail
	//equ smtphost to system<10,3>;*system or workstation
	//equ toemail to system<10,4>;*transaction
	//equ subject to system<10,5>;*transaction
	//equ agencyaddress to system<18>
	//equ agencycontacts to system<19>
	//equ htmlviewer to system<11> ;*OBSOLETE may contain 1,2,3 from old
	//equ envvars to system<12>
	//equ envstrs to system<13>
	//equ currentclientmark to system<14>
	//equ masterlogin to system<15> SESSION
	//equ getcleared to system<16>;*SESSION flag per session indicating that documents file has been cleared of document: that no longer exist
	//equ docscleaned to system<16>
	//equ currdatasetcode to system<17> ;*SESSION
	//equ outsidebackup to system<20>
	//equ preventauth to system<21>
	//equ sleepsecs to system<22> no longer used
	//equ currdatasetname to system<23>;*SESSION
	//equ process number/connection number to system<24>;*SESSION

	//processing time should not be set until any init convertion is over
	//otherwise esc.to.exit/giveway might abort processing
	//equ processingstarttime to system<25>
	//equ processingslepttime to system<26>
	SYSTEM.r(25, "");

	//equ last processes file update date.time system<27>

	//equ deleteoldsecs to system<28>
	//equ suspendwaiting to system<29>
	//equ maindataset to system<30>
	//equ accounting to system<31>
	//equ getlevel to system<32>;* while performing a print program was system<4>
	//accounting<1>=1 for accounting system present
	//accounting<2>=1 for only stock ledgers
	//accounting<3> for allocationorder
	//accounting<4> for mv buffer of currency or unitcodes
	//accounting<5> for mv buffer of fmts for <4>
	var interactive = not SYSTEM.a(33);
	//equ generalresultcode to system<34>
	//equ nrequests=system<35>
	//equ defaultclientmark to system<36>
	//equ maincompanycode to system<37>
	//equ datasetno to system<38>;*SESSION
	//equ defaultallowedIPranges to system<39>
	//equ currentconnection to system<40>
	//equ current responsefilename to system<41>
	//equ backupfilename to system<42>
	//equ originalusername to system<43>;*SESSION
	//equ originalstation to system<44>;*SESSION
	//equ globaldatasetid to system<45>;*SESSION

	//equ thcolor=system<46,1>
	//equ tdcolor=system<46,2>
	//equ reportfont=system<46,3>
	//equ formheadcolor=system<46,4>
	//equ formbodycolor=system<46,5>
	//equ formfont=system<46,6>
	//equ formfontsize=system<46,7>
	//equ reportfontsize to system<46,8>

	//equ systemstyle=system<47> copy of 46 for default (46 can be per user)
	//equ processlockno=system<48>
	//equ uploadpath to system<49>
	//equ cygwinbinpath to system<50>
	//only ACCOUNTS or ADAGENCY at the moment
	//equ systemcode to system<51>
	//equ processclosed to system<52>
	//equ maintenancestatus to system<53>
	//<54,5> is pid
	//equ connection to system<54>
	//equ controlurl to system<55>
	//equ httpproxy to system<56>;*httphost:port vm bypasslist vm user vm pass
	//equ installationdesc to system<57>;*should be like hostname
	//equ databasecodes to system<58> *multivalued
	//equ numprocessesreq to system<59> *mv with 58
	//equ backupreq to system<60> *mv with 58
	//system<61> to testdata or notlivedata
	//system<62> to copyreq mv with 58
	//system<63> to testdatabasecode mv with 58
	//free 63-70
	//system<71> to system<99> used for backup.cfg details
	//system<99>
	//system<100> mv times of SYSTEM.CFG,..\..\SYSTEM.CFG etc detect reconfig
	//system<101> to system<109> used for smtp.cfg details
	//system<110> to notes
	//system<111> to cid()
	//system<112> to installfilename0
	//system<113> to dicttempvars
	//system<114> to mv webaddress
	//system<115> to mv webaddressdescription
	//system<116> to email domains separated by spaces
	//system<117> to forcedemail
	//system<118> to timeoffset
	//system<119> to addsignature blank/n=no a=above b=below l=left r=right
	//system<120> to sysgmtoffset - auto detected in cid()
	//system<121> to syncdata required (every hour to ..\data2\dataset.1/2/3
	//system<122> to emailblockingusers
	//system<123> to installationgroup defaults to GLOBAL (for ..\..\GLOBAL.END)

	//std::wcout<<"Opening Definitions ... "<<std::flush;
	var definitionsfilename="DEFINITIONS";
	if (!DEFINITIONS.open(definitionsfilename))
	{
		if (!DEFINITIONS.createfile(definitionsfilename)||!DEFINITIONS.open(definitionsfilename))
		{
			std::wcerr<<"Cannot create "<<definitionsfilename<<std::endl;
			return false;
	}
	}
	//std::wcout<<"OK"<<std::endl;

	if (not neosysid.osread("NEOSYS.ID")) {
		neosysid = "";
	}

	call log2("*determine the pid if possible", logtime);
	if (not SYSTEM.a(54)) {
		SYSTEM.r(54, SYSTEM.a(33));
	}
	var pidfilename = SYSTEM.a(54, 5) ^ ".pid";
	call osread(pidrec, pidfilename);
	pidfilename.osdelete();
	if (pidrec) {
		SYSTEM.r(54, 5, pidrec.a(1));
	}

	//check version of database versus version of program
	//prevent login (except as maintenance with confirmation)
	//NB this date should be revised whenever any conversion or upgrade is done
	if (not neosysid) {
		var oldmethod = 1;
		if (oldmethod) {
			dbdate = "30 MAR 2007";
			dbtime = "12:00:00";
			dbdatetimerequired = dbdate.iconv("D") + dbtime.iconv("MT") / 86400;
		}else{
		}
		if (gen._definitions.open("DEFINITIONS")) {
			var dbversion;
			if (not(dbversion.read(gen._definitions, "DBVERSION"))) {
				goto updateversion;
			}
			if (oldmethod and dbversion.a(1) == 14334.5) {
			}
			if (dbversion.a(1) > dbdatetimerequired) {
				msg = "Software version " ^ (dbversion.a(1)).oconv("D") ^ " " ^ (dbversion.a(1).field(".", 2)).oconv("MTS") ^ " is incompatible with" ^ FM ^ "Database version " ^ dbdate ^ " " ^ dbtime;
				msg = msg.oconv("L#60");
				//abort since db is advanced
				var options;
				var msgx;
				if (not interactive) {
badversion:
					USER4 = msg;
					gosub fail();

					return false;

				}else{
decideversion:
					options = "Quit (RECOMMENDED)";
					options.r(-1, "Continue");
					options.r(-1, "Mark database as version " ^ dbdate ^ " " ^ dbtime ^ " and continue");
					if (not(decide("!" ^ msg ^ "", options, reply))) {
						reply = 1;
					}
					if (reply == 1) {
						goto badversion;
					}
					msgx =
						"!WARNING *UNPREDICTABLE* CONSEQUENCES"  _FM_
						"WHAT IS THE PASSWORD?";
					input(msgx, reply2);
					if (reply2 ne "UNPREDICTABLE") {
						call mssg("THAT IS NOT THE CORRECT PASSWORD");
						goto decideversion;
					}
					if (reply == 3) {
						goto updateversion;
					}
				}
			} else if (dbversion.a(1) < dbdatetimerequired) {
updateversion:
				dbversion = dbdatetimerequired;
				dbversion.r(2, dbdate);
				dbversion.r(3, dbtime);
				dbversion.write(gen._definitions, "DBVERSION");
			}
		}
	}

	call log2("*save the original username and station", logtime);
	if (not SYSTEM.a(43)) {
		SYSTEM.r(43, USERNAME);
	}
	if (not SYSTEM.a(44)) {
		SYSTEM.r(44, mv.STATION.trim());
	}

	call log2("*convert reports file", logtime);
	var reports;
	if (reports.open("REPORTS")) {
		reports.select();
		var reportkey;
		while (reports.readnext(reportkey)) {
			var reportkey;
			if (RECORD.read(reports, reportkey)) {
				var filename = reportkey.field("*", 1);
				var file;
				if (file.open(filename)) {
					var keyx = reportkey.field("*", 2);
					keyx.swapper("%2A", "*");
					if (RECORD.a(1) == "%DELETED%") {
						var rec;
						if (rec.read(file, keyx)) {
							//if rec<1>='NEOSYS' then delete file,keyx
							//if rec<8>='NEOSYS' then delete file,keyx
							file.deleterecord(keyx);
						}
					}else{
						var oldrecord;
						if (not(oldrecord.read(file, keyx))) {
							oldrecord = "";
						}
						//only update documents if changed anything except update timedate
						if (filename == "DOCUMENTS") {
							oldrecord.r(8, RECORD.a(8));
						}
						if (RECORD ne oldrecord) {
							RECORD.write(file, keyx);
						}
					}
				}
			}
		}
	}

	//if OPENFILE('DEFINITIONS',DEFINITIONS) then null
	if (not(gen._definitions.open("DEFINITIONS"))) {
		var().chr(7).output();
		msg = "The DEFINITIONS file is missing";
		msg.r(-1, "Did you startup using the right command file/datasettype?");
		call mssg(msg);
	}

	call log2("*count the number of other processes", logtime);
	var notherusers = otherusers("").a(1);

	call log2("*check the operating system date", logtime);
//checksysdate:
	var config = "";
	if (not config.osread("NEOSYS.CFG")) {
		if (config.osread("\\lastdate.rev\\")) {
			{}
		}
	}

	var lastdate = config.a(1);

	call log2("*update last used date", logtime);
	if (var().date() ne lastdate) {
		config.r(1, var().date());
		call oswrite(config, "NEOSYS.CFG");
	}

	call log2("*check for invalid characters in workstation name", logtime);
	mv.STATION.converter(SQ^DQ,"");

	//save current system so we can restore various runtime parameters
	//which are unfortunately stored with configuration params

	var oldsystem = SYSTEM;

	call log2("*get DEFINITIONS SYSTEM parameters", logtime);
	//do in reverse order so that the higher levels get priority
	if (not(SYSTEM.read(gen._definitions, "SYSTEM"))) {
		SYSTEM = "";
	}
	gosub getsystem("SYSTEM.CFG", 2);
	gosub getsystem("..\\..\\" "SYSTEM.CFG", 1);

	//default installation group is GLOBAL
	if (not SYSTEM.a(123)) {
		SYSTEM.r(123, "GLOBAL");
	}

	call log2("*determine systemid from old smtp sender name", logtime);
	if (not SYSTEM.a(57)) {
		call osread(smtp, "..\\..\\SMTP.CFG");
		if (not smtp.a(1)) {
			call osread(smtp, "SMTP.CFG");
		}
		if (not smtp.a(1)) {
			if (not(smtp.read(gen._definitions, "SMTP.CFG"))) {
				{}
			}
		}
		if (smtp.a(1)) {
			var sysname = smtp.a(1).field("@", 1).lcase();
			//remove all punctuation
			sysname.converter("!\"$%^&*()_+-=[]{};:@,./<>?", "");
			SYSTEM.r(57, sysname);
			call osread(tt, "SYSTEM.CFG");
			tt.r(57, sysname);
			call oswrite(tt, "SYSTEM.CFG");
		}
	}

	//force reevaluation of cid
	//SYSTEM.r(111, "");
	//SYSTEM.r(111, cid());

	SYSTEM.r(51, ACCOUNT);

	SYSTEM.r(33, oldsystem.a(33));

	call log2("*restore session parameters", logtime);
	SYSTEM.r(1, oldsystem.a(1));
	//masterlogin
	SYSTEM.r(15, oldsystem.a(15));
	//cleaned
	SYSTEM.r(16, oldsystem.a(16));
	SYSTEM.r(17, oldsystem.a(17));
	//sessionid
	SYSTEM.r(4, oldsystem.a(4));
	//server mode (not interactive)
	var s33 = SYSTEM.a(33);
	//currdatasetname
	SYSTEM.r(23, oldsystem.a(23));
	//process/connection number
	SYSTEM.r(24, oldsystem.a(24));
	//datasetno
	SYSTEM.r(38, oldsystem.a(38));
	//originalusername
	SYSTEM.r(43, oldsystem.a(43));
	//original station id
	SYSTEM.r(44, oldsystem.a(44));
	//globaldatasetid
	SYSTEM.r(45, oldsystem.a(45));
	//processlock
	SYSTEM.r(48, oldsystem.a(48));
	SYSTEM.r(54, oldsystem.a(54));

	oldsystem = "";

	//call log2('*determine time offset')
	//blank means dont offset - use system server timezone whatever it is
	//NB system<118>should be configured PER DATABASE not per INSTALLATION
	//otherwise moving a database will get confused if the new server is not gmt
	//@sw<1> is the ADJUSTMENT to get display time from server time
	//@sw<2> is the difference from gmt/utc to server time
	//@sw<3> could be the adjustment to get dbtz from servertz
	mv.SW = "";
	//now ONLY supporting display time different from server time on gmt/utc servers
	if (SYSTEM.a(120)) {
		//if server not on gmt/utc and user tz is set then warning
		//since a) usertime will be server time b) database will get non-gmt date/times
		if (SYSTEM.a(118)) {
			call mssg("WARNING: User time zone ignored and|Database storing non-GMT/UTC date/time|because current server is not GMT/UTC");
		}
	}else{
		mv.SW.r(1, SYSTEM.a(118));
		//if display time is not server/gmt/utc then adjust offset to server/gmt/utc
		if ((mv.SW.a(1)).length()) {
			//system time offset is currently automatically determined by CID()
			//CID will tell you the current server tz
			//but should this be recorded in the dataset in case it is moved
			//between servers with different default timezones
			//this issue can be avoided if all servers are kept on gmt/utc+0 timezone

			//NB currently the only neosys databases with tz in system<118> are on nl1/nl1b
			//where the server is configured to gmt

			//NB
			//the @sw<2> (server tz) is only used when local tz is in system<118> (even 0)
			//THEREFORE if you want to add tz in system<118> on non-gmt servers
			//you should run CHANGETZ to standardised the database datetimes to gmt
			//so that the database can thereafter be moved to servers on any tz

			//best standardisation procedure
			//CHANGETZ to gmt/utc
			//put something in database system<118> (even 0 for display in gmt)

	/*meaningless while we only support user tz if server is gmt/utc;
				//cid sets system<120> server tz (only use server tz if display tz set)
				tt=system<120>;
				@sw<1>=@sw<1>+tt;
				if tt then @sw<2>=tt;
	*/
		}
	}

	call log2("*determine upload path", logtime);
	tt = SYSTEM.a(49);
	if (tt == "") {
		tt="..\\images\\";
	} else if (tt[-1] ne SLASH) {
		tt ^= SLASH;
	}
	SYSTEM.r(49, tt).convert("\\/", SLASH^SLASH);

	call log2("*default form and report color", logtime);
	if (not(SYSTEM.a(46, 1))) {
		SYSTEM.r(46, 1, "#FFFF80");
	}
	if (not(SYSTEM.a(46, 2))) {
		SYSTEM.r(46, 2, "#FFFFC0");
	}
	//if system<46,3> else system<46,3>=''

	call log2("*backup the system default style", logtime);
	SYSTEM.r(47, SYSTEM.a(46));

	call log2("*first uses of getenv", logtime);
	if (not(osgetenv("OS", os))) {
		os = "";
	}
	if (not(osgetenv("VER", ver))) {
		ver = "";
	}
	if (not(osgetenv("NUMBER_OF_PROCESSORS", tt))) {
		tt = 1;
	}
	SYSTEM.r(9, tt);

	//done in logon.old now
	//call log2('*check/detect current dataset',logtime)
	//currdataset=field(definitions[index(ucase(definitions),'\':'DATA':'\',1)+6,9999],'\',1)
	//system<17>=currdataset
	var currdataset = SYSTEM.a(17);

	call log2("*try to update upload.dll", logtime);
	for (var extn = 1; extn <= 2; ++extn) {
		var ext = var("net,w3c").field(",", extn);
		var path = "..\\neosys." ^ ext ^ "\\neosys\\dll\\";
		tt = (path ^ "upload.dl_").osfile();
		if (tt) {
			if (tt ne (path ^ "upload.dll").osfile()) {
				call osdelete(path ^ "upload.dll");
				if (not((path ^ "upload.dll").osfile())) {
					//call shell('ren ':path:'upload.dl_ upload.dll')
					osshell("copy " ^ path ^ "upload.dl_ " ^ path ^ "upload.dll");
				}
			}
		}
	};//extn;

	call log2("*find cygwin", logtime);
	var locations = SYSTEM.a(50);
	locations.r(1, -1, "C:\\CYGWIN\\BIN\\" _VM_ "\\CYGWIN\\BIN\\" _VM_ "..\\..\\CYGWIN\\BIN\\");
	var nn = locations.count(VM) + 1;
	for (var ii = 1; ii <= nn; ++ii) {
		var location = locations.a(1, ii);
		if (location[-1] ne "\\") {
			location ^= "\\";
		}
		tt = var().oslistf(location ^ "*.*");
		if (tt) {
			SYSTEM.r(50, location);
		}
		if (tt == "") {
		};//ii;
	}

	/* using proxycfg;
	The following example specifies that HTTP servers are accessed through;
	the http_proxy proxy and HTTPS servers are accessed through https_proxy.;
	Local intranet sites and any site in the *.microsoft.com domain bypass the proxy.;
		proxycfg -p "http=http_proxy https=https_proxy" "<local>;*.microsoft.com";
	Removing ProxyCfg.exe;
	To remove the registry entries that ProxyCfg.exe creates,;
	you must delete the WinHttpSettings value from the following registry key.;
		HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Internet Settings\Connections\WinHttpSettings;
	Deleting the WinHttpSettings value removes all proxy configurations.;
	*/
	call log2("*find http proxy", logtime);
	if (SYSTEM.a(56) == "") {
		var cmd = "proxycfg";
		var result;
getproxy:
		result = cmd.osshellread().lcase();
		tt = result.index("proxy server(s)", 1).convert("\n","\r");
		if (tt) {
			result=result.substr(tt);
			tt = result.field(";", 1).field(var().chr(13), 1);
			tt = tt.field(":", 2, 99).trim();
			tt.swapper("http=", "");
		} else if (cmd == "proxycfg") {
			cmd = "netsh winhttp import proxy ie";
			goto getproxy;
		} else {
			tt = "";
		}
		//todo get proxybypasslist
		SYSTEM.r(56, tt);
	}

	call log2("*put up the backup reminder", logtime);
	var reminder = "";
	if (not SYSTEM.a(20) and not resetting) {
		call backupreminder(currdataset);
	}

	call log2("*default date format is DD/MM/YY", logtime);
	mv.DATEFORMAT = "D2/E";

	call log2("*get security ... also in LISTEN", logtime);
	if (not(gen._security.read(gen._definitions, "SECURITY"))) {
		if (temp.open("DICT", "DEFINITIONS")) {
			if (gen._security.read(temp, "SECURITY")) {
				gen._security.write(gen._definitions, "SECURITY");
			}
		}
	}
//	gen._security = gen._security.invert();

	call log2("*create user file", logtime);
	var users;
	if (not(users.open("USERS"))) {

		if (not(openfile("USERS", users, "DEFINITIONS"))) {
			{}
		}

		call log2("*zzz should create full user record not just the name", logtime);
		var usercodes = gen._security.a(1);
		var nusers = usercodes.count(VM) + (usercodes ne "");
		for (var usern = 1; usern <= nusers; ++usern) {
			var USER = usercodes.a(1, usern);
			if (not(USER.index("---", 1))) {
				USER.writev(users, USER, 1);
			}
		};//usern;

	}

	call log2("*terminate old session", logtime);
	if (sessionid ne "") {
		call logprocess(sessionid, "LOGOFF");
		SYSTEM.r(4, "");
	}

	call log2("*get user name", logtime);
	if (not resetting) {
		//call neosyslogin("INIT");
		USERNAME="SYSTEM";
	}

	call log2("*init.general waiting for exclusive access ...", logtime);
	var definitions;
	if (definitions.open("DEFINITIONS")) {
		//timeout and crash out quickly otherwise might get unlimited processes
		//this lock is in INIT.GENERAL and MONITOR2 (to avoid starting processes)
		if (not(lockrecord("DEFINITIONS", definitions, "INIT.GENERAL.LOGIN", "", 9))) {
			msg = "INIT.GENERAL couldnt get exclusive lock";
			//maybe a long upgrade process is running
			//call sysmsg(msg)
			//tt-=1
			//if tt then goto locksys
			call mssg(msg);
			if (SYSTEM.a(33)) {
				gosub fail();
			}

			//abort startup
			return false;
		}
	}

	call log2("*start new session", logtime);
	sessionid = "";
	call logprocess(sessionid, "LOGIN");
	SYSTEM.r(4, sessionid);

	call log2("*open general files", logtime);
	var valid = 1;
	gen._definitions = "";
	if (not(openfile("ALANGUAGE", xx, "DEFINITIONS"))) {
		valid = 0;
	}
	if (not(openfile("COMPANIES", gen.companies, "DEFINITIONS"))) {
		valid = 0;
	}
	if (not(openfile("CURRENCIES", gen.currencies, "DEFINITIONS"))) {
		valid = 0;
	}
	if (not(openfile("UNITS", gen.units, "DEFINITIONS"))) {
		valid = 0;
	}
	if (not(openfile("ADDRESSES", gen.addresses, "DEFINITIONS"))) {
		valid = 0;
	}
	if (not(openfile("DOCUMENTS", gen.documents, "DEFINITIONS"))) {
		valid = "";
	}
	if (not(openfile("TIMESHEETS", gen.timesheets, "DEFINITIONS"))) {
		valid = "";
	}

	if (not(openfile("CURRENCY_VERSIONS", xx, "CURRENCIES"))) {
		valid = "";
	}
	if (xx.open("MARKETS")) {
		if (not(openfile("MARKET_VERSIONS", xx, "MARKETS"))) {
			valid = "";
		}
	}
	if (not(openfile("COMPANY_VERSIONS", xx, "COMPANIES"))) {
		valid = "";
	}

	call log2("*open processes own lists file", logtime);
	var workdir = "NEOS" ^ (SYSTEM.a(24)).oconv("R(0)#4");

	//not needed yet
	//var workpath = "DATAVOL\\" ^ workdir ^ "\\";
	//if (not oslistf(workpath)) {
	//	osmkdir(workpath);
	//}

	call log2("*open/make/clear lists file", logtime);
	var lists;
	if (not(lists.open("LISTS"))) {
		var filename="LISTS";
		lists.createfile(filename);
		if (not(lists.open(filename))) {
			call sysmsg(DQ ^ (filename ^ DQ) ^ " could not be created by INIT.GENERAL");
		}
	}
	var().clearfile(lists);

	call log2("*make global process and statistics files", logtime);
	var FILENAMES = "PROCESSES" _VM_ "STATISTICS";
	for (var ii = 1; ii <= 999; ++ii) {
		var filename = FILENAMES.a(1, ii);
		if (filename) {
			//TODO lock/prevent double create with other processes
			var file;
			if (not(file.open(filename))) {
				file.createfile(filename);
				if (not(file.open(filename))) {
					call sysmsg(DQ ^ (filename ^ DQ) ^ " could not be created by INIT.GENERAL");
				}
			}
		};//ii;
	}

	call log2("*perform the autoexec task BEFORE initialising other systems", logtime);
	if (not neosysid) {
		if (openfile("DEFINITIONS", gen._definitions)) {
			if (temp.read(gen._definitions, "AUTOEXEC")) {
				perform("TASK AUTOEXEC");
			}
		}
	}

	call log2("*get first company for init.acc", logtime);
	gen.companies.clearselect();
	//TODO should be a sequence code on companies to sort the important companies first
	gen.companies.select();
	var companycode;
	gen.companies.readnext(companycode);

	call log2("*open accounts system files", logtime);
	if (xx.open("ACCOUNTS")) {
	call log2("*open accounts system files", logtime);

	call initacc();

	call log2("*open accounts system files", logtime);
	}
	call log2("*open accounts system files", logtime);

	call log2("*definitions file", logtime);
	//backward compatible with DEFINITIONS file in \data\accounts directory
	if (not(openfile("DEFINITIONS", gen._definitions))) {
		valid = 0;
	}

	call log2("*open advertising system files INIT.AGENCY", logtime);
	if (xx.open("SCHEDULES")) {
		call initagency();
		//perform("initagency");
	}
/* move to initagency
	call log2("*add new indexes", logtime);
	if (not(indices2("TIMESHEETS", "JOB_NO"))) {
		if (ACCOUNT == "ADAGENCY") {
			var("MAKEINDEX TIMESHEETS JOB_NO").execute();
			if (not(openfile("TIMESHEETS", gen.timesheets))) {
				gen.timesheets = "";
			}
		}
	}
*/
	call log2("*add number format to company records", logtime);
	gen.companies.select();
	var numberformat = "";
	var currcompany = "";
	while (gen.companies.readnext(currcompany)) {
		var tempcompany;
		if (tempcompany.read(gen.companies, currcompany)) {
			if (tempcompany.a(22) == "") {
				if (not numberformat) {
					if (not(decide("Which format do you want for numbers ?||(See \"NUMBER FORMAT\" on the company file)", "1.000,00 (dot for thousands)" _VM_ "1,000.00 (comma for thousands)", reply))) {
						reply = 2;
					}
					if (reply == 1) {
						numberformat = "1.000,00";
					} else
						numberformat = "1,000.00";
				}
				numberformat.writev(gen.companies, currcompany, 22);
			}
		}
	}

	call log2("*get the company description", logtime);
	gen.company = "";
	var currperiod = "";
	//call init.company('')
	//change so that interactive ADAGENCY gets a company code
	//force acquisition of language
	gen.glang = "";
	call initcompany(currcompany);

	log2("save gen.gcurrcompany", logtime);
	fin.currcompanycodes = currcompany;
	gen.gcurrcompany = currcompany;
	SYSTEM.r(37, gen.gcurrcompany);

	call log2("*check currency accounts", logtime);
	var account,accounts;
	if (accounts.open("ACCOUNTS")) {
		msg = "";
		if (not(account.read(accounts, gen.company.a(4)))) {
			msg.r(-1, gen.company.a(4));
		}
		if (not(account.read(accounts, gen.company.a(5)))) {
			msg.r(-1, gen.company.a(5));
		}
		if (not(account.read(accounts, gen.company.a(12)))) {
			msg.r(-1, gen.company.a(12));
		}
		if (not(account.read(accounts, gen.company.a(19)))) {
			msg.r(-1, gen.company.a(19));
		}
		if (msg) {
			var().chr(7).output();
			msg.swapper(FM, " ");
			if (interactive) {
				call mssg(DQ ^ (msg ^ DQ) ^ "|account missing - see company setup");
			}
		}
	}

	if (not SYSTEM.a(23))
		SYSTEM.r(23,"Default");
	if (not SYSTEM.a(17))
		SYSTEM.r(17,"exodus");

	call log2("*ensure random key exists", logtime);
	var datasetid;
	if (not(datasetid.read(gen._definitions, "GLOBALDATASETID"))) {
newdatasetid:
		dostime=ostime().round(2);
		datasetid = var().date() ^ "." ^ dostime;
		datasetid.converter(".", "");
		datasetid = datasetid.oconv("MX");
		datasetid = (datasetid ^ datasetid ^ datasetid ^ datasetid).substr(1, 8);
adddatasetcodename:
		datasetid.r(2, SYSTEM.a(23));
		datasetid.r(3, SYSTEM.a(17));
		datasetid.write(gen._definitions, "GLOBALDATASETID");
	}
	if (datasetid.a(3) == "") {
		goto adddatasetcodename;
	}

	//lock database to particular computers unless logging in as NEOSYS
	//if @username<>'NEOSYS' and datasetid<4> then
	//lock even to NEOSYS to prevent installation where NEOSYS pass is known
	//if (datasetid.a(4)) {
	//	if (not(datasetid.locate(cid(), xx, 4))) {
	//		USER4 = DQ ^ ("CANNOT USE THIS DATABASE ON THIS COMPUTER" ^ DQ);
	//		goto failbatch;
	//	}
	//}

	call log2("*readbakpars and decide livedb or testdb", logtime);
	call readbakpars(bakpars);
	SYSTEM.r(61, bakpars.a(11));

	call log2("*suggest change globaldatasetid if changed datasetname or datasetid", logtime);
	if (datasetid.a(2) ne SYSTEM.a(23) or datasetid.a(3) ne SYSTEM.a(17)) {
		//if (not SYSTEM.a(61) and interactive and USERNAME == "NEOSYS") {
		if (not SYSTEM.a(61)) {
			if (datasetid.a(1) ne "1EEC633B") {
				var question = 
					"This database has been copied or|"
					"the database name or code has been changed.|"
					"Is this going to be a unique new master database?";
				var options =
					"Yes - Going to be a new independent database" _VM_
					"No - just backing up, renaming or recoding the database";
				call decide(question, options, reply, 2);
				if (reply == 1) {
					goto newdatasetid;
				}
			}
			goto adddatasetcodename;
		}
	}
	SYSTEM.r(45, datasetid.a(1));

	call log2("*clean up document keys", logtime);
	if (gen.documents.open("DOCUMENTS")) {
		gen.documents.select();

		var docid;
		while (gen.documents.readnext(docid)) {
			var docid2 = field2(docid, "\\", -1).field(".", 1);
			if (docid2 ne docid) {
				var doc;
				if (doc.read(gen.documents, docid)) {
					doc.write(gen.documents, docid2);
					gen.documents.deleterecord(docid);
				}
			}
		}
	}

	call log2("*save upgrade history and email notification", logtime);
	if (version.osread("GENERAL\\VERSION.DAT")) {
		version = version.field("\r", 1).field(0x1A, 1).trim();
		//idate=iconv(field(version,' ',2,3),'D')
		//itime=iconv(field(version,' ',1),'MT')
		call osread(upgradelog, "UPGRADE.CFG");
		//if field2(upgradelog,crlf,-1) ne version then
		if (field2(upgradelog, var().chr(10), -1) ne version) {
			if (upgradelog) {
				upgradelog ^= "\r\n";
			}
			upgradelog ^= version;
			call oswrite(upgradelog, "UPGRADE.CFG");

			//should only email users on live database (ie not copies that are not backedup)
			if (not SYSTEM.a(61) and not var("NEOSYS.ID").osfile()) {
				if (decide("Email users about upgrade/to clear cache?|(F5 EMAILUSERS UPGRADE)|Mandatory if there are significant changes in web UI", xx) == 1) {
					perform("EMAILUSERS UPGRADE " ^ version);
				} else {
					call sysmsg("NEOSYS Software Upgrade " ^ version);
				}
			}

		}
	}

	if (interactive and USERNAME == "NEOSYS") {
		perform("FINDDEADALL");
	}

/*TODO
	call log2("*installing authorised keys", logtime);
	perform("INSTALLAUTHKEYS (S)");

	call log2("*installing authorised hosts", logtime);
	perform("INSTALLALLOWHOSTS (S)");
*/
	call log2("*create user name index", logtime);
	var convkey = "CONVERTED*USERNAMEINDEX";
	if (not(xx.read(gen._definitions, convkey))) {
		call usersubs("CREATEUSERNAMEINDEX");
		var().date().write(gen._definitions, convkey);
	}

	call log2("*stop init.general", logtime);
	call unlockrecord("DEFINITIONS", definitions, "INIT.GENERAL.LOGIN");

	call initgeneral2("FIXURLS", logtime);
	call initgeneral2("UPDATECONNECTIONS", logtime);

	call log2("*indicate success to LOGIN", logtime);
	if (SYSTEM.a(33, 10)) {
		call oswrite("OK", SYSTEM.a(33, 10) ^ ".$2");
	}

	//indicate successfull initialisation
	call log2("-----initgeneral exit", logtime);
	return true;

}

subroutine getsystem(in filename, in filen) {
	call log2("*get " ^ filename ^ " parameters", logtime);
	if (not systemx.osread(filename)) {
		return;
	}

	//ensure default style is null
	var tt3 = systemx.a(46);
	tt3.converter(VM, "");
	tt3.swapper("Default", "");
	if (tt3 == "") {
		systemx.r(46, "");
	}

	//parameters in the neosys\system file override params from definitions
	//ie global installation parameters override dataset parameters
	var ni = systemx.count(FM) + 1;
	for (var ii = 1; ii <= ni; ++ii) {
		if ((systemx.a(ii)).length()) {
			SYSTEM.r(ii, systemx.a(ii));
		}
	};//ii;

	//save config file time so can detect if restart required
	SYSTEM.r(100, filen, filename.osfile().a(3));

	return;

}

subroutine backupreminder(in currdataset) {

	//get last backup details
	var paramrec;
	if (not(paramrec.osread("..\\data\\" ^ currdataset ^ "\\params2"))) {
		return;
	}
	var lastbackupdate = paramrec.a(2);

	//skip if no backup or backup in last one day (to midnight)
	//if lastbackupdate and lastbackupdate lt date()-1 then
	//assume backup on same day (ie after last midnight)
	if (not lastbackupdate or lastbackupdate ge date())
		return;

	var msg = "The last backup was ";
	var ndays = var().date() - lastbackupdate;
	msg ^= ndays ^ " day" ^ var("s").substr(1, ndays ne 1) ^ " ago.";
	msg ^= "   (" ^ lastbackupdate.oconv("D") ^ ")";
	msg.swapper("(0", "(");
	msg.r(-1, "NEOSYS recommends that you \"BACKUP\" your data ");
	msg.r(-1, "regularly to prevent total loss of data due to");
	msg.r(-1, "power failure, disk damage or other accidents.");

	if (msg) {
		call mssg(msg);
	};
	return;
}

subroutine fail() {

	msg = USER4;

	call log2("*put up brief warning for three seconds and close - " ^ msg, logtime);
	var s33 = SYSTEM.a(33);
	SYSTEM.r(33, "");
	call mssg(msg ^ "", "T3");
	SYSTEM.r(33, s33);

	msg.swapper(FM ^ FM, FM);
	msg.converter(FM, "|");
	call oswrite(msg, SYSTEM.a(33, 10) ^ ".$2");
	//print char(12):char(7):@user4

	return;
}


libraryexit()
