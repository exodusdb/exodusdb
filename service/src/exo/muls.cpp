#include <exodus/library.h>
libraryinit()

function main(in multipliers1, in multipliers2, in sep) {

	if (multipliers1 == "" and multipliers2 == "") {
		return multipliers1;
	}
	dim array1;
	dim array2;

	var nn1=array1.split(multipliers1.convert(sep,FM));
	var nn2=array2.split(multipliers2.convert(sep,FM));

	var min_nn=nn2>nn1?nn1:nn2;
	var max_nn=nn2>nn1?nn2:nn1;

	dim output(max_nn);

	//remember is 1 based

	//deal with the common elements first for speed
	for (int ii=1;ii<=min_nn;++ii) {
		//return nulls where both nulls
		//otherwise multiply, treating nulls as zeros
		if (array1(ii).length() and array2(ii).length()) {
			output(ii)=array1(ii)*array2(ii);
		} else
			output(ii)="";
	}

	//deal with the remaining elements which must all be null or zero
	for (int ii=min_nn+1;ii<=max_nn;++ii) {
		//return nulls where nulls
		//return zeros otherwise
		if (ii>nn1) {
			//array2 is longer than array1
			if (array2(ii).length())
				output(ii)=0;
			else
				output(ii)="";
		} else {
			//array1 is longer than array2
			if (array1(ii).length())
				output(ii)=0;
			else
				output(ii)="";
		}
	}
	return output.join().convert(FM,sep);
}

libraryexit()
