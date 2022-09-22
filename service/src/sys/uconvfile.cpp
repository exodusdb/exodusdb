#include <exodus/library.h>
libraryinit()

#include <shell2.h>

#include <sys_common.h>

var encoding1;
var encoding2;
var exe;
var ii;
var encoding;
var errors;

function main(in inputfilename, in encoding1i, in encoding2i, out result, out msg) {
	//c sys in,in,in,out,out

	//currently only used by convcsv

	//uses the unicode uconv to convert file format
	//
	//http://www.microsoft.com/globaldev/reference/cphome.mspx
	#include <system_common.h>

	if (encoding1i.unassigned()) {
		encoding1 = "";
	} else {
		encoding1 = encoding1i;
	}
	if (encoding2i.unassigned()) {
		encoding2 = "";
	} else {
		encoding2 = encoding2i;
	}
	result = 0;

	//make cygwin command
	//look for local or cygwin wget.exe otherwise quit
	if (oscwd().contains(":")) {
		exe = ".exe";
	} else {
		exe = "";
	}
	var cmd = "uconv" ^ exe;
	if (not(cmd.osfile())) {
		cmd.prefixer(SYSTEM.f(50));
	}
	if (not(cmd.osfile())) {
		msg = "UCONVFILE: Cannot find " ^ cmd;
		//indicate failed but because of lack of uconv.exe file
		result = "";

		return 0;
	}

	//determine the encoding1
	if (encoding1 eq "CODEPAGE") {
		call osgetenv("CODEPAGE", encoding1);
		var oemcodepages = "437" _VM "720" _VM "737" _VM "775" _VM "850" _VM "852" _VM "855" _VM "857" _VM "858" _VM "862" _VM "866" _VM "874" _VM "932" _VM "936" _VM "949" _VM "950" _VM "1258";
		var wincodepages = "1252" _VM "1256" _VM "1253" _VM "1257" _VM "1252" _VM "1252" _VM "1251" _VM "1254" _VM "1252" _VM "1255" _VM "1251" _VM "874" _VM "932" _VM "936" _VM "949" _VM "950" _VM "874";
		if (oemcodepages.locate(encoding1, ii)) {
			encoding1 = wincodepages.f(1, ii);
		}
	}
	if (not encoding1) {
		msg = "UCONVFILE: Missing encoding1";
		return 0;
	}

	//dont convert if latin
	//hopefully to reduce chance of screwups/reduce filesize when latin
	if (encoding1 eq 1252) {
		result = 1;
		return 0;
	}

	//determine the encoding2
	if (encoding2 eq "CODEPAGE") {
		call osgetenv("CODEPAGE", encoding2);
	}
	if (not encoding2) {
		msg = "UCONVFILE: Missing encoding2";
	}

	if (encoding1 ne encoding) {

		//invent a temporary filename
		var tempfilename = inputfilename;
		tempfilename.paster(-3, 3, "$CP");

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
		if (VOLUMES) {
			cmd = "xcopy " ^ tempfilename ^ " " ^ inputfilename ^ " /y";
		} else {
			cmd = "cp " ^ tempfilename ^ " " ^ inputfilename;
		}
		result = shell2(cmd, errors);
		if (errors) {
			msg = "UCONVFILE: xcopy " ^ errors;
			return 0;
		}

		//delete the temporary
		tempfilename.osremove();

	}

	result = 1;

	return 0;
}

libraryexit()
