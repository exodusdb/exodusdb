#include <exodus/library.h>

libraryinit(authorised)
//---------------------
#include <authorised.h>

function main() {
	return authorised("PRODUCT CATEGORY ACCESS " ^ (ID.quote()));
}
libraryexit(authorised)
