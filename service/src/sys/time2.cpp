#include <exodus/library.h>
libraryinit()

#include <time2.h>

var mode;
var outputx;

function main(in type, in input0, in mode0, out output) {
	//c sys in,in,in,out

	//uses @sw to output to or input from "local time"
	//NO LONGER since requires date to be effective (use [DATETIME])

	if (input0 eq "") {
		output = "";
		return 0;
	}
	if (mode0) {
		mode = mode0;
	} else {
		mode = "MT";
	}

	if (type eq "OCONV") {

		if (mode.contains("48")) {
			var modex = mode;
			modex.replacer("48", "");
			if (modex eq "") {
				modex = "MTS";
			}
			output = oconv(input0, modex);
			if (input0 ge 86400) {
				output = output.fieldstore(":", 1, 1, (output.field(":", 1) + 24).oconv("R(0)#2"));
			}
		} else {
			//modex=mode
			//swap '48' with '' in modex
			//if modex='' then modex='MT'
			//output=OCONV(INPUT0,modex)
			//output=OCONV(INPUT0,mode)
			//output=OCONV(mod(INPUT0+@sw<1>,86400),mode)
			output = oconv(input0.mod(86400), mode);
		}

		//25 hour format (25:00 is 01:00 the next day)
		//output=oconv(INPUT0,'MT.')
		//if INPUT0>86340 then output=(output+24) 'R(0)#5'
		//convert '.' to ':' in output

	//mode eg 48MT etc completely ignored in iconv
	} else if (type eq "ICONV") {

		//handle multivalues
		if (input0.contains(VM)) {
			output = "";
			let nn = input0.count(VM) + 1;
			for (const var ii : range(1, nn)) {
				call time2(type, input0.f(1, ii), mode, outputx);
				pickreplacer(output, 1, ii, outputx);
			} //ii;
			return 0;
		}

		var input2 = input0;
		input2.converter(":.", "::");
		var temp = input2.field(":", 1);
		if (temp.isnum() and temp ge 24) {
			input2 = input2.fieldstore(":", 1, 1, temp - 24);
			//output=iconv(input2,'MT')
			//output=mod(iconv(input2,'MT')-@sw<1>,86400)
			output = input2.iconv("MT").mod(86400);
			output += 86400;
		} else {
			//modex=mode
			//swap '48' with '' in modex
			//if modex else modex='MT'
			//output=iconv(INPUT0,modex)
			//output=mod(iconv(INPUT0,'MT')-@sw<1>,86400)
			output = input0.iconv("MT").mod(86400);
		}

	}

	return 0;
}

libraryexit()
