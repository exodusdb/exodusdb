/* Copyright (c) 2007 Stephen John Bush - see LICENCE.TXT*/
//#include <iostream>
//#include "server.h"

//#include "mvroutine.h"

//TODO replace wcout, with printl to remove dependency on <iostream>

//perhaps following is not necessary
//and only need to grant www-data group access (+sgid) to data since exodus is owner
//of data therefore can delete files created and owned by www-data there
//
//needs to read and delete files written by www-data
//needs to create files readable and deletable by www-data
//therefore
//1.create a www-exodus group
//2.add www-data and exodus server to it
//3.set the data directory (where www-data and exodus communicate) to www-exodus
/*
sudo groupadd www-exodus
sudo usermod -a -G www-exodus exodus
sudo usermod -a -G www-exodus www-data
sudo chmod -R g+x /home/exodus/service #and all parent directories
sudo chmod -R g+rwxs /home/exodus/service/data
sudo chgrp -R www-exodus /home/exodus/service/data
*/
//posix permissions principles
//only root can change the user of a file
//you can only change the group to groups that you belong to
//the group of a created files can be forced to be the group of the directory
//set the sgid chmod +s
//sometimes you can rename files but not write/delete them
//you must have x privilege to all directories above in order to traverse into one
//"all directories above" means of the real directories not apparent dir if using symbolic links
//check permissions by sudo su <user> and cd/touch etc

#include <exodus/program.h>
programinit()

#include <win.h>//common
#include <gen.h>//general common
#include <fin.h>//finance common
#include <agy.h>//agency common

#include <initgeneral.h>
#include <select2.h>
#include <securitysubs.h>
#include <sysmsg.h>
#include <loginnet.h>

#include "window.hpp"

	const var fefdfcfb = _FM_ _VM_ _SM_ _TM_;

	bool routineexists;

//	var hexx[256];
	var sleepms;//num
	var origscrn;
	var origattr;
	var locks;
//	var logfilename;
//	var logfile;
	var logptr;//num
	var xx;
	var logx;
	var timenow;
	var linkfilenames;
	var linkfilename0;
	var charx;
	var buffer;
	var reply;
	var bakreq;
	var lastbakattemptdate;
	var bakdows;
	var minbaktime;
	var maxbaktime;
	var testdata;
	var bakdisk;
	var requeststarttime;//num
	var invaliduser;
	var nostack;
	var newfilename;
	var ok;
	var createnotallowed;
	var file;
	var lockauthorised;//num
	var requeststoptime;//num
	var iodatlen;

	var request1;
	var request2;
	var request3;
	var request4;
	var request5;

	var linkfile1;

	var code;
	var response;
	var responsefilename;
	var linkfilename1;//request
	var linkfilename2;//request data and response data
	var linkfilename3;//response
	var linkfilename5;//temporary response before renamed to achieve atomicity
	var replyfilename;
	var filename;
	var keyx;
	var sessionid;
	var Serverstation;
	var Serverusername;
	var relistlocks;

	var record;
	var state;//num
	var timeouttime;//num
	var lockdurationinmins;//num
	var newsessionid;
	var connection;
	var filename2;
	var msg0;
	var positive;
	//var msg2;
	var timeoutsecs;
	var ostimex;

	var maxstringsize;
	var Serverversion;
	var origsentence;
	var md;
	var halt;
	var timex;
	var breaktime;
	var dataset;
	var username;
	var password;
	var Serverfailure;
	var origprivilege;
	var nrequests;
	var tracing;
	var origbatchmode;

	var portno;

	var datasetcode;
	var neopath;
	var globalend;
	var allcols;

	var servername;
	var onserver;
	var waitfor;
	var waitsecs;
	var serverflagfilename;
	var intranet;
	var webpath;
	var inpath;
	var batchmode;
	var ostimenow;//num
	var lockduration;
	var lockkey;
	var lockx;
	//delete files older than ...
	var ageinsecs;
	var lockrec;
	var withlock;
	var updatenotallowed;
	var logpath;
	var timestarted;

function main()
{
	//CREATE LABELLED COMMON
	mv.labelledcommon[1]=new win_common;

STATION="";
PSEUDO="";
SYSTEM="";

	//initialise or abort
	if (not perform("initgeneral")){
		return 1;
	}

	maxstringsize=1024*1024;

	portno = 5700;
	if (SYSTEM.a(38))
		portno += SYSTEM.a(38) - 1;

	//delete files older than ...
	ageinsecs = 60 * 60;

	halt = 0;

//TODO	var(L"ADDMFS SHADOW.MFS FILEORDER.COMMON").perform();

	Serverversion = "";
	origsentence = SENTENCE;

	//open 'messages' to messages else return 999999
//	SYSTEM.printl();

	//openqm connection
	//std::wcout,"Connecting DB ... ",std::flush;
	//if (!this->SESSION.connect("127.0.0.1","4243","steve","stetempp","QMSYS"))
	//{
	//	std::wcout,"couldn't connect to QMSYS",std::endl;
	//	return false;
	//}
	//std::wcout,"OK",std::endl;
	var conninfo="";//"host=localhost port=5432 dbname=exodus user=exodus password=somesillysecret connect_timeout=10";
	if (!SESSION.connect(conninfo))
	{
		errputl("MvEnvironment::init: Couldn't connect to local database");
		return false;
	}

	/* arev's byte/character bit inverter not available for now
	//std::wcout,"Reading Security ... ",std::flush;
	if (!SECURITY.read(DEFINITIONS,"SECURITY"))
	{
		//std::wcout,"Cannot read SECURITY",std::endl;
		//return false;
		SECURITY="";
	}
	SECURITY=SECURITY.invert();
//	std::wcout,"OK",std::endl;
	*/

	//std::wcout,"Opening MD ... ",std::flush;
	var mdfilename="MD";
	var md;
	if (!md.open(mdfilename))
	{
			if (!md.createfile(mdfilename)||!md.open(mdfilename))
		{
			errputl("Cannot create "^mdfilename);
			return false;
		}
	}
	//std::wcout,"OK",std::endl;

	//std::wcout,"Opening MENUS ... ",std::flush;
	var menufilename="MENUS";
	var menus;
	if (!menus.open(menufilename))
	{
		if (!menus.createfile(menufilename)||!menus.open(menufilename))
		{
			errputl("Cannot create "^menufilename);
			return false;
		}
	}
	//std::wcout,"OK",std::endl;

	//std::wcout,"MvEnvironment::init: completed ",std::endl;
	datasetcode = SYSTEM.a(17);
	if (datasetcode == "")
//		datasetcode = "DEFAULT";
		datasetcode = "exodus";

#ifdef _WIN32
	neopath = "d:\\neosys\\";
#else
//	neopath = "/cygdrive/d/neosys/neosys/";
	//look for data in parent and grandparent directory
	neopath = "../";
	if (! osdir(neopath^"data"))
		neopath = "../../";
#endif

	neopath.converter("/\\",SLASH^SLASH);
	globalend = neopath ^ "GLOBAL.END";
	if (!allcols.open("ACCESSIBLE_COLUMNS"))
		allcols = "";
/*
	//prepare for sending and receiving escaped iodat > 64kbh
	for (int ii = 0; ii <= 255; ii++)
	{
		hexx[ii]= ("%" ^ (var(ii).oconv("MX")).oconv("R(0)#2"));
		//hexx[ii]="%00";
//		std::wcout,hexx[ii]," ";
	}
*/
	//discover the server name
	//servername=field(getdrivepath(drive()[1,2])[3,9999],'\',1)
	servername = "";
	onserver = servername == "" || STATION.trim() == servername;

	//should be less than 25 unless waiting.exe modified to update the server flag
	waitfor = 1;
	waitsecs = 10;

	//if not on "local" drive c,d or e then longer sleep to save network traffic
	if (onserver) {
		sleepms = 10;
	}else{
		sleepms = 100;
	}

	//make up a serverflagfilename
	serverflagfilename = "GLOBAL.SVR";

	intranet = origsentence.index("INTRANET", 1);

	gosub getbakpars();

	//if @username='neosys.net' then system<33>=1
	origbatchmode = SYSTEM.a(33);

	//webpath=field(origsentence,' ',3)
	webpath = neopath ^ "data/";
	if (!webpath)
		webpath = "../data/";
	webpath.converter("/\\",SLASH ^ SLASH);
	if (webpath[-1] ne SLASH)
		webpath ^= SLASH;
	if (!md.open("MD")) {
		mssg("CANNOT OPEN MD FILE");
		return 999999;
	}

	inpath = webpath ^ datasetcode ^ SLASH;

	//set time before calling deleteoldfiles which is time sensitive
	//call settime(webpath[1,2])

	gosub deleteoldfiles(inpath,"^.*$");

	Serverusername = USERNAME;
	Serverstation = STATION;

	//savescreen(origscrn, origattr);

	//tracing=(@username='neosys' or trim(@station)='sbcp1800')
	//tracing=(not(index(origsentence,'auto',1)))

	//if @username='demo' then tracing=0
	//tracing=0

	tracing = 1;

	//ensure unique sorttempfile
	//if (!(sysvar("SET", 192, 102, "R" ^ var("0000" ^ SYSTEM.a(24)).substr(-5, 5) ^ ".SFX")))
	//	{}

	nrequests = SYSTEM.a(35) + 0;
	//if tracing then
	printl("NEOSYS.NET SERVICE ", SYSTEM.a(24));
	printl(" STARTED ", var().timedate());
	printl();
	printl("Station  : ", STATION);
	printl("Drive : ", var().oscwd());
	printl("Server   : ", servername);
	printl("Data  : ", inpath);

	//print 'tcp port : ':portno
	//end else
	// *xpos=20
	// *ypos=7
	// *call makewin(xpos,ypos,40,10,'',buffer)
	// *print @(xpos+2,ypos+2):'serving
	//
	// *switch into interactive mode to put up message
	// s33=system<33>
	// system<33>=''
	//
	// * call note('neosys.net service' 'c#30':'||||||press esc and wait 10 secs to interrupt','ub')
	//
	// *switch back to silent mode
	// system<33>=s33
	//
	// print @aw<30>:
	//
	// end

	origprivilege = PRIVILEGE;
	//setprivilege("");

	request1 = "";

	//image=''


	if (!(openfile2("LOCKS", locks, "LISTS", 1))) {
		mssg("CANNOT OPEN LOCKS FILE");
		return "";
	}

	relistlocks = 1;

	timeoutsecs = SYSTEM.a(33, 5);
	if (!timeoutsecs.isnum())
		timeoutsecs = "";
	//if timeoutsecs='' then timeoutsecs=20*60;*ie 20 minutes

	gosub gettimeouttime();
	timestarted = var().time();

	gosub flagserveractive();

/*
	//open a log file
	logfilename = "";
	logpath = neopath ^ "/LOGS/";
	logpath.converter("/\\",SLASH^SLASH);
	if (var().oslistf(logpath ^ datasetcode)=="")
	{
		if (var().oslist(logpath)=="")
			logpath.osmkdir();
		(logpath ^ datasetcode).osmkdir();
	}
	var datex = (var().date()).oconv("D2.");
	logfilename = logpath ^ datasetcode ^ SLASH ^ datex.substr(-2, 2) ^ datex.substr(1, 2) ^ datex.substr(4, 2) ^ var("00" ^ SYSTEM.a(24)).substr(-2, 2);
	logfilename ^= ".xml";

	if (logfilename.osopen()) {
		logptr = logfilename.osfile().a(1);

		//osbread x from logfilename at logptr-6 length 6
		var tt=logptr-6;
		xx.osbread(logfilename, tt, 6);
		if (xx == "</Log>")
			logptr -= 6;

	}else{
		var("").oswrite(logfilename);
		if (logfilename.osopen()) {
			logptr = 0;
			logx = "<?xml version=\"1.0\"?>" ^ (var().chr(13) ^ var().chr(10));
			logx ^= "<?xml-stylesheet type=\'text/xsl\' href=\'.\\loglist.xsl\'?>" ^ (var().chr(13) ^ var().chr(10));
			//logx:="<?xml-stylesheet type='text/xsl'?>":crlf
			logx ^= "<Log";
			logx ^= " Process=" ^ SYSTEM.a(24).quote();
			logx ^= " Dataset=" ^ datasetcode.quote();
			logx ^= ">";
			gosub writelogx2();

			osbwritex("</Log>", logfilename, logfilename, logptr);

		}else{
		printl("CANNOT OPEN LOG FILE ", logfilename);
			logfilename = "";
		}
	}
*/

	linkfilename1 = "";
	replyfilename = "";

	//TCLSTACK = TCLSTACK.field(FM, 1, 10);
	//<arev>
//	var("RESET").execute();
	//</arev>

	if (tracing)
	printl(var("-").str(79));
	RECORD = "";
	ID = "";
	USER0 = "";
	USER1 = "";
	USER3 = "";
	USER4 = "";
	//garbagecollect;

	//linkfilename1=''

	//force into interactive mode for deb ugging
	//if tracing then
	if (1) {
		SYSTEM.r(33, 1);
	}else{
		SYSTEM.r(33, "");
	}
	//end else
	// end

	//force into server mode - suppress interactive messages
	batchmode = origbatchmode;
	if (batchmode == "") {
		batchmode = SYSTEM.a(33);
		SYSTEM.r(33, "1");
	}
	// if trim(@station)='sb_compaq' and @username='neosys' then system<33>=''

	//make a per process request input file
	while (true)
	{
		linkfilename1 = inpath ^ var(99999999).rnd() ^ ".0";
		linkfilename1.osdelete();
		if (linkfilename1.osfile()=="")
			break;

		//if (tracing)
	printl("CANNOT DELETE ", linkfilename1, " GENERATING ANOTHER");

	}
	linkfilename0 = linkfilename1.substr(inpath.length() + 1, 9999);

//printl("linkfilename1:", linkfilename1);
//printl("linkfilename0:", linkfilename0);

	//inpath
	//neopath
	//waitsecs
	//sleepms
	//datasetcode

	while (requestloop()){};

	return "";

}

function requestloop()
{

	//on win95 this runs at same speed in full screen or windowed

	locks.unlock( "REQUEST*" ^ linkfilename1);
	locks.unlock( "REQUEST*" ^ replyfilename);
//print 'unlock locks,request*':linkfilename1
//print 'unlock locks,request*':replyfilename

	timex = var().time();
	breaktime = "X";

	//call getstationid(stationid)
	//if lockrecord('messages',messages,stationid,'','') else null
	//indicate that we are active
//		messaging("CHECK");

	//TODO needs a break out of loop forever
	while (serviceloop()){}

	return false;

}

function serviceloop()
{

	//delete old response files every 10 requests or service loop
	if (nrequests[-1] == "0")
		gosub deleteoldfiles(inpath,".*\\.4$");

	//if tracing then
	logput(AT(0));
	logput((var().time()).oconv("MTS"));
	logput(" "^SYSTEM.a(17));
	//logput(" "^ROLLOUTFILE.field2("\\", -1).field(".", 1, 1));
	logput(" "^SYSTEM.a(24));
	logput(" "^nrequests);
	logput(" Listening ...");
	logput(AT(-4));

	//prevent sleep in esc.to.exit
	timenow=var().ostime().round(2);
	SYSTEM.r(25, timenow);
	SYSTEM.r(26, timenow);

	if (esctoexit())
		return false;

	//look for db stoppers in program directory
	for (int filen = 1; filen <= 2; filen++) {
		var filename = (globalend ^ VM ^ neopath ^ datasetcode ^ ".END").a(1, filen);
		if (filename.osfile()) {

			if (USER1.osread(filename)) {

				USER1.converter(var().chr(13) ^ var().chr(10), FM);

				if (USER1.a(1) == "") {
stopper:
					request1 = "STOPDB";
					return false;
				}

				var datetime = (var().date() ^ "." ^ (var().time()).oconv("R(0)#5")) + 0;

				var stopdatetime = (USER1.substr(1, 10)).iconv("D") ^ "." ^ (USER1.substr(11, 10)).iconv("MT");
				if (stopdatetime.isnum()) {
					if (datetime <= stopdatetime + 0)
						goto stopper;
				}

				filename.osdelete();

			}

		}

	};//filen;

	gosub flagserveractive();

//printl(inpath," ",".*\\.1$");

	var linkfilename;
	int nn=1000/sleepms*waitsecs;
	for (int ii=0;ii<=nn;ii++)
	{
		linkfilename=linkfilename.oslistf(inpath,".*\\.1$");
		if (linkfilename)
		{
			linkfilename=linkfilename.a(1);
			var(inpath^linkfilename).osrename(linkfilename1);
			break;
		}

//		if (var(neopath^"GLOBAL.END").osfile()!="") break;
//		if (var(neopath^datasetcode^".END").osfile()!="") break;

		linkfilename.ossleep(sleepms);

	}

	//if got file then process and quit serviceloop
	if (linkfilename1.osfile()!="") {

		linkfilenames = linkfilename0;

		return processlink();
	}


	//if time()>(timex+10*60) then

	//if not(tracing) and time()>(timestarted+10*60) then
	// request1='restart'
	// return 999999
	// end

	//handle midnight!
	if (var().time() < timestarted)
		timestarted = var().time();

	timex = var().time();

	//timeout if no activity
	timenow=var().ostime().round(2);
	var now = (var().date() ^ "." ^ (timenow.floor()).oconv("R(0)#5"));
	now+=0;
	//if timeouttime and now>timeouttime then return 999999

	gosub flagserveractive();

	//check for upgrade to Server

	//switch into interactive mode to check for operator input
	var s33 = SYSTEM.a(33);
	SYSTEM.r(33, "");

	//check for esc key to exit
	//if esc.to.exit() then return 999999

	var charx;
//		charx=keypressed();
	charx="";

	if (charx.length())
	{

		//charx.input(-1, 2);
		charx = charx[1].ucase();

		//esc
		if (charx == INTCONST.a(1)) {
			mssg("You have pressed the [Esc]  key to exit|press again to confirm|", "UB", buffer, "");
			while (true) {
				reply.input(-1);
			//BREAK;
			if (reply) break;;
			}//loop;
//				mssg("", "DB", buffer, "");
			if (reply == INTCONST.a(1))
				return false;
			if (reply == INTCONST.a(7))
				charx = reply;
		}

		//r=restart
		if (charx == "R") {
			request1 = "RESTART";
			return false;
		}

		//f5
		//if (charx == PRIORITYINT.a(2))
		//	var("").execute();

		//f10
		//if (charx == INTCONST.a(7))
		//	var("RUNMENU " ^ ENVIRONSET.a(37)).execute();

		if (charx != "") {

			//"d" = de bug
			//if charx='d' and @username='neosys' then
			// de bug
			// end

			gosub gettimeouttime();
		}

		//switch back to not interactive mode
		SYSTEM.r(33, s33);

		gosub getbakpars();

		//perform backup if right time range and not backed up today
		if (bakreq && var().date() != lastbakattemptdate) {

			var dow = var().date() - 1 % 7 + 1;

			//override min/max times
			//if @username='neosys' and trim(@station)='sbcp1800' then

			if (charx == "B" || ( bakdows.index(dow, 1) && timex > minbaktime && timex < maxbaktime)) {

				if (testdata)
					//var("OFF").perform();
					return false;

				lastbakattemptdate = var().date();
				lastbakattemptdate.writev(DEFINITIONS, "BACKUP", 1);

				USER4 = "";
				perform("FILEMAN BACKUP " ^ datasetcode ^ " " ^ bakdisk ^ " SYSTEM");

				//quit and indicate to calling program that a backup has been done
				//if tracing else
				PSEUDO = "BACKUP";
				if (USER4)
					return false;
				// end

			}
		}
	}

	gosub deleteoldfiles(inpath, "^.*$");

	return true;

}

function processlink()
{

//printl("processlink:"^linkfilenames);

	nrequests += 1;
	SYSTEM.r(35,0,0,nrequests);

	//get the earliest time possible for the log
	requeststarttime=var().ostime().round(2);
	SYSTEM.r(25, requeststarttime);

	gosub gettimeouttime();

	var nlinkfiles = linkfilenames.count(FM) + 1;

	//find a request to process (only one since renaming to target file)
	for (int linkfilen = 1; linkfilen <= nlinkfiles; linkfilen++) {

		linkfilename1 = inpath ^ linkfilenames.a(linkfilen);

		//lock it to prevent other Servers from processing it
		//unlock locks,'request*':linkfilename1
		if (!(lockrecord("", locks, "REQUEST*" ^ linkfilename1))) {
			if (tracing)
			{
				errputl("CANNOT LOCK LOCKS," ^ ("REQUEST*" ^ linkfilename1).quote());
			}
			continue;//goto nextlinkfile;
		}

		//open it or skip
		//var().osflush();
		if (!(linkfilename1.osopen())) {
			//TODO remove from future candidate files?
			unlockrecord("", locks, "REQUEST*" ^ linkfilename1);
			if (tracing)
			errputl("CANNOT OPEN "^ linkfilename1.quote());
			var().ossleep(100);
			continue;//goto nextlinkfile;
		}

		//get the .1 file which contains the request
		//timex=time()+2
readlink1:
/*
		//osbread request from linkfilename1 at 0 length 256*256-4
		var offset=0;
		USER0.osbread(linkfilename1, offset, 256 * 256 - 4);
*/
		//problem osreading utf8 data on linux ubuntu 13.10 x65
		//USER0.osread(linkfilename1, "utf8");
		var origrequest=osbread(linkfilename1,xx=0,999999);
		USER0=decode(origrequest);

		//if cannot read it then try again
		if (USER0 == "" && var().time() == timex) {
			//var().osflush();
			unlockrecord("", locks, "REQUEST*" ^ linkfilename1);
			var().ossleep(100);
			linkfilename1.osclose();
			if (tracing)
			errputl("CANNOT READ " ^ linkfilename1.quote());
			if (!(linkfilename1.osopen()))
				{}
			goto readlink1;
		}

//printl("processlink:read request ok");

		//hack to remove UTF16 BOM mark
		if (USER0[1]=="\uFEFF")
			USER0.splicer(1,1,"");

		//cleanup the input request
		//TODO check ALL CHARACTERS are converted F8-FF and on all input/output
		//convert '&' to fm in request
		USER0.swapper(var().chr(13) ^ var().chr(10), FM);
		USER0.converter(var().chr(13) ^ var().chr(10), _FM_ _FM_ );
		USER0.swapper("\\\\", "\\");
		USER0.swapper("\\r", FM);
		USER0.swapper("%FF", RM);
		USER0.swapper("%FE", FM);
		USER0.swapper("%FD", VM);
		USER0.swapper("%FC", SVM);
		USER0.swapper("%FB", TM);
		USER0.swapper("%FA", STM);
		USER0.swapper("%F9", SSTM);
		USER0.trimmerb(FM);

//USER0.outputl("USER0 after decode =");

		//extract and remove reply filename
		replyfilename = USER0.a(1);
		USER0.eraser(1, 0, 0);

		//check request is VERSION 3
		if (USER0.a(1) ne "VERSION 3") {
			if (tracing)
			errputl("REQUEST TYPE MUST BE VERSION 3 BUT IS "^USER0.a(1).quote());
			unlockrecord("", locks, "REQUEST*" ^ linkfilename1);
			continue;//goto nextlinkfile;
		}

		//lock the replyfilename to prevent other Servers from processing it
		//unlock locks,'request*':replyfilename
		if (!(lockrecord("", locks, "REQUEST*" ^ replyfilename))) {
			if (tracing)
			errputl("CANNOT LOCK LOCKS," ^ ("REQUEST*" ^ replyfilename).quote());
			unlockrecord("", locks, "REQUEST*" ^ linkfilename1);
			continue;//goto nextlinkfile;
		}

//printl("lock locks,request*"^replyfilename);

		//delete then unlock the request file
		var ntries = 0;
deleterequest:
		linkfilename1.osclose();
		//linkfilename1.osdelete();
		var savelinkfilename1=linkfilename1^"$";
		linkfilename1.osrename(savelinkfilename1);
		if (linkfilename1.osfile()) {
			//var().osflush();
			//garbagecollect;
			var().ossleep(100);
			ntries += 1;
			//if tracing then print 'could not delete ':linkfilename1
			if (ntries < 100)
				goto deleterequest;
			if (tracing)
			errputl("COULD NOT DELETE "^ linkfilename1.quote());
		}

		//leave these in place for the duration of the process
		//they should be cleared by unlock all somewhere at the end or beginning
		//unlock locks,'request*':replyfilename
		//unlock locks,'request*':linkfilename1

		//found a good one so process it
		var result=processrequest();

		//will be left behind for debugging if request crashes
		savelinkfilename1.osdelete();

		return result;

	};//linkfilen;

	return true;

}

function processrequest()
{

//printl("processrequest:",linkfilename1);

	nrequests += 1;

	//if tracing then
	//print @(0):@(-4):time() 'mts':' ':count(program.stack(),fm):
	//print @(0):@(-4):time() 'mts':' ':field2(linkfilename1,'\',-1):' ':field2(replyfilename,'\',-1):
	//print @(0):@(-4):time() 'mts':' ':field2(replyfilename,'\',-1):
	logput(AT(0)^
		AT(-4)^
		var().time().oconv("MTS")^
		" "
	);
	//end else
	// print @(25,@crthigh/2+1):
	// print ('processing request : ':nrequests) 'c#30':
	// end

	//clear out buffers just to be sure
	//request=''
	USER1 = "";
	//USER2=''
	USER3 = "Error: Response not set in Server.";
	USER4 = "";

	Serverfailure = 0;

	//request lines:

	//replyfilename (stripped out by the time we get here)

	//VERSION 3
	//6 (pre-fields after replyfilename)
	//127.0.0.1
	//127.0.0.1
	//https
	//session

	//database
	//usercode
	//password

	//request1 eg LOGIN
	//request2
	//etc.

	//analyse the input file
	var nconnectionfields = USER0.a(2);

	//extract and remove connection details
	//VERSION 3/ipno/hostname/https/session (nprefields is deleted)
	connection = USER0.field(FM, 1, nconnectionfields);
	connection.eraser(2);
	connection.converter(FM, VM);
	USER0 = USER0.field(FM,  nconnectionfields+1, 99999);

	//save connection details
	SYSTEM.r(40, connection);
//connection.outputl("connection=");
//USER0.outputl("USER0 after removal of connection=");

	//get and remove the dataset user and password
	dataset = USER0.a(1).ucase();
	username = USER0.a(2).ucase();
	password = USER0.a(3).ucase();
	USER0 = USER0.field(FM, 4, 99999);

	//remove CACHE prerequest
	if (USER0.a(1) == "CACHE")
		USER0.eraser(1);

	//USER0 is now the request
	//get the first 5 parts
	request1 = USER0.a(1).ucase();
	request2 = USER0.a(2);
	request3 = USER0.a(3);
	request4 = USER0.a(4);
	request5 = USER0.a(5);
//USER0.outputl("USER0 finally=");

/*
	if (logfilename!="") {

		var datex = var().date();
		timex = requeststarttime;

		var tt = var().chr(13) ^ var().chr(10) ^ "<Message ";
		tt ^= " Date=" ^ xmlquote(datex.oconv("D"));
		tt ^= " Time=" ^ xmlquote(timex.oconv("MTS"));
		tt ^= " DateTime=" ^ xmlquote(datex.oconv("DJ-") ^ "T" ^ timex.oconv("MTS") ^ "." ^ timex.field(".", 2, 1));
		tt ^= " User=" ^ xmlquote(username);
		tt ^= " File=" ^ xmlquote(replyfilename.field2("\\", -1));
		//remote_addr remote_host https
		tt ^= " IP_NO=" ^ xmlquote(connection.a(1, 2));
		tt ^= " Host=" ^ xmlquote(connection.a(1, 3));
		tt ^= " HTTPS=" ^ xmlquote(connection.a(1, 4));
		tt ^= ">" ^ (var().chr(13) ^ var().chr(10));

		tt ^= "<Request ";

		logx = USER0;
		gosub convlogx();
		logx.converter("^", FM);
		logx.r(1, request1);
		if (logx.a(1)!="")
			tt ^= " Req1=" ^ logx.a(1).quote();
		if (logx.a(5)!="")
			tt ^= " Req2=" ^ logx.a(5).quote();
		if (logx.a(6)!="")
			tt ^= " Req3=" ^ logx.a(6).quote();
		if (logx.a(7)!="")
			tt ^= " Req4=" ^ logx.a(7).quote();
		if (logx.a(8)!="")
			tt ^= " Req5=" ^ logx.a(8).quote();
		var req6up = logx.field(FM, 9, 9999);
		req6up.converter(FM, "^");
		if (req6up.length())
			tt ^= " Req6up=" ^ req6up.quote();

		tt ^= "/>" ^ (var().chr(13) ^ var().chr(10));
		tt.transfer(logx);
		gosub writelogx2();

		osbwritex("<DataIn>", logfilename, logfilename, logptr);

	}
*/
	var anydata = 0;

	if (request2 == "JOURNALS") {
		request2 = "BATCHES";
		if (request3.count("*") == 3 && request3[-1] == "*")
			request3.splicer(-1, 1, "");
	}

	//print the request (hide dataset and password)
	if (tracing) {
		var tt = username ^ " " ^ USER0;
		tt.convert(FM, " ");

		var t2 = connection.a(1, 2);
		if (connection.a(1, 3) != t2)
			t2 ^= " " ^ connection.a(1, 3);

		t2.trimmer();
		logputl(" " ^ (t2 ^ " " ^ tt).trim().quote());
	}

	linkfilename2 = replyfilename.splice(-1, 1, 2);
	linkfilename2.swapper("/\\",SLASH^SLASH);	
	//if (linkfilename2.index(":"))
	//{
	//	linkfilename2.swapper(":",SLASH);
	//	linkfilename2="/cygdrive/"^linkfilename2;
	//}

	linkfilename3 = replyfilename.splice(-1, 1, 3);
	linkfilename3.swapper("/\\",SLASH^SLASH);
	//if (linkfilename3.index(":"))
	//{
	//	linkfilename3.swapper(":",SLASH);
	//	linkfilename3="/cygdrive/"^linkfilename3;
	//}

	//temporary responsefilename before renaming
	linkfilename5 = linkfilename3.splice(-1, 1, 5);

	//save the response file name
	//so that if Server fails then net the calling program can still respond
	PRIORITYINT.r(100, linkfilename3);

	USER1="";
	var linkfilename2size = linkfilename2.osfile().a(1);
	if (!linkfilename2size) {
		//zero file size not nice or necessary but tolerable

	} else if (linkfilename2size > maxstringsize) {
		USER3 = "Error: Maximum record size " ^ maxstringsize ^ " exceeded in Server";
		Serverfailure = 1;

	}else if (!(linkfilename2.osopen())) {
		USER3 = "Error: Server cannot open " ^ linkfilename2;
		Serverfailure = 1;

	}else if (not (USER1=osbread(linkfilename2,xx=0,999999))) {
		USER3 = "Error: Server cannot read " ^ linkfilename2;
		Serverfailure = 1;

	} else {
		USER1=decode(USER1);
	}

	if (linkfilename2.osopen()) {
		linkfilename2.osclose();
		linkfilename2.osdelete();
	}

	//update security table every few secs and every login
	if (request1 == "LOGIN" || var("036").index(var().time())[-1])
		gosub getsecurity();

	//validate username and password (like login)
	//and assume that identity if ok
	gosub validate();

	SYSTEM.r(2, linkfilename2);

	try
	{
		gosub process();
	}
//dont catch general MVExceptions so we can debug
//	catch (MVException& mvexception)
	catch (std::wstring& message)
	{
		USER3 = message;
		//gosub properunlock();
		gosub formatresponse();
	}
	catch (var& message)
	{
		USER3 = message;
		//gosub properunlock();
		gosub formatresponse();
	}

	//restore the program stack
	//limit on 299 "programs" and dictionary entries count as 1!!!
//	xx = programstack(stack);

	gosub requestexit();

	if (halt)
		return false;

	if (request1 == "STOPDB" && USER3 == "OK")
		return false;

	if (request1 == "RESTART" && USER3 == "OK")
		return false;

	return true;

}

subroutine login()
{

	USER1 = "";
	USER4 = "";
	var authcompcodes;

	//custom login routine
	//returns iodat (cookie string "x=1&y=2" etc and optional comment)
	//if (xx.read(md, "LOGIN.NET")) {
		USER1 = "";
		USER4 = "";
		loginnet(dataset, username, USER1, USER4, authcompcodes);
		if (USER1 == "") {
			USER3 = USER4;
			return;
		}
	//}

	if (!(USER1))
		USER1 = "X=X";
	USER3 = var("OK " ^ USER4).trim();

	//record the last login per user
	var users;
	if (users.open("USERS")) {
		var userrec;
		if (userrec.read(users, username)) {
			//save prior login
			userrec.r(15, userrec.a(13));
			userrec.r(16, userrec.a(14));
			//save current login
			var datetime = var().date() ^ "." ^ (var().time()).oconv("R(0)#5");
			userrec.r(13, datetime);
			userrec.r(14, SYSTEM.a(40, 2));

			if (authcompcodes) {
				userrec.r(33, authcompcodes);
			}

			userrec.write(users, username);

		}
	}

	return;
}

subroutine validate()
{
	invaliduser = "Error: Invalid username and/or password";
//printl("validate");
	//encrypt the passwors and check it
	var encrypt0 = encrypt2(password ^ "");
	var usern;
	var systemrec;
	var tt;
	var connections;

	if (username=="NEOSYS") goto userok;
	else if (SECURITY.locate(username, usern, 1)) {
		if (encrypt0 == SECURITY.a(4, usern, 2).field(TM, 7)) {

			//determine allowed connections
			connections = SECURITY.a(6, usern);

			//skip old integer sleep times
			if (connections.match("1N0N"))
				connections = "";

			if (!connections) {

				//look for ip numbers in following users of the same group
				int nn = (SECURITY.a(6)).count(VM) + 1;
				for (int ii = usern + 1; ii <= nn; ii++) {
					connections = SECURITY.a(6, ii);

					//skip old integer sleep times
					if (connections.match("1N0N"))
						connections = "";

				//BREAK;
				if (!(!connections && SECURITY.a(1, ii + 1) != "---")) break;;
				};//ii;

				//otherwise use system default
				if (!connections)
					connections = SYSTEM.a(39);

				//otherwise use traditional private ip ranges
				//cannot implement this until check existing clients installations
				//if connections else connections='192.168.*.*;10.*.*.*'

			}

			//convert wildcards
			if (connections[-1] != " ") {

				connections.converter(",; ", SVM ^ SVM ^ SVM);
				int nn = connections.dcount(SVM);
				for (int ii = 1; ii <= nn; ii++) {

					var connectionx = connections.a(1, 1, ii).quote();
					connectionx.swapper("*", "\"0N\"");

					//invert the bits that are "quoted" to suit the match syntax '"999"0n'
					//""0n"."0n"" will become 0n"."0n
					connectionx.swapper("\"\"", "");

					//pad missing dots so that 192.* becomes 192"."0n"."0n"."0n
					var ndots = connectionx.count(".");
					if (ndots < 3)
						connectionx ^= var("\".\"0N").str(3 - ndots);

					connections.r(1, 1, ii, connectionx);
				};//ii;

				//buffer it (space prevents reassement even if none)
				//trailing space also means wildcards have been converted
				SECURITY.r(6, usern, 0, connections ^ " ");
			}

			//check if allowed to connect from xxx

			if (connections.trim()) {
				int nn = connections.dcount(SVM);
				int ii;
				for (ii = 1; ii <= nn; ii++) {
					var connectionx = (connections.a(1, 1, ii)).trimb();
				//BREAK;
				if ((connection.a(1, 2)).match(connectionx)) break;;
				};//ii;
				if (ii > nn) {
					invaliduser = username ^ " is not authorised to login" ^ FM ^ "on this computer (IP Number: " ^ connection.a(1, 2) ^ ")";
						return;
					}
				}

userok:
				USERNAME=username;
				tt = connection.a(1, 2);
				if (connection.a(1, 3) != tt)
					tt ^= "_" ^ connection.a(1, 3);
			tt.converter(". ", "_");
			STATION=tt;
			invaliduser = "";
		}
	}else{

		//check revelation system file
		if (systemrec.read(DEFINITIONS, "SYSUSER*"^username)) {
			//if systemrec<7>=encrypt0 and (speed<1.5 or mode='sleep') then ok=1
			if (systemrec.a(7) == encrypt0)
				goto userok;
		}
username.outputl("cant find user in definitions, security = ");

	}

	//return

}

subroutine requestexit()
{
	var().unlockall();

	for (var ii=0;ii<10 and var().statustrans();++ii) {
		printl("Rolling back uncommitted transaction");
		var().rollbacktrans();
	}
		
	if (USER3 == "")
		USER3 = "Error: No response";

//printl("requestexit:"^USER3.quote());

	if (USER3.index("ERROR NO:", 1))
		logger("Server", USER3);

	USERNAME=Serverusername;
	STATION=Serverstation;

	//if USER3[1,6].ucase()='ERROR:' then iodat=''

	//have to call it here as well :(
	requeststoptime=var().ostime().round(2);

	var rawresponse = USER3;
	rawresponse.converter(var().chr(13) ^ var().chr(10), "|");

/*
	if (logfilename!="") {

		var secs = (requeststoptime - requeststarttime).oconv("MD20P");
		logx = "<Response";
		logx ^= " ProcessingSecs=" ^ secs.quote();
		logx ^= ">";
		gosub writelogx2();

		logx = USER3;

		//convert non ascii to hexcode
		//escape % to %25 FIRST!
		logx.swapper("%", "%25");
		//for (int ii = 128; ii <= 255; ii++)
		for (int ii = 249; ii <= 255; ii++)
			logx.swapper(var().chr(ii), hexx[ii]);
		for (int ii = 0; ii <= 31; ii++)
			logx.swapper(var().chr(ii), hexx[ii]);

		gosub writelogx();

		logx = "</Response>" ^ (var().chr(13) ^ var().chr(10));
		iodatlen = USER1.length();
		if (iodatlen)
			logx ^= "<DataOut>";
		gosub writelogx2();

	}
*/

	if (USER1 == "%DIRECTOUTPUT%") {

/*
		logx = USER1;
		logx.swapper("%", "%25");
		gosub writelogx();
*/
	}else{

		//var("").oswrite(linkfilename2);
		//if (linkfilename2.osopen()) {

/*
			//escape the escape character first!
			USER1.swapper("%", "%25");

			//allow top eight pick language characters to pass through
			//to whatever corresponding unicode characters are desired by front end
			for (int ii = 249; ii <= 255; ii++) {
				USER1.swapper(var().chr(ii), hexx[int(ii)]);
				//should not be done per block but is code economic
				USER3.swapper(var().chr(ii), hexx[int(ii)]);
			};//ii;
*/
			//write BEFORE converting control characters
			//since writing restores 10-17 back up to F8-FF
/*
			var ptr=0;
USER1.outputl("USER1 written:");
			osbwritex(USER1, linkfilename2, linkfilename2, ptr);
//USER1.outputl("USER1 written:");
			ptr += USER1.length();
*/

		if (oswrite(encode(USER1), linkfilename2, "utf8")) {

/*
			//encode control characters to show in log
			//should convert x10-x17 back to proper characters
			for (int ii = 0; ii <= 31; ii++) {
				USER1.swapper(var().chr(ii), hexx[int(ii)]);
				//should not be done per block but is code economic
				USER3.swapper(var().chr(ii), hexx[int(ii)]);
			};//ii;

			if (logfilename!="") {
				USER1.transfer(logx);
				gosub writelogx();
			}
*/
			USER1 = "";

			linkfilename2.osclose();

		}else{

			USER3 = "ERROR: Server cannot create temp " ^ linkfilename2;

		}

	}

	//try to flush file open
	//if (linkfilename2.osopen())
	//	linkfilename2.osclose();
/*
	if (logfilename!="") {
		var tt = "";
		if (iodatlen)
			tt ^= "</DataOut>";
		tt ^= "</Message>" ^ (var().chr(13) ^ var().chr(10));
		osbwritex(tt, logfilename, logfilename, logptr);
		logptr += tt.length();

		osbwritex("</Log>", logfilename, logfilename, logptr);

	}
*/
	//swap '|' with wchar_t(13) in USER3
	//swap fm with wchar_t(13) in USER3

//printl("writing response:", USER3, linkfilename3);
	//write the response

	oswrite(encode(USER3), linkfilename5, "utf8");
	osdelete(linkfilename3);
	osrename(linkfilename5, linkfilename3);

	//trace responded
	requeststoptime=var().ostime().round(2);
	if (tracing) {

		printl("Responding in ", (requeststoptime - requeststarttime).oconv("MD20P"), " SECS ", rawresponse);
		//print linkfilename1
	}

	//flush
	//suspend 'dir>nul'

	//if tracing then print

	return;

}

subroutine exit()
{

/*
	if (logfilename!="")
		logfilename.osclose();
*/
	//remove lock indicating processing
	//gosub sysunlock

	var().unlockall();

	//restorescreen(origscrn, origattr);

	//remove flag that this dataset is being served ("listened")
	//osdelete inpath:serverflagfilename

	//get into interactive mode
	//system<33>=origbatchmode
	SYSTEM.r(33, "");
	PRIVILEGE=origprivilege;

	if (request1 == "RESTART") {
		//chain 'Server'
		SYSTEM.r(35, nrequests);
		//msg='restart'
		//stop

		origsentence.swapper(" INTRANET", "");
		origsentence.chain();
	}

	//BREAK OFF;
	//BREAK ON;

	//if (origbatchmode || request1 == "STOPDB" || halt) {
	//	//BREAK OFF;
		//var("OFF").perform();
	//}

	//msg is @user4
	USER4 = "TERMINATED OK";

	var().stop();

}

subroutine process()
{
//printl("process:"^USER0);
	//process the input
	//////////////////

	//failure in Server above
	if (Serverfailure) {

	//invalid username or password or connection
	}else if (invaliduser!="") {
		USER1 = "";
		USER3 = invaliduser;
//call oswrite(username:fm:password:fm:encrypt0:fm:SECURITY,date()[-3,3]:time():'.$$$')
		//if no request then possibly some failure in file sharing

	}else if (USER0 == "") {
		USER1 = "";
		USER3 = "Error: No request";

	//empty loopback to test connection (no username or password required)
	}else if (request1 == "TEST") {
		//iodat='' if they pass iodat, pass it back
		USER3 = "OK";

	//check if can login
	}else if (request1 == "LOGIN") {

		gosub login();

	//find index values
	}else if (request1.substr(1, 14) == "GETINDEXVALUES") {

		getindexvalues();

	//select some data
	}else if (request1 == "SELECT") {

		gosub requestselect();

	//lock a record
	}else if (request1 == "LOCK" || request1 == "RELOCK") {

		filename = request2;
		keyx = request3;
		sessionid = request4;

		//lockduration is the number of minutes to automatic lock expiry
		lockdurationinmins = request5;

		gosub lock();
		sessionid.transfer(USER1);

	}else if (request1 == "UNLOCK") {

		filename = request2;
		keyx = request3;
		sessionid = request4;

		gosub unlock();

		//read a record

	}else if (request1 == "READ" || request1 == "READO" || request1 == "READU") {

		withlock = request1 == "READU";
		updatenotallowed = "";
		USER1 = "";
		sessionid = "";

		filename = request2;
		keyx = request3;
		lockdurationinmins = request4;
		var readenvironment = request5;
		win.templatex = readenvironment;

		//reduce chance of using old common
		//for (int ii=0;ii<10;ii++)
		//	win.registerx[ii]="";
		win.registerx="";

		var library="";
/*TODO reimplement as simple external function
		if (!library.load(filename))
			{};//throw MVException("MVCipc() library.load() " ^ filename.quote() ^ " unknown filename");
*/
		newfilename="";
		//if (library) newfilename=library.call("GETALIAS");

		//disallow read/write to unknown files
		if (false&&newfilename == "") {
			USER3 = request1 ^ " " ^ filename.quote() ^ " is not allowed";
			return;
		}
		if (newfilename == "") newfilename = filename;

		filename2 = singular(newfilename);
		filename2.converter(".", " ");
		//if security(filename2:' access',msg0,'') else
		// if security('#':filename2:' access ':quote(keyx),msg2,'') else
		// transfer msg0 to USER3
		// gosub formatresponse
		// return
		// end
		// end
		if (!filesecurity("ACCESS")) return;

		//security check - cannot lock so cannot write or delete
		if (withlock) {
			//if 1 then
			if (authorised(filename2 ^ " UPDATE", updatenotallowed, ""))
				{}
			if (authorised(filename2 ^ " CREATE", createnotallowed, ""))
				{}
			//if updatenotallowed and createnotallowed then
			// USER3='error: sorry, you are not authorised to create or update records in the ':lcase(filename):' file.'
			// *USER3='error: lock not authorised'
			// gosub formatresponse
			// return
			// end
		}

		var file;
		if (!file.open(filename)) {
			gosub badfile();
			return;
		}

		if (library) {
			keyx.transfer(ID);
/*TODO reimplement as simple external function
			library.call("PREREAD");
*/
			DATA = "";
			ID.transfer(keyx);
		}

		var autokey = 0;
		if (keyx == "" || keyx[1] == "*" || keyx[-1] == "*" || keyx.index("**", 1)) {

			//must provide a key unless locking
			if (!withlock) {
				USER3 = "Error: NEOSYS.NET Key missing and not READU";
					gosub formatresponse();
					return;
				}

getnextkey:

				//setup environment for def.sk
			win.wlocked = 0;
			RECORD = "";
			ID = keyx;
			win.datafile = filename;
			win.srcfile = file;

//TODO			generalsubs("DEF.SK." ^ readenvironment);
//			keyx = isdflt;
keyx="";

			if (keyx == "") {
				USER3 = "Error: Next number was not produced" ^ FM ^ USER4;
				gosub formatresponse();
				return;
			}

			autokey = 1;

		}

		sessionid = "";
		if (withlock) {
			//lockduration is the number of minutes to automatic lock expiry
			//lockdurationinmins=request4
			lockdurationinmins = 10;
			USER3 = "";

			gosub lock();

			//if cannot lock then get next key
			if (USER3 == "NOT OK" && autokey)
				goto getnextkey;

			//cannot do this because should return the record but without a lock
			//even if a lock requested
			//if USER3 then
			// gosub formatresponse
			// return
			// end
		}

		FILEERRORMODE = 1;
		FILEERROR = "";
		if (USER1.read(file, keyx)) {

			//if record already on file somehow then get next key
			if (autokey)
				goto getnextkey;

			//prevent update
			if (withlock && updatenotallowed) {
				if (sessionid)
					gosub unlock();
				// *do this after unlock which sets response to ok
				//USER3=updatenotallowed
				//gosub formatresponse
				//return
				lockauthorised = 0;
				sessionid = "";
			}

			if (withlock && sessionid == "") {
				USER3 = "Error: CANNOT LOCK RECORD";
			}else{
				USER3 = "OK";
			}
			if (sessionid)
				USER3 ^= " SESSIONID " ^ sessionid;
		}else{
			//if @file.error<1>='100' then
			//no file error for jbase
			if (!FILEERROR || FILEERROR.a(1) == "100") {

				//prevent create
				if (withlock) {
					if (createnotallowed) {
						if (sessionid)
							gosub unlock();
						//do this after unlock which sets response to ok
						USER3 = createnotallowed;
						gosub formatresponse();
						return;
					}
				}

				//USER3='error: ':quote(keyx):' does not exist in the ':quote(newfilename):' file'
				USER1 = "";
				USER3 = "Error: NO RECORD";
				if (sessionid)
					USER3 ^= " SESSIONID " ^ sessionid;

				//response/@user3/recordkey may be used in postread
				if (autokey) {
					//USER3:=' recordkey ':keyx
					var tt = keyx;
					//horrible cludge to allow space in recordkey to be understood in client.htm
					tt.swapper(" ", "{20}");
					USER3 ^= " RECORDKEY " ^ tt;
				}

			}else{
				gosub geterrorresponse();
			}
		}

		if (withlock) {
			if (!lockauthorised)
				USER3 ^= " LOCK NOT AUTHORISED";
		}

		//postread (something similar also in select2)
		if (library) {

			//simulate environment for postread
			win.srcfile = file;
			win.datafile = filename;
			if (!DICT.open("dict_"^win.datafile)) {
				USER3 = "Server::gosub process() DICT." ^ win.datafile ^ " file cannot be opened";
				gosub formatresponse();
				if (sessionid)
					gosub unlock();
				return;
			}
			keyx.transfer(ID);
			USER1.transfer(RECORD);
			win.orec = RECORD;
			win.wlocked = sessionid;
			var origresponse = USER3;
			//_USER3=''
			USER4 = "";
			var resetx = 0;
/*TODO reimplement as simple external function
			library.call("POSTREAD");
*/
			DATA = "";

//restore this programs environment
			RECORD.transfer(USER1);
			ID.transfer(keyx);

			USER1.cropper();

			//postread can request abort by setting msg or resetx>=5
			if (resetx >= 5 || USER4) {
				if (withlock) {
					gosub unlock();
					//win.wlocked=0
				}
				//if msg then msg='error: ':msg
				USER3 = USER4;
				if (USER3 == "")
					USER3 = "ACCESS REFUSED";
				USER1 = "";
				gosub formatresponse();
				//USER3='error: ':USER3
			}else{
				USER3 = origresponse;
				//postread may have provided a record where non-was found
				//only if it unlocks it as well! otherwise
				//removal of no record will cause failure in client
				if (!win.wlocked && USER1) {
					USER3.swapper("Error: NO RECORD", "");
					USER3.trimmerb();
					if (USER3 == "")
						USER3 = "OK";
				}

			}

			//postread may have unlocked the record
			//(and removed it from the locks file)
			//but make sure
			if (sessionid && !win.wlocked) {

				var storeresponse = USER3;
				gosub unlock();
				USER3 = storeresponse;

				//remove session id
				var tt = USER3.index("SESSIONID", 1);
				if (tt) {
					USER3.splicer(tt, var("SESSIONID " ^ sessionid).length(), "");
					USER3.trimmerb();
					if (USER3 == "")
						USER3 = "OK";
				}

			}

			//do not indicate record found
			//otherwise delete button will be enabled and deleting a record
			//that does not exist is disallowed but lock will be unlocked
			//if iodat then
			// USER3='ok'
			// if sessionid then USER3:=' sessionid ':sessionid
			// end

		}

		//prevent reading passwords postread and postwrite
		if (filename == "DEFINITIONS" && keyx == "SECURITY")
			USER1.r(4, "");

		USER1.cropper();

	}else if (request1 == "WRITEU" || request1 == "DELETE" || request1 == "WRITE") {

		filename = request2;
		keyx = request3;
		var fieldno = request4;
		sessionid = request5;

//		routineexists=routines.count(filename.towstring());
//		MVRoutine *routine;
//		if (routineexists)
//		{
//			routine=routines[filename.towstring()];
//			newfilename=routine->execute("GETALIAS");
//		}
//		else
		var library="";
/*TODO reimplement as simple external function
		if (!library.load(filename))
			{};
*/
	   newfilename="";
		//if (library) newfilename=library.call("GETALIAS");

		//disallow read/write to unknown files
		if (false&&newfilename == "") {
			USER3 = request1 ^ " " ^ filename.quote() ^ " is not allowed";
			return;
		}
		if (newfilename == "") newfilename = filename;

		filename2 = singular(newfilename);
		filename2.converter(".", " ");

		//double check allowed access to file
		if (!filesecurity("ACCESS")) return;

		//if security(filename2:' access',msg0,'') else
		// if security('#':filename2:' access ':quote(keyx),msg2,'') else
		// transfer msg0 to USER3
		// gosub formatresponse
		// return
		// end
		// end

		//simulate environment

		//reduce chance of using old common
		//for (int ii=0;ii<10;ii++)
		//	win.registerx[ii]="";
		win.registerx="";

		ID = keyx;
		//@record=iodat
		USER1.transfer(RECORD);
		win.datafile = filename;
		//not really needed because pre/post code should assume that it is win.wlocked
		//but some code does not know that (eg postread called from postwrite)
		win.wlocked = sessionid;
		win.saverec = !(request1 == "DELETE");
		win.deleterec = (request1 == "DELETE");

		//trim excess field and value marks
		RECORD.cropper();

		if (!win.srcfile.open(win.datafile)) {
			USER3 = win.datafile.quote() ^ " file is not available";
			return;
		}

		if (!DICT.open("dict_"^win.datafile)) {
			USER3 = ("DICT." ^ win.datafile).quote() ^ " file is not available";
			return;
		}

		//make sure that the record is already locked to the user
		lockkey = filename ^ "*" ^ ID;
		if (!lockx.read(locks, lockkey))
			lockx = FM ^ FM ^ FM ^ FM ^ "NO LOCK RECORD";
		if (sessionid != lockx.a(5)) {
			USER3 = "Somebody has updated this record." _VM_ "Your update cannot be applied." "" _VM_ "The session id does not agree " ^ lockx.a(5).quote();
			gosub formatresponse();
			return;
		}

		//get a proper lock on the file
		//possibly not necessary as the locks file entry will prevent other programs
		//proper lock will prevent index mfs hanging on write

		win.valid = 1;
		gosub properlock();
		if (!win.valid)
			return;

		if (!win.orec.read(win.srcfile, ID))
			win.orec = "";

		//trim excess field and value marks
		win.orec.cropper();

		//double check not updated by somebody else
		//nb this does not work for delete until client provides
		//record or at least date_time in delete
		//not implemented for timesheets due to possibility of old mac client not working
		if (request1 != "DELETE" && filename != "TIMESHEETS") {
			// *cannot do this unless
			//a) return new record (done) or at least date_time in writeu to client
			//b) provide record in delete
			if (allcols != "") {
				var dictrec;
				//if (dictrec.reado(allcols, filename ^ "*DATE_TIME")) {
				if (dictrec.read(allcols, filename ^ "*DATE_TIME")) {
					var datetimefn = dictrec.a(2);
					var olddatetime = win.orec.a(datetimefn);
					var newdatetime = RECORD.a(datetimefn);
					if (olddatetime && olddatetime != newdatetime) {
						USER3 = "Somebody else has updated this record." _VM_ "Your update cannot be applied." "" _VM_ "The time stamp does not agree";
						gosub formatresponse();
						gosub properunlock();
						return;
					}
				}
			}
		}

		//detect if custom postwrite or postdelete called
		var postroutine = 0;

		if (request1 == "WRITEU" || request1 == "WRITE") {

			//prevent writing an empty record
			if (RECORD == "") {
emptyrecorderror:
					USER3 = "Write empty data record is disallowed.";
				gosub properunlock();
				gosub formatresponse();
				return;
			}

			//double check that the record has not been updated since read
			//cannot do this unless during write (not writeu) we pass back the new timedate
			//readv datetimefn from @dict,'date_time',2 then
			// if @record<datetimefn> ne win.orec<datetimefn> then
			// _USER3='somebody else has updated this record." _VM_ "your update cannot be applied'
			// gosub properunlock
			// gosub formatresponse
			// return
			// end
			// end

			//custom prewrite processing
			if (library) {
				//call @updatesubs('prewrite')
/*TODO reimplement as simple external function
				library.call("PREWRITE");
*/
				DATA = "";
			}

			if (!win.valid) {
				gosub properunlock();
				USER3 = USER4;
				gosub formatresponse();
				return;
			}

			//allow for prewrite to change the key (must not lock the new key)
			//prewrite must not unlock the record because write will fail by index mfs hanging if any
			//is the above true? because unlock just below will
			//remove the lock file record
			//and actually the unlock will fail if it cannot true lock the record
			//daybook.subs4 unlocks and it works fine and will leave lock hanging if it does not
			if (ID != keyx) {

				gosub unlock();
				gosub properunlock();

				keyx = ID;

				//cannot place a lock file entry if the prewrite has locked the new key
				file.unlock( keyx);

				gosub lock();
				if (USER3 != "OK")
					return;

				gosub properlock();
				if (!win.valid)
					return;

			}

			RECORD.cropper();

			//check for empty record again in case updatesubs screwed up
			if (RECORD == "")
				goto emptyrecorderror;

			//failsafe in case prewrite unlocks key?
			//gosub properlock

			RECORD.write(win.srcfile, keyx);

			//custom post write processing
			if (library) {
				postroutine = 1;
/*TODO reimplement as simple external function
				library.call("POSTWRITE");
*/
				DATA = "";
			}

			//send back revised data or nothing
			//nb data is now '' to save space so always send back data unless @record is cleared
			if (RECORD == USER1) {
				USER1 = "";
			}else{
				USER1 = RECORD;
			}

			//prevent reading passwords postread and postwrite
			if (filename == "DEFINITIONS" && keyx == "SECURITY")
				USER1.r(4, "");

		}else if (request1 == "DELETE") {

			//ensure that deletion works on the orig record
			RECORD = win.orec;

			//prevent deleting if record does not exist
			if (RECORD == "") {
				USER3 = "Cannot delete because " ^ keyx.quote() ^ " does not exist in the " ^ newfilename.quote() ^ " file";
				gosub formatresponse();
				return;
			}

			//check allowed to delete
			//if security(filename2:' delete',msg,'') else
			// USER3=msg
			// gosub formatresponse
			// return
			// end
			if (!filesecurity("DELETE")) return;

			//custom predelete processing
			if (library) {
				//call @updatesubs('predelete')
/*TODO reimplement as simple external function
				library.call("PREDELETE");
*/
				DATA = "";
			}
			if (!win.valid) {
				gosub properunlock();
				USER3 = USER4;
				gosub formatresponse();
				return;
			}

			win.srcfile.deleterecord(keyx);

			//custom post delete processing
			if (library) {
				postroutine = 1;
/*TODO reimplement as simple external function
				library.call("POSTDELETE");
*/
				DATA = "";
			}

			//send back no iodat
			USER1 = "";

			}

			//remove locks file entry
		if (request1 != "WRITE") {

			locks.deleterecord(lockkey);
			relistlocks = 1;

			//unlock local lock
			win.srcfile.unlock( keyx);

		}else{
			gosub properunlock();
		}

		//even postwrite/postdelete can now set invalid (to indicate invalid mode etc)
		if (win.valid) {
			USER3 = "OK";
		}else{
			USER3 = "Error:";
		}

		if (request1 != "DELETE") {
			var tt = ID;
			//USER3:=' recordkey ':@id
			//horrible cludge to allow space in recordkey to be understood in client.htm
			tt.swapper(" ", "{20}");
			USER3 ^= " RECORDKEY " ^ tt;
			if (sessionid)
				USER3 ^= " SESSIONID " ^ sessionid;
		}

		//pre and post routines can return warnings/notes in msg

		if (USER4)
			USER3 ^= " " ^ USER4;

		gosub formatresponse();

		//if postroutine else call flush.index(filename)
		//mv.flushindex(filename);

		//execute a request

	}else if (request1 == "EXECUTE") {

		//for security, requests can only call program names
		//ending in proxy like mediaproxy, productionproxy etc		
		var mdcmd = request2;
		if (mdcmd == "") {
//badproxy:
			USER3 = "Error: " ^ mdcmd.quote() ^ " proxy is not available";
			return;
		}
		mdcmd ^= "PROXY";
		
		//provide an output file for the program to be executed
		//nb response file name for detaching processes
		//will be obtained from the output file name Server2 respond
		//this could be improved to work
		var printfilename = linkfilename2;
		var tt = var().oscwd();
		tt.splicer(-7, 7, "");
		if (printfilename.substr(1, tt.length()) == tt)
			printfilename.splicer(1, tt.length(), "..\\");

		//t=printfilename[-1,'b.']
		tt = printfilename.field2(".", -1);
		printfilename.splicer(-tt.length(), tt.length(), "htm");
		SYSTEM.r(2, printfilename);
		if (tracing)
		logput("Waiting for output ... ");

		//switch to server mode and html output
		var s6 = SYSTEM.a(6);
		var s7 = SYSTEM.a(7);
		var s11 = SYSTEM.a(11);
		SYSTEM.r(6, 1);
		SYSTEM.r(7, 1);
		SYSTEM.r(11, 1);

		//execute the program
		USER3 = "OK";
		win.valid = 1;
		USER4 = "";
		//call @mdcmd(field(request,fm,3,99999),iodat,_USER3)
		//request, iodat and response are now passed and returned in @user0,1 and 3
		//other messages are passed back in @user4
		//execute instead of call prevents program crashes from crashing Server
		//USER0 = USER0.field(FM, firstrequestfieldn + 2, 99999);

		//pass the output file in linkfilename2
		//not good method, pass in system?
		if (var("LIST SELECTBATCHES").locateusing(USER0.a(1)," "))
			USER1 = linkfilename2;
		if ((USER0.a(1)).substr(1, 4) == "VAL.")
			USER1 = linkfilename2;

		//cut off EXECUTE and proxyname, leaving on the real request to the proxy program
		USER0=USER0.field(FM,3,9999);

		//if we have debugger attached, perform directly without try/catch
		if (false) {
			try {
				perform(mdcmd.lcase());
			} catch (...) {
				outputl("error in " ^ mdcmd.lcase());
			}
		} else {
			perform(mdcmd.lcase());
		}

		//discard any stored input
		DATA = "";

		//send errors to neosys
		if (USER4.index("An internal error", 1))
			sysmsg(USER4);

		USER4.cropper();
		USER3.cropper();

		if (USER4) {
			USER1 = "";
			USER3 = "Error: " ^ USER4;
			gosub formatresponse();
		}

		if (USER3 == "") {
			USER3 = "Error: No OK from " ^ mdcmd ^ " " ^ USER0;
			gosub formatresponse();
		}

		//switch off server mode
		SYSTEM.r(6, s6);
		SYSTEM.r(7, s7);
		SYSTEM.r(11, s11);

		//get the printfilename in case the print program changed it
		printfilename = SYSTEM.a(2);
		if (tracing) {
			logput(AT(0)^ AT(-4));
		}

		//make sure that the output file is closed
		if (printfilename.osopen())
			printfilename.osclose();

	}else if (request1 == "STOPDB") {

		if (globalend.osfile()) {
			USER3 = "Error: Database already stopped/stopping";

		}else{

			var("").oswrite(globalend);

			timex = var().time();
			while (true) {

				if (not (otherusers("").a(1) && (var().time() - timex).abs() < 30))
					break;
					
				var().ossleep(1000);
				
			}//loop;

			USER1 = "";

			if (otherusers("").a(1)) {
				USER3 = "Error: Could not terminate " ^ otherusers("") ^ " users|" ^ otherdatasetusers("*");
				globalend.osdelete();
			}else{
				var("NET STOP NEOSYSSERVICE").osshell();

				if (request2 == "RESTART") {
					globalend.osdelete();
					var("NET START NEOSYSSERVICE").osshell();
				}
				USER3 = "OK";
			}

		}

	}else if (request1 == "RESTART") {
		USER1 = "";
		USER3 = "OK";

	}else if (request1 == "BACKUP") {
		gosub getbakpars();

		//force additional processes ... that should fail due to backup lock
		(inpath ^ serverflagfilename).osdelete();

		//backup will respond to user itself if it starts
		USER4 = "";
		perform("FILEMAN BACKUP " ^ datasetcode ^ " " ^ bakdisk);

		//if backup has already responded to user
		//then quit and indicate to calling program that a backup has been done
		//user will be emailed
		if (SYSTEM.a(2) == "") {
			PSEUDO = "BACKUP2";
			if (USER4)
				var().stop();
		}

		//note: if backup did not respond already then the requestexit will
		//respond as usual with the error message from backup
		USER1 = "";

	}else if (request1 == "VERSION") {
		USER1 = "";
		USER3 = "UNKNOWN VERSION";

	}else if (request1 == "INSTALL") {
		DATA ^= var().chr(13);
		if (request2 == "")
			request2 = APPLICATION;
		execute("INSTALL " ^ request2 ^ " I !:");
		USER3 = USER4;

		//or give an error message

	}else{
		USER1 = "";
		USER3 = "Error: " ^ USER0.field(FM, 4, 9999).quote() ^ " unknown request";
	}

	return;

}

subroutine geterrorresponse()
{
	var fileerror = FILEERROR;
	USER3 = "Error: FS" ^ fileerror.a(1, 1);//.xlate("SYS_MESSAGES", 11, "X");
//	USER3.swapper("%1%", handlefilename(fileerror.a(2, 1)));
	USER3.swapper("%2%", fileerror.a(2, 2));
	gosub formatresponse();
	return;

}

subroutine formatresponse()
{

	//trim everything after <esc> (why?)
	var tt = USER3.index("<ESC>", 1);
	if (tt)
		USER3 = USER3.substr(1, tt - 1);

	//cannot remove since these may be proper codepage letters
	USER3.converter("|", FM);
	USER3.converter(VM, FM);
	if (USER3[1] == FM)
		USER3.splicer(1, 1, "");
	USER3.swapper(FM, var().chr(13) ^ var().chr(10));

	return;

}

subroutine lockit()
{
	state=1;
return;
	//attempt to lock the record
	//BYPASS ordinary lock,file,key process otherwise
	//the lock record will be checked - and in this case
	//we our own lock record to be present
	code = 5;
	//lockit2:
	var nextbfs = "";
	var handle = file;
	//<arev>
	
	//handle = handle.substr(-1, "B" ^ VM);
	handle = handle.field2(VM,-1);
	//</arev>
	var keyorfilename = keyx;
	var fmc = 2;
	gosub lockit2();
	return;
}

subroutine lockit2()
{
	//rtp57(code, nextbfs, handle, keyorfilename, fmc, record, state);
	errputl("MFS file handle not handled yet");
	state=1;
	return;

	state=0;
	if (code==5) state=file.lock(keyx);
	else if (code==6)
	{
		 file.unlock(keyx);
		 state=1;
	}
	return;

}

subroutine unlockit()
{
	return;
	//unlock file,keyx
	code = 6;
	gosub lockit2();
	return;
}

subroutine gettimeouttime()
{
	if (!timeoutsecs) {
		timeouttime = "";
		return;
	}

	var timeoutdate = var().date();
	timeouttime=var().ostime().round(2);
	timeouttime += timeoutsecs;
	if (timeouttime > 24 * 60 * 60) {
		timeoutdate += 1;
		timeouttime -= 24 * 60 * 60;
	}
	timeouttime = (timeoutdate ^ "." ^ (timeouttime.floor()).oconv("R(0)#5")) + 0;
	return;
}

subroutine properlock()
{
	//must lock it properly otherwise indexing will try to lock it and fail
	//because it is only in the locks file and not properly locked
	win.valid = 1;

	//must exclude the shadowing system otherwise the entry in the locks file
	//also in balances
	//will cause the lock statement to fail
	var srcfile2 = win.srcfile;
	//< arev >;
	if (!win.srcfile.unassigned())
		srcfile2.swapper("SHADOW.MFS" ^ SVM, "");
	//</arev>

	//dont pass the filename because that causes persistent lock checking
	//in jbase version of lockrecord()
	if (!(lockrecord("", srcfile2, keyx))) {
		if (STATUS != 1) {
			win.valid = 0;
			USER3 = "Error: " ^ keyx.quote() ^ " CANNOT BE WRITTEN BECAUSE IT IS LOCKED ELSEWHERE";
		}
	}

	return;
}

subroutine properunlock()
{
	//nb i think that shadow.mfs is not programmed to remove the locks file entry

	//must unlock it properly otherwise indexing will try to lock it and fail
	//because it is only in the locks file and not properly locked

	//must exclude the shadowing system otherwise the entry in the locks file
	//will be removed as well
	var srcfile2 = win.srcfile;
	//<arev>
	srcfile2.swapper("SHADOW.MFS" ^ SVM, "");
	//</arev>

	srcfile2.unlock( keyx);

	return;

}

subroutine lock()
{
	/////

	//cannot do update security check here, have to do it after
	// we know if the record exists or not
	//security check - cannot lock so cannot write or delete

	lockauthorised = 1;

	//lockdurationinmins is the number of minutes to retain the lock
	if (lockdurationinmins.unassigned())
		lockdurationinmins = 10;
	if (!lockdurationinmins.isnum())
		lockdurationinmins = 10;
	if (!lockdurationinmins)
		lockdurationinmins = 10;

	//sessionid which if relocking must match the one on the lock rec
	//otherwise it is assumed that somebody else has taken over the lock
	//if sessionid is blank then a new session id is created and returned in response
	if (request1 == "RELOCK") {
		newsessionid = sessionid;
	}else{
		newsessionid = "";
		for (int ii = 1; ii <= 8; ii++)
			newsessionid ^= var("01234567890ABDCEF").substr(var(16).rnd() + 1, 1);
	}

	if (!file.open(filename)) {
		gosub badfile();
		return;
	}

	USER3 = "";
	if (request1 == "RELOCK") {
		gosub lockit();
	}else{
		if (lockrecord(filename, file, keyx)) {
			state = 1;
		}else{
			state = 0;
		}
	}
	if (!state) {
		USER3 = "NOT OK";
		return;
	}

	//check locks file
	gosub getostime();
	lockkey = filename ^ "*" ^ keyx;
	if (lockrec.read(locks, lockkey)) {

		if (lockrec.a(5) != newsessionid) {

			//fail if other lock has not timed out
			if (lockrec.a(1) > ostimenow) {
				USER3 = "NOT OK";
				goto lockexit;
			}

			//other lock has timed out so ok
			//no need to delete as will be overwritten below
			//delete locks,lockkey

		}

		//our own session ... so must be relocking (to extend timeout)

	}else{

		if (request1 == "RELOCK") {
			//nb the word "expired" is a key word used in _formtriggers.htm
			USER3 = "Error: Your lock expired and somebody else updated";
			goto lockexit;
		}

		//no lock record so ok

	}

	USER3 = "OK";

	//convert minutes to fraction of one day (windows time format)
	lockduration = lockdurationinmins / (24 * 60);

	//write the lock in the locks file
	lockrec = "";
	lockrec.r(1, lockduration + ostimenow);
	lockrec.r(2, ostimenow);
	lockrec.r(3, connection ? connection.a(1, 2): STATION);
	lockrec.r(4, USERNAME);
	lockrec.r(5, newsessionid);
	FILEERRORMODE = 1;
	FILEERROR = "";
	USER3 = "OK";
	lockrec.write(locks, lockkey);
	if (FILEERROR) {
		mssg("CANNOT WRITE LOCKS RECORD " ^ lockkey);
		gosub geterrorresponse();
	}
	relistlocks = 1;

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

subroutine badfile()
{
	////////
	USER3 = "Error: " ^ filename.quote() ^ " file does not exist";
	return;
}

subroutine unlock()
{
	///////

	//sessionid is used as a check that only the locker can unlock
	if (!file.open(filename)) {
		gosub badfile();
		return;
	}

	//lock file,keyx
	gosub lockit();
	if (!state) {
		//zzz perhaps should try again a few times in case somebody else
		//is trying to lock but failing because of our remote lock
		USER3 = "Error: Somebody else has locked the record";
		return;
	}

	//get the current lock else return ok
	lockkey = filename ^ "*" ^ keyx;
	FILEERRORMODE = 1;
	FILEERROR = "";
	if (!lockrec.read(locks, lockkey))
		lockrec = "";
	if (!lockrec) {
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
	if (!(sessionid == lockrec.a(5))) {

		//cannot unlock because the lock belongs to somebody else
		USER3 = "Error: Cannot unlock - ";
		if (sessionid == "") {
			USER3 ^= "missing session id";
		}else{
			USER3 ^= "wrong session id";
		}

		goto unlockexit;
	}

	//delete the lock
	FILEERRORMODE = 1;
	FILEERROR = "";
	USER3 = "OK";
	locks.deleterecord(lockkey);
	if (FILEERROR) {
		mssg("CANNOT DELETE LOCK KEY " ^ lockkey);
		gosub geterrorresponse();
	}
	relistlocks = 1;

unlockexit:

	gosub unlockit();

	return;

}
subroutine getsecurity()
{
	if (!SECURITY.read(DEFINITIONS, "SECURITY"))
	{
		errputl("CANNOT READ SECURITY");
		SECURITY="";
	}
	//SECURITY.inverter();
	return;

}

subroutine getostime()
{
	ostimenow=var().ostime().round(2);
	//convert to windows based date/time (ndays since 1/1/1900)
	//31/12/67 in rev date() format equals 24837 in windows date format
	ostimenow = 24837 + var().date() + ostimenow / 24 / 3600;
	return;

}

subroutine flagserveractive()
{

	//flag that this dataset is being served ("listened") (needed for old mac)
	//does not seem to cause any filesharing errors (stress tested at max speed)
	var("").oswrite(inpath ^ serverflagfilename);

	return;

}

subroutine getbakpars()
{
	//backup params
	//1=date last attempted
	// *2= not used ... was workstation for backup (blank defaults to server)
	//3=min time of day for backup to start (defaults to 03:00)
	//4=max time of day for backup to start (defaults to 4:00)
	//5=days to do backup 12345=sun/mon/tue/wed/thu night!!!
	//2/3/4/5 could be multivalued in parallel (to be programmed)
	//6=tech support email addresse(s) separated by ';'
	//7=backup disk letter
	//8=mv list of datasets not to backup
	//9=1 disable backup
	//10=email addresses to send backup.zip to or replacement for 6 if 6 missing
	//backups@neosys.com is excluded in net from backup.zip recipients

	//determine if backup required
	var bakpars;
	if (!bakpars.read(DEFINITIONS, "BACKUP"))
		bakpars = "";
	var tt;
	if (tt.osread("BACKUP.CFG")) {
		for (int ii = 1; ii <= 99; ii++) {
			if (tt.a(ii))
				bakpars.r(ii, tt.a(ii));
		};//ii;
	}
	var lastbakattemptdate = bakpars.a(1);
	//if bakpars<2> then
	// bakreq=(trim(@station)=bakpars<2>)
	//end else
	// bakreq=onserver
	// end
	var testdata = 0;
	if (datasetcode.substr(-4, 4) == "TEST")
		testdata = 1;
	if (SYSTEM.a(23).ucase().index("TRAINING", 1))
		testdata = 1;
	if (SYSTEM.a(23).ucase().index("TESTING", 1))
		testdata = 1;
	if (bakpars.locate(datasetcode, xx, 8))
		testdata = 1;
	//testdata will not backup but will terminate if bakreq
	//bakreq=not(testdata)
	var bakreq = 1;
	if (bakpars.a(9))
		bakreq = 0;
	//address to email backup is bakpars<10>

	var minbaktime = bakpars.a(3);
	if (minbaktime == "")
		minbaktime = var("2:00").iconv("MT");
	//if @username='neosys' then minbaktime=iconv('00:00','mt')
	var maxbaktime = bakpars.a(4);
	//if maxbaktime='' then maxbaktime=iconv('2:05','mt')
	if (maxbaktime == "")
		maxbaktime = minbaktime + 60 * 10;
	//if @username='neosys' then maxbaktime=iconv('23:59','mt')

	var bakdows = bakpars.a(5);
	if (bakdows == "")
		bakdows = "1234567";

	var bakdisk = bakpars.a(7);

	return;
}

subroutine convlogx()
{

	//assumes at least 0-31 and 249-255 encoded like $hh

	//reserve/use special characters for field separators
	//not really necessary in unicode?
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
	//swap sm with '\' in logx
	//swap tm with "[" in logx
	//convert fefdfcfb to '^]\[' in logx
	logx.converter(fefdfcfb, "^]\\[");
	logx.swapper(STM, "%FA");
	logx.swapper(SSTM, "%F9");

	logx.swapper("%20", " ");
	//does not seem to format in xml
	//swap '%20' with '&nbsp;' in logx

	logx.swapper("&", "&amp;");
	logx.swapper(DQ, "&quot;");
	logx.swapper("<", "&lt;");
	logx.swapper(">", "&gt;");
	logx.swapper("%26", "&amp;");
	logx.swapper("%22", "&quot;");
	logx.swapper("%3C", "&lt;");
	logx.swapper("%3E", "&gt;");

	return;
}

/*
subroutine writelogx()
{
	gosub convlogx();
	gosub writelogx2();
	return;
}

subroutine writelogx2()
{
	osbwritex(logx, logfilename, logfilename, logptr);
	logptr += logx.length();
	logx = "";
	return;
}

subroutine writelogx3()
{
	osbwritex(logx, logfilename, logfilename, logptr);
	logx = "";
	return;
}

*/

function filesecurity(in secmode)
{
	if (authorised(filename2 ^ " " ^ secmode, msg0, "")) {
		positive = "";
	}else{
		positive = "#";
	}
	var msgx="";
	if (!(authorised(positive ^ filename2 ^ " " ^ secmode ^ " " ^ keyx.quote(), msgx, ""))) {
		if (positive) {
			msg0.transfer(USER3);
		}else{
			msgx.transfer(USER3);
		}
		gosub formatresponse();
		return 0;
	}
	return 1;

}

subroutine respond()
{
	//method to allow batch programs to respond and detach before finishing

	//determine the responsefile name from the printfilename
	//responsefilename=system<2>
	//if responsefilename else return
	//t=field2(responsefilename,'.',-1)
	//responsefilename[-len(t),len(t)]='3'

	//linkfilename3
	responsefilename = PRIORITYINT.a(100);
	if (!responsefilename)
		return;

	//detach the calling process
	USER3 = request2;
	USER3.converter(VM ^ VM, FM ^ FM);
	USER3.swapper(FM, "\r\n");

	USER3.oswrite(responsefilename);
	//osclose responsefilename

	var().ossleep(2000);

	//indicate that response has been made
	SYSTEM.r(2, "");

	return;

}

subroutine getindexvalues()
{

	return;

/* NOT IMPLEMENTED YET - REQUIRES DB SIDE CODING

 	filename=request1;
 	fieldname=request2;
 	prefix=request3;
 	sortby=request4;

	_USER1 = "";
	if (sortby) {
		if (!(var("AL AR DL DR").locateusing(sortby, " "))) {
			USER3 = "Invalid sortby " ^ sortby.quote() ^ " in Server,GETINDEXVALUES";
			return;
		}
	}

	//security check
	var temp = filename;

	//zzz
	if (temp == "BATCHES")
		temp = "JOURNAL";

	temp.convert(".", " ");
	temp = singular(temp);
	if (!(authorised(temp ^ " ACCESS", USER4, ""))) {
		USER3 = USER4;
		return;
	}
	if (!(authorised(temp ^ " LIST", USER4, ""))) {
		USER3 = USER4;
		return;
	}

	collectixvals(filename, fieldname, prefix);
	PSEUDO.transfer(USER1);
	if (USER1[1] == FM)
		USER1.splicer(1, 1, "");
	USER3 = "OK";

	if (sortby && USER1) {
		USER1.convert(FM, RM);
		USER1 ^= RM;
		v119("S", "", sortby[1], sortby[2], USER1, flag);
		USER1.convert(RM, FM);
		USER1.splicer(-1, 1, "");
	}

	if (USER0.index("XML", 1)) {
		if (USER1) {
			USER1.swap(FM, "</" ^ fieldname ^ ">" "</record>" ^ (var().chr(13) ^ var().chr(10)) ^ "<record><" ^ fieldname ^ ">");
			USER1.splicer(1, 0, "<record><" ^ fieldname ^ ">");
			USER1 ^= "</" ^ fieldname ^ ">" "</record>";
		}
		USER1.splicer(1, 0, "<records>");
		USER1 ^= "</records>";
	}else{
		//convert fm to vm in iodat
	}

	return;

*/

}


subroutine requestselect()
{

	//USER1=''

//TODO
//	var().clearselect();

	var filename0 = request2;
	var filename = filename0.field(" ", 1, 1);
	var sortselect = request3;
	var dictids = request4;
	var options = request5;

	var file;
	if (!file.open(filename)) {
		USER3 = "Server::select " ^ filename.quote() ^ " cannot open file";
		return;
	}

	//security check
	if (not var("MENUS ADMENUS").locateusing(filename, " ")) {
		var temp = filename;
		temp.convert(".", " ");
		temp = singular(temp);
		if (!(authorised(temp ^ " ACCESS", USER4, ""))) {
			var msgx="";
			if (!(authorised("!#" ^ temp ^ " ACCESS PARTIAL", msgx, ""))) {
				USER3 = USER4;
				return;
			}
		}
		USER4 = "";
	}

	//any data passed to select is assumed to be a selectlist

	if (USER1) {
//		makelist("", USER1, "", "");
//		sortselect ^= "%SELECTLIST%";
//		USER1 = "";
		if (not USER1.index(DQ)) {
			USER1.quoter().swapper(" ", DQ^" "^DQ);
		}
		if (sortselect.ucase().index("WITH")) {
			sortselect^=" AND ";
		}
		sortselect^="WITH @ID EQ ";
		sortselect^=USER1;
	}
//USER1.outputl("USER1=");
	call select2(filename0, SYSTEM.a(2), sortselect, dictids, options, USER1, USER3, "", "", "");

	if (USER4) {
		USER3 = USER4.a(1).trim(FM);
	}else{
		USER1 = "%DIRECTOUTPUT%";
		//USER3='ok'
	}

	return;

};

subroutine deleteoldfiles(in inpath, in pattern)
{

	var deletetime = var().date() * 24 * 60 * 60 + var().time() - ageinsecs;

	//for each suitable file
	var filenames=var().oslistf(inpath,pattern);

	while (filenames) {

		//get the file time
		var filename = filenames.field(FM, 1, 1);
		filenames.splicer(1,filename.length()+1,"");
		filename = inpath ^ filename;

		if (!(var(".JPG .GIF").locateusing(filename.substr(-4, 4).ucase()," "))) {

			//a file ending .4 means that the .2 and .3 files need to be deleted
			if (filename.substr(-2, 2) == ".4") {
				filename.osdelete();
				filename.splicer(-1, 1, "2");
				filename.osdelete();
				filename.splicer(-1, 1, "3");
				filename.osdelete();

			}else{
				//and delete it if older than the cut off time
				//and has a file extension (ie leave params and params2)
				var fileattributes = filename.osfile();
				var filetime = fileattributes.a(2) * 24 * 60 * 60 + fileattributes.a(3);
				if ((filename.substr(-4, 4)).index(".", 1) && filetime <= deletetime) {
					filename.osdelete();
				}else{
				}
			}

		}

	}//loop;

	return;

}

//in server and select2 for directoutput
function encode(in instr) {
	//do xml character encoding (do % FIRST!)
	return instr
	.swap(L"%",L"%25")
	.swap(L"<",L"%3C")
	.swap(L">",L"%3E")
	.swap(L"&",L"%26");
}

function decode(in instr) {
	//undo xml character encoding
	return instr
	.swap(L"%3C",L"<")
	.swap(L"%3E",L">")
	.swap(L"%26",L"&")
	.swap(L"%25",L"%");
}

//use debug if using debugger to get stack backtrace
debugprogramexit()
//programexit()

