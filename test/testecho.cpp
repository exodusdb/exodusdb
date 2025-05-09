#include <exodus/library.h>
libraryinit()

function main() {
//	printl(THREADNO ^ ": echo says 'Hello World!'");
	// Loop echoing input and close if receive empty string.
	for (;;) {
		var v1;
		mv.input_queue->wait_and_pop(v1);
		if (v1.empty())
			break;
		mv.output_queue->push(123.34);
//		mv.output_queue->push(std::move(v1));
	}
//	printl(THREADNO ^ ": echo says 'Goodbye World!'");
	return 0;
}

}; // }; // libraryexit
