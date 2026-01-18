#include <exodus/program.h>
programinit()

//#include "nlist.h"
//#include <exodus/nlist.h>

func main() {

	/////////////////////////////////////////////////////////////////
	// EITHER 1. Call/perform nlist library if paging is not required
	/////////////////////////////////////////////////////////////////

	if (not TERMINAL or OPTIONS.contains("N")) {

		// TODO Consume the list immediately.
		// and dont rely on calling deletelist in exit which will not be called in case of abort
		var listid = "";
		if (not locate("lists", COMMAND)) {
			listid = "default";
			if (not getlist(listid))
				listid = "";
			else
				logputl("Using select list ", listid);
		}

		// Change list into nlist in order to call the nlist library
		let result = perform("n" ^ SENTENCE);

		if (listid)
			deletelist("default");

		return result;
	}

	//////////////////////////////////////////
	// OR 2. re-execute within an output pager
	//////////////////////////////////////////

	var oscmd = "";

	// Prepare a bash command
	// Reconstruct the list command into a format suitable for processing by bash
	// If word has any bash special characters or spaces
	// then escape any single quotes and wrap in single quotes
	for (var word : COMMAND) {

		if (word.starts(DQ) and word.ends(DQ)) {
			word.squoter();
		}
		else if (word.starts(SQ) and word.ends(SQ)) {
			word.quoter();
		}
		else {
			let word2 = word.convert(R"( `~!@#$%^&*(){}[]:;'"|<>?\)" to "");
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
			word = _BSL ^ word;

		oscmd ^= word ^ " ";
	}

	// Add any options and NOPAGE option
	oscmd ^= "{N"^ OPTIONS ^ "}";

	// Pipe into pager
	oscmd ^= " | pager --chop-long-lines --quit-if-one-screen";

	// osshell
	var result = osshell(oscmd);
	var lasterrorx = lasterror();

	if (not result)
		abort(lasterrorx);

	return 0;
}

}; // programexit()
