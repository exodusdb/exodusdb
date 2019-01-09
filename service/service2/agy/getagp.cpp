#include <exodus/library.h>
libraryinit()

#include <addcent.h>

function main(in agpparams, in period0) {
	//c agy
	//output selected agpparam

	if (agpparams == "") {
		return "";
	}

	var nn = agpparams.count(VM) + 1;
	if (nn == 1) {
		if (agpparams.a(1, 1, 2) == "") {
			var agpparam = agpparams.a(1, 1, 1);
	//returnsomething:
			if (agpparam == "%DEFAULT%") {
				agpparam = "";
			}
			return agpparam;
		}
	}

	var reqyearperiod = addcent(period0.field("/", 2)) ^ "." ^ (period0.field("/", 1)).oconv("R(0)#2");

	//look for most effective period
	var latestyearperiod = "";
	var agpparam = "";
	for (var ii = 1; ii <= nn; ++ii) {
		var effectiveyearperiod = agpparams.a(1, ii, 2);
		if (effectiveyearperiod <= reqyearperiod) {
			if (effectiveyearperiod >= latestyearperiod) {
				agpparam = agpparams.a(1, ii, 1);
				latestyearperiod = effectiveyearperiod;
			}
		}
	};//ii;

	if (agpparam == "%DEFAULT%") {
		agpparam = "";
	}
	return agpparam;

}


libraryexit()