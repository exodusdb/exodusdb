#include <exodus/program.h>
programinit()

function main() {
	var cmd=SENTENCE.substr(2);
	var file,key;
	file.select(cmd);
	if (file.readnext(key)) {
		key.outputl("first key=");
	} else {
		printl("Nothing selected");
	}
	return 0;
}

programexit()

