#include <exodus/library.h>
libraryinit()

var string1;

function main(in string0) {
	//c sys
	if (string0.unassigned()) {
		//de bug
		string1 = "UNASSIGNED";
	}else{
		string1 = string0;
	}

	string1.swapper("&", "&amp;");
	string1.swapper(DQ, "&quot;");
	string1.swapper("<", "&lt;");
	string1.swapper(">", "&gt;");

	string1.converter(DQ, "'");
	string1.swapper(VM, "\" \"");
	string1.swapper(FM, "\" \"");
	return string1.quote();
}

libraryexit()
