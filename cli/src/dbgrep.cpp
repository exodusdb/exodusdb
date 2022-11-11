#include <exodus/program.h>
programinit()

function main() {

	//init:
	let reqfile = COMMAND.f(2);
	var reqtext = COMMAND.f(3);
	var padding = COMMAND.f(4);

	bool hasreqtext = reqtext.len();
	if (not hasreqtext) {
		errputl("syntax: dbgrep filename text");
		errputl("        dbgrep - text");
		//stop();
	}

	bool caseinsensitive = OPTIONS.contains("I");
	if (caseinsensitive)
		reqtext.lcaser();

	if (not padding)
		padding = 256;

	var colored_reqtext = reqtext;
	if (TERMINAL)
				colored_reqtext = "\033[1;31m" ^ reqtext ^ "\033[0m";

	// get filenames
	let filenames = listfiles();

	for (var filename : filenames) {

		if (filename.contains("preselect") or filename.contains("select_stage2"))
			continue;

		// filter by reqfilename. "-" means all files
		if (reqfile and reqfile != "-" and filename != reqfile.lcase())
			continue;

		// OPEN FILE or skip file
		var file;
		if (not(file.open(filename))) {
			errputl(filename.quote() ^ " file cannot be opened");
			continue;
		}

		//file.select("SELECT " ^ filename ^ " (SR)");
		if (not file.select("SELECT " ^ filename ^ " (SR)")) {
			loglasterror();
		}

		let nresults = 1;
		while (file.readnext(RECORD, ID, MV)) {

			if (esctoexit())
				abort("");

//			// get record or skip
//			let RECORD;
//			if (not(RECORD.read(file, ID))) {
//				//abort(ID.quote()^" is missing from " ^ file.quote());
//				continue;
//			}

			if (caseinsensitive) {
				RECORD.lcaser();
				ID.lcaser();
			}

			if (not RECORD.contains(reqtext) and not ID.contains(reqtext)) {
				continue;
			}

			// padding & startpos used to contextualise found reqtext
			var startpos = RECORD.index(reqtext);
			let startpos2 = startpos - padding;
			if (startpos2 < 1) {
				padding += startpos2;
				startpos = 1;
			}
			//var leftcontext = RECORD.b(startpos - padding, padding).field2(FM, -1);
			let leftcontext = RECORD.b(startpos2, padding).field2(FM, -1);
			let rightcontext = RECORD.b(startpos + len(reqtext), padding).f(1);

			printl(filename, ID, leftcontext ^ colored_reqtext ^ rightcontext);
			//var(filename ^ " : " ^ ID ^ " :: " ^ context).outputl();
		}
	}

	return 0;
}

programexit()
