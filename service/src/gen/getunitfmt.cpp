#include <exodus/library.h>
libraryinit()

#include <gen_common.h>

var fmtn;
var ndecs;//num

function main(in code, out fmtx) {
	//c gen in,out
	//jbase
	#include <general_common.h>

	if (gen.accounting.a(4).locate(code,fmtn)) {
		fmtx = gen.accounting.a(5, fmtn);
		return 0;
	}

	var unitx;
	if (unitx.read(gen.currencies, code)) {
		ndecs = unitx.a(3);
	}else{
		ndecs = "";
		//doesnt seem to work so hack out for the moment
		//if unassigned(units) else
		// if units then read unitx from units,code then ndecs=unitx<5>
		// end
		//if ndecs='' then
		// if @username='NEOSYS' then call msg(quote(code):' currency code is missing')
		// end
	}

	if (not(ndecs.length())) {
		ndecs = BASEFMT[3];
		if (not(ndecs.match("^\\d$"))) {
			ndecs = 2;
		}
	}

	fmtx = "MD" ^ ndecs ^ "0P";
	gen.accounting.r(4, fmtn, code);
	gen.accounting.r(5, fmtn, fmtx);

	return 0;
}

libraryexit()
