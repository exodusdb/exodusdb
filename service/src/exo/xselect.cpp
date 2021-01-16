#include <exodus/library.h>
libraryinit()


function main(in cmd) {
	//c sys
	//if @username='EXODUS' then oswrite cmd on 'XSELECT.'
	//oswrite cmd on 'XSELECT.'
	var supersilent = index(cmd,"(SS");
	if (SYSTEM.a(33) and not supersilent) {
		printl();
		print(cmd, ":");
	}
	select(cmd);
	if (SYSTEM.a(33) and not supersilent) {
		printl("done");
	}
	return 0;

}


libraryexit()
