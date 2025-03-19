#include <exodus/program.h>
programinit()

func main() {
	let cmd = SENTENCE.field(" ", 2, 9999);
	if (not cmd)
		printl("Syntax is call libraryname - main function MUST have no arguments");
	perform(cmd);
	if (len(ANS))
		printl(ANS);
	return 0;
}

programexit()
