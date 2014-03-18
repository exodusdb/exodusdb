#include <exodus/library.h>
libraryinit()

#include <v119.h>

var ff;
var flag;

function main(io array, io fns, io order, io justification) {

	//sorts one or more parallel mved fields in a record
	//WARNING cannot have character zero in the data

	//array is the record which the field(s) to be sorted are present
	//fns is a mv array of parallel field numbers to be sorted
	//order is D or A (can be repeated to sort for multiple sorts per fn)
	//eg AA would sort the first two fns ascending
	//justification is L or R for left (text) or right (numberic)

	if (order.unassigned()) {
		order = "A";
	}
	if (justification.unassigned()) {
		justification = "L";
	}

	var nfns = fns.count(VM) + 1;

	var ff(nfns);//TODO:var ff(nfns);
	ff.init("");

	//get the initial field to sort
	ff[1] = array.a(fns.a(1, 1));
	var DATA = ff[1];

	//attach any other associated fields
	var n1 = (ff[1]).count(VM) + (ff[1] ne "");
	for (var fnn = 2; fnn <= nfns; ++fnn) {
		var fn = fns.a(1, fnn);
		ff[fnn] = array.a(fn);

		//work out the maximum number of values
		var nn = n1;
		var n2 = (ff[fnn]).count(VM) + (ff[fnn] ne "");
		if (n2 > nn) {
			nn = n2;
		}

		DATA = DATA  (var().chr(0) ^ VM).str(nn - 1) ^ var().chr(0)  ff[fnn];
	};//fnn;

	//add the required terminating ff
	DATA ^= RM;
	//and turn from 00 fd array into fe ff array
	DATA.converter(VM ^ 0x00, RM ^ FM);

	//call the fast native code sorting engine
	call v119("S", "", order, justification, DATA, flag);

	if (nfns > 1) {

		ff.init("");

		var pos = 1;
		var lendata = DATA.length();

		while (true) {
		///BREAK;
		if (not(pos < lendata)) break;;

			//extract the next sorted chunk of fields
			var temp = DATA.substr(pos, RM);
			pos = _COL2 + 1;

			//build up the original fields
			for (var fnn = 1; fnn <= nfns; ++fnn) {
				var fn = fns.a(1, fnn);
				ff[fnn] ^= temp.a(fnn) ^ VM;
			};//fnn;

		}//loop;

		//replace the original fields
		for (var fnn = 1; fnn <= nfns; ++fnn) {
			(ff[fnn]).splicer(-1, 1, "");
			array.r(fns.a(1, fnn), ff[fnn]);
		};//fnn;

	}else{
		//array=data[1,len(data)-1]
		//convert @rm to @vm in array
		DATA.splicer(-1, 1, "");
		DATA.converter(RM, VM);
		array.r(fns.a(1, 1), DATA);
	}

	return 0;

}


libraryexit()