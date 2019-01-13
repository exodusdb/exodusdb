#include <exodus/library.h>
libraryinit()

#include <shell2.h>

#include <gen.h>

var ii;
var errors;

function main(in inputfilename, io encoding1, io encoding2, out result, out msg) {
	//c sys in,io,io,out,out

	//uses the unicode uconv to convert file format
	//
	//http://www.microsoft.com/globaldev/reference/cphome.mspx

	result = 0;

	//make cygwin command
	//look for local or cygwin wget.exe otherwise quit
	var cmd = "uconv.exe";
	if (not cmd.osfile()) {
		cmd.splicer(1, 0, SYSTEM.a(50));
	}
	if (not cmd.osfile()) {
		msg = "UCONVFILE: Cannot find " ^ cmd;
		//indicate failed but because of lack of uconv.exe file
		result = "";

		return 0;
	}

	//determine the encoding1
	if (encoding1 == "CODEPAGE") {
		call osgetenv("CODEPAGE", encoding1);
		var oemcodepages = "437" _VM_ "720" _VM_ "737" _VM_ "775" _VM_ "850" _VM_ "852" _VM_ "855" _VM_ "857" _VM_ "858" _VM_ "862" _VM_ "866" _VM_ "874" _VM_ "932" _VM_ "936" _VM_ "949" _VM_ "950" _VM_ "1258";
		var wincodepages = "1252" _VM_ "1256" _VM_ "1253" _VM_ "1257" _VM_ "1252" _VM_ "1252" _VM_ "1251" _VM_ "1254" _VM_ "1252" _VM_ "1255" _VM_ "1251" _VM_ "874" _VM_ "932" _VM_ "936" _VM_ "949" _VM_ "950" _VM_ "874";
		if (oemcodepages.locateusing(encoding1, VM, ii)) {
			encoding1 = wincodepages.a(1, ii);
		}
	}
	if (not encoding1) {
		msg = "UCONVFILE: Missing encoding1";
		return 0;
	}

	//dont convert if latin
	//hopefully to reduce chance of screwups/reduce filesize when latin
	if (encoding1 == 1252) {
		result = 1;
		return 0;
	}

	//determine the encoding2
	if (encoding2 == "CODEPAGE") {
		call osgetenv("CODEPAGE", encoding2);
	}
	if (not encoding2) {
		msg = "UCONVFILE: Missing encoding2";
	}

	//invent a temporary filename
	var tempfilename = inputfilename;
	tempfilename.splicer(-3, 3, "$CP");

	cmd ^= " -f windows=" ^ encoding1 ^ " -t " ^ encoding2;

	cmd ^= " -o " ^ tempfilename;
	cmd ^= " " ^ inputfilename;

	//run the conversion command
	result = shell2(cmd, errors);
	if (errors) {
		msg = "UCONVFILE: uconv " ^ errors;
		return 0;
	}

	//overwrite the input file with the temporary
	cmd = "xcopy " ^ tempfilename ^ " " ^ inputfilename ^ " /y";
	result = shell2(cmd, errors);
	if (errors) {
		msg = "UCONVFILE: xcopy " ^ errors;
		return 0;
	}

	//delete the temporary
	tempfilename.osdelete();

	result = 1;

	return 0;

}


libraryexit()