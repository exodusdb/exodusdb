#include <exodus/library.h>
libraryinit()

function main() {
	//c gen

	var cmd = "LIST ADDRESSES ID-SUPP";

	cmd ^= " BY COMPANY";

	//cmd:=' COMPANY ADDRESS COUNTRY TEL FAX EMAIL ADDRESS_REF'

	cmd ^= " HEADING " ^ (var("LIST OF ADDRESSES   \'T\'").quote());

	perform(cmd);

	return 0;
}

libraryexit()
