#include <exodus/library.h>
libraryinit()

#include <listen5.h>
#include <sysmsg.h>
#include <singular.h>
#include <authorised.h>
#include <collectixvals.h>
#include <htmllib2.h>
#include <select2.h>
#include <openfile.h>
#include <rtp57.h>
#include <listen4.h>
#include <otherusers.h>
#include <getbackpars.h>
#include <esctoattr.h>
#include <scrnio.h>

#include <sys_common.h>
#include <win_common.h>

var initdir;
var request2;
var request3;
var request4;
var request5;
var request6;
//var filenamesx;
//var filename;
var processes;
var tt;
var xx;
var inpath;
var pattern;
var ageinsecs;//num
var ii;//num
var patchfile;
var offset;
var firstblock;
var patchid;
var keyandrec;
var rec;
var versiondatetime;
var versionkey;
var patchdatetime;
var skipreason;
var skipemail;//num
var lastpatchid;
var runoncekey;
var runonce;
var bottomline;
var stopn;
var locks;
var lockid;
var dostime;//num
var yy;
var zz;
var serverend;
var bakpars;
var msg2;
var fileattributes;
var filetime;
var offset_zero;

function main(in request1, in request2in, in request3in, in request4in, in request5in, in request6in) {
	//c sys in,in,in,in,in,in

	#include <system_common.h>
	//$insert abp,common
	//$insert bp,agency.common
	//global ii,passwordexpired,lastlogindate,maxnologindays,validips
	//global filetime,fileattributes,initdir
	//global runonce,runoncekey,lastpatchid,skipemail,skipreason,patchdatetime,versionkey,versiondatetime,rec,keyandrec,patchid,offset

	//TODO share various files with LISTEN to prevent slowing down by opening?

	#define request_ USER0
	#define iodat_ USER1
	#define response_ USER3
	#define msg_ USER4
	var tracing = 1;
	initdir = "";

	//no output in arguments allowed since c++ doesnt allow
	// defaulting or value based arguments and setting them
	// and this means calling listen2 would require passing variables for all
	//in the few cases listen2 need to respond to caller, it sets @ans
	if (request2in.unassigned()) {
		request2 = "";
	} else {
		request2 = request2in;
	}
	if (request3in.unassigned()) {
		request3 = "";
	} else {
		request3 = request3in;
	}
	if (request4in.unassigned()) {
		request4 = "";
	} else {
		request4 = request4in;
	}
	if (request5in.unassigned()) {
		request5 = "";
	} else {
		request5 = request5in;
	}
	if (request6in.unassigned()) {
		request6 = "";
	} else {
		request6 = request6in;
	}
	var logx = request2;

	var isdevsys = var("exodus.id").osfile();

	if (request1 eq "RUNS") {

		//db start commands from xhttp.php
		var filenamesx = oslistf("*.run");

		//for (const var filen : range(1, 9999)) {
		for (var filename : filenamesx) {
			//filename = filenamesx.a(filen);
			///BREAK;
			if (not filename) break;
			if (lockrecord("PROCESSES", processes, "START*" ^ filename)) {
				if (tt.osread(filename)) {
					tt.swapper("\r\n", FM);
					tt.converter("\r\n", FM ^ FM);
					//dont start if there is a database stop command
					if (not((tt.a(1).lcase() ^ ".end").osfile())) {
						if (tt.a(5)) {
							//var cmd = "CMD /C START EXODUS.JS /system " ^ tt.a(5) ^ " /database " ^ tt.a(1) ^ " /pid " ^ tt.a(6);

							//TRACE: tt="tamra2^EXODUS^^^ADAGENCY^/root/hosts/tamra/data/tamra2/~4025798^"
							//TRACE: osgetenv("EXO_SERVICE_CODE")="agy_live@tamra"

							//var application = tt.a(5).substr(1,3).lcase();
							var app_code = osgetenv("EXO_SERVICE_CODE").field("_",1);
							if (not app_code)
								app_code = osgetenv("APP_CODE");

							var database = tt.a(1);
							var mode="live";
							if (database.ends("_test")) {
								mode = "test";
								database.splicer(-5, "");
							}

							var cmd = "systemctl start " ^ app_code ^ "_" ^ mode ^ "@" ^ database;
							//printl(AT(-40), var().time().oconv("MTS"), " ", tt);
							if (TERMINAL)
								print(AT(-40));
							printl(var().time().oconv("MTS"), " ", cmd);

							cmd.osshell();
						}
						filename.osremove();
					}
				}
				xx = unlockrecord("", processes, "START*" ^ filename);
			}
		} //filen;

	} else if (request1 eq "DELETEOLDFILES2") {

		inpath = request3;
		tt = "\\/";
		inpath.converter(tt, OSSLASH_ OSSLASH_);
		//delete old response and temp files every 1 minute
		call listen5("DELETEOLDFILES", "*.4", inpath);
		call listen5("DELETEOLDFILES", "*.5", inpath);
		//call listen5('DELETEOLDFILES','*.$2','./',tt,yy,zz)
		//shell2 cannot/does not delete its tempfiles due to wget remaining in background
		call listen5("DELETEOLDFILES", "vdm*.tmp", "./", 60);
		call listen5("DELETEOLDFILES", "*.$$*", "./", 60);

	} else if (request1 eq "DELETEOLDFILES") {

		pattern = request2;
		inpath = request3;

		//delete files older than x
		ageinsecs = request4;
		if (not ageinsecs) {
			ageinsecs = SYSTEM.a(28);
		}
		if (ageinsecs eq "") {
			ageinsecs = 60 * 60;
		}

		gosub deleteoldfiles();

	} else if (request1 eq "CHECKRESTART") {

		//check for corruption in system record
		//if index(system,char(0),1) then
		if (SYSTEM.index(var().chr(0))) {
			var(SYSTEM).oswrite("system.bad");
			call sysmsg("Corrupt SYSTEM record in LISTEN - RESTARTING");
			ANS = "CORRUPTSYSTEM";
			SYSTEM.converter(var().chr(0), "");
			ANS = "RESTART " ^ ANS;
			return 0;
		}

		//detect system parameter changes and restart
		//this has the effect of detecting corruption in system which inserts lines
		var s100 = SYSTEM.a(100);
		var exohome=osgetenv("EXO_HOME");
		var ospaths = "../../system.cfg,system.cfg";
		ospaths ^= "," ^ exohome ^ "/dat/";
		ospaths.converter("/", OSSLASH);
		var npaths = dcount(ospaths, ",");
		for (const var ii : range(1, npaths)) {
			var ospath = ospaths.field(",",ii);
			//order is significant
			var newtime = (ospath[-1] eq OSSLASH) ? ospath.osdir().a(3) : ospath.osfile().a(3);
			var oldtime = s100.a(1, ii);
			if (newtime ne oldtime) {
				if (oldtime) {
					ANS = "RESTART " ^ ospath;
					return 0;
				} else {
					SYSTEM(100, ii) = newtime;
				}
			}
		} //ii;

		//check for upgrade to LISTEN
		var gbp;
		if (gbp.open("GBP", "")) {
			var listen;
			if (listen.read(gbp, "$LISTEN")) {
				listen = field2(listen, FM, -1);
				if (s100.a(1, 3)) {
					if (s100.a(1, 3) ne listen) {
						//this will only restart listen, not the whole process/thread
						ANS = "RESTART $LISTEN";
						return 0;
					}
						//comment to solve c++ decomp problem
				} else {
					SYSTEM(100, 3) = listen;
				}
			}
		}

		ANS = "";
		return 0;

	} else if (request1 eq "PATCHANDRUNONCE") {

		//never patch and run on development systems (therefore can only test elsewhere)
		//OFF while developing this feature
		isdevsys = isdevsys and var().date() gt 19034;
		//if system<61> or isdevsys then
		if (isdevsys or not(VOLUMES)) {
			ANS = "";
			return 0;
		}

		var live = request2;
		processes = request3;

		////////////////////////////////////////////////////////////////////////////
		//Look for NEOPATCH.1 file in three locations
		////////////////////////////////////////////////////////////////////////////
		//The file must have been created after the exodus version date
		// and after the last patch date if any
		////////////////////////////////////////////////////////////////////////////
		//1. DATA\CLIENTX\NEOPATCH.1
		//               > install in one EXODUS database
		//2. DATA\NEOPATCH.1
		//               > install in all active EXODUS databases in one installation
		//3. D:\NEOPATCH.1 or D:\HOSTS\NEOPATCH.1
		//               > install in all active EXODUS database and installations
		var patchcode = "NEOPATCH";
		var patchdirs = "../DATA/" ^ SYSTEM.a(17) ^ FM ^ "../DATA/" ^ FM ^ "../../";
		patchdirs.converter("/", OSSLASH);

		for (const var patchn : range(1, 3)) {

			//skip if no patch file or not dated today
			var patchfilename = patchdirs.a(patchn) ^ patchcode ^ ".1";
			var patchfileinfo = patchfilename.osfile();
			if (patchfileinfo.a(2) lt var().date()) {
				goto nextpatch;
			}

			//open patch file
			if (not(patchfile.osopen(patchfilename))) {
				goto nextpatch;
			}

			//ensure patch file is complete
			offset = patchfileinfo.a(1) - 18;
			call osbread(tt, patchfile, offset, 18);
			if (tt ne ("!" ^ FM ^ "!END!OF!INSTALL!")) {
				goto nextpatch;
			}

			//verify correct file heading and determine patchid from the file
			offset_zero = 0;
			call osbread(firstblock, patchfile, offset_zero, 65000);
			patchid = firstblock.a(2).substr(6, 9999);
			if (firstblock.a(1) ne "00000DEFINITIONS" or patchid.substr(1, 8) ne "INSTALL*") {
				goto nextpatch;
			}

			//skip if patchid is older than 30 days
			//if field(patchid,'*',3) lt date()-30 then
			// goto nextpatch
			// end

			//extract DEFINITIONS install key and rec from first block
			//rec is "00000DEFINITIONS^00033INSTALL*X*19034*35287^...."
			//remove 00000DEFINITIONS^
			firstblock.remover(1);
			//remove 5 byte length field and cut out key+rec
			keyandrec = firstblock.substr(6, firstblock.substr(1, 5));
			//remove key
			rec = keyandrec.remove(1, 0, 0);

			//installation wide lock on it
			if (not(lockrecord("", processes, patchid))) {
				goto nextpatch;
			}

			//skip if already installed in this database
			if (xx.read(DEFINITIONS, patchid)) {
				call unlockrecord("", processes, patchid);
				goto nextpatch;
			}

			//prevent from ever running this patch again on this database
			rec.write(DEFINITIONS, patchid);

			//get current EXODUS version timestamp
			versiondatetime = "";
			versionkey = "general/version.dat";
			versionkey.converter("/", OSSLASH);
			if (tt.osread(versionkey)) {
				tt.trimmer();
				var vdate = tt.field(" ", 2, 3).iconv("D");
				var vtime = tt.field(" ", 1).iconv("MT");
				if (vdate and vtime) {
					versiondatetime = vdate ^ "." ^ vtime.oconv("R(0)#5");
				}
			}

			//get patch timestamp
			patchdatetime = patchid.field("*", 3);

			//skip patch if older than installation
			skipreason = "";
			skipemail = "";
			if (versiondatetime and patchdatetime lt versiondatetime) {
				skipreason = "Patch is older than installation version - " ^ oconv(versiondatetime, "[DATETIME,4*]");
				skipemail = 1;
			}

			//skip patch if older than last patch
			if (not(lastpatchid.read(DEFINITIONS, "INSTALL*LAST"))) {
				lastpatchid = "";
			}
			if (lastpatchid) {
				var lastpatchdatetime = lastpatchid.field("*", 3);
				if (patchdatetime lt lastpatchdatetime) {
					skipreason = "Patch is older than the last patch - " ^ oconv(lastpatchdatetime, "[DATETIME,4*]");
				}
			}

			//ensure that we only ever runonce something just loaded from a patch
			runoncekey = "$" ^ patchid.field("*", 2) ^ ".RUNONCE";
			DEFINITIONS.deleterecord(runoncekey);

			if (not skipemail) {

				//list the files and records that were installed
				var subject = rec.a(1) ^ " - " ^ patchid.field("*", 2) ^ " " ^ oconv(patchid.field("*", 3), "[DATETIME,4*]");
				var body = subject ^ " " ^ patchfilename ^ FM;
				if (skipreason) {
					body(-1) = FM ^ "NOT PATCHED - " ^ skipreason ^ FM ^ FM;
				}
				var nfiles = rec.a(3).count(VM) + 1;
				for (const var filen : range(1, nfiles)) {
					body(-1) = rec.a(3, filen) ^ " " ^ rec.a(4, filen) ^ "  " ^ rec.a(5, filen);
				} //filen;

				//message EXODUS only
				call sysmsg(body, "NEOPATCH: " ^ subject, "EXODUS");

			}

			if (not skipreason) {

				//perform the installation
				var cmd = "INSTALL " ^ patchcode ^ " " ^ oscwd().substr(1, 2) ^ " (IO)";
				printl(cmd);
				perform(cmd);

			}

			//record success/failure before any autorun
			(var().date() ^ "." ^ var().time().oconv("R(0)#5")).writev(DEFINITIONS, patchid, 6);

			skipreason.writev(DEFINITIONS, patchid, 7);

			if (skipreason) {
				//release
				call unlockrecord("", processes, patchid);
				goto nextpatch;
			}

			//save the last patch info - used to prevent backward patching
			patchid.write(DEFINITIONS, "INSTALL*LAST");

			//post install runonce if installed
			//if $PATCH.RUNONCE or $datasetcode.RUNONCE appears in definitions
			//if the runonce record appears in the definitions then
			//run it, save it and delete it
			if (runonce.read(DEFINITIONS, runoncekey)) {
				perform("RUN DEFINITIONS " ^ runoncekey.substr(2, 9999));
				//leave it for inspection
				//delete definitions,runoncekey
			}

			//trigger other processes to restart by updating SYSTEM.CFG
			if (tt.osread("system.cfg")) {
				var(tt).oswrite("system.cfg");
			}

			//release
			call unlockrecord("", processes, patchid);

			//indicate patches applied and may need restart
			ANS = 1;

			//dont check any other patchdirs
			return 0;

nextpatch:;
		} //patchn;

		ANS = "";

	} else if (request1 eq "CONVLOG") {

		//assumes at least 0-31 and 128-255 encoded like $hh

		//reserve/use special characters for field separators
		logx.swapper("^", "%5E");
		logx.swapper("]", "%5D");
		logx.swapper("\\", "%5C");
		logx.swapper("[", "%5B");
		logx.swapper("%FE", "^");
		logx.swapper("%FD", "]");
		logx.swapper("%FC", "\\");
		logx.swapper("%FB", "[");
		logx.swapper(RM, "%FF");
		//swap fm with '^' in logx
		//swap vm with ']' in logx
		//swap sm with '\' in logx;*backslash
		//swap tm with "[" in logx
		logx.converter(_FM_ _VM_ _SM_ _TM_, "^]\\[");
		//fefdfcfb=char(254):char(253):char(252):char(251)
		//convert fefdfcfb to '^]\[' in logx;*backslash
		logx.swapper(STM, "%FA");

		logx.swapper("%20", " ");
		//does not seem to format in XML
		//swap '%20' with '&nbsp;' in logx

		logx.swapper("&", "&amp;");
		logx.swapper(DQ, "&quot;");
		logx.swapper("<", "&lt;");
		logx.swapper(">", "&gt;");
		logx.swapper("%26", "&amp;");
		logx.swapper("%22", "&quot;");
		logx.swapper("%3C", "&lt;");
		logx.swapper("%3E", "&gt;");

/* obsolete
	} else if (request1 eq "PROCESSINIT") {
		//put username and database on the bottom line of the screen
		var username = USERNAME.trim();
	//username=rnd(1000000)
		var s23 = SYSTEM.a(23);
		var s17 = SYSTEM.a(17);
		s23.converter(" ", FM);
		//locate 'TEST' in s23 setting xx then username:='*' else
		if (s23.locate("TESTDATA", xx)) {
			username ^= "*";
		} else {
			if (s23.locate("TESTING", xx)) {
				username ^= "*";
			} else {
				if (s23.locate("TRAINING", xx)) {
					username ^= "*";
				} else {
					if (s17.index("test")) {
						username ^= "*";
					}
				}
			}
		}
		// end
		bottomline = (s17 ^ " " ^ SYSTEM.a(24)).oconv("L#40") ^ username.oconv("R#40");
		gosub printbottomline();

	} else if (request1 eq "PROCESSEXIT") {
		bottomline = var(80).space();
		gosub printbottomline();
*/
	} else if (request1.substr(1, 8) eq "GETINDEX") {

		iodat_ = "";
		var filename = request2;
		var fieldname = request3;
		var prefix = request4;
		var sortby = request5;
		if (not sortby) {
			sortby = "AL";
		}
		if (sortby) {
			if (not(var("AL,AR,DL,DR").locateusing(",", sortby, xx))) {
				response_ = "Invalid sortby " ^ (sortby.quote()) ^ " in LISTEN,GETINDEXVALUES";
				return 0;
			}
		}
		var active = request6;

		if (not(VOLUMES)) {
			if (filename eq "JOURNALS") {
				filename = "JOURNALS";
			}
		}

		if (filename eq "JOURNALS" or filename eq "JOURNALS") {
		} else {
			gosub fileaccesscheck(filename);
			if (USER3) {
				return 0;
			}
		}

		//security check
		var temp = filename;

		//zzz
		if (temp eq "JOURNALS") {
			temp = "JOURNAL";
		}

		if (not(authorised(singular(temp) ^ " LIST", msg_, ""))) {
			response_ = USER4;
			return 0;
		}

getvalues:
		call collectixvals(filename, fieldname, prefix);
		PSEUDO.transfer(USER1);
		if (iodat_[1] eq FM) {
			USER1.splicer(1, 1, "");
		}
		USER3 = "OK";

		if (sortby and iodat_) {
			//convert fm to rm in iodat
			//iodat:=rm
			//call v119('S','',sortby[1,1],sortby[2,1],iodat,flag)
			//convert rm to fm in iodat
			//iodat[-1,1]=''
			//v119 C/ASM sort routine cannot be easily converted/reimplemented in c++
			USER1.converter(FM, VM);
			call sortarray(iodat_, 1, sortby);
			USER1.converter(VM, FM);
		}

		var execstoplist;
		if (execstoplist.read(DEFINITIONS, "INDEXVALUES*" ^ filename ^ "*" ^ fieldname)) {

			//execs will be in field 1
			//stopped reason will be in parallel in field 2
			//stopped execs will be at the end
			iodat_.converter(FM, VM);

			//remove or move any stopped execs to the end and add reason
			var nn = USER1.count(VM) + 1;
			var nn2 = nn;
			for (ii = 1; ii <= nn; ++ii) {
				var execcode = iodat_.a(1, ii);
				if (execstoplist.a(1).locate(execcode, stopn)) {
					var reason = execstoplist.a(2, stopn);
					if (reason) {
						USER1.remover(1, ii);
						iodat_.remover(2, ii);
						if (not active) {
							USER1(1, nn2) = execcode;
							iodat_(2, nn2) = reason;
						}
						ii -= 1;
						nn -= 1;
					}
				}
			} //ii;

			//show inactive if none are active
			if (USER1 eq "" and active) {
				active = "";
				goto getvalues;
			}

			//1=force vm to ensure xml has empty not missing tags
			iodat_(2, 1) = USER1.a(2, 1);
			iodat_ = invertarray(USER1, 1);

		}

		if (request_.index("RECORD")) {
			iodat_ = invertarray(USER1, 1);

		} else if (USER0.index("XML")) {
			if (iodat_) {
				call htmllib2("STRIPTAGS", USER1);
				iodat_.swapper(FM, "</" ^ fieldname ^ ">" "</record>" "\r\n" "<record><" ^ fieldname ^ ">");
				USER1.splicer(1, 0, "<record><" ^ fieldname ^ ">");
				iodat_ ^= "</" ^ fieldname ^ ">" "</record>";
				if (USER1.index(VM)) {
					iodat_.swapper("</" ^ fieldname ^ ">", "</STOPPED>");
					USER1.swapper(VM, "</" ^ fieldname ^ ">" "<STOPPED>");
				}
			}
			iodat_.splicer(1, 0, "<records>");
			USER1 ^= "</records>";
		} else {
			//convert fm to vm in iodat
		}

	//also called from financeproxy to return voucher details for popup
	} else if (request1 eq "SELECT") {

		var filename0 = request2;
		if (not(VOLUMES)) {
			filename0.converter(".", "_");
			filename0.swapper("MEDIA_TYPE", "JOB_TYPE");
		}
		var filename = filename0.field(" ", 1);
		var sortselect = request3;
		var dictids = request4;
		var options = request5;
		var maxnrecs = request6;

		if (not(sortselect.index("%SELECTLIST%"))) {
			clearselect();
		}

		var file;
		if (not(file.open(filename, ""))) {
			response_ = filename.quote() ^ " file does not exist in LISTEN SELECT";
			return 0;
		}

		gosub fileaccesscheck(filename);
		if (USER3) {
			return 0;
		}

		//get the current program stack
		//programstack=program.stack(nostack)

		//and data passed to SELECT is assumed to be a selectlist

		if (iodat_) {
			makelist("", USER1);
			sortselect ^= "%SELECTLIST%";
			iodat_ = "";
		}

		call select2(filename0, SYSTEM.a(2), sortselect, dictids, options, USER1, response_, "", "", "", maxnrecs);
		//restore the program stack although this is done in LISTEN per request
		//rev has a limit on 299 "programs" and dictionary entries count as 1 each!
		//call program.stack(programstack)

		if (msg_) {
			USER3 = trim(USER4.a(1), FM);
		} else {
			iodat_ = "%DIRECTOUTPUT%";
			//response='OK'
		}

	} else if (request1 eq "LISTLOCKS") {

		gosub getdostime();

		/*;

			msg='';

			filename='LOCKS';
			//sortselect='BY DATETIME_EXPIRES'
			sortselect='BY USER';
			sortselect:=' WITH DATETIME_EXPIRES >= ':dostime;
			dictids='USER STATION FILENAME KEY';
			options='';
			msg='';

			call select2(filename,'',sortselect,dictids,options,select2data,select2response,'','','');

			//abort to net program which will clear the lists file
			if msg then stop;

		*/

		if (not(openfile("LOCKS", locks))) {
			call mssg("LISTEN2 CANNOT OPEN LOCKS FILE");
			return 0;
		}

		select(locks);
		var select2data = "";
		var nlocks = 0;
nextlock:
		if (readnext(lockid)) {
			var lockx;
			if (lockx.read(locks, lockid)) {
				if (lockx.a(1) lt dostime) {
					goto nextlock;
				}
				nlocks += 1;
				select2data(nlocks, 1) = lockx.a(4) ^ VM ^ lockx.a(3) ^ VM ^ lockid.field("*", 1) ^ VM ^ lockid.field("*", 2, 999);
				goto nextlock;
			}
		}

		if (tracing) {
			printl();
		} else {
			print(AT(0, 1));
		}
		var nn = select2data.count(FM);
		var maxlines = 20;
		if (not tracing) {
			nn = maxlines;
			if (nn gt maxlines) {
				nn = maxlines;
			}
			printl("+------Active-------+");
		}
		if (tracing) {
			printl(var("User").oconv("L#19"), " ", var("Station").oconv("L#19"), " ", var("File").oconv("L#19"), " ", var("Record").oconv("L#19"));
		}
		for (ii = 1; ii <= nn; ++ii) {
			///BREAK;
			if (tracing and nn gt 20) break;
			var row = select2data.a(ii);
			if (not tracing) {
				print("|");
			}
			tt = row.a(1, 1).trim();
			if (not tracing) {
				tt ^= " " ^ row.a(1, 3);
			}
			print(tt.oconv("L#19"));
			if (not tracing) {
				printl("|");
			}
			if (tracing) {
				printl(" ", row.a(1, 2).oconv("L#19"), " ", row.a(1, 3).oconv("L#19"), " ", row.a(1, 4).oconv("L#19"));
			}
		} //ii;

		if (not tracing) {
			printl("+-------------------+");
		}

		select2data = "";
		var select2response = "";

	} else if (request1 eq "UNLOCKLONGPROCESS") {

		//IF revRELEASE()>= 2.1 THEN
		// lockmode=36
		// unlockmode=37
		//END ELSE
		var lockmode = 23;
		var unlockmode = 24;
		// END

		//NB ZZZ will hang if is not locked eg via unlock all
		//unlock long process unlimited cpu time flag
		tt = SYSTEM.a(48);
		if (tt) {
			//unlock in request loop using lockkey stored in system<48> by giveway
			//lock first to avoid hanging if try to unlock when not locked
			xx = "";
			call rtp57(lockmode, "", xx, tt, "", yy, zz);
			call rtp57(unlockmode, "", xx, tt, "", yy, zz);
			SYSTEM(48) = "";
		}

	} else if (request1 eq "STOPDB") {

		//equ install.end to request3
		//equ server.end to request4

		//check authorised
		tt = request2;
		if (not tt) {
			tt = "STOP";
		}
		if (not(authorised("DATABASE " ^ tt, msg_, "LS"))) {
			response_ = "Error: " ^ USER4;
			return 0;
		}

		if (request3.osfile() or serverend.osfile()) {
			//response='Error: Database already stopped/stopping'
			call listen4(19, USER3);

		} else {

			call oswrite("", request3);

			//stop server
			if (request2.index("ALL")) {
				call oswrite("", request4);
			}

			//wait up to 30 seconds for other users to quit
			var timex = var().time();
			while (true) {
				///BREAK;
				if (not(otherusers().a(1) and ((var().time() - timex).abs() lt 30))) break;
				call ossleep(1000*1);
			}//loop;

			USER1 = "";

			var otherusersx = otherusers();
			if (otherusersx) {
				//response='Error: Could not terminate ':otherusersx<1>:' processes|':otherusersx<2>
				call listen4(20, response_, otherusersx);
				request3.osremove();
			} else {
				osshell("NET STOP EXODUSSERVICE");

				if (request2.substr(1, 7) eq "RESTART") {
					request3.osremove();
					osshell("NET START EXODUSSERVICE");
				}

				USER3 = "OK";
			}

			if (request2.index("ALL")) {
				request4.osremove();
			}
		}

	} else if (request1 eq "BACKUP") {

		//similar code in LISTEN and LISTEN2

		//gosub getbakpars
		call getbackpars(bakpars);

		//backup may respond to user itself if it starts
		msg_ = "";
		perform("FILEMAN BACKUP " ^ SYSTEM.a(17) ^ " " ^ bakpars.a(7));

		//if backup has already responded to user
		//then quit and indicate to calling program that a backup has been done
		//user will be emailed
		if (SYSTEM.a(2) eq "") {
			PSEUDO = "BACKUP2 " ^ bakpars.a(7);
			if (USER4) {
				stop();
			}
		}

		response_ = msg_;
		USER3.converter(FM ^ VM, "\r\r");

		call sysmsg(response_, "EXODUS Backup");

		if (USER3.ucase().index("SUCCESS")) {
			response_.splicer(1, 0, "OK ");
		}

		//note: if backup did not respond already then the requestexit will
		//respond as usual with the error message from backup
		iodat_ = "";

	} else {
		printl(request1.quote(), " invalid request in LISTEN5");
	}

	return 0;
}

subroutine getdostime() {
	dostime = ostime();
	//convert to Windows based date/time (ndays since 1/1/1900)
	//31/12/67 in rev date() format equals 24837 in windows date format
	dostime = 24837 + var().date() + dostime / 24 / 3600;
	return;
}

subroutine fileaccesscheck(in filename) {
	USER3 = "";

	var securityfilename = filename;
	if (filename eq "JOURNALS") {
		securityfilename = "JOURNALS";
	}

	//security check
	//dont check markets and companies because really must have access to some
	//and eliminates need for clumsy task COMPANY ACCESS PARTIAL task
	if (not(var("MENUS,ADMENUS").locateusing(",", filename, xx))) {
		var temp = securityfilename;
		temp.converter(".", " ");
		temp = singular(temp);
		if (not(authorised(temp ^ " ACCESS", USER4, ""))) {
			//we could use securityfilename LIST instead of securityfilename ACCESS PARTIAL clumsy
			//ie list without general access means there are some records
			//specifically allowed
			if (not(authorised("!#" ^ temp ^ " ACCESS PARTIAL", msg2, ""))) {
				//if there is an authorised dictionary item then leave it up to that
				if (not((var("AUTHORISED").xlate("DICT." ^ filename, 8, "X")).index("ALLOWPARTIALACCESS"))) {
					response_ = msg_;
					return;
				}
			}
		}
		USER4 = "";
	}

	return;
}

subroutine printbottomline() {
	if (CRTHIGH gt 24) {
		yy = CRTHIGH - 1;
	} else {
		yy = CRTHIGH;
	}
	call scrnio(0, yy, bottomline.substr(1, 80), esctoattr(ENVIRONSET.a(21)));
	return;
}

subroutine deleteoldfiles() {

	var deletetime = var().date() * 24 * 60 * 60 + var().time() - ageinsecs;

	var filespec = (inpath ^ pattern).lcase();

	//failsafe - only allow delete .* in data folder
	if (filespec.substr(-2, 2) eq ".*") {
		var tdir = "/data/";
		tdir.converter("/", OSSLASH);
		if (not(filespec.index(tdir))) {
			return;
		}
	}

	//for each suitable file
	var filenamesx = oslistf(filespec);

nextfiles:
	if (filenamesx) {

		//get the file time
		var filename0 = filenamesx.field(FM, 1);
		filenamesx.splicer(1, filenamesx.field(FM, 1).length() + 1, "");

		var filename = inpath ^ filename0;

		//replaced by databasecode.SVR
		//if filename0='GLOBAL.SVR' then goto deleteit

		if (not(var(".jpg,.png,.gif,.svr,.cfg").locateusing(",", (filename.substr(-4, 4)).lcase(), xx))) {

			//a file ending .4 is a request to delete the .2 and .3 files
			if (filename.substr(-2, 2) eq ".4") {
				filename.osremove();
				filename.splicer(-1, 1, "2");
				filename.osremove();
				filename.splicer(-1, 1, "3");
				filename.osremove();

			} else {
				if (filename.substr(-4, 4) eq ".TMP") {
					goto deleteit;
				}
				//and delete it if older than the cut off time
				//and has a file extension (ie leave PARAMS and PARAMS2)
				fileattributes = filename.osfile();
				filetime = fileattributes.a(2) * 24 * 60 * 60 + fileattributes.a(3);
				if (((filename.substr(-4, 4)).index(".")) and filetime le deletetime) {
deleteit:
					filename.osremove();
				} else {
				}
			}

		}

		goto nextfiles;
	}

	return;
}

libraryexit()
