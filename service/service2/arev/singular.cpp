#include <exodus/library.h>
libraryinit()

function main() {
	//jbase linemark
	var temp = pluralnoun;
	var temp2 = pluralnoun;
	temp2.converter(LOWERCASE, UPPERCASE);

	if (temp2.substr(-2, 2) == "ES") {

		//COMPANIES=COMPANY
		if (temp2.substr(-3, 3) == "IES") {
			temp.splicer(-3, 3, "Y");

			//ADDRESSES=ADDRESS
			goto 150;
		}
		if (temp2.substr(-4, 4) == "SSES") {
			temp.splicer(-2, 2, "");
			goto 150;
		}
		if (temp2.substr(-4, 4) == "SHES") {
			temp.splicer(-2, 2, "");
			goto 150;
		}
		if (temp2.substr(-4, 4) == "CHES") {
			temp.splicer(-2, 2, "");
			goto 150;
		}
		if (1) {
			temp.splicer(-1, 1, "");
		}
L150:
	}else{

		if (temp2.substr(-1, 1) == "S") {
			//ANALYSIS, DOS
			if (temp2.substr(-2, 2) ne "IS" and temp2.substr(-2, 2) ne "OS") {
				temp.splicer(-1, 1, "");
			}
		}

	}

	return temp;

}


libraryexit()