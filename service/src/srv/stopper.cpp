#include <exodus/library.h>
#include <srv_common.h>

libraryinit()

#include <service_common.h>

var mode;

func main(in mode0, in time, in date, io /*buffer*/, out msg) {

	msg.defaulter("");
	mode = mode0.or_default("");

	if (time.unassigned() or date.unassigned()) {
		return 0;
	}

	var text = elapsedtimetext(date.ostimestamp(time), ostimestamp());

	printl();

	var result = "";
	if (mode) {
		text.replacer(", ", FM);
	} else {
		var msgx = msg;
		if (msgx) {
			msgx ^= "||";
		}
		result = msgx ^ "Finished in " ^ text;
		if (interactive_) {
			call note(result ^ "|What next boss ?");
		}
	}

	return result;
}

}; // libraryexit()
