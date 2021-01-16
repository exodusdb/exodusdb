#include <exodus/program.h>
programinit()

function main() {

//init:
	var reqfile = COMMAND.a(2);
	var reqtext = COMMAND.a(3);
	var padding = COMMAND.a(4);

	if ( not(reqfile or reqtext)) {
		errputl("syntax: dbgrep filename text");
		errputl("        dbgrep - text");
		//stop();
	}

	// get filenames
	var filenames = listfiles();
	var nfiles=filenames.dcount(FM);

	for (int filen = 1; filen <= nfiles; ++filen) {

		var filename=filenames.a(filen);

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

		file.select();

		var nresults = 1;
		var key;
		while (file.readnext(key)) {

			// get record or skip
			var data;
			if (not(data.read(file,key)) ) {
				//abort(key.quote()^" is missing from " ^ file.quote());
				continue;
			}

			if ( reqtext and (not(data.lcase().index(reqtext.lcase())) or not(key.lcase().index(reqtext.lcase())))) {
				continue;
			}

			// padding & startpos used to contextualise found reqtext
			var startpos = data.index(reqtext);

			var leftcontext = data.substr(startpos-padding, padding );
			var rightcontext = data.substr( startpos+length(reqtext), padding );

			var context =leftcontext^"\033[1;31m"^reqtext^"\033[0m"^rightcontext;

			var(filename ^ " : " ^ key ^ " :: " ^ context).outputl();

		}

    }

	return 0;

}

programexit()

