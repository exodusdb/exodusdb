#include <exodus/program.h>
programinit()
function main() {

	let prog =
		"#include <exodus/program.h>\n"
		"programinit()\n"
		"function main() {\n" ^
		SENTENCE.field(" ", 2, 99999).unquote() ^
		"\n"
		"return 0;}\n"
		"programexit()\n";

	let tempfilebase = "eevalt";
	let tempfilesrc	 = tempfilebase ^ ".cpp";
	//oswrite(prog, tempfilesrc);
	if (not oswrite(prog, tempfilesrc)) {
		loglasterror();
	}
	//osshell("compile " ^ tempfilesrc);
	if (not osshell("compile " ^ tempfilesrc)) {
		loglasterror();
	}
	//osshell(tempfilebase);
	if (not osshell(tempfilebase)) {
		loglasterror();
	}

	return 0;
}
programexit()
