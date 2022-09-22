#include <exodus/library.h>
libraryinit()

function main(in type, in in0, in mode, out out0) {
	//c sys in,in,in,out

	//used in timesheets which stores duration as decimal hours internally
	//eg 1.5 internally shows as 1:30 externally ie one hour and 30 minutes

	//input decimal hours, output hours:mins but allow more than 24 (eg 30:00)
	//iconv  is the reverse

	var in2 = in0;
	out0 = "";

	//call msg(in2:'')

	//Z option suppresses zeros, returns ''
	if (mode.contains("Z")) {
		if (not in0) {
			out0 = "";
			return 0;
		}
	}

	if (type eq "ICONV") {

		//convert '.' to ':' in in2

		if (in2.isnum()) {
			out0 = in2;
			return 0;
		}

		var hours = in2.field(":", 1);
		var mins = in2.field(":", 2);

		//check numeric
		if (not(hours.isnum() and mins.isnum())) {
			STATUS = 2;
			return 0;
		}

		out0 = hours + mins / 60;

	} else {
		if (not(in2.isnum())) {
			out0 = "";
			STATUS = 2;
			return 0;
		}
		out0 = (in2.field(".", 1) + 0) ^ ":";
		var temp = in2.field(".", 2);
		if (temp) {
			temp.prefixer(".");
		}
		temp = (temp * 60).oconv("MD00P");
		out0 ^= temp.oconv("R(0)#2");

	}

	return 0;
}

libraryexit()
