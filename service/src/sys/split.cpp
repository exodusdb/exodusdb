#include <exodus/library.h>
libraryinit()

function main(in input0, out unitx) {
	//c sys in,out

	unitx = input0;
	unitx.converter(" -.0123456789", "");

	var inputx = input0;
	//convert ' ABCDEFGHIJKLMNOPQRSTUVWXYZ' to '' in inputx
	//patsalides db PT0303 base currency code contains $,
	//must be removed otherwise B16 in daybook.subs4
	inputx.converter(" ABCDEFGHIJKLMNOPQRSTUVWXYZ$", "");

	return inputx;
}

libraryexit()
