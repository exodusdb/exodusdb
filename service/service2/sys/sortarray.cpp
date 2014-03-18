#include <exodus/library.h>
libraryinit()

//function main(io array, in fns, in order0="", in justification0="") {
function main(io array, in fns, in order="A", in justification="L") {

	//sorts one or more parallel mved fields in a record
	//WARNING cannot have character zero in the data

	//array is the record which the field(s) to be sorted are present
	//fns is a mv array of parallel field numbers to be sorted
	//order is D or A (can be repeated to sort for multiple sorts per fn)
	//eg AA would sort the first two fns ascending
	//justification is L or R for left (text) or right (numberic)

	//TODO only sorts on first field at the moment

	var orderby=order^justification;
//printl(orderby);

	var nfns = fns.dcount(VM);
	var fn;

	var sortfn=fns.a(1,1);
	var sortablefield=array.a(sortfn);
	var sortedfield="";

	//TODO speed up from current slow insertion sort

	//save current array
	var oarray=array;

	var nv=sortablefield.dcount(VM);
	var tt;
	for (var fnn=2;fnn<=nfns;++fnn) {
		fn=fns.a(1,fnn);

		//clear existing field
		array.r(fn,"");

		//work out maximum mv and clear fields
		tt=oarray.a(fn).dcount(VM);
		if (tt>nv)
			nv=tt;
	}

	var element;
	var newvn;
	for (var oldvn=1;oldvn<=nv;++oldvn){
		element=sortablefield.a(1,oldvn);

		sortedfield.locateby(element,orderby,newvn,1);

//printt(sortedfield,tt,orderby,newvn);
		sortedfield.inserter(1,newvn,element);
//printt(sortedfield.oconv("HEX"));printl();

		for (var fnn=2;fnn<=nfns;++fnn) {
			fn=fns.a(1,fnn);
			array.inserter(fn,newvn,oarray.a(fn,oldvn));
		}
	}

	array.r(sortfn,sortedfield);
	return 0;

}


libraryexit()
