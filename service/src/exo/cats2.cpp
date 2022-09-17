#include <exodus/library.h>
libraryinit()

function main(in list1, in infix, in list2) {

	//joins multivalue lists in parallel with a separator

	//list1 = a vm b
	//infix = :
	//list2 = c vm d vm e
	//output =
	//a : c vm b : d vm : e

	var nl1=list1.fcount(VM);
	var nl2=list2.fcount(VM);
	var nn=nl2>nl2?nl2:nl1;
	var output="";
	for (const var ii : range(1, nn)) {
		output^=list1.f(1,ii) ^ infix ^ list2.f(1,ii) ^ VM;
	}
	output.popper();
	return output;
}

libraryexit()
