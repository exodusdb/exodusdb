#include <exodus/library.h>
libraryinit()

#include <giveway.h>
#include <delay.h>


var v3;
var buffer;
var reply;

function main() {
	var v1 = "";
	if (not PLAYBACK) {
		DATA.transfer(v1);
		var v2 = "";

		//possibly give way and escape if running too long
		var notinteractive = SYSTEM.a(33);
		if (notinteractive) {
			if (giveway("")) {
				return 1;
			}
		}
		v3.input(-1, 2);
		if (not(v3 == "")) {
			//common /system/ system
			//notinteractive=system<33>
			if (notinteractive) {
				SYSTEM.r(33, "");
				}
			if (v3 == INTCONST.a(1) or v3 == INTCONST.a(7) or v3 == var().chr(3)) {
				var printing = GETPRINTER();
				var hfactive = _HFACTIVE;
				var PAGENO = PAGE;
				var BREAK = BREAK;
				SYSE3_13 = 0;
				PRINTER OFF;
				if (v3 == INTCONST.a(7)) {
mainmenu:
					var("RUNMENU " ^ ENVIRONSET.a(37)).execute();
					if (printing) {
						PRINTER ON;
					}
					SYSE3_13 = hfactive;
					PAGE = PAGENO;
					BREAK = BREAK;
					if (notinteractive) {
						SYSTEM.r(33, notinteractive);
					}
					return 0;
				}
				cout << var().chr(7);
				call msg2("You have pressed the [Esc]  key to exit,|press again to confirm,|or press the space bar to resume.", "UB", buffer, "");
				while (true) {
					reply.input(-1, 2);
					call delay(1);
				///BREAK;
				if (reply) break;;
				}//loop;
				call msg2("", "DB", buffer, "");
				if (reply == INTCONST.a(7)) {
					goto mainmenu;
				}
				if (printing) {
					PRINTER ON;
				}
				SYSE3_13 = hfactive;
				PAGE = PAGENO;
				BREAK = BREAK;
				if (notinteractive) {
					SYSTEM.r(33, notinteractive);
				}
				if (reply == 0x1B) {
					return 1;
				}else{
					return 0;
				}
			}else{
				v2 ^= v3;
			}
			if (notinteractive) {
				SYSTEM.r(33, notinteractive);
			}
			goto 77;
		}
		if (v2.length()) {
			if (CAPTURE) {
				SCRIPT ^= v2;
			}
			v1 ^= v2;
		}
		v1.transfer(DATA);
	}
	return 0;

}


libraryexit()