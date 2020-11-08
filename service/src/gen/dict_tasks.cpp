#include <exodus/library.h>

libraryinit(authorised)
//---------------------
#include <validcode2.h>

var brands;
var msg;

function main() {
	var valid = 1;
	var job = xlate("JOBS", RECORD.a(1), "", "X");
	if (not(validcode2(job.a(14), "", job.a(2), brands, msg))) {
		valid = 0;
	}
	return valid;
}
libraryexit(authorised)
