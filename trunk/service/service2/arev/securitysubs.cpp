#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <systemfile.h>
#include <pop_up.h>
#include <inputchar.h>
#include <sortarray.h>
#include <usersubs.h>
#include <generalsubs.h>
#include <sysmsg.h>
#include <sendmail.h>
#include <swap.h>
#include <invertarray.h>
#include <reverse.h>
#include <setuser.h>
#include <drvready.h>
#include <noechomsg.h>
#include <inputbox.h>
#include <singular.h>

#include <gen.h>
#include <win.h>

var newpassword;
var msg;
var usern;//num
var USER;
var sysrec;
var passwordfn;//num
var taskn;//num
var task;
var keyx;
var temp;//num
var lowerkeys;
var alllocks;
var reply;//num
var ownkeys;
var xx;
var charx;
var datax;
var filename;
var defaultlock;
var ousern;//num
var newtaskn;
var userfields;
var nuserfields;
var email;
var newusers;
var userrec;
var origuserrec;
var isnew;//num
var replyto;
var attachfilename;
var deletex;
var errormsg;
var diskette;
var ok;//num
var autopassword;//num
var v10;
var buffer;

function main(io mode) {
	//jbase
	var interactive = not SYSTEM.a(33);

	//SEE AUTHORISATION_DICT in UI
	//@record/user grouped mvs
	//1=username
	//2=keys
	//3=expiry_date (was menu but menu moved to user<34>)
	//4=password
	//5=was auto workstation DOS ... now hourly rates
	//6=was sleeptime ... now allowable ip addresses eg 1.1.1.1,1.1.1.*
	//7=emailaddress
	//8=fullname
	//20=startn
	//21=endn
	//22=possible menus - not used
	//23=hidden users
	//24=otherkeys
	//25=passwordautoexpirydays

	//NB (not any more) valid companies are buffered in userprivs<9>

	win.valid = 1;

	//no validation for NEOSYS
	if ((win.registerx[6]).index("NEOSYS", 1)) {
		if (mode.substr(1, 4) == "VAL." and mode ne "VAL.USER") {
			return 0;
		}
	}

	if (mode == "GENERATEPASSWORD") {
		gosub generatepassword();
		win.is = newpassword;

	if (mode == "PERP") {

	}else if (win.registerx[3].a(1) == "NEXT.MV.WI") {
			win.mvnext = win.registerx[3].a(2);
			win.winext = win.registerx[3].a(3);
			win.reset = 4;
			win.registerx[3] = "";
			return 0;
		}

		if (not win.amvaction) {
			return 0;
		}

		if (win.templatex ne "SECURITY") {
			msg = "YOU CANNOT ADD OR DELETE USERS HERE|GOTO AUTHORISATION";
			gosub EOF_381();
			goto perpexit;
		}

		if ((win.registerx[6]).index("NEOSYS", 1)) {
			goto perpexit;
		}

		//check if allowed to do this item
		if (win.amvaction == 2) {
			if (win.si.a(4) <= 4) {
				if (authorised("USER DELETE", msg, "")) {
					gosub checkrank();
					if (win.valid and win.mvx == usern) {
						msg = "YOU CANNOT DELETE YOURSELF";
						gosub EOF_381();
					}
				}else{
					gosub EOF_381();
				}
			}else{
				gosub checkauthority();
			}
		}

		//prevent insert new users/groups above self
		if (win.amvaction == 3) {
			if (win.si.a(4) <= 4) {
				if (authorised("USER CREATE", msg, "")) {
					gosub checkrank();
					if (win.mvx <= usern) {
						msg = "YOU CANNOT INSERT LINES ABOVE YOUR OWN LINE";
						gosub EOF_381();
					}
				}else{
					gosub EOF_381();
				}
			}
		}

perpexit:
		if (not win.valid) {
			win.amvaction = 0;
		}

		//insert/delete any hidden columns
		if (win.si.a(4) <= 4) {
			if (win.amvaction == 2) {
				for (var fn = 5; fn <= 8; ++fn) {
					RECORD.eraser(fn, win.mvx);
				};//fn;

			if (win.amvaction == 3) {
				for (var fn = 5; fn <= 8; ++fn) {
			}else if (RECORD.a(fn)) {
						RECORD.inserter(fn, win.mvx, "");
					}
				};//fn;
			}
		}

		goto 9142;
	}
	if (mode == "MAKESYSREC") {
		USER = win.is.a(1);
		newpassword = win.is.a(2);
		sysrec = "";
		passwordfn = 7;
		var lastfn = 9;
		gosub makesysrec();
		win.is = sysrec;

		goto 9142;
	}
	if (mode == "VAL.TASK") {
		if (win.is == win.isorig) {
			return 0;
		}

		//check if have authority
		gosub checkauthority();
		if (not win.valid) {
			return 0;
		}

		//check new task is not a hidden disallowed task
		if (win.registerx[7].a(10).locateusing(win.is, VM, taskn)) {
			msg = "YOU ARE NOT AUTHORISED TO DO THIS TASK";
			goto EOF_357;
		}

		goto 9142;
	}
	if (mode == "VAL.LOCKS") {
		if (win.is == win.isorig) {
			return 0;
		}
		win.is.converter(" ", "");

		//check if have authority
		gosub checkauthority();
		if (not win.valid) {
			return 0;
		}

		//if user appears to be locking themself out then add new keys to their list
		//(user has authority for this item)
		gen._security.r(11, win.mvx, win.is);
		if (not(authorised(task, msg, ""))) {

			//get higher & lower keys
			if (not(RECORD.a(1).locateusing(win.registerx[6], VM, usern))) {
				usern = 999;
			}
			gosub gethigherlowerkeys();

			//extract the additional keys needed by yourself
			var addkeys = "";
			var nkeys = win.is.count(",") + 1;
			for (var keyn = 1; keyn <= nkeys; ++keyn) {
				keyx = win.is.field(",", keyn);
				var isorig2 = win.isorig;
				isorig2.converter(",", VM);
				if (not(isorig2.a(1).locateusing(keyx, VM, temp))) {
					if (not(lowerkeys.a(1).locateusing(keyx, VM, temp))) {

						//sep 98
						if (not(win.registerx[5])) {

							if (alllocks.a(1).locateusing(keyx, VM, temp)) {
								msg = DQ ^ (keyx ^ DQ) ^ " - YOU CANNOT USE THIS KEY BECAUSE IT|IS \"OWNED\" BY SOMEBODY ELSE";
								goto EOF_381;
							}
						}
					}
					addkeys ^= "," ^ keyx;
				}
			};//keyn;

			//sep 98
			if (not(win.registerx[5] and win.registerx[4])) {

				//get users confirmation otherwise invalid
				msg = "The following key(s) will be added|to your personal list of keys.||(Otherwise you would lock|yourself out of this task)||" ^ addkeys.substr(2, 999) ^ "||IS THIS OK ?";
				if (not(decide("!" ^ msg, "Yes" _VM_ "No", reply))) {
					reply = 2;
				}
				if (not(reply == 1)) {
					win.is = win.isorig;
					win.valid = 0;
					return 0;
				}

				//add new keys to self
				if (RECORD.a(1).locateusing(win.registerx[6], VM, usern)) {
					temp = RECORD.a(2, usern);
					if (not temp) {
						addkeys.splicer(1, 1, "");
					}
					RECORD.r(2, usern, temp ^ addkeys);
				}
				gen._security = RECORD;

			}

			win.displayaction = 5;
		}

	if (mode == "VAL.USER") {

	}else if (win.is == win.isorig) {
			return 0;
		}

		if (win.is == win.isorig) {
			return 0;
		}
		if (win.amvaction == 2) {
			return 0;
		}

		//check rank
		gosub checkrank();
		if (not win.valid) {
			return 0;
		}

		win.is.trimmer();

		//if usern=mv then
		// msg='YOU CANNOT CHANGE YOUR OWN NAME'
		// goto invalid
		// end

		//group separator
		if (win.is == "---") {
			return 0;
		}

		//prevent false names
		if (not(UPPERCASE.index(win.is.substr(1.ucase(1)), 1))) {
			msg = "User name must start with a letter|or be a group separator \"---\" (three dashes)";
			goto EOF_381;
		}

		//check unique user names
		//locate is in delete(@record<1>,1,mv,0)<1> setting temp then
		var tt = gen._security.a(1) ^ VM ^ RECORD.a(1);
		if (tt.a(1).locateusing(win.is, VM, temp)) {
			msg = DQ ^ (win.is ^ DQ) ^ " this user already exists";
			goto EOF_381;
		}

		if (win.is.index("NEOSYS", 1) and not (win.registerx[6]).index("NEOSYS", 1)) {
			goto baduser;
		}
		//prevent use of reserved user names
		if (temp.read(systemfile(), win.is)) {
			if (not((win.registerx[6]).index("NEOSYS", 1))) {
baduser:
				msg = DQ ^ (win.is ^ DQ) ^ " - USER NAME IS RESERVED AND CANNOT BE USED";
				goto EOF_381;
			}
		}

		goto 9142;
	}
	if (mode == "F2.KEYS") {

		//get list of higher/lower keys
		var higherkeys = RECORD.a(2).field(VM, 1, win.mvx - 1);
		if (higherkeys.index("---", 1)) {
			temp = (field2(higherkeys, "-", -1)).length();
			higherkeys.splicer(1, higherkeys.length() - temp, "");
		}
		lowerkeys = RECORD.a(2).field(VM, win.mvx + 1, 999);
		temp = lowerkeys.index("---", 1);
		if (temp) {
			lowerkeys = lowerkeys.substr(1, temp);
		}
		var otherkeys = higherkeys ^ VM ^ lowerkeys;
		otherkeys.converter(",", VM);

		//get a sorted list of locks being available
		//(exclude keys owned by seniors and juniors is same group)
		var locklist = "";
		var popupdata = "";
		alllocks = RECORD.a(11);
		var ntasks = alllocks.count(VM) + 1;
		for (var taskn = 1; taskn <= ntasks; ++taskn) {
			var locks = alllocks.a(1, taskn);
			if (locks) {
				var nlocks = locks.count(",") + 1;
				for (var lockn = 1; lockn <= nlocks; ++lockn) {
					var lockx = locks.field(",", lockn);
					if (not(otherkeys.a(1).locateusing(lockx, VM, temp))) {
						if (not(locklist.locatebyusing(lockx, "AL", temp, FM))) {
							locklist.inserter(temp, lockx);
							popupdata.inserter(temp, lockx ^ VM ^ VM ^ RECORD.a(10, taskn));
						}
					}
				};//lockn;
			}
		};//taskn;

		//add flag if we already have the lock
		var ownlocks = RECORD.a(2, win.mvx);
		if (ownlocks) {
			var nownlocks = ownlocks.count(",") + 1;
			for (var ownlockn = 1; ownlockn <= nownlocks; ++ownlockn) {
				var ownlock = ownlocks.field(",", ownlockn);
				if (locklist.locateusing(ownlock, FM, temp)) {
					popupdata.r(temp, 2, "Yes");
				}
			};//ownlockn;
		}

		var options = "2:10:L::Include\\1:10:::Lock\\3:40:::Example task";
		var question = "Which lock(s) do you want ?";
		popupdata = pop_up(5, 5, "", popupdata, options, "R", "T" ^ FM ^ 2 ^ FM ^ 1, question, "", "", "", "");

		//get a list of those keys that have been selected
		var newkeys = "";
		var nlocks = popupdata.count(FM) + 1;
		for (var lockn = 1; lockn <= nlocks; ++lockn) {
			if (popupdata.a(lockn, 2) == "Yes") {
				newkeys ^= "," ^ popupdata.a(lockn, 1);
			}
		};//lockn;
		newkeys.splicer(1, 1, "");
		ANS = newkeys;

		goto 9142;
	}
	if (mode == "VAL.KEYS") {
		if (win.is == win.isorig) {
			return 0;
		}
		win.is.converter(" ", "");

		//check group sep line
		if ((RECORD.a(1, win.mvx)).index("---", 1)) {
			msg = "You cannot enter information|on group separator lines";
			goto EOF_381;
		}

		//check rank
		gosub checkrank();
		if (not win.valid) {
			return 0;
		}

		//check that any NEW keys are either found lower (in the same group)
		// or if not then they are not found higher up (unless we have access to higher groups)
		gosub gethigherlowerkeys();

		var nkeys = win.is.count(",") + (win.is ne "");
		for (var keyn = 1; keyn <= nkeys; ++keyn) {
			keyx = win.is.field(",", keyn);
			var isorig2 = win.isorig;
			isorig2.converter(",", VM);
			if (not(isorig2.a(1).locateusing(keyx, VM, temp))) {

				if (keyx == "NEOSYS" and not USERNAME.index("NEOSYS", 1)) {
					msg = DQ ^ (keyx ^ DQ) ^ " - YOU CANNOT USE THIS KEY BECAUSE IT|IS \"OWNED\" BY NEOSYS";
					goto EOF_381;
				}

				if (not(lowerkeys.a(1).locateusing(keyx, VM, temp))) {
					//locate KEYX in higherkeys<1> setting temp then
					// msg=quote(KEYX):' - YOU CANNOT USE THIS KEY BECAUSE IT|IS "OWNED" BY SOMEBODY ELSE'
					// goto invalid
					// end

					//sep 98
					if (not(win.registerx[5])) {

						if (alllocks.a(1).locateusing(keyx, VM, temp)) {
							msg = DQ ^ (keyx ^ DQ) ^ " - YOU CANNOT USE THIS KEY BECAUSE IT|IS \"OWNED\" BY SOMEBODY ELSE";
							goto EOF_381;
						}
					}
				}
			}
		};//keyn;

		//check that we are not removing any keys
		//that are not held by juniors in the same group
		//and are used in the task list
		if (RECORD.a(1).locateusing(win.registerx[6], VM, usern)) {
			var save = RECORD;
			RECORD.r(4, win.mvx, win.is);
			gosub gethigherlowerkeys();
			RECORD = save;
			nkeys = win.isorig.count(",") + (win.isorig ne "");
			for (var keyn = 1; keyn <= nkeys; ++keyn) {
				keyx = win.isorig.field(",", keyn);
				var is2 = win.is;
				is2.converter(",", VM);
				if (not(is2.a(1).locateusing(keyx, VM, temp))) {
					if (not(ownkeys ^ VM ^ lowerkeys.a(1).locateusing(keyx, VM, temp))) {
						if (alllocks.a(1).locateusing(keyx, VM, temp)) {
							msg = DQ ^ (keyx ^ DQ) ^ " - You cannot remove keys that you need yourself";
							goto EOF_381;
						}
					}
				}
			};//keyn;
		}

		goto 9142;
	}
	if (mode == "PERSONAL.PASSWORD") {

		//check guest status
		gosub gueststatus();
		if (not win.valid) {
			var().stop();
		}

		RECORD = gen._security;

		//lock the security file
		if (not(lockrecord("", gen._definitions, "SECURITY"))) {
			msg = "You cannot change your password at|this time because somebody else is|updating authorisations.";
			msg.r(-1, "|(Try again after a few minutes)|");
			gosub EOF_381();
			var().stop();
		}

		//get the latest security info
		if (not(gen._security.read(gen._definitions, "SECURITY"))) {
			gen._security = "";
		}
		gen._security = gen._security.invert();

		//find the user
		var inauthfile = 1;
		if (not(RECORD.a(1).locateusing(USERNAME, VM, win.mvx))) {
			inauthfile = 0;
		}
		USER = USERNAME;
		gosub changepassx();

		if (win.valid) {

			if (inauthfile) {
				//update the password in the data set
				RECORD.invert().write(gen._definitions, "SECURITY");
			}

			//update the system password file if necessary
			//if @username<>@account then
			// read temp from system.file(),@username then
			// write SYSREC on system.file(),@username
			// end
			// end

		}

		xx = unlockrecord("", gen._definitions, "SECURITY");

		goto 9142;
	}
	if (mode == "DEF.PASSWORD") {
defpassword:
		//skip the password if they press any non editing keys
		//otherwise jump into centre screen message to get password
		cout << var().cursor(win.crtvx, win.crtvy);
		call inputchar(charx);

		if (MOVEKEYS.locateusing(charx, FM, temp)) {
			if (temp <= 9) {
revkey:
				DATA.splicer(1, 0, charx);
				return 0;
			}
		}else{
			if (INTCONST.locateusing(charx, FM, temp)) {
				DATA.splicer(1, 0, var().chr(13) ^ charx);
				return 0;
			}
			//allow insert/delete line
			if (charx == EDITKEYS.a(11) or charx == EDITKEYS.a(12)) {
				DATA.splicer(1, 0, var().chr(13) ^ charx);
				return 0;
			}
		}

		//refuse if protected
		if (win.si.a(18) == "VP") {
			call mssg("W810");
			return 0;
		}

		//check group sep line
		if ((RECORD.a(1, win.mvx)).index("---", 1)) {
			msg = "You cannot enter information|on group separator lines";
			goto EOF_381;
		}

		//check rank
		gosub checkrank();
		if (not win.valid) {
			goto defpassword;
		}

		//delete key means remove the password
		if (USERNAME == "NEOSYS") {
			if (charx == " " or charx == (0x00 ^ "S")) {
				if (not(decide("Do you want to remove the password", "", reply))) {
					reply = 2;
				}
				if (reply == 2) {
					goto defpassword;
				}
				DATA ^= " " "\r";
				return 0;
			}
		}

		//otherwise get new password and system record

		USER = RECORD.a(1, win.mvx);
}

subroutine changepassx() {
		datax = RECORD.a(4, win.mvx);
		sysrec = datax.a(1, 1, 2);
		sysrec.converter(TM ^ STM ^ SSTM, FM ^ VM ^ SVM);
		if (not sysrec) {
			if (not(sysrec.read(systemfile(), USER))) {
				sysrec = "USER";
				sysrec.r(2, ACCOUNT);
				sysrec.r(5, "NEOSYS");
			}
		}

		if (interactive) {
			gosub newpass2();
		}else{
			gosub newpass3();
		}

		if (win.valid) {
			//on screen the password shows as <hidden>
			win.is.r(1, 1, 1, "<hidden>");

			//store the new password and system record
			temp = sysrec;
			temp.converter(FM ^ VM ^ SVM, TM ^ STM ^ SSTM);
			win.is.r(1, 1, 2, temp);
			RECORD.r(4, win.mvx, win.is);

			//if mode='PERSONAL.PASSWORD' else
			// msg='THE NEW PASSWORD WILL ONLY BECOME EFFECTIVE'
			// gosub note
			// end

		}

		if (not(mode == "PERSONAL.PASSWORD")) {
			DATA ^= "" "\r";
		}

		//called from DEFINITION.SUBS POSTREAD for key SECURITY

	if (mode == "SETUP") {

		//check allowed access
	}else if (win.templatex.unassigned()) {
			win.templatex = "";
		}
		if (win.templatex == "SECURITY") {
			filename = "AUTHORISATION";
			defaultlock = "GS";
		}else{
			if (win.templatex == "HOURLYRATES") {
				filename = "HOURLY RATE";
				win.registerx[5] = 1;
				win.registerx[4] = 1;
				defaultlock = "TA";
			}else{
				msg = DQ ^ (win.templatex ^ DQ) ^ " unknown template in security.subs";
				goto EOF_381;
			}
		}

		if (not(authorised(filename ^ " ACCESS", msg, defaultlock))) {
			gosub EOF_381();
			if (interactive) {
				var().stop();
			}
			return;
		}

		if (win.templatex == "SECURITY") {

			//check guest status
			gosub gueststatus();
			if (not win.valid) {
				var().stop();
			}

			//check supervisor status
			win.registerx[4] = authorised("AUTHORISATION UPDATE LOWER GROUPS", msg, "");
			win.registerx[5] = authorised("AUTHORISATION UPDATE HIGHER GROUPS", msg, "");

		}

		//if logged in as account then same as logged in as NEOSYS
		if (var("012").index(PRIVILEGE, 1)) {
			win.registerx[6] = "NEOSYS";
		}else{
			win.registerx[6] = USERNAME;
		}

		//check security
		if (authorised(filename ^ " UPDATE", msg, defaultlock)) {
			if (interactive) {
				if (not(lockrecord("", gen._definitions, "SECURITY"))) {
					msg = "Somebody else is updating " ^ filename.lcase() ^ "|you may look at but not update the information";
					gosub EOF_426();
					goto protect;
				}
			}else{
				//wlocked=1 (done in LISTEN)
			}
		}else{
protect:
			for (var ii = 1; ii <= win.wcnt; ++ii) {
				if (win.ww[ii].a(18) ne "P") {
					win.ww[ii].r(18, "VP");
				}
			};//ii;

			if (not interactive) {
				win.wlocked = 0;
			}

		}

		if (not(gen._security.read(gen._definitions, "SECURITY"))) {
			gen._security = "";
		}
		gen._security = gen._security.invert();
		gosub cleartemp();

		//never send the passwords to browser
		//(restored in prewrite except for new passwords)
		if (not interactive) {
			RECORD.r(4, "");
		}

		//sort the tasks
		call sortarray(gen._security, "10" _VM_ "11");

		win.registerx[7] = gen._security;

		RECORD = gen._security;

		//@record<9>=curruser

		//don't delete users for hourly rates
		if (win.templatex == "HOURLYRATES") {
			win.registerx[5] = 1;
			win.registerx[4] = 1;
		}

		//delete disallowed tasks (except all master type user to see all tasks)
		if (not(win.registerx[5] and win.registerx[4])) {
			var tasks = RECORD.a(10);
			var locks = RECORD.a(11);
			var ntasks = tasks.count(VM) + (tasks ne "");
			for (var taskn = ntasks; taskn >= 1; --taskn) {
				task = tasks.a(1, taskn);
force error here TODO: check trigraph following;
				temp = (task.substr(1, 10) == "DOCUMENT: ") ? "#" : "";
				if (not(authorised("!" ^ temp ^ task, msg, ""))) {
					RECORD.eraser(10, taskn);
					RECORD.eraser(11, taskn);
				}
			};//taskn;
		}

		//hide higher/lower users
		if (not (win.registerx[6]).index("NEOSYS", 1)) {

			var usercodes = RECORD.a(1);
			var nusers = usercodes.count(VM) + (usercodes ne "");

			if (not(usercodes.a(1).locateusing(USERNAME, VM, usern))) {
				msg = USERNAME ^ " user not in in authorisation file";
				goto EOF_381;
				var().stop();
			}

			//hide higher users
			if (win.registerx[5]) {
				win.registerx[8] = 1;
			}else{
				win.registerx[8] = usern;
				while (true) {
				///BREAK;
				if (not(win.registerx[8] > 1 and RECORD.a(2, win.registerx[8] - 1) == "")) break;;
					win.registerx[8] -= 1;
				}//loop;
			}

			//hide lower users
			if (win.registerx[4]) {
				win.registerx[9] = nusers;
			}else{
				win.registerx[9] = usern;
				while (true) {
				///BREAK;
				if (not(win.registerx[9] < nusers and RECORD.a(1, win.registerx[9] + 1) ne "---")) break;;
					win.registerx[9] += 1;
				}//loop;
			}

			//extract out the allowable users and keys
			//also in prewrite

			if (win.registerx[8] ne 1 or win.registerx[9] ne nusers) {
				var nn = win.registerx[9] - win.registerx[8] + 1;

				if (not interactive) {

					//save hidden users for remote client in field 23
					temp = RECORD.a(1).field(VM, 1, win.registerx[8]);
					temp ^= VM ^ RECORD.a(1).field(VM, win.registerx[9] + 1, 9999);
					temp.converter(VM, ",");
					RECORD.r(23, temp);

					//save hidden keys for remote client in field 23

					var visiblekeys = RECORD.a(2).field(VM, win.registerx[8], nn);
					visiblekeys.converter(",", VM);
					visiblekeys = trim2(visiblekeys, VM);

					var invisiblekeys = RECORD.a(2).field(VM, 1, win.registerx[8]);
					invisiblekeys ^= VM ^ RECORD.a(2).field(VM, win.registerx[9] + 1, 9999);
					invisiblekeys.converter(",", VM);
					invisiblekeys = trim2(invisiblekeys, VM);

					var otherkeys = "";
					if (invisiblekeys) {
						var nkeys = invisiblekeys.count(VM) + 1;
						for (var keyn = 1; keyn <= nkeys; ++keyn) {
							keyx = invisiblekeys.a(1, keyn);
							if (not(otherkeys.a(1).locateusing(keyx, VM, xx))) {
								if (not(visiblekeys.a(1).locateusing(keyx, VM, xx))) {
									otherkeys ^= VM ^ keyx;
								}
							}
						};//keyn;
						otherkeys.splicer(1, 1, "");
						otherkeys.converter(VM, ",");
					}
					RECORD.r(24, otherkeys);

				}

				//delete higher and lower users if not allowed
				for (var fn = 1; fn <= 8; ++fn) {
					RECORD.r(fn, RECORD.a(fn).field(VM, win.registerx[8], nn));
				};//fn;

			}else{
				win.registerx[8] = "";
				win.registerx[9] = "";
			}

		}

		//get the local passwords from the system file for users that exist there
		//also get any user generated passwords
		var usercodes = RECORD.a(1);
		var nusers = usercodes.count(VM) + (usercodes ne "");
		for (var usern = 1; usern <= nusers; ++usern) {
			USER = usercodes.a(1, usern);
			sysrec = RECORD.a(4, usern, 2);
			var pass = USER.xlate("USERS", 4, "X");
			if (pass and pass ne sysrec.field(TM, 7)) {
				RECORD.r(4, usern, 2, sysrec.fieldstore(TM, 7, 1, pass));
			}
			var sysrec2;
			if (sysrec2.read(systemfile(), USER)) {
				sysrec2.converter(FM ^ VM ^ SVM, TM ^ STM ^ SSTM);
				if (sysrec2 ne sysrec) {
					RECORD.r(4, usern, "<hidden>" ^ SVM ^ sysrec2);
				}
			}
		};//usern;

		RECORD.r(20, win.registerx[8]);
		RECORD.r(21, win.registerx[9]);

		//enable flowing text keys in postread
		//restore comma format in prewrite
		var keys = RECORD.a(2);
		keys.converter(",", " ");
		RECORD.r(2, keys);

		if (not interactive) {

				/*;
				//pass possible menus to remote client
				DATAX='';
				if @account='ADAGENCY' then filename='ADMENUS' else filename='MENUS';
				call select2(filename:' USING MENUS','','WITH MENU.TITLE BY MENU.TITLE','MENU.TITLE ID','',DATAX,response,'','','');
				if response<>'OK' then;
					transfer response to msg;
					return invalid();
					end;
				convert \FE\:\FD\ to \FD\:\FC\ in DATAX;
				@record<22>=DATAX;
				*/

			//group separators act as data in intranet client forcing menu and passwords
			for (var fn = 1; fn <= 2; ++fn) {
				temp = RECORD.a(fn);
				temp.swapper("---", "");
				RECORD.r(fn, temp);
			};//fn;

			//save orec (after removing stuff) for prewrite
			if (win.wlocked) {
				RECORD.invert().write(gen._definitions, "SECURITY.OREC");
			}

		}

		//called as prewrite in noninteractive mode
		goto 9142;
	}
	if (mode == "SAVE") {

		//get/clear temporary storage
		win.registerx[8] = RECORD.a(20);
		win.registerx[9] = RECORD.a(21);

		if (not interactive) {
			if (not((win.registerx[7]).read(gen._definitions, "SECURITY"))) {
				msg = "SECURITY missing from DEFINITIONS";
				goto EOF_381;
			}
			win.registerx[7] = (win.registerx[7]).invert();

			//simulate orec
			if (win.orec.read(gen._definitions, "SECURITY.OREC")) {
			}else{
				msg = "SECURITY.OREC is missing from DEFINITIONS";
				goto EOF_381;
			}
			win.orec = win.orec.invert();

			//safety check
			if (win.orec.a(20) ne win.registerx[8] or win.orec.a(21) ne win.registerx[9]) {

				//trace
				win.orec.write(gen._definitions, "SECURITY.OREC.BAD");
				RECORD.write(gen._definitions, "SECURITY.REC.BAD");

				msg = "An internal error: REC 20 AND 21 DO NOT AGREE WITH .OREC";
				goto EOF_381;
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

		gosub cleartemp();

		//if locked then skip out
		if (interactive and (win.ww[2].a(18)).count("P")) {
			return;
		}

		if (not interactive or win.templatex == "SECURITY") {

			//check all users have names/passwords
			var usercodes = RECORD.a(1);
			var nusers = usercodes.count(VM) + (usercodes ne "");
			for (var usern = 1; usern <= nusers; ++usern) {

				//recover password
				if (not interactive) {

					USER = RECORD.a(1, usern);

					newpassword = RECORD.a(4, usern);
					if (newpassword and newpassword.length() < 4) {
						msg = DQ ^ (USER ^ DQ) ^ " user password cannot be less than " ^ 4;
						goto EOF_381;
					}

					if (not(win.registerx[7].a(1).locateusing(USER, VM, ousern))) {
						ousern = 0;
					}

					if (newpassword) {

						win.mvx = usern;
						win.is = "";
						gosub changepassx();

						//remove old password so that changing password TO THE SAME PASSWORD
						//still triggers update of users file log section
						if (ousern) {
							win.registerx[7].r(4, ousern, "");
						}

						//zap any user generated pass in case they reset it while security was locked
						if (ousern) {
							var users;
							if (users.open("USERS", "")) {
								//writev field(@record<4,usern,2>,tm,7) on users,user,4
								var("").writev(users, USER, 4);
							}
						}

					}else{
						//recover old password
						if (ousern) {
							var oldpassword = win.registerx[7].a(4, ousern);
							RECORD.r(4, usern, oldpassword);
						}
					}

				}

				if (not(RECORD.a(4, usern))) {
					var USERNAME = RECORD.a(1, usern);
					if (not USERNAME) {
						//msg='USER NAME IS MISSING IN LINE ':USERN
						//goto invalid
						USERNAME = "---";
						RECORD.r(4, usern, USERNAME);
					}
					if (not(USERNAME.index("---", 1) or USERNAME == "BACKUP")) {
						if (not(RECORD.a(7, usern))) {
							//msg=quote(username):'|You must first give a password to this user'
							msg = DQ ^ (USERNAME ^ DQ) ^ "|You must give an email or password for this user";
							goto EOF_381;
						}
					}
				}

				//check ALL emails in one batch
				win.is = RECORD.a(7);
				call usersubs("VAL.EMAIL");
				if (not win.valid) {
					return;
				}

			};//usern;

			//check if you are locking yourself out
			if (interactive) {
				var storeuserprivs = gen._security;
				gen._security = RECORD;
				if (authorised("AUTHORISATION ACCESS", msg, "")) {
					if (not(authorised("AUTHORISATION UPDATE", msg, ""))) {
						goto lockout;
					}
				}else{
lockout:
					var().chr(7).output();
					msg = "*** WARNING ***";
					msg.r(-1, "|You will lock yourself out of this");
					msg.r(-1, "authorisation program if you continue.");
					msg.r(-1, "|ARE YOU SURE YOU WANT TO DO THIS ?");
					if (not(decide(msg, "", reply))) {
						reply = 2;
					}
					if (not(reply == 1)) {
						win.valid = 0;
						gen._security = storeuserprivs;
						return;
					}
				}
			}

			//mark empty users and keys with "---" to assist identification of groups
			nusers = (RECORD.a(1)).count(VM) + (RECORD.a(1) ne "");
			for (var usern = 1; usern <= nusers; ++usern) {
				temp = RECORD.a(1, usern);
				if (temp == "" or temp.index("---", 1)) {
					RECORD.r(1, usern, "---");
					RECORD.r(2, usern, "---");
				}
			};//usern;

			//put back any hidden users
			if (win.registerx[8]) {
				var nn = win.registerx[9] - win.registerx[8] + 1;
				var nvms = (RECORD.a(1)).count(VM);
				for (var fn = 1; fn <= 8; ++fn) {
					temp = RECORD.a(fn);
					temp ^= VM.str(nvms - temp.count(VM));
					RECORD.r(fn, (win.registerx[7].a(fn)).fieldstore(VM, win.registerx[8], -nn, temp));
				};//fn;
			}

			//put back any hidden tasks
			var tasks = win.registerx[7].a(10);
			var locks = win.registerx[7].a(11);
			var ntasks = tasks.count(VM) + (tasks ne "");
			for (var taskn = 1; taskn <= ntasks; ++taskn) {
				task = tasks.a(1, taskn);
				if (task) {
					if (not(win.orec.a(10).locateusing(task, VM, newtaskn))) {
						var lockx = locks.a(1, taskn);

						//locate task in @record<10> by 'AL' setting newtaskn else null
						//LOCKX=locks<1,taskn>
						//@record=insert(@record,10,newtaskn,0,task)
						//@record=insert(@record,11,newtaskn,0,LOCKX)

						if (not(RECORD.a(10).locatebyusing(task, "AL", newtaskn, VM))) {
							RECORD.inserter(10, newtaskn, task);
							RECORD.inserter(11, newtaskn, lockx);
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
		if (temp.read(gen._definitions, "SECURITY")) {
			if (not(xx.read(gen._definitions, "SECURITY." ^ var().date()))) {
				temp.write(gen._definitions, "SECURITY." ^ var().date());
			}
			temp = "";
		}

		call cropper(RECORD);
		//dont save record in noninteractive mode as we are in prewrite stage
		if (interactive and RECORD) {
			RECORD.invert().write(gen._definitions, "SECURITY");
		}
		gen._security = RECORD;

		//enable flowing text keys in postread
		//restore comma format in prewrite
		var keys = RECORD.a(2);
		keys.converter(" ", ",");
		RECORD.r(2, keys);

		if (interactive and win.templatex ne "SECURITY") {
			return;
		}

		userfields = "";
		//userfields<-1>='Code:0'
		userfields.r(-1, "Name:1");
		userfields.r(-1, "Email:7");
		userfields.r(-1, "Dept:21");
		userfields.r(-1, "Expiry:35:[DATE,4*]");
		userfields.converter(":", VM);
		nuserfields = userfields.count(FM) + 1;
		email = "";
		newusers = "";

		var users;
		if (not(users.open("USERS", ""))) {
			users = "";
		}
		//update users in the central system file if they exist there (direct login)
		var usercodes = RECORD.a(1);
		var useremails = RECORD.a(7);
		var usernames = RECORD.a(8);
		var userpasswords = RECORD.a(4);
		//oswrite usercodes on 'x'
		var nusers = usercodes.count(VM) + (usercodes ne "");
		for (var usern = 1; usern <= nusers; ++usern) {
			USER = usercodes.a(1, usern);

			if (not(USER.index("---", 1))) {

				//get the original and current system records
				sysrec = RECORD.a(4, usern, 2);
				//locate user in orec<1> setting ousern then
				var menuid = "";
				if (win.registerx[7].a(1).locateusing(USER, VM, ousern)) {
					//oSYSREC=orec<4,ousern,2>
					var osysrec = win.registerx[7].a(4, ousern, 2);
				}else{
					var osysrec = "";

					//new users look for legacy menu in following (lower rank) users
					for (var usern2 = usern + 1; usern2 <= nusers; ++usern2) {
						var usercode2 = usercodes.a(1, usern2);
						if (usercode2) {
							if (not(menuid.readv(users, usercode2, 34))) {
								menuid = "";
							}
						}
					///BREAK;
					if (not(not menuid)) break;;
					};//usern2;

				}

				//update the users file
				if (users) {

					//get the current user record
					var userrec;
					if (not(userrec.read(users, USER))) {
						userrec = "";
						userrec.r(34, menuid);
					}
					origuserrec = userrec;

					//determine the user department
					call generalsubs("GETUSERDEPT," ^ USER);
					var departmentcode = ANS.trim();
					var departmentcode2 = departmentcode;
					departmentcode.converter("0123456789", "");

					var USERNAME = usernames.a(1, usern);
					if (not USERNAME) {
						USERNAME = USER;
					}

					//update the user record
					userrec.r(1, USERNAME);
					userrec.r(5, departmentcode);
					userrec.r(7, useremails.a(1, usern));
					//userrec<8>=username
					userrec.r(11, usern);
					userrec.r(21, departmentcode2);
					//expirydate
					userrec.r(35, RECORD.a(3, usern));

					//new password cause entry in users log to renable login if blocked
					//save historical logins/password resets in listen2 and security.subs
					//similar in security.subs and user.subs
					var userpass = userpasswords.a(1, usern);
					var ouserpass = win.registerx[7].a(4, ousern);
					if (userpass ne ouserpass) {
						//datetime=(date():'.':time() 'R(0)#5')+0
						var datetime = var().date() ^ "." ^ (var().time()).oconv("R(0)#5");
						userrec.inserter(15, 1, datetime);
						userrec.inserter(16, 1, SYSTEM.a(40, 2));

						var text = "OK New password by " ^ USERNAME ^ ".";
						userrec.inserter(18, 1, text);

						userrec.r(36, datetime);
					}

					if (userrec ne origuserrec) {
						userrec.write(users, USER);

						//similar code in user.subs and security.subs
	/////////////
updatemirror:
	/////////////
						//save the user keyed as username too
						//because we save the user name and executive code in many places
						//and we still need to get the executive email if they are a user
						var mirror = userrec.fieldstore(FM, 13, 5, "");
						mirror = userrec.fieldstore(FM, 31, 3, "");
						var mirrorkey = "%" ^ userrec.a(1).ucase() ^ "%";
						mirror.r(1, USER);
						mirror.write(users, mirrorkey);

						isnew = origuserrec.a(1) == "";
						//only warn about new users with emails (ignore creation of groups/testusers)
						gosub getusertx();

					}
				}

				//make a new system record if this is a completely new user
				//if SYSREC='' and oSYSREC='' then
				// newpassword=''
				// password.fn=7
				// last.fn=9
				// gosub makeSYSREC
				// end

				//update the central system file if user already exists there
				//comment this out to prevent change the NEOSYS password
				//if SYSREC and SYSREC ne oSYSREC and user<>@account then
				// convert \FB\:\FA\:\F9\ to \FE\:\FD\:\FC\ in SYSREC
				// read temp from system.file(),user then
				// write SYSREC on system.file(),user
				// end
				// end

			}
		};//usern;

		//delete any deleted users from the system file for direct login
		usercodes = win.orec.a(1);
		nusers = usercodes.count(VM) + (usercodes ne "");
		for (var usern = 1; usern <= nusers; ++usern) {
			USER = usercodes.a(1, usern);
			if (not(USER.index("---", 1))) {
				if (USER and not USER.index("NEOSYS", 1)) {
					if (not(RECORD.a(1).locateusing(USER, VM, temp))) {
						var userrec;
						if (userrec.read(users, USER)) {
							if (users) {
								users.deleterecord(USER);
							}
							isnew = -1;
							origuserrec = "";
							gosub getusertx();
						}
						if (temp.read(systemfile(), USER)) {
							if (temp.a(1) == "USER") {
								(systemfile()).deleterecord(USER);
							}
						}
					}
				}
			}
		};//usern;

		if (not interactive) {

			if (email) {
				call sysmsg("User File Amendments" ^ FM ^ email);

				//email new users if requested to do so
				if (newusers and RECORD.a(26)) {
					var nn!!! = newusers.count(FM) + 1;
					for (var ii!!! = 1; ii!!! <= nn!!!; ++ii!!!) {
						if (USERNAME == "NEOSYS") {
							replyto = "support@neosys.com";
						}else{
							replyto = USERNAME.xlate("USERS", 7, "X");
						}
						var toaddress = newusers.a(ii!!!, 3);
						var ccaddress = replyto;
						var subject = "NEOSYS New User " ^ newusers.a(ii!!!, 1);

						var body = "";
						body.r(1, -1, "A new NEOSYS user account has been created for you.");

						body ^= VM;
						body.r(1, -1, "Username: %USERNAME%");
						body.r(1, -1, "Email:    %EMAIL%");

						body ^= VM;
						body.r(1, -1, "Login:");
						var baselinks = SYSTEM.a(114);
						var baselinkdescs = SYSTEM.a(115);
						var nlinks = baselinks.count(VM) + (baselinks ne "");
						if (nlinks) {
							for (var linkn = 1; linkn <= nlinks; ++linkn) {
								body.r(1, -1, baselinkdescs.a(1, linkn) ^ " " ^ baselinks.a(1, linkn));
								//if @account='ACCOUNTS' then body:='?ACCOUNTS'
							};//linkn;
						}else{
							body.r(1, -1, "Please contact your IT support or colleagues");
						}

						body ^= VM;
						body.r(1, -1, "Password:");
						body.r(1, -1, "You must use \"Password Reset\" on the Login Screen");
						body.r(1, -1, "http://userwiki.neosys.com/index.php/How_to_reset_the_password%3F");

						body ^= VM;
						body.r(1, -1, "Browser Configuration REQUIRED");
						body.r(1, -1, "http://userwiki.neosys.com/index.php/gettingstarted");

						if (replyto) {
							body ^= VM;
							body.r(1, -1, "Support:");
							body.r(1, -1, replyto);
						}

						body.swapper("%USERNAME%", newusers.a(ii!!!, 1));
						body.swapper("%EMAIL%", toaddress);
						body.swapper(VM, var().chr(13));

						call sendmail(toaddress, ccaddress, subject, body, attachfilename, deletex, errormsg, replyto);

						if (errormsg) {
							call note(errormsg);
						}

					};//ii!!!;
				}

			}

			//prepare to write the inverted record in noninteractive mode
			RECORD = RECORD.invert();

			//remove the temp file
			gen._definitions.deleterecord("SECURITY.OREC");

		}

		goto 9142;
	}
	if (mode == "POSTAPP") {

		//also called in postwrite in noninteractive mode

		if (not(gen._security.read(gen._definitions, "SECURITY"))) {
			gen._security = "";
		}
		gen._security = gen._security.invert();
		if (interactive) {
			xx = unlockrecord("", gen._definitions, "SECURITY");
		}

		goto 9142;
	}
	if (mode == "EXPORT") {
		if (not(authorised("AUTHORISATION EXPORT", msg, ""))) {
			goto EOF_381;
		}
		gosub selectdiskette();
		if (not diskette) {
			return;
		}
		call oswrite(gen._security.invert(), diskette ^ "\\TASKAUTH.DAT");
		if (mv.STATUS) {
			msg = "EXPORT FAILED";
		}else{
			msg = "EXPORT SUCCESSFUL";
		}
		gosub EOF_426();

		goto 9142;
	}
	if (mode == "IMPORT") {
		if (not(authorised("AUTHORISATION IMPORT", msg, ""))) {
			goto EOF_381;
		}
		gosub selectdiskette();
		if (not diskette) {
			return;
		}

		//check that the date
		if ((diskette ^ "\\TASKAUTH.DAT").osfile().a(2) < var().date() - 31) {
			msg = "CANNOT IMPORT BECAUSE THE|AUTHORISATION DISK IS THE WRONG VERSION";
			goto EOF_381;
		}

		if (not(datax.osread(diskette ^ "\\TASKAUTH.DAT"))) {
			msg = "\"TASKAUTH.DAT\" - AUTHORISATION EXPORT FILE IS MISSING|(ARE YOU USING THE RIGHT DISKETTE ?)";
			gosub EOF_381();
			goto selectdiskette;
		}

		RECORD = datax.invert();
		win.displayaction = 5;

		win.reset = 7;
		msg = "IMPORT SUCCESSFUL";
		gosub EOF_426();
		datax.write(gen._definitions, "SECURITY");
		gen._security = datax.invert();

		goto 9142;
	}
	if (mode.field(".", 1) == "LISTAUTH") {
		var temprec = RECORD;
		temprec.r(4, "");

		//remove expired users
		var expirydates = temprec.a(3);
		var nn!!! = expirydates.count(VM) + 1;
		for (var ii!!! = nn!!!; ii!!! >= 1; --ii!!!) {
			var expirydate = expirydates.a(1, ii!!!);
			if (expirydate) {
				if (expirydate <= var().date()) {
					for (var fn = 1; fn <= 9; ++fn) {
						var tt!!! = temprec.a(fn);
						if (tt!!!) {
							temprec.r(fn, tt!!!.erase(1, ii!!!, 0));
						}
					};//fn;
				}
			}
		};//ii!!!;

		//add group marks
		temprec.r(1, swap(VM ^ VM, VM ^ "<hr/>" ^ VM, temprec.a(1)));

		//reverse multivalues
		//trim any multivalued fields with more than nusers multivalues
		var nusers = (temprec.a(1)).count(VM) + 1;
		var nfs = temprec.count(FM) + 1;
		for (var fn = 1; fn <= nfs; ++fn) {
			temprec.r(fn, temprec.a(fn).field(VM, 1, nusers));
		};//fn;
		temprec = invertarray(reverse(invertarray(temprec)));

		var tempkey = "SECURITY." ^ var(1000000).rnd() ^ "." ^ var().time() ^ ".$$$";
		temprec.write(gen._definitions, tempkey);
	//call oswrite(temp,'x')
		temp = "";

		var mode2 = mode.field(".", 2);
		var cmd = "LIST DEFINITIONS " ^ (DQ ^ (tempkey ^ DQ));
		if (mode2 == "TASKS") {
			cmd ^= " TASKS LOCKS";
		}else{
			//cmd:=' LIMIT USER_EXPIRY_DATE < ':quote(date() '[DATE,4*]')
			//cmd:=' REMOVE_EXPIRED_USERS'
			//cmd:=' ADD_GROUP_MARKS'
			//cmd:=' REVERSE_MULTIVALUES'
			cmd ^= " USERS FULL_NAME EMAIL_ADDRESS";
			cmd ^= " LAST_LOGIN_DATE_TIME LAST_LOGIN_LOCATION";
			cmd ^= " PASSWORD_AGE USER_EXPIRY_DATE KEYS";
		}
		cmd ^= " HEADING " ^ (DQ ^ ("LIST OF AUTHORISATIONS - " ^ mode2 ^ "   \'T\'   Page \'PL\'" ^ DQ));

		cmd ^= " ID-SUPP DBL-SPC";
		if (interactive) {
			cmd.splicer(1, 0, "GET ");
		}
		cmd.execute();

		gen._definitions.deleterecord(tempkey);

		goto 9142;
	}
	if (mode == "XREFAUTH") {
		if (RECORD ne win.orec) {
			msg = "Please save your changes first";
			goto EOF_381;
		}
		var("GET NEW NOHTML PRINTSECURITY").execute();

		goto 9142;
	}
	if (mode.field(".", 1) == "GETTASKS") {

		var disallowed = mode.field(".", 2) == "NOT";
		var USERNAME = mode.field(".", 3);

		var origuser = USERNAME;
		if (USERNAME) {
			call setuser(USERNAME);
		}

		var tasks2 = "";
		var locks2 = "";
		var tasks = gen._security.a(10);
		var locks = gen._security.a(11);
		var ntasks = tasks.count(VM) + 1;
		var lasttask = "";
		for (var taskn = 1; taskn <= ntasks; ++taskn) {
			task = tasks.a(1, taskn);
			if (authorised(task, "", "")) {
				ok = 1;
			}else{
				ok = 0;
			}
			if (disallowed) {
				ok = not ok;
			}
			if (ok) {

				//shorten duplicated task names
				var task2 = task;
				for (var ii = 1; ii <= 9; ++ii) {
					temp = task.field(" ", ii);
					if (temp) {
						if (temp == lasttask.field(" ", ii)) {
							task = task.fieldstore(" ", ii, 1, "%SAME%");
						}else{
							temp = "";
						}
					}
				///BREAK;
				if (not temp) break;;
				};//ii;
				task ^= "   ";
				task.swapper("%SAME% ", "+");
				task.trimmerb();
				lasttask = task2;

				tasks2 ^= VM ^ task;
				locks2 ^= VM ^ locks.a(1, taskn);
			}
		};//taskn;
		tasks2.splicer(1, 1, "");
		locks2.splicer(1, 1, "");
		//transfer tasks2 to @ans

		ANS = tasks2;
		tasks2 = "";
		if (not disallowed) {
			ANS ^= FM ^ locks2;
		}

		if (USERNAME) {
			call setuser(origuser);
		}

		goto 9142;
	}
	if (mode == "SHOWKEYSTASKS") {
		if (win.si.a(4) == 10 or win.si.a(4) == 11) {
			keyx = RECORD.a(11, win.mvx);
		}else{
			keyx = "";
		}
inpkey:
		call note2("What key do you want ?", "RC", keyx);
		if (not keyx) {
			return;
		}
		var ntasks = (RECORD.a(10)).count(VM) + 1;
		var tasklist = "";
		for (var taskn = 1; taskn <= ntasks; ++taskn) {
			var tkeys = RECORD.a(11, taskn);
			tkeys.converter(",", VM);
			if (tkeys.a(1).locateusing(keyx, VM, xx)) {
				tasklist.r(-1, RECORD.a(10, taskn));
			}
		};//taskn;
		if (not tasklist) {
			call mssg(DQ ^ (keyx ^ DQ) ^ " No tasks are locked with this key");
			goto inpkey;
		}
		if (not(decide("Key " ^ (DQ ^ (keyx ^ DQ)) ^ " locks these tasks.", "" ^ tasklist, reply))) {
			return;
		}
		goto inpkey;
		//locate tasklist<reply> in @record<10> setting taskn then
		// tstore='NEXT.MV.WI':fm:taskn:fm:3
		// @DATA=@MOVE.KEYS<4>:@MOVE.KEYS<5>
		// return
		// end

		goto 9142;
	}
	if (mode == "SETWORKSTATION" or mode == "SETSLEEP") {
		if (not(win.si.a(4) == 1)) {
			msg = "YOU MUST BE ON THE USER NAME|TO SET THE WORKSTATION OR SLEEP TIME";
			goto EOF_381;
		}

		if (mode == "SETWORKSTATION") {
			if (not(authorised("#AUTHORISATION SET USER WORKSTATION", msg, ""))) {
				goto EOF_381;
			}
			reply = RECORD.a(5, win.mvx);
			call note2("What is the workstation|number for " ^ RECORD.a(1, win.mvx) ^ " ?|(enter \"CURRENT\" for current station)|(blank for default)", "RCE", reply, "");
			if (reply == 0x1B) {
				return;
			}
			if (reply == "CURRENT") {
				reply = STATION;
			}
			RECORD.r(5, win.mvx, reply);
		}else{
			if (not(authorised("#AUTHORISATION SET USER SLEEP TIME", msg, ""))) {
				goto EOF_381;
			}
			reply = RECORD.a(6, win.mvx);
inpsleep:
			call note2("How many seconds before sleeping|for " ^ RECORD.a(1, win.mvx) ^ " ?|(blank for default)", "RCE", reply, "");
			if (reply == 0x1B) {
				return;
			}
			if (reply < 60 or reply > 1000000) {
				var().chr(7).output();
				call mssg("Please enter a number|between 60 and 1000000");
				goto inpsleep;
			}
			RECORD.r(6, win.mvx, reply);
		}

		goto 9142;
	}
	if (1) {
		msg = DQ ^ (mode ^ DQ) ^ " invalid mode in SECURITY.SUBS";
		goto EOF_381;

	}
L9142:
	return;
}

subroutine selectdiskette() {
	diskette = decide("Please insert diskette now.||Which diskette are you using ?|(Press Esc to cancel)", "A:" _VM_ "B:", reply);
	if (not diskette) {
		return;
	}
	if (not(drvready(diskette == "B:"))) {
		var().chr(7).output();
		call mssg("THERE IS A PROBLEM WITH DRIVE " ^ diskette ^ "|HAVE YOU INSERTED THE DISKETTE CORRECTLY ?");
		goto selectdiskette;
	}
	return;

}

subroutine checkauthority() {
	var storeuserprivs = gen._security;
	gen._security = RECORD;
	task = RECORD.a(10, win.mvx);
	if (not(authorised(task, msg, ""))) {
		msg = "YOU CANNOT CHANGE OR DELETE ITEMS|THAT YOU ARE NOT AUTHORISED TO DO";
		gosub EOF_381();
	}
	gen._security = storeuserprivs;
	return;

}

subroutine checkrank() {

	if (not(RECORD.a(1).locateusing(win.registerx[6], VM, usern))) {
		usern = 999;
	}
	if ((win.registerx[6]).index("NEOSYS", 1)) {
		return;
	}
	if (win.mvx < usern and not win.registerx[5]) {
badrank:
		msg = "YOU CAN ONLY CHANGE YOURSELF,|";
		msg ^= "OR LOWER USERS IN YOUR OWN GROUP";
		if (win.registerx[4]) {
			msg ^= ",|OR USERS IN LOWER GROUPS";
		}
		goto EOF_381;
	}

	//allow/prevent access to lower groups
	if (not(win.registerx[4])) {
		var startx = usern + 1;
		var number = win.mvx - usern;
		if (win.amvaction == 3) {
			number -= 1;
		}
		temp = RECORD.a(1).field(VM, startx, number);
		if (temp.index("---", 1)) {
			goto badrank;
		}
	}

	return;

}

subroutine gethigherlowerkeys() {

	//get higher keys
	if (usern == 1) {
		var higherkeys = "";
	}else{
		var higherkeys = RECORD.a(2).field(VM, 1, usern - 1);
		higherkeys.converter(",", VM);
	}

	//get own keys
	ownkeys = RECORD.a(2).field(VM, win.mvx, 999);
	ownkeys.converter(",", VM);

	//get lower keys
	lowerkeys = RECORD.a(2).field(VM, win.mvx + 1, 999);
	lowerkeys.converter(",", VM);

	//remove any keys in lower groups if not allowed to update lower groups
	//feb 97
	if (not(win.registerx[4])) {
		temp = lowerkeys.index("---", 1);
		if (temp) {
			lowerkeys.splicer(temp, 9999, "");
		}
	}

	//get all locks in use
	alllocks = RECORD.a(11) ^ VM ^ win.registerx[7].a(11);
	alllocks.converter(",", VM);

	return;

}

subroutine newpass2() {
	win.valid = 0;
	newpassword = "";
	var lastfn = 9;
	if (authorised("AUTHORISATION INVENT OWN PASSWORDS", msg, "")) {
		if (not(decide("Do you want a random password ?", "Yes|No (use your own)", reply))) {
			return;
		}
		autopassword = reply == 1;
	}else{
		autopassword = 1;
	}

	if (autopassword) {
genpass:
		gosub generatepassword();

		//show the new password
		var question = "The new password will be " ^ (DQ ^ (newpassword ^ DQ));
		question ^= "|Is this password acceptable to you ?";
		if (not(decide(question, "No - make a different password|Yes - use this one|Cancel", reply))) {
			return;
		}
		if (reply == 1) {
			goto genpass;
		}
		if (reply ne 2) {
			return;
		}

		//get the new password
inpautopassword:
		var().getcursor();
		call note2("What is the new password ?||<               >|(This is a check that you have|the new password correct)", "UB", buffer, "");
		cout << var().cursor(39, _CRTHIGH / 2);
		var newpassword2 = "";
		call noechomsg(newpassword2);
		call mssg("", "DB", buffer, "");
		v10.setcursor();
		buffer = "";

		if (not newpassword2) {
			var().chr(7).output();
			call mssg("Password has NOT been changed");
			return;
		}

		newpassword2.ucaser(;
		if (newpassword2 ne newpassword) {
			var().chr(7).output();
			reply = "Y";
			msg = "" _VM_ "The entry of the new password" _VM_ "did not match." _VM_ "" _VM_ "Do you want to re-enter it? [Y/N]" _VM_ "";
			call msg2(msg, "RC", reply, "");
			if (reply == "Y") {
				goto inpautopassword;
			}
			var().chr(7).output();
			call mssg("Password has NOT been changed");
			return;
		}
	}
}

subroutine newpass3() {
	win.valid = 0;
	lastfn = 9;

	if (sysrec.a(1) == "USER" or sysrec == "") {
		passwordfn = 7;
		goto 10014;
	}
	if (sysrec.a(1) == "ACCOUNT") {
		passwordfn = 6;

	if (1) {
		call mssg("W123", "", "", USER);
		return;
	}
L10014:

	var v12 = 0;

inpnewpassword:
	}else if (newpassword == "") {
inpnewpassword1:
		//get the new password
	/*;
			get.cursor(v10);
			call note2('What is the new password ?|<               >|(5 characters minimum)', 'UB', buffer, '');
			print @(39, @crthigh / 2):
			call noecho.msg(newpassword);
			call msg('', 'DB', buffer, '');
			put.cursor(v10);
	*/

		msg = "What is the new password ?|[               ]|(Alphanumeric, 5 char minimum)";
		var maxlen = 15;
		var show = 0;
		var allowablechars = LOWERCASE.substr(1, 26) ^ UPPERCASE.substr(1, 26) ^ "123456789";
		newpassword = "";
		var escx = "";
		call inputbox(msg, maxlen, show, allowablechars, newpassword, escx);

		buffer = "";

		//get the new password again unless the new password is null
		if (newpassword > "") {
			if (newpassword.length() < 5) {
				var().chr(7).output();
				call mssg("Passwords should be at|least 5 characters long");
				if (not(USERNAME.index("NEOSYS", 1))) {
					goto inpnewpassword1;
				}
			}
inpownpass2:

	/*;
				v13='';
				get.cursor(v10);
				call note('Please enter the new password again.|<               >|', 'UB', buffer, '');
				print @(39, @crthigh / 2):
				call noecho.msg(v13);
				call msg('', 'DB', buffer, '');
				put.cursor(v10);
	*/
			msg = "Re-enter the new password |[               ]|";
			maxlen = 15;
			show = 0;
			allowablechars = LOWERCASE.substr(1, 26) ^ UPPERCASE.substr(1, 26) ^ "123456789";
			var v13 = "";
			escx = "";
			call inputbox(msg, maxlen, show, allowablechars, v13, escx);

			buffer = "";

			if (newpassword == v13) {
				v12 = 0;
				goto 10342;
			}

			reply = "Y";
			var().chr(7).output();
			msg = "" _VM_ "The entry of the new password" _VM_ "did not match." _VM_ "" _VM_ "Do you want to re-enter it? [Y/N]" _VM_ "";
			call mssg(msg, "RC", reply, "");
			if (reply == "Y") {
				v12 = 1;
				goto 10342;
			}
			v12 = 0;
			newpassword = "";
			var().chr(7).output();
			call mssg("The password has NOT been changed");
		}

L10342:

		if (newpassword == "" or newpassword == 0x1B) {
			return;
		}
		newpassword.ucaser(;
	}

	if (v12) {
		newpassword = "";
		var v13 = "";
	}
	if (v12) {
		goto inpnewpassword;
	}
}

subroutine makesysrec() {
	if (not sysrec.a(1)) {
force error here TODO: check trigraph following;
		sysrec = sysrec ? (1) : (0.replace(0, if (passwordfn == 7) {, "USER", "ACCOUNT"));
	}
	if (not sysrec.a(2)) {
		sysrec.r(2, ACCOUNT);
	}
	if (not sysrec.a(5)) {
		sysrec.r(5, "NEOSYS");
	}
	//if SYSREC<last.fn> else SYSREC<last.fn>='xxxxx'
	if (not sysrec.a(9)) {
		sysrec.r(9, "xxxxx");
	}

	//store the encrypted new password
	var encryptx = newpassword;
	gosub makepass();
	sysrec.r(passwordfn, encryptx);

	encryptx = USER ^ FM ^ sysrec.field(FM, 2, lastfn - 2);
	gosub makepass();
	sysrec.r(lastfn, encryptx);

	encryptx = USER ^ FM ^ sysrec.field(FM, 2, lastfn - 2);
	gosub makepass();
	sysrec.r(lastfn, encryptx);

	win.valid = 1;

	return;

}

subroutine makepass() {
	var encryptkey = 1234567;

	//pass1
	while (true) {
	///BREAK;
	if (not(encryptx ne "")) break;;
		encryptkey = (encryptkey % 390001) * (encryptx.substr(1, 1)).seq() + 1;
		encryptx.splicer(1, 1, "");
	}//loop;

	//pass2
	while (true) {
		encryptx ^= var().chr(65 + (encryptkey % 50));
		encryptkey = (encryptkey / 50).floor();
	///BREAK;
	if (not encryptkey) break;;
	}//loop;

	return;

}

subroutine generatepassword() {
	var consonants = UPPERCASE.substr(1, 26);
	var vowels = "AEIOUY";
	consonants.converter(vowels ^ "QX", "");
	newpassword = "";
	for (var ii = 1; ii <= 4 / 2; ++ii) {
		newpassword ^= consonants.substr((consonants.length()).rnd() + 1, 1);
		newpassword ^= vowels.substr((vowels.length()).rnd() + 1, 1);
	};//ii;
	return;

}

subroutine gueststatus() {
	if (SYSTEM.a(21)) {
		msg = "YOU ARE A TEMPORARY GUEST|YOU CANNOT ACCESS AUTHORISATION";
		gosub EOF_381();
	}
	return;

}

subroutine cleartemp() {
	//set in postread (setup) for neosys.net
	//@record
	//20 start
	//21 end
	//22 possible menus (no longer since menus moved to users field <34>
	//23 other user codes
	//24 other keys
	for (var ii = 20; ii <= 24; ++ii) {
		RECORD.r(ii, "");
	};//ii;
	return;

}

subroutine getusertx() {

	if (not userrec.a(7)) {
		return;
	}

	//save to email new users about their login
	if (isnew > 0) {
		newusers.r(-1, USER ^ VM ^ userrec.a(1) ^ VM ^ userrec.a(7));
	}

	//dont notify everybody about @neosys.com, test users
	if ((userrec.a(7).ucase()).index("NEOSYS.COM", 1)) {
		return;
	}

	var tx = "";
	for (var fieldn = 1; fieldn <= nuserfields; ++fieldn) {
		var field = userfields.a(fieldn);
		var fn = field.a(1, 2);
		var old = (origuserrec.a(fn)).oconv(field.a(1, 3));
		var new = (userrec.a(fn)).oconv(field.a(1, 3));
		if (new ne old) {
			tx.r(-1, (field.a(1, 1) ^ ":").oconv("L#8"));
			tx ^= new;
			if (old) {
				tx ^= " (was:" ^ old ^ ")";
			}
		}
	};//fieldn;
	if (tx) {
		email.r(-1, FM ^ var("UserId:").oconv("L#8") ^ USER);
		if (isnew > 0) {
			email ^= " *CREATED*";
			goto 11144;
		}
		if (isnew < 0) {
			email ^= " *DELETED*";
		}else{
			//email:=' *AMENDED*'
		}
		if (origuserrec.a(1) == userrec.a(1)) {
			email.r(-1, var("Name:").oconv("L#8") ^ userrec.a(1));
		}
		email.r(-1, tx);
	}
	return;

}


libraryexit()