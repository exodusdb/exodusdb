#include <exodus/library.h>

libraryinit(other_system_names)
//-----------------------------
function main() {
	return calculate("OTHER_SYSTEM_CODES").xlate("SYSTEMS", 1, "X");
}
libraryexit(other_system_names)
