#include <exodus/library.h>
libraryinit()

function main() {
	var filename = COMMAND.a(2);
	ID = COMMAND.a(3);

	var file;
	if (not open(filename,file) ){
		call fsmsg();
		abort();
	}

	if(not read(RECORD,file,ID) ){
		call fsmsg();
		abort();
	}

	call makelist("",RECORD);

	var key;
	readnext(key);

	printl(key);

	return 0;
}

libraryexit()

