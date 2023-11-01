#include <exodus/library.h>

libraryinit(email_address)
//------------------------
function main() {
	// @ans=xlate('USERS',@record<1>,7,'X')
	return RECORD.f(1).xlate("USERS", 7, "X");
}
libraryexit(email_address)


libraryinit(full_name)
//--------------------
function main() {
	ANS = RECORD.f(1).xlate("USERS", 1, "X");
	let nn = ANS.fcount(VM);
	for (const var ii : range(1, nn)) {
		if (ANS.f(1, ii) == RECORD.f(1, ii)) {
			ANS(1, ii) = "";
		}
	} // ii;
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
		let logindate = ANS.f(1, ii).floor();
		if (logindate) {
			ANS(1, ii) = (date() - logindate).oconv("MD00P");
		}
	} // ii;
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
		let passworddate = ANS.f(1, ii);
		if (passworddate) {
			ANS(1, ii) = (date() - passworddate).oconv("MD00P");
		}
	} // ii;
	return ANS;
}
libraryexit(password_age)


libraryinit(process_comment2)
//---------------------------
#include <srv_common.h>

function main() {
	ANS = "";
	if (ID.field("*", 2) == SYSTEM.f(32)) {
		ANS(1, -1) = "Current user session";
	}
	if (RECORD.f(3).trim() == STATION.trim()) {
		ANS(1, -1) = "Current workstation";
	}
	if (RECORD.f(5) == USERNAME) {
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
	let secsperday = 24 * 60 * 60;
	var uptodate = RECORD.f(11);
	var uptotime = RECORD.f(12);
	if (uptodate == "") {
		uptodate = date();
	}
	if (uptotime == "") {
		uptotime = time();
	}
	let daysecs = (uptodate - RECORD.f(1)) * secsperday;
	let timesecs = uptotime - RECORD.f(2);
	let totsecs = daysecs + timesecs;

	let hours = (totsecs / 60 / 60).floor();
	let mins = ((totsecs / 60).floor()).mod(60);
	let secs = totsecs.mod(60).floor();
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
	let emails = usercodes.xlate("USERS", 7, "X");
	let nusers = usercodes.fcount(VM);
	for (const var usern : range(1, nusers)) {
		let usercode = usercodes.f(1, usern);
		if (usercode) {
			if (emails.f(1, usern) == "") {
				usercodes(1, usern) = "<B>" ^ usercode ^ "</B>";
			}
		}
	} // usern;
	return usercodes;
}
libraryexit(user_code_html)


libraryinit(user_code_with_email)
//-------------------------------
function main() {
	var usercodes = RECORD.f(1, MV);
	let emails = RECORD.f(7, MV);

	let nusers = usercodes.fcount(VM);
	for (const var usern : range(1, nusers)) {
		if (emails.f(1, usern) == "") {
			usercodes(1, usern) = "";
		}
	} // usern;
	return usercodes;
}
libraryexit(user_code_with_email)
