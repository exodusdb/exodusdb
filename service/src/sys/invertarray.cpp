#include <exodus/library.h>
libraryinit()

var force;//num

function main(in input, in force0=(0)) {
	//c sys in,=(0)
	if (force0.unassigned()) {
		force = 0;
	}else{
		force = force0;
	}
	var output = "";
	var nfs = input.count(FM) + (input ne "");
	//for force to work, the first field must have full number of vns
	var maxnvs = 0;
	for (var fn = 1; fn <= nfs; ++fn) {
		var fieldx = input.field(FM, fn);
		if (fieldx.length() or force) {
			var nvs = fieldx.count(VM) + 1;
			if (force) {
				if (nvs > maxnvs) {
					maxnvs = nvs;
				}
			}else{
				maxnvs = nvs;
			}
			for (var vn = 1; vn <= maxnvs; ++vn) {
				var cell = fieldx.field(VM, vn);
				if (cell.length() or force) {
					output.r(vn, fn, cell);
				}
			};//vn;
		}
	};//fn;

	return output;
}

libraryexit()
