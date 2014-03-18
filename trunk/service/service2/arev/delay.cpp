#include <exodus/library.h>
libraryinit()

#include <keypressed.h>

function main() {
	//checking time() triggers windows loop detector and avoids 100% cpu
	//checking keypressed enables user interrupt without disabling loop detector
	var time = var().time();
	var time2 = time + delaysecs;
	while (true) {
		if (keypressed()) {
			return 0;
		}
	///BREAK;
	if (not(var().time() >= time and var().time() < time2)) break;;
	}//loop;

	//see also timedelay() which uses 100% processor time
	//keypressed() is a very short assembly routine

	//should compile with line marks perhaps to make sure not too tight
	//for windows auto suspend to handle?
	//without is
	//COMPILE GBP DELAY (CL)

}

               return 0;
               }

libraryexit()