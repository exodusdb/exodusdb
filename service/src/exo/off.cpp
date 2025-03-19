#include <exodus/library.h>
libraryinit()

// OBSOLETE replaced by logoff(); everywhere

func main() {
	// printl("Goodbye");
	// exit(0);
	throw ExoLogoff("Logoff");
	//std::unreachable();
}

libraryexit()
