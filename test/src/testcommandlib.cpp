#include <exodus/library.h>
libraryinit()

func main() {
	//USER1 = SENTENCE.convert(FM, "^") ^ "\n" ^ COMMAND.convert(FM, "^") ^ "\n" ^ OPTIONS ^ "\n";
	USER1 = SENTENCE.convert(FM, "^") ^ "\n" ^ COMMAND ^ "\n" ^ OPTIONS ^ "\n";
	return 0;
}

libraryexit()

