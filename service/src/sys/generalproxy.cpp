#include <exodus/library.h>
libraryinit()

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
#include <convpdf.h>
#include <addcent4.h>

#include <gen_common.h>
#include <win_common.h>

var stationery;
var mode;
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
var errors;

function main() {
	//!subroutine general(request,data,response)
	//c sys
	//global task

	#define request USER0
	#define data USER1
	#define response USER3
	#define msg USER4

	#include <general_common.h>
	//global fn,stationery,mode

	//clearcommon();
	win.valid = 1;
	USER4 = "";
	stationery = "";

	call cropper(USER0);
	call cropper(USER1);
	mode = USER0.a(1).ucase();

	//request 2 - can be anything actually

	win.datafile = USER0.a(2);
	var keyx = USER0.a(3);

	USER3 = "OK";

	if (mode eq "TEST") {

	} else if (mode eq "PERIODTABLE") {

		var year = USER0.a(2).field("-", 1).field("/", 2);
		var finyear = USER0.a(3);

		perform("PERIODTABLE " ^ year ^ " " ^ finyear ^ " (H)");

		gosub checkoutputfileexists();

	} else if (mode eq "FILEMAN" and USER0.a(2) eq "COPYDB") {

		var copydb = USER0.a(3);
		if (not(SYSTEM.a(58).locate(copydb,dbn))) {
			{}
		}
		var todb = SYSTEM.a(63, dbn);
		if (not todb) {
			call mssg("\"Copy to\" database must be configured (and saved) for database " ^ copydb ^ " first");
			stop();
		}

		//ensure authorised to login to one or the other database
		//by ensuring the user is currently logged in to one or other database
		if ((USERNAME ne "EXODUS" and copydb ne SYSTEM.a(17)) and todb ne SYSTEM.a(17)) {
			USER4 = "In order to copy database " ^ (copydb.quote()) ^ " to " ^ (todb.quote()) ^ ",";
			USER4.r(-1, "you must be logged in to database " ^ (copydb.quote()) ^ " or " ^ (todb.quote()));
			USER4.r(-1, "but you are currently logged in to database " ^ (SYSTEM.a(17).quote()));
			call mssg(USER4);
			stop();
		}

		//should really have an option to close the live dataset and then copy
		if (not(authorised("DATASET COPY", USER4, "LS"))) {
			call mssg(USER4);
			stop();
		}

		var started = var().time().oconv("MTS");
		var otherusersx = otherusers(copydb);
		var log = started ^ " Started copy database " ^ copydb ^ " to " ^ todb;
		log ^= "|" ^ started ^ " Other processes online:" ^ otherusersx.a(1);

		perform("COPYDB " ^ copydb ^ " " ^ todb);

		USER3 = USER4;
		if (not USER3) {
			log ^= "|" ^ var().time().oconv("MTS") ^ " Finished";
			call sysmsg(log);
			USER3 = "OK " ^ log;
		}

	} else if (mode eq "EMAILUSERS") {

		var groupids = USER1.a(1);
		var jobfunctionids = "";
		var userids = USER1.a(2);
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

		var subject = USER1.a(4);
		var message = USER1.a(5);

		message.converter(TM, VM);
		call emailusers("", subject, message, groupids, jobfunctionids, userids, options, emailresult);

		if (emailresult) {
			emailresult.converter(VM ^ FM, "\r\r");
			USER3 = "OK Email Sent to" "\r" ^ emailresult;
		}else{
			call mssg("No users can be found to email,|or some problem with email server");
		}
		USER1 = "";

	} else if (mode eq "VAL.EMAIL") {
		win.is = USER0;
		call usersubs(mode);

	//case mode[-3,3]='SSH'
	// call ssh(mode)

	} else if (mode eq "PASSWORDRESET") {

		if (not(authorised("PASSWORD RESET", USER4))) {
			call mssg(USER4);
			stop();
		}

		var users;
		if (not(users.open("USERS", ""))) {
			call mssg("USERS file is missing");
			stop();
		}

		ID = USER0.a(2);
		var emailaddress = USER0.a(3);

		var baduseroremail = "Either " ^ (ID.quote()) ^ " or " ^ (emailaddress.quote()) ^ " does not exist";

		var userrec;
		if (userrec.read(users, ID)) {
			usersordefinitions = users;
			userkey = ID;

			if (emailaddress.ucase() eq userx.a(7).ucase()) {

				//signify ok
				baduseroremail = "";

				call securitysubs("GENERATEPASSWORD");
				newpassword = ANS;
				userrec.r(4, newpassword);

			}

		}else{
			usersordefinitions = DEFINITIONS;
			userkey = "BADUSER*" ^ ID;
			if (not(userrec.read(usersordefinitions, userkey))) {
				userrec = "";
			}
		}

		//record historical resets/attempts
		//datetime=(date():'.':time() 'R(0)#5')+0
		var datetime = var().date() ^ "." ^ var().time().oconv("R(0)#5");
		userrec.inserter(15, 1, datetime);
		userrec.inserter(16, 1, SYSTEM.a(40, 2));
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
		var emailaddrs = userrec.a(7);
		var ccaddrs = "";
		var subject = "EXODUS Password Reset";
		var body = "User: " ^ ID;
		body.r(-1, "Your new password is " ^ newpassword);
		call sendmail(emailaddrs, ccaddrs, subject, body, "", "", xx);

	} else if (mode eq "MAKEUPLOADPATH") {
		call uploadsubs("MAKEUPLOADPATH." ^ USER0.a(2));

	} else if (mode eq "POSTUPLOAD") {
		call uploadsubs("POSTUPLOAD");

	} else if (mode eq "VERIFYUPLOAD") {
		call uploadsubs("VERIFYUPLOAD." ^ USER0.a(2));

	} else if (mode eq "OPENUPLOAD") {
		call uploadsubs("OPENUPLOAD." ^ USER0.a(2));

	} else if (mode eq "DELETEUPLOAD") {
		call uploadsubs("DELETEUPLOAD." ^ USER0.a(2));

	} else if (mode eq "GETCODEPAGE") {
		USER1 = "";
		//have to skip char zero it seems to be treated as string terminator
		//somewhere on the way to the browser (not in revelation)
		for (var ii = 1; ii <= 255; ++ii) {
			USER1 ^= var().chr(ii);
		};//ii;
		//data='xxx'
		USER3 = "OK";

	} else if (mode eq "SETCODEPAGE") {
		if (not(gen.alanguage.open("ALANGUAGE", ""))) {
			call fsmsg();
			stop();
		}

		var codepage = USER0.a(3);
		if (not codepage) {
			goto badsetcodepage;
		}

		if (USER0.a(2) eq "SORTORDER") {

			/* should be inverted but bother since cant get arev only collated ascii;
				if len(data) ne 254 then;
					call msg('SORTORDER data is ':len(data):' but must be 254 characters long|(initial char 0 excluded)');
					stop;
					end;
				write char(0):data on alanguage,'SORTORDER*':codepage;
			*/

		} else if (USER0.a(2) eq "UPPERCASE") {
			fn = 9;
setcodepagecase:
			var recordx;
			if (not(recordx.read(gen.alanguage, "GENERAL*" ^ codepage))) {
				if (not(recordx.read(gen.alanguage, "GENERAL"))) {
					recordx = "";
				}
			}
			var temp = USER1;
			temp.swapper(RM, "%FF");
			temp.swapper(FM, "%FE");
			temp.swapper(VM, "%FD");
			recordx.r(1, fn, temp);

			recordx.write(gen.alanguage, "GENERAL*" ^ codepage);

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

		} else if (USER0.a(2) eq "LOWERCASE") {
			fn = 10;
			goto setcodepagecase;

		} else {
badsetcodepage:
			call mssg("Request must be SETCODEPAGE SORTORDER|UPPERCASE|LOWERCASE codepage");
			return 0;
		}

		USER3 = "OK";

	} else if (mode eq "GETDATASETS") {
		ANS = "";
		call generalsubs("GETDATASETS");
		USER1 = ANS;
		if (USER1) {
			USER3 = "OK";
		}else{
			USER3 = "Error: No datasets found";
		}

	} else if (mode eq "LISTPROCESSES") {

		perform("SORT PROCESSES");

		gosub checkoutputfileexists();

	} else if (mode eq "LISTREQUESTLOG") {

		PSEUDO = USER1;
		perform("LISTREQUESTLOG");

		//printopts='L'
		gosub checkoutputfileexists();

	} else if (mode eq "LISTLOCKS") {

		perform("SORT LOCKS WITH NO LOCK_EXPIRED");

		gosub checkoutputfileexists();

	} else if (mode eq "GETVERSIONDATES") {

		call changelogsubs("GETVERSIONDATES");

	} else if (mode.ucase() eq "WHATSNEW") {

		call changelogsubs("SELECTANDLIST" ^ FM ^ USER1);
		if (USER4) {
			USER4.transfer(USER3);
			stop();
		}

		//printopts='L'
		gosub checkoutputfileexists();

	} else if (mode eq "LISTADDRESSES") {

		perform("LISTADDRESSES");

		//printopts='L'
		gosub checkoutputfileexists();

	} else if (mode eq "GETDEPTS") {
		call usersubs("GETDEPTS");
		USER1 = ANS;
		USER3 = "OK";

	} else if (mode eq "LISTACTIVITIES") {

		perform("LISTACTIVITIES " ^ USER0.a(2));

		gosub checkoutputfileexists();

	} else if (mode eq "ABOUT") {
		perform("ABOUT");
		//transfer @user4 to data
		//response='OK'
		USER4.transfer(USER3);
		USER3.splicer(1, 0, "OK ");

	} else if (mode eq "UTIL") {
		perform("UTIL");
		USER3 = "OK";

	} else if (mode eq "PROG") {
		perform("PROG");
		USER3 = "OK";

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

		gosub checkoutputfileexists();

	} else if (mode eq "READUSERS") {

		win.templatex = "SECURITY";
		call securitysubs("SETUP");
		if (not(win.valid)) {
			stop();
		}

		USER1 = RECORD;
		USER3 = "OK";

	} else if (mode eq "GETREPORTS") {

		task = USER0.a(2);
		gosub gettaskprefix();
		if (taskprefix) {
			if (not(authorised(taskprefix ^ " ACCESS", USER4, ""))) {
				call mssg(USER4);
				stop();
			}
		}

		var select = "SELECT DOCUMENTS BY-DSND EXODUS_STANDARD BY DESCRIPTION";

		var instructions = USER0.a(2);
		instructions.swapper(VM, "%FD");
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
			if (report.read(gen.documents, reportid)) {
				repn += 1;

				var temp = report.a(2);
				temp.swapper("&", "&amp;");
				temp.swapper("<", "&lt;");
				temp.swapper(">", "&gt;");
				report.r(2, temp);

				//!dont send instructions since takes up space and not needed
				//DO send now to have info in requestlog
				//report<5>=''

				report.converter(VM, RM);
				var nn = report.count(FM) + 1;
				for (var ii = 1; ii <= nn; ++ii) {
					USER1.r(ii, repn, report.a(ii));
				};//ii;

				USER1.r(9, repn, reportid);

	//print repn,data<1>
			}
			//if len(data)<65000 then goto nextrep
			if (USER1.length() lt maxstrsize - 530) {
				goto nextrep;
			}
		}
		USER3 = "OK";

	} else if (mode eq "DELETEREPORT") {

		gosub opendocuments();

		var docnos = USER0.a(2);

		var ndocs = docnos.count(VM) + 1;
		for (var docn = 1; docn <= ndocs; ++docn) {
			ID = docnos.a(1, docn);
			if (ID) {
				if (RECORD.read(gen.documents, ID)) {
					win.orec = RECORD;
					call getsubs("PREDELETE");
					if (not(win.valid)) {
						goto exit;
					}

					if (win.valid) {
						gen.documents.deleterecord(ID);

						call getsubs("POSTDELETE");
					}
				}
			}
		};//docn;

	} else if (mode eq "UPDATEREPORT") {

		gosub opendocuments();

		var doc;
		if (not(doc.read(gen.documents, USER0.a(2)))) {
			call mssg("Document " ^ (USER0.a(2).quote()) ^ " is missing");
			stop();
		}

		//TODO security

		doc.r(6, lower(USER1));

		doc.write(gen.documents, USER0.a(2));

	} else if (mode eq "COPYREPORT") {

		gosub opendocuments();

		var doc;
		if (not(doc.read(gen.documents, USER0.a(2)))) {
			call mssg("Document " ^ (USER0.a(2).quote()) ^ " is missing");
			stop();
		}

		task = doc.a(5);
		gosub gettaskprefix();
		if (taskprefix) {
			if (not(authorised(taskprefix ^ " CREATE", USER4, ""))) {
				call mssg(USER4);
				stop();
			}
		}

		call getsubs("DEF.DOCUMENT.NO");
		if (not(win.valid)) {
			stop();
		}

		var description = doc.a(2);
		doc.r(2, description ^ " (Copy)");

		//prevent copy from appearing like a exodus standard
		doc.r(10, "");

		doc.r(1, USERNAME);
		doc.write(gen.documents, ID);

		USER1 = ID ^ FM ^ doc;

		USER3 = "OK";

	} else if (mode eq "GETREPORT") {

		//printopts='L'
		gosub opendocuments();

		//get parameters from documents into @pseudo

		if (not(gen.document.read(gen.documents, USER0.a(2)))) {
			USER4 = "Document " ^ (USER0.a(2).quote()) ^ " does not exist";
			call mssg(USER4);
			stop();
		}

		task = gen.document.a(5);
		gosub gettaskprefix();
		if (taskprefix) {
			if (not(authorised(taskprefix ^ " ACCESS", USER4, ""))) {
				call mssg(USER4);
				stop();
			}
		}

		//if task='BALANCES' then printopts='P'
		//if index(task,'MEDIADIARY',1) then printopts='X'

		PSEUDO = gen.document.a(6);
		PSEUDO.converter(RM, VM);
		PSEUDO = raise(PSEUDO);

		//merge any runtime parameters into the real parameters
		for (fn = 1; fn <= 999; ++fn) {
			var tt = USER1.a(fn);
			if (tt) {
				PSEUDO.r(fn, tt);
			}
		};//fn;
		USER1 = "";

		//save runtime params in case saving below for scheduled reruns
		//document<11>=lower(data)

		var sentencex = gen.document.a(5);
		sentencex.converter(VM, " ");
		USER1 = PSEUDO;

		//in case we are calling another proxy
		if (gen.document.a(5, 1).substr(-5,5) eq "PROXY") {

			//run but suppress email
			//perform 'TEST ':request<2>:' (S)'

			USER0 = raise(gen.document.a(5)).field(FM, 2, 999999) ^ FM ^ USER0.a(2);
			//moved up so parameters show in any emailed error messages
			//data=@pseudo
			//override the saved period with a current period
			var runtimeperiod = var().date().oconv("D2/E").substr(4,5);
			if (runtimeperiod[1] eq "0") {
				runtimeperiod.splicer(1, 1, "");
			}
			USER1.swapper("{RUNTIME_PERIOD}", runtimeperiod);
			goto performreport;

		}else{

performreport:
			USER3 = "";

			perform(sentencex);
			if (not USER3) {

				gosub checkoutputfileexists();
			}

		}

	} else if (mode eq "USAGESTATISTICS") {

		PSEUDO = USER1;
		perform("LISTSTATS");

		//printopts='L'
		gosub checkoutputfileexists();

	} else if (mode eq "VIEWLOG") {

		gosub initlog();
		var datedict = "LOG_DATE";

		var cmd = "LIST LOG" ^ logyear;
		cmd ^= " BY LOG_DATE BY LOG_TIME";
		cmd ^= " LOG_DATE LOG_TIME LOG_USERNAME LOG_WORKSTATION LOG_SOURCE LOG_MESSAGE2";
		if (logfromdate) {
			if (loguptodate) {
				cmd ^= "  xAND WITH LOG_DATE BETWEEN " ^ (logfromdate.oconv("D4").quote()) ^ " AND " ^ (loguptodate.oconv("D4").quote());
			}else{
				cmd ^= "  xAND WITH " ^ datedict ^ " >= " ^ (logfromdate.oconv("D4").quote());
			}
		}
		if (logkey) {
			cmd ^= " xAND WITH LOG_SOURCE1 STARTING " ^ (logkey.quote());
		}
		if (logsearch) {
			cmd ^= " xAND WITH ALLUPPERCASE [] " ^ (logsearch.quote());
		}
		cmd ^= " ID-SUPP";
		var temp = cmd.index("xAND");
		if (temp) {
			cmd.splicer(temp, 5, "");
		}
		cmd.swapper("xAND", "AND");
		perform(cmd);

		//printopts='L'
		gosub checkoutputfileexists();

	} else if (mode eq "LISTMARKETS") {

		var cmd = "SORT MARKETS WITH AUTHORISED BY SEQ";
		cmd ^= " HEADING " ^ (var("List of Markets     'T'     Page 'PL'").quote());
		perform(cmd);

		gosub checkoutputfileexists();

	} else if (mode eq "LISTCURRENCIES") {

		perform("LISTCURRENCIES " ^ USER0.a(2));

		gosub checkoutputfileexists();

	} else if (mode eq "LISTCOMPANIES") {

		perform("LISTCOMPANIES");

		//printopts='L'
		gosub checkoutputfileexists();

	} else if (mode.field(".", 1) eq "GETTASKS") {

		call securitysubs("GETTASKS." ^ USER0.a(2) ^ "." ^ USER0.a(3));
		USER1 = ANS;

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

subroutine checkoutputfileexists() {
	if (SYSTEM.a(2).osfile().a(1) gt 5) {
		USER1 = SYSTEM.a(2);
		USER3 = "OK";

		//make pdf available as well
		if (stationery gt 2) {
			call convpdf(stationery, errors);
			if (errors) {
				USER4.r(-1, errors);
			}
		}

		//convert to http path
		USER1 = SYSTEM.a(2);
		var tt = USER1.index(OSSLASH "data" OSSLASH);
		if (tt) {
			USER1 = ".." OSSLASH ".." OSSLASH ^ USER1.substr(tt,999999);
			SYSTEM.r(2, USER1);
		}

		if (USER4) {
			USER3 ^= " " ^ USER4;
		}
		USER4 = "";
	}else{
		USER3 = USER4;
		if (USER3 eq "") {
			USER3 = "Error: No output file in GENERALPROXY " ^ mode;
			call sysmsg(USER3);
		}
	}
	return;
}

subroutine initlog() {

	logkey = USER0.a(2);
	logyear = USER0.a(3);
	var logformat = USER0.a(4);
	var logoptions = USER0.a(5);

	if (logoptions.match("^\\d*/\\d{2}$")) {
		logyear = addcent4(logoptions.field("/", 2));
		logoptions = "";
	}

	logfromdate = USER1.a(5);
	loguptodate = USER1.a(6);
	logsearch = USER1.a(7);

	if (logoptions eq "TODAY") {
		logfromdate = var().date();
		loguptodate = logfromdate;
	}

	if (not logyear) {
		var tt = logfromdate;
		var tt2 = loguptodate;
		if (not tt) {
			tt = var().date();
		}
		if (not tt2) {
			tt2 = tt;
		}
		logyear = tt.oconv("D").substr(-4,4);
		var logtoyear = tt2.oconv("D").substr(-4,4);
		if (logyear ne logtoyear) {
			USER3 = "Dates must be within one calendar year";
			stop();
		}
	}

	if (not(authorised("LOG ACCESS", USER4, ""))) {
		USER3 = USER4;
		stop();
	}

	if (not(authorised("LOG ACCESS " ^ (logkey.quote()), USER4, ""))) {
		USER3 = USER4;
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
	if (not(gen.documents.open("DOCUMENTS", ""))) {
		call fsmsg();
		stop();
	}
	return;
}

libraryexit()
