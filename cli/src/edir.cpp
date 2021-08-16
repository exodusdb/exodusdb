#include <exodus/program.h>
programinit()

	function main() {
	//hard coded editor at the moment
	//http://www.nano-editor.org/docs.php
	var editor = "nano";
//restrict to editing records for now
//#define ALLOW_EDIC
#ifdef ALLOW_EDIC
	if (dcount(COMMAND, FM) < 3) {
#else
	if (dcount(COMMAND, FM) < 2) {
#endif

		//quit if arguments
		if (dcount(COMMAND, FM) < 2)
			abort(
				"Syntax is:"
				"\nedit databasefilename key ..."
#ifdef ALLOW_EDIC
				"\nor"
				"\nedit osfilename"
#endif
			);

		//switch to edic if only one argument
		osshell(COMMAND.pickreplace(1, 0, 0, "edic").convert(FM, " "));
		stop();
	}

	var filename = COMMAND.a(2).convert(".", "_");
	var key = COMMAND.a(3).unquote().unquote();	 //spaces are quoted
	var fieldno = COMMAND.a(4);

	//connect to the database
	//if (not connect())
	//	stop("Please login");

	//check the file exists
	var file;
	if (not open(filename, file))
		stop("Cannot open file " ^ filename);

	//get the record from the database
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

	var text = origrecord.a(fieldno);
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

		//convert to record format
		text2.swapper("\n", sepchar);
		text2.swapper("\\r", "\r");
		text2.swapper("\\n", "\n");
		text2.swapper("\\\\", "\\");

		if (text2 ne text) {

			//print("Ok to update? ");
			//var reply=inputl();
			var reply = "Y";

			var newrecord = fieldno ? origrecord.r(fieldno, text2) : text2;

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
	osdelete(temposfilename);

	return 0;
}

programexit()
