#include <exodus/program.h>
programinit()

func main() {

	if (not connect())
		lasterror().errputl();

	printl(dblist().convert(FM, "\n"));

	return 0;
}

}; // programexit()
