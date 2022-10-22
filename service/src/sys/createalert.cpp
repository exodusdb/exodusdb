#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <autorun2.h>
#include <generalalerts.h>

#include <system_common.h>

#include <sys_common.h>

	var msg;
var runasuser;
var authtasks;
var title;
var request;
var datax;

function main() {

	var sentencex = SENTENCE.field("(", 1);
	//cheap parser requires ~ for spaces
	sentencex.converter(" ~", FM ^ " ");

	var alertid		  = sentencex.f(2);
	var module		  = sentencex.f(3);
	var alerttype	  = sentencex.f(4);
	var runhours	  = sentencex.f(5);
	var runasusercode = sentencex.f(6);
	//fixed to suppress at the moment TODO provide more control
	var skipdaysoffandholidays = "*";

	var targetusercodes = sentencex.f(7).convert(",", VM);
	var testemail		= sentencex.f(8);

	var options	  = SENTENCE.field("(", 2);
	var silent	  = options.contains("S");
	var overwrite = options.contains("O");

	if (options.contains("R")) {

		if (not alertid) {
			goto syntax;
		}
		if (not silent) {
			var xx;
			if (not(xx.read(sys.documents, alertid))) {
				abort(alertid.quote() ^ " document doesnt exist");
			}
		}

		sys.documents.deleterecord(alertid);

		//RO means delete and overwrite to force rerun now (loss of lastrun time)
		if (not overwrite) {
			stop();
		}

	} else {
		if (not(silent or overwrite)) {
			var xx;
			if (xx.read(sys.documents, alertid)) {
				call mssg(alertid.quote() ^ " already exists");
				stop();
			}
		}
	}

	if (not runasusercode) {
syntax:
		msg		= "Syntax is";
		msg(-1) = "CREATEALERT alertid module alerttype runhours,...:rundoms,...";
		msg(-1) = "runasusercode {targetusercode,...} {testemail} (options)";
		msg(-1) = FM ^ "module=GENERAL/FINANCE/AGENCY/MEDIA/JOB";
		msg(-1) = "6 required parameters plus {} means optional";
		msg(-1) = "options: O=Overwrite R=Remove S=Silent ROS=forget lastruntime";
		call mssg(msg);
		stop();
	}

	var users;
	if (not(users.open("USERS", ""))) {
		abort(lasterror());
	}

	if (not targetusercodes) {
		targetusercodes = runasusercode;
		//goto syntax
	}

	var tt = runhours;
	if (not(tt.len())) {
		goto syntax;
	}
	tt.converter(":0123456789," ^ VM, "");
	if (tt) {
		msg		= runhours.quote() ^ " invalid runhours/rundates";
		msg(-1) = "Examples:";
		msg(-1) = "2,17 for noon and 5pm";
		msg(-1) = "2,17:7,14 for noon and 5pm on the 7th and 14th";
		call mssg(msg);
		stop();
	}

	if (runasusercode ne "EXODUS" and not(runasusercode.xlate("USERS", 1, "X"))) {
		call mssg(runasusercode.quote() ^ " runas user does not exist");
		stop();
	}

	//alertroutine=module:'.ALERTS'
	//call @alertroutine(alerttype,runasuser, authtasks,title,request,data)
	//name the variable after any one of the existing functions
	//so c++ conversion works
	//GENERAL.ALERTS MEDIA.ALERTS JOB.ALERTS FINANCE.ALERTS
	tt = module ^ ".ALERTS";
	//c++ variation
	if (not(VOLUMES)) {
		tt.lcaser();
		tt.converter(".", "");
	}
	generalalerts = tt;
	call generalalerts(alerttype, runasuser, authtasks, title, request, datax);

	if (authtasks) {
		let ntasks = authtasks.fcount(VM);
		for (const var taskn : range(1, ntasks)) {
			var task = authtasks.f(1, taskn);
			if (not(authorised(task, msg, "", runasusercode))) {
				msg = "\"Runas\" user is not authorised to do " ^ task ^ FM ^ FM ^ msg;
				call mssg(msg);
				stop();
			}
		}  //taskn;
	}

	var docid = alertid;

	sys.document	 = "";
	sys.document(30) = testemail;
	sys.document(22) = runhours.field(":", 1);
	sys.document(23) = runhours.field(":", 2);
	sys.document(24) = runhours.field(":", 3);
	sys.document(25) = runhours.field(":", 4);
	sys.document(26) = runhours.field(":", 5);
	sys.document(27) = runhours.field(":", 6);
	//document<28>=
	sys.document(31) = module;
	sys.document(32) = alerttype;

	call autorun2("WRITE", title, module, request, datax, runasusercode, targetusercodes, sys.document, docid, msg);

	if (msg) {
		call mssg(msg);
		stop();
	}

	return 0;
}

libraryexit()
