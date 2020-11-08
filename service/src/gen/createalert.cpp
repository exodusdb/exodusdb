#include <exodus/library.h>
libraryinit()

#include <generalalerts.h>
#include <authorised.h>
#include <autorun2.h>

#include <gen_common.h>

var msg;
var runasuser;
var authtasks;
var title;
var request;
var datax;

function main() {
	//c gen

	#include <general_common.h>

	var sentencex = SENTENCE.field("(", 1);
	//cheap parser requires ~ for spaces
	sentencex.converter(" ~", FM ^ " ");

	var alertid = sentencex.a(2);
	var module = sentencex.a(3);
	var alerttype = sentencex.a(4);
	var runhours = sentencex.a(5);
	var runasusercode = sentencex.a(6);
	//fixed to suppress at the moment TODO provide more control
	var skipdaysoffandholidays = "*";

	var targetusercodes = sentencex.a(7).convert(",", VM);
	var testemail = sentencex.a(8);

	var options = SENTENCE.field("(", 2);
	var silent = options.index("S");
	var overwrite = options.index("O");

	if (options.index("R")) {

		if (not alertid) {
			goto syntax;
		}
		if (not silent) {
			var xx;
			if (not(xx.read(gen.documents, alertid))) {
				call fsmsg();
				stop();
			}
		}

		gen.documents.deleterecord(alertid);

		//RO means delete and overwrite to force rerun now (loss of lastrun time)
		if (not overwrite) {
			stop();
		}

	}else{
		if (not(silent or overwrite)) {
			var xx;
			if (xx.read(gen.documents, alertid)) {
				call mssg(alertid.quote() ^ " already exists");
				stop();
			}
		}
	}

	if (not runasusercode) {
syntax:
		msg = "Syntax is";
		msg.r(-1, "CREATEALERT alertid module alerttype runhours,...:rundoms,...");
		msg.r(-1, "runasusercode {targetusercode,...} {testemail} (options)");
		msg.r(-1, FM ^ "module=GENERAL/FINANCE/AGENCY/MEDIA/JOB");
		msg.r(-1, "6 required parameters plus {} means optional");
		msg.r(-1, "options: O=Overwrite R=Remove S=Silent ROS=forget lastruntime");
		call mssg(msg);
		stop();
	}

	var users;
	if (not(users.open("USERS", ""))) {
		call fsmsg();
		stop();
	}

	if (not targetusercodes) {
		targetusercodes = runasusercode;
		//goto syntax
	}

	var tt = runhours;
	if (not(tt.length())) {
		goto syntax;
	}
	tt.converter(":0123456789," ^ VM, "");
	if (tt) {
		msg = runhours.quote() ^ " invalid runhours/rundates";
		msg.r(-1, "Examples:");
		msg.r(-1, "2,17 for noon and 5pm");
		msg.r(-1, "2,17:7,14 for noon and 5pm on the 7th and 14th");
		call mssg(msg);
		stop();
	}

	if (runasusercode ne "NEOSYS" and not(runasusercode.xlate("USERS", 1, "X"))) {
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
		var ntasks = authtasks.count(VM) + 1;
		for (var taskn = 1; taskn <= ntasks; ++taskn) {
			var task = authtasks.a(1, taskn);
			if (not(authorised(task, msg, "", runasusercode))) {
				msg = "\"Runas\" user is not authorised to do " ^ task ^ FM ^ FM ^ msg;
				call mssg(msg);
				stop();
			}
		};//taskn;
	}

	var docid = alertid;

	gen.document = "";
	gen.document.r(30, testemail);
	gen.document.r(22, runhours.field(":", 1));
	gen.document.r(23, runhours.field(":", 2));
	gen.document.r(24, runhours.field(":", 3));
	gen.document.r(25, runhours.field(":", 4));
	gen.document.r(26, runhours.field(":", 5));
	gen.document.r(27, runhours.field(":", 6));
	//document<28>=
	gen.document.r(31, module);
	gen.document.r(32, alerttype);

	call autorun2("WRITE", title, module, request, datax, runasusercode, targetusercodes, gen.document, docid, msg);

	if (msg) {
		call mssg(msg);
		stop();
	}

	return 0;
}

libraryexit()
