#include <exodus/library.h>
libraryinit()

#include <openfile.h>

var datasetcode;
var reply;

function main() {
	//jbase linemark
	if (autocreate.unassigned()) {
		autocreate = 1;
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
				var("MAKEFILE " ^ cmd).execute();
				var("ATTACH " ^ cmd).execute();
				firsttry = 0;
				goto tryagain;
			}
		}
	}

	cout << var().chr(7);
	call mssg("THE " ^ (DQ ^ (filename ^ DQ)) ^ " FILE IS MISSING");
	file = "";
	return 0;

}

               return 0;
               }

libraryexit()