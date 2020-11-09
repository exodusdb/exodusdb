#include <exodus/library.h>
libraryinit()

#include <listen5.h>
#include <sysmsg.h>
#include <singular.h>
#include <authorised.h>
#include <collectixvals.h>
#include <sortarray.h>
#include <htmllib2.h>
#include <select2.h>
#include <openfile.h>
#include <rtp57.h>
#include <listen4.h>
#include <otherusers.h>
#include <getbackpars.h>
#include <esctoattr.h>
#include <scrnio.h>

#include <gen_common.h>
//#include <fin_common.h>
//#include <agy_common.h>
#include <win_common.h>

var request2;
var request3;
var request4;
var request5;
var request6;
var filename;
var processes;
var tt;
var xx;
var inpath;
var pattern;
var ageinsecs;//num
var ii;//num
var patchfile;
var bottomline;
var stopn;
var locks;
var lockid;
var dostime;//num
var yy;
var zz;
var bakpars;
var msg2;
var fileattributes;
var filetime;

function main(in request1, in request2in, in request3in, in request4in, in request5in, in request6in) {
	//c sys in,in,in,in,in,in

	#include <general_common.h>
//	#include <common.h>
//	#include <agency_common.h>
	//global ii,userencrypt0,passwordexpired,lastlogindate,maxnologindays,validips
	//global filetime,fileattributes

	//TODO share various files with LISTEN to prevent slowing down by opening?

	#define request USER0
	#define iodat USER1
	#define response USER3
	#define msg USER4
	var tracing = 1;

	//no output in arguments allowed since c++ doesnt allow
	// defaulting or value based arguments and setting them
	// and this means calling listen2 would require passing variables for all
	//in the few cases listen2 need to respond to caller, it sets @ans
	if (request2in.unassigned()) {
		request2 = "";
	}else{
		request2 = request2in;
	}
	if (request3in.unassigned()) {
		request3 = "";
	}else{
		request3 = request3in;
	}
	if (request4in.unassigned()) {
		request4 = "";
	}else{
		request4 = request4in;
	}
	if (request5in.unassigned()) {
		request5 = "";
	}else{
		request5 = request5in;
	}
	if (request6in.unassigned()) {
		request6 = "";
	}else{
		request6 = request6in;
	}
	var logx = request2;

	var isdevsys = var("neosys.id").osfile();

	if (request1 == "RUNS") {

		//db start commands
		//initdir '*.RUN'
		//filenamesx=dirlist()
		var filenamesx = oslistf("*.run");

		for (var filen = 1; filen <= 9999; ++filen) {
			filename = filenamesx.a(filen);
			///BREAK;
			if (not filename) break;
			if (lockrecord("PROCESSES", processes, "START*" ^ filename)) {
				if (tt.osread(filename)) {
					tt.converter("\r\n", FM);
					//dont start if there is a database stop command
					if (not((tt.a(1).lcase() ^ ".end").osfile())) {
						if (tt.a(5)) {
							//garbagecollect;
							tt = "CMD /C START NEOSYS.JS /system " ^ tt.a(5) ^ " /database " ^ tt.a(1) ^ " /pid " ^ tt.a(6);
							printl(var().at(0), var().at(-4), var().time().oconv("MTS"), " ", tt);
							tt.osshell();
						}
						filename.osdelete();
					}
				}
				xx = unlockrecord("", processes, "START*" ^ filename);
			}
		};//filen;

	} else if (request1 == "DELETEOLDFILES2") {

		inpath = request3;
		tt = "\\/";
		inpath.converter(tt, OSSLASH OSSLASH);
		//delete old response and temp files every 1 minute
		call listen5("DELETEOLDFILES", "*.4", inpath);
		call listen5("DELETEOLDFILES", "*.5", inpath);
		//call listen5('DELETEOLDFILES','*.$2','./',tt,yy,zz)
		//shell2 cannot/does not delete its tempfiles due to wget remaining in background
		call listen5("DELETEOLDFILES", "vdm*.tmp", "./", 60);
		call listen5("DELETEOLDFILES", "*.$$*", "./", 60);

	} else if (request1 == "DELETEOLDFILES") {

		pattern = request2;
		inpath = request3;

		//delete files older than x
		ageinsecs = request4;
		if (not ageinsecs) {
			ageinsecs = SYSTEM.a(28);
		}
		if (ageinsecs == "") {
			ageinsecs = 60 * 60;
		}

		gosub deleteoldfiles();

	} else if (request1 == "CHECKRESTART") {

		//check for corruption in system record
		//if index(system,char(0),1) then
		if (SYSTEM.index(var().chr(0))) {
			var(SYSTEM).oswrite("system.bad");
			call sysmsg("Corrupt SYSTEM record in LISTEN - RESTARTING");
			ANS = "CORRUPTSYSTEM";
			SYSTEM.converter(var().chr(0), "");
restart:
			ANS = "RESTART " ^ ANS;
			return 0;
		}

		//detect system parameter changes and restart
		//this has the effect of detecting corruption in system which inserts lines
		var s100 = SYSTEM.a(100);
		for (ii = 1; ii <= 2; ++ii) {
			//order is significant
			tt = var("../../system.cfg,system.cfg").field(",", ii);
			tt.converter("/", OSSLASH);
			var t2 = tt.osfile().a(3);
			var t3 = s100.a(1, ii);
			if (t2 ne t3) {
				if (t3) {
					ANS = tt;
					goto restart;
				}else{
					SYSTEM.r(100, ii, t2);
				}
			}
		};//ii;

		//check for upgrade to LISTEN
		var gbp;
		if (gbp.open("GBP", "")) {
			var listen;
			if (listen.read(gbp, "$LISTEN")) {
				listen = field2(listen, FM, -1);
				if (s100.a(1, 3)) {
					if (s100.a(1, 3) ne listen) {
						ANS = "$LISTEN";
						goto restart;
					}
						//comment to solve c++ decomp problem
				}else{
					SYSTEM.r(100, 3, listen);
				}
			}
		}
		ANS = "";
		return 0;

	} else if (request1 == "PATCHANDRUNONCE") {

		//never patch and run on development systems (therefore can only test elsewhere)
		//or on test systems which can be patched via dataset.1 if needed
		//TODO work out a way to ensure both live and test programs are updated
		if (SYSTEM.a(61) or isdevsys) {
nopatch:
			ANS = "";
			return 0;
		}

		//1. patchcode=PATCH - cannot be used to patch data (only system)
		// since it may be picked up by any databases listening process
		//2. patchcode=databasecode - can be used to patch one database (and system)
		//3. no way to patch all databases datafiles
		var patchcode = request2;
		processes = request3;

		//if patch appears then install it
		var patchfilename = patchcode ^ ".1";
		var patchfiledir = patchfilename.osfile();
		if (not patchfiledir) {
			goto nopatch;
		}

		//if patching blocked (eg failed to delete last time) then also quit
		var blockpatchfilename = patchfilename;
		blockpatchfilename.splicer(-1, 1, "X");
		if (blockpatchfilename.osfile()) {
			goto nopatch;
		}

		//ensure patch file is complete
		if (not(patchfile.osopen(patchfilename))) {
			goto nopatch;
		}

		tt = patchfilename.osfile().a(1) - 18;
		//osbread tt from patchfile at tt length 18
		call osbread(tt, patchfile,  tt, 18);
		if (tt ne ("!" ^ FM ^ "!END!OF!INSTALL!")) {
			goto nopatch;
		}

		//installation wide lock on it
		if (not(lockrecord("", processes, patchfilename))) {
			goto nopatch;
		}

		//ensure that we only ever runonce something just loaded from a patch
		var runoncekey = "$" ^ patchcode ^ ".RUNONCE";
		DEFINITIONS.deleterecord(runoncekey);

		//indicate patched/may need restart
		ANS = 1;

		if (not isdevsys) {

			var cmd = "INSTALL " ^ patchcode ^ " " ^ oscwd().substr(1,2) ^ " (IO)";
			printl(cmd);
			perform(cmd);

			//17/12/2009
			//tt='Size:':patchfiledir<1>:' ':patchfiledir<2> '[DATE,4*]':' ':patchfiledir<3> 'MTS'
			//call sysmsg(cmd:fm:tt)

		}

		//prevent it being installed again
		(patchfilename ^ "O").osdelete();
		("CMD /C REN " ^ patchfilename ^ " " ^ patchfilename ^ "O").osshell();
		patchfilename.osdelete();

		//if cannot delete then put a blocker on it
		if (patchfilename.osfile()) {
			var(var().date() ^ FM ^ var().time()).oswrite(blockpatchfilename);
		}

		//if $PATCH.RUNONCE or $datasetcode.RUNONCE appears in definitions
		//if the runonce record appears in the definitions then
		//run it, save it and delete it
		var runonce;
		if (runonce.read(DEFINITIONS, runoncekey)) {
			if (not isdevsys) {
				perform("RUN DEFINITIONS " ^ runoncekey.substr(2,9999));
			}
			runonce.write(DEFINITIONS, runoncekey ^ "*LAST");
			DEFINITIONS.deleterecord(runoncekey);

		}
		runonce = "";

		//trigger other processes to restart by updating SYSTEM.CFG
		if (tt.osread("system.cfg")) {
			var(tt).oswrite("system.cfg");
		}

		//release
		call unlockrecord("", processes, patchfilename);

		//indicate patches applied and may need restart
		ANS = 1;
		return 0;

	} else if (request1 == "CONVLOG") {

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

	} else if (request1 == "PROCESSINIT") {

		//put username and database on the bottom line of the screen
		var username = USERNAME.trim();
	//username=rnd(1000000)
		var s23 = SYSTEM.a(23);
		var s17 = SYSTEM.a(17);
		s23.converter(" ", FM);
		//locate 'TEST' in s23 setting xx then username:='*' else
		if (s23.locate("TESTDATA",xx)) {
			username ^= "*";
		}else{
			if (s23.locate("TESTING",xx)) {
				username ^= "*";
			}else{
				if (s23.locate("TRAINING",xx)) {
					username ^= "*";
				}else{
					if (s17.index("TEST")) {
						username ^= "*";
					}
				}
			}
		}
		// end
		bottomline = (s17 ^ " " ^ SYSTEM.a(24)).oconv("L#40") ^ username.oconv("R#40");
		gosub printbottomline();

	} else if (request1 == "PROCESSEXIT") {
		bottomline = var(80).space();
		gosub printbottomline();

	} else if (request1.substr(1,14) == "GETINDEXVALUES") {

		USER1 = "";
		filename = request2;
		var fieldname = request3;
		var prefix = request4;
		var sortby = request5;
		if (not sortby) {
			sortby = "AL";
		}
		if (sortby) {
			if (not(var("AL,AR,DL,DR").locateusing(",",sortby,xx))) {
				USER3 = "Invalid sortby " ^ (sortby.quote()) ^ " in LISTEN,GETINDEXVALUES";
				return 0;
			}
		}
		var active = request6;

		if (filename == "BATCHES") {
		}else{
			gosub fileaccesscheck();
			if (USER3) {
				return 0;
			}
		}

		//security check
		var temp = filename;

		//zzz
		if (temp == "BATCHES") {
			temp = "JOURNAL";
		}

		if (not(authorised(singular(temp) ^ " LIST", USER4, ""))) {
			USER3 = USER4;
			return 0;
		}

getvalues:
		call collectixvals(filename, fieldname, prefix);
		PSEUDO.transfer(USER1);
		if (USER1[1] == FM) {
			USER1.splicer(1, 1, "");
		}
		USER3 = "OK";

		if (sortby and USER1) {
			//convert fm to rm in iodat
			//iodat:=rm
			//call v119('S','',sortby[1,1],sortby[2,1],iodat,flag)
			//convert rm to fm in iodat
			//iodat[-1,1]=''
			//v119 C/ASM sort routine cannot be easily converted/reimplemented in c++
			USER1.converter(FM, VM);
			call sortarray(USER1, 1, sortby);
			USER1.converter(VM, FM);
		}

		var execstoplist;
		if (execstoplist.read(DEFINITIONS, "INDEXVALUES*" ^ filename ^ "*" ^ fieldname)) {

			//execs will be in field 1
			//stopped reason will be in parallel in field 2
			//stopped execs will be at the end
			USER1.converter(FM, VM);

			//remove or move any stopped execs to the end and add reason
			var nn = USER1.count(VM) + 1;
			var nn2 = nn;
			for (ii = 1; ii <= nn; ++ii) {
				var execcode = USER1.a(1, ii);
				if (execstoplist.a(1).locate(execcode,stopn)) {
					var reason = execstoplist.a(2, stopn);
					if (reason) {
						USER1.remover(1, ii);
						USER1.remover(2, ii);
						if (not active) {
							USER1.r(1, nn2, execcode);
							USER1.r(2, nn2, reason);
						}
						ii -= 1;
						nn -= 1;
					}
				}
			};//ii;

			//show inactive if none are active
			if ((USER1 == "") and active) {
				active = "";
				goto getvalues;
			}

			//1=force vm to ensure xml has empty not missing tags
			USER1.r(2, 1, USER1.a(2, 1));
			USER1 = invertarray(USER1, 1);

		}

		if (USER0.index("RECORD")) {
			USER1 = invertarray(USER1, 1);

		} else if (USER0.index("XML")) {
			if (USER1) {
				call htmllib2("STRIPTAGS", USER1);
				USER1.swapper(FM, "</" ^ fieldname ^ ">" "</record>" "\r\n" "<record><" ^ fieldname ^ ">");
				USER1.splicer(1, 0, "<record><" ^ fieldname ^ ">");
				USER1 ^= "</" ^ fieldname ^ ">" "</record>";
				if (USER1.index(VM)) {
					USER1.swapper("</" ^ fieldname ^ ">", "</STOPPED>");
					USER1.swapper(VM, "</" ^ fieldname ^ ">" "<STOPPED>");
				}
			}
			USER1.splicer(1, 0, "<records>");
			USER1 ^= "</records>";
		} else {
			//convert fm to vm in iodat
		}

	//also called from financeproxy to return voucher details for popup
	} else if (request1 == "SELECT") {

		var filename0 = request2;
		filename = filename0.field(" ", 1);
		var sortselect = request3;
		var dictids = request4;
		var options = request5;
		var maxnrecs = request6;

		if (not(sortselect.index("%SELECTLIST%"))) {
			clearselect();
		}

		var file;
		if (not(file.open(filename, ""))) {
			USER3 = filename.quote() ^ " file does not exist in LISTEN SELECT";
			return 0;
		}

		gosub fileaccesscheck();
		if (USER3) {
			return 0;
		}

		//get the current program stack
		//programstack=program.stack(nostack)

		//and data passed to SELECT is assumed to be a selectlist

		if (USER1) {
			makelist("",USER1);
			sortselect ^= "%SELECTLIST%";
			USER1 = "";
		}

		call select2(filename0, SYSTEM.a(2), sortselect, dictids, options, USER1, USER3, "", "", "", maxnrecs);
		//restore the program stack although this is done in LISTEN per request
		//arev has a limit on 299 "programs" and dictionary entries count as 1 each!
		//call program.stack(programstack)

		if (USER4) {
			USER3 = trim(USER4.a(1), FM);
		}else{
			USER1 = "%DIRECTOUTPUT%";
			//response='OK'
		}

	} else if (request1 == "LISTLOCKS") {

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
				if (lockx.a(1) < dostime) {
					goto nextlock;
				}
				nlocks += 1;
				select2data.r(nlocks, 1, lockx.a(4) ^ VM ^ lockx.a(3) ^ VM ^ lockid.field("*", 1) ^ VM ^ lockid.field("*", 2, 999));
				goto nextlock;
			}
		}

		if (tracing) {
			printl();
		}else{
			print(var().at(0, 1));
		}
		var nn = select2data.count(FM);
		var maxlines = 20;
		if (not tracing) {
			nn = maxlines;
			if (nn > maxlines) {
				nn = maxlines;
			}
			printl("+------Active-------+");
		}
		if (tracing) {
			printl(var("User").oconv("L#19"), " ", var("Station").oconv("L#19"), " ", var("File").oconv("L#19"), " ", var("Record").oconv("L#19"));
		}
		for (ii = 1; ii <= nn; ++ii) {
			///BREAK;
			if (tracing and (nn > 20)) break;
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
		};//ii;

		if (not tracing) {
			printl("+-------------------+");
		}

		select2data = "";
		var select2response = "";

	} else if (request1 == "UNLOCKLONGPROCESS") {

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
			SYSTEM.r(48, "");
		}

	} else if (request1 == "STOPDB") {

		#define installend request3
		#define serverend request4

		//check authorised
		tt = request2;
		if (not tt) {
			tt = "STOP";
		}
		if (not(authorised("DATABASE " ^ tt, USER4, "LS"))) {
			USER3 = "Error: " ^ USER4;
			return 0;
		}

		if (var(installend).osfile() or var(serverend).osfile()) {
			//response='Error: Database already stopped/stopping'
			call listen4(19, USER3);

		}else{

			call oswrite("", installend);

			//stop server
			if (request2.index("ALL")) {
				call oswrite("", serverend);
			}

			//wait up to 30 seconds for other users to quit
			var timex = var().time();
			while (true) {
				///BREAK;
				if (not(otherusers().a(1) and ((var().time() - timex).abs() < 30))) break;
				call ossleep(1000*1);
			}//loop;

			USER1 = "";

			var otherusersx = otherusers();
			if (otherusersx) {
				//response='Error: Could not terminate ':otherusersx<1>:' processes|':otherusersx<2>
				call listen4(20, USER3, otherusersx);
				var(installend).osdelete();
			}else{
				osshell("NET STOP NEOSYSSERVICE");

				if (request2.substr(1,7) == "RESTART") {
					var(installend).osdelete();
					osshell("NET START NEOSYSSERVICE");
				}

				USER3 = "OK";
			}

			if (request2.index("ALL")) {
				var(serverend).osdelete();
			}
		}

	} else if (request1 == "BACKUP") {

		//similar code in LISTEN and LISTEN2

		//gosub getbakpars
		call getbackpars(bakpars);

		//backup may respond to user itself if it starts
		USER4 = "";
		perform("FILEMAN BACKUP " ^ SYSTEM.a(17) ^ " " ^ bakpars.a(7));

		//if backup has already responded to user
		//then quit and indicate to calling program that a backup has been done
		//user will be emailed
		if (SYSTEM.a(2) == "") {
			PSEUDO = "BACKUP2 " ^ bakpars.a(7);
			if (USER4) {
				stop();
			}
		}

		USER3 = USER4;
		USER3.converter(FM ^ VM, "\r\r");

		call sysmsg(USER3, "NEOSYS Backup");

		if (USER3.ucase().index("SUCCESS")) {
			USER3.splicer(1, 0, "OK ");
		}

		//note: if backup did not respond already then the requestexit will
		//respond as usual with the error message from backup
		USER1 = "";

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

subroutine fileaccesscheck() {
	USER3 = "";

	var securityfilename = filename;
	if (filename == "BATCHES") {
		securityfilename = "JOURNALS";
	}

	//security check
	//dont check markets and companies because really must have access to some
	//and eliminates need for clumsy task COMPANY ACCESS PARTIAL task
	if (not(var("MENUS,ADMENUS").locateusing(",",filename,xx))) {
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
					USER3 = USER4;
					return;
				}
			}
		}
		USER4 = "";
	}

	return;
}

subroutine printbottomline() {
	if (CRTHIGH > 24) {
		yy = CRTHIGH - 1;
	}else{
		yy = CRTHIGH;
	}
	call scrnio(0, yy, bottomline.substr(1,80), esctoattr(ENVIRONSET.a(21)));
	return;
}

subroutine deleteoldfiles() {

	var deletetime = var().date() * 24 * 60 * 60 + var().time() - ageinsecs;

	var filespec = (inpath ^ pattern).lcase();

	//failsafe - only allow delete .* in data folder
	if (filespec.substr(-2,2) == ".*") {
		var tdir = "/data/";
		tdir.converter("/", OSSLASH);
		if (not(filespec.index(tdir))) {
			return;
		}
	}

	//for each suitable file
	//initdir filespec
	//filenamesx=dirlist()
	var filenamesx = oslistf(filespec);

	while (true) {
		///BREAK;
		if (not filenamesx) break;

		//get the file time
		var filename0 = filenamesx.field(FM, 1);
		filenamesx.splicer(1, filenamesx.field(FM, 1).length() + 1, "");
		if (not filenamesx) {
			filenamesx = var().oslistf();
		}

		filename = inpath ^ filename0;

		//replaced by databasecode.SVR
		//if filename0='GLOBAL.SVR' then goto deleteit

		if (not(var(".jpg,.png,.gif,.svr").locateusing(",",(filename.substr(-4,4)).lcase(),xx))) {

			//a file ending .4 is a request to delete the .2 and .3 files
			if (filename.substr(-2,2) == ".4") {
				filename.osdelete();
				filename.splicer(-1, 1, "2");
				filename.osdelete();
				filename.splicer(-1, 1, "3");
				filename.osdelete();

			}else{
				if (filename.substr(-4,4) == ".TMP") {
					goto deleteit;
				}
				//and delete it if older than the cut off time
				//and has a file extension (ie leave PARAMS and PARAMS2)
				fileattributes = filename.osfile();
				filetime = fileattributes.a(2) * 24 * 60 * 60 + fileattributes.a(3);
				if (((filename.substr(-4,4)).index(".")) and (filetime <= deletetime)) {
deleteit:
					filename.osdelete();
				}else{
				}
			}

		}

	}//loop;

	return;
}

libraryexit()
