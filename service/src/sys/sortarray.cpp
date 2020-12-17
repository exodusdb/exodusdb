#include <exodus/library.h>
libraryinit()

var orderby;
var newvn;

function main(io array, in fns=0, in orderby0="") {
	//c sys io,0,""

	//sorts one or more parallel mved fields in a record
	//WARNING cannot have character zero in the data

	//array is the record which the field(s) to be sorted are present
	//fns is a mv array of parallel field numbers to be sorted
	//order is D or A (can be repeated to sort for multiple sorts per fn)
	//eg AA would sort the first two fns ascending
	//justification is L or R for left (text) or right (numberic)

	//MULTIPLE SORTS NOT SUPPORTED CURRENTLY
	//MULTIPLE FIELDS WILL BE SORTED IN PARALLEL ON THE FIRST FIELD ONLY

	if (orderby0.unassigned()) {
		orderby = "AL";
	}else{
		orderby = orderby0;
	}

	var nfns = fns.count(VM) + 1;

	var sortfn = fns.a(1, 1);
	var unsorted = array.a(sortfn);
	var sorted = "";

	//insert into a new array without other fields for speed
	var newarray = "";
	var nv = unsorted.count(VM) + (unsorted ne "");
	for (var vn = 1; vn <= nv; ++vn) {
		var value = unsorted.a(1, vn);
		if (not(sorted.locateby(orderby,value,newvn))) {
			{}
		}
		sorted.inserter(1, newvn, value);

		//insert any parallel fields
		for (var fnn = 2; fnn <= nfns; ++fnn) {
			var fn = fns.a(1, fnn);
			var othervalue = array.a(fn, vn);
			newarray.inserter(fn, newvn, othervalue);
		};//fnn;

	};//vn;

	array.r(sortfn, sorted);

	//put any parallel fields back into the original array
	for (var fnn = 2; fnn <= nfns; ++fnn) {
		var fn = fns.a(1, fnn);
		array.r(fn, newarray.a(fn));
	};//fnn;

	return 0;
}

libraryexit()
