#include <exodus/library.h>
#include <srv_common.h>

libraryinit()

#include <service_common.h>


//var xx;
var origusername;

func main(in cmd, in username) {

	if (username) {
		var tt;
		if (not SECURITY.locate(username, tt, 1)) {
			call note(username.quote() ^ " user does not exist in executeas()");
			return 0;
		}
		origusername	= USERNAME;
		let newusername = username ^ "(" ^ USERNAME ^ ")";
		USERNAME		= (newusername);
	}

	// /////////
	execute(cmd);
	// /////////

	if (username) {
		USERNAME = (origusername);
	}

	return 0;
}

}; // libraryexit()
