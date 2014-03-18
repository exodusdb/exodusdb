#include <exodus/library.h>
libraryinit()


#include <gen.h>

var USERNAME;
var msgusername;
var taskn;//num
var taskn2;
var xx;
var usern;

function main() {
	//call msg(task)
	//jbase
	if (username0.unassigned()) {
nousername0:
		//allow for username like FINANCE(STEVE)
		//so security is done like FINANCE but record is kept of actual user
		//this allows for example billing module users to post as finance module users
		USERNAME = USERNAME.field("(", 1);
		msgusername = USERNAME;
		goto 98;
	}
	if (username0 == "") {
		goto nousername0;
	}else{
		USERNAME = username0;
		msgusername = USERNAME;
	}
L98: *if username='NEOSYS' or username='STEVE' then call msg(task:'');

	if (task.substr(1, 1) == " ") {
		call mssg(DQ ^ (task ^ DQ));
	}
	//Each task may have many "locks", each users may have many "keys"
	//A user must have keys to all the locks in order to pass

	if (not task) {
		goto ok;
	}

	task.ucaser();
	task.converter(RM ^ FM ^ VM ^ SVM, "\\\\\\");
	task.swapper(" FILE ", " ");
	task.swapper(" - ", " ");
	task.converter(".", " ");
	task.trimmer();

	var msg = "";
	//**CALL note(' ':TASK)

	if (task.substr(1, 2) == "..") {
		// call note(task:'')
		return 1;
	}

	var noadd = task.substr(1, 1) == "!";
	if (noadd) {
		task.splicer(1, 1, "");
	}
	//if noadd else NOADD=((TASK[-1,1]='"') and (len(userprivs)<10000))
	if (not noadd) {
		var lenuserprivs = gen._security.length();
		noadd = task.substr(-1, 1) == DQ or lenuserprivs > 48000;
	}
	var positive = task.substr(1, 1);
	if (positive == "#") {
		task.splicer(1, 1, "");
		goto 302;
	}
	positive = "";

	//? as first character of task (after positive) means
	//security is being used as a configuration and user neosys has no special privs
	if (task.substr(1, 1) == "?") {
		var isneosys = 0;
		task.splicer(1, 1, "");
		goto 339;
	}
	var isneosys = USERNAME == "NEOSYS";

	var deleting = task.substr(1, 8) == "%DELETE%";
	if (deleting) {
		task.splicer(1, 8, "");
	}
	var updating = task.substr(1, 8) == "%UPDATE%";
	if (updating) {
		task.splicer(1, 8, "");
	}
	var renaming = task.substr(1, 8) == "%RENAME%";
	if (renaming) {
		task.splicer(1, 8, "");
	}

	//find the task
	if (gen._security.a(10).locateusing(task, VM, taskn)) {
		if (deleting) {
			gen._security.eraser(10, taskn);
			gen._security.eraser(11, taskn);
updateprivs:
			gosub writeuserprivs();
			return 1;
			goto 618;
		}
		if (renaming) {
			//delete any existing rename target task
			if (gen._security.a(10).locateusing(defaultlock, VM, taskn2)) {
				gen._security.eraser(10, taskn2);
				gen._security.eraser(11, taskn2);
				if (taskn2 < taskn) {
					taskn -= 1;
				}
			}
			gen._security.r(10, taskn, defaultlock);
			if (renaming) {
				call note(task ^ "|TASK RENAMED|" ^ defaultlock);
			}
			goto updateprivs;
			goto 618;
		}
		if (updating) {
			var tt = defaultlock;
			if (gen._security.a(10).locateusing(defaultlock, VM, taskn2)) {
				tt = gen._security.a(11, taskn2);
			}
			gen._security.r(11, taskn, tt);
			goto updateprivs;
		}
		goto 820;
	}
	if (deleting) {
		return 1;
	}
	if (renaming) {
		//if the task to be renamed doesnt exist .. just add the target task
		call authorised(defaultlock);
		return 1;
	}
	if (not noadd) {
		gosub readuserprivs();
		if (USERNAME == "NEOSYS") {
			var interactive = not SYSTEM.a(33);
			if (interactive) {
				call note(task ^ "|TASK ADDED");
			}
		}
		if (gen._security.length() < 65000) {
			if (not(gen._security.a(10).locatebyusing(task, "AL", taskn, VM))) {
				if (defaultlock.unassigned()) {
					defaultlock = "";
					goto 784;
				}
				if (gen._security.a(10).locateusing(defaultlock, VM, xx)) {
					defaultlock = gen._security.a(11, xx);
				}
				gen._security.inserter(10, taskn, task);
				gen._security.inserter(11, taskn, defaultlock);
				gosub writeuserprivs();
			}
		}
	}

	//if no locks then pass ok unless positive locking required
	var locks = gen._security.a(11, taskn);
	if (locks == "") {
		if (positive and not isneosys) {
notallowed:
			//MSG=capitalise(TASK):'||Sorry, ':capitalise(msgusername):', you are not authorised to do this.|'
			if (msgusername ne USERNAME) {
				msg = capitalise(msgusername) ^ "is not";
				goto 892;
			}
			msg = "Sorry, " ^ capitalise(msgusername) ^ ", you are";
			msg ^= " not";
			if (positive) {
				msg ^= " specifically";
			}
			msg ^= " authorised to do||" ^ capitalise(task);
			return 0;
			goto 937;
		}
		goto ok;
		goto 953;
	}
	if (locks == "NOONE") {
		goto notallowed;
	}

	//if index('012',@privilege,1) then goto ok
	if (isneosys) {
		goto ok;
	}

	//find the user (add to bottom if not found)
	//surely this is not necessary since users are in already
	if (not(gen._security.a(1).locateusing(USERNAME, VM, usern))) {
		if (USERNAME ne "NEOSYS" and USERNAME ne ACCOUNT) {
			gosub readuserprivs();
			usern = (gen._security.a(1)).count(VM) + (gen._security.a(1) ne "") + 1;
			if (gen._security.length() < 65000) {
				var users;
				if (not(users.open("USERS", ""))) {
					goto notallowed;
				}
				var USER;
				if (not(USER.read(users, USERNAME))) {
					goto notallowed;
				}
				gen._security.inserter(1, usern, USERNAME);
				gen._security.inserter(2, usern, "");
				//add in memory only
				//gosub writeuserprivs
			}
		}
	}

	//user must have all the keys for all the locks on this task
	//following users up to first blank line also have the same keys
	var keys = gen._security.a(2).field(VM, usern, 999);
	var temp = keys.index("---", 1);
	if (temp) {
		keys.splicer(temp - 1, 999, "");
	}
	//convert ',' to vm in keys
	//convert ',' to vm in locks
	keys.converter("," _VM_ "", "  ");
	locks.converter(",", " ");
	//NLOCKS=COUNT(LOCKS,vm)+1
	var nlocks = locks.count(" ") + 1;

	for (var lockn = 1; lockn <= nlocks; ++lockn) {
		//LOCKx=FIELD(LOCKS,vm,LOCKN)
		var lockx = locks.field(" ", lockn);
		if (keys.locateusing(lockx, " ", temp)) {
	//call note(task:' ok')
			goto 1223;
		}
		//MSG=capitalise(TASK):'||Sorry, ':capitalise(msgusername):', you are not authorised to do this.|'
		//MSG='Sorry, ':capitalise(msgusername):', you are not authorised to do||':capitalise(task)
	//call note(task:' ko')
		//RETURN 0
		goto notallowed;
	};//lockn;

ok:
	//CALL STATUP(2,3,TASK)
	return 1;

}

subroutine readuserprivs() {
	//IF DEFINITIONS THEN
	//put back in case called from FILEMAN due to no datasets
	//taken out again but why?
	//if definitions then
	if (gen._definitions.open("DEFINITIONS", "")) {
		if (not(gen._security.read(gen._definitions, "SECURITY"))) {
			gen._security = "";
		}
		gen._security = gen._security.invert();
		goto 1296;
	}
	gen._security = "";
	// end
	// END
	return;

}

subroutine writeuserprivs() {
	gen._security.r(9, "");
	//IF DEFINITIONS THEN
	//put back in case called from FILEMAN due to no datasets
	//taken out again but why?
	//if definitions then
	if (gen._definitions.open("DEFINITIONS", "")) {
		gen._security.invert().write(gen._definitions, "SECURITY");
	}
	// end
	// end
	return;

}


libraryexit()