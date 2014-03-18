#include <exodus/library.h>
libraryinit()

#include <field2.h>


var filename1;
var filename2;
var xx;
var msg;

function main() {
	//jbase
	if (datasetcode.unassigned()) {
		datasetcode = "";
	}
	var nomsg = filename.substr(1, 1) == "*";
	if (nomsg) {
		filename.splicer(1, 1, "");
	}
	if (filename.substr(1, 4) == "DICT") {
		filename1 = "DICT";
		filename2 = filename.substr(6, 9999);
	}else{
		filename1 = "";
		filename2 = filename;
	}
open:
	var filevar;
	if (filevar.open(filename1, filename2)) {

		//for JBASE
		if (not(FILES[0].locateUsing(filename, FM, xx))) {
			FILES[0].r(-1, filename);
		}

		//check if in otherdataset
		if (datasetcode) {
			var otherfile = filevar;
			otherfile.swapper("DATA\\" ^ system.system.a(17) ^ "\\", "DATA\\" ^ datasetcode ^ "\\");
			var osfile = (field2(otherfile, VM, -1)).substr(14, 9999);
			if (osfile.osfile()) {
				filevar = otherfile;
			}else{
				if (not nomsg) {
					if (("..\\DATA\\" ^ datasetcode ^ "\\GENERAL\\REVMEDIA.LK").osfile()) {
						msg = DQ ^ (osfile ^ DQ) ^ " file is missing";
					}else{
						msg = DQ ^ (datasetcode ^ DQ) ^ " database is missing";
					}
					cout << var().chr(7);
					call mssg(msg);
				}
				return 0;
			}
		}

		var file = filevar;
		return 1;

	}else{
		if (filename2 == "VOC") {
			filename2 = "MD";
			goto open;
		}
		if (not nomsg) {
			cout << var().chr(7);
			call mssg("The " ^ (DQ ^ (filename ^ DQ)) ^ " file is missing");
		}
		var file = "";
		return 0;
	}

}

               return 0;
               }

libraryexit()