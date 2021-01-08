#include <exodus/library.h>
libraryinit()

function main(in type, in input, in mode, io output) {
	//c sys in,in,in,io

	//just wraps the standard iconv oconv
	//so that we can find "[IOCONV,MT" etc in the source code instead of "MT"

	if (input eq "") {
		output = "";
		return 0;
	}

	if (type eq "OCONV") {
		output = oconv(input, mode);

	} else if (type eq "ICONV") {
		output = iconv(input, mode);
	}

	return 0;
}

libraryexit()
