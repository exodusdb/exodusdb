#include <exodus/program.h>
programinit()

	function main() {

	var filename = field(SENTENCE, " ", 2).lcase();
	if (filename.substr(1, 5) != "dict_")
		filename = "dict_" ^ filename;
	//perform("list " ^ dictfilename ^ " by type \"F\" by FMC by PART");

	var cmd = "list " ^ filename;

	if (filename eq "dict_all")
		cmd ^= " ID-SUPP TABLE_NAME COLUMN_NAME @CRT BY TABLE_NAME";

	cmd ^= " by TYPE by FMC by PART";

	osshell(cmd);

	return 0;
}

programexit()
