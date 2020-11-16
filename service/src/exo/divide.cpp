#include <exodus/library.h>
libraryinit()

function main(in numerators, in denominator, in sep) {
	//c sys
	//linemark
	if (numerators == "" or denominator == 1) {
		return numerators;
	}
	dim output;
	var nn=output.split(numerators.convert(sep,FM));
	for (int ii=1;ii<nn;++ii) {
		//null or zero elements remain null or zero
		if (output(ii)) {
			output(ii)=output(ii)/denominator;
		}
	}
	return output.join().convert(FM,sep);
}


libraryexit()
