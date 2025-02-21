#include <exodus/program.h>
programinit()

function main() {

	//hard coded editor at the moment
	//http://www.nano-editor.org/docs.php
	let editor = "nano";

	//quit if arguments
	if (fcount(COMMAND, FM) < 2)
		abort(
			"Syntax is:"
			"\nedir DBFILENAME KEY [FIELDNO]"
			"\nor"
			"\nedir OSFILEPATH [LINENO] [OPTIONS]"
			"\n"
			"\nOPTION 'R' = Show raw tm/sm etc."
			"\nOPTION 'T' = Print to standard output (terminal)"
		);

	// Allow for first argument to be an os file path
	if (COMMAND.f(2).contains(OSSLASH))
		COMMAND.inserter(2, "DOS");

	//var dbfilename = COMMAND.f(2).convert(".", "_");
	let dbfilename = COMMAND.f(2);
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

	//check the dbfile exists
	var dbfile;
	if (not open(dbfilename, dbfile))
//		abort("Cannot open db file " ^ dbfilename);
		abort(lasterror());

	//get the record from the database (or filesystem if "dbfilename" is "DOS")
	bool is_new_record = false;
	if (not read(RECORD, dbfile, ID)) {
		//check if exists in upper or lower case
		var key2 = ID.ucase();
		if (key2 == ID)
			key2.lcaser();
		if (read(RECORD, dbfile, key2))
			ID = key2;
		else {
			RECORD = "";
			is_new_record = true;
		}
	}

	if (is_new_record)
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
	let converttext = dbfilename != "DOS" or text.contains(FM);
	if (converttext)
		text = text.oconv(txtfmt);

	//put the text on a temp osfile in order to edit it
	var temposfilename = dbfilename ^ "~" ^ ID;
	let invalidfilechars = "/ \"\'\u00A3$%^&*(){}[]:;#<>?,./\\|";
	temposfilename.lcaser();
	temposfilename.converter(invalidfilechars, str("-", len(invalidfilechars)));
	temposfilename ^= "-pid" ^ ospid();
	if (dbfilename.starts("dict.") and fieldno)
		temposfilename ^= ".sql";
	else
		temposfilename ^= ".tmp";
	//oswrite(text, temposfilename);
	temposfilename.prefixer(ostempdirpath());
	if (osfile(temposfilename) && not osremove(temposfilename))
		abort(lasterror());
	if (not is_new_record and not oswrite(text, temposfilename)) {
		abort(lasterror());
	}

	//record osfile update timedate
	let fileinfo = osfile(temposfilename);
	if (not is_new_record and not fileinfo)
		abort("Could not write local copy for editing " ^ temposfilename);

	let isdict = dbfilename.starts("dict.") or (dbfilename == "DOS" and ID.contains("dat/dict."));

	let editcmd = editor ^ " " ^ temposfilename.quote();
	while (true) {

		//fire up the editor
		printl(editcmd);
		if (not osshell(editor ^ " " ^ temposfilename))
			lasterror().errputl("edir:");

		//process after editor has been closed

		let fileinfo2 = osfile(temposfilename);

		//get edited osfile info or abort if not new record (never saved)
		if (not fileinfo2) {
			if (not is_new_record)
				abort("Could not read local copy after editing " ^ temposfilename);
			stop();
		}

		//osfile has been edited
		else if (fileinfo2 != fileinfo) {

			var text2 = osread(temposfilename);

			if (text2 == "") {

				//abort("Could not read local copy after editing " ^ temposfilename);
				var reply;
				var options = "Yes]No"_var;
				if (not is_new_record)
					options ^= VM ^ "Delete it.";
				if (not decide("Warning: Writing an empty record.\nAre you sure?", options, reply, 2)) {
					continue;
				}
				if (reply == 2)
					break;

				if (reply == 3) {
					if (not dbfile.deleterecord(ID))
						abort(lasterror());
					printl(dbfilename, ID, "deleted.");
					stop();
				}
			}

			// Unescape text back to data format
			//if (dbfilename != "DOS") {
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
				if (dbfilename == "DOS") {
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

			if (text2 != text or is_new_record) {

				//printx("Ok to update? ");
				//var reply=inputl();
				let reply = "Y";

				let newrecord = fieldno ? RECORD.pickreplace(fieldno, text2) : text2;

				//keep trying to update - perhaps futilely
				//at least temp osfile will be left in the directory
				while (ucase(reply).starts("Y") and true) {

//					if (write(newrecord, dbfile, ID)) {
					bool ok = true;
					if (dbfile == "DOS")
						ok = oswrite(newrecord on ID);
					else
						write(newrecord, dbfile, ID);
					if (ok) {
						printl(dbfilename ^ " " ^ ID ^ " > db");

						//generate/update database functions if saved a symbolic dictionary record
						if (isdict and newrecord.f(1) == "S" and newrecord.f(8).contains("/"
																												"*pgsql")) {
							let oscmd = "dict2sql " ^ dbfilename ^ " " ^ ID;
							if (not osshell(oscmd))
								lasterror().errputl("edir:");
						}

						break;
					}
					var temp;
					if (not temp.input()) {}
				}
			}
		}
		break;
	}

	//clean up any temporary osfile
	if (osfile(temposfilename)) {
		if (not osremove(temposfilename)) {
			lasterror().errputl();
		}
	}

	return 0;
}

programexit()
