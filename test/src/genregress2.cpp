#include <exodus/program.h>
programinit()

func main() {

	// This program can be used to generate files in OLDER versions of exodus
	// These files can be used in test_regress in NEWER versions of exodus
	//
	// Currently only implements paster tests
	//
	// The existing regression test files in exodus/test/data were generated in AREV not in exodus
	// in order to verify that exodus has some degree of Pick OS compatibility

	// SPLICER1 and SPLICER2 - paster(var,pos,str) and paster(var,pos,len,str)
	////////////////////////

	var n1 = 3;
	var n2 = 5;
	for (const var nchars_source : range(0, n1)) {
		var source = substr("123456789", 1, nchars_source);

		for (const var nchars_insert : range(0, n1)) {
			var insert = substr("abcdefghi", 1, nchars_insert);

			for (const var ii : range(-n2, n2)) {

				var expression = "paste(" ^ source.quote() ^ "," ^ ii ^ "," ^ insert.quote() ^ ")";
				//TRACE(expression);
				//var value      = paste(source,ii,insert));
				var value = source;
				paster(value,ii,insert);
				//TRACE(value);
				printt(value, "SPLICER1", source, ii, "", insert);
				printl();

				for (const var jj : range(-n2, n2)) {

					var expression = "paste(" ^ source.quote() ^ "," ^ ii ^ "," ^ jj ^ "," ^ insert.quote() ^ ")";
					//TRACE(expression);
					var value      = paste(source,ii,jj,insert);
					//TRACE(value);
					printt(value, "SPLICER2", source, ii, jj, insert);
					printl();

				}

			}
		}
	}

	return 0;
}

programexit()
