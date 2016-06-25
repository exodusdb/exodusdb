#include <exodus/library.h>
libraryinit()

//recursive call
#include <authorised.h>

function main(in task0, out msg, in defaultlock="", in username0="") {

	var username;
	var msgusername;
	var taskn;//num
	var taskn2;
	var xx;
	var usern;

	var task=task0;
	if (username0.unassigned() or username0 eq "") {
		//allow for username like FINANCE(STEVE)
		//so security is done like FINANCE but record is kept of actual user
		//this allows for example billing module users to post as finance module users
		username = USERNAME.field("(", 1);
		msgusername = USERNAME;
	}else{
		username = username0;
		msgusername = username;
	}

	//if username='NEOSYS' or username='STEVE' then call msg(task:'');

	if (task[1] == " ") {
		call mssg(DQ ^ (task0 ^ DQ));
	}
	//Each task may have many "locks", each users may have many "keys"
	//A user must have keys to all the locks in order to pass

	if (not task) {
		return 1;
	}

	task.ucaser();
	task.converter(RM ^ FM ^ VM ^ SVM, "\\\\\\");
	task.swapper(" FILE ", " ");
	task.swapper(" - ", " ");
	task.converter(".", " ");
	task.trimmer();

	msg = "";
	// **CALL note(' ':TASK)

	if (task.substr(1, 2) == "..") {
		// call note(task:'')
		return 1;
	}

	var noadd = task[1] == "!";
	if (noadd) {
		task.splicer(1, 1, "");
	}

	var positive = task[1];
	if (positive == "#")
		task.splicer(1, 1, "");
	else
		positive = "";

	//? as first character of task (after positive) means
	//security is being used as a configuration and user neosys has no special privs
	var isneosys;
	if (task[1] == "?") {
		isneosys = 0;
		task.splicer(1, 1, "");
	} else
		isneosys = username == "NEOSYS";

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
	if (SECURITY.locate(task, taskn, 10)) {
		if (deleting) {
			SECURITY.eraser(10, taskn);
			SECURITY.eraser(11, taskn);
updateprivs:
			gosub writeuserprivs();
			return 1;
		} else if (renaming) {
			//delete any existing rename target task
			if (SECURITY.locate(defaultlock, taskn2, 10)) {
				SECURITY.eraser(10, taskn2);
				SECURITY.eraser(11, taskn2);
				if (taskn2 < taskn) {
					taskn -= 1;
				}
			}
			SECURITY.r(10, taskn, defaultlock);
			if (renaming) {
				call note(task ^ "|TASK RENAMED|" ^ defaultlock);
			}
			goto updateprivs;
		} else if (updating) {
			var tt = defaultlock;
			if (SECURITY.locate(defaultlock, taskn2, 10)) {
				tt = SECURITY.a(11, taskn2);
			}
			SECURITY.r(11, taskn, tt);
			goto updateprivs;
		}
	} else {

		if (deleting) {
			return 1;
		}
		if (renaming) {
			//if the task to be renamed doesnt exist just add the target task
			call authorised(defaultlock,msg);
			return 1;
		}
		if (not noadd) {
			noadd = task[-1] == DQ or SECURITY.length() > 48000;
			//if passed a default lock then add even tasks ending like "XXXXX"
			if (not defaultlock.unassigned()) {
				if (defaultlock)
					noadd = 0;
			}
		}

		if (not noadd) {
			gosub readuserprivs();
			//if (SECURITY.length() < 65000) {
			if (true) {
var x=var();
				if (not(SECURITY.locateby(task, "AL", taskn, 10))) {
					var newlock=defaultlock;
					//get locks on default task if present otherwise new locks are none
					if (newlock and SECURITY.locate(newlock, xx, 10)) {
						newlock = SECURITY.a(11, xx);
					}
					SECURITY.inserter(10, taskn, task);
					SECURITY.inserter(11, taskn, newlock);
					gosub writeuserprivs();
					if (username == "NEOSYS") {
						call note(task ^ "|TASK ADDED");
					}
				}
			}
		}
	}

	//if no locks then pass ok unless positive locking required
	var locks = SECURITY.a(11, taskn);
	if (locks == "") {
		if (positive and not isneosys) {
notallowed:
			//MSG=capitalise(TASK):'||Sorry, ':capitalise(msgusername):', you are not authorised to do this.|'
			if (msgusername ne USERNAME)
				msg = capitalise(msgusername) ^ "is not";
			else
				msg = "Sorry, " ^ capitalise(msgusername) ^ ", you are";

			msg ^= " not";
			if (positive) {
				msg ^= " specifically";
			}
			msg ^= " authorised to do||" ^ capitalise(task);

			return 0;

		} else
			return 1;

	} else if (locks == "NOONE") {
		goto notallowed;
	}

	//if index('012',@privilege,1) then goto ok
	if (isneosys) {
		return 1;
	}

	//find the user (add to bottom if not found)
	//surely this is not necessary since users are in already
	if (not(SECURITY.locate(username, usern, 1))) {
		if (username ne "NEOSYS" and username ne APPLICATION) {
			gosub readuserprivs();
			usern = (SECURITY.a(1)).count(VM) + (SECURITY.a(1) ne "") + 1;
			if (SECURITY.length() < 65000) {
				var users;
				if (not(users.open("USERS", ""))) {
					goto notallowed;
				}
				var USER;
				if (not(USER.read(users, username))) {
					goto notallowed;
				}
				SECURITY.inserter(1, usern, username);
				SECURITY.inserter(2, usern, "");
				//add in memory only
				//gosub writeuserprivs
			}
		}
	}

	//user must have all the keys for all the locks on this task
	//following users up to first blank line also have the same keys
	var keys = SECURITY.a(2).field(VM, usern, 999);
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
		if (keys.locateusing(lockx, " ")) {
			//call note(task:' ok')
		} else
		//MSG=capitalise(TASK):'||Sorry, ':capitalise(msgusername):', you are not authorised to do this.|'
		//MSG='Sorry, ':capitalise(msgusername):', you are not authorised to do||':capitalise(task)
	//call note(task:' ko')
		//RETURN 0
		goto notallowed;
	};//lockn;

//ok:
	//CALL STATUP(2,3,TASK)
	return 1;

}

subroutine readuserprivs() {
	if (not DEFINITIONS or not(SECURITY.read(DEFINITIONS, "SECURITY"))) {
		SECURITY = "";
	}
	return;
}

subroutine writeuserprivs() {
	SECURITY.r(9, "");
	if (DEFINITIONS) {
		SECURITY.write(DEFINITIONS, "SECURITY");
	}
	return;
}


libraryexit()
