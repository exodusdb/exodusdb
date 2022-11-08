#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <securitysubs.h>
#include <singular.h>
#include <sysmsg.h>
#include <usersubs.h>

#include <service_common.h>

#include <srv_common.h>
#include <req_common.h>

#include <request.hpp>

	var msg;
var xx;
var usern;	  // num
var newuser;  // num
var text;
var usercode;
var depts;
var reply;
var deptn;
var op;
var op2;
var wspos;
var wsmsg;

function main(in mode) {

	// $insert abp,common

	var users;
	if (not(users.open("USERS", ""))) {
		msg = "USERS file is missing";
		return invalid(msg);
	}

	if (mode eq "POSTREAD") {

		// no longer needed and prevents access to obsolete users
		// ie ones left in the users file after being deleted from auth table
		// gosub getusern
		// if valid else return

		// get email from userlist
		// it is in the user file already
		// if @id ne 'EXODUS' then @record<7>=userprivs<7,usern>

		if (req.wlocked and ID ne USERNAME) {

			// only exodus to access exodus
			if (ID eq "EXODUS") {
				msg = "Not authorised";
				return invalid(msg);
			}

			// maybe can only access self - now commented out in order to:
			// allow user update to people like timesheet administrator
			// without access granting authorisation access
			// and rely on USER UPDATE task
			// if security('AUTHORISATION ACCESS',msg) else goto invalid

			// maybe can only update self
			// if security('AUTHORISATION UPDATE',xx) else
			if (not(authorised("USER UPDATE", xx))) {
				req.srcfile.unlock(ID);
				req.wlocked = 0;
			}
		}

		// prevent new users with punctuation characters etc
		if (req.orec eq "" and req.wlocked) {
			var temp = ID;
			temp.converter(
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"0123456789",
				"");
			if (temp) {
				msg		= ID.quote() ^ " user doesnt exist and new usercodes";
				msg(-1) = "must be alphanumeric characters only";
				return invalid(msg);
			}
		}

		// password autoexpiry inform UI directly to allow warning
		// without an additional request to the server to get the expiry days
		let expirydays = SECURITY.f(25);
		if (expirydays) {
			// password date default to lastlogin date
			// if no login date then consider the account to have expired
			// and they would not have been able to login with it.
			if (not(RECORD.f(36))) {
				let lastlogindate = RECORD.f(13).field(".", 1);
				RECORD(36)		  = lastlogindate;
			}
			let expirydate = RECORD.f(36) + expirydays;
			RECORD(37)	   = expirydate;
		} else {
			// indicate no expiry
			RECORD(37) = "";
		}

	} else if (mode eq "VAL.EMAIL") {

		// assume email addresses in IS

		// adminstrator email addresses
		// tempting to also allow any domains held by admin emails but
		// those could have special priviliges to blackberry etc
		var sysemails = " " ^ SYSTEM.f(76);
		sysemails ^= " " ^ SYSTEM.f(84);
		sysemails ^= " " ^ SYSTEM.f(101);
		sysemails ^= " " ^ SYSTEM.f(102);
		sysemails.converter(";,", "  ");
		sysemails.trimmer();

		// check all email addresses versus domain and email restrictions
		// email domains is a space separated list of domains and/or email addresses
		var emaildomains = SYSTEM.f(116).lcase();
		if (emaildomains) {
			// admin *emails* (NOT DOMAINS) are specifically allowed
			emaildomains ^= " " ^ sysemails;
		} else {
			// if no specific domain restrictions then only admin DOMAINS are allowed
			if (sysemails) {

				// strip out the domain names
				let nn = sysemails.fcount(" ");
				for (const var ii : range(1, nn)) {
					var word = sysemails.field(" ", ii);
					word	 = field2(word, "@", -1);
					// remove smtp. mailout. etc from smtp host domain
					if (var("smtp,mail,mailout").locateusing(",", word.field(".", 1), xx)) {
						word = word.field(".", 2, 999);
					}
					sysemails = sysemails.fieldstore(" ", ii, 1, word);
				}  // ii;
				emaildomains ^= " " ^ sysemails;
			}
		}

		// always allow anything @neosys.com
		emaildomains ^= " neosys.com";

		if (emaildomains) {
			// check emails
			// emails=lcase(@record<7>)
			var emails = req.is.lcase();
			emails.converter(FM ^ VM ^ SM ^ ", ", ";;;;;");
			let nn = emails.fcount(";");
			for (const var ii : range(1, nn)) {
				let email = emails.field(";", ii);
				if (email) {
					let emaildomain = email.field("@", 2);
					if (not(emaildomains.locateusing(" ", emaildomain, xx))) {
						if (not(emaildomains.locateusing(" ", email, xx))) {
							msg = "Neither " ^ (emaildomain.quote()) ^ " nor " ^ (email.quote()) ^ "|is in the list of allowed email domains/addresses";
							return invalid(msg);
						}
					}
				}
			}  // ii;
		}

		// can be PREWRITE.RESETPASSWORD
	} else if (mode.field(".", 1) eq "PREWRITE") {

		var resetpassword = mode.field(".", 2) eq "RESETPASSWORD";

		if (SECURITY.read(DEFINITIONS, "SECURITY")) {
		}

		if (SECURITY.f(1).locate(ID, usern)) {
			newuser = 0;
			if (RECORD.f(4) and RECORD.f(4) ne req.orec.f(4)) {
				resetpassword = 1;
			}
		} else {
			newuser		  = 1;
			resetpassword = 0;
		}

		if (not(lockrecord("DEFINITIONS", DEFINITIONS, "SECURITY", xx, 0))) {
			if (resetpassword) {
				resetpassword = 2;
			} else {
				msg = "Somebody is updating the Authorisation File now. Please retry later";
				return invalid(msg);
			}
		}

		// get the latest userprivs
		if (not(SECURITY.read(DEFINITIONS, "SECURITY"))) {
			msg = "SECURITY is missing from DEFINITIONS";
			gosub unlocksec();
			call  sysmsg(msg);
			return invalid(msg);
		}

		let olduserprivs = SECURITY;

		// exodus usually not in the users list or shouldnt update security record
		if (ID eq "EXODUS") {
			return 0;
		}

		// prevent amendment of expiry date and password date in UI if not authorised
		// also name, email and department
		if (mode eq "PREWRITE") {
			if (req.orec) {
				if (not(authorised("AUTHORISATION UPDATE", xx))) {
					// expiry date
					if (req.orec.f(35)) {
						RECORD(35) = req.orec.f(35);
					}
					// password date
					if (req.orec.f(36)) {
						RECORD(36) = req.orec.f(36);
					}
					// tt=@record<19>
					// swap 'Default' with '' in tt
					// username
					RECORD(1) = req.orec.f(1);
					// department
					RECORD(5) = req.orec.f(5);
					// email
					RECORD(7) = req.orec.f(7);
					// department2
					RECORD(21) = req.orec.f(21);
				}
			}
		}

		// verify email domains
		req.is = RECORD.f(7);
		call usersubs("VAL.EMAIL");
		if (not(req.valid)) {
			gosub unlocksec();
			return 0;
		}

		// create new user in userprivs
		if (newuser) {

			let userdept = RECORD.f(21);
			if (not userdept) {
				msg = "USER GROUP CODE is required to create new users";
				gosub invalid(msg);
				gosub unlocksec();
				return 0;
			}

			if (not(SECURITY.f(1).locate(userdept, usern))) {
				msg = (userdept ^ " USER GROUP does not exist").quote();
				gosub invalid(msg);
				gosub unlocksec();
				return 0;
			}

			// insert an empty user
			for (const var fn : range(1, 9)) {
				SECURITY.inserter(fn, usern, "");
			}  // fn;

			let newusername		= RECORD.f(1);
			let newipnos		= RECORD.f(40);
			let newemailaddress = RECORD.f(7);

			SECURITY(1, usern) = ID;
			// userprivs<2,usern>=newkeys
			// userprivs<3,usern>=newexpirydate
			// userprivs<4,usern>=newpass
			// userprivs<5,usern>=newhourlyrate
			SECURITY(6, usern) = newipnos;
			SECURITY(7, usern) = newemailaddress;
			SECURITY(8, usern) = newusername;
		}

		gosub getusern();
		if (not(req.valid)) {
			gosub unlocksec();
			return 0;
		}

		if (resetpassword lt 2) {

			// email address
			SECURITY(7, usern) = RECORD.f(7);

			// user name
			SECURITY(8, usern) = RECORD.f(1);
		}

		// reencrypt new password
		var ans = RECORD.f(4);
		if ((newuser or resetpassword) and ans) {

			req.is = ID ^ FM ^ ans.f(1);
			// uses hasspass to encrypt
			call securitysubs("MAKESYSREC");
			ans = req.is;

			// save the encrypted bit in case we cannot update userprivs
			RECORD(4) = ans.f(7);

			// update security if managed to lock it
			if (resetpassword lt 2) {
				ans.converter(FM, TM);
				let tt			   = "<hidden>" ^ SM ^ ans;
				SECURITY(4, usern) = tt;
			}
		}

		if (resetpassword lt 2 and SECURITY ne olduserprivs) {
			SECURITY.write(DEFINITIONS, "SECURITY");
			// no need on user if on userprivs
			RECORD(4) = "";
		}

		// new password cause entry in users log to renable login if blocked
		// similar in security.subs and user.subs
		if (resetpassword or newuser) {
			// datetime=(date():'.':time() 'R(0)#5')+0
			let datetime = date() ^ "." ^ time().oconv("R(0)#5");
			RECORD.inserter(15, 1, datetime);
			RECORD.inserter(16, 1, SYSTEM.f(40, 2));

			if (USERNAME eq APPLICATION) {
				text = "OK New password sent to " ^ RECORD.f(7);
			} else {
				text = "OK New password set by " ^ USERNAME;
			}
			RECORD.inserter(18, 1, text);

			RECORD(36) = datetime;
		}

		// sort holidays in reverse order
		call sortarray(RECORD, 22 ^ VM ^ 23, "DR");

	} else if (mode eq "POSTWRITE") {

		gosub updatemirror();

		call unlockrecord("DEFINITIONS", DEFINITIONS, "SECURITY");

		return 0;

	} else if (mode eq "CREATEUSERNAMEINDEX") {
		req.srcfile = users;
		select(req.srcfile);

		while (true) {
			if (not(readnext(ID))) {
				ID = "*!%";
			}
			// /BREAK;
			if (not(ID ne "*!%"))
				break;
			if (not(ID.starts("%"))) {
				if (RECORD.read(users, ID)) {
					gosub updatemirror();
				}
			}
		}  // loop;

	} else if (mode.field(",", 1) eq "GETUSERDEPTX") {
		// does not popup any errormessage
		gosub getuserdept2(mode);
		if (ANS eq "") {
			ANS = "Deleted";
		}

	} else if (mode.field(",", 1) eq "GETUSERDEPT") {
		gosub getuserdept2(mode);
		if (ANS eq "") {
			msg = usercode.quote() ^ " - USER DOES NOT EXIST";
			return invalid(msg);
		}
		return 0;

	} else if (mode eq "GETDEPTS") {
		gosub getdepts();
		ANS = depts;

	} else {
		msg = mode.quote() ^ " is invalid in USER.SUBS";
		return invalid(msg);
	}

	return 0;
}

subroutine unlocksec() {
	call unlockrecord("DEFINITIONS", DEFINITIONS, "SECURITY");
	return;
}

subroutine getusern() {
	// fail safe only allowed to update existing users
	if (ID eq "EXODUS") {
		// usern remains unassigned to force an error if used later on
	} else {
		if (not(SECURITY.f(1).locate(ID, usern))) {
			msg = ID.quote() ^ " User does not exist";
			gosub invalid(msg);
			return;
		}
	}
	return;

	// similar code in user.subs and security.subs
}

subroutine updatemirror() {
	// save the user keyed as username too
	// because we save the user name and executive code in many places
	// and we still need to get the executive email if they are a user
	var mirror	  = RECORD.fieldstore(FM, 13, 5, "");
	mirror		  = RECORD.fieldstore(FM, 31, 3, "");
	let username  = RECORD.f(1).ucase();
	let mirrorkey = "%" ^ username ^ "%";
	mirror(1)	  = ID;
	mirror.write(req.srcfile, mirrorkey);
	return;
}

subroutine getuserdept2(in mode) {

	// locate the user in the table
	usercode = mode.field(",", 2);
	if (not(SECURITY.f(1).locate(usercode, usern))) {
		if (usercode eq "EXODUS") {
			ANS = "EXODUS";
			return;
		} else {
			ANS = "";
			return;
		}
	}

	// locate divider, or usern+1
	let nusers1 = SECURITY.f(1).fcount(VM);
	for (usern += 1; usern <= nusers1; ++usern) {
		// /BREAK;
		if (SECURITY.f(1, usern) eq "---")
			break;
	}  // usern;

	// get the department code
	ANS = SECURITY.f(1, usern - 1);
	return;
}

subroutine getdepts() {
	depts		= "";
	let nusers2 = SECURITY.f(1).fcount(VM);
	for (usern = 2; usern <= nusers2 + 1; ++usern) {
		text = SECURITY.f(1, usern);
		if (text eq "---" or text eq "") {
			text = SECURITY.f(1, usern - 1);
			text.converter("0123456789", "");
			text.trimmer();
			if (text and text ne "---") {
				if (not(depts.locateusing(FM, text, deptn))) {
					depts(-1) = text;
				}
			}
		}
	}  // usern;

	return;
}

libraryexit()
