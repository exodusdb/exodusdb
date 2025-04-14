#include <exodus/library.h>
libraryinit()

func main(in filename0, in dictid, in prefix = "", in orderby = "") {

	let lenprefix = prefix.len();

	let filename = filename0.convert(".", "_");

	// check file exists
	var file;
	if (not open(filename, file)) {
		abort(lasterror());
	}

	// separate cursor
	var	 v69;
	var	 v70;
	var	 v71;
	call pushselect(v69);

	var cmd = "SELECT " ^ filename ^ " DISTINCT " ^ dictid;
	if (prefix)
		cmd ^= " WITH " ^ dictid ^ " STARTING " ^ prefix.quote();
	//file.select(cmd);
	if (not file.select(cmd)) {
		//null
	}

	var indexvalue;
	var indexvalues = "";
	while (file.readnext(indexvalue)) {

		// remove prefix if specified
		if (prefix) {
			// failsafe - skip indexes with wrong prefix
			if (not indexvalue.starts(prefix)) {
				continue;
			}
			// remove prefix
			indexvalue.cutter(lenprefix);
		}
		// indexvalue.outputl("index value=");
		// accumulate the index values
		if (orderby) {
			var indexn;
			if (not indexvalues.locateby(orderby, indexvalue, indexn))
				indexvalues.inserter(1, indexn, indexvalue);
		} else {
			indexvalues ^= indexvalue ^ FM;
		}
	}

	// correct the output
	if (orderby)
		indexvalues.converter(VM, FM);
	else
		indexvalues.popper();

	// return value in PSEUDO
//	move(indexvalues, PSEUDO);
	PSEUDO = move(indexvalues);

	clearselect();
	call popselect(v69);

	return 0;
}

}; // libraryexit()
