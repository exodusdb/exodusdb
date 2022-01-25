#include <exodus/library.h>
libraryinit()

function main(io listitems) {
	//c sys io

	//convert vm list of numbers to list of numbers and ranges
	//eg 1,2,3,6,8,9,10 -> 1-3,6,8-10 where commas represent vms

	//can handle fixed decimal places
	//eg 2020.09,2020.10 -> 2020.09-2020.10

	call sortarray(listitems, 1, "AR");

	var nn = listitems.count(VM) + 1;

	//convert single periods to single period ranges
	for (var ii : range(1, nn)) {
		listitems(1, ii, 2) = listitems.a(1, ii, 1);
	} //ii;

	//merge ranges if consecutive
	//working backwards deleting merged ranges
	for (var ii = nn; ii >= 2; --ii) {
		var listitem1 = listitems.a(1, ii - 1, 2);
		var listitem2 = listitems.a(1, ii, 1);
		//if (listitem1+0.01)=listitem2 then
		listitem1.converter(".", "");
		listitem2.converter(".", "");
		if (listitem1 + 1 eq listitem2) {
			listitems(1, ii - 1, 2) = listitems.a(1, ii, 2);
			listitems.remover(1, ii);
			nn -= 1;
		}
	} //ii;

	//convert single period ranges to single periods
	for (var ii : range(1, nn)) {
		var tt = listitems.a(1, ii);
		var tt1 = tt.a(1, 1, 1);
		if (tt1 eq tt.a(1, 1, 2)) {
			listitems(1, ii) = tt1;
		}
	} //ii;

	listitems.converter(SVM, "-");

	return 0;
}

libraryexit()
