#include <exodus/library.h>
libraryinit()

#include <gen_common.h>

function main(in line1, in width, out addresstext) {
	//c gen in,in,out

	#include <general_common.h>
	addresstext = "";

	if (line1 and line1 ne "STATEMENT") {
		addresstext.r(-1, calculate(line1));
	}

	addresstext.r(-1, gen.address.a(1));
	addresstext.r(-1, gen.address.a(4));
	//IF ADDRESS<8> NE company<23> THEN ADDRESSTEXT<-1>=ADDRESS<8>
	//really should suppressed if same as main market code name
	addresstext.r(-1, gen.address.a(8));

	//value marks only
	addresstext.converter(FM, VM);

	//trim blank lines
	addresstext.converter(" " _VM_, _VM_ " ");
	addresstext.trimmer();
	addresstext.converter(_VM_ " ", " " _VM_);

	//maximum width
	if (width and width.isnum()) {
		var tt = width - 1;
		var fmt = "T#" ^ tt;
		addresstext = oconv(addresstext, fmt).trim();
		addresstext.swapper(TM, _VM_ " ");
	}

	return 0;
}

libraryexit()
