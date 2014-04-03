#include <exodus/library.h>
libraryinit()

#include <getsubs.h>

#include <gen.h>

var usern;//num

function main(in mode, in title0, in module, in request, in data, in runasusercode0, in targetusercodes0, io document0, io docid, out msg) {

	var runasusercode = runasusercode0;
	var targetusercodes = targetusercodes0;
	var document;
	var title;
	if (document0.unassigned())
		document = "";
	else
		document = document0;
	if (title0)
		title = title0;
	else
		title = document.a(1);

	//creates an autorun record in documents

	//default runtime to once
	if (not gen.document.field(FM, 20, 10).convert(FM, "")) {
		//run once now
		gen.document.r(27, 1);
	}

	var fullrequest = module ^ "PROXY" ^ FM ^ request;

	//get target usercodes

	var users;
	if (not(users.open("USERS", ""))) {
		msg = "USERS file is missing";
		return 0;
	}

	//determine the runas usercode if not specified
	if (not runasusercode) {
		//allow for runuser to be passed in document
		runasusercode = gen.document.a(1);
	}
	if (runasusercode) {
		var runasuser;
		if (not(runasuser.read(users, runasusercode))) {
			if (not(runasusercode == "NEOSYS")) {
				msg = DQ ^ (runasusercode ^ DQ) ^ " user doesnt exist";
				return 0;
			}
			runasuser = "";
		}
	}else{

		//run as the bottom user in the current users group
		//assuming we are going to send it to all members of the group
		var usercode = USERNAME;
		runasusercode = usercode.xlate("USERS", 5, "");

		//if not a proper user (eg NEOSYS then skip)
		if (not runasusercode) {
			msg = DQ ^ (usercode ^ DQ) ^ " can only autorun if specified as the runas user";
			return 0;
		}

	}

	//if no targets specified then
	//send to the "run as" user and all users above in the same group
	if (targetusercodes == "") {
		//allow for targetusers to be passed in document
		targetusercodes = gen.document.a(14);
		//runasuser is NOT a target unless there are no recipients
		//if targetusercodes='' then
		// targetusercodes=runasusercode
		// end
	}

	//auto targeting
	if (targetusercodes == "{GROUP}") {
		var tt = runasusercode;
		if (SECURITY.locate(tt, usern, 1)) {
			while (true) {
				var USER;
				if (USER.read(users, tt)) {
					//only to users with emails
					if (USER.a(7)) {
						var seniorfirst = 1;
						if (seniorfirst) {
							targetusercodes.inserter(1, 1, tt);
						}else{
							targetusercodes.r(1, -1, tt);
						}
					}
				}
				usern -= 1;
				tt = SECURITY.a(1, usern);
			///BREAK;
			if (not(usern and tt and tt ne "---")) break;;
			}//loop;
		}
	}else{

		//check all users exist (even if they dont have emails)
		if (targetusercodes) {
			var nusers = targetusercodes.count(VM) + 1;
			for (var usern = nusers; usern >= 1; --usern) {
				var usercode = targetusercodes.a(1, usern);
				var USER;
				if (not(USER.read(users, usercode))) {
					if (not(usercode == "NEOSYS")) {
						var msg = DQ ^ (usercode ^ DQ) ^ " user doesnt exist";
						return 0;
					}
					USER = "";
				}
			};//usern;
		}

	}

	//if no targets have emails then skip
	if (targetusercodes0 and targetusercodes == "") {
		var msg = "No target users have email addresses";
		return 0;
	}

	//got all data, prepare autorun document

	gen.document.r(2, title);
	gen.document.r(5, lower(fullrequest));
	gen.document.r(6, lower(data));
	gen.document.r(1, runasusercode);
	gen.document.r(14, targetusercodes);

	gen.document.r(7, APPLICATION);
	gen.document.r(3, var().date());
	gen.document.r(4, var().time());
	if (gen.document.a(12) == "") {
		gen.document.r(12, 1);
	}

	//email delivery - save if report successful or not run
	if (docid) {
		//dont lose last run date/time to avoid rerunning.
		//to force rerun delete and recreate
		var olddoc;
		if (olddoc.read(gen.documents, docid)) {
			gen.document.r(13, olddoc.a(13));
		}
	}else{
		var saveid = ID;
		call getsubs("DEF.DOCUMENT.NO");
		docid = ID;
		ID = saveid;
		if (not docid) {
			var msg = "Could not get next document number in AUTORUN2|Please try again after 1 minute";
			return 0;
		}
	}

	//prevent document from being run until the request has been processed
	//relies on unlock all in listen
	if (mode == "ASAP") {
		if (not(gen.documents.lock( docid))) {
			{}
		}
	}

	gen.document.write(gen.documents, docid);

	return 1;

}


libraryexit()
