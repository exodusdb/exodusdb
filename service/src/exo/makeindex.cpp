#include <exodus/library.h>
libraryinit()

function main() {
	var filename=field(COMMAND," ",2);
	var fieldname=field(COMMAND," ",3);
	var indextype=field(COMMAND," ",4);
	if (indextype and indextype ne "BTREE")
		fieldname^="_"^indextype;
	var file;
	var cmd="createindex " ^ filename ^ " " ^ fieldname ^ " " ^ OPTIONS.convert("()","{}");
	cmd.outputl("cmd=");
	return osshell(cmd);
}

libraryexit()

