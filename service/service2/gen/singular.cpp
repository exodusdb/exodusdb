#include <exodus/library.h>
libraryinit()

function main(in pluralnoun) {
	//c gen
	var temp = pluralnoun;
	var temp2 = pluralnoun;
	temp2.ucaser();

	if (temp2.substr(-2,2) == "ES") {

		//COMPANIES=COMPANY
		if (temp2.substr(-3,3) == "IES") {
			temp.splicer(-3, 3, "Y");

		//ADDRESSES=ADDRESS
		} else if (temp2.substr(-4,4) == "SSES") {
			temp.splicer(-2, 2, "");
		} else if (temp2.substr(-4,4) == "SHES") {
			temp.splicer(-2, 2, "");
		} else if (temp2.substr(-4,4) == "CHES") {
			temp.splicer(-2, 2, "");
		} else {
			temp.splicer(-1, 1, "");
		}
//L151:
		//comment to help c++ decompiler
	}else{

		if (temp2[-1] == "S") {
			//ANALYSIS, DOS
			if (temp2.substr(-2,2) ne "IS" and temp2.substr(-2,2) ne "OS") {
				temp.splicer(-1, 1, "");
			}
		}

	}

	return temp;

}


libraryexit()
