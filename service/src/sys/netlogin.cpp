#include <exodus/library.h>
libraryinit()

#include <secid.h>
#include <hashpass.h>
#include <systemfile.h>
#include <inputbox.h>

#include <sys_common.h>

var mode;
var userx;
var password;
var ok;//num
var usern;
var xdata;
var msg;
var show;//num
var maxlen;//num
var now;//num
var escx;
var now2;//num
var encryptx;
var allowablechars;

function main(in mode0) {
	//c sys
	#include <system_common.h>
	//global encryptx,password,maxlen,show,ok,msg,userx

	#define prikey_ "1000" ^ "100"

	if (mode0.unassigned()) {
		mode = "";
	} else {
		mode = mode0;
	}

	var().osflush();
	//break off

	//prevent time out
	var timeoutx = SYSTEM.a(22);
	SYSTEM(22) = "";

	var nfailures = "";
	var nallowable = 3;
	var lockx = "";
	var speed = 0;

	//autologin
	if (SYSTEM.a(33)) {
		//break on
		userx = SYSTEM.a(33, 2).ucase();
		password = SYSTEM.a(33, 3).ucase();
		goto chknameandpass;
	}

	gosub getsec();

		/*dos and not supported any longer;
		//auto login for particular station ids
		if addresses='' then;
			if len(@station) then;
				locate @station in userprivs<5> setting stationn then;
					userx=userprivs<1,stationn>;
					ok=1;
					goto okfail;
					end;
				end;
			end;
		*/

	//if there is a user called MASTER with no password then login automatically
	// unless already automatically logged in
	ok = 0;
	if (not(SYSTEM.a(15))) {
		if (SECURITY.a(1).locate("MASTER", usern)) {
			if (SECURITY.a(4, usern, 2).field(TM, 7) eq "") {
				SYSTEM(22) = 1000000;
				userx = "MASTER";
				SYSTEM(15) = 1;
				ok = 1;
				goto okfail;
			}
		}
	}

	//sleep
	//if mode='SLEEP' then
	// call clearscreen(buffer,attribute)
	// call mssg('This computer workstation has gone to sleep,|please enter your password to wake it up,| or press Esc to exit EXODUS.','UB':fm:fm:fm:1,buff,'')
	// end

	//get the user name
inpname:
	if (mode eq "SLEEP") {
		xdata = USERNAME;
	} else {

		//unlock all
		var xx = unlockrecord();

		//prevent any messages to the old name
		USERNAME=(APPLICATION);
		//unfortunately this is removing the workstation lock on messages
		//so we cannot see the dataset being in use
		//unlock all
		xx = unlockrecord();

		msg = "EXODUS SECURITY|What is your name ?| || |Please enter your name,|or press Esc to exit.";
		show = 1;
		maxlen = 20;
		gosub inputx();
	}

	//exit the system if name not given
	if (xdata eq "") {
fail:
	//  if @username='EXODUS' then stop
		perform("OFF");
		var().logoff();
	}

	userx = xdata;

	gosub getsec();

	//check the user name and password
	ok = 0;
	if (SECURITY.a(1).locate(userx, usern)) {
		if (SECURITY.a(4, usern, 2).field(TM, 7) eq "") {
			ok = 1;
			goto okfail;
		}
	}

	//get the password
	msg = "EXODUS SECURITY|What is your PASSWORD ?| || |Please enter your password,|or press Esc to cancel.";
	show = 0;
	maxlen = 20;
	now = ostime();
	gosub inputx();

	if (escx eq "0") {
		//magic method entry of password (pressing enter on blank then entering pass)
		//defeats the password entry speed check
		speed = 0;
	} else {
		now2 = ostime();
		speed = now2 - now;
	}
	//call note(speed)

	if (xdata eq "") {
		if (mode ne "SLEEP") {
			goto inpname;
		}
		//if decide2('!WARNING: Unsaved work (if any) will|be lost if you continue.||','OK|Cancel',reply,2) else reply=2
		var reply = 2;
		if (reply ne 1) {
			goto inpname;
		}
		goto fail;
	}
	password = xdata;

	//long distance override
	if (xdata eq "?") {

		if (not(userx eq lockx.substr(1, userx.length()))) {
			lockx = userx ^ " " ^ var(1000000).rnd();
			if (SECURITY.a(1).locate(userx, usern)) {
				lockx ^= " " ^ SECURITY.a(4, usern, 2).field(TM, 7).oconv("HEX2");
			}
		}

		var keyfail = 0;
	//inp.key:
		var keyx = "";
		call mssg("The lock is " ^ (lockx.quote()) ^ "|What is the key ?", "RC", keyx, "");

		ok = keyx eq secid(lockx, "1000" "100");
		if (ok) {
			goto okfail;
		}

		//allowed in but no access to authorisation screen
		ok = keyx eq secid(lockx, 132456);
		if (ok) {
			SYSTEM(21) = "1";
			goto okfail;
		}

		goto okfail;

		//print char(7):
		//msg='That is not the correct key'
		//call msg(msg)
		//if keyfail ge 3 then goto fail
		//keyfail+=1
		//goto inp.key

	}

chknameandpass:
///////////////
	//encrypt the password
	//encryptx=password
	//gosub makepass
	encryptx = hashpass(password);

	//check the user name and password
	ok = 0;
	if (SECURITY.a(1).locate(userx, usern)) {
		if (SECURITY.a(4, usern, 2).field(TM, 7) eq encryptx) {
			ok = 1;
		} else {
		}
	}
	//failed login not interactive still works
	if (not(ok)) {
		if (SYSTEM.a(33)) {
			userx = APPLICATION;
			ok = 1;
		}
	}

	//check revelation system file (must belong to this account)
	if (not ok) {
		var sysrec;
		if (sysrec.read(systemfile(), userx)) {
			if (sysrec.a(7) eq encryptx and ((speed lt 2 or mode eq "SLEEP"))) {
				ok = 1;
			}
		}
	}

okfail:
	//handle failure
	if (not ok) {

		if (SYSTEM.a(33)) {
			msg = "INVALID USERNAME OR PASSWORD " ^ (userx.quote());
			call oswrite(msg, SYSTEM.a(33, 10) ^ ".$2");
			//print char(12):char(7):msg
			printl(msg);
			var().logoff();
		}

		nfailures += 1;
		//print char(7):char(7):char(7):
		msg = "EXODUS SECURITY|";

		msg(-1) = "Either the name or the password";
		msg(-1) = "that you entered is NOT CORRECT.";
		msg(-1) = "(maybe you made a typing error)|";

		if (nfailures lt nallowable) {
			msg(-1) = "PLEASE TRY AGAIN.|";
		}
		call mssg(msg);

		if (nfailures lt nallowable) {
			goto inpname;
		}
		goto fail;
	}

	USERNAME=(userx);

	var sysrec;
	if (not(sysrec.read(systemfile(), userx))) {
		sysrec = "";
	}
	if (USERNAME eq "EXODUS") {
		sysrec(4) = 0;
	}
	//call setprivilegesysrec.a(4));

	//break on
	SYSTEM(22) = timeoutx;

	//if mode='SLEEP' then
	// call mssg('','DB',buff,'')
	// //call restorescreenbuffer,attribute)
	// end
	return 0;
}

subroutine inputx() {
	call inputbox(msg, maxlen, show, allowablechars, xdata, escx);
	return;
	/*;
	/////////
	makepass:
	/////////
		encryptkey = 1234567;

		//pass1
		loop;
			while encryptx # '';
			encryptkey = mod(encryptkey, 390001) * seq(encryptx[1, 1]) + 1;
			encryptx[1, 1]='';
			repeat;

		//pass2
		loop;
			encryptx := char(65 + mod(encryptkey, 50));
			encryptkey = int(encryptkey / 50);
		while encryptkey repeat;

		return;
	*/
}

subroutine getsec() {
	if (not(SECURITY.read(DEFINITIONS, "SECURITY"))) {
		SECURITY = "";
	}
	if (VOLUMES) {
		SECURITY = SECURITY.invert();
	}
	return;
}

libraryexit()
