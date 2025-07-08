#include <exodus/library.h>
#include <srv_common.h>
#include <req_common.h>

#include <req_common.h>

libraryinit()

#include <authorised.h>
#include <dedup.h>
#include <hashpass.h>
#include <securitysubs2.h>
#include <sendmail.h>
#include <sysmsg.h>
#include <systemfile.h>
#include <usersubs.h>

#include <service_common.h>

#include <request.hpp>

// SEE AUTHORISATION_DICT in UI
//
// clang-format off
//  @record/user grouped mvs
//  1 = username
//  2 = keys
//  3 = expiry_date (was menu but menu moved to userx<34>)
//  4 = password
//  5 = was auto workstation DOS - now hourly rates
//  6 = was sleeptime - now allowable ip addresses eg 1.1.1.1,1.1.1.*
//  7 = emailaddress
//  8 = fullname
//  9 = buffered keys
// 20 = startn
// 21 = endn
// 22 = possible menus - not used
// 23 = hidden users
// 24 = otherkeys
// 25 = passwordautoexpirydays (also see system<128>)
// 26 = emailnewusers

#define minpasswordchars_ 4
#define emailnewusers_ RECORD.f(26)

//#define tstore_             req.registerx(3) not used this program
#define updatelowergroups_  req.registerx[4]
#define updatehighergroups_ req.registerx[5]
#define curruser_           req.registerx[6]
#define origfullrec_        req.registerx[7]
#define startn_             req.registerx[8]
#define endn_               req.registerx[9]

// clang-format on

// NB (not any more) valid companies are buffered in userprivs<9>

//var newpassword;
//var userx;
//var sysrec;
//var passwordfn;	 // num
//var lastfn;		 // num
//var filename;
//var defaultlock;
var msg;
//var usern;	// num
//var xx;
//var ousern;	 // num
//var newtaskn;
//var userfields;
//var nuserfields;
//var emailtx;
//var newusers;
//var userrec;
//var origuserrec;
//var isnew;	// num
//var replyto;
//var attachfilename;
//var deletex;
//var errmsg;
//var ok;	 // num
//var encryptx;
//var op;
//var op2;
//var wspos;
//var wsmsg;

func main(in mode) {

	let interactive = false;  //not(SYSTEM.f(33));

	req.valid = 1;

	// no validation for EXODUS
	if (curruser_.contains("EXODUS")) {
		if (mode.starts("VAL.") and mode != "VAL.USER") {
			return 0;
		}
	}

	if (mode == "GENERATEPASSWORD") {
		let newpassword = gosub generatepassword();
		req.is = newpassword;

		// only to allow maintenance mode
	} else if (mode == "VAL.USER") {
	} else if (mode == "PERP") {

	} else if (mode == "MAKESYSREC") {
		let userx			= req.is.f(1);
		let newpassword		= req.is.f(2);
		var sysrec			= "";
//		let passwordfn	= 7;
//		let lastfn		= 9;
		gosub makesysrec(sysrec, userx, newpassword);
		req.is = sysrec;

		// called from DEFINITION.SUBS POSTREAD for key SECURITY
	} else if (mode == "SETUP") {

		// check allowed access
//		if (req.templatex.unassigned()) {
//			req.templatex = "";
//		}
		req.templatex.defaulter("");
		var filename;
		var defaultlock;
		if (req.templatex == "SECURITY") {
			filename	= "AUTHORISATION";
			defaultlock = "GS";
		} else {
			if (req.templatex == "HOURLYRATES") {
				filename			= "HOURLY RATE";
				updatehighergroups_ = 1;
				updatelowergroups_	= 1;
				defaultlock			= "TA";
			} else {
				msg = req.templatex.quote() ^ " unknown template in security.subs";
				return invalid(msg);
			}
		}

		if (not authorised(filename ^ " ACCESS", msg, defaultlock)) {
			gosub invalid(msg);
			if (interactive) {
				stop();
			}
			return 0;
		}

		if (req.templatex == "SECURITY") {

			// check guest status
			gosub gueststatus();
			if (not req.valid) {
				stop();
			}

			// check supervisor status
			updatelowergroups_	= authorised("AUTHORISATION UPDATE LOWER GROUPS", msg, "");
			updatehighergroups_ = authorised("AUTHORISATION UPDATE HIGHER GROUPS", msg, "");
		}

		// if logged in as account then same as logged in as EXODUS
		if (PRIVILEGE and var("012").contains(PRIVILEGE)) {
			curruser_ = "EXODUS";
		} else {
			curruser_ = USERNAME;
		}

		// check security
		if (authorised(filename ^ " UPDATE", msg, defaultlock)) {
		} else {
			if (not interactive) {
				req.wlocked = 0;
			}
		}

		if (not SECURITY.read(DEFINITIONS, "SECURITY")) {
			SECURITY = "";
		}
		// in case not cleared in save/write
		gosub cleartemp();

		// never send the passwords to browser
		// (restored in prewrite except for new passwords)
		if (not interactive) {
			RECORD(4) = "";
		}

		// sort the tasks
		call sortarray(SECURITY, "10" ^ VM ^ "11", "AL");

		origfullrec_ = SECURITY;

		RECORD = SECURITY;

		// @record<9>=curruser

		// don't delete users for hourly rates
		if (req.templatex == "HOURLYRATES") {
			updatehighergroups_ = 1;
			updatelowergroups_	= 1;
		}

		// delete disallowed tasks (except all master type user to see all tasks)
		if (not(updatehighergroups_ and updatelowergroups_)) {
			let tasks  = RECORD.f(10);
			let locks  = RECORD.f(11);
			let ntasks = tasks.fcount(VM);
			//for (var taskn = ntasks; taskn >= 1; --taskn) {
			for (let taskn : reverse_range(1, ntasks)) {
				let task = tasks.f(1, taskn);
				let temp = task.starts("DOCUMENT: ") ? "#" : "";
				if (not authorised("!" ^ temp ^ task, msg, "")) {
					RECORD.remover(10, taskn);
					RECORD.remover(11, taskn);
				}
			}  // taskn;
		}

		// hide higher/lower users
		if (not curruser_.contains("EXODUS")) {

			let usercodes = RECORD.f(1);
			let nusers	  = usercodes.fcount(VM);

			var usern;
			if (not usercodes.locate(USERNAME, usern, 1)) {
				msg = USERNAME ^ " user not in in authorisation file";
				return invalid(msg);
				//stop();
			}

			// hide higher users
			if (updatehighergroups_) {
				startn_ = 1;
			} else {
				startn_ = usern;
				while (true) {
					// /BREAK;
					if (not(startn_ > 1 and (RECORD.f(2, startn_ - 1) == "")))
						break;
					startn_ -= 1;
				}  // loop;
			}

			// hide lower users
			if (updatelowergroups_) {
				endn_ = nusers;
			} else {
				endn_ = usern;
				while (true) {
					// /BREAK;
					if (not(endn_ < nusers and (RECORD.f(1, endn_ + 1) != "---")))
						break;
					endn_ += 1;
				}  // loop;
			}

			// extract out the allowable users and keys
			// also in prewrite

			if (startn_ != 1 or endn_ != nusers) {
				let nallowed = endn_ - startn_ + 1;

				if (not interactive) {

					// save hidden users for remote client in field 23
					var temp = RECORD.f(1).field(VM, 1, startn_);
					temp ^= VM ^ RECORD.f(1).field(VM, endn_ + 1, 9999);
					temp.converter(VM, ",");
					RECORD(23) = temp;

					// save hidden keys for remote client in field 23

					var visiblekeys = RECORD.f(2).field(VM, startn_, nallowed);
					visiblekeys.converter(",", VM);
					visiblekeys = trim(visiblekeys, VM);

					var invisiblekeys = RECORD.f(2).field(VM, 1, startn_);
					invisiblekeys ^= VM ^ RECORD.f(2).field(VM, endn_ + 1, 9999);
					invisiblekeys.converter(",", VM);
					invisiblekeys = trim(invisiblekeys, VM);

					var otherkeys = "";
					if (invisiblekeys) {
						let nkeys = invisiblekeys.fcount(VM);
						for (const var keyn : range(1, nkeys)) {
							let keyx = invisiblekeys.f(1, keyn);
							var tt;
							if (not otherkeys.locate(keyx, tt, 1)) {
								if (not visiblekeys.f(1).locate(keyx)) {
									otherkeys ^= VM ^ keyx;
								}
							}
						}  // keyn;
						otherkeys.cutter(1);
						otherkeys.converter(VM, ",");
					}
					RECORD(24) = otherkeys;
				}

				// delete higher and lower users if not allowed
				for (const var fn : range(1, 8)) {
					RECORD(fn) = RECORD.f(fn).field(VM, startn_, nallowed);
				}  // fn;

			} else {
				startn_ = "";
				endn_	= "";
			}
		}

		// get the local passwords from the system file for users that exist there
		// also get any user generated passwords
		let usercodes = RECORD.f(1);
		//let nnr	  = usercodes.fcount(VM);
		// for (usern = 1; usern <= nnr; ++usern) {
		for (const var usern : range(1, usercodes.fcount(VM))) {
			let userx	 = usercodes.f(1, usern);
			var sysrec	 = RECORD.f(4, usern, 2);
			let pass	 = userx.xlate("USERS", 4, "X");
			if (pass and pass != sysrec.field(TM, 7)) {
				RECORD(4, usern, 2) = sysrec.fieldstore(TM, 7, 1, pass);
			}
			var sysrec2;
			if (sysrec2.read(systemfile(), userx)) {
				// convert \FE\:\FD\:\FC\ TO \FB\:\FA\:\F9\ in SYSREC2
				sysrec2.converter(FM ^ VM ^ SM, TM ^ STM ^ chr(249));
				if (sysrec2 != sysrec) {
					RECORD(4, usern) = "<hidden>" ^ SM ^ sysrec2;
				}
			}
		}  // usern;

		RECORD(20) = startn_;
		RECORD(21) = endn_;

		// enable flowing text keys in postread
		// restore comma format in prewrite
		var keys = RECORD.f(2);
		keys.converter(",", " ");
		RECORD(2) = keys;

		if (not interactive) {

			// group separators act as data in intranet client forcing menu and passwords
			for (const var fn : range(1, 2)) {
				var temp = RECORD.f(fn);
				temp.replacer("---", "");
				RECORD(fn) = temp;
			}  // fn;

			// save orec (after removing stuff) for prewrite
			if (req.wlocked) {
				RECORD.write(DEFINITIONS, "SECURITY.OREC");
			}
		}

		// called as prewrite in noninteractive mode
	} else if (mode == "SAVE") {

		// get/clear temporary storage
		startn_ = RECORD.f(20);
		endn_	= RECORD.f(21);

		if (not interactive) {
			if (not origfullrec_.read(DEFINITIONS, "SECURITY")) {
				msg = "SECURITY missing from DEFINITIONS";
				return invalid(msg);
			}

			// simulate orec
			if (req.orec.read(DEFINITIONS, "SECURITY.OREC")) {
			} else {
				msg = "SECURITY.OREC is missing from DEFINITIONS";
				return invalid(msg);
			}

			// safety check
			if (req.orec.f(20) != startn_ or req.orec.f(21) != endn_) {

				// trace
				req.orec.write(DEFINITIONS, "SECURITY.OREC.BAD");
				RECORD.write(DEFINITIONS, "SECURITY.REC.BAD");

				msg = "An internal error: REC 20 AND 21 DO NOT AGREE WITH .OREC";
				return invalid(msg);
			}
		}

		gosub cleartemp();

		// if locked then skip out
		if (interactive and req.ww[2].f(18).count("P")) {
			return 0;
		}

		if (not(interactive) or req.templatex == "SECURITY") {

			// check all users have names/passwords
			var usercodes = RECORD.f(1);
			usercodes.converter(" ", "-");
			RECORD(1)  = usercodes;
			let nusers = usercodes.fcount(VM);
			// for (usern = 1; usern <= nusers; ++usern) {
			for (const var usern : range(1, nusers)) {

				// recover password
				if (not interactive) {

					let userx = RECORD.f(1, usern);

					let newpassword = RECORD.f(4, usern);
					if (newpassword and newpassword.len() < 4) {
						msg = userx.quote() ^ " user password cannot be less than " ^ minpasswordchars_;
						return invalid(msg);
					}

					var ousern;
					if (not origfullrec_.locate(userx, ousern, 1)) {
						ousern = 0;
					}

					if (newpassword) {

						req.mvx = usern;
						req.is	= "";
						gosub changepassx(userx, newpassword);

						// remove old password so that changing password TO THE SAME PASSWORD
						// still triggers update of users file log section
						if (ousern) {
							origfullrec_(4, ousern) = "";
						}

						// zap any user generated pass in case they reset it while security was locked
						if (ousern) {
							var users;
							if (users.open("USERS", "")) {
								// writef field(@record<4,usern,2>,tm,7) on users,userx,4
								var("").writef(users, userx, 4);
							}
						}

					} else {
						// recover old password
						if (ousern) {
							let oldpassword	 = origfullrec_.f(4, ousern);
							RECORD(4, usern) = oldpassword;
						}
					}
				}

				if (not RECORD.f(4, usern)) {
					var username = RECORD.f(1, usern);
					if (not username) {
						// msg='USER NAME IS MISSING IN LINE ':USERN
						// goto invalid
						username		 = "---";
						RECORD(4, usern) = username;
					}
					if (not(username.contains("---") or username == "BACKUP")) {
						if (not RECORD.f(7, usern)) {
							// msg=quote(username):'|You must first give a password to this user'
							msg = username.quote() ^ "|You must give an email or password for this user";
							return invalid(msg);
						}
					}
				}

				// check ALL emails in one pass
				req.is = RECORD.f(7);
				call usersubs("VAL.EMAIL");
				if (not req.valid) {
					return 0;
				}

			}  // usern;

			// mark empty users and keys with "---" to assist identification of groups
			// let nusers = RECORD.f(1).fcount(VM);
			// or (usern = 1; usern <= nusers; ++usern) {
			for (const var usern : range(1, nusers)) {
				let temp = RECORD.f(1, usern);
				if (temp == "" or temp.contains("---")) {
					RECORD(1, usern) = "---";
					RECORD(2, usern) = "---";
				}
			}  // usern;

			// put back any hidden users
			if (startn_) {
				let nhidden	 = endn_ - startn_ + 1;
				let nvms = RECORD.f(1).count(VM);
				for (const var fn : range(1, 8)) {
					var temp = RECORD.f(fn);
					temp ^= VM.str(nvms - temp.count(VM));
					RECORD(fn) = origfullrec_.f(fn).fieldstore(VM, startn_, -nhidden, temp);
				}  // fn;
			}

			// put back any hidden tasks
			let tasks  = origfullrec_.f(10);
			let locks  = origfullrec_.f(11);
			let ntasks = tasks.fcount(VM);
			for (const var taskn : range(1, ntasks)) {
				let task = tasks.f(1, taskn);
				if (task) {
					var newtaskn;
					if (not req.orec.locate(task, newtaskn, 10)) {
						let lockx = locks.f(1, taskn);

						if (not RECORD.f(10).locateby("AL", task, newtaskn)) {
							RECORD.inserter(10, newtaskn, task);
							RECORD.inserter(11, newtaskn, lockx);
						}
					}
				}
			}  // taskn;
		}

		// backup copy one per day
		var temp;
		if (temp.read(DEFINITIONS, "SECURITY")) {
			if (not var().read(DEFINITIONS, "SECURITY." ^ date())) {
				temp.write(DEFINITIONS, "SECURITY." ^ date());
			}
			temp = "";
		}

		call cropper(RECORD);
		// dont save record in noninteractive mode as we are in prewrite stage
		if (interactive and RECORD) {
			RECORD.write(DEFINITIONS, "SECURITY");
		}
		SECURITY = RECORD;

		// enable flowing text keys in postread
		// restore comma format in prewrite
		var keys = RECORD.f(2);
		keys.converter(" ", ",");
		RECORD(2) = keys;

		// no further processing if HOURLYRATES
		if (interactive and req.templatex != "SECURITY") {
			return 0;
		}

		// field numbers in users file
		var userfields = "";
		// userfields<-1>='Code:0'
		userfields(-1) = "User Name:1";
		userfields(-1) = "Email:7";
		userfields(-1) = "Group:21";
		userfields(-1) = "Expiry:35:[DATE,4*]";
		userfields(-1) = "IPNos:40";
		userfields(-1) = "Keys:41";
		userfields.converter(":", VM);
		let nuserfields = userfields.fcount(FM);

		// Will be built up from processesing all users
		var emailtx	 = "";
		var newusers = "";

		var users;
		if (not users.open("USERS", "")) {
			users = "";
		}

		// 1. subsection to update
		{
			// update users in the central system file if they exist there (direct login)
			let usercodes	  = RECORD.f(1);
			let useremails	  = RECORD.f(7);
			let usernames	  = RECORD.f(8);
			let userpasswords = RECORD.f(4);
			let nusers		  = usercodes.fcount(VM);
			// for (usern = 1; usern <= nusers; ++usern) {
			for (const var usern : range(1, nusers)) {
				let userx = usercodes.f(1, usern);

				if (not userx.contains("---")) {

					// get the original and current system records
					var sysrec = RECORD.f(4, usern, 2);
					// locate user in orec<1> setting ousern then
					var menuid = "";
					var ousern;
					if (origfullrec_.locate(userx, ousern, 1)) {
						// oSYSREC=orec<4,ousern,2>
						let osysrec = origfullrec_.f(4, ousern, 2);
					} else {
						let osysrec = "";

						// new users look for legacy menu in following (lower rank) users
						for (const var usern2 : range(usern + 1, nusers)) {
							let usercode2 = usercodes.f(1, usern2);
							if (usercode2) {
								if (not menuid.readf(users, usercode2, 34)) {
									menuid = "";
								}
							}
							// /BREAK;
							if (not not(menuid))
								break;
						}  // usern2;
					}

					// update the users file
					if (users) {

						// get the current user record
						var userrec;
						if (not userrec.read(users, userx)) {
							userrec		= "";
							userrec(34) = menuid;
						}
						let origuserrec = userrec;

						// determine the user department
						call usersubs("GETUSERDEPT," ^ userx);
						var	 departmentcode	 = ANS.trim();
						var	 departmentcode2 = departmentcode;
						departmentcode.converter("0123456789", "");

						var username = usernames.f(1, usern);
						if (not username) {
							username = userx;
						}

						// update the user record
						userrec(1) = username;
						userrec(5) = departmentcode;
						userrec(7) = useremails.f(1, usern);
						// userrec<8>=username
						userrec(11) = usern;
						userrec(21) = departmentcode2;
						// expirydate
						userrec(35) = RECORD.f(3, usern);
						userrec(40) = RECORD.f(6, usern);
						userrec(41) = RECORD.f(2, usern);

						// new password cause entry in users log to renable login if blocked
						// save historical logins/password resets in listen2 and security.subs
						// similar in security.subs and user.subs
						let userpass  = userpasswords.f(1, usern);
						let ouserpass = origfullrec_.f(4, ousern);
						if (userpass != ouserpass) {
							// datetime=(date():'.':time() 'R(0)#5')+0
							let datetime = date() ^ "." ^ time().oconv("R(0)#5");
							userrec.inserter(15, 1, datetime);
							userrec.inserter(16, 1, SYSTEM.f(40, 2));

							let text = "OK New password by " ^ USERNAME ^ ".";
							userrec.inserter(18, 1, text);

							userrec(36) = datetime;
						}

						if (userrec != origuserrec) {
							userrec.write(users, userx);

							// similar code in user.subs and security.subs
							// ///////////
							// updatemirror:
							// ///////////
							// save the user keyed as username too
							// because we save the user name and executive code in many places
							// and we still need to get the executive email if they are a user
							var mirror	  = userrec.fieldstore(FM, 13, 5, "");
							mirror		  = userrec.fieldstore(FM, 31, 3, "");
							let mirrorkey = "%" ^ userrec.f(1).ucase() ^ "%";
							mirror(1)	  = userx;
							mirror.write(users, mirrorkey);

							let isnew = origuserrec.f(1) == "";
							// only warn about new users with emails (ignore creation of groups/testusers)
							gosub getemailtx(emailtx, newusers, isnew, userx, userrec, origuserrec, userfields, nuserfields);
						}
					}
				}
			}  // usern;
		}

		// 2. subsection for deletion of old
		{
			// delete any deleted users from the system file for direct login
			let usercodes = req.orec.f(1);
			let nusers	  = usercodes.fcount(VM);
			// for (usern = 1; usern <= nusers; ++usern) {
			for (const var usern : range(1, nusers)) {
				let userx = usercodes.f(1, usern);
				if (not userx.contains("---")) {
					if (userx and not(userx.contains("EXODUS"))) {
						if (not RECORD.locate(userx, temp, 1)) {
							var userrec;
							if (userrec.read(users, userx)) {
								if (users) {
									users.deleterecord(userx);
								}
								let isnew			= -1;
								let origuserrec = "";
								gosub getemailtx(emailtx, newusers, isnew, userx, userrec, origuserrec, userfields, nuserfields);
							}
							if (temp.read(systemfile(), userx)) {
								if (temp.f(1) == "USER") {
									systemfile().deleterecord(userx);
								}
							}
						}
					}
				}
			}  // usern;

		}  // deletion of old

		if (emailtx) {
			emailtx.prefixer("User File Amendments:" ^ FM);
		}

		call securitysubs2("GETCHANGEDTASKS");
		if (ANS) {
			emailtx(-1) = FM ^ "Task Locks:" ^ FM ^ ANS;
		}

		if (not interactive) {

			// sendmail summarising user and task changes
			if (emailtx) {
				call sysmsg(emailtx);
			}

			// email new users if requested to do so
			if (newusers and emailnewusers_) {
				//let nnewusers = newusers.fcount(FM);
				for (const var ii : range(1, newusers.fcount(FM))) {

					let replyto = (USERNAME == "EXODUS") ? "support@neosys.com" : USERNAME.xlate("USERS", 7, "X");
					let toaddress = newusers.f(ii, 3);
					var ccaddress = replyto;

					// also inform accounts although cancelled users are not emailed to accounts
					if (not (toaddress ^ ccaddress).contains("accounts@neosys.com")) {
						if (ccaddress) {
							ccaddress ^= ";";
						}
						ccaddress ^= "accounts@neosys.com";
					}

					let subject = "EXODUS New User " ^ newusers.f(ii, 2) ^ " (" ^ newusers.f(ii, 1) ^ ")";

					var body	= "";
					body(1, -1) = "A new EXODUS user account has been created for you.";

					body ^= VM;
					body(1, -1) = "User Code: %USERCODE%";
					body(1, -1) = "User Name: %USERNAME%";
					body(1, -1) = "Email:     %EMAIL%";
					body(1, -1) = "Group:     %GROUP%";
					body(1, -1) = "Database:  %DATABASE%";

					body ^= VM;
					body(1, -1)		  = "Login:";
					let baselinks	  = SYSTEM.f(114);
					let baselinkdescs = SYSTEM.f(115);
					let nlinks		  = baselinks.fcount(VM);
					if (nlinks) {
						for (const var linkn : range(1, nlinks)) {
							body(1, -1) = baselinkdescs.f(1, linkn) ^ " " ^ baselinks.f(1, linkn);
							// if @account='ACCOUNTS' then body:='?ACCOUNTS'
						}  // linkn;
						body.replacer("Internet Explorer", "MS Edge");
					} else {
						body(1, -1) = "Please contact your IT support or colleagues";
					}

					body ^= VM;
					body(1, -1) = "Password:";
					body(1, -1) = "You must use \"Password Reset\" on the Login Screen";
					body(1, -1) = "http://userwiki.neosys.com/index.php/resettingpassword";

					// in security.subs and listen2
					// body:=vm
					body(1, -1) = "Browser Configuration *REQUIRED*";
					body(1, -1) = "http://userwiki.neosys.com/index.php/gettingstarted";

					if (replyto) {
						body ^= VM;
						body(1, -1) = "Support:";
						body(1, -1) = replyto;
					}

					body.replacer("%USERCODE%", newusers.f(ii, 1));
					body.replacer("%USERNAME%", newusers.f(ii, 2));
					body.replacer("%EMAIL%", toaddress);
					body.replacer("%GROUP%", newusers.f(ii, 4));
					body.replacer("%DATABASE%", SYSTEM.f(14, 1) ^ " (" ^ SYSTEM.f(17, 1) ^ ")");

					//body.replacer(VM, chr(13));
					// chr(13) no longer compatible with Postfix 3.6.4-1ubuntu1.2
					// sendmail.cpp converts FM to newline
					body.replacer(VM, FM);

					var dummy;
					var dummy2;
					var errmsg;
					call sendmail(toaddress, ccaddress, subject, body, dummy, dummy2, errmsg, replyto);

					if (errmsg) {
						call note(errmsg);
					}

				}  // ii;
			}

			// prepare to write the inverted record in noninteractive mode

			// remove the temp file
			DEFINITIONS.deleterecord("SECURITY.OREC");
		}

	} else if (mode == "POSTAPP") {

		// also called in postwrite in noninteractive mode

		if (not SECURITY.read(DEFINITIONS, "SECURITY")) {
			SECURITY = "";
		}
		if (interactive) {
			unlockrecord("", DEFINITIONS, "SECURITY");
		}

	} else if (mode.field(".", 1) == "LISTAUTH") {
		var temprec = RECORD;
		temprec(4)	= "";

		// remove expired users
		let expirydates = temprec.f(3);
		//let nexpired			= expirydates.fcount(VM);
		//for (var ii = nexpired; ii >= 1; --ii) {
		for (let ii : reverse_range(1, expirydates.fcount(VM))) {
			let expirydate = expirydates.f(1, ii);
			if (expirydate) {
				if (expirydate <= date()) {
					for (const var fn : range(1, 9)) {
						let tt = temprec.f(fn);
						if (tt) {
							temprec(fn) = tt.remove(1, ii, 0);
						}
					}  // fn;
				}
			}
		}  // ii;

		// remove empty groups
		{
			let usercodes = temprec.f(1);
			let nusers		  = usercodes.fcount(VM);
			//for (var ii = nusers; ii >= 2; --ii) {
			for (let ii : reverse_range(2, nusers)) {
				let usercode = usercodes.f(1, ii);
				if (usercode) {
					if (temprec.f(8, ii) == "" or temprec.f(7, ii) == "") {
						if (temprec.f(1, ii - 1) == "") {
							for (const var fn : range(1, 9)) {
								let tt = temprec.f(fn);
								if (tt) {
									temprec(fn) = tt.remove(1, ii, 0);
								}
							}  // fn;
						}
					}
				}
			}  // ii;
		}

		// add group marks
		temprec(1) = temprec.f(1).replace(VM ^ VM, VM ^ "<hr/>" ^ VM);

		// reverse users so department shows at the top (and higher users at bottom sadly)
		if (mode.field(".", 2) == "USERS") {
			// trim any multivalued fields with more than nusers multivalues
			let nusers = temprec.f(1).fcount(VM);
			let nfs	   = temprec.fcount(FM);
			for (const var fn : range(1, nfs)) {
				temprec(fn) = temprec.f(fn).field(VM, 1, nusers);
			}  // fn;
			// temprec = invertarray(reverse(invertarray(temprec)));
			temprec = invertarray(reverse(invertarray(temprec)));
		}

		let tempkey = "SECURITY." ^ var(1000000).rnd() ^ "." ^ time() ^ ".$$$";
		temprec.write(DEFINITIONS, tempkey);
		// call oswrite(temprec,'x')
		let temp = "";

		let mode2 = mode.field(".", 2);
		var cmd	  = "LIST DEFINITIONS " ^ (tempkey.quote());
		if (mode2 == "TASKS") {
			cmd ^= " TASKS LOCKS";
		} else {
			cmd ^= " USER_CODE_HTML FULL_NAME EMAIL_ADDRESS";
			cmd ^= " LAST_LOGIN_DATE_TIME LAST_LOGIN_LOCATION";
			cmd ^= " LAST_LOGIN_AGE PASSWORD_AGE LAST_OS LAST_BROWSER KEYS";
		}
		cmd ^= " HEADING " ^ (("LIST OF " ^ mode2 ^ "   'T'   Page 'PL'").quote());

		cmd ^= " ID-SUPP DBL-SPC";
		if (interactive) {
			cmd.prefixer("GET ");
		}
		execute(cmd);

		DEFINITIONS.deleterecord(tempkey);

	} else if (mode.field(".", 1) == "GETTASKS") {

		let disallowed = mode.field(".", 2) == "NOT";
		let username   = mode.field(".", 3);

		let origuser = USERNAME;
		if (username) {
			USERNAME = (username);
		}

		var tasks2	 = "";
		var locks2	 = "";
		let tasks	 = SECURITY.f(10);
		let locks	 = SECURITY.f(11);
		let ntasks	 = tasks.fcount(VM);
		var lasttask = "";
		for (const var taskn : range(1, ntasks)) {
			var task = tasks.f(1, taskn);
			var ok = authorised(task);
			if (disallowed) {
				ok = not(ok);
			}
			if (ok) {

				// shorten duplicated task names
				let task2 = task;
				for (const var ii : range(1, 9)) {
					var temp = task.field(" ", ii);
					if (temp) {
						if (temp == lasttask.field(" ", ii)) {
							task = task.fieldstore(" ", ii, 1, "%SAME%");
						} else {
							temp = "";
						}
					}
					// /BREAK;
					if (not temp)
						break;
				}  // ii;
				task ^= "   ";
				task.replacer("%SAME% ", "+");
				task.trimmerlast();
				lasttask = task2;

				tasks2 ^= VM ^ task;
				locks2 ^= VM ^ locks.f(1, taskn);
			}
		}  // taskn;
		tasks2.cutter(1);
		locks2.cutter(1);
		// transfer tasks2 to @ans

		ANS	   = tasks2;
		tasks2 = "";
		if (not disallowed) {
			ANS ^= FM ^ locks2;
		}

		if (username) {
			USERNAME = (origuser);
		}
	} else {
		msg = mode.quote() ^ " invalid mode in SECURITY.SUBS";
		return invalid(msg);
	}

	return 0;
}

subr changepassx(in userx, in newpassword) {
	let datax = RECORD.f(4, req.mvx);
	var sysrec	  = datax.f(1, 1, 2);
	sysrec.converter(TM ^ STM ^ chr(249), FM ^ VM ^ SM);
	if (not sysrec) {
		if (not sysrec.read(systemfile(), userx)) {
			sysrec	  = "USER";
			sysrec(2) = APPLICATION;
			sysrec(5) = "EXODUS";
		}
	}

	gosub newpass3(sysrec, userx, newpassword);

	if (req.valid) {
		// on screen the password shows as <hidden>
		req.is(1, 1, 1) = "<hidden>";

		// store the new password and system record
		var temp = sysrec;
		temp.converter(FM ^ VM ^ SM, TM ^ STM ^ chr(249));
		req.is(1, 1, 2)	   = temp;
		RECORD(4, req.mvx) = req.is;
	}
	return;
}

func generatepassword() {
	var consonants = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	let vowels	   = "AEIOUY";
	consonants.converter(vowels ^ "QX", "");
	var newpassword = "";
	for (const var ii : range(1, minpasswordchars_ / 2)) {
		newpassword ^= consonants.at(consonants.len().rnd() + 1);
		newpassword ^= vowels.at(vowels.len().rnd() + 1);
	}  // ii;
	return newpassword;
}

subr newpass3(io sysrec, in userx, in newpassword) {

	req.valid  = 0;
//	let lastfn = 9;

//	var passwordfn;
//	if (sysrec.f(1) == "USER" or sysrec == "") {
//		passwordfn = 7;
//	} else if (sysrec.f(1) == "ACCOUNT") {
//		passwordfn = 6;
//	} else {
//		var dummy;
//		call note("W123", "", dummy, userx);
//		return;
//	}

//	let v12 = 0;

	gosub makesysrec(sysrec, userx, newpassword);
	return;
}

subr makesysrec(io sysrec, in userx, in newpassword) {

	if (not sysrec.f(1)) {
		sysrec(1) = "USER";
	}
	if (not sysrec.f(2)) {
		sysrec(2) = APPLICATION;
	}
	if (not sysrec.f(5)) {
		sysrec(5) = "EXODUS";
	}
	// if SYSREC<last.fn> else SYSREC<last.fn>='xxxxx'
	if (not sysrec.f(9)) {
		sysrec(9) = "xxxxx";
	}

	// store the encrypted new password
	var encryptx	   = hashpass(newpassword);
	let passwordfn	   = 7;
	sysrec(passwordfn) = encryptx;

	let lastfn = 9;

	encryptx	   = userx ^ FM ^ sysrec.field(FM, 2, lastfn - 2);
	encryptx	   = hashpass(encryptx);
	sysrec(lastfn) = encryptx;

	encryptx	   = userx ^ FM ^ sysrec.field(FM, 2, lastfn - 2);
	encryptx	   = hashpass(encryptx);
	sysrec(lastfn) = encryptx;

	req.valid = 1;

	return;
}

subr gueststatus() {
	if (SYSTEM.f(21)) {
		msg = "YOU ARE A TEMPORARY GUEST|YOU CANNOT ACCESS AUTHORISATION";
		gosub invalid(msg);
	}
	return;
}

subr cleartemp() {
	// set in postread (setup) for exodus.net
	// @record
	// 20 start
	// 21 end
	// 22 possible menus (no longer since menus moved to users field <34>
	// 23 other user codes
	// 24 other keys
	for (const var ii : range(20, 24)) {
		RECORD(ii) = "";
	}  // ii;
	return;
}

subr getemailtx(io emailtx, io newusers, in isnew, in userx, in userrec, in origuserrec, in userfields, in nuserfields) {
	// dont sysmsg/log new/amend/deleting users @neosys.com unless in testdata or dev
	if (userrec.f(7).ucase().contains("@EXODUS.COM") and not SYSTEM.f(17, 1).ends("_test") and not var("exodus.id").osfile()) {
		return;
	}

	// send to email about how to login to new users with email addresses
	if (userrec.f(7) and isnew > 0) {
		newusers(-1) = userx ^ VM ^ userrec.f(1) ^ VM ^ userrec.f(7) ^ VM ^ userrec.f(5);
	}

	// build up log email for sysmsg
	var tx = "";
	for (const var fieldn : range(1, nuserfields)) {
		let fieldx = userfields.f(fieldn);
		let fn	   = fieldx.f(1, 2);
		var old	   = oconv(origuserrec.f(fn), fieldx.f(1, 3));
		var newx   = oconv(userrec.f(fn), fieldx.f(1, 3));
		if (newx != old) {
			var was = "was:";
			// dedup ipnos and keys
			if (not(isnew) and ((fn == 40 or fn == 41))) {
				call dedup("", old, newx, " ,");
				if (newx) {
					newx = "added " ^ newx;
				}
				if (old) {
					old = "removed " ^ old;
				}
				if (not newx.len()) {
					old.move(newx);
				}
				was = "";
			}
			if ((newx ^ old).len()) {
				tx(-1) = (fieldx.f(1, 1) ^ ":").oconv("L#10");
				tx ^= newx;
				if (old or ((newx and not(isnew)))) {
					tx(-1) = was.oconv("R#10") ^ old;
				}
			}
		}
	}  // fieldn;

	if (tx) {
		emailtx(-1) = FM ^ var("User Code:").oconv("L#10") ^ userx;
		if (isnew > 0) {
			emailtx ^= " *CREATED*";
		} else if (isnew < 0) {
			emailtx ^= " *DELETED*";
		} else {
			// emailtx:=' *AMENDED*'
		}

		// always show user name, if different from user code
		if (origuserrec.f(1) == userrec.f(1) and userrec.f(1) != userx) {
			emailtx(-1) = var("User Name:").oconv("L#10") ^ userrec.f(1);
		}
		// always show user group
		if (origuserrec.f(21) == userrec.f(21)) {
			emailtx(-1) = var("Group:").oconv("L#10") ^ userrec.f(21);
		}
		emailtx(-1) = tx;
	}

	return;
}

}; // libraryexit()
