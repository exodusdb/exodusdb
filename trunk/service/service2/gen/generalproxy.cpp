#include <exodus/library.h>
libraryinit()

#include <emailusers.h>
#include <usersubs.h>
#include <authorised.h>
#include <securitysubs.h>
#include <sendmail.h>
#include <uploadsubs.h>
#include <generalsubs.h>
#include <changelogsubs.h>
#include <getsubs.h>
#include <sysmsg.h>
#include <addcent.h>

#include <gen.h>
#include <win.h>

var dbn;
var emailresult;
var usersordefinitions;
var userkey;
var USER;
var newpassword;
var taskprefix;
var logyear;
var logfromdate;
var loguptodate;
var logkey;
var logsearch;
var proxyname;
var mode;

function main() {
print("generalproxy");
	//*subroutine general(request,data,response)
	//jbase

	//var().clearcommon();
	proxyname="GENERALPROXY";

	win.valid = 1;
	USER4 = "";

	call cropper(USER0);
	call cropper(USER1);
	mode = USER0.a(1).ucase();

	//request 2 ... can be anything actually

	win.datafile = USER0.a(2);
	var keyx = USER0.a(3);

	USER3 = "OK";

	if (mode == "TEST") {

	} else if (mode == "FILEMAN" and USER0.a(2) == "COPYDB") {

		var copydb = USER0.a(3);
		if (not(SYSTEM.a(58).locateusing(copydb, VM, dbn))) {
			{}
		}
		var todb = SYSTEM.a(63, dbn);
		if (not todb) {
			call mssg("\"Copy to\" database must be configured (and saved) for database " ^ copydb ^ " first");
			return 1;
		}
		perform("COPYDB " ^ copydb ^ " " ^ todb);
		USER3 = USER4;
		if (not USER3) {
			USER3 = "OK Copy database " ^ copydb ^ " to " ^ todb ^ " completed";
		}

	} else if (mode == "EMAILUSERS") {

		var groupids = USER1.a(1);
		var jobfunctionids = "";
		var userids = USER1.a(2);
		if (not(groupids or jobfunctionids or userids)) {
			call mssg("You must specify some groups or users to email");
			return 1;
		}

		//ensure sender has an email address
		//not absolutely necessary but provides a return email address
		if (USERNAME ne "NEOSYS" and not USERNAME.xlate("USERS", 7, "X")) {
			call mssg("You cannot send email because you do not have an email address for replies");
			return 1;
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
			call mssg("No users can be found to email");
		}
		USER1 = "";

	} else if (mode == "VAL.EMAIL") {
		win.is = USER0;
		call usersubs(mode);

//	} else if (mode.substr(-3, 3) == "SSH") {
//		call ssh(mode);

	} else if (mode == "PASSWORDRESET") {

		if (not(authorised("PASSWORD RESET", USER4))) {
			call mssg(USER4);
			return 1;
		}

		var users;
		if (not(users.open("USERS", ""))) {
			call mssg("USERS file is missing");
			return 1;
		}

		ID = USER0.a(2);
		var emailaddress = USER0.a(3);

		var baduseroremail = "Either " ^ (DQ ^ (ID ^ DQ)) ^ " or " ^ (DQ ^ (emailaddress ^ DQ)) ^ " does not exist";

		var userrec;
		if (userrec.read(users, ID)) {
			usersordefinitions = users;
			userkey = ID;

			if (emailaddress.ucase() == USER.a(7).ucase()) {

				//signify ok
				baduseroremail = "";

				call securitysubs("GENERATEPASSWORD");
				newpassword = ANS;
				userrec.r(4, newpassword);

			}

		}else{
			usersordefinitions = gen._definitions;
			userkey = "BADUSER*" ^ ID;
			if (not(userrec.read(usersordefinitions, userkey))) {
				userrec = "";
			}
		}

		//record historical resets/attempts
		//datetime=(date():'.':time() 'R(0)#5')+0
		var datetime = var().date() ^ "." ^ (var().time()).oconv("R(0)#5");
		userrec.inserter(15, 1, datetime);
		userrec.inserter(16, 1, SYSTEM.a(40, 2));
		userrec.inserter(18, 1, "Password Reset " ^ baduseroremail.trim());

		if (baduseroremail) {
			userrec.write(usersordefinitions, userkey);
			call mssg(baduseroremail);
			return 1;
		}

		//prewrite (locks authorisation file or fails)
		win.valid = 1;
		call usersubs("PREWRITE");
		if (not win.valid) {
			return 1;
		}

		userrec.write(usersordefinitions, userkey);

		//postwrite
		call usersubs("POSTWRITE");

		//cc the user too if the username is valid
		//call sysmsg('User: ':username:fm:'From IP: ':system<40,2>:fm:text,'Password Reset',userkey)

		//send the new password to the user
		var emailaddrs = userrec.a(7);
		var ccaddrs = "";
		var subject = "NEOSYS Password Reset";
		var body = "User: " ^ ID;
		body.r(-1, "Your new password is " ^ newpassword);
		var xx;
		call sendmail(emailaddrs, ccaddrs, subject, body, "", "", xx);

	} else if (mode == "MAKEUPLOADPATH") {
		call uploadsubs("MAKEUPLOADPATH." ^ USER0.a(2));

	} else if (mode == "POSTUPLOAD") {
		call uploadsubs("POSTUPLOAD");

	} else if (mode == "VERIFYUPLOAD") {
		call uploadsubs("VERIFYUPLOAD." ^ USER0.a(2));

	} else if (mode == "OPENUPLOAD") {
		call uploadsubs("OPENUPLOAD." ^ USER0.a(2));

	} else if (mode == "DELETEUPLOAD") {
		call uploadsubs("DELETEUPLOAD." ^ USER0.a(2));

	} else if (mode == "GETCODEPAGE") {
		USER1 = "";
		//have to skip char zero it seems to be treated as string terminator
		//somewhere on the way to the browser (not in revelation)
		for (var ii = 1; ii <= 255; ++ii) {
			USER1 ^= var().chr(ii);
		};//ii;
		//data='xxx'
		USER3 = "OK";

	} else if (mode == "SETCODEPAGE") {
		var alanguage;
		if (not(alanguage.open("ALANGUAGE", ""))) {
			call fsmsg();
			return 1;
		}

		var codepage = USER0.a(3);
		if (not codepage) {
			goto badsetcodepage;
		}

		if (USER0.a(2) == "SORTORDER") {

			/* should be inverted but bother since cant get arev only collated ascii;
				if len(data) ne 254 then;
					call msg('SORTORDER data is ':len(data):' but must be 254 characters long|(initial char 0 excluded)');
	oswrite data on 'x';
					stop;
					end;
				write char(0):data on alanguage,'SORTORDER*':codepage;
			*/

		} else if (USER0.a(2) == "UPPERCASE" or USER0.a(2) == "LOWERCASE") {

			var fn;
			if (USER0.a(2) == "UPPERCASE")
				fn = 9;
			else
				fn = 10;

			if (not(RECORD.read(alanguage, "GENERAL*" ^ codepage))) {
				if (not(RECORD.read(alanguage, "GENERAL"))) {
					RECORD = "";
				}
			}
			var temp = USER1;
			temp.swapper(RM, "%FF");
			temp.swapper(FM, "%FE");
			temp.swapper(VM, "%FD");
			RECORD.r(1, fn, temp);

			RECORD.write(alanguage, "GENERAL*" ^ codepage);

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

		} else {
badsetcodepage:
			call mssg("Request must be SETCODEPAGE SORTORDER|UPPERCASE|LOWERCASE codepage");
			return 0;
		}

		USER3 = "OK";

	} else if (mode == "GETDATASETS") {
		ANS = "";
		call generalsubs("GETDATASETS");
		USER1 = ANS;
		if (USER1) {
			USER3 = "OK";
		}else{
			USER3 = "Error: No datasets found";
		}

	} else if (mode == "LISTPROCESSES") {
		perform("SORT PROCESSES");
		gosub checkoutputfileexists();

	} else if (mode == "LISTLOCKS") {
		perform("SORT LOCKS WITH NO LOCK_EXPIRED");
		gosub checkoutputfileexists();

	} else if (mode == "GETUPGRADEDATES") {

		call changelogsubs("GETUPGRADEDATES");

	} else if (mode.ucase() == "WHATSNEW") {

		call changelogsubs("SELECTANDLIST" ^ FM ^ USER1);
		if (USER4) {
			USER4.transfer(USER3);
			return 1;
		}

		gosub checkoutputfileexists();

	} else if (mode == "LISTADDRESSES") {
		perform("LISTADDRESSES");
		gosub checkoutputfileexists();

	} else if (mode == "GETDEPTS") {
		call generalsubs("GETDEPTS");
		USER1 = ANS;
		USER3 = "OK";

	} else if (mode == "LISTACTIVITIES") {
		perform("LISTACTIVITIES " ^ USER0.a(2));
		gosub checkoutputfileexists();

	} else if (mode == "ABOUT") {
		perform("ABOUT");
		//transfer @user4 to data
		//response='OK'
		USER4.transfer(USER3);
		USER3.splicer(1, 0, "OK ");

	} else if (mode == "UTIL") {
		perform("UTIL");
		USER3 = "OK";

	} else if (mode == "PROG") {
		perform("PROG");
		USER3 = "OK";

	} else if (mode.field(".", 1) == "LISTAUTH") {
		win.wlocked = 0;
		win.templatex = "SECURITY";
		call securitysubs("SETUP");
		if (not win.valid) {
			return 1;
		}
		//call security.subs('LISTAUTH')
		call securitysubs(mode);
		if (not win.valid) {
			return 1;
		}
		call securitysubs("POSTAPP");
		gosub checkoutputfileexists();

	} else if (mode == "READUSERS") {

		win.templatex = "SECURITY";
		call securitysubs("SETUP");
		if (not win.valid) {
			return 1;
		}

		USER1 = RECORD;
		USER3 = "OK";

	} else if (mode == "GETREPORTS") {

		var task = USER0.a(2);
		gosub gettaskprefix();
		if (taskprefix) {
			if (not(authorised(taskprefix ^ " ACCESS", USER4, ""))) {
				call mssg(USER4);
				return 1;
			}
		}

		var select = "SELECT DOCUMENTS BY-DSND NEOSYS_STANDARD BY DESCRIPTION";

		var instructions = USER0.a(2);
		instructions.swapper(VM, "%FD");
		select ^= " WITH INSTRUCTIONS2 " ^ (DQ ^ (instructions ^ DQ));

		var xx;
		if (not authorised("DOCUMENTS: ACCESS OTHER PEOPLES DOCUMENTS", xx)) {
			select ^= " AND WITH CREATEDBY " ^ (DQ ^ (USERNAME ^ DQ));
		}

//TODO		call safeselect(select ^ " (S)");
		var().select(select ^ " (S)");

		if (not opendocuments()) {
			return 0;
		}

		USER1 = "";
		var repn = 0;

		var reportid;
		while (var().readnext(reportid)) {
			var report;
			if (report.read(gen.documents, reportid)) {
				repn += 1;

				var temp = report.a(2);
				temp.swapper("&", "&amp;");
				temp.swapper("<", "&lt;");
				temp.swapper(">", "&gt;");
				report.r(2, temp);

				//dont send instructions since takes up space and not needed
				report.r(5, "");

				report.converter(VM, var().chr(255));
				var nn = report.count(FM) + 1;
				for (var ii = 1; ii <= nn; ++ii) {
					USER1.r(ii, repn, report.a(ii));
				};//ii;

				USER1.r(9, repn, reportid);

			}
			if (USER1.length() > 65000) {
				break;
			}
		}
		USER3 = "OK";

	} else if (mode == "DELETEREPORT") {

		if (not opendocuments()) {
			return 0;
		}

		var docnos = USER0.a(2);

		var ndocs = docnos.count(VM) + 1;
		for (var docn = 1; docn <= ndocs; ++docn) {
			ID = docnos.a(1, docn);
			if (ID) {
				if (RECORD.read(gen.documents, ID)) {
					win.orec = RECORD;
					call getsubs("PREDELETE");
					if (not win.valid) {
						return errorexit();
					}

					if (win.valid) {
						gen.documents.deleterecord(ID);
						call getsubs("POSTDELETE");
					}
				}
			}
		};//docn;

	} else if (mode == "UPDATEREPORT") {

		if (not opendocuments()) {
			return 0;
		}

		var doc;
		if (not(doc.read(gen.documents, USER0.a(2)))) {
			call mssg("Document " ^ (DQ ^ (USER0.a(2) ^ DQ)) ^ " is missing");
			return 1;
		}

		//TODO security

		doc.r(6, lower(USER1));

		doc.write(gen.documents, USER0.a(2));

	} else if (mode == "COPYREPORT") {

		if (not opendocuments()) {
			return 0;
		}

		var doc;
		if (not(doc.read(gen.documents, USER0.a(2)))) {
			call mssg("Document " ^ (DQ ^ (USER0.a(2) ^ DQ)) ^ " is missing");
			return 1;
		}

		var task = doc.a(5);
		gosub gettaskprefix();
		if (taskprefix) {
			if (not(authorised(taskprefix ^ " CREATE", USER4, ""))) {
				call mssg(USER4);
				return 1;
			}
		}

		call getsubs("DEF.DOCUMENT.NO");
		if (not win.valid) {
			return 1;
		}

		var description = doc.a(2);
		doc.r(2, description ^ " (Copy)");

		//prevent copy from appearing like a neosys standard
		doc.r(10, "");

		doc.r(1, USERNAME);
		doc.write(gen.documents, ID);

		USER1 = ID ^ FM ^ doc;

		USER3 = "OK";

	} else if (mode == "GETREPORT") {

		if (not opendocuments()) {
			return 0;
		}

		//get parameters from documents into @pseudo
		if (not(gen.document.read(gen.documents, USER0.a(2)))) {
			USER4 = "Document " ^ (DQ ^ (USER0.a(2) ^ DQ)) ^ " does not exist";
			call mssg(USER4);
			return 1;
		}

		var task = gen.document.a(5);
		gosub gettaskprefix();
		if (taskprefix) {
			if (not(authorised(taskprefix ^ " ACCESS", USER4, ""))) {
				call mssg(USER4);
				return 1;
			}
		}

		mv.PSEUDO = gen.document.a(6);
		mv.PSEUDO.converter(var().chr(255), VM);
		mv.PSEUDO.raiser();

		//merge any runtime parameters into the real parameters
		for (var fn = 1; fn <= 999; ++fn) {
			var tt = USER1.a(fn);
			if (tt) {
				mv.PSEUDO.r(fn, tt);
			}
		};//fn;
		USER1 = "";

		//save runtime params in case saving below for scheduled reruns
		//document<11>=lower(data)

		var sentencex = gen.document.a(5);
		sentencex.converter(VM, " ");
		USER1 = mv.PSEUDO;

		//in case we are calling another proxy
		if ((gen.document.a(5, 1)).substr(-5, 5) == "PROXY") {

			//run but suppress email
			//perform 'TEST ':request<2>:' (S)'

			USER0 = gen.document.a(5).field(FM, 2, 999999).raise();
			//moved up so parameters show in any emailed error messages
			//data=@pseudo
			//override the saved period with a current period
			var runtimeperiod = ((var().date()).oconv("D2/E")).substr(4, 5);
			if (runtimeperiod.substr(1, 1) == "0") {
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

	/* no longer allowed;
		case mode='CLEARLOG';

			gosub initlog;
			if valid else stop;

			if security('LOG DELETE ',msg,'NEOSYS') else;
				response=msg;
				stop;
				end;

			cmd='SELECT LOG':logyear;
			cmd:=' WITH LOG_SOURCE1 ':quote(logkey);
			perform cmd:' (S)';
			response='LOG':logyear:' ':logkey:' ';
			if @list.active then;
				perform 'DELETE LOG':logyear:' (S)';
				response:='has been cleared';
			end else;
				response:='was empty';
				end;
	*/

	} else if (mode == "USAGESTATISTICS") {
		mv.PSEUDO = USER1;
		perform("LISTSTATS");
		gosub checkoutputfileexists();

	} else if (mode == "VIEWLOG") {

		if (not initlog()) {
			return 0;
		}
		var datedict = "LOG_DATE";

		var cmd = "LIST LOG" ^ logyear;
		cmd ^= " BY LOG_DATE BY LOG_TIME";
		cmd ^= " LOG_DATE LOG_TIME LOG_USERNAME LOG_WORKSTATION LOG_SOURCE LOG_MESSAGE2";
		if (logfromdate) {
			if (loguptodate) {
				cmd ^= "  xAND WITH LOG_DATE BETWEEN " ^ (DQ ^ (logfromdate.oconv("[DATE]") ^ DQ)) ^ " AND " ^ (DQ ^ (loguptodate.oconv("[DATE]") ^ DQ));
			}else{
				cmd ^= "  xAND WITH " ^ datedict ^ " >= " ^ (DQ ^ (logfromdate.oconv("[DATE]") ^ DQ));
			}
		}
		if (logkey) {
			cmd ^= " xAND WITH LOG_SOURCE1 STARTING " ^ (DQ ^ (logkey ^ DQ));
		}
		if (logsearch) {
			cmd ^= " xAND WITH ALLUPPERCASE [] " ^ (DQ ^ (logsearch ^ DQ));
		}
		cmd ^= " ID-SUPP";
		var temp = cmd.index("xAND", 1);
		if (temp) {
			cmd.splicer(temp, 5, "");
		}
		cmd.swapper("xAND", "AND");
		perform(cmd);
		gosub checkoutputfileexists();

	} else if (mode == "LISTCURRENCIES") {
		perform("LISTCURRENCIES " ^ USER0.a(2));
		gosub checkoutputfileexists();

	} else if (mode.field(".", 1) == "GETTASKS") {

		call securitysubs("GETTASKS." ^ USER0.a(2) ^ "." ^ USER0.a(3));
		USER1 = ANS;

	} else if (1) {
		call mssg("System Error: " ^ (DQ ^ (mode ^ DQ)) ^ " invalid request in " ^ proxyname);
		return 0;

	}

///exit:
	return 1;

}

function errorexit() {
	//call unlockall(locklist);
	USER3 = "Error: " ^ USER3;
	return 0;
}

function errorresponse() {
	USER4.converter("|", FM);
	USER4.trimmerf(FM).trimmerb(FM);
	USER4.converter("||", FM ^ FM);
	USER4.swapper(FM ^ FM, "\r\n");
	USER4.swapper(FM, " ");
	USER3 = "Error: " ^ USER4;
	return 0;
}

function opendatafile() {
	if (not(win.srcfile.open(win.datafile, ""))) {
		USER4 = "The " ^ (DQ ^ (win.datafile ^ DQ)) ^ " file is not available";
		gosub errorresponse();
		return false;
	}

	var temp = win.datafile;
	if (temp.substr(-3, 3) == "IES") {
		temp.splicer(-3, 3, "Y");
	}
	if (temp.substr(-1, 1) == "S") {
		temp.splicer(-1, 1, "");
	}
	if (not(authorised(temp ^ " ACCESS", USER4, ""))) {
		gosub errorresponse();
		return false;
	}

	if (not(DICT.open("DICT", win.datafile))) {
		USER4 = "The " ^ (DQ ^ ("DICT." ^ win.datafile ^ DQ)) ^ " file is not available";
		gosub errorresponse();
		return false;
	}
	return true;
}

subroutine checkoutputfileexists() {
	if ((SYSTEM.a(2)).osfile().a(1) > 5) {
		USER1 = SYSTEM.a(2);
		USER3 = "OK";
		if (USER4) {
			USER3 ^= " " ^ USER4;
		}
		USER4 = "";
	}else{
		USER3 = USER4;
		if (USER3 == "") {
			USER3 = "Error: No output file in  " ^ proxyname ^ "," ^ mode;
			call sysmsg(USER3);
		}
	}

	return;
}

function initlog() {

	logkey = USER0.a(2);
	logyear = USER0.a(3);
	var logformat = USER0.a(4);
	var logoptions = USER0.a(5);

	if (logoptions.match("0N\"/\"2N")) {
		logyear = addcent(logoptions.field("/", 2));
		logoptions = "";
	}

	logfromdate = USER1.a(5);
	loguptodate = USER1.a(6);
	logsearch = USER1.a(7);

	if (logoptions == "TODAY") {
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
		logyear = (tt.oconv("D")).substr(-4, 4);
		var logtoyear = (tt2.oconv("D")).substr(-4, 4);
		if (logyear ne logtoyear) {
			USER3 = "Dates must be within one calendar year";
			return false;
		}
	}

	if (not(authorised("LOG ACCESS", USER4, ""))) {
		USER3 = USER4;
		return false;
	}

	if (not(authorised("LOG ACCESS " ^ (DQ ^ (logkey ^ DQ)), USER4, ""))) {
		USER3 = USER4;
		return false;
	}

	return true;

	//in get.subs and generalproxy
}

subroutine gettaskprefix() {
	taskprefix = "";
	var task = task.field(" ", 1);
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

function opendocuments() {
	if (not(gen.documents.open("DOCUMENTS", ""))) {
		call fsmsg();
		return 0;
	}
	return 1;

}


libraryexit()
