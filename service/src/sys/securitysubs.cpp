#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <sortarray.h>
#include <systemfile.h>
#include <usersubs.h>
#include <securitysubs2.h>
#include <sysmsg.h>
#include <sendmail.h>
#include <reverse.h>
#include <dedup.h>
#include <singular.h>

#include <gen_common.h>
#include <win_common.h>

#include <window.hpp>

var newpassword;
var userx;
var sysrec;
var passwordfn;//num
var lastfn;//num
var filename;
var defaultlock;
var msg;
var usern;//num
var xx;
var ousern;//num
var newtaskn;
var userfields;
var nuserfields;
var emailtx;
var newusers;
var userrec;
var origuserrec;
var isnew;//num
var replyto;
var attachfilename;
var deletex;
var errormsg;
var ok;//num
var encryptx;
var op;
var op2;
var wspos;
var wsmsg;

function main(in mode) {
	//c sys
	#include <general_common.h>
	//global sysrec,lastfn,encryptx

	var interactive = not(SYSTEM.a(33));

	//SEE AUTHORISATION_DICT in UI
	//@record/user grouped mvs
	//1=username
	//2=keys
	//3=expiry_date (was menu but menu moved to userx<34>)
	//4=password
	//5=was auto workstation DOS - now hourly rates
	//6=was sleeptime - now allowable ip addresses eg 1.1.1.1,1.1.1.*
	//7=emailaddress
	//8=fullname
	//9=buffered keys
	//20=startn
	//21=endn
	//22=possible menus - not used
	//23=hidden users
	//24=otherkeys
	//25=passwordautoexpirydays (also see system<128>)
	//26=emailnewusers
	#define minpasswordchars 4
	#define emailnewusers RECORD.a(26)

	#define tstore win.registerx(3)
	#define updatelowergroups win.registerx(4)
	#define updatehighergroups win.registerx(5)
	#define curruser win.registerx(6)
	#define origfullrec win.registerx(7)
	#define startn win.registerx(8)
	#define endn win.registerx(9)

	//NB (not any more) valid companies are buffered in userprivs<9>

	win.valid = 1;

	//no validation for EXODUS
	if (curruser.index("EXODUS")) {
		if ((mode.substr(1,4) == "VAL.") and mode ne "VAL.USER") {
			return 0;
		}
	}

	if (mode == "GENERATEPASSWORD") {
		gosub generatepassword();
		win.is = newpassword;

	//only to allow maintenance mode
	} else if (mode == "VAL.USER") {
	} else if (mode == "PERP") {

	} else if (mode == "MAKESYSREC") {
		userx = win.is.a(1);
		newpassword = win.is.a(2);
		sysrec = "";
		passwordfn = 7;
		lastfn = 9;
		gosub makesysrec();
		win.is = sysrec;

	/*;
		CASE MODE='VAL.TASK';
			IF IS=IS.ORIG THEN RETURN;

			//check if have authority
			gosub checkauthority;
			if valid else return 0;

			//check new task is not a hidden disallowed task
			locate is in origfullrec<10> setting taskn then;
				msg='YOU ARE NOT AUTHORISED TO DO THIS TASK';
				goto invalidq;
				end;

		CASE MODE='VAL.LOCKS';
			IF IS=IS.ORIG THEN RETURN;
			convert ' ' to '' in is;

			//check if have authority
			gosub checkauthority;
			if valid else return 0;

			//if user appears to be locking themself out then add new keys to their list
			//(user has authority for this item)
			userprivs<11,MV>=IS;
			IF SECURITY(TASK,msg,'') ELSE;

				//get higher & lower keys
				locate curruser in @record<1> setting usern else usern=999;
				gosub gethigherlowerkeys;

				//extract the additional keys needed by yourself
				addkeys='';
				nkeys=count(is,',')+1;
				for keyn=1 to nkeys;
					KEYX=field(is,',',keyn);
					is.orig2=is.orig;
					convert ',' to @vm in is.orig2;
					locate KEYX in is.orig2<1> setting temp else;
						locate KEYX in lowerkeys<1> setting temp else;

							//sep 98
							if updatehighergroups else;

								locate KEYX in alllocks<1> setting temp then;
									msg=quote(KEYX):' - YOU CANNOT USE THIS KEY BECAUSE IT|IS "OWNED" BY SOMEBODY ELSE';
									return invalid(msg);
									end;
								end;
							end;
						addkeys:=',':KEYX;
						end;
					next keyn;

				//sep 98
				if updatehighergroups and updatelowergroups else;

					//get users confirmation otherwise invalid
					msg='The following key(s) will be added|to your personal list of keys.||(Otherwise you would lock|yourself out of this task)||':addkeys[2,999]:'||IS THIS OK ?';
					IF DECIDE('!':MSG,'Yes|No',REPLY) ELSE REPLY=2;
					IF REPLY=1 ELSE;
						IS=IS.ORIG;
						VALID=0;
						return 0;
						END;

					//add new keys to self
					locate curruser in @record<1> setting usern then;
						temp=@record<2,usern>;
						if temp else addkeys[1,1]='';
						@record<2,usern>=temp:addkeys;
						end;
					userprivs=@record;

					end;

				display.action=5;
				END;

		CASE MODE EQ 'VAL.USER';

			if is=is.orig then return 0;

			IF IS=IS.ORIG THEN RETURN;
			if amv.action=2 then return 0;

			//check rank
			gosub checkrank;
			if valid else return 0;

			is=trim(is);

			//if usern=mv then
			// msg='YOU CANNOT CHANGE YOUR OWN NAME'
			// goto invalid
			// end

			//group separator
			if is='---' then return 0;

			//prevent false names
			if index(@upper.case,ucase(is[1,1]),1) else;
				msg='User name must start with a letter|or be a group separator "---" (three dashes)';
				return invalid(msg);
				end;

			//check unique user names
			//locate is in delete(@record<1>,1,mv,0)<1> setting temp then
			tt=userprivs<1>:vm:@record<1>;
			locate is in tt<1> setting temp then;
				msg=quote(is):' this user already exists';
				return invalid(msg);
				end;

			if index(is,'EXODUS',1) and not(index(curruser,'EXODUS',1)) then goto baduser;
			//prevent use of reserved user names
			read temp from system.file(),is then;
				if index(curruser,'EXODUS',1) else;
	baduser:
					msg=QUOTE(IS):' - USER NAME IS RESERVED AND CANNOT BE USED';
					return invalid(msg);
					end;
				end;

		CASE MODE EQ 'VAL.KEYS';
			IF IS=IS.ORIG THEN RETURN;
			convert ' ' to '' in is;

			//check group sep line
			if index(@record<1,mv>,'---',1) then;
				msg='You cannot enter information|on group separator lines';
				return invalid(msg);
				end;

			//check rank
			gosub checkrank;
			if valid else return 0;

			//check that any NEW keys are either found lower (in the same group)
			// or if not then they are not found higher up (unless we have access to higher groups)
			gosub gethigherlowerkeys;

			nkeys=count(is,',')+(is ne '');
			for keyn=1 to nkeys;
				KEYX=field(is,',',keyn);
				is.orig2=is.orig;
				convert ',' to @vm in is.orig2;
				locate KEYX in is.orig2<1> setting temp else;

					if KEYX='EXODUS' and not(index(@username,'EXODUS',1)) then;
						msg=quote(KEYX):' - YOU CANNOT USE THIS KEY BECAUSE IT|IS "OWNED" BY EXODUS';
						return invalid(msg);
						end;

					locate KEYX in lowerkeys<1> setting temp else;
						//locate KEYX in higherkeys<1> setting temp then
						// msg=quote(KEYX):' - YOU CANNOT USE THIS KEY BECAUSE IT|IS "OWNED" BY SOMEBODY ELSE'
						// goto invalid
						// end

						//sep 98
						if updatehighergroups else;

							locate KEYX in alllocks<1> setting temp then;
								msg=quote(KEYX):' - YOU CANNOT USE THIS KEY BECAUSE IT|IS "OWNED" BY SOMEBODY ELSE';
								return invalid(msg);
								end;
							end;
						end;
					end;
				next keyn;

			//check that we are not removing any keys
			//that are not held by juniors in the same group
			//and are used in the task list
			locate curruser in @record<1> setting usern then;
				save=@record;
				@record<4,mv>=is;
				gosub gethigherlowerkeys;
				@record=save;
				nkeys=count(is.orig,',')+(is.orig#'');
				for keyn=1 to nkeys;
					KEYX=field(is.orig,',',keyn);
					is2=is;
					convert ',' to @vm in is2;
					locate KEYX in is2<1> setting temp else;
						locate KEYX in (ownkeys:vm:lowerkeys)<1> setting temp else;
							locate KEYX in alllocks<1> setting temp then;
								msg=quote(KEYX):' - You cannot remove keys that you need yourself';
								return invalid(msg);
								end;
							end;
						end;
					next keyn;
				end;

		case mode='PERSONAL.PASSWORD';

			//check guest status
			gosub gueststatus;
			if valid else stop;

			@record=userprivs;

			//lock the security file
			if lockrecord('',definitions,'SECURITY') else;
				msg='You cannot change your password at|this time because somebody else is|updating authorisations.';
				msg<-1>='|(Try again after a few minutes)|';
				gosub invalid;
				stop;
				end;

			//get the latest security info
			READ userprivs FROM DEFINITIONS,'SECURITY' ELSE userprivs='';
			userprivs=INVERT(userprivs);

			//find the user
			inauthfile=1;
			locate @username in @record<1> setting mv else inauthfile=0;
			userx=@username;
			gosub changepassx;

			if valid then;

				if inauthfile then;
					//update the password in the data set
					write invert(@record) on definitions,'SECURITY';
					end;

				//update the system password file if necessary
				if @username<>@account then;
					read temp from system.file(),@username then;
						write SYSREC on system.file(),@username;
						end;
					end;

				end;

			xx=unlockrecord('',definitions,'SECURITY');

		case mode='DEF.PASSWORD';
	def.password:
			//skip the password if they press any non editing keys
			//otherwise jump into centre screen message to get password
			print @(crt.vx,crt.vy):
			call input.char(charX);

			locate CHARX in @move.keys using fm setting temp then;
				if temp le 9 then;
	revkey:
					@data[1,0]=CHARX;
					return 0;
					end;
			end else;
				locate CHARX in @int.const using fm setting temp then;
					@data[1,0]=char(13):CHARX;
					return 0;
					end;
				//allow insert/delete line
				if charX=@edit.keys<11> or CHARX=@edit.keys<12> then;
					@data[1,0]=char(13):charX;
					return 0;
					end;
				end;

			//refuse if protected
			if si<18>='VP' then;
				call note('W810');
				return 0;
				end;

			//check group sep line
			if index(@record<1,mv>,'---',1) then;
				msg='You cannot enter information|on group separator lines';
				return invalid(msg);
				end;

			//check rank
			gosub checkrank;
			if valid else goto def.password;

			//delete key means remove the password
			if @username='EXODUS' then;
				if CHARX=' ' or CHARX=char(0):'S' then;
					if decide('Do you want to remove the password','',reply) else reply=2;
					if reply=2 then goto def.password;
					data ' ';
					return 0;
					end;
				end;

			//otherwise get new password and system record

			userx=@record<1,mv>;

	changepassx:
			DATAX=@record<4,mv>;
			SYSREC=DATAX<1,1,2>;
			convert \FB\:\FA\:char(249) to \FE\:\FD\:\FC\ in SYSREC;
			if SYSREC else;
				read SYSREC from system.file(),userx else;
					SYSREC='USER';
					SYSREC<2>=@account;
					SYSREC<5>='EXODUS';
					end;
				end;

			if interactive then;
				gosub newpass2;
			end else;
				gosub newpass3;
				end;

			if valid then;
				//on screen the password shows as <hidden>
				is<1,1,1>='<hidden>';

				//store the new password and system record
				temp=SYSREC;
				convert \FE\:\FD\:\FC\ TO \FB\:\FA\:char(249) in temp;
				is<1,1,2>=temp;
				@record<4,mv>=is;

				//if mode='PERSONAL.PASSWORD' else
				// msg='THE NEW PASSWORD WILL ONLY BECOME EFFECTIVE'
				// gosub note
				// end

				end;
	*/
		//if mode='PERSONAL.PASSWORD' else data ''

	//called from DEFINITION.SUBS POSTREAD for key SECURITY
	} else if (mode == "SETUP") {

		//check allowed access
		if (win.templatex.unassigned()) {
			win.templatex = "";
		}
		if (win.templatex == "SECURITY") {
			filename = "AUTHORISATION";
			defaultlock = "GS";
		}else{
			if (win.templatex == "HOURLYRATES") {
				filename = "HOURLY RATE";
				updatehighergroups = 1;
				updatelowergroups = 1;
				defaultlock = "TA";
			}else{
				msg = win.templatex.quote() ^ " unknown template in security.subs";
				return invalid(msg);
			}
		}

		if (not(authorised(filename ^ " ACCESS", msg, defaultlock))) {
			gosub invalid(msg);
			if (interactive) {
				stop();
			}
			return 0;
		}

		if (win.templatex == "SECURITY") {

			//check guest status
			gosub gueststatus();
			if (not(win.valid)) {
				stop();
			}

			//check supervisor status
			updatelowergroups = authorised("AUTHORISATION UPDATE LOWER GROUPS", msg, "");
			updatehighergroups = authorised("AUTHORISATION UPDATE HIGHER GROUPS", msg, "");

		}

		//if logged in as account then same as logged in as EXODUS
		if (var("012").index(PRIVILEGE)) {
			curruser = "EXODUS";
		}else{
			curruser = USERNAME;
		}

		//check security
		if (authorised(filename ^ " UPDATE", msg, defaultlock)) {
			//if interactive then
			// if LOCKrecord('',DEFINITIONS,'SECURITY') ELSE
			//  msg='Somebody else is updating ':lcase(filename):'|you may look at but not update the information'
			//  GOSUB NOTE
			//  GOTO PROTECT
			//  END
			//end else
			// *wlocked=1 (done in LISTEN)
			// end
		}else{
	//PROTECT:
			//FOR I=1 TO W.CNT
			// IF W(I)<18> NE 'P' THEN W(I)<18>='VP'
			// NEXT I

			if (not interactive) {
				win.wlocked = 0;
			}

		}

		if (not(SECURITY.read(DEFINITIONS, "SECURITY"))) {
			SECURITY = "";
		}
		SECURITY = SECURITY.invert();
		//in case not cleared in save/write
		gosub cleartemp();

		//never send the passwords to browser
		//(restored in prewrite except for new passwords)
		if (not interactive) {
			RECORD.r(4, "");
		}

		//sort the tasks
		call sortarray(SECURITY, "10" ^ VM ^ "11", "AL");

		origfullrec = SECURITY;

		RECORD = SECURITY;

		//@record<9>=curruser

		//don't delete users for hourly rates
		if (win.templatex == "HOURLYRATES") {
			updatehighergroups = 1;
			updatelowergroups = 1;
		}

		//delete disallowed tasks (except all master type user to see all tasks)
		if (not(updatehighergroups and updatelowergroups)) {
			var tasks = RECORD.a(10);
			var locks = RECORD.a(11);
			var ntasks = tasks.count(VM) + (tasks ne "");
			for (var taskn = ntasks; taskn >= 1; --taskn) {
				var task = tasks.a(1, taskn);
				//WARNING TODO: check ternary op following;
				var temp = (task.substr(1,10) == "DOCUMENT: ") ? "#" : "";
				if (not(authorised("!" ^ temp ^ task, msg, ""))) {
					RECORD.remover(10, taskn);
					RECORD.remover(11, taskn);
				}
			};//taskn;
		}

		//hide higher/lower users
		if (not(curruser.index("EXODUS"))) {

			var usercodes = RECORD.a(1);
			var nusers = usercodes.count(VM) + (usercodes ne "");

			if (not(usercodes.a(1).locate(USERNAME,usern))) {
				msg = USERNAME ^ " user not in in authorisation file";
				return invalid(msg);
				stop();
			}

			//hide higher users
			if (updatehighergroups) {
				startn = 1;
			}else{
				startn = usern;
				while (true) {
					///BREAK;
					if (not((startn > 1) and (RECORD.a(2, startn - 1) == ""))) break;
					startn -= 1;
				}//loop;
			}

			//hide lower users
			if (updatelowergroups) {
				endn = nusers;
			}else{
				endn = usern;
				while (true) {
					///BREAK;
					if (not((endn < nusers) and (RECORD.a(1, endn + 1) ne "---"))) break;
					endn += 1;
				}//loop;
			}

			//extract out the allowable users and keys
			//also in prewrite

			if (startn ne 1 or endn ne nusers) {
				var nn = endn - startn + 1;

				if (not interactive) {

					//save hidden users for remote client in field 23
					var temp = RECORD.a(1).field(VM, 1, startn);
					temp ^= VM ^ RECORD.a(1).field(VM, endn + 1, 9999);
					temp.converter(VM, ",");
					RECORD.r(23, temp);

					//save hidden keys for remote client in field 23

					var visiblekeys = RECORD.a(2).field(VM, startn, nn);
					visiblekeys.converter(",", VM);
					visiblekeys = trim(visiblekeys, VM);

					var invisiblekeys = RECORD.a(2).field(VM, 1, startn);
					invisiblekeys ^= VM ^ RECORD.a(2).field(VM, endn + 1, 9999);
					invisiblekeys.converter(",", VM);
					invisiblekeys = trim(invisiblekeys, VM);

					var otherkeys = "";
					if (invisiblekeys) {
						var nkeys = invisiblekeys.count(VM) + 1;
						for (var keyn = 1; keyn <= nkeys; ++keyn) {
							var keyx = invisiblekeys.a(1, keyn);
							if (not(otherkeys.a(1).locate(keyx,xx))) {
								if (not(visiblekeys.a(1).locate(keyx,xx))) {
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
					RECORD.r(fn, RECORD.a(fn).field(VM, startn, nn));
				};//fn;

			}else{
				startn = "";
				endn = "";
			}

		}

		//get the local passwords from the system file for users that exist there
		//also get any user generated passwords
		var usercodes = RECORD.a(1);
		var nusers = usercodes.count(VM) + (usercodes ne "");
		for (usern = 1; usern <= nusers; ++usern) {
			userx = usercodes.a(1, usern);
			sysrec = RECORD.a(4, usern, 2);
			var pass = userx.xlate("USERS", 4, "X");
			if (pass and pass ne sysrec.field(TM, 7)) {
				RECORD.r(4, usern, 2, sysrec.fieldstore(TM, 7, 1, pass));
			}
			var sysrec2;
			if (sysrec2.read(systemfile(), userx)) {
				//convert \FE\:\FD\:\FC\ TO \FB\:\FA\:\F9\ in SYSREC2
				sysrec2.converter(FM ^ VM ^ SVM, TM ^ STM ^ var().chr(249));
				if (sysrec2 ne sysrec) {
					RECORD.r(4, usern, "<hidden>" ^ SVM ^ sysrec2);
				}
			}
		};//usern;

		RECORD.r(20, startn);
		RECORD.r(21, endn);

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
					return invalid(msg);
					end;
				convert \FE\:\FD\ to \FD\:\FC\ in DATAX;
				@record<22>=DATAX;
				*/

			//group separators act as data in intranet client forcing menu and passwords
			for (var fn = 1; fn <= 2; ++fn) {
				var temp = RECORD.a(fn);
				temp.swapper("---", "");
				RECORD.r(fn, temp);
			};//fn;

			//save orec (after removing stuff) for prewrite
			if (win.wlocked) {
				RECORD.invert().write(DEFINITIONS, "SECURITY.OREC");
			}

		}

	//called as prewrite in noninteractive mode
	} else if (mode == "SAVE") {

		//get/clear temporary storage
		startn = RECORD.a(20);
		endn = RECORD.a(21);

		if (not interactive) {
			if (not(origfullrec.read(DEFINITIONS, "SECURITY"))) {
				msg = "SECURITY missing from DEFINITIONS";
				return invalid(msg);
			}
			origfullrec = origfullrec.invert();

			//simulate orec
			if (win.orec.read(DEFINITIONS, "SECURITY.OREC")) {
			}else{
				msg = "SECURITY.OREC is missing from DEFINITIONS";
				return invalid(msg);
			}
			win.orec = win.orec.invert();

			//safety check
			if (win.orec.a(20) ne startn or win.orec.a(21) ne endn) {

				//trace
				win.orec.write(DEFINITIONS, "SECURITY.OREC.BAD");
				RECORD.write(DEFINITIONS, "SECURITY.REC.BAD");

				msg = "An internal error: REC 20 AND 21 DO NOT AGREE WITH .OREC";
				return invalid(msg);
			}

	/*;
				//remove unauthorised users
				orec=origfullrec;
				if startn and endn then;
					nn=endn-startn+1;
					for fn=1 to 8;
						orec<fn>=field(orec<fn>,vm,startn,nn);
						next fn;
					end;

				//remove unauthorised tasks
	*/

		}

		gosub cleartemp();

		//if locked then skip out
		if (interactive and win.ww(2).a(18).count("P")) {
			return 0;
		}

		if (not(interactive) or (win.templatex == "SECURITY")) {

			//check all users have names/passwords
			var usercodes = RECORD.a(1);
			usercodes.converter(" ", "-");
			RECORD.r(1, usercodes);
			var nusers = usercodes.count(VM) + (usercodes ne "");
			for (usern = 1; usern <= nusers; ++usern) {

				//recover password
				if (not interactive) {

					userx = RECORD.a(1, usern);

					newpassword = RECORD.a(4, usern);
					if (newpassword and (newpassword.length() < 4)) {
						msg = userx.quote() ^ " user password cannot be less than " ^ minpasswordchars;
						return invalid(msg);
					}

					if (not(origfullrec.a(1).locate(userx,ousern))) {
						ousern = 0;
					}

					if (newpassword) {

						win.mvx = usern;
						win.is = "";
						gosub changepassx();

						//remove old password so that changing password TO THE SAME PASSWORD
						//still triggers update of users file log section
						if (ousern) {
							origfullrec.r(4, ousern, "");
						}

						//zap any user generated pass in case they reset it while security was locked
						if (ousern) {
							var users;
							if (users.open("USERS", "")) {
								//writev field(@record<4,usern,2>,tm,7) on users,userx,4
								var("").writev(users, userx, 4);

							}
						}

					}else{
						//recover old password
						if (ousern) {
							var oldpassword = origfullrec.a(4, ousern);
							RECORD.r(4, usern, oldpassword);
						}
					}

				}

				if (not(RECORD.a(4, usern))) {
					var username = RECORD.a(1, usern);
					if (not username) {
						//msg='USER NAME IS MISSING IN LINE ':USERN
						//goto invalid
						username = "---";
						RECORD.r(4, usern, username);
					}
					if (not(username.index("---") or (username == "BACKUP"))) {
						if (not(RECORD.a(7, usern))) {
							//msg=quote(username):'|You must first give a password to this user'
							msg = username.quote() ^ "|You must give an email or password for this user";
							return invalid(msg);
						}
					}
				}

				//check ALL emails in one batch
				win.is = RECORD.a(7);
				call usersubs("VAL.EMAIL");
				if (not(win.valid)) {
					return 0;
				}

			};//usern;

			//mark empty users and keys with "---" to assist identification of groups
			nusers = RECORD.a(1).count(VM) + (RECORD.a(1) ne "");
			for (usern = 1; usern <= nusers; ++usern) {
				var temp = RECORD.a(1, usern);
				if ((temp == "") or temp.index("---")) {
					RECORD.r(1, usern, "---");
					RECORD.r(2, usern, "---");
				}
			};//usern;

			//put back any hidden users
			if (startn) {
				var nn = endn - startn + 1;
				var nvms = RECORD.a(1).count(VM);
				for (var fn = 1; fn <= 8; ++fn) {
					var temp = RECORD.a(fn);
					temp ^= VM.str(nvms - temp.count(VM));
					RECORD.r(fn, origfullrec.a(fn).fieldstore(VM, startn, -nn, temp));
				};//fn;
			}

			//put back any hidden tasks
			var tasks = origfullrec.a(10);
			var locks = origfullrec.a(11);
			var ntasks = tasks.count(VM) + (tasks ne "");
			for (var taskn = 1; taskn <= ntasks; ++taskn) {
				var task = tasks.a(1, taskn);
				if (task) {
					if (not(win.orec.a(10).locate(task,newtaskn))) {
						var lockx = locks.a(1, taskn);

						//locate task in @record<10> by 'AL' setting newtaskn else null
						//LOCKX=locks<1,taskn>
						//@record=insert(@record,10,newtaskn,0,task)
						//@record=insert(@record,11,newtaskn,0,LOCKX)

						if (not(RECORD.a(10).locateby("AL",task,newtaskn))) {
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
		var temp;
		if (temp.read(DEFINITIONS, "SECURITY")) {
			if (not(xx.read(DEFINITIONS, "SECURITY." ^ var().date()))) {
				temp.write(DEFINITIONS, "SECURITY." ^ var().date());
			}
			temp = "";
		}

		call cropper(RECORD);
		//dont save record in noninteractive mode as we are in prewrite stage
		if (interactive and RECORD) {
			RECORD.invert().write(DEFINITIONS, "SECURITY");
		}
		SECURITY = RECORD;

		//enable flowing text keys in postread
		//restore comma format in prewrite
		var keys = RECORD.a(2);
		keys.converter(" ", ",");
		RECORD.r(2, keys);

		//no further processing if HOURLYRATES
		if (interactive and win.templatex ne "SECURITY") {
			return 0;
		}

		//field numbers in users file
		userfields = "";
		//userfields<-1>='Code:0'
		userfields.r(-1, "User Name:1");
		userfields.r(-1, "Email:7");
		userfields.r(-1, "Group:21");
		userfields.r(-1, "Expiry:35:[DATE,4*]");
		userfields.r(-1, "IPNos:40");
		userfields.r(-1, "Keys:41");
		userfields.converter(":", VM);
		nuserfields = userfields.count(FM) + 1;
		emailtx = "";
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
		var nusers = usercodes.count(VM) + (usercodes ne "");
		for (usern = 1; usern <= nusers; ++usern) {
			userx = usercodes.a(1, usern);

			if (not(userx.index("---"))) {

				//get the original and current system records
				sysrec = RECORD.a(4, usern, 2);
				//locate user in orec<1> setting ousern then
				var menuid = "";
				if (origfullrec.a(1).locate(userx,ousern)) {
					//oSYSREC=orec<4,ousern,2>
					var osysrec = origfullrec.a(4, ousern, 2);
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
						if (not(not(menuid))) break;
					};//usern2;

				}

				//update the users file
				if (users) {

					//get the current user record
					if (not(userrec.read(users, userx))) {
						userrec = "";
						userrec.r(34, menuid);
					}
					origuserrec = userrec;

					//determine the user department
					call usersubs("GETUSERDEPT," ^ userx);
					var departmentcode = ANS.trim();
					var departmentcode2 = departmentcode;
					departmentcode.converter("0123456789", "");

					var username = usernames.a(1, usern);
					if (not username) {
						username = userx;
					}

					//update the user record
					userrec.r(1, username);
					userrec.r(5, departmentcode);
					userrec.r(7, useremails.a(1, usern));
					//userrec<8>=username
					userrec.r(11, usern);
					userrec.r(21, departmentcode2);
					//expirydate
					userrec.r(35, RECORD.a(3, usern));
					userrec.r(40, RECORD.a(6, usern));
					userrec.r(41, RECORD.a(2, usern));

					//new password cause entry in users log to renable login if blocked
					//save historical logins/password resets in listen2 and security.subs
					//similar in security.subs and user.subs
					var userpass = userpasswords.a(1, usern);
					var ouserpass = origfullrec.a(4, ousern);
					if (userpass ne ouserpass) {
						//datetime=(date():'.':time() 'R(0)#5')+0
						var datetime = var().date() ^ "." ^ var().time().oconv("R(0)#5");
						userrec.inserter(15, 1, datetime);
						userrec.inserter(16, 1, SYSTEM.a(40, 2));

						var text = "OK New password by " ^ USERNAME ^ ".";
						userrec.inserter(18, 1, text);

						userrec.r(36, datetime);
					}

					if (userrec ne origuserrec) {
						userrec.write(users, userx);

	//similar code in user.subs and security.subs
	/////////////
	//updatemirror:
	/////////////
	//save the user keyed as username too
	//because we save the user name and executive code in many places
	//and we still need to get the executive email if they are a user
						var mirror = userrec.fieldstore(FM, 13, 5, "");
						mirror = userrec.fieldstore(FM, 31, 3, "");
						var mirrorkey = "%" ^ userrec.a(1).ucase() ^ "%";
						mirror.r(1, userx);
						mirror.write(users, mirrorkey);

						isnew = origuserrec.a(1) == "";
						//only warn about new users with emails (ignore creation of groups/testusers)
						gosub getemailtx();

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
				//comment this out to prevent change the EXODUS password
				//if SYSREC and SYSREC ne oSYSREC and user<>@account then
				// convert \FB\:\FA\:\F9\ to \FE\:\FD\:\FC\ in SYSREC
				// read temp from system.file(),user then
				//  write SYSREC on system.file(),user
				//  end
				// end

			}
		};//usern;

		//delete any deleted users from the system file for direct login
		usercodes = win.orec.a(1);
		nusers = usercodes.count(VM) + (usercodes ne "");
		for (usern = 1; usern <= nusers; ++usern) {
			userx = usercodes.a(1, usern);
			if (not(userx.index("---"))) {
				if (userx and not(userx.index("EXODUS"))) {
					if (not(RECORD.a(1).locate(userx,temp))) {
						if (userrec.read(users, userx)) {
							if (users) {
								users.deleterecord(userx);

							}
							isnew = -1;
							origuserrec = "";
							gosub getemailtx();
						}
						if (temp.read(systemfile(), userx)) {
							if (temp.a(1) == "USER") {
								systemfile().deleterecord(userx);

							}
						}
					}
				}
			}
		};//usern;

		if (emailtx) {
			emailtx.splicer(1, 0, "User File Amendments:" ^ FM);
		}

		call securitysubs2("GETCHANGEDTASKS");
		if (ANS) {
			emailtx.r(-1, FM ^ "Task Locks:" ^ FM ^ ANS);
		}

		if (not interactive) {

			//sendmail summarising user and task changes
			if (emailtx) {
				call sysmsg(emailtx);
			}

			//email new users if requested to do so
			if (newusers and emailnewusers) {
				var nn = newusers.count(FM) + 1;
				for (var ii = 1; ii <= nn; ++ii) {

					if (USERNAME == "EXODUS") {
						replyto = "support@neosys.com";
					}else{
						replyto = USERNAME.xlate("USERS", 7, "X");
					}

					var toaddress = newusers.a(ii, 3);
					var ccaddress = replyto;

					//also inform accounts although cancelled users are not emailed to accounts
					if (not((toaddress ^ ccaddress).index("accounts@neosys.com"))) {
						if (ccaddress) {
							ccaddress ^= ";";
						}
						ccaddress ^= "accounts@neosys.com";
					}

					var subject = "EXODUS New User " ^ newusers.a(ii, 2) ^ " (" ^ newusers.a(ii, 1) ^ ")";

					var body = "";
					body.r(1, -1, "A new EXODUS user account has been created for you.");

					body ^= VM;
					body.r(1, -1, "User Code: %USERCODE%");
					body.r(1, -1, "User Name: %USERNAME%");
					body.r(1, -1, "Email:     %EMAIL%");
					body.r(1, -1, "Group:     %GROUP%");
					body.r(1, -1, "Database:  %DATABASE%");

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
					body.r(1, -1, "http://userwiki.neosys.com/index.php/resettingpassword");

					//in security.subs and listen2
					body ^= VM;
					body.r(1, -1, "Browser Configuration *REQUIRED*");
					body.r(1, -1, "http://userwiki.neosys.com/index.php/gettingstarted");

					if (replyto) {
						body ^= VM;
						body.r(1, -1, "Support:");
						body.r(1, -1, replyto);
					}

					body.swapper("%USERCODE%", newusers.a(ii, 1));
					body.swapper("%USERNAME%", newusers.a(ii, 2));
					body.swapper("%EMAIL%", toaddress);
					body.swapper("%GROUP%", newusers.a(ii, 4));
					body.swapper("%DATABASE%", SYSTEM.a(14, 1) ^ " (" ^ SYSTEM.a(17, 1) ^ ")");
					body.swapper(VM, var().chr(13));

					call sendmail(toaddress, ccaddress, subject, body, attachfilename, deletex, errormsg, replyto);

					if (errormsg) {
						call note(errormsg);
					}

				};//ii;
			}

			//prepare to write the inverted record in noninteractive mode
			RECORD = RECORD.invert();

			//remove the temp file
			DEFINITIONS.deleterecord("SECURITY.OREC");

		}

	} else if (mode == "POSTAPP") {

		//also called in postwrite in noninteractive mode

		if (not(SECURITY.read(DEFINITIONS, "SECURITY"))) {
			SECURITY = "";
		}
		SECURITY = SECURITY.invert();
		if (interactive) {
			xx = unlockrecord("", DEFINITIONS, "SECURITY");
		}

	} else if (mode.field(".", 1) == "LISTAUTH") {
		var temprec = RECORD;
		temprec.r(4, "");

		//remove expired users
		var expirydates = temprec.a(3);
		var nn = expirydates.count(VM) + 1;
		for (var ii = nn; ii >= 1; --ii) {
			var expirydate = expirydates.a(1, ii);
			if (expirydate) {
				if (expirydate <= var().date()) {
					for (var fn = 1; fn <= 9; ++fn) {
						var tt = temprec.a(fn);
						if (tt) {
							temprec.r(fn, tt.remove(1, ii, 0));
						}
					};//fn;
				}
			}
		};//ii;

		//remove empty groups
		var usercodes = temprec.a(1);
		nn = usercodes.count(VM) + 1;
		for (var ii = nn; ii >= 2; --ii) {
			var usercode = usercodes.a(1, ii);
			if (usercode) {
				if ((temprec.a(8, ii) == "") or (temprec.a(7, ii) == "")) {
					if (temprec.a(1, ii - 1) == "") {
						for (var fn = 1; fn <= 9; ++fn) {
							var tt = temprec.a(fn);
							if (tt) {
								temprec.r(fn, tt.remove(1, ii, 0));
							}
						};//fn;
					}
				}
			}
		};//ii;

		//add group marks
		temprec.r(1, temprec.a(1).swap(VM ^ VM, VM ^ "<hr/>" ^ VM));

		//reverse users so department shows at the top (and higher users at bottom sadly)
		if (mode.field(".", 2) == "USERS") {
			//trim any multivalued fields with more than nusers multivalues
			var nusers = temprec.a(1).count(VM) + 1;
			var nfs = temprec.count(FM) + 1;
			for (var fn = 1; fn <= nfs; ++fn) {
				temprec.r(fn, temprec.a(fn).field(VM, 1, nusers));
			};//fn;
			temprec = invertarray(reverse(invertarray(temprec)));
		}

		var tempkey = "SECURITY." ^ var(1000000).rnd() ^ "." ^ var().time() ^ ".$$$";
		temprec.write(DEFINITIONS, tempkey);
	//call oswrite(temprec,'x')
		var temp = "";

		var mode2 = mode.field(".", 2);
		var cmd = "LIST DEFINITIONS " ^ (tempkey.quote());
		if (mode2 == "TASKS") {
			cmd ^= " TASKS LOCKS";
		}else{
			//cmd:=' LIMIT USER_EXPIRY_DATE < ':quote(date() 'D4')
			//cmd:=' REMOVE_EXPIRED_USERS'
			//cmd:=' ADD_GROUP_MARKS'
			//cmd:=' REVERSE_MULTIVALUES'
			cmd ^= " USER_CODE_HTML FULL_NAME EMAIL_ADDRESS";
			cmd ^= " LAST_LOGIN_DATE_TIME LAST_LOGIN_LOCATION";
			//cmd:=' PASSWORD_AGE USER_EXPIRY_DATE LAST_BROWSER KEYS'
			cmd ^= " LAST_LOGIN_AGE PASSWORD_AGE LAST_OS LAST_BROWSER KEYS";
		}
		cmd ^= " HEADING " ^ (("LIST OF " ^ mode2 ^ "   \'T\'   Page \'PL\'").quote());

		cmd ^= " ID-SUPP DBL-SPC";
		if (interactive) {
			cmd.splicer(1, 0, "GET ");
		}
		execute(cmd);

		DEFINITIONS.deleterecord(tempkey);

	/*;
		case mode='XREFAUTH';
			if @record ne orec then;
				msg='Please save your changes first';
				return invalid(msg);
				end;
			execute 'GET NEW NOHTML PRINTSECURITY';
	*/
	} else if (mode.field(".", 1) == "GETTASKS") {

		var disallowed = mode.field(".", 2) == "NOT";
		var username = mode.field(".", 3);

		var origuser = USERNAME;
		if (username) {
			USERNAME=(username);
		}

		var tasks2 = "";
		var locks2 = "";
		var tasks = SECURITY.a(10);
		var locks = SECURITY.a(11);
		var ntasks = tasks.count(VM) + 1;
		var lasttask = "";
		for (var taskn = 1; taskn <= ntasks; ++taskn) {
			var task = tasks.a(1, taskn);
			if (authorised(task, xx)) {
				ok = 1;
			}else{
				ok = 0;
			}
			if (disallowed) {
				ok = not(ok);
			}
			if (ok) {

				//shorten duplicated task names
				var task2 = task;
				for (var ii = 1; ii <= 9; ++ii) {
					var temp = task.field(" ", ii);
					if (temp) {
						if (temp == lasttask.field(" ", ii)) {
							task = task.fieldstore(" ", ii, 1, "%SAME%");
						}else{
							temp = "";
						}
					}
					///BREAK;
					if (not temp) break;
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

		if (username) {
			USERNAME=(origuser);
		}
	/*;
		case mode='SHOWKEYSTASKS';
			if si<4>=10 or si<4>=11 then;
				KEYX=@record<11,mv>;
			end else;
				KEYX='';
				end;
	inpkey:
			call note2('What key do you want ?','RC',KEYX);
			if KEYX else return 0;
			ntasks=count(@record<10>,vm)+1;
			tasklist='';
			for taskn=1 to ntasks;
				tkeys=@record<11,taskn>;
				convert ',' to @vm in tkeys;
				locate KEYX in tkeys<1> setting xx then;
					tasklist<-1>=@record<10,taskn>;
					end;
				next taskn;
			if tasklist else;
				call note(quote(KEYX):' No tasks are locked with this key');
				goto inpkey;
				end;
			if decide('Key ':quote(KEYX):' locks these tasks.','':tasklist,reply) else return 0;
			goto inpkey;
			//locate tasklist<reply> in @record<10> setting taskn then
			// tstore='NEXT.MV.WI':fm:taskn:fm:3
			// @DATA=@MOVE.KEYS<4>:@MOVE.KEYS<5>
			// return 0
			// end

		case mode='SETWORKSTATION' or mode='SETSLEEP';
			if si<4>=1 else;
				msg='YOU MUST BE ON THE USER NAME|TO SET THE WORKSTATION OR SLEEP TIME';
				return invalid(msg);
				end;

			if mode='SETWORKSTATION' then;
				if security('#AUTHORISATION SET USER WORKSTATION',msg,'') else return invalid(msg);
				reply=@record<5,mv>;
				call note2('What is the workstation|number for ':@record<1,mv>:' ?|(enter "CURRENT" for current station)|(blank for default)','RCE',reply,'');
				if reply="\x0B" then return 0;
				if reply='CURRENT' then reply=@station;
				@record<5,mv>=reply;
			end else;
				if security('#AUTHORISATION SET USER SLEEP TIME',msg,'') else return invalid(msg);
				reply=@record<6,mv>;
	inp.sleep:
				call note2('How many seconds before sleeping|for ':@record<1,mv>:' ?|(blank for default)','RCE',reply,'');
				if reply="\x0B" then return 0;
				if reply lt 60 or reply gt 1000000 then;
					print char(7):
					call note('Please enter a number|between 60 and 1000000');
					goto inp.sleep;
					end;
				@record<6,mv>=reply;
				end;
	*/
	} else {
		msg = mode.quote() ^ " invalid mode in SECURITY.SUBS";
		return invalid(msg);

	}

	return 0;
}

subroutine changepassx() {
	var datax = RECORD.a(4, win.mvx);
	sysrec = datax.a(1, 1, 2);
	//convert \FB\:\FA\:\F9\ to \FE\:\FD\:\FC\ in SYSREC
	sysrec.converter(TM ^ STM ^ var().chr(249), FM ^ VM ^ SVM);
	if (not sysrec) {
		if (not(sysrec.read(systemfile(), userx))) {
			sysrec = "USER";
			sysrec.r(2, APPLICATION);
			sysrec.r(5, "EXODUS");
		}
	}

	//if interactive then
	// gosub newpass2
	//end else
	gosub newpass3();
	// end

	if (win.valid) {
		//on screen the password shows as <hidden>
		win.is.r(1, 1, 1, "<hidden>");

		//store the new password and system record
		var temp = sysrec;
		//convert \FE\:\FD\:\FC\ TO \FB\:\FA\:\F9\ in temp
		temp.converter(FM ^ VM ^ SVM, TM ^ STM ^ var().chr(249));
		win.is.r(1, 1, 2, temp);
		RECORD.r(4, win.mvx, win.is);

		//if mode='PERSONAL.PASSWORD' else
		// msg='THE NEW PASSWORD WILL ONLY BECOME EFFECTIVE'
		// gosub note
		// end

	}
	return;
}

subroutine generatepassword() {
	var consonants = UPPERCASE.substr(1,26);
	var vowels = "AEIOUY";
	consonants.converter(vowels ^ "QX", "");
	newpassword = "";
	for (var ii = 1; ii <= minpasswordchars / 2; ++ii) {
		newpassword ^= consonants[consonants.length().rnd() + 1];
		newpassword ^= vowels[vowels.length().rnd() + 1];
	};//ii;
	return;

	/*;
	///////////////
	CHECKAUTHORITY:
	///////////////
		STORE.userprivs=userprivs;
		userprivs=@RECORD;
		TASK=@RECORD<10,MV>;
		IF SECURITY(TASK,msg,'') ELSE;
			MSG='YOU CANNOT CHANGE OR DELETE ITEMS|THAT YOU ARE NOT AUTHORISED TO DO';
			GOSUB INVALID;
			END;
		userprivs=STORE.userprivs;
		RETURN;

	//////////
	checkrank:
	//////////

		locate curruser in @record<1> setting usern else usern=999;
		if index(curruser,'EXODUS',1) then return;
		if mv lt usern and not(updatehighergroups) then;
	badrank:
			msg='YOU CAN ONLY CHANGE YOURSELF,|';
			msg:='OR LOWER USERS IN YOUR OWN GROUP';
			if updatelowergroups then msg:=',|OR USERS IN LOWER GROUPS';
			return invalid(msg);
			end;

		//allow/prevent access to lower groups
		if updatelowergroups else;
			STARTX=usern+1;
			number=mv-usern;
			if amv.action=3 then number-=1;
			temp=field(@record<1>,vm,STARTX,number);
			if index(temp,'---',1) then goto badrank;
			end;

		return;

	///////////////////
	gethigherlowerkeys:
	///////////////////

		//get higher keys
		if usern=1 then;
			higherkeys='';
		end else;
			higherkeys=field(@record<2>,@vm,1,usern-1);
			convert ',' to vm in higherkeys;
			end;

		//get own keys
		ownkeys=field(@record<2>,vm,mv,999);
		convert ',' to vm in ownkeys;

		//get lower keys
		lowerkeys=field(@record<2>,vm,mv+1,999);
		convert ',' to vm in lowerkeys;

		//remove any keys in lower groups if not allowed to update lower groups
		//feb 97
		if updatelowergroups else;
			temp=index(lowerkeys,'---',1);
			if temp then lowerkeys[temp,9999]='';
			end;

		//get all locks in use
		alllocks=@record<11>:vm:origfullrec<11>;
		convert ',' to vm in alllocks;

		return;
	*/
	/*;
	/////////
	newpass2:
	/////////
		valid=0;
		newpassword='';
		last.fn=9;
		if security('AUTHORISATION INVENT OWN PASSWORDS',msg,'') then;
			if decide('Do you want a random password ?','Yes|No (use your own)',reply) else return;
			autopassword=(reply=1);
		end else;
			autopassword=1;
			end;

		if autopassword then;
	genpass:
			gosub generatepassword;

			//show the new password
			question='The new password will be ':quote(newpassword);
			question:='|Is this password acceptable to you ?';
			if decide(question,'No - make a different password|Yes - use this one|Cancel',reply) else return;
			if reply eq 1 then goto genpass;
			if reply ne 2 then return;

			//get the new password
	inp.autopassword:
			get.cursor(v10);
			call note2('What is the new password ?||<               >|(This is a check that you have|the new password correct)', 'UB', buffer, '');
			print @(39, @crthigh / 2):
			newpassword2='';
			call noecho.msg(newpassword2);
			call note('', 'DB', buffer, '');
			put.cursor(v10);
			buffer = '';

			if newpassword2 else;
				print char(7):
				call note('Password has NOT been changed');
				return;
				end;

			convert @lower.case to @upper.case in newpassword2;
			if newpassword2 ne newpassword then;
				print char(7):
				reply = 'Y';
				msg='|The entry of the new password|did not match.||Do you want to re-enter it? [Y/N]|';
				call mssg(msg,'RC',reply,'');
				IF reply = 'Y' THEN goto inp.autopassword;
				print char(7):
				call note('Password has NOT been changed');
				return;
				end;
			end;
	*/

}

subroutine newpass3() {

	win.valid = 0;
	lastfn = 9;

	if ((sysrec.a(1) == "USER") or (sysrec == "")) {
		passwordfn = 7;
	} else if (sysrec.a(1) == "ACCOUNT") {
		passwordfn = 6;
	} else {
		call note("W123", "", xx, userx);
		return;
	}

	var v12 = 0;

	/*;
	inp.newpassword:
		if newpassword = '' THEN;
	inp.newpassword1:
			//get the new password

			msg='What is the new password ?|[               ]|(Alphanumeric, 5 char minimum)';
			maxlen=15;
			show=0;
			allowablechars=@lower.case[1,26]:@upper.case[1,26]:'123456789';
			newpassword='';
			escx='';
			call inputbox(msg,maxlen,show,allowablechars,newpassword,escx);

			buffer = '';

			//get the new password again unless the new password is null
			if newpassword > '' then;
				if len(newpassword)<5 then;
					print char(7):
					call note('Passwords should be at|least 5 characters long');
					if index(@username,'EXODUS',1) else goto inp.newpassword1;
					end;
	inp.ownpass.2:

				msg='Re-enter the new password |[               ]|';
				maxlen=15;
				show=0;
				allowablechars=@lower.case[1,26]:@upper.case[1,26]:'123456789';
				v13='';
				escx='';
				call inputbox(msg,maxlen,show,allowablechars,v13,escx);

				buffer = '';

				IF newpassword=v13 THEN;
					V12 = 0;
					GOTO 569;
					END;

				reply = 'Y';
				print char(7):
				msg='|The entry of the new password|did not match.||Do you want to re-enter it? [Y/N]|';
				call note(msg, 'RC', reply, '');
				IF reply = 'Y' THEN;
					V12 = 1;
					GOTO 569;
					END;
				V12 = 0;
				newpassword  = '';
				print char(7):
				call note('The password has NOT been changed');
				END;

	569

			if newpassword = '' or newpassword = "\x0B" then return;
			convert @lower.case to @upper.case in newpassword;
			end;

	590
		IF v12 THEN;
			newpassword = '';
			v13 = '';
			END;
		IF V12 THEN;
			goto inp.newpassword;
			end;
	*/
}

subroutine makesysrec() {
	if (not(sysrec.a(1))) {
		if (passwordfn == 7) {
			sysrec.r(1, "USER");
		}else{
			sysrec.r(1, "ACCOUNT");
		}
	}
	if (not(sysrec.a(2))) {
		sysrec.r(2, APPLICATION);
	}
	if (not(sysrec.a(5))) {
		sysrec.r(5, "EXODUS");
	}
	//if SYSREC<last.fn> else SYSREC<last.fn>='xxxxx'
	if (not(sysrec.a(9))) {
		sysrec.r(9, "xxxxx");
	}

	//store the encrypted new password
	encryptx = newpassword;
	gosub makepass();
	sysrec.r(passwordfn, encryptx);

	encryptx = userx ^ FM ^ sysrec.field(FM, 2, lastfn - 2);
	gosub makepass();
	sysrec.r(lastfn, encryptx);

	encryptx = userx ^ FM ^ sysrec.field(FM, 2, lastfn - 2);
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
		if (not(encryptx ne "")) break;
		encryptkey = (encryptkey % 390001) * (encryptx[1]).seq() + 1;
		encryptx.splicer(1, 1, "");
	}//loop;

	//pass2
	while (true) {
		encryptx ^= var().chr(65 + (encryptkey % 50));
		encryptkey = (encryptkey / 50).floor();
		///BREAK;
		if (not encryptkey) break;
	}//loop;

	return;
}

subroutine gueststatus() {
	if (SYSTEM.a(21)) {
		msg = "YOU ARE A TEMPORARY GUEST|YOU CANNOT ACCESS AUTHORISATION";
		gosub invalid(msg);
	}
	return;
}

subroutine cleartemp() {
	//set in postread (setup) for exodus.net
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

subroutine getemailtx() {
	//dont sysmsg/log new/amend/deleting users @neosys.com unless in testdata or dev
	if ((userrec.a(7).ucase().index("@EXODUS.COM") and (SYSTEM.a(17, 1).substr(-4,4) ne "TEST")) and not(var("exodus.id").osfile())) {
		return;
	}

	//send to email about how to login to new users with email addresses
	if (userrec.a(7) and (isnew > 0)) {
		newusers.r(-1, userx ^ VM ^ userrec.a(1) ^ VM ^ userrec.a(7) ^ VM ^ userrec.a(5));
	}

	//build up log email for sysmsg
	var tx = "";
	for (var fieldn = 1; fieldn <= nuserfields; ++fieldn) {
		var fieldx = userfields.a(fieldn);
		var fn = fieldx.a(1, 2);
		var old = oconv(origuserrec.a(fn), fieldx.a(1, 3));
		var newx = oconv(userrec.a(fn), fieldx.a(1, 3));
		if (newx ne old) {
			var was = "was:";
			//dedup ipnos and keys
			if (not(isnew) and (((fn == 40) or (fn == 41)))) {
				call dedup("", old, newx, " ,");
				if (newx) {
					newx = "added " ^ newx;
				}
				if (old) {
					old = "removed " ^ old;
				}
				if (not(newx.length())) {
					old.transfer(newx);
				}
				was = "";
			}
			if ((newx ^ old).length()) {
				tx.r(-1, (fieldx.a(1, 1) ^ ":").oconv("L#10"));
				tx ^= newx;
				if (old or ((newx and not(isnew)))) {
					tx.r(-1, was.oconv("R#10") ^ old);
				}
			}
		}
	};//fieldn;

	if (tx) {
		emailtx.r(-1, FM ^ var("User Code:").oconv("L#10") ^ userx);
		if (isnew > 0) {
			emailtx ^= " *CREATED*";
		} else if (isnew < 0) {
			emailtx ^= " *DELETED*";
		} else {
			//emailtx:=' *AMENDED*'
		}

		//always show user name, if different from user code
		if ((origuserrec.a(1) == userrec.a(1)) and userrec.a(1) ne userx) {
			emailtx.r(-1, var("User Name:").oconv("L#10") ^ userrec.a(1));
		}
		//always show user group
		if (origuserrec.a(21) == userrec.a(21)) {
			emailtx.r(-1, var("Group:").oconv("L#10") ^ userrec.a(21));
		}
		emailtx.r(-1, tx);
	}

	return;
}

libraryexit()
