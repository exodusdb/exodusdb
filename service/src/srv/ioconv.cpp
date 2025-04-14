#include <exodus/library.h>
libraryinit()

// TODO replace all occurrences in client code and obsolete this function

func main(in type, in input0, in mode, io output) {

	// just wraps the standard iconv oconv
	// so that we can find "[IOCONV,MT" etc in the source code instead of "MT"

	if (input0 == "") {
		output = "";
		return 0;
	}

	if (type == "OCONV") {
		output = oconv(input0, mode);

	} else if (type == "ICONV") {
		output = iconv(input0, mode);
	}

	return 0;
}

}; // libraryexit()
