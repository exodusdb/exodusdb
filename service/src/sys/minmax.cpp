#include <exodus/library.h>
libraryinit()

var bit;
var term;

function main(in list0, out min, out max) {

	//find the min and max in a list

	//linemark

	//initialise min and max to the first element
	var result = "";
	var start = 0;
	bit=list0.substr2(start, term);
	if (bit.len()) {
		min = bit;
		max = bit;
	}

	while (true) {
		///BREAK;
		if (not term) break;
		bit=list0.substr2(start, term);
		if (bit.len()) {
			if (bit lt min) {
				min = bit;
			}
			if (bit gt max) {
				max = bit;
			}
		}
	}//loop;

	return result;
}

libraryexit()
