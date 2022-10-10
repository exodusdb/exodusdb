#include <exodus/program.h>
programinit()

//#include "list.h"

function main() {

	//////////////////////////////////////////////////////
	// call/perform list library if paging is not required
	//////////////////////////////////////////////////////

	if (not TERMINAL or OPTIONS.contains("N")) {
		//call list();
		//TRACE(SENTENCE)
		return perform(SENTENCE);
	}

	///////////////////////////////////////
	// Or re-execute within an output pager
	///////////////////////////////////////

	// Switch off TERMINAL
	var oscmd = "TERM=";

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

	//TRACE(oscmd)
	return osshell(oscmd);

}

programexit()
