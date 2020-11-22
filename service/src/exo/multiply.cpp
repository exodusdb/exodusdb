#include <exodus/library.h>
libraryinit()

function main(in multipliers, in multiplicand, in sep) {
	//c sys
	//linemark
	if (multipliers == "" or multiplicand == 1) {
		return multipliers;
	}
	dim output;
	var nn=output.split(multipliers.convert(sep,FM));
	for (int ii=1;ii<=nn;++ii) {
		//return null if null
		//return zero if multiplicand is zero
		if (output(ii)) {
			output(ii)=output(ii)*multiplicand;
		}
	}
	return output.join().convert(FM,sep);
}


libraryexit()
