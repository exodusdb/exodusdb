#include <exodus/library.h>
libraryinit()

#include <sysmsg.h>
#include <sendmail.h>
#include <emailusers.h>

#include <gen.h>

var xx;

var toemails;
var ccemails;
var subject;
var body;
var nsent;

function main(in mode, in subject0, in body0, in groupids0, in jobids0, in userids0, in options, io emaillog) {

	//options
	//R = REPLYTO=@username email address if exists
	//W = Groups by Word eg user with dept MEDIA BUYING matches group MEDIA

	var interactive = not SYSTEM.a(33);

	if (false) print(jobids0);//evade compiler warning on unused argument

	if (mode.field(" ", 1) eq "UPGRADE") {

		var subject = "NEOSYS Upgrade: " ^ SYSTEM.a(23);
		if (SYSTEM.a(17) ne SYSTEM.a(23)) {
			subject ^= " (" ^ SYSTEM.a(17) ^ ")";
		}
		var version = mode.field(" ", 2);
		subject ^= version;

		var body = "";
		body ^= "The NEOSYS system software has been upgraded.";
		body ^= VM;
		body ^= VM ^ "Before you login to NEOSYS, please follow the instructions at";
		body ^= VM ^ "http://userwiki.neosys.com/index.php/cache to avoid errors using NEOSYS.";
		body ^= VM;
		body ^= VM ^ "Please email SUPPORT@NEOSYS.COM for any assistance.";
		//body:=vm
		//body:=vm:'This is an automated email. You cannot reply to it.'
		body.converter(VM, var().chr(13));

		call emailusers(mode, subject, body, "", "", "", "R", emaillog);

		if (not emaillog) {
			emaillog = "(nobody)";
		}
		emaillog = "Upgrade Notification emailed to:" ^ VM ^ emaillog;
		emaillog.swapper(VM, var().chr(13));

		call sysmsg(emaillog, "Upgrade to version " ^ version);

		return 0;

	} else if (mode ne "") {
		var msg = DQ ^ (mode ^ DQ) ^ " is invalid in EMAILUSERS";
		if (interactive) {
			call mssg(msg);
		}else{
			call sysmsg(msg);
		}
		return 1;
	}

//init:

	//if target and options='' or index(options,'U',1) then
	// end
	var groupword = options.index("W", 1);
	nsent = 0;

	var subject = subject0;
	var body = body0;

	//read fromuser from users,@username else fromuser=''
	var replyto = "";
	if (options.index("R", 1)) {
		if (USERNAME == "NEOSYS" or USERNAME == "ADAGENCY" or USERNAME == "ACCOUNTS") {
			replyto = "support@neosys.com";
		}else{
			var fromuser = USERNAME.xlate("USERS", "", "X");
			replyto = fromuser.a(7);
			var fromline = "From " ^ fromuser.a(1);
			if (USERNAME ne fromuser.a(1)) {
				fromline ^= " (" ^ USERNAME ^ ")";
			}
			subject.splicer(1, 0, fromline ^ " : ");
		}
	}

	var usercodes = gen._security.a(1);
	var nusers = usercodes.count(VM) + 1;
	var usern = 0;
	emaillog = "";
	var alreadyemailed = "";
	body.converter(FM ^ VM, var().chr(13) ^ var().chr(13));

	var groupids = groupids0;
	groupids.converter(",", VM);
	var ngroups = groupids.count(VM) + 1;

	var userids = userids0;
	userids.converter(",", VM);

	toemails = "";
	ccemails = "";

	var currdept = "";

	var users;
	if (not(users.open("USERS", ""))) {
		call fsmsg();
		return 0;
	}

	for (usern=1; usern<=nusers;++usern) {

		//interrupt
		if (esctoexit()) {
			break;
		}

		//skip empty users
		var usercode = usercodes.a(1, usern);
		if (usercode == "") {
			continue;
		}

		//only users on file
		var USER;
		if (not(USER.read(users, usercode))) {
			continue;
		}

		//not expired users
		var expirydate = USER.a(35);
		if (expirydate and expirydate <= var().date()) {
			continue;
		}

		//skip users with no email at all
		//users may have 0 or more email addresses eg xyz@abc.com;123@345.com etc
		var emails = USER.a(7);
		if (emails == "") {
			continue;
		}

		//always email to self last
		if (usercode == USERNAME and replyto) {
			continue;
		}

		var ok = 0;
		if (not ok and userids) {
			if (userids.locate(usercode, xx, 1)) {
				ok = 1;
			}
			if (not ok and not groupids) {
				continue;
			}
		}

		//skip users not of required type (eg FINANCE is ok in FINANCE CONTROLLER)
		//could determine user type from what menus they can access eg MEDIA
		if (not ok and groupids) {
			if (groupword) {
				//eq search for MEDIA in user department like MEDIA BUYER
				for (var groupn = 1; groupn <= ngroups; ++groupn) {
					ok = (USER.a(5)).index(groupids.a(1, groupn), 1);

					if (not(not ok))
						break;;
				};//groupn;
			}else{
				//exact groups
				if (groupids.locate(USER.a(21), xx, 1)) {
					ok = 1;
				}
			}
			if (not ok) {
				continue;
			}
		}

		//must be last to avoid adding emails to sent list unless actually sent
		//remove any emails that have already been emailed before
		emails.converter(" ", "");
		emails.converter(";", VM);
		var nn = emails.count(VM) + (emails ne "");
		for (var ii = nn; ii >= 1; --ii) {
			var email = emails.a(1, ii);
			if (alreadyemailed.locateusing(email, VM)) {
				emails.eraser(1, ii);
			}else{
				alreadyemailed ^= VM ^ email;
			}
		};//ii;
		emails.converter(VM, ";");

		//skip users that have already been emailed before
		if (emails == "") {
			continue;
		}

//userinit:

		print(usercode, " ");

		if (currdept and USER.a(5) ne currdept) {
			gosub sendemails(emaillog);
		}
		currdept = USER.a(5);

		if (toemails == "") {
			toemails = emails;

		//sending to users (or groups and users)
		} else if (userids) {
			toemails ^= ";" ^ emails;

		//sending to groups then to the first and cc the rest
		}else{
			ccemails ^= ";" ^ emails;
		}

		emaillog ^= VM ^ usercode ^ " " ^ emails;

	}//usern

//exit:
	gosub sendemails(emaillog);

	//trim trailing . vm and fm
	emaillog.trimmerb("."^VM^FM);

	//always email to self
	if (nsent and replyto) {
		toemails = replyto;
		body ^= VM ^ VM ^ "-- Sent to --" ^ emaillog;
		body.swapper(VM, var().chr(13));
		gosub sendemails(emaillog);
	}

	if (not nsent) {
		emaillog = "";
	}

	//trim trailing . vm and fm
	emaillog.trimmerb("."^VM^FM);

	return 0;

}

subroutine sendemails(io emaillog) {

	var errormsg;

	if (not toemails) {
		return;
	}

	ccemails.trimmerf(";");

	call sendmail(toemails, ccemails, subject, body, "", "", errormsg);

	if (errormsg and errormsg ne "OK") {
		emaillog ^= VM ^ errormsg;
	}else{
		nsent += 1;
	}

	emaillog ^= VM;

	toemails = "";
	ccemails = "";

	return;

}


libraryexit()
