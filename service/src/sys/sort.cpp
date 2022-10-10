#include <exodus/library.h>
libraryinit()

function main() {

	//redirect to execute/perform 'list'
	SENTENCE.prefixer("LIST ");

	perform(SENTENCE);

	return 0;
}

libraryexit()
