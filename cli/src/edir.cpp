#include <exodus/program.h>
programinit()

function main() {

	//hard coded editor at the moment
	//http://www.nano-editor.org/docs.php
	var editor = "nano";

	//quit if arguments
	if (dcount(COMMAND, FM) < 2)
		abort(
			"Syntax is:\n"
			"edir DBFILENAME KEY [FIELDNO]\n"
			"or"
			"edir OSFILEPATH [LINENO]"
		);

	// Allow for first argument to be an os file path
	if (COMMAND.a(2).index(OSSLASH))
		COMMAND.inserter(2,"DOS");

	var filename = COMMAND.a(2).convert(".", "_");
	var key = COMMAND.a(3).unquote().unquote();	 //spaces are quoted
	var fieldno = COMMAND.a(4);

	if (not fieldno.isnum())
		abort("fieldno must be numeric");

	//connect to the database
	//if (not connect())
	//	stop("Please login");

	//check the file exists
	var file;
	if (not open(filename, file))
		stop("Cannot open file " ^ filename);

	//get the record from the database (or filesystem if "filename" is "DOS")
	var origrecord;
	if (not read(origrecord, file, key)) {
		//check if exists in upper or lower case
		var key2 = key.ucase();
		if (key2 eq key)
			key2.lcaser();
		if (read(origrecord, file, key2))
			key = key2;
		else
			origrecord = "";
	}

	if (not origrecord)
		printl("New record");

	//if no FM present then treat \n as FM
	var line_sep = FM;
	var record = origrecord;
	if (not origrecord.index(FM)) {
		if (record.index("\r\n"))
			line_sep = "\r\n";
		else if (record.index("\n"))
			line_sep = "\n";
		if (line_sep ne FM)
			record=origrecord.swap(line_sep, FM);
	}
	if (filename eq "DOS")
		line_sep = "\n";

	//var text = origrecord.a(fieldno);
	var text = record.a(fieldno);
	var sepchar;
	if (not fieldno)
		sepchar = FM;
	else {
		if (text.index(VM))
			sepchar = VM;
		else if (text.index(SM))
			sepchar = SM;
		else if (text.index(TM))
			sepchar = TM;
		else if (text.index(STM))
			sepchar = STM;
		else
			sepchar = VM;
	}
	//convert to text format
	text.swapper("\\", "\\\\");
	text.swapper("\r", "\\r");
	text.swapper("\n", "\\n");
	text.swapper(sepchar, "\n");

	//put the text on a temp file in order to edit it
	var temposfilename = filename ^ "~" ^ key;
	var invalidfilechars = "/ \"\'\u00A3$%^&*(){}[]:;#<>?,./\\|";
	temposfilename.lcaser().converter(invalidfilechars, str("-", len(invalidfilechars)));
	temposfilename ^= "-pid" ^ ospid();
	if (filename.substr(1, 5) == "dict." and fieldno)
		temposfilename ^= ".sql";
	else
		temposfilename ^= ".tmp";
	oswrite(text, temposfilename);

	//record file update timedate
	var fileinfo = osfile(temposfilename);
	if (not fileinfo)
		abort("Could not write local copy for editing " ^ temposfilename);

	//fire up the editor
	var editcmd = editor ^ " " ^ temposfilename.quote();
	printl(editcmd);
	osshell(editor ^ " " ^ temposfilename);

	//process after editor has been closed

	var fileinfo2 = osfile(temposfilename);

	//get edited file info or abort
	if (not fileinfo2) {
		abort("Could not read local copy after editing " ^ temposfilename);
	}

	//file has been edited
	else if (fileinfo2 ne fileinfo) {

		var text2;
		osread(text2, temposfilename);

		if (text2 == "") {
			abort("Could not read local copy after editing " ^ temposfilename);
		}

		//remove trailing lf or cr or crlf
		trimmerb(text2, "\r\n");

		//convert to original format
		text2.swapper("\n", sepchar);
		text2.swapper("\\r", "\r");
		text2.swapper("\\n", "\n");
		text2.swapper("\\\\", "\\");

		if (text2 ne text) {

			//print("Ok to update? ");
			//var reply=inputl();
			var reply = "Y";

			//var newrecord = fieldno ? origrecord.r(fieldno, text2) : text2;
			var newrecord = fieldno ? record.r(fieldno, text2) : text2;

			if (line_sep ne FM)
				newrecord.swapper(FM, line_sep);

			//keep trying to update - perhaps futilely
			//at least temp file will be left in the directory
			while (ucase(reply)[1] eq "Y" and true) {

				if (write(newrecord, file, key)) {
					printl(filename ^ " " ^ key ^ " > db");

					//generate/update database functions if saved a symbolic dictionary record
					var x = filename.substr(1, 5) == "dict.";
					var y = newrecord.a(1) eq "S";
					var z = newrecord.a(8).index(
						"/"
						"*pgsql");
					if (filename.substr(1, 5) == "dict." and newrecord.a(1) eq "S" and newrecord.a(8).index("/"
																											"*pgsql")) {
						var oscmd = "dict2sql " ^ filename ^ " " ^ key;
						//oscmd.outputl("oscmd=");
						osshell(oscmd);
					}

					break;
				}
				var temp;
				temp.input();
			}
		}
	}

	//clean up any temporary file
	osremove(temposfilename);

	return 0;
}

programexit()
