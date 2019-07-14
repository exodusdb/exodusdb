#include <exodus/library.h>
libraryinit()

function main() {
	var filename=field(SENTENCE," ",2);
	var fieldname=field(SENTENCE," ",3);
	var indextype=field(SENTENCE," ",4);
	if (indextype)
		fieldname^="_"^indextype;
	var file;
	return osshell("createindex " ^ filename ^ " " ^ fieldname ^ " " ^ indextype ^ " " ^ OPTIONS);
}

libraryexit()

