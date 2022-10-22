#include <exodus/program.h>
programinit()

//#include "nlist.h"
//#include <exodus/nlist.h>

function main() {

	/////////////////////////////////////////////////////////////////
	// EITHER 1. Call/perform nlist library if paging is not required
	/////////////////////////////////////////////////////////////////

	if (not TERMINAL or OPTIONS.contains("N")) {

		//call nlist();

		//TRACE(SENTENCE)
		// Change list into nlist in order to call the nlist library
		var result = perform("n" ^ SENTENCE);
		return result;
	}

	//////////////////////////////////////////
	// OR 2. re-execute within an output pager
	//////////////////////////////////////////

	// Switch off TERMINAL
	var oscmd = "TERM=";

	// Prepare a bash command
	// Reconstruct the list command into a format suitable for processing by bash
	// If word has any bash special characters or spaces
	// then escape any single quotes and wrap in single quotes
	for (var word : COMMAND) {

		if (word[1] eq DQ and word[-1] eq DQ) {
			word.squoter();
		}
		else if (word[1] eq SQ and word[-1] eq SQ) {
			word.quoter();
		}
		else {
			var word2 = word.convert(R"( `~!@#$%^&*(){}[]:;'"|<>?\)" to "");
			if (word2.len() < word.len()) {

				// Single quotes inside single quotes can be represented as '"'"' for bash
				// The leading and trailng aingle quotes in '"'"' terminate and resume the single quoted string
				// so 'abc'"'"'def' is actually abc'def
				word.replacer("'", R"('"'"')");

				word.squoter();
			}
		}

		// Escape any ( ) or { }
		if (var("(){}").contains(word))
			word = _BS ^ word;

		oscmd ^= " " ^ word;
	}

	// Add any options
	if (OPTIONS)
		oscmd ^= " {"^ OPTIONS ^ "}";

	// Pipe into pager
	oscmd ^= " | pager --chop-long-lines --quit-if-one-screen";

	if (not osshell(oscmd))
		abort(lasterror());

	return 0;
}

programexit()
