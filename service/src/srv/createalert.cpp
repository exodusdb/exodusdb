#include <exodus/library.h>
#include <srv_common.h>

// performed in initgeneral2 and client init code

libraryinit()

#include <authorised.h>
#include <autorun2.h>
#include <generalalerts.h>

#include <service_common.h>


var msg;
var runasuser;
var authtasks;
var title;
var request;
var datax;

func main() {

	var sentencex = SENTENCE.field("(", 1);
	// cheap parser requires ~ for spaces
	sentencex.converter(" ~", FM ^ " ");

	let alertid		  = sentencex.f(2);
	let module		  = sentencex.f(3);
	let alerttype	  = sentencex.f(4);
	let runhours	  = sentencex.f(5);
	let runasusercode = sentencex.f(6);
	// fixed to suppress at the moment TODO provide more control
	let skipdaysoffandholidays = "*";

	var targetusercodes = sentencex.f(7).convert(",", VM);
	let testemail		= sentencex.f(8);

	let options	  = SENTENCE.field("(", 2);
	let silent	  = options.contains("S");
	let overwrite = options.contains("O");

	if (options.contains("R")) {

		if (not alertid) {
			goto syntax;
		}
		if (not silent) {
			if (not var().read(srv.documents, alertid)) {
				abort(alertid.quote() ^ " document doesnt exist");
			}
		}

		srv.documents.deleterecord(alertid);

		// RO means delete and overwrite to force rerun now (loss of lastrun time)
		if (not overwrite) {
			stop();
		}

	} else {
		if (not(silent or overwrite)) {
			if (var().read(srv.documents, alertid)) {
				call note(alertid.quote() ^ " already exists");
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
		call note(msg);
		stop();
	}

	var users;
	if (not users.open("USERS", "")) {
		abort(lasterror());
	}

	if (not targetusercodes) {
		targetusercodes = runasusercode;
		// goto syntax
	}

	var tt = runhours;
	if (not tt.len()) {
		goto syntax;
	}
	tt.converter(":0123456789," ^ VM, "");
	if (tt) {
		msg		= runhours.quote() ^ " invalid runhours/rundates";
		msg(-1) = "Examples:";
		msg(-1) = "2,17 for noon and 5pm";
		msg(-1) = "2,17:7,14 for noon and 5pm on the 7th and 14th";
		call note(msg);
		stop();
	}

	if (runasusercode != "EXODUS" and not(runasusercode.xlate("USERS", 1, "X"))) {
		call note(runasusercode.quote() ^ " runas user does not exist");
		stop();
	}

	// alertroutine=module:'.ALERTS'
	// call @alertroutine(alerttype,runasuser, authtasks,title,request,data)
	// name the variable after any one of the existing functions
	// so c++ conversion works
	// GENERAL.ALERTS MEDIA.ALERTS JOB.ALERTS FINANCE.ALERTS
	tt = module ^ ".ALERTS";
	// c++ variation
	if (not VOLUMES) {
		tt.lcaser();
		tt.converter(".", "");
	}
	generalalerts = tt;
	call generalalerts(alerttype, runasuser, authtasks, title, request, datax);

	if (authtasks) {
		let ntasks = authtasks.fcount(VM);
		for (const var taskn : range(1, ntasks)) {
			let task = authtasks.f(1, taskn);
			if (not authorised(task, msg, "", runasusercode)) {
				msg = "\"Runas\" user is not authorised to do " ^ task ^ FM ^ FM ^ msg;
				call note(msg);
				stop();
			}
		}  // taskn;
	}

	var docid = alertid;

	srv.document	 = "";
	srv.document(30) = testemail;
	srv.document(22) = runhours.field(":", 1);
	srv.document(23) = runhours.field(":", 2);
	srv.document(24) = runhours.field(":", 3);
	srv.document(25) = runhours.field(":", 4);
	srv.document(26) = runhours.field(":", 5);
	srv.document(27) = runhours.field(":", 6);
	// document<28>=
	srv.document(31) = module;
	srv.document(32) = alerttype;

	call autorun2("WRITE", title, module, request, datax, runasusercode, targetusercodes, srv.document, docid, msg);

	if (msg) {
		call note(msg);
		stop();
	}

	return 0;
}

}; // libraryexit()
