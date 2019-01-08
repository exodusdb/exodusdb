#include <exodus/library.h>
libraryinit()

#include <openfile.h>

var autocreate;//num
var datasetcode;
var reply;

function main(in filename, io file, in similarfilename="", in autocreate0="") {
	//c sys in,io,"",""
	if (autocreate0.unassigned()) {
		autocreate = 1;
	}else{
		autocreate = autocreate0;
	}
	var firsttry = 1;
tryagain:

	if (openfile("*" ^ filename, file, datasetcode)) {
		return 1;
	}

	if (firsttry) {
		//user option to create file if it does not exist
		var tt;
		if (tt.open("FILES", "")) {
			if (tt.read(tt, similarfilename)) {
				var vol = tt.a(1);
				if (not autocreate) {
					tt = "THE " ^ (DQ ^ (filename ^ DQ)) ^ " FILE DOES NOT EXIST|DO YOU WANT TO CREATE IT ?";
					if (not(decide(tt, "No" _VM_ "Yes", reply))) {
						var().stop();
					}
					if (reply ne 2) {
						return 0;
					}
				}
				var cmd = vol ^ " DATA " ^ filename ^ " (S)";
				execute("MAKEFILE " ^ cmd);
				execute("ATTACH " ^ cmd);
				firsttry = 0;
				goto tryagain;
			}
		}
	}

	var().chr(7).output();
	call mssg("THE " ^ (DQ ^ (filename ^ DQ)) ^ " FILE IS MISSING");
	file = "";
	return 0;

}


libraryexit()