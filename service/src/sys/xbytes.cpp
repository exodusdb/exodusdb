#include <exodus/library.h>
libraryinit()

var output2;
var ndecs;//num

function main(in type, in input0, in mode, out output) {
	//c sys in,in,in,out

	if (mode) {
		var sigfigs = mode;
	}else{
		var sigfigs = 3;
	}

	if (type == "OCONV") {
		output = input0;
		if (output < 1024) {
			output ^= " bytes";
		}else{
			output = output / 1024;
			if (output < 1024) {
				output2 = "KiB";
			}else{
				output = output / 1024;
				if (output < 1024) {
					output2 = "MiB";
				}else{
					output = output / 1024;
					output2 = "GiB";
				}
			}
			if (output < 1) {
				ndecs = 3;
			} else if (output < 10) {
				ndecs = 2;
			} else if (output < 100) {
				ndecs = 1;
			} else {
				ndecs = 0;
			}
			output = output.oconv("MD" ^ ndecs ^ "0P") ^ output2;
		}

	} else if (type == "ICONV") {
		//not implemented yet
		output = input0;

	} else {
		output = input0;
	}

	return 0;
}

libraryexit()
