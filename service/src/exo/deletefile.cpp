#include <exodus/library.h>
libraryinit()

func main() {
	// printl("deletefile says 'Hello World!'");
	//deletefile(field(COMMAND, _FM, 2, 999));
	if (not deletefile(field(COMMAND, _FM, 2, 999))) {
		loglasterror();
	}
	return 0;
}

}; // libraryexit()
