#include <exodus/program.h>
programinit()

function main() {

	//hard coded editor at the moment
	//http://www.nano-editor.org/docs.php
	let editor = "nano";

	// Allow for first argument to be an os file path
	if (COMMAND.f(2).contains(OSSLASH))
		COMMAND.inserter(2, "DOS");

	// Extract main args
	let dbfilename = COMMAND.f(2);
	let keys       = COMMAND.f(3).convert(",", FM);
	let fieldno    = COMMAND.f(4);

	// (R)aw option only converts FM to NL and leaves VM SM TM ST untouched
	let txtfmt = OPTIONS.contains("R") ? "TX1" : "TX";

	// Try to activate a default select list if no keys provided
	var listid = "";
	if (not COMMAND.f(3) and COMMAND.f(2) ne "DOS") {
		listid = "default";
		if (getlist(listid))
			logputl("Using list: ", listid);
		else
			listid = "";
	}

	//quit if arguments missing. IDs is optional if a select is available
	if (not dbfilename or (not keys and listid.empty()))
		abort(
			"Syntax is:"
			"\nedir DBFILENAME KEYS,... [FIELDNO]"
			"\nor"
			"\nedir OSFILEPATH [LINENO] [OPTIONS]"
			"\n"
			"\nMultiple keys must be separated by commas, not spaces."
			"\nKeys with spaces in them must be quoted."
			"\n"
			"\nOPTION 'R' = Show raw tm/sm etc."
			"\nOPTION 'T' = Print to standard output (terminal)"
		);

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

	// If ID from command line
	if (keys)
		selectkeys(keys);

	// Maximum linux execution command line is crudely 130-140,000 short filename of 15 bytes
	// getconf ARG_MAX
	// 2,097,152

	// Create and use a temporary work dir
	let workdir = ostempdirpath() ^ "~e" ^ rnd(999'999);
	if (not osmkdir(workdir))
		abort(lasterror());

	// Keep in parallel
	var IDs = "";
	var osfilenames = "";
	var fileinfo_olds = "";
	var is_new_records = "";

	var converttext;

	while (readnext(ID)) {

		ID.unquoter(); // Spaces in keys have to be quoted
		ID.unquoter(); // Handle overquoting

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

		if (is_new_record and keys)
			printl("New record");

		// fieldno 0/"" means whole record
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
			// TODO continue for select list?
			return 0;
		}

		// Escape data format to text format
		converttext = dbfilename != "DOS" or text.contains(FM);
		if (converttext)
			text = text.oconv(txtfmt);

		//put the text on a temp osfile in order to edit it
		let temposfilename = getosfilename(workdir, dbfilename, fieldno, ID);

		// Remove any previous version of the OS file
//		if (osfile(temposfilename) && not osremove(temposfilename))
//			abort(lasterror());

		// Check osfile does not already exist in the new dir
		if (osfile(temposfilename))
			abort(ID.quote() ^ " should not already exist in " ^ workdir.quote());

		// Write the osfile
		if (not is_new_record and not oswrite(text, temposfilename))
			abort(lasterror());

		// Record osfile update time
		let fileinfo_old = osfile(temposfilename);
		if (not is_new_record and not fileinfo_old)
			abort("Could not write local copy for editing " ^ temposfilename);

		// Keep these stricty in parallel for use in the post edit processing
		IDs            ^= ID ^ FM_;
		fileinfo_olds  ^= fileinfo_old.lower() ^ FM_;
		is_new_records ^= is_new_record ^ FM_;

		osfilenames ^= temposfilename.quote() ^ " ";

	} // multiple IDs

	// Consume the default list
	if (listid)
		deletelist(listid);

	let isdict = not fieldno and ( dbfilename.starts("dict.") or (dbfilename == "DOS" and ID.contains("dat/dict.")));

	//fire up the editor initially for all temposfilenames, IDs
	if (not osshell(editor ^ " " ^ osfilenames))
		lasterror().errputl("edir:");

	var IDno = 0;

	for (let id : IDs) {

		// Ensure the real ID is available in case any future code starts relying on it
		ID = id;

		IDno ++;

		// Recalculate the temp os file name of the record ID
		let temposfilename = getosfilename(workdir, dbfilename, fieldno, ID);

		// Extract saved info about the db record
		bool is_new_record = is_new_records.f(IDno);
		let fileinfo_old = fileinfo_olds.f(IDno).raise();

		let editcmd = editor ^ " " ^ temposfilename.quote();
		var passno = 0;

		while (true) {

			passno++;

			//fire up the editor to handle issues
			if (passno > 1) {
				printl(editcmd);
				if (not osshell(editor ^ " " ^ temposfilename))
					lasterror().errputl("edir:");
			}

			// Get the original current unupdated dbrecord in case it has been updated since we started
			if (not read(RECORD, dbfile, ID)) {
				RECORD = "";
				if (not is_new_record) {
					logput("Warning: Record was deleted from db since we started editing it");
					// TODO question user what to do.
					is_new_record = true;
//					var reply;
//					var options = "Yes]No"_var;
//					if (not is_new_record)
//						options ^= VM ^ "Delete it.";
//					if (not decide("Warning: Writing an empty record.\nAre you sure?", options, reply, 2)) {
//						continue;
//					}
//					if (reply == 2)
//						break;
				}
			}
			let orig_unconverted_text = RECORD.f(fieldno);

			//get edited osfile info or abort if not new record (never saved)
			let fileinfo_new = osfile(temposfilename);
			if (not fileinfo_new) {
				if (not is_new_record)
					abort("Could not read local copy after editing " ^ temposfilename);
				stop();
			}

			//osfile has been edited
			else if (fileinfo_new != fileinfo_old) {

				let edited_text = osread(temposfilename);

				if (edited_text == "") {

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

				// lambda deconverter
				auto deconverter = [](in edited_text, const bool converttext, in fieldno, in txtfmt) -> var {

					// Unescape text back to data format
					//if (dbfilename != "DOS") {
					var deconverted_text = edited_text;
					if (converttext) {

						deconverted_text.trimmerlast(_EOL);

						//convert to original format \n -> FM etc.
						deconverted_text = deconverted_text.iconv(txtfmt);

					}

					// Convert FMs back to VMs etc. if editing a specific field
					if (fieldno)
						deconverted_text.lowerer();

					return deconverted_text;
				};

				// Deconvert text
				let deconverted_text = deconverter(edited_text, converttext, fieldno, txtfmt);

				// Validate dict F/S items
				if (isdict) {
					var dictrec = deconverted_text;

//					// Convert to FMs to check dict item format
//					if (dbfilename == "DOS") {
//						trimmerlast(dictrec, _EOL);
//						dictrec = dictrec.iconv(txtfmt);
//					}

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
				} // validate dict

				if (deconverted_text != orig_unconverted_text or is_new_record) {

					//printx("Ok to update? ");
					//var reply=inputl();
					let reply = "Y";

					let newrecord = fieldno ? RECORD.update(fieldno, deconverted_text) : deconverted_text;

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
						if (not temp.input("Press Enter:")) {}
					}
				}
			}
			break;
		}

//		//clean up any temporary osfile
//		if (osfile(temposfilename)) {
//			if (not osremove(temposfilename)) {
//				lasterror().errputl();
//			}
//		}

	} // osfilename

	//clean up any temporary osfile
	if (not workdir.osrmdir(true))
		loglasterror();

	return 0;
}

function getosfilename(in dir, in dbfilename, in fieldno, in id) {

	var temposfilename = dbfilename ^ "~" ^ id;
	let invalidfilechars = R"__(\"')__" "\u00A3 $%^&*(){}[]:;#<>?,./|";
	temposfilename.lcaser();
	// TODO use escaped chars to avoid two records converting to the same key
	// e.g. currently both "aa,bb" and "aa.bb" will be converted to "aa-bb"
	temposfilename.converter(invalidfilechars, str("-", len(invalidfilechars)));
//	temposfilename ^= "-pid" ^ ospid();
	if (dbfilename.starts("dict.") and fieldno)
		temposfilename ^= ".sql";
//	else
//		temposfilename ^= ".dat";

	if (dir.last() != "/")
		temposfilename.prefixer("/");

	temposfilename.prefixer(dir);

	return temposfilename;
}

programexit()
