#include <exodus/library.h>
libraryinit()

#include <getsubs.h>

#include <service_common.h>

#include <srv_common.h>

var mode;
var datax;
var runasusercode;
var targetusercodes;
var title;
var usern;	// num

function main(in mode0, in title0, in module, in request, in data0, in runasusercode0, in targetusercodes0, io document0, io docid, out msg) {

	// creates an autorun record in documents
	// GBP CREATEALERT mode WRITE
	// GBP GENERAL.SUBS2 mode ASAP
	// BP MEDIADIARY mode WRITE/ASP for media diary
	// ABP none

	// default unassigned parameters
	if (mode0.unassigned()) {
		mode = "";
	} else {
		mode = mode0;
	}
	if (data0.unassigned()) {
		datax = "";
	} else {
		datax = data0;
	}
	if (runasusercode0.unassigned()) {
		runasusercode = "";
	} else {
		runasusercode = runasusercode0;
	}
	if (targetusercodes0.unassigned()) {
		targetusercodes = "";
	} else {
		targetusercodes = targetusercodes0;
	}
	if (document0.unassigned()) {
		srv.document = "";
	} else {
		srv.document = document0;
	}
	if (docid.unassigned()) {
		docid = "";
	}
	if (title0) {
		title = title0;
	} else {
		title = srv.document.f(1);
	}

	// default runtime to once
	if (not(srv.document.field(FM, 20, 10).convert(FM, ""))) {
		// run once now
		srv.document(27) = 1;
	}

	let fullrequest = module ^ "PROXY" ^ FM ^ request;

	// get target usercodes

	var users;
	if (not users.open("USERS", "")) {
		msg = "USERS file is missing";
		return 0;
	}

	// determine the runas usercode if not specified
	if (not runasusercode) {
		// allow for runuser to be passed in document
		runasusercode = srv.document.f(1);
	}
	if (runasusercode) {
		var runasuser;
		if (not runasuser.read(users, runasusercode)) {
			if (not(runasusercode == "EXODUS")) {
				msg = runasusercode.quote() ^ " user doesnt exist";
				return 0;
			}
			runasuser = "";
		}
	} else {

		// run as the bottom user in the current users group
		// assuming we are going to send it to all members of the group
		let usercode  = USERNAME;
		runasusercode = usercode.xlate("USERS", 5, "");

		// if not a proper user (eg EXODUS then skip)
		if (not runasusercode) {
			msg = usercode.quote() ^ " can only autorun if specified as the runas user";
			return 0;
		}
	}

	// if no targets specified then
	// send to the "run as" user and all users above in the same group
	if (targetusercodes == "") {
		// allow for targetusers to be passed in document
		targetusercodes = srv.document.f(14);
		// runasuser is NOT a target unless there are no recipients
		// if targetusercodes='' then
		// targetusercodes=runasusercode
		// end
	}

	// auto targeting
	let initialtargetusercodes = targetusercodes;
	if (targetusercodes == "{GROUP}") {
		var tt = runasusercode;
		if (SECURITY.f(1).locate(tt, usern)) {
			while (true) {
				var userx;
				if (userx.read(users, tt)) {
					// only to users with emails
					if (userx.f(7)) {
						let seniorfirst = 1;
						if (seniorfirst) {
							targetusercodes.inserter(1, 1, tt);
						} else {
							targetusercodes(1, -1) = tt;
						}
					}
				}
				usern -= 1;
				tt = SECURITY.f(1, usern);
				// /BREAK;
				if (not((usern and tt) and tt != "---"))
					break;
			}  // loop;
		}
	} else {

		// check all users exist (even if they dont have emails)
		if (targetusercodes) {
			let nusers = targetusercodes.fcount(VM);
			for (usern = nusers; usern >= 1; --usern) {
				let usercode = targetusercodes.f(1, usern);
				var userx;
				if (not userx.read(users, usercode)) {
					if (not(usercode == "EXODUS")) {
						msg = usercode.quote() ^ " user doesnt exist";
						return 0;
					}
					userx = "";
				}
			}  // usern;
		}
	}

	// if no targets have emails then skip
	if (initialtargetusercodes and targetusercodes == "") {
		msg = "No target users have email addresses";
		return 0;
	}

	// got all data, prepare autorun document

	srv.document(2)	 = title;
	srv.document(5)	 = lower(fullrequest);
	srv.document(6)	 = lower(datax);
	srv.document(1)	 = runasusercode;
	srv.document(14) = targetusercodes;

	srv.document(7) = APPLICATION;
	srv.document(3) = date();
	srv.document(4) = time();
	if (srv.document.f(12) == "") {
		srv.document(12) = 1;
	}

	// email delivery - save if report successful or not run
	if (docid) {
		// dont lose last run date/time to avoid rerunning.
		// to force rerun delete and recreate
		var olddoc;
		if (olddoc.read(srv.documents, docid)) {
			srv.document(13) = olddoc.f(13);
		}
	} else {
		var	 saveid = ID;
		call getsubs("DEF.DOCUMENT.NO");
		docid = ID;
		ID	  = saveid;
		if (not docid) {
			msg = "Could not get next document number in AUTORUN2|Please try again after 1 minute";
			return 0;
		}
	}

	// prevent document from being run until the request has been processed
	// relies on unlock all in listen
	if (mode == "ASAP") {
		if (not(srv.documents.lock(docid))) {
			// ignore and proceed even cannot lock the document. why?
		}
	}

	srv.document.write(srv.documents, docid);

	return 1;
}

libraryexit()
