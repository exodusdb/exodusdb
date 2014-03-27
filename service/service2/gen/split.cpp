#include <exodus/library.h>
libraryinit()

function main(in input0, out unitx ) {

	var inputx=input0;

	inputx.converter(" ", "");
	var temp = inputx;
	var char1 = temp[1];
	if (char1 == "-") {
		temp.splicer(1, 1, "");
	}
	//convert '0123456789-.,' to ' ' in temp
	temp.converter("0123456789.,", "            ");
	//numlen=len(temp)-len(trimf(temp))
	var numlen = temp.length() - (temp.trimf()).length() + (char1 == "-");
	var numx = inputx.substr(1, numlen);
	if (numx == "-") {
		numx = "";
	}
	unitx = inputx.substr(numlen + 1, 99);

	//convert to decimal format
	numx.converter(",", ".");
	var nn = numx.count(".");
	for (var ii = 1; ii <= nn - 1; ++ii) {
		var tt = numx.index(".", 1);
		numx.splicer(tt, 1, "");
	};//ii;

	return numx;

}


libraryexit()
