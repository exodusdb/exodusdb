#include <exodus/library.h>

libraryinit()

func main(in cmd, out errors) {

	// var tempdir=std::experimental::filesystem::temp_directory_path().wstring();

	let tempdir		 = "/tmp/";
	let tempfilename = tempdir ^ OSSLASH ^ rnd(99999999) ^ ".tmp";
	let output		 = osshellread(cmd ^ " 2> " ^ tempfilename);

	errors = osread(tempfilename);
	//osremove(tempfilename);
	if (tempfilename.osfile() and not osremove(tempfilename)) {
		abort(lasterror());
	}

	return output;
}

}; // libraryexit()
