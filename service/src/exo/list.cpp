#include <exodus/library.h>
libraryinit()

function main() {

	// redirect to cli 'nlist' library
	SENTENCE.prefixer("N");

	perform(SENTENCE);

	return 0;
}

libraryexit()
