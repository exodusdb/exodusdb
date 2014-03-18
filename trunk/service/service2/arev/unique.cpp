#include <exodus/library.h>
libraryinit()

var bit;
var term;
var xx;

function main() {
	//linemark
	var result = "";
	var start = 0;
	while (true) {
		bit=list0.remove(start, term);
		if (bit.length()) {
			if (not(result.locateusing(bit, VM, xx))) {
				result ^= bit ^ VM;
			}
		}
	///BREAK;
	if (not term) break;;
	}//loop;
	result.splicer(-1, 1, "");
	return result;

}


libraryexit()