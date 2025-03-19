#include <exodus/program.h>
programinit()

var redcolour="\033[1;31m";
var endcolour = "\033[0m";

// View dict and data together
func main() {

	//F | T |DICT NAME              |SCHEDULES:J1061
	//12| S |PERIOD                 |3/23
	//13| S |CURRENCY_CODE          |AED
	//14| M |MARKET_CODE            |UAE

    let filename = COMMAND.f(2);
	var key = COMMAND.f(3);
	var filterfieldnos = COMMAND.f(4);
	let dosfilename = COMMAND.f(5);

	var syntaxmsg = "Syntax: EXO_DATA=<DBCODE> ddview <FILENAME> <KEY> [FIELDNO or DOS] [DOSFILENAME] {OPTIONS}";
		syntaxmsg ^= "\n                   e.g ddview schedules J1061 ";
		syntaxmsg ^= "\n                   e.g ddview schedules J1061 12-14";
		syntaxmsg ^= "\n                   e.g ddview schedules J1061 12,13,14";
		syntaxmsg ^= "\n                   e.g ddview schedules FAKE  DOS /root/J1061_record";
		syntaxmsg ^= "\n                   e.g ddview schedules J1061 {M} #verbose Multivalue details";

	key.ucaser();

	if (not filename and not key) {
		abort(syntaxmsg);
	}

	// Expand filtered field range
	if (filterfieldnos.contains("-")) {

		let min = filterfieldnos.field("-", 1);
		let max = filterfieldnos.field("-", 2);
		let diff = max - min;

		filterfieldnos = "";
		for (var ii : range(min, max)) {
			filterfieldnos ^= ii ^ VM;
		}
		filterfieldnos.popper();
		filterfieldnos = "\"" ^ filterfieldnos.replace(VM, "\" \"") ^ "\"";

	} else if (filterfieldnos.contains(",")) {

		filterfieldnos.trimmer(",");
		filterfieldnos.replacer(",", "\" \"");
		filterfieldnos.quoter();

	} else {
		filterfieldnos = "";
	}


	// Open dict file
	var dict;
    if (not open("dict." ^ filename, dict)) {
        abort(("dict." ^ filename).quote() ^ " file does not exist.");
	}

	// Get RECORD from database or osfile
	var datarec;
	if (dosfilename) {

		// Read record from osfile
		if (not datarec.osread(dosfilename))
	        abort(dosfilename.quote() ^ " file does not exist.");

		datarec.replacer("\n", FM);

	} else {

		// Open db data file
	    var file;
	    if (not (open(filename, file) and datarec.read(file, key))) {
	        abort(lasterror());
		}
	}

	select("dict." ^ filename ^ " by FMC WITH FMC " ^ filterfieldnos ^ " (SR)");
	if (not LISTACTIVE) {
		printl("No Records found!");
	}

	// Print column headings
	printl(redcolour ^ "  F|SM  |DICT NAME                |DATA in " ^ filename.ucase() ^ ":" ^ key ^ endcolour);

	// Print lines
	while (readnext(RECORD, ID, MV)) {

		let dictfieldno = RECORD.f(2);
		let dictfieldname = RECORD.f(3);
		var dictvaluetype = RECORD.f(4);

		// simplify multi-value detail like 1.3 to just "M"
		if (not OPTIONS.contains("M")) {
			if (dictvaluetype != "S")
				dictvaluetype = "M";
		}

		var line = "";
		line ^= dictfieldno.oconv("R#3") ^ "|";
		line ^= dictvaluetype.oconv("T#4") ^ "|";
		line ^= ID.first(25).oconv("T#25") ^ "|";
		line ^= datarec.f(dictfieldno).convert(_ALL_FMS, _VISIBLE_FMS);
		printl(line);
	}

	return 0;
}


programexit()
