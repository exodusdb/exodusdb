#include <exodus/library.h>
libraryinit()

#include <select2.h>

#include <win.h>

function main() {
	printl("testd2 says 'Hello World!'");

	var data;
	win.srcfile="";
	win.datafile="";
	win.orec="";
	win.wlocked="";
	win.reset="";
	win.valid="";

	call select2("USERS", "", "BY RANK WITH DEPARTMENT_CODE NE @ID", "USER_CODE USER_AND_DEPT_NAME", "", data, USER3, "", "", "");
//print(data.swap(FM," FM \n").swap(VM,"\n VM "));
print(data.swap(FM," FM ").swap(VM," VM "));
	return 0;
}

//debug
libraryexit()

