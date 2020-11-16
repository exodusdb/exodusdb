#include <exodus/library.h>
libraryinit()

#include <listen5.h>
#include <openfile.h>
#include <getbackpars.h>
#include <heartbeat.h>
#include <autorun3.h>
#include <monitor2.h>
#include <log2.h>
#include <hexcode.h>
#include <listen4.h>
#include <xmlquote.h>
#include <listen2.h>
#include <log.h>
#include <sysmsg.h>
#include <htmllib2.h>
#include <listen3.h>
#include <singular.h>
#include <systemsubs.h>
#include <generalsubs.h>
#include <authorised.h>
#include <flushindex.h>
#include <rtp57.h>

#include <gen_common.h>
#include <win_common.h>

var logfilename;
var portno;//num
var lastmonitortime;//num
var lastautorun;//num
var halt;//num
var origsentence;
var processes;
var reqlog;
var defaultlockmins;//num
var datasetcode;
var live;
var neopath;
var installend;
var serverend;
var databaseend;
var atcol0;
var clreol;
var allcols;
var maxstrlen;//num
var nblocks;//num
var inblocksize;//num
var outblocksize;
dim datx;
dim hexx;
var ii;//num
var servername;
var onserver;
var request1;
var waitfor;//num
var waitsecs;//num
var sleepms;//num
var serverflagfilename;
var origbatchmode;
var webpath;
var voc;
var inpath;
var listenusername;
var listenstation;
var origscrn;
var origattr;
var tracing;//num
var nrequests;//num
var timeoutsecs;//num
var batchmode;
var origprivilege;
var locks;
var onactive;//num
var bakpars;
var datex;
var logpath;
var tdir;
var xx;
var logfile;
var logptr;//num
var tt;//num
var logx;
var linkfilename1;
var replyfilename;
var stack;
var lastrequestdate;
var lastrequesttime;
var breaktime;
var processno;
var timenow;
var cmd;
var linkfilename0;
var linkfilenames;
var dostime;//num
var now;
var s33;
var charx;
var buffer;
var reply;
var patched;
var dow;
var logtime;
var requestdate;
var requesttime;//num
var nlinkfiles;
var linkfilen;//num
var linkfile1;
var timex;
var ntries;//num
var listenfailure;//num
var connection;
var dataset;
var username;
var password;
var ipno;
var netid;
var firstrequestfieldn;//num
var request2;
var request3;
var request4;
var request5;
var request6;
var responsetime;//num
var rawresponse;
var t2;
var yy;
var req6up;
var anydata;//num
var linkfilename2;
var linkfilename3;
var linkfile2size;
var linkfile2;
var inptr;//num
var blockn;//num
var lendata;//num
var savelogptr;
var invaliduser;
var secs;//num
var iodatlen;
var ptr;//num
var blk;
var filename;
var keyx;
var sessionid;
var lockmins;//num
var masterlock;
var withlock;
var readenv;
var filetitle;
var triggers;
var postread;
var filetitle2;
var secmode;
var ok;//num
var file;
var keyx0;
var preread;
var autokey;//num
var lockkeyx;
var badcomp;
var lockauthorised;//num
var createnotallowed;
var origresponse;
var keyx2;
var storeresponse;
var fieldno;
var prewrite;
var lockkey;
var lockrec;
var lockduration;//num
var dictrec;
var datetimefn;
var olddatetime;
var newdatetime;
var postroutine;//num
var replacewrite;
var postwrite;
var predelete;
var postdelete;
var voccmd;
var printfilename;
var user4x;
var printfile;
var fileerrorx;
var timeouttime;//num
var timeoutdate;//num
var srcfile2;
var newsessionid;
var state;//num
var masterlockkey;
var sublockrec;
var code;//num
var nextbfs;
var handle;
var keyorfilename;
var fmc;//num
var msg0;
var positive;
var posmsg;
var dictfile;
var compcode;
var logid;

function main() {
	//
	//c sys
	#include <general_common.h>
	//global all

	//finance?!
	//$insert abp,common

	/*;
	COMMAND  ARGS                SUCCESS    FAILURE;
	-------  ----                -------    -------;
	LOGIN    user,pass,conninfo  userinfo   various reasons;
	READ     file,key            rec        key doesnt exist;
	READU    file,key            rec,lockid key already locked by someone else;
	WRITE    rec,file,key,lockid rec        wrong lockid, or timestamp differs;
	WRITEU   rec,file,key,lockid (same, but unlocks after writing - rarely used);
	DELETE   file,key            ok         key currently locked by someone else;
	SELECT   select statement    data       no recs found;
	GETINDEX file,field,args     data       no recs found;
	LOCK     file,key            lockid     key already locked by someone else;
	RELOCK   file,key,lockid     ok         wrong lockid;
	UNLOCK   file,key,lockid     ok         wrong lockid;
	EXECUTE  cmd,args,data       data       command chooses to fail;
	*/

	//notes
	//use ABORT or ABORT ALL to terminate EXECUTED programs to clear out program stack

	//ed ub

	//clearcommon();

	logfilename = "";

	portno = 5700;
	if (SYSTEM.a(38)) {
		portno += SYSTEM.a(38) - 1;
	}
	//lastmonitortime=0
	//dont call monitor for approx 60 seconds after startup
	//to allow pressing B for backup and quit without other processes starting up
	lastmonitortime = var().time();
	//dont do autoruns until 1 or 2 mins after starting (to let other processes start)
	lastautorun = var().time() + var(60).rnd();
	//do autorun immediately on dev system
	if (var("exodus.id").osfile()) {
		lastautorun = var().time() - 60;
	}

	//delete any old (5min) login response pid name based files
	call listen5("DELETEOLDFILES", "*.$2", "./", 300, "");

	halt = 0;

	// perform 'ADDMFS SHADOW.MFS FILEORDER.COMMON'

	#define request USER0
	#define iodat USER1
	#define response USER3
	#define msg USER4

	origsentence = SENTENCE;

	//need a file to do central locking not per database
	if (not(processes.open("PROCESSES", ""))) {
		processes = "";
	}
	if (not(reqlog.open("REQUESTLOG", ""))) {
		reqlog = "";
	}

	//lockmins is the number of minutes to retain the lock
	//default to 5 mins. lock extension is done every 5/1.1 mins by the user interface
	defaultlockmins = 5;

	datasetcode = SYSTEM.a(17);
	live = datasetcode.ucase().substr(-4,4) ne "TEST";

	neopath = "../exodus/";
	neopath.converter("/", OSSLASH);
	installend = "global.end";
	serverend = "../../" ^ SYSTEM.a(123).lcase() ^ ".end";
	serverend.converter("/", OSSLASH);
	databaseend = datasetcode.lcase() ^ ".end";
	if (VOLUMES) {
		atcol0 = var().at(0);
		clreol = var().at(-4);
	}else{
		atcol0 = "";
		clreol = "";
	}

	if (not(allcols.open("ACCESSIBLE_COLUMNS", ""))) {
		allcols = "";
	}

	//prepare for sending and receiving escaped iodat > 64Kb
	if (VOLUMES) {
		maxstrlen = 65530;
		nblocks = 4;
		inblocksize = 65500;
		//inblocksize=50000
		outblocksize = (maxstrlen / 3).floor();
		nblocks = 4;
	}else{
		inblocksize = 1048576;
		maxstrlen = inblocksize * 4;
		outblocksize = (maxstrlen / 3).floor();
		nblocks = 1;
	}
	datx.redim(nblocks);
	hexx.redim(256);
	if (VOLUMES) {
		for (ii = 0; ii <= 255; ++ii) {
			hexx(ii) = "%" ^ ii.oconv("MX").oconv("R(0)#2");
		};//ii;
	}

	/////
	//init:
	/////

	//discover the server name
	//servername=field(getdrivepath(drive()[1,2])[3,9999],'/',1)
	servername = "";
	onserver = (servername == "") or (STATION.trim() == servername);

	request1 = "";

	//should be less than 25 unless waiting.exe modified to update the server flag
	waitfor = 1;
	waitsecs = 10;

	//if not on "local" drive C,D or E then longer sleep to save network traffic
	if (onserver) {
		sleepms = 10;
	}else{
		sleepms = 100;
	}

	//make up a serverflagfilename
	serverflagfilename = datasetcode ^ ".svr";

	//intranet=index(origsentence,'INTRANET',1)

	//if @username='EXODUS.NET' then system<33>=1
	origbatchmode = SYSTEM.a(33);

	//webpath=field(origsentence,' ',3)
	webpath = "";
	if (not webpath) {
		webpath = "../" "data/";
	}
	if (webpath[-1] ne "/") {
		webpath ^= "/";
	}
	webpath.converter("/", OSSLASH);
	if (not(voc.open("VOC", ""))) {
		call fsmsg();
		gosub exit();
	}

	inpath = webpath ^ datasetcode ^ "/";
	inpath.converter("/", OSSLASH);

	//set time before calling deleteoldfiles which is time sensitive
	//call settime(webpath[1,2])
	call listen5("DELETEOLDFILES", "*.*", inpath);

	listenusername = USERNAME;
	listenstation = STATION;

	//call savescreenorigscrn, origattr);
	//print @aw<30>:

	//tracing=(@username='EXODUS' or trim(@station)='SBCP1800')
	//tracing=(not(index(origsentence,'AUTO',1)))

	//if @username='DEMO' then tracing=0
	//tracing=0
	tracing = 1;

	//ensure unique sorttempfile
	//if sysvar('SET',192,102,'R':('0000':system<24>)[-5,5]:'.SFX') else null
	//call sysvar_192_102"SET", "R" ^ ("0000" ^ SYSTEM.a(24)).substr(-5,5) ^ ".SFX");

	nrequests = SYSTEM.a(35) + 0;

	printl(var("-").str(79));
	printl("EXODUS.NET SERVICE ", SYSTEM.a(24), " STARTED ", var().timedate());
	printl();
	printl("Station  : ", STATION.oconv("L#25"), "Drive : ", oscwd());

	SYSTEM.r(33, 1);

	timeoutsecs = SYSTEM.a(33, 5);
	if (not(timeoutsecs.isnum())) {
		timeoutsecs = "";
	}
	//if timeoutsecs='' then timeoutsecs=20*60;*ie 20 minutes

	//force into server mode - suppress interactive messages
	batchmode = origbatchmode;
	if (batchmode == "") {
		batchmode = SYSTEM.a(33);
		SYSTEM.r(33, "1");
	}

	origprivilege = PRIVILEGE;
	//call setprivilege"");

	//image=''

	if (not(openfile("LOCKS", locks, "DEFINITIONS", 1))) {
		call fsmsg();
		gosub exit();
	}

	gosub gettimeouttime();

	gosub flagserveractive();

	onactive = 0;

	call getbackpars(bakpars);

	//open an XML log file
	datex = var().date().oconv("D.");
	//if 1 then
	logpath = ("../logs/" ^ datasetcode ^ "/" ^ datex.substr(-4,4)).lcase();
	logpath.converter("/", OSSLASH);

	//check/make the dataset folder
	//subdirs=''
	//call subdirs('../logs/':datasetcode:char(0),subdirs)
	//if count(subdirs,fm) else
	tdir = "../logs/" ^ datasetcode;
	tdir.converter("/", OSSLASH);
	if (not(xx.osopen(tdir.lcase()))) {
		if (STATUS ne 2) {
			call osmkdir(tdir.lcase());
		}
	}

	//check/make the annual log folder
	//call shell('MD ':logpath)
	if (not(xx.osopen(logpath))) {
		if (STATUS ne 2) {
			call osmkdir(logpath);
		}
	}

	logfilename = logpath ^ "/" ^ datex.substr(-2,2) ^ datex.substr(1,2) ^ datex.substr(4,2) ^ ("00" ^ SYSTEM.a(24)).substr(-2,2);
	logfilename ^= ".xml";
	logfilename.converter("/", OSSLASH);

	if (logfile.osopen(logfilename)) {

		//append the logfile
		logptr = logfilename.osfile().a(1);

		//if the last bit closes the log, overwrite it to continue the log
		//t2=logptr-6
		//call osbread(tt,logfile,logfilename,t2,6)
		//if tt='</Log>' then logptr=t2

		//backup and read the last 6 characters of the log
		logptr -= 6;
		call osbread(tt, logfile,  logptr, 6);
		//maybe backup the log pointer to overwrite the closing tag
		if (tt == "</Log>") {
			logptr -= 6;
		}

	}else{

		//initialise xml log file header
		call oswrite("", logfilename);
		if (logfile.osopen(logfilename)) {
			logptr = 0;
			logx = "<?xml version=\"1.0\"?>" "\r\n";
			logx ^= "<Log>" "\r\n";
			gosub writelogx2();

			gosub writelogxclose();

		}else{
		// print 'CANNOT OPEN LOG FILE ':logfilename
			logfilename = "";
		}
	}

	linkfilename1 = "";
	replyfilename = "";

	//save the current program stack
	//limit on 299 "programs" and dictionary entries count as 1!!!
	//see "arev stack and rtp documentation.htm"
	//and http://www.revelation.com/knowledge.nsf/07dbcbabb6b3e379852566f50064cf25/daca1813e510f571852563b6006a9b9c?OpenDocument
	stack = "";
	//call programstackstack);

////////////
nextrequest:
////////////

	lastrequestdate = var().date();
	lastrequesttime = var().time();
	win.registerx="";

	//forcedemail
	SYSTEM.r(117, "");

	//no interruptfilename
	SYSTEM.r(6, "");

	TCLSTACK = TCLSTACK.field(FM, 1, 10);
	//<AREV>
	execute("RESET");
	//</AREV>
	SYSTEM.r(33, 1);
	RECORD = "";
	ID = "";
	MV = 0;
	USER0 = "";
	USER1 = "";
	USER3 = "";
	USER4 = "";
	//garbagecollect;
	win.wlocked = "";

	//linkfilename1=''

nextsearch:
///////////

	//BREAK ON;

	//clear any file handles
	var().osflush();

	//on win95 this runs at same speed in full screen or windowed

	locks.unlock( "REQUEST*" ^ linkfilename1);
	locks.unlock( "REQUEST*" ^ replyfilename);
	//print 'unlock locks,REQUEST*':linkfilename1
	//print 'unlock locks,REQUEST*':replyfilename
	call listen5("UNLOCKLONGPROCESS");

	breaktime = "X";

nextsearch0:
////////////
	//restore the program stack
	//call programstackstack);

	//clear out @EW, @PW, @VW and @XW globals for PER REQUEST GLOBAL BUFFERS

	//validcode3
	//equ codebuff to @ew   edit win?
	//equ authbuff to @pw   popup win?
	EW = EW.field(FM, 1, 9);
	PW = PW.field(FM, 1, 9);

	//validcode2
	//equ codebuff to @vw vol win?
	//equ authbuff to @xw xref win?
	VW = VW.field(FM, 1, 9);
	//@vw<10>=@username
	XW = XW.field(FM, 1, 9);

	//ATTACH UTILITY, ED UBP * for documentation on @VW etc

	//@MW menu, @HW help FREE FOR USE?
	MW = MW.field(FM, 1, 9);
	HW = HW.field(FM, 1, 9);

	//@AW application window
	//print @aw<30>
	AW = AW.field(FM, 1, 30);

	//@SW SQL win @sw<1> <2> <3> see init.general timezone

	//lets clear some standard stuff too
	ID = "";
	RECORD = "";
	DICT = "";
	MV = 0;

	processno = SYSTEM.a(24);

	//print time() '[TIME2,MTS]':
	//similar in LISTEN and AUTORUN
	tt = var().time().oconv("MTS") ^ " " ^ datasetcode ^ " " ^ processno ^ " " ^ nrequests ^ " " ^ memspace(999999).oconv("MD13P") ^ " Listening" " " ^ elapsedtimetext(lastrequestdate, lastrequesttime);
	if (VOLUMES) {
		print(atcol0, tt, " : ", clreol);
	}else{
		var(tt).oswrite("process." ^ processno);
	}

	call unlockrecord("PROCESSES", processes, processno);

	//prevent sleep in esc.to.exit
	//garbagecollect;
	timenow = ostime();
	SYSTEM.r(25, timenow);
	SYSTEM.r(26, timenow);

	//check for a variety of reasons to restart including
	//corrupt system record
	//changed system.cfg or ../../system.cfg
	//recompiled $LISTEN
	call listen5("CHECKRESTART");
	if (ANS) {
		request1 = ANS;
		gosub exit();
		stop();
	}

	//start *.RUN commands (start processes)
	call listen5("RUNS");

	//update process status and quit if global.end ../../global.end or database.end
	//unassigned xx means trigger check for quitting
	call heartbeat("CHECK");

	gosub flagserveractive();

	//run autorun, syncdata and clear old files once a minute
	if ((var().time() - lastautorun > 60) or (var().time() < lastautorun - 600)) {
		lastautorun = var().time();

		//call autorun instead of perform to allow output to remain on screen
		//(BUT errors cause listen to crash and restart)
		if (not(var("autorun.end").osfile())) {
			call autorun3();
		}

		//run scheduled reports etc except on test data (ie not backed up)
		//if not(system<61>) or dir("exodus.id") then
		// if system<121> then call syncdata
		// end

		//delete old response and temp files every 1 minute
		call listen5("DELETEOLDFILES2", "", inpath);

		//autorun might have run so avoid processing another request without resetting
		goto nextsearch0;
	}

	//garbagecollect;
	//if index(drive(),':',1) then exe='.exe' else exe=''
	//cmd='WAITING.EXE'
	cmd = "waiting";
	//fast exit on server.end and install.end. database.end only checked every 10 secs
	cmd ^= " " ^ inpath ^ "*.1 " ^ waitsecs ^ " " ^ sleepms ^ " " ^ serverend;

	linkfilename1 = inpath ^ "neos" ^ processno.oconv("R(0)#4") ^ ".0";
	linkfilename1.osdelete();
	if (linkfilename1.osfile()) {
		if (tracing) {
			printl("CANNOT DELETE ", linkfilename1, " GENERATING ANOTHER");
			//linkfilename1=inpath:rnd(99999999):'.0'
			linkfilename1 = var(99999999).rnd() ^ ".0";
		}
	}
	linkfilename0 = linkfilename1.substr(inpath.length() + 1,9999);
	cmd ^= " " ^ linkfilename0 ^ " " ^ inpath ^ " " ^ portno;
	cmd.lcaser();
	cmd.converter("/", OSSLASH);

	//remove lock indicating processing (hangs if unlock and not locked)
	//gosub sysunlock
	//print cmd
	//break off
	//garbagecollect;
	//unfortunately cant use this because on SOME datasets?!?!?
	//eg TESTMARK/BMTEST (MARKONE/BMDATA runs!!!!) both MASTERBR and MASTTEST dont
	//on nl1 it will not run the waiting.exe command
	//solved by removing path before WAITING.EXE above (unknown cause though)
	if (SYSTEM.a(29)) {

		osshell(cmd);

	}else{
		//print timedate():' ':cmd

		cmd.osshell();

	}
	//break on

	//place a lock to indicate processing
	//should really retry in case blocked by other processes checking it
	//call rtp57(syslock, '', '', trim(@station):system<24>, '', '', '')
	call lockrecord("PROCESSES", processes, processno, "", 999999);

	//pause forever while any quiet time process (eg hourly backup) is working
	//maybe not necessary on test data
	if (live) {
		if (lockrecord("PROCESSES", processes, "ALL", "", 999999)) {
			processes.unlock( "ALL");
		}
	}

	//if got file then quit
	if (linkfilename1.osfile()) {

		//flush
		//osopen linkfilename1 to linkfile1 else
		// call ossleep(1000*.1)
		// if tracing then print 'CANNOT OPEN ':quote(linkfilename1)
		// goto nextrequest
		// end
		//goto requestinit
		linkfilenames = linkfilename0;
		goto gotlink;
	}

	//timeout if no activity
	dostime = ostime();
	now = (var().date() ^ "." ^ dostime.floor().oconv("R(0)#5")) + 0;
	//if timeouttime and now>timeouttime then gosub exit

	gosub flagserveractive();

	//switch into interactive mode to check for operator input
	s33 = SYSTEM.a(33);
	SYSTEM.r(33, "");

	//check for esc key to exit
	//if esc.to.exit() then gosub exit

	charx.input("", -1);
	//charx=ucase(charx[1,1])
	//charx=charx[1,1]

	//esc
	if (charx.index(INTCONST.a(1))) {
		call mssg("You have pressed the [Esc]  key to exit|press again to confirm|", "UB", buffer, "");
		//loop
		// input reply,-1:
		//until reply
		// call ossleep(1000*1)
		// repeat
		reply.input("", 1);
		call mssg("", "DB", buffer, "");
		if (reply == INTCONST.a(1)) {
			gosub exit();
		}
		if (reply == INTCONST.a(7)) {
			charx = reply;
		}
	}

	//R=Restart
	//if charx='R' then
	// request1='RESTART RPRESSED'
	// gosub exit
	// end

	//f5
	if (charx == PRIORITYINT.a(2)) {
		cmd = "";
		if (not(VOLUMES)) {
			printl();
			print("Enter command : ");
			cmd.input();
		}
		execute(cmd);
	}

	//f10
	if (charx == INTCONST.a(7)) {
		execute("RUNMENU " ^ ENVIRONSET.a(37));
	}

	if (charx ne "") {

		//"U" = unlock all locks
		if (charx == "U") {
			var().clearfile(locks);
			//unlock all
			//if tracing then
			printl(" ", "ALL LOCKS RELEASED");
			//end else
			// print @(25,@crthigh/2):
			// print 'ALL LOCKS RELEASED' 'C#30':
			// end
		}

		//"D" = De bug
		//if charx='D' and @username='EXODUS' then
		// DE BUG
		// end

		gosub gettimeouttime();
	}

	//switch back to not interactive mode
	////////////////////////////////////
	SYSTEM.r(33, s33);

	//gosub getbakpars
	call getbackpars(bakpars);

	//call monitor approx every minute +/- 10 seconds to avoid checking all the time
	if ((var().time() - lastmonitortime).abs() > 60 + var(20).rnd() - 10) {
	//if abs(time()-lastmonitortime)>(0+rnd(20)-10) then

		//monitor updates nagios and optionally checks for upgrades
		call monitor2();
		lastmonitortime = var().time();

		//install and run patches
		//patched=0
		//!system patches only on live data ... or only test data
		//!to avoid loading programs into test where they are not available to users
		//!PATCH.1 on live or PATCHT.1 on test
		//if live then tt='path' else tt='patcht'
		//call listen5('PATCHANDRUNONCE',tt,processes)
		call listen5("PATCHANDRUNONCE", live, processes);
		patched = ANS;

		//database specific patches (can load into test)
		//call listen5('PATCHANDRUNONCE',datasetcode,processes)
		//if @ans then patched=1

		if (patched) {
			request1 = "RESTART PATCHED";

			gosub exit();
		}

		//monitor might have run so avoid processing another request without resetting
		goto nextsearch0;
	}

	//run autorun, syncdata and clear old files once a minute
	//has been moved up so it can run immediately after %newautorun%

	//A forces autorun
	if (charx == "A") {
		lastautorun = "";
	}

	//backup
	if (var("Bb").index(charx)) {
		goto backup;
	}
	if ((var().time() >= bakpars.a(3)) and (var().time() <= bakpars.a(4))) {

		//call log2('LISTEN: Backup time for ':datasetcode,logtime)

		//delay closedown randomly to avoid conflict with identically configured processes
		call ossleep(1000*var(10).rnd());

		dow = (var().date() - 1) % 7 + 1;

		//optionally perform backup and/or shutdown and not backed up today
		if (bakpars.a(9)) {
			//call log2('Backup is disabled',logtime)

		} else if (var().date() == bakpars.a(1)) {
			//call log2('Backup already done today',logtime)

		} else if (bakpars.a(11)) {
			call log2("backup is suppressed. Quitting.", logtime);
			perform("OFF");
			var().logoff();

		} else if (not(bakpars.a(5).index(dow))) {
			call log2("Not right day of week " ^ bakpars.a(5) ^ " Logging off", logtime);
			perform("OFF");
			var().logoff();

		} else {
			//call log2('Preventing further automatic backups today',logtime)
			((var().date() + var().time() / 86400).oconv("MD50P")).writev(DEFINITIONS, "BACKUP", 1);

backup:
			//similar code in LISTEN and LISTEN2
			USER4 = "";
			cmd = "FILEMAN BACKUP " ^ datasetcode ^ " " ^ bakpars.a(7) ^ " SYSTEM";
			call log2("Attempting backup " ^ cmd, logtime);
			perform(cmd);

			//quit and indicate to calling program that a backup has been done
			PSEUDO = "BACKUP";
			//B=BACKUP=backup/copydb/upgrade/quit
			//b=BACKUP2=backup, send email and resume -ie dont copydb/upgrade/quit
			if (charx == "b") {
				PSEUDO ^= "2";
			}
			PSEUDO ^= " " ^ bakpars.a(7) ^ " " ^ bakpars.a(12);
			if (USER4) {
				stop();
			}

		}

	}

	call listen5("DELETEOLDFILES", "*.*", inpath, "", "");

	goto nextsearch0;

gotlink:
////////

	//get the earliest time possible for the log
	requestdate = var().date();
	requesttime = ostime();
	SYSTEM.r(25, requesttime);

	gosub gettimeouttime();

	nlinkfiles = linkfilenames.count(FM) + 1;

	//find a request to process
	for (linkfilen = 1; linkfilen <= nlinkfiles; ++linkfilen) {

		linkfilename1 = inpath ^ linkfilenames.a(linkfilen);

		//lock it to prevent other listeners from processing it
		//unlock locks,'REQUEST*':linkfilename1
		if (not(lockrecord("", locks, "REQUEST*" ^ linkfilename1, xx))) {
			if (tracing) {
				printl("CANNOT LOCK LOCKS,", ("REQUEST*" ^ linkfilename1).quote());
			}
			goto nextlinkfile;
		}
	//print 'lock locks,REQUEST*':linkfilename1

		var().osflush();
		if (not(linkfile1.osopen(linkfilename1))) {
			//remove from future candidate files?
			call ossleep(1000*.1);
			if (tracing) {
				printl("CANNOT OPEN RW ", linkfilename1.quote());
			}
			goto nextlinkfile;
		}

		//get the .1 file which contains the request
		timex = var().time();
readlink1:
		USER0 = "";
		//osbread request from linkfile1 at 0 length 256*256-4
		tt = 0;
		call osbread(USER0, linkfile1,  tt, 256 * 256 - 4);

		//if cannot read it then try again
		if ((USER0 == "") and (var().time() == timex)) {
			var().osflush();
			call ossleep(1000*.1);
			linkfile1.osclose();
			if (not(linkfile1.osopen(linkfilename1))) {
				{}
			}
			goto readlink1;
		}

		//cleanup the input file
		//convert '&' to fm in request
		USER0.converter("\r\n", FM);
		USER0.swapper("\\\\", "\\");
		USER0.swapper("\\r", FM);
		//convert @lower.case to @upper.case in request
		while (true) {
			///BREAK;
			if (not(USER0[-1] == FM)) break;
			USER0.splicer(-1, 1, "");
		}//loop;
		//swap '%FF' with rm  in request
		//swap '%FE' with fm in request
		//swap '%FD' with vm in request
		//swap '%FC' with sm in request
		//swap '%FB' with tm in request
		//swap '%FA' with stm in request
		//swap '%F9' with \F9\ in request
		if (VOLUMES) {
			//HEXX1
			//for ii=249 to 255
			// swap hexx(ii) with char(ii) in request
			// next ii
			//decode %FA-%FF
			call hexcode(3, USER0);
		//end else
			//swap 'MEDIA':'.TYPE' with 'JOB_TYPE' in request
			//swap 'PRODUCT':'.CATEGORIES' with 'PRODUCT_CATEGORIES' in request
		}

		//replyfilename=ucase(request<1>)
		//eg D:\EXODUS\DATA\DEVDTEST\|3130570.1
		//eg /var/www/html/exodus2/EXODUS//data/BASIC/~9979714.1
		replyfilename = USER0.a(1);
		USER0.remover(1);

		//php requests that responses are to be written to linux file system files
		//but if being served by dos/windows then need to reply to dos/win files
		if (VOLUMES) {
			//replyfilename could be D:\hosts\test\data\TEST/~7538977.1
			//tt=index(replyfilename,'/data/',1)
			replyfilename.converter("/", OSSLASH);
			//tt=index(replyfilename,'\data\',1)
			tt = replyfilename.index(OSSLASH "data" OSSLASH);
			if (tt) {
				//eg drive() = D:\EXODUS\EXODUS\ ...
				//replyfilename='..\':replyfilename[tt+1,9999]
				replyfilename = ".." OSSLASH ^ replyfilename.substr(tt + 1,9999);
			}
		}

		//lock the replyfilename to prevent other listeners from processing it
		//unlock locks,'REQUEST*':replyfilename
		if (not(lockrecord("", locks, "REQUEST*" ^ replyfilename, xx))) {
			//if tracing then print 'CANNOT LOCK LOCKS,':quote('REQUEST*':replyfilename)
			goto nextlinkfile;
		}

	//print 'lock locks,REQUEST*':replyfilename

		//delete then unlock the request file
		ntries = 0;
deleterequest:
		linkfile1.osclose();
		linkfilename1.osdelete();
		if (linkfilename1.osfile()) {
			var().osflush();
			//garbagecollect;
			call ossleep(1000*.1);
			ntries += 1;
			//if tracing then print 'COULD NOT DELETE ':linkfile1
			if (ntries < 100) {
				goto deleterequest;
			}
			if (tracing) {
				printl("COULD NOT DELETE ", linkfile1);
			}
		}

		//leave these in place for the duration of the process
		//they should be cleared by unlock all somewhere at the end or beginning
		//unlock locks,'REQUEST*':replyfilename
		//unlock locks,'REQUEST*':linkfilename1

		//found a good one so process it
		gosub requestinit();

		goto nextrequest;

nextlinkfile:
		{}

	};//linkfilen;

	goto nextsearch;

	return "";
}

subroutine requestinit() {

	nrequests += 1;

	print(atcol0, clreol, var().time().oconv("MTS"), " ");

	//clear out buffers just to be sure
	//request=''
	USER1 = "";
	//response=''
	//response='Error: Response not set in LISTEN'
	call listen4(1, USER3);
	USER4 = "";

	//only go on active if we actually get the request
	//to avoid multiple processes going on active for only one request
	onactive = 0;

	listenfailure = 0;

	//cut off the initial connection info fields
	tt = USER0.a(1);
	if (tt == "VERSION 3") {
		//nconnectionfields
		tt = USER0.a(2) - 1;
		USER0.remover(2);
	} else if (USER0.a(1) == "VERSION 2") {
		tt = 4;
	} else {
		tt = 0;
	}
	connection = USER0.field(FM, 1, tt);

	USER0 = USER0.field(FM, tt + 1, 999999);

	dataset = USER0.a(1).ucase();
	username = USER0.a(2).ucase();
	password = USER0.a(3).ucase();

	//REMOTE_ADDR, REMOTE_HOST, HTTPS, SESSIONID
	//if connection<1>='::1' then connection<1>='127.0.0.1'
	connection.swapper(FM ^ "::1", FM ^ "127.0.0.1");
	connection.converter(FM, VM);
	connection.r(1, 10, username);
	SYSTEM.r(40, connection);
	ipno = connection.a(1, 2);
	netid = connection.a(1, 5);

	firstrequestfieldn = 4;
	request1 = USER0.a(firstrequestfieldn);

	//throw away initial request CACHE
	if (request1 == "CACHE") {
		USER0.remover(firstrequestfieldn);
		request1 = USER0.a(firstrequestfieldn);
	}

	request1.ucaser();
	if (request1 == "LOGIN") {
		USER0.r(5, USER0.a(2));
	}
	//convert @lower.case to @upper.case in request
	request2 = USER0.a(5);
	request3 = USER0.a(6);
	request4 = USER0.a(7);
	request5 = USER0.a(8);
	request6 = USER0.a(9);

	//remove dataset and password
	USER0 = USER0.field(FM, firstrequestfieldn, 99999);

	responsetime = "";
	rawresponse = "";
	gosub updreqlog();

	if (logfilename) {

		datex = var().date();
		timex = requesttime;

		tt = "<Message ";
		tt ^= " Date=" ^ xmlquote(datex.oconv("D"));
		tt ^= " Time=" ^ xmlquote(timex.oconv("MTS"));
		tt ^= " DateTime=" ^ xmlquote(datex.oconv("DJ-") ^ "T" ^ timex.oconv("MTS") ^ "." ^ timex.field(".", 2));
		tt ^= " User=" ^ xmlquote(username);
		tt ^= " File=" ^ xmlquote(field2(replyfilename, OSSLASH, -1));
		//REMOTE_ADDR REMOTE_HOST HTTPS
		tt ^= " IP_NO=" ^ xmlquote(connection.a(1, 2));
		tt ^= " Host=" ^ xmlquote(connection.a(1, 3));
		tt ^= " HTTPS=" ^ xmlquote(connection.a(1, 4));
		tt ^= " Session=" ^ xmlquote(netid);
		tt ^= ">" "\r\n";

		tt ^= "<Request ";

		logx = USER0;
		//gosub convlogx
		t2 = "CONVLOG";
		call listen5(t2, logx, xx, yy);
		logx.converter("^", FM);
		logx.r(1, request1);
		if (logx.a(1)) {
			tt ^= " Req1=" ^ (logx.a(1).quote());
		}
		if (logx.a(2)) {
			tt ^= " Req2=" ^ (logx.a(2).quote());
		}
		if (logx.a(3)) {
			tt ^= " Req3=" ^ (logx.a(3).quote());
		}
		if (logx.a(4)) {
			tt ^= " Req4=" ^ (logx.a(4).quote());
		}
		if (logx.a(5)) {
			tt ^= " Req5=" ^ (logx.a(5).quote());
		}
		req6up = logx.field(FM, 6, 9999);
		req6up.converter(FM, "^");
		if (req6up.length()) {
			tt ^= " Req6up=" ^ (req6up.quote());
		}

		tt ^= "/>";
		tt.transfer(logx);
		gosub writelogx2();

	}

	anydata = 0;

	if (request2 == "JOURNALS") {
		request2 = "BATCHES";
		if ((request3.count("*") == 3) and (request3[-1] == "*")) {
			request3.splicer(-1, 1, "");
		}
	}

	//print the request
	if (tracing) {
		tt = username ^ " " ^ USER0;
		//hide dataset and passord
		//has been removed above now
		//tt<firstrequestfieldn-1>=''
		//tt<firstrequestfieldn-3>=''
		tt.converter(FM, " ");

		t2 = connection.a(1, 2);
		if (connection.a(1, 3) ne t2) {
			t2 ^= " " ^ connection.a(1, 3);
		}

		print(t2.oconv("L#15"), " ", tt, " : ");
	}

	linkfilename2 = replyfilename;
	linkfilename2.splicer(-1, 1, 2);
	linkfilename3 = replyfilename;
	linkfilename3.splicer(-1, 1, 3);

	//save the response file name
	//so that if listen fails then NET the calling program can still respond
	PRIORITYINT.r(100, linkfilename3);

	linkfile2size = linkfilename2.osfile().a(1);
	if (linkfile2size > maxstrlen) {

		if (linkfile2.osopen(linkfilename2)) {

			//read blocks of iodat
			datx="";
			inptr = 0;
			for (blockn = 1; blockn <= nblocks; ++blockn) {

				//osbread datx(blockn) from linkfilename2 at ((blockn-1)*inblocksize) length inblocksize
				//tt=(blockn-1)*inblocksize
				call osbread(datx(blockn), linkfile2,  inptr, inblocksize);

				///BREAK;
				if (not(datx(blockn).length())) break;

				//avoid hexcode spanning block end by moving one or two bytes backwards
				if (blockn > 1) {
					tt = ((datx(blockn - 1)).substr(-2,2)).index("%");
					if (tt) {
						datx(blockn - 1) ^= datx(blockn).substr(1,tt);
						datx(blockn).splicer(1, tt, "");
					}
				}

			};//blockn;

			//unescape all blocks
			lendata = 0;
			for (blockn = 1; blockn <= nblocks; ++blockn) {

				if (datx(blockn).length()) {

					//output to log before unescaping since log is xml
					if (logfilename) {

						logx = "";

						//log <DataIn>
						if (not(anydata)) {
							anydata = 1;
							logx ^= "\r\n" "<DataIn>";
						}

						logx ^= datx(blockn);
						gosub writelogx();

					}

					if (VOLUMES) {
						//HEXX2
							/*;
							for ii=0 to 36;
								swap HEXX(ii) with char(ii) in datx(blockn);
								next ii;
							for ii=38 to 255;
								swap HEXX(ii) with char(ii) in datx(blockn);
								next ii;
							//convert %25 to % last
							swap HEXX(37) with char(37) in datx(blockn);
							*/
						//decode all
						call hexcode(2, datx(blockn));
					}

					lendata += datx(blockn).length();

				}

			};//blockn;

			//check max iodat size <= maxstrlen
			if (lendata > maxstrlen) {
				USER1 = "";
				//response='Error: Maximum record size of ':maxstrlen '[XBYTES]':' exceeded in LISTEN'
				call listen4(2, USER3, maxstrlen);
				listenfailure = 1;

			//otherwise join the blocks
			}else{
				USER1 = "";
				for (blockn = 1; blockn <= nblocks; ++blockn) {
					USER1 ^= datx(blockn);
					datx(blockn) = "";
				};//blockn;
			}

		//cannot open linkfilename2 means no iodat
		}else{
cannotopenlinkfile2:
			listenfailure = 1;
			USER1 = "";
			//response='Error: LISTEN cannot open ':linkfilename2
			call listen4(3, USER3, linkfilename2);
		}

	}else{

		if (not(linkfile2size)) {

			USER1 = "";

		}else{

			if (not(linkfile2.osopen(linkfilename2))) {
				goto cannotopenlinkfile2;
			}

			//read whole file upto limit
			call osbread(USER1, linkfile2,  0, maxstrlen);

			//unescape
			//for ii=0 to 255

			//output to log
			if (logfilename) {

				//start after the last <DataIn>
				if (not anydata) {
					anydata = 1;
					logx = "\r\n" "<DataIn>";
					gosub writelogx();
				}

				logx = USER1;
				gosub writelogx();
			}

			if (VOLUMES) {
				//HEXX3
					/*;
					for ii=0 to 36;
						swap HEXX(ii) with char(ii) in iodat;
						next ii;
					for ii=38 to 255;
						swap HEXX(ii) with char(ii) in iodat;
						next ii;
					swap HEXX(37) with char(37) in iodat;
					*/
				//decode all
				call hexcode(2, USER1);
			}

			// next i

		}

	}

	if (linkfile2.osopen(linkfilename2)) {
		linkfile2.osclose();
		linkfilename2.osdelete();
		//osread response from linkfilename3 else response=''
	}

	if (logfilename) {

		if (anydata) {
			logx = "</DataIn>";
			gosub writelogx2();
		}

		savelogptr = logptr;
		logx = "\r\n" "</Message>" "\r\n" "</log>";
		gosub writelogx2();
		logptr = savelogptr;
	}

	//update security table every few secs and every login
	//if request1='LOGIN' or index('036',time()[-1,1],1) then gosub getsecurity
	if (SECURITY.read(DEFINITIONS, "SECURITY")) {
		SECURITY = SECURITY.invert();
	}

	//validate username and password ('LOGIN' gets special treatment)
	//and assume that identity if ok
	call listen2("VALIDATE" ^ FM ^ request1, username, password, connection, invaliduser, dataset);

	SYSTEM.r(2, linkfilename2);

	//get the current program stack
	//stack=""
	//call program.stack(stack)
	RECORD = "";
	ID = "";
	MV = 0;

	call listen5("PROCESSINIT");

	gosub process();

	call listen5("PROCESSEXIT");

	//restore the program stack
	//limit on 299 "programs" and dictionary entries count as 1!!!
	//call program.stack(stack)

	//goto requestexit

	////////////
	//requestexit:
	////////////

	call listen5("UNLOCKLONGPROCESS");

	//1. failsafe unlock everything that might still be locked
	//2. a very few processes rely on all locks being released
	//   eg autorun2 locks so that autoruns are not processed until
	//   the request completes
	//3. generally all processes are careful to unlock what they have locked
	//   but crashed processes will not do this

	var().unlockall();

	//in case any select list left open
	clearselect();

	if (USER3 == "") {
		call listen4(1, USER3);
	}

	if (USER3.index("ERROR NO:")) {
		call log("LISTEN", USER3);
	}

	USERNAME=(listenusername);
	//call sysvar('SET',109,110,listenstation)
	//call sysvar_109_110('SET',listenstation)
	STATION=(listenstation);

	//if ucase(response[1,6])='ERROR:' then iodat=''

	//have to call it here as well :(
	responsetime = ostime();
	secs = ((responsetime - requesttime).oconv("MD60P")) + 0;
	if (secs < 0) {
		secs += 86400;
	}

	tt = USER3.ucase();
	if ((tt.index("R18.6") or tt.index("RTP20 MISSING")) or tt.index("TOO MANY LEVELS OF TCL")) {
		halt = 1;
		//response<-1>='INTERNAL ERROR Closing current EXODUS server process'
		call listen4(1, USER3);
		call sysmsg(USER3);
	}

	rawresponse = USER3;
	rawresponse.converter("\r\n", "|");

	gosub updreqlog();

	if (logfilename) {
		logx = "\r\n" "<Response ProcessingSecs=" ^ (secs.quote()) ^ ">";
		gosub writelogx2();

		//convert non ascii to hexcode
		logx = USER3;
		if (VOLUMES) {
			//HEXX4
				/*;
				swap '%' with '%25' in logx;
				for ii=128 to 255;
					swap char(ii) with HEXX(ii) in logx;
					next ii;
				for ii=0 to 15;
					swap char(ii) with HEXX(ii) in logx;
					next ii;
				for ii=24 to 31;
					swap char(ii) with HEXX(ii) in logx;
					next ii;
				*/
			//encode %->%25 %00-%0F %18-%1F %FA-%FF
			call hexcode(1, logx);
		}
		gosub writelogx();

		logx = "</Response>";
		iodatlen = USER1.length();
		if (iodatlen) {
			logx ^= "\r\n" "<DataOut>";
		}
		gosub writelogx2();

	}

	if (USER1 == "%DIRECTOUTPUT%") {

		logx = USER1;
		//convert '%' to '%25' in logx
		logx.swapper("%", "%25");
		gosub writelogx();

	}else{

		call oswrite("", linkfilename2);
		if (linkfile2.osopen(linkfilename2)) {

			//split into blocks and convert to escape chars
			datx="";
			ptr = 0;
			for (blockn = 1; blockn <= nblocks; ++blockn) {
				blk = USER1.substr(1,outblocksize);
				USER1.splicer(1, outblocksize, "");
				///BREAK;
				if (not(blk.length())) break;

				//in LISTEN and SELECT2 for direct output

				if (VOLUMES) {

					//HEXX5
						/*;
						swap '%' with '%25' in blk;
						//changed to allow language characters to pass through x80-xF9
						for ii=249 to 255;
							swap char(ii) with HEXX(ii) in blk;
							//should not be done per block but is code economic
							swap char(ii) with HEXX(ii) in response;
							next ii;

						//is the following really necessary?
						for ii=0 to 15;
							swap char(ii) with HEXX(ii) in blk;
							//should not be done per block but is code economic
							swap char(ii) with HEXX(ii) in response;
							next ii;
						for ii=24 to 31;
							swap char(ii) with HEXX(ii) in blk;
							//should not be done per block but is code economic
							swap char(ii) with HEXX(ii) in response;
							next ii;
						*/

					//encode %->%25 %00-%0F %18-%1F %FA-%FF
					call hexcode(1, blk);
					call hexcode(1, USER3);

				}

				//convert some things for XML log
				//swap '&' with '%26' in blk
				//swap '"' with '%22' in blk
				//swap '<' with '%3C' in blk
				//swap '>' with '%3E' in blk

				call osbwrite(blk, linkfile2,  ptr);

				if (logfilename) {
					blk.transfer(logx);
					gosub writelogx();
				}

				blk = "";

			};//blockn;

			linkfile2.osclose();

		}else{

			//response='ERROR: LISTEN cannot create temp ':linkfilename2
			call listen4(22, USER3, linkfilename2);

		}

	}

	//try to flush file open
	if (linkfile2.osopen(linkfilename2)) {
		linkfile2.osclose();
	}

	if (logfilename) {

		logx = "";
		if (iodatlen) {
			logx ^= "</DataOut>";
		}
		logx ^= "\r\n" "</Message>" "\r\n";
		gosub writelogx2();

		gosub writelogxclose();

	}

	//swap '|' with char(13) in response
	//swap fm with char(13) in response

	//write the response
	call oswrite(USER3, linkfilename3);

	//trace responded
	responsetime = ostime();
	if (tracing) {
		//tt=''
		//call program.stack(tt)
		//tt=count(tt,fm)+1:'/':count(tt,fm)+1
		tt = " " ^ oconv((responsetime - requesttime) % 86400, "[NUMBER,2]") ^ "s ";
		//seconds
		tt ^= rawresponse.a(1, 1).field("|", 1).a(1, 1);
		if (tt.index("<")) {
			call htmllib2("STRIPTAGS", tt);
		}
		tt.swapper("SESSIONID ", "");
		printl(tt);
		//print linkfilename1
	}

	//flush
	//suspend 'dir>nul'

	//if tracing then print

	if (halt) {
		gosub exit();
	}

	if (USER3 == "OK") {
		if ((request1 == "STOPDB") or (request1.substr(1,7) == "RESTART")) {
			gosub exit();
		}
	}

	//goto nextrequest
	return;
}

subroutine process() {

	//process the input
	//////////////////

	//failure in LISTEN above
	if (listenfailure) {

	//invalid username or password or connection
	} else if (invaliduser) {
		USER1 = "";
		USER3 = invaliduser;
	//call oswrite(username:fm:password:fm:encrypt0:fm:userprivs,date()[-3,3]:time():'.$$$')
	//if no request then possibly some failure in file sharing
	} else if (USER0 == "") {
		USER1 = "";
		//response='Error: No request'
		call listen4(4, USER3);

	//empty loopback to test connection (no username or password required)
	} else if (request1 == "TEST") {
		//iodat='' if they pass iodat, pass it back
		USER3 = "OK";

	//get cookies for company, market, menus etc (may fail if no allowed companies)
	//username and password already validated above
	} else if (request1 == "LOGIN") {

		call listen2(request1, dataset, username, connection, request5);

	//find index values
	//case request1[1,14]='GETINDEXVALUES'
	} else if (request1.substr(1,8) == "GETINDEX") {

		//call listen3(request2,'GETINDEXVALUES')
		call listen5(request1, request2, request3, request4, request5, request6);

	//select some data
	} else if (request1 == "SELECT") {

		//call listen3(request2,request1)
		call listen5(request1, request2, request3, request4, request5, request6);

	//lock a record
	} else if ((request1 == "LOCK") or (request1 == "RELOCK")) {

		call listen3(request2, request1);

		filename = request2;
		keyx = request3;
		sessionid = request4;

		//lockduration is the number of minutes to automatic lock expiry
		lockmins = request5;

		//LOCK request may specify a master lock in form filename*key*sessionid
		//that must exist with the right sessionid
		//such locks are called subsidiary locks and
		//depend on the locking status of the master
		//currently they are not cleared out when the master lock is unlocked
		//maybe unlock could unlock all locks with the same sessionid
		//RELOCK doesnt have any meaning for subsidiary locks
		//since their status is dependent on the master lock status
		masterlock = request6;

		gosub lock();
		sessionid.transfer(USER1);

	} else if (request1 == "UNLOCK") {

		call listen3(request2, request1);

		filename = request2;
		keyx = request3;
		sessionid = request4;

		gosub unlock();

	//read a record
	} else if (((request1 == "READ") or (request1 == "READO")) or (request1 == "READU")) {

		withlock = request1 == "READU";
		//updatenotallowed=''
		USER1 = "";
		sessionid = "";

		filename = request2;
		keyx = request3;
		lockmins = request4;

		readenv = request5;
		win.templatex = readenv;
		call listen3(filename, request1, filetitle, triggers);
		postread = triggers.a(3);

		//reduce chance of using old common
		win.registerx="";

		//allow read to unknown files for the time being
		if (filetitle == "") {
			filetitle = filename;
		}

		filetitle2 = singular(filetitle);
		filetitle2.converter(".", " ");
		//if postread else
		secmode = "ACCESS";
		gosub filesecurity();
		if (not ok) {
			return;
		}
		// end

		//security check - cannot lock so cannot write or delete
		//if withlock then
		// if security(filetitle2:' UPDATE',updatenotallowed,'') then null
		// if security(filetitle2:' CREATE',createnotallowed,'') then null
		// end

		if (not(file.open(filename, ""))) {
			gosub badfile();
			return;
		}

		keyx0 = keyx;
		preread = triggers.a(1);
		if (preread) {
			keyx.transfer(ID);
			win.srcfile = file;
			win.datafile = filename;
			systemsubs = preread;
			call systemsubs(triggers.a(2));
			DATA = "";
			ID.transfer(keyx);
		}

		autokey = 0;
		if ((((keyx == "") or (keyx[1] == "*")) or (keyx[-1] == "*")) or keyx.index("**")) {

			//must provide a key unless locking
			if (not withlock) {
				//response='Error: EXODUS.NET Key missing and not READU'
				call listen4(5, USER3);
				gosub fmtresp();
				return;
			}

getnextkey:

			//setup environment for DEF.SK
			win.wlocked = 0;
			RECORD = "";
			ID = keyx;
			MV = 0;
			win.datafile = filename;
			win.srcfile = file;
			win.isdflt = "";

			call generalsubs("DEF.SK." ^ readenv);

			keyx = win.isdflt;

			if (keyx == "") {
				//response='Error: Next number was not produced':fm:msg
				call listen4(6, USER3, USER4);
				gosub fmtresp();
				return;
			}

			autokey = 1;

		}

		sessionid = "";
		lockkeyx = keyx;
		if (withlock) {
			USER3 = "";

			masterlock = "";

			gosub lock();

			//if cannot lock then get next key
			if ((USER3 == "NOT OK") and autokey) {
				goto getnextkey;
			}

			//cannot do this because should return the record but without a lock
			//even if a lock requested
			//if response then
			// gosub fmtresp
			// return
			// end
		}

		FILEERRORMODE = 1;
		FILEERROR = "";

		if (USER1.read(file, keyx)) {

			//if record already on file somehow then get next key
			if (autokey) {
				goto getnextkey;
			}

			//prevent update
			//if withlock and updatenotallowed then
			if (withlock) {
				if (not(authorised(filetitle2 ^ " UPDATE"))) {
					goto noupdate;
				}

				//check if allowed to update this company code
				gosub checkcompany();
				if (badcomp) {
noupdate:
					if (sessionid) {
						gosub unlock();
					}
					sessionid = "";
					//after unlock which sets response to OK
					lockauthorised = 0;
				}

			}

			if (withlock and (sessionid == "")) {
				//response='Error: CANNOT LOCK RECORD'
				call listen4(7, USER3);
				gosub addlockholder();
			}else{
				USER3 = "OK";
			}
			if (sessionid) {
				USER3 ^= " SESSIONID " ^ sessionid;
			}

		//record doesnt exist
		}else{
			//if @file.error<1>='100' then
			//NO FILE ERROR FOR JBASE
			if (not(FILEERROR) or (FILEERROR.a(1) == "100")) {

				//no spaces in new keys
				//allow in multipart keys on the assumption that they are old keys with spaces
				if ((withlock and keyx.index(" ")) and not(keyx.index("*"))) {
					//if sessionid then gosub unlock
					gosub unlock();
					//msg='Error: ':quote(params<1>):' must not contain a space character'
					call listen4(28, USER3, keyx);
					gosub fmtresp();
					return;
				}

				//prevent create (unless there is a postread which might recover records)
				//nb any postread must test create allowed
				//if withlock then if createnotallowed and not(postread) then
				if (withlock and not(postread)) {

					//must provide a key unless locking
					if (not(authorised(filetitle2 ^ " CREATE", createnotallowed))) {
						if (sessionid) {
							gosub unlock();
						}
						USER3 = createnotallowed;
						gosub fmtresp();
						return;
					}

				}

				//response='Error: ':quote(keyx):' does not exist in the ':quote(filetitle):' file'
				USER1 = "";
				//response='Error: NO RECORD'
				call listen4(8, USER3);
				if (sessionid) {
					USER3 ^= " SESSIONID " ^ sessionid;
				}else{
					if (withlock) {
						gosub addlockholder();
					}
				}
				//response/@user3/RECORDKEY may be used in POSTREAD
				//if autokey then
				// *response:=' RECORDKEY ':keyx
				// tt=keyx
				// *horrible cludge to allow space in recordkey to be understood in client.htm
				// swap ' ' with '{20}' in tt
				// response:=' RECORDKEY ':tt
				// end

			}else{
				gosub geterrorresponse();
			}
		}

		if (withlock) {
			if (not lockauthorised) {
				USER3 ^= " LOCK NOT AUTHORISED";
			}
		}

		//response/@user3/RECORDKEY may be used in POSTREAD
		if (keyx ne keyx0) {
			//response:=' RECORDKEY ':keyx
			tt = keyx;
			//ugly cludge to allow space in recordkey to be understood in client.htm
			tt.swapper(" ", "{20}");
			USER3 ^= " RECORDKEY " ^ tt;
		}

		//postread (something similar also in select2)
		if (postread) {

			//simulate window environment for POSTREAD
			win.srcfile = file;
			win.datafile = filename;
			if (not(DICT.open("dict_" ^ win.datafile))) {
				if (sessionid) {
					gosub unlock();
				}
				//response=quote('DICT.':datafile):' CANNOT BE OPENED'
				call listen4(9, USER3, "DICT." ^ win.datafile);
				gosub fmtresp();
				return;
			}
			keyx.transfer(ID);
			USER1.transfer(RECORD);
			win.orec = RECORD;
			win.wlocked = sessionid;
			origresponse = USER3;
			//response=''
			USER4 = "";
			win.reset = 0;

			systemsubs = postread;
			call systemsubs(triggers.a(4));
			DATA = "";

	//restore this programs environment
			RECORD.transfer(USER1);
			ID.transfer(keyx);

			call cropper(USER1);

			//postread can request abort by setting msg or reset>=5
			//msg with reset<0 results in comment to client
			//if reset>=5 or msg then
			if ((win.reset >= 5) or ((USER4 and (win.reset ne -1)))) {
				if (withlock) {
					gosub unlock();
					//wlocked=0
				}
				//if msg then msg='Error: ':msg
				USER3 = USER4;
				if (USER3 == "") {
					USER3 = "ACCESS REFUSED";
				}
				USER1 = "";
				gosub fmtresp();
				//response='Error: ':response
			}else{
				USER3 = origresponse;
				//postread may have provided a record where non-was found
				//ONLY if it unlocks it as well! otherwise
				//removal of NO RECORD will cause failure in client
				if (not(win.wlocked) and USER1) {

					if (withlock and sessionid) {

						//in case postread changed the key and didnt unlock
						keyx2 = keyx;
						keyx = lockkeyx;

						gosub unlock();

						keyx = keyx2;
					}

					USER3.swapper("Error: NO RECORD", "");

					//swap 'RECORDKEY ' with '' in response
					USER3.trimmerb();
					if (USER3 == "") {
						USER3 = "OK";
					}

					//in case postread has changed the key
					if (keyx ne keyx0 and not(USER3.index("RECORDKEY"))) {
						tt = keyx;
						tt.swapper(" ", "%20");
						USER3 = (USER3 ^ " RECORDKEY " ^ tt).trim();
					}

					if (USER4) {
						USER3 ^= " " ^ USER4;
					}
				}

			}

			//postread may have unlocked the record
			//(and removed it from the locks file)
			//but make sure
			if (sessionid and not(win.wlocked)) {

				storeresponse = USER3;
				gosub unlock();
				USER3 = storeresponse;

				//remove session id
				tt = USER3.index("SESSIONID");
				if (tt) {
					USER3.splicer(tt, ("SESSIONID " ^ sessionid).length(), "");
					USER3.trimmerb();
					if (USER3 == "") {
						USER3 = "OK";
					}
				}

			}

			//do not indicate record found
			//otherwise Delete button will be enabled and deleting a record
			//that does not exist is disallowed but lock will be unlocked
			//if iodat then
			// response='OK'
			// if sessionid then response:=' SESSIONID ':sessionid
			// end

		}

		//prevent reading passwords postread and postwrite
		if ((filename == "DEFINITIONS") and (keyx == "SECURITY")) {
			USER1.r(4, "");
		}

		call cropper(USER1);

	} else if (((request1 == "WRITEU") or (request1 == "DELETE")) or (request1 == "WRITE")) {
	//write:
		filename = request2;
		keyx = request3;
		fieldno = request4;
		sessionid = request5;

		readenv = request6;
		win.templatex = readenv;
		call listen3(filename, request1, filetitle, triggers);
		prewrite = triggers.a(1);

		//reduce chance of using old common
		win.registerx="";

		//disallow read/write to unknown files for the time being
		if (filetitle == "") {
			//response=request1:' ':quote(filename):' is not allowed'
			call listen4(11, USER3, request1 ^ FM ^ filename);
			return;
		}
		//allow read to unknown files for the time being
		if (filetitle == "") {
			filetitle = filename;
		}

		filetitle2 = singular(filetitle);
		filetitle2.converter(".", " ");

		//double check allowed access to file
		//if prewrite else
		secmode = "ACCESS";
		gosub filesecurity();
		if (not ok) {
			return;
		}
		// end

		//simulate window environment

		ID = keyx;
		MV = 0;
		//@record=iodat
		USER1.transfer(RECORD);
		win.datafile = filename;
		//not really needed because pre/post code should assume that it is wlocked
		//but some code does not know that (eg postread called from postwrite)
		win.wlocked = sessionid;
		win.saverec = request1 ne "DELETE";
		win.deleterec = request1 == "DELETE";

		//trim excess field and value marks
		call cropper(RECORD);

		if (not(win.srcfile.open(win.datafile, ""))) {
			//response=quote(datafile):' CANNOT BE OPENED'
			call listen4(9, USER3, win.datafile);
			return;
		}

		if (not(DICT.open("dict_" ^ win.datafile))) {
			//response=quote('DICT.':datafile):' file is not available'
			call listen4(9, USER3, "DICT." ^ win.datafile);
			return;
		}

		//make sure that the record is already locked to the user
		lockkey = filename ^ "*" ^ ID;
		if (not(lockrec.read(locks, lockkey))) {
			lockrec = FM ^ FM ^ FM ^ FM ^ "NO LOCK RECORD";
		}
		if (sessionid ne lockrec.a(5)) {
			//response='Somebody has updated this record.|Your update cannot be applied.':'|The session id does not agree ':quote(lockrec<5>)
			call listen4(12, USER3, lockrec.a(5));
			gosub fmtresp();
			return;
		}

		//update the lock session time
		//similar code in lock: and write:
		gosub getdostime();
		lockduration = defaultlockmins / (24 * 60);
		lockrec.r(1, lockduration + dostime);
		lockrec.r(2, dostime);
		lockrec.write(locks, lockkey);

		//get a proper lock on the file
		//possibly not necessary as the locks file entry will prevent other programs
		//proper lock will prevent index mfs hanging on write

		win.valid = 1;
		gosub properlk();
		if (not(win.valid)) {
			return;
		}

		if (not(win.orec.read(win.srcfile, ID))) {
			win.orec = "";
		}

		//trim excess field and value marks
		call cropper(win.orec);

		//double check not updated by somebody else
		//nb this does not work for delete until client provides
		//record or at least DATE_TIME in delete
		if (request1 ne "DELETE") {
			//!cannot do this unless
			//a) return new record (done) or at least DATE_TIME in WRITEU to client
			//b) provide record in DELETE
			if (allcols ne "") {
				if (dictrec.reado(allcols, filename ^ "*DATE_TIME")) {
					datetimefn = dictrec.a(2);
					olddatetime = win.orec.a(datetimefn);
					newdatetime = RECORD.a(datetimefn);
					if (olddatetime and olddatetime ne newdatetime) {
						gosub properunlk();
						//response='Somebody else has updated this record.|Your update cannot be applied.':'|The time stamp does not agree'
						call listen4(13, USER3);
						gosub fmtresp();
						return;
					}
				}
			}
		}

		//detect if defined postwrite or postdelete called
		postroutine = 0;

		if ((request1 == "WRITEU") or (request1 == "WRITE")) {

			//prevent writing an empty record
			if (RECORD == "") {
emptyrecorderror:
				//response='Write empty data record is disallowed.'
				call listen4(14, USER3);
badwrite:
				gosub properunlk();
				gosub fmtresp();
				return;
			}

			//check if allowed to update this company code
			gosub checkcompany();
			if (badcomp) {
				USER3 = badcomp;
				goto badwrite;
			}

			//double check that the record has not been updated since read
			//Cannot do this unless during WRITE (not WRITEU) we pass back the new timedate
			//readv datetimefn from @dict,'DATE_TIME',2 then
			// if @record<datetimefn> ne orec<datetimefn> then
			//  response='Somebody else has updated this record.|Your update cannot be applied'
			//  gosub properunlk
			//  gosub fmtresp
			//  return
			//  end
			// end

			//prewrite processing
			if (prewrite) {
				//call @updatesubs('PREWRITE')
				systemsubs = prewrite;
				call systemsubs(triggers.a(2));
				DATA = "";
			}

			if (not(win.valid)) {
				gosub properunlk();
				USER3 = USER4;
				gosub fmtresp();
				return;
			}

			//allow for prewrite to change the key (must not lock the new key)
			//prewrite must NOT unlock the record because write will fail by index mfs hanging if any
			//is the above true? because unlock just below will
			//remove the lock file record
			//and actually the unlock will fail if it cannot true lock the record
			//daybook.subs4 unlocks and it works fine and will leave lock hanging if it does not
			if (ID ne keyx) {

				gosub unlock();
				gosub properunlk();

				keyx = ID;

				//cannot place a lock file entry if the prewrite has locked the new key
				file.unlock( keyx);

				masterlock = "";

				lockmins = defaultlockmins;
				gosub lock();
				if (USER3 ne "OK") {
					return;
				}

				gosub properlk();
				if (not(win.valid)) {
					return;
				}

			}

			call cropper(RECORD);

			//check for empty record again in case updatesubs screwed up
			if (RECORD == "") {
				goto emptyrecorderror;
			}

			//failsafe in case prewrite unlocks key?
			//gosub properlk

			replacewrite = triggers.a(5);
			if (replacewrite) {
				systemsubs = replacewrite;
				call systemsubs(triggers.a(6));
				//in case it changes @record?
				if (not(RECORD.read(win.srcfile, keyx))) {
					{}
				}
			}else{
				RECORD.write(win.srcfile, keyx);
			}

			//post write processing
			postwrite = triggers.a(3);
			if (postwrite) {
				postroutine = 1;
				//call @updatesubs('POSTWRITE')
				systemsubs = postwrite;
				call systemsubs(triggers.a(4));
				DATA = "";
			}

			//send back revised data or nothing
			//NB data is now '' to save space so always send back data unless @record is cleared
			if (RECORD == USER1) {
				USER1 = "";
			}else{
				USER1 = RECORD;
			}

			//prevent reading passwords postread and postwrite
			if ((filename == "DEFINITIONS") and (keyx == "SECURITY")) {
				USER1.r(4, "");
			}

		} else if (request1 == "DELETE") {

			predelete = triggers.a(1);

			//ensure that deletion works on the orig record
			RECORD = win.orec;

			//prevent deleting if record does not exist
			if (RECORD == "") {
				//response='Cannot delete because ':quote(keyx):' does not exist in the ':quote(filetitle):' file'
				call listen4(15, USER3, keyx ^ FM ^ filetitle);
				gosub fmtresp();
				return;
			}

			//check allowed to delete
			//if security(filetitle2:' DELETE',msg,'') else
			// response=msg
			// gosub fmtresp
			// return
			// end
			//if predelete else
			secmode = "DELETE";
			gosub filesecurity();
			if (not ok) {
				return;
			}
			// end

			//predelete processing
			if (predelete) {
				//call @updatesubs('PREDELETE')
				systemsubs = predelete;
				call systemsubs(triggers.a(2));
				DATA = "";
			}
			if (not(win.valid)) {
				gosub properunlk();
				USER3 = USER4;
				gosub fmtresp();
				return;
			}

			win.srcfile.deleterecord(keyx);

			//post delete processing
			postdelete = triggers.a(3);
			if (postdelete) {
				postroutine = 1;
				systemsubs = postdelete;
				call systemsubs(triggers.a(4));
				DATA = "";
			}

			//send back no iodat
			USER1 = "";

		}

		//remove LOCKS file entry
		if (request1 ne "WRITE") {

			locks.deleterecord(lockkey);

			//unlock local lock
			win.srcfile.unlock( keyx);

		}else{
			gosub properunlk();
		}

		//even postwrite/postdelete can now set invalid (to indicate invalid mode etc)
		//if valid then response='OK' else response='Error:'
		//WARNING TODO: check ternary op following;
		USER3 = win.valid ? "OK" : "Error:";

		if (request1 ne "DELETE") {
			tt = ID;
			//response:=' RECORDKEY ':@id
			//horrible cludge to allow space in recordkey to be understood in client.htm
			tt.swapper(" ", "{20}");
			USER3 ^= " RECORDKEY " ^ tt;
			if (sessionid) {
				USER3 ^= " SESSIONID " ^ sessionid;
			}
		}

		//pre and post routines can return warnings/notes in msg

		if (USER4) {
			USER3 ^= " " ^ USER4;
		}

		//if postroutine else call flush.index(filename)
		call flushindex(filename);

		gosub fmtresp();
		return;

	//execute a request
	} else if (request1 == "EXECUTE") {

		//if @username='EXODUS' then
		// oswrite @user0 on 'USER0'
		// oswrite @user1 on 'USER1'
		// end

		//build command from request and check is a valid program
		voccmd = request2;
		if (voccmd == "") {
			//response='LISTEN:EXECUTE: Module name is missing from request'
			call listen4(32, USER3);
			return;
		}
		voccmd ^= "PROXY";
		if (VOLUMES) {
			if (not(xx.read(voc, voccmd))) {
				//response='Error: ':quote(voccmd):' module is not available'
				call listen4(16, USER3, voccmd);
				return;
			}
		}

		//provide an output file for the program to be executed
		//NB response file name for detaching processes
		//will be obtained from the output file name LISTEN2 RESPOND
		//this could be improved to work
		//..\data\DEVDTEST\|5916783.2
		///root/exodus/service/data/exodus/~3810873.2
		printfilename = linkfilename2;

		//tt=drive()
		//tt[-7,7]=''
		//if printfilename[1,len(tt)]=tt then printfilename[1,len(tt)]='..':OSSLASH

		//tt=printfilename[-1,'B.']
		//tt=field2(printfilename,'.',-1)
		//printfilename[-len(tt),len(tt)]='htm'
		printfilename.splicer(-1, 1, "htm");
		SYSTEM.r(2, printfilename);

		//provide interruptfilename
		//esc.to.exit will return 1 if it finds this file
		//clear it if running non-interruptable processes and giveway maybe called
		tt = linkfilename2;
		tt.splicer(-1, 1, "5");
		SYSTEM.r(6, tt);

		//execute the program
		USER3 = "OK";
		win.valid = 1;
		USER4 = "";

		//request, iodat and response are now passed and returned in @user0,1 and 3
		//other messages are passed back in @user4
		//execute instead of call prevents program crashes from crashing LISTEN
		//changed now that dataset and password are removed in requestinit
		//request=field(request,fm,firstrequestfieldn+2,99999)
		USER0 = USER0.field(FM, 3, 99999);

		//pass the output file in linkfilename2
		//not good method, pass in system?
		if (var("LIST,SELECTBATCHES").locateusing(",",USER0.a(1),xx)) {
			USER1 = linkfilename2;
		}
		if (USER0.a(1).substr(1,4) == "VAL.") {
			USER1 = linkfilename2;
		}

		//execute so that failures dont cause failures of LISTEN
		//but not listen resets itself nicely perhaps this isnt needed
		execute(voccmd);

		//reformat for reqlog
		voccmd.splicer(-5, 5, "");
		USER0.r(1, voccmd ^ "_" ^ USER0.a(1));

		//discard any stored input
		DATA = "";

		if (USER4) {

			user4x = USER4.ucase();

			//convert error message (could also be a system error eg dict says indexed but isnt
			if ((user4x.index("IN INDEX.REDUCER") or user4x.index("IN RTP21")) or user4x.index("IN RTP20")) {
				//@user4='Please select fewer records and/or simplify your request'
				call listen4(17, USER4);
			}

			//send errors to exodus
			if (((USER4 ^ user4x).index("INTERNAL ERROR")) or user4x.index("DAMAGED FILE")) {
				call sysmsg(USER4);
			}

		}

		call cropper(USER4);
		call cropper(USER3);

		if (USER4) {
			USER1 = "";
			USER3 = "Error: " ^ USER4;
			gosub fmtresp();
		}

		if (USER3 == "") {
			//response='Error: No OK from ':voccmd:' ':request
			call listen4(18, USER3, voccmd);
			gosub fmtresp();
		}

		//speed up next autorun if new one has been added
		tt = USER3.index("%NEWAUTORUN%");
		if (tt) {
			USER3.swapper("%NEWAUTORUN%", "");
			lastautorun = "";
		}

		//get the printfilename in case the print program changed it
		printfilename = SYSTEM.a(2);
		//if tracing then
		// *print ' got it'
		// print atcol0:clreol:
		// end

		//make sure that the output file is closed
		if (printfile.osopen(printfilename)) {
			printfile.osclose();
		}
	/*;
		case request1='STOPDB';
			call listen5(request1,request2,install.end,server.end);

		case request1='RESTART';
			iodat='';
			response='OK';
	*/

	} else if (request1 == "BACKUP") {

		//trigger additional processes that should fail due to backup lock
		(inpath ^ serverflagfilename).osdelete();

		call listen5(request1);

	//or give an error message
	} else {
		USER1 = "";
		//response='Error: ':quote(field(request,fm,4,9999)):' unknown request'
		//response='Error: ':quote(request):' unknown request'
		call listen4(21, USER3, USER0);
	}

	//put no code here because some returns above will short cut it

	return;
}

subroutine exit() {

	if (logfilename) {
		logfile.osclose();
	}

	//remove lock indicating processing
	//gosub sysunlock

	call listen5("UNLOCKLONGPROCESS");

	var().unlockall();

	//call restorescreenorigscrn, origattr);

	//remove flag that this dataset is being served ("listened")
	//osdelete inpath:serverflagfilename

	//get into interactive mode
	//system<33>=origbatchmode
	SYSTEM.r(33, "");
	//call setprivilegeorigprivilege);
	if (request1.substr(1,7) == "RESTART") {
		USER4 = request1;
		//return to net which will restart LISTEN
		stop();
	}

	//break off
	//break on

	//esc does this
	if ((origbatchmode or (request1 == "STOPDB")) or halt) {
		//break off
		perform("OFF");
		var().logoff();
	}

	//msg is @user4
	USER4 = "TERMINATED OK";

	stop();
}

subroutine geterrorresponse() {
	fileerrorx = FILEERROR;
	USER3 = "Error: " ^ ("FS" ^ fileerrorx.a(1, 1)).xlate("SYS.MESSAGES", 11, "X");
	USER3.swapper("%1%", handlefilename(fileerrorx.a(2, 1)));
	USER3.swapper("%2%", fileerrorx.a(2, 2));
	gosub fmtresp();
	return;
}

subroutine fmtresp() {

	//trim everything after <ESC> (why?)
	tt = USER3.index("<ESC>");
	if (tt) {
		USER3 = USER3.substr(1,tt - 1);
	}

	//cannot remove since these may be proper codepage letters
	USER3.converter("|", FM);
	USER3.converter(VM, FM);
	if (USER3[1] == FM) {
		USER3.splicer(1, 1, "");
	}
	USER3.swapper(FM, "\r\n");

	return;
}

subroutine gettimeouttime() {
	if (not timeoutsecs) {
		timeouttime = "";
		return;
	}

	timeoutdate = var().date();
	timeouttime = ostime();
	timeouttime += timeoutsecs;
	if (timeouttime > 24 * 60 * 60) {
		timeoutdate += 1;
		timeouttime -= 24 * 60 * 60;
	}
	timeouttime = (timeoutdate ^ "." ^ timeouttime.floor().oconv("R(0)#5")) + 0;
	return;
}

subroutine properlk() {
	//must lock it properly otherwise indexing will try to lock it and fail
	//because it is only in the LOCKS file and not properly locked
	win.valid = 1;

	//must exclude the shadowing system otherwise the entry in the LOCKS file
	//also in balances
	//will cause the LOCK statement to fail
	srcfile2 = win.srcfile;
	//<AREV>
	if (not(win.srcfile.unassigned())) {
		srcfile2.swapper("SHADOW.MFS" ^ SVM, "");
	}
	//</AREV>

	//dont pass the filename because that causes persistent lock checking
	//in jbase version of lockrecord()
	if (not(lockrecord("", srcfile2, keyx, xx))) {
		if (STATUS ne 1) {
			win.valid = 0;
			//response='Error: ':quote(keyx):' CANNOT BE WRITTEN BECAUSE IT IS LOCKED ELSEWHERE'
			call listen4(23, USER3, keyx);
		}
	}

	return;
}

subroutine properunlk() {
	//NB i think that shadow.mfs is NOT programmed to remove the locks file entry

	//must unlock it properly otherwise indexing will try to lock it and fail
	//because it is only in the LOCKS file and not properly locked

	//must exclude the shadowing system otherwise the entry in the LOCKS file
	//will be removed as well
	srcfile2 = win.srcfile;
	//<AREV>
	srcfile2.swapper("SHADOW.MFS" ^ SVM, "");
	//</AREV>

	srcfile2.unlock( keyx);

	return;
}

subroutine lock() {
	//called from LOCK/RELOCK/READU (and WRITE if prewrite changes the key)

	//cannot do update security check here, have to do it AFTER
	// we know if the record exists or not
	//security check - cannot lock so cannot write or delete

	lockauthorised = 1;
	//if unassigned(lockmins) or not(num(lockmins)) or not(lockmins) then
	if (not(lockmins.isnum()) or not(lockmins)) {
		lockmins = defaultlockmins;
	}

	//sessionid which if RELOCKING must match the one on the lock rec
	//otherwise it is assumed that somebody else has taken over the lock
	//if sessionid is blank then a new session id is created and returned in response
	if (request1 == "RELOCK") {
		newsessionid = sessionid;
	}else{
		newsessionid = "";
		for (ii = 1; ii <= 8; ++ii) {
			newsessionid ^= var("01234567890ABDCEF")[var(16).rnd() + 1];
		};//ii;
	}

	if (not(file.open(filename, ""))) {
		gosub badfile();
		return;
	}

	USER3 = "";
	if (request1 == "RELOCK") {
		gosub lockit();
	}else{
		if (lockrecord(filename, file, keyx, xx)) {
			state = 1;
		}else{
			state = 0;
		}
	}

	if (not state) {
		//response='NOT OK'
		//response='Error: CANNOT LOCK RECORD'
		call listen4(7, USER3);
		gosub addlockholder();
		return;
	}

	gosub getdostime();

	//check locks file
	lockkey = filename ^ "*" ^ keyx;
	if (lockrec.read(locks, lockkey)) {

		//handle a subsidiary lock (has master lock details)
		//very similar code in LISTEN and SHADOW.MFS
		if (lockrec.a(6)) {

			masterlockkey = lockrec.a(6).field("*", 1, 2);
			sublockrec = lockrec;

			//if masterlock is missing or doesnt have the same session id
			//then the subsidiary is considered to have expired
			if (not(lockrec.read(locks, masterlockkey))) {
				goto nolock;
			}
			if (lockrec.a(5) ne sublockrec.a(5)) {
				goto nolock;
			}

		}

		//during RELOCK the session id will normally be the same
		//unless someone else got the lock because relocking was suspended somehow
		if (lockrec.a(5) ne newsessionid) {

			if (request1 == "RELOCK") {
				goto nolock;
			}

			//fail if other lock has not timed out
			if (lockrec.a(1) > dostime) {
				USER3 = "NOT OK";
				goto lockexit;
			}

			//other lock has timed out so ok
			//no need to delete as will be overwritten below
			//delete locks,lockkey

		}

		//our own session so must be relocking (to extend timeout)

	}else{
nolock:
		if (request1 == "RELOCK") {
			//NB the word "EXPIRED" is a key word used in _formfunctions.htm
			USER3 = "Error: Your lock expired and/or somebody else updated";
			goto lockexit;
		}

		//no lock record so ok

	}

	if (masterlock) {

		//fail if masterlock is missing or doesnt have the right session id
		if (not(tt.read(locks, masterlock.field("*", 1, 2)))) {
			goto nolock;
		}
		if (tt.a(5) ne masterlock.field("*", 3)) {
			goto nolock;
		}

		//subsidiary lock gets the same sessionid as the master
		newsessionid = tt.a(5);

	}

	USER3 = "OK";

	//convert minutes to fraction of one day (windows time format)
	lockduration = lockmins / (24 * 60);

	//write the lock in the locks file
	lockrec = "";

	//similar code in lock: and write:
	lockrec.r(1, lockduration + dostime);
	lockrec.r(2, dostime);

	//lockrec<3>=if connection then connection<1,2> else @station
	if (connection) {
		lockrec.r(3, connection.a(1, 2));
	}else{
		lockrec.r(3, STATION);
	}
	lockrec.r(4, USERNAME);
	lockrec.r(5, newsessionid);
	lockrec.r(6, masterlock);
	FILEERRORMODE = 1;
	FILEERROR = "";
	USER3 = "OK";
	lockrec.write(locks, lockkey);
	if (FILEERROR) {
		call fsmsg();
		gosub geterrorresponse();
	}

	sessionid = newsessionid;

lockexit:

	//unlock file,keyx
	if (request1 == "RELOCK") {
		gosub unlockit();
	}else{
		file.unlock( keyx);
	}

	return;
}

subroutine lockit() {
	//attempt to lock the record
	//bypass ordinary lock,file,key process otherwise
	//the lock record will be checked - and in this case
	//we our own lock record to be present
	code = 5;
	nextbfs = "";
	handle = file;
	//<AREV>
	//handle=handle[-1,'B':vm]
	handle = field2(handle, VM, -1);
	//</AREV>
	keyorfilename = keyx;
	fmc = 2;
	gosub lockit2(code, nextbfs, handle, keyorfilename, fmc, state);
	return;
}

subroutine unlockit() {
	//unlock file,keyx
	code = 6;
	gosub lockit2(code, nextbfs, handle, keyorfilename, fmc, state);
	return;
}

subroutine lockit2(in code, in nextbfs, io handle, in keyorfilename, in fmc, io state) {

	call rtp57(code, nextbfs, handle, keyorfilename, fmc, xx, state);
	return;
}

subroutine badfile() {
	//response='Error: ':quote(filename):' file does not exist'
	call listen4(24, USER3, filename);
	return;
}

subroutine unlock() {

	//sessionid is used as a check that only the locker can unlock
	if (not(file.open(filename, ""))) {
		gosub badfile();
		return;
	}

	//lock file,keyx
	gosub lockit();
	if (not state) {
		//zzz perhaps should try again a few times in case somebody else
		//is trying to lock but failing because of our remote lock
		//response='Error: Somebody else has locked the record'
		call listen4(25, USER3);
		return;
	}

	//get the current lock else return ok
	lockkey = filename ^ "*" ^ keyx;
	FILEERRORMODE = 1;
	FILEERROR = "";
	if (not(lockrec.read(locks, lockkey))) {
		lockrec = "";
	}
	if (not lockrec) {
		if (FILEERROR.a(1) == 100) {
			//lock is missing but ignore it
			//because we are unlocking anyway
			USER3 = "OK";
		}else{
			gosub geterrorresponse();
		}
		goto unlockexit;
	}

	//check that the current lock agrees with the session id provided
	if (not(sessionid == lockrec.a(5))) {

		//cannot unlock because the lock belongs to somebody else
		//response='Error: Cannot unlock - '
		if (sessionid == "") {
			//response:='missing session id'
			tt = "missing";
		}else{
			//response:='wrong session id'
			tt = "wrong";
		}
		call listen4(26, USER3, tt);

		goto unlockexit;
	}

	//delete the lock
	FILEERRORMODE = 1;
	FILEERROR = "";
	USER3 = "OK";
	locks.deleterecord(lockkey);

	if (FILEERROR) {
		call fsmsg();
		gosub geterrorresponse();
	}

unlockexit:

	gosub unlockit();

	return;
}

subroutine getdostime() {
	dostime = ostime();
	//convert to Windows based date/time (ndays since 1/1/1900)
	//31/12/67 in rev date() format equals 24837 in windows date format
	dostime = 24837 + var().date() + dostime / 24 / 3600;
	return;
}

subroutine flagserveractive() {

	//flag that this dataset is being served ("listened") (needed for old MAC)
	//does not seem to cause any filesharing errors (stress tested at max speed)
	call oswrite("", inpath ^ serverflagfilename);

	return;
}

subroutine writelogx() {
	t2 = "CONVLOG";
	call listen5(t2, logx, xx, yy);
	gosub writelogx2();
	return;
}

subroutine writelogxclose() {
	logx = "</Log>";
	gosub writelogx2();
	//backup to overwrite if and when another transaction is received
	logptr -= 6;
	return;
}

subroutine writelogx2() {
	call osbwrite(logx, logfile,  logptr);
	logx = "";
	return;
}

subroutine filesecurity() {

	ok = 1;
	if (keyx.index("*")) {
		return;
	}
	if (authorised(filetitle2 ^ " " ^ secmode, msg0, "")) {
		positive = "";
	}else{
		positive = "#";
	}
	if (not(authorised(positive ^ filetitle2 ^ " " ^ secmode ^ " " ^ (keyx.quote()), posmsg))) {
		//!*use the FILENAME ACCESS/DELETE "ID" message because gives clue
		//!*that they may be allowed to access other records
		if (positive) {
			msg0.transfer(USER3);
		}else{
			posmsg.transfer(USER3);
		}
		ok = 0;
		gosub fmtresp();
	}
	return;
}

subroutine addlockholder() {
	if (tt.read(locks, filename ^ "*" ^ keyx)) {
		//if tt<6> then read tt from locks,field(tt<6>,'*',1,2) else null
		USER3 ^= ", LOCKHOLDER: " ^ (tt.a(4).quote());

	}
	return;
}

subroutine checkcompany() {
	badcomp = "";

	//quit no COMPANY_CODE in dict (or cannot determine)
	if (not allcols) {
		return;
	}
	if (not(xx.reado(allcols, filename ^ "*COMPANY_CODE"))) {
		return;
	}

	if (not(dictfile.open("DICT." ^ filename, ""))) {
		return;
	}

	if (request1.index("READ")) {
		compcode = calculate("COMPANY_CODE", dictfile, keyx, USER1, 0);
	}else{
		//compcode={COMPANY_CODE}
		//compcode=calculate('COMPANY_CODE')
		compcode = calculate("COMPANY_CODE", DICT, ID, RECORD, 0);
	}
	if (not compcode) {
		return;
	}

	if (authorised("COMPANY UPDATE " ^ (compcode.quote()), badcomp)) {
		return;
	}

	return;
}

subroutine updreqlog() {
	if (not reqlog) {
		return;
	}

	tt = username ^ FM ^ ipno ^ FM ^ netid ^ FM ^ FM ^ responsetime ^ FM ^ rawresponse.a(1);
	tt.r(11, USER0);
	logid = datasetcode ^ "*" ^ requestdate ^ "*" ^ requesttime ^ "*" ^ processno;
	tt.write(reqlog, logid);

	return;
}

libraryexit()
