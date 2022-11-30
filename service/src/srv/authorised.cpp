#include <exodus/library.h>
libraryinit()

#include <authorised.h>

#include <service_common.h>

#include <srv_common.h>

var username;
var msgusername;
var isexodus;  // num
var taskn;	   // num
var taskn2;
//var xx;
var newlock;
var usern;

function main(in task0, out msg, in defaultlock = "", in username0 = "") {

	// NB de bugging afer recompile requires restart since SECURITY is in PRELOAD
	// call msg(task)

	var task = task0;

	if (username0.unassigned()) {
nousername0:
		// allow for username like FINANCE(STEVE)
		// so security is done like FINANCE but record is kept of actual user
		// this allows for example billing module users to post as finance module users
		username	= USERNAME.field("(", 1);
		msgusername = USERNAME;
	} else if (username0 == "") {
		goto nousername0;
	} else {
		username	= username0;
		msgusername = username;
	}

	// if username='EXODUS' or username='STEVE' then call msg(task:'')

	if (task.starts(" ")) {
		call mssg(task.quote());
	}
	// Each task may have many "locks", each users may have many "keys"
	// A user must have keys to all the locks in order to pass

	if (not task) {
		return 1;
	}

	task.ucaser();
	task.converter(RM ^ FM ^ VM ^ SM, "\\\\\\");
	task.replacer(" FILE ", " ");
	task.replacer(" - ", " ");
	task.converter(".", " ");
	task.trimmer();

	msg = "";
	// !*CALL note(' ':TASK)

	if (task.starts("..")) {
		// call note(task:'')
		return 1;
	}

	var noadd = task.starts("!");
	if (noadd) {
		task.cutter(1);
	}

	var positive = task[1];
	if (positive == "#") {
		task.cutter(1);
	} else {
		positive = "";
	}

	// ? as first character of task (after positive) means
	// security is being used as a configuration and user exodus has no special privs
	if (task.starts("?")) {
		isexodus = 0;
		task.cutter(1);
	} else {
		isexodus = username == "EXODUS";
	}

	let deleting = task.starts("%DELETE%");
	if (deleting) {
		task.cutter(8);
	}
	let updating = task.starts("%UPDATE%");
	if (updating) {
		task.cutter(8);
	}
	let renaming = task.starts("%RENAME%");
	if (renaming) {
		task.cutter(8);
	}

	// find the task
	if (SECURITY.f(10).locate(task, taskn)) {

		if (deleting) {
			SECURITY.remover(10, taskn);
			SECURITY.remover(11, taskn);
updateprivs:
			gosub writeuserprivs();
			return 1;
		} else if (renaming) {
			// delete any existing rename target task
			if (SECURITY.f(10).locate(defaultlock, taskn2)) {
				SECURITY.remover(10, taskn2);
				SECURITY.remover(11, taskn2);
				if (taskn2 < taskn) {
					taskn -= 1;
				}
			}
			SECURITY(10, taskn) = defaultlock;
			if (renaming) {
				// skip warning except for live databases included in startup
				if (not SYSTEM.f(61)) {
					if (SYSTEM.f(58).locate(SYSTEM.f(17))) {
						call note("Task renamed:|Old: " ^ task ^ "|New: " ^ defaultlock);
					}
				}
			}
			goto updateprivs;
		} else if (updating) {
			var tt = defaultlock;
			if (SECURITY.f(10).locate(defaultlock, taskn2)) {
				tt = SECURITY.f(11, taskn2);
			}
			SECURITY(11, taskn) = tt;
			goto updateprivs;
		}

	} else {
		if (deleting) {
			return 1;
		}
		if (renaming) {
			// if the task to be renamed doesnt exist .. just add the target task
			var	 newtask = defaultlock;
			call authorised(newtask);
			return 1;
		}
		if (not noadd) {
			// NOADD=((TASK[-1,1]='"') or (len(userprivs)>48000))
			noadd = (task.ends(DQ)) or (SECURITY.len() > maxstrsize_ * 2 / 3);
			// if passed a default lock then add even tasks ending like "XXXXX"
			if (not defaultlock.unassigned()) {
				if (defaultlock) {
					noadd = 0;
				}
			}
		}
		if (not noadd) {
			gosub readuserprivs();
			if (username == "EXODUS") {
				let interactive = false;  //not(SYSTEM.f(33));
				if (interactive) {
					call note(task ^ "|TASK ADDED");
				}
			}
			// if len(userprivs) < 65000 then
			if (SECURITY.len() < maxstrsize_ - 530) {
				if (not SECURITY.f(10).locateby("AL", task, taskn)) {
					if (defaultlock.unassigned()) {
						newlock = "";
					} else {
						var lockn;
						if (SECURITY.f(10).locate(defaultlock, lockn)) {
							newlock = SECURITY.f(11, lockn);
						} else {
							newlock = defaultlock;
						}
					}
					SECURITY.inserter(10, taskn, task);
					SECURITY.inserter(11, taskn, newlock);
					gosub writeuserprivs();
				}
			}
		}
	}

	// if no locks then pass ok unless positive locking required
	var locks = SECURITY.f(11, taskn);
	if (locks == "") {

		// not positive ok
		if (not positive) {
			return 1;

			// exodus always ok
		} else if (isexodus) {
			return 1;

			// positive and no lock always fail
		} else {
notallowed:
			// MSG=capitalise(TASK):'||Sorry ':capitalise(msgusername):', you are not authorised to do this.|'
			if (msgusername != USERNAME) {
				msg = capitalise(msgusername) ^ "is not";
			} else {
				msg = "Sorry " ^ capitalise(msgusername) ^ ", you are";
			}
			msg ^= " not";
			if (positive) {
				msg ^= " specifically";
			}
			msg ^= " authorised to do||" ^ capitalise(task);
			// RETURN 0
			return "";
		}

		// NB not NOBODY/EVERYBODY

		// special lock NOONE
	} else if (locks == "NOONE") {
		goto notallowed;

		// special lock EVERYONE
	} else if (locks == "EVERYONE") {
		return 1;
	}

	// exodus user always passes
	if (isexodus) {
		return 1;
	}

	// find the user
	if (not SECURITY.f(1).locate(username, usern)) {
	}

	// user must have all the keys for all the locks on this task
	// following users up to first blank line also have the same keys
	var keys = SECURITY.f(2).field(VM, usern, 65535);
	var temp = keys.index("---");
	if (temp) {
		keys.paster(temp - 1, 65535, "");
	}
	keys.converter("," ^ VM, "  ");
	locks.converter(",", " ");
	locks.trimmer();
	let nlocks = locks.fcount(" ");

	for (const var lockn : range(1, nlocks)) {
		let lockx = locks.field(" ", lockn);
		if (keys.locateusing(" ", lockx, temp)) {
		} else {
			goto notallowed;
		}
	}  // lockn;

	// OK:
	return 1;
}

subroutine readuserprivs() {
	// in case called from FILEMAN due to no datasets
	if (DEFINITIONS.open("DEFINITIONS", "")) {
		if (not SECURITY.read(DEFINITIONS, "SECURITY")) {
			SECURITY = "";
		}
	} else {
		SECURITY = "";
	}
	return;
}

subroutine writeuserprivs() {
	SECURITY(9) = "";
	if (DEFINITIONS.open("DEFINITIONS", "")) {
		SECURITY.write(DEFINITIONS, "SECURITY");
	}
	return;
}

libraryexit()
