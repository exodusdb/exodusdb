#include <exodus/library.h>
libraryinit()

function main(in input0, in max) {

	//use the max as part of the randomisation so that
	//it acts as "salt" and slightly different max produce very different results
	var inputx = max ^ input0;

	//insensitive to ascii punctuation and spaces
	inputx.converter(" ~!\"$%^&*()_+|{}:@<>?#-=\\[];,./" "\'", "");

	//insensitive to case
	inputx.ucaser();

	//use the cleaned in to initialise the random number generator
	inputx.initrnd();

	//and return the first random number generated, up to the max required
	return rnd(max);

}


libraryexit()
