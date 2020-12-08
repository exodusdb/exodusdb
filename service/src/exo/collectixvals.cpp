#include <exodus/library.h>
libraryinit()

function main(in filename, in dictid, in prefix="", in orderby="") {
	//c xxx in,in,"",""

	var lenprefix = prefix.length();

	//check file exists
	var file;
	if (not open(filename,file)) {
		call fsmsg();
		stop();
	}

        //separate cursor
        var v69;
        var v70;
        var v71;
        call pushselect(0, v69, v70, v71);

	var cmd = "SELECT " ^ filename ^ " DISTINCT " ^ dictid;
	if (prefix)
		cmd ^= " WITH " ^ dictid ^ " STARTING " ^ prefix.quote();
	file.select(cmd);

	var indexvalue;
	var indexvalues = "";
	while (file.readnext(indexvalue)) {

		//remove prefix if specified
		if (prefix) {
			//failsafe - skip indexes with wrong prefix
			if (indexvalue.substr(1,lenprefix) ne prefix) {
				continue;
			}
			//remove prefix
			indexvalue.splicer(1, lenprefix, "");
		}
//indexvalue.outputl("index value=");
		//accumulate the index values
		if (orderby) {
			var indexn;
			if (not(indexvalues.locateby(orderby, indexvalue, indexn)))
				indexvalues.inserter(1, indexn, indexvalue);
		} else
			indexvalues ^= indexvalue ^ FM;
	}

	//correct the output
	if (orderby)
		indexvalues.converter(VM,FM);
	else
		indexvalues.splicer(-1, 1, "");

	//return value in PSEUDO
	transfer(indexvalues, PSEUDO);

        clearselect();
        call popselect(0, v69, v70, v71);

	return 0;

}

libraryexit()
