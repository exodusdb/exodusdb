#include <exodus/library.h>
libraryinit()

#include <videorw.h>

#include <service_common.h>

#include <srv_common.h>

var mode;

function main(in mode0, in time, in date, io buffer, out msg) {

	if (msg.unassigned()) {
		msg = "";
	}
	if (mode0.unassigned()) {
		mode = "";
	} else {
		mode = mode0;
	}

	if (time.unassigned() or date.unassigned()) {
		return 0;
	}

	// gosub gettext
	var text = elapsedtimetext(date.timestamp(time), timestamp());

	printl();

	var result = "";
	if (mode) {
		text.replacer(", ", FM);
	} else {
		// SWAP @FM WITH ', ' IN TEXT
		var msgx = msg;
		if (msgx) {
			msgx ^= "||";
		}
		result = msgx ^ "Finished in " ^ text;
		if (interactive_) {
			call note(result ^ "|What next boss ?");
		}
	}

	if (not(buffer.unassigned())) {
		call videorw(0, 0, CRTWIDE - 1, CRTHIGH - 1, "W", buffer);
	}

	return result;
}

libraryexit()
