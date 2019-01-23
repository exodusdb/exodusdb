#include <exodus/library.h>
libraryinit()

var sep;

function main(in input) {

	//rather poor implementation at the moment
	//ASSUMES FM seps

	dim temp;
	var nfields=temp.parse(input);
	dim temp2(nfields);
	for (int ii=1;ii<=nfields;ii++) {
		temp2(nfields-ii+1)=temp(ii);
	}
	return temp2.unparse();

/*
	//c sys
	//linemark
	if (input.length() == 0) {
		return input;
	}

	if (input.index(FM, 1)) {
		sep = FM;
	} else if (input.index(VM, 1)) {
		sep = VM;
	} else if (input.index(SVM, 1)) {
		sep = SVM;
	} else if (input.index(TM, 1)) {
		sep = TM;
	} else if (input.index(STM, 1)) {
		sep = STM;
	} else {
		return input;
	}
//L127:
	var output = input.substr(-1,"B" ^ sep);
	while (true) {
	///BREAK;
	if (not COL1) break;;
		output ^= sep ^ input.substr(COL1 - 1,"B" ^ sep);
		//print col1():',':col2()
	}//loop;

	return output;
*/

}


libraryexit()
