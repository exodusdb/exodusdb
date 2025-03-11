#include <exodus/library.h>
#include <srv_common.h>
#include <req_common.h>

#include <req_common.h>

libraryinit()

#include <authorised.h>
#include <generalalerts.h>
#include <holiday.h>
#include <listen2.h>
#include <listen4.h>
#include <sendmail.h>
#include <sysmsg.h>

#include <service_common.h>

var docids;
var options;
var inpath;
var docn;  // num
var docid;
//var xx;
var forceemail;
var toaddress;
var useraddress;
var marketcode;
var market;
var agp;
var holidaytype;
var workdate;
//var tt;	 // num
var authtasks;
var title;
var datax;
var start_timestamp;
var daysago;   // num
var xdate;	   // num
var opendate;  // num
var printfilename;
var body;
var attachfilename;
var errormsg;
var voccmd;
var tracing;  // num
var linkfilename2;
var requeststarttime;  // num
var requeststoptime;   // num
var printfile;

function main(in docids0 = "", in options0 = "") {

	// LISTEN calls this every minute
	// print 'autorun'

//	if (docids0.unassigned()) {
//		docids = "";
//	} else {
//		docids = docids0;
//	}
//	if (options0.unassigned()) {
//		options = "";
//	} else {
//		options = options0;
//	}
	docids = docids0.or_default("");
	options = options0.or_default("");

	// used to email EXODUS if EXODUS user in list of recipients
	let sysmsgatexodus = "sysmsg@neosys.com";

	// used to email if run as EXODUS unless document's forced email address is set
	let devatexodus = "dev@neosys.com";

	// logging=@username='EXODUS'
	let logging = 0;

	let suppressemail = options.contains("S");

	var users;
	if (not users.open("USERS", "")) {
		call note(lasterror());
		return 0;
	}

	let datasetcode = SYSTEM.f(17);
	if (not datasetcode) {
		printl("========== DATASETCODE MISSING ==========");
	}
	let islivedb = not(SYSTEM.f(61));

	// allow one autorun per database - hopefully this wont overload the server
	let lockfilename = "DOCUMENTS";
	var lockfile	 = srv.documents;
	let lockkey		 = "%" ^ datasetcode ^ "%";
	if (not lockrecord(lockfilename, lockfile, lockkey, "", 1)) {

		return 0;
	}

	// path to output reports
	var webpath = "";
	// if webpath else webpath='..\':'data\'
	if (not webpath) {
		webpath =
			"../"
			"data/";
	}
	webpath.converter("/", OSSLASH);
	if (not webpath.ends(OSSLASH)) {
		webpath ^= OSSLASH;
	}
	inpath = webpath ^ datasetcode ^ OSSLASH;

	// initdoc:
	// //////
	if (docids) {
		docids.replacer(",", _FM);
		docn = 0;
	} else {
		select(srv.documents);
	}
	var locked		   = 0;
	var ndocsprocessed = 0;

nextdoc:
	// //////

	if (locked) {
		call unlockrecord("DOCUMENTS", srv.documents, docid);
		locked = 0;
	}

	// dont process all documents one after the other within one
	// call of autorun to avoid overloading the subroutine stack cache etc
	// so docexit doesnt goto nextdoc .. it goes to exit
	if (ndocsprocessed > 1) {
		gosub exit(lockfilename, lockfile, lockkey);
		return 0;
	}
	if (docids) {
		docn += 1;
		docid = docids.f(docn);
		if (not docid) {
			gosub exit(lockfilename, lockfile, lockkey);
			return 0;
		}
	} else {
		if (not readnext(docid)) {
			gosub exit(lockfilename, lockfile, lockkey);
			return 0;
		}
	}

readdoc:
	// //////
	// to save processing time, dont lock initially until looks like it needs processing
	// then lock/read/check again in case other processes doing the same
	// depending on the autorunkey then this may be redundant

	// get document
	if (not srv.document.read(srv.documents, docid)) {
		printl(docid.quote(), " document doesnt exist in AUTORUN3");
		// call ossleep(1000*1)
		goto nextdoc;
	}

	// skip List of Current Users on ACCOUNTS systems
	if (APPLICATION == "ACCOUNTS" and docid == "CURRUSERS") {
		goto nextdoc;
	}

	// only do saved and enabled documents for now
	// 0/1 saved disabled/enabled. Blank=Ordinary documents
	if (not srv.document.f(12)) {
		goto nextdoc;
	}

	if (logging) {
		printx(docid, " ");
	}

	// determine current datetime
currdatetime:
	// ///////////
	let itime = time();
	var idate = date();
	// handle rare case where passes midnight between time() and date()
	if (time() < itime) {
		goto currdatetime;
	}
	let currdatetime = idate + itime / 86400;

	// skip if scanning all docs and not time to process yet
	// allow processing individual docs manually despite timing/scheduling
	if (docids == "") {

		// would be faster to work out nextdatetime once initially - but how to do it?
		// if not(docids) and currdatetime<nextdatetime then goto nextdoc

		let lastdatetime = srv.document.f(13);

		// skip if already run in the last 60 minutes. this is an easy way
		// to avoid reruns but maximum scheduling frequency is hourly
		if ((currdatetime - lastdatetime).abs() <= 1 / 24.0) {
			goto nextdoc;
		}

		// scheduling in document all can be multivalued
		// 21 min 0-59 (not used currently)
		// 22 hour 0-23 (minimum hour if only one)
		// 23 day of month 1-31 means 1st-31st
		// 24 month of year 1-12 means Jan-Dec
		// 25 day of week 1-7 means Mon-Sun
		// 26 date
		// 27 max number of times

		var restrictions = trimlast(srv.document.field(_FM, 21, 7), _FM);
		restrictions.converter(",", _VM);

		// skip if no restrictions applied yet
		if (restrictions == "") {
			goto nextdoc;
		}

		// only run scheduled reports on live data (but do run queued reports)
		// queued reports have only maxtimes=1 set
		if (((restrictions ne(FM.str(6) ^ 1)) and not(islivedb)) and not(var("exodus.id").osfile())) {
			if (logging) {
				printl("scheduled report but not live db");
			}
			goto nextdoc;
		}

		// hour of day restrictions
		let hours = restrictions.f(2);
		if (hours != "") {
			let hournow = itime.oconv("MT").first(2) + 0;

			// if one hour then treat it as a minimum hour
			if (hours.isnum()) {
				if (hournow < hours.mod(24)) {
					if (logging) {
						printl("not yet hour");
					}
					goto nextdoc;
				} else {
					// ensure not done already today in a previous hour
					goto preventsameday;
				}

				// or specific multiple hours
			} else {
				if (not hours.locate(hournow)) {
					if (logging) {
						printl("wrong hour");
					}
					goto nextdoc;
				}
			}

			// if no hourly restrictions then skip if already run today
		} else {
preventsameday:
			if (currdatetime.floor() == lastdatetime.floor()) {
				if (logging) {
					printl("already run today");
				}
				goto nextdoc;
			}
		}

		let date = idate.oconv("D/E");

		// day of month restrictions
		if (restrictions.f(3)) {
			if (not restrictions.f(3).locate(date.field("/", 1) + 0)) {
				if (logging) {
					printl("wrong day of month");
				}
				goto nextdoc;
			}
		}

		// month of year restrictions
		if (restrictions.f(4)) {
			if (not restrictions.f(4).locate(date.field("/", 2) + 0)) {
				if (logging) {
					printl("wrong month");
				}
				goto nextdoc;
			}
		}

		// day of week restrictions
		if (restrictions.f(5)) {
			if (not restrictions.f(5).locate((idate - 1).mod(7) + 1)) {
				if (logging) {
					printl("wrong day of week");
				}
				goto nextdoc;
			}
		}

		// date restrictions
		if (restrictions.f(6)) {
			if (not restrictions.f(6).locate(idate)) {
				if (logging) {
					printl("wrong date");
				}
				goto nextdoc;
			}
		}

		// would be better to work out next run time once - but how to work it out
		// if not(docids) and currdatetime<nextdatetime then goto nextdoc
	}

	// lock documents that need processing
	// but reread after lock in case another process has not started processing it
	if (not locked) {
		if (not lockrecord("DOCUMENTS", srv.documents, docid)) {
			if (logging) {
				printl("locked");
			}
			goto nextdoc;
		}
		locked = 1;
		goto readdoc;
	}

	// register that the document has been processed
	// even if nobody present to be emailed
	srv.document(13) = currdatetime;
	if (srv.document.f(27) != "") {
		srv.document(27) = srv.document.f(27) - 1;
	}

	// Delete once-off documents
	if (srv.document.f(27) != "" and not(srv.document.f(27))) {
		srv.documents.deleterecord(docid);

	} else {
		srv.document.write(srv.documents, docid);
	}

	// force all emails to be routed to test address
	// if on development system they are ALWAYS routed
	// so this is mainly for testing on client systems
	forceemail = srv.document.f(30);
	// if not(forceemail) and @username='EXODUS' then forceemail=devATexodus

	// On disabled systems all autorun documents except once-off documents
	// go to neosys.com and do NOT go to the actual users
	// TODO Use a configurable email from SYSTEM
	if (not(srv.document.f(27)) && var("../../disabled.cfg").osfile())
		forceemail = devatexodus;

	// report is always run as the document owning user
	let runasusercode = srv.document.f(1);
	var userx;
	if (not userx.read(users, runasusercode)) {
		if (runasusercode != "EXODUS") {
			printl("runas user ", runasusercode, " doesnt exist");
			goto nextdoc;
		}
		userx = "";
	}
	// allow running as EXODUS and emailing to sysmsg@neosys.com
	if (userx.f(7) == "" and runasusercode == "EXODUS") {
		userx	 = "EXODUS";
		userx(7) = sysmsgatexodus;
	}

	// HAS RECIPIENTS
	// if there are any recipients then
	// build email addresses of all recipients
	// depending on weekends/personal holidays etc
	// and skip if no recipients
	// recipients may get nothing if it is a
	// dynamically emailed report
	// they may get just summary report

	// HASNT RECIPIENTS
	// dynamically emailed reports need no recipents
	// The runasuser (if have emailaddress) will get
	// any output regardless of if they are on holiday

	let ccaddress = "";
	let usercodes = srv.document.f(14);
	if (usercodes == "") {
		toaddress = userx.f(7);
	} else {
		toaddress	  = "";
		let nusers	  = usercodes.fcount(VM);
		let backwards = 1;
		//for (var usern = nusers; usern >= 1; --usern) {
		for (let usern : reverse_range(1, nusers)) {

			// get the user record
			let usercode = usercodes.f(1, usern);
			if (not userx.read(users, usercode)) {
				if (usercode != "EXODUS") {
					goto nextuser;
				}
				userx = "EXODUS";
			}

			// skip if user has no email address
			if (userx.f(7) == "" and usercode == "EXODUS") {
				userx(7) = sysmsgatexodus;
			}
			useraddress = userx.f(7);
			if (useraddress) {

				// if running as EXODUS always add user EXODUS
				// regardless of holidays - to allow testing on weekends etc
				// if usercode='EXODUS' then
				if (USERNAME == "EXODUS" and usercode == "EXODUS") {
					goto adduseraddress;

					// optionally skip people on holiday (even EXODUS unless running as EXODUS)
				} else {

					marketcode = userx.f(25);
					if (not marketcode) {
						marketcode = srv.company.f(30, 1);
					}
					market = marketcode;
					if (srv.markets) {
						if (not market.read(srv.markets, marketcode)) {
							market = marketcode;
						}
					}

					idate = date();
					agp	  = "";
					call holiday("GETTYPE", idate, usercode, userx, marketcode, market, agp, holidaytype, workdate);

					if (not holidaytype) {
adduseraddress:
						if (backwards) {
							toaddress.inserter(1, useraddress);
						} else {
							toaddress(-1) = useraddress;
						}
					}
				}
			}
nextuser:;
		}  // usern;

		// skip if nobody to email to
		if (not toaddress) {
			if (logging) {
				printl("nobody to email");
			}
			goto nextdoc;
		}

		toaddress.converter(_FM, ";");
	}

	// before running the document refresh the title, request and data
	let module	  = srv.document.f(31);
	let alerttype = srv.document.f(32);

	if (module and alerttype) {

		var tt = module ^ ".ALERTS";

		// c++ variation
		if (not VOLUMES) {
			tt.lcaser();
			tt.converter(".", "");
		}

		generalalerts = tt;
		call generalalerts(alerttype, runasusercode, authtasks, title, request_, datax);

		// update the document and documents file if necessary
		call cropper(srv.document);
		var	 origdocument = srv.document;

		srv.document(2) = title;
		srv.document(5) = lower(module ^ "PROXY" _FM ^ request_);
		srv.document(6) = lower(datax);

		call cropper(datax);
		if (srv.document != origdocument) {
			srv.document.write(srv.documents, docid);
		}

	} else {
		authtasks = "";
	}

	// check if runasuser is authorised to run the task
	if (authtasks) {
		let ntasks = authtasks.fcount(VM);
		for (const var taskn : range(1, ntasks)) {
			let task = authtasks.f(1, taskn);
			if (not authorised(task, msg_, "", runasusercode)) {
				msg_ = runasusercode.quote() ^ " is not authorised to do " ^ task;
				printl(msg_);
				goto nextdoc;
			}
		}  // taskn;
	}

	// docinit:
	// //////
	if (logging) {
		printl("running as ", runasusercode);
	}

	start_timestamp	 = ostimestamp();

	ndocsprocessed += 1;

	// become the user so security is relative to the document "owner"
	var connection = "VERSION 3";
	connection(2)  = "0.0.0.0";
	connection(3)  = "SERVER";
	connection(4)  = "";
	connection(5)  = "";
	call listen2("BECOMEUSERANDCONNECTION", runasusercode, "", connection);

	// request='EXECUTE':fm:'GENERAL':fm:'GETREPORT':fm:docid
	// voccmd='GENERALPROXY'
	request_ = raise("EXECUTE" _VM ^ srv.document.f(5));

	data_ = raise(srv.document.f(6));

	// override the saved period with a current period

	// get today's period
	var runtimeperiod = date().oconv("D2/E").b(4, 5);
	if (runtimeperiod.starts("0")) {
		runtimeperiod.cutter(1);
	}
	// should backdate period to maximum open period for all selected companies
	// to avoid "year is not open" type messages
	// TODO

	data_.replacer("{RUNTIME_PERIOD}", runtimeperiod);
	data_.replacer("{TODAY}", date());
	data_.replacer("{7DAYSAGO}", date() - 7);
	data_.replacer("{14DAYSAGO}", date() - 14);
	data_.replacer("{21DAYSAGO}", date() - 21);
	data_.replacer("{28DAYSAGO}", date() - 28);
	data_.replacer("{30DAYSAGO}", date() - 30);
	data_.replacer("{60DAYSAGO}", date() - 60);
	data_.replacer("{90DAYSAGO}", date() - 90);
	data_.replacer("{YESTERDAY}", date() - 1);
	data_.replacer("{TOMORROW}", date() + 1);
	if (data_.contains("{2WORKINGDAYSAGO}")) {
		daysago = 2;
		gosub getdaysago();
		data_.replacer("{2WORKINGDAYSAGO}", xdate);
	}
	if (data_.contains("{3WORKINGDAYSAGO}")) {
		daysago = 3;
		gosub getdaysago();
		data_.replacer("{3WORKINGDAYSAGO}", xdate);
	}
	let closedperiod = srv.company.f(37);
	if (closedperiod) {
		opendate = iconv(closedperiod, srv.company.f(6)) + 1;
	} else {
		opendate = 11689;
	}
	data_.replacer("{OPERATIONS_OPEN_DATE}", opendate);

	// convert {TODAY-99} to today minus 99
	// and {TODAY+999} to today+999
	var sign = "-";
nextsign:
	var tt = data_.index("{TODAY" ^ sign);
	if (tt) {
		let t2 = (data_.cut(tt)).field("}", 1);
		data_.replacer("{" ^ t2 ^ "}", date() + t2.cut(5));
	}
	if (sign == "-") {
		sign = "+";
		goto nextsign;
	}

	// run the report
	gosub exec();

	// docexit:
	// //////

	if (not suppressemail) {

		// email only if there is an outputfile
		// ANALTIME2 emails everything out and returns 'OK ... ' in response
		// if dir(printfilename)<1> else goto nextdoc

		var subject = "EXODUS";
		// if repeatable then include report number to allow filtering
		if (srv.document.f(27) == "") {
			subject ^= " " ^ docid;
		}
		subject ^= ": %RESULT%" ^ srv.document.f(2);

		// email it
//		if (response_.first(2) != "OK" or printfilename.osfile().f(1) < 10) {
		if (not response_.starts("OK") or printfilename.osfile().f(1) < 10) {

			// plain "OK" with no file means nothing to email
			if (response_ == "OK") {
				goto nextdoc;
			}

			body	 = "";
			body(-1) = response_;
			if (response_.starts("Error:")) {
				response_.paster(1, 6, "Result:");
			}
			if (response_.contains("Error")) {
				subject ^= " ERROR";
				//var(response_).oswrite("xyz.xyz");
				//if (not var(response_).oswrite("xyz.xyz"))
				//	loglasterror();
			}
			// swap 'Error:' with 'Result:' in body
			body(-1) = ("Document: " ^ srv.document.f(2) ^ " (" ^ docid ^ ")").trim();
			body(-1) = "Database: " ^ SYSTEM.f(23) ^ " (" ^ SYSTEM.f(17) ^ ")";
			// swap '%RESULT%' with '* ' in subject
			subject.replacer("%RESULT%", "");

			// treat all errors as system errors for now
			// since autorun doesnt really know a user to send them to
			// NB programs should return OK+message if no report is required (eg "OK no ads found")
			if (response_.starts("OK")) {
				response_ = response_.cut(2).trimfirst();
			} else {
				call sysmsg(subject ^ _FM ^ body);
				goto nextdoc;
			}

			attachfilename = "";

		} else {

			let timetext = elapsedtimetext(start_timestamp, ostimestamp());

			// if ucase(printfilename[-4,4])='.XLS' then
			tt = (field(printfilename, ".", -1)).lcase();
			if (tt.contains("htm") and srv.document.f(33) != "2") {
				// insert body from file
				body = "@" ^ printfilename;
				subject ^= " in " ^ timetext;
			} else {
				attachfilename = oscwd();
				if (not VOLUMES) {
					attachfilename ^= OSSLASH;
				}
				attachfilename ^= printfilename;
				body = timetext;
			}

			subject.replacer("%RESULT%", "");
		}
		//body.replacer(_FM, chr(13));
		// chr(13) no longer compatible with Postfix 3.6.4-1ubuntu1.2
		// sendmail.cpp converts FM to newline

		// Option to force the actual email recipient
		let system117 = SYSTEM.f(117);
		if (forceemail)
			SYSTEM(117) = forceemail;

		printx(" Emailing ", toaddress, ":");
		call sendmail(toaddress, ccaddress, subject, body, attachfilename, "", errormsg);
		printx("done");

		// Restore original forced email or lack thereof
		SYSTEM(117) = system117;

		if (errormsg and errormsg != "OK") {
			// call msg(errormsg)
			call sysmsg(errormsg);
			printl(errormsg);
		}
	}

	printl();
	goto nextdoc;
}

subroutine exec() {

	voccmd = request_.f(2);

	tracing = 1;

	// generate a unique random output file
	while (true) {
		// linkfilename2=inpath:str(rnd(10^15),8)[1,8]
		linkfilename2 = inpath ^ ("00000000" ^ var(99999999).rnd()).last(8);
		// /BREAK;
		if (not oslistf(linkfilename2 ^ ".*"))
			break;
	}  // loop;

	linkfilename2 ^= ".2";
	//call oswrite("", linkfilename2);
	if (not oswrite("", linkfilename2))
		abort(lasterror());

	// turn interactive off in case running from command line
	// to avoid any reports prompting for input here
	let s33	   = SYSTEM.f(33);
	SYSTEM(33) = 1;

	gosub exec2();

	// restore interactivity status
	SYSTEM(33) = s33;

	return;
}

subroutine exec2() {
	requeststarttime = ostime();
	// system<25>=requeststarttime
	// allow autorun processes to run for ever
	SYSTEM(25) = "";
	request_   = request_.field(_FM, 3, 99999);

	// localtime=mod(time()+@sw<1>,86400)
	// print @(0):@(-4):localtime 'MTS':' AUTORUN ':docid:
	// similar in LISTEN and AUTORUN
	printl();
	printx(time().oconv("MTS"), " AUTORUN ", docid, " ", USERNAME, " ", request_.convert(_FM, " "), " ", srv.document.f(2), ":");

	// print 'link',linkfilename2
	// print 'request',request
	// print 'iodat',iodat

	// following simulates LISTEN's 'EXECUTE'

	// provide an output file for the program to be executed
	// NB response file name for detaching processes
	// will be obtained from the output file name LISTEN2 RESPOND
	// this could be improved to work
	printfilename = linkfilename2;
	var tt		  = oscwd();
	tt.cutter(-7);
	if (printfilename.starts(tt)) {
		printfilename.paster(1, tt.len(), "../");
	}
	printfilename.converter("/", OSSLASH);

	// tt=printfilename[-1,'B.']
	tt = field(printfilename, ".", -1);
	printfilename.paster(-tt.len(), tt.len(), "htm");
	SYSTEM(2) = printfilename;
	// if tracing then
	// print datasetcode:' Waiting for output:':
	// end

	// execute the program
	// request, iodat and response are now passed and returned in @user0,1 and 3
	// other messages are passed back in @user4
	// execute instead of call prevents program crashes from crashing LISTEN
	response_ = "OK";
	req.valid = 1;
	msg_	  = "";

	// pass the output file in linkfilename2
	// not good method, pass in system?
	if (var("LIST,SELECTJOURNALS").locateusing(",", request_.f(1))) {
		data_ = linkfilename2;
	}

	SYSTEM(117) = forceemail;

	execute(voccmd);

	SYSTEM(117) = "";

	// discard any stored input
	DATA = "";

	// detect memory corruption?
	// @user4='R18.6'
	if (msg_.contains("R18.6")) {
		let halt = 1;
		msg_(-1) = "Corrupt temporary file. Restart Needed.";
		msg_(-1) = "EXODUS.NET TERMINATED";
	}

	// convert error message
	if (msg_.contains(" IN INDEX.REDUCER AT ") or msg_.index(" IN RTP21 AT ")) {
		// @user4='Please select fewer records and/or simplify your request'
		call listen4(17, msg_);
	}

	// no records are not system errors
	if (response_.starts("No record") or response_.starts("No item")) {
		response_.prefixer("OK ");
		msg_ = "";
	}

	// send errors to exodus
	if (msg_.contains("An internal error") or msg_.contains("Error:")) {
		msg_.move(response_);
		goto sysmsgit;
	}

	// send errors to exodus
//	if (response_ == "" or response_.first(2) != "OK") {
	if (response_ == "" or not response_.starts("OK")) {
		if (not response_) {
			response_ = "No response from " ^ voccmd;
		}
sysmsgit:
		call sysmsg("AUTORUN " ^ docid ^ " " ^ srv.document.f(2) ^ _FM ^ response_);
	}

	call cropper(msg_);
	call cropper(response_);

	if (msg_) {
		data_	  = "";
		response_ = "Error: " ^ msg_;
		gosub fmtresp();
	}

	if (response_ == "") {
		// response='Error: No OK from ':voccmd:' ':request
		call  listen4(18, response_, voccmd);
		gosub fmtresp();
	}

	var rawresponse = response_;
	rawresponse.replacer("\r\n", "|");
	rawresponse.converter("\n", "|");

	// get the printfilename in case the print program changed it
	printfilename = SYSTEM.f(2);
	// and close it in case print program didnt (try to avoid sendmail attach errors)
	printfilename.osclose();
	if (tt.osopen(printfilename)) {
		tt.osclose();
	}
	var().osflush();

	// trace responded
	requeststoptime = ostime();
	if (tracing) {
		// print @(0):@(-4):'Responded in ':(requeststoptime-requeststarttime) 'MD20P':' SECS ':rawresponse
		printx(" ", ((requeststoptime - requeststarttime).mod(86400)).oconv("MD20P"), "secs ", rawresponse);
		// do after "emailing" message
		// print str('-',79)
		// print linkfilename1
	}

	// make sure that the output file is closed
	if (printfile.osopen(printfilename)) {
		printfile.osclose();
	}

	return;
}

subroutine exit(in lockfilename, io lockfile, in lockkey) {

	call unlockrecord(lockfilename, lockfile, lockkey);

	return;
}

subroutine fmtresp() {

	// trim everything after <ESC> (why?)
	var tt = response_.index("<ESC>");
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

subroutine getdaysago() {
	var weekend = "67";
	marketcode	= srv.company.f(30, 1);
	if (marketcode) {
		var marketweekend = marketcode.xlate("MARKETS", 9, "X");
		if (marketweekend) {
			weekend = marketweekend;
		}
	}

	xdate = date();
	while (true) {
		xdate -= 1;
		// if (not weekend.contains((xdate - 1).mod(7) + 1)) {
		if (not weekend.contains(xdate.oconv("DW"))) {
			daysago -= 1;
		}
		// /BREAK;
		if (not daysago)
			break;
	}  // loop;

	return;
}

libraryexit()
