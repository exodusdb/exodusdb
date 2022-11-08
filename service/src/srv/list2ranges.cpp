#include <exodus/library.h>
libraryinit()

function main(io listitems) {

	// convert vm list of numbers to list of numbers and ranges
	// eg 1,2,3,6,8,9,10 -> 1-3,6,8-10 where commas represent vms

	// can handle fixed decimal places
	// eg 2020.09,2020.10 -> 2020.09-2020.10

	call sortarray(listitems, 1, "AR");

	var nn = listitems.fcount(VM);

	// convert single periods to single period ranges
	for (const var ii : range(1, nn)) {
		listitems(1, ii, 2) = listitems.f(1, ii, 1);
	}  // ii;

	// merge ranges if consecutive
	// working backwards deleting merged ranges
	for (var ii = nn; ii >= 2; --ii) {
		var listitem1 = listitems.f(1, ii - 1, 2);
		var listitem2 = listitems.f(1, ii, 1);
		// if (listitem1+0.01)=listitem2 then
		listitem1.converter(".", "");
		listitem2.converter(".", "");
		if (listitem1 + 1 eq listitem2) {
			listitems(1, ii - 1, 2) = listitems.f(1, ii, 2);
			listitems.remover(1, ii);
			nn -= 1;
		}
	}  // ii;

	// convert single period ranges to single periods
	for (const var ii : range(1, nn)) {
		let tt	= listitems.f(1, ii);
		let tt1 = tt.f(1, 1, 1);
		if (tt1 eq tt.f(1, 1, 2)) {
			listitems(1, ii) = tt1;
		}
	}  // ii;

	listitems.converter(SM, "-");

	return 0;
}

libraryexit()
