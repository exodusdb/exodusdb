#include <exodus/library.h>
libraryinit()

#include <gen_common.h>

var xx;
var origusername;

function main(in cmd, in username) {
	//c sys

	#include <system_common.h>

	if (username) {
		if (not(SECURITY.a(1).locate(username, xx))) {
			call mssg(username.quote() ^ " user does not exist in executeas()");
			return 0;
		}
		origusername = USERNAME;
		var newusername = username ^ "(" ^ USERNAME ^ ")";
		USERNAME=(newusername);
	}

	///////////
	execute(cmd);
	///////////

	if (username) {
		USERNAME=(origusername);
	}

	return 0;
}

libraryexit()
