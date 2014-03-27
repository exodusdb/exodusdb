#include <exodus/program.h>
programinit()

function main() {
	var cmd=SENTENCE.substr(2);
	var file,key;
	file.select(cmd);

	var nkeys=0;
	var keys="";
	while (file.readnext(key)) {
		if (nkeys) {
			keys^="\n";
		}
		nkeys++;
		keys^=key;
	}
	var result;
	if (nkeys) {
		oswrite(keys,"sselect.lst");
		printl(nkeys^" record(s) selected");
	} else {
		printl("No records found");
	}
	return nkeys==0;//0=success/1=failure
}

programexit()

