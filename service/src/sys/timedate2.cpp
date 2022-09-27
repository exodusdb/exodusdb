#include <exodus/library.h>
libraryinit()

#include <getdatetime.h>

#include <system_common.h>

#include <sys_common.h>

var localdate;
var localtime;
var x3;
var x4;
var x5;
var x6;

function main(in localdate0, in localtime0) {
	//c sys

	//caserevised*

	//use parameters only if both are provided
	if (localtime0.unassigned()) {
		call getdatetime(localdate, localtime, x3, x4, x5, x6);
	} else {
		localdate = localdate0;
		localtime = localtime0;
	}

	var temp = "";
	if (not temp) {
		temp = "MTH";
	}
	temp = oconv(localtime, temp);
	if (temp.starts("0")) {
		temp.cutter(1);
	}
	if (temp.ends("AM")) {
		temp.paster(-2, 2, sys.glang.f(16));
	}
	if (temp.ends("PM")) {
		temp.paster(-2, 2, sys.glang.f(17));
	}

	if (DATEFMT eq "") {
		DATEFMT = "D/E";
	}
	temp.prefixer(oconv(localdate, "[DATE,*4]") ^ " ");

	return temp;
}

libraryexit()
