#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <sysmsg.h>
#include <securitysubs.h>
#include <sortarray.h>
#include <usersubs.h>

#include <gen.h>
#include <fin.h>
#include <win.h>

var msg;
var xx;
var usern;
var text;
var wsmsg;

#include <window.hpp>

function main(in mode) {

	var users;
	if (not(users.open("USERS", ""))) {
		msg = "USERS file is missing";
		return invalid(msg);
	}

	if (mode == "POSTREAD") {

		//no longer needed and prevents access to obsolete users
		//ie ones left in the users file after being deleted from auth table
		//gosub getusern
		//if valid else return 0

		//get email from userlist
		//it is in the user file already
		//if @id ne 'NEOSYS' then @record<7>=userprivs<7,usern>

		if (win.wlocked and ID ne USERNAME) {

			//only neosys to access neosys
			if (ID == "NEOSYS") {
				msg = "Not authorised";
				return invalid(msg);
			}

			//maybe can only access self - now commented out in order to:
			//allow user update to people like timesheet administrator
			//without access granting authorisation access
			//and rely on USER UPDATE task
			//if security('AUTHORISATION ACCESS',msg) else goto invalid

			//maybe can only update self
			//if security('AUTHORISATION UPDATE') else
			if (not(authorised("USER UPDATE", xx))) {
				win.srcfile.unlock( ID);
				win.wlocked = 0;
			}

		}

		//prevent new users with punctuation characters etc
		if (win.orec == "" and win.wlocked) {
			var temp = ID;
			temp.converter(UPPERCASE ^ "0123456789", "");
			if (temp) {
				msg = DQ ^ (ID ^ DQ) ^ " user doesnt exist and new usercodes";
				msg.r(-1, "must be alphanumeric characters only");
				return invalid(msg);
			}
		}

		//password autoexpiry inform UI directly to allow warning
		//without an additional request to the server to get the expiry days
		var expirydays = gen._security.a(25);
		if (expirydays) {
			//password date default to lastlogin date
			//if no login date then consider the account to have expired
			//and they would not have been able to login with it.
			if (not RECORD.a(36)) {
				var lastlogindate = RECORD.a(13).field(".", 1);
				RECORD.r(36, lastlogindate);
			}
			var expirydate = RECORD.a(36) + expirydays;
			RECORD.r(37, expirydate);
		}else{
			//indicate no expiry
			RECORD.r(37, "");
		}

	} else if (mode == "VAL.EMAIL") {

		//assume email addresses in win.is

		//adminstrator email addresses
		//tempting to also allow any domains held by admin emails but
		//those could have special priviliges to blackberry etc
		var sysemails = " " ^ SYSTEM.a(76);
		sysemails ^= " " ^ SYSTEM.a(84);
		sysemails ^= " " ^ SYSTEM.a(101);
		sysemails ^= " " ^ SYSTEM.a(102);
		sysemails.converter(";,", "  ");
		sysemails.trimmer();

		//check all email addresses versus domain and email restrictions
		//email domains is a space separated list of domains and/or email addresses
		var emaildomains = SYSTEM.a(116).lcase();
		if (emaildomains) {
			//admin *emails* (NOT DOMAINS) are specifically allowed
			emaildomains ^= " " ^ sysemails;
		}else{
			//if no specific domain restrictions then only admin DOMAINS are allowed
			if (sysemails) {

				//strip out the domain names
				var nn = sysemails.count(" ") + 1;
				for (var ii = 1; ii <= nn; ++ii) {
					var word = sysemails.field(" ", ii);
					word = field2(word, "@", -1);
					//remove smtp. mailout. etc from smtp host domain
					if (var("smtp,mail,mailout").locateusing(word.field(".", 1), ",", xx)) {
						word = word.field(".", 2, 999);
					}
					sysemails = sysemails.fieldstore(" ", ii, 1, word);
				};//ii;
				emaildomains ^= " " ^ sysemails;
			}
		}

		if (emaildomains) {
			//check emails
			//emails=lcase(@record<7>)
			var emails = win.is.lcase();
			emails.converter(FM ^ VM ^ SVM ^ ", ", ";;;;;");
			var nn = emails.count(";") + 1;
			for (var ii = 1; ii <= nn; ++ii) {
				var email = emails.field(";", ii);
				if (email) {
					var emaildomain = email.field("@", 2);
					if (not(emaildomains.locateusing(emaildomain, " ", xx))) {
						if (not(emaildomains.locateusing(email, " ", xx))) {
							msg = DQ ^ (emaildomain ^ DQ) ^ " of " ^ (DQ ^ (email ^ DQ)) ^ " is not in the list of allowed email domains/addresses";
							return invalid(msg);
						}
					}
				}
			};//ii;
		}

	//can be PREWRITE.RESETPASSWORD
	} else if (mode.field(".", 1) == "PREWRITE") {

		var resetpassword = mode.field(".", 2) == "RESETPASSWORD" or ( RECORD.a(4) and RECORD.a(4) ne win.orec.a(4));

		//neosys usually not in the users list or shouldnt update security record
		if (ID == "NEOSYS") {
			return 0;
		}

		if (not(lockrecord("DEFINITIONS", gen._definitions, "SECURITY", xx, 0))) {
			if (resetpassword) {
				resetpassword = 2;
			}else{
				msg = "Somebody is updating the Authorisation File now. Please retry later";
				return invalid(msg);
			}
		}

		//prevent amendment of expiry date and password date in UI
		if (mode == "PREWRITE") {
			if (win.orec.a(35)) {
				RECORD.r(35, win.orec.a(35));
			}
			if (win.orec.a(36)) {
				RECORD.r(36, win.orec.a(36));
			}
			var tt = RECORD.a(19);
			tt.swapper("Default", "");
		}

		//verify email domains
		win.is = RECORD.a(7);
		call usersubs("VAL.EMAIL");
		if (not win.valid) {
			goto unlocksecurity;
		}

		gosub getusern();
		if (not win.valid) {
unlocksecurity:
			call unlockrecord("DEFINITIONS", gen._definitions, "SECURITY");
			return 0;
		}

		//get the latest userprivs
		if (not(gen._security.read(gen._definitions, "SECURITY"))) {
			msg = "SECURITY is missing from DEFINITIONS";
			call unlockrecord("DEFINITIONS", gen._definitions, "SECURITY");
			call sysmsg(msg);
			return invalid(msg);
		}
		//gen._security = gen._security.invert();

		var olduserprivs = gen._security;

		if (resetpassword < 2) {

			//email address
			gen._security.r(7, usern, RECORD.a(7));

			//user name
			gen._security.r(8, usern, RECORD.a(1));

		}

		//reencrypt new password
		ANS = RECORD.a(4);
		if (resetpassword and ANS) {

			win.is = ID ^ FM ^ ANS.a(1);
			call securitysubs("MAKESYSREC");
			ANS = win.is;

			//save the encrypted bit in case we cannot update userprivs
			RECORD.r(4, ANS.a(7));

			//update security if managed to lock it
			if (resetpassword < 2) {
				ANS.converter(FM, TM);
				var tt = "<hidden>" ^ SVM ^ ANS;
				gen._security.r(4, usern, tt);
			}

		}

		if (resetpassword < 2 and gen._security ne olduserprivs) {
			//gen._security.invert().write(gen._definitions, "SECURITY");
			gen._security.write(gen._definitions, "SECURITY");
			//no need on user if on userprivs
			RECORD.r(4, "");
		}

		//new password cause entry in users log to renable login if blocked
		//similar in security.subs and user.subs
		if (resetpassword) {
			//datetime=(date():'.':time() 'R(0)#5')+0
			var datetime = var().date() ^ "." ^ (var().time()).oconv("R(0)#5");
			RECORD.inserter(15, 1, datetime);
			RECORD.inserter(16, 1, SYSTEM.a(40, 2));

			if (USERNAME == APPLICATION) {
				text = "OK New password sent to " ^ RECORD.a(7);
			}else{
				text = "OK New password set by " ^ USERNAME;
			}
			RECORD.inserter(18, 1, text);

			RECORD.r(36, datetime);
		}

		//sort holidays in reverse order
		call sortarray(RECORD, 22 ^ VM ^ 23, "D", "R");

	} else if (mode == "POSTWRITE") {

		gosub updatemirror();

		call unlockrecord("DEFINITIONS", gen._definitions, "SECURITY");

	} else if (mode == "CREATEUSERNAMEINDEX") {
		win.srcfile = users;
		win.srcfile.select();

		while (win.srcfile.readnext(ID)) {
			if (not(ID[1] == "%")) {
				if (RECORD.read(users, ID)) {
					gosub updatemirror();
				}
			}
		}

	} else {
		msg = DQ ^ (mode ^ DQ) ^ " is invalid in USER.SUBS";
		return invalid(msg);

	}

	return 0;

}

subroutine getusern() {
	//fail safe only allowed to update existing users
	if (ID == "NEOSYS") {
		//usern remains unassigned to force an error if used later on
	}else{
		if (not(gen._security.locate(ID, usern, 1))) {
			msg = DQ ^ (ID ^ DQ) ^ " User does not exist";
			gosub invalid(msg);
		}
	}
	return;

	//similar code in user.subs and security.subs
}

subroutine updatemirror() {
	//save the user keyed as username too
	//because we save the user name and executive code in many places
	//and we still need to get the executive email if they are a user
	var mirror = RECORD.fieldstore(FM, 13, 5, "");
	mirror = RECORD.fieldstore(FM, 31, 3, "");
	var username = RECORD.a(1).ucase();
	var mirrorkey = "%" ^ username ^ "%";
	mirror.r(1, ID);//name on mirror is used to store usercode
	mirror.write(win.srcfile, mirrorkey);
	return;

}


libraryexit()
