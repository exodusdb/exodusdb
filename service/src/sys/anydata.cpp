#include <exodus/library.h>
libraryinit()

var v69;
var v70;
var v71;
var filenamesx;
var filename;
var nrecs;//num

function main(in filenames0, out result) {
	//c sys in,out

	//$insert bp,agency.common

	call pushselect(0, v69, v70, v71);

	if (filenames0) {
		filenamesx = filenames0;
	}else{
		filenamesx = "PLANS" _VM_ "SCHEDULES" _VM_ "JOBS" _VM_ "BALANCES";
	}

	var nfiles = filenamesx.count(VM) + 1;

	for (var filen = 1; filen <= nfiles; ++filen) {

		filename = filenamesx.a(1, filen);
		gosub getnrecs();
		if (nrecs) {
			result = 1;
			goto exit;
		}

	};//filen;

	result = 0;

exit:
	call popselect(0, v69, v70, v71);
	return 0;
}

subroutine getnrecs() {
	nrecs = "";
	var file;
	if (not(file.open(filename, ""))) {
		return;
	}
	nrecs = file.reccount();
	if (not nrecs) {
		select(file);
		if (readnext(ID)) {
			nrecs = 1;
		}
		clearselect();
	}
	return;
}

libraryexit()
