#include <exodus/library.h>
libraryinit()

var bit;
var term;

function main(in list0, out min, out max) {
	//c sys in,out,out

	//find the min and max in a list

	//linemark

	//initialise min and max to the first element
	var result = "";
	var start = 0;
	bit=list0.substr2(start, term);
	if (bit.length()) {
		min = bit;
		max = bit;
	}

	while (true) {
		///BREAK;
		if (not term) break;
		bit=list0.substr2(start, term);
		if (bit.length()) {
			if (bit < min) {
				min = bit;
			}
			if (bit > max) {
				max = bit;
			}
		}
	}//loop;

	return result;
}

libraryexit()
