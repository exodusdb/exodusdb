#include <exodus/library.h>
libraryinit()

function main() {
	//c gen

	var title = "List of Companies     \'T\'";
	var cmd = "SORT COMPANIES WITH AUTHORISED BY-DSND CURRENT_YEAR BY COMPANY_NAME HEADING " ^ (title.quote());
	perform(cmd);

	return 0;
}

libraryexit()
