#include <exodus/library.h>
libraryinit()


function main(in cmd) {
	//c sys
	//if @username='NEOSYS' then oswrite cmd on 'XSELECT.'
	//oswrite cmd on 'XSELECT.'
	if (SYSTEM.a(33)) {
		printl();
		print(cmd, ":");
	}
	select(cmd);
	if (SYSTEM.a(33)) {
		print("done");
	}
	return 0;

}


libraryexit()
