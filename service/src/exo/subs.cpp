#include <exodus/library.h>
libraryinit()

function main(in args1, in args2, in sep) {

	if (args1 == "" and args2 == "") {
		return args1;
	}
	//	dim array1;
	//	dim array2;
	//	var nn1=array1.split(args1.convert(sep,FM));
	//	var nn2=array2.split(args2.convert(sep,FM));

	dim array1 = args1.split(sep);
	dim array2 = args2.split(sep);
	int nn1	   = array1.rows();
	int nn2	   = array2.rows();

	int min_nn = nn2 > nn1 ? nn1 : nn2;
	int max_nn = nn2 > nn1 ? nn2 : nn1;

	dim output(max_nn);

	//remember is 1 based

	//deal with the common elements first for speed
	for (int ii = 1; ii <= min_nn; ++ii) {
		//subtract arg2, if not null, from arg1
		if (array2(ii).len())
			output(ii) = array1(ii) - array2(ii);
		else
			output(ii) = array1(ii);
	}

	//deal with the remaining elements which must all be null or zero
	for (int ii = min_nn + 1; ii <= max_nn; ++ii) {
		if (ii > nn1) {
			//array2 is longer than array1
			if (array2(ii).len())
				output(ii) = -array2(ii);
			else
				output(ii) = "";
		} else {
			//array1 is longer than array2
			output(ii) = array1(ii);
		}
	}
	return output.join(sep);
}

libraryexit()
