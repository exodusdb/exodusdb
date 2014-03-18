#include <exodus/library.h>
libraryinit()

#include <clearscreen.h>
#include <setuser.h>
#include <decide2.h>
#include <secid.h>
#include <systemfile.h>
#include <setprivilege.h>
#include <restorescreen.h>
#include <inputbox.h>

#include <gen.h>

var usern;
var buffer;
var attribute;
var buff;
var xdata;
var now;//num
var escx;
var now2;//num
var reply;//num
var allowablechars;

function main(in mode) {
	//notjbase

	if (mode.unassigned()) {
		mode = "";
	}

	var().osflush();
	BREAK OFF;

	//prevent time out
	var timeoutx = SYSTEM.a(22);
	SYSTEM.r(22, "");

	var nfailures = "";
	var nallowable = 3;
	var lockx = "";
	var speed = 0;

	//autologin
	if (SYSTEM.a(33)) {
		BREAK ON;
		var USER = SYSTEM.a(33.ucase(2));
		var password = SYSTEM.a(33.ucase(3));
		goto chknameandpass;
	}

	gosub getsecurity();

		/*dos and not supported any longer;
		//auto login for particular station ids
		if addresses='' then;
			if len(@station) then;
				locate @station in userprivs<5> setting stationn then;
					user=userprivs<1,stationn>;
					ok=1;
					goto okfail;
					end;
				end;
			end;
		*/

	//if there is a user called MASTER with no password then login automatically
	// unless already automatically logged in
	var ok = 0;
	if (not SYSTEM.a(15)) {
		if (gen._security.a(1).locateusing("MASTER", VM, usern)) {
			if (gen._security.a(4, usern, 2).field(TM, 7) == "") {
				SYSTEM.r(22, 1000000);
				var USER = "MASTER";
				SYSTEM.r(15, 1);
				ok = 1;
				goto okfail;
			}
		}
	}

	//sleep
	if (mode == "SLEEP") {
		call clearscreen(buffer, attribute);
		call msg2("This computer workstation has gone to sleep,|please enter your password to wake it up,| or press Esc to exit NEOSYS.", "UB" ^ FM ^ FM ^ FM ^ 1, buff, "");
	}

	//get the user name
inpname:
	if (mode == "SLEEP") {
		xdata = USERNAME;
	}else{

		//unlock all
		var xx = unlockrecord("", "", "");

		//prevent any messages to the old name
		call setuser(ACCOUNT);
		//unfortunately this is removing the workstation lock on messages
		//so we cannot see the dataset being in use
		//unlock all
		xx = unlockrecord("", "", "");

		var msg = "NEOSYS SECURITY|What is your name ?| || |Please enter your name,|or press Esc to exit.";
		var show = 1;
		var maxlen = 20;
		gosub input();
	}

	//exit the system if name not given
	if (xdata == "") {
fail:
	// if @username='NEOSYS' then stop
		perform("OFF");
	}

	var USER = xdata;

	gosub getsecurity();

	//check the user name and password
	ok = 0;
	if (gen._security.a(1).locateusing(USER, VM, usern)) {
		if (gen._security.a(4, usern, 2).field(TM, 7) == "") {
			ok = 1;
			goto okfail;
		}
	}

	//get the password
	var msg = "NEOSYS SECURITY|What is your PASSWORD ?| || |Please enter your password,|or press Esc to cancel.";
	var show = 0;
	var maxlen = 20;
	call dostime(now);
	gosub input();

	if (0 and escx == "0") {
		//magic method entry of password (pressing enter on blank then entering pass)
		//defeats the password entry speed check
		speed = 0;
	}else{
		call dostime(now2);
		speed = now2 - now;
	}
	//call note(speed)

	if (xdata == "") {
		if (mode ne "SLEEP") {
			goto inpname;
		}
		if (not(decide2("!WARNING: Unsaved work (if any) will|be lost if you continue.||", "OK" _VM_ "Cancel", reply, 2))) {
			reply = 2;
		}
		if (reply ne 1) {
			goto inpname;
		}
		goto fail;
	}
	var password = xdata;

	//long distance override
	if (xdata == "?") {

		if (not(USER == lockx.substr(1, USER.length()))) {
			lockx = USER ^ " " ^ var(1000000).rnd();
			if (gen._security.a(1).locateusing(USER, VM, usern)) {
				lockx ^= " " ^ (gen._security.a(4, usern, 2).field(TM, 7)).oconv("HEX");
			}
		}

		var keyfail = 0;
inpkey:
		var keyx = "";
		call msg2("The lock is " ^ (DQ ^ (lockx ^ DQ)) ^ "|What is the key ?", "RC", keyx, "");

		ok = keyx == secid(lockx, "1000" "100");
		if (ok) {
			goto okfail;
		}

		//allowed in but no access to authorisation screen
		ok = keyx == secid(lockx, 132456);
		if (ok) {
			SYSTEM.r(21, "1");
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
	var encryptx = password;
	gosub makepass();

	//check the user name and password
	ok = 0;
	if (gen._security.a(1).locateusing(USER, VM, usern)) {
		if (gen._security.a(4, usern, 2).field(TM, 7) == encryptx) {
			ok = 1;
		}else{
		}
	}
	//failed login not interactive still works
	if (not ok) {
		if (SYSTEM.a(33)) {
			USER = ACCOUNT;
			ok = 1;
		}
	}

	//check revelation system file (must belong to this account)
	if (not ok) {
		var sysrec;
		if (sysrec.read(systemfile(), USER)) {
			//if sysrec<2>=@account then
			if (sysrec.a(7) == encryptx and (speed < 1.5 or mode == "SLEEP")) {
				ok = 1;
			}
			// end
		}
	}

okfail:
	//handle failure
	if (not ok) {

		if (SYSTEM.a(33)) {
			msg = "INVALID USERNAME OR PASSWORD " ^ (DQ ^ (USER ^ DQ));
			call oswrite(msg, SYSTEM.a(33, 10) ^ ".$2");
			cout << var().chr(12);
			var().chr(7).output();
			cout << msg << endl;
			var().logoff();
		}

		nfailures += 1;
		var().chr(7).output();
		var().chr(7).output();
		var().chr(7).output();
		msg = "NEOSYS SECURITY|";

		msg.r(-1, "Either the name or the password");
		msg.r(-1, "that you entered is NOT CORRECT.");
		msg.r(-1, "(maybe you made a typing error)|");

		if (nfailures < nallowable) {
			msg.r(-1, "PLEASE TRY AGAIN.|");
		}
		call mssg(msg);

		if (nfailures < nallowable) {
			goto inpname;
		}
		goto fail;
	}

	call setuser(USER);

	var sysrec;
	if (not(sysrec.read(systemfile(), USER))) {
		sysrec = "";
	}
	if (USERNAME == "NEOSYS") {
		sysrec.r(4, 0);
	}
	call setprivilege(sysrec.a(4));

	BREAK ON;
	SYSTEM.r(22, timeoutx);

	if (mode == "SLEEP") {
		call msg2("", "DB", buff, "");
		call restorescreen(buffer, attribute);
	}
	return 0;

}

subroutine input() {
	call inputbox(msg, maxlen, show, allowablechars, xdata, escx);
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

subroutine getsecurity() {
	if (not(gen._security.read(gen._definitions, "SECURITY"))) {
		gen._security = "";
	}
	gen._security = gen._security.invert();
	return;

}


libraryexit()