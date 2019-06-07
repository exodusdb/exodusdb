#include <exodus/program.h>
programinit()

function main() {
	var dictfilename="dict_" ^ field(SENTENCE," ",2);
	//perform("list " ^ dictfilename ^ " by type \"F\" by FMC by PART");
	osshell("list " ^ dictfilename ^ " by TYPE by FMC by PART");
	return 0;
}

programexit()

