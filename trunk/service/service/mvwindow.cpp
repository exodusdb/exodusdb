/* Copyright (c) 2007 Stephen John Bush - see LICENCE.TXT*/
#define NEO_MVWINDOW_CPP
#include <iostream>
#include "mvwindow.h"
#include "Market.h"

//using namespace std;

namespace exodus
{

bool MvWindow::security(const var& mode)
{
	/////////
	//valid=1 move down
	var op = env.singular(datafile);
	//if op[-3,3]='ies' then op[-3,3]='y'
	//if op[-1,1]='s' then op[-1,1]=''
	//op:=' file'
	return security2(mode,op);
}

bool MvWindow::security2(const var& mode, const var& op0)
{
//	valid = 1;
	var op=op0.convert(L"_.", L"  ");
	var op2 = L"";
	if (op.substr(-1, 1) == DQ) {
//		op2 = (op.substr(-2, L"B\"")).quote();
		op2=op.field2(DQ,-2);
		op.splicer(-op2.length(), op2.length(), L"");
		op2.splicer(1, 0, L" ");
	}
	if (mode.index(L"INIT", 1)) {
		var msg;
		if (!(env.authorised(op ^ L" ACCESS" ^ op2, msg, L"")))
			return invalid(msg);

	}else if (mode.index(L"READ", 1) || mode.index(L"WRITE", 1)) {
		if (!wlocked) {
			op ^= L" ACCESS";
		}else{
			if (orec) {
				op ^= L" UPDATE";
			}else{
				op ^= L" CREATE";
			}
		}
		var msg;
		if (!(
            
            env.authorised
            
            (op ^ op2, msg, L""))) {
			if (orec == L"" || !wlocked) {
				invalid();
				reset = 5;
			}
			if (wlocked) {
				env.unlockrecord(datafile, srcfile, env.ID);
				wlocked = 0;
			}
		}

	}else if (mode.index(L"DELETE", 1)) {
		var msg;
		if (!(env.authorised(op ^ L" DELETE" ^ op2, msg, L"")))
			return invalid(msg);

	}else if (1) {
		return security3(op,op2);
	}
	return true;
}

bool MvWindow::security3(const var& op, const var& op2)
{
	var op2b=op2.assigned()?op2:L"";
	var msg;
	if (!(env.authorised(op ^ op2b, msg, L"")))
		return invalid(msg);

	return true;
}

bool MvWindow::invalidq(const var& msg)
{
	return invalid(msg.splice(1, 0, (is.extract(1, 1, 1)).quote() ^ L" "));
}

bool MvWindow::invalid()
{
	return invalid(L"");
}

bool MvWindow::invalid(const var& msg)
{
	valid = 0;
	//if is.orig then is=is.orig
	if (!isorig.unassigned()) {
		is = isorig;
		if (!reset)
			reset = 1;
	}
	if (!msg) return true;
	env.note(msg);
	return false;
}

var MvWindow::securitysubs(const var& mode)
{

	std::wcout<<L"MVMvWindow::securitysubs(const var& mode)"<<L" "<<mode<<std::endl;

	var interactive = ! env.SYSTEM.extract(33);

	if (mode == L"SETUP") {

		//called as postread in noninteractive mode (from Security.subs)

		//determine filename and default lock
		var datafilename;
		var defaultlock;
		if (templatex.unassigned())
			templatex = L"";
		if (templatex == L"SECURITY") {
			datafilename = L"AUTHORISATION";
			defaultlock = L"GS";
		}else if (templatex == L"HOURLYRATES") {
			datafilename = L"HOURLY RATE";
			registerx[5] = 1;
			registerx[4] = 1;
			defaultlock = L"TS3";
		}else{
			var msg = templatex.quote() ^ L" unknown template in securitysubs";
			return invalid(msg);
		}

		//check allowed access
		var msg;
		if (!(env.authorised(datafilename ^ L" ACCESS", msg, defaultlock))) {
			return invalid(msg);
		}

		if (templatex == L"SECURITY") {

			//check guest status
			if (env.SYSTEM.extract(21)) {
				return invalid(L"YOU ARE A TEMPORARY GUEST|YOU CANNOT ACCESS AUTHORISATION");
			}

			//check supervisor status
			registerx[4] = env.authorised(L"AUTHORISATION UPDATE LOWER GROUPS", msg, L"");
			registerx[5] = env.authorised(L"AUTHORISATION UPDATE HIGHER GROUPS", msg, L"");

		}

		//if logged in as account then same as logged in as neosys
		if (var(L"012").index(env.PRIVILEGE, 1)) {
			registerx[6] = L"NEOSYS";
		}else{
			registerx[6] = env.USERNAME;
		}

		//check security
		if (env.authorised(datafilename ^ L" UPDATE", msg, defaultlock)) {
		}else{
			if (!interactive)
				wlocked = 0;

		}

		if (!env.SECURITY.read(env.DEFINITIONS, L"SECURITY"))
			env.SECURITY = L"";
		//env.SECURITY.inverter();

		securitysubs_cleartemp(env.RECORD);

		//never send the passwords to browser
		//(restored in prewrite except for new passwords)
		if (!interactive)
			env.RECORD.replacer(4, 0, 0, L"");

		//sort the tasks
//TODO		sortarray(env.SECURITY, L"10" ^ VM ^ L"11");

		registerx[7] = env.SECURITY;

		env.RECORD = env.SECURITY;

		//@record<9>=curruser

		//don't delete users for hourly rates
		if (templatex != L"SECURITY")
			return false;

		//delete disallowed tasks (except all master type user to see all tasks)
		if (!(registerx[5] and registerx[4])) {
			var tasks = env.RECORD.extract(10);
			var locks = env.RECORD.extract(11);
			var ntasks = tasks.dcount(VM);
			for (int taskn = ntasks; taskn >= 1; taskn--) {
				var task = tasks.extract(1, taskn);
				var temp = (task.substr(1, 10) == L"DOCUMENT: ") ? L"#" : L"";
				if (!(env.authorised(L"!" ^ temp ^ task, msg, L""))) {
					env.RECORD.eraser(10, taskn, 0);
					env.RECORD.eraser(11, taskn, 0);
				}
			};//taskn;
		}

		//hide higher/lower users
		if (not (registerx[6]).index(L"NEOSYS", 1)) {

			var usercodes = env.RECORD.extract(1);
			var nusers = usercodes.dcount(VM);

			var usern;
			if (!(usercodes.locateusing(env.USERNAME, VM, usern)))
				return invalid(env.USERNAME ^ L" user not in in authorisation file");

			//hide higher users
			if (registerx[5]) {
				registerx[8] = 1;
			}else{
				registerx[8] = usern;
				while (true) {
				//BREAK;
				if (!(registerx[8] > 1 and env.RECORD.extract(2, registerx[8] - 1) == L"")) break;;
					registerx[8] -= 1;
				}//loop;
			}

			//hide lower users
			if (registerx[4]) {
				registerx[9] = nusers;
			}else{
				registerx[9] = usern;
				while (true) {
				//BREAK;
				if (!(registerx[9] < nusers and env.RECORD.extract(1, registerx[9] + 1) not_eq L"---")) break;;
					registerx[9] += 1;
				}//loop;
			}

			//extract out the allowable users and keys
			//also in prewrite

			if (registerx[8] not_eq 1 or registerx[9] not_eq nusers) {
				int nn = registerx[9] - registerx[8] + 1;

				if (!interactive) {

					//save hidden users for remote client in field 23
					var temp = (env.RECORD.extract(1)).field(VM, 1, registerx[8]);
					temp ^= VM ^ (env.RECORD.extract(1)).field(VM, registerx[9] + 1, 9999);
					temp.converter(VM, L",");
					env.RECORD.replacer(23, 0, 0, temp);

					//save hidden keys for remote client in field 23

					var visiblekeys = (env.RECORD.extract(2)).field(VM, registerx[8], nn);
					visiblekeys.converter(L",", VM);
					visiblekeys.trimmer(VM);

					var invisiblekeys = (env.RECORD.extract(2)).field(VM, 1, registerx[8]);
					invisiblekeys ^= VM ^ (env.RECORD.extract(2)).field(VM, registerx[9] + 1, 9999);
					invisiblekeys.converter(L",", VM);
					invisiblekeys.trimmer(VM);

					var otherkeys = L"";
					if (invisiblekeys) {
						var nkeys = invisiblekeys.count(VM) + 1;
						for (int keyn = 1; keyn <= nkeys; keyn++) {
							var keyx = invisiblekeys.extract(1, keyn);
							var xx;
							if (!(otherkeys.locateusing(keyx, VM, xx))) {
								if (!(visiblekeys.locateusing(keyx, VM, xx)))
									otherkeys ^= VM ^ keyx;
							}
						};//keyn;
						otherkeys.splicer(1, 1, L"");
						otherkeys.converter(VM, L",");
					}
					env.RECORD.replacer(24, 0, 0, otherkeys);

				}

				//delete higher and lower users if not allowed
				for (int fn = 1; fn <= 8; fn++)
					env.RECORD.replacer(fn, 0, 0, (env.RECORD.extract(fn)).field(VM, registerx[8], nn));

			}else{
				registerx[8] = L"";
				registerx[9] = L"";
			}

		}

		//get the local passwords from the system file for users that exist there
/*
		var usercodes = env.RECORD.extract(1);
		var nusers = usercodes.count(VM) + (usercodes not_eq L"");
		for (int usern = 1; usern <= nusers; usern++) {
			var user = usercodes.extract(1, usern);
			var sysrec = env.RECORD.extract(4, usern, 2);
			var sysrec2;
			if (sysrec2.read(env.DEFINITIONS, user)) {
				sysrec2.converter(FM ^ VM ^ SVM, TM ^ STM ^ L"ù");
				if (sysrec2 not_eq sysrec)
					env.RECORD.replacer(4, usern, 0, L"<hidden>" ^ SVM ^ sysrec2);
			}
		};//usern;
*/

		env.RECORD.replacer(20, 0, 0, registerx[8]);
		env.RECORD.replacer(21, 0, 0, registerx[9]);

		//pass possible menus to remote client
		var datax = L"";
		var menufilename;
		if (env.ACCOUNT == L"ADAGENCY") {
			menufilename = L"ADMENUS";
		}else{
			menufilename = L"MENUS";
		}
		var response=L"SELECT2 not implemented in MvWindow yet";
//TODO		select2(menufilename ^ L" USING MENUS", L"", L"WITH MENU.TITLE BY MENU.TITLE", L"MENU.TITLE ID", L"", datax, response, L"", L"", L"");
		if (response not_eq L"OK") {
//			response.transfer(msg);
//			return invalid(msg);
			return invalid(response);
		}
		datax.converter(FM ^ VM, VM ^ SVM);
		env.RECORD.replacer(22, 0, 0, datax);

		//group separators act as data in intranet client forcing menu and passwords
		for (int fn = 1; fn <= 2; fn++) {
			var temp = env.RECORD.extract(fn);
			temp.swapper(L"---", L"");
			env.RECORD.replacer(fn, 0, 0, temp);
		};//fn;

		//save orec (after removing stuff) for prewrite
		if (wlocked)
			//env.RECORD.invert().write(env.DEFINITIONS, L"SECURITY.OREC");
			env.RECORD.write(env.DEFINITIONS, L"SECURITY.OREC");

	}else if (mode == L"SAVE") {

		//called as prewrite in noninteractive mode

		//get/clear temporary storage
		registerx[8] = env.RECORD.extract(20);
		registerx[9] = env.RECORD.extract(21);

		if (!interactive) {
			if (!((registerx[7]).read(env.DEFINITIONS, L"SECURITY")))
				return invalid(L"SECURITY missing from env.DEFINITIONS");

			//registerx[7].inverter();

			//simulate orec
			if (!orec.read(env.DEFINITIONS, L"SECURITY.OREC"))
				return invalid(L"SECURITY.OREC is missing from DEFINITIONS");

			//orec.inverter();

			//safety check
			if (orec.extract(20) not_eq registerx[8] or orec.extract(21) not_eq registerx[9]) {

				//trace
				orec.write(env.DEFINITIONS, L"SECURITY.OREC.BAD");
				env.RECORD.write(env.DEFINITIONS, L"SECURITY.REC.BAD");

				return invalid(L"INTERNAL ERROR REC 20 AND 21 DO NOT AGREE WITH .OREC");
			}

	/*;
				//remove unauthorised users
				orec=origfullrec;
				if startn and endn then;
					n=endn-startn+1;
					for fn=1 to 8;
						orec<fn>=field(orec<fn>,vm,startn,n);
						next fn;
					end;

				//remove unauthorised tasks
	*/

		}

		securitysubs_cleartemp(env.RECORD);

		if (not ( interactive or templatex == L"SECURITY") ) {

			//check all users have names/passwords
			var usercodes = env.RECORD.extract(1);
			var nusers = usercodes.dcount(VM);
			for (int usern = 1; usern <= nusers; usern++) {

				//recover password
				if (!interactive) {

					var newpassword = env.RECORD.extract(4, usern);
					var usercode = env.RECORD.extract(1, usern);

					if (newpassword) {
						securitysubs_changepassx(newpassword, usercode, usern);

					}else{
						var oldusern;
						if (registerx[7].locateusing(usercode, VM, oldusern)) {
							var oldpassword = registerx[7].extract(4, oldusern);
							env.RECORD.replacer(4, usern, 0, oldpassword);
						}
					}

				}

				if (!(env.RECORD.extract(4, usern))) {
					var username = env.RECORD.extract(1, usern);
					if (!username) {
						//msg='user name is missing in line ':usern
						//return invalid(msg)
						username = L"---";
						env.RECORD.replacer(4, usern, 0, username);
					}
					if (!(username.index(L"---", 1) or username == L"BACKUP"))
						return invalid(username.quote() ^ L"|You must first give a password to this user");
				}
			};//usern;

			//mark empty users and keys with "---" to assist identification of groups
			nusers = (env.RECORD.extract(1)).dcount(VM);
			for (int usern = 1; usern <= nusers; usern++) {
				var temp = env.RECORD.extract(1, usern);
				if (temp == L"" or temp.index(L"---", 1)) {
					env.RECORD.replacer(1, usern, 0, L"---");
					env.RECORD.replacer(2, usern, 0, L"---");
				}
			};//usern;

			//put back any hidden users
			if (registerx[8]) {
				int nn = registerx[9] - registerx[8] + 1;
				var nvms = (env.RECORD.extract(1)).count(VM);
				for (int fn = 1; fn <= 8; fn++) {
					var temp = env.RECORD.extract(fn);
					temp ^= VM.str(nvms - temp.count(VM));
					env.RECORD.replacer(fn, 0, 0, (registerx[7].extract(fn)).fieldstore(VM, registerx[8], -nn, temp));
				};//fn;
			}

			//put back any hidden tasks
			var tasks = registerx[7].extract(10);
			var locks = registerx[7].extract(11);
			var ntasks = tasks.dcount(VM);
			for (int taskn = 1; taskn <= ntasks; taskn++) {
				var task = tasks.extract(1, taskn);
				if (task) {
					var newtaskn;
					if (!(orec.locateusing(task, VM, newtaskn, 10))) {
						var lockx = locks.extract(1, taskn);

						//locate task in @record<10> by 'al' setting newtaskn else null
						//lockx=locks<1,taskn>
						//@record=insert(@record,10,newtaskn,0,task)
						//@record=insert(@record,11,newtaskn,0,lockx)

						if (!(env.RECORD.locateby(task, L"AL", newtaskn, 10 ))) {
							env.RECORD.inserter(10, newtaskn, 0, task);
							env.RECORD.inserter(11, newtaskn, 0, lockx);
						}

					}
				}
			};//taskn;

		}
	/*;
			//delete any superfluous tasks
			tasks=@record<10>;
			locks=@record<11>;
			ntasks=count(tasks,vm)+(tasks ne '');
			for taskn=ntasks to 1 step -1;
				LOCKX=locks<1,taskn>;
				if LOCKX='' then;
					task=tasks<1,taskn>;
					if task[-1,1]='"' then;
						@record=delete(@record,10,taskn,0);
						@record=delete(@record,11,taskn,0);
						end;
					end;
				next taskn;
	*/
		//backup copy one per day
		var temp;
		if (temp.read(env.DEFINITIONS, L"SECURITY")) {
			var xx;
			if (!(xx.read(env.DEFINITIONS, L"SECURITY." ^ var().date())))
				temp.write(env.DEFINITIONS, L"SECURITY." ^ var().date());
			temp = L"";
		}

		env.RECORD.cropper();
		//dont save record in noninteractive mode as we are in prewrite stage
		if (interactive and env.RECORD)
			//env.RECORD.invert().write(env.DEFINITIONS, L"SECURITY");
			env.RECORD.write(env.DEFINITIONS, L"SECURITY");
		env.SECURITY = env.RECORD;

		var users;
		if (!users.open(L"USERS"))
			users = L"";
		//update users in the central system file if they exist there (direct login)
		var usercodes = env.RECORD.extract(1);
		//oswrite usercodes on 'x'
		var nusers = usercodes.dcount(VM);
		for (int usern = 1; usern <= nusers; usern++) {
			var usercode = usercodes.extract(1, usern);

			if (!(usercode.index(L"---", 1))) {

				//get the original and current system records
				var sysrec = env.RECORD.extract(4, usern, 2);
				//o=old
				var ousern;
				var osysrec;
				if (orec.locateusing(usercode, VM, ousern, 1)) {
					osysrec = orec.extract(4, ousern, 2);
				}else{
					osysrec = L"";
				}

				//update the users file
				if (users) {

					//get the current user record
					var userrec;
					if (!userrec.read(users, usercode))
						userrec = L"";
					var origuserrec = userrec;

					//determine the user department
					var deptcode = env.getuserdept(usercode).trim();
					deptcode.converter(L"0123456789", L"");

					//update the user record
					userrec.replacer(1, 0, 0, usercode);
					userrec.replacer(5, 0, 0, deptcode);
					userrec.replacer(11, 0, 0, usern);
					if (userrec not_eq origuserrec)
						userrec.write(users, usercode);
				}

				//make a new system record if this is a completely new user
				//if sysrec='' and osysrec='' then
				// newpassword=''
				// password.fn=7
				// last.fn=9
				// gosub makesysrec
				// end

				//update the central system file if user already exists there
				if (sysrec and sysrec not_eq osysrec and usercode not_eq env.ACCOUNT) {
					sysrec.converter(TM ^ STM ^ SSTM, FM ^ VM ^ SVM);
					if (temp.read(env.DEFINITIONS, usercode))
						sysrec.write(env.DEFINITIONS, usercode);
				}

			}
		};//usern;

		//delete any deleted users from the system file for direct login
		usercodes = orec.extract(1);
		nusers = usercodes.dcount(VM);
		for (int usern = 1; usern <= nusers; usern++) {
			var usercode = usercodes.extract(1, usern);
			if (!(usercode.index(L"---", 1))) {
				if (usercode and not usercode.index(L"NEOSYS", 1)) {
					if (!(env.RECORD.locateusing(usercode, VM, temp, 1))) {
						if (users) {
							users.deleterecord(usercode);
						}
						if (temp.read(env.DEFINITIONS, usercode)) {
							if (temp.extract(1) == L"USER") {
								(env.DEFINITIONS).deleterecord(usercode);
							}
						}
					}
				}
			}
		};//usern;

		if (!interactive) {

			//prepare to write the inverted record in noninteractive mode
			//env.RECORD.inverter();

			//remove the temp file
			env.DEFINITIONS.deleterecord(L"SECURITY.OREC");

		}

	}else if (mode.field(L".", 1, 1) == L"GETTASKS") {

		var disallowed = mode.field(L".", 2, 1) == L"NOT";
		var username = mode.field(L".", 3, 1);

		var origuser = env.USERNAME;
		if (username)
			env.USERNAME=username;

		var tasks2 = L"";
		var locks2 = L"";
		var tasks = env.SECURITY.extract(10);
		var locks = env.SECURITY.extract(11);
		var ntasks = tasks.count(VM) + 1;
		var lasttask = L"";
		for (int taskn = 1; taskn <= ntasks; taskn++) {
			var task = tasks.extract(1, taskn);
			var ok;
			if (env.authorised(task))
				ok = 1;
			else
				ok = 0;
			if (disallowed)
				ok = not ok;
			if (ok) {

				//shorten duplicated task names
				var task2 = task;
				var temp=L"";
				for (int ii = 1; ii <= 9; ii++) {
					temp = task.field(L" ", ii, 1);
					if (temp) {
						if (temp == lasttask.field(L" ", ii, 1)) {
							task.fieldstorer(L" ", ii, 1, L"%SAME%");
						}else{
							temp = L"";
						}
					}
				//BREAK;
				if (!temp) break;;
				};//ii;
				task ^= L"   ";
				task.swapper(L"%SAME% ", L"+");
				task = task.trimb();
				lasttask = task2;

				tasks2 ^= VM ^ task;
				locks2 ^= VM ^ locks.extract(1, taskn);
			}
		};//taskn;
		tasks2.splicer(1, 1, L"");
		locks2.splicer(1, 1, L"");
		//transfer tasks2 to @ans

		env.ANS = tasks2;
		tasks2 = L"";
		if (!disallowed)
			env.ANS ^= FM ^ locks2;

		if (username)
			env.USERNAME=origuser;

	}else if (1) {
		return invalid(mode.quote() ^ L" invalid mode in SECURITY.SUBS");
	}

	return true;
}

void MvWindow::securitysubs_changepassx(const var& newpassword, const var& usercode, const var& usern)
{

	var datax = env.RECORD.extract(4, usern);
	var sysrec = datax.extract(1, 1, 2);
	sysrec.converter(TM ^ STM ^ SSTM, FM ^ VM ^ SVM);
/*
	if (!sysrec) {
		if (!sysrec.read(systemfile(), user)) {
			sysrec = L"USER";
			sysrec.replacer(2, 0, 0, env.ACCOUNT);
			sysrec.replacer(5, 0, 0, L"NEOSYS");
		}
	}
*/

	var lastfn = 9;

	var passwordfn;

	if (sysrec.extract(1) == L"USER" or sysrec == L"") {
		passwordfn = 7;

	}else if (sysrec.extract(1) == L"ACCOUNT") {
		passwordfn = 6;

	}else if (1) {
		invalid(L"In changepassx, sysrec should contain either ACCOUNT or USER");
		return;
	}

//makesysrec:

	if (!(sysrec.extract(1))) {
		sysrec.replacer(1, 0, 0, passwordfn == 7 ? L"USER" : L"ACCOUNT");
	}
	if (!(sysrec.extract(2)))
		sysrec.replacer(2, 0, 0, env.ACCOUNT);
	if (!(sysrec.extract(5)))
		sysrec.replacer(5, 0, 0, L"NEOSYS");
	if (!sysrec.extract(lastfn))
		sysrec.replacer(lastfn, 0, 0, L"xxxxx");

	//store the encrypted new password
	var encryptx = env.encrypt2(newpassword);
	//gosub_makepass();
	sysrec.replacer(passwordfn, 0, 0, encryptx);

	encryptx = env.encrypt2(usercode ^ FM ^ sysrec.field(FM, 2, lastfn - 2));
	//gosub_makepass();
	sysrec.replacer(lastfn, 0, 0, encryptx);

	encryptx = env.encrypt2(usercode ^ FM ^ sysrec.field(FM, 2, lastfn - 2));
	//gosub_makepass();
	sysrec.replacer(lastfn, 0, 0, encryptx);

	//store the new password and system record
	sysrec.converter(FM ^ VM ^ SVM, TM ^ STM ^ SSTM);
	env.RECORD.replacer(4, usern, 2, sysrec);


	valid = 1;

}

void MvWindow::securitysubs_cleartemp(var& record)
{
	//@record
	//20 start
	//21 end
	//22 possible menus
	//23 other user codes
	//24 other keys
	for (int ii = 20; ii <= 24; ii++)
		record.replacer(ii, 0, 0, L"");
	return;
}

void MvWindow::select2(const var& filenamex, const var& linkfilename2, const var& sortselect0, const var& dictids0, const var& options, var& datax, var& response, const var& limitfields0, const var& limitchecks, const var& limitvalues)
{

	var v69;
	var v70;
	var v71;
	var storer;
	var storeid;
	var storedict;
	var storemv;
	var savesrcfile;
	var savedatafile;
	var savewlocked;
	var savemsg;
	var resetx;//num
	var savereset;
	var savevalid;
	var dictfilename;
	var linkfile2;
	var dictvoc;
	var realfilename;
	var triggers;
	var ndictids;
	var row;
	//jbase
	//nb add %selectlist% to sortselect to use active select list

	//given a sort/select statement
	//returns a dynamic array or file of xml data
	//needs something like the following in the calling program
	//$insert gbp,arev.common
	//clearcommon

	//declared high up outside range of goto exit statement
	var selectresult;
	var dataptr;
	var cmd;
	var xml;
	var oconvsx;
	var tx;
	var xx;
	var records;
	var recn;
	var postread;
	var postreadmode;
	var dictrecs;
	var library;

	var filename = filenamex;
	datax = L"";

	var crlf2 = var().chr(13) ^ var().chr(10);
	//crlf2=''
    var sortselect=sortselect0.swap(L"%SELECTLIST%", L"");
	var useactivelist = sortselect.index(L"%SELECTLIST%", 1);
	if (not env.LISTACTIVE)
		useactivelist = 0;

	if (!useactivelist)
//TODO:		pushselect(0, v69, v70, v71);

	env.RECORD.transfer(storer);
	env.ID.transfer(storeid);
	env.DICT.transfer(storedict);
	env.MV.transfer(storemv);

	srcfile.transfer(savesrcfile);
	datafile.transfer(savedatafile);
	wlocked.transfer(savewlocked);
	env.USER4.transfer(savemsg);
	resetx.transfer(savereset);
	valid.transfer(savevalid);

    var limitfields=limitfields0.unassigned()?L"":limitfields0;

    var dictids=dictids0.unassigned()?L"":dictids0;
	if (dictids == L"")
		dictids = L"ID";
	dictids.trimmer();
	dictids.converter(L" ", FM);
	dictrecs = L"";

	//filename can be 'filename using dictfilename'

	var filename0 = filename;
	if (filename.field(L" ", 2, 1) == L"USING") {
		dictfilename = filename.field(L" ", 3, 1);
		filename = filename.field(L" ", 1, 1);
	}else{
		dictfilename = filename;
	}

	response = L"OK";
	var file;
	if (!file.open(filename)) {
		response = L"Error: " ^ (filename.quote()) ^ L" file is not available";
		goto exit;
	}

	if (linkfilename2) {
			var(L"").oswrite(linkfilename2);
		if (!(linkfile2.osopen(linkfilename2))) {
			response = L"Error: " ^ (linkfilename2.quote()) ^ L" cannot open output file";
			goto exit;
		}
	}else{
		datax = L"";
	}
	dataptr = 0;

	cmd = L"SELECT " ^ filename0;
	//if trim(@station)='sbcp1800' then cmd='select 10 ':filename

	xml = options.index(L"XML", 1);

	oconvsx = L"";

	//check no @ in xml dict ids because cannot return xml tag with @
	if (xml and dictids.index(L"@", 1)) {
		response = L"Error: XML dictids cannot contain @ characters in SELECT2";
		goto exit;
	}

	if (!env.DICT.open(L"DICT", dictfilename)) {
		response = L"Error: " ^ ((L"DICT." ^ filename).quote()) ^ L" file is not available";
		goto exit;
	}

	//if (!env.openfile(L"DICT.VOC", dictvoc))
		dictvoc = L"";

	library="";
	if (!library.load(filename))
	{
		//throw L"MvWindow::select2() " ^ filename ^ L" unknown filename";
	}
	
	//check/get dict recs

	if (dictids not_eq L"RECORD") {
		while (dictids.substr(-1, 1) == FM) {
			dictids.splicer(-1, 1, L"");
		}
		if (dictids == L"")
			dictids = L"ID";
		ndictids = dictids.count(FM) + 1;
		for (int dictidn = 1; dictidn <= ndictids; dictidn++) {
			var dictid = dictids.extract(dictidn);
			var dictrec;
			if (!dictrec.read(env.DICT, dictid)) {
				if (!dictvoc||!dictrec.read(dictvoc, dictid)) {
					if (dictid == L"ID") {
						dictrec=var(L"F" _VM_ L"0" _VM_ L"No" _VM_ L"" _VM_ L"" _VM_ L"" _VM_ L"" _VM_ L"" _VM_ L"L" _VM_ L"15" _VM_ L"").raise();
					}else{
						response = L"Error: " ^ (dictid.quote()) ^ L" IS MISSING FROM DICT." ^ filename;
						goto exit;
					}
				}
			}

			//pick items
//TODO:			if (var(L"DI").index(dictrec.extract(1), 1))
//				dicti2a(dictrec);

			//pick a is revelation f
			if (dictrec.extract(1) == L"A")
				dictrec.replacer(1, 0, 0, L"F");

			dictrec.lowerer();
			dictrecs.replacer(dictidn, 0, 0, dictrec);
			oconvsx.replacer(dictidn, 0, 0, dictrec.extract(1, 7));
		};//dictidn;
	}

	tx = L"";
	if (xml and linkfilename2) {
		//tx:='<xml id=':quote(lcase(filename)):'>':crlf
		tx ^= L"<records>" ^ crlf2;
		env.osbwritex(tx, linkfile2, linkfilename2, dataptr);
	}
	dataptr += tx.length();

	//zzz should for validity of select parameters first
	//otherwise in server mode it loops with a very long error message

	//perform 'select schedules with brand_code "lu" by year_period and with id ne "[*i" by id'

	//if filename='jobs' or filename='companies' then

	if (xx.read(env.DICT, L"env.authorised")) {
		if (!(sortselect.index(L" WITH env.authorised", 1))) {
			if (var(L" " ^ sortselect).index(L" WITH ", 1))
				sortselect ^= L" AND";
			sortselect ^= L" WITH env.authorised";
		}
	}

	//if not sorted then try use %records% if present and <200 chars
	records = L"";
	//if @list.active or index(' ':sortselect,' by ',1) or index(sortselect,'with env.authorised',1) else
	if (!(env.LISTACTIVE or var(L" " ^ sortselect).index(L" BY ", 1))) {
		if (records.read(file, L"%RECORDS%")) {
			if (records.length() < 200) {
				records.swapper(FM, L"\" \"");
				sortselect.splicer(1, 0, records.quote() ^ L" ");
			}
		}
	}

	//if @list.active else call safeselect(cmd:' ':sortselect:' (s)')
	//oswrite cmd:' ':sortselect on 'x'
//TODO:	if (not env.LISTACTIVE or sortselect)
//		safeselect(cmd ^ L" " ^ sortselect ^ L" (S)");
    filename.select();

	//handle invalid cmd
	//r18.1 is normal 'no records found' message
	if (env.USER4 and not env.USER4.index(L"R18.1", 1)) {
		if (env.USER4.field(L" ", 1, 1) == L"W156")
			env.USER4 = (env.USER4.field(L" ", 2, 1)).quote() ^ L" is not in the dictionary.||" ^ cmd ^ L" " ^ sortselect;
		response = env.USER4;
		goto exit;
	}

	//return empty results even if no records selected
	//if @list.active then

	//moved up var selectresult = L"";

	recn = L"";

	datafile = filename;
	srcfile = file;

	//read each record and add the required columns to the selectresult
selectnext:

	if (var(L"").readnext(env.ID,env.MV)) {

		if (env.RECORD.read(file, env.ID)) {

			//filter out unwanted multivalues that the stupid rev sortselect leaves in
			if (limitfields) {
				var nlimitfields = limitfields.count(VM) + 1;
				var value, reqvalue, limitcheck;
				for (int limitfieldn = 1; limitfieldn <= nlimitfields; limitfieldn++) {
					value = var(limitfields.extract(1, limitfieldn)).calculate();
					reqvalue = limitvalues.extract(1, limitfieldn);
					limitcheck = limitchecks.extract(1, limitfieldn);
					if (limitcheck == L"EQ") {
						if (value not_eq reqvalue)
							goto selectnext;

					}else if (limitcheck == L"NE") {
						if (value == reqvalue)
							goto selectnext;

					}else if (1) {
						env.mssg(limitcheck.quote() ^ L" invalid limitcheck in select2");
						goto exit;
					}
				};//limitfieldn;
			}

			recn += 1;

			if (dictids == L"RECORD") {

				//postread (something similar also in listen/read)
				if (library) {

					//simulate window environment for postread
					orec = env.RECORD;
					wlocked = 1;
					env.USER4 = L"";
					resetx = 0;

                    //dictlib(L"POSTREAD");
					library.call(filename,"POSTREAD");

					env.DATA = L"";

					//call trimexcessmarks(iodat)

					//postread can request abort by setting msg or reset>=5
					if (resetx >= 5 or env.USER4)
						goto selectnext;

				}

				//prevent reading passwords postread and postwrite
				if (filename == L"DEFINITIONS" and env.ID == L"SECURITY")
					env.RECORD.replacer(4, 0, 0, L"");

				env.RECORD.transfer(row);

				var prefix = env.ID ^ FM;

				if (dataptr)
					prefix.splicer(1, 0, RM);
				row.splicer(1, 0, prefix);

			}else{
				row = L"";

				for (int dictidn = 1; dictidn <= ndictids; dictidn++) {
					var dictid = dictids.extract(dictidn);
					var dictid2 = dictid;
					dictid2.converter(L"@", L"");
					var cell = L"";
//TODO:calculate					var cell = var(dictid).calculate();
                    if (dictid==L"ID")
id:
                        cell=env.ID;
					else if (dictrecs.extract(dictidn,1)=="F")
					{
						var fn=dictrecs.extract(dictidn,2);
						if (!fn) goto id;
						var vn=dictrecs.extract(dictidn,4).substr(1,1)==L"M";
						if (vn)
							vn=env.MV;
						else
							vn=0;
						cell=env.RECORD.extract(fn,vn);
					}
					else
					{
						//cell = dictlib(dictid);
						cell=library.call(dictid);
						//cell=env.ANS;
					}
					if (oconvsx.extract(dictidn))
						cell = cell.oconv(oconvsx.extract(dictidn));
					if (xml) {
						//cell='x'
						//convert "'":'".+/,()&%:-1234567890abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz' to '' in cell
						cell.swapper(L"%", L"%25");
						cell.swapper(L"<", L"&lt;");
						cell.swapper(L">", L"&rt;");
						//if cell then deb ug
						//cell=quote(str(cell,10))
						row ^= L"<" ^ dictid2 ^ L">" ^ cell ^ L"</" ^ dictid2 ^ L">" ^ crlf2;
					}else{
						row.replacer(1, dictidn, 0, cell);
					}
				};//dictidn;

				if (xml) {
					row = L"<RECORD>" ^ crlf2 ^ row ^ L"</RECORD>" ^ crlf2;
					row.swapper(L"&", L"&amp;");
					//swap "'" with "" in row
				}
				if (dataptr) {
					if (xml) {
					}else{
						row.splicer(1, 0, FM);
					}
				}else{
					if (xml) {
						row.splicer(1, 0, L"<records>" ^ crlf2);
						//row[1,0]='<xml id=':quote(lcase(filename)):'>':crlf2
					}
				}

			}

			if (linkfilename2) {
				env.osbwritex(row, linkfile2, linkfilename2, dataptr);

			}else{
				datax ^= row;
			}
			dataptr += row.length();

		}
		if (xml or datax.length() < 64000)
			goto selectnext;
	}

	// end

	if (xml and linkfilename2) {
		var tt = L"</records>";
		env.osbwritex(tt, linkfile2, linkfilename2, dataptr);
	}

	if (linkfilename2) {
		linkfile2.osclose();
		var().osflush();
	}

exit:

	savesrcfile.transfer(srcfile);
	savedatafile.transfer(datafile);
	savewlocked.transfer(wlocked);
	savemsg.transfer(env.USER4);
	savereset.transfer(resetx);
	savevalid.transfer(valid);

	storer.transfer(env.RECORD);
	storeid.transfer(env.ID);
	storedict.transfer(env.DICT);
	storemv.transfer(env.MV);

//TODO:	if (!useactivelist)
//		popselect(0, v69, v70, v71);

	return;

}

}
