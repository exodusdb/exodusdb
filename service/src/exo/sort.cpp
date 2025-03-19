#include <exodus/library.h>
libraryinit()

func main() {

	// redirect to cli 'nlist' library
	SENTENCE.prefixer("NLIST ");

	perform(SENTENCE);

	return 0;
}

libraryexit()
