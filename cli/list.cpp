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
		// perform returns false on abort (see ExoProgram::perform / exoprog_callsmf)
		if (not perform("n" ^ SENTENCE)) {
			if (listid)
				deletelist("default");
			abort(lasterror());
		}

		if (listid)
			deletelist("default");

		return 0;
	}

	//////////////////////////////////////////
	// OR 2. re-execute within an output pager
	//////////////////////////////////////////

	var oscmd = "";

	// Reconstruct the list command for bash (see bash -c below).
	// If word has any shell special characters or spaces
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

				// Single quotes inside single quotes: '"'"'
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

	// Stream list into pager (keeps concurrency / low latency; no full temp file).
	// pipefail so a failing list is not masked by pager exit 0.
	// osshell uses system() → /bin/sh (usually bash but could be dash);
	// dash has no pipefail, so run the pipeline under bash -c explicitly.
	oscmd = "set -o pipefail; " ^ oscmd ^ " | pager --chop-long-lines --quit-if-one-screen";
	oscmd = "bash -c " ^ oscmd.squote();

	// osshell — child already printed any error (e.g. unrecognized word)
	if (not osshell(oscmd))
		return 1;

	return 0;
}

}; // programexit()
