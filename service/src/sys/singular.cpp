#include <exodus/library.h>
libraryinit()

function main(in pluralnoun) {
	//c gen
	var temp = pluralnoun;
	var temp2 = pluralnoun;
	temp2.ucaser();

	if (temp2.ends("ES")) {

		//COMPANIES=COMPANY
		if (temp2.ends("IES")) {
			temp.paster(-3, 3, "Y");

		//ADDRESSES=ADDRESS
		} else if (temp2.ends("SSES")) {
			temp.paster(-2, 2, "");
		} else if (temp2.ends("SHES")) {
			temp.paster(-2, 2, "");
		} else if (temp2.ends("CHES")) {
			temp.paster(-2, 2, "");
		} else {
			temp.popper();
		}

	//comment to help c++ decompiler
	}else{

		if (temp2.ends("S")) {
			//ANALYSIS, DOS
			if ((temp2.last(2) ne "IS") and (temp2.last(2) ne "OS")) {
				temp.popper();
			}
		}

	}

	return temp;
}

libraryexit()
