#include <exodus/library.h>
libraryinit()

function main() {
	// Called by generalproxy/SYSTEST only
	abort(COMMAND.field(FM, 2, 9999));
	return 0;
}
};
