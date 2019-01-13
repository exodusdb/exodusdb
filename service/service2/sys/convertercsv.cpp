#include <exodus/library.h>
libraryinit()


function main(in mode, io line, in params, in filename) {
	//c sys in,io,in,in

	//mode=HEAD or LINE
	//input line,params,filename
	//output line

	//copy this example and create another like CONVERTER.XYZ
	//and if importing VOUCHERS, put an entry CONVERTER*VOUCHERS
	//if DEFINITIONS with first line XYZ

	//line conversion code below
	//
	if (filename == "xxxxxx") {
		if (mode == "HEAD") {
			if (params) {
			}

			//...
			line = line;

		}else{

			//...
			line = line;

		}

	}else{
		call mssg(filename ^ " not handled in CONVERTER.CSV");
		var().stop();
	}

	return 0;

}


libraryexit()