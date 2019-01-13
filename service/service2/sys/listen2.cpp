#include <exodus/library.h>
libraryinit()

#include <listen2.h>
#include <sysmsg.h>
#include <encrypt2.h>
#include <authorised.h>
#include <whois.h>
#include <systemfile.h>
#include <securitysubs.h>
#include <usersubs.h>
#include <sendmail.h>
#include <safeselect.h>
#include <loginnet.h>
#include <openfile.h>
#include <singular.h>
//#include <collectixvals.h>
//#include <v119.h>
#include <invertarray.h>
#include <htmllib2.h>
#include <select2.h>
#include <trim2.h>
#include <rtp57.h>
#include <listen4.h>
#include <otherusers.h>
#include <readbakpars.h>
#include <initcompany.h>

#include <gen.h>
#include <fin.h>
#include <agy.h>
#include <win.h>

var filename;
var processes;
var tt;
var xx;
var inpath;
var yy;
var zz;
var tdd;
var pattern;
var ageinsecs;//num
var patchfile;
var usern;//num
var userencrypt0;
var validips;
var addvalidips;
var ii;//num
var ipno;
var maxnologindays;//num
var lastlogindate;
var whoistx;
var body;
var sysrec;
var text;
var usersordefinitions;
var userkey;
var newpassword;
var emailsubject;
var lastuserid;
var ucomps;
var statistic;
var userrec;
var locks;
var flag;
var stopn;
var lockid;
var dostime;//num
var bakpars;
var styles2;
var fields;
var msg2;
var fileattributes;
var filetime;

function main(in request1, in request2in="", in request3in="", in request4in="", in request5in="", in request6in="") {
	//c sys in,"","","","",""
	//jbase
	//global ii,userencrypt0,passwordexpired,lastlogindate,maxnologindays,validips
	//global filetime,fileattributes

	//TODO share various files with LISTEN to prevent slowing down by opening?

	var tracing = 1;

	//no output in arguments allowed since c++ doesnt allow
	// defaulting or value based arguments and setting them
	// and this means calling listen2 would require passing variables for all
	//in the few cases listen2 need to respond to caller, it sets @ans
	var request2 = request2in;
	var request3 = request3in;
	var request4 = request4in;
	var request5 = request5in;
	var request6 = request6in;
	var logx = request2;

	var isdevsys = var("NEOSYS.ID").osfile();

	if (request1 == "RUNS") {

		//db start commands
		//initdir '*.RUN'
		//filenamesx=dirlist()
		var filenamesx = oslistf("*.RUN");

		for (var filen = 1; filen <= 9999; ++filen) {
			filename = filenamesx.a(filen);
		///BREAK;
		if (not filename) break;;
			if (lockrecord("PROCESSES", processes, "START*" ^ filename)) {
				if (tt.osread(filename)) {
					tt.converter("\r\n", FM);
					//dont start if there is a database stop command
					if (not((tt.a(1) ^ ".END").osfile())) {
						if (tt.a(5)) {
							//garbagecollect;
							tt = "CMD /C START NEOSYS.JS /system " ^ tt.a(5) ^ " /database " ^ tt.a(1) ^ " /pid " ^ tt.a(6);
							printl(var().at(0), var().at(-4), (var().time()).oconv("MTS"), " ", tt);
							tt.osshell();
						}
						filename.osdelete();
					}
				}
				xx = unlockrecord("", processes, "START*" ^ filename);
			}
		};//filen;

	} else if (request1 == "DELETEOLDFILES2") {

		inpath = request3;
		//delete old response and temp files every 1 minute
		tt = "DELETEOLDFILES";
		tt = "";
		var t2 = "*.4";
		call listen2(tt, t2, inpath, tt, yy, zz);
		t2 = "*.5";
		call listen2(tt, t2, inpath, tt, yy, zz);
		//call listen2(tt,'*.$2','.\',tt,yy,zz)
		//shell2 cannot/does not delete its tempfiles due to wget remaining in background
		var t60 = 60;
		t2 = "VDM*.tmp";
		call listen2(tt, t2, ".\\", t60, yy, zz);
		t2 = "*.$$*";
		call listen2(tt, tdd, ".\\", t60, yy, zz);

	} else if (request1 == "DELETEOLDFILES") {

		pattern = request2;
		inpath = request3;

		//delete files older than x
		ageinsecs = request4;
		if (not ageinsecs) {
			ageinsecs = SYSTEM.a(28);
		}
		if (ageinsecs == "") {
			ageinsecs = 60 * 60;
		}

		gosub deleteoldfiles();

	} else if (request1 == "CHECKRESTART") {

		//return result in @ans

		var s100 = SYSTEM.a(100);

		if (SYSTEM.index(0x00, 1)) {
			var(SYSTEM).oswrite("SYSTEM.BAD");
			call sysmsg("Corrupt SYSTEM record in LISTEN - RESTARTING");
			ANS = "CORRUPTSYSTEM";
			SYSTEM.converter(0x00, "");
restart:
			ANS = "RESTART " ^ ANS;
			return 0;
		}

		//detect system parameter changes and restart
		//this has the effect of detecting corruption in system which inserts lines
		tt = "SYSTEM.CFG";
		if (tt.osfile().a(3) ne s100.a(1, 2)) {
			ANS = tt;
			goto restart;
		}
		tt = "..\\..\\" "SYSTEM.CFG";
		if (tt.osfile().a(3) ne s100.a(1, 1)) {
			ANS = tt;
			goto restart;
		}

		//check for upgrade to LISTEN
		var gbp;
		if (gbp.open("GBP", "")) {
			var listen;
			if (listen.read(gbp, "$LISTEN")) {
				listen = field2(listen, FM, -1);
				if (s100.a(1, 3)) {
					if (s100.a(1, 3) ne listen) {
						ANS = "$LISTEN";
						goto restart;
					}
				}else{
					SYSTEM.r(100, 3, listen);
				}
			}
		}
		ANS = "";
		return 0;

	} else if (request1 == "PATCHANDRUNONCE") {

		//never patch and run on development systems (therefore can only test elsewhere)
		//or on test systems which can be patched via dataset.1 if needed
		//TODO work out a way to ensure both live and test programs are updated
		if (SYSTEM.a(61) or isdevsys) {
nopatch:
			ANS = "";
			return 0;
		}

		//1. patchcode=PATCH - cannot be used to patch data (only system)
		// since it may be picked up by any databases listening process
		//2. patchcode=databasecode - can be used to patch one database (and system)
		//3. no way to patch all databases datafiles
		var patchcode = request2;
		processes = request3;

		//if patch appears then install it
		var patchfilename = patchcode ^ ".1";
		var patchfiledir = patchfilename.osfile();
		if (not patchfiledir) {
			goto nopatch;
		}

		//if patching blocked (eg failed to delete last time) then also quit
		var blockpatchfilename = patchfilename;
		blockpatchfilename.splicer(-1, 1, "X");
		if (blockpatchfilename.osfile()) {
			goto nopatch;
		}

		//ensure patch file is complete
		if (not patchfile.osopen(patchfilename)) {
			goto nopatch;
		}

		tt = patchfilename.osfile().a(1) - 18;
		tt.osbread(patchfile, tt, 18);
		if (tt ne ("!" ^ FM ^ "!END!OF!INSTALL!")) {
			goto nopatch;
		}

		//installation wide lock on it
		if (not(lockrecord("", processes, patchfilename))) {
			goto nopatch;
		}

		//ensure that we only ever runonce something just loaded from a patch
		var runoncekey = "$" ^ patchcode ^ ".RUNONCE";
		DEFINITIONS.deleterecord(runoncekey);
		

		//indicate patched/may need restart
		ANS = 1;

		if (not isdevsys) {

			var cmd = "INSTALL " ^ patchcode ^ " " ^ oscwd().substr(1,2) ^ " (IO)";
			printl(cmd);
			perform(cmd);

			//17/12/2009
			//tt='Size:':patchfiledir<1>:' ':patchfiledir<2> '[DATE,4*]':' ':patchfiledir<3> 'MTS'
			//call sysmsg(cmd:fm:tt)

		}

		//prevent it being installed again
		(patchfilename ^ "O").osdelete();
		("CMD /C REN " ^ patchfilename ^ " " ^ patchfilename ^ "O").osshell();
		patchfilename.osdelete();

		//if cannot delete then put a blocker on it
		if (patchfilename.osfile()) {
			var(var().date() ^ FM ^ var().time()).oswrite(blockpatchfilename);
		}

		//if $PATCH.RUNONCE or $datasetcode.RUNONCE appears in definitions
		//if the runonce record appears in the definitions then
		//run it, save it and delete it
		var runonce;
		if (runonce.read(DEFINITIONS, runoncekey)) {
			if (not isdevsys) {
				perform("RUN DEFINITIONS " ^ runoncekey.substr(2,9999));
			}
			runonce.write(DEFINITIONS, runoncekey ^ "*LAST");
			DEFINITIONS.deleterecord(runoncekey);
			
		}
		runonce = "";

		//trigger other processes to restart by updating SYSTEM.CFG
		if (tt.osread("SYSTEM.CFG")) {
			var(tt).oswrite("SYSTEM.CFG");
		}

		//release
		call unlockrecord("", processes, patchfilename);

		//indicate patches applied and may need restart
		ANS = 1;
		return 0;

	} else if (request1 == "CONVLOG") {

		//assumes at least 0-31 and 128-255 encoded like $hh

		//reserve/use special characters for field separators
		logx.swapper("^", "%5E");
		logx.swapper("]", "%5D");
		logx.swapper("\\", "%5C");
		logx.swapper("[", "%5B");
		logx.swapper("%FE", "^");
		logx.swapper("%FD", "]");
		logx.swapper("%FC", "\\");
		logx.swapper("%FB", "[");
		logx.swapper(RM, "%FF");
		//swap fm with '^' in logx
		//swap vm with ']' in logx
		//swap sm with '\' in logx
		//swap tm with "[" in logx
		logx.converter("\xFE\x22\x20\x5F\x56\x4D\x5F\x20\x22\x22\x20\x5F\x53\x4D\x5F\x20\x22\x22\x20\x5F\x54\x4D\x5F\x20\x22", "^]\\[");
		//fefdfcfb=char(254):char(253):char(252):char(251)
		//convert fefdfcfb to '^]\[' in logx
		logx.swapper(STM, "%FA");

		logx.swapper("%20", " ");
		//does not seem to format in XML
		//swap '%20' with '&nbsp;' in logx

		logx.swapper("&", "&amp;");
		logx.swapper(DQ, "&quot;");
		logx.swapper("<", "&lt;");
		logx.swapper(">", "&gt;");
		logx.swapper("%26", "&amp;");
		logx.swapper("%22", "&quot;");
		logx.swapper("%3C", "&lt;");
		logx.swapper("%3E", "&gt;");

	} else if (request1 == "PROCESSINIT") {

		//put username and database on the bottom line of the screen
		var username = USERNAME.trim();
	//username=rnd(1000000)
		var s23 = SYSTEM.a(23);
		var s17 = SYSTEM.a(17);
		s23.converter(" ", FM);
		//locate 'TEST' in s23 setting xx then username:='*' else
		if (s23.locateusing("TESTDATA", VM, xx)) {
			username ^= "*";
		}else{
			if (s23.locateusing("TESTING", VM, xx)) {
				username ^= "*";
			}else{
				if (s23.locateusing("TRAINING", VM, xx)) {
					username ^= "*";
				}else{
					if (s17.index("TEST", 1)) {
						username ^= "*";
					}
				}
			}
		}
		// end
		var bottomline = (s17 ^ " " ^ SYSTEM.a(24)).oconv("L#40") ^ username.oconv("R#40");
		gosub printbottomline();

	} else if (request1 == "PROCESSEXIT") {
		var bottomline = var(80).space();
		gosub printbottomline();

	} else if (request1.a(1) == "VALIDATE") {

		var username = request2;
		var password = request3;
		var connection = request4;
		var dataset = request6;
		var word2 = request1.a(2);

		//1. ANYBODY can request password reset with their email address
		//2. email addresses are not secret and usernames are guessable
		//3. magic character in password on logins causes password reset if email
		//as the password. Therefore NO @ characters in passwords
		var passwordreset = word2 == "LOGIN" and password.index("@", 1);

		//determine username from emailaddress
		//only for users with single, unique emails
		if (passwordreset) {
			tt = SECURITY.ucase();
			//password is email address when resetting
			if (tt.a(7).locateusing(password.ucase(), VM, usern)) {
				//only if email address occurs more than once
				tt = tt.a(7);
				tt.r(1, usern, "");
				tt.converter("; ", VM);
				if (not tt.locateusing(password.ucase(), VM, xx)) {
					username = SECURITY.a(1, usern);
				}
			}
		}

		//see also removal of "username and/or " below
		var invalidlogin = "";
		//invalidlogin:='<div style="text-align:left;width:400px;margin-left:auto;margin-right:auto">'
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
		invalidlogin ^= "|Don\'t have a username?";
		invalidlogin ^= "|or not 100% certain what your username is?";
		invalidlogin ^= "|or not 100% certain what your email address <b>IS IN NEOSYS</b>?";
		invalidlogin ^= "|<b>&rarr;Contact your manager.<b>||";
		invalidlogin ^= "</div>";
		//invalidlogin:='||If you know and have access to the email address of this account'
		//invalidlogin:='|<b>you can click Password Reset to get a new password.<b>'
		//NB make sure "Password Reset" does not appear exactly in message
		invalidlogin.swapper("Password Reset", "Password&nbsp;Reset");

		var realreason = "";

		var maxnfails = 3;
		var failn = 1;

		//encrypt the password and check it

		var encrypt0 = encrypt2(password ^ "");

		var USER = "";
		var users;
		if (users.open("USERS", "")) {
			//NEOSYS may have no user record
			if (not USER.read(users, username)) {
				{}
			}
		}

		//NB if "NEOSYS" is in Auth File then some user restrictions apply eg password/ips

		//check username exists
		if (not SECURITY.a(1).locateusing(username, VM, usern)) {
			usern = 0;
		}

		//1. user is in auth file
		if (usern) {
			//read the user immediately even though (currently) most info is
			//obtained from userprivs
			if (users) {

				if (not USER) {
					goto validateexit;
				}

				//check failed logins no more than maxnfails consecutive fails
				//in the users' login log
				//TODO implement the same on bad usernames to avoid
				//different response to good versus bad usernames
				//which would allow detection of valid usernames
				for (var failn = 1; failn <= 999999; ++failn) {
					tt = USER.a(18, failn);
				///BREAK;
				if (not(tt ne "" and tt.substr(1,2) ne "OK")) break;;
				};//failn;
				//do normal authorisation to show type of failure - but fail even if ok
				//if failn>=maxnfails then
				// goto validateexit
				// end

				//check account expiry
				if (USER.a(35) and var().date() >= USER.a(35)) {

					realreason = "Login user account expired";

					//not any more
					//expired account always appears like failed user/password
					//to end user - there is no indication that the failure to login
					//is due to expiry. They can even do password reset etc.
					//but the passwords just wont work
					//this is to allow terminating access without explanation
					//realreason=''

					goto passfail;
				}

				//TODO prevent login after termination
				//holiday dates are currently used to indicate termination
				//in a way that arent nice/easy to understand for termination

			}

			//find the encrypted password
			//userencrypt0=userprivs<4,usern>
			//use the password on the user if present by preference
			//because new password might not have been put on authorisation file
			//if the file was locked at the time user reset their password
			userencrypt0 = USER.a(4, 1);
			if (userencrypt0 == "") {
				//TODO remove all encrypted passwords from userprivs and put all on user
				//in PREWRITE new security
				userencrypt0 = SECURITY.a(4, usern, 2).field(TM, 7);
			}

			//check password is correct
			//if encrypt0<>field(userencrypt0<1,1,2>,tm,7) then
			if (encrypt0 ne userencrypt0) {

				realreason = "Wrong password";

passfail:
				{}

				//password is correct
			}else{

	//passok:

				//determine allowed validips

				//priority for ip restrictions (no merging)
				//0. user cache
				//1. user
				//2. group user
				//3. system default
				//4. standard LAN

				//get the users own/cached valid ip nos (cleared every security read)
				validips = SECURITY.a(6, usern);

				//+ in user ipnos means add to group user ip ranges
				if (validips.index("+", 1)) {
					validips.transfer(addvalidips);
				}else{
					addvalidips = "";
				}

				//default users valid ipnos if not defined (or cached)
				if (not validips) {

					var nn = (SECURITY.a(6)).count(VM) + 1;

					//get ipnos of group user
					for (ii = usern + 1; ii <= nn; ++ii) {
					///BREAK;
					if (not(SECURITY.a(1, ii + 1) ne "---")) break;;
					};//ii;
					validips = SECURITY.a(6, ii);

					//+ in group ipnos means add to system config ip ranges
					if (validips.index("+", 1)) {
						addvalidips ^= " " ^ validips;
						validips = "";
					}

					//otherwise use system default
					if (not validips) {
						validips = SYSTEM.a(39);

						//+ in system config ipnos means add traditional private LAN ip ranges
						if (validips.index("+", 1)) {
							addvalidips ^= " " ^ validips;
							validips = "";
						}

					}

					//otherwise use traditional private ip ranges
					if (not validips) {
						//validips='192.168;10;127'
						//!172.16-31 put at the end for speed
						//for ii=16 to 31
						// validips:=';172.':ii
						// next ii
						validips = "192.168 10 127 172.16 172.17 172.18 172.19 172.20 172.21 172.22 172.23 172.24 172.25 172.26 172.27 172.28 172.29 172.30 172.31";
					}

				}

				//convert wildcards to matches and cache the result
				//converted cache is indicate by a trailing ' '
				if (validips[-1] ne " ") {

					//prefix any additional validips - guessing is faster on average
					if (addvalidips) {
						addvalidips.converter("+", " ");
						validips = addvalidips ^ " " ^ validips;
					}

					//gosub convvalidips
					validips.converter(",;/ " ^ FM, "     ");
					validips.swapper(".*", "");
					validips.trimmer();

					//1. allow connection from anywhere if * is present
					//2. otherwise add allowable connection from 127.* always
					//3. trailing space indicates defaults and wild cards expanded
					// and can use cache
					if (validips.locateusing("*", " ", xx)) {
						validips = " ";
					}else{
						validips ^= " 127 ";
					}

					//cache the users's converted valid ip nos (ending in ' ')
					SECURITY.r(6, usern, validips);

				}

				//check allowed access to this database - or exit
				//only checking during LOGIN. TODO is this ok?
				//no longer has to be done after setting @username
				if (word2 == "LOGIN") {
					if (not(authorised("DATASET ACCESS " ^ (DQ ^ (SYSTEM.a(17) ^ DQ)), USER4, "", username))) {
						invalidlogin = USER4;
						goto validateexit;
					}
				}

checkip:
	////////

				//check is ip no is allowed - or exit
				ipno = connection.a(1, 2);

				if (validips.trim() and ipno) {
					//look for 192.168 first since most common
					//then full ip number, then first bit (eg 10), then 1.2.3 last
					var ip2 = ipno.field(".", 1, 2);
					if (not(validips.locateusing(ip2, " ", xx))) {
						if (not(validips.locateusing(ipno, " ", xx))) {
							if (not(validips.locateusing(ipno.field(".", 1), " ", xx))) {
								if (not(validips.locateusing(ipno.field(".", 1, 3), " ", xx))) {
invalidip:
									invalidlogin = username ^ " is not authorised to login from this location (IP Number: " ^ ipno ^ ")";
									invalidlogin ^= "|Allowed:" ^ (DQ ^ (validips ^ DQ));
									goto validateexit;
								}
							}
						}
					}

					//NEOSYS is validated versus DEFINITIONS, IPNOS*NEOSYS
					//which is calculated in INIT.GENERAL from GBP $HOSTS.ALLOW

					//prevent NEOSYS from using CONFIGURED fully formed LAN ips
					//which are deemed to be NAT routers possibly providing WAN access
					//but NEOSYS should not have unrestricted access from WAN
					if (username == "NEOSYS" and not (SYSTEM.a(17)).index("DEMO", 1)) {
						if (ip2 == "192.168") {
neosyslocalip:
							if (validips.locateusing(ipno, " ", xx)) {
								goto invalidip;
							}
						}else{
							tt = ipno.field(".", 1);
							if (tt == "10") {
								goto neosyslocalip;
							}
							if (tt == "172") {
								goto neosyslocalip;
							}
						}
					}

					//ip not checked because no ip restrictions
				}else{
				}

				//even if all login checks are ok, fail if too many successive failures
				//since login is blocked, only a password reset or update
				//can reenable the account by getting an OK into the user<18> log
				if (failn > maxnfails) {

					//but send the normal "invalid user/password" reply to user logging in
					//since we dont want a guesser to know they have guessed the right pass
					request5 = invalidlogin;

					//indicate ok but excessive failures to the user/sysadmins
					invalidlogin = "Too many login failures - " ^ (DQ ^ (username ^ DQ)) ^ " is blocked";
					if (USER.a(7)) {
						invalidlogin ^= "|" ^ (DQ ^ (username ^ DQ)) ^ " can and must get a new password unless the account is expired";
						invalidlogin ^= "|by clicking Password Reset on the NEOSYS Login screen";
						invalidlogin ^= "|and entering one of their email addresses as follows:";
						invalidlogin ^= "|" ^ USER.a(7);
					}else{
						invalidlogin ^= "|and has no email address assigned. An administrator must provide a new password for the user";
					}

					//validateexit2 because we dont want request5=invalidlogin
					goto validateexit2;

				}

				if (username ne "NEOSYS") {

					var passworddate = USER.a(36);

					//check password not expired if expiry days is configured
					var passwordexpirydays = SECURITY.a(25);
					if (passwordexpirydays) {

						//use the last login date if no password date (backward compatible)
						//if no last login date then treat as password expired
						if (not passworddate) {
							passworddate = USER.a(13);
						}

						//int() to ignore time of day so expiring in one day means they can
						//login on the day that the password was changed
						if (var().date() >= passworddate.floor() + passwordexpirydays) {
							goto passwordexpired;
						}

					}

					//check not exceeded max nologin days
					//default is 31 days from last login or password reset
					maxnologindays = SYSTEM.a(128);
					if (maxnologindays == "") {
						maxnologindays = 31;
					}

					if (maxnologindays) {

						lastlogindate = USER.a(13);

						//if password was reset more recently than last login date then use that
						if (passworddate > lastlogindate) {
							lastlogindate = passworddate;
						}

						//int() to remove time part of date
						if (var().date() >= lastlogindate.floor() + maxnologindays) {

passwordexpired:

							//some users may be allowed to login rarely without password renewal
							if (not(authorised("AUTHORISATION PASSWORD NEVER EXPIRES", xx, "NEOSYS", username))) {
								invalidlogin = "Password has expired. Get a new one using Password Reset";
								goto validateexit;
							}

						}
					}

				}

				//check not concurrent use of same username - or exit
				//allowing relock for now on duplicate logins TODO block them
				if (word2 ne "LOGIN" and word2 ne "RELOCK") {
					//if connection ne user<39> then
					//ONLY CHECKING IP NUMBER FOR NOW
					//if we check session number then it causes problem for multiple
					//login on the same computer but
					//ip number is the same for many/all users if
					//1. client office is accessing server remotely
					//2. client lan has a natting router between (all or some) users and the server
					//perhaps we can check the client host name but this is unreliable
					//as it can be configured per workstation to be the same for different users
					//the only reliable test is the session no but this prevents multiple
					//login on the same computer in different tabs or browsers
					//
					//if there is no need for user number control (unilimited user licence)
					//or some user name should allow multiple logins then need
					//an option to allow no control or max no logins per database or user
					//
					//
					//if session number agrees then dont test anything else
					//this will allow ip number to change due to network proxy etc
					if (connection.a(1, 5) ne USER.a(39, 5) and username ne "NEOSYS") {

						//the word "automatic" hardcoded in browser to unlock and lose any work
						tt = "You have been automatically logged out due to another login as " ^ (DQ ^ (username ^ DQ));
						tt.r(-1, "Please login or try again later.");

						//refuse browser change
						if (connection.a(1, 6) ne USER.a(39, 6)) {
							invalidlogin = tt;
							realreason = "Duplicate login.";
							goto validateexit;
						}

						//refuse http/https change
						if (connection.a(1, 4) ne USER.a(39, 4)) {
							invalidlogin = tt;
							realreason = "Duplicate login .";
							goto validateexit;
						}

						//refuse ip number change
						if (connection.a(1, 2) ne USER.a(39, 2)) {
							invalidlogin = tt;
							realreason = "Duplicate login";
							goto validateexit;
						}

						//refuse session change (least definite so skip for now)
						//invalidlogin=tt
						//goto validateexit

					}
				}

				//send email to user and IT on first or novel ipno logins (not NEOSYS)
				if (word2 == "LOGIN") {
					if (username ne "NEOSYS" or isdevsys) {

						var ulogins = USER.a(18);
						if (ulogins.locateusing("OK", VM, xx)) {

							//if ipno unrestricted
							if (validips == " " or isdevsys) {

								//find a previous SUCCESSFUL login from same ipnet (ip part 1 & 2 only)
								var isnewipnet = 1;
								var ipno12 = ipno.field(".", 1, 2);
								var uipnos = USER.a(16);
								var nn = uipnos.count(VM);
								for (ii = 1; ii <= nn; ++ii) {
									if (ulogins.a(1, ii) == "OK" and uipnos.a(1, ii).field(".", 1, 2) == ipno12) {
										isnewipnet = 0;
									}
								///BREAK;
								if (not isnewipnet) break;;
								};//ii;

								//email if login unrestricted and on new non-lan ipno
								//locate ipno in uipnos setting xx else
								if (isnewipnet) {

									//send email for non-lan ipnos
									//whois returns 0 for lan ipnos (127 10 100 172 192.168 etc)
									call whois("", ipno, whoistx);

									if (whoistx ne "0") {

										body = "This is an automated email from your NEOSYS database " ^ SYSTEM.a(17, 1);
										body ^= " recording a successful login from the ip number " ^ ipno;
										body.r(-1, "by " ^ USER.a(1) ^ " (" ^ username ^ ")");
										//not in the last 100 login ips
										body.r(-1, FM ^ "You will not be notified of any further successful logins by " ^ username ^ " from this ip number for a while.");

										if (whoistx) {
											gosub addwhoistx();
										}

										var subject = "Login on " ^ ipno ^ " of " ^ USER.a(1) ^ " - " ^ username;
										call sysmsg(body, subject, username);

									}

								}

							}

							//no OK means first login
						}else{

							body = "Welcome to NEOSYS!";
							//in security.subs and listen2
							body.r(1, -1, VM ^ "Browser configuration *REQUIRED*");
							body.r(1, -1, "http://userwiki.neosys.com/index.php/gettingstarted");

							//add whoistx for non-lan ipnos
							call whois("", ipno, whoistx);
							if (whoistx) {
								gosub addwhoistx();
							}

							var subject = "First login of " ^ USER.a(1) ^ " - " ^ username;

							call sysmsg(body, subject, username);
						}

					}
				}

				//validated OK
				invalidlogin = 0;
				gosub becomeuserandconnection( request2,  request4);

			}

		//2. NEOSYS check pass in system.file
		} else if (username == "NEOSYS") {

			//check for (NEOSYS) user and password in revelation system file
			if (not sysrec.read(systemfile(), username)) {
				goto validateexit;
			}

			//check password
			if (sysrec.a(7) ne encrypt0) {
				realreason = "Wrong password";
				goto validateexit;
			}

			//NEOSYS valid ipnos calculated in INIT.GENERAL2 see IPNOS*NEOSYS
			//from GBP, $HOSTS.ALLOW at process startup
			if (validips.read(DEFINITIONS, "IPNOS*" ^ username)) {
				//should be removed after medialine is upgraded
				validips.converter(SVM, " ");
			}else{
				validips = "127 192.168 10 172";
			}

			goto checkip;

		//3. not in users file and not NEOSYS
		} else {
		}
//L4212:
validateexit:
	/////////////
		request5 = invalidlogin;

validateexit2:
	//////////////
		//save invalid logins in userfile (definitions file for bad usernames)
		//should only check on LOGIN requests?
		if (invalidlogin) {

			if (passwordreset) {
				//"Password Reset Failed" is hardcoded in default.htm

				text = "Password&nbsp;Reset Failed:" "\r\n";
				//text:=crlf:'1. ':password:' not known'
				//text:=crlf:'2. no email known for ':username
				text ^= "\r\n" ^ password ^ " and " ^ username ^ " are both unrecognised";
				text ^= "\r\n" " Database: " ^ SYSTEM.a(17);
			}else{
				text = invalidlogin;

				if (failn > maxnfails) {
					tt = failn + 1;
					realreason = "Too many consecutive login failures: " ^ tt ^ ", max is " ^ maxnfails;
					invalidlogin ^= "\r\n" ^ (DQ ^ (username ^ DQ)) ^ " login is disabled pending password reset by an administrator";
					if (USER.a(7)) {
						invalidlogin ^= "\r\n" "or user requesting a password reset to " ^ USER.a(7);
					}
				}

			}

			text.converter("\xFF\xFE\x22\x20\x5F\x56\x4D\x5F\x20\x22", "" _SM_ "" _SM_ "" _SM_ "");

			//open 'USERS' to users then
			if (users) {

				win.srcfile = users;

				if (RECORD.read(users, username)) {
					usersordefinitions = users;
					userkey = username;

					if (passwordreset) {

						//!if password is one of users email addresses
						//!locate ucase(password) in ucase(@record<7>) using ';' setting xx then
						//ONLY if user has an email address
						if (RECORD.a(7)) {

							//signify ok
							passwordreset = 2;

							text = "OK Password Reset";

							//random pseudoword
							call securitysubs("GENERATEPASSWORD");
							newpassword = win.is;

							RECORD.r(4, newpassword);

							//request5='New password emailed to ':@record<7>

						}

					}

					//username does not exist
				}else{
					text.swapper(" and/or password", "");
					usersordefinitions = DEFINITIONS;
					userkey = "BADUSER*" ^ username;
					realreason = "Invalid usercode";
					if (not RECORD.read(usersordefinitions, userkey)) {
						RECORD = "";
					}
				}

				//save historical login results in listen2 and security.subs
				if (passwordreset ne 2) {

					//datetime=(date():'.':time() 'R(0)#5')+0
					var datetime = var().date() ^ "." ^ (var().time()).oconv("R(0)#5");
					RECORD.inserter(15, 1, datetime);
					RECORD.inserter(16, 1, connection.a(1, 2));
					text.swapper("username and/or ", "");

					tt = text.field("|", 1);
					if (tt.length() > 80) {
						tt.splicer(81, 999999, "...");
					}

					if (realreason) {
						tt = realreason;
					}

					//trim leading html tags
					while (true) {
					///BREAK;
					if (not(tt[1] == "<")) break;;
						tt = tt.field(">", 2, 99999);
					}//loop;

					RECORD.inserter(18, 1, tt);

					//user log is updated in user.subs
				}else{

					ID = username;

					//prewrite locks authorisation file or fails
					win.valid = 1;
					win.orec = RECORD;
					call usersubs("PREWRITE.RESETPASSWORD");
					if (not win.valid) {
						USER4.transfer(request5);
						return 0;
					}

				}

				//trim long user records
				if (RECORD.length() > 5000) {
					var nitems = (RECORD.a(15)).count(VM) + 1;
					RECORD.r(15, RECORD.a(15).field(VM, 1, nitems - 5));
					RECORD.r(16, RECORD.a(16).field(VM, 1, nitems - 5));
					RECORD.r(18, RECORD.a(18).field(VM, 1, nitems - 5));
				}

				//update without locking if not resetting password ?!
				RECORD.write(usersordefinitions, userkey);

				if (passwordreset == 1) {
					invalidlogin = text;
					text.transfer(request5);

				} else if (passwordreset == 2) {

					//postwrite updates and unlocks authorisation file
					call usersubs("POSTWRITE");

					//cc the user too if the username is valid
					//call sysmsg('User: ':username:fm:'From IP: ':system<40,2>:fm:text,'Password Reset',userkey)

					//send the new password to the user
					//all email addresses not just the one used to get the reset new password
					var emailaddrs = RECORD.a(7);
					var ccaddrs = "";
					var subject = "NEOSYS Password Reset";
					body = "";
					body.r(-1, "Hi! Your new password is " ^ newpassword);
					body.r(-1, FM ^ "* Your usercode is " ^ ID);
					body.r(-1, FM ^ "This is for");
					body.r(-1, "Database: " ^ SYSTEM.a(23) ^ " (" ^ SYSTEM.a(17) ^ ")");
					body.r(-1, "System: " ^ SYSTEM.a(44));

					body.swapper(FM, "\r\n");
					call sendmail(emailaddrs, ccaddrs, subject, body, "", "", xx);

					//"Password Reset" is hardcoded in default.htm
					request5 = "Password Reset ok:" "\r\n";
					request5 ^= "\r\n";
					request5 ^= "A new password for usercode " ^ (DQ ^ (ID ^ DQ)) ^ " for database " ^ (DQ ^ (SYSTEM.a(17, 1) ^ DQ)) ^ "\r\n";
					request5 ^= "has been emailed to " ^ emailaddrs ^ "\r\n";
					request5 ^= "\r\n";
					request5 ^= "<b>Please check your email to get the new password then" "\r\n";
					//request5:='login as user code ':quote(username):' using the new password.</b>'
					request5 ^= "login as user code " ^ (DQ ^ (username ^ DQ)) ^ " to database " ^ (DQ ^ (SYSTEM.a(17, 1) ^ DQ)).oconv(":</b>");

				}
//L5253:
				//users file exists
			}

			//cc the user too if the username is valid
			if (passwordreset < 2) {
				if (passwordreset) {
					emailsubject = "Password Reset Failure " ^ password;
				}else{
					//Note: "Login Failure" hardcoded in sysmsg not to send to NEOSYS
					emailsubject = "Login Failure " ^ username;
					if (realreason) {
						emailsubject ^= " - " ^ realreason;
					}
				}
				if (failn > 1) {
					emailsubject.swapper("Failure", "Failure (" ^ failn ^ ")");
				}

				var fromipno = connection.a(1, 2);
				USER4 = "User: " ^ username ^ FM ^ "From IP: " ^ fromipno;

				var cmd = "SELECT USERS BY-DSND LAST_LOGIN_DATETIME WITH LAST_LOGIN_LOCATION " ^ (DQ ^ (fromipno ^ DQ)) ^ " AND WITH @ID NOT STARTING \"%\"";
				call safeselect(cmd ^ " (S)");
				var lastuser = "";
				if (readnext(lastuserid)) {
					if (lastuser.read(users, lastuserid)) {
						USER4 ^= " (last login was " ^ lastuser.a(1);
						if (lastuser.a(1)) {
							if (lastuserid ne lastuser.a(1)) {
								USER4 ^= " (" ^ lastuserid ^ ")";
							}
						}else{
							USER4 ^= lastuserid;
						}
						USER4 ^= " on " ^ (lastuser.a(13)).oconv("[DATETIME,4*,MTS]") ^ ")";
					}
					var().clearselect();
				}
				body = USER4;

				//add whoistx info for non-private ip nos with no prior successful login
				if (lastuser == "" or isdevsys) {
					call whois("", fromipno, whoistx);
					ipno = fromipno;
					gosub addwhoistx();
				}

				//call sysmsg(msg:fm:text,emailsubject,userkey)
				call sysmsg(body, emailsubject, userkey);
			}

			//validated OK
		}else{

			//done in becomeuser now
			//gosub getuserstyle

			//switch to users first company if they arent authorised for the current comp
			//NEOSYS may not have user record
			//perhaps we need to switch back to users last company every request
			//in which case even neosys could avoid random companies
			ucomps = USER.a(33);
			gosub switchcompany();

			//record stats per database/sessionid/username/ip per hour
			var statistics;
			if (statistics.open("STATISTICS", "")) {
				var key = dataset;
				key ^= "*" ^ connection.a(1, 5);
				key ^= "*" ^ username;
				key ^= "*" ^ connection.a(1, 3);
				key ^= "*" ^ var().date();
				tt = (var().time() / 3600).floor() + 1;
				key ^= "*" ^ tt;
				if (not statistic.read(statistics, key)) {
					statistic = "";
				}
				//to avoid garbagecollect delay required to avoid MD conversion bug
				//by not doing 'MD50P'
				//statistic<1>=(date()+time()/86400) 'MD50P'
				statistic.r(1, (var().date() + var().time() / 86400).substr(1,12));
				statistic.r(2, statistic.a(2) + 1);
				//enable fast stats without cross database user access
				statistic.r(3, userrec.a(5));
				statistic.write(statistics, key);
			}

		}

	} else if (request1 == "BECOMEUSERANDCONNECTION") {
		gosub becomeuserandconnection( request2,  request4);

	} else if (request1 == "CONNECTION") {

	//username and password is already validated at this point
	//but we need to get users cookies for company, market, menus etc (or fail)
	} else if (request1 == "LOGIN") {

		var dataset = request2.ucase();
		var username = request3.ucase();

		USER1 = "";
		USER4 = "";
		var authcompcodes = "";

		//special login routine
		//returns iodat (cookie string "x=1&y=2" etc and optional comment msg)
		var voc;
		if (voc.open("VOC", "")) {
			if (xx.read(voc, "LOGIN.NET")) {
				var cookie = "";
				var loginmsg = "";
				//dont pass system variables
				call loginnet(dataset, username, cookie, loginmsg, authcompcodes);
				USER1 = cookie;
				USER4 = loginmsg;
				if (USER1 == "") {
					USER3 = USER4;
					return 0;
				}
			}
		}

		if (not USER1) {
			USER1 = "X=X";
		}
		USER3 = ("OK " ^ USER4).trim();

		//record the last login per user
		var users;
		if (users.open("USERS", "")) {

			//allow for NEOSYS not in users - all others should be but if not then create
			if (not userrec.read(users, username)) {
				userrec = "";
			}

			//convert old data
			if (userrec.a(13) ne userrec.a(15, 1) and userrec.a(18) == "") {
				if (userrec.a(15) and not userrec.a(18)) {
					userrec.r(18, "OK");
				}
				userrec.inserter(15, 1, userrec.a(13));
				userrec.inserter(16, 1, userrec.a(14));
				userrec.inserter(18, 1, "OK");
			}

			//save current login
			//datetime=(date():'.':time() 'R(0)#5')+0
			var datetime = var().date() ^ "." ^ (var().time()).oconv("R(0)#5");
			userrec.r(13, datetime);
			userrec.r(14, SYSTEM.a(40, 2));

			//save historical logins
			userrec.inserter(15, 1, userrec.a(13));
			userrec.inserter(16, 1, userrec.a(14));
			userrec.inserter(18, 1, "OK");

			//only keep the last 100 logins
			userrec.r(15, userrec.a(15).field(VM, 1, 100));
			userrec.r(16, userrec.a(16).field(VM, 1, 100));
			userrec.r(18, userrec.a(18).field(VM, 1, 100));

			if (authcompcodes) {
				userrec.r(33, authcompcodes);
			}

			//connection
			userrec.r(39, request4);

			userrec.write(users, username);

			//ensure this user gets control of the user record
			//to prevent "user is locked message" in case
			//another user is logged in on the same name or workstation failure
			if (openfile("LOCKS", locks)) {
				locks.deleterecord("USERS*" ^ username);
				
			}

		}

	} else if (request1 == "RESPOND") {

		//method to allow batch programs to respond and detach before finishing

		/////////////////////////////////////////////////////////////////////
		//VERY IMPORTANT TO AVOID LONG RUNNING BATCH PROCESSES FROM TIMING OUT
		/////////////////////////////////////////////////////////////////////
		SYSTEM.r(25, "");

		//determine the responsefile name from the printfilename
		//responsefilename=system<2>
		//if responsefilename else return
		//t=field2(responsefilename,'.',-1)
		//responsefilename[-len(t),len(t)]='3'

		//linkfilename3
		var responsefilename = PRIORITYINT.a(100);
		if (not responsefilename) {
			return 0;
		}

		//detach the calling process
		USER3 = request2;
		USER3.converter(VM ^ "|", FM ^ FM);
		USER3.swapper(FM, "\r\n");

		call oswrite(USER3, responsefilename);
		//osclose responsefilename

		call ossleep(1000*2);

		//indicate that response has been made
		SYSTEM.r(2, "");
/*
	} else if (request1.substr(1,14) == "GETINDEXVALUES") {

		USER1 = "";
		filename = request2;
		var fieldname = request3;
		var prefix = request4;
		var sortby = request5;
		if (sortby) {
			if (not(var("AL,AR,DL,DR").locateusing(sortby, ",", xx))) {
				USER3 = "Invalid sortby " ^ (DQ ^ (sortby ^ DQ)) ^ " in LISTEN,GETINDEXVALUES";
				return 0;
			}
		}
		var active = request6;

		if (filename == "BATCHES") {
		}else{
			gosub fileaccesscheck();
			if (USER3) {
				return 0;
			}
		}

		//security check
		var temp = filename;

		//zzz
		if (temp == "BATCHES") {
			temp = "JOURNAL";
		}

		if (not(authorised(singular(temp) ^ " LIST", USER4, ""))) {
			USER3 = USER4;
			return 0;
		}

getvalues:
		call collectixvals(filename, fieldname, prefix);
		PSEUDO.transfer(USER1);
		if (USER1[1] == FM) {
			USER1.splicer(1, 1, "");
		}
		USER3 = "OK";

		if (sortby and USER1) {
			USER1.converter(FM, RM);
			USER1 ^= RM;
			call v119("S", "", sortby[1], sortby[2], USER1, flag);
			USER1.converter(RM, FM);
			USER1.splicer(-1, 1, "");
		}

		var execstoplist;
		if (execstoplist.read(DEFINITIONS, "INDEXVALUES*" ^ filename ^ "*" ^ fieldname)) {

			//execs will be in field 1
			//stopped reason will be in parallel in field 2
			//stopped execs will be at the end
			USER1.converter(FM, VM);

			//remove or move any stopped execs to the end and add reason
			var nn = USER1.count(VM) + 1;
			var nn2 = nn;
			for (ii = 1; ii <= nn; ++ii) {
				var execcode = USER1.a(1, ii);
				if (execstoplist.a(1).locateusing(execcode, VM, stopn)) {
					var reason = execstoplist.a(2, stopn);
					if (reason) {
						USER1.eraser(1, ii);
						USER1.eraser(2, ii);
						if (not active) {
							USER1.r(1, nn2, execcode);
							USER1.r(2, nn2, reason);
						}
						ii -= 1;
						nn -= 1;
					}
				}
			};//ii;

			//show inactive if none are active
			if (USER1 == "" and active) {
				active = "";
				goto getvalues;
			}

			//1=force vm to ensure xml has empty not missing tags
			USER1.r(2, 1, USER1.a(2, 1));
			USER1 = invertarray(USER1, 1);

		}

		if (USER0.index("RECORD", 1)) {
			USER1 = invertarray(USER1, 1);

		} else if (USER0.index("XML", 1)) {
			if (USER1) {
				call htmllib2("STRIPTAGS", USER1);
				USER1.swapper(FM, "</" ^ fieldname ^ ">" "</record>" "\r\n" "<record><" ^ fieldname ^ ">");
				USER1.splicer(1, 0, "<record><" ^ fieldname ^ ">");
				USER1 ^= "</" ^ fieldname ^ ">" "</record>";
				if (USER1.index(VM, 1)) {
					USER1.swapper("</" ^ fieldname ^ ">", "</STOPPED>");
					USER1.swapper(VM, "</" ^ fieldname ^ ">" "<STOPPED>");
				}
			}
			USER1.splicer(1, 0, "<records>");
			USER1 ^= "</records>";
		} else {
			//convert fm to vm in iodat
		}
//L7246:
*/
	//also called from financeproxy to return voucher details for popup
	} else if (request1 == "SELECT") {

		var filename0 = request2;
		filename = filename0.field(" ", 1);
		var sortselect = request3;
		var dictids = request4;
		var options = request5;
		var maxnrecs = request6;

		if (not(sortselect.index("%SELECTLIST%", 1))) {
			var().clearselect();
		}

		var file;
		if (not(file.open(filename, ""))) {
			USER3 = DQ ^ (filename ^ DQ) ^ " file does not exist in LISTEN SELECT";
			return 0;
		}

		gosub fileaccesscheck();
		if (USER3) {
			return 0;
		}

		//get the current program stack
		//programstack=program.stack(nostack)

		//and data passed to SELECT is assumed to be a selectlist

		if (USER1) {
			var().makelist("",USER1);
			sortselect ^= "%SELECTLIST%";
			USER1 = "";
		}

		call select2(filename0, SYSTEM.a(2), sortselect, dictids, options, USER1, USER3, "", "", "", maxnrecs);
		//restore the program stack although this is done in LISTEN per request
		//arev has a limit on 299 "programs" and dictionary entries count as 1 each!
		//call program.stack(programstack)

		if (USER4) {
			USER3 = trim2(USER4.a(1), FM);
		}else{
			USER1 = "%DIRECTOUTPUT%";
			//response='OK'
		}

	} else if (request1 == "LISTLOCKS") {

		gosub getdostime();

		/*;

			msg='';

			filename='LOCKS';
			//sortselect='BY DATETIME_EXPIRES'
			sortselect='BY USER';
			sortselect:=' WITH DATETIME_EXPIRES >= ':dostime;
			dictids='USER STATION FILENAME KEY';
			options='';
			msg='';

			call select2(filename,'',sortselect,dictids,options,select2data,select2response,'','','');

			//abort to net program which will clear the lists file
			if msg then stop;

		*/

		if (not(openfile("LOCKS", locks))) {
			call mssg("LISTEN2 CANNOT OPEN LOCKS FILE");
			return 0;
		}

		locks.select();
		var select2data = "";
		var nlocks = 0;
nextlock:
		if (readnext(lockid)) {
			var lockx;
			if (lockx.read(locks, lockid)) {
				if (lockx.a(1) < dostime) {
					goto nextlock;
				}
				nlocks += 1;
				select2data.r(nlocks, 1, lockx.a(4) ^ VM ^ lockx.a(3) ^ VM ^ lockid.field("*", 1) ^ VM ^ lockid.field("*", 2, 999));
				goto nextlock;
			}
		}

		if (tracing) {
			printl();
		}else{
			print(var().at(0, 1));
		}
		var nn = select2data.count(FM);
		var maxlines = 20;
		if (not tracing) {
			nn = maxlines;
			if (nn > maxlines) {
				nn = maxlines;
			}
			printl("+------Active-------+");
		}
		if (tracing) {
			printl(var("User").oconv("L#19"), " ", var("Station").oconv("L#19"), " ", var("File").oconv("L#19"), " ", var("Record").oconv("L#19"));
		}
		for (ii = 1; ii <= nn; ++ii) {
		///BREAK;
		if (tracing and nn > 20) break;;
			var row = select2data.a(ii);
			if (not tracing) {
				print("|");
			}
			tt = row.a(1, 1).trim();
			if (not tracing) {
				tt ^= " " ^ row.a(1, 3);
			}
			print(tt.oconv("L#19"));
			if (not tracing) {
				printl("|");
			}
			if (tracing) {
				printl(" ", (row.a(1, 2)).oconv("L#19"), " ", (row.a(1, 3)).oconv("L#19"), " ", (row.a(1, 4)).oconv("L#19"));
			}
		};//ii;

		if (not tracing) {
			printl("+-------------------+");
		}

		select2data = "";
		var select2response = "";

	} else if (request1 == "UNLOCKLONGPROCESS") {

		//IF revRELEASE()>= 2.1 THEN
		// lockmode=36
		// unlockmode=37
		//END ELSE
		var lockmode = 23;
		var unlockmode = 24;
		// END

		//NB ZZZ will hang if is not locked eg via unlock all
		//unlock long process unlimited cpu time flag
		tt = SYSTEM.a(48);
		if (tt) {
			//unlock in request loop using lockkey stored in system<48> by giveway
			//lock first to avoid hanging if try to unlock when not locked
			call rtp57(lockmode, "", xx, tt, "", yy, zz);
			call rtp57(unlockmode, "", xx, tt, "", yy, zz);
			SYSTEM.r(48, "");
		}

	} else if (request1 == "STOPDB") {

		//check authorised
		tt = request2;
		if (not tt) {
			tt = "STOP";
		}
		if (not(authorised("DATABASE " ^ tt, USER4, "LS"))) {
			USER3 = "Error: " ^ USER4;
			return 0;
		}

		if (request3.osfile() or request4.osfile()) {
			//response='Error: Database already stopped/stopping'
			call listen4(19, USER3);

		}else{

			call oswrite("", request3);

			//stop server
			if (request2.index("ALL", 1)) {
				call oswrite("", request4);
			}

			//wait up to 30 seconds for other users to quit
			var timex = var().time();
			while (true) {
			///BREAK;
			if (not(otherusers().a(1) and (var().time() - timex).abs() < 30)) break;;
				call ossleep(1000*1);
			}//loop;

			USER1 = "";

			var otherusersx = otherusers();
			if (otherusersx) {
				//response='Error: Could not terminate ':otherusersx<1>:' processes|':otherusersx<2>
				call listen4(20, USER3, otherusersx);
				request3.osdelete();
			}else{
				osshell("NET STOP NEOSYSSERVICE");

				if (request2.substr(1,7) == "RESTART") {
					request3.osdelete();
					osshell("NET START NEOSYSSERVICE");
				}

				USER3 = "OK";
			}

			if (request2.index("ALL", 1)) {
				request4.osdelete();
			}
		}

	} else if (request1 == "BACKUP") {

		//similar code in LISTEN and LISTEN2

		//gosub getbakpars
		call readbakpars(bakpars);

		//backup may respond to user itself if it starts
		USER4 = "";
		perform("FILEMAN BACKUP " ^ SYSTEM.a(17) ^ " " ^ bakpars.a(7));

		//if backup has already responded to user
		//then quit and indicate to calling program that a backup has been done
		//user will be emailed
		if (SYSTEM.a(2) == "") {
			PSEUDO = "BACKUP2 " ^ bakpars.a(7);
			if (USER4) {
				var().stop();
			}
		}

		USER3 = USER4;
		USER3.converter(FM ^ VM, "\r\r");

		call sysmsg(USER3, "NEOSYS Backup");

		if ((USER3.ucase()).index("SUCCESS", 1)) {
			USER3.splicer(1, 0, "OK ");
		}

		//note: if backup did not respond already then the requestexit will
		//respond as usual with the error message from backup
		USER1 = "";

	} else {
		printl(DQ ^ (request1 ^ DQ), " invalid request in LISTEN2");
	}
//L8482:
	return 0;

}

subroutine becomeuserandconnection(in request2, in request4) {
	//becomeuserandconnection(in request2, in request4)
	var username = request2;
	var connection = request4;

	//set @username
	USERNAME=(username);

	//set @station
	tt = connection.a(1, 2);
	if (connection.a(1, 3) ne tt) {
		tt ^= "_" ^ connection.a(1, 3);
	}
	tt.converter(". ", "_");
	//call sysvar('SET',109,110,t)
	//call sysvar_109_110('SET',t)
	STATION=(tt);

	//restore default style
	SYSTEM.r(46, SYSTEM.a(47));

	//in LISTEN2 and INIT.COMPANY
	var companystyle = gen.company.a(70);
	if (companystyle) {
		SYSTEM.r(46, companystyle);
	}

	//get user style
	userrec = "";
	var users;
	if (users.open("USERS", "")) {
		if (userrec.read(users, USERNAME)) {

			ucomps = userrec.a(33);
			gosub switchcompany();

			var styles = userrec.a(19);
			styles.swapper("Default", "");
			if (not styles2.readv(users, userrec.a(21), 19)) {
				styles2 = "";
			}

			//save time on every request by ignoring this authorisation
			//chkauth=index(userprivs,'USER UPDATE "',1)
			var chkauth = 0;
			if (chkauth) {
				fields = "";
				fields.r(1, "REPORT HEAD COLOR");
				fields.r(2, "REPORT BODY COLOR");
				fields.r(3, "REPORT FONT NAME");
				fields.r(8, "REPORT FONT SIZE");
			}

			var runstyles = SYSTEM.a(46);
			for (var vn = 1; vn <= 9; ++vn) {
				var tt2 = styles.a(1, vn);

				//skip user styles if not authorised
				if (chkauth) {
					var fieldname = fields.a(vn);
					if (fieldname) {
						for (ii = 1; ii <= 3; ++ii) {
							//USER UPDATE "REPORT"
							//USER UPDATE "REPORT HEAD"
							//USER UPDATE "REPORT HEAD COLOR"
							//etc
							if (not(authorised("USER UPDATE " ^ (DQ ^ (fieldname.field(" ", ii) ^ DQ)), xx))) {
								tt2 = "";
							}
						};//ii;
					}
				}

				//departmental default
				if (not tt2.length()) {
					tt2 = styles2.a(1, vn);
				}

				//dont copy defaults so the system default will apply
				if (tt2 == "") {
				} else if (tt2 == "Default") {
				} else if (tt2 == "100") {
				} else {
					runstyles.r(1, vn, tt2);
				}
			};//vn;
			SYSTEM.r(46, runstyles);
			//system<10,1>=userrec<1>
			//system<10,2>=userrec<7>

		}

	}
	return;

}

subroutine getdostime() {
	dostime = ostime();
	//convert to Windows based date/time (ndays since 1/1/1900)
	//31/12/67 in rev date() format equals 24837 in windows date format
	dostime = 24837 + var().date() + dostime / 24 / 3600;
	return;

}

subroutine fileaccesscheck() {
	USER3 = "";

	var securityfilename = filename;
	if (filename == "BATCHES") {
		securityfilename = "JOURNALS";
	}

	//security check
	//dont check markets and companies because really must have access to some
	//and eliminates need for clumsy task COMPANY ACCESS PARTIAL task
	if (not(var("MENUS,ADMENUS").locateusing(filename, ",", xx))) {
		var temp = securityfilename;
		temp.converter(".", " ");
		temp = singular(temp);
		if (not(authorised(temp ^ " ACCESS", USER4, ""))) {
			//we could use securityfilename LIST instead of securityfilename ACCESS PARTIAL clumsy
			//ie list without general access means there are some records
			//specifically allowed
			if (not(authorised("!#" ^ temp ^ " ACCESS PARTIAL", msg2, ""))) {
				//if there is an authorised dictionary item then leave it up to that
				if (not((var("AUTHORISED").xlate("DICT." ^ filename, 8, "X")).index("ALLOWPARTIALACCESS", 1))) {
					USER3 = USER4;
					return;
				}
			}
		}
		USER4 = "";
	}

	return;

}

subroutine addwhoistx() {
	if (whoistx) {
		body.r(-1, FM ^ "\"whois\" ip number " ^ ipno ^ " information:");
		body.r(-1, FM ^ whoistx);
		body.r(-1, FM ^ "end of " "\"whois\" ip number " ^ ipno ^ " information:");
	}
	return;

}

subroutine switchcompany() {
	if (not ucomps.locateusing(gen.gcurrcompany, VM, xx)) {
		tt = ucomps.a(1, 1);
		if (xx.read(gen.companies, tt)) {
			call initcompany(tt);
		}
	}
	return;

}

subroutine printbottomline() {
	if (CRTHIGH > 24) {
		yy = CRTHIGH - 1;
	}else{
		yy = CRTHIGH;
	}
	//CALL SCRN.IO(0,yy,bottomline[1,80],ESC.TO.ATTR(@ENVIRON.SET<21>))
	return;

}

subroutine deleteoldfiles() {

	var deletetime = var().date() * 24 * 60 * 60 + var().time() - ageinsecs;

	var filespec = (inpath ^ pattern).ucase();

	//failsafe - only allow delete .* in data folder
	if (filespec.substr(-2,2) == ".*") {
		if (not filespec.index("\\D" "ATA\\", 1)) {
			return;
		}
	}

	//for each suitable file
	//initdir filespec
	//filenamesx=dirlist()
	var filenamesx = oslistf(filespec);

	while (true) {
	///BREAK;
	if (not filenamesx) break;;

		//get the file time
		var filename0 = filenamesx.field(FM, 1);
		filenamesx.splicer(1, (filenamesx.field(FM, 1)).length() + 1, "");
		if (not filenamesx) {
			filenamesx = var().oslistf();
		}

		filename = inpath ^ filename0;

		//replaced by databasecode.SVR
		if (filename0 == "GLOBAL.SVR") {
			goto deleteit;
		}

		if (not(var(".JPG,.PNG,.GIF,.SVR").locateusing(filename.substr(-4,4), ",", xx))) {

			//a file ending .4 is a request to delete the .2 and .3 files
			if (filename.substr(-2,2) == ".4") {
				filename.osdelete();
				filename.splicer(-1, 1, "2");
				filename.osdelete();
				filename.splicer(-1, 1, "3");
				filename.osdelete();

			}else{
				if (filename.substr(-4,4) == ".TMP") {
					goto deleteit;
				}
				//and delete it if older than the cut off time
				//and has a file extension (ie leave PARAMS and PARAMS2)
				fileattributes = filename.osfile();
				filetime = fileattributes.a(2) * 24 * 60 * 60 + fileattributes.a(3);
				if ((filename.substr(-4,4)).index(".", 1) and filetime <= deletetime) {
deleteit:
					filename.osdelete();
				}else{
				}
			}

		}

	}//loop;

	return;

}


libraryexit()
