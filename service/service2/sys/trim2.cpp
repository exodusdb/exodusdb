#include <exodus/library.h>
libraryinit()

function main(in string1, in char1, in fb=var()) {
	//c sys in,in,=var()

	//#jbase
	var string2 = string1;
	if (char1.length() ne 1) {
		var xx = xx;
	}
	if (fb.unassigned()) {
		goto trimall;

	} else if (fb == "F") {
		while (true) {
		///BREAK;
		if (not(string2[1] == char1)) break;;
			string2.splicer(1, 1, "");
		}//loop;

	} else if (fb == "B") {
		while (true) {
		///BREAK;
		if (not(string2[-1] == char1)) break;;
			string2.splicer(-1, 1, "");
		}//loop;

	} else if (fb == "FB") {
		while (true) {
		///BREAK;
		if (not(string2[1] == char1)) break;;
			string2.splicer(1, 1, "");
		}//loop;
		while (true) {
		///BREAK;
		if (not(string2[-1] == char1)) break;;
			string2.splicer(-1, 1, "");
		}//loop;

	} else if (1) {
trimall:
		string2.converter(char1 ^ " ", " " ^ char1);
		string2.trimmer();
		string2.converter(char1 ^ " ", " " ^ char1);
	}
	return string2;

}


libraryexit()