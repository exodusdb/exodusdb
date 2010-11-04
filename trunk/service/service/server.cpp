/* Copyright (c) 2007 Stephen John Bush - see LICENCE.TXT*/
#include <iostream>
#include "server.h"

//#include "mvroutine.h"

//TODO replace wcout<< with printl to remove dependency on <iostream>

namespace exodus {

var Server::run()
{
	nblocks = 4;

	renaming = 1;
	portno = 5700;
	if (env.SYSTEM.extract(38))
		portno += env.SYSTEM.extract(38) - 1;

	onalertsecs = renaming ? 0 : 1;

	//delete files older than ...
	ageinsecs = 60 * 60;

	halt = 0;

//TODO	var(L"ADDMFS SHADOW.MFS FILEORDER.COMMON").perform();

	maxstringsize = 65530;
	inblocksize = 65500;
	//inblocksize=50000
	outblocksize = (maxstringsize / 3).floor();

	Serverversion = L"";
	origsentence = env.SENTENCE;

	//open 'messages' to messages else return 999999
//	env.SYSTEM.printl();

	datasetcode = env.SYSTEM.extract(17);
	if (datasetcode == L"")
		datasetcode = L"DEFAULT";

#ifdef _WIN32
    neopath = L"d:\\neosys\\";
#else
	neopath = L"/cygdrive/d/neosys/neosys/";
#endif

    neopath.converter(L"/\\",_SLASH^_SLASH);
	globalend = neopath ^ L"GLOBAL.END";
	if (!allcols.open(L"ACCESSIBLE_COLUMNS"))
		allcols = L"";

	//prepare for sending and receiving escaped iodat > 64kbh
	for (int ii = 0; ii <= 255; ii++)
    {
		hexx[ii]= (L"%" ^ (var(ii).oconv(L"MX")).oconv(L"R(0)#2"));
        //hexx[ii]="%00";
//        std::wcout<<hexx[ii]<<L" ";
    }
    std::wcout<<L"Hex Done"<<std::endl;
	//discover the server name
	//servername=field(getdrivepath(drive()[1,2])[3,9999],'\',1)
	servername = L"";
	onserver = servername == L"" || env.STATION.trim() == servername;

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
	serverflagfilename = L"GLOBAL.SVR";

	intranet = origsentence.index(L"INTRANET", 1);

	gosub_getbakpars();

	//if @username='neosys.net' then system<33>=1
	origbatchmode = env.SYSTEM.extract(33);

	//webpath=field(origsentence,' ',3)
	webpath = neopath ^ L"data/";
	if (!webpath)
		webpath = L"../data/";
	webpath.converter(L"/\\",_SLASH^_SLASH);
	if (webpath.substr(-1, 1) != _SLASH)
		webpath ^= _SLASH;
	if (!voc.open(L"VOC")) {
		env.mssg(L"CANNOT OPEN VOC FILE");
		return 999999;
	}

	inpath = webpath ^ datasetcode ^ _SLASH;

	//set time before calling deleteoldfiles which is time sensitive
	//call settime(webpath[1,2])

	gosub_deleteoldfiles(inpath,L"^.*$");

	Serverusername = env.USERNAME;
	Serverstation = env.STATION;

	//savescreen(origscrn, origattr);

//	std::wcout << env.AW.extract(30);

	//tracing=(@username='neosys' or trim(@station)='sbcp1800')
	//tracing=(not(index(origsentence,'auto',1)))

	//if @username='demo' then tracing=0
	//tracing=0

	tracing = 1;

	//ensure unique sorttempfile
	//if (!(sysvar(L"SET", 192, 102, L"R" ^ var(L"0000" ^ env.SYSTEM.extract(24)).substr(-5, 5) ^ L".SFX")))
	//	{}

	nrequests = env.SYSTEM.extract(35) + 0;
	//if tracing then
	std::wcout << L"NEOSYS.NET SERVICE "<< env.SYSTEM.extract(24);
	std::wcout << L" STARTED "<< var().timedate() << std::endl;
	std::wcout << std::endl;

	std::wcout << L"Station  : "<< env.STATION<<std::endl;
    std::wcout << L"Drive : "<< var().oscwd() << std::endl;

	std::wcout << L"Server   : "<< servername<<std::endl;
    std::wcout << L"Data  : "<< inpath << std::endl;

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
	// * call note2('neosys.net service' 'c#30':'||||||press esc and wait 10 secs to interrupt','ub')
	//
	// *switch back to silent mode
	// system<33>=s33
	//
	// print @aw<30>:
	//
	// end

	origprivilege = env.PRIVILEGE;
	//env.setprivilege(L"");

	request1 = L"";

	//image=''


	if (!(env.openfile2(L"LOCKS", locks, L"LISTS", 1))) {
		env.mssg(L"CANNOT OPEN LOCKS FILE");
		return L"";
	}

	relistlocks = 1;

	timeoutsecs = env.SYSTEM.extract(33, 5);
	if (!timeoutsecs.isnum())
		timeoutsecs = L"";
	//if timeoutsecs='' then timeoutsecs=20*60;*ie 20 minutes

	gosub_gettimeouttime();
	timestarted = var().time();

	gosub_flagserveractive();

	onalert = onalertsecs;

	//open a log file
	logfilename = L"";
	logpath = neopath ^ L"/LOGS/";
	logpath.converter(L"/\\",_SLASH^_SLASH);
	if (var().oslistf(logpath ^ datasetcode)==L"")
	{
		if (var().oslist(logpath)==L"")
			logpath.osmkdir();
		(logpath ^ datasetcode).osmkdir();
	}
	var datex = (var().date()).oconv(L"D2.");
	logfilename = logpath ^ datasetcode ^ _SLASH ^ datex.substr(-2, 2) ^ datex.substr(1, 2) ^ datex.substr(4, 2) ^ var(L"00" ^ env.SYSTEM.extract(24)).substr(-2, 2);
	logfilename ^= L".xml";

	if (logfile.osopen(logfilename)) {
		logptr = logfilename.osfile().extract(1);

		//osbread x from logfile at logptr-6 length 6
		xx.osbread(logfile, logptr - 6, 6);
		if (xx == L"</Log>")
			logptr -= 6;

	}else{
		var(L"").oswrite(logfilename);
		if (logfile.osopen(logfilename)) {
			logptr = 0;
			logx = L"<?xml version=\"1.0\"?>" ^ (var().chr(13) ^ var().chr(10));
			logx ^= L"<?xml-stylesheet type=\'text/xsl\' href=\'.\\loglist.xsl\'?>" ^ (var().chr(13) ^ var().chr(10));
			//logx:="<?xml-stylesheet type='text/xsl'?>":crlf
			logx ^= L"<Log";
			logx ^= L" Process=" ^ env.SYSTEM.extract(24).quote();
			logx ^= L" Dataset=" ^ datasetcode.quote();
			logx ^= L">";
			gosub_writelogx2();

			env.osbwritex(L"</Log>", logfile, logfilename, logptr);

		}else{
			std::wcout << L"CANNOT OPEN LOG FILE "<< logfilename << std::endl;
			logfilename = L"";
		}
	}

	linkfilename1 = L"";
	replyfilename = L"";

	env.TCLSTACK = env.TCLSTACK.field(FM, 1, 10);
	//<arev>
	var(L"RESET").execute();
	//</arev>

	if (tracing)
		std::wcout << var(L"-").str(79) << std::endl;
	env.RECORD = L"";
	env.ID = L"";
	env.USER0 = L"";
	env.USER1 = L"";
	env.USER3 = L"";
	env.USER4 = L"";
	//garbagecollect;

	//linkfilename1=''

	//force into interactive mode for deb ugging
	//if tracing then
	if (1) {
		env.SYSTEM.replacer(33, 0, 0, 1);
	}else{
		env.SYSTEM.replacer(33, 0, 0, L"");
	}
	//end else
	// end

	//force into server mode - suppress interactive messages
	batchmode = origbatchmode;
	if (batchmode == L"") {
		batchmode = env.SYSTEM.extract(33);
		env.SYSTEM.replacer(33, 0, 0, L"1");
	}
	// if trim(@station)='sb_compaq' and @username='neosys' then system<33>=''

	while (requestloop()){};

	return L"";

}

var Server::requestloop()
{

	//on win95 this runs at same speed in full screen or windowed

	locks.unlock( L"REQUEST*" ^ linkfilename1);
	locks.unlock( L"REQUEST*" ^ replyfilename);
//print 'unlock locks,request*':linkfilename1
//print 'unlock locks,request*':replyfilename

	timex = var().time();
	breaktime = L"X";

	//call getstationid(stationid)
	//if env.lockrecord('messages',messages,stationid,'','') else null
	//indicate that we are active
//		messaging(L"CHECK");

	while (serviceloop()){}

	return false;

}

var Server::serviceloop()
{

	//delete old response files every 10 requests or service loop
	if (nrequests.substr(-1, 1) == L"0")
		gosub_deleteoldfiles(inpath,L".*\\.4$");

		//if tracing then
	std::wcout << env.at(0);
	std::wcout << (var().time()).oconv(L"MTS");
	std::wcout << L" " << env.SYSTEM.extract(17);
	std::wcout << L" " << env.ROLLOUTFILE.field2(L"\\", -1).field(L".", 1, 1);
	std::wcout << L" " << nrequests;
	std::wcout << L" " << onalert.oconv(L"MD10P");
	std::wcout << L" Listening ..."<< env.at(-4)<<std::endl;

	//end else
	// print @(25,@crthigh/2-1):
	// print (date() 'd':' ':timex 'mts') 'c#30':
	// print @(25,@crthigh/2+1):
	// print ('requests: ':nrequests:' on alert: ':onalert 'md10p') 'c#30':
	// *print @(25,@crthigh/2+2):('on alert: ':onalert 'md10p') 'c#30':
	// end

	//prevent sleep in esc.to.exit
	timenow=var().ostime();
	env.SYSTEM.replacer(25, 0, 0, timenow);
	env.SYSTEM.replacer(26, 0, 0, timenow);

	if (env.esctoexit())
		return false;

	//look for db stoppers in program directory
	for (int filen = 1; filen <= 2; filen++) {
		var filename = (globalend ^ VM ^ neopath ^ datasetcode ^ L".END").extract(1, filen);
		if (filename.osfile()) {

			if (env.USER1.osread(filename)) {

				env.USER1.converter(var().chr(13) ^ var().chr(10), FM);

				if (env.USER1.extract(1) == L"") {
stopper:
						request1 = L"STOPDB";
					return false;
				}

				var datetime = (var().date() ^ L"." ^ (var().time()).oconv(L"R(0)#5")) + 0;

				var stopdatetime = (env.USER1.substr(1, 10)).iconv(L"D") ^ L"." ^ (env.USER1.substr(11, 10)).iconv(L"MT");
				if (stopdatetime.isnum()) {
					if (datetime <= stopdatetime + 0)
						goto stopper;
				}

				filename.osdelete();

			}

		}

	};//filen;

	gosub_flagserveractive();

	if (renaming) {
		linkfilename1 = inpath ^ env.ROLLOUTFILE.field2(L"\\", -1);
		var ext=linkfilename1.field2(L".",-1);
		linkfilename1.splicer(-ext.length(),99999,L"0");
		while (true)
		{

			linkfilename1.osdelete();

			if (linkfilename1.osfile()==L"")
				break;

			//if (tracing)
			std::wcout << L"CANNOT DELETE "<< linkfilename1<< L" GENERATING ANOTHER" << std::endl;
			linkfilename1 = inpath ^ var(99999999).rnd() ^ L".0";

		}
		linkfilename0 = linkfilename1.substr(inpath.length() + 1, 9999);
	}

	//inpath
	//neopath
	//waitsecs
	//sleepms
	//datasetcode

	var linkfilename;
    int nn=1000/sleepms*waitsecs;
	for (int ii=0;ii<=nn;ii++)
	{
        linkfilename=linkfilename.oslistf(inpath,L".*\\.1$");
		if (linkfilename)
		{
            linkfilename=linkfilename.extract(1);
			var(inpath^linkfilename).osrename(linkfilename1);
			break;
		}

//		if (var(neopath^"GLOBAL.END").osfile()!=L"") break;
//		if (var(neopath^datasetcode^".END").osfile()!=L"") break;

		linkfilename.ossleep(sleepms);

	}

/*
			//remove lock indicating processing (hangs if unlock and not locked)
			//gosub sysunlock
			//print cmd
//				BREAK OFF;
			//garbagecollect;
			std::wcout<<cmd<<std::endl;
//			cmd.osshell();
//				BREAK ON;

			//place a lock to indicate processing
			//should really retry in case blocked by other processes checking it
			//call rtp57(syslock, '', '', trim(@station):system<24>, '', '', '')
*/

	//if got file then quit

    if (renaming) {
		if (linkfilename1.osfile()!=L"") {

			//flush
			//osopen linkfilename1 to linkfile1 else
			// call timedelay(.1)
			// if tracing then print 'cannot open ':quote(linkfilename1)
			// goto nextrequest
			// end
			//goto requestinit
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
		timenow=var().ostime();
		var now = (var().date() ^ L"." ^ (timenow.floor()).oconv(L"R(0)#5"));
		now+=0;
		//if timeouttime and now>timeouttime then return 999999

		gosub_flagserveractive();

		//check for upgrade to Server

		//switch into interactive mode to check for operator input
		var s33 = env.SYSTEM.extract(33);
		env.SYSTEM.replacer(33, 0, 0, L"");

		//check for esc key to exit
		//if esc.to.exit() then return 999999

		var charx;
//		charx=keypressed();
		charx=L"";

		if (charx.length())
		{

			//charx.input(-1, 2);
			charx = charx.substr(1, 1).ucase();

			//esc
			if (charx == env.INTCONST.extract(1)) {
				env.msg2(L"You have pressed the [Esc]  key to exit|press again to confirm|", L"UB", buffer, L"");
				while (true) {
					reply.input(-1);
				//BREAK;
				if (reply) break;;
				}//loop;
//				env.msg2(L"", L"DB", buffer, L"");
				if (reply == env.INTCONST.extract(1))
					return false;
				if (reply == env.INTCONST.extract(7))
					charx = reply;
			}

			//r=restart
			if (charx == L"R") {
				request1 = L"RESTART";
				return false;
			}

			//f5
			if (charx == env.PRIORITYINT.extract(2))
				var(L"").execute();

			//f10
			if (charx == env.INTCONST.extract(7))
				var(L"RUNMENU " ^ env.ENVIRONSET.extract(37)).execute();

			if (charx != L"") {

				//"d" = de bug
				//if charx='d' and @username='neosys' then
				// de bug
				// end

				gosub_gettimeouttime();
			}

			//switch back to not interactive mode
			env.SYSTEM.replacer(33, 0, 0, s33);

			gosub_getbakpars();

			//perform backup if right time range and not backed up today
			if (bakreq && var().date() != lastbakattemptdate) {

				var dow = var().date() - 1 % 7 + 1;

				//override min/max times
				//if @username='neosys' and trim(@station)='sbcp1800' then

				if (charx == L"B" || bakdows.index(dow, 1) && timex > minbaktime && timex < maxbaktime) {

					if (testdata)
						var(L"OFF").perform();

					lastbakattemptdate = var().date();
					lastbakattemptdate.writev(env.DEFINITIONS, L"BACKUP", 1);

					env.USER4 = L"";
					var(L"FILEMAN BACKUP " ^ datasetcode ^ L" " ^ bakdisk ^ L" SYSTEM").perform();

					//quit and indicate to calling program that a backup has been done
					//if tracing else
					env.PSEUDO = L"BACKUP";
					if (env.USER4)
						return false;
					// end

				}

			}
		}
	}

	gosub_deleteoldfiles(inpath, L"^.*$");

	return true;

}

var Server::processlink()
{
	nrequests += 1;
	env.SYSTEM.replacer(35,0,0,nrequests);

	//get the earliest time possible for the log
	requeststarttime=var().ostime();
	env.SYSTEM.replacer(25, 0, 0, requeststarttime);

	gosub_gettimeouttime();

	var nlinkfiles = linkfilenames.count(FM) + 1;

	//find a request to process
	for (int linkfilen = 1; linkfilen <= nlinkfiles; linkfilen++) {

		linkfilename1 = inpath ^ linkfilenames.extract(linkfilen);

		//lock it to prevent other Servers from processing it
		//unlock locks,'request*':linkfilename1
		if (!(env.lockrecord(L"", locks, L"REQUEST*" ^ linkfilename1))) {
			if (tracing)
			{
				std::wcout<< L"CANNOT LOCK LOCKS,";
				std::wcout<<(L"REQUEST*" ^ linkfilename1).quote();
				std::wcout<< std::endl;
			}
			continue;//goto nextlinkfile;
		}
//print 'lock locks,request*':linkfilename1

		var().osflush();
		if (!(linkfile1.osopen(linkfilename1))) {
			//remove from future candidate files?
			//qqq
			var().ossleep(100);
			if (tracing)
				std::wcout << L"CANNOT OPEN " << linkfilename1.quote() << std::endl;
			continue;//goto nextlinkfile;
		}

		//get the .1 file which contains the request
		//timex=time()+2
readlink1:
			//osbread request from linkfile1 at 0 length 256*256-4
		env.USER0.osbread(linkfile1, 0, 256 * 256 - 4);

		//if cannot read it then try again
		if (env.USER0 == L"" && var().time() == timex) {
			var().osflush();
			//qqq
			var().ossleep(100);
			linkfile1.osclose();
			if (!(linkfile1.osopen(linkfilename1)))
				{}
			goto readlink1;
		}

		//hack to remove UTF16 BOM mark
		if (env.USER0.substr(1,1)==L"\uFEFF") env.USER0.splicer(1,1,L"");

//TODO check ALL CHARACTERS are converted F8-FF and on all input/output
		//cleanup the input file
		//convert '&' to fm in request
		env.USER0.converter(var().chr(13) ^ var().chr(10), FM);
		env.USER0.swapper(L"\\\\", L"\\");
		env.USER0.swapper(L"\\r", FM);
        env.USER0.trimmerb(FM);
		env.USER0.swapper(L"%FF", RM);
		env.USER0.swapper(L"%FE", FM);
		env.USER0.swapper(L"%FD", VM);
		env.USER0.swapper(L"%FC", SVM);
		env.USER0.swapper(L"%FB", TM);
		env.USER0.swapper(L"%FA", STM);
		env.USER0.swapper(L"%F9", SSTM);

		replyfilename = env.USER0.extract(1).ucase();
		env.USER0.eraser(1, 0, 0);

		//lock the replyfilename to prevent other Servers from processing it
		//unlock locks,'request*':replyfilename
		if (!(env.lockrecord(L"", locks, L"REQUEST*" ^ replyfilename))) {
			if (tracing)
				std::wcout << L"CANNOT LOCK LOCKS," << (L"REQUEST*" ^ replyfilename).quote() << std::endl;
			continue;//goto nextlinkfile;
		}

//print 'lock locks,request*':replyfilename

		//delete then unlock the request file
			var ntries = 0;
deleterequest:
			linkfile1.osclose();
			linkfilename1.osdelete();
			if (linkfilename1.osfile()) {
				var().osflush();
				//garbagecollect;
			var().ossleep(100);
			ntries += 1;
			//if tracing then print 'could not delete ':linkfile1
			if (ntries < 100)
				goto deleterequest;
			if (tracing)
				std::wcout << L"COULD NOT DELETE "<< linkfile1 << std::endl;
		}

		//leave these in place for the duration of the process
		//they should be cleared by unlock all somewhere at the end or beginning
		//unlock locks,'request*':replyfilename
		//unlock locks,'request*':linkfilename1

		//found a good one so process it
		return processrequest();

	};//linkfilen;

	return true;

}

var Server::processrequest()
{

	nrequests += 1;

	//if tracing then
	//print @(0):@(-4):time() 'mts':' ':count(program.stack(),fm):
	//print @(0):@(-4):time() 'mts':' ':field2(linkfilename1,'\',-1):' ':field2(replyfilename,'\',-1):
	//print @(0):@(-4):time() 'mts':' ':field2(replyfilename,'\',-1):
	std::wcout << env.at(0)<< env.at(-4)<< (var().time()).oconv(L"MTS")<< L" ";
	//end else
	// print @(25,@crthigh/2+1):
	// print ('processing request : ':nrequests) 'c#30':
	// end

	//clear out buffers just to be sure
	//request=''
	env.USER1 = L"";
	//env.USER3=''
	env.USER3 = L"Error: Response not set in Server.";
	env.USER4 = L"";

	//only go on alert if we actually get the request
	//to avoid multiple processes going on alert for only one request
	onalert = onalertsecs;

	Serverfailure = 0;

	//analyse the input file
	connection = L"";
	if (env.USER0.extract(1) == L"VERSION 2") {
		//remote_addr remote_host https
		connection = env.USER0.field(FM, 1, 4);
		env.USER0 = env.USER0.field(FM, 5, 99999);
	}
	connection.converter(FM, VM);
	env.SYSTEM.replacer(40, 0, 0, connection);

	dataset = env.USER0.extract(1).ucase();
	username = env.USER0.extract(2).ucase();
	password = env.USER0.extract(3).ucase();
	firstrequestfieldn = 4;
	request1 = env.USER0.extract(firstrequestfieldn);

	if (request1 == L"CACHE") {
		env.USER0.eraser(firstrequestfieldn, 0, 0);
		request1 = env.USER0.extract(firstrequestfieldn);
	}
	request1 = request1.ucase();
	//convert @lower.case to @upper.case in request
	request2 = env.USER0.extract(5);
	request3 = env.USER0.extract(6);
	request4 = env.USER0.extract(7);
	request5 = env.USER0.extract(8);

    //remove dataset and password
    env.USER0 = env.USER0.field(FM, firstrequestfieldn, 99999);
	if (logfilename!=L"") {

		var datex = var().date();
		timex = requeststarttime;

		var tt = var().chr(13) ^ var().chr(10) ^ L"<Message ";
		tt ^= L" Date=" ^ env.xmlquote(datex.oconv(L"D"));
		tt ^= L" Time=" ^ env.xmlquote(timex.oconv(L"MTS"));
		tt ^= L" DateTime=" ^ env.xmlquote(datex.oconv(L"DJ-") ^ L"T" ^ timex.oconv(L"MTS") ^ L"." ^ timex.field(L".", 2, 1));
		tt ^= L" User=" ^ env.xmlquote(username);
		tt ^= L" File=" ^ env.xmlquote(replyfilename.field2(L"\\", -1));
		//remote_addr remote_host https
		tt ^= L" IP_NO=" ^ env.xmlquote(connection.extract(1, 2));
		tt ^= L" Host=" ^ env.xmlquote(connection.extract(1, 3));
		tt ^= L" HTTPS=" ^ env.xmlquote(connection.extract(1, 4));
		tt ^= L">" ^ (var().chr(13) ^ var().chr(10));

		tt ^= L"<Request ";

		logx = env.USER0;
		gosub_convlogx();
		logx.converter(L"^", FM);
		logx.replacer(1, 0, 0, request1);
		if (logx.extract(1)!=L"")
			tt ^= L" Req1=" ^ logx.extract(1).quote();
		if (logx.extract(5)!=L"")
			tt ^= L" Req2=" ^ logx.extract(5).quote();
		if (logx.extract(6)!=L"")
			tt ^= L" Req3=" ^ logx.extract(6).quote();
		if (logx.extract(7)!=L"")
			tt ^= L" Req4=" ^ logx.extract(7).quote();
		if (logx.extract(8)!=L"")
			tt ^= L" Req5=" ^ logx.extract(8).quote();
		var req6up = logx.field(FM, 9, 9999);
		req6up.converter(FM, L"^");
		if (req6up.length())
			tt ^= L" Req6up=" ^ req6up.quote();

		tt ^= L"/>" ^ (var().chr(13) ^ var().chr(10));
		tt.transfer(logx);
		gosub_writelogx2();

		env.osbwritex(L"<DataIn>", logfile, logfilename, logptr);

	}

	var anydata = 0;

	if (request2 == L"JOURNALS") {
		request2 = L"BATCHES";
		if (request3.count(L"*") == 3 && request3.substr(-1, 1) == L"*")
			request3.splicer(-1, 1, L"");
	}

	//print the request (hide dataset and password)
	if (tracing) {
        var tt = username ^ L" " ^ env.USER0;
//		tt.replace(firstrequestfieldn - 1, 0, 0, L"");
//		tt.replace(firstrequestfieldn - 3, 0, 0, L"");
		tt.convert(FM, L" ");

		var t2 = connection.extract(1, 2);
		if (connection.extract(1, 3) != t2)
			t2 ^= L" " ^ connection.extract(1, 3);

//		t2=t2^" "^tt;

		t2.trimmer();
		std::wcout << L" " << (t2 ^ L" " ^ tt).trim().quote() << std::endl;
	}

	//if renaming else replyfilename=linkfilename1
	linkfilename2 = replyfilename.splice(-1, 1, 2);
	
	//should be made into a MvLib
	linkfilename2.swapper(L"/\\",_SLASH^_SLASH);
	//if (linkfilename2.index(L":"))
	//{
	//	linkfilename2.swapper(L":",_SLASH);
	//	linkfilename2="/cygdrive/"^linkfilename2;
	//}

	linkfilename3 = replyfilename.splice(-1, 1, 3);
	
	//should be made into a MvLib
	linkfilename3.swapper(L"/\\",_SLASH^_SLASH);
	//if (linkfilename3.index(L":"))
	//{
	//	linkfilename3.swapper(L":",_SLASH);
	//	linkfilename3="/cygdrive/"^linkfilename3;
	//}

	//save the response file name
	//so that if Server fails then net the calling program can still respond
	env.PRIORITYINT.replacer(100, 0, 0, linkfilename3);

	var linkfile2size = linkfilename2.osfile().extract(1);
	if (linkfile2size > maxstringsize) {

		if (linkfile2.osopen(linkfilename2)) {

			//read blocks of iodat
			for (int blockn = 1; blockn <= nblocks; blockn++)
				datx[blockn]=L"";
//				datx.initarray(L"");
			for (int blockn = 1; blockn <= nblocks; blockn++) {

				//osbread datx(blockn) from linkfilename2 at ((blockn-1)*inblocksize) length inblocksize
				env.osbreadx(datx[blockn], linkfile2, linkfilename2, (blockn - 1) * inblocksize, inblocksize);

				//BREAK;
				if (!((datx[blockn]).length())) break;;

				//hack to remove UTF16 BOM mark
				if (blockn==1&&datx[blockn].substr(1,1)==L"\uFEFF")
					datx[blockn].splicer(1,1,L"");

				//avoid hexcode spanning block end by moving one or two bytes backwards
				if (blockn > 1) {
					var tt = ((datx[int(blockn - 1)]).substr(-2, 2)).index(L"%", 1);
					if (tt) {
						datx[int(blockn - 1)] ^= (datx[blockn]).substr(1, tt);
						(datx[blockn]).splicer(1, tt, L"");
					}
				}

			};//blockn;

			//unescape all blocks
			var lendata = 0;
			for (int blockn = 1; blockn <= nblocks; blockn++) {

				if ((datx[blockn]).length()) {

					//hack to remove UTF16 BOM mark
					if (blockn==1&&datx[blockn].substr(1,1)==L"\uFEFF")
						datx[blockn].splicer(1,1,L"");

					//output to log
					if (logfilename) {

						//start after the last <datain>
						if (!anydata) {
							anydata = 1;
							logptr += 8;
						}

						logx = datx[blockn];
						gosub_writelogx();

					}

					for (int ii = 0; ii <= 36; ii++)
						datx[blockn].swapper(hexx[ii], var().chr(ii));
					for (int ii = 38; ii <= 255; ii++)
						datx[blockn].swapper(hexx[ii], var().chr(ii));
					datx[blockn].swapper(hexx[37], var().chr(37));

					lendata += (datx[blockn]).length();

				}

			};//blockn;

			//check max iodat size <= maxstringsize
			if (lendata > maxstringsize) {
				env.USER1 = L"";
				env.USER3 = L"Error: Maximum record size of 64Kb exceeded in Server";
				Serverfailure = 1;

				//otherwise join the blocks
			}else{
				env.USER1 = L"";
				for (int blockn = 1; blockn <= nblocks; blockn++) {
					env.USER1 ^= datx[blockn];
					datx[blockn] = L"";
				};//blockn;
			}

			//cannot open linkfilename2 means no iodat
			}else{
cannotopenlinkfile2:
				Serverfailure = 1;
				env.USER1 = L"";
			env.USER3 = L"Error: Server cannot open " ^ linkfilename2;
		}

	}else{

		if (!linkfile2size) {

			env.USER1 = L"";

		}else{

			if (!(linkfile2.osopen(linkfilename2)))
				goto cannotopenlinkfile2;

			//osread iodat from linkfilename2 else goto cannotopenlinkfile2
			env.osbreadx(env.USER1, linkfile2, linkfilename2, 0, maxstringsize);

			//hack to remove UTF16 BOM mark
			if (env.USER1.substr(1,1)==L"\uFEFF")
				env.USER1.splicer(1,1,L"");

			//unescape
			//for ii=0 to 255

			//output to log
			if (logfilename) {

				//start after the last <datain>
				if (!anydata) {
					anydata = 1;
					logptr += 8;
				}

				logx = env.USER1;
				gosub_writelogx();
			}

			for (int ii = 0; ii <= 36; ii++)
				env.USER1.swapper(hexx[ii], var().chr(ii));
			for (int ii = 38; ii <= 255; ii++)
				env.USER1.swapper(hexx[ii], var().chr(ii));
			//unescape %25 to % LAST!
			env.USER1.swapper(hexx[37], var().chr(37));

			// next i

		}

	}

	if (linkfile2.osopen(linkfilename2)) {
		linkfile2.osclose();
		linkfilename2.osdelete();
		//osread _USER3 from linkfilename3 else env.USER3=''
	}

	if (logfilename!=L"") {

		if (anydata) {
			logx = L"</DataIn>";
			gosub_writelogx2();
		}

		//to be overwritten unless fails
		logx = L"</Message></Log>";
		gosub_writelogx3();

	}

	//update security table every few secs and every login
	if (request1 == L"LOGIN" || var(L"036").index(var().time()).substr(-1, 1))
		gosub_getsecurity();

	//validate username and password (like login)
	//and assume that identity if ok
	gosub_validate();

	env.SYSTEM.replacer(2, 0, 0, linkfilename2);

	//get the current program stack
//	var stack = programstack(nostack);

	try
	{
		gosub_process();
	}
//dont catch general MVExceptions to force debugging
//	catch (MVException& mvexception)
	catch (std::wstring& message)
	{
		env.USER3 = message;
		//gosub_properunlock();
		gosub_formatresponse();
	}
	catch (var& message)
	{
		env.USER3 = message;
		//gosub_properunlock();
		gosub_formatresponse();
	}

	//restore the program stack
	//limit on 299 "programs" and dictionary entries count as 1!!!
//	xx = programstack(stack);

	gosub_requestexit();

	if (halt)
		return false;

	if (request1 == L"STOPDB" && env.USER3 == L"OK")
		return false;

	if (request1 == L"RESTART" && env.USER3 == L"OK")
		return false;

	return true;

}

void Server::gosub_login()
{

	env.USER1 = L"";
	env.USER4 = L"";

	//custom login routine
	//returns iodat (cookie string "x=1&y=2" etc and optional comment)
	//if (xx.read(voc, L"LOGIN.NET")) {
		env.USER1 = L"";
		env.USER4 = L"";
		env.loginnet(dataset, username, env.USER1, env.USER4);
		if (env.USER1 == L"") {
			env.USER3 = env.USER4;
			return;
		}
	//}

	if (!(env.USER1))
		env.USER1 = L"X=X";
	env.USER3 = var(L"OK " ^ env.USER4).trim();

	//record the last login per user
	var users;
	if (users.open(L"USERS")) {
		var userrec;
		if (userrec.read(users, username)) {
			//save prior login
			userrec.replacer(15, 0, 0, userrec.extract(13));
			userrec.replacer(16, 0, 0, userrec.extract(14));
			//save current login
			var datetime = var().date() ^ L"." ^ (var().time()).oconv(L"R(0)#5");
			userrec.replacer(13, 0, 0, datetime);
			userrec.replacer(14, 0, 0, env.SYSTEM.extract(40, 2));
			userrec.write(users, username);
		}
	}

	return;
}

void Server::gosub_validate()
{
	invaliduser = L"Error: Invalid username and/or password";

	//encrypt the passwors and check it
	var encrypt0 = env.encrypt2(password ^ L"");
	var usern;
	var systemrec;
	var tt;
	var connections;

    if (username==L"NEOSYS") goto userok;
	else if (env.SECURITY.locate(username, usern, 1)) {
		if (encrypt0 == (env.SECURITY.extract(4, usern, 2)).field(TM, 7, 1)) {

			//determine allowed connections
			connections = env.SECURITY.extract(6, usern);

			//skip old integer sleep times
			if (connections.match(L"1N0N"))
				connections = L"";

			if (!connections) {

				//look for ip numbers in following users of the same group
				int nn = (env.SECURITY.extract(6)).count(VM) + 1;
				for (int ii = usern + 1; ii <= nn; ii++) {
					connections = env.SECURITY.extract(6, ii);

					//skip old integer sleep times
					if (connections.match(L"1N0N"))
						connections = L"";

				//BREAK;
				if (!(!connections && env.SECURITY.extract(1, ii + 1) != L"---")) break;;
				};//ii;

				//otherwise use system default
				if (!connections)
					connections = env.SYSTEM.extract(39);

				//otherwise use traditional private ip ranges
				//cannot implement this until check existing clients installations
				//if connections else connections='192.168.*.*;10.*.*.*'

			}

			//convert wildcards
			if (connections.substr(-1, 1) != L" ") {

				connections.converter(L",; ", SVM ^ SVM ^ SVM);
				int nn = connections.dcount(SVM);
				for (int ii = 1; ii <= nn; ii++) {

					var connectionx = connections.extract(1, 1, ii).quote();
					connectionx.swapper(L"*", L"\"0N\"");

					//invert the bits that are "quoted" to suit the match syntax '"999"0n'
					//""0n"."0n"" will become 0n"."0n
					connectionx.swapper(L"\"\"", L"");

					//pad missing dots so that 192.* becomes 192"."0n"."0n"."0n
					var ndots = connectionx.count(L".");
					if (ndots < 3)
						connectionx ^= var(L"\".\"0N").str(3 - ndots);

					connections.replacer(1, 1, ii, connectionx);
				};//ii;

				//buffer it (space prevents reassement even if none)
				//trailing space also means wildcards have been converted
				env.SECURITY.replacer(6, usern, 0, connections ^ L" ");
			}

			//check if allowed to connect from xxx

			if (connections.trim()) {
				int nn = connections.dcount(SVM);
				int ii;
				for (ii = 1; ii <= nn; ii++) {
					var connectionx = (connections.extract(1, 1, ii)).trimb();
				//BREAK;
				if ((connection.extract(1, 2)).match(connectionx)) break;;
				};//ii;
				if (ii > nn) {
					invaliduser = username ^ L" is not authorised to login" ^ FM ^ L"on this computer (IP Number: " ^ connection.extract(1, 2) ^ L")";
						return;
					}
				}

userok:
				env.USERNAME=username;
				tt = connection.extract(1, 2);
				if (connection.extract(1, 3) != tt)
					tt ^= L"_" ^ connection.extract(1, 3);
			tt.converter(L". ", L"_");
			env.STATION=tt;
			invaliduser = L"";
		}
	}else{

		//check revelation system file
		if (systemrec.read(env.DEFINITIONS, L"SYSUSER*"^username)) {
			//if systemrec<7>=encrypt0 and (speed<1.5 or mode='sleep') then ok=1
			if (systemrec.extract(7) == encrypt0)
				goto userok;
		}

	}

	//return

}

void Server::gosub_requestexit()
{

	var().unlockall();

	if (env.USER3 == L"")
		env.USER3 = L"Error: No response";

	if (env.USER3.index(L"ERROR NO:", 1))
		env.logger(L"Server", env.USER3);

	env.USERNAME=Serverusername;
	env.STATION=Serverstation;

	//if env.USER3[1,6].ucase()='ERROR:' then iodat=''

	//have to call it here as well :(
	requeststoptime=var().ostime();

	var rawresponse = env.USER3;
	rawresponse.converter(var().chr(13) ^ var().chr(10), L"|");

	if (logfilename!=L"") {

		var secs = (requeststoptime - requeststarttime).oconv(L"MD20P");
		logx = L"<Response";
		logx ^= L" ProcessingSecs=" ^ secs.quote();
		logx ^= L">";
		gosub_writelogx2();

		//convert non ascii to hexcode
		logx = env.USER3;
		//escape % to %25 FIRST!
		logx.swapper(L"%", L"%25");
		//for (int ii = 128; ii <= 255; ii++)
		for (int ii = 249; ii <= 255; ii++)
			logx.swapper(var().chr(ii), hexx[ii]);
		for (int ii = 0; ii <= 31; ii++)
			logx.swapper(var().chr(ii), hexx[ii]);
		gosub_writelogx();

		logx = L"</Response>" ^ (var().chr(13) ^ var().chr(10));
		iodatlen = env.USER1.length();
		if (iodatlen)
			logx ^= L"<DataOut>";
		gosub_writelogx2();

	}

	if (env.USER1 == L"%DIRECTOUTPUT%") {

		logx = env.USER1;
		logx.swapper(L"%", L"%25");
		gosub_writelogx();

	}else{

		var(L"").oswrite(linkfilename2);
		if (linkfile2.osopen(linkfilename2)) {

			//split into blocks and convert to escape chars
			for (int blockn = 1; blockn <= nblocks; blockn++)
				datx[blockn]=L"";
//			datx.initarray(L"");
			var ptr = 0;
			for (int blockn = 1; blockn <= nblocks; blockn++) {
				var blk = env.USER1.substr(1, outblocksize);
				env.USER1.splicer(1, outblocksize, L"");
			//BREAK;
			if (!blk.length()) break;;

				//escape the escape character first!
				blk.swapper(L"%", L"%25");

				//allow top eight pick language characters to pass through
				//to whatever corresponding unicode characters are desired by front end
				for (int ii = 249; ii <= 255; ii++) {
					blk.swapper(var().chr(ii), hexx[int(ii)]);
					//should not be done per block but is code economic
					env.USER3.swapper(var().chr(ii), hexx[int(ii)]);
				};//ii;

				//write BEFORE converting control characters
				//since writing restores 10-17 back up to F8-FF
				env.osbwritex(blk, linkfile2, linkfilename2, ptr);
				ptr += blk.length();

				//encode control characters to show in log
				//should convert x10-x17 back to proper characters
				for (int ii = 0; ii <= 31; ii++) {
					blk.swapper(var().chr(ii), hexx[int(ii)]);
					//should not be done per block but is code economic
					env.USER3.swapper(var().chr(ii), hexx[int(ii)]);
				};//ii;

				if (logfilename!=L"") {
					blk.transfer(logx);
					gosub_writelogx();
				}

				blk = L"";

			};//blockn;

			linkfile2.osclose();

		}else{

			env.USER3 = L"ERROR: Server cannot create temp " ^ linkfilename2;

		}

	}

	//try to flush file open
	if (linkfile2.osopen(linkfilename2))
		linkfile2.osclose();

	if (logfilename!=L"") {
		var tt = L"";
		if (iodatlen)
			tt ^= L"</DataOut>";
		tt ^= L"</Message>" ^ (var().chr(13) ^ var().chr(10));
		env.osbwritex(tt, logfile, logfilename, logptr);
		logptr += tt.length();

		env.osbwritex(L"</Log>", logfile, logfilename, logptr);

	}

	//swap '|' with wchar_t(13) in env.USER3
	//swap fm with wchar_t(13) in env.USER3

	//write the response
	env.oswritex(env.USER3, linkfilename3);

	//trace responded
	requeststoptime=var().ostime();
	if (tracing) {

		std::wcout << L"Responding in "<< (requeststoptime - requeststarttime).oconv(L"MD20P")<< L" SECS "<< rawresponse << std::endl;
		//print linkfilename1
	}

	//flush
	//suspend 'dir>nul'

	//if tracing then print

	return;

}

void Server::gosub_exit()
{

	if (logfilename!=L"")
		logfile.osclose();

	//remove lock indicating processing
	//gosub sysunlock

	var().unlockall();

	//restorescreen(origscrn, origattr);

	//remove flag that this dataset is being served (L"listened")
	//osdelete inpath:serverflagfilename

	//get into interactive mode
	//system<33>=origbatchmode
	env.SYSTEM.replacer(33, 0, 0, L"");
	env.PRIVILEGE=origprivilege;

	if (request1 == L"RESTART") {
		//chain 'Server'
		env.SYSTEM.replacer(35, 0, 0, nrequests);
		//msg='restart'
		//stop

		origsentence.swapper(L" INTRANET", L"");
		origsentence.chain();
	}

	//BREAK OFF;
	//BREAK ON;

	if (origbatchmode || request1 == L"STOPDB" || halt) {
		//BREAK OFF;
		var(L"OFF").perform();
	}

	//msg is @user4
	env.USER4 = L"TERMINATED OK";

	var().stop();

}

void Server::gosub_process()
{

	//process the input
	//////////////////

	//failure in Server above
	if (Serverfailure) {

	//invalid username or password or connection
	}else if (invaliduser!=L"") {
		env.USER1 = L"";
		env.USER3 = invaliduser;
//call oswrite(username:fm:password:fm:encrypt0:fm:env.SECURITY,date()[-3,3]:time():'.$$$')
		//if no request then possibly some failure in file sharing

	}else if (env.USER0 == L"") {
		env.USER1 = L"";
		env.USER3 = L"Error: No request";

	//empty loopback to test connection (no username or password required)
	}else if (request1 == L"TEST") {
		//iodat='' if they pass iodat, pass it back
		env.USER3 = L"OK";

	//check if can login
	}else if (request1 == L"LOGIN") {

		gosub_login();

	//find index values
	}else if (request1.substr(1, 14) == L"GETINDEXVALUES") {

		getindexvalues();

	//select some data
	}else if (request1 == L"SELECT") {

		select();

	//lock a record
	}else if (request1 == L"LOCK" || request1 == L"RELOCK") {

		filename = request2;
		keyx = request3;
		sessionid = request4;

		//lockduration is the number of minutes to automatic lock expiry
		lockdurationinmins = request5;

		gosub_lock();
		sessionid.transfer(env.USER1);

	}else if (request1 == L"UNLOCK") {

		filename = request2;
		keyx = request3;
		sessionid = request4;

		gosub_unlock();

		//read a record

	}else if (request1 == L"READ" || request1 == L"READO" || request1 == L"READU") {

		withlock = request1 == L"READU";
		updatenotallowed = L"";
		env.USER1 = L"";
		sessionid = L"";

		filename = request2;
		keyx = request3;
		lockdurationinmins = request4;
		var readenvironment = request5;
        win.templatex = readenvironment;

		//reduce chance of using old common
		for (int ii=0;ii<10;ii++)
			win.registerx[ii]=L"";

		var library=L"";
		if (!library.load(filename))
			{};//throw MVException("MVCipc() library.load() " ^ filename.quote() ^ L" unknown filename");

        newfilename=L"";
        //if (library) newfilename=library.call(L"GETALIAS");

		//disallow read/write to unknown files
		if (false&&newfilename == L"") {
			env.USER3 = request1 ^ L" " ^ filename.quote() ^ L" is not allowed";
			return;
		}
		if (newfilename == L"") newfilename = filename;
        
		filename2 = env.singular(newfilename);
		filename2.converter(L".", L" ");
		//if security(filename2:' access',msg0,'') else
		// if security('#':filename2:' access ':quote(keyx),msg2,'') else
		// transfer msg0 to env.USER3
		// gosub formatresponse
		// return
		// end
		// end
		if (!filesecurity(L"ACCESS")) return;

		//security check - cannot lock so cannot write or delete
		if (withlock) {
			//if 1 then
			if (env.authorised(filename2 ^ L" UPDATE", updatenotallowed, L""))
				{}
			if (env.authorised(filename2 ^ L" CREATE", createnotallowed, L""))
				{}
			//if updatenotallowed and createnotallowed then
			// env.USER3='error: sorry, you are not authorised to create or update records in the ':lcase(filename):' file.'
			// *env.USER3='error: lock not authorised'
			// gosub formatresponse
			// return
			// end
		}

		var file;
		if (!file.open(filename)) {
			gosub_badfile();
			return;
		}

        if (library) {
			keyx.transfer(env.ID);
			library.call(L"PREREAD");
			env.DATA = L"";
			env.ID.transfer(keyx);
		}

		var autokey = 0;
		if (keyx == L"" || keyx.substr(1, 1) == L"*" || keyx.substr(-1, 1) == L"*" || keyx.index(L"**", 1)) {

			//must provide a key unless locking
			if (!withlock) {
				env.USER3 = L"Error: NEOSYS.NET Key missing and not READU";
					gosub_formatresponse();
					return;
				}

getnextkey:

				//setup environment for def.sk
			win.wlocked = 0;
			env.RECORD = L"";
			env.ID = keyx;
			win.datafile = filename;
			win.srcfile = file;

//TODO			generalsubs(L"DEF.SK." ^ readenvironment);
//			keyx = isdflt;
keyx=L"";

			if (keyx == L"") {
				env.USER3 = L"Error: Next number was not produced" ^ FM ^ env.USER4;
				gosub_formatresponse();
				return;
			}

			autokey = 1;

		}

		sessionid = L"";
		if (withlock) {
			//lockduration is the number of minutes to automatic lock expiry
			//lockdurationinmins=request4
			lockdurationinmins = 10;
			env.USER3 = L"";

			gosub_lock();

			//if cannot lock then get next key
			if (env.USER3 == L"NOT OK" && autokey)
				goto getnextkey;

			//cannot do this because should return the record but without a lock
			//even if a lock requested
			//if env.USER3 then
			// gosub formatresponse
			// return
			// end
		}

		env.FILEERRORMODE = 1;
		env.FILEERROR = L"";
		if (env.USER1.read(file, keyx)) {

			//if record already on file somehow then get next key
			if (autokey)
				goto getnextkey;

			//prevent update
			if (withlock && updatenotallowed) {
				if (sessionid)
					gosub_unlock();
				// *do this after unlock which sets response to ok
				//env.USER3=updatenotallowed
				//gosub formatresponse
				//return
				lockauthorised = 0;
				sessionid = L"";
			}

			if (withlock && sessionid == L"") {
				env.USER3 = L"Error: CANNOT LOCK RECORD";
			}else{
				env.USER3 = L"OK";
			}
			if (sessionid)
				env.USER3 ^= L" SESSIONID " ^ sessionid;
		}else{
			//if @file.error<1>='100' then
			//no file error for jbase
			if (!env.FILEERROR || env.FILEERROR.extract(1) == L"100") {

				//prevent create
				if (withlock) {
					if (createnotallowed) {
						if (sessionid)
							gosub_unlock();
						//do this after unlock which sets response to ok
						env.USER3 = createnotallowed;
						gosub_formatresponse();
						return;
					}
				}

				//env.USER3='error: ':quote(keyx):' does not exist in the ':quote(newfilename):' file'
				env.USER1 = L"";
				env.USER3 = L"Error: NO RECORD";
				if (sessionid)
					env.USER3 ^= L" SESSIONID " ^ sessionid;

				//response/@user3/recordkey may be used in postread
				if (autokey) {
					//env.USER3:=' recordkey ':keyx
					var tt = keyx;
					//horrible cludge to allow space in recordkey to be understood in client.htm
					tt.swapper(L" ", L"{20}");
					env.USER3 ^= L" RECORDKEY " ^ tt;
				}

			}else{
				gosub_geterrorresponse();
			}
		}

		if (withlock) {
			if (!lockauthorised)
				env.USER3 ^= L" LOCK NOT AUTHORISED";
		}

		//postread (something similar also in select2)
		if (library) {

			//simulate environment for postread
			win.srcfile = file;
			win.datafile = filename;
			if (!env.DICT.open(L"dict_"^win.datafile)) {
				env.USER3 = L"Server::gosub_process() DICT." ^ win.datafile ^ L" file cannot be opened";
				gosub_formatresponse();
				if (sessionid)
					gosub_unlock();
				return;
			}
			keyx.transfer(env.ID);
			env.USER1.transfer(env.RECORD);
			win.orec = env.RECORD;
			win.wlocked = sessionid;
			var origresponse = env.USER3;
			//_USER3=''
			env.USER4 = L"";
			var resetx = 0;

			library.call(L"POSTREAD");
			env.DATA = L"";

//restore this programs environment
			env.RECORD.transfer(env.USER1);
			env.ID.transfer(keyx);

			env.USER1.cropper();

			//postread can request abort by setting msg or resetx>=5
			if (resetx >= 5 || env.USER4) {
				if (withlock) {
					gosub_unlock();
					//win.wlocked=0
				}
				//if msg then msg='error: ':msg
				env.USER3 = env.USER4;
				if (env.USER3 == L"")
					env.USER3 = L"ACCESS REFUSED";
				env.USER1 = L"";
				gosub_formatresponse();
				//env.USER3='error: ':env.USER3
			}else{
				env.USER3 = origresponse;
				//postread may have provided a record where non-was found
				//only if it unlocks it as well! otherwise
				//removal of no record will cause failure in client
				if (!win.wlocked && env.USER1) {
					env.USER3.swapper(L"Error: NO RECORD", L"");
					env.USER3.trimmerb();
					if (env.USER3 == L"")
						env.USER3 = L"OK";
				}

			}

			//postread may have unlocked the record
			//(and removed it from the locks file)
			//but make sure
			if (sessionid && !win.wlocked) {

				var storeresponse = env.USER3;
				gosub_unlock();
				env.USER3 = storeresponse;

				//remove session id
				var tt = env.USER3.index(L"SESSIONID", 1);
				if (tt) {
					env.USER3.splicer(tt, var(L"SESSIONID " ^ sessionid).length(), L"");
					env.USER3.trimmerb();
					if (env.USER3 == L"")
						env.USER3 = L"OK";
				}

			}

			//do not indicate record found
			//otherwise delete button will be enabled and deleting a record
			//that does not exist is disallowed but lock will be unlocked
			//if iodat then
			// env.USER3='ok'
			// if sessionid then env.USER3:=' sessionid ':sessionid
			// end

		}

		//prevent reading passwords postread and postwrite
		if (filename == L"DEFINITIONS" && keyx == L"SECURITY")
			env.USER1.replacer(4, 0, 0, L"");

		env.USER1.cropper();

	}else if (request1 == L"WRITEU" || request1 == L"DELETE" || request1 == L"WRITE") {

		filename = request2;
		keyx = request3;
		var fieldno = request4;
		sessionid = request5;

//		routineexists=routines.count(filename.towstring());
//		MVRoutine *routine;
//		if (routineexists)
//		{
//			routine=routines[filename.towstring()];
//			newfilename=routine->execute(L"GETALIAS");
//		}
//		else
		var library="";
		if (!library.load(filename))
			{};
        newfilename=L"";
        //if (library) newfilename=library.call(L"GETALIAS");

		//disallow read/write to unknown files
		if (false&&newfilename == L"") {
			env.USER3 = request1 ^ L" " ^ filename.quote() ^ L" is not allowed";
			return;
		}
		if (newfilename == L"") newfilename = filename;

		filename2 = env.singular(newfilename);
		filename2.converter(L".", L" ");

		//double check allowed access to file
		if (!filesecurity(L"ACCESS")) return;

		//if security(filename2:' access',msg0,'') else
		// if security('#':filename2:' access ':quote(keyx),msg2,'') else
		// transfer msg0 to env.USER3
		// gosub formatresponse
		// return
		// end
		// end

		//simulate environment

		//reduce chance of using old common
		for (int ii=0;ii<10;ii++)
			win.registerx[ii]=L"";

		env.ID = keyx;
		//@record=iodat
		env.USER1.transfer(env.RECORD);
		win.datafile = filename;
		//not really needed because pre/post code should assume that it is win.wlocked
		//but some code does not know that (eg postread called from postwrite)
		win.wlocked = sessionid;
		win.saverec = !(request1 == L"DELETE");
		win.deleterec = (request1 == L"DELETE");

		//trim excess field and value marks
		env.RECORD.cropper();

		if (!win.srcfile.open(win.datafile)) {
			env.USER3 = win.datafile.quote() ^ L" file is not available";
			return;
		}

		if (!env.DICT.open(L"dict_"^win.datafile)) {
			env.USER3 = (L"DICT." ^ win.datafile).quote() ^ L" file is not available";
			return;
		}

		//make sure that the record is already locked to the user
		lockkey = filename ^ L"*" ^ env.ID;
		if (!lockx.read(locks, lockkey))
			lockx = FM ^ FM ^ FM ^ FM ^ L"NO LOCK RECORD";
		if (sessionid != lockx.extract(5)) {
			env.USER3 = L"Somebody has updated this record." _VM_ L"Your update cannot be applied." L"" _VM_ L"The session id does not agree " ^ lockx.extract(5).quote();
			gosub_formatresponse();
			return;
		}

		//get a proper lock on the file
		//possibly not necessary as the locks file entry will prevent other programs
		//proper lock will prevent index mfs hanging on write

		win.valid = 1;
		gosub_properlock();
		if (!win.valid)
			return;

		if (!win.orec.read(win.srcfile, env.ID))
			win.orec = L"";

		//trim excess field and value marks
		win.orec.cropper();

		//double check not updated by somebody else
		//nb this does not work for delete until client provides
		//record or at least date_time in delete
		//not implemented for timesheets due to possibility of old mac client not working
		if (request1 != L"DELETE" && filename != L"TIMESHEETS") {
			// *cannot do this unless
			//a) return new record (done) or at least date_time in writeu to client
			//b) provide record in delete
			if (allcols != L"") {
				var dictrec;
				//if (dictrec.reado(allcols, filename ^ L"*DATE_TIME")) {
				if (dictrec.read(allcols, filename ^ L"*DATE_TIME")) {
					var datetimefn = dictrec.extract(2);
					var olddatetime = win.orec.extract(datetimefn);
					var newdatetime = env.RECORD.extract(datetimefn);
					if (olddatetime && olddatetime != newdatetime) {
						env.USER3 = L"Somebody else has updated this record." _VM_ L"Your update cannot be applied." L"" _VM_ L"The time stamp does not agree";
						gosub_formatresponse();
						gosub_properunlock();
						return;
					}
				}
			}
		}

		//detect if custom postwrite or postdelete called
		var postroutine = 0;

		if (request1 == L"WRITEU" || request1 == L"WRITE") {

			//prevent writing an empty record
			if (env.RECORD == L"") {
emptyrecorderror:
					env.USER3 = L"Write empty data record is disallowed.";
				gosub_properunlock();
				gosub_formatresponse();
				return;
			}

			//double check that the record has not been updated since read
			//cannot do this unless during write (not writeu) we pass back the new timedate
			//readv datetimefn from @dict,'date_time',2 then
			// if @record<datetimefn> ne win.orec<datetimefn> then
			// _USER3='somebody else has updated this record." _VM_ L"your update cannot be applied'
			// gosub properunlock
			// gosub formatresponse
			// return
			// end
			// end

			//custom prewrite processing
			if (library) {
				//call @updatesubs('prewrite')
				library.call(L"PREWRITE");
				env.DATA = L"";
			}

			if (!win.valid) {
				gosub_properunlock();
				env.USER3 = env.USER4;
				gosub_formatresponse();
				return;
			}

			//allow for prewrite to change the key (must not lock the new key)
			//prewrite must not unlock the record because write will fail by index mfs hanging if any
			//is the above true? because unlock just below will
			//remove the lock file record
			//and actually the unlock will fail if it cannot true lock the record
			//daybook.subs4 unlocks and it works fine and will leave lock hanging if it does not
			if (env.ID != keyx) {

				gosub_unlock();
				gosub_properunlock();

				keyx = env.ID;

				//cannot place a lock file entry if the prewrite has locked the new key
				file.unlock( keyx);

				gosub_lock();
				if (env.USER3 != L"OK")
					return;

				gosub_properlock();
				if (!win.valid)
					return;

			}

			env.RECORD.cropper();

			//check for empty record again in case updatesubs screwed up
			if (env.RECORD == L"")
				goto emptyrecorderror;

			//failsafe in case prewrite unlocks key?
			//gosub properlock

			env.RECORD.write(win.srcfile, keyx);

			//custom post write processing
			if (library) {
				postroutine = 1;
				library.call(L"POSTWRITE");
				env.DATA = L"";
			}

			//send back revised data or nothing
			//nb data is now '' to save space so always send back data unless @record is cleared
			if (env.RECORD == env.USER1) {
				env.USER1 = L"";
			}else{
				env.USER1 = env.RECORD;
			}

			//prevent reading passwords postread and postwrite
			if (filename == L"DEFINITIONS" && keyx == L"SECURITY")
				env.USER1.replacer(4, 0, 0, L"");

		}else if (request1 == L"DELETE") {

			//ensure that deletion works on the orig record
			env.RECORD = win.orec;

			//prevent deleting if record does not exist
			if (env.RECORD == L"") {
				env.USER3 = L"Cannot delete because " ^ keyx.quote() ^ L" does not exist in the " ^ newfilename.quote() ^ L" file";
				gosub_formatresponse();
				return;
			}

			//check allowed to delete
			//if security(filename2:' delete',msg,'') else
			// env.USER3=msg
			// gosub formatresponse
			// return
			// end
			if (!filesecurity(L"DELETE")) return;

			//custom predelete processing
			if (library) {
				//call @updatesubs('predelete')
				library.call(L"PREDELETE");
				env.DATA = L"";
			}
			if (!win.valid) {
				gosub_properunlock();
				env.USER3 = env.USER4;
				gosub_formatresponse();
				return;
			}

			win.srcfile.deleterecord(keyx);

			//custom post delete processing
			if (library) {
				postroutine = 1;
				library.call(L"POSTDELETE");
				env.DATA = L"";
			}

			//send back no iodat
			env.USER1 = L"";

			}

			//remove locks file entry
		if (request1 != L"WRITE") {

			locks.deleterecord(lockkey);
			relistlocks = 1;

			//unlock local lock
			win.srcfile.unlock( keyx);

		}else{
			gosub_properunlock();
		}

		//even postwrite/postdelete can now set invalid (to indicate invalid mode etc)
		if (win.valid) {
			env.USER3 = L"OK";
		}else{
			env.USER3 = L"Error:";
		}

		if (request1 != L"DELETE") {
			var tt = env.ID;
			//env.USER3:=' recordkey ':@id
			//horrible cludge to allow space in recordkey to be understood in client.htm
			tt.swapper(L" ", L"{20}");
			env.USER3 ^= L" RECORDKEY " ^ tt;
			if (sessionid)
				env.USER3 ^= L" SESSIONID " ^ sessionid;
		}

		//pre and post routines can return warnings/notes in msg

		if (env.USER4)
			env.USER3 ^= L" " ^ env.USER4;

		gosub_formatresponse();

		//if postroutine else call flush.index(filename)
		env.flushindex(filename);

		//execute a request

	}else if (request1 == L"EXECUTE") {

		//check request is a win.valid program
		var voccmd = request2;
		if (voccmd == L"") {
//badproxy:
			env.USER3 = L"Error: " ^ voccmd.quote() ^ L" proxy is not available";
			return;
		}
		voccmd ^= L"PROXY";
//		if (!xx.read(voc, voccmd))
//			goto badproxy;
//		routineexists=routines.count(voccmd.towstring());
//		MVRoutine *routine;
//		if (!routineexists)
//			goto badproxy;
//		routine=routines[voccmd.towstring()];
		//newfilename=routine->execute(L"GETALIAS");

		//provide an output file for the program to be executed
		//nb response file name for detaching processes
		//will be obtained from the output file name Server2 respond
		//this could be improved to work
		var printfilename = linkfilename2;
		var tt = var().oscwd();
		tt.splicer(-7, 7, L"");
		if (printfilename.substr(1, tt.length()) == tt)
			printfilename.splicer(1, tt.length(), L"..\\");

		//t=printfilename[-1,'b.']
		tt = printfilename.field2(L".", -1);
		printfilename.splicer(-tt.length(), tt.length(), L"htm");
		env.SYSTEM.replacer(2, 0, 0, printfilename);
		if (tracing)
			std::wcout << L"Waiting for output ... ";

		//switch to server mode and html output
		var s6 = env.SYSTEM.extract(6);
		var s7 = env.SYSTEM.extract(7);
		var s11 = env.SYSTEM.extract(11);
		env.SYSTEM.replacer(6, 0, 0, 1);
		env.SYSTEM.replacer(7, 0, 0, 1);
		env.SYSTEM.replacer(11, 0, 0, 1);

		//execute the program
		//print timedate2():' starting ':indata[1,60]
		//call env.msg2('processing remote request','ub',buffer,'')
		env.USER3 = L"OK";
		win.valid = 1;
		env.USER4 = L"";
		//call @voccmd(field(request,fm,3,99999),iodat,_USER3)
		//request, iodat and response are now passed and returned in @user0,1 and 3
		//other messages are passed back in @user4
		//execute instead of call prevents program crashes from crashing Server
		env.USER0 = env.USER0.field(FM, firstrequestfieldn + 2, 99999);

		//pass the output file in linkfilename2
		//not good method, pass in system?
		if (var(L"LIST" _VM_ L"SELECTBATCHES").locate(env.USER0.extract(1),xx,1))
			env.USER1 = linkfilename2;
		if ((env.USER0.extract(1)).substr(1, 4) == L"VAL.")
			env.USER1 = linkfilename2;

		//TODO
		//voccmd.execute();
//		routine->execute(voccmd);
		//call @voccmd
		//send errors to neosys
//		env.USER4="NOT IMPLEMENTED YET:\rEXECUTE " ^ voccmd ^ "\r" ^ env.USER0;

		//discard any stored input
		env.DATA = L"";

		//call env.msg2('','db',buffer,'')
		//print timedate2():' finished ':indata[1,60]

		//detect memory corruption?
		//@user4='r18.6'
		if (env.USER4.index(L"R18.6", 1)) {
			//oswriterequest on 'r186.$1'
			//oswriteiodat on 'r186.$2'
			//oswriteresponse on 'r186.$3'
			//oswritemsg on 'r186.$4'
			halt = 1;
			env.USER4.replacer(-1, 0, 0, L"Corrupt temporary file. Restart Needed.");
			env.USER4.replacer(-1, 0, 0, L"NEOSYS.NET TERMINATED");
		}

		//send errors to neosys
		if (env.USER4.index(L"An internal error", 1))
			env.sysmsg(env.USER4);

		//convert error message
		if (env.USER4.index(L"INDEX.REDUCER", 1))
			env.USER4 = L"Please select fewer records";

		env.USER4.cropper();
		env.USER3.cropper();

		if (env.USER4) {
			env.USER1 = L"";
			env.USER3 = L"Error: " ^ env.USER4;
			gosub_formatresponse();
		}

		if (env.USER3 == L"") {
			env.USER3 = L"Error: No OK from " ^ voccmd ^ L" " ^ env.USER0;
			gosub_formatresponse();
		}

		//switch off server mode
		env.SYSTEM.replacer(6, 0, 0, s6);
		env.SYSTEM.replacer(7, 0, 0, s7);
		env.SYSTEM.replacer(11, 0, 0, s11);

		//get the printfilename in case the print program changed it
		printfilename = env.SYSTEM.extract(2);
		if (tracing) {
			//print ' got it'
			std::wcout << env.at(0)<< env.at(-4);
		}

		//make sure that the output file is closed
		if (printfile.osopen(printfilename))
			printfile.osclose();

	}else if (request1 == L"STOPDB") {

		if (globalend.osfile()) {
			env.USER3 = L"Error: Database already stopped/stopping";

		}else{

			var(L"").oswrite(globalend);

			timex = var().time();
			while (true) {
			//BREAK;
			if (!(env.otherusers(L"").extract(1) && (var().time() - timex).abs() < 30)) break;;
				var().ossleep(1000);
			}//loop;

			env.USER1 = L"";

			if (env.otherusers(L"").extract(1)) {
				env.USER3 = L"Error: Could not terminate " ^ env.otherusers(L"") ^ L" users|" ^ env.otherdatasetusers(L"*");
				globalend.osdelete();
			}else{
				var(L"NET STOP NEOSYSSERVICE").osshell();

				if (request2 == L"RESTART") {
					globalend.osdelete();
					var(L"NET START NEOSYSSERVICE").osshell();
				}
				env.USER3 = L"OK";
			}

		}

	}else if (request1 == L"RESTART") {
		env.USER1 = L"";
		env.USER3 = L"OK";

	}else if (request1 == L"BACKUP") {
		gosub_getbakpars();

		//force additional processes ... that should fail due to backup lock
		(inpath ^ serverflagfilename).osdelete();

		//backup will respond to user itself if it starts
		env.USER4 = L"";
		var(L"FILEMAN BACKUP " ^ datasetcode ^ L" " ^ bakdisk).perform();

		//if backup has already responded to user
		//then quit and indicate to calling program that a backup has been done
		//user will be emailed
		if (env.SYSTEM.extract(2) == L"") {
			env.PSEUDO = L"BACKUP2";
			if (env.USER4)
				var().stop();
		}

		//note: if backup did not respond already then the requestexit will
		//respond as usual with the error message from backup
		env.USER1 = L"";

	}else if (request1 == L"VERSION") {
		env.USER1 = L"";
		env.USER3 = L"UNKNOWN VERSION";

	}else if (request1 == L"INSTALL") {
		env.DATA ^= var().chr(13);
		if (request2 == L"")
			request2 = env.ACCOUNT;
		(L"INSTALL " ^ request2 ^ L" I !:").execute();
		env.USER3 = env.USER4;

		//or give an error message

	}else if (1) {
		env.USER1 = L"";
		env.USER3 = L"Error: " ^ env.USER0.field(FM, 4, 9999).quote() ^ L" unknown request";
	}

	return;

}

void Server::gosub_geterrorresponse()
{
	var fileerror = env.FILEERROR;
	env.USER3 = L"Error: FS" ^ fileerror.extract(1, 1).xlate(L"SYS.MESSAGES", 11, L"X");
	env.USER3.swapper(L"%1%", env.handlefilename(fileerror.extract(2, 1)));
	env.USER3.swapper(L"%2%", fileerror.extract(2, 2));
	gosub_formatresponse();
	return;

}

void Server::gosub_formatresponse()
{

	//trim everything after <esc> (why?)
	var tt = env.USER3.index(L"<ESC>", 1);
	if (tt)
		env.USER3 = env.USER3.substr(1, tt - 1);

	//cannot remove since these may be proper codepage letters
	env.USER3.converter(L"|", FM);
	env.USER3.converter(VM, FM);
	if (env.USER3.substr(1, 1) == FM)
		env.USER3.splicer(1, 1, L"");
	env.USER3.swapper(FM, var().chr(13) ^ var().chr(10));

	return;

}

void Server::gosub_lockit()
{
    state=1;
return;
	//attempt to lock the record
	//BYPASS ordinary lock,file,key process otherwise
	//the lock record will be checked - and in this case
	//we our own lock record to be present
	code = 5;
	//lockit2:
	var nextbfs = L"";
	var handle = file;
	//<arev>
    
	//handle = handle.substr(-1, L"B" ^ VM);
    handle = handle.field2(VM,-1);
	//</arev>
	var keyorfilename = keyx;
	var fmc = 2;
	gosub_lockit2();
	return;
}

void Server::gosub_lockit2()
{
	//rtp57(code, nextbfs, handle, keyorfilename, fmc, record, state);
	std::wcout<<L"MFS file handle not handled yet"<<std::endl;
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

void Server::gosub_unlockit()
{
    return;
	//unlock file,keyx
	code = 6;
	gosub_lockit2();
	return;
}

void Server::gosub_gettimeouttime()
{
	if (!timeoutsecs) {
		timeouttime = L"";
		return;
	}

	var timeoutdate = var().date();
	timeouttime=var().ostime();
	timeouttime += timeoutsecs;
	if (timeouttime > 24 * 60 * 60) {
		timeoutdate += 1;
		timeouttime -= 24 * 60 * 60;
	}
	timeouttime = (timeoutdate ^ L"." ^ (timeouttime.floor()).oconv(L"R(0)#5")) + 0;
	return;
}

void Server::gosub_properlock()
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
		srcfile2.swapper(L"SHADOW.MFS" ^ SVM, L"");
	//</arev>

	//dont pass the filename because that causes persistent lock checking
	//in jbase version of env.lockrecord()
	if (!(env.lockrecord(L"", srcfile2, keyx))) {
		if (env.STATUS != 1) {
			win.valid = 0;
			env.USER3 = L"Error: " ^ keyx.quote() ^ L" CANNOT BE WRITTEN BECAUSE IT IS LOCKED ELSEWHERE";
		}
	}

	return;
}

void Server::gosub_properunlock()
{
	//nb i think that shadow.mfs is not programmed to remove the locks file entry

	//must unlock it properly otherwise indexing will try to lock it and fail
	//because it is only in the locks file and not properly locked

	//must exclude the shadowing system otherwise the entry in the locks file
	//will be removed as well
	var srcfile2 = win.srcfile;
	//<arev>
	srcfile2.swapper(L"SHADOW.MFS" ^ SVM, L"");
	//</arev>

	srcfile2.unlock( keyx);

	return;

}

void Server::gosub_lock()
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
	if (request1 == L"RELOCK") {
		newsessionid = sessionid;
	}else{
		newsessionid = L"";
		for (int ii = 1; ii <= 8; ii++)
			newsessionid ^= var(L"01234567890ABDCEF").substr(var(16).rnd() + 1, 1);
	}

	if (!file.open(filename)) {
		gosub_badfile();
		return;
	}

	env.USER3 = L"";
	if (request1 == L"RELOCK") {
		gosub_lockit();
	}else{
		if (env.lockrecord(filename, file, keyx)) {
			state = 1;
		}else{
			state = 0;
		}
	}
	if (!state) {
		env.USER3 = L"NOT OK";
		return;
	}

	//check locks file
	gosub_getostime();
	lockkey = filename ^ L"*" ^ keyx;
	if (lockrec.read(locks, lockkey)) {

		if (lockrec.extract(5) != newsessionid) {

			//fail if other lock has not timed out
			if (lockrec.extract(1) > ostimenow) {
				env.USER3 = L"NOT OK";
				goto lockexit;
			}

			//other lock has timed out so ok
			//no need to delete as will be overwritten below
			//delete locks,lockkey

		}

		//our own session ... so must be relocking (to extend timeout)

	}else{

		if (request1 == L"RELOCK") {
			//nb the word "expired" is a key word used in _formtriggers.htm
			env.USER3 = L"Error: Your lock expired and somebody else updated";
			goto lockexit;
		}

		//no lock record so ok

	}

	env.USER3 = L"OK";

	//convert minutes to fraction of one day (windows time format)
	lockduration = lockdurationinmins / (24 * 60);

	//write the lock in the locks file
	lockrec = L"";
	lockrec.replacer(1, 0, 0, lockduration + ostimenow);
	lockrec.replacer(2, 0, 0, ostimenow);
	lockrec.replacer(3, 0, 0, connection ? connection.extract(1, 2): env.STATION);
	lockrec.replacer(4, 0, 0, env.USERNAME);
	lockrec.replacer(5, 0, 0, newsessionid);
	env.FILEERRORMODE = 1;
	env.FILEERROR = L"";
	env.USER3 = L"OK";
	lockrec.write(locks, lockkey);
	if (env.FILEERROR) {
		env.mssg(L"CANNOT WRITE LOCKS RECORD " ^ lockkey);
		gosub_geterrorresponse();
	}
	relistlocks = 1;

	sessionid = newsessionid;

lockexit:

	//unlock file,keyx
	if (request1 == L"RELOCK") {
		gosub_unlockit();
	}else{
		file.unlock( keyx);
	}

	return;

}

void Server::gosub_badfile()
{
	////////
	env.USER3 = L"Error: " ^ filename.quote() ^ L" file does not exist";
	return;
}

void Server::gosub_unlock()
{
	///////

	//sessionid is used as a check that only the locker can unlock
	if (!file.open(filename)) {
		gosub_badfile();
		return;
	}

	//lock file,keyx
	gosub_lockit();
	if (!state) {
		//zzz perhaps should try again a few times in case somebody else
		//is trying to lock but failing because of our remote lock
		env.USER3 = L"Error: Somebody else has locked the record";
		return;
	}

	//get the current lock else return ok
	lockkey = filename ^ L"*" ^ keyx;
	env.FILEERRORMODE = 1;
	env.FILEERROR = L"";
	if (!lockrec.read(locks, lockkey))
		lockrec = L"";
	if (!lockrec) {
		if (env.FILEERROR.extract(1) == 100) {
			//lock is missing but ignore it
			//because we are unlocking anyway
			env.USER3 = L"OK";
		}else{
			gosub_geterrorresponse();
		}
		goto unlockexit;
	}

	//check that the current lock agrees with the session id provided
	if (!(sessionid == lockrec.extract(5))) {

		//cannot unlock because the lock belongs to somebody else
		env.USER3 = L"Error: Cannot unlock - ";
		if (sessionid == L"") {
			env.USER3 ^= L"missing session id";
		}else{
			env.USER3 ^= L"wrong session id";
		}

		goto unlockexit;
	}

	//delete the lock
	env.FILEERRORMODE = 1;
	env.FILEERROR = L"";
	env.USER3 = L"OK";
	locks.deleterecord(lockkey);
	if (env.FILEERROR) {
		env.mssg(L"CANNOT DELETE LOCK KEY " ^ lockkey);
		gosub_geterrorresponse();
	}
	relistlocks = 1;

unlockexit:

	gosub_unlockit();

	return;

}
void Server::gosub_getsecurity()
{
	if (!env.SECURITY.read(env.DEFINITIONS, L"SECURITY"))
	{
		//std::wcout<<L"CANNOT READ SECURITY"<<std::endl;
		//return;
        env.SECURITY=L"";
	}
	//env.SECURITY.inverter();
	return;

}

void Server::gosub_getostime()
{
	ostimenow=var().ostime();
	//convert to windows based date/time (ndays since 1/1/1900)
	//31/12/67 in rev date() format equals 24837 in windows date format
	ostimenow = 24837 + var().date() + ostimenow / 24 / 3600;
	return;

}

void Server::gosub_flagserveractive()
{

	//flag that this dataset is being served (L"listened") (needed for old mac)
	//does not seem to cause any filesharing errors (stress tested at max speed)
	var(L"").oswrite(inpath ^ serverflagfilename);

	return;

}

void Server::gosub_getbakpars()
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
	if (!bakpars.read(env.DEFINITIONS, L"BACKUP"))
		bakpars = L"";
	var tt;
	if (tt.osread("BACKUP.CFG")) {
		for (int ii = 1; ii <= 99; ii++) {
			if (tt.extract(ii))
				bakpars.replacer(ii, 0, 0, tt.extract(ii));
		};//ii;
	}
	var lastbakattemptdate = bakpars.extract(1);
	//if bakpars<2> then
	// bakreq=(trim(@station)=bakpars<2>)
	//end else
	// bakreq=onserver
	// end
	var testdata = 0;
	if (datasetcode.substr(-4, 4) == L"TEST")
		testdata = 1;
	if (env.SYSTEM.extract(23).ucase().index(L"TRAINING", 1))
		testdata = 1;
	if (env.SYSTEM.extract(23).ucase().index(L"TESTING", 1))
		testdata = 1;
	if (bakpars.locate(datasetcode, xx, 8))
		testdata = 1;
	//testdata will not backup but will terminate if bakreq
	//bakreq=not(testdata)
	var bakreq = 1;
	if (bakpars.extract(9))
		bakreq = 0;
	//address to email backup is bakpars<10>

	var minbaktime = bakpars.extract(3);
	if (minbaktime == L"")
		minbaktime = var(L"2:00").iconv(L"MT");
	//if @username='neosys' then minbaktime=iconv('00:00','mt')
	var maxbaktime = bakpars.extract(4);
	//if maxbaktime='' then maxbaktime=iconv('2:05','mt')
	if (maxbaktime == L"")
		maxbaktime = minbaktime + 60 * 10;
	//if @username='neosys' then maxbaktime=iconv('23:59','mt')

	var bakdows = bakpars.extract(5);
	if (bakdows == L"")
		bakdows = L"1234567";

	var bakdisk = bakpars.extract(7);

	return;
}

void Server::gosub_convlogx()
{

	//assumes at least 0-31 and 249-255 encoded like $hh

	//reserve/use special characters for field separators
	//not really necessary in unicode?
	logx.swapper(L"^", L"%5E");
	logx.swapper(L"]", L"%5D");
	logx.swapper(L"\\", L"%5C");
	logx.swapper(L"[", L"%5B");
	logx.swapper(L"%FE", L"^");
	logx.swapper(L"%FD", L"]");
	logx.swapper(L"%FC", L"\\");
	logx.swapper(L"%FB", L"[");
	logx.swapper(RM, L"%FF");
	//swap fm with '^' in logx
	//swap vm with ']' in logx
	//swap sm with '\' in logx
	//swap tm with "[" in logx
	//convert fefdfcfb to '^]\[' in logx
	logx.converter(fefdfcfb, L"^]\\[");
	logx.swapper(STM, L"%FA");
	logx.swapper(SSTM, L"%F9");

	logx.swapper(L"%20", L" ");
	//does not seem to format in xml
	//swap '%20' with '&nbsp;' in logx

	logx.swapper(L"&", L"&amp;");
	logx.swapper(DQ, L"&quot;");
	logx.swapper(L"<", L"&lt;");
	logx.swapper(L">", L"&gt;");
	logx.swapper(L"%26", L"&amp;");
	logx.swapper(L"%22", L"&quot;");
	logx.swapper(L"%3C", L"&lt;");
	logx.swapper(L"%3E", L"&gt;");

	return;
}

void Server::gosub_writelogx()
{
	gosub_convlogx();
	gosub_writelogx2();
	return;
}

void Server::gosub_writelogx2()
{
	env.osbwritex(logx, logfile, logfilename, logptr);
	logptr += logx.length();
	logx = L"";
	return;
}

void Server::gosub_writelogx3()
{
	env.osbwritex(logx, logfile, logfilename, logptr);
	logx = L"";
	return;
}


bool Server::filesecurity(const var& secmode)
{
	if (env.authorised(filename2 ^ L" " ^ secmode, msg0, L"")) {
		positive = L"";
	}else{
		positive = L"#";
	}
	var msgx=L"";
	if (!(env.authorised(positive ^ filename2 ^ L" " ^ secmode ^ L" " ^ keyx.quote(), msgx, L""))) {
		if (positive) {
			msg0.transfer(env.USER3);
		}else{
			msgx.transfer(env.USER3);
		}
		gosub_formatresponse();
		return 0;
	}
	return 1;

};

void Server::gosub_respond()
{
	//method to allow batch programs to respond and detach before finishing

	//determine the responsefile name from the printfilename
	//responsefilename=system<2>
	//if responsefilename else return
	//t=field2(responsefilename,'.',-1)
	//responsefilename[-len(t),len(t)]='3'

	//linkfilename3
	responsefilename = env.PRIORITYINT.extract(100);
	if (!responsefilename)
		return;

	//detach the calling process
	env.USER3 = request2;
	env.USER3.converter(VM ^ VM, FM ^ FM);
	env.USER3.swapper(FM, L"\r\n");

	env.USER3.oswrite(responsefilename);
	//osclose responsefilename

	var().ossleep(2000);

	//indicate that response has been made
	env.SYSTEM.replacer(2, 0, 0, L"");

	return;

}

void Server::getindexvalues()
{

	return;

/* NOT IMPLEMENTED YET - REQUIRES DB SIDE CODING

 	filename=request1;
 	fieldname=request2;
 	prefix=request3;
 	sortby=request4;

	_USER1 = L"";
	if (sortby) {
		if (!(var(L"AL" _VM_ "AR" _VM_ "DL" _VM_ "DR").locate(sortby, xx))) {
			env.USER3 = L"Invalid sortby " ^ sortby.quote() ^ L" in Server,GETINDEXVALUES";
			return;
		}
	}

	//security check
	var temp = filename;

	//zzz
	if (temp == L"BATCHES")
		temp = L"JOURNAL";

	temp.convert(L".", L" ");
	temp = env.singular(temp);
	if (!(env.authorised(temp ^ L" ACCESS", env.USER4, L""))) {
		env.USER3 = env.USER4;
		return;
	}
	if (!(env.authorised(temp ^ L" LIST", env.USER4, L""))) {
		env.USER3 = env.USER4;
		return;
	}

	collectixvals(filename, fieldname, prefix);
	env.PSEUDO.transfer(env.USER1);
	if (env.USER1.substr(1, 1) == FM)
		env.USER1.splicer(1, 1, L"");
	env.USER3 = L"OK";

	if (sortby && env.USER1) {
		env.USER1.convert(FM, env.RM);
		env.USER1 ^= env.RM;
		v119(L"S", L"", sortby.substr(1, 1), sortby.substr(2, 1), env.USER1, flag);
		env.USER1.convert(env.RM, FM);
		env.USER1.splicer(-1, 1, L"");
	}

	if (env.USER0.index(L"XML", 1)) {
		if (env.USER1) {
			env.USER1.swap(FM, L"</" ^ fieldname ^ L">" "</record>" ^ (var().chr(13) ^ var().chr(10)) ^ L"<record><" ^ fieldname ^ L">");
			env.USER1.splicer(1, 0, L"<record><" ^ fieldname ^ L">");
			env.USER1 ^= L"</" ^ fieldname ^ L">" "</record>";
		}
		env.USER1.splicer(1, 0, L"<records>");
		env.USER1 ^= L"</records>";
	}else{
		//convert fm to vm in iodat
	}

	return;

*/

}


void Server::select()
{
	//env.USER1=''

//TODO
//	var().clearselect();

	var filename0 = request2;
	var filename = filename0.field(L" ", 1, 1);
	var sortselect = request3;
	var dictids = request4;
	var options = request5;

	var file;
	if (!file.open(filename)) {
        env.USER3 = L"Server::select " ^ filename.quote() ^ L" cannot open file";
		return;
	}

	//security check
	if (!(var(L"MENUS" _VM_ L"ADMENUS").locate(filename, xx))) {
		var temp = filename;
		temp.convert(L".", L" ");
		temp = env.singular(temp);
		if (!(env.authorised(temp ^ L" ACCESS", env.USER4, L""))) {
			var msgx=L"";
			if (!(env.authorised(L"!#" ^ temp ^ L" ACCESS PARTIAL", msgx, L""))) {
				env.USER3 = env.USER4;
				return;
			}
		}
		env.USER4 = L"";
	}

	//any data passed to select is assumed to be a selectlist

	if (env.USER1) {
//TODO:
env.USER3=L"GeneralProxy: MAKELIST not implemented yet";
return;
//		makelist(L"", env.USER1, L"", L"");
		sortselect ^= L"%SELECTLIST%";
		env.USER1 = L"";
	}

//TODO:
//env.USER3="GeneralProxy: SELECT2 not implemented yet";
//return;
	win.select2(filename0, env.SYSTEM.extract(2), sortselect, dictids, options, env.USER1, env.USER3, L"", L"", L"");

	if (env.USER4) {
		env.USER3 = env.USER4.extract(1).trim(FM);
	}else{
		env.USER1 = L"%DIRECTOUTPUT%";
		//env.USER3='ok'
	}

	return;

};

void Server::gosub_deleteoldfiles(const var& inpath, const var& pattern)
{

	var deletetime = var().date() * 24 * 60 * 60 + var().time() - ageinsecs;

	//for each suitable file
	var filenames=var().oslistf(inpath,pattern);

	while (filenames) {

		//get the file time
		var filename = filenames.field(FM, 1, 1);
		filenames.splicer(1,filename.length()+1,L"");
        filename = inpath ^ filename;

		if (!(var(L".JPG" _VM_ L".GIF").locate(filename.substr(-4, 4).ucase(),xx,1))) {

			//a file ending .4 means that the .2 and .3 files need to be deleted
			if (filename.substr(-2, 2) == L".4") {
				filename.osdelete();
				filename.splicer(-1, 1, L"2");
				filename.osdelete();
				filename.splicer(-1, 1, L"3");
				filename.osdelete();

			}else{
				//and delete it if older than the cut off time
				//and has a file extension (ie leave params and params2)
				var fileattributes = filename.osfile();
				var filetime = fileattributes.extract(2) * 24 * 60 * 60 + fileattributes.extract(3);
				if ((filename.substr(-4, 4)).index(L".", 1) && filetime <= deletetime) {
					filename.osdelete();
				}else{
				}
			}

		}

	}//loop;

	return;

}

}
