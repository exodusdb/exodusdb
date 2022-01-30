#include <exodus/program.h>
programinit()

function main() {

	//init:
	var reqfile = COMMAND.a(2);
	var reqtext = COMMAND.a(3);
	var padding = COMMAND.a(4);

	bool hasreqtext = reqtext.len();
	if (not hasreqtext) {
		errputl("syntax: dbgrep filename text");
		errputl("        dbgrep - text");
		//stop();
	}

	bool caseinsensitive = OPTIONS.index("I");
	if (caseinsensitive)
		reqtext.lcaser();

	if (not padding)
		padding = 256;

	var colored_reqtext=reqtext;
	if (TERMINAL)
				colored_reqtext = "\033[1;31m" ^ reqtext ^ "\033[0m";

	// get filenames
	var filenames = listfiles();

	for (var filename : filenames) {

		if (filename.index("preselect") or filename.index("select_stage2"))
			continue;

		// filter by reqfilename. "-" means all files
		if (reqfile and reqfile ne "-" and filename ne reqfile.lcase())
			continue;

		// OPEN FILE or skip file
		var file;
		if (not(file.open(filename))) {
			errputl(filename.quote() ^ " file cannot be opened");
			continue;
		}

		file.select("SELECT " ^ filename ^ " (SR)");

		var nresults = 1;
		while (file.readnext(RECORD, ID, MV)) {

			if (esctoexit())
				stop();

//			// get record or skip
//			var RECORD;
//			if (not(RECORD.read(file, ID))) {
//				//abort(ID.quote()^" is missing from " ^ file.quote());
//				continue;
//			}

			if (caseinsensitive) {
				RECORD.lcaser();
				ID.lcaser();
			}

			if (not RECORD.index(reqtext) and not ID.index(reqtext)) {
				continue;
			}

			// padding & startpos used to contextualise found reqtext
			var startpos = RECORD.index(reqtext);
			var startpos2 = startpos - padding;
			if (startpos2 < 1) {
				padding += startpos2;
				startpos = 1;
			}
			//var leftcontext = RECORD.substr(startpos - padding, padding).field2(FM, -1);
			var leftcontext = RECORD.substr(startpos2, padding).field2(FM, -1);
			var rightcontext = RECORD.substr(startpos + length(reqtext), padding).a(1);

			printl(filename, ID, leftcontext ^ colored_reqtext ^ rightcontext);
			//var(filename ^ " : " ^ ID ^ " :: " ^ context).outputl();
		}
	}

	return 0;
}

programexit()
