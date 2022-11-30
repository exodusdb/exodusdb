#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <hashpass.h>
#include <initcompany.h>
#include <loginnet.h>
#include <openfile.h>
#include <safeselect.h>
#include <securitysubs.h>
#include <sendmail.h>
#include <sysmsg.h>
#include <systemfile.h>
#include <usersubs.h>
#include <whois.h>

#include <service_common.h>

#include <srv_common.h>
#include <req_common.h>

var usern;	// num
//var xx;
var userencrypt0;
var validips;
var addvalidips;
//var ii;	 // num
var ipno;
var maxnologindays;	 // num
var lastlogindate;
var whoistx;
var body;
var text;
var usersordefinitions;
var userkey;
var newpassword;
var emailsubject;
var lastuserid;
var ucomps;
var userrec;
var locks;
var styles2;
var fields;

function main(in request1, in request2, in request3, in request4, io request5, in request6 = "") {

	/* LISTEN COMMANDS;

	COMMAND  ARGS                SUCCESS    FAILURE;
	-------  ----                -------    -------;
	LOGIN    user,pass,conninfo  userinfo   various reasons;
	READ     file,key            rec        key doesnt exist;
	READU    file,key            rec,lockid key already locked by someone else;
	WRITE    rec,file,key,lockid rec        wrong lockid, or timestamp differs;
	WRITEU   rec,file,key,lockid (same, but unlocks after writing - rarely used);
	DELETE   file,key            ok         key currently locked by someone else;
	SELECT   select statement    data       no recs found;
	GETINDEX file,field,args     data       no recs found;
	LOCK     file,key            lockid     key already locked by someone else;
	RELOCK   file,key,lockid     ok         wrong lockid;
	UNLOCK   file,key,lockid     ok         wrong lockid;
	EXECUTE  cmd,args,data       data       command chooses to fail;
	*/

	// handles VALIDATE,BECOMEUSERANDCONNECTION,LOGIN and RESPOND

	// $insert abp,common
	// $insert bp,agency.common

	// TODO share various files with LISTEN to prevent slowing down by opening?

	let tracing = 1;

	// no output in arguments allowed since c++ doesnt allow
	// defaulting or value based arguments and setting them
	// and this means calling listen2 would require passing variables for all
	// in the few cases listen2 need to respond to caller, it sets @ans
	// if unassigned(request2in) then requestin='' else request2=request2in
	// if unassigned(request3in) then requestin='' else request3=request3in
	// if unassigned(request4in) then requestin='' else request4=request4in
	// if unassigned(request5in) then requestin='' else request5=request5in
	// if unassigned(request6in) then requestin='' else request6=request6in
	let logx = request2;

	let isdevsys = var("exodus.id").osfile();

	// will return "" in request5 if valid or an error response message
	if (request1.f(1) == "VALIDATE") {

		var username	 = request2;
		let password	 = request3;
		let connection	 = request4;
		let dataset		 = request6;
		let origrequest1 = request1.f(2);

		// 1. ANYBODY can request password reset with their email address
		// 2. email addresses are not secret and usernames are guessable
		// 3. magic character in password on logins causes password reset if email
		// as the password. Therefore NO @ characters in passwords
		var passwordreset = origrequest1 == "LOGIN" and password.contains("@");

		// determine username from emailaddress
		// only for users with single, unique emails
		if (passwordreset) {
			var tt = SECURITY.f(7).ucase();
			// password is email address when resetting
			if (tt.locate(password.ucase(), usern)) {
				// only if email address occurs more than once
				tt(1, usern) = "";
				tt.converter("; ", _VM);
				if (not tt.locate(password.ucase())) {
					username = SECURITY.f(1, usern);
				}
			}
		}

		// see also removal of "username and/or " below
		var invalidlogin = "";
		// invalidlogin:='<div style="text-align:left;width:400px;margin-left:auto;margin-right:auto">'
		invalidlogin ^= "<div style=\"text-align:left\">";
		invalidlogin ^= "Sorry, your login has been refused.";
		invalidlogin ^= "|<b><font color=red>Please choose one of the following options.</font></b>";
		invalidlogin ^= "|";
		invalidlogin ^= "|Maybe mistyped password?";
		invalidlogin ^= "|or using a multilingual keyboard?";
		invalidlogin ^= "|<b>&rarr;Click \"Show Password\", check it and try again</b>";
		invalidlogin ^= "|";
		invalidlogin ^= "|Wrong database selected?";
		invalidlogin ^= "|<b>&rarr;Select the right DATABASE and try again.</b>";
		invalidlogin ^= "|";
		invalidlogin ^= "|First time logging in?";
		invalidlogin ^= "|or not 100% certain of your password?";
		invalidlogin ^= "|<b><font color=red>or three or more failed login attempts?</font></b>";
		invalidlogin ^= "|or password has expired?";
		invalidlogin ^= "|<b>&rarr;Click Password Reset to get a new password.</b>";
		invalidlogin ^= "|";
		invalidlogin ^= "|Don't have a username?";
		invalidlogin ^= "|or not 100% certain what your username is?";
		invalidlogin ^= "|or not 100% certain what your email address <b>IS IN EXODUS</b>?";
		invalidlogin ^= "|<b>&rarr;Contact your manager.<b>||";
		invalidlogin ^= "</div>";
		// invalidlogin:='||If you know and have access to the email address of this account'
		// invalidlogin:='|<b>you can click Password Reset to get a new password.<b>'
		// NB make sure "Password Reset" does not appear exactly in message
		invalidlogin.replacer("Password Reset", "Password&nbsp;Reset");

		var realreason = "";

		let maxnfails = 3;
		var failn	  = 1;

		// encrypt the password and check it

		let encrypt0 = hashpass(password);

		var userx = "";
		var users;
		if (users.open("USERS", "")) {
			// EXODUS may have no user record
			if (not userx.read(users, username)) {
				userx = "";
			}
		}

		// NB if "EXODUS" is in Auth File then some user restrictions apply eg password/ips

		// check username exists
		if (not SECURITY.f(1).locate(username, usern)) {
			usern = 0;
		}

		// 1. user is in auth file
		if (usern) {
			// read the user immediately even though (currently) most info is
			// obtained from userprivs
			if (users) {

				if (not userx) {
					goto validateexit;
				}

				// check failed logins no more than maxnfails consecutive fails
				// in the users' login log
				// TODO implement the same on bad usernames to avoid
				// different response to good versus bad usernames
				// which would allow detection of valid usernames
				for (failn = 1; failn <= 999999; ++failn) {
					let tt = userx.f(18, failn);
					// /BREAK;
					if (not(tt != "" and tt.first(2) != "OK"))
						break;
				}  // failn;
				   //do normal authorisation to show type of failure - but fail even if ok
				   //if failn>=maxnfails then
				   // goto validateexit
				   // end
				   //too many consecutive fails
				if (failn > maxnfails) {
					goto validateexit;
				}
				// check account expiry
				if (userx.f(35) and date() >= userx.f(35)) {

					realreason = "Login user account expired";

					// not any more
					// expired account always appears like failed user/password
					// to end user - there is no indication that the failure to login
					// is due to expiry. They can even do password reset etc.
					// but the passwords just wont work
					// this is to allow terminating access without explanation
					// realreason=''

					// goto passfail;
					goto validateexit;
				}

				// TODO prevent login after termination
				// holiday dates are currently used to indicate termination
				// in a way that arent nice/easy to understand for termination
			}

			// find the encrypted password
			// userencrypt0=userprivs<4,usern>
			// use the password on the user if present by preference
			// because new password might not have been put on authorisation file
			// if the file was locked at the time user reset their password
			userencrypt0 = userx.f(4, 1);
			if (userencrypt0 == "") {
				// TODO remove all encrypted passwords from userprivs and put all on user
				// in PREWRITE new security
				userencrypt0 = SECURITY.f(4, usern, 2).field(TM, 7);
			}

			// check password is correct
			// if encrypt0<>field(userencrypt0<1,1,2>,tm,7) then
			if (encrypt0 != userencrypt0) {

				realreason = "Wrong password";

				// passfail:
				// password is correct
			} else {

				// passok:

				// determine allowed validips

				// priority for ip restrictions (no merging)
				// 0. user cache
				// 1. user
				// 2. group user
				// 3. system default
				// 4. standard LAN

				// get the users own/cached valid ip nos (cleared every security read)
				validips = SECURITY.f(6, usern);

				// + in user ipnos means add to group user ip ranges
				if (validips.contains("+")) {
					validips.move(addvalidips);
				} else {
					addvalidips = "";
				}

				// default users valid ipnos if not defined (or cached)
				if (not validips) {

					let nn = SECURITY.f(6).fcount(_VM);

					// get ipnos of group user
					var ii;
					for (ii = usern + 1; ii <= nn; ++ii) {
						// /BREAK;
						if (not(SECURITY.f(1, ii + 1) != "---"))
							break;
					}  // ii;
					validips = SECURITY.f(6, ii);

					// + in group ipnos means add to system config ip ranges
					if (validips.contains("+")) {
						addvalidips ^= " " ^ validips;
						validips = "";
					}

					// otherwise use system default
					if (not validips) {
						validips = SYSTEM.f(39);

						// + in system config ipnos means add traditional private LAN ip ranges
						if (validips.contains("+")) {
							addvalidips ^= " " ^ validips;
							validips = "";
						}
					}

					// otherwise use traditional private ip ranges
					if (not validips) {
						// validips='192.168;10;127'
						// !172.16-31 put at the end for speed
						// for ii=16 to 31
						// validips:=';172.':ii
						// next ii
						validips = "192.168 10 127 172.16 172.17 172.18 172.19 172.20 172.21 172.22 172.23 172.24 172.25 172.26 172.27 172.28 172.29 172.30 172.31";
					}
				}

				// convert wildcards to matches and cache the result
				// converted cache is indicate by a trailing ' '
				if (not validips.ends(" ")) {

					// prefix any additional validips - guessing is faster on average
					if (addvalidips) {
						addvalidips.converter("+", " ");
						validips = addvalidips ^ " " ^ validips;
					}

					// gosub convvalidips
					validips.converter(",;/ " _FM, "     ");
					validips.replacer(".*", "");
					validips.trimmer();

					// 1. allow connection from anywhere if * is present
					// 2. otherwise add allowable connection from 127.* always
					// 3. trailing space indicates defaults and wild cards expanded
					//   and can use cache
					//if (validips.locateusing(" ", "*", xx)) {
					if (validips.locateusing(" ", "*")) {
						validips = " ";
					} else {
						validips ^= " 127 ";
					}

					// cache the users's converted valid ip nos (ending in ' ')
					SECURITY(6, usern) = validips;
				}

				// check allowed access to this database - or exit
				// only checking during LOGIN. TODO is this ok?
				// no longer has to be done after setting @username
				if (origrequest1 == "LOGIN") {
					if (not authorised("DATASET ACCESS " ^ (SYSTEM.f(17).quote()), msg_, "", username)) {
						invalidlogin = msg_;
						goto validateexit;
					}
				}

checkip:
				// //////

				// check is ip no is allowed - or exit
				ipno = connection.f(1, 2);

				if (validips.trim() and ipno) {
					// look for 192.168 first since most common
					// then full ip number, then first bit (eg 10), then 1.2.3 last
					let ip2 = ipno.field(".", 1, 2);
					if (not validips.locateusing(" ", ip2)) {
						if (not validips.locateusing(" ", ipno)) {
							if (not validips.locateusing(" ", ipno.field(".", 1))) {
								if (not validips.locateusing(" ", ipno.field(".", 1, 3))) {
invalidip:
									invalidlogin = username ^ " is not authorised to login from this location (IP Number: " ^ ipno ^ ")";
									invalidlogin ^= "|Allowed:" ^ (validips.quote());
									goto validateexit;
								}
							}
						}
					}

					// EXODUS is validated versus DEFINITIONS, IPNOS*EXODUS
					// which is calculated in INIT.GENERAL from GBP $HOSTS.ALLOW

					// prevent EXODUS from using CONFIGURED fully formed LAN ips
					// which are deemed to be NAT routers possibly providing WAN access
					// but EXODUS should not have unrestricted access from WAN
					if (username == "EXODUS" and not(SYSTEM.f(17).contains("DEMO"))) {
						if (ip2 == "192.168") {
exoduslocalip:
							if (validips.locateusing(" ", ipno)) {
								goto invalidip;
							}
						} else {
							let tt = ipno.field(".", 1);
							if (tt == "10") {
								goto exoduslocalip;
							}
							if (tt == "172") {
								goto exoduslocalip;
							}
						}
					}

					// ip not checked because no ip restrictions
				} else {
				}

				// even if all login checks are ok, fail if too many successive failures
				// since login is blocked, only a password reset or update
				// can reenable the account by getting an OK into the user<18> log
				if (failn > maxnfails) {

					// but send the normal "invalid user/password" reply to user logging in
					// since we dont want a guesser to know they have guessed the right pass
					request5 = invalidlogin;

					// indicate ok but excessive failures to the user/sysadmins
					invalidlogin = "Too many login failures - " ^ (username.quote()) ^ " is blocked";
					if (userx.f(7)) {
						invalidlogin ^= "|" ^ (username.quote()) ^ " can and must get a new password unless the account is expired";
						invalidlogin ^= "|by clicking Password Reset on the EXODUS Login screen";
						invalidlogin ^= "|and entering one of their email addresses as follows:";
						invalidlogin ^= "|" ^ userx.f(7);
					} else {
						invalidlogin ^= "|and has no email address assigned. An administrator must provide a new password for the user";
					}

					// validateexit2 because we dont want request5=invalidlogin
					goto validateexit2;
				}

				if (username != "EXODUS") {

					var passworddate = userx.f(36);

					// check password not expired if expiry days is configured
					let passwordexpirydays = SECURITY.f(25);
					if (passwordexpirydays) {

						// use the last login date if no password date (backward compatible)
						// if no last login date then treat as password expired
						if (not passworddate) {
							passworddate = userx.f(13);
						}

						// int() to ignore time of day so expiring in one day means they can
						// login on the day that the password was changed
						if (date() >= passworddate.floor() + passwordexpirydays) {
							goto passwordexpired;
						}
					}

					// check not exceeded max nologin days
					// default is 31 days from last login or password reset
					maxnologindays = SYSTEM.f(128);
					if (maxnologindays == "") {
						maxnologindays = 31;
					}

					if (maxnologindays) {

						lastlogindate = userx.f(13);

						// if password was reset more recently than last login date then use that
						if (passworddate > lastlogindate) {
							lastlogindate = passworddate;
						}

						// int() to remove time part of date
						if (date() >= lastlogindate.floor() + maxnologindays) {

passwordexpired:

							// some users may be allowed to login rarely without password renewal
							var xx;
							if (not authorised("AUTHORISATION PASSWORD NEVER EXPIRES", xx, "EXODUS", username)) {
								invalidlogin = "Password has expired. Get a new one using Password Reset";
								goto validateexit;
							}
						}
					}
				}

				// check not concurrent use of same username - or exit
				// allowing relock for now on duplicate logins TODO block them
				if (origrequest1 != "LOGIN" and origrequest1 != "RELOCK") {
					// if connection != user<39> then
					// ONLY CHECKING IP NUMBER FOR NOW
					// if we check session number then it causes problem for multiple
					// login on the same computer but
					// ip number is the same for many/all users if
					// 1. client office is accessing server remotely
					// 2. client lan has a natting router between (all or some) users and the server
					// perhaps we can check the client host name but this is unreliable
					// as it can be configured per workstation to be the same for different users
					// the only reliable test is the session no but this prevents multiple
					// login on the same computer in different tabs or browsers
					//
					// if there is no need for user number control (unilimited user licence)
					// or some user name should allow multiple logins then need
					// an option to allow no control or max no logins per database or user
					//
					//
					// if session number agrees then dont test anything else
					// this will allow ip number to change due to network proxy etc
					if (connection.f(1, 5) != userx.f(39, 5) and username != "EXODUS") {

						// the word "automatic" hardcoded in browser to unlock and lose any work
						var tt = "You have been automatically logged out due to another login as " ^ (username.quote());
						tt(-1) = "Please login or try again later.";

						// refuse browser change
						if (connection.f(1, 6) != userx.f(39, 6)) {
							invalidlogin = tt;
							realreason	 = "Duplicate login.";
							goto validateexit;
						}

						// refuse http/https change
						if (connection.f(1, 4) != userx.f(39, 4)) {
							invalidlogin = tt;
							realreason	 = "Duplicate login .";
							goto validateexit;
						}

						// refuse ip number change
						if (connection.f(1, 2) != userx.f(39, 2)) {
							invalidlogin = tt;
							realreason	 = "Duplicate login";
							goto validateexit;
						}

						// refuse session change (least definite so skip for now)
						// invalidlogin=tt
						// goto validateexit
					}
				}

				// send email to user and IT on first or novel ipno logins (not EXODUS)
				if (origrequest1 == "LOGIN") {
					if (username != "EXODUS" or isdevsys) {

						let ulogins = userx.f(18);
						if (ulogins.locate("OK")) {

							// if ipno unrestricted
							if (validips == " " or isdevsys) {

								// find a previous SUCCESSFUL login from same ipnet (ip part 1 & 2 only)
								var isnewipnet = 1;
								let ipno12	   = ipno.field(".", 1, 2);
								let uipnos	   = userx.f(16);
								let nn		   = uipnos.count(_VM);
								for (let ii : range(1, nn)) {
									if (ulogins.f(1, ii) == "OK" and uipnos.f(1, ii).field(".", 1, 2) == ipno12) {
										isnewipnet = 0;
									}
									// /BREAK;
									if (not isnewipnet)
										break;
								}  // ii;

								// email if login unrestricted and on new non-lan ipno
								// locate ipno in uipnos setting xx else
								if (isnewipnet) {

									// send email for non-lan ipnos
									// whois returns 0 for lan ipnos (127 10 100 172 192.168 etc)
									call whois("", ipno, whoistx);

									if (whoistx != "0") {

										body = "This is an automated email from your EXODUS database " ^ SYSTEM.f(17, 1);
										body ^= " recording a successful login from the ip number " ^ ipno;
										body(-1) = "by " ^ userx.f(1) ^ " (" ^ username ^ ")";
										// not in the last 100 login ips
										body(-1) = _FM "You will not be notified of any further successful logins by " ^ username ^ " from this ip number for a while.";

										if (whoistx) {
											gosub addwhoistx();
										}

										var	 subject = "Login on " ^ ipno ^ " by " ^ userx.f(1) ^ " - " ^ username;
										call sysmsg(body, subject, username);
									}
								}
							}

							// no OK means first login
						} else {

							body = "Welcome to EXODUS!";
							// in security.subs and listen2
							body(1, -1) = _VM "Browser configuration *REQUIRED*";
							body(1, -1) = "http://userwiki.neosys.com/index.php/gettingstarted";

							// add whoistx for non-lan ipnos
							call whois("", ipno, whoistx);
							if (whoistx) {
								gosub addwhoistx();
							}

							let subject = "First login of " ^ userx.f(1) ^ " - " ^ username;

							call sysmsg(body, subject, username);
						}
					}
				}

				// validated OK
				invalidlogin = 0;
				gosub becomeuserandconnection(request2, request4);
			}

			// 2. EXODUS check pass in system.file
		} else if (username == "EXODUS") {

			// check for (EXODUS) user and password in revelation system file
			var sysrec;
			if (not sysrec.read(systemfile(), username)) {
				goto validateexit;
			}

			// check password
			if (sysrec.f(7) != encrypt0) {
				realreason = "Wrong password";
				goto validateexit;
			}

			// EXODUS valid ipnos calculated in INIT.GENERAL2 see IPNOS*EXODUS
			// from GBP, $HOSTS.ALLOW at process startup
			if (validips.read(DEFINITIONS, "IPNOS*" ^ username)) {
				// should be removed after medialine is upgraded
				validips.converter(_SM, " ");
			} else {
				validips = "127 192.168 10 172";
			}

			goto checkip;

			// 3. not in users file and not EXODUS
		} else {
			goto validateexit;
		}

validateexit:
		// ///////////
		request5 = invalidlogin;

validateexit2:
		// ////////////
		// save invalid logins in userfile (definitions file for bad usernames)
		// should only check on LOGIN requests?
		if (invalidlogin) {

			if (passwordreset) {
				// "Password Reset Failed" is hardcoded in index.htm

				// Using \r\n for email text format *not* Windows
				text =
					"Password&nbsp;Reset Failed:"
					"\r\n";
				text ^= "\r\n" ^ password ^ " and " ^ username ^ " are unrecognised or expired";
				text ^=
					"\r\n"
					" Database: " ^
					SYSTEM.f(17);
			} else {
				text = invalidlogin;

				// Early exit if relocking since we dont want lots of errors and emails
				if (origrequest1 == "RELOCK") {
					return 0;
				}

				if (failn > maxnfails) {
					// var tt = failn + 1;
					realreason = "Too many consecutive login failures: " ^ failn ^ ", max is " ^ maxnfails;
					invalidlogin ^= "\r\n" ^ (username.quote()) ^ " login is disabled pending password reset by an administrator";
					if (userx.f(7)) {
						invalidlogin ^=
							"\r\n"
							"or user requesting a password reset to " ^
							userx.f(7);
					}
				}
			}

			text.converter(_RM _FM _VM, _SM _SM _SM);

			// open 'USERS' to users then
			if (users) {

				req.srcfile = users;

				if (RECORD.read(users, username)) {
					usersordefinitions = users;
					userkey			   = username;

					if (passwordreset) {

						// !if password is one of users email addresses
						// !locate ucase(password) in ucase(@record<7>) using ';' setting xx then
						// ONLY if user has an email address
						// and account is not expired
						let expirydate = RECORD.f(35);
						if (RECORD.f(7) and ((expirydate == "" or expirydate > date()))) {

							// signify ok
							passwordreset = 2;

							text = "OK Password Reset";

							// random pseudoword
							call securitysubs("GENERATEPASSWORD");
							newpassword = req.is;

							RECORD(4) = newpassword;

							// request5='New password emailed to ':@record<7>
						}
					}

					// username does not exist
				} else {
					text.replacer(" and/or password", "");
					usersordefinitions = DEFINITIONS;
					userkey			   = "BADUSER*" ^ username;
					realreason		   = "Invalid usercode";
					if (not RECORD.read(usersordefinitions, userkey)) {
						RECORD = "";
					}
				}

				// save historical login results in listen2 and security.subs
				if (passwordreset != 2) {

					// datetime=(date():'.':time() 'R(0)#5')+0
					let datetime = date() ^ "." ^ time().oconv("R(0)#5");
					RECORD.inserter(15, 1, datetime);
					RECORD.inserter(16, 1, connection.f(1, 2));
					text.replacer("username and/or ", "");

					var tt = text.field("|", 1);
					if (tt.len() > 80) {
						// tt[81, 999999] = '...'
						// tt.paster(81, 999999, "...");
						// tt.pasterall(81, "...");
						// tt.firster(80) ^= "...";
						tt.firster(80);
						tt ^= "...";
					}

					if (realreason) {
						tt = realreason;
					}

					// trim leading html tags
					while (tt.starts("<")) {
						tt = tt.field(">", 2, 99999);
					}

					RECORD.inserter(18, 1, tt);

					// user log is updated in user.subs
				} else {

					ID = username;

					// prewrite locks authorisation file or fails
					req.valid = 1;
					req.orec  = RECORD;
					call usersubs("PREWRITE.RESETPASSWORD");
					if (not req.valid) {
						msg_.move(request5);
						return 0;
					}
				}

				// trim long user records
				if (RECORD.len() > 5000) {
					let nitems = RECORD.f(15).fcount(_VM);
					RECORD(15) = RECORD.f(15).field(_VM, 1, nitems - 5);
					RECORD(16) = RECORD.f(16).field(_VM, 1, nitems - 5);
					RECORD(18) = RECORD.f(18).field(_VM, 1, nitems - 5);
				}

				// update without locking if not resetting password ?!
				RECORD.write(usersordefinitions, userkey);

				if (passwordreset == 1) {
					invalidlogin = text;
					text.move(request5);

				} else if (passwordreset == 2) {

					// postwrite updates and unlocks authorisation file
					call usersubs("POSTWRITE");

					// cc the user too if the username is valid
					// call sysmsg('User: ':username:fm:'From IP: ':system<40,2>:fm:text,'Password Reset',userkey)

					// send the new password to the user
					// all email addresses not just the one used to get the reset new password
					let emailaddrs = RECORD.f(7);
					let ccaddrs	   = "";
					let subject	   = "EXODUS Password Reset";
					body		   = "";
					body(-1)	   = "Hi! Your new password is " ^ newpassword;
					body(-1)	   = _FM "* Your usercode is " ^ ID;
					body(-1)	   = _FM "This is for";
					body(-1)	   = "Database: " ^ SYSTEM.f(23) ^ " (" ^ SYSTEM.f(17) ^ ")";
					body(-1)	   = "System: " ^ SYSTEM.f(44);

					body.replacer(_FM, "\r\n");
					var xx;
					call sendmail(emailaddrs, ccaddrs, subject, body, "", "", xx);

					// "Password Reset" is hardcoded in index.htm
					request5 =
						"Password Reset ok:"
						"\r\n";
					request5 ^= "\r\n";
					request5 ^= "A new password for usercode " ^ (ID.quote()) ^ " for database " ^ (SYSTEM.f(17, 1).quote()) ^ "\r\n";
					request5 ^= "has been emailed to " ^ emailaddrs ^ "\r\n";
					request5 ^= "\r\n";
					request5 ^=
						"<b>Please check your email to get the new password then"
						"\r\n";
					// request5:='login as user code ':quote(username):' using the new password.</b>'
					request5 ^= "login as user code " ^ (username.quote()) ^ " to database " ^ (SYSTEM.f(17, 1).quote()) ^ "</b>";
				}

				// users file exists
			}

			// cc the user too if the username is valid
			if (passwordreset < 2) {
				if (passwordreset) {
					emailsubject = "Password Reset Failure " ^ password;
				} else {
					// Note: "Login Failure" hardcoded in sysmsg not to send to EXODUS
					emailsubject = "Login Failure " ^ username;
					if (realreason) {
						emailsubject ^= " - " ^ realreason;
					}
				}
				if (failn > 1) {
					emailsubject.replacer("Failure", "Failure (" ^ failn ^ ")");
				}

				let fromipno = connection.f(1, 2);
				msg_		 = "User: " ^ username ^ _FM "From IP: " ^ fromipno;

				var	 cmd = "SELECT USERS BY-DSND LAST_LOGIN_DATETIME WITH LAST_LOGIN_LOCATION " ^ (fromipno.quote()) ^ " AND WITH @ID NOT STARTING \"%\"";
				call safeselect(cmd ^ " (S)");
				var	 lastuser = "";
				if (readnext(lastuserid)) {
					if (lastuser.read(users, lastuserid)) {
						msg_ ^= " (last login was " ^ lastuser.f(1);
						if (lastuser.f(1)) {
							if (lastuserid != lastuser.f(1)) {
								msg_ ^= " (" ^ lastuserid ^ ")";
							}
						} else {
							msg_ ^= lastuserid;
						}
						msg_ ^= " on " ^ oconv(lastuser.f(13), "[DATETIME,4*,MTS]") ^ ")";
					}
					clearselect();
				}
				body = msg_;

				// add whoistx info for non-private ip nos with no prior successful login
				if (lastuser == "" or isdevsys) {
					call whois("", fromipno, whoistx);
					ipno = fromipno;
					gosub addwhoistx();
				}

				// call sysmsg(msg:fm:text,emailsubject,userkey)
				call sysmsg(body, emailsubject, userkey);
			}

			// validated OK
		} else {

			// done in becomeuser now
			// gosub getuserstyle

			// switch to users first company if they arent authorised for the current comp
			// EXODUS may not have user record
			// perhaps we need to switch back to users last company every request
			// in which case even exodus could avoid random companies
			ucomps = userx.f(33);
			gosub switchcompany();

			// record stats per database/sessionid/username/ip per hour
			var statistics;
			if (statistics.open("STATISTICS", "")) {
				var key = dataset;
				key ^= "*" ^ connection.f(1, 5);
				key ^= "*" ^ username;
				key ^= "*" ^ connection.f(1, 3);
				key ^= "*" ^ date();
				let tt = (time() / 3600).floor() + 1;
				key ^= "*" ^ tt;
				var statistic;
				if (not statistic.read(statistics, key)) {
					statistic = "";
				}
				// by not doing 'MD50P'
				// statistic<1>=(date()+time()/86400) 'MD50P'
				statistic(1) = (date() + time() / 86400).first(12);
				if (origrequest1 != "RELOCK") {
					statistic(2) = statistic.f(2) + 1;
				}
				// enable fast stats without cross database user access
				statistic(3) = userrec.f(5);
				statistic.write(statistics, key);
			}
		}

	} else if (request1 == "BECOMEUSERANDCONNECTION") {
		gosub becomeuserandconnection(request2, request4);

	} else if (request1 == "CONNECTION") {

		// username and password is already validated at this point
		// but we need to get users cookies for company, market, menus etc (or fail)
	} else if (request1 == "LOGIN") {

		let dataset	 = request2.ucase();
		let username = request3.ucase();

		data_			  = "";
		msg_			  = "";
		var authcompcodes = "";

		// special login routine
		// returns iodat (cookie string "x=1&y=2" etc and optional comment msg)
		// open 'VOC' to voc then
		// read xx from voc,'LOGIN.NET' then
		var cookie	 = "";
		var loginmsg = "";
		// dont pass system variables
		call loginnet(dataset, username, cookie, loginmsg, authcompcodes);
		data_ = cookie;
		msg_  = loginmsg;
		if (data_ == "") {
			response_ = msg_;
			return 0;
		}
		//  end
		// end

		if (not data_) {
			data_ = "X=X";
		}
		response_ = ("OK " ^ msg_).trim();

		// record the last login per user
		var users;
		if (users.open("USERS", "")) {

			// allow for EXODUS not in users - all others should be but if not then create
			if (not userrec.read(users, username)) {
				userrec = "";
			}

			// convert old data
			if (userrec.f(13) != userrec.f(15, 1) and userrec.f(18) == "") {
				if (userrec.f(15) and not(userrec.f(18))) {
					userrec(18) = "OK";
				}
				userrec.inserter(15, 1, userrec.f(13));
				userrec.inserter(16, 1, userrec.f(14));
				userrec.inserter(18, 1, "OK");
			}

			// save current login
			// datetime=(date():'.':time() 'R(0)#5')+0
			let datetime = date() ^ "." ^ time().oconv("R(0)#5");
			userrec(13)	 = datetime;
			userrec(14)	 = SYSTEM.f(40, 2);

			// save historical logins
			userrec.inserter(15, 1, userrec.f(13));
			userrec.inserter(16, 1, userrec.f(14));
			userrec.inserter(18, 1, "OK");

			// only keep the last 100 logins
			userrec(15) = userrec.f(15).field(_VM, 1, 100);
			userrec(16) = userrec.f(16).field(_VM, 1, 100);
			userrec(18) = userrec.f(18).field(_VM, 1, 100);

			if (authcompcodes) {
				userrec(33) = authcompcodes;
			}

			// connection
			userrec(39) = request4;

			userrec.write(users, username);

			// ensure this user gets control of the user record
			// to prevent "user is locked message" in case
			// another user is logged in on the same name or workstation failure
			// if (openfile("LOCKS", locks)) {
			// Use LOCKS file on same connection as the USERS file
			if (locks.open("LOCKS", users)) {
				locks.deleterecord("USERS*" ^ username);
			}
		}

	} else if (request1 == "RESPOND") {

		// method to allow sys programs to respond and detach before finishing

		// ////////////////////////////////////////////////////////////////////
		// VERY IMPORTANT TO AVOID LONG RUNNING SYSTEM PROCESSES FROM TIMING OUT
		// ////////////////////////////////////////////////////////////////////
		SYSTEM(25) = "";

		// determine the responsefile name from the printfilename
		// responsefilename=system<2>
		// if responsefilename else return
		// t=field2(responsefilename,'.',-1)
		// responsefilename[-len(t),len(t)]='3'

		// linkfilename3
		let responsefilename = PRIORITYINT.f(100);
		if (not responsefilename) {
			return 0;
		}

		// detach the calling process
		response_ = request2;
		response_.converter(_VM "|", _FM _FM);
		response_.replacer(_FM, _EOL);

		//call oswrite(response_, responsefilename);
		if (not oswrite(response_, responsefilename))
			abort(lasterror());
		// osclose responsefilename

		call ossleep(1000 * 2);

		// indicate that response has been made
		SYSTEM(2) = "";

	} else {
		printl(request1.quote(), " invalid request in LISTEN2");
	}

	return 0;
}

subroutine becomeuserandconnection(in request2, in request4) {

	let username   = request2;
	let connection = request4;

	// set @username
	USERNAME = (username);

	// set @station
	var tt = connection.f(1, 2);
	if (connection.f(1, 3) != tt) {
		tt ^= "_" ^ connection.f(1, 3);
	}
	tt.converter(". ", "_");
	// call sysvar('SET',109,110,t)
	// call sysvar_109_110('SET',t)
	STATION = (tt);

	// restore default style
	SYSTEM(46) = SYSTEM.f(47);

	// in LISTEN2 and INIT.COMPANY
	let companystyle = srv.company.f(70);
	if (companystyle) {
		SYSTEM(46) = companystyle;
	}

	// get user style
	userrec = "";
	var users;
	if (users.open("USERS", "")) {
		if (userrec.read(users, USERNAME)) {

			ucomps = userrec.f(33);
			gosub switchcompany();

			var styles = userrec.f(19);
			styles.replacer("Default", "");
			if (not styles2.readf(users, userrec.f(21), 19)) {
				styles2 = "";
			}

			// save time on every request by ignoring this authorisation
			// chkauth=index(userprivs,'USER UPDATE "',1)
			let chkauth = 0;
			if (chkauth) {
				fields	  = "";
				fields(1) = "REPORT HEAD COLOR";
				fields(2) = "REPORT BODY COLOR";
				fields(3) = "REPORT FONT NAME";
				fields(8) = "REPORT FONT SIZE";
			}

			var runstyles = SYSTEM.f(46);
			for (const var vn : range(1, 9)) {
				var tt2 = styles.f(1, vn);

				// skip user styles if not authorised
				if (chkauth) {
					let fieldname = fields.f(vn);
					if (fieldname) {
						for (let ii : range(1, 3)) {
							// USER UPDATE "REPORT"
							// USER UPDATE "REPORT HEAD"
							// USER UPDATE "REPORT HEAD COLOR"
							// etc
							var xx;
							if (not authorised("USER UPDATE " ^ (fieldname.field(" ", ii).quote()), xx)) {
								tt2 = "";
							}
						}  // ii;
					}
				}

				// departmental default
				if (not tt2.len()) {
					tt2 = styles2.f(1, vn);
				}

				// dont copy defaults so the system default will apply
				if (tt2 == "") {
				} else if (tt2 == "Default") {
				} else if (tt2 == "100") {
				} else {
					runstyles(1, vn) = tt2;
				}
			}  // vn;
			SYSTEM(46) = runstyles;
			// system<10,1>=userrec<1>
			// system<10,2>=userrec<7>
		}
	}
	return;
}

subroutine addwhoistx() {
	if (whoistx) {
		body(-1) = _FM "\"whois\" ip number " ^ ipno ^ " information:";
		body(-1) = _FM ^ whoistx;
		body(-1) = _FM
				   "end of "
				   "\"whois\" ip number " ^
				   ipno ^ " information:";
	}
	return;
}

subroutine switchcompany() {
	if (not ucomps.locate(srv.gcurrcompcode)) {
		let tc = ucomps.f(1, 1);
		var tt;
		if (tt.read(srv.companies, tc)) {
			call initcompany(tc);
		}
	}
	return;
}

libraryexit()
