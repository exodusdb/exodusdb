#include <exodus/library.h>
libraryinit()

var string1;

func main(in string0) {
//	if (string0.unassigned()) {
//		// de bug
//		string1 = "UNASSIGNED";
//	} else {
//		string1 = string0;
//	}
	string1 = string0.or_default("UNASSIGNED");

	string1.replacer("&", "&amp;");
	string1.replacer(DQ, "&quot;");
	string1.replacer("<", "&lt;");
	string1.replacer(">", "&gt;");

	string1.converter(DQ, "'");
	string1.replacer(VM, "\" \"");
	string1.replacer(FM, "\" \"");

	return string1.quote();
}

libraryexit()
