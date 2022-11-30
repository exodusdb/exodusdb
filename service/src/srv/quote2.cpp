#include <exodus/library.h>
libraryinit()

function main(in string0) {
	if (not string0.len()) {
		return string0;
	}

	var string1 = string0;

	string1.converter(DQ, "'");
	string1.replacer(SM, "\" \"");
	string1.replacer(VM, "\" \"");
	string1.replacer(FM, "\" \"");

	return string1.quote();
}

libraryexit()
