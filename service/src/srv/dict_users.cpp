#include <exodus/library.h>

libraryinit(authorised)
//---------------------
#include <authorised.h>

#include <srv_common.h>

var usern;
var ans; //num
var userx;

function main() {
	#include <service_common.h>
	if (ID == "EXODUS") {
		goto unknown;
	}
	if (SECURITY.f(1).locate(USERNAME, usern)) {
		if (authorised("TIMESHEET ADMINISTRATOR")) {
			ans = 1;
			return ans;
		}
	// may not be allowed to access higher users/groups
		if (usern > RECORD.f(11)) {
			ans = authorised("AUTHORISATION UPDATE HIGHER GROUPS");
		} else {

		// look for the user in the same group as the active user
			for (const var usern2 : range(usern, 9999)) {
				userx = SECURITY.f(1, usern2);
				// /BREAK;
				if (not((userx and userx != "---") and userx != ID))
					break;
			} // usern2;

		// if found then same group therefore ok
			if (userx == ID) {
				ans = 1;
			} else {

			// not found therefore possibly not allowed access
				ans = authorised("AUTHORISATION UPDATE LOWER GROUPS");

			}
		}
	} else {
unknown:
		ans = USERNAME == "EXODUS";
	}
	return ans;
}
libraryexit(authorised)


libraryinit(authorised_job_update)
//--------------------------------
#include <authorised.h>

var msg;
function main() {
	var dummy;
	if (authorised("JOB UPDATE", msg, dummy, ID)) {
		ANS = 1;
	} else {
		ANS = 0;
	}
	return ANS;
}
libraryexit(authorised_job_update)


libraryinit(authorised_journal_post)
//----------------------------------
#include <authorised.h>

var msg;

function main() {
	if (authorised("JOURNAL POST", msg, "", ID)) {
		ANS = 1;
	} else {
		ANS = 0;
	}
	return ANS;
}
libraryexit(authorised_journal_post)


libraryinit(authorised_task_access)
//---------------------------------
#include <authorised.h>

var msg;

function main() {
	if (authorised("TASK ACCESS", msg, "", ID)) {
		ANS = 1;
	} else {
		ANS = 0;
	}
	return ANS;
}
libraryexit(authorised_task_access)


libraryinit(authorised_task_create)
//---------------------------------
#include <authorised.h>

var msg;

function main() {
	var dummy;
	if (authorised("TASK CREATE", msg, dummy, ID)) {
		ANS = 1;
	} else {
		ANS = 0;
	}
	return ANS;
}
libraryexit(authorised_task_create)


libraryinit(authorised_timesheet_administration)
//----------------------------------------------
#include <authorised.h>

var msg;

function main() {
	if (authorised("TIMESHEET ADMINISTRATION", msg, "", ID)) {
		ANS = 1;
	} else {
		ANS = 0;
	}
	return ANS;
}
libraryexit(authorised_timesheet_administration)


libraryinit(dept_and_user_name)
//-----------------------------
function main() {
	return RECORD.f(5) ^ " - " ^ RECORD.f(1);
}
libraryexit(dept_and_user_name)


libraryinit(is_department)
//------------------------
function main() {
	if (RECORD.f(5) == ID) {
		return 1;
	}
	return "";
	return ANS;
}
/*pgsql
	if split_part(data,FM,5) = key then
		ANS='1';
	else
		ANS='';
	end if;
*/
libraryexit(is_department)


libraryinit(keys)
//---------------
#include <srv_common.h>

var usern;

function main() {
	#include <service_common.h>
	if (SECURITY.f(1).locate(ID, usern)) {
		ANS = SECURITY.f(2, usern);
	} else {
		ANS = "";
	}
	return ANS;
}
libraryexit(keys)


libraryinit(last_browser)
//-----------------------
#include <exodus/htmllib2.h>

function main() {
	var ans = RECORD.f(39, 6);
	call htmllib2("OCONV.AGENT.BROWSER", ans);
	return ans;
}
libraryexit(last_browser)


libraryinit(last_os)
//------------------
#include <exodus/htmllib2.h>

function main() {
	var ans = RECORD.f(39, 6);
	call htmllib2("OCONV.AGENT.OS", ans);
	return ans;
}
libraryexit(last_os)


libraryinit(live_user_with_email)
//-------------------------------
function main() {
	if (RECORD.f(7)) {
		let expired = RECORD.f(35);
		if (expired and expired <= date()) {
			ANS = 0;
		} else {
			ANS = 1;
		}
	} else {
		ANS = "";
	}
	return ANS;
}
libraryexit(live_user_with_email)


libraryinit(user_and_dept_name)
//-----------------------------
function main() {
	return RECORD.f(1) ^ " - " ^ RECORD.f(5);
}
libraryexit(user_and_dept_name)
