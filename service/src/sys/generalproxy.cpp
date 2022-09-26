#include <exodus/library.h>
libraryinit()

#include <gethtml.h>
#include <authorised.h>
#include <otherusers.h>
#include <sysmsg.h>
#include <emailusers.h>
#include <usersubs.h>
#include <securitysubs.h>
#include <sendmail.h>
#include <uploadsubs.h>
#include <generalsubs.h>
#include <changelogsubs.h>
#include <safeselect.h>
#include <getsubs.h>
#include <proxysubs.h>
#include <addcent4.h>
#include <singular.h>

#include <sys_common.h>
#include <win_common.h>

#include <window.hpp>

var stationery;
var mode;
var html;
var dbn;
var emailresult;
var usersordefinitions;
var userkey;
var userx;
var newpassword;
var xx;
var fn;//num
var task;
var taskprefix;
var reportid;
var logyear;
var logfromdate;
var loguptodate;
var logkey;
var logsearch;

function main() {
	//!subroutine general(request,data,response)
	//c sys
	//global mode,stationery,task

	#define request_ USER0
	#define data_ USER1
	#define response_ USER3
	#define msg_ USER4

	#include <system_common.h>
	//global fn,stationery,mode

    //use app specific version of generalsubs
    if (APPLICATION ne "EXODUS") {
        generalsubs = "generalsubs_app" ;
    }


	//clearcommon();
	win.valid = 1;
	msg_ = "";
	stationery = "";

	call cropper(request_);
	call cropper(data_);
	mode = USER0.f(1).ucase();

	//request 2 - can be anything actually

	win.datafile = request_.f(2);
	var keyx = USER0.f(3);

	response_ = "OK";

	if (mode eq "TEST") {

	} else if (mode eq "TRANTEST") {

		/* TRANTEST - check speed and accuracy of "concurrent" updates by mass updating a single record.

		Simply add 1 to a field 1 of record key "1", 100 times in file exodus_trantest

		The Total: *MUST* go up by the expected number even if multiple browsers are running concurrently

		e.g. TWO browsers (not two tabs of one browser) running the test CONCURRENTLY using 10 repeats
		*MUST* increase the Total: by 2 x 10 x 100 = 2000

		The speed seems to go down a lot after thousands of update, perhaps due to postgresql needing
		a VACUUM to remove the historical versions of the updates records since it never actually
		overwrites the original data blocks.
		*/

		var transactions = "exodus_trantest";
		if (not open(transactions,transactions))
			createfile(transactions);

		//ERROR:  VACUUM cannot run inside a transaction block sqlstate:25001
		//if (not transactions.sqlexec("vacuum " ^ transactions.f(1))) {
		//	response_ = var().lasterror();
		//	stop();
		//}

		ID = "1";
		let ntrans = 100;
		//printl();
		//printl("TRANTEST 1.", THREADNO);
		for (const var recn : range(1, ntrans)) {
			//if (TERMINAL and recn % 100)
			//	printl(recn);
			if (not read(RECORD, transactions, ID))
				RECORD = "";
			if (not write(RECORD+1, transactions, ID)) {
				response_ = "ERROR: " ^ var().lasterror();
				stop();
			}
		}
		//printl();
		//printl("TRANTEST 2.", THREADNO);
		response_ = "OK Thread No. " ^ THREADNO ^ ", Total: " ^ (RECORD + 1);

	} else if (mode.f(1) eq "PREVIEWLETTERHEAD") {

		//comma sep
		var compcodes = request_.f(2);
		var testing = USER0.f(3);

		var allhtml = "";

		//if testing, style borders of td and divs for visual insight
		if (testing) {
			allhtml(-1) = "<style>";
			allhtml(-1) = "td {border:dotted 1px #EEEEEE;}";
			allhtml(-1) = "div {border:dashed 1px lightgrey;}";
			allhtml(-1) = "</style>";
		}

		let ncomps = compcodes.count(",") + 1;
		for (const var compn : range(1, ncomps)) {

			mode = "HEAD";
			var compcode = compcodes.field(",", compn);
			call gethtml(mode, html, compcode);

			allhtml(-1) = "<br />Company " ^ compcode ^ " from " ^ mode;
			allhtml(-1) = "<br />";

			//if not testing then wrap html in hr for clarity
			//if testing else allhtml<-1>='<hr/>'
			allhtml(-1) = html;
			//if testing else allhtml<-1>='<hr/>'

		} //compn;
		allhtml(-1) = "Press F5 to refresh any images just uploaded.";
		allhtml.replacer(FM, "\r\n");
		var(allhtml).oswrite(SYSTEM.f(2));
		gosub postproc();

	} else if (mode eq "PERIODTABLE") {

		var year = request_.f(2).field("-", 1).field("/", 2);
		var finyear = USER0.f(3);

		perform("PERIODTABLE " ^ year ^ " " ^ finyear ^ " (H)");

		gosub postproc();

	} else if (mode eq "FILEMAN" and request_.f(2) eq "COPYDB") {

		var copydb = USER0.f(3);
		if (not(SYSTEM.f(58).locate(copydb, dbn))) {
			{}
		}
		var todb = SYSTEM.f(63, dbn);
		if (not todb) {
			call mssg("\"Copy to\" database must be configured (and saved) for database " ^ copydb ^ " first");
			stop();
		}

		//ensure authorised to login to one or the other database
		//by ensuring the user is currently logged in to one or other database
		if ((USERNAME ne "EXODUS" and copydb ne SYSTEM.f(17)) and todb ne SYSTEM.f(17)) {
			USER4 = "In order to copy database " ^ (copydb.quote()) ^ " to " ^ (todb.quote()) ^ ",";
			msg_(-1) = "you must be logged in to database " ^ (copydb.quote()) ^ " or " ^ (todb.quote());
			USER4(-1) = "but you are currently logged in to database " ^ (SYSTEM.f(17).quote());
			call mssg(msg_);
			stop();
		}

		//should really have an option to close the live dataset and then copy
		if (not(authorised("DATASET COPY", USER4, "LS"))) {
			call mssg(msg_);
			stop();
		}

		var started = time().oconv("MTS");
		var otherusersx = otherusers(copydb);
		var log = started ^ " Started copy database " ^ copydb ^ " to " ^ todb;
		log ^= "|" ^ started ^ " Other processes online:" ^ otherusersx.f(1);

		perform("COPYDB " ^ copydb ^ " " ^ todb);

		USER3 = USER4;
		if (not response_) {
			log ^= "|" ^ time().oconv("MTS") ^ " Finished";
			call sysmsg(log);
			USER3 = "OK " ^ log;
		}

	} else if (mode eq "EMAILUSERS") {

		var groupids = USER1.f(1);
		var jobfunctionids = "";
		var userids = data_.f(2);
		if (not((groupids or jobfunctionids) or userids)) {
			call mssg("You must specify some groups or users to email");
			stop();
		}

		//ensure sender has an email address
		//not absolutely necessary but provides a return email address
		if (USERNAME ne "EXODUS" and not(USERNAME.xlate("USERS", 7, "X"))) {
			call mssg("You cannot send email because you do not have an email address for replies");
			stop();
		}

		//T=
		//R=reply to is current user
		//W=group is word to be found in user department
		var options = "TR";

		var subject = USER1.f(4);
		var message = data_.f(5);

		message.converter(TM, VM);
		call emailusers("", subject, message, groupids, jobfunctionids, userids, options, emailresult);

		if (emailresult) {
			emailresult.converter(VM ^ FM, "\r\r");
			response_ = "OK Email Sent to" "\r" ^ emailresult;
			} else {
			call mssg("No users can be found to email,|or some problem with email server");
		}
		USER1 = "";

	} else if (mode eq "CREATEDATABASE") {
		//For patsalides

		if (not(authorised("DATASET CREATE",msg_))) {
		    stop();
		}

		var targetdbname = USER1.f(1);
		var targetdbcode = USER1.f(2).lcase();
		var sourcedbcode = USER1.f(3).lcase();

		var sourcedatadir = "../data/" ^ sourcedbcode;
		var targetdatadir = "../data/" ^ targetdbcode;

		var dbcodes = dblist();

		//check source database exists
		if (not locateusing(FM, sourcedbcode, dbcodes)) {
			return invalid("Source database doesn't exists.");
		}

		//check new database and data dir dont already exist
		if (locateusing(FM, targetdbcode, dbcodes)) {
			return invalid("Sorry, " ^ targetdbcode.quote() ^ " already exists");
		}

		//stop source live service
		if (not osshell("systemctl stop agy_live@" ^ sourcedbcode)) {
			return invalid("Cannot stop " ^ sourcedbcode ^ "'s service");
		}

		//Copy source db to target db
        // ERROR:  CREATE DATABASE cannot run inside a transaction block
        //if (not dbcopy(sourcedbcode,targetdbcode)) {
		osshell("dbcopy " ^ sourcedbcode ^ " " ^ targetdbcode);

		//start source live service
		osshell("systemctl start agy_live@" ^ sourcedbcode);

		//check target db created
		var newdbcodes = dblist();
		if (not locateusing(FM, targetdbcode, newdbcodes)) {
			return invalid(targetdbname ^ targetdbcode.quote() ^ " not created");
		}

		//create target data dir
		//oscopy(sourcedatadir, targetdatadir);
		//KEEP IN SYNC. SIMILAR code in create_site, create_service and generalproxy.cpp
        osshell("mkdir -p " ^ targetdatadir);
        osshell("chmod a+srw " ^ targetdatadir);
        osshell("setfacl -d -m g::rw " ^ targetdatadir);
		if (not osdir(targetdatadir)) {
			return invalid("Error in creating target data directory");
		}

		//skip - target image dir created on first upload

		//update database name file
		if (not	oswrite(targetdbname, "../data/" ^ targetdbcode ^ "/name")) {
			return invalid("Cannot update database file name");
		}

		//email confirmaion
		call sysmsg("", targetdbname ^ " - (" ^ targetdbcode ^ ") created");

		response_ = "OK Database " ^ targetdbname ^ " " ^ targetdbcode.quote() ^ " has been created.";

	} else if (mode eq "VAL.EMAIL") {
		win.is = request_;
		call usersubs(mode);

	//case mode[-3,3]='SSH'
	// call ssh(mode)

	} else if (mode eq "PASSWORDRESET") {

		if (not(authorised("PASSWORD RESET", msg_))) {
			call mssg(USER4);
			stop();
		}

		var users;
		if (not(users.open("USERS", ""))) {
			call mssg("USERS file is missing");
			stop();
		}

		ID = USER0.f(2);
		var emailaddress = request_.f(3);

		var baduseroremail = "Either " ^ (ID.quote()) ^ " or " ^ (emailaddress.quote()) ^ " does not exist";

		var userrec;
		if (userrec.read(users, ID)) {
			usersordefinitions = users;
			userkey = ID;

			if (emailaddress.ucase() eq userx.f(7).ucase()) {

				//signify ok
				baduseroremail = "";

				call securitysubs("GENERATEPASSWORD");
				newpassword = ANS;
				userrec(4) = newpassword;

			}

		} else {
			usersordefinitions = DEFINITIONS;
			userkey = "BADUSER*" ^ ID;
			if (not(userrec.read(usersordefinitions, userkey))) {
				userrec = "";
			}
		}

		//record historical resets/attempts
		//datetime=(date():'.':time() 'R(0)#5')+0
		var datetime = date() ^ "." ^ time().oconv("R(0)#5");
		userrec.inserter(15, 1, datetime);
		userrec.inserter(16, 1, SYSTEM.f(40, 2));
		userrec.inserter(18, 1, ("Password Reset " ^ baduseroremail).trim());

		if (baduseroremail) {
			userrec.write(usersordefinitions, userkey);
			call mssg(baduseroremail);
			stop();
		}

		//prewrite (locks authorisation file or fails)
		win.valid = 1;
		call usersubs("PREWRITE");
		if (not(win.valid)) {
			stop();
		}

		userrec.write(usersordefinitions, userkey);

		//postwrite
		call usersubs("POSTWRITE");

		//cc the user too if the username is valid
		//call sysmsg('User: ':username:fm:'From IP: ':system<40,2>:fm:text,'Password Reset',userkey)

		//send the new password to the user
		var emailaddrs = userrec.f(7);
		var ccaddrs = "";
		var subject = "EXODUS Password Reset";
		var body = "User: " ^ ID;
		body(-1) = "Your new password is " ^ newpassword;
		call sendmail(emailaddrs, ccaddrs, subject, body, "", "", xx);

	} else if (mode eq "MAKEUPLOADPATH") {
		call uploadsubs("MAKEUPLOADPATH." ^ USER0.f(2));

	} else if (mode eq "POSTUPLOAD") {
		call uploadsubs("POSTUPLOAD");

	} else if (mode eq "VERIFYUPLOAD") {
		call uploadsubs("VERIFYUPLOAD." ^ request_.f(2));

	} else if (mode eq "OPENUPLOAD") {
		call uploadsubs("OPENUPLOAD." ^ USER0.f(2));

	} else if (mode eq "DELETEUPLOAD") {
		call uploadsubs("DELETEUPLOAD." ^ request_.f(2));

	} else if (mode eq "GETCODEPAGE") {
		data_ = "";
		//have to skip char zero it seems to be treated as string terminator
		//somewhere on the way to the browser (not in revelation)
		for (const var ii : range(1, 255)) {
			USER1 ^= chr(ii);
		} //ii;
		//data='xxx'
		USER3 = "OK";

	} else if (mode eq "SETCODEPAGE") {
		if (not(sys.alanguage.open("ALANGUAGE", ""))) {
			call fsmsg();
			stop();
		}

		var codepage = USER0.f(3);
		if (not codepage) {
			goto badsetcodepage;
		}

		if (request_.f(2) eq "SORTORDER") {

			/* should be inverted but bother since cant get DOS only collated ascii;
				if len(data) ne 254 then;
					call msg('SORTORDER data is ':len(data):' but must be 254 characters long|(initial char 0 excluded)');
					stop;
					end;
				write char(0):data on alanguage,'SORTORDER*':codepage;
			*/

		} else if (USER0.f(2) eq "UPPERCASE") {
			fn = 9;
setcodepagecase:
			var recordx;
			if (not(recordx.read(sys.alanguage, "GENERAL*" ^ codepage))) {
				if (not(recordx.read(sys.alanguage, "GENERAL"))) {
					recordx = "";
				}
			}
			var temp = data_;
			temp.replacer(RM, "%FF");
			temp.replacer(FM, "%FE");
			temp.replacer(VM, "%FD");
			recordx(1, fn) = temp;

			recordx.write(sys.alanguage, "GENERAL*" ^ codepage);

				/*;
				//check lower=lcase(upper) and vice versa
				//but ucase() may strip accents whereas lcase() may leave them
				x=@lower.case;
				y=@upper.case;
				convert @lower.case to @upper.case in x;
				convert @upper.case to @lower.case in y;
				convert @lower.case to @upper.case in y;
				convert @upper.case to @lower.case in x;
				for ii=1 to len(x);
					//if x[ii,1] ne @lower.case[ii,1] then print 'x: ':ii:' ':seq(x[ii,1]) 'MX':' ':seq(@lower.case[ii,1]) 'MX'
					if y[ii,1] ne @upper.case[ii,1] then print 'y: ':ii:' ':seq(y[ii,1]) 'MX':' ':seq(@upper.case[ii,1]) 'MX';
					next ii;
				*/

		} else if (request_.f(2) eq "LOWERCASE") {
			fn = 10;
			goto setcodepagecase;

		} else {
badsetcodepage:
			call mssg("Request must be SETCODEPAGE SORTORDER|UPPERCASE|LOWERCASE codepage");
			return 0;
		}

		response_ = "OK";

	} else if (mode eq "GETDATASETS") {
		ANS = "";
		call generalsubs("GETDATASETS");
		USER1 = ANS;
		if (data_) {
			USER3 = "OK";
		} else {
			response_ = "Error: No datasets found";
		}

	} else if (mode eq "LISTPROCESSES") {

		perform("SORT PROCESSES");

		gosub postproc();

	} else if (mode eq "LISTREQUESTLOG") {

		PSEUDO = USER1;
		perform("LISTREQUESTLOG");

		//printopts='L'
		gosub postproc();

	} else if (mode eq "LISTLOCKS") {

		perform("SORT LOCKS WITH NO LOCK_EXPIRED");

		gosub postproc();

	} else if (mode eq "GETVERSIONDATES") {

		call changelogsubs("GETVERSIONDATES");

	} else if (mode.ucase() eq "WHATSNEW") {

		call changelogsubs("SELECTANDLIST" ^ FM ^ data_);
		if (msg_) {
			USER4.move(USER3);
			stop();
		}

		//printopts='L'
		gosub postproc();

	} else if (mode eq "LISTADDRESSES") {

		perform("LISTADDRESSES");

		//printopts='L'
		gosub postproc();

	} else if (mode eq "GETDEPTS") {
		call usersubs("GETDEPTS");
		USER1 = ANS;
		response_ = "OK";

	} else if (mode eq "LISTACTIVITIES") {

		perform("LISTACTIVITIES " ^ USER0.f(2));

		gosub postproc();

	} else if (mode eq "ABOUT") {
		perform("ABOUT");
		//transfer @user4 to data
		//response='OK'
		msg_.move(USER3);
		response_.prefixer("OK ");

	} else if (mode eq "UTIL") {
		perform("UTIL");
		USER3 = "OK";

	} else if (mode eq "PROG") {
		perform("PROG");
		response_ = "OK";

	//LISTAUTH.TASKS = list of tasks LISTTASKS
	//LISTAUTH.USERS = list of users LISTUSERS
	} else if (mode.field(".", 1) eq "LISTAUTH") {

		win.wlocked = 0;
		win.templatex = "SECURITY";
		call securitysubs("SETUP");
		if (not(win.valid)) {
			stop();
		}
		//call security.subs('LISTAUTH')
		call securitysubs(mode);
		if (not(win.valid)) {
			stop();
		}
		call securitysubs("POSTAPP");

		gosub postproc();

	} else if (mode eq "READUSERS") {

		win.templatex = "SECURITY";
		call securitysubs("SETUP");
		if (not(win.valid)) {
			stop();
		}

		data_ = RECORD;
		USER3 = "OK";

	} else if (mode eq "GETREPORTS") {

		task = request_.f(2);
		gosub gettaskprefix();
		if (taskprefix) {
			if (not(authorised(taskprefix ^ " ACCESS", USER4, ""))) {
				call mssg(msg_);
				stop();
			}
		}

		var select = "SELECT DOCUMENTS BY-DSND EXODUS_STANDARD BY DESCRIPTION";

		var instructions = USER0.f(2);
		instructions.replacer(VM, "%FD");
		select ^= " WITH INSTRUCTIONS2 " ^ (instructions.quote());

		if (not(authorised("DOCUMENTS: ACCESS OTHER PEOPLES DOCUMENTS", xx, ""))) {
			select ^= " AND WITH CREATEDBY " ^ (USERNAME.quote());
		}

		call safeselect(select ^ " (S)");

		gosub opendocuments();

		USER1 = "";
		var repn = 0;
nextrep:
		if (readnext(reportid)) {
			var report;
			if (report.read(sys.documents, reportid)) {
				repn += 1;

				var temp = report.f(2);
				temp.replacer("&", "&amp;");
				temp.replacer("<", "&lt;");
				temp.replacer(">", "&gt;");
				report(2) = temp;

				//!dont send instructions since takes up space and not needed
				//DO send now to have info in requestlog
				//report<5>=''

				report.converter(VM, RM);
				let nn = report.count(FM) + 1;
				for (const var ii : range(1, nn)) {
					data_(ii, repn) = report.f(ii);
				} //ii;

				USER1(9, repn) = reportid;

	//print repn,data<1>
			}
			//if len(data)<65000 then goto nextrep
			if (data_.len() lt maxstrsize_ - 530) {
				goto nextrep;
			}
		}
		response_ = "OK";

	} else if (mode eq "DELETEREPORT") {

		gosub opendocuments();

		var docnos = request_.f(2);

		let ndocs = docnos.count(VM) + 1;
		for (const var docn : range(1, ndocs)) {
			ID = docnos.f(1, docn);
			if (ID) {
				if (RECORD.read(sys.documents, ID)) {
					win.orec = RECORD;
					call getsubs("PREDELETE");
					if (not(win.valid)) {
						goto exit;
					}

					if (win.valid) {
						sys.documents.deleterecord(ID);

						call getsubs("POSTDELETE");
					}
				}
			}
		} //docn;

	} else if (mode eq "UPDATEREPORT") {

		gosub opendocuments();

		var doc;
		if (not(doc.read(sys.documents, USER0.f(2)))) {
			call mssg("Document " ^ (request_.f(2).quote()) ^ " is missing");
			stop();
		}

		//TODO security

		doc(6) = lower(USER1);

		doc.write(sys.documents, USER0.f(2));

	} else if (mode eq "COPYREPORT") {

		gosub opendocuments();

		var doc;
		if (not(doc.read(sys.documents, request_.f(2)))) {
			call mssg("Document " ^ (USER0.f(2).quote()) ^ " is missing");
			stop();
		}

		task = doc.f(5);
		gosub gettaskprefix();
		if (taskprefix) {
			if (not(authorised(taskprefix ^ " CREATE", USER4, ""))) {
				call mssg(msg_);
				stop();
			}
		}

		call getsubs("DEF.DOCUMENT.NO");
		if (not(win.valid)) {
			stop();
		}

		var description = doc.f(2);
		doc(2) = description ^ " (Copy)";

		//prevent copy from appearing like a exodus standard
		doc(10) = "";

		doc(1) = USERNAME;
		doc.write(sys.documents, ID);

		data_ = ID ^ FM ^ doc;

		USER3 = "OK";

	} else if (mode eq "GETREPORT") {

		//printopts='L'
		gosub opendocuments();

		//get parameters from documents into @pseudo

		if (not(sys.document.read(sys.documents, request_.f(2)))) {
			USER4 = "Document " ^ (USER0.f(2).quote()) ^ " does not exist";
			call mssg(msg_);
			stop();
		}

		task = sys.document.f(5);
		gosub gettaskprefix();
		if (taskprefix) {
			if (not(authorised(taskprefix ^ " ACCESS", USER4, ""))) {
				call mssg(msg_);
				stop();
			}
		}

		//if task='BALANCES' then printopts='P'
		//if index(task,'MEDIADIARY',1) then printopts='X'

		PSEUDO = sys.document.f(6);
		PSEUDO.converter(RM, VM);
		PSEUDO = raise(PSEUDO);

		//merge any runtime parameters into the real parameters
		for (fn = 1; fn <= 999; ++fn) {
			var tt = USER1.f(fn);
			if (tt) {
				PSEUDO(fn) = tt;
			}
		} //fn;
		data_ = "";

		//save runtime params in case saving below for scheduled reruns
		//document<11>=lower(data)

		var sentencex = sys.document.f(5);
		sentencex.converter(VM, " ");
		USER1 = PSEUDO;

		//in case we are calling another proxy
		if (sys.document.f(5, 1).ends("PROXY")) {

			//run but suppress email
			//perform 'TEST ':request<2>:' (S)'

			request_ = raise(sys.document.f(5)).field(FM, 2, 999999) ^ FM ^ USER0.f(2);
			//moved up so parameters show in any emailed error messages
			//data=@pseudo
			//override the saved period with a current period
			var runtimeperiod = date().oconv("D2/E").b(4, 5);
			if (runtimeperiod.starts("0")) {
				runtimeperiod.cutter(1);
			}
			data_.replacer("{RUNTIME_PERIOD}", runtimeperiod);
			goto performreport;

		} else {

performreport:
			response_ = "";

			perform(sentencex);
			if (not USER3) {

				gosub postproc();
			}

		}

	} else if (mode eq "USAGESTATISTICS") {

		PSEUDO = USER1;
		perform("LISTSTATS");

		//printopts='L'
		gosub postproc();

	} else if (mode eq "VIEWLOG") {

		gosub initlog();
		var datedict = "LOG_DATE";

		var cmd = "LIST LOG" ^ logyear;
		cmd ^= " BY LOG_DATE BY LOG_TIME";
		cmd ^= " LOG_DATE LOG_TIME LOG_USERNAME LOG_WORKSTATION LOG_SOURCE LOG_MESSAGE2";
		if (logfromdate) {
			if (loguptodate) {
				cmd ^= "  xAND WITH LOG_DATE BETWEEN " ^ (logfromdate.oconv("D4").quote()) ^ " AND " ^ (loguptodate.oconv("D4").quote());
			} else {
				cmd ^= "  xAND WITH " ^ datedict ^ " >= " ^ (logfromdate.oconv("D4").quote());
			}
		}
		if (logkey) {
			cmd ^= " xAND WITH LOG_SOURCE1 STARTING " ^ (logkey.quote());
		}
		if (logsearch) {
			cmd ^= " xAND WITH ALLUPPERCASE CONTAINING " ^ (logsearch.quote());
		}
		cmd ^= " ID-SUPP";
		var temp = cmd.index("xAND");
		if (temp) {
			cmd.paster(temp, 5, "");
		}
		cmd.replacer("xAND", "AND");
		perform(cmd);

		//printopts='L'
		gosub postproc();

	} else if (mode eq "LISTMARKETS") {

		var cmd = "SORT MARKETS WITH AUTHORISED BY SEQ";
		cmd ^= " HEADING " ^ (var("List of Markets     'T'     Page 'PL'").quote());
		perform(cmd);

		gosub postproc();

	} else if (mode eq "LISTCURRENCIES") {

		perform("LISTCURRENCIES " ^ request_.f(2));

		gosub postproc();

	} else if (mode eq "LISTCOMPANIES") {

		perform("LISTCOMPANIES");

		//printopts='L'
		gosub postproc();

	} else if (mode.field(".", 1) eq "GETTASKS") {

		call securitysubs("GETTASKS." ^ USER0.f(2) ^ "." ^ request_.f(3));
		data_ = ANS;

	} else {
		call mssg("System Error: " ^ (mode.quote()) ^ " invalid request in GENERALPROXY");

	}

/////
exit:
/////
	stop();

	/*;
	//////////
	errorexit:
	//////////
		response='Error: ':response;
		stop;

	//////////////
	errorresponse:
	//////////////
		convert '|' to fm in msg;
		msg=trim2(msg,fm,'F');
		msg=trim2(msg,fm,'B');
		//convert '||' to fm:fm in msg
		swap fm:fm with crlf in msg;
		swap fm with ' ' in msg;
		response='Error: ':msg;
		stop;

	/////////////
	opendatafile:
	/////////////
		open datafile to src.file else;
			msg='The ':quote(datafile ):' file is not available';
			goto errorresponse;
			end;
		open 'DICT',datafile to @dict else;
			msg='The ':quote('DICT.':datafile ):' file is not available';
			goto errorresponse;
			end;
		return 0;
	*/

	return "";
}

subroutine postproc() {
	call proxysubs("GENERAL", mode, stationery);
	return;

	/*;
	//////////////////////
	checkoutputfileexists:
	//////////////////////
		if dir(system<2>)<1> gt 5 then;
			data=system<2>;
			response='OK';

			//make pdf available as well
			if stationery>2 then;
				call convpdf(stationery,errors);
				if errors then msg<-1>=errors;
				end;

			//convert to http path
			data=system<2>;
			tt=index(data,OSSLASH:'data':OSSLASH,1);
			if tt then;
				data='..':OSSLASH:'..':OSSLASH:data[tt,999999];
				system<2>=data;
				end;

			if msg then response:=' ':msg;
			msg='';
		end else;
			response=msg;
			if response='' then;
				response='Error: No output file in GENERALPROXY ':mode;
				call sysmsg(response);
				end;
			end;
		return;
	*/

}

subroutine initlog() {

	logkey = USER0.f(2);
	logyear = request_.f(3);
	var logformat = USER0.f(4);
	var logoptions = request_.f(5);

	if (logoptions.match("^\\d*/\\d{2}$")) {
		logyear = addcent4(logoptions.field("/", 2));
		logoptions = "";
	}

	logfromdate = USER1.f(5);
	loguptodate = data_.f(6);
	logsearch = USER1.f(7);

	if (logoptions eq "TODAY") {
		logfromdate = date();
		loguptodate = logfromdate;
	}

	if (not logyear) {
		var tt = logfromdate;
		var tt2 = loguptodate;
		if (not tt) {
			tt = date();
		}
		if (not tt2) {
			tt2 = tt;
		}
		logyear = tt.oconv("D").last(4);
		var logtoyear = tt2.oconv("D").last(4);
		if (logyear ne logtoyear) {
			response_ = "Dates must be within one calendar year";
			stop();
		}
	}

	if (not(authorised("LOG ACCESS", USER4, ""))) {
		USER3 = msg_;
		stop();
	}

	if (not(authorised("LOG ACCESS " ^ (logkey.quote()), USER4, ""))) {
		response_ = msg_;
		stop();
	}

	return;

	//in get.subs and generalproxy
}

subroutine gettaskprefix() {
	taskprefix = "";
	task = task.field(" ", 1);
	if (task eq "ANAL") {
		taskprefix = "BILLING REPORT";
	} else if (task eq "BALANCES") {
		taskprefix = "FINANCIAL REPORT";
	} else if (task eq "ANALSCH") {
		taskprefix = "BILLING REPORT";
	} else {
		taskprefix = "";
	}
	return;
}

subroutine opendocuments() {
	if (not(sys.documents.open("DOCUMENTS", ""))) {
		call fsmsg();
		stop();
	}
	return;
}

libraryexit()
