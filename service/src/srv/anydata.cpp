#include <exodus/library.h>
libraryinit()

var v69;
var v70;
var v71;
var filenamesx;
var filename;
var nrecs;	// num

func main(in filenames0, out result) {

	// $insert bp,agency.common

	call pushselect(v69);

	if (filenames0) {
		filenamesx = filenames0;
	} else {
		filenamesx = "PLANS" _VM "SCHEDULES" _VM "JOBS" _VM "BALANCES";
	}

	let nfiles = filenamesx.fcount(VM);

	for (const var filen : range(1, nfiles)) {

		filename = filenamesx.f(1, filen);
		gosub getnrecs();
		if (nrecs) {
			result = 1;
			goto exit;
		}

	}  // filen;

	result = 0;

exit:
	call popselect(v69);
	return 0;
}

subr getnrecs() {
	nrecs = "";
	var file;
	if (not file.open(filename, "")) {
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
