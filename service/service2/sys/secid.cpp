#include <exodus/library.h>
libraryinit()

function main(in input, in max) {
	//c sys in,in
	var inputx = input;
	inputx.converter(" ~!\"$%^&*()_+|{}:@<>?#-=\\[];,./" "\'", "");
	inputx.ucaser();
	//return (inputx.invert()).hash(max, 0);
	return inputx.hash(max.toLong());

}


libraryexit()
