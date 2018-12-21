#include <exodus/library.h>
libraryinit()

function main(in filename, io file, in similarfilename="") {

	//opens a filename and returns file variable and true
	//otherwise if similar file name it tries to create in same fashion
	//otherwise 

	file = "";

	for (var tryn=1; tryn<=2; ++tryn) {

		if (file.open(filename)) {
			return true;
		}

		//option to create file if it does not exist
		var tt="";
		if (similarfilename == filename or tt.open(similarfilename)) {
			var().createfile(filename);
		}
	}

	call mssg(filename.quote() ^ " file is missing");
	return false;

}

libraryexit()
