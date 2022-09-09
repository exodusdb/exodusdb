#include <exodus/library.h>
libraryinit()

#include <log2.h>
#include <initgeneral2.h>
#include <inputbox.h>
#include <shadowmfs.h>
#include <systemfile.h>
#include <otherusers.h>
#include <getauthorisation.h>
#include <cid.h>
#include <getbackpars.h>
#include <shell2.h>
#include <diskfreespace.h>
#include <authorised.h>
#include <openfile.h>
#include <logprocess.h>
#include <netlogin.h>
#include <sysmsg.h>
#include <systemsubs.h>
#include <initcompany.h>

#include <sys_common.h>

var lastlog;
var logtime;
var msg;
var pidfilename;
var pidrec;
var dbdate;
var dbtime;
var dbdatetimerequired;
var reply;
var reply2;
var tt;//num
var colors;
var reportkey;
var tt2;//num
var s33;
var smtp;
var bakpars;
var vn;
var datacfg;
var codepage;
var xx;
var os;
var ver;
var vern;
var cpu;
var errors;
var lists;
var companycode;
var dostime;
var docid;
var codepaging;
var version;
var upgradelog;
var systemx;
var yy;

function main() {
	//
	//c sys

	//NB @VOLUMES blank on C++ and used to run slightly differently on C++
	//eg function called is generalalerts instead of general.alerts

	#include <system_common.h>
	//global tt,tt2,s33,lastlog

	//!WARNING decide() returns REPLY number instead of VALUE when not interactive

	//ensure cursor on the left
	//print @(0):

	call log2("GETLASTLOG", lastlog);

	call log2("*Starting INIT.GENERAL --- STARTUP/RESTART ---", logtime);

	//unused system variables like @user0 not @recur0
	//http://www.sprezzatura.com/library/revmedia/attack.php
	//@sw<10>='XYZ'
	//@vw<10>='ABC'

	//!!! do not use getenv before initialised - or shell etc which uses it
	//gosub getenv
	call initgeneral2("GETENV", logtime);

	var resetting = PSEUDO eq "RESET";
	var initmode = SENTENCE.field(" ", 2);
	PSEUDO = "";
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
	//equ sessionid to system<4>;*SESSION WAS system<32>
	var sessionid = SYSTEM.f(4);
	//equ printptr to system<5>;*used in BP and GBP/MSG3. could be moved
	//equ interruptfilename to system<6>
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
	//equ sleepsecs to system<22>
	//equ currdatasetname to system<23>;*SESSION
	//equ process number/connection number to system<24>;*SESSION

	//processing time should not be set until any init convertion is over
	//otherwise esc.to.exit/giveway might abort processing
	//equ processingstarttime to system<25>
	//equ processingslepttime to system<26>
	SYSTEM(25) = "";

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
	var interactive = false; //not(SYSTEM.f(33));
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

	//equ installationdesc to system<57>;*should be like hostname eg unilever
	//will be used for system email sender and ddns like
	//eg domain name eg unilever -> zoneedit UNILEVER.HOSTS.EXODUS.COM
	//dns is invaluable for server on dynamic ip number but useful for any server
	//because will create automatically and adapt if and when ip changes
	//may need to specify TTL of 60 seconds on zoneedit if created automatically
	//because auto created domains may get default TTL eg 600 seconds/10 mins

	//equ databasecodes to system<58> *multivalued
	//equ numprocessesreq to system<59> *mv with 58
	//equ backupreq to system<60> *mv with 58
	//LOGON.OLD sets to 1 if "TEST", INIT.GENERAL sets to 1 if not backed up
	//system<61> to testdata or notlivedata
	//system<62> to copyreq mv with 58
	//system<63> to testdatabasecode mv with 58
	//free 63-70
	//system<71> to system<99> used for backup.cfg details
	//bakpars from DOS BACKUP.CFG and readbakpars. created from SYSTEM.CFG
	//system<73>=bakpars<3> backup time from
	//system<74>=bakpars<4> backup time upto
	//system<76>=bckpars<6> alert email addresses
	//system<77>=bakpars<7> backup drive
	//system<82>=bakpars<12> backup drive for uploads/images
	//system<99>
	//system<100> mv times of SYSTEM.CFG,../../SYSTEM.CFG, ~/dat/ etc detect restart
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
	//system<121> to syncdata required (every hour to ../data2/dataset.1/2/3
	//system<122> to emailblockingusers
	//system<123> to installationgroup defaults to global (for ../../global.end)
	//system<124> to checkforupgrades 0 1 default to yes
	//system<125> to closeafterbackups 0 1 default to yes
	//system<126> to testcanstartprocesses 0 1 default to no
	//system<127> to charset eg 1252. greek is windows-1253, arabic is windows-1256
	//mode con cp -> Code page:
	//720 -> arabic
	//737 -> greek
	//437 -> standard dos latin us
	//system<128> to max nologin days
	//system<129> to initwaitsecs default to 55
	//see also systemconfiguration_dict.js
	//system<130> to allpunctuation
	//system<131> to monitor https port - default 4428
	//system<132> to lasterror returnstack
	//system<133> to modules eg. GENERAL vm FINANCE vm ADAGENCY

	//system<134> financial base.currency
	//system<135> financial curr.period
	//system<136> financial curr.year
	//system<137> agency default market code
	//system<138> agency last day of week mon-sun 1-7
	//system<139> domain name prefix eg d- t-
	//system<140> cid eg 79350001

	call log2("*init.general obtaining exclusive access:", logtime);
	var voc;
	if (voc.open("VOC", "")) {
		//timeout and crash out quickly otherwise might get unlimited processes
		//this lock is in INIT.GENERAL and MONITOR2 (to avoid starting processes)
		var initwaitsecs = SYSTEM.f(129);
		if (not initwaitsecs) {
			//initwaitsecs = 110 + var(10).rnd();
			//increase to 10 minutes to allow things like reindexing and trimrequestlog to finish
			//although nightly backup should only restarting _test databases
			initwaitsecs = 600 + var(10).rnd();
		}
		//also in LOGON.OLD and INIT.GENERAL
		var lockkey = "INIT.GENERAL.LOGIN";
		if (not(voc.lock( lockkey))) {
			//np if own lock
			if (FILEERROR ne "414") {
				if (not(lockrecord("VOC", voc, "INIT.GENERAL.LOGIN", "", initwaitsecs))) {
					msg = "INIT.GENERAL couldnt get exclusive lock. Quitting.";
					//maybe a long upgrade process is running
					call note(msg);
					if (SYSTEM.f(33)) {
						gosub failsys();
					}
					perform("OFF");
					logoff();
				}
			}
		}
	}

	var exodusid = var("exodus.id").osfile();

	call log2("*determine the pid if possible", logtime);
	if (not(SYSTEM.f(54))) {
		SYSTEM(54) = SYSTEM.f(33);
	}
//	if (VOLUMES) {
//		pidfilename = SYSTEM.f(54, 5) ^ ".pid";
//	} else {
		pidfilename = "/run/neo/neo@" ^ SYSTEM.f(17);
		if (SYSTEM.f(17).substr(-5) eq "_test") {
			pidfilename.swapper("neo@", "tst@");
			pidfilename.swapper("_test", "");
			pidfilename ^= ".pid";
		}
//	}
	call osread(pidrec, pidfilename);
	//osremove pidfilename
	if (pidrec) {
		SYSTEM(54, 5) = pidrec.f(1);
	}

	//check version of database versus version of program
	//prevent login (except as maintenance with confirmation)
	//NB this date should be revised whenever any conversion or upgrade is done
	if (not exodusid) {
		var oldmethod = 1;
		if (oldmethod) {
			dbdate = "30 MAR 2007";
			dbtime = "12:00:00";
			dbdatetimerequired = dbdate.iconv("D") + dbtime.iconv("MT") / 86400;
		} else {
		}
		if (DEFINITIONS.open("DEFINITIONS", "")) {
			var dbversion;
			if (not(dbversion.read(DEFINITIONS, "DBVERSION"))) {
				goto updateversion;
			}
			if (oldmethod and dbversion.f(1) eq "14334.5") {
			}
			if (dbversion.f(1) gt dbdatetimerequired) {
				msg = "Software version " ^ dbversion.f(1).oconv("D") ^ " " ^ dbversion.f(1).field(".", 2).oconv("MTS") ^ " is incompatible with" ^ FM ^ "Database version " ^ dbdate ^ " " ^ dbtime;
				msg = msg.oconv("L#60");
				//abort since db is advanced
				if (not(interactive)) {
badversion:
					USER4 = msg;
					gosub failsys();
					stop();

				} else {
decideversion:
					var options = "Quit (RECOMMENDED)";
					options(-1) = "Continue";
					options(-1) = "Mark database as version " ^ dbdate ^ " " ^ dbtime ^ " and continue";
					call decide("!" ^ msg ^ "", options, reply);
					if (reply eq 1) {
						goto badversion;
					}
					var msgx = "!WARNING *UNPREDICTABLE* CONSEQUENCES" ^ FM ^ FM ^ "WHAT IS THE PASSWORD?";
					call inputbox(msgx, 20, 0, "", reply2, "\x1B");
					if (reply2 ne "UNPREDICTABLE") {
						call note("THAT IS NOT THE CORRECT PASSWORD");
						goto decideversion;
					}
					if (reply eq 3) {
						goto updateversion;
					}
				}
			} else if (dbversion.f(1) lt dbdatetimerequired) {
updateversion:
				dbversion = dbdatetimerequired;
				dbversion(2) = dbdate;
				dbversion(3) = dbtime;
				dbversion.write(DEFINITIONS, "DBVERSION");
			}

		}
	}

	//install any dat files in the current database
	//also load any new or amended pgsql functions found in dict files in dat
	call log2("*install any new db dat data", logtime);
	osshell("sync_dat");

	call log2("*copy any new data records", logtime);
	var bp;
	if (bp.open("GBP", "")) {
		for (const var ii : range(1, 9999)) {
			var rec;
			if (not(rec.read(bp, "$DATA." ^ ii))) {
				rec = "";
			}
			///BREAK;
			if (rec eq "") break;
			var filename = rec.f(1);
			var id = rec.f(2);
			rec = rec.field(FM, 3, 99999);
			var file;
			if (file.open(filename, "")) {
				rec.write(file, id);
			}
		} //ii;
	}

	//syslock=if revrelease()>=2.1 then 36 else 23

	//place lock indicating processing (initialising actually)
	//if interactive else call rtp57(syslock, '', '', trim(@station):system<24>, '', '', '')

	//EMS MEMORY notes
	//explanation of how to use power-on setup or emsmagic
	//to enable EMS as good as possible
	//NB EXODUS memory is tested in LOGON.OLD
	//http://www.columbia.edu/~em36/wpdos/emsxp.html

	//startup options
	//http://www.revelation.com/knowledge.nsf/461ff2bd5a8ddeed852566f50065057d/3c57c588ce21daf9852563920051f70b?OpenDocument
	//AREV adagency,exodus /X /M4096
	///X tells the system to use EMS memory for variable storage.
	///M tells the system to use EMS memory for running and executing programs.
	///M can also be followed by an option memory allocation amount in kilobytes.
	//You should never allocate more than 4096K of EMS memory otherwise unstable
	///O tells the system to not use any EMS memory at all.
	///E tells the system to not use the on-board math chip and use a math chip emulator instead.
	///, prompts for a user name

	call log2("*force replication system to reinitialise", logtime);
	call shadowmfs("EXODUSINIT", "");

//	call log2("*reattach data", logtime);
//	var volumesx = VOLUMES;
//	var nvolumes = VOLUMES.count(FM) + 1;
//	for (const var volumen : range(1, nvolumes)) {
//		var volume = volumesx.f(volumen);
//		var tpath = "../" "DATA" "/";
//		tpath.converter("/", OSSLASH);
//		if (volume.substr(1, 8) eq tpath) {
//			execute("ATTACH " ^ volume ^ " (S)");
//		}
//	} //volumen;
//
//	if (VOLUMES) {
//		call log2("*perform RUN GBP LOGON.OLD UPGRADEVOC", logtime);
//		perform("RUN GBP LOGON.OLD UPGRADEVOC");
//	}

	//call log2('*set workstation time to server time',logtime)
	//call settime('')

	call log2("*save the original username and station", logtime);
	if (not(SYSTEM.f(43))) {
		SYSTEM(43) = USERNAME;
	}
	if (not(SYSTEM.f(44))) {
		SYSTEM(44) = STATION.trim();
	}

	call log2("*detach merges", logtime);
	//locate 'MERGES' in @files using fm setting xx then perform 'DETACH MERGES (S)'
	//open 'MERGES' to xx then perform 'DETACH MERGES (S)'
	if (tt.open("MERGES", "")) {
		perform("DELETEFILE MERGES (S)");
	}

	call log2("*get user video table if any", logtime);
	var temp = ENVIRONKEYS.f(2);
	temp.converter(".", "");
	if (colors.osread(temp ^ ".vid")) {
		var color2;
		if (not(color2.read(systemfile(), ENVIRONKEYS ^ ".VIDEO"))) {
			color2 = "";
		}
		if (colors ne color2) {
			colors.write(systemfile(), ENVIRONKEYS ^ ".VIDEO");
			//call colortoescold);
		}
	}

	call log2("*setup escape sequence for standard color and background", logtime);
	temp = HW.f(3)[4] ^ HW.f(8)[4];
	if (temp eq "00") {
		temp = "70";
	}
	tt = "\x1B";
	tt ^= "C";
	tt ^= temp;
	AW(30) = tt;

	call log2("*convert reports file", logtime);
	if (SYSTEM.f(17) ne "DEVDTEST") {
		var reports;
		if (reports.open("REPORTS", "")) {
			select(reports);
nextreport:
			if (readnext(reportkey)) {
				var recordx;
				if (recordx.read(reports, reportkey)) {
					var filename = reportkey.field("*", 1);
					var file;
					if (file.open(filename, "")) {
						var keyx = reportkey.field("*", 2);
						keyx.swapper("%2A", "*");
						if (recordx.f(1) eq "%DELETED%") {
							var rec;
							if (rec.read(file, keyx)) {
								//if rec<1>='EXODUS' then delete file,keyx
								//if rec<8>='EXODUS' then delete file,keyx
								file.deleterecord(keyx);

							}
						} else {
							var oldrecord;
							if (not(oldrecord.read(file, keyx))) {
								oldrecord = "";
							}
							//only update documents if changed anything except update timedate
							if (filename eq "DOCUMENTS") {
								oldrecord(8) = recordx.f(8);
							}
							if (recordx ne oldrecord) {
								recordx.write(file, keyx);
							}
						}
					}
				}
				goto nextreport;
			}
		}
	}

	if (not(DEFINITIONS.open("DEFINITIONS", ""))) {
		var().chr(7).output();
		msg = "The DEFINITIONS file is missing";
		msg(-1) = "Did you startup using the right command file/datasettype?";
		call note(msg);
	}

	var notherusers = otherusers("").f(1);

	call log2("*check/get authorisation", logtime);
	if (SYSTEM.f(4) eq "" and not(SYSTEM.f(33))) {
		var nusers = getauthorisation();
		if (not nusers) {

			call log2("*if sysmode then respond to response file and close", logtime);
			if (SYSTEM.f(33)) {
				gosub failsys();
				stop();
			}

			call log2("*inform user and close", logtime);
			msg = var("594F552043414E4E4F5420555345204E454F53595320534F465457415245204F4E205448495320434F4D50555445527C554E54494C20594F5520484156452054484520415554484F5249534154494F4E204E554D4245522E").iconv("HEX2");

			//var().chr(7).output().str(3);
			call note(msg);
			perform("OFF");
			logoff();

		}

		call log2("*check users", logtime);
		//if nusers lt otherusers('')+1 then
		if (nusers lt notherusers + 1) {
			msg = var("4D4158494D554D20415554484F5249534544204E554D424552204F46205553455253204558434545444544").iconv("HEX2");
			call note(msg);
			if (SYSTEM.f(33)) {
				gosub failsys();
			}
			perform("OFF");
			logoff();
		}

	}

	call log2("*prevent use of this program via F10", logtime);
	if (((initmode ne "LOGIN" and LEVEL ne 1) and interactive) and not(resetting)) {
		msg = "You cannot quit from within another program via F10.";
		msg(-1) = "Please quit all programs first and then try again.";
		var().chr(7).output();
		call note(msg);
		stop();
	}

	call log2("*check the operating system date", logtime);
	//CHECKSYSDATE:
	var config = "";
	if (not(config.osread("EXODUS.CFG"))) {
		var lastdate = "/lastdate.rev/";
		lastdate.converter("/", OSSLASH);
		if (config.osread(lastdate)) {
			{}
		}
	}
	var lastdate = config.f(1);
	/*;
		IF LASTDATE and interactive THEN;
			IF DATE() LT LASTDATE OR DATE() GT (LASTDATE+14) THEN;
				MSG='';
				MSG<-1>="Is today's date ":OCONV(DATE(),'D4'):' ?';
				MSG<-1>=' ';
				MSG<-1>='(According to internal records, the';
				MSG<-1>='computer was last used on the ':LASTDATE 'D4':",|but the computer's date is now ":DATE() 'D4':')';
				MSG<-1>=' ';
				MSG<-1>=' ';
				if DECIDE(MSG,'',REPLY) ELSE EXECUTE 'OFF';logoff;
				//if not today's date then use DOS to set it
				IF REPLY NE 1 THEN;
					DATA '';
					PERFORM 'PC DATE';
					GOTO CHECKSYSDATE;
					END;
				END;
			END;
	*/
	call log2("*update \"last used date\"", logtime);
	if (var().date() ne lastdate) {
		config(1) = var().date();
		//call OSWRITE(CONFIG,'exodus.cfg')
		var(config).oswrite("exodus.cfg");
	}

	call log2("*check for invalid characters in workstation name", logtime);
	if (STATION.index("'") or STATION.index(DQ)) {
		msg = "WARNING: EXODUS WILL NOT WORK PROPERLY BECAUSE";
		msg ^= FM ^ "YOUR WORKSTATION NAME (" ^ STATION.trim() ^ ")";
		msg ^= FM ^ "CONTAINS QUOTATION MARKS. PLEASE ASK YOUR ";
		msg ^= FM ^ "TECHNICIAN TO CHANGE THE WORKSTATION NAME.";
		var().chr(7).output();
		call note("!" ^ msg ^ "|");
	}

	call log2("*convert SYSTEM to SYSTEM.CFG", logtime);
	//leave old SYSTEM around in case old version of software reinstalled
	//can be deleted manually
	if (tt.osread("system")) {
		if (not(var("system.cfg").osfile())) {
			var(tt).oswrite("system.cfg");
		}
	}

	//save current system so we can restore various runtime parameters
	//which are unfortunately stored with configuration params

	var oldsystem = SYSTEM;

	call log2("*get DEFINITIONS SYSTEM parameters", logtime);
	//do in reverse order so that the higher levels get priority
	if (not(SYSTEM.read(DEFINITIONS, "SYSTEM"))) {
		SYSTEM = "";
	}
	tt = "system.cfg";
	tt2 = 2;
	gosub getsystem();
	var tpath = "../../";
	tpath.converter("/", OSSLASH);
	tt = tpath ^ "system.cfg";
	tt2 = 1;
	gosub getsystem();

	////c++ only
	//SYSTEM(58) = oslistd("../data/").convert(FM,VM);
	//SYSTEM.fieldstorer(FM, 59, 5, "");
    //update dbcodes with dir dbcodes if dbcodes in sys cfg file contain lcase
    var ttdbcodes = SYSTEM.f(58).convert("abcdefghijklmnopqrstuvwx","").length();
    if(ttdbcodes eq SYSTEM.f(58).length()){
        //c++ only
        SYSTEM(58) = oslistd("../data/").convert(FM,VM);
        SYSTEM.fieldstorer(FM, 59, 5, "");
	}

	call log2("*restore session parameters", logtime);
	//restore IMMEDIATELY to avoid bugs creeping in
	SYSTEM(33) = oldsystem.f(33);
	SYSTEM(1) = oldsystem.f(1);
	//masterlogin
	SYSTEM(15) = oldsystem.f(15);
	//cleaned
	SYSTEM(16) = oldsystem.f(16);
	SYSTEM(17) = oldsystem.f(17);
	//sessionid
	SYSTEM(4) = oldsystem.f(4);
	//server mode (not interactive)
	s33 = SYSTEM.f(33);
	//currdatasetname
    SYSTEM(23) = oldsystem.f(23);
    var tt = osread("../data/" ^ oldsystem.f(17) ^ "/name");
    if (tt){
        SYSTEM(23) = tt;
    }
	//process/connection number
	SYSTEM(24) = oldsystem.f(24);
	//datasetno
	SYSTEM(38) = oldsystem.f(38);
	//originalusername
	SYSTEM(43) = oldsystem.f(43);
	//original station id
	SYSTEM(44) = oldsystem.f(44);
	//globaldatasetid
	SYSTEM(45) = oldsystem.f(45);
	//processlock
	SYSTEM(48) = oldsystem.f(48);
	SYSTEM(54) = oldsystem.f(54);
	SYSTEM(133) = "GENERAL";

	oldsystem = "";

	//! system configuration defaults
	// installation group
	if (not(SYSTEM.f(123))) {
		SYSTEM(123) = "GLOBAL";
	}
	// upgrades yes
	if (SYSTEM.f(124) eq "") {
		SYSTEM(124) = 1;
	}
	//close after backup yes
	if (not(SYSTEM.f(125))) {
		SYSTEM(125) = 2;
	}
	//test should start missing processes
	if (not(SYSTEM.f(126))) {
		SYSTEM(126) = 0;
	}

	if (not(SYSTEM.f(57))) {
		call log2("*determine systemid from old smtp sender name", logtime);
		tpath = "../../";
		tpath.converter("/", OSSLASH);
		call osread(smtp, tpath ^ "smtp.cfg");
		if (not(smtp.f(1))) {
			call osread(smtp, "smtp.cfg");
		}
		if (not(smtp.f(1))) {
			if (not(smtp.read(DEFINITIONS, "SMTP.CFG"))) {
				{}
			}
		}
		if (smtp.f(1)) {
			var sysname = smtp.f(1).field("@", 1).lcase();
			//remove all punctuation
			sysname.converter("!\"#$%^&*()_+-=[]{};:@,./<>?", "");
			SYSTEM(57) = sysname;
			//call osread(tt,'system.cfg')
			if (not(tt.osread("system.cfg"))) {
				tt = "";
			}
			tt(57) = sysname;
			//call oswrite(tt,'system.cfg')
			var(tt).oswrite("system.cfg");
		}
	}

	//call log2('*save database system parameters',logtime)
	//dont do this anymore since copying system params from place
	//to place is probably wrong
	//read tt from definitions,'SYSTEM' else
	// write system on definitions,'SYSTEM'
	// end

	call log2("*force reevaluation of cid", logtime);
	SYSTEM(111) = "";

	SYSTEM(111) = cid();
	//try twice because was unreliable and is important
	if (not(SYSTEM.f(111))) {
		SYSTEM(111) = cid();
	}

	SYSTEM(51) = APPLICATION;

	//<61>=testdb (not livedb)
	call getbackpars(bakpars);
	SYSTEM(61) = bakpars.f(11);

	//call log2('*determine time offset')
	//blank means dont offset - use system server timezone whatever it is
	//NB system<118>should be configured PER DATABASE not per INSTALLATION
	//otherwise moving a database will get confused if the new server is not gmt
	//@sw<1> is the ADJUSTMENT to get display time from server time
	//@sw<2> is the difference from gmt/utc to server time
	//@sw<3> could be the adjustment to get dbtz from servertz
	SW = "";
	//now ONLY supporting display time different from server time on gmt/utc servers
	//if sysgmtoffset
	if (SYSTEM.f(120)) {
		//if server not on gmt/utc and user tz is set then warning
		//since a) usertime will be server time b) database will get non-gmt date/times
		//if system<118> then
		// call note('WARNING: User time zone ignored and|Database storing non-GMT/UTC date/time|because current server is not GMT/UTC (is ':system<120>:')')
		// end
	} else {
		SW(1) = SYSTEM.f(118);
		//if display time is not server/gmt/utc then adjust offset to server/gmt/utc
		if (SW.f(1).length()) {
			//system time offset is currently automatically determined by CID()
			//CID will tell you the current server tz
			//but should this be recorded in the dataset in case it is moved
			//between servers with different default timezones
			//this issue can be avoided if all servers are kept on gmt/utc+0 timezone

			//NB currently the only exodus databases with tz in system<118> are on nl1/nl1b
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
	tt = SYSTEM.f(49);
	if (tt eq "") {
		//system<49>='..\images\'
		var imagesdir = "../images/";
		imagesdir.converter("/", OSSLASH);
		SYSTEM(49) = imagesdir;
	} else if (tt[-1] ne OSSLASH) {
		//system<49>=tt:'\'
		SYSTEM(49) = tt ^ OSSLASH;
	}

	if (not(SYSTEM.f(46, 1))) {
		SYSTEM(46, 1) = "#FFFF80";
	}
	if (not(SYSTEM.f(46, 2))) {
		SYSTEM(46, 2) = "#FFFFC0";
	}
	//if system<46,3> else system<46,3>=''

	call log2("*backup the system default style", logtime);
	SYSTEM(47) = SYSTEM.f(46);

	//why is this called again?
	//call initgeneral2("GETENV", logtime);

	call log2("*get codepage as an environment variable", logtime);
	//will not override entries in SYSTEM.CFG
	if (not(SYSTEM.f(12).locate("CODEPAGE", vn))) {

//		if (VOLUMES) {
//			tt = "../data/" ^ SYSTEM.f(17) ^ "/data.cfg";
//			tt.swapper("/", OSSLASH);
//			if (not(datacfg.osread(tt))) {
//				tt = "../data/data.cfg";
//				tt.swapper("/", OSSLASH);
//				if (not(datacfg.osread(tt))) {
//					datacfg = "";
//				}
//			}
//			if (datacfg) {
//				datacfg.converter("\r\n", _FM _FM);
//				codepage = datacfg.f(1).trim();
//			} else {
//				var output = shell2("mode CON:", xx);
//				output = trim(output.convert("\r\n", FM ^ FM), FM);
//				codepage = field2(output, " ", -1);
//			}
//		} else {
			//en_US.UTF-8
			call osgetenv("LANG", codepage);
//		}

		SYSTEM(12, vn) = "CODEPAGE";
		SYSTEM(13, vn) = codepage;

		//used in readcss for output reports and documents
		//<meta http-equiv="content-type" content="text/html;charset=..." />
		if (SYSTEM.f(127) eq "") {
			//tt=''
			//if codepage=720 then tt='windows-1256'
			//if codepage=737 then tt='windows-1253'
			//if codepage='UTF-8' then tt='UTF-8'
			if (codepage.index("UTF-8")) {
				codepage = "UTF-8";
			}
			if (var("UTF-8,720,737").locateusing(",", codepage, tt)) {
				tt = var("UTF-8,windows-1256,windows-1253").field(",", tt);
			} else {
				tt = "windows-1252";
			}
			SYSTEM(127) = tt;
		}

	}

	//allpunctuation
	SYSTEM(130) = " `~@#$%^&*()_+-=[]{};\\:\"|,./<>?\\|" "'";

	SYSTEM(9) = 1;

	call log2("*first uses of getenv", logtime);
//	if (VOLUMES) {
//		if (not(osgetenv("OS", os))) {
//			os = "";
//		}
//		if (not(osgetenv("VER", ver))) {
//			ver = "";
//		}
//		if (not(osgetenv("NUMBER_OF_PROCESSORS", tt))) {
//			tt = 1;
//		}
//		SYSTEM(9) = tt;
//
//		call log2("*get windows version", logtime);
//		ver = shell2("ver", xx);
//		ver.converter(FM ^ VM, "  ");
//		ver.converter("\r\n", "  ");
//		//if index(ver,'Windows 9',1) or index(ver,'NT',1) or index(os,'NT',1) then
//		ver = ver.trim().ucase();
//		if ((ver.index("WINDOWS") or ver.index("NT")) or os.index("NT")) {
//			SYSTEM(12, -1) = "WORDSIZE";
//			SYSTEM(13, -1) = "32";
//		}
//		ver = ver.field(" ", 4).field("]", 1);
//		//Windows Server 2003         5.2.3790 (24.04.2003)
//		//Windows Server 2003, SP1    5.2.3790.1180
//		//Windows Server 2003         5.2.3790.1218
//		//Windows Server 2008         6.0.6001 (27.02.2008)
//		var ver2 = ver.field(".", 1, 2);
//		if (var("5.0,5.1,5.2,6.0,6.1,6.2,6.3").locateusing(",", ver2, vern)) {
//			ver2 = var("2000,XP,2003,2008,2008R2,2012,2012R2").field(",", vern);
//			ver = ver.fieldstore(".", 1, -2, "Win" ^ ver2);
//			ver.swapper(".6001", "");
//			ver.swapper(".6002", " SP2");
//			ver.swapper(".3790", "");
//		}
//
//	//not Windows
//	} else {
		//Description:\tUbuntu 18.04.3 LTS\n
		ver = shell2("lsb_release -d").field(var().chr(9), 2).trim();
		ver.popper();

		//CPU(s):              2
		//Thread(s) per core:  1
		//Core(s) per socket:  2
		//var nthreads_percpu = (((shell2("lscpu|grep \"Thread(s) per core\"")).field(":", 2)).field(var().chr(10), 1)).trim();
		//var ncpus = (((shell2("lscpu|grep \"CPU(s)\"")).field(":", 2)).field(var().chr(10), 1)).trim();
		//if (nthreads_percpu.isnum() and ncpus.isnum()) {
		//	SYSTEM(9) = nthreads_percpu * ncpus;
		var ncpus = shell2("nproc").field("\n").trim();
		if (not ncpus or not ncpus.isnum())
			ncpus = 1;
		SYSTEM(9) = ncpus;
//	}

	//save VER
	if (not(SYSTEM.f(12).locate("VER", tt))) {
		{}
	}
	SYSTEM(12, tt) = "VER";
	SYSTEM(13, tt) = ver.trim();

	call log2("*get cpu version", logtime);
	if (var("wmic.cfg").osfile()) {
		//grep "model name" /proc/cpuinfo
		cpu = "Unknown";
	} else {
//		if (VOLUMES) {
//			cpu = shell2("wmic CPU GET NAME", xx);
//			if (cpu.substr(1, 2) eq _RM _FM) {
//				cpu.splicer(1, 2, "");
//				cpu.converter(var().chr(0), "");
//			}
//			cpu.swapper("\r\n", FM);
//			cpu.remover(1);
//		} else {
			cpu = ((shell2("cat /proc/cpuinfo|grep -i \"model name\"")).field(var().chr(9), 2)).trim();
			cpu.converter(var().chr(10), FM);
			cpu.converter(":", "");
			cpu.trimmer();
//		}
		if (cpu[-1] eq FM) {
			cpu.popper();
		}

		//Name.
		//Intel(R) Xeon(R) CPU           E5645  @ 2.40GHz

		//Name
		//Intel(R) Xeon(TM) CPU 3.40GHz
		//Intel(R) Xeon(TM) CPU 3.40GHz
		cpu.swapper("(R)", "");
		cpu.swapper("(TM)", "");
		cpu.swapper("Intel", "");
		cpu.swapper("CPU", "");
		cpu.converter("-", " ");
		var nsockets = cpu.count(FM) + 1;
		var nlogical = SYSTEM.f(9);
		cpu = cpu.f(1).trim() ^ " ";
		tt = nlogical / nsockets;
		if (tt gt 1) {
			cpu ^= nsockets ^ "x" ^ tt;
		} else {
			cpu ^= "x" ^ nsockets;
		}
	}

	SYSTEM(12, -1) = "CPU";
	SYSTEM(13, -1) = cpu;

	var currdataset = SYSTEM.f(17);

	/*;
		if @volumes then;
			call log2('*warning if too little or too much EMS memory',logtime);
			ems.allocated=get.ems('');
			if interactive and not(index(os,'NT',1)) and not(index(ver,'Millennium',1)) then;
				//if ems.allocated>1024000 or ems.allocated<512000 then
				if ems.allocated>1100000 or ems.allocated<512000 then;
					msg='WARNING: EXODUS may not work reliably because it|has ';
					if ems.allocated>1024000 then;
						msg:='been given more than';
					end else;
						msg:='not been given';
						end;
					msg:=' 1024Kb of "EMS" memory|';
					msg:='|1. Close EXODUS' 'L#35';
					msg:='|2. Right click the EXODUS icon' 'L#35';
					msg:='|3. Choose "Properties"' 'L#35';
					msg:='|4. click "Memory"' 'L#35';
					msg:='|5. Set "EMS" memory to 1024' 'L#35';
					msg:='|6. Click "OK" and restart EXODUS' 'L#35';
					print char(7):
					call note('!':msg:'|');
					end;
				end;
			end;
	*/

	call log2("*get diskfreespace", logtime);
	var reqfreemb = 10;
	var freemb = (diskfreespace(oscwd()) / 1024 / 1024).oconv("MD00");

	call log2("*check if diskfreespace is sufficient " ^ freemb ^ "Mb", logtime);
	//notherusers=otherusers('')+1
	if (freemb lt reqfreemb * notherusers) {
	//if 1 then
		msg = "THERE IS NOT ENOUGH FREE DISK SPACE AVAILABLE";
		msg ^= "||EXODUS needs at least " ^ reqfreemb ^ "Mb PER USER|of free space on disk " ^ oscwd().substr(1, 2) ^ " but";
		msg ^= "|there is only " ^ freemb ^ "Mb available.";
		if (notherusers) {
			msg ^= "||There is/are " ^ notherusers ^ " other users online.";
		}
		var().chr(7).output();
		call note(msg ^ "|");
		//stop
		//if freemb then perform 'OFF'
	}

//	if (VOLUMES) {
//		call log2("*try to update upload.dll", logtime);
//		//for extn=1 to 3
//		// ext=field('net,w3c,www',',',extn)
//		var ext = "NET";
//		var path = "../exodus." ^ ext ^ "/exodus/dll/";
//		path.converter("/", OSSLASH);
//		tt = (path ^ "upload.dl_").osfile();
//		if (tt) {
//			if (tt ne (path ^ "upload.dll").osfile()) {
//				call osremove(path ^ "upload.dll");
//				if (not((path ^ "upload.dll").osfile())) {
//					//call shell('ren ':path:'upload.dl_ upload.dll')
//					osshell("copy " ^ path ^ "upload.dl_ " ^ path ^ "upload.dll");
//				}
//			}
//		}
//		// next extn
//	}

//	//Windows Cygwin
//	if (VOLUMES) {
//		call log2("*find cygwin", logtime);
//		var locations = SYSTEM.f(50);
//		locations(1, -1) = "C:\\CYGWIN\\BIN\\,\\CYGWIN\\BIN\\,..\\..\\CYGWIN\\BIN\\";
//		var nn = locations.count(",") + 1;
//		for (const var ii : range(1, nn)) {
//			var location = locations.field(",", ii);
//			if (location[-1] ne "\\") {
//				location ^= "\\";
//			}
//			//initdir location:'*.*'
//			//tt=dirlist()
//			tt = oslistf(location ^ "*.*");
//			if (tt) {
//				SYSTEM(50) = location;
//			}
//			///BREAK;
//			if (not(tt eq "")) break;
//		} //ii;
//	}

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
//	if (VOLUMES) {
//		call log2("*find http proxy", logtime);
//		if (SYSTEM.f(56) eq "") {
//			var cmd = "proxycfg";
//getproxy:
//			var result = shell2(cmd, errors).lcase();
//			tt = result.index("proxy server(s)");
//			if (tt) {
//				//tt=result[tt,';'][1,char(13)]
//				tt = result.substr(tt, 9999);
//				tt = tt.field(";", 1).field(var().chr(13), 1);
//				tt = tt.field(":", 2, 99).trim();
//				tt.swapper("http=", "");
//			} else if (cmd eq "proxycfg") {
//				cmd = "netsh winhttp import proxy ie";
//				goto getproxy;
//				{}
//			} else {
//				tt = "";
//			}
//			{}
//			//todo get proxybypasslist
//			SYSTEM(56) = tt;
//		}
//	}

	/*;
		call log2('*put up the backup reminder',logtime);
		reminder='';
		if interactive and not(system<20>) and not(resetting) then;
			call backupreminder(currdataset,msg);
			if msg then call mssg(msg,'UB':FM:FM:FM:2,reminder,'');
			end;
	*/
	call log2("*default date format is DD/MM/YY", logtime);
	DATEFMT = "D2/E";

	call log2("*get security - also in LISTEN", logtime);
	if (not(SECURITY.read(DEFINITIONS, "SECURITY"))) {
		if (temp.open("DICT.DEFINITIONS")) {
			if (SECURITY.read(temp, "SECURITY")) {
				SECURITY.write(DEFINITIONS, "SECURITY");
			}
		}
	}
//	if (VOLUMES) {
//		SECURITY = SECURITY.invert();
//	}

	//must be before init.acc, init.agency or any task adding
	//call security.subs2('FIXUSERPRIVS')

	//call log2('*create MASTER user if no users',logtime)
	//if userprivs<1> else
	// userprivs<1>='MASTER'
	// write invert(userprivs) on definitions,'SECURITY'
	// end

	//setup a few tasks in advance since failure to find task in client
	//doesnt cause automatic addition into auth file since only disallowed
	//tasks are sent to client for speed
	//Failure to show all possible tasks would mean difficulty to know
	//what tasks are available to be locked
	//in init.xxx files per module
	//if @account='ACCOUNTS' then
	tt = "GS";
	//end else
	// tt='EXODUS'
	// end
	if (not(authorised("DATABASE STOP", msg, tt))) {
		{}
	}
	if (not(authorised("DATABASE RESTART", msg, tt))) {
		{}
	}
	if (not(authorised("DATASET COPY", msg, tt))) {
		{}
	}
	//if security('%RENAME%':'AUTHORISATION UPDATE KEYS',msg,'AUTHORISATION UPDATE GROUPS') else null
	if (not(authorised("AUTHORISATION UPDATE", msg, tt))) {
		{}
	}
	if (not(authorised("AUTHORISATION UPDATE LOCKS", msg, tt))) {
		{}
	}
	if (not(authorised("AUTHORISATION UPDATE LOWER GROUPS", msg, tt))) {
		{}
	}
	if (not(authorised("AUTHORISATION UPDATE HIGHER GROUPS", msg, tt))) {
		{}
	}

	if (tt eq "GS") {
		tt = "LS";
	}
	if (not(authorised("SYSTEM CONFIGURATION CREATE", msg, tt))) {
		{}
	}
	if (not(authorised("SYSTEM CONFIGURATION UPDATE", msg, tt))) {
		{}
	}
	if (not(authorised("SYSTEM CONFIGURATION DELETE", msg, tt))) {
		{}
	}

	if (not(authorised("CURRENCY UPDATE DECIMALS", msg, "EXODUS"))) {
		{}
	}
	if (not(authorised("MENU SUPPORT", msg, "LS"))) {
		{}
	}
	//if security('%DELETE%':'SUPPORT MENU ACCESS',xx) else null
	//if security('%DELETE%':'USE TCL COMMAND KEY F5',xx) else null
	if (not(authorised("UPLOAD CREATE", xx))) {
		{}
	}
	if (not(authorised("REQUESTLOG ACCESS", msg, "LS"))) {
		{}
	}

	//if security('%RENAME%':'AUTHORISATION CREATE USERS',xx,'USER CREATE') else null
	//if security('%RENAME%':'AUTHORISATION DELETE USERS',xx,'USER DELETE') else null
	//!if security('%UPDATE%':'USER UPDATE',xx,'AUTHORISATION UPDATE') else null
	if (not(authorised("USER UPDATE", xx, "AUTHORISATION UPDATE"))) {
		{}
	}

	call log2("*create user file", logtime);
	var users;
	if (not(users.open("USERS", ""))) {

		if (not(openfile("USERS", users, "DEFINITIONS", 1))) {
			{}
		}

		call log2("*zzz should create full user record not just the name", logtime);
		var usercodes = SECURITY.f(1);
		var nusers = usercodes.count(VM) + (usercodes ne "");
		for (const var usern : range(1, nusers)) {
			var userx = usercodes.f(1, usern);
			if (not(userx.index("---"))) {
				userx.writev(users, userx, 1);

			}
		} //usern;

	}

	call log2("*terminate old session", logtime);
	if (sessionid ne "") {
		call logprocess(sessionid, "LOGOFF", "", "", "", "");
		SYSTEM(4) = "";
	}

	call log2("*get user name", logtime);
	//prevent F5
	if (not resetting) {
		ENVIRONSET(38) = "";
		call netlogin("INIT");
	}

	//moved up before any updating
	//call log2('*init.general obtaining exclusive access:',logtime)

	call log2("*start new session", logtime);
	sessionid = "";
	call logprocess(sessionid, "LOGIN", "", "", "", "");
	SYSTEM(4) = sessionid;

	if (not(SYSTEM.f(22).length())) {
		SYSTEM(22) = 300;
	}

	//call log2("*suppress background indexing if time out is active", logtime);
	//if (SYSTEM.f(22)) {
	//	INDEXTIME = 0;
	//}

	call log2("*suppress words not to be indexed", logtime);
	//see ED SYSTEM * ... ENVIRONMENT words like LTD
	DEFAULTSTOPS = "";

	//this may not be necessary now that menus are determined by authorisation
	call log2("*get maintenance user menu if any", logtime);
	if (xx.open("SCHEDULES", "")) {
		temp = "ADAGENCY";
	} else {
		temp = "EXODUS2";
	}
	ENVIRONSET(37) = temp;

	call log2("*force the F1 key to be general help (Ctrl+F1) instead of context help", logtime);
	PRIORITYINT(1) = var().chr(0) ^ ";";

	//if security('USE SPECIAL KEYS',MSG,'') then
	INTCONST(4) = var().chr(0) ^ var("41").iconv("HEX2");
	INTCONST(18) = var().chr(0) ^ var("1F").iconv("HEX2");
	INTCONST(26) = var().chr(0) ^ var("19").iconv("HEX2");
	PRIORITYINT(7) = var("1F").iconv("HEX2");
	MOVEKEYS(27) = var("14").iconv("HEX2");
	MOVEKEYS(25) = var("05").iconv("HEX2");
		//break key on
	//end else
	// @int.const<4>=''
	// @int.const<18>=''
	// @int.const<26>=''
	// @priority.int<7>=''
	// @move.keys<27>=''
	// @move.keys<25>=''
	// *break key off
	// end

	//call log2('*allow/disallow F5',logtime)
	//if security('USE TCL COMMAND KEY F5',MSG,'') then
	ENVIRONSET(38) = 1;
	// *@priority.int<2>=char(0):iconv('3F','HEX')
	//end else
	// @environ.set<38>=''
	// *@priority.int<2>=''
	// end

//	if (VOLUMES) {
//		perform("ADDMFS SHADOW.MFS FILEORDER.COMMON");
//	}

	call log2("*open general files", logtime);
	var valid = 1;
	//DEFINITIONS='' why was this commented out?
	if (not(openfile("ALANGUAGE", sys.alanguage, "DEFINITIONS"))) {
		valid = 0;
	}
	if (not(openfile("COMPANIES", sys.companies, "DEFINITIONS"))) {
		valid = 0;
	}
	if (not(openfile("COMPANY_VERSIONS", xx, "COMPANIES"))) {
		valid = 0;
	}
	if (not(openfile("CURRENCIES", sys.currencies, "DEFINITIONS"))) {
		valid = 0;
	}
	if (not(openfile("CURRENCY_VERSIONS", xx, "CURRENCIES"))) {
		valid = 0;
	}
	if (not(openfile("MARKETS", sys.markets, "DEFINITIONS"))) {
		valid = 0;
	}
	if (not(openfile("MARKET_VERSIONS", xx, "MARKETS"))) {
		valid = 0;
	}
	if (not(openfile("UNITS", sys.units, "CURRENCIES"))) {
		valid = 0;
		sys.units = "";
	}
	if (not(openfile("ADDRESSES", sys.addresses, "DEFINITIONS"))) {
		valid = 0;
	}
	if (not(openfile("DOCUMENTS", sys.documents, "DEFINITIONS"))) {
		valid = 0;
	}
	//IF OPENFILE2('SHADOW',shadow,'DEFINITIONS',1) ELSE valid=0
	if (xx.open("SHADOW", "")) {
		perform("DELETEFILE SHADOW (S)");
	}

	if (not valid) {
		var().chr(7).output();
		if (interactive) {
			call note("DO NOT CONTINUE UNLESS YOU KNOW WHAT YOU ARE DOING");
		}
	}

//	if (VOLUMES) {
//
//		call log2("*open processes own lists file", logtime);
//
//		var workdir = "NEOS" ^ SYSTEM.f(24).oconv("R(0)#4");
//		var workpath = "DATAVOL/" ^ workdir ^ "/";
//		workpath.converter("/", OSSLASH);
//
//		//check/create folder
//		//initdir workpath:'REVMEDIA.*'
//		//tt=dirlist()
//		tt = oslistf(workpath ^ "REVMEDIA.*");
//		if (not tt) {
//			osshell("mkdir " ^ workpath);
//			perform("NM " ^ workpath ^ " " ^ var().timedate() ^ "(S)");
//		}
//
//		//attach folder
//		perform("ATTACH " ^ workpath ^ " (S)");
//
//		//check/make LISTS file
//		if (not(lists.open("LISTS", ""))) {
//			lists = "";
//		}
//		if (not(lists.index(workpath))) {
//			createfile("" ^ workpath ^ " LISTS (S)");
//			if (not(lists.open("LISTS", ""))) {
//				lists = "";
//			}
//			if (not(lists.index(workpath))) {
//				//call note('FAILED TO MAKE LISTS FILE ON ':workpath
//			}
//		}
//
//		call log2("*check lists file exists", logtime);
//		if (not(lists.open("LISTS", ""))) {
//			clearfile(lists);
//		}
//
//	}

	if (not(openfile("LISTS", lists, "DEFINITIONS"))) {
		{}
	}

	//call log2('*check lists file are not corrupted and zero them if they are',logtime)
	//no need since always cleared above now
	////call checkfile'LISTS')

	call log2("*check indexing, locks and processes files are not corrupted and zero them if they are", logtime);
	//call checkfile"!INDEXING");
	//call checkfile"LOCKS");
	//call checkfile"PROCESSES");
	//call checkfile"STATISTICS");

	call log2("*check/fix !xxx records in !xxx files", logtime);
	//call fixindexes);

	//clear out ancient expired locks
	//TODO

	/*;
		call log2('*fix LOG file location bug',logtime);
		//was logging per process number instead of per database
		//due to misassociation with LISTS file instead of DEFINITIONS file
		//will correctly reassociate IF process number remains de facto related to db
		year=(date() 'D')[-4,4];
		//proper place for log file is next to definitions
		if openfile('LOG':year,log,'DEFINITIONS') then;
			//if log file is in the work folder
			if index(log,'DATAVOL',1) then;
				//volume=log[-28,17]
				//get rid of the bad file by renaming it
				perform 'RENAMEFILE LOG':year:' LOG':year:'.BAD (S)';
				//get back to any existing correct file
				tattach='ATTACH ../data/':system<17>:'/GENERAL LOG':year:' (S)';
				convert '/' to OSSLASH_ in tattach;
				perform tattach;
				//open/create new in the right place
				if openfile('LOG':year,log,'DEFINITIONS') then;
					//and copy the bad log file records to the good log file
					perform 'COPY LOG':year:'.BAD * (SO) TO: (LOG':year;
					end;
				end;
			end;
	*/

	call log2("*make global per installation files", logtime);
	var filenamesx = "PROCESSES,STATISTICS,REQUESTLOG";
	for (const var ii : range(1, 999)) {
		var filename = filenamesx.field(",", ii);
		///BREAK;
		if (not filename) break;
		//TODO lock/prevent double create with other processes
		var file;
		if (not(file.open(filename, ""))) {
			createfile("REVBOOT " ^ filename ^ " (S)");
//			if (VOLUMES) {
//				perform("CONVGLOBAL REVBOOT GLOBAL " ^ filename ^ " (S)");
//				perform("DELETEFILE DICT." ^ filename ^ " (S)");
//				perform("ATTACH .\\GENERAL DICT." ^ filename ^ " (S)");
//				perform("ATTACH REVBOOT " ^ filename ^ " (S)");
//			}
			if (not(file.open(filename, ""))) {
				call sysmsg(filename.quote() ^ " could not be created by INIT.GENERAL");
			}
		}
	} //ii;

	call log2("*perform the autoexec task BEFORE initialising other systems", logtime);
	if (not exodusid) {
		if (temp.read(DEFINITIONS, "AUTOEXEC")) {
			perform("TASK AUTOEXEC");
		}
	}

	// Attach foreign files BEFORE any database operations
	var foreignfiles = osshellread("dbattach {L}").converter("\n ", FM ^ VM).trim(FM);
	var attach = "";
	var foreign_dbno;
	//group filenames by foreign dbname
	for (var foreignfile : foreignfiles) {
		var foreign_dbname = foreignfile.f(1, 1);
		if (not foreign_dbname)
			continue;
		var filename = foreignfile.f(1, 2);
		if (not attach.locate(foreign_dbname, foreign_dbno, 1))
			attach.inserter(1, foreign_dbno, foreign_dbname);
		attach.inserter(2, foreign_dbno, -1, filename);
	}
	//attach filenames per foreign dbname
	var foreign_dbnames = attach.f(1).convert(VM, FM);
	foreign_dbno = 0;
	for (var foreign_dbname : foreign_dbnames) {
		if (not(foreign_dbname))
			continue;
		var foreign_dbconn;
		if (not foreign_dbconn.connect(foreign_dbname))
			abort("Error: serve_agy: Cannot connect attach to " ^ foreign_dbname.quote());
		foreign_dbno++;
		var foreign_filenames = attach.f(2, foreign_dbno);
		if (not foreign_dbconn.attach(foreign_filenames.convert(SM, FM))) {
			var msg="Error: serve_agy: Cannot attach to " ^ foreign_filenames.convert(FM, " ") ^ " on " ^ foreign_dbconn;
			msg.errputl();
			abort(msg);
		}
	}

	call log2("*get first company for init.acc", logtime);
	clearselect();
	select(sys.companies);
	if (not LISTACTIVE) {
		//call note('WARNING: *** NO COMPANY RECORD ***')
		call log2("WARNING: --- NO COMPANY RECORD ---");
		sys.company = "";
	}
	var anyfixed = -1;
fixnextcompany:
	anyfixed += 1;
	sys.gcurrcompany = "";
	var maxyear = "";
	if (readnext(companycode)) {
		if (not(sys.company.read(sys.companies, companycode))) {
			goto fixnextcompany;
		}

//		if (VOLUMES) {
//			sys.company(27) = sys.company.f(27).invert();
//		}

		//initialise with a recent company
		tt = sys.company.f(2).field("/", 2);
		if (tt gt maxyear) {
			maxyear = tt;
			sys.gcurrcompany = companycode;
		}

		var marketcode = sys.company.f(30);
		if (marketcode) {
			if (sys.markets.open("MARKETS", "")) {
				var market;
				if (not(market.read(sys.markets, marketcode))) {
					market = "";
					msg = marketcode.quote() ^ " is missing from company " ^ companycode;
					call note(msg);
					goto fixcompany;
				}
			}
		}

		call log2("*remove obsolete period 13 from deloitte data", logtime);
		if (sys.company.index("13X4WEEK,1/7,5")) {
			tt = sys.company.f(16);
			tt.swapper("13/", "12/");
			sys.company(16) = tt;
fixcompany:
			sys.company.write(sys.companies, companycode);
			goto fixnextcompany;
		}

		if (anyfixed) {
			goto fixnextcompany;
		}
	}
	clearselect();

	var menu = "";

	call log2("*open accounts system files", logtime);
	if (APPLICATION eq "ACCOUNTS" or APPLICATION eq "ADAGENCY") {
		//call init.acc()
		//call indirectly to avoid c++ include
		systemsubs = "initacc";
		call systemsubs(menu);
	}
//	if (VOLUMES) {
//		perform("MACRO ACCOUNTS");
//	}

	call log2("*open advertising system files INIT.AGENCY", logtime);
	if (APPLICATION eq "ADAGENCY") {
		//call init.agency()
		//call indirectly to avoid c++ include
		systemsubs = "initagency";
		var agencymenu = "";
		call systemsubs(agencymenu);
		//insert the agency menu media/jobs/timesheets before (to the right)
		//of the finance menu from above above
		menu = agencymenu ^ FM ^ menu;
		agencymenu = "";
	}

	call log2("*make/update menu.htm", logtime);
	//add support and help menus after (to the left of)
	//and add the closing html
	call initgeneral2("MAKEMENU", logtime, menu);

	call log2("*add number format to company records", logtime);
	clearselect();
	select(sys.companies);
	var numberformat = "";
	var compcode = "";
convcompany:
	if (readnext(compcode)) {
		var tempcompany;
		if (tempcompany.read(sys.companies, compcode)) {
			if (tempcompany.f(22) eq "") {
				if (not numberformat) {
					call decide("Which format do you want for numbers ?||(See \"NUMBER FORMAT\" on the company file)", "1.000,00 (dot for thousands)" ^ VM ^ "1,000.00 (comma for thousands)", reply);
					if (reply eq 1) {
						numberformat = "1.000,00";
					} else {
						numberformat = "1,000.00";
					}
				}
				numberformat.writev(sys.companies, compcode, 22);

				goto convcompany;
			}
		}
	}
	clearselect();

	call log2("*get the company description", logtime);
	sys.company = "";
	var currperiod = "";
	//call init.company('')
	//change so that interactive ADAGENCY gets a company code
	//force acquisition of language
	sys.glang = "";
	//gcurr.company=''
	call initcompany(sys.gcurrcompany);
	SYSTEM(37) = sys.gcurrcompany;

	call log2("*ensure random key exists", logtime);
	var datasetid;
	if (not(datasetid.read(DEFINITIONS, "GLOBALDATASETID"))) {
newdatasetid:
		dostime = ostime();
		datasetid = var().date() ^ "." ^ dostime;
		datasetid.converter(".", "");
		datasetid = datasetid.oconv("MX");
		datasetid = (datasetid ^ datasetid ^ datasetid ^ datasetid).substr(1, 8);
adddatasetcodename:
		datasetid(2) = SYSTEM.f(23);
		datasetid(3) = SYSTEM.f(17);
		datasetid.write(DEFINITIONS, "GLOBALDATASETID");
	}
	if (datasetid.f(3) eq "" and SYSTEM.f(17)) {
		goto adddatasetcodename;
	}

	//lock database to particular computers unless logging in as EXODUS
	//if @username<>'EXODUS' and datasetid<4> then
	//lock even to EXODUS to prevent installation where EXODUS pass is known
	if (datasetid.f(4)) {
		if (not(datasetid.f(4).locate(cid(), xx))) {
			USER4 = var("CANNOT USE THIS DATABASE ON THIS COMPUTER").quote();
			gosub failsys();
			stop();
		}
	}

	//<61>=testdb (not livedb)
	//call readbakpars(bakpars)
	//system<61>=bakpars<11>

	//suggest change globaldatasetid if changed datasetname or datasetid
	if (datasetid.f(2) ne SYSTEM.f(23) or datasetid.f(3) ne SYSTEM.f(17)) {
		//if system<17>[-4,4]<>'TEST' and interactive and @username='EXODUS' then
		if ((not(SYSTEM.f(61)) and interactive) and USERNAME eq "EXODUS") {
			if (datasetid.f(1) ne "1EEC633B") {
				call decide("This database has been copied or|the database name or code has been changed.|Is this going to be a unique new master database?", "Yes - Going to be a new independent database" ^ VM ^ "No - just backing up, renaming or recoding the database", reply, 2);
				if (reply eq 1) {
					goto newdatasetid;
				}
			}
			goto adddatasetcodename;
		}
	}
	SYSTEM(45) = datasetid.f(1);

	//call log2('*check supported',logtime)
	//if @username<>'EXODUS' then
	// locate datasetid<1> in '1DFE7C58' setting xx then
	//  if index(ucase(system<23>),'QATAR',1) then
	//   perform 'OFF'
	//   logoff
	//   end
	//  end
	// end

	//call log2('*take down the backup reminder',logtime)
	//if reminder then call mssg('','DB',reminder,'')

	if (not resetting) {

		call log2("*show and update last login time", logtime);
		var userx;
		if (not(userx.read(DEFINITIONS, "USER*" ^ USERNAME))) {
			userx = "";
		}
		if (userx.f(4) and interactive) {
			var day = var("Mon,Tue,Wed,Thu,Fri,Sat,Sun").field(",", (userx.f(4) - 1).mod(7) + 1);
			call note("Info:||" ^ USERNAME ^ " last used " ^ currdataset ^ " on||" ^ day ^ " " ^ userx.f(4).oconv("D") ^ " at " ^ userx.f(5).oconv("MTH") ^ "||" ^ ("on workstation " ^ userx.f(6).trim()).oconv("C#40") ^ "|");
		}

		call log2("*save last login time", logtime);
		userx.write(DEFINITIONS, "USER*" ^ USERNAME ^ "*LAST");

		call log2("*update the last login time", logtime);
		userx(4) = var().date();
		userx(5) = var().time();
		userx(6) = STATION;
		userx.write(DEFINITIONS, "USER*" ^ USERNAME);

		//call log2('*check processes',logtime)
		//clearselect
		//if interactive and ems.allocated then perform 'LISTPROCESSES CHECK'

	}

	call log2("*clean up document keys", logtime);
	if (sys.documents.open("DOCUMENTS", "")) {
		select(sys.documents);
nextdoc:
		if (readnext(docid)) {
			var docid2 = (field2(docid, "\\", -1)).field(".", 1);
			if (docid2 ne docid) {
				var doc;
				if (doc.read(sys.documents, docid)) {
					doc.write(sys.documents, docid2);
					sys.documents.deleterecord(docid);

				}
			}
			goto nextdoc;
		}
	}

	if (interactive and USERNAME eq "EXODUS") {
		perform("FINDDEADALL");
	}

//	if (VOLUMES) {
//		if (not(VOLUMES.locateusing(FM, "DATAVOL", xx))) {
//			//pcperform 'MD DATAVOL'
//			//call mkdir('DATAVOL':char(0),xx)
//			call osmkdir("DATAVOL");
//			perform("NM DATAVOL " ^ var().timedate() ^ "(S)");
//			perform("ATTACH DATAVOL (S)");
//		}
//	}
//
//	//windows stuff
//	if (VOLUMES) {
//
//		call log2("*convert codepage", logtime);
//		if (codepaging.osread("CODEPAGE.CFG")) {
//			if (not(codepage.read(DEFINITIONS, "PARAM*CODEPAGE"))) {
//				codepage = "0" ^ FM ^ codepaging.f(2);
//			}
//			if ((codepage.f(2) eq "737" and not(codepage.f(1))) and codepaging.f(3) eq "1253") {
//				perform("CONVGREEK (U)");
//			}
//		}
//
//		call log2("*installing authorised keys", logtime);
//		perform("INSTALLAUTHKEYS (S)");
//
//		call log2("*installing authorised hosts", logtime);
//		perform("INSTALLALLOWHOSTS (S)");
//
//		voc.deleterecord("$FILEMAN");
//
//		call log2("*put the admenus program as the system menu file", logtime);
//		//as there is no way to have multiple menus files
//		if (APPLICATION eq "ADAGENCY") {
//			var setfilecmd = "SETFILE ./ADAGENCY GLOBAL ADMENUS SYS.MENUS";
//			setfilecmd.converter("/", OSSLASH);
//			perform(setfilecmd);
//		}
//
//	}

	//call log2('*create user name index',logtime)
	//convkey='CONVERTED*USERNAMEINDEX'
	//read xx from definitions,convkey else
	// perform 'WINDOWSTUB USER.SUBS CREATEUSERNAMEINDEX'
	// write date() on definitions,convkey
	// end

	if (not resetting) {

		call initgeneral2("FIXURLS", logtime);
		call initgeneral2("UPDATEIPNOS4EXODUS", logtime);
		call initgeneral2("UPDATEUSERS", logtime);
		call initgeneral2("CREATEALERTS", logtime);
		//call init.general2('LASTLOGWARNING':fm:lastlog,logtime)
		call initgeneral2("OSCLEANUP", logtime);

		//per installation or slow things done only in live
		//so assuming test starts first then at least test is running
		//live systems only
		if (not(SYSTEM.f(61))) {
			call initgeneral2("COMPRESSLOGS", logtime);
			call initgeneral2("TRIMREQUESTLOG", logtime);
			call initgeneral2("REORDERDBS", logtime);
		}

	}

	//in INIT.GENERAL and DEFINITION.SUBS
	//if dir('ddns.cmd') then
	// perform 'STARTDDNS'
	// end

	//FOLLOWING MUST ALL BE DONE LAST OF ALL

	call log2("*save upgrade history and email notification", logtime);
	if (version.osread("GENERAL" OSSLASH_ "VERSION.DAT")) {

		//get version installed
		var versioninstalled = version.field("\r", 1).trim();

		//get version last run
		call osread(upgradelog, "UPGRADE.CFG");
		var versionlastrun = field2(upgradelog, var().chr(10), -1);

		//if changed then log and email users
		if (versioninstalled ne versionlastrun) {

			//log the upgraded version
			if (upgradelog) {
				upgradelog ^= "\r\n";
			}
			upgradelog ^= versioninstalled;
			//call oswrite(upgradelog,'UPGRADE.CFG')
			var(upgradelog).oswrite("upgrade.cfg");

			//upgrade locally installed SYSOBJ files
			call log2("*update sysobj $msg $rtp25", logtime);
			perform("SETFILE REVBOOT SYSPROG,SALADS SYSOBJ SYSOBJ (S)");
			perform("NCOPY GBP $MSG $RTP25 (SON) TO: (SYSOBJ)");

			//make dos screen run at full speed-must have been done everywhere now
			//call log2('*fixvideo',logtime)
			//call fixvideo

		}

		//update software version in database
		tt2 = "VERSION*LASTEMAILED";
		if (not(tt.read(DEFINITIONS, tt2))) {
			tt = "";
		}
		if (tt ne versioninstalled) {
			versioninstalled.write(DEFINITIONS, tt2);

			//email users on live systems LISTED IN SYSTEM CONFIGURATION only
			if (SYSTEM.f(58).locate(SYSTEM.f(17), xx)) {
				if (not(SYSTEM.f(61))) {
					var idate = version.field(" ", 2, 4).iconv("D");
					var itime = version.field(" ", 1).iconv("MT");
					//tt=idate 'D/J':' ':itime 'MT'
					tt = idate.oconv("D/E");
					tt = idate.substr(-4, 4) ^ "/" ^ tt.substr(1, 5) ^ " " ^ itime.oconv("MT");
					call decide("Email users about upgrade?|(or later on do F5 EMAILUSERS UPGRADE " ^ tt ^ ")", "", reply);
					if (reply eq 1) {
						perform("EMAILUSERS UPGRADE " ^ tt);
					}
					call sysmsg("EXODUS Software Upgrade " ^ tt);
				}
			}

		}

	}

	call log2("*emailing any notifications, warnings or errors", logtime);
	if (USER4) {
		call sysmsg(USER4, "Messages from INIT.GENERAL");
		USER4 = "";
	}

	call log2("*stop init.general - releasing exclusive access", logtime);
	call unlockrecord("VOC", voc, "INIT.GENERAL.LOGIN");

	if (not resetting) {

		//perform 'REPLICATION UPDATE'

		if (interactive and USERNAME ne "EXODUS.NET") {
			call log2("*do a few escapes then F10 HOME to initialise the menu system", logtime);
			//do home enter home to open the first menu at the first option
			DATA ^= var().chr(27) ^ var().chr(27) ^ var().chr(27) ^ INTCONST.f(7);
			DATA ^= MOVEKEYS.f(15) ^ var().chr(13) ^ MOVEKEYS.f(15);

			call log2("*if not interactive then start the required process", logtime);
			//by pressing F5

		} else {
			//data char(13)
			//chain 'RUNMENU LISTEN'

			call log2("*indicate success to LOGIN", logtime);
			if (SYSTEM.f(33, 10)) {
				//call oswrite('OK',system<33,10>:'.$2')
				var("OK").oswrite(SYSTEM.f(33, 10) ^ ".$2");
			}

			call log2("*chain to NET AUTO (" ^ SYSTEM.f(17) ^ ") INIT.GENERAL Quitting.", logtime);
			chain("NET AUTO");
		}

	}

	stop();
	return 0;

	return "";
}

subroutine getsystem() {
	call log2("*get " ^ tt ^ " parameters", logtime);
	if (not(systemx.osread(tt))) {
		return;
	}

	//ensure default style is null
	var tt3 = systemx.f(46);
	tt3.converter(VM, "");
	tt3.swapper("Default", "");
	if (tt3 eq "") {
		systemx(46) = "";
	}

	//parameters in the exodus\system file override params from definitions
	//ie global installation parameters override dataset parameters
	var ni = systemx.count(FM) + 1;
	for (const var ii : range(1, ni)) {
		if (systemx.f(ii).length()) {
			SYSTEM(ii) = systemx.f(ii);
		}
	} //ii;

	//save config file time so can detect if restart required
	SYSTEM(100, tt2) = tt.osfile().f(3);

	return;
}

subroutine failsys() {
	msg = USER4;

	tt = "*Authorisation Failure. " ^ msg;
	tt.converter(FM, "|");
	call log2(tt, logtime);

	//onscreen message
	s33 = SYSTEM.f(33);
	SYSTEM(33) = "";
	call mssg(msg ^ "", "T3", yy, "");
	SYSTEM(33) = s33;

	//respond to user
	msg.swapper(FM ^ FM, FM);
	msg.converter(FM, "|");
	//call oswrite(msg,system<33,10>:'.$2')
	var(msg).oswrite(SYSTEM.f(33, 10) ^ ".$2");

	//and quit
	perform("OFF");
	logoff();

	return;
}

libraryexit()
