#include <exodus/library.h>

libraryinit(chequedesign_account_name)
//------------------------------------
function main() {
	// @ans=xlate('ACCOUNTS',field(@id,'*',2),1,'X')
	return ID.field("*", 2).xlate("ACCOUNTS", 1, "X");
}
libraryexit(chequedesign_account_name)


libraryinit(chequedesign_is_default)
//----------------------------------
function main() {
	ANS = RECORD.f(14);
	if (ANS) {
		ANS = "Default";
	}
	return ANS;
}
libraryexit(chequedesign_is_default)


libraryinit(email_address)
//------------------------
function main() {
	//@ans=xlate('USERS',@record<1>,7,'X')
	return RECORD.f(1).xlate("USERS", 7, "X");
}
libraryexit(email_address)


libraryinit(full_name)
//--------------------
function main() {
	ANS = RECORD.f(1).xlate("USERS", 1, "X");
	let nn = ANS.fcount(VM);
	for (const var ii : range(1, nn)) {
		if (ANS.f(1, ii) eq RECORD.f(1, ii)) {
			ANS(1, ii) = "";
		}
	} //ii;
	return ANS;
}
libraryexit(full_name)


libraryinit(key1)
//---------------
function main() {
	return ID.field("*", 1);
}
libraryexit(key1)


libraryinit(key2)
//---------------
function main() {
	return ID.field("*", 2);
}
libraryexit(key2)


libraryinit(last_browser)
//-----------------------
function main() {
	return xlate("USERS", calculate("USER_CODE_WITH_EMAIL"), "LAST_BROWSER", "X");
}
libraryexit(last_browser)


libraryinit(last_login_age)
//-------------------------
function main() {
	ANS = calculate("USER_CODE_WITH_EMAIL").xlate("USERS", 13, "X");
	let nn = ANS.fcount(VM);
	for (const var ii : range(1, nn)) {
		var logindate = ANS.f(1, ii).floor();
		if (logindate) {
			ANS(1, ii) = (date() - logindate).oconv("MD00P");
		}
	} //ii;
	return ANS;
}
libraryexit(last_login_age)


libraryinit(last_login_date_time)
//-------------------------------
function main() {
	return calculate("USER_CODE_WITH_EMAIL").xlate("USERS", 13, "X");
}
libraryexit(last_login_date_time)


libraryinit(last_login_location)
//------------------------------
function main() {
	return calculate("USER_CODE_WITH_EMAIL").xlate("USERS", 14, "X");
}
libraryexit(last_login_location)


libraryinit(last_os)
//------------------
function main() {
	return xlate("USERS", calculate("USER_CODE_WITH_EMAIL"), "LAST_OS", "X");
}
libraryexit(last_os)


libraryinit(password_age)
//-----------------------
function main() {
	ANS = calculate("USER_CODE_WITH_EMAIL").xlate("USERS", 36, "X");
	let nn = ANS.fcount(VM);
	for (const var ii : range(1, nn)) {
		var passworddate = ANS.f(1, ii);
		if (passworddate) {
			ANS(1, ii) = (date() - passworddate).oconv("MD00P");
		}
	} //ii;
	return ANS;
}
libraryexit(password_age)


libraryinit(process_comment2)
//---------------------------
#include <sys_common.h>

function main() {
	ANS = "";
	if (ID.field("*", 2) eq SYSTEM.f(32)) {
		ANS(1, -1) = "Current user session";
	}
	if (RECORD.f(3).trim() eq STATION.trim()) {
		ANS(1, -1) = "Current workstation";
	}
	if (RECORD.f(5) eq USERNAME) {
		ANS(1, -1) = "Current user";
		}
	return ANS;
}
libraryexit(process_comment2)


libraryinit(process_count)
//------------------------
function main() {
	return 1;
}
libraryexit(process_count)


libraryinit(process_duration)
//---------------------------
function main() {
	var secsperday = 24 * 60 * 60;
	var uptodate = RECORD.f(11);
	var uptotime = RECORD.f(12);
	if (uptodate eq "") {
		uptodate = date();
	}
	if (uptotime eq "") {
		uptotime = time();
	}
	var daysecs = (uptodate - RECORD.f(1)) * secsperday;
	var timesecs = uptotime - RECORD.f(2);
	var totsecs = daysecs + timesecs;

	var hours = (totsecs / 60 / 60).floor();
	var mins = ((totsecs / 60).floor()).mod(60);
	var secs = totsecs.mod(60).floor();
	return hours ^ ":" ^ mins.oconv("R(0)#2") ^ ":" ^ secs.oconv("R(0)#2") ^ "." ^ totsecs.field(".", 2).oconv("R#2");
}
libraryexit(process_duration)


libraryinit(process_weekcommencing)
//---------------------------------
function main() {
	return RECORD.f(1) - ((RECORD.f(1) - 1).mod(7));
}
libraryexit(process_weekcommencing)


libraryinit(reverse_the_users)
//----------------------------

function main() {
	RECORD = invertarray(reverse(invertarray(RECORD)));
	return ANS;
}
libraryexit(reverse_the_users)


libraryinit(user_code_html)
//-------------------------
function main() {
	var usercodes = calculate("USERS");
	var emails = usercodes.xlate("USERS", 7, "X");
	let nusers = usercodes.fcount(VM);
	for (const var usern : range(1, nusers)) {
		var usercode = usercodes.f(1, usern);
		if (usercode) {
			if (emails.f(1, usern) eq "") {
				usercodes(1, usern) = "<B>" ^ usercode ^ "</B>";
			}
		}
	} //usern;
	return usercodes;
}
libraryexit(user_code_html)


libraryinit(user_code_with_email)
//-------------------------------
function main() {
	var usercodes = RECORD.f(1, MV);
	var emails = RECORD.f(7, MV);

	let nusers = usercodes.fcount(VM);
	for (const var usern : range(1, nusers)) {
		if (emails.f(1, usern) eq "") {
			usercodes(1, usern) = "";
		}
	} //usern;
	return usercodes;
}
libraryexit(user_code_with_email)
