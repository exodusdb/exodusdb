#include <exodus/library.h>
libraryinit()

function main(in input0, out unitx) {
	//c gen in,out

	unitx = input0;
	unitx.converter(" -.0123456789", "");

	var inputx = input0;
	inputx.converter(" ABCDEFGHIJKLMNOPQRSTUVWXYZ", "");

	return inputx;
}

libraryexit()
