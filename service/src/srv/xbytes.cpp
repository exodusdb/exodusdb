#include <exodus/library.h>
libraryinit()

var output2;
var ndecs;	// num

function main(in type, in input0, in mode, out output) {

	if (mode) {
		let sigfigs = mode;
	} else {
		let sigfigs = 3;
	}

	if (type eq "OCONV") {
		output = input0;
		if (output lt 1024) {
			output ^= " bytes";
		} else {
			output = output / 1024;
			if (output lt 1024) {
				output2 = "KiB";
			} else {
				output = output / 1024;
				if (output lt 1024) {
					output2 = "MiB";
				} else {
					output	= output / 1024;
					output2 = "GiB";
				}
			}
			if (output lt 1) {
				ndecs = 3;
			} else if (output lt 10) {
				ndecs = 2;
			} else if (output lt 100) {
				ndecs = 1;
			} else {
				ndecs = 0;
			}
			output = output.oconv("MD" ^ ndecs ^ "0P") ^ output2;
		}

	} else if (type eq "ICONV") {
		// not implemented yet
		output = input0;

	} else {
		output = input0;
	}

	return 0;
}

libraryexit()
