//#include <iostream>
//#include <cstdio>
//#include <cstdlib>
//#include <filesystem>
//namespace fs = std::filesystem;

#include <exodus/program.h>
programinit()

function main() {

	var defsrec;
	if (not osread(defsrec from COMMAND(2)))
		abort(lasterror());
	defsrec.converter("\n\t", _FM _FM);
	defsrec.trimmer();

	dim defs = defsrec.split();

//	select("dict.schedules with FMC between 1 and 9999 (R)");
//	let ids="";
//	while (readnext(RECORD, ID, MV)) {
//		let fn = RECORD(2);
//		if (fn.isnum()) {
//			ids(RECORD.f(2)) = ID;
//		}
//	}

	var osfilen = 0;
	let nosfiles = fcount(COMMAND, FM) -2;
	for (var osfilename : COMMAND.remove(1).remove(1)) {

		var oldcpp, newcpp;
		if (not osread(oldcpp from osfilename))
			abort(lasterror());

		if (TERMINAL) {
			printx(AT(-40), ++osfilen ^ "/" ^ nosfiles ^ ".", osfilename);
			osflush();
		}
		newcpp = oldcpp;
		for (in def : defs) {

			if (def.field(" ", 1) != "#define")
				continue;

			var value = def.field(" ", 3);
			var name = def.field(" ", 2);

			// Reject simple numeric definitions like "define xyz 2"
			if (not value or not name or value.isnum())
				continue;

//			let oldcode = "(orec|RECORD)\\(" ^ value ^ "\\b";
//			let newcode = "\\1\\(" ^ name ^ "";
//
//			newcpp.regex_replacer(oldcode, newcode);
//
//			oldcode = "(orec|RECORD)\\.f\\(" ^ value ^ "\\b";
//			newcode = "\\1.f\\(" ^ name ^ "";
//
//			newcpp.regex_replacer(oldcode, newcode);

			if (name.last() == "+")
				name.popper() ^= value;

			value.replacer("(", "\\(");
			value.replacer(")", "\\)");
			value.replacer(".", "\\.");

			//var regex = "\\b" ^ value ^ "\\b";
			//var regex = "\\b" ^ value;
			let regex = value;

			let newcode = name;

//			printl("", regex, "->", newcode);

			let found = newcpp.match(regex);
			if (found)
				printl("", found, "->", newcode);

			newcpp.regex_replacer(regex, newcode);

		}

		if (newcpp != oldcpp) {
			printt(osfilename, "");
			if (OPTIONS.contains("U")) {
				//oswrite(newcpp on osfilename);
				if (not oswrite(newcpp on osfilename))
					abort(lasterror());
				printl(" updated");
			} else {
				//newcpp.output();
				printl(" updatable");
			}
		}

//		let pending = newcpp.match("RECORD\\(([1-9].*?)\\)").unique();
//		if (pending) {
//			printl(pending);
//			let pending2 = "";
//			for (var part : pending)
//				pending2(-1) = part.f(1,2);
//			pending2.uniquer().sorter();
//			for (var part : pending2) {
//				printl(osfilename, part);
//				let value = part.field(" ", 1);
//				if (value.isnum() and fns(fn)) {
//					printl("#define", fns.f(fn).lcase() ^ "_", fn);
//				}
//			}
//		}

	} // next osfilename

	printl();

	return 0;
}

programexit()

