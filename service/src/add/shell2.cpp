#include <exodus/library.h>

libraryinit()

function main(in cmd,out errors) {

	//var tempdir=std::experimental::filesystem::temp_directory_path().wstring();

	var tempdir=".";
	var tempfilename=tempdir ^ SLASH ^ rnd(99999999) ^ ".\\$";

	var capturingcmd=cmd^" 2> " ^ tempfilename;
	//capturingcmd.outputl("capturingcmd=");

	var output=capturingcmd.osshellread();

	errors=osread(tempfilename);
	osdelete(tempfilename);

	return output;
}

libraryexit()

