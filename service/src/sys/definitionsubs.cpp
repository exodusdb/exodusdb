#include <exodus/library.h>
libraryinit()

#include <systemsubs.h>
#include <authorised.h>
#include <securitysubs.h>
#include <collectixvals.h>
#include <anydata.h>
#include <shell2.h>
#include <definitionsubs.h>
#include <singular.h>

#include <gen_common.h>
#include <win_common.h>

#include <window.hpp>

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
	//c sys

	#include <general_common.h>

	//use agp<> instead of @record<> to assist source code searching for agp<?>

	if (ID == "TAXES") {
		//call tax.subs(mode)
		//make definition.subs independent for c++
		systemsubs = "taxsubs";
		call systemsubs(mode);
		return 0;
	}

	if (ID == "ALL") {
		//call voucher.type.subs(mode)
		//make definition.subs independent for c++
		systemsubs = "vouchertypesubs";
		call systemsubs(mode);
		return 0;
	}

	if (ID == "TIMESHEET.PARAMS") {
		//call timesheetparam.subs(mode)
		//make definition.subs independent for c++
		systemsubs = "timesheetparamsubs";
		call systemsubs(mode);
		return 0;
	}

	var systemcodes = "SYSTEM.CFG,SYSTEM,..\\..\\SYSTEM.CFG";

	if (mode == "PREREAD") {

		//system configuration
		//locate @id in 'SYSTEM.CFG,SYSTEM,..\..\SYSTEM.CFG' using ',' setting xx then
		if (systemcodes.locateusing(",",ID,xx)) {

			op = "SYSTEM CONFIGURATION";
			gosub security2(mode, op);
			if (not(win.valid)) {
				return 0;
			}
		}

		if (ID == "SECURITY*USERS") {
			ID = "SECURITY";
			win.templatex = "USERS";
			if (not(authorised("USER LIST", msg, "USER ACCESS"))) {
				win.valid = 0;
			}
			return 0;
		}

	} else if (mode == "POSTREAD") {

		//NB also called from postwrite

		//system configuration
		//locate @id in 'SYSTEM.CFG,SYSTEM,..\..\SYSTEM.CFG' using ',' setting xx then
		if (systemcodes.locateusing(",",ID,xx)) {

			op = "SYSTEM CONFIGURATION";
			gosub security2(mode, op);
			if (not(win.valid)) {
				return 0;
			}

			var backupkey = ID;
			backupkey.swapper("SYSTEM", "BACKUP");
			var smtpkey = ID;
			smtpkey.swapper("SYSTEM", "SMTP");

			// backup rec on definitions
			if (ID == "SYSTEM") {
				if (not(backuprec.read(DEFINITIONS, backupkey))) {
					backuprec = "";
				}
				smtpkey = "SMTP.CFG";
				if (not(smtprec.read(DEFINITIONS, smtpkey))) {
					smtprec = "";
				}

			//get system and backup rec from dos
			}else{
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

			RECORD = RECORD.fieldstore(FM, 71, 29, backuprec.field(FM, 1, 29));
			RECORD = RECORD.fieldstore(FM, 101, 9, smtprec.field(FM, 1, 9));

			return 0;

		}

		if (ID == "SECURITY") {

			if (win.templatex == "USERS") {
				RECORD = RECORD.invert();
				return 0;
			}

			//template now provided by readenvironment in client
			//either SECURITY or HOURLYRATES
			if (win.templatex == "") {
				win.templatex = "SECURITY";
			}
			call securitysubs("SETUP");
			return 0;
		}

		//default cheque format
		if (win.wlocked and (ID.field("*", 1) == "CHEQUEDESIGN")) {

			//prevent update
			if (not(authorised("CHEQUE DESIGN", msg, "UA"))) {
				win.srcfile.unlock( ID);
				win.wlocked = "";
			}

			if (not(RECORD.a(1))) {

				var temp;
				if (temp.read(win.srcfile, "CHEQUEDESIGN*DEFAULT")) {

					if (not(RECORD.read(win.srcfile, "CHEQUEDESIGN*" ^ temp.a(1, 1)))) {
						goto nochequeformat;
					}

				}else{

nochequeformat:
					if (not(temp.read(win.srcfile, "CHEQUEDESIGN*ZZZ999"))) {
						if (temp.open("ALANGUAGE", "")) {
							if (not(RECORD.read(temp, "VOUCHERS**CHEQUE"))) {
								RECORD = "";
							}
						}
					}

				}

				//flag a new record despite being copied
				RECORD.r(10, "");
				RECORD.r(11, "");
				RECORD.r(14, "");

			}

			return 0;
		}

		//control access and updating of things like
		//INDEXVALUES*PLANS*EXECUTIVE_CODE
		//INDEXVALUES*SCHEDULES*EXECUTIVE_CODE
		//INDEXVALUES*JOBS*EXECUTIVE_CODE
		if (ID.field("*", 1) == "INDEXVALUES") {

			//currently only EXECUTIVE from UI
			//default to authorised ATM
			var pretendfile = ID.field("*", 3).field("_", 1);
			if (not(authorised(pretendfile ^ " ACCESS", msg))) {
				goto unlockdefinitions;
			}

			if (win.wlocked and (not(authorised(pretendfile ^ " UPDATE", msg)))) {
				goto preventupdate;
			}

			//add any additional exectives that somehow crept into the index
			//without being added to DEFINITIONS record
			var filename = ID.field("*", 2);
			var fieldname = ID.field("*", 3);
			call collectixvals(filename, fieldname);
			var nn = PSEUDO.count(FM) + 1;
			for (var ii = 1; ii <= nn; ++ii) {
				var val = PSEUDO.a(ii);
				if (val) {
					if (not(RECORD.a(1).locateby("AL",val,vn))) {
						RECORD.inserter(1, vn, val);
						RECORD.inserter(2, vn, "");
						//any more in parallel
					}
				}
			};//ii;

			//return
		}

		if (ID.field("*", 2) == "ANALDESIGN") {

			//check allowed access
			if (not(authorised("BILLING REPORT ACCESS", msg, ""))) {
unlockdefinitions:
				win.reset = 5;
				xx = unlockrecord("DEFINITIONS", win.srcfile, ID);
				return invalid(msg);
			}

			//always allowed to update or delete own records
			if (RECORD.a(8) ne USERNAME) {
				op = "BILLING REPORT";
				gosub security2(mode, op);
				if (not(win.valid)) {
					return 0;
				}
			}

			//prevent update or delete of EXODUS definitions
			if ((win.wlocked and RECORD.a(8).index("EXODUS")) and not(USERNAME.index("EXODUS"))) {
preventupdate:
				win.wlocked = 0;
				xx = unlockrecord("DEFINITIONS", win.srcfile, ID);
			}

			return 0;
		}

		if (ID == "AGENCY.PARAMS") {

			//configuration is locked to EXODUS initially
			if (not(authorised("AGENCY CONFIGURATION UPDATE", xx, "EXODUS"))) {
				goto preventupdate;
			}

		}

		gosub postreadfix();

	} else if (mode == "PREWRITE") {

		if (ID == "SECURITY") {

			//no difference between security update and hourlyrate update now
			//template='SECURITY'
			//now there is!
			if (win.templatex == "") {
				win.templatex = "SECURITY";
			}

			call securitysubs("SAVE");

			return 0;
		}

		//save cheque design for exodus
		if (ID.field("*", 1) == "CHEQUEDESIGN") {

			//prevent update
			if (not(authorised("CHEQUE DESIGN", msg, "UA"))) {
				return invalid(msg);
			}

			if (RECORD.a(14)) {

				//remove old default
				var temp;
				if (temp.read(win.srcfile, "CHEQUEDESIGN*DEFAULT")) {
					var temp2;
					if (temp2.read(win.srcfile, "CHEQUEDESIGN*" ^ temp.a(1, 1))) {
						temp2.r(14, "");
						temp2.write(win.srcfile, "CHEQUEDESIGN*" ^ temp.a(1, 1));
					}
				}

				//set new default
				ID.field("*", 2).write(win.srcfile, "CHEQUEDESIGN*DEFAULT");

				//if EXODUS then save the default as EXODUS programs default
				if (USERNAME == "EXODUS") {
					if (temp.open("ALANGUAGE", "")) {
						RECORD.write(temp, "VOUCHERS**CHEQUE");
					}
				}

			}else{

				//remove as default (assume is this account)
				if (win.orec.a(14)) {
					win.srcfile.deleterecord("CHEQUEDESIGN*DEFAULT");

				}

			}

		}

		//update exodus standard (in case doing this on the programming system)
		//the programming standard is installed into all clients
		//on first login after upgrade
		//this is also done in copygbp perhaps could be removed from there
		//almost identical code in definition.subs and get.subs (for documents)
		//field 10 in documents and definitions xxx*analdesign means the same
		if (((ID.field("*", 2) == "ANALDESIGN") and (USERNAME == "EXODUS")) and RECORD.a(10)) {
			var reports;
			if (reports.open("REPORTS", "")) {
				var key = ID;
				key.swapper("*", "%2A");
				key = "DEFINITIONS*" ^ key;
				RECORD.write(reports, key);
			}
		}

		if (ID == "AGENCY.PARAMS") {

			//prevent changing "invno by company" after data is entered
			if (RECORD.a(48) ne win.orec.a(48)) {
				call anydata("", anydataexists);
				if (anydataexists) {
					RECORD.r(48, win.orec.a(48));
					msg = "You cannot change Invoice Number Sequence after data is entered";
					msg.r(-1, "That change has been ignored.");
					call note(msg);
				}
			}

			//check authorised to set posting
			for (var fn = 100; fn <= 102; ++fn) {
				var usercode = RECORD.a(fn);
				if (usercode) {
					if (not(SECURITY.a(1).locate(usercode,xx))) {
						msg = usercode.quote() ^ " is not a valid financial usercode";
						return invalid(msg);
					}
					if (usercode ne win.orec.a(fn)) {
						//updater must themselves be authorised to post journals
						if (not(authorised("JOURNAL POST", msg))) {
							msg = "You are not authorised to change financial usercode" ^ FM ^ FM ^ msg;
							return invalid(msg);
						}
						//financial usercode must be authorised to post journals
						if (not(authorised("JOURNAL POST", msg, "", usercode))) {
							msg = usercode.quote() ^ " financial user is not authorised to do Journal Post";
							return invalid(msg);
						}
					}
				}
			};//fn;

			var t49 = field2(RECORD.a(49), VM, -1);
			var t149 = field2(RECORD.a(149), VM, -1);
			var t150 = field2(RECORD.a(150), VM, -1);
			if (t49 and (t49 == t149)) {
				msg = "Media adjustment no pattern must be different from Media invoice no pattern, or left blank";
				return invalid(msg);
			}

			if (t49 and (t49 == t150)) {
				msg = "Media credit note no pattern must be different from Media invoice no pattern, or left blank";
				return invalid(msg);
			}

			if (t149 and (t149 == t150)) {
				msg = "Media adjustment no pattern must be different from Media credit note no pattern, or left blank";
				return invalid(msg);
			}

			if ((RECORD.a(146) and not(RECORD.a(149))) or ((not(RECORD.a(146)) and RECORD.a(149)))) {
				msg = "Media adjustment no pattern is required if media adjustment journal type is specified and vice versa";
				return invalid(msg);
			}

			if ((RECORD.a(147) and not(RECORD.a(150))) or ((not(RECORD.a(147)) and RECORD.a(150)))) {
				msg = "Media credit note no pattern is required if media credit note journal type is specified and vice versa";
				return invalid(msg);
			}

			if (((RECORD.a(146) and (RECORD.a(46) == RECORD.a(146))) or ((RECORD.a(147) and (RECORD.a(46) == RECORD.a(147))))) or ((RECORD.a(147) and (RECORD.a(146) == RECORD.a(147))))) {
				msg = "Media Accounting Voucher type codes must all be different (or left blank)";
				return invalid(msg);
			}

			//save the requester if requested createads
			if (RECORD.a(125) ne win.orec.a(125)) {
				RECORD.r(128, USERNAME);
			}

			//obfuscate the mark
			RECORD.r(1, RECORD.a(1).invert());

		}

	} else if (mode == "POSTWRITE") {

		//system configuration
		if (systemcodes.locateusing(",",ID,xx)) {

			//op='SYSTEM CONFIGURATION'
			//gosub security2
			//if valid else return

			var backupkey = ID;
			backupkey.swapper("SYSTEM", "BACKUP");
			backuprec = RECORD.field(FM, 71, 29);

			var smtpkey = ID;
			smtpkey.swapper("SYSTEM", "SMTP");
			smtprec = RECORD.field(FM, 101, 9);

			//ensure default style is null
			tt = RECORD.a(46);
			tt.converter(VM, "");
			tt.swapper("Default", "");
			if (tt == "") {
				RECORD.r(46, "");
			}

			//write backup rec on definitions
			if (ID == "SYSTEM") {

				//get lastbackupdate in unlikely event that it has changed during update
				if (not(tt.readv(DEFINITIONS, backupkey, 1))) {
					tt = "";
				}
				backuprec.r(1, tt);

				backuprec.write(DEFINITIONS, backupkey);
				smtpkey = "SMTP.CFG";
				smtprec.write(DEFINITIONS, smtpkey);

			//write system and backup rec in dos
			}else{
				call oswrite(RECORD, ID.lcase());
				call oswrite(backuprec, backupkey.lcase());
				call oswrite(smtprec, smtpkey.lcase());
			}

			//warning if backup drive does not exist/cannot be written to
			var backupdrives = RECORD.a(77);
			if (RECORD.a(82) ne RECORD.a(77)) {
				backupdrives.r(-1, RECORD.a(82));
			}
			for (var driven = 1; driven <= 999; ++driven) {
				var drive = backupdrives.a(driven)[1];
				///BREAK;
				if (not drive) break;
				//TODO replace with checkwritable()
				drive ^= ":";
				call osmkdir(drive ^ OSSLASH "data.bak");
				call shell2("dir " ^ drive.substr(1,2), errors);
				if (errors) {
					call note("Note: Backup Drive " ^ drive ^ " cannot be accessed");
				}else{
					//tempfilename=lcase(drive:'\data.bak':'\':rnd(8))
					var tempfilename = (drive ^ "/data.bak" "/" ^ var(8).rnd()).lcase();
					if (VOLUMES) {
						tempfilename ^= ".$$$";
					}else{
						tempfilename ^= "./$";
					}
					tempfilename.converter("/", OSSLASH);
					var(var().date()).oswrite(tempfilename);
					if (tt.osread(tempfilename)) {
						tempfilename.osdelete();
					}else{
						call note("Note: Cannot write to backup drive " ^ drive);
					}
				}
			};//driven;

			//create/update ddns configuration if necessary
			//actually only the ddns.cmd file really need be updated
			//but atm it recreates everything from scratch
			var oldhostname = win.orec.a(57);
			var newhostname = RECORD.a(57);
			if (newhostname and newhostname ne oldhostname) {
				SYSTEM.r(57, newhostname);
				//in INIT.GENERAL and DEFINITION.SUBS
				if (var("ddns.cmd").osfile()) {
					perform("STARTDDNS");
				}
			}

			SYSTEM.r(58, RECORD.a(58));
			gosub reorderdbs();

			return 0;

		}

		//have to pass back the inverted, partial record
		if (ID == "SECURITY") {

			//could be HOURLYRATES which has different authorisation
			if (win.templatex == "") {
				win.templatex = "SECURITY";
			}

			//to get lastest userprivs
			call securitysubs("POSTAPP");

			//to the get the filtered record again
			call definitionsubs("POSTREAD");

		}

		gosub postreadfix();

		//trigger restart if necessary (is this really necessary now)
		//TODO make it only per database
		if (USERNAME ne "EXODUS") {
			if (var("AGENCY.PARAMS,ALL,SECURITY,TAXES,TIMESHEET.PARAMS").locateusing(",",ID,xx)) {
				//TODO prevent a write from system configuration file
				call osread(tt, "system.cfg");
				call oswrite(tt, "system.cfg");
			}
		}

	} else if (mode == "PREDELETE") {

		//system configuration
		//locate @id in 'SYSTEM.CFG,SYSTEM,..\..\SYSTEM.CFG' using ',' setting xx then
		if (systemcodes.locateusing(",",ID,xx)) {

			op = "SYSTEM CONFIGURATION";
			gosub security2(mode, op);
			if (not(win.valid)) {
				return 0;
			}

			//get from operating system
			if (ID ne "SYSTEM") {
				ID.osdelete();
			}

			return 0;

		}

		if (ID == "SECURITY") {
			msg = "DELETE SECURITY not allowed in DEFINITION.SUBS";
			return invalid(msg);
		}

		if (ID.field("*", 2) == "ANALDESIGN") {

			//prevent update
			if (not(authorised("CHEQUE DESIGN", msg, "UA"))) {
				return invalid(msg);
			}

			//always allowed to delete own records
			if (RECORD.a(8) ne USERNAME) {
				op = "BILLING REPORT";
				gosub security2(mode, op);
				if (not(win.valid)) {
					return 0;
				}
			}

		}

		//remove default cheque design
		if (ID.field("*", 1) == "CHEQUEDESIGN") {
			if (RECORD.a(14)) {
				win.srcfile.deleterecord("CHEQUEDESIGN*DEFAULT");

			}
		}

		//update exodus standard (in case doing this on the programming system)
		//%DELETED% ensures that deleted EXODUS documents get deleted
		//on upgrading clients
		if (((ID.field("*", 2) == "ANALDESIGN") and (USERNAME == "EXODUS")) and RECORD.a(10)) {
			var reports;
			if (reports.open("REPORTS", "")) {
				var key = ID;
				key.swapper("*", "%2A");
				key = "DEFINITIONS*" ^ key;
				if (xx.read(reports, key)) {
					var("%DELETED%").write(reports, key);
				}
				//delete reports,key
			}
		}

	} else if (mode == "POSTDELETE") {

	} else if (mode == "REORDERDBS") {
		gosub reorderdbs();

	} else {
		msg = mode.quote() ^ " invalid mode in definition.subs";
		return invalid(msg);
	}

	return 0;
}

subroutine reorderdbs() {

	//reorder dbs in ADAGENCY.VOL, ACCOUNTS.VOL etc
	//according to dbs listed in configuration
	var newdbcodes = SYSTEM.a(58);

	//read the existing dbdir or silently quit
	var dbdirfilename = APPLICATION.lcase() ^ ".vol";
	if (not(dbdir.osread(dbdirfilename))) {
		return;
	}
	var olddbdir = dbdir;

	//convert from DOS
	//dbdir=field(dbdir,char(26),1)
	dbdir.converter("\r\n", FM ^ FM);
	dbdir.swapper(FM ^ FM, FM);
	dbdir.converter(",*", SVM ^ VM);

	//extract substitution and dblist from dbdir line 1
	var substitution = dbdir.a(1).field(" ", 1);
	var dblist = dbdir.a(1).field(" ", 2, 9999);

	//create newdblist in order of given newdbcodes
	//if not found in new order then append in order found
	var ndbs = dblist.count(VM) + 1;
	var newdblist = "";
	for (var dbn = 1; dbn <= ndbs; ++dbn) {
		var db = dblist.a(1, dbn);
		var dbcode = db.a(1, 1, 2);
		if (not(newdbcodes.locate(dbcode,newdbcoden))) {
			newdbcodes.r(1, newdbcoden, dbcode);
		}
		newdblist.r(1, newdbcoden, db);
	};//dbn;

	//replace substitution and dblist
	var newdbdir = dbdir;
	newdbdir.r(1, substitution ^ " " ^ newdblist);

	//convert to DOS
	if (newdbdir[-1] ne FM) {
		newdbdir ^= FM;
	}
	newdbdir.converter(SVM ^ VM, ",*");
	newdbdir.swapper(FM, "\r\n");

	if (newdbdir ne olddbdir) {
		call oswrite(newdbdir, dbdirfilename);
		//call sysmsg(newdbcodes,'List of databases reordered')
	}
	return;
}

subroutine postreadfix() {
	if (ID == "AGENCY.PARAMS") {

		RECORD.r(1, RECORD.a(1).invert());

		if (RECORD.a(55) == "") {
			RECORD.r(55, "Budget");
		}
		if (RECORD.a(56) == "") {
			RECORD.r(56, "F/cast");
		}
		if (RECORD.a(72) == "") {
			RECORD.r(72, "Media Plan");
		}
		if (RECORD.a(73) == "") {
			RECORD.r(73, "Media Schedule");
		}
		if (RECORD.a(74) == "") {
			RECORD.r(74, "Estimate");
		}

		if (RECORD.a(49) == "") {
			RECORD.r(49, "<NUMBER>");
		}
		if (RECORD.a(49, 1, 2) == "") {
			RECORD.r(49, 1, 2, "2000.01");
		}
		if (RECORD.a(50) == "") {
			RECORD.r(50, "<NUMBER>");
		}
		if (RECORD.a(50, 1, 2) == "") {
			RECORD.r(50, 1, 2, "2000.01");
		}

		//done in agency.subs getnextid
		if (RECORD.a(53) == "") {
			RECORD.r(53, "<NUMBER>");
		}
		if (RECORD.a(63) == "") {
			RECORD.r(63, "<NUMBER>");
		}
		if (RECORD.a(69) == "") {
			RECORD.r(69, "<NUMBER>");
		}
		if (RECORD.a(70) == "") {
			RECORD.r(70, "<NUMBER>");
		}
		if (RECORD.a(71) == "") {
			RECORD.r(71, "<NUMBER>");
		}

		if (RECORD.a(25) == "") {
			RECORD.r(25, "ACC<YEAR>");
		}
		if (RECORD.a(26) == "") {
			RECORD.r(26, "WIP<YEAR>");
		}

		//copy schedule footer to plan footer
		if (not(RECORD.a(34))) {
			RECORD.r(34, RECORD.a(11));
		}

	}

	return;
}

libraryexit()
