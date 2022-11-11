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
		//dictdb.osgetenv("EXO_DICT");
		if (not dictdb.osgetenv("EXO_DICT")) {
			//null
		}
		if (not dictdb) {
			//dictdb = "exodus_dict";
			dictdb = "exodus";
		}
		var conn;
		if (not conn.connect(dictdb)) {
			//connect to default db
			//conn.connect();
			if (not conn.connect()) {
				abort(lasterror());
			}
		}
//GB version
//		let dictdb = "";
//		if (not dictdb.osgetenv("EXO_DICT")) {
//			dictdb = "";
//			//default is exodus i.e ""
//		}
//		let conn;
//		if (not conn.connect(dictdb)) {
//			abort();



		var dictfilenames = "";
		let filenames = conn.listfiles();
		for (var filename : filenames) {
			if (filename.starts("dict."))
				dictfilenames ^= filename ^ FM;
		}
		dictfilenames.replacer(FM, "\n");
		print(dictfilenames);

		return 0;

	}

	var cmd = "list " ^ filename;

	// Need to escape/add quotes for bash
	//cmd ^= " " ^ COMMAND.field(FM, 3, 999999999).convert(FM, " ");

	if (filename eq "dict.all")
		cmd ^= " ID-SUPP FILE_NAME FIELD_NAME @CRT BY FILE_NAME";

	cmd ^= " by TYPE by FMC by PART by MASTER_FLAG";
	//cmd ^= " by TYPE by-dsnd MASTER_FLAG by FMC by PART";

	//cmd ^= " " ^ COMMAND.field(FM, 3, 999999999).convert(FM, " ");
	cmd ^= " {" ^ OPTIONS ^ "}";

	if (OPTIONS.contains("V"))
		logputl(cmd);

	osshell(cmd) or lasterror().errputl("listdict:");

	return 0;
}

programexit()
