#include <exodus/library.h>
libraryinit()

#include "win.h"
#include "window.hpp"

function main(in mode)
{

	std::wcout<<L"MVMvWindow::securitysubs(const var& mode)"<<L" "<<mode<<std::endl;

	var interactive = ! SYSTEM.a(33);

	if (mode == L"SETUP") {

		//called as postread in noninteractive mode (from Security.subs)

		//determine filename and default lock
		var datafilename;
		var defaultlock;
		if (win.templatex.unassigned())
			win.templatex = L"";
		if (win.templatex == L"SECURITY") {
			datafilename = L"AUTHORISATION";
			defaultlock = L"GS";
		}else if (win.templatex == L"HOURLYRATES") {
			datafilename = L"HOURLY RATE";
			win.registerx(5) = 1;
			win.registerx(4) = 1;
			defaultlock = L"TS3";
		}else{
			var msg = win.templatex.quote() ^ L" unknown template in securitysubs";
			return invalid(msg);
		}

		//check allowed access
		var msg;
		if (!(mv.authorised(datafilename ^ L" ACCESS", msg, defaultlock))) {
			return invalid(msg);
		}

		if (win.templatex == L"SECURITY") {

			//check guest status
			if (SYSTEM.a(21)) {
				return invalid(L"YOU ARE A TEMPORARY GUEST|YOU CANNOT ACCESS AUTHORISATION");
			}

			//check supervisor status
			win.registerx(4) = mv.authorised(L"AUTHORISATION UPDATE LOWER GROUPS", msg, L"");
			win.registerx(5) = mv.authorised(L"AUTHORISATION UPDATE HIGHER GROUPS", msg, L"");

		}

		//if logged in as account then same as logged in as neosys
		if (var(L"012").index(mv.PRIVILEGE, 1)) {
			win.registerx(6) = L"NEOSYS";
		}else{
			win.registerx(6) = USERNAME;
		}

		//check security
		if (mv.authorised(datafilename ^ L" UPDATE", msg, defaultlock)) {
		}else{
			if (!interactive)
				win.wlocked = 0;

		}

		if (!SECURITY.read(DEFINITIONS, L"SECURITY"))
			SECURITY = L"";
		//SECURITY.inverter();

		securitysubs_cleartemp(RECORD);

		//never send the passwords to browser
		//(restored in prewrite except for new passwords)
		if (!interactive)
			RECORD.r(4, L"");

		//sort the tasks
//TODO		sortarray(SECURITY, L"10" ^ VM ^ L"11");

		win.registerx(7) = SECURITY;

		RECORD = SECURITY;

		//@record<9>=curruser

		//don't delete users for hourly rates
		if (win.templatex != L"SECURITY")
			return false;

		//delete disallowed tasks (except all master type user to see all tasks)
		if (!(win.registerx(5) and win.registerx(4))) {
			var tasks = RECORD.a(10);
			var locks = RECORD.a(11);
			var ntasks = tasks.dcount(VM);
			for (int taskn = ntasks; taskn >= 1; taskn--) {
				var task = tasks.a(1, taskn);
				var temp = (task.substr(1, 10) == L"DOCUMENT: ") ? L"#" : L"";
				if (!(mv.authorised(L"!" ^ temp ^ task, msg, L""))) {
					RECORD.eraser(10, taskn, 0);
					RECORD.eraser(11, taskn, 0);
				}
			};//taskn;
		}

		//hide higher/lower users
		if (not (win.registerx(6)).index(L"NEOSYS", 1)) {

			var usercodes = RECORD.a(1);
			var nusers = usercodes.dcount(VM);

			var usern;
			if (!(usercodes.locate(USERNAME, usern, 1)))
				return invalid(USERNAME ^ L" user not in in authorisation file");

			//hide higher users
			if (win.registerx(5)) {
				win.registerx(8) = 1;
			}else{
				win.registerx(8) = usern;
				while (true) {
				//BREAK;
				if (!(win.registerx(8) > 1 and RECORD.a(2, win.registerx(8) - 1) == L"")) break;;
					win.registerx(8) -= 1;
				}//loop;
			}

			//hide lower users
			if (win.registerx(4)) {
				win.registerx(9) = nusers;
			}else{
				win.registerx(9) = usern;
				while (true) {
				//BREAK;
				if (!(win.registerx(9) < nusers and RECORD.a(1, win.registerx(9) + 1) not_eq L"---")) break;;
					win.registerx(9) += 1;
				}//loop;
			}

			//extract out the allowable users and keys
			//also in prewrite

			if (win.registerx(8) not_eq 1 or win.registerx(9) not_eq nusers) {
				int nn = win.registerx(9) - win.registerx(8) + 1;

				if (!interactive) {

					//save hidden users for remote client in field 23
					var temp = (RECORD.a(1)).field(VM, 1, win.registerx(8));
					temp ^= VM ^ (RECORD.a(1)).field(VM, win.registerx(9) + 1, 9999);
					temp.converter(VM, L",");
					RECORD.r(23, temp);

					//save hidden keys for remote client in field 23

					var visiblekeys = (RECORD.a(2)).field(VM, win.registerx(8), nn);
					visiblekeys.converter(L",", VM);
					visiblekeys.trimmer(VM);

					var invisiblekeys = (RECORD.a(2)).field(VM, 1, win.registerx(8));
					invisiblekeys ^= VM ^ (RECORD.a(2)).field(VM, win.registerx(9) + 1, 9999);
					invisiblekeys.converter(L",", VM);
					invisiblekeys.trimmer(VM);

					var otherkeys = L"";
					if (invisiblekeys) {
						var nkeys = invisiblekeys.count(VM) + 1;
						for (int keyn = 1; keyn <= nkeys; keyn++) {
							var keyx = invisiblekeys.a(1, keyn);
							var xx;
							if (!(otherkeys.locate(keyx, xx, 1))) {
								if (!(visiblekeys.locate(keyx, xx, 1)))
									otherkeys ^= VM ^ keyx;
							}
						};//keyn;
						otherkeys.splicer(1, 1, L"");
						otherkeys.converter(VM, L",");
					}
					RECORD.r(24, otherkeys);

				}

				//delete higher and lower users if not allowed
				for (int fn = 1; fn <= 8; fn++)
					RECORD.r(fn, (RECORD.a(fn)).field(VM, win.registerx(8), nn));

			}else{
				win.registerx(8) = L"";
				win.registerx(9) = L"";
			}

		}

		//get the local passwords from the system file for users that exist there
/*
		var usercodes = RECORD.a(1);
		var nusers = usercodes.count(VM) + (usercodes not_eq L"");
		for (int usern = 1; usern <= nusers; usern++) {
			var user = usercodes.a(1, usern);
			var sysrec = RECORD.a(4, usern, 2);
			var sysrec2;
			if (sysrec2.read(DEFINITIONS, user)) {
				sysrec2.converter(FM ^ VM ^ SVM, TM ^ STM ^ SSTM);
				if (sysrec2 not_eq sysrec)
					RECORD.r(4, usern, 0, L"<hidden>" ^ SVM ^ sysrec2);
			}
		};//usern;
*/

		RECORD.r(20, win.registerx(8));
		RECORD.r(21, win.registerx(9));

		//pass possible menus to remote client
		var datax = L"";
		var menufilename;
		if (ACCOUNT == L"ADAGENCY") {
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
		RECORD.r(22, datax);

		//group separators act as data in intranet client forcing menu and passwords
		for (int fn = 1; fn <= 2; fn++) {
			var temp = RECORD.a(fn);
			temp.swapper(L"---", L"");
			RECORD.r(fn, temp);
		};//fn;

		//save win.orec (after removing stuff) for prewrite
		if (win.wlocked)
			//RECORD.invert().write(DEFINITIONS, L"SECURITY.win.orec");
			RECORD.write(DEFINITIONS, L"SECURITY.win.orec");

	}else if (mode == L"SAVE") {

		//called as prewrite in noninteractive mode

		//get/clear temporary storage
		win.registerx(8) = RECORD.a(20);
		win.registerx(9) = RECORD.a(21);

		if (!interactive) {
			if (!((win.registerx(7)).read(DEFINITIONS, L"SECURITY")))
				return invalid(L"SECURITY missing from DEFINITIONS");

			//win.registerx(7).inverter();

			//simulate win.orec
			if (!win.orec.read(DEFINITIONS, L"SECURITY.win.orec"))
				return invalid(L"SECURITY.win.orec is missing from DEFINITIONS");

			//win.orec.inverter();

			//safety check
			if (win.orec.a(20) not_eq win.registerx(8) or win.orec.a(21) not_eq win.registerx(9)) {

				//trace
				win.orec.write(DEFINITIONS, L"SECURITY.win.orec.BAD");
				RECORD.write(DEFINITIONS, L"SECURITY.REC.BAD");

				return invalid(L"INTERNAL ERROR REC 20 AND 21 DO NOT AGREE WITH .win.orec");
			}

	/*;
				//remove unauthorised users
				win.orec=origfullrec;
				if startn and endn then;
					n=endn-startn+1;
					for fn=1 to 8;
						win.orec<fn>=field(win.orec<fn>,vm,startn,n);
						next fn;
					end;

				//remove unauthorised tasks
	*/

		}

		securitysubs_cleartemp(RECORD);

		if (not ( interactive or win.templatex == L"SECURITY") ) {

			//check all users have names/passwords
			var usercodes = RECORD.a(1);
			var nusers = usercodes.dcount(VM);
			for (int usern = 1; usern <= nusers; usern++) {

				//recover password
				if (!interactive) {

					var newpassword = RECORD.a(4, usern);
					var usercode = RECORD.a(1, usern);

					if (newpassword) {
						securitysubs_changepassx(newpassword, usercode, usern);

					}else{
						var oldusern;
						if (win.registerx(7).locate(usercode, oldusern, 1)) {
							var oldpassword = win.registerx(7).a(4, oldusern);
							RECORD.r(4, usern, 0, oldpassword);
						}
					}

				}

				if (!(RECORD.a(4, usern))) {
					var username = RECORD.a(1, usern);
					if (!username) {
						//msg='user name is missing in line ':usern
						//return invalid(msg)
						username = L"---";
						RECORD.r(4, usern, 0, username);
					}
					if (!(username.index(L"---", 1) or username == L"BACKUP"))
						return invalid(username.quote() ^ L"|You must first give a password to this user");
				}
			};//usern;

			//mark empty users and keys with "---" to assist identification of groups
			nusers = (RECORD.a(1)).dcount(VM);
			for (int usern = 1; usern <= nusers; usern++) {
				var temp = RECORD.a(1, usern);
				if (temp == L"" or temp.index(L"---", 1)) {
					RECORD.r(1, usern, 0, L"---");
					RECORD.r(2, usern, 0, L"---");
				}
			};//usern;

			//put back any hidden users
			if (win.registerx(8)) {
				int nn = win.registerx(9) - win.registerx(8) + 1;
				var nvms = (RECORD.a(1)).count(VM);
				for (int fn = 1; fn <= 8; fn++) {
					var temp = RECORD.a(fn);
					temp ^= VM.str(nvms - temp.count(VM));
					RECORD.r(fn, (win.registerx(7).a(fn)).fieldstore(VM, win.registerx(8), -nn, temp));
				};//fn;
			}

			//put back any hidden tasks
			var tasks = win.registerx(7).a(10);
			var locks = win.registerx(7).a(11);
			var ntasks = tasks.dcount(VM);
			for (int taskn = 1; taskn <= ntasks; taskn++) {
				var task = tasks.a(1, taskn);
				if (task) {
					var newtaskn;
					if (!(win.orec.locate(task, newtaskn, 10))) {
						var lockx = locks.a(1, taskn);

						//locate task in @record<10> by 'al' setting newtaskn else null
						//lockx=locks<1,taskn>
						//@record=insert(@record,10,newtaskn,0,task)
						//@record=insert(@record,11,newtaskn,0,lockx)

						if (!(RECORD.locateby(task, L"AL", newtaskn, 10 ))) {
							RECORD.inserter(10, newtaskn, 0, task);
							RECORD.inserter(11, newtaskn, 0, lockx);
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
					if task[-1,1)='"' then;
						@record=delete(@record,10,taskn,0);
						@record=delete(@record,11,taskn,0);
						end;
					end;
				next taskn;
	*/
		//backup copy one per day
		var temp;
		if (temp.read(DEFINITIONS, L"SECURITY")) {
			var xx;
			if (!(xx.read(DEFINITIONS, L"SECURITY." ^ var().date())))
				temp.write(DEFINITIONS, L"SECURITY." ^ var().date());
			temp = L"";
		}

		RECORD.cropper();
		//dont save record in noninteractive mode as we are in prewrite stage
		if (interactive and RECORD)
			//RECORD.invert().write(DEFINITIONS, L"SECURITY");
			RECORD.write(DEFINITIONS, L"SECURITY");
		SECURITY = RECORD;

		var users;
		if (!users.open(L"USERS"))
			users = L"";
		//update users in the central system file if they exist there (direct login)
		var usercodes = RECORD.a(1);
		//oswrite usercodes on 'x'
		var nusers = usercodes.dcount(VM);
		for (int usern = 1; usern <= nusers; usern++) {
			var usercode = usercodes.a(1, usern);

			if (!(usercode.index(L"---", 1))) {

				//get the original and current system records
				var sysrec = RECORD.a(4, usern, 2);
				//o=old
				var ousern;
				var osysrec;
				if (win.orec.locate(usercode, ousern, 1)) {
					osysrec = win.orec.a(4, ousern, 2);
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
					var deptcode = mv.getuserdept(usercode).trim();
					deptcode.converter(L"0123456789", L"");

					//update the user record
					userrec.r(1, usercode);
					userrec.r(5, deptcode);
					userrec.r(11, usern);
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
				if (sysrec and sysrec not_eq osysrec and usercode not_eq ACCOUNT) {
					sysrec.converter(TM ^ STM ^ SSTM, FM ^ VM ^ SVM);
					if (temp.read(DEFINITIONS, usercode))
						sysrec.write(DEFINITIONS, usercode);
				}

			}
		};//usern;

		//delete any deleted users from the system file for direct login
		usercodes = win.orec.a(1);
		nusers = usercodes.dcount(VM);
		for (int usern = 1; usern <= nusers; usern++) {
			var usercode = usercodes.a(1, usern);
			if (!(usercode.index(L"---", 1))) {
				if (usercode and not usercode.index(L"NEOSYS", 1)) {
					if (!(RECORD.locate(usercode, temp, 1))) {
						if (users) {
							users.deleterecord(usercode);
						}
						if (temp.read(DEFINITIONS, usercode)) {
							if (temp.a(1) == L"USER") {
								(DEFINITIONS).deleterecord(usercode);
							}
						}
					}
				}
			}
		};//usern;

		if (!interactive) {

			//prepare to write the inverted record in noninteractive mode
			//RECORD.inverter();

			//remove the temp file
			DEFINITIONS.deleterecord(L"SECURITY.win.orec");

		}

	}else if (mode.field(L".", 1, 1) == L"GETTASKS") {

		var disallowed = mode.field(L".", 2, 1) == L"NOT";
		var username = mode.field(L".", 3, 1);

		var origuser = USERNAME;
		if (username)
			USERNAME=username;

		var tasks2 = L"";
		var locks2 = L"";
		var tasks = SECURITY.a(10);
		var locks = SECURITY.a(11);
		var ntasks = tasks.count(VM) + 1;
		var lasttask = L"";
		for (int taskn = 1; taskn <= ntasks; taskn++) {
			var task = tasks.a(1, taskn);
			var ok;
			if (mv.authorised(task))
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
				locks2 ^= VM ^ locks.a(1, taskn);
			}
		};//taskn;
		tasks2.splicer(1, 1, L"");
		locks2.splicer(1, 1, L"");
		//transfer tasks2 to @ans

		ANS = tasks2;
		tasks2 = L"";
		if (!disallowed)
			ANS ^= FM ^ locks2;

		if (username)
			USERNAME=origuser;

	}else if (1) {
		return invalid(mode.quote() ^ L" invalid mode in SECURITY.SUBS");
	}

	return true;
}

subroutine securitysubs_changepassx(const var& newpassword, const var& usercode, const var& usern)
{

	var datax = RECORD.a(4, usern);
	var sysrec = datax.a(1, 1, 2);
	sysrec.converter(TM ^ STM ^ SSTM, FM ^ VM ^ SVM);
/*
	if (!sysrec) {
		if (!sysrec.read(systemfile(), user)) {
			sysrec = L"USER";
			sysrec.r(2, ACCOUNT);
			sysrec.r(5, L"NEOSYS");
		}
	}
*/

	var lastfn = 9;

	var passwordfn;

	if (sysrec.a(1) == L"USER" or sysrec == L"") {
		passwordfn = 7;

	}else if (sysrec.a(1) == L"ACCOUNT") {
		passwordfn = 6;

	}else if (1) {
		invalid(L"In changepassx, sysrec should contain either ACCOUNT or USER");
		return;
	}

//makesysrec:

	if (!(sysrec.a(1))) {
		sysrec.r(1, passwordfn == 7 ? L"USER" : L"ACCOUNT");
	}
	if (!(sysrec.a(2)))
		sysrec.r(2, ACCOUNT);
	if (!(sysrec.a(5)))
		sysrec.r(5, L"NEOSYS");
	if (!sysrec.a(lastfn))
		sysrec.r(lastfn, L"xxxxx");

	//store the encrypted new password
	var encryptx = mv.encrypt2(newpassword);
	//gosub_makepass();
	sysrec.r(passwordfn, encryptx);

	encryptx = mv.encrypt2(usercode ^ FM ^ sysrec.field(FM, 2, lastfn - 2));
	//gosub_makepass();
	sysrec.r(lastfn, encryptx);

	encryptx = mv.encrypt2(usercode ^ FM ^ sysrec.field(FM, 2, lastfn - 2));
	//gosub_makepass();
	sysrec.r(lastfn, encryptx);

	//store the new password and system record
	sysrec.converter(FM ^ VM ^ SVM, TM ^ STM ^ SSTM);
	RECORD.r(4, usern, 2, sysrec);


	win.valid = 1;

}

subroutine securitysubs_cleartemp(var& record)
{
	//@record
	//20 start
	//21 end
	//22 possible menus
	//23 other user codes
	//24 other keys
	for (int ii = 20; ii <= 24; ii++)
		record.r(ii, L"");
	return;
}

libraryexit()

