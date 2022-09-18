#include <exodus/program.h>
programinit()

function main() {

	var filename = COMMAND.f(2);//field(SENTENCE, " ", 2).lcase();
	if (filename && not filename.starts("dict."))
		filename = "dict." ^ filename;
	//perform("list " ^ dictfilename ^ " by type \"F\" by FMC by PART");

	//list the dictionary filenames if none provided
	if (not filename) {

		//copy of rules in mvdbpostgres.cpp
		var dictdb = "";
		dictdb.osgetenv("EXO_DICT");
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
		dictfilenames.replacer(FM, "\n");
		print(dictfilenames);

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
