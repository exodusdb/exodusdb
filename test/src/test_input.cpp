#include <exodus/program.h>
programinit()

let osfn1 = "t_bytes.bin";
var tx;

func main() {

	// Self test
	if (not COMMAND.contains("TEST")) {

		// Create a 256 byte file containing bytes 0 to 255
		tx = "";
		for (var cn : range(0, 255)) {
			tx ^= chr(cn);
		}
		if (not oswrite(tx on osfn1))
			abort(lasterror());

		// Dump file for interest
		printl(osfn1);
		if (not osshell("xxd " ^ osfn1))
			loglasterror();

		// 1. Self test with inputn(10) gets 10 bytes at a time
		// Needs care with UTF8
		self_test(10);

		// 2. Self test with inputn(0) gets all
		// UTF8 NO PROBLEM
		self_test(0);

		// 3. Self test with input gets it line by line WITHOUT nl
		// UTF8 NO PROBLEM
		self_test("input");

//		// 4. inputn(1) waits until a key is pressed and returns its value or code
//		self_test(1);

//		// 5. inputn(-1) gets a single key press if any key is pressed or "" if none
//		self_test(-1);

		printl("Test passed.");
		stop();

	}

////////////////////////////////////////////////////////
// Actual test of input from stdout and output to stdout
////////////////////////////////////////////////////////

	var mode = COMMAND.f(3);

	if (mode == "input") {

		// input()
		while (not eof()) {

			var bin = input();

			// with new line
			outputl(bin);
		}
	} else {

		// inputn()
		for (;;) {
			var bin = inputn(mode);
			if (not bin.len())
				break;

			// No new line
			output(bin);
		}
	}

	return 0;
}

func self_test(in mode) {

	// Call self to read and generate the output file using mode
	let osfn2 = osfn1 ^ "2";
	let cmd = EXECPATH ^ " TEST " ^ mode ^ " < " ^ osfn1 ^ " 1> " ^ osfn2;
	printl();
	printl(cmd);
	if (not osshell(cmd))
		abort(lasterror());

	// get the output file
	var tx2;
	if (not osread(tx2 from osfn2))
		abort(lasterror());

	// Remove the trailing \n added by the input/outputl loop
	bool ignore_trailing_nl = mode == "input";
	if (ignore_trailing_nl)
		tx2.popper();

	var ignoring = ignore_trailing_nl ? " (ignoring an additional trailing nl)" : "";

	// Quit if not the same
	if (tx2 ne tx) {
		printl(osfn2 ^ " is different from " ^ osfn1 ^ " but should not be.", ignoring);
		if (not osshell("xxd " ^ osfn2))
			loglasterror();
		abort();
	} else
		printl(osfn2 ^ " is identical.", ignoring);

	return true;

}

programexit()
