#include <exodus/program.h>
programinit()

function main() {

//init:
	var reqfile = COMMAND.a(2);
	var reqkey = COMMAND.a(3);
	var reqtext = COMMAND.a(4);

	if ( not( reqfile or reqtext) )
		printl("syntax: <pgsqlfiles> [filename] [key/datatosearch] [datatosearch] [docmd]");
	// find all dict_files that contain pgsql code
	// find all files that 



//initfile:
	// GET ALL FILENAMES
	var filenames = listfiles();
	var nfiles=filenames.dcount(FM);

//nextfile:
        for (int filen = 1; filen <= nfiles; ++filen) { // for each file do

		// filter filename
        	var filename=filenames.a(filen);
		if (reqfile and reqfile ne "-" and not(filename.index(reqfile.lcase())))
			continue;

		// OPEN FILE or skip file
		var file;
		if (not(file.open(filename))) {
			printl(filename.quote() ^ " file cannot be opened");
			continue;
		}
//fileinit:

//initrec:
		// SELECT
		file.select();

//nextrec:
		var key;
		while (file.readnext(key)) {

			// get record or skip
			var data;
			if( not(data.read(file,key)) ) {
				abort(key.quote()^" is missing from " ^ file.quote());
				continue;
			}

			// case: reqtext not provided OR if reqtext found in data
			if (reqtext and not(data.lcase().index(reqtext.lcase()))) {
				continue;
			}

			printl(filename," ",key," ",data.substr(1,10));
//recinit:
//recexit:
		//goto nextrec
		}
//fileexit:
	//goto nextfile

        }
//exit:

	return 0;

}

programexit()

