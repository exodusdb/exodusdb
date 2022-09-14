#include <exodus/library.h>
libraryinit()

var sep;

function main(in input0) {

	//rather poor implementation at the moment
	//ASSUMES FM seps

	dim temp;
	var nfields=temp.split(input0);
	dim temp2(nfields);
	for (int ii = 1; ii <= nfields; ii++) {
		temp2(nfields-ii+1)=temp(ii);
	}
	return temp2.join();

/*
	//c sys
	//linemark
	if (input0.len() == 0) {
		return input0;
	}

	if (input0.contains(FM, 1)) {
		sep = FM;
	} else if (input0.contains(VM, 1)) {
		sep = VM;
	} else if (input0.contains(SM, 1)) {
		sep = SM;
	} else if (input0.contains(TM, 1)) {
		sep = TM;
	} else if (input0.contains(ST, 1)) {
		sep = ST;
	} else {
		return input0;
	}
//L127:
	var output = input0.substr(-1,"B" ^ sep);
	while (true) {
	///BREAK;
	if (not COL1) break;;
		output ^= sep ^ input0.substr(COL1 - 1,"B" ^ sep);
		//print col1():',':col2()
	}//loop;

	return output;
*/

}


libraryexit()
