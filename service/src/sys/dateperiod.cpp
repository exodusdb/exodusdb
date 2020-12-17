#include <exodus/library.h>
libraryinit()

#include <addcent4.h>

var period;//num
var year;//num

function main(in type, in input0, in mode, out output) {
	//c sys in,in,in,out

	var firstmonth = mode.field(",", 1);
	var maxperiod = mode.field(",", 2);

	//if iconv then convert period (MM/YY or YYMM) to internal last date of month
	if (type == "ICONV") {
		//return the last day of the period (internal format)
		if (input0.index("/")) {
			period = input0.field("/", 1) + 1;
			year = input0.field("/", 2);
		}else{
			year = input0.substr(1,2);
			period = input0.substr(-2,2) + 1;
		}
		if (firstmonth and firstmonth.isnum()) {
			period += firstmonth - 1;
		}
		if (period > 12) {
			period -= 12;
			year = (year + 1).oconv("R(0)#2");
		}else{
			if (period < 1) {
				period += 12;
				year = (addcent4(year - 1)).oconv("R(0)#2");
			}
		}

		if (firstmonth >= 7) {
			year -= 1;
		}

		output = ("1/" ^ period ^ "/" ^ ("00" ^ year).substr(-2,2)).iconv("DE") - 1;
		return 0;
	}

	//if oconv then convert internal date to year:period
	var temp = input0.oconv("D2-E");
	year = temp.substr(-2,2);
	period = temp.substr(4,2);
	if (firstmonth and firstmonth.isnum()) {
		period -= firstmonth - 1;
		if (period < 1) {
			period += 12;
			year = (addcent4(year - 1)).oconv("R(0)#2");
		}
		period = ("00" ^ period).substr(-2,2);
	}

	if (year and (firstmonth >= 7)) {
		year += 1;
	}

	output = ("00" ^ year).substr(-2,2) ^ period;

	return 0;
}

libraryexit()
