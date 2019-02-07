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
#include <authorised.h>
#include <openfile.h>
#include <logprocess.h>
#include <neosyslogin.h>
#include <sysmsg.h>
#include <initacc.h>
#include <initagency.h>
#include <initcompany.h>

#include <gen.h>
#include <fin.h>

var lastlog;
var logtime;
var msg;
var pidrec;
var dbdate;
var dbtime;
var dbdatetimerequired;
var reply;
var reply2;
var colors;
var tt;//num
var reportkey;
var tt2;//num
var smtp;
var s33;
var bakpars;
var vn;
var os;
var ver;
var vern;
var cpu;
var errors;
var shadow;
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
	//c gen

	//$insert gbp,arev.common2
	//global tt,tt2,s33

	//!WARNING decide() returns REPLY number instead of VALUE when not interactive

	//ensure cursor on the left
	print(var().at(0));

	call log2("GETLASTLOG", lastlog);

	call log2("*Starting INIT.GENERAL --- STARTUP/RESTART ---", logtime);

	//unused system variables like @user0 not @recur0
	//http://www.sprezzatura.com/library/revmedia/attack.php
	//@sw<10>='XYZ'
	//@vw<10>='ABC'

	//!!! do not use getenv before initialised - or shell etc which uses it
	//gosub getenv
	call initgeneral2("GETENV", logtime);

	var resetting = PSEUDO == "RESET";
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
	var sessionid = SYSTEM.a(4);
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

	//equ installationdesc to system<57>;*should be like hostname eg unilever
	//will be used for system email sender and ddns like
	//eg domain name eg unilever -> zoneedit UNILEVER.HOSTS.NEOSYS.COM
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
	//system<124> to checkforupgrades 0 1 default to yes
	//system<125> to closeafterbackups 0 1 default to yes
	//system<126> to testcanstartprocesses 0 1 default to no
	//system<127> to charset eg greek is windows-1253, arabic is windows-1256
	//mode con cp -> Code page:
	//720 -> arabic
	//737 -> greek
	//437 -> standard dos latin us
	//system<128> to max nologin days
	//system<129> to initwaitsecs default to 55
	//see also systemconfiguration_dict.js
	//system<130> to allpunctuation
	//system<131> to monitor https port - default 4428

	call log2("*init.general obtaining exclusive access:", logtime);
	var verbs;
	if (verbs.open("VERBS", "")) {
		//timeout and crash out quickly otherwise might get unlimited processes
		//this lock is in INIT.GENERAL and MONITOR2 (to avoid starting processes)
		var initwaitsecs = SYSTEM.a(129);
		if (not initwaitsecs) {
			initwaitsecs = 50 + var(10).rnd();
		}
		//also in LOGON.OLD and INIT.GENERAL
		var lockkey = "INIT.GENERAL.LOGIN";
		if (not(verbs.lock( lockkey))) {
			//np if own lock
			if (FILEERROR ne "414") {
				if (not(lockrecord("VERBS", verbs, "INIT.GENERAL.LOGIN", "", initwaitsecs))) {
					msg = "INIT.GENERAL couldnt get exclusive lock. Quitting.";
					//maybe a long upgrade process is running
					call note(msg);
					if (SYSTEM.a(33)) {
						gosub failbatch();
					}
					perform("OFF");
					var().logoff();
				}
			}
		}
	}

	var neosysid = var("NEOSYS.ID").osfile();

	call log2("*determine the pid if possible", logtime);
	if (not(SYSTEM.a(54))) {
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
		if (DEFINITIONS.open("DEFINITIONS", "")) {
			var dbversion;
			if (not(dbversion.read(DEFINITIONS, "DBVERSION"))) {
				goto updateversion;
			}
			if (oldmethod and (dbversion.a(1) == 14334.5)) {
			}
			if (dbversion.a(1) > dbdatetimerequired) {
				msg = "Software version " ^ dbversion.a(1).oconv("D") ^ " " ^ dbversion.a(1).field(".", 2).oconv("MTS") ^ " is incompatible with" ^ FM ^ "Database version " ^ dbdate ^ " " ^ dbtime;
				msg = msg.oconv("L#60");
				//abort since db is advanced
				if (not interactive) {
badversion:
					USER4 = msg;
					gosub failbatch();
					var().stop();

				}else{
decideversion:
					var options = "Quit (RECOMMENDED)";
					options.r(-1, "Continue");
					options.r(-1, "Mark database as version " ^ dbdate ^ " " ^ dbtime ^ " and continue");
					call decide("!" ^ msg ^ "", options, reply);
					if (reply == 1) {
						goto badversion;
					}
					var msgx = "!WARNING *UNPREDICTABLE* CONSEQUENCES" ^ FM ^ FM ^ "WHAT IS THE PASSWORD?";
					call inputbox(msgx, 20, 0, "", reply2, 0x1B);
					if (reply2 ne "UNPREDICTABLE") {
						call note("THAT IS NOT THE CORRECT PASSWORD");
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
				dbversion.write(DEFINITIONS, "DBVERSION");
			}
//L923:
		}
	}

	call log2("*copy any new data records", logtime);
	var bp;
	if (bp.open("GBP", "")) {
		for (var ii = 1; ii <= 9999; ++ii) {
			var rec;
			if (not(rec.read(bp, "$DATA." ^ ii))) {
				rec = "";
			}
		///BREAK;
		if (rec == "") break;;
			var filename = rec.a(1);
			var id = rec.a(2);
			rec = rec.field(FM, 3, 99999);
			var file;
			if (file.open(filename, "")) {
				rec.write(file, id);
			}
		};//ii;
	}

	//syslock=if revrelease()>=2.1 then 36 else 23

	//place lock indicating processing (initialising actually)
	//if interactive else call rtp57(syslock, '', '', trim(@station):system<24>, '', '', '')

	//EMS MEMORY notes
	//explanation of how to use power-on setup or emsmagic
	//to enable EMS as good as possible
	//NB NEOSYS memory is tested in LOGON.OLD
	//http://www.columbia.edu/~em36/wpdos/emsxp.html

	//AREV startup options
	//http://www.revelation.com/knowledge.nsf/461ff2bd5a8ddeed852566f50065057d/3c57c588ce21daf9852563920051f70b?OpenDocument
	//AREV adagency,neosys /X /M4096
	// /X tells the system to use EMS memory for variable storage.
	// /M tells the system to use EMS memory for running and executing programs.
	// /M can also be followed by an option memory allocation amount in kilobytes.
	//You should never allocate more than 4096K of EMS memory otherwise unstable
	// /O tells the system to not use any EMS memory at all.
	// /E tells the system to not use the on-board math chip and use a math chip emulator instead.
	// /, prompts for a user name

	/* no longer required since CONFIG.NT and AUTOEXEC.NT now in neosys\neosys;
		call log2('*improve NT/XP memory configuration',logtime);
		sys32='c:\windows\system32\';
		osread temp from sys32:'config.nt' else;
			sys32='c:\winnt\system32\';
			end;
		//improve NT/XP memory configuration
		//autoconfigure no DOSX.EXE if no EMS
		//always do this now since some EMS systems use normal memory for both blocks
		//if not(get.ems('')) else
			//remove DOSX.EXE but only if no EMS available
			//since it can cause problems for other system programs
			osread temp from sys32:'autoexec.nt' then;
				temp2=temp;
				//tt='lh %SystemRoot%\system32\mscdext.exe'
				//tt<-1>='lh %SystemRoot%\system32\dosx'
				//tt<-1>='lh %SystemRoot%\system32\redir'
				//NB change case to DOSX.EXE so that if someone uncomments it
				//it will not be converted again
				//swap 'lh %SystemRoot%\system32\dosx' with ucase('rem neosys ':'lh %SystemRoot%\system32\dosx') in temp2
				//now comment out ALL lh lines changing case to LH so uncommenting will work
				swap \0A\:'lh ' with \0A\:'rem NEOSYS LH ' in temp2;
				if temp2<>temp then call oswrite(temp2,sys32:'autoexec.nt');
				end;
		// end
		osread temp from sys32:'config.nt' then;
			temp2=temp;
			pos1=index(temp,\0A\:'files=',1);
			if pos1 then;
				line=temp[pos1+1,\0D\];
				temp2[pos1+1,len(line)]='FILES=200';
				if temp2<>temp then call oswrite(temp2,sys32:'config.nt');
				end;
			end;
	*/

	call log2("*force replication system to reinitialise", logtime);
	call shadowmfs("NEOSYSINIT", "");

	call log2("*reattach data", logtime);
	var volumesx = VOLUMES;
	var nvolumes = VOLUMES.count(FM) + 1;
	for (var volumen = 1; volumen <= nvolumes; ++volumen) {
		var volume = volumesx.a(volumen);
		if (volume.substr(1,8) == ("..\\" "DATA" "\\")) {
			execute("ATTACH " ^ volume ^ " (S)");
		}
	};//volumen;

	call log2("*perform RUN GBP LOGON.OLD UPGRADEVERBS", logtime);
	perform("RUN GBP LOGON.OLD UPGRADEVERBS");

	//call log2('*set workstation time to server time',logtime)
	//call settime('')

	call log2("*save the original username and station", logtime);
	if (not(SYSTEM.a(43))) {
		SYSTEM.r(43, USERNAME);
	}
	if (not(SYSTEM.a(44))) {
		SYSTEM.r(44, STATION.trim());
	}

	call log2("*detach merges", logtime);
	//locate 'MERGES' in @files using fm setting xx then perform 'DETACH MERGES (S)'
	var xx;
	if (xx.open("MERGES", "")) {
		perform("DETACH MERGES (S)");
	}

	call log2("*get user video table if any", logtime);
	var temp = ENVIRONKEYS.a(2);
	temp.converter(".", "");
	if (colors.osread(temp ^ ".VID")) {
		var color2;
		if (not(color2.read(systemfile(), ENVIRONKEYS ^ ".VIDEO"))) {
			color2 = "";
		}
		if (colors ne color2) {
			colors.write(systemfile(), ENVIRONKEYS ^ ".VIDEO");
			//call colortoescold);
		}
	}
	//fix monochrome video problem
	//TEMP=CHAR(27):'C70'
	//IF @EW<3>=TEMP AND @EW<6>=TEMP THEN @EW<3>=CHAR(27):'C07'

	call log2("*setup escape sequence for standard color and background", logtime);
	temp = HW.a(3)[4] ^ HW.a(8)[4];
	if (temp == "00") {
		temp = "70";
	}
	tt = 0x1B;
	tt ^= "C";
	tt ^= temp;
	AW.r(30, tt);

	call log2("*convert reports file", logtime);
	if (SYSTEM.a(17) ne "DEVDTEST") {
		var reports;
		if (reports.open("REPORTS", "")) {
			select(reports);
nextreport:
			if (readnext(reportkey)) {
				if (RECORD.read(reports, reportkey)) {
					var filename = reportkey.field("*", 1);
					var file;
					if (file.open(filename, "")) {
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
				goto nextreport;
			}
		}
	}

	if (not(DEFINITIONS.open("DEFINITIONS", ""))) {
		var().chr(7).output();
		msg = "The DEFINITIONS file is missing";
		msg.r(-1, "Did you startup using the right command file/datasettype?");
		call note(msg);
	}

	var notherusers = otherusers("").a(1);

	call log2("*check/get authorisation", logtime);
	if ((SYSTEM.a(4) == "") and not SYSTEM.a(33)) {
		var nusers = getauthorisation();
		if (not nusers) {

			call log2("*if batchmode then respond to response file and close", logtime);
			if (SYSTEM.a(33)) {
				gosub failbatch();
				var().stop();
			}

			call log2("*inform user and close", logtime);
			msg = var("594F552043414E4E4F5420555345204E454F53595320534F465457415245204F4E205448495320434F4D50555445527C554E54494C20594F5520484156452054484520415554484F5249534154494F4E204E554D4245522E").iconv("HEX");

			var().chr(7).output().str(3);
			call note(msg);
			perform("OFF");
			var().logoff();

		}

		call log2("*check users", logtime);
		//if nusers lt otherusers('')+1 then
		if (nusers < notherusers + 1) {
			msg = var("4D4158494D554D20415554484F5249534544204E554D424552204F46205553455253204558434545444544").iconv("HEX");
			call note(msg);
			if (SYSTEM.a(33)) {
				gosub failbatch();
			}
			perform("OFF");
			var().logoff();
		}

	}

	call log2("*prevent use of this program via F10", logtime);
	if (((initmode ne "LOGIN" and LEVEL ne 1) and interactive) and not resetting) {
		msg = "You cannot quit from within another program via F10.";
		msg.r(-1, "Please quit all programs first and then try again.");
		var().chr(7).output();
		call note(msg);
		var().stop();
	}

	call log2("*check the operating system date", logtime);
	//CHECKSYSDATE:
	var config = "";
	if (not(config.osread("NEOSYS.CFG"))) {
		if (config.osread("\\lastdate.rev\\")) {
			{}
		}
	}
	var lastdate = config.a(1);
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
		config.r(1, var().date());
		call oswrite(config, "NEOSYS.CFG");
	}

	call log2("*check for invalid characters in workstation name", logtime);
	if (STATION.index("\'") or STATION.index(DQ)) {
		msg = "WARNING: NEOSYS WILL NOT WORK PROPERLY BECAUSE";
		msg ^= FM ^ "YOUR WORKSTATION NAME (" ^ STATION.trim() ^ ")";
		msg ^= FM ^ "CONTAINS QUOTATION MARKS. PLEASE ASK YOUR ";
		msg ^= FM ^ "TECHNICIAN TO CHANGE THE WORKSTATION NAME.";
		var().chr(7).output();
		call note("!" ^ msg ^ "|");
	}

	call log2("*convert SYSTEM to SYSTEM.CFG", logtime);
	//leave old SYSTEM around in case old version of software reinstalled
	//can be deleted manually
	if (tt.osread("SYSTEM")) {
		if (not(var("SYSTEM.CFG").osfile())) {
			var(tt).oswrite("SYSTEM.CFG");
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
	tt = "SYSTEM.CFG";
	tt2 = 2;
	gosub getsystem();
	tt = "..\\..\\" "SYSTEM.CFG";
	tt2 = 1;
	gosub getsystem();

	//! system configuration defaults
	// installation group
	if (not(SYSTEM.a(123))) {
		SYSTEM.r(123, "GLOBAL");
	}
	// upgrades yes
	if (SYSTEM.a(124) == "") {
		SYSTEM.r(124, 1);
	}
	//close after backup yes
	if (not(SYSTEM.a(125))) {
		SYSTEM.r(125, 2);
	}
	//test should start missing processes
	if (not(SYSTEM.a(126))) {
		SYSTEM.r(126, 0);
	}

	if (not(SYSTEM.a(57))) {
		call log2("*determine systemid from old smtp sender name", logtime);
		call osread(smtp, "..\\..\\SMTP.CFG");
		if (not(smtp.a(1))) {
			call osread(smtp, "SMTP.CFG");
		}
		if (not(smtp.a(1))) {
			if (not(smtp.read(DEFINITIONS, "SMTP.CFG"))) {
				{}
			}
		}
		if (smtp.a(1)) {
			var sysname = smtp.a(1).field("@", 1).lcase();
			//remove all punctuation
			sysname.converter("!\"#$%^&*()_+-=[]{};:@,./<>?", "");
			SYSTEM.r(57, sysname);
			call osread(tt, "SYSTEM.CFG");
			tt.r(57, sysname);
			call oswrite(tt, "SYSTEM.CFG");
		}
	}

	//call log2('*save database system parameters',logtime)
	//dont do this anymore since copying system params from place
	//to place is probably wrong
	//read tt from definitions,'SYSTEM' else
	// write system on definitions,'SYSTEM'
	// end

	call log2("*force reevaluation of cid", logtime);
	SYSTEM.r(111, "");
	SYSTEM.r(111, cid());
	//try twice because was unreliable and is important
	if (not(SYSTEM.a(111))) {
		SYSTEM.r(111, cid());
	}

	SYSTEM.r(51, APPLICATION);

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
	s33 = SYSTEM.a(33);
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

	//<61>=testdb (not livedb)
	call getbackpars(bakpars);
	SYSTEM.r(61, bakpars.a(11));

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
	if (SYSTEM.a(120)) {
		//if server not on gmt/utc and user tz is set then warning
		//since a) usertime will be server time b) database will get non-gmt date/times
		//if system<118> then
		// call note('WARNING: User time zone ignored and|Database storing non-GMT/UTC date/time|because current server is not GMT/UTC (is ':system<120>:')')
		// end
	}else{
		SW.r(1, SYSTEM.a(118));
		//if display time is not server/gmt/utc then adjust offset to server/gmt/utc
		if (SW.a(1).length()) {
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
		SYSTEM.r(49, "..\\images\\");
	} else if (tt[-1] ne "\\") {
		SYSTEM.r(49, tt ^ "\\");
	}
//L3185:
	if (not(SYSTEM.a(46, 1))) {
		SYSTEM.r(46, 1, "#FFFF80");
	}
	if (not(SYSTEM.a(46, 2))) {
		SYSTEM.r(46, 2, "#FFFFC0");
	}
	//if system<46,3> else system<46,3>=''

	call log2("*backup the system default style", logtime);
	SYSTEM.r(47, SYSTEM.a(46));

	//why is this called again?
	call initgeneral2("GETENV", logtime);

	call log2("*get codepage as an environment variable", logtime);
	//will not override entries in SYSTEM.CFG
	if (not(SYSTEM.a(12).locateusing("CODEPAGE", VM, vn))) {
		var output = shell2("MODE CON:", xx);
		output = trim(output.convert("\r\n", FM ^ FM), FM);
		var codepage = field2(output, " ", -1);
		SYSTEM.r(12, vn, "CODEPAGE");
		SYSTEM.r(13, vn, codepage);

		if (SYSTEM.a(127) == "") {
			tt = "";
			if (codepage == 720) {
				tt = "windows-1256";
			}
			if (codepage == 737) {
				tt = "windows-1253";
			}
			SYSTEM.r(127, tt);
		}

	}

	//allpunctuation
	SYSTEM.r(130, " `~@#$%^&*()_+-=[]{};\\:\"|,./<>?\\|" "\'");

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

	call log2("*get windows version", logtime);
	ver = shell2("VER", xx);
	ver.converter(FM ^ VM, "  ");
	ver.converter("\r\n", "  ");
	//if index(ver,'Windows 9',1) or index(ver,'NT',1) or index(os,'NT',1) then
	ver = ver.trim().ucase();
	if ((ver.index("WINDOWS") or ver.index("NT")) or os.index("NT")) {
		SYSTEM.r(12, -1, "WORDSIZE");
		SYSTEM.r(13, -1, "32");
	}
	ver = ver.field(" ", 4).field("]", 1);
	//Windows Server 2003 5.2.3790 (24.04.2003)
	//Windows Server 2003, SP1 5.2.3790.1180
	//Windows Server 2003 5.2.3790.1218
	//Windows Server 2008 6.0.6001 (27.02.2008)
	var ver2 = ver.field(".", 1, 2);
	if (var("5.0,5.1,5.2,6.0,6.1,6.2,6.3").locateusing(ver2, ",", vern)) {
		ver2 = var("2000,XP,2003,2008,2008R2,2012,2012R2").field(",", vern);
		ver = ver.fieldstore(".", 1, -2, "Win" ^ ver2);
		ver.swapper(".6001", "");
		ver.swapper(".6002", " SP2");
		ver.swapper(".3790", "");
	}
	SYSTEM.r(12, -1, "VER");
	SYSTEM.r(13, -1, ver.trim());

	call log2("*get cpu version", logtime);
	if (var("WMIC.CFG").osfile()) {
		//grep "model name" /proc/cpuinfo
		cpu = "Unknown";
	}else{
		cpu = shell2("WMIC CPU GET NAME", xx);

		//Name
		//Intel(R) Xeon(R) CPU E5645 @ 2.40GHz

		//Name
		//Intel(R) Xeon(TM) CPU 3.40GHz
		//Intel(R) Xeon(TM) CPU 3.40GHz

		if (cpu.substr(1,2) == "\xFF\xFE") {
			cpu.splicer(1, 2, "");
			cpu.converter(0x00, "");
		}
		cpu.swapper("(R)", "");
		cpu.swapper("(TM)", "");
		cpu.swapper("Intel", "");
		cpu.swapper("CPU", "");
		cpu.converter("-", " ");
		var nsockets = cpu.count("\r") - 1;
		cpu.converter("\r\n", FM ^ FM);
		var nlogical = SYSTEM.a(9);
		cpu = cpu.a(3).trim() ^ " ";
		tt = nlogical / nsockets;
		if (tt > 1) {
			cpu ^= nsockets ^ "x" ^ tt;
		}else{
			cpu ^= "x" ^ nsockets;
		}
	}

	SYSTEM.r(12, -1, "CPU");
	SYSTEM.r(13, -1, cpu);

	//done in logon.old now
	//call log2('*check/detect current dataset',logtime)
	//currdataset=field(definitions[index(ucase(definitions),'\':'DATA':'\',1)+6,9999],'\',1)
	//system<17>=currdataset
	var currdataset = SYSTEM.a(17);

	/*;
		call log2('*warning if too little or too much EMS memory',logtime);
		ems.allocated=get.ems('');
		if interactive and not(index(os,'NT',1)) and not(index(ver,'Millennium',1)) then;
			//if ems.allocated>1024000 or ems.allocated<512000 then
			if ems.allocated>1100000 or ems.allocated<512000 then;
				msg='WARNING: NEOSYS may not work reliably because it|has ';
				if ems.allocated>1024000 then;
					msg:='been given more than';
				end else;
					msg:='not been given';
					end;
				msg:=' 1024Kb of "EMS" memory|';
				msg:='|1. Close NEOSYS' 'L#35';
				msg:='|2. Right click the NEOSYS icon' 'L#35';
				msg:='|3. Choose "Properties"' 'L#35';
				msg:='|4. click "Memory"' 'L#35';
				msg:='|5. Set "EMS" memory to 1024' 'L#35';
				msg:='|6. Click "OK" and restart NEOSYS' 'L#35';
				print char(7):
				call note('!':msg:'|');
				end;
			end;

		call log2('*get diskfreespace',logtime);
		reqfreemb=10;
		freemb=(diskfreespace(drive()[1,2])/1024/1024) 'MD00';

		call log2('*check if diskfreespace is sufficient',logtime);
		//notherusers=otherusers('')+1
		if freemb lt (reqfreemb*notherusers) then;
		//if 1 then
			msg='THERE IS NOT ENOUGH FREE DISK SPACE AVAILABLE';
			msg:='||NEOSYS needs at least ':reqfreemb:'Mb PER USER|of free space on disk ':drive()[1,2]:' but';
			msg:='|there is only ':freemb:'Mb available.';
			if notherusers then msg:='||There is/are ':notherusers:' other users online.';
			print char(7):
			call note(msg:'|');
			//stop
			//if freemb then perform 'OFF'
			end;
	*/

	call log2("*try to update upload.dll", logtime);
	//for extn=1 to 3
	// ext=field('net,w3c,www',',',extn)
	var ext = "NET";
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
	// next extn

	call log2("*find cygwin", logtime);
	var locations = SYSTEM.a(50);
	locations.r(1, -1, "C:\\CYGWIN\\BIN\\,\\CYGWIN\\BIN\\,..\\..\\CYGWIN\\BIN\\");
	var nn = locations.count(",") + 1;
	for (var ii = 1; ii <= nn; ++ii) {
		var location = locations.field(",", ii);
		if (location[-1] ne "\\") {
			location ^= "\\";
		}
		//initdir location:'*.*'
		//tt=dirlist()
		tt = oslistf(location ^ "*.*");
		if (tt) {
			SYSTEM.r(50, location);
		}
	///BREAK;
	if (not(tt == "")) break;;
	};//ii;

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
getproxy:
		var result = shell2(cmd, errors).lcase();
		tt = result.index("proxy server(s)");
		if (tt) {
			//tt=result[tt,';'][1,char(13)]
			tt = result.substr(tt,9999);
			tt = tt.field(";", 1).field(var().chr(13), 1);
			tt = tt.field(":", 2, 99).trim();
			tt.swapper("http=", "");
		} else if (cmd == "proxycfg") {
			cmd = "netsh winhttp import proxy ie";
			goto getproxy;
			{}
		} else {
			tt = "";
		}
		{}
		//todo get proxybypasslist
		SYSTEM.r(56, tt);
	}
	/*;
		call log2('*put up the backup reminder',logtime);
		reminder='';
		if interactive and not(system<20>) and not(resetting) then;
			call backupreminder(currdataset,msg);
			if msg then call mssg(msg,'UB':FM:FM:FM:2,reminder,'');
			end;
	*/
	call log2("*default date format is DD/MM/YY", logtime);
	DATEFORMAT = "D2/E";

	call log2("*get security - also in LISTEN", logtime);
	if (not(SECURITY.read(DEFINITIONS, "SECURITY"))) {
		if (temp.open("dict_DEFINITIONS")) {
			if (SECURITY.read(temp, "SECURITY")) {
				SECURITY.write(DEFINITIONS, "SECURITY");
			}
		}
	}
	SECURITY = SECURITY.invert();

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
	if (APPLICATION == "ACCOUNTS") {
		tt = "GS";
	}else{
		tt = "NEOSYS";
	}
	if (not(authorised("DATABASE STOP", msg, tt))) {
		{}
	}
	if (not(authorised("DATABASE RESTART", msg, tt))) {
		{}
	}
	if (not(authorised("DATASET COPY", msg, tt))) {
		{}
	}
	if (not(authorised("%RENAME%" "AUTHORISATION UPDATE KEYS", msg, "AUTHORISATION UPDATE GROUPS"))) {
		{}
	}
	if (not(authorised("AUTHORISATION UPDATE LOCKS", msg, tt))) {
		{}
	}
	if (tt == "GS") {
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
	if (not(authorised("CURRENCY UPDATE DECIMALS", msg, "NEOSYS"))) {
		{}
	}
	if (not(authorised("MENU SUPPORT", msg, "SUPPORT MENU ACCESS"))) {
		{}
	}
	if (not(authorised("%DELETE%" "SUPPORT MENU ACCESS", xx))) {
		{}
	}
	if (not(authorised("UPLOAD CREATE", xx))) {
		{}
	}
	if (not(authorised("REQUESTLOG ACCESS", msg, "MENU SUPPORT"))) {
		{}
	}

	if (not(authorised("%RENAME%" "AUTHORISATION CREATE USERS", xx, "USER CREATE"))) {
		{}
	}
	if (not(authorised("%RENAME%" "AUTHORISATION DELETE USERS", xx, "USER DELETE"))) {
		{}
	}
	//if security('%UPDATE%':'USER UPDATE',xx,'AUTHORISATION UPDATE') else null
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
		var usercodes = SECURITY.a(1);
		var nusers = usercodes.count(VM) + (usercodes ne "");
		for (var usern = 1; usern <= nusers; ++usern) {
			var userx = usercodes.a(1, usern);
			if (not(userx.index("---"))) {
				userx.writev(users, userx, 1);
				
			}
		};//usern;

	}

	call log2("*terminate old session", logtime);
	if (sessionid ne "") {
		call logprocess(sessionid, "LOGOFF", "", "", "", "");
		SYSTEM.r(4, "");
	}

	call log2("*get user name", logtime);
	//prevent F5
	if (not resetting) {
		ENVIRONSET.r(38, "");
		call neosyslogin("INIT");
	}

	//moved up before any updating
	//call log2('*init.general obtaining exclusive access:',logtime)

	call log2("*start new session", logtime);
	sessionid = "";
	call logprocess(sessionid, "LOGIN", "", "", "", "");
	SYSTEM.r(4, sessionid);

	if (not(SYSTEM.a(22).length())) {
		SYSTEM.r(22, 300);
	}

	call log2("*suppress background indexing if time out is active", logtime);
	if (SYSTEM.a(22)) {
		INDEXTIME = 0;
	}

	call log2("*suppress words not to be indexed", logtime);
	//see ED SYSTEM * ... ENVIRONMENT words like LTD
	DEFAULTSTOPS = "";

	//this may not be necessary now that menus are determined by authorisation
	call log2("*get maintenance user menu if any", logtime);
	if (xx.open("SCHEDULES", "")) {
		temp = "ADAGENCY";
	}else{
		temp = "NEOSYS2";
	}
	ENVIRONSET.r(37, temp);

	call log2("*force the F1 key to be general help (Ctrl+F1) instead of context help", logtime);
	PRIORITYINT.r(1, var().chr(0) ^ ";");

	if (authorised("USE SPECIAL KEYS", msg, "")) {
		INTCONST.r(4, var().chr(0) ^ var("41").iconv("HEX"));
		INTCONST.r(18, var().chr(0) ^ var("1F").iconv("HEX"));
		INTCONST.r(26, var().chr(0) ^ var("19").iconv("HEX"));
		PRIORITYINT.r(7, var("1F").iconv("HEX"));
		MOVEKEYS.r(27, var("14").iconv("HEX"));
		MOVEKEYS.r(25, var("05").iconv("HEX"));
		//break key on
	}else{
		INTCONST.r(4, "");
		INTCONST.r(18, "");
		INTCONST.r(26, "");
		PRIORITYINT.r(7, "");
		MOVEKEYS.r(27, "");
		MOVEKEYS.r(25, "");
		//break key off
	}

	call log2("*allow/disallow F5", logtime);
	if (authorised("USE TCL COMMAND KEY F5", msg, "")) {
		ENVIRONSET.r(38, 1);
		//@priority.int<2>=char(0):iconv('3F','HEX')
	}else{
		ENVIRONSET.r(38, "");
		//@priority.int<2>=''
	}

	if (VOLUMES) {
		perform("ADDMFS SHADOW.MFS FILEORDER.COMMON");
	}

	call log2("*open general files", logtime);
	var valid = 1;
	//DEFINITIONS='' why was this commented out?
	if (not(openfile("ALANGUAGE", fin.alanguage, "DEFINITIONS"))) {
		valid = 0;
	}
	if (not(openfile("COMPANIES", gen.companies, "DEFINITIONS"))) {
		valid = 0;
	}
	if (not(openfile("CURRENCIES", gen.currencies, "COMPANIES"))) {
		valid = 0;
	}
	if (not(openfile("UNITS", gen.units, "CURRENCIES"))) {
		valid = 0;
		gen.units = "";
	}
	if (not(openfile("ADDRESSES", gen.addresses, "COMPANIES"))) {
		valid = 0;
	}
	if (not(openfile("DOCUMENTS", gen.documents, "ADDRESSES", 1))) {
		valid = "";
	}
	if (not(openfile("TIMESHEETS", gen.timesheets, "ADDRESSES", 1))) {
		valid = "";
	}
	if (not(openfile("SHADOW", shadow, "COMPANIES", 1))) {
		valid = "";
	}
	if (not valid) {
		var().chr(7).output();
		if (interactive) {
			call note("DO NOT CONTINUE UNLESS YOU KNOW WHAT YOU ARE DOING");
		}
	}

	if (not(openfile("CURRENCY_VERSIONS", xx, "CURRENCIES", 1))) {
		valid = "";
	}
	if (xx.open("MARKETS", "")) {
		if (not(openfile("MARKET_VERSIONS", xx, "MARKETS", 1))) {
			valid = "";
		}
	}
	if (not(openfile("COMPANY_VERSIONS", xx, "COMPANIES", 1))) {
		valid = "";
	}

	call log2("*open processes own lists file", logtime);

	var workdir = "NEOS" ^ SYSTEM.a(24).oconv("R(0)#4");
	var workpath = "DATAVOL\\" ^ workdir ^ "\\";

	//check/create folder
	//initdir workpath:'REVMEDIA.*'
	//tt=dirlist()
	tt = oslistf(workpath ^ "REVMEDIA.*");
	if (not tt) {
		osshell("mkdir " ^ workpath);
		perform("NM " ^ workpath ^ " " ^ var().timedate() ^ "(S)");
	}

	//attach folder
	perform("ATTACH " ^ workpath ^ " (S)");

	//check/make LISTS file
	var lists;
	if (not(lists.open("LISTS", ""))) {
		lists = "";
	}
	if (not(lists.index(workpath))) {
		var cmd = "MAKEFILE " ^ workpath ^ " LISTS";
		perform(cmd ^ " (S)");
		if (not(lists.open("LISTS", ""))) {
			lists = "";
		}
		if (not(lists.index(workpath))) {
			//call note('FAILED TO MAKE LISTS FILE ON ':workpath
		}
	}

	call log2("*check lists file exists", logtime);
	if (not(lists.open("LISTS", ""))) {
		var().clearfile(lists);
	}

	//call log2('*check lists file are not corrupted and zero them if they are',logtime)
	//no need since always cleared above now
	////call checkfile'LISTS')

	call log2("*check indexing, locks and processes files are not corrupted and zero them if they are", logtime);
	//call checkfile"!INDEXING");
	//call checkfile"LOCKS");
	//call checkfile"PROCESSES");

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
				perform 'ATTACH ..\DATA\':system<17>:'\GENERAL LOG':year:' (S)';
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
	for (var ii = 1; ii <= 999; ++ii) {
		var filename = filenamesx.field(",", ii);
	///BREAK;
	if (not filename) break;;
		//TODO lock/prevent double create with other processes
		var file;
		if (not(file.open(filename, ""))) {
			perform("MAKEFILE REVBOOT " ^ filename ^ " (S)");
			perform("CONVGLOBAL REVBOOT GLOBAL " ^ filename ^ " (S)");
			perform("DELETEFILE DICT." ^ filename ^ " (S)");
			perform("ATTACH .\\GENERAL DICT." ^ filename ^ " (S)");
			perform("ATTACH REVBOOT " ^ filename ^ " (S)");
			if (not(file.open(filename, ""))) {
				call sysmsg(DQ ^ (filename ^ DQ) ^ " could not be created by INIT.GENERAL");
			}
		}
	};//ii;

	call log2("*perform the autoexec task BEFORE initialising other systems", logtime);
	if (not neosysid) {
		if (temp.read(DEFINITIONS, "AUTOEXEC")) {
			perform("TASK AUTOEXEC");
		}
	}

	call log2("*get first company for init.acc", logtime);
	clearselect();
	select(gen.companies);
	var anyfixed = -1;
fixnextcompany:
	anyfixed += 1;
	if (readnext(companycode)) {
		if (not(gen.company.read(gen.companies, companycode))) {
			goto fixnextcompany;
		}

		var marketcode = gen.company.a(30);
		if (marketcode) {
			var markets;
			if (markets.open("MARKETS", "")) {
				var market;
				if (not(market.read(markets, marketcode))) {
					msg = DQ ^ (marketcode ^ DQ) ^ " is missing from company " ^ companycode;
					call note(msg);
					goto fixcompany;
				}
			}
		}

		call log2("*remove obsolete period 13 from deloitte data", logtime);
		if (gen.company.index("13X4WEEK,1/7,5")) {
			tt = gen.company.a(16);
			tt.swapper("13/", "12/");
			gen.company.r(16, tt);
fixcompany:
			gen.company.write(gen.companies, companycode);
			goto fixnextcompany;
		}

		if (anyfixed) {
			goto fixnextcompany;
		}
	}

	call log2("*open accounts system files", logtime);
	if ((APPLICATION == "ACCOUNTS") or (APPLICATION == "ADAGENCY")) {
		call initacc();
	}
	if (VOLUMES) {
		perform("MACRO ACCOUNTS");
	}

	call log2("*open advertising system files INIT.AGENCY", logtime);
	if (APPLICATION == "ADAGENCY") {
		call initagency();
	}

	call log2("*add number format to company records", logtime);
	clearselect();
	select(gen.companies);
	var numberformat = "";
	fin.currcompany = "";
convcompany:
	if (readnext(fin.currcompany)) {
		var tempcompany;
		if (tempcompany.read(gen.companies, fin.currcompany)) {
			if (tempcompany.a(22) == "") {
				if (not numberformat) {
					call decide("Which format do you want for numbers ?||(See \"NUMBER FORMAT\" on the company file)", "1.000,00 (dot for thousands)" ^ VM ^ "1,000.00 (comma for thousands)", reply);
					if (reply == 1) {
						numberformat = "1.000,00";
					}else{
						numberformat = "1,000.00";
					}
				}
				numberformat.writev(gen.companies, fin.currcompany, 22);
				
				goto convcompany;
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
	call initcompany(fin.currcompany);
	fin.currcompanycodes = fin.currcompany;
	gen.gcurrcompany = fin.currcompany;
	SYSTEM.r(37, gen.gcurrcompany);

	call log2("*check currency accounts", logtime);
	if (xx.open("ACCOUNTS", "")) {
		if (xx.open("ABP", "")) {
			msg = "";
			if (not(fin.account.read(fin.accounts, gen.company.a(4)))) {
				msg.r(-1, gen.company.a(4));
			}
			if (not(fin.account.read(fin.accounts, gen.company.a(5)))) {
				msg.r(-1, gen.company.a(5));
			}
			if (not(fin.account.read(fin.accounts, gen.company.a(12)))) {
				msg.r(-1, gen.company.a(12));
			}
			if (not(fin.account.read(fin.accounts, gen.company.a(19)))) {
				msg.r(-1, gen.company.a(19));
			}
			if (msg) {
				var().chr(7).output();
				msg.swapper(FM, " ");
				if (interactive) {
					call note(DQ ^ (msg ^ DQ) ^ "|account missing - see company setup");
				}
			}
		}
	}

	call log2("*ensure random key exists", logtime);
	var datasetid;
	if (not(datasetid.read(DEFINITIONS, "GLOBALDATASETID"))) {
newdatasetid:
		dostime = ostime();
		datasetid = var().date() ^ "." ^ dostime;
		datasetid.converter(".", "");
		datasetid = datasetid.oconv("MX");
		datasetid = (datasetid ^ datasetid ^ datasetid ^ datasetid).substr(1,8);
adddatasetcodename:
		datasetid.r(2, SYSTEM.a(23));
		datasetid.r(3, SYSTEM.a(17));
		datasetid.write(DEFINITIONS, "GLOBALDATASETID");
	}
	if (datasetid.a(3) == "") {
		goto adddatasetcodename;
	}

	//lock database to particular computers unless logging in as NEOSYS
	//if @username<>'NEOSYS' and datasetid<4> then
	//lock even to NEOSYS to prevent installation where NEOSYS pass is known
	if (datasetid.a(4)) {
		if (not(datasetid.a(4).locateusing(cid(), VM, xx))) {
			USER4 = DQ ^ ("CANNOT USE THIS DATABASE ON THIS COMPUTER" ^ DQ);
			gosub failbatch();
			var().stop();
		}
	}

	//<61>=testdb (not livedb)
	//call readbakpars(bakpars)
	//system<61>=bakpars<11>

	//suggest change globaldatasetid if changed datasetname or datasetid
	if (datasetid.a(2) ne SYSTEM.a(23) or datasetid.a(3) ne SYSTEM.a(17)) {
		//if system<17>[-4,4]<>'TEST' and interactive and @username='NEOSYS' then
		if ((not SYSTEM.a(61) and interactive) and (USERNAME == "NEOSYS")) {
			if (datasetid.a(1) ne "1EEC633B") {
				call decide("This database has been copied or|the database name or code has been changed.|Is this going to be a unique new master database?", "Yes - Going to be a new independent database" ^ VM ^ "No - just backing up, renaming or recoding the database", reply, 2);
				if (reply == 1) {
					goto newdatasetid;
				}
			}
			goto adddatasetcodename;
		}
	}
	SYSTEM.r(45, datasetid.a(1));

	//call log2('*check supported',logtime)
	//if @username<>'NEOSYS' then
	// locate datasetid<1> in '1DFE7C58' setting xx then
	// if index(ucase(system<23>),'QATAR',1) then
	// perform 'OFF'
	// logoff
	// end
	// end
	// end

	//call log2('*take down the backup reminder',logtime)
	//if reminder then call mssg('','DB',reminder,'')

	if (not resetting) {

		call log2("*show and update last login time", logtime);
		var userx;
		if (not(userx.read(DEFINITIONS, "USER*" ^ USERNAME))) {
			userx = "";
		}
		if (userx.a(4) and interactive) {
			var day = var("Mon,Tue,Wed,Thu,Fri,Sat,Sun").field(",", (userx.a(4) - 1) % 7 + 1);
			call note("Info:||" ^ USERNAME ^ " last used " ^ currdataset ^ " on||" ^ day ^ " " ^ userx.a(4).oconv("D") ^ " at " ^ userx.a(5).oconv("MTH") ^ "||" ^ ("on workstation " ^ userx.a(6).trim()).oconv("C#40") ^ "|");
		}

		call log2("*save last login time", logtime);
		userx.write(DEFINITIONS, "USER*" ^ USERNAME ^ "*LAST");

		call log2("*update the last login time", logtime);
		userx.r(4, var().date());
		userx.r(5, var().time());
		userx.r(6, STATION);
		userx.write(DEFINITIONS, "USER*" ^ USERNAME);

		//call log2('*check processes',logtime)
		//clearselect
		//if interactive and ems.allocated then perform 'LISTPROCESSES CHECK'

	}

	call log2("*clean up document keys", logtime);
	if (gen.documents.open("DOCUMENTS", "")) {
		select(gen.documents);
nextdoc:
		if (readnext(docid)) {
			var docid2 = (field2(docid, "\\", -1)).field(".", 1);
			if (docid2 ne docid) {
				var doc;
				if (doc.read(gen.documents, docid)) {
					doc.write(gen.documents, docid2);
					gen.documents.deleterecord(docid);
					
				}
			}
			goto nextdoc;
		}
	}

	if (interactive and (USERNAME == "NEOSYS")) {
		perform("FINDDEADALL");
	}

	if (VOLUMES) {
		if (not(VOLUMES.locateusing("DATAVOL", FM, xx))) {
			//pcperform 'MD DATAVOL'
			//call mkdir('DATAVOL':char(0),xx)
			call osmkdir("DATAVOL");
			perform("NM DATAVOL " ^ var().timedate() ^ "(S)");
			perform("ATTACH DATAVOL (S)");
		}
	}

	//windows stuff
	if (VOLUMES) {

		call log2("*convert codepage", logtime);
		if (codepaging.osread("CODEPAGE.CFG")) {
			var codepage;
			if (not(codepage.read(DEFINITIONS, "PARAM*CODEPAGE"))) {
				codepage = "0" ^ FM ^ codepaging.a(2);
			}
			if (((codepage.a(2) == "737") and not codepage.a(1)) and (codepaging.a(3) == "1253")) {
				perform("CONVGREEK (U)");
			}
		}

		call log2("*installing authorised keys", logtime);
		perform("INSTALLAUTHKEYS (S)");

		call log2("*installing authorised hosts", logtime);
		perform("INSTALLALLOWHOSTS (S)");

		verbs.deleterecord("$FILEMAN");
		

		call log2("*put the admenus program as the system menu file", logtime);
		//as there is no way to have multiple menus files
		if (APPLICATION == "ADAGENCY") {
			perform("SETFILE .\\ADAGENCY GLOBAL ADMENUS SYS.MENUS");
		}

	}

	call log2("*create user name index", logtime);
	var convkey = "CONVERTED*USERNAMEINDEX";
	if (not(xx.read(DEFINITIONS, convkey))) {
		perform("WINDOWSTUB USER.SUBS CREATEUSERNAMEINDEX");
		var().date().write(DEFINITIONS, convkey);
	}

	if (not resetting) {

		call initgeneral2("FIXURLS", logtime);
		call initgeneral2("UPDATEIPNOS4NEOSYS", logtime);
		call initgeneral2("UPDATEUSERS", logtime);
		call initgeneral2("CREATEALERTS", logtime);
		//call init.general2('LASTLOGWARNING':fm:lastlog,logtime)
		call initgeneral2("OSCLEANUP", logtime);

		//per installation or slow things done only in live
		//so assuming test starts first then at least test is running
		//live systems only
		if (not SYSTEM.a(61)) {
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
	if (version.osread("GENERAL\\VERSION.DAT")) {

		//get version installed
		var versioninstalled = version.field("\r", 1).field(0x1A, 1).trim();

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
			call oswrite(upgradelog, "UPGRADE.CFG");

			//upgrade locally installed SYSOBJ files
			call log2("*update sysobj $msg $rtp25", logtime);
			perform("SETFILE REVBOOT SYSPROG,SALADS SYSOBJ SYSOBJ (S)");
			perform("NCOPY GBP $MSG $RTP25 (SON) TO: (SYSOBJ)");

			//make arev dos screen run at full speed-must have been done everywhere now
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
			if (SYSTEM.a(58).locateusing(SYSTEM.a(17), VM, xx)) {
				if (not SYSTEM.a(61)) {
					var idate = version.field(" ", 2, 4).iconv("D");
					var itime = version.field(" ", 1).iconv("MT");
					tt = idate.oconv("D/J") ^ " " ^ itime.oconv("MT");
					call decide("Email users about upgrade?|(or later on do F5 EMAILUSERS UPGRADE " ^ tt ^ ")", "", reply);
					if (reply == 1) {
						perform("EMAILUSERS UPGRADE " ^ tt);
					}
					call sysmsg("NEOSYS Software Upgrade " ^ tt);
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
	call unlockrecord("VERBS", verbs, "INIT.GENERAL.LOGIN");

	if (not resetting) {

		//perform 'REPLICATION UPDATE'

		if (interactive and USERNAME ne "NEOSYS.NET") {
			call log2("*do a few escapes then F10 HOME to initialise the menu system", logtime);
			//do home enter home to open the first menu at the first option
			DATA ^= var().chr(27) ^ var().chr(27) ^ var().chr(27) ^ INTCONST.a(7);
			DATA ^= MOVEKEYS.a(15) ^ var().chr(13) ^ MOVEKEYS.a(15);

			call log2("*if not interactive then start the required process", logtime);
			//by pressing F5

		}else{
			//data char(13)
			//chain 'RUNMENU LISTEN'

			call log2("*indicate success to LOGIN", logtime);
			if (SYSTEM.a(33, 10)) {
				call oswrite("OK", SYSTEM.a(33, 10) ^ ".$2");
			}

			call log2("*chain to NET AUTO (" ^ SYSTEM.a(17) ^ ") INIT.GENERAL Quitting.", logtime);
			var("NET AUTO").chain();
		}

	}

	var().stop();
	return 0;

	return "";
}

subroutine getsystem() {
	call log2("*get " ^ tt ^ " parameters", logtime);
	if (not(systemx.osread(tt))) {
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
		if (systemx.a(ii).length()) {
			SYSTEM.r(ii, systemx.a(ii));
		}
	};//ii;

	//save config file time so can detect if restart required
	SYSTEM.r(100, tt2, tt.osfile().a(3));

	return;

}

subroutine failbatch() {
	msg = USER4;

	tt = "*Authorisation Failure. " ^ msg;
	tt.converter(FM, "|");
	call log2(tt, logtime);

	//onscreen message
	s33 = SYSTEM.a(33);
	SYSTEM.r(33, "");
	call mssg(msg ^ "", "T3", yy, "");
	SYSTEM.r(33, s33);

	//respond to user
	msg.swapper(FM ^ FM, FM);
	msg.converter(FM, "|");
	call oswrite(msg, SYSTEM.a(33, 10) ^ ".$2");

	//and quit
	perform("OFF");
	var().logoff();

	return;

}


libraryexit()
