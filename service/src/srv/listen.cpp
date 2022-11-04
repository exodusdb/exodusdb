#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <autorun3.h>
#include <exodus/htmllib2.h>
#include <flushindex.h>
#include <generalsubs.h>
#include <getbackpars.h>
#include <heartbeat.h>
#include <hexcode.h>
#include <listen2.h>
#include <listen3.h>
#include <listen4.h>
#include <listen5.h>
#include <log.h>
#include <log2.h>
#include <monitor2.h>
#include <openfile.h>
#include <rtp57.h>
#include <singular.h>
#include <sysmsg.h>
#include <systemsubs.h>
#include <xmlquote.h>

#include <service_common.h>

#include <srv_common.h>
#include <req_common.h>

var logfilename;
var portno;			  // num
var lastmonitortime;  // num
var lastautorun;	  // num
var halt;			  // num
var origsentence;
var processes;
var reqlog;
var defaultlockmins;  // num
var datasetcode;
var live;
//var processno;
var neopath;
var installend;
var serverend;
var databaseend;
var allcols;
var maxstrlen;	  // num
var nblocks;	  // num
var inblocksize;  // num
var outblocksize;
dim datx;
dim hexx;
var ii;	 // num
var servername;
var onserver;
var request1;
var waitfor;   // num
var waitsecs;  // num
var sleepms;   // num
var serverflagfilename;
var origsysmode;
var webpath;
var voc;
var inpath;
var listenusername;
var listenstation;
var origscrn;
var origattr;
var tracing;	  // num
var nrequests;	  // num
var timeoutsecs;  // num
var sysmode;
var origprivilege;
var locks1;
var leaselocks;
var onactive;  // num
var bakpars;
var datex;
var logpath;
var tdir;
var xx;
var logfile;
var logptr;	 // num
var tt;		 // num
var logx;
var linkfilename1;
var replyfilename;
var stack;
var lastrequestdate;
var lastrequesttime;
var breaktime;
var timenow;
var cmd;
var linkfilename0;
var linkfilenames;
var dostime;  // num
var now;
var s33;
var charx;
var buffer;
var reply;
var patched;
var dow;
var logtime;
var requestdate;
var requesttime;  // num
var nlinkfiles;
var linkfilen;	// num
var linkfile1;
var timex;
var ntries;	 // num
//var listenfailure; // num
var connection;
var datasetx;
var username;
var password;
var ipno;
var netid;
var firstrequestfieldn;	 // num
var request2;
var request3;
var request4;
var request5;
var request6;
var responsetime;  // num
var rawresponse;
var t2;
var yy;
var req6up;
var anydata;  // num
var linkfilename2;
var linkfilename3;
var linkfile2size;
var linkfile2;
var inptr;	  // num
var blockn;	  // num
var lendata;  // num
var savelogptr;
var invaliduser;
var secs;  // num
var iodatlen;
var ptr;  // num
var blk;
var filename;
var keyx;
var sessionid;
var lockmins;  // num
var masterlock;
var withlock;
var readenv;
var filetitle;
var triggers;
var postread;
var filetitle2;
var secmode;
var ok;	 // num
var file;
var keyx0;
var preread;
var autokey;  // num
var lockkeyx;
var badcomp;
var lockauthorised;	 // num
var createnotallowed;
var origresponse;
var keyx2;
var storeresponse;
var fieldno;
var prewrite;
var lockkey;
var lockrec;
var lockduration;  // num
var dictrec;
var datetimefn;
var olddatetime;
var newdatetime;
var postroutine;  // num
var replacewrite;
var postwrite;
var predelete;
var postdelete;
var voccmd;
var printfilename;
var user4x;
var printfile;
var fileerrorx;
var timeouttime;  // num
var timeoutdate;  // num
var srcfile2;
var newsessionid;
var state;	// num
var masterlockkey;
var sublockrec;
var code;  // num
var nextbfs;
var handle;
//var keyorfilename;
//var fmc; // num
var msg0;
var positive;
var posmsg;
var dictfile;
var compcode;
var logid;

var nextconnection;

function main() {

	if (main_init()) {

		// Main loop
		while (true) {

			// May call autorun
			if (not loop_init())
				break;

			gosub wait();

			// process one request file
			if (got_link()) {

				if (request_init()) {
					gosub process();
				}

				if (not request_exit())
					break;
			}

			if (!loop_exit())
				break;
		}
	}

	gosub main_exit();

	stop();

	// never gets here
	return 0;
}

function main_init() {

	if (APPLICATION ne "EXODUS") {

		// use app specific version of listen3
		listen3 = "listen3_app";

		// use app specific version of generalsubs
		generalsubs = "generalsubs_app";
	}

	logfilename = "";

	portno = 5700;
	if (SYSTEM.f(38)) {
		portno += SYSTEM.f(38) - 1;
	}
	// lastmonitortime=0
	// dont call monitor for approx 60 seconds after startup
	// to allow pressing B for backup and quit without other processes starting up
	lastmonitortime = time();
	// dont do autoruns until 1 or 2 mins after starting (to let other processes start)
	lastautorun = time() + var(60).rnd();
	// do autorun immediately on dev system
	if (var("exodus.id").osfile()) {
		lastautorun = time() - 60;
	}

	// delete any old (5min) login response pid name based files
	call listen5("DELETEOLDFILES", "*.$2", "./", 300, "");

	halt = 0;

	origsentence = SENTENCE;

	// need a file to do central locking not per database
	if (not(processes.open("PROCESSES", ""))) {
		processes = "";
	}
	if (not(reqlog.open("REQUESTLOG", ""))) {
		reqlog = "";
	}

	// lockmins is the number of minutes to retain the lock
	// default to 5 mins. lock extension is done every 5/1.1 mins by the user interface
	defaultlockmins = 5;

	datasetcode = SYSTEM.f(17);
	live		= not datasetcode.ucase().ends("_test");
	// processno = SYSTEM.f(24);

	neopath = "../exodus/";
	neopath.converter("/", OSSLASH);
	installend = "global.end";
	serverend  = "../../" ^ SYSTEM.f(123).lcase() ^ ".end";
	serverend.converter("/", OSSLASH);
	databaseend = datasetcode.lcase() ^ ".end";

	if (not(allcols.open("ACCESSIBLE_COLUMNS", ""))) {
		allcols = "";
	}

	// prepare for sending and receiving escaped iodat > 64Kb
	inblocksize	 = 1048576;
	maxstrlen	 = inblocksize * 4;
	outblocksize = (maxstrlen / 3).floor();
	nblocks		 = 1;

	datx.redim(nblocks);

	// ///
	// init:
	// ///

	servername = "";
	onserver   = servername eq "" or STATION.trim() eq servername;

	request1 = "";

	// should be less than 25 unless waiting.exe modified to update the server flag
	waitfor	 = 1;
	waitsecs = 10;

	if (onserver) {
		sleepms = 10;
	} else {
		sleepms = 100;
	}

	// make up a serverflagfilename
	serverflagfilename = datasetcode ^ ".svr";

	origsysmode = SYSTEM.f(33);

	// webpath=field(origsentence,' ',3)
	webpath = "";
	if (not webpath) {
		webpath =
			"../"
			"data/";
	}
	if (not webpath.ends("/")) {
		webpath ^= "/";
	}
	webpath.converter("/", OSSLASH);
	if (not(voc.open("VOC", ""))) {
		call mssg(lasterror());
		// gosub main_exit();
		return false;
	}

	inpath = webpath ^ datasetcode ^ "/";
	inpath.converter("/", OSSLASH);

	// set time before calling deleteoldfiles which is time sensitive
	// call settime(webpath[1,2])
	call listen5("DELETEOLDFILES", "*.*", inpath);

	listenusername = USERNAME;
	listenstation  = STATION;

	// call savescreenorigscrn, origattr);
	// print @aw<30>:

	// tracing=(@username='EXODUS' or trim(@station)='SBCP1800')
	// tracing=(not(index(origsentence,'AUTO',1)))

	// if @username='DEMO' then tracing=0
	// tracing=0
	tracing = 1;

	// ensure unique sorttempfile
	// if sysvar('SET',192,102,'R':('0000':THREADNO)[-5,5]:'.SFX') else null
	// call sysvar_192_102"SET", "R" ^ ("0000" ^ THREADNO).last(5) ^ ".SFX");

	nrequests = SYSTEM.f(35) + 0;

	printl(var("-").str(50));
	printl(THREADNO ^ ":", "EXODUS", datasetcode, oscwd());
	printl(var("-").str(50));

	SYSTEM(33) = 1;

	timeoutsecs = SYSTEM.f(33, 5);
	if (not(timeoutsecs.isnum())) {
		timeoutsecs = "";
	}
	// if timeoutsecs='' then timeoutsecs=20*60;*ie 20 minutes

	// force into server mode - suppress interactive messages
	sysmode = origsysmode;
	if (sysmode eq "") {
		sysmode	   = SYSTEM.f(33);
		SYSTEM(33) = "1";
	}

	origprivilege = PRIVILEGE;
	// call setprivilege"");

	// image=''

	if (not(openfile("LOCKS", locks1, "DEFINITIONS", 1))) {
		call mssg(lasterror());
		// gosub main_exit();
		return false;
	}

	gosub gettimeouttime();

	gosub flagserveractive();

	onactive = 0;

	call getbackpars(bakpars);

	// open an XML log file
	datex = date().oconv("D.");
	// if 1 then
	logpath = ("../logs/" ^ datasetcode ^ "/" ^ datex.last(4)).lcase();
	logpath.converter("/", OSSLASH);

	// check/make the dataset folder
	// subdirs=''
	// call subdirs('../logs/':datasetcode:char(0),subdirs)
	// if count(subdirs,fm) else
	tdir = "../logs/" ^ datasetcode;
	tdir.converter("/", OSSLASH);
	if (not(xx.osopen(tdir.lcase()))) {
		if (STATUS ne 2) {
			//call osmkdir(tdir.lcase());
			if (not osmkdir(tdir.lcase()))
				abort(lasterror());
		}
	}

	// check/make the annual log folder
	// call shell('MD ':logpath)
	if (not(xx.osopen(logpath))) {
		if (STATUS ne 2) {
			//call osmkdir(logpath);
			if (not osmkdir(logpath))
				abort(lasterror());
		}
	}

	logfilename = logpath ^ "/" ^ datex.last(2) ^ datex.first(2) ^ datex.b(4, 2) ^ ("00" ^ THREADNO).last(2);
	logfilename ^= ".xml";
	logfilename.converter("/", OSSLASH);

	if (logfile.osopen(logfilename)) {

		// append the logfile
		logptr = logfilename.osfile().f(1);

		// if the last bit closes the log, overwrite it to continue the log
		// t2=logptr-6
		// call osbread(tt,logfile,logfilename,t2,6)
		// if tt='</Log>' then logptr=t2

		// backup and read the last 6 characters of the log
		logptr -= 6;
		//call osbread(tt, logfile, logptr, 6);
		if (not osbread(tt, logfile, logptr, 6))
			abort(lasterror());
		// maybe backup the log pointer to overwrite the closing tag
		if (tt eq "</Log>") {
			logptr -= 6;
		}

	} else {

		// initialise xml log file header
		//call oswrite("", logfilename);
		if (not oswrite("", logfilename))
			abort(lasterror());
		if (logfile.osopen(logfilename)) {
			logptr = 0;
			logx   = "<?xml version=\"1.0\"?>" _EOL;
			logx ^= "<Log>" _EOL;
			gosub writelogx2();

			gosub writelogxclose();

		} else {
			// print 'CANNOT OPEN LOG FILE ':logfilename
			logfilename = "";
		}
	}

	linkfilename1 = "";
	replyfilename = "";

	// save the current program stack
	// limit on 299 "programs" and dictionary entries count as 1!!!
	// see "rev stack and rtp documentation.htm"
	// and http://www.revelation.com/knowledge.nsf/07dbcbabb6b3e379852566f50064cf25/daca1813e510f571852563b6006a9b9c?OpenDocument
	stack = "";
	// call programstackstack);

	//nextconnection.connect(getenv("EXO_DATA"));
	if (not nextconnection.connect(getenv("EXO_DATA")))
		loglasterror();

	return true;

}  // main_init

function loop_init() {

	// disconnect any connections added while processing requests
	nextconnection.disconnectall();

	lastrequestdate = date();
	lastrequesttime = time();
	req.registerx	= "";  //dim

	// forcedemail
	SYSTEM(117) = "";

	// no interruptfilename
	SYSTEM(6) = "";

	TCLSTACK = TCLSTACK.field(FM, 1, 10);
	/*
	execute("RESET");
	*/
	SYSTEM(33)	= 1;
	RECORD		= "";
	ID			= "";
	MV			= 0;
	request_	= "";
	data_		= "";
	response_	= "";
	msg_		= "";
	req.wlocked = "";

	// linkfilename1=''

	// nextsearch:
	// /////////

	// clear any file handles
	var().osflush();

	// on win95 this runs at same speed in full screen or windowed

	locks1.unlock("REQUEST*" ^ linkfilename1);
	locks1.unlock("REQUEST*" ^ replyfilename);
	// print 'unlock locks,REQUEST*':linkfilename1
	// print 'unlock locks,REQUEST*':replyfilename
	call listen5("UNLOCKLONGPROCESS");

	breaktime = "X";

nextsearch0:
	// //////////

	if (TERMINATE_req or RELOAD_req) {
		printl("Closing THREADNO " ^ THREADNO);
		return false;
	}

	var().cleardbcache();

	// restore the program stack
	// call programstackstack);

	// clear out @EW, @PW, @VW and @XW globals for PER REQUEST GLOBAL BUFFERS

	// validcode3
	// equ codebuff to @ew   edit win?
	// equ authbuff to @pw   popup win?
	EW = EW.field(FM, 1, 9);
	PW = PW.field(FM, 1, 9);

	// validcode2
	// equ codebuff to @vw vol win?
	// equ authbuff to @xw xref win?
	VW = VW.field(FM, 1, 9);
	// @vw<10>=@username
	XW = XW.field(FM, 1, 9);

	// ATTACH UTILITY, ED UBP * for documentation on @VW etc

	// @MW menu, @HW help FREE FOR USE?
	MW = MW.field(FM, 1, 9);
	HW = HW.field(FM, 1, 9);

	// @AW application window
	// print @aw<30>
	AW = AW.field(FM, 1, 30);

	// @SW SQL win @sw<1> <2> <3> see init.general timezone

	// lets clear some standard stuff too
	ID	   = "";
	RECORD = "";
	DICT   = "";
	MV	   = 0;

	// print time() '[TIME2,MTS]':
	// similar in LISTEN and AUTORUN
	tt = time().oconv("MTS") ^ " " ^ datasetcode ^ " " ^ THREADNO ^ " " ^ nrequests ^
		 " Listening"
		 " " ^
		 elapsedtimetext(lastrequestdate.timestamp(lastrequesttime), timestamp());
	//var(tt).oswrite("process." ^ THREADNO);
	if (not var(tt).oswrite("process." ^ THREADNO))
		loglasterror();

	call unlockrecord("PROCESSES", processes, THREADNO);

	// prevent sleep in esc.to.exit
	timenow	   = ostime();
	SYSTEM(25) = timenow;
	SYSTEM(26) = timenow;

	// check for a variety of reasons to restart including
	// corrupt system record
	// changed system.cfg or ../../system.cfg
	// recompiled $LISTEN
	call listen5("CHECKRESTART");
	if (ANS) {
		request1 = ANS;
		// gosub main_exit();
		// stop();
		return false;
	}

	// start *.RUN commands (start processes)
	call listen5("RUNS");

	// update process status and quit if global.end ../../global.end or database.end
	// unassigned xx means trigger check for quitting
	call heartbeat("CHECK");

	gosub flagserveractive();

	// run autorun, syncdata and clear old files once a minute
	if ((time() - lastautorun gt 60) or time() lt(lastautorun - 600)) {
		lastautorun = time();

		// call autorun instead of perform to allow output to remain on screen
		// (BUT errors cause listen to crash and restart)
		if (not(var("autorun.end").osfile())) {
			call autorun3();
		}

		// run scheduled reports etc except on test data (ie not backed up)
		// if not(system<61>) or dir("exodus.id") then
		// if system<121> then call syncdata
		// end

		// delete old response and temp files every 1 minute
		call listen5("DELETEOLDFILES2", "", inpath);

		// autorun might have run so avoid processing another request without resetting
		goto nextsearch0;
	}

	return true;

}  // loop_init

subroutine wait() {

	// if index(drive(),':',1) then exe='.exe' else exe=''
	// cmd='WAITING.EXE'
	cmd = "waiting";
	// fast exit on server.end and install.end. database.end only checked every 10 secs
	cmd ^= " " ^ inpath ^ "*.1 " ^ waitsecs ^ " " ^ sleepms ^ " " ^ serverend;

	linkfilename1 = inpath ^ "neos" ^ THREADNO.oconv("R(0)#4") ^ ".0";
	//linkfilename1.osremove();
	if (osfile(linkfilename1) and not linkfilename1.osremove())
		abort(lasterror());

	if (linkfilename1.osfile()) {
		if (tracing) {
			printl("CANNOT DELETE ", linkfilename1, " GENERATING ANOTHER");
			linkfilename1 = var(99999999).rnd() ^ ".0";
		}
	}
	linkfilename0 = linkfilename1.cut(inpath.len());
	cmd ^= " " ^ linkfilename0 ^ " " ^ inpath ^ " " ^ portno;
	cmd.lcaser();
	cmd.converter("/", OSSLASH);

	// remove lock indicating processing (hangs if unlock and not locked)
	// gosub sysunlock
	// print cmd
	// break off
	// unfortunately cant use this because on SOME datasets?!?!?
	// eg TESTMARK/BMTEST (MARKONE/BMDATA runs!!!!) both MASTERBR and MASTTEST dont
	// on nl1 it will not run the waiting.exe command
	// solved by removing path before WAITING.EXE above (unknown cause though)
	perform(cmd);

	// quit if connection no longer active. postgres restarted/stopped.
	if (not var().sqlexec("SELECT NOW()"))
		logoff();

	// place a lock to indicate processing
	// should really retry in case blocked by other processes checking it
	// call rtp57(syslock, '', '', trim(@station):THREADNO, '', '', '')
	if (not lockrecord("PROCESSES", processes, THREADNO, "", 999999))
		logoff();

	// pause forever while any quiet time process (eg hourly backup) is working
	// maybe not necessary on test data
	if (live) {
		if (lockrecord("PROCESSES", processes, "ALL", "", 999999)) {
			processes.unlock("ALL");
		}
	}

	return;

}  // wait

function loop_exit() {

	// timeout if no activity
	dostime = ostime();
	now		= (date() ^ "." ^ dostime.floor().oconv("R(0)#5")) + 0;
	// if timeouttime and now>timeouttime then gosub exit

	gosub flagserveractive();

	// switch into interactive mode to check for operator input
	s33		   = SYSTEM.f(33);
	SYSTEM(33) = "";

	// check for esc key to exit
	// if esc.to.exit() then gosub exit

	charx.inputn(-1);
	// charx=ucase(charx[1,1])
	// charx=charx[1,1]

	// esc or "q" on linux
	tt = INTCONST.f(1, 1);
	charx.lcaser();
	if (charx.contains(tt)) {
		// leading space to avoid chars after ESC pressed being ANSI control sequences
		tt.replacer(chr(27), "Esc");
		call mssg("You have pressed the " ^ tt ^ " key to exit|press again to confirm|", "UB", buffer, "");
		// loop
		// input reply,-1:
		// until reply
		// call ossleep(1000*1)
		// repeat
		echo(0);
		reply.inputn(1);
		reply.lcaser();
		echo(1);
		call mssg("", "DB", buffer, "");
		if (reply eq INTCONST.f(1)) {
			// space to defeat ANSI control chars after pressing Esc
			print(" ");
			// gosub main_exit();
			return false;
		}
		// "x"
		if (reply eq INTCONST.f(7)) {
			charx = reply;
		}
	}

	// R=Restart
	// if charx='R' then
	// request1='RESTART RPRESSED'
	// gosub exit
	// end

	// f5 or 'x' on linux
	if (charx.lcase() eq PRIORITYINT.f(2)) {
		cmd = "";
		cmd.input(oscwd() ^ " Command? ");
		SYSTEM(2) = "";

		// Check if something like ~/lib/libxxxxxxxx.so exists where xxxxxxxx is the first word of the command
		if (not libinfo(cmd.field(" "))) {
			errputl(cmd.field(" "), " does not exist.");

		} else {
			try {

				// Execute the command as user EXODUS
				var username = USERNAME;
				USERNAME	 = "EXODUS";

				if (not cmd.starts("list")) {
					printl("Begin transaction");
					//begintrans();
					if (not begintrans())
						abort(lasterror());
				}

				// Manual commands
				// ////////////////////
				var ok = execute(cmd);
				// ////////////////////

				if (statustrans()) {
					if (decide("Commit any database updates ?", "Commit" _VM "Rollback") eq "Commit") {
						print("Committing transaction ... ");
						//committrans();
						if (not committrans())
							loglasterror();
						printl("done.");
					} else {
						print("*ROLLING BACK* transaction ... ");
						//rollbacktrans();
						if (not rollbacktrans())
							loglasterror();
						printl("done.");
					}
				}

				USERNAME = username;

			} catch (VarError& e) {
				errputl(e.description);
			}
		}

		osflush();
		return true;
	}

	if (charx == "\n") {
		printl();
		return true;
	}

	// 	// f10 or ? on linux
	// 	if (charx eq INTCONST.f(7)) {
	// 		// execute("RUNMENU " ^ ENVIRONSET.f(37));
	// 		execute("RUNMENU");
	// 		return true;
	// 	}

	if (charx ne "") {

		// "U" = unlock all locks
		if (charx eq "U") {
			//clearfile(locks1);
			if (not clearfile(locks1))
				abort(lasterror());
			printl(" ", "ALL LOCKS RELEASED");
		}

		gosub gettimeouttime();
		return true;
	}

	// switch back to not interactive mode
	// //////////////////////////////////
	SYSTEM(33) = s33;

	// gosub getbakpars
	call getbackpars(bakpars);

	// call monitor approx every minute +/- 10 seconds to avoid checking all the time
	if ((time() - lastmonitortime).abs() gt 60 + var(20).rnd() - 10) {
		// if abs(time()-lastmonitortime)>(0+rnd(20)-10) then

		// monitor updates nagios and optionally checks for upgrades
		call monitor2();
		lastmonitortime = time();

		// install and run patches
		call listen5("PATCHANDRUNONCE", live, processes);
		patched = ANS;

		if (patched) {
			request1 = "RESTART PATCHED";

			// gosub main_exit();
			return false;
		}

		// monitor might have run so avoid processing another request without resetting
		// goto nextsearch0;
		return true;
	}

	// run autorun, syncdata and clear old files once a minute
	// has been moved up so it can run immediately after %newautorun%

	// A forces autorun
	if (charx eq "A") {
		lastautorun = "";
	}

	// backup
	if (charx and var("Bb").contains(charx)) {
		goto backup;
	}
	if (time() ge bakpars.f(3) and time() le bakpars.f(4)) {

		// call log2('LISTEN: Backup time for ':datasetcode,logtime)

		// delay closedown randomly to avoid conflict with identically configured processes
		call ossleep(1000 * var(10).rnd());

		dow = date().oconv("DW");

		// optionally perform backup and/or shutdown and not backed up today
		if (bakpars.f(9)) {
			// call log2('Backup is disabled',logtime)

		} else if (date() eq bakpars.f(1)) {
			// call log2('Backup already done today',logtime)

		} else if (bakpars.f(11)) {
			call log2("backup is suppressed. Quitting.", logtime);
			perform("OFF");
			logoff();

		} else if (not(bakpars.f(5).contains(dow))) {
			call log2("Not right day of week " ^ bakpars.f(5) ^ " Logging off", logtime);
			perform("OFF");
			logoff();

		} else {
			// call log2('Preventing further automatic backups today',logtime)
			((date() + time() / 86400).oconv("MD50P")).writef(DEFINITIONS, "BACKUP", 1);

backup:
			// similar code in LISTEN and LISTEN2
			msg_ = "";
			cmd	 = "FILEMAN BACKUP " ^ datasetcode ^ " " ^ bakpars.f(7) ^ " SYSTEM";
			call log2("Attempting backup " ^ cmd, logtime);
			perform(cmd);

			// quit and indicate to calling program that a backup has been done
			PSEUDO = "BACKUP";
			// B=BACKUP=backup/copydb/upgrade/quit
			// b=BACKUP2=backup, send email and resume -ie dont copydb/upgrade/quit
			if (charx eq "b") {
				PSEUDO ^= "2";
			}
			PSEUDO ^= " " ^ bakpars.f(7) ^ " " ^ bakpars.f(12);
			if (msg_) {
				// stop();
				return false;
			}
		}
	}

	call listen5("DELETEOLDFILES", "*.*", inpath, "", "");

	return true;

}  // loop_exit()

subroutine main_exit() {

	if (logfilename) {
		logfile.osclose();
	}

	// remove lock indicating processing
	// gosub sysunlock

	call listen5("UNLOCKLONGPROCESS");

	var().unlockall();

	// call restorescreenorigscrn, origattr);

	// remove flag that this dataset is being served ("listened")
	// osremove inpath:serverflagfilename

	// get into interactive mode
	// system<33>=origsysmode
	SYSTEM(33) = "";
	// call setprivilegeorigprivilege);
	if (request1.starts("RESTART")) {
		msg_ = request1;
		// return to net which will restart LISTEN
		// stop();
		return;
	}

	// esc does this
	if ((origsysmode or request1 eq "STOPDB") or halt) {
		perform("OFF");
		logoff();
	}

	// msg is @user4
	msg_ = "TERMINATED OK";

	// stop();
	return;
}

function got_link() {

	if (not linkfilename1.osfile())
		return false;

	linkfilenames = linkfilename0;

	// get the earliest time possible for the log
	requestdate = date();
	requesttime = ostime();
	SYSTEM(25)	= requesttime;

	gosub gettimeouttime();

	nlinkfiles = linkfilenames.fcount(FM);

	// find a request to process
	for (linkfilen = 1; linkfilen <= nlinkfiles; ++linkfilen) {

		linkfilename1 = inpath ^ linkfilenames.f(linkfilen);

		// lock it to prevent other listeners from processing it
		// unlock locks,'REQUEST*':linkfilename1
		if (not(lockrecord("", locks1, "REQUEST*" ^ linkfilename1, xx))) {
			if (tracing) {
				printl("CANNOT LOCK LOCKS,", ("REQUEST*" ^ linkfilename1).quote());
			}
			continue;
		}
		// print 'lock locks,REQUEST*':linkfilename1

		var().osflush();
openlink1:
		if (not(linkfile1.osopen(linkfilename1))) {
			// remove from future candidate files?
			call ossleep(1000 * 1 / 10.0);
			if (tracing) {
				printl("CANNOT OPEN RW ", linkfilename1.quote());
			}
			continue;
		}

		// get the .1 file which contains the request
		timex = time();
		// readlink1:
		request_ = "";
		// osbread request from linkfile1 at 0 length 256*256-4
		tt = 0;
		//call osbread(request_, linkfile1, tt, 256 * 256 - 4);
		if (not osbread(request_, linkfile1, tt, 256 * 256 - 4))
			abort(lasterror());

		// if cannot read it then try again
		if (request_ eq "" and time() eq timex) {
			var().osflush();
			call ossleep(1000 * 1 / 10.0);
			linkfile1.osclose();
			// 			if (not(linkfile1.osopen(linkfilename1))) {
			// 				{}
			// 			}
			// 			goto readlink1;
			goto openlink1;
		}

		// cleanup the input file
		// convert '&' to fm in request
		request_.converter("\r\n", _FM);
		request_.replacer("\\\\", "\\");
		request_.replacer("\\r", _FM);
		request_.trimmerlast(_FM);

		// replyfilename=ucase(request<1>)
		// eg D:\EXODUS\DATA\DEVDTEST\|3130570.1
		// eg /var/www/html/exodus2/EXODUS//data/BASIC/~9979714.1
		replyfilename = request_.f(1);
		request_.remover(1);

		// lock the replyfilename to prevent other listeners from processing it
		// unlock locks,'REQUEST*':replyfilename
		if (not(lockrecord("", locks1, "REQUEST*" ^ replyfilename, xx))) {
			// if tracing then print 'CANNOT LOCK LOCKS,':quote('REQUEST*':replyfilename)
			continue;
		}

		// print 'lock locks,REQUEST*':replyfilename

		// delete then unlock the request file
		ntries = 0;
deleterequest:
		linkfile1.osclose();
		//linkfilename1.osremove();
		if (osfile(linkfilename1) and not linkfilename1.osremove())
			abort(lasterror());
		if (linkfilename1.osfile()) {
			var().osflush();
			call ossleep(1000 * 1 / 10.0);
			ntries += 1;
			// if tracing then print 'COULD NOT DELETE ':linkfile1
			if (ntries lt 100) {
				goto deleterequest;
			}
			if (tracing) {
				printl("COULD NOT DELETE ", linkfile1);
			}
		}

		// leave these in place for the duration of the process
		// they should be cleared by unlock all somewhere at the end or beginning
		// unlock locks,'REQUEST*':replyfilename
		// unlock locks,'REQUEST*':linkfilename1

		// found a good one so process it
		// gosub onerequest();
		return true;

	}  // linkfilen;

	return false;

}  // got_link

function request_init() {

	nrequests += 1;

	// similar in listen and log2
	print(THREADNO ^ ": ");

	// clear out buffers just to be sure
	data_ = "";
	call listen4(1, response_);
	msg_ = "";

	// only go on active if we actually get the request
	// to avoid multiple processes going on active for only one request
	onactive = 0;

	var request_init_ok = true;

	// cut off the initial connection info fields
	tt = request_.f(1);
	if (tt eq "VERSION 3") {
		// nconnectionfields
		tt = request_.f(2) - 1;
		request_.remover(2);
	} else if (request_.f(1) eq "VERSION 2") {
		tt = 4;
	} else {
		tt = 0;
	}
	connection = request_.field(FM, 1, tt);

	request_ = request_.field(FM, tt + 1, 999999);

	datasetx = request_.f(1).ucase();
	username = request_.f(2).ucase();
	password = request_.f(3).ucase();

	// REMOTE_ADDR, REMOTE_HOST, HTTPS, SESSIONID
	// if connection<1>='::1' then connection<1>='127.0.0.1'
	connection.replacer(FM ^ "::1", FM ^ "127.0.0.1");
	connection.converter(FM, VM);
	connection(1, 10) = username;
	SYSTEM(40)		  = connection;
	ipno			  = connection.f(1, 2);
	netid			  = connection.f(1, 5);

	firstrequestfieldn = 4;
	request1		   = request_.f(firstrequestfieldn);

	// throw away initial request CACHE
	if (request1 eq "CACHE") {
		request_.remover(firstrequestfieldn);
		request1 = request_.f(firstrequestfieldn);
	}

	request1.ucaser();
	if (request1 eq "LOGIN") {
		request_(5) = request_.f(2);
	}
	// convert @lower.case to @upper.case in request
	request2 = request_.f(5);
	request3 = request_.f(6);
	request4 = request_.f(7);
	request5 = request_.f(8);
	request6 = request_.f(9);

	// remove dataset and password
	request_ = request_.field(FM, firstrequestfieldn, 99999);

	responsetime = "";
	rawresponse	 = "PENDING";
	gosub updreqlog();

	if (logfilename) {

		datex = date();
		timex = requesttime;

		tt = "<Message ";
		tt ^= " Date=" ^ xmlquote(datex.oconv("D"));
		tt ^= " Time=" ^ xmlquote(timex.oconv("MTS"));
		tt ^= " DateTime=" ^ xmlquote(datex.oconv("D") ^ "T" ^ timex.oconv("MTS") ^ "." ^ timex.field(".", 2));
		tt ^= " User=" ^ xmlquote(username);
		tt ^= " File=" ^ xmlquote(field2(replyfilename, OSSLASH, -1));
		// REMOTE_ADDR REMOTE_HOST HTTPS
		tt ^= " IP_NO=" ^ xmlquote(connection.f(1, 2));
		tt ^= " Host=" ^ xmlquote(connection.f(1, 3));
		tt ^= " HTTPS=" ^ xmlquote(connection.f(1, 4));
		tt ^= " Session=" ^ xmlquote(netid);
		tt ^= ">" _EOL;

		tt ^= "<Request ";

		logx = request_;
		// gosub convlogx
		t2 = "CONVLOG";
		call listen5(t2, logx, xx, yy);
		logx.converter("^", FM);
		logx(1) = request1;
		if (logx.f(1)) {
			tt ^= " Req1=" ^ (logx.f(1).quote());
		}
		if (logx.f(2)) {
			tt ^= " Req2=" ^ (logx.f(2).quote());
		}
		if (logx.f(3)) {
			tt ^= " Req3=" ^ (logx.f(3).quote());
		}
		if (logx.f(4)) {
			tt ^= " Req4=" ^ (logx.f(4).quote());
		}
		if (logx.f(5)) {
			tt ^= " Req5=" ^ (logx.f(5).quote());
		}
		req6up = logx.field(FM, 6, 9999);
		req6up.converter(FM, "^");
		if (req6up.len()) {
			tt ^= " Req6up=" ^ (req6up.quote());
		}

		tt ^= "/>";
		tt.move(logx);
		gosub writelogx2();
	}

	anydata = 0;

	if (request2 eq "JOURNALS") {
		request2 = "JOURNALS";
		if (request3.count("*") eq 3 and (request3.ends("*"))) {
			request3.popper();
		}
	}

	// print the request
	if (tracing) {
		tt = username ^ " " ^ request_;
		// hide dataset and passord
		// has been removed above now
		tt.converter(FM, " ");

		t2 = connection.f(1, 2);
		if (connection.f(1, 3) ne t2) {
			t2 ^= " " ^ connection.f(1, 3);
		}

		print(t2.oconv("L#15"), tt, ":");
	}

	linkfilename2 = replyfilename;
	linkfilename2.paster(-1, 1, "2");
	linkfilename3 = replyfilename;
	linkfilename3.paster(-1, 1, "3");

	// save the response file name
	// so that if listen fails then NET the calling program can still respond
	PRIORITYINT(100) = linkfilename3;

	linkfile2size = linkfilename2.osfile().f(1);
	if (linkfile2size gt maxstrlen) {

		if (linkfile2.osopen(linkfilename2)) {

			// read blocks of iodat
			datx  = "";	 //dim
			inptr = 0;
			for (blockn = 1; blockn <= nblocks; ++blockn) {

				// osbread datx(blockn) from linkfilename2 at ((blockn-1)*inblocksize) length inblocksize
				// tt=(blockn-1)*inblocksize
				//call osbread(datx(blockn), linkfile2, inptr, inblocksize);
				if (not osbread(datx(blockn), linkfile2, inptr, inblocksize))
					abort(lasterror());

				if (not(datx(blockn).len()))
					break;

				// avoid hexcode spanning block end by moving one or two bytes backwards
				if (blockn gt 1) {
					tt = ((datx(blockn - 1)).last(2)).index("%");
					if (tt) {
						datx(blockn - 1) ^= datx(blockn).first(tt);
						datx(blockn).cutter(tt);
					}
				}

			}  // blockn;

			// unescape all blocks
			lendata = 0;
			for (blockn = 1; blockn <= nblocks; ++blockn) {

				if (datx(blockn).len()) {

					// output to log before unescaping since log is xml
					if (logfilename) {

						logx = "";

						// log <DataIn>
						if (not(anydata)) {
							anydata = 1;
							logx ^= _EOL "<DataIn>";
						}

						logx ^= datx(blockn);
						gosub writelogx();
					}

					lendata += datx(blockn).len();
				}

			}  // blockn;

			// check max iodat size not greater than maxstrlen
			if (lendata gt maxstrlen) {
				data_ = "";
				// response='Error: Maximum record size of ':maxstrlen '[XBYTES]':' exceeded in LISTEN'
				call listen4(2, response_, maxstrlen);
				request_init_ok = false;

				// otherwise join the blocks
			} else {
				data_ = "";
				for (blockn = 1; blockn <= nblocks; ++blockn) {
					data_ ^= datx(blockn);
					datx(blockn) = "";
				}  // blockn;
			}

			// cannot open linkfilename2 means no iodat
		} else {
cannotopenlinkfile2:
			request_init_ok = false;
			data_			= "";
			// response='Error: LISTEN cannot open ':linkfilename2
			call listen4(3, response_, linkfilename2);
		}

	} else {

		if (not(linkfile2size)) {

			data_ = "";

		} else {

			if (not(linkfile2.osopen(linkfilename2))) {
				goto cannotopenlinkfile2;
			}

			// read whole file upto limit
			var	 offset_zero = 0;
			//call osbread(data_, linkfile2, offset_zero, maxstrlen);
			if (not osbread(data_, linkfile2, offset_zero, maxstrlen))
				abort(lasterror());

			// output to log
			if (logfilename) {

				// start after the last <DataIn>
				if (not anydata) {
					anydata = 1;
					logx	= _EOL "<DataIn>";
					gosub writelogx();
				}

				logx = data_;
				gosub writelogx();
			}
		}
	}

	if (linkfile2.osopen(linkfilename2)) {
		linkfile2.osclose();
		//linkfilename2.osremove();
		if (osfile(linkfilename2) and not linkfilename2.osremove())
			abort(lasterror());
		// osread response from linkfilename3 else response=''
	}

	if (logfilename) {

		if (anydata) {
			logx = "</DataIn>";
			gosub writelogx2();
		}

		savelogptr = logptr;
		logx	   = _EOL "</Message>" _EOL "</log>";
		gosub writelogx2();
		logptr = savelogptr;
	}

	// update security table every few secs and every login
	// if request1='LOGIN' or index('036',time()[-1,1],1) then gosub getsecurity
	if (var newsecurity; newsecurity.read(DEFINITIONS, "SECURITY")) {
		SECURITY = newsecurity;
	}

	// validate username and password ('LOGIN' gets special treatment)
	// and assume that identity if ok
	// invaliduser will be "" if valid and a response message if not
	call listen2("VALIDATE" ^ FM ^ request1, username, password, connection, invaliduser, datasetx);

	SYSTEM(2) = linkfilename2;

	RECORD = "";
	ID	   = "";
	MV	   = 0;

	return request_init_ok;
}

subroutine process() {

	// Call process2 wrapped inside a transaction

	// ACID Transactions and record locking
	// ------------------------------------
	//
	// ACID = atomic, consistent, isolated, and durable.
	//
	// Exodus is currently using Postgresql Isolation Level "Read Committed"
	// https://www.postgresql.org/docs/12/transaction-iso.html
	//
	// TODO consider using stricter Postgreql isolation levels. These may require resubmitting transactions in case of concurrent updates.
	//
	// The application programmer must use locks to stop any other transaction from starting that may update
	// any of the records that are going to be read or updated. Otherwise transaction commit may fail?
	//
	// As long as locks are used as above then transactions are only necessary to rollback partial updates in case of program errors mid updating.
	//
	// It is up to the application programmer to perform steps 2 and 3 strictly in order otherwise data inconsistency may occur.
	//
	// 1. Transaction BEGIN
	// 2. "lock" ALL records to be read or updated (new/amend/delete)
	//    - These are logical locks per database.
	//    - If any lock cannot be obtained (Reasonable waiting is acceptable) then STEP 3 MUST BE OMITTED.
	// 3. Read/Update all records as required.
	// 4. Transaction COMMIT (or ROLLBACK if any error)
	//    - All writes appear simultaneously in the database
	//    - All logical locks are removed automatically afterwards
	//    - Commit may fail if any conflicting updates occur. Due to failure to implement steps 2 and 3?
	//
	// Notes:
	//
	// Locks:
	//
	// 1. All "locks" remain in place until the transaction is committed or rolled back. Unlocking cannot be done except in transaction commit/rollback.
	// 2. It is critical to obtain ALL locks required before reading/updating ANY record. OTHERWISE TRANSACTION IS NOT ACID.
	//    You MUST stop any other transaction from starting that may read and update any of the records you are going to read or update.
	// 3. Locks are nominally per file and record but actually are logical and by agreement can be anything suitable.
	// 4. Locks are logical and do not prevent updates. i.e. they are by agreement, not enforced by the database.
	// 5. The "locks" file actually holds "leases" and does not represent the logical locks discussed here.
	//
	// Data visibility:
	//
	// 1. No writes are visible to other processes until transaction is committed
	// 2. All reads see records that have been committed by other transactions up to the time of the read operation.

	// Postgresql lock types
	//
	// Exodus does not use postgres table, page or row locks. It uses "advisory" locks.
	// Within transactions Exodus uses pg_try_advisory_xact_lock()
	// Outside transactions Exodus uses pg_try_advisory_lock()
	// Summary: https://www.postgresql.org/docs/12/functions-admin.html#FUNCTIONS-ADVISORY-LOCKS
	// Details: https://www.postgresql.org/docs/12/explicit-locking.html#ADVISORY-LOCKS

	// Process the request - inside a transaction or not
	// ////////////////////

	//  ISOLATION LEVEL READ COMMITTED

	// Without try/catch so will break into debugger
	if (osgetenv("EXO_DEBUG")) {

		// Identical below
		//begintrans();
		if (not begintrans())
			abort(lasterror());
		gosub process2();
		if (not committrans())
			response_ = "Error: Cannot commit " ^ var().lasterror() ^ FM ^ response_;
	}

	// With try/catch so errors can be dealt with properly
	else
		try {

			// Identical above
			//begintrans();
			if (not begintrans())
				abort(lasterror());
			gosub process2();
			if (not committrans())
				response_ = "Error: Cannot commit " ^ var().lasterror() ^ FM ^ response_;

		} catch (VarError& e) {
			//rollbacktrans();
			if (not rollbacktrans())
				loglasterror();
			// Similar code in net.cpp and listen.cpp
			response_ = e.description.unassigned("No error message") ^ FM ^ e.stack();
		}

	return;

}  // process

subroutine process2() {

	// process the input
	// /////////////////

	// 	// failure in LISTEN above
	// 	if (listenfailure) {
	//
	// 	// invalid username or password or connection
	// 	} else

	if (invaliduser) {
		data_	  = "";
		response_ = invaliduser;

		// if no request then possibly some failure in file sharing
	} else if (request_ eq "") {
		data_ = "";
		// response='Error: No request'
		call listen4(4, response_);

		// empty loopback to test connection (no username or password required)
	} else if (request1 eq "TEST") {
		// iodat='' if they pass iodat, pass it back
		response_ = "OK";

		// get cookies for company, market, menus etc (may fail if no allowed companies)
		// username and password already validated above
	} else if (request1 eq "LOGIN") {

		call listen2(request1, datasetx, username, connection, request5);

		// find index values
		// case request1[1,14]='GETINDEXVALUES'
	} else if (request1.starts("GETINDEX")) {

		// call listen3(request2,'GETINDEXVALUES')
		call listen5(request1, request2, request3, request4, request5, request6);

		// select some data
	} else if (request1 eq "SELECT") {

		// call listen3(request2,request1)
		call listen5(request1, request2, request3, request4, request5, request6);

		// lock a record
	} else if (request1 eq "LOCK" or request1 eq "RELOCK") {

		call listen3(request2, request1);

		filename  = request2;
		keyx	  = request3;
		sessionid = request4;

		// lockduration is the number of minutes to automatic lock expiry
		lockmins = request5;

		// LOCK request may specify a master lock in form filename*key*sessionid
		// that must exist with the right sessionid
		// such locks are called subsidiary locks and
		// depend on the locking status of the master
		// currently they are not cleared out when the master lock is unlocked
		// maybe unlock could unlock all locks with the same sessionid
		// RELOCK doesnt have any meaning for subsidiary locks
		// since their status is dependent on the master lock status
		masterlock = request6;

		if (not(file.open(filename, ""))) {
			gosub badfile();
			return;
		}
		leaselocks = "";
		gosub leaselock();
		sessionid.move(data_);

	} else if (request1 eq "UNLOCK") {

		call listen3(request2, request1);

		filename  = request2;
		keyx	  = request3;
		sessionid = request4;

		if (not(file.open(filename, ""))) {
			gosub badfile();
			return;
		}
		leaselocks = "";
		gosub leaseunlock();

		// read a record
	} else if ((request1 eq "READ" or request1 eq "READO") or request1 eq "READU") {

		withlock = request1 eq "READU";
		// updatenotallowed=''
		data_	  = "";
		sessionid = "";

		filename = request2;
		keyx	 = request3;
		lockmins = request4;

		readenv		  = request5;
		req.templatex = readenv;
		call listen3(filename, request1, filetitle, triggers);
		postread = triggers.f(3);

		// reduce chance of using old common
		req.registerx = "";	 //dim

		// allow read to unknown files for the time being
		if (filetitle eq "") {
			filetitle = filename;
		}

		filetitle2 = singular(filetitle);
		filetitle2.converter(".", " ");
		// if postread else
		secmode = "ACCESS";
		gosub filesecurity();
		if (not ok) {
			return;
		}
		// end

		// security check - cannot lock so cannot write or delete
		// if withlock then
		// if security(filetitle2:' UPDATE',updatenotallowed,'') then null
		// if security(filetitle2:' CREATE',createnotallowed,'') then null
		// end

		if (not(file.open(filename, ""))) {
			gosub badfile();
			return;
		}
		leaselocks = "";

		keyx0	= keyx;
		preread = triggers.f(1);
		if (preread) {
			keyx.move(ID);
			req.srcfile	 = file;
			req.datafile = filename;
			systemsubs	 = preread;
			call systemsubs(triggers.f(2));
			DATA = "";
			ID.move(keyx);
		}

		autokey = 0;
		if (((keyx eq "" or (keyx.starts("*"))) or (keyx.ends("*"))) or keyx.contains("**")) {

			// must provide a key unless locking
			if (not withlock) {
				// response='Error: EXODUS.NET Key missing and not READU'
				call  listen4(5, response_);
				gosub fmtresp();
				return;
			}

getnextkey:

			// setup environment for DEF.SK
			req.wlocked	 = 0;
			RECORD		 = "";
			ID			 = keyx;
			MV			 = 0;
			req.datafile = filename;
			req.srcfile	 = file;
			req.isdflt	 = "";

			call generalsubs("DEF.SK." ^ readenv);

			keyx = req.isdflt;

			if (keyx eq "") {
				// response='Error: Next number was not produced':fm:msg
				call  listen4(6, response_, msg_);
				gosub fmtresp();
				return;
			}

			autokey = 1;
		}

		sessionid = "";
		lockkeyx  = keyx;
		if (withlock) {
			response_ = "";

			masterlock = "";

			gosub leaselock();

			// if cannot lock then get next key
			if (response_ eq "NOT OK" and autokey) {
				goto getnextkey;
			}

			// cannot do this because should return the record but without a lock
			// even if a lock requested
			// if response then
			// gosub fmtresp
			// return
			// end
		}

		FILEERRORMODE = 1;
		FILEERROR	  = "";

		if (data_.read(file, keyx)) {

			// if record already on file somehow then get next key
			if (autokey) {
				goto getnextkey;
			}

			// prevent update
			// if withlock and updatenotallowed then
			if (withlock) {
				if (not(authorised(filetitle2 ^ " UPDATE"))) {
					goto noupdate;
				}

				// check if allowed to update this company code
				gosub checkcompany();
				if (badcomp) {
noupdate:
					if (sessionid) {
						gosub leaseunlock();
					}
					sessionid = "";
					// after unlock which sets response to OK
					lockauthorised = 0;
				}
			}

			if (withlock and sessionid eq "") {
				// response='Error: CANNOT LOCK RECORD'
				call  listen4(7, response_);
				gosub addlockholder();
			} else {
				response_ = "OK";
			}
			if (sessionid) {
				response_ ^= " SESSIONID " ^ sessionid;
			}

			// record doesnt exist
		} else {
			// if @file.error<1>='100' then
			// NO FILE ERROR FOR JBASE
			if (not(FILEERROR) or FILEERROR.f(1) eq "100") {

				// no spaces in new keys
				// allow in multipart keys on the assumption that they are old keys with spaces
				if ((withlock and keyx.contains(" ")) and not(keyx.contains("*"))) {
					// if sessionid then gosub leaseunlock
					gosub leaseunlock();
					// msg='Error: ':quote(params<1>):' must not contain a space character'
					call  listen4(28, response_, keyx);
					gosub fmtresp();
					return;
				}

				// prevent create (unless there is a postread which might recover records)
				// nb any postread must test create allowed
				// if withlock then if createnotallowed and not(postread) then
				if (withlock and not(postread)) {

					// must provide a key unless locking
					if (not(authorised(filetitle2 ^ " CREATE", createnotallowed))) {
						if (sessionid) {
							gosub leaseunlock();
						}
						response_ = createnotallowed;
						gosub fmtresp();
						return;
					}
				}

				// response='Error: ':quote(keyx):' does not exist in the ':quote(filetitle):' file'
				data_ = "";
				// response='Error: NO RECORD'
				call listen4(8, response_);
				if (sessionid) {
					response_ ^= " SESSIONID " ^ sessionid;
				} else {
					if (withlock) {
						gosub addlockholder();
					}
				}

			} else {
				gosub geterrorresponse();
			}
		}

		if (withlock) {
			if (not lockauthorised) {
				response_ ^= " LOCK NOT AUTHORISED";
			}
		}

		// response/@user3/RECORDKEY may be used in POSTREAD
		if (keyx ne keyx0) {
			// response:=' RECORDKEY ':keyx
			tt = keyx;
			// ugly cludge to allow space in recordkey to be understood in client.htm
			tt.replacer(" ", "{20}");
			response_ ^= " RECORDKEY " ^ tt;
		}

		// postread (something similar also in select2)
		if (postread) {

			// simulate window environment for POSTREAD
			req.srcfile	 = file;
			req.datafile = filename;
			if (not(DICT.open("DICT." ^ req.datafile))) {
				if (sessionid) {
					gosub leaseunlock();
				}
				// response=quote('DICT.':datafile):' CANNOT BE OPENED'
				call  listen4(9, response_, "DICT." ^ req.datafile);
				gosub fmtresp();
				return;
			}
			keyx.move(ID);
			data_.move(RECORD);
			req.orec	 = RECORD;
			req.wlocked	 = sessionid;
			origresponse = response_;
			// response=''
			msg_	  = "";
			req.reset = 0;

			systemsubs = postread;
			call systemsubs(triggers.f(4));
			DATA = "";

			// restore this programs environment
			RECORD.move(data_);
			ID.move(keyx);

			call cropper(data_);

			// postread can request abort by setting msg or reset>=5
			// msg with reset<0 results in comment to client
			// if reset>=5 or msg then
			if (req.reset ge 5 or ((msg_ and (req.reset ne - 1)))) {
				if (withlock) {
					gosub leaseunlock();
					// wlocked=0
				}
				// if msg then msg='Error: ':msg
				response_ = msg_;
				if (response_ eq "") {
					response_ = "ACCESS REFUSED";
				}
				data_ = "";
				gosub fmtresp();
				// response='Error: ':response
			} else {
				response_ = origresponse;
				// postread may have provided a record where non-was found
				// ONLY if it unlocks it as well! otherwise
				// removal of NO RECORD will cause failure in client
				if (not(req.wlocked) and data_) {

					if (withlock and sessionid) {

						// in case postread changed the key and didnt unlock
						keyx2 = keyx;
						keyx  = lockkeyx;

						gosub leaseunlock();

						keyx = keyx2;
					}

					response_.replacer("Error: NO RECORD", "");

					// swap 'RECORDKEY ' with '' in response
					response_.trimmerlast();
					if (response_ eq "") {
						response_ = "OK";
					}

					// in case postread has changed the key
					if (keyx ne keyx0 and not(response_.contains("RECORDKEY"))) {
						tt = keyx;
						tt.replacer(" ", "%20");
						response_ = (response_ ^ " RECORDKEY " ^ tt).trim();
					}

					if (msg_) {
						response_ ^= " " ^ msg_;
					}
				}
			}

			// postread may have unlocked the record
			// (and removed it from the locks file)
			// but make sure
			if (sessionid and not(req.wlocked)) {

				storeresponse = response_;
				gosub leaseunlock();
				response_ = storeresponse;

				// remove session id
				tt = response_.index("SESSIONID");
				if (tt) {
					response_.paster(tt, ("SESSIONID " ^ sessionid).len(), "");
					response_.trimmerlast();
					if (response_ eq "") {
						response_ = "OK";
					}
				}
			}

			// do not indicate record found
			// otherwise Delete button will be enabled and deleting a record
			// that does not exist is disallowed but lock will be unlocked
			// if iodat then
			// response='OK'
			// if sessionid then response:=' SESSIONID ':sessionid
			// end
		}

		// prevent reading passwords postread and postwrite
		if (filename eq "DEFINITIONS" and keyx eq "SECURITY") {
			data_(4) = "";
		}

		call cropper(data_);

	} else if ((request1 eq "WRITEU" or request1 eq "DELETE") or request1 eq "WRITE") {
		// write:
		filename  = request2;
		keyx	  = request3;
		fieldno	  = request4;
		sessionid = request5;

		readenv		  = request6;
		req.templatex = readenv;
		call listen3(filename, request1, filetitle, triggers);
		prewrite = triggers.f(1);

		// reduce chance of using old common
		req.registerx = "";	 //dim

		// disallow read/write to unknown files for the time being
		if (filetitle eq "") {
			// response=request1:' ':quote(filename):' is not allowed'
			call listen4(11, response_, request1 ^ FM ^ filename);
			return;
		}
		// allow read to unknown files for the time being
		if (filetitle eq "") {
			filetitle = filename;
		}

		filetitle2 = singular(filetitle);
		filetitle2.converter(".", " ");

		// double check allowed access to file
		// if prewrite else
		secmode = "ACCESS";
		gosub filesecurity();
		if (not ok) {
			return;
		}
		// end

		// simulate window environment

		ID = keyx;
		MV = 0;
		data_.move(RECORD);
		req.datafile = filename;
		// not really needed because pre/post code should assume that it is wlocked
		// but some code does not know that (eg postread called from postwrite)
		req.wlocked	  = sessionid;
		req.saverec	  = request1   ne "DELETE";
		req.deleterec = request1 eq "DELETE";

		// trim excess field and value marks
		call cropper(RECORD);

		if (not(req.srcfile.open(req.datafile, ""))) {
			// response=quote(datafile):' CANNOT BE OPENED'
			call listen4(9, response_, req.datafile);
			return;
		}

		if (not(DICT.open("DICT." ^ req.datafile))) {
			// response=quote('DICT.':datafile):' file is not available'
			call listen4(9, response_, "DICT." ^ req.datafile);
			return;
		}

		// TODO consider using RELOCKING to do the following leaselock check/update
		//     and or getting a properlock before doing it

		// Open the leaselocks on the same connection as the data file
		if (not openleaselocks(req.srcfile))
			return;

		// make sure that the record is already leaselocked to the user
		lockkey = filename ^ "*" ^ ID;
		if (not(lockrec.read(leaselocks, lockkey))) {
			lockrec = FM ^ FM ^ FM ^ FM ^ "NO LOCK RECORD";
		}
		if (sessionid ne lockrec.f(5)) {
			// response='Somebody has updated this record.|Your update cannot be applied.':'|The session id does not agree ':quote(lockrec<5>)
			call  listen4(12, response_, lockrec.f(5));
			gosub fmtresp();
			return;
		}

		// update the leaselock session time
		// similar code in lock: and write:
		gosub getdostime();
		lockduration = defaultlockmins / (24 * 60);
		lockrec(1)	 = lockduration + dostime;
		lockrec(2)	 = dostime;
		lockrec.write(leaselocks, lockkey);

		// get a proper lock on the file
		// possibly not necessary as the leaselocks file entry will prevent other programs
		// proper lock will prevent index mfs hanging on write

		req.valid = 1;
		gosub properlock();
		if (not(req.valid)) {
			return;
		}

		if (not(req.orec.read(req.srcfile, ID))) {
			req.orec = "";
		}

		// trim excess field and value marks
		call cropper(req.orec);

		// double check not updated by somebody else
		// nb this does not work for delete until client provides
		// record or at least DATE_TIME in delete
		if (request1 ne "DELETE") {
			// !cannot do this unless
			// a) return new record (done) or at least DATE_TIME in WRITEU to client
			// b) provide record in DELETE
			if (allcols ne "") {
				if (dictrec.readc(allcols, filename ^ "*DATE_TIME")) {
					datetimefn	= dictrec.f(2);
					olddatetime = req.orec.f(datetimefn);
					newdatetime = RECORD.f(datetimefn);
					if (olddatetime and olddatetime ne newdatetime) {
						gosub properunlock();
						// response='Somebody else has updated this record.|Your update cannot be applied.':'|The time stamp does not agree'
						call  listen4(13, response_);
						gosub fmtresp();
						return;
					}
				}
			}
		}

		// detect if defined postwrite or postdelete called
		postroutine = 0;

		if (request1 eq "WRITEU" or request1 eq "WRITE") {

			// prevent writing an empty record
			if (RECORD eq "") {
emptyrecorderror:
				// response='Write empty data record is disallowed.'
				call listen4(14, response_);
badwrite:
				gosub properunlock();
				gosub fmtresp();
				return;
			}

			// check if allowed to update this company code
			gosub checkcompany();
			if (badcomp) {
				response_ = badcomp;
				goto badwrite;
			}

			// double check that the record has not been updated since read
			// Cannot do this unless during WRITE (not WRITEU) we pass back the new timedate
			// readf datetimefn from @dict,'DATE_TIME',2 then
			// if @record<datetimefn> ne orec<datetimefn> then
			//  response='Somebody else has updated this record.|Your update cannot be applied'
			//  gosub properunlock
			//  gosub fmtresp
			//  return
			//  end
			// end

			// prewrite processing
			if (prewrite) {
				// call @updatesubs('PREWRITE')
				systemsubs = prewrite;
				call systemsubs(triggers.f(2));
				DATA = "";
			}

			if (not(req.valid)) {
				gosub properunlock();
				response_ = msg_;
				gosub fmtresp();
				return;
			}

			// allow for prewrite to change the key (must not lock the new key)
			// prewrite must NOT unlock the record because write will fail by index mfs hanging if any
			// is the above true? because unlock just below will
			// remove the lock file record
			// and actually the unlock will fail if it cannot true lock the record
			// journal.subs4 unlocks and it works fine and will leave lock hanging if it does not
			if (ID ne keyx) {

				gosub leaseunlock();
				gosub properunlock();

				keyx = ID;

				// cannot place a lock file entry if the prewrite has locked the new key
				file.unlock(keyx);

				masterlock = "";

				lockmins = defaultlockmins;
				gosub leaselock();
				if (response_ ne "OK") {
					return;
				}

				gosub properlock();
				if (not(req.valid)) {
					return;
				}
			}

			call cropper(RECORD);

			// check for empty record again in case updatesubs screwed up
			if (RECORD eq "") {
				goto emptyrecorderror;
			}

			// failsafe in case prewrite unlocks key?
			// gosub properlock

			replacewrite = triggers.f(5);
			if (replacewrite) {
				systemsubs = replacewrite;
				call systemsubs(triggers.f(6));
				// in case it changes @record?
				if (not(RECORD.read(req.srcfile, keyx))) {
					RECORD = "";
				}
			} else {
				RECORD.write(req.srcfile, keyx);
			}

			// post write processing
			postwrite = triggers.f(3);
			if (postwrite) {
				postroutine = 1;
				// call @updatesubs('POSTWRITE')
				systemsubs = postwrite;
				call systemsubs(triggers.f(4));
				DATA = "";
			}

			// send back revised data or nothing
			// NB data is now '' to save space so always send back data unless @record is cleared
			if (RECORD eq data_) {
				data_ = "";
			} else {
				data_ = RECORD;
			}

			// prevent reading passwords postread and postwrite
			if (filename eq "DEFINITIONS" and keyx eq "SECURITY") {
				data_(4) = "";
			}

		} else if (request1 eq "DELETE") {

			predelete = triggers.f(1);

			// ensure that deletion works on the orig record
			RECORD = req.orec;

			// prevent deleting if record does not exist
			if (RECORD eq "") {
				// response='Cannot delete because ':quote(keyx):' does not exist in the ':quote(filetitle):' file'
				call  listen4(15, response_, keyx ^ FM ^ filetitle);
				gosub fmtresp();
				return;
			}

			// check allowed to delete
			// if security(filetitle2:' DELETE',msg,'') else
			// response=msg
			// gosub fmtresp
			// return
			// end
			// if predelete else
			secmode = "DELETE";
			gosub filesecurity();
			if (not ok) {
				return;
			}
			// end

			// predelete processing
			if (predelete) {
				// call @updatesubs('PREDELETE')
				systemsubs = predelete;
				call systemsubs(triggers.f(2));
				DATA = "";
			}
			if (not(req.valid)) {
				gosub properunlock();
				response_ = msg_;
				gosub fmtresp();
				return;
			}

			req.srcfile.deleterecord(keyx);

			// post delete processing
			postdelete = triggers.f(3);
			if (postdelete) {
				postroutine = 1;
				systemsubs	= postdelete;
				call systemsubs(triggers.f(4));
				DATA = "";
			}

			// send back no iodat
			data_ = "";
		}

		// remove LOCKS file entry
		if (request1 ne "WRITE") {

			leaselocks.deleterecord(lockkey);

			// unlock local lock
			req.srcfile.unlock(keyx);

		} else {
			gosub properunlock();
		}

		// even postwrite/postdelete can now set invalid (to indicate invalid mode etc)
		// if valid then response='OK' else response='Error:'
		response_ = req.valid ? "OK" : "Error:";

		if (request1 ne "DELETE") {
			tt = ID;
			// response:=' RECORDKEY ':@id
			// horrible cludge to allow space in recordkey to be understood in client.htm
			tt.replacer(" ", "{20}");
			response_ ^= " RECORDKEY " ^ tt;
			if (sessionid) {
				response_ ^= " SESSIONID " ^ sessionid;
			}
		}

		// pre and post routines can return warnings/notes in msg

		if (msg_) {
			response_ ^= " " ^ msg_;
		}

		// if postroutine else call flush.index(filename)
		call flushindex(filename);

		gosub fmtresp();
		return;

		// execute a request
	} else if (request1 eq "EXECUTE") {

		// if @username='EXODUS' then
		// oswrite @user0 on 'request_'
		// oswrite @user1 on 'data_'
		// end

		// build command from request and check is a valid program
		voccmd = request2;
		if (voccmd eq "") {
			// response='LISTEN:EXECUTE: Module name is missing from request'
			call listen4(32, response_);
			return;
		}
		voccmd ^= "PROXY";
		if (not libinfo(voccmd.lcase())) {
			// response='Error: ':quote(voccmd):' module is not available'
			call listen4(16, response_, voccmd);
			return;
		}

		// provide an output file for the program to be executed
		// NB response file name for detaching processes
		// will be obtained from the output file name LISTEN2 RESPOND
		// this could be improved to work
		// /root/exodus/service/data/exodus/~3810873.2
		printfilename = linkfilename2;

		printfilename.paster(-1, 1, "htm");
		SYSTEM(2) = printfilename;

		// provide interruptfilename
		// esc.to.exit will return 1 if it finds this file
		// clear it if running non-interruptable processes and giveway maybe called
		tt = linkfilename2;
		tt.paster(-1, 1, "5");
		SYSTEM(6) = tt;

		// execute the program
		response_ = "OK";
		req.valid = 1;
		msg_	  = "";

		// request, iodat and response are now passed and returned in @user0,1 and 3
		// other messages are passed back in @user4
		// execute instead of call prevents program crashes from crashing LISTEN
		// changed now that dataset and password are removed in request()
		// request=field(request,fm,firstrequestfieldn+2,99999)
		request_ = request_.field(FM, 3, 99999);

		// pass the output file in linkfilename2
		// not good method, pass in system?
		if (var("LIST,SELECTJOURNALS").locateusing(",", request_.f(1), xx)) {
			data_ = linkfilename2;
		}
		if (request_.f(1).starts("VAL.")) {
			data_ = linkfilename2;
		}

		// execute so that failures dont cause failures of LISTEN
		// but not listen resets itself nicely perhaps this isnt needed
		execute(voccmd);

		// reformat for reqlog
		voccmd.cutter(-5);
		request_(1) = voccmd ^ "_" ^ request_.f(1);

		// discard any stored input
		DATA = "";

		if (msg_) {

			user4x = msg_.ucase();

			// convert error message (could also be a system error eg dict says indexed but isnt
			if ((user4x.contains("IN INDEX.REDUCER") or user4x.contains("IN RTP21")) or user4x.contains("IN RTP20")) {
				// @user4='Please select fewer records and/or simplify your request'
				call listen4(17, msg_);
			}

			// send errors to exodus
			if (((msg_ ^ user4x).contains("INTERNAL ERROR")) or user4x.contains("DAMAGED FILE")) {
				call sysmsg(msg_);
			}
		}

		call cropper(msg_);
		call cropper(response_);

		if (msg_) {
			data_	  = "";
			response_ = "Error: " ^ msg_;
			gosub fmtresp();
		}

		if (response_ eq "") {
			// response='Error: No OK from ':voccmd:' ':request
			call  listen4(18, response_, voccmd);
			gosub fmtresp();
		}

		// speed up next autorun if new one has been added
		tt = response_.index("%NEWAUTORUN%");
		if (tt) {
			response_.replacer("%NEWAUTORUN%", "");
			lastautorun = "";
		}

		// get the printfilename in case the print program changed it
		// multiple files may be passed back in iodat/user1 see proxy checkoutputfileexisits
		printfilename = SYSTEM.f(2);
		// if tracing then
		// *print ' got it'
		// print @(0):@(-4):
		// end

		// make sure that the output file is closed
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

	} else if (request1 eq "BACKUP") {

		// trigger additional processes that should fail due to backup lock
		//(inpath ^ serverflagfilename).osremove();
		var serverflagfilepath = inpath ^ serverflagfilename;
		if (serverflagfilepath.osfile() and not serverflagfilepath.osremove())
			abort(lasterror());

		call listen5(request1);

		// or give an error message
	} else {
		data_ = "";
		// response='Error: ':quote(field(request,fm,4,9999)):' unknown request'
		// response='Error: ':quote(request):' unknown request'
		call listen4(21, response_, request_);
	}

	// put no code here because some returns above will short cut it

	return;

}  // process2

function request_exit() {

	// Handle the response
	// ////////////////////

	// //////////
	// requestexit:
	// //////////

	call listen5("UNLOCKLONGPROCESS");

	// 1. failsafe unlock everything that might still be locked
	// 2. a very few processes rely on all locks being released
	//   eg autorun2 locks so that autoruns are not processed until
	//   the request completes
	// 3. generally all processes are careful to unlock what they have locked
	//   but crashed processes will not do this

	var().unlockall();

	// in case any select list left open
	clearselect();

	if (response_ eq "") {
		call listen4(1, response_);
	}

	if (response_.contains("ERROR NO:")) {
		call log("LISTEN", response_);
	}

	USERNAME = listenusername;
	// call sysvar('SET',109,110,listenstation)
	// call sysvar_109_110('SET',listenstation)
	STATION = listenstation;

	// if ucase(response[1,6])='ERROR:' then iodat=''

	// have to call it here as well :(
	responsetime = ostime();
	secs		 = ((responsetime - requesttime).oconv("MD60P")) + 0;
	if (secs lt 0) {
		secs += 86400;
	}

	tt = response_.ucase();
	if ((tt.contains("R18.6") or tt.contains("RTP20 MISSING")) or tt.contains("TOO MANY LEVELS OF TCL")) {
		halt = 1;
		// response<-1>='INTERNAL ERROR Closing current EXODUS server process'
		call listen4(1, response_);
		call sysmsg(response_);
	}

	rawresponse = response_;
	rawresponse.converter("\r\n", "|");

	gosub updreqlog();

	if (logfilename) {
		logx = _EOL "<Response ProcessingSecs=" ^ (secs.quote()) ^ ">";
		gosub writelogx2();

		// convert non ascii to hexcode
		logx = response_;
		gosub writelogx();

		logx	 = "</Response>";
		iodatlen = data_.len();
		if (iodatlen) {
			logx ^= _EOL "<DataOut>";
		}
		gosub writelogx2();
	}

	if (data_ eq "%DIRECTOUTPUT%") {

		logx = data_;
		// convert '%' to '%25' in logx
		logx.replacer("%", "%25");
		gosub writelogx();

	} else {

		//call oswrite("", linkfilename2);
		if (not oswrite("", linkfilename2))
			abort(lasterror());
		if (linkfile2.osopen(linkfilename2)) {

			// split into blocks and convert to escape chars
			datx = "";	// dim
			ptr	 = 0;
			for (blockn = 1; blockn <= nblocks; ++blockn) {
				blk = data_.first(outblocksize);
				data_.cutter(outblocksize);

				if (not(blk.len()))
					break;

				// in LISTEN and SELECT2 for direct output
				//call osbwrite(blk, linkfile2, ptr);
				if (not osbwrite(blk, linkfile2, ptr))
					abort(lasterror());

				if (logfilename) {
					blk.move(logx);
					gosub writelogx();
				}

				blk = "";

			}  // blockn;

			linkfile2.osclose();

		} else {

			// response='ERROR: LISTEN cannot create temp ':linkfilename2
			call listen4(22, response_, linkfilename2);
		}
	}

	// try to flush file open
	if (linkfile2.osopen(linkfilename2)) {
		linkfile2.osclose();
	}

	if (logfilename) {

		logx = "";
		if (iodatlen) {
			logx ^= "</DataOut>";
		}
		logx ^= _EOL "</Message>" _EOL;
		gosub writelogx2();

		gosub writelogxclose();
	}

	// swap '|' with char(13) in response
	// swap fm with char(13) in response

	// write the response
	//call oswrite(response_, linkfilename3);
	if (not oswrite(response_, linkfilename3))
		abort(lasterror());

	// trace responded
	responsetime = ostime();
	if (tracing) {
		tt = " " ^ oconv((responsetime - requesttime).mod(86400), "[NUMBER,2]") ^ "s ";
		// seconds
		tt ^= rawresponse.f(1, 1).field("|", 1).f(1, 1);
		if (tt.contains("<")) {
			call htmllib2("STRIPTAGS", tt);
		}
		tt.replacer("SESSIONID ", "");
		printl(tt);
		// print linkfilename1
	}

	if (halt) {
		// gosub main_exit();
		return false;
	}

	if (response_ eq "OK") {
		if (request1 eq "STOPDB" or request1.starts("RESTART")) {
			// gosub main_exit();
			return false;
		}
	}

	return true;

}  // request_exit

subroutine geterrorresponse() {
	// 	fileerrorx = FILEERROR;
	// 	response_ = "Error: " ^ ("FS" ^ fileerrorx.f(1, 1)).xlate("SYS.MESSAGES", 11, "X");
	// 	response_.replacer("%1%", handlefilename(fileerrorx.f(2, 1)));
	// 	response_.replacer("%2%", fileerrorx.f(2, 2));
	response_ = "Error: " ^ FILEERROR;
	gosub fmtresp();
	return;
}

subroutine fmtresp() {

	// trim everything after <ESC> (why?)
	tt = response_.index("<ESC>");
	if (tt) {
		response_ = response_.first(tt - 1);
	}

	// cannot remove since these may be proper codepage letters
	response_.converter("|", _FM);
	response_.converter(_VM, _FM);
	if (response_.starts(_FM)) {
		response_.cutter(1);
	}
	response_.replacer(_FM, _EOL);

	return;
}

subroutine gettimeouttime() {
	if (not timeoutsecs) {
		timeouttime = "";
		return;
	}

	timeoutdate = date();
	timeouttime = ostime();
	timeouttime += timeoutsecs;
	if (timeouttime gt 24 * 60 * 60) {
		timeoutdate += 1;
		timeouttime -= 24 * 60 * 60;
	}
	timeouttime = (timeoutdate ^ "." ^ timeouttime.floor().oconv("R(0)#5")) + 0;
	return;
}

subroutine properlock() {
	// must lock it properly otherwise indexing will try to lock it and fail
	// because it is only in the LOCKS file and not properly locked
	req.valid = 1;

	// must exclude the shadowing system otherwise the entry in the LOCKS file
	// also in balances
	// will cause the LOCK statement to fail
	srcfile2 = req.srcfile;
	/*
	if (not(req.srcfile.unassigned())) {
		srcfile2.replacer("SHADOW.MFS" ^ SM, "");
	}
	*/

	// dont pass the filename because that causes persistent lock checking
	// in jbase version of lockrecord()
	if (not(lockrecord("", srcfile2, keyx, xx))) {
		if (STATUS ne 1) {
			req.valid = 0;
			// response='Error: ':quote(keyx):' CANNOT BE WRITTEN BECAUSE IT IS LOCKED ELSEWHERE'
			call listen4(23, response_, keyx);
		}
	}

	return;
}

subroutine properunlock() {
	// NB i think that shadow.mfs is NOT programmed to remove the locks file entry

	// must unlock it properly otherwise indexing will try to lock it and fail
	// because it is only in the LOCKS file and not properly locked

	// must exclude the shadowing system otherwise the entry in the LOCKS file
	// will be removed as well
	srcfile2 = req.srcfile;
	/*
	srcfile2.replacer("SHADOW.MFS" ^ SM, "");
	*/

	srcfile2.unlock(keyx);

	return;
}

subroutine leaselock() {
	// called from LOCK/RELOCK/READU (and WRITE if prewrite changes the key)

	// cannot do update security check here, have to do it AFTER
	// we know if the record exists or not
	// security check - cannot lock so cannot write or delete

	lockauthorised = 1;
	// if unassigned(lockmins) or not(num(lockmins)) or not(lockmins) then
	if (not(lockmins.isnum()) or not(lockmins)) {
		lockmins = defaultlockmins;
	}

	// sessionid which if RELOCKING must match the one on the lock rec
	// otherwise it is assumed that somebody else has taken over the lock
	// if sessionid is blank then a new session id is created and returned in response
	if (request1 eq "RELOCK") {
		newsessionid = sessionid;
	} else {
		newsessionid = "";
		for (ii = 1; ii <= 8; ++ii) {
			newsessionid ^= var("01234567890ABDCEF")[var(16).rnd() + 1];
		}  // ii;
	}

	if (not(file.open(filename, ""))) {
		gosub badfile();
		return;
	}

	// Open the leaselocks on the same connection as the data file
	if (not openleaselocks(file))
		return;

	response_ = "";
	if (request1 eq "RELOCK") {
		gosub rawlock();
	} else {
		if (lockrecord(filename, file, keyx, xx)) {
			state = 1;
		} else {
			state = 0;
		}
	}

	if (not state) {
		// response='NOT OK'
		// response='Error: CANNOT LOCK RECORD'
		call  listen4(7, response_);
		gosub addlockholder();
		return;
	}

	gosub getdostime();

	// check locks file
	lockkey = filename ^ "*" ^ keyx;
	if (lockrec.read(leaselocks, lockkey)) {

		// handle a subsidiary lock (has master lock details)
		// very similar code in LISTEN and SHADOW.MFS
		if (lockrec.f(6)) {

			masterlockkey = lockrec.f(6).field("*", 1, 2);
			sublockrec	  = lockrec;

			// if masterlock is missing or doesnt have the same session id
			// then the subsidiary is considered to have expired
			if (not(lockrec.read(leaselocks, masterlockkey))) {
				goto nolock;
			}
			if (lockrec.f(5) ne sublockrec.f(5)) {
				goto nolock;
			}
		}

		// during RELOCK the session id will normally be the same
		// unless someone else got the lock because relocking was suspended somehow
		if (lockrec.f(5) ne newsessionid) {

			if (request1 eq "RELOCK") {
				goto nolock;
			}

			// fail if other lock has not timed out
			if (lockrec.f(1) gt dostime) {
				response_ = "NOT OK";
				goto lockexit;
			}

			// other lock has timed out so ok
			// no need to delete as will be overwritten below
			// delete leaselocks,lockkey
		}

		// our own session so must be relocking (to extend timeout)

	} else {
nolock:
		if (request1 eq "RELOCK") {
			// NB the word "EXPIRED" is a key word used in _formfunctions.htm
			response_ = "Error: Your lock expired and/or somebody else updated";
			goto lockexit;
		}

		// no lock record so ok
	}

	if (masterlock) {

		// fail if masterlock is missing or doesnt have the right session id
		if (not(tt.read(leaselocks, masterlock.field("*", 1, 2)))) {
			goto nolock;
		}
		if (tt.f(5) ne masterlock.field("*", 3)) {
			goto nolock;
		}

		// subsidiary lock gets the same sessionid as the master
		newsessionid = tt.f(5);
	}

	response_ = "OK";

	// convert minutes to fraction of one day (windows time format)
	lockduration = lockmins / (24 * 60);

	// write the lock in the leaselocks file
	lockrec = "";

	// similar code in lock: and write:
	lockrec(1) = lockduration + dostime;
	lockrec(2) = dostime;

	// lockrec<3>=if connection then connection<1,2> else @station
	if (connection) {
		lockrec(3) = connection.f(1, 2);
	} else {
		lockrec(3) = STATION;
	}
	lockrec(4)	  = USERNAME;
	lockrec(5)	  = newsessionid;
	lockrec(6)	  = masterlock;
	FILEERRORMODE = 1;
	FILEERROR	  = "";
	response_	  = "OK";
	lockrec.write(leaselocks, lockkey);
	if (FILEERROR) {
		call  mssg(lasterror());
		gosub geterrorresponse();
	}

	sessionid = newsessionid;

lockexit:

	// unlock file,keyx
	if (request1 eq "RELOCK") {
		gosub rawunlock();
	} else {
		file.unlock(keyx);
	}

	return;
}

subroutine rawlock() {
	// attempt to lock the record
	// bypass ordinary lock,file,key process otherwise
	// the lock record will be checked - and in this case
	// we our own lock record to be present
	// code = 5;
	// nextbfs = "";
	// handle = file;

	/*
	// handle=handle[-1,'B':vm]
	handle = field2(handle, VM, -1);
	*/

	// keyorfilename = keyx;
	// fmc = 2;
	// gosub rawlock2(code, nextbfs, handle, keyorfilename, fmc, state);
	call rtp57(5, "", file, keyx, 2, xx, state);
	return;
}

subroutine rawunlock() {
	// unlock file,keyx
	// code = 6;
	// gosub rawlock2(code, nextbfs, handle, keyorfilename, fmc, state);
	call rtp57(6, "", file, keyx, 2, xx, state);
	return;
}

// subroutine rawlock2(in code, in nextbfs, io handle, in keyorfilename, in fmc, io state) {
//
//	// called from rawlock and rawunlock
//	call rtp57(code, nextbfs, handle, keyorfilename, fmc, xx, state);
//
//	return;
//}

subroutine badfile() {
	// response='Error: ':quote(filename):' file does not exist'
	call listen4(24, response_, filename);
	return;
}

function openleaselocks(in file) {

	// Open the leaselocks on the same connection as the data file
	if (not(leaselocks.open("LOCKS", file))) {
		response_ = "Error: LOCKS for " ^ file ^ " cannot be opened.";
		// call listen4(9, response_, req.datafile);
		return false;
	}

	return true;
}

subroutine leaseunlock() {

	// sessionid is used as a check that only the locker can unlock
	if (not(file.open(filename, ""))) {
		gosub badfile();
		return;
	}

	// Open the leaselocks on the same connection as the data file
	if (not openleaselocks(file))
		return;

	// lock file,keyx
	gosub rawlock();
	if (not state) {
		// zzz perhaps should try again a few times in case somebody else
		// is trying to lock but failing because of our remote lock
		// response='Error: Somebody else has locked the record'
		call listen4(25, response_);
		return;
	}

	// get the current lock else return ok
	lockkey		  = filename ^ "*" ^ keyx;
	FILEERRORMODE = 1;
	FILEERROR	  = "";
	if (not(lockrec.read(leaselocks, lockkey))) {
		lockrec = "";
	}
	if (not lockrec) {
		if (FILEERROR.f(1) eq 100) {
			// lock is missing but ignore it
			// because we are unlocking anyway
			response_ = "OK";
		} else {
			gosub geterrorresponse();
		}
		goto unlockexit;
	}

	// check that the current lock agrees with the session id provided
	if (not(sessionid eq lockrec.f(5))) {

		// cannot unlock because the lock belongs to somebody else
		// response='Error: Cannot unlock - '
		if (sessionid eq "") {
			// response:='missing session id'
			tt = "missing";
		} else {
			// response:='wrong session id'
			tt = "wrong";
		}
		call listen4(26, response_, tt);

		goto unlockexit;
	}

	// delete the lock
	FILEERRORMODE = 1;
	FILEERROR	  = "";
	response_	  = "OK";
	leaselocks.deleterecord(lockkey);

	if (FILEERROR) {
		call  mssg(lasterror());
		gosub geterrorresponse();
	}

unlockexit:

	gosub rawunlock();

	return;
}

subroutine getdostime() {
	dostime = ostime();
	// convert to Windows based date/time (ndays since 1/1/1900)
	// 31/12/67 in rev date() format equals 24837 in windows date format
	dostime = 24837 + date() + dostime / 24 / 3600;
	return;
}

subroutine flagserveractive() {

	// flag that this dataset is being served ("listened") (needed for old MAC)
	// does not seem to cause any filesharing errors (stress tested at max speed)
	//call oswrite("", inpath ^ serverflagfilename);
	if (not oswrite("", inpath ^ serverflagfilename))
		abort(lasterror());

	return;
}

subroutine writelogx() {
	t2 = "CONVLOG";
	call  listen5(t2, logx, xx, yy);
	gosub writelogx2();
	return;
}

subroutine writelogxclose() {
	logx = "</Log>";
	gosub writelogx2();
	// backup to overwrite if and when another transaction is received
	logptr -= 6;
	return;
}

subroutine writelogx2() {
	//call osbwrite(logx, logfile, logptr);
	if (not osbwrite(logx, logfile, logptr))
		abort(lasterror());
	logx = "";
	return;
}

subroutine filesecurity() {

	ok = 1;
	if (keyx.contains("*")) {
		return;
	}
	if (authorised(filetitle2 ^ " " ^ secmode, msg0, "")) {
		positive = "";
	} else {
		positive = "#";
	}
	if (not(authorised(positive ^ filetitle2 ^ " " ^ secmode ^ " " ^ (keyx.quote()), posmsg))) {
		// !*use the FILENAME ACCESS/DELETE "ID" message because gives clue
		// !*that they may be allowed to access other records
		if (positive) {
			msg0.move(response_);
		} else {
			posmsg.move(response_);
		}
		ok = 0;
		gosub fmtresp();
	}
	return;
}

subroutine addlockholder() {
	if (tt.read(leaselocks, filename ^ "*" ^ keyx)) {
		// if tt<6> then read tt from locks,field(tt<6>,'*',1,2) else null
		response_ ^= ", LOCKHOLDER: " ^ (tt.f(4).quote());
	}
	return;
}

subroutine checkcompany() {
	badcomp = "";

	// quit no COMPANY_CODE in dict (or cannot determine)
	if (not allcols) {
		return;
	}
	if (not(xx.readc(allcols, filename ^ "*COMPANY_CODE"))) {
		return;
	}

	if (not(dictfile.open("DICT." ^ filename, ""))) {
		return;
	}

	if (request1.contains("READ")) {
		compcode = calculate("COMPANY_CODE", dictfile, keyx, data_, 0);
	} else {
		// compcode={COMPANY_CODE}
		// compcode=calculate('COMPANY_CODE')
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
	// if request1 eq 'RELOCK' then return

	tt = username ^ FM ^ ipno ^ FM ^ netid ^ FM ^ FM ^ responsetime ^ FM ^ rawresponse.f(1);
	if (request_.f(1) eq "EXECUTE") {
		tt(11) = request_.f(2) ^ "_" ^ request_.f(3) ^ FM ^ request_.field(FM, 4, 9999);
	} else {
		tt(11) = request_;
	}

	logid = datasetcode ^ "*" ^ requestdate ^ "*" ^ requesttime ^ "*" ^ THREADNO;
	tt.write(reqlog, logid);

	return;
}

libraryexit()
