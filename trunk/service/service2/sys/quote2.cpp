#include <exodus/library.h>
libraryinit()

function main(in string0) {
	//jbase linemark
	if (not len(string0))
		return string0;
	var string1 = string0;
	string1.converter(DQ, "\'");
	string1.swapper(SVM, "\" \"");
	string1.swapper(VM, "\" \"");
	string1.swapper(FM, "\" \"");
	return string1.quote();

}


libraryexit()
