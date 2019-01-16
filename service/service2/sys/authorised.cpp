#include <exodus/library.h>
libraryinit()

#include <authorised.h>

#include <gen.h>

var username;
var msgusername;
var isneosys;//num
var taskn;//num
var taskn2;
var xx;
var newlock;
var usern;

function main(in task0, out msg, in defaultlock="", in username0="") {
	//c sys in,out,"",""

	//NB de bugging afer recompile requires restart since SECURITY is in PRELOAD
	//call msg(task)
	//jbase
	var task = task0;

	if (username0.unassigned()) {
nousername0:
		//allow for username like FINANCE(STEVE)
		//so security is done like FINANCE but record is kept of actual user
		//this allows for example billing module users to post as finance module users
		username = USERNAME.field("(", 1);
		msgusername = USERNAME;
	} else if (username0 == "") {
		goto nousername0;
		{}
	} else {
		username = username0;
		msgusername = username;
	}
//L127:
	//if username='NEOSYS' or username='STEVE' then call msg(task:'')

	if (task[1] == " ") {
		call mssg(DQ ^ (task ^ DQ));
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
	//!*CALL note(' ':TASK)

	if (task.substr(1,2) == "..") {
		// call note(task:'')
		return 1;
	}

	var noadd = task[1] == "!";
	if (noadd) {
		task.splicer(1, 1, "");
	}

	var positive = task[1];
	if (positive == "#") {
		task.splicer(1, 1, "");
	}else{
		positive = "";
	}

	//? as first character of task (after positive) means
	//security is being used as a configuration and user neosys has no special privs
	if (task[1] == "?") {
		isneosys = 0;
		task.splicer(1, 1, "");
	}else{
		isneosys = username == "NEOSYS";
	}

	var deleting = task.substr(1,8) == "%DELETE%";
	if (deleting) {
		task.splicer(1, 8, "");
	}
	var updating = task.substr(1,8) == "%UPDATE%";
	if (updating) {
		task.splicer(1, 8, "");
	}
	var renaming = task.substr(1,8) == "%RENAME%";
	if (renaming) {
		task.splicer(1, 8, "");
	}

	//find the task
	if (SECURITY.a(10).locateusing(task, VM, taskn)) {

		if (deleting) {
			SECURITY.eraser(10, taskn);
			SECURITY.eraser(11, taskn);
updateprivs:
			gosub writeuserprivs();
			return 1;
		} else if (renaming) {
			//delete any existing rename target task
			if (SECURITY.a(10).locateusing(defaultlock, VM, taskn2)) {
				SECURITY.eraser(10, taskn2);
				SECURITY.eraser(11, taskn2);
				if (taskn2 < taskn) {
					taskn -= 1;
				}
			}
			SECURITY.r(10, taskn, defaultlock);
			if (renaming) {
				//skip warning except for live databases included in startup
				if (not SYSTEM.a(61)) {
					if (SYSTEM.a(58).locateusing(SYSTEM.a(17), VM, xx)) {
						call note("Task renamed:|Old: " ^ task ^ "|New: " ^ defaultlock);
					}
				}
			}
			goto updateprivs;
			goto updateprivs;
		}
		if (updating) {
			var tt = defaultlock;
			if (SECURITY.a(10).locateusing(defaultlock, VM, taskn2)) {
				tt = SECURITY.a(11, taskn2);
			}
			SECURITY.r(11, taskn, tt);
			goto updateprivs;
			goto updateprivs;
		}

	}else{
		if (deleting) {
			return 1;
		}
		if (renaming) {
			//if the task to be renamed doesnt exist .. just add the target task
			var newtask = defaultlock;
			call authorised(newtask, xx);
			return 1;
		}
		if (not noadd) {
			noadd = (task[-1] == DQ) or (SECURITY.length() > 48000);
			//if passed a default lock then add even tasks ending like "XXXXX"
			if (not defaultlock.unassigned()) {
				if (defaultlock) {
					noadd = 0;
				}
			}
		}
		if (not noadd) {
			gosub readuserprivs();
			if (username == "NEOSYS") {
				var interactive = not SYSTEM.a(33);
				if (interactive) {
					call note(task ^ "|TASK ADDED");
				}
			}
			if (SECURITY.length() < 65000) {
				if (not(SECURITY.a(10).locateby(task, "AL", taskn))) {
					if (defaultlock.unassigned()) {
						newlock = "";
					}else{
						if (SECURITY.a(10).locateusing(defaultlock, VM, xx)) {
							newlock = SECURITY.a(11, xx);
						}else{
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

	//if no locks then pass ok unless positive locking required
	var locks = SECURITY.a(11, taskn);
	if (locks == "") {

		//not positive ok
		if (not positive) {
			return 1;

		//neosys always ok
		} else if (isneosys) {
			return 1;

		//positive and no lock always fail
		} else {
			{}
notallowed:
			//MSG=capitalise(TASK):'||Sorry ':capitalise(msgusername):', you are not authorised to do this.|'
			if (msgusername ne USERNAME) {
				msg = capitalise(msgusername) ^ "is not";
				}else{
				msg = "Sorry " ^ capitalise(msgusername) ^ ", you are";
			}
			msg ^= " not";
			if (positive) {
				msg ^= " specifically";
			}
			msg ^= " authorised to do||" ^ capitalise(task);
			return 0;
		}

		//NB not NOBODY/EVERYBODY

	//special lock NOONE
	} else if (locks == "NOONE") {
		{}
		goto notallowed;
		{}

	//special lock EVERYONE
	} else if (locks == "EVERYONE") {
		return 1;
	}

	//neosys user always passes
	if (isneosys) {
		return 1;
	}

	//find the user
	if (not(SECURITY.a(1).locateusing(username, VM, usern))) {
			/*20180312;
			if username<>'NEOSYS' and username<>@account then;
				gosub readuserprivs;
				usern=count(userprivs<1>,vm)+(userprivs<1> ne '')+1;
				if len(userprivs) lt 65000 then;
					open 'USERS' to users else goto notallowed;
					read user from users,username else;
						goto notallowed;
						end;
					userprivs=INSERT(userprivs,1,USERN,0,username);
					userprivs=INSERT(userprivs,2,USERN,0,'');
					//add in memory only
					//gosub writeuserprivs
					end;
				end;
			*/
	}

	//user must have all the keys for all the locks on this task
	//following users up to first blank line also have the same keys
	var keys = SECURITY.a(2).field(VM, usern, 65535);
	var temp = keys.index("---", 1);
	if (temp) {
		keys.splicer(temp - 1, 65535, "");
	}
	keys.converter("," ^ VM, "  ");
	locks.converter(",", " ");
	locks.trimmer();
	var nlocks = locks.count(" ") + 1;

	for (var lockn = 1; lockn <= nlocks; ++lockn) {
		var lockx = locks.field(" ", lockn);
		if (keys.locateusing(lockx, " ", temp)) {
		}else{
			goto notallowed;
		}
	};//lockn;

	//OK:
	return 1;

}

subroutine readuserprivs() {
	//in case called from FILEMAN due to no datasets
	if (DEFINITIONS.open("DEFINITIONS", "")) {
		if (not(SECURITY.read(DEFINITIONS, "SECURITY"))) {
			SECURITY = "";
		}
		SECURITY = SECURITY.invert();
	}else{
		SECURITY = "";
	}
	return;

}

subroutine writeuserprivs() {
	SECURITY.r(9, "");
	if (DEFINITIONS.open("DEFINITIONS", "")) {
		SECURITY.invert().write(DEFINITIONS, "SECURITY");
	}
	return;

}


libraryexit()
