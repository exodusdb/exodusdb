#include <exodus/library.h>
#include <srv_common.h>
#include <req_common.h>

// executed by listen to handle EXECUTE commands from web clients

#include <req_common.h>

libraryinit()

#include <addcent4.h>
#include <authorised.h>
#include <changelogsubs.h>
#include <emailusers.h>
#include <generalsubs.h>
#include <gethtml.h>
#include <getsubs.h>
#include <otherusers.h>
#include <proxysubs.h>
#include <safeselect.h>
#include <securitysubs.h>
#include <sendmail.h>
#include <sysmsg.h>
#include <uploadsubs.h>
#include <usersubs.h>

#include <service_common.h>

#include <request.hpp>

var stationery;
var mode;
var html;
var dbn;
var emailresult;
var usersordefinitions;
var userkey;
var userx;
var newpassword;
//var xx;
//var fn;	 // num
var task;
var taskprefix;
var reportid;
var logyear;
var logfromdate;
var loguptodate;
var logkey;
var logsearch;

function main() {
	// !subroutine general(request,data,response)

	// use app specific version of generalsubs
	if (APPLICATION != "EXODUS") {
		generalsubs = "generalsubs_app";
	}

	req.valid  = 1;
	msg_	   = "";
	stationery = "";

	call cropper(request_);
	call cropper(data_);
	mode = request_.f(1).ucase();

	// request 2 - can be anything actually

	req.datafile = request_.f(2);
	let keyx	 = request_.f(3);

	response_ = "OK";

	if (mode == "TEST") {

	} else if (mode == "TRANTEST") {

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
		if (not open(transactions, transactions))
			//createfile(transactions);
			if (not createfile(transactions))
				abort(lasterror());

		// ERROR:  VACUUM cannot run inside a transaction block sqlstate:25001
		// if (not transactions.sqlexec("vacuum " ^ transactions.f(1))) {
		// 	response_ = var().lasterror();
		// 	stop();
		// }

		ID		   = "1";
		let ntrans = 100;
		// printl();
		// printl("TRANTEST 1.", THREADNO);
		for (const var recn : range(1, ntrans)) {
			// if (TERMINAL and recn % 100)
			// 	printl(recn);
			if (not read(RECORD, transactions, ID))
				RECORD = "";
			write(RECORD + 1, transactions, ID);
		}
		// printl();
		// printl("TRANTEST 2.", THREADNO);
		response_ = "OK Thread No. " ^ THREADNO ^ ", Total: " ^ (RECORD + 1);

	} else if (mode.f(1) == "PREVIEWLETTERHEAD") {

		// comma sep
		let compcodes = request_.f(2);
		let testing	  = request_.f(3);

		var allhtml = "";

		// if testing, style borders of td and divs for visual insight
		if (testing) {
			allhtml(-1) = "<style>";
			allhtml(-1) = "td {border:dotted 1px #EEEEEE;}";
			allhtml(-1) = "div {border:dashed 1px lightgrey;}";
			allhtml(-1) = "</style>";
		}

		let ncomps = compcodes.fcount(",");
		for (const var compn : range(1, ncomps)) {

			mode		  = "HEAD";
			var	 compcode = compcodes.field(",", compn);
			call gethtml(mode, html, compcode);

			allhtml(-1) = "<br />Company " ^ compcode ^ " from " ^ mode;
			allhtml(-1) = "<br />";

			// if not testing then wrap html in hr for clarity
			// if testing else allhtml<-1>='<hr/>'
			allhtml(-1) = html;
			// if testing else allhtml<-1>='<hr/>'

		}  // compn;
		allhtml(-1) = "Press F5 to refresh any images just uploaded.";
		allhtml.replacer(_FM, _EOL);
		//var(allhtml).oswrite(SYSTEM.f(2));
		if (not var(allhtml).oswrite(SYSTEM.f(2)))
			abort(lasterror());
		gosub postproc();

	} else if (mode == "PERIODTABLE") {

		let year	= request_.f(2).field("-", 1).field("/", 2);
		let finyear = request_.f(3);

		perform("PERIODTABLE " ^ year ^ " " ^ finyear ^ " (H)");

		gosub postproc();

	} else if (mode == "FILEMAN" and request_.f(2) == "COPYDB") {

		let copydb = request_.f(3);
		if (not SYSTEM.f(58).locate(copydb, dbn)) {
			// Not found. dbn points to last + 1
		}
		let todb = SYSTEM.f(63, dbn);
		if (not todb) {
			abort("\"Copy to\" database must be configured (and saved) for database " ^ copydb ^ " first");
		}

		// ensure authorised to login to one or the other database
		// by ensuring the user is currently logged in to one or other database
		if ((USERNAME != "EXODUS" and copydb != SYSTEM.f(17)) and todb != SYSTEM.f(17)) {
			msg_	 = "In order to copy database " ^ (copydb.quote()) ^ " to " ^ (todb.quote()) ^ ",";
			msg_(-1) = "you must be logged in to database " ^ (copydb.quote()) ^ " or " ^ (todb.quote());
			msg_(-1) = "but you are currently logged in to database " ^ (SYSTEM.f(17).quote());
			abort(msg_);
		}

		// should really have an option to close the live dataset and then copy
		if (not authorised("DATASET COPY", msg_, "LS")) {
			abort(msg_);
		}

		let started		= time().oconv("MTS");
		let otherusersx = otherusers(copydb);
		var log			= started ^ " Started copy database " ^ copydb ^ " to " ^ todb;
		log ^= "|" ^ started ^ " Other processes online:" ^ otherusersx.f(1);

		perform("COPYDB " ^ copydb ^ " " ^ todb);

		response_ = msg_;
		if (not response_) {
			log ^= "|" ^ time().oconv("MTS") ^ " Finished";
			call sysmsg(log);
			response_ = "OK " ^ log;
		}

	} else if (mode == "EMAILUSERS") {

		let groupids	   = data_.f(1);
		let jobfunctionids = "";
		let userids		   = data_.f(2);
		if (not((groupids or jobfunctionids) or userids)) {
			abort("You must specify some groups or users to email");
		}

		// ensure sender has an email address
		// not absolutely necessary but provides a return email address
		if (USERNAME != "EXODUS" and not(USERNAME.xlate("USERS", 7, "X"))) {
			abort("You cannot send email because you do not have an email address for replies");
		}

		// T=
		// R=reply to is current user
		// W=group is word to be found in user department
		let options = "TR";

		let subject = data_.f(4);
		var message = data_.f(5);

		message.converter(_TM, _VM);
		call emailusers("", subject, message, groupids, jobfunctionids, userids, options, emailresult);

		if (emailresult) {
			emailresult.converter(_VM _FM, "\r\r");
			response_ =
				"OK Email Sent to"
				"\r" ^
				emailresult;
		} else {
			call note("No users can be found to email,|or some problem with email server");
		}
		data_ = "";

	} else if (mode == "CREATEDATABASE") {
		// For patsalides

		if (not authorised("DATASET CREATE", msg_)) {
			abort(msg_);
		}

		let targetdbname = data_.f(1);
		let targetdbcode = data_.f(2).lcase();
		let sourcedbcode = data_.f(3).lcase();

		let sourcedatadir = "../data/" ^ sourcedbcode;
		let targetdatadir = "../data/" ^ targetdbcode;

		// source database services will be stopped for source to target database copy
		// reject source db if same as current database
		let currentdbcode = SYSTEM.f(17);
		if (sourcedbcode == currentdbcode) {
			//return invalid("The old database and the database you are curently logged into cannot be the same.\nLogin to a different database and try again.");
			abort("The old database and the database you are curently logged into cannot be the same.\nLogin to a different database and try again.");
		}

		let dbcodes = dblist();

		// check source database exists
		if (not locateusing(_FM, sourcedbcode, dbcodes)) {
			//return invalid("Source database doesn't exists.");
			abort(sourcedbcode ^ " is not in list of databases");
		}

		// verify new database doesnt exist
		if (locateusing(_FM, targetdbcode, dbcodes)) {
			//return invalid("Sorry, " ^ targetdbcode.quote() ^ " already exists");
			abort(targetdbcode ^ " already exists in list of databases");
		}

		// get service name eg agy_live@XXXX
		var liveortest = "test";
		if (not sourcedbcode.ends("_test")) {
			liveortest = "live";
		}
		let srcservicecode = "agy_" ^ liveortest ^ "@" ^ sourcedbcode.replace("_test", "");

		// stop source db live/test service
		//if (not osshell("systemctl stop agy_live@" ^ sourcedbcode)) {
		if (not osshell("service " ^ srcservicecode ^ " stop")) {
			//return invalid("Cannot stop " ^ sourcedbcode ^ "'s service");
			abort(lasterror());
		}

		// Copy source db to target db
		// ERROR:  CREATE DATABASE cannot run inside a transaction block
		// if (not dbcopy(sourcedbcode,targetdbcode)) {
		//osshell("dbcopy " ^ sourcedbcode ^ " " ^ targetdbcode);
		// check target db created
		//var newdbcodes = dblist();
		//if (not osshell("dbcopy " ^ sourcedbcode ^ " " ^ targetdbcode)) {
		//	return invalid("Failed to create new database: " ^ targetdbcode.quote() ^ " - " ^ targetdbname);
		//}
		if (not osshell("dbcopy " ^ sourcedbcode ^ " " ^ targetdbcode)) {
			// start source database service before aborting with dbcopy error
			let dbcopyfailmsg = lasterror();
			if (not osshell("service " ^ srcservicecode ^ " start")) {
				abort(dbcopyfailmsg ^ "\n" ^ lasterror());
			}

			abort(dbcopyfailmsg);
		}

		// start source live service
		if (not osshell("service " ^ srcservicecode ^ " start")) {
			abort(lasterror());
		}

		// Setup target database data dir.
		// Target image dir created automatically on first upload
		// oscopy(sourcedatadir, targetdatadir);
		// KEEP IN SYNC. SIMILAR code in create_site, create_service and generalproxy.cpp
		if (not osshell("mkdir -p " ^ targetdatadir))
			//return invalid(lasterror());
			abort(lasterror());
		if (not osshell("chmod a+srw " ^ targetdatadir))
			//return invalid(lasterror());
			abort(lasterror());
		if (not osshell("setfacl -d -m g::rw " ^ targetdatadir))
			//return invalid(lasterror());
			abort(lasterror());

        // TODO create service - not req for ptcy running service auto creates new services

		// update database name file
		if (not oswrite(targetdbname, "../data/" ^ targetdbcode ^ "/name")) {
			abort(lasterror());
		}

		// email confirmaion
		call sysmsg("", targetdbname ^ " - (" ^ targetdbcode ^ ") created");

		response_ = "OK Database " ^ targetdbname ^ " " ^ targetdbcode.quote() ^ " has been created.";

	} else if (mode == "VAL.EMAIL") {
		req.is = request_;
		call usersubs(mode);

		// case mode[-3,3]='SSH'
		// call ssh(mode)

	} else if (mode == "PASSWORDRESET") {

		if (not authorised("PASSWORD RESET", msg_)) {
			abort(msg_);
		}

		var users;
		if (not users.open("USERS", "")) {
			abort("USERS file is missing");
		}

		ID				 = request_.f(2);
		let emailaddress = request_.f(3);

		var baduseroremail = "Either " ^ (ID.quote()) ^ " or " ^ (emailaddress.quote()) ^ " does not exist";

		var userrec;
		if (userrec.read(users, ID)) {
			usersordefinitions = users;
			userkey			   = ID;

			if (emailaddress.ucase() == userx.f(7).ucase()) {

				// signify ok
				baduseroremail = "";

				call securitysubs("GENERATEPASSWORD");
				newpassword = ANS;
				userrec(4)	= newpassword;
			}

		} else {
			usersordefinitions = DEFINITIONS;
			userkey			   = "BADUSER*" ^ ID;
			if (not userrec.read(usersordefinitions, userkey)) {
				userrec = "";
			}
		}

		// record historical resets/attempts
		// datetime=(date():'.':time() 'R(0)#5')+0
		let datetime = date() ^ "." ^ time().oconv("R(0)#5");
		userrec.inserter(15, 1, datetime);
		userrec.inserter(16, 1, SYSTEM.f(40, 2));
		userrec.inserter(18, 1, ("Password Reset " ^ baduseroremail).trim());

		if (baduseroremail) {
			userrec.write(usersordefinitions, userkey);
			abort(baduseroremail);
		}

		// prewrite (locks authorisation file or fails)
		req.valid = 1;
		call usersubs("PREWRITE");
		if (not req.valid) {
			abort();
		}

		userrec.write(usersordefinitions, userkey);

		// postwrite
		call usersubs("POSTWRITE");

		// cc the user too if the username is valid
		// call sysmsg('User: ':username:fm:'From IP: ':system<40,2>:fm:text,'Password Reset',userkey)

		// send the new password to the user
		let emailaddrs = userrec.f(7);
		let ccaddrs	   = "";
		let subject	   = "EXODUS Password Reset";
		var body	   = "User: " ^ ID;
		body(-1)	   = "Your new password is " ^ newpassword;
		call sendmail(emailaddrs, ccaddrs, subject, body, "", "");

	} else if (mode == "MAKEUPLOADPATH") {
		call uploadsubs("MAKEUPLOADPATH." ^ request_.f(2));

	} else if (mode == "POSTUPLOAD") {
		call uploadsubs("POSTUPLOAD");

	} else if (mode == "VERIFYUPLOAD") {
		call uploadsubs("VERIFYUPLOAD." ^ request_.f(2));

	} else if (mode == "OPENUPLOAD") {
		call uploadsubs("OPENUPLOAD." ^ request_.f(2));

	} else if (mode == "DELETEUPLOAD") {
		call uploadsubs("DELETEUPLOAD." ^ request_.f(2));

	} else if (mode == "GETCODEPAGE") {
		data_ = "";
		// have to skip char zero it seems to be treated as string terminator
		// somewhere on the way to the browser (not in revelation)
		for (const var ii : range(1, 255)) {
			data_ ^= chr(ii);
		}  // ii;
		// data='xxx'
		response_ = "OK";

	} else if (mode == "SETCODEPAGE") {
		if (not srv.alanguage.open("ALANGUAGE", "")) {
			abort(lasterror());
		}

		let codepage = request_.f(3);
		if (not codepage) {
			goto badsetcodepage;
		}

		if (request_.f(2) == "SORTORDER") {

			/* should be inverted but bother since cant get DOS only collated ascii;
				if len(data) != 254 then;
					call msg('SORTORDER data is ':len(data):' but must be 254 characters long|(initial char 0 excluded)');
					stop;
					end;
				write char(0):data on alanguage,'SORTORDER*':codepage;
			*/

		} else if (request_.f(2) == "UPPERCASE" or request_.f(2) == "LOWERCASE") {

			var recordx;
			if (not recordx.read(srv.alanguage, "GENERAL*" ^ codepage)) {
				if (not recordx.read(srv.alanguage, "GENERAL")) {
					recordx = "";
				}
			}
			var temp = data_;
			temp.replacer(_RM, "%FF");
			temp.replacer(_FM, "%FE");
			temp.replacer(_VM, "%FD");

			let fn = request_.f(2) == "UPPERCASE" ? 9 : 10;
			recordx(1, fn) = temp;

			recordx.write(srv.alanguage, "GENERAL*" ^ codepage);

//			// check lower=lcase(upper) and vice versa
//			// but ucase() may strip accents whereas lcase() may leave them
//			x=@lower.case;
//			y=@upper.case;
//			convert @lower.case to @upper.case in x;
//			convert @upper.case to @lower.case in y;
//			convert @lower.case to @upper.case in y;
//			convert @upper.case to @lower.case in x;
//			for ii=1 to len(x);
//				// if x[ii,1] != @lower.case[ii,1] then print 'x: ':ii:' ':seq(x[ii,1]) 'MX':' ':seq(@lower.case[ii,1]) 'MX'
//				if y[ii,1] != @upper.case[ii,1] then print 'y: ':ii:' ':seq(y[ii,1]) 'MX':' ':seq(@upper.case[ii,1]) 'MX';
//				next ii;

		} else {
badsetcodepage:
			call note("Request must be SETCODEPAGE SORTORDER|UPPERCASE|LOWERCASE codepage");
			return 0;
		}

		response_ = "OK";

	} else if (mode == "GETDATASETS") {
		ANS = "";
		call generalsubs("GETDATASETS");
		data_ = ANS;
		if (data_) {
			response_ = "OK";
		} else {
			response_ = "Error: No datasets found";
		}

	} else if (mode == "LISTPROCESSES") {

		perform("SORT PROCESSES");

		gosub postproc();

	} else if (mode == "LISTREQUESTLOG") {

		PSEUDO = data_;
		perform("LISTREQUESTLOG");

		// printopts='L'
		gosub postproc();

	} else if (mode == "LISTLOCKS") {

		perform("SORT LOCKS WITH NO LOCK_EXPIRED");

		gosub postproc();

	} else if (mode == "GETVERSIONDATES") {

		call changelogsubs("GETVERSIONDATES");

	} else if (mode.ucase() == "WHATSNEW") {

		call changelogsubs("SELECTANDLIST" ^ FM ^ data_);
		if (msg_) {
			msg_.move(response_);
			stop();
		}

		// printopts='L'
		gosub postproc();

	} else if (mode == "LISTADDRESSES") {

		perform("LISTADDRESSES");

		// printopts='L'
		gosub postproc();

	} else if (mode == "GETDEPTS") {
		call usersubs("GETDEPTS");
		data_	  = ANS;
		response_ = "OK";

	} else if (mode == "LISTACTIVITIES") {

		perform("LISTACTIVITIES " ^ request_.f(2));

		gosub postproc();

	} else if (mode == "ABOUT") {
		perform("ABOUT");
		// transfer @user4 to data
		// response='OK'
		msg_.move(response_);
		response_.prefixer("OK ");

	} else if (mode == "UTIL") {
		perform("UTIL");
		response_ = "OK";

	} else if (mode == "PROG") {
		perform("PROG");
		response_ = "OK";

		// LISTAUTH.TASKS = list of tasks LISTTASKS
		// LISTAUTH.USERS = list of users LISTUSERS
	} else if (mode.field(".", 1) == "LISTAUTH") {

		req.wlocked	  = 0;
		req.templatex = "SECURITY";
		call securitysubs("SETUP");
		if (not req.valid) {
			abort();
		}
		// call security.subs('LISTAUTH')
		call securitysubs(mode);
		if (not req.valid) {
			abort();
		}
		call securitysubs("POSTAPP");

		gosub postproc();

	} else if (mode == "READUSERS") {

		req.templatex = "SECURITY";
		call securitysubs("SETUP");
		if (not req.valid) {
			abort();
		}

		data_	  = RECORD;
		response_ = "OK";

	} else if (mode == "GETREPORTS") {

		task = request_.f(2);
		gosub gettaskprefix();
		if (taskprefix) {
			if (not authorised(taskprefix ^ " ACCESS", msg_, "")) {
				abort(msg_);
			}
		}

		var select = "SELECT DOCUMENTS BY-DSND EXODUS_STANDARD BY DESCRIPTION";

		var instructions = request_.f(2);
		instructions.replacer(_VM, "%FD");
		select ^= " WITH INSTRUCTIONS2 " ^ (instructions.quote());

		var dummy;
		if (not authorised("DOCUMENTS: ACCESS OTHER PEOPLES DOCUMENTS", dummy, "")) {
			select ^= " AND WITH CREATEDBY " ^ (USERNAME.quote());
		}

		call safeselect(select ^ " (S)");

		gosub opendocuments();

		data_	 = "";
		var repn = 0;
nextrep:
		if (readnext(reportid)) {
			var report;
			if (report.read(srv.documents, reportid)) {
				repn += 1;

				var temp = report.f(2);
				temp.replacer("&", "&amp;");
				temp.replacer("<", "&lt;");
				temp.replacer(">", "&gt;");
				report(2) = temp;

				// !dont send instructions since takes up space and not needed
				// DO send now to have info in requestlog
				// report<5>=''

				report.converter(_VM, _RM);
				let nn = report.fcount(_FM);
				for (const var ii : range(1, nn)) {
					data_(ii, repn) = report.f(ii);
				}  // ii;

				data_(9, repn) = reportid;

				// print repn,data<1>
			}
			// if len(data)<65000 then goto nextrep
			if (data_.len() < maxstrsize_ - 530) {
				goto nextrep;
			}
		}
		response_ = "OK";

	} else if (mode == "DELETEREPORT") {

		gosub opendocuments();

		let docnos = request_.f(2);

		let ndocs = docnos.fcount(_VM);
		for (const var docn : range(1, ndocs)) {
			ID = docnos.f(1, docn);
			if (ID) {
				if (RECORD.read(srv.documents, ID)) {
					req.orec = RECORD;
					call getsubs("PREDELETE");
					if (not req.valid) {
						goto exit;
					}

					if (req.valid) {
						srv.documents.deleterecord(ID);

						call getsubs("POSTDELETE");
					}
				}
			}
		}  // docn;

	} else if (mode == "UPDATEREPORT") {

		gosub opendocuments();

		var doc;
		if (not doc.read(srv.documents, request_.f(2))) {
			abort("Document " ^ (request_.f(2).quote()) ^ " is missing");
		}

		// TODO security

		doc(6) = lower(data_);

		doc.write(srv.documents, request_.f(2));

	} else if (mode == "COPYREPORT") {

		gosub opendocuments();

		var doc;
		if (not doc.read(srv.documents, request_.f(2))) {
			abort("Document " ^ (request_.f(2).quote()) ^ " is missing");
		}

		task = doc.f(5);
		gosub gettaskprefix();
		if (taskprefix) {
			if (not authorised(taskprefix ^ " CREATE", msg_, "")) {
				abort(msg_);
			}
		}

		call getsubs("DEF.DOCUMENT.NO");
		if (not req.valid) {
			abort();
		}

		let description = doc.f(2);
		doc(2)			= description ^ " (Copy)";

		// prevent copy from appearing like a exodus standard
		doc(10) = "";

		doc(1) = USERNAME;
		doc.write(srv.documents, ID);

		data_ = ID ^ FM ^ doc;

		response_ = "OK";

	} else if (mode == "GETREPORT") {

		// printopts='L'
		gosub opendocuments();

		// get parameters from documents into @pseudo

		if (not srv.document.read(srv.documents, request_.f(2))) {
			msg_ = "Document " ^ (request_.f(2).quote()) ^ " does not exist";
			abort(msg_);
		}

		task = srv.document.f(5);
		gosub gettaskprefix();
		if (taskprefix) {
			if (not authorised(taskprefix ^ " ACCESS", msg_, "")) {
				abort(msg_);
			}
		}

		// if task='BALANCES' then printopts='P'
		// if index(task,'MEDIADIARY',1) then printopts='X'

		PSEUDO = srv.document.f(6);
		PSEUDO.converter(_RM, _VM);
		PSEUDO = raise(PSEUDO);

		// merge any runtime parameters into the real parameters
		for (let fn : range(1, 999)) {
			let tt = data_.f(fn);
			if (tt) {
				PSEUDO(fn) = tt;
			}
		}  // fn;
		data_ = "";

		// save runtime params in case saving below for scheduled reruns
		// document<11>=lower(data)

		var sentencex = srv.document.f(5);
		sentencex.converter(_VM, " ");
		data_ = PSEUDO;

		// in case we are calling another proxy
		if (srv.document.f(5, 1).ends("PROXY")) {

			// run but suppress email
			// perform 'TEST ':request<2>:' (S)'

			request_ = raise(srv.document.f(5)).field(_FM, 2, 999999) ^ FM ^ request_.f(2);
			// moved up so parameters show in any emailed error messages
			// data=@pseudo
			// override the saved period with a current period
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
			if (not response_) {

				gosub postproc();
			}
		}

	} else if (mode == "USAGESTATISTICS") {

		PSEUDO = data_;
		perform("LISTSTATS");

		// printopts='L'
		gosub postproc();

	} else if (mode == "VIEWLOG") {

		gosub initlog();
		var	  datedict = "LOG_DATE";

		var cmd = "LIST LOG" ^ logyear;
		cmd ^= " BY LOG_DATE BY LOG_TIME";
		cmd ^= " LOG_DATE LOG_TIME LOG_USERNAME LOG_WORKSTATION LOG_SOURCE LOG_MESSAGE2";
		if (logfromdate) {
			if (loguptodate) {
				cmd ^= "  xAND WITH LOG_DATE BETWEEN " ^ (logfromdate.oconv("D4").quote()) ^ " AND " ^ (loguptodate.oconv("D4").quote());
			} else {
				cmd ^= "  xAND WITH " ^ datedict ^ " GE " ^ (logfromdate.oconv("D4").quote());
			}
		}
		if (logkey) {
			cmd ^= " xAND WITH LOG_SOURCE1 STARTING " ^ (logkey.quote());
		}
		if (logsearch) {
			cmd ^= " xAND WITH ALLUPPERCASE CONTAINING " ^ (logsearch.quote());
		}
		cmd ^= " ID-SUPP";
		let temp = cmd.index("xAND");
		if (temp) {
			cmd.paster(temp, 5, "");
		}
		cmd.replacer("xAND", "AND");
		perform(cmd);

		// printopts='L'
		gosub postproc();

	} else if (mode == "LISTMARKETS") {

		var cmd = "SORT MARKETS WITH AUTHORISED BY SEQ";
		cmd ^= " HEADING " ^ (var("List of Markets     'T'     Page 'PL'").quote());
		perform(cmd);

		gosub postproc();

	} else if (mode == "LISTCURRENCIES") {

		perform("LISTCURRENCIES " ^ request_.f(2));

		gosub postproc();

	} else if (mode == "LISTCOMPANIES") {

		perform("LISTCOMPANIES");

		// printopts='L'
		gosub postproc();

	} else if (mode.field(".", 1) == "GETTASKS") {

		call securitysubs("GETTASKS." ^ request_.f(2) ^ "." ^ request_.f(3));
		data_ = ANS;

	} else {
		call note("System Error: " ^ (mode.quote()) ^ " invalid request in GENERALPROXY");
	}

// ///
exit:
	// ///
	return "";
}

subroutine postproc() {
	call proxysubs("GENERAL", mode, stationery);
	return;
}

subroutine initlog() {

	logkey		   = request_.f(2);
	logyear		   = request_.f(3);
	let logformat  = request_.f(4);
	var logoptions = request_.f(5);

	if (logoptions.match("^\\d*/\\d{2}$")) {
		logyear	   = addcent4(logoptions.field("/", 2));
		logoptions = "";
	}

	logfromdate = data_.f(5);
	loguptodate = data_.f(6);
	logsearch	= data_.f(7);

	if (logoptions == "TODAY") {
		logfromdate = date();
		loguptodate = logfromdate;
	}

	if (not logyear) {
		var tt	= logfromdate;
		var tt2 = loguptodate;
		if (not tt) {
			tt = date();
		}
		if (not tt2) {
			tt2 = tt;
		}
		logyear		  = tt.oconv("DY");
		let logtoyear = tt2.oconv("DY");
		if (logyear != logtoyear) {
			response_ = "Dates must be within one calendar year";
			abort();
		}
	}

	if (not authorised("LOG ACCESS", msg_, "")) {
		response_ = msg_;
		abort();
	}

	if (not authorised("LOG ACCESS " ^ (logkey.quote()), msg_, "")) {
		response_ = msg_;
		abort();
	}

	return;

	// in get.subs and generalproxy
}

subroutine gettaskprefix() {
	taskprefix = "";
	task	   = task.field(" ", 1);
	if (task == "ANAL") {
		taskprefix = "BILLING REPORT";
	} else if (task == "BALANCES") {
		taskprefix = "FINANCIAL REPORT";
	} else if (task == "ANALSCH") {
		taskprefix = "BILLING REPORT";
	} else {
		taskprefix = "";
	}
	return;
}

subroutine opendocuments() {
	if (not srv.documents.open("DOCUMENTS", "")) {
		abort(lasterror());
	}
	return;
}

libraryexit()
