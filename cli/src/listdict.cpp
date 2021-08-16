#include <exodus/program.h>
programinit()

	function main() {

	var filename = COMMAND.a(2);//field(SENTENCE, " ", 2).lcase();
	if (filename && filename.substr(1, 5) != "dict.")
		filename = "dict." ^ filename;
	//perform("list " ^ dictfilename ^ " by type \"F\" by FMC by PART");

	//list the dictionary filenames if none provided
	if (not filename) {

		//copy of rules in mvdbpostgres.cpp
		var dictdb = "";
		dictdb.osgetenv("EXO_DICTDBNAME");
		if (not dictdb)
			dictdb = "exodus_dict";
		var conn;
		if (not conn.connect(dictdb))
			conn.connect();

		var dictfilenames = "";
		var filenames = conn.listfiles();
		for (var filename : filenames) {
			if (filename.starts("dict."))
				dictfilenames ^= filename ^ FM;
		}
		dictfilenames.swapper(FM, "\n");
		printl(dictfilenames);

		stop();

	}

	var cmd = "list " ^ filename;

	if (filename eq "dict.all")
		cmd ^= " ID-SUPP TABLE_NAME COLUMN_NAME @CRT BY TABLE_NAME";

	cmd ^= " by TYPE by FMC by PART";

	osshell(cmd);

	return 0;
}

programexit()
