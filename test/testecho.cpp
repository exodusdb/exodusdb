#include <exodus/library.h>
libraryinit()

function main() {
//	printl(THREADNO ^ ": echo says 'Hello World!'");
	// Loop echoing input and close if receive empty string.
	for (;;) {
		var v1;
		ev.input_queue->wait_and_pop(v1);
		if (v1.empty())
			break;
//		ev.output_queue->push(123.34);
//		ev.output_queue->push(std::move(v1));
		ev.output_queue->push(v1.move());
	}
//	printl(THREADNO ^ ": echo says 'Goodbye World!'");
	return 0;
}

}; // }; // libraryexit
