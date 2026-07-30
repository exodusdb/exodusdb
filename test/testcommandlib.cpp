#include <exodus/library.h>
libraryinit()

func main() {
	// OPTIONS (A) — abort so callers can test: if (not perform(...)) ...
	if (OPTIONS.contains("A"))
		abort("testcommandlib aborted for test");

	//USER1 = SENTENCE.convert(FM, "^") ^ "\n" ^ COMMAND.convert(FM, "^") ^ "\n" ^ OPTIONS ^ "\n";
	USER1 = SENTENCE.convert(FM, "^") ^ "\n" ^ COMMAND ^ "\n" ^ OPTIONS ^ "\n";
	// Truthy so if (not perform(...)) is only true on abort (0/"" are falsy)
	return true;
}

}; // libraryexit()
