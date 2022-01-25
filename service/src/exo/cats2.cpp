#include <exodus/library.h>
libraryinit()

function main(in list1, in infix, in list2) {

	//joins multivalue lists in parallel with a separator

	//list1 = a vm b
	//infix = :
	//list2 = c vm d vm e
	//output =
	//a : c vm b : d vm : e

	var nl1=list1.dcount(VM);
	var nl2=list2.dcount(VM);
	var nn=nl2>nl2?nl2:nl1;
	var output="";
	for (var ii : range(1, nn)) {
		output^=list1.a(1,ii) ^ infix ^ list2.a(1,ii) ^ VM;
	}
	output.splicer(-1,1,"");
	return output;
}

libraryexit()
