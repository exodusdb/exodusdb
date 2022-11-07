#include <exodus/library.h>
libraryinit()

#include <emailusers.h>
#include <sendmail.h>
#include <sysmsg.h>

#include <service_common.h>

#include <srv_common.h>

var mode;
var subject;
var body;
var nsent;	// num
var replyto;
var toemails;
var ccemails;
var usercode;
var userx;
var expirydate;
var emails;
var ok;	 // num
var xx;
var nn;
var errormsg;

function main(in mode0, in subject0, in body0, in groupids0, in /*jobids0*/, in userids0, in options, io emaillog) {

	var interactive = false;  //not(SYSTEM.f(33));

	// options
	// R = REPLYTO=@username email address if exists
	// W = Groups by Word eg user with dept MEDIA BUYING matches group MEDIA

	if (mode.unassigned()) {
		var mod = "";
	} else {
		mode = mode0;
	}

	if (SENTENCE.field(" ", 1) eq "EMAILUSERS") {

		mode		= SENTENCE.field(" ", 2);
		var version = SENTENCE.field(" ", 3, 9999);
		SENTENCE	= "";

		if (mode ne "UPGRADE") {
			var msg = mode.quote() ^ " is invalid in EMAILUSERS";
			if (interactive) {
				call mssg(msg);
			} else {
				call sysmsg(msg);
			}
			stop();
		}

		subject = "EXODUS Upgrade: " ^ SYSTEM.f(23);
		if (SYSTEM.f(17) ne SYSTEM.f(23)) {
			subject ^= " (" ^ SYSTEM.f(17) ^ ")";
		}

		body = "";
		body ^= "The EXODUS system software has been upgraded.";
		// body:=vm
		// body:=vm:'In case of errors, please follow the instructions at'
		// body:=vm:'http://userwiki.neosys.com/index.php/cache.'
		body ^= VM;
		body ^= VM ^ "Please email SUPPORT@EXODUS.COM for any assistance.";
		// body:=vm
		// body:=vm:'This is an automated email. You cannot reply to it.'
		body.converter(VM, chr(13));

		// call emailusers(mode,subject,body,'','','','R',emaillog)
		// no more replyto support@neosys.com because no longer needed
		// because installations sender emails like clientxyz@neosys.com
		// are configured as aliases of support@neosys.com
		// and replyto option suppresses the email header
		// urn:schemas:mailheader:X-Auto-Response-Suppress = RN, NRN, OOF
		// thereby allowing Out of Office replies which we do not want
		call emailusers(mode, subject, body, "", "", "", "", emaillog);

		if (not emaillog) {
			emaillog = "(nobody)";
		}
		emaillog = "Upgrade Notification emailed to:" ^ VM ^ emaillog;
		emaillog.replacer(VM, chr(13));
		call sysmsg(emaillog, "Upgrade to version " ^ version);

		stop();
	}

	// init:
	// ///
	// if target and options='' or index(options,'U',1) then
	// end
	bool groupword = options.contains("W");
	nsent		   = 0;

	subject = subject0;
	body	= body0;

	// read fromuser from users,@username else fromuser=''
	replyto = "";
	if (options.contains("R")) {
		if ((USERNAME eq "EXODUS" or USERNAME eq "ADAGENCY") or USERNAME eq "ACCOUNTS") {
			replyto = "support@neosys.com";
		} else {
			var fromuser = xlate("USERS", USERNAME, "", "X");
			replyto		 = fromuser.f(7);
			var fromline = "From " ^ fromuser.f(1);
			if (USERNAME ne fromuser.f(1)) {
				fromline ^= " (" ^ USERNAME ^ ")";
			}
			subject.prefixer(fromline ^ " : ");
		}
	}

	var usercodes	   = SECURITY.f(1);
	var nusers		   = usercodes.fcount(VM);
	var usern		   = 0;
	emaillog		   = "";
	var alreadyemailed = "";
	body.converter(FM ^ VM, chr(13) ^ chr(13));

	var groupids = groupids0;
	groupids.converter(",", VM);
	let ngroups = groupids.fcount(VM);

	var userids = userids0;
	userids.converter(",", VM);

	toemails	 = "";
	ccemails	 = "";
	var currdept = "";

	var users;
	if (not(users.open("USERS", ""))) {
		call mssg(lasterror());
		return 0;
	}

// ///////
nextuser:
	// ///////
	if (esctoexit()) {
		goto exit;
	}

	usern += 1;
	if (usern gt nusers) {
		goto exit;
	}

	// skip empty users
	usercode = usercodes.f(1, usern);
	if (usercode eq "") {
		goto nextuser;
	}

	// only users on file
	if (not(userx.read(users, usercode))) {
		goto nextuser;
	}

	// not expired users
	expirydate = userx.f(35);
	if (expirydate and expirydate le date()) {
		goto nextuser;
	}

	// skip users with no email at all
	// users may have 0 or more email addresses eg xyz@abc.com;123@345.com etc
	emails = userx.f(7);
	if (emails eq "") {
		goto nextuser;
	}

	// always email to self last
	if (usercode eq USERNAME and replyto) {
		goto nextuser;
	}

	ok = 0;
	if (not(ok) and userids) {
		if (userids.locate(usercode, xx)) {
			ok = 1;
		}
		if (not(ok) and not(groupids)) {
			goto nextuser;
		}
	}

	// skip users not of required type (eg FINANCE is ok in FINANCE CONTROLLER)
	// could determine user type from what menus they can access eg MEDIA
	if (not(ok) and groupids) {
		if (groupword) {
			// eq search for MEDIA in user department like MEDIA BUYER
			for (const var groupn : range(1, ngroups)) {
				ok = userx.f(5).contains(groupids.f(1, groupn));
				// /BREAK;
				if (not(not(ok)))
					break;
			}  // groupn;
		} else {
			// exact groups
			if (groupids.locate(userx.f(21), xx)) {
				ok = 1;
			}
		}
		if (not ok) {
			goto nextuser;
		}
	}

	// must be last to avoid adding emails to sent list unless actually sent
	// remove any emails that have already been emailed before
	emails.converter(" ", "");
	emails.converter(";", VM);
	nn = emails.fcount(VM);
	for (var ii = nn; ii >= 1; --ii) {
		var email = emails.f(1, ii);
		if (alreadyemailed.locate(email, xx)) {
			emails.remover(1, ii);
		} else {
			alreadyemailed ^= VM ^ email;
		}
	}  // ii;
	emails.converter(VM, ";");

	// skip users that have already been emailed before
	if (emails eq "") {
		goto nextuser;
	}

	// userinit:
	// ///////
	print(usercode, " ");

	if (currdept and userx.f(5) ne currdept) {
		gosub sendemails(emaillog);
	}
	currdept = userx.f(5);

	if (toemails eq "") {
		toemails = emails;

		// sending to users (or groups and users)
	} else if (userids) {
		toemails ^= ";" ^ emails;

		// sending to groups then to the first and cc the rest
	} else {
		ccemails ^= ";" ^ emails;
	}

	emaillog ^= VM ^ usercode ^ " " ^ emails;

	// userexit:
	// ///////
	goto nextuser;

exit:
	// ///
	gosub sendemails(emaillog);

	while (true) {
		// /BREAK;
		if (not(("." ^ VM ^ FM).contains(emaillog[-1])))
			break;
		emaillog.popper();
	}  // loop;

	// always email to self
	if (nsent and replyto) {
		toemails = replyto;
		body ^= VM ^ VM ^ "-- Sent to --" ^ emaillog;
		body.replacer(VM, chr(13));
		gosub sendemails(emaillog);
	}

	// no longer suppress sendmail errors
	// since this hides problems with email server/configuration
	// if nsent else
	// emaillog=''
	// end

	while (true) {
		// /BREAK;
		if (not(("." ^ VM ^ FM).contains(emaillog[-1])))
			break;
		emaillog.popper();
	}  // loop;

	return 0;
}

subroutine sendemails(io emaillog) {

	if (not toemails) {
		return;
	}

	if (ccemails.starts(";")) {
		ccemails.cutter(1);
	}

	call sendmail(toemails, ccemails, subject, body, "", "", errormsg, replyto);

	if (errormsg and errormsg ne "OK") {
		emaillog ^= VM ^ errormsg;
	} else {
		nsent += 1;
	}

	emaillog ^= VM;

	toemails = "";
	ccemails = "";

	return;
}

libraryexit()