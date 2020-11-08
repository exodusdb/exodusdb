#include <exodus/library.h>
libraryinit()

#include <videorw.h>

function main(in mode, in time, in date, out buffer, out msg) {
	//c sys in,in,in,out,out

	var interactive = not(SYSTEM.a(33));
	if (msg.unassigned()) {
		msg = "";
	}

	//gosub gettext
	var text = elapsedtimetext(date, time);

	var result = "";
	if (mode) {
		text.swapper(", ", FM);
	}else{
		//SWAP @FM WITH ', ' IN TEXT
		var msgx = msg;
		if (msgx) {
			msgx ^= "||";
		}
		result = msgx ^ "Finished in " ^ text;
		if (interactive) {
			call note(result ^ "|What next boss ?");
		}
	}

	if (not(buffer.unassigned())) {
		call videorw(0, 0, CRTWIDE - 1, CRTHIGH - 1, "W", buffer);
	}

	return result;
}

libraryexit()
