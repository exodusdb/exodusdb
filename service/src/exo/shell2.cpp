#include <exodus/library.h>

libraryinit()

function main(in cmd, out errors) {

	// var tempdir=std::experimental::filesystem::temp_directory_path().wstring();

	var tempdir		 = "/tmp/";
	var tempfilename = tempdir ^ OSSLASH ^ rnd(99999999) ^ ".tmp";
	var output		 = osshellread(cmd ^ " 2> " ^ tempfilename);

	errors = osread(tempfilename);
	osremove(tempfilename);

	return output;
}

libraryexit()
