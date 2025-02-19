#include <exodus/program.h>
programinit()

function main() {

	//hard coded editor at the moment
	//http://www.nano-editor.org/docs.php
	let editor = "nano";

	//quit if arguments
	if (fcount(COMMAND, FM) < 2)
		abort(
			"Syntax is:\n"
			"edir DBFILENAME KEY [FIELDNO]\n"
			"or\n"
			"edir OSFILEPATH [LINENO] [OPTIONS]\n"
			"\n"
			"OPTION 'R' = Show raw tm/sm etc."
			"OPTION 'T' = Print to standard output (terminal)"
		);

	// Allow for first argument to be an os file path
	if (COMMAND.f(2).contains(OSSLASH))
		COMMAND.inserter(2, "DOS");

	//var filename = COMMAND.f(2).convert(".", "_");
	let filename = COMMAND.f(2);
	ID = COMMAND.f(3).unquote().unquote();	 //spaces are quoted
	let fieldno = COMMAND.f(4);

	// Raw
	var txtfmt = "TX";
	// Raw option only converts FM to NL and leaves VM SM TM ST as are
	if (OPTIONS.contains("R"))
		txtfmt ^= "1";

	if (not fieldno.isnum())
		abort("fieldno must be numeric");

	//connect to the database
	//if (not connect())
	//	stop("Please login");

	//check the file exists
	var file;
	if (not open(filename, file))
		abort("Cannot open file " ^ filename);

	//get the record from the database (or filesystem if "filename" is "DOS")
	if (not read(RECORD, file, ID)) {
		//check if exists in upper or lower case
		var key2 = ID.ucase();
		if (key2 == ID)
			key2.lcaser();
		if (read(RECORD, file, key2))
			ID = key2;
		else
			RECORD = "";
	}

	if (not RECORD)
		printl("New record");

	var text = RECORD.f(fieldno);

	// Treat VMs as FMs etc if editing a specific field
	if (fieldno)
		text.raiser();

	// print record to standard output (terminal)
	// useful (and standardises) reading records from bash scripts
	if (OPTIONS.contains("T")) {
		text.converter(_ALL_FMS, _VISIBLE_FMS);
		if (not osshell("echo " ^ text.quote())) {
			loglasterror();
		}
		return 0;
	}

	// Escape data format to text format
	let converttext = filename != "DOS" or text.contains(FM);
	if (converttext)
		text = text.oconv(txtfmt);

	//put the text on a temp file in order to edit it
	var temposfilename = filename ^ "~" ^ ID;
	let invalidfilechars = "/ \"\'\u00A3$%^&*(){}[]:;#<>?,./\\|";
	temposfilename.lcaser();
	temposfilename.converter(invalidfilechars, str("-", len(invalidfilechars)));
	temposfilename ^= "-pid" ^ ospid();
	if (filename.starts("dict.") and fieldno)
		temposfilename ^= ".sql";
	else
		temposfilename ^= ".tmp";
	//oswrite(text, temposfilename);
	if (not oswrite(text, temposfilename)) {
		abort(lasterror());
	}

	//record file update timedate
	let fileinfo = osfile(temposfilename);
	if (not fileinfo)
		abort("Could not write local copy for editing " ^ temposfilename);

	let isdict = filename.starts("dict.") or (filename == "DOS" and ID.contains("dat/dict."));

	let editcmd = editor ^ " " ^ temposfilename.quote();
	while (true) {

		//fire up the editor
		printl(editcmd);
		if (not osshell(editor ^ " " ^ temposfilename))
			lasterror().errputl("edir:");

		//process after editor has been closed

		let fileinfo2 = osfile(temposfilename);

		//get edited file info or abort
		if (not fileinfo2) {
			abort("Could not read local copy after editing " ^ temposfilename);
		}

		//file has been edited
		else if (fileinfo2 != fileinfo) {

			var text2 = osread(temposfilename);

			if (text2 == "") {
				abort("Could not read local copy after editing " ^ temposfilename);
			}

			// Unescape text back to data format
			//if (filename != "DOS") {
			if (converttext) {

				trimmerlast(text2, _EOL);

				//convert to original format
				text2 = text2.iconv(txtfmt);

			}

			// Convert FMs back to VMs etc. if editing a specific field
			if (fieldno)
				text2.lowerer();

			// Validate dict F/S items
			if (isdict) {
				var dictrec = text2;

				// Convert to FMs to check dict item format
				if (filename == "DOS") {
					trimmerlast(dictrec, _EOL);
					dictrec = dictrec.iconv(txtfmt);
				}

				// Very similar code in edir and syncdat
				if (var("FS").contains(dictrec.f(1))) {

					let options = "";
					var reply = "";

					// Check justification
					if (not var("LRTC").contains(dictrec.f(9))) {
						if (decide("Field 9 of F/S dict items cannot be " ^ dictrec.f(9).quote() ^ "\nField 9 of F/S dict items must be L, R, C, T.\nFix it?", options, reply, 1) != "Yes")
							abort("");
						continue;
					}

					// Check width
					if (not dictrec.f(10).isnum()) {
						if (decide("Field 10 of F/S items cannot be " ^ dictrec.f(10).quote() ^ "\nField 10 of F/S items must be numeric\nFix it?", options, reply, 1) != "Yes")
							abort("");
						continue;
					}
				}
			}

			if (text2 != text) {

				//printx("Ok to update? ");
				//var reply=inputl();
				let reply = "Y";

				let newrecord = fieldno ? RECORD.pickreplace(fieldno, text2) : text2;

				//keep trying to update - perhaps futilely
				//at least temp file will be left in the directory
				while (ucase(reply).starts("Y") and true) {

//					if (write(newrecord, file, ID)) {
					bool ok = true;
					if (file == "DOS")
						ok = oswrite(newrecord on ID);
					else
						write(newrecord, file, ID);
					if (ok) {
						printl(filename ^ " " ^ ID ^ " > db");

						//generate/update database functions if saved a symbolic dictionary record
						if (isdict and newrecord.f(1) == "S" and newrecord.f(8).contains("/"
																												"*pgsql")) {
							let oscmd = "dict2sql " ^ filename ^ " " ^ ID;
							if (not osshell(oscmd))
								lasterror().errputl("edir:");
						}

						break;
					}
					var temp;
					temp.input();
				}
			}
		}
		break;
	}

	//clean up any temporary file
	if (osfile(temposfilename)) {
		if (not osremove(temposfilename)) {
			lasterror().errputl();
		}
	}

	return 0;
}

programexit()
