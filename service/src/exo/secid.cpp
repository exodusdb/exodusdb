#include <exodus/library.h>
libraryinit()

function main(in input0, in max) {

	var inputx;
	var maxx;
	var iscmd;
	if (SENTENCE.field(" ", 1) == "secid") {
		iscmd  = true;
		inputx = SENTENCE.field(" ", 2);
		maxx   = SENTENCE.field(" ", 3);
		if (not inputx or maxx == "" or not maxx.isnum()) {
			errputl("Syntax is 'secid TEXT NUMBER'\nTEXT must have no spaces.");
			stop();
		}
	} else {
		iscmd  = false;
		inputx = input0;
		maxx   = max;
	}

	// use the max as part of the randomisation so that
	// it acts as "salt" and slightly different max produce very different results
	inputx = maxx ^ inputx;

	// insensitive to ascii punctuation and spaces
	inputx.converter(
		" ~!\"$%^&*()_+|{}:@<>?#-=\\[];,./"
		"\'",
		"");

	// insensitive to case
	inputx.ucaser();

	// use the cleaned in to initialise the random number generator
	inputx.initrnd();

	// return the first random number generated, up to the max required

	if (iscmd) {
		printl(rnd(maxx));
		return 0;
	}

	return rnd(maxx);
}

libraryexit()
