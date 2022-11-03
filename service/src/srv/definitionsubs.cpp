#include <exodus/library.h>
libraryinit()

#include <anydata.h>
#include <authorised.h>
#include <collectixvals.h>
#include <definitionsubs.h>
#include <securitysubs.h>
#include <shell2.h>
#include <singular.h>
#include <systemsubs.h>

#include <service_common.h>

#include <srv_common.h>
#include <req_common.h>

#include <request.hpp>

var xx;
var op;
var msg;
var backuprec;
var smtprec;
var vn;
var anydataexists;
var tt;
var errors;
var dbdir;
var newdbcoden;
var op2;
var wspos;
var wsmsg;

function main(in mode) {

	// use agp<> instead of @record<> to assist source code searching for agp<?>

	if (ID eq "TAXES") {
		// call tax.subs(mode)
		// make definition.subs independent for c++
		systemsubs = "taxsubs";
		call systemsubs(mode);
		return 0;
	}

	if (ID eq "ALL") {
		// call voucher.type.subs(mode)
		// make definition.subs independent for c++
		systemsubs = "vouchertypesubs";
		call systemsubs(mode);
		return 0;
	}

	if (ID eq "TIMESHEET.PARAMS") {
		// call timesheetparam.subs(mode)
		// make definition.subs independent for c++
		systemsubs = "timesheetparamsubs";
		call systemsubs(mode);
		return 0;
	}

	var systemcodes = "SYSTEM.CFG,SYSTEM,..\\..\\SYSTEM.CFG";

	if (mode eq "PREREAD") {

		// system configuration
		// locate @id in 'SYSTEM.CFG,SYSTEM,..\..\SYSTEM.CFG' using ',' setting xx then
		if (systemcodes.locateusing(",", ID, xx)) {

			op = "SYSTEM CONFIGURATION";
			gosub security2(mode, op);
			if (not(req.valid)) {
				return 0;
			}
		}

		if (ID eq "SECURITY*USERS") {
			ID			  = "SECURITY";
			req.templatex = "USERS";
			if (not(authorised("USER LIST", msg, "USER ACCESS"))) {
				req.valid = 0;
			}
			return 0;
		}

	} else if (mode eq "POSTREAD") {

		// NB also called from postwrite

		// system configuration
		// locate @id in 'SYSTEM.CFG,SYSTEM,..\..\SYSTEM.CFG' using ',' setting xx then
		if (systemcodes.locateusing(",", ID, xx)) {

			op = "SYSTEM CONFIGURATION";
			gosub security2(mode, op);
			if (not(req.valid)) {
				return 0;
			}

			var backupkey = ID;
			backupkey.replacer("SYSTEM", "BACKUP");
			var smtpkey = ID;
			smtpkey.replacer("SYSTEM", "SMTP");

			// backup rec on definitions
			if (ID eq "SYSTEM") {
				if (not(backuprec.read(DEFINITIONS, backupkey))) {
					backuprec = "";
				}
				smtpkey = "SMTP.CFG";
				if (not(smtprec.read(DEFINITIONS, smtpkey))) {
					smtprec = "";
				}

				// get system and backup rec from dos
			} else {
				if (not(RECORD.osread(ID.lcase()))) {
					RECORD = "";
				}
				if (not(backuprec.osread(backupkey.lcase()))) {
					backuprec = "";
				}
				if (not(smtprec.osread(smtpkey.lcase()))) {
					smtprec = "";
				}
			}

			RECORD = RECORD.fieldstore(_FM, 71, 29, backuprec.field(_FM, 1, 29));
			RECORD = RECORD.fieldstore(_FM, 101, 9, smtprec.field(_FM, 1, 9));

			return 0;
		}

		if (ID eq "SECURITY") {

			if (req.templatex eq "USERS") {
				return 0;
			}

			// template now provided by readenvironment in client
			// either SECURITY or HOURLYRATES
			if (req.templatex eq "") {
				req.templatex = "SECURITY";
			}
			call securitysubs("SETUP");
			return 0;
		}

		// default cheque format
		if (req.wlocked and ID.field("*", 1) eq "CHEQUEDESIGN") {

			// prevent update
			if (not(authorised("CHEQUE DESIGN", msg, "UA"))) {
				req.srcfile.unlock(ID);
				req.wlocked = "";
			}

			if (not(RECORD.f(1))) {

				var temp;
				if (temp.read(req.srcfile, "CHEQUEDESIGN*DEFAULT")) {

					if (not(RECORD.read(req.srcfile, "CHEQUEDESIGN*" ^ temp.f(1, 1)))) {
						goto nochequeformat;
					}

				} else {

nochequeformat:
					if (not(temp.read(req.srcfile, "CHEQUEDESIGN*ZZZ999"))) {
						if (temp.open("ALANGUAGE", "")) {
							if (not(RECORD.read(temp, "VOUCHERS**CHEQUE"))) {
								RECORD = "";
							}
						}
					}
				}

				// flag a new record despite being copied
				RECORD(10) = "";
				RECORD(11) = "";
				RECORD(14) = "";
			}

			return 0;
		}

		// control access and updating of things like
		// INDEXVALUES*PLANS*EXECUTIVE_CODE
		// INDEXVALUES*SCHEDULES*EXECUTIVE_CODE
		// INDEXVALUES*JOBS*EXECUTIVE_CODE
		if (ID.field("*", 1) eq "INDEXVALUES") {

			// currently only EXECUTIVE from UI
			// default to authorised ATM
			var pretendfile = ID.field("*", 3).field("_", 1);
			if (not(authorised(pretendfile ^ " ACCESS", msg))) {
				goto unlockdefinitions;
			}

			if (req.wlocked and (not(authorised(pretendfile ^ " UPDATE", msg)))) {
				goto preventupdate;
			}

			// add any additional exectives that somehow crept into the index
			// without being added to DEFINITIONS record
			var	 filename  = ID.field("*", 2);
			var	 fieldname = ID.field("*", 3);
			call collectixvals(filename, fieldname);
			let	 nn = PSEUDO.fcount(_FM);
			for (const var ii : range(1, nn)) {
				var val = PSEUDO.f(ii);
				if (val) {
					if (not(RECORD.f(1).locateby("AL", val, vn))) {
						RECORD.inserter(1, vn, val);
						RECORD.inserter(2, vn, "");
						// any more in parallel
					}
				}
			}  // ii;

			// return
		}

		if (ID.field("*", 2) eq "ANALDESIGN") {

			// check allowed access
			if (not(authorised("BILLING REPORT ACCESS", msg, ""))) {
unlockdefinitions:
				req.reset = 5;
				xx		  = unlockrecord("DEFINITIONS", req.srcfile, ID);
				return invalid(msg);
			}

			// always allowed to update or delete own records
			if (RECORD.f(8) ne USERNAME) {
				op = "BILLING REPORT";
				gosub security2(mode, op);
				if (not(req.valid)) {
					return 0;
				}
			}

			// prevent update or delete of EXODUS definitions
			if ((req.wlocked and RECORD.f(8).contains("EXODUS")) and not(USERNAME.contains("EXODUS"))) {
preventupdate:
				req.wlocked = 0;
				xx			= unlockrecord("DEFINITIONS", req.srcfile, ID);
			}

			return 0;
		}

		if (ID eq "AGENCY.PARAMS") {

			// configuration is locked to EXODUS initially
			if (not(authorised("AGENCY CONFIGURATION UPDATE", xx, "EXODUS"))) {
				goto preventupdate;
			}
		}

		gosub postreadfix();

	} else if (mode eq "PREWRITE") {

		if (ID eq "SECURITY") {

			// no difference between security update and hourlyrate update now
			// template='SECURITY'
			// now there is!
			if (req.templatex eq "") {
				req.templatex = "SECURITY";
			}

			call securitysubs("SAVE");

			return 0;
		}

		// save cheque design for exodus
		if (ID.field("*", 1) eq "CHEQUEDESIGN") {

			// prevent update
			if (not(authorised("CHEQUE DESIGN", msg, "UA"))) {
				return invalid(msg);
			}

			if (RECORD.f(14)) {

				// remove old default
				var temp;
				if (temp.read(req.srcfile, "CHEQUEDESIGN*DEFAULT")) {
					var temp2;
					if (temp2.read(req.srcfile, "CHEQUEDESIGN*" ^ temp.f(1, 1))) {
						temp2(14) = "";
						temp2.write(req.srcfile, "CHEQUEDESIGN*" ^ temp.f(1, 1));
					}
				}

				// set new default
				ID.field("*", 2).write(req.srcfile, "CHEQUEDESIGN*DEFAULT");

				// if EXODUS then save the default as EXODUS programs default
				if (USERNAME eq "EXODUS") {
					if (temp.open("ALANGUAGE", "")) {
						RECORD.write(temp, "VOUCHERS**CHEQUE");
					}
				}

			} else {

				// remove as default (assume is this account)
				if (req.orec.f(14)) {
					req.srcfile.deleterecord("CHEQUEDESIGN*DEFAULT");
				}
			}
		}

		// update exodus standard (in case doing this on the programming system)
		// the programming standard is installed into all clients
		// on first login after upgrade
		// this is also done in copygbp perhaps could be removed from there
		// almost identical code in definition.subs and get.subs (for documents)
		// field 10 in documents and definitions xxx*analdesign means the same
		if ((ID.field("*", 2) eq "ANALDESIGN" and USERNAME eq "EXODUS") and RECORD.f(10)) {
			var reports;
			if (reports.open("REPORTS", "")) {
				var key = ID;
				key.replacer("*", "%2A");
				key = "DEFINITIONS*" ^ key;
				RECORD.write(reports, key);
			}
		}

		if (ID eq "AGENCY.PARAMS") {

			// prevent changing "invno by company" after data is entered
			if (RECORD.f(48) ne req.orec.f(48)) {
				call anydata("", anydataexists);
				if (anydataexists) {
					RECORD(48) = req.orec.f(48);
					msg		   = "You cannot change Invoice Number Sequence after data is entered";
					msg(-1)	   = "That change has been ignored.";
					call note(msg);
				}
			}

			// check authorised to set posting
			for (const var fn : range(100, 102)) {
				var usercode = RECORD.f(fn);
				if (usercode) {
					if (not(SECURITY.f(1).locate(usercode, xx))) {
						msg = usercode.quote() ^ " is not a valid financial usercode";
						return invalid(msg);
					}
					if (usercode ne req.orec.f(fn)) {
						// updater must themselves be authorised to post journals
						if (not(authorised("JOURNAL POST", msg))) {
							msg = "You are not authorised to change financial usercode" _FM _FM ^ msg;
							return invalid(msg);
						}
						// financial usercode must be authorised to post journals
						if (not(authorised("JOURNAL POST", msg, "", usercode))) {
							msg = usercode.quote() ^ " financial user is not authorised to do Journal Post";
							return invalid(msg);
						}
					}
				}
			}  // fn;

			var t49	 = field2(RECORD.f(49), _VM, -1);
			var t149 = field2(RECORD.f(149), _VM, -1);
			var t150 = field2(RECORD.f(150), _VM, -1);
			if (t49 and t49 eq t149) {
				msg = "Media adjustment no pattern must be different from Media invoice no pattern, or left blank";
				return invalid(msg);
			}

			if (t49 and t49 eq t150) {
				msg = "Media credit note no pattern must be different from Media invoice no pattern, or left blank";
				return invalid(msg);
			}

			if (t149 and t149 eq t150) {
				msg = "Media adjustment no pattern must be different from Media credit note no pattern, or left blank";
				return invalid(msg);
			}

			if ((RECORD.f(146) and not(RECORD.f(149))) or ((not(RECORD.f(146)) and RECORD.f(149)))) {
				msg = "Media adjustment no pattern is required if media adjustment journal type is specified and vice versa";
				return invalid(msg);
			}

			if ((RECORD.f(147) and not(RECORD.f(150))) or ((not(RECORD.f(147)) and RECORD.f(150)))) {
				msg = "Media credit note no pattern is required if media credit note journal type is specified and vice versa";
				return invalid(msg);
			}

			if (((RECORD.f(146) and RECORD.f(46) eq RECORD.f(146)) or ((RECORD.f(147) and RECORD.f(46) eq RECORD.f(147)))) or ((RECORD.f(147) and RECORD.f(146) eq RECORD.f(147)))) {
				msg = "Media Accounting Voucher type codes must all be different (or left blank)";
				return invalid(msg);
			}

			// save the requester if requested createads
			if (RECORD.f(125) ne req.orec.f(125)) {
				RECORD(128) = USERNAME;
			}
		}

	} else if (mode eq "POSTWRITE") {

		// system configuration
		if (systemcodes.locateusing(",", ID, xx)) {

			// op='SYSTEM CONFIGURATION'
			// gosub security2
			// if valid else return

			var backupkey = ID;
			backupkey.replacer("SYSTEM", "BACKUP");
			backuprec = RECORD.field(_FM, 71, 29);

			var smtpkey = ID;
			smtpkey.replacer("SYSTEM", "SMTP");
			smtprec = RECORD.field(_FM, 101, 9);

			// ensure default style is null
			tt = RECORD.f(46);
			tt.converter(_VM, "");
			tt.replacer("Default", "");
			if (tt eq "") {
				RECORD(46) = "";
			}

			// write backup rec on definitions
			if (ID eq "SYSTEM") {

				// get lastbackupdate in unlikely event that it has changed during update
				if (not(tt.readf(DEFINITIONS, backupkey, 1))) {
					tt = "";
				}
				backuprec(1) = tt;

				backuprec.write(DEFINITIONS, backupkey);
				smtpkey = "SMTP.CFG";
				smtprec.write(DEFINITIONS, smtpkey);

				// write system and backup rec in dos
			} else {
				//call oswrite(RECORD, ID.lcase());
				if (not oswrite(RECORD, ID.lcase()))
					loglasterror();
				//call oswrite(backuprec, backupkey.lcase());
				if (not oswrite(backuprec, backupkey.lcase()))
					loglasterror();
				//call oswrite(smtprec, smtpkey.lcase());
				if (not oswrite(smtprec, smtpkey.lcase()))
					loglasterror();
			}
			//// Since exodus backups handled outside of service in bash scripts
			//// warning if backup drive does not exist/cannot be written to
			//var backupdrives = RECORD.f(77);
			//if (RECORD.f(82) ne RECORD.f(77)) {
			//	backupdrives(-1) = RECORD.f(82);
			//}
			//for (const var driven : range(1, 999)) {
			//	var drive = backupdrives.f(driven)[1];
			//	// /BREAK;
			//	if (not drive)
			//		break;
			//	// TODO replace with checkwritable()
			//	drive ^= ":";
			//
			//	//call osmkdir(drive ^ _OSSLASH "data.bak");
			//	if (not osmkdir(drive ^ _OSSLASH "data.bak"))
			//		abort(lasterror());
			//
			//	call shell2("dir " ^ drive.first(2), errors);
			//
			//	if (errors) {
			//		call note("Note: Backup Drive " ^ drive ^ " cannot be accessed");
			//	} else {
			//		// tempfilename=lcase(drive:'\data.bak':'\':rnd(8))
			//		var tempfilename = (drive ^
			//							"/data.bak"
			//							"/" ^
			//							var(8).rnd())
			//							   .lcase();
			//		// if (VOLUMES) {
			//		//	tempfilename ^= ".$$$";
			//		//} else {
			//			tempfilename ^= "./$";
			//		//}
			//		tempfilename.converter("/", OSSLASH);
			//
			//		//var(date()).oswrite(tempfilename);
			//		//if (tt.osread(tempfilename)) {
			//		//	tempfilename.osremove();
			//		//} else {
			//		//	call note("Note: Cannot write to backup drive " ^ drive);
			//		//}
			//		if (not date().oswrite(tempfilename))
			//			abort("Note: Cannot write to backup drive " ^ drive);
			//	}
			//}  // driven;

			// create/update ddns configuration if necessary
			// actually only the ddns.cmd file really need be updated
			// but atm it recreates everything from scratch
			var oldhostname = req.orec.f(57);
			var newhostname = RECORD.f(57);
			if (newhostname and newhostname ne oldhostname) {
				SYSTEM(57) = newhostname;
				// in INIT.GENERAL and DEFINITION.SUBS
				if (var("ddns.cmd").osfile()) {
					perform("STARTDDNS");
				}
			}

			SYSTEM(58) = RECORD.f(58);
			gosub reorderdbs();

			return 0;
		}

		// have to pass back the partial record
		if (ID eq "SECURITY") {

			// could be HOURLYRATES which has different authorisation
			if (req.templatex eq "") {
				req.templatex = "SECURITY";
			}

			// to get lastest userprivs
			call securitysubs("POSTAPP");

			// to the get the filtered record again
			call definitionsubs("POSTREAD");
		}

		gosub postreadfix();

		// Trigger restart if necessary (is this really necessary now)
		// Processes restart if system.cfg date/time changes
		// TODO make it only per database
		if (USERNAME ne "EXODUS") {
			if (var("AGENCY.PARAMS,ALL,SECURITY,TAXES,TIMESHEET.PARAMS").locateusing(",", ID, xx)) {
//				// TODO prevent a write from system configuration file
//				//call osread(tt, "system.cfg");
//				if (not osread(tt, "system.cfg"))
//					abort(lasterror());
//				//call oswrite(tt, "system.cfg");
//				if (not oswrite(tt, "system.cfg"))
//					loglasterror();
				if (not osshell("touch system.cfg"))
					abort(lasterror());
			}
		}

	} else if (mode eq "PREDELETE") {

		// system configuration
		// locate @id in 'SYSTEM.CFG,SYSTEM,..\..\SYSTEM.CFG' using ',' setting xx then
		if (systemcodes.locateusing(",", ID, xx)) {

			op = "SYSTEM CONFIGURATION";
			gosub security2(mode, op);
			if (not(req.valid)) {
				return 0;
			}

			// get from operating system
			if (ID ne "SYSTEM") {
				//ID.osremove();
				if (ID.osfile() and not ID.osremove())
					abort(lasterror());
			}

			return 0;
		}

		if (ID eq "SECURITY") {
			msg = "DELETE SECURITY not allowed in DEFINITION.SUBS";
			return invalid(msg);
		}

		if (ID.field("*", 2) eq "ANALDESIGN") {

			// prevent update
			if (not(authorised("CHEQUE DESIGN", msg, "UA"))) {
				return invalid(msg);
			}

			// always allowed to delete own records
			if (RECORD.f(8) ne USERNAME) {
				op = "BILLING REPORT";
				gosub security2(mode, op);
				if (not(req.valid)) {
					return 0;
				}
			}
		}

		// remove default cheque design
		if (ID.field("*", 1) eq "CHEQUEDESIGN") {
			if (RECORD.f(14)) {
				req.srcfile.deleterecord("CHEQUEDESIGN*DEFAULT");
			}
		}

		// update exodus standard (in case doing this on the programming system)
		// %DELETED% ensures that deleted EXODUS documents get deleted
		// on upgrading clients
		if ((ID.field("*", 2) eq "ANALDESIGN" and USERNAME eq "EXODUS") and RECORD.f(10)) {
			var reports;
			if (reports.open("REPORTS", "")) {
				var key = ID;
				key.replacer("*", "%2A");
				key = "DEFINITIONS*" ^ key;
				if (xx.read(reports, key)) {
					var("%DELETED%").write(reports, key);
				}
				// delete reports,key
			}
		}

	} else if (mode eq "POSTDELETE") {

	} else if (mode eq "REORDERDBS") {
		gosub reorderdbs();

	} else {
		msg = mode.quote() ^ " invalid mode in definition.subs";
		return invalid(msg);
	}

	return 0;
}

subroutine reorderdbs() {

	// reorder dbs in ADAGENCY.VOL, ACCOUNTS.VOL etc
	// according to dbs listed in configuration
	var newdbcodes = SYSTEM.f(58);

	// read the existing dbdir or silently quit
	var dbdirfilename = APPLICATION.lcase() ^ ".vol";
	if (not(dbdir.osread(dbdirfilename))) {
		return;
	}
	var olddbdir = dbdir;

	// convert from DOS
	// dbdir=field(dbdir,char(26),1)
	dbdir.converter("\r\n", _FM _FM);
	dbdir.replacer(_FM _FM, _FM);
	dbdir.converter(",*", _SM _VM);

	// extract substitution and dblist from dbdir line 1
	var substitution = dbdir.f(1).field(" ", 1);
	var dblist		 = dbdir.f(1).field(" ", 2, 9999);

	// create newdblist in order of given newdbcodes
	// if not found in new order then append in order found
	let ndbs	  = dblist.fcount(_VM);
	var newdblist = "";
	for (const var dbn : range(1, ndbs)) {
		var db	   = dblist.f(1, dbn);
		var dbcode = db.f(1, 1, 2);
		if (not(newdbcodes.locate(dbcode, newdbcoden))) {
			newdbcodes(1, newdbcoden) = dbcode;
		}
		newdblist(1, newdbcoden) = db;
	}  // dbn;

	// replace substitution and dblist
	var newdbdir = dbdir;
	newdbdir(1)	 = substitution ^ " " ^ newdblist;

	// convert to DOS
	if (not newdbdir.ends(_FM)) {
		newdbdir ^= _FM;
	}
	newdbdir.converter(_SM _VM, ",*");
	newdbdir.replacer(_FM, _EOL);

	if (newdbdir ne olddbdir) {
		//call oswrite(newdbdir, dbdirfilename);
		if (not oswrite(newdbdir, dbdirfilename))
			loglasterror();
		// call sysmsg(newdbcodes,'List of databases reordered')
	}
	return;
}

subroutine postreadfix() {
	if (ID eq "AGENCY.PARAMS") {

		if (RECORD.f(55) eq "") {
			RECORD(55) = "Budget";
		}
		if (RECORD.f(56) eq "") {
			RECORD(56) = "F/cast";
		}
		if (RECORD.f(72) eq "") {
			RECORD(72) = "Media Plan";
		}
		if (RECORD.f(73) eq "") {
			RECORD(73) = "Media Schedule";
		}
		if (RECORD.f(74) eq "") {
			RECORD(74) = "Estimate";
		}

		if (RECORD.f(49) eq "") {
			RECORD(49) = "<NUMBER>";
		}
		if (RECORD.f(49, 1, 2) eq "") {
			RECORD(49, 1, 2) = "2000.01";
		}
		if (RECORD.f(50) eq "") {
			RECORD(50) = "<NUMBER>";
		}
		if (RECORD.f(50, 1, 2) eq "") {
			RECORD(50, 1, 2) = "2000.01";
		}

		// done in agency.subs getnextid
		if (RECORD.f(53) eq "") {
			RECORD(53) = "<NUMBER>";
		}
		if (RECORD.f(63) eq "") {
			RECORD(63) = "<NUMBER>";
		}
		if (RECORD.f(69) eq "") {
			RECORD(69) = "<NUMBER>";
		}
		if (RECORD.f(70) eq "") {
			RECORD(70) = "<NUMBER>";
		}
		if (RECORD.f(71) eq "") {
			RECORD(71) = "<NUMBER>";
		}

		if (RECORD.f(25) eq "") {
			RECORD(25) = "ACC<YEAR>";
		}
		if (RECORD.f(26) eq "") {
			RECORD(26) = "WIP<YEAR>";
		}

		// copy schedule footer to plan footer
		if (not(RECORD.f(34))) {
			RECORD(34) = RECORD.f(11);
		}
	}

	return;
}

libraryexit()
