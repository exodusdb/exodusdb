#include <exodus/program.h>
programinit()

function main() {

	//default to previous edit/compile - similar code in edic and compile
	//check command syntax
	var edic_hist_dir = osgetenv("HOME") ^ "/.config/exodus/";
	var edic_hist = edic_hist_dir ^ "edic_hist.txt";
	if (fcount(COMMAND, FM) < 2) {
		if (osread(COMMAND, edic_hist)) {
			OPTIONS = COMMAND.f(2);
			COMMAND = raise(COMMAND.f(1));
		} else {
			abort("Syntax is 'edic osfilename'");
		}
	}

	if (not osdir(edic_hist_dir))
		osmkdir(edic_hist_dir);
	if (not oswrite(lower(COMMAND) ^ FM ^ OPTIONS, edic_hist))
		printl("Cannot write to ", edic_hist);

	var verbose = OPTIONS.ucase().contains("V");

	var exo_HOME = osgetenv("EXO_HOME");
	if (not exo_HOME)
		exo_HOME = osgetenv("HOME");

	var editor = osgetenv("VISUAL");
	var linenopattern = "$LINENO ";
	if (not editor)
		// get nano on linux possibly
		editor.osgetenv("EDITOR");

	//TODO simplify this editor finding code

	//enable edit at first error for crimson editor (no quotes around filename for cedt)
	if (editor.lcase().contains("cedt") and not editor.contains("$"))
		editor ^= " /L:$LINENO $FILENAME";

	//look for installed nano
	//if (OSSLASH eq "\\" and not index(PLATFORM_,"x64")) {
	var nanopath = "";
	if (OSSLASH eq "\\") {

		//look for nano.exe next to edic.exe
		if (not editor)
			nanopath = EXECPATH.replace("edic", "nano");
		if (nanopath.osfile())
			editor = "nano $LINENO'$FILENAME'";
	}

	//look for nano in parent bin
	if (not editor) {
		nanopath = "..\\bin\\nano.exe";
		if (nanopath.osfile())
			editor = "nano $LINENO'$FILENAME'";
	}

	//look for nano in release directory during exodus development
	//if (not editor) {
	if (not editor and OSSLASH eq "\\") {
		nanopath = "..\\..\\release\\cygwin\\bin\\nano.exe";
		if (nanopath.osfile())
			editor = "..\\..\\release\\cygwin\\bin\\nano $LINENO'$FILENAME'";
		else {
			nanopath = "..\\" ^ nanopath;
			if (nanopath.osfile()) {
				editor = "..\\..\\..\\release\\cygwin\\bin\\nano $LINENO'$FILENAME'";
			}
		}
	}

	if (editor.contains("nano"))
		linenopattern = "+$LINENO ";

	//otherwise on windows try to locate CYGWIN nano or vi
	var cygwinpath = "";
	if (not editor and OSSLASH eq "\\") {
		//from environment variable
		cygwinpath = osgetenv("CYGWIN_BIN");
		//else from current disk
		if (not cygwinpath)
			cygwinpath = "\\cygwin\\bin\\";
		//else from c:
		if (not osdir(cygwinpath))
			cygwinpath = "c:\\cygwin\\bin\\";
		//else give up
		if (not osdir(cygwinpath))
			cygwinpath = "";

		if (cygwinpath and cygwinpath[-1] ne OSSLASH)
			cygwinpath ^= OSSLASH;
		//editor=cygwinpath^"bash --login -i -c \"/bin/";
		editor = cygwinpath;
		if (osfile(cygwinpath ^ "nano.exe") or osfile("nano.exe")) {
			editor = "nano $LINENO'$FILENAME'";
			if (osfile(cygwinpath ^ "nano.exe"))
				editor.prefixer(cygwinpath);
			//editor^="\"";
			linenopattern = "+$LINENO ";
		} else if (osfile(cygwinpath ^ "vi.exe") or osfile("vi.exe")) {
			editor = "vi -c \":$LINENO\" $FILENAME";
			if (osfile(cygwinpath ^ "vi.exe"))
				editor.prefixer(cygwinpath);
			//editor^="\"";
		} else
			editor = "";
	}
	if (OSSLASH eq "\\") {
		//configure nanorc (on windows)
		//TODO same for non-windows
		//nano on windows looks for nanorc config file as follows (probably merges all found)
		//C:\cygwin\usr\local\etc\nanorc
		//C:\cygwin\etc\nanorc (only if cygwin exists)
		//C:\Documents and Settings\USERNAME\.nanorc  ($HOMEDRIVE$HOMEPATH)
		var nanorcfilename;
		if (cygwinpath) {
			nanorcfilename = cygwinpath.field(OSSLASH, 1, fcount(cygwinpath, OSSLASH) - 2) ^ OSSLASH ^ "etc" ^ OSSLASH ^ "nanorc";
		} else {
			nanorcfilename = osgetenv("HOME");
			if (not nanorcfilename)
				nanorcfilename = osgetenv("HOMEDRIVE") ^ osgetenv("HOMEPATH");
			if (nanorcfilename[-1] ne OSSLASH)
				nanorcfilename ^= OSSLASH;
			nanorcfilename ^= ".nanorc";
		}
		if (not osfile(nanorcfilename)) {
			//var nanorctemplatefilename=EXECPATH.field(OSSLASH,1,fcount(EXECPATH,OSSLASH)-1) ^ OSSLASH ^ "nanorc";
			var nanorctemplatefilename = nanopath.field(OSSLASH, 1, fcount(nanopath, OSSLASH) - 1) ^ OSSLASH ^ "nanorc";
			if (not osfile(nanorctemplatefilename))
				nanorctemplatefilename.replacer("release", "..\\release");
			//if (not osfile(nanorctemplatefilename))
			//	nanorctemplatefilename.replacer("release","..\\"^PLATFORM_^"\\release");
			if (oscopy(nanorctemplatefilename, nanorcfilename)) {
				printl("Copied " ^ nanorctemplatefilename.quote() ^ " to " ^ nanorcfilename.quote());
				var().input("Note: nano c++ syntax highlighting has been installed. Press Enter ... ");
			} else {
				errputl("Could not copy " ^ nanorctemplatefilename.quote() ^ " to " ^ nanorcfilename.quote());
				if (not osfile(nanorctemplatefilename))
					errputl("nano syntax highlighting file is missing.");
			}
		}
		if (not osgetenv("HOME"))
			ossetenv("HOME", osgetenv("HOMEDRIVE") ^ osgetenv("HOMEPATH"));
	}

	if (not editor) {
		if (OSSLASH eq "/")
			editor = "nano ";
		else
			editor = "notepad";
		printl("Environment EDITOR not set. Using " ^ editor);
	}

	//editor="vi";
	editor.regex_replacer("^[^ ]*\\bnano\\b", "nano --positionlog --const --suspend +$LINENO");
	//editor.replacer("nano ", "nano --positionlog --const --nowrap --autoindent --suspend --speller=compile +$LINENO ");

	if (editor.contains("nano"))
		printl("http://www.nano-editor.org/dist/v2.1/nano.html");

	//configure nano syntax highlighting
	var filenames = field(COMMAND, FM, 2, 99999);
	var nfiles = fcount(filenames, FM);
	var filen = 0;
	while (filen < nfiles) {
		filen += 1;
		var filename = filenames.f(filen).unquote();

		if (not filename.len())
			continue;

		//split out trailing line number after :
		var startatlineno = field(filename, ":", 2);
		if (startatlineno.isnum())
			filename = field(filename, ":", 1);
		else
			startatlineno = "";

		filename.trimmerb(".");
		if (not field2(filename, OSSLASH, -1).contains("."))
			filename ^= ".cpp";

		//allow compiler to copy .h files to ~/inc dir
		//var iscompilable = filename.field2(".", -1).at(1).lcase() ne "h";
		var iscompilable = true;//filename.field2(".", -1)[1].lcase() ne "h";

		//search paths and convert to absolute filename
		//SIMILAR CODE IN EDIC and COMPILE
		if (not(osfile(filename)) and not(filename.contains(OSSLASH))) {
			var paths = osgetenv("CPLUS_INCLUDE_PATH").convert(";", ":");
			if (verbose)
				paths.outputl("paths=");
//			let npaths = fcount(paths, ":");
//			for (const var pathn : range(1, npaths - 1)) {
//				var filename2 = paths.field(":", pathn) ^ "/" ^ filename;
			for (let path : paths.convert(":", FM)) {
				var filename2 = path ^ "/" ^ filename;
				if (verbose)
					filename2.outputl("osfilename=");
				if (osfile(filename2)) {
					filename = filename2;
					break;
				}
			}
		}

		//also look in ~/inc for backlinks to source
		//SIMILAR CODE IN EDIC and COMPILE
		if (not(osfile(filename)) and not(filename.contains(OSSLASH))) {
			var headerfilename = exo_HOME ^ OSSLASH ^ "inc" ^ OSSLASH ^ filename;
			// Try .h files for library subroutines first
			headerfilename.fieldstorer(".", -1, 1, "h");
			if (verbose)
				headerfilename.outputl("headerfile=");
			// Try .H header files for programs
			if (!osfile(headerfilename)) {
				headerfilename.fieldstorer(".", -1, 1, "H");
				if (verbose)
					headerfilename.outputl("headerfile=");
			}
			if (osfile(headerfilename)) {
				var headerline1 = osread(headerfilename).field("\n", 1), field("\r", 1);
				//generated by exodus "compile /root/neosys/src/gen/addcent.cpp"
				var filename2 = headerline1.field2(" ", -1).field("\"", 1);
				if (osfile(filename2))
					filename = filename2;
			}
		}

		//make absolute in case EDITOR changes current working directory
		var editcmd = editor;
		if (editcmd.contains("$ABSOLUTEFILENAME")) {
			editcmd.replacer("$ABSOLUTEFILENAME", "$FILENAME");

			filename = oscwd() ^ OSSLASH ^ filename;
		}
		//prepare a skeleton exodus cpp file
		var newfile = false;
		if (iscompilable and not osfile(filename)) {

			var basefilename = field2(filename, OSSLASH, -1);
			basefilename = basefilename.field(".", fcount(basefilename, ".") - 1);

			var progtype;
			var question = "1=Normal Program, 2=External Subroutine or Function";
			//question^="\n3=main(), 4=simple so/dll\n";
			question ^= "\n" ^ basefilename.quote() ^ " does not exist. Create what? (1-2) ";
			while (true) {

				if (basefilename.lcase().starts("dict."))
					progtype = 5;
				else
					progtype.input(question);

				if (progtype eq 2)
					progtype = "classlib";
				else if (progtype eq 3)
					progtype = "main";
				else if (progtype eq 4)
					progtype = "mainlib";
				else if (progtype eq 1)
					progtype = "class";
				else if (progtype eq 5)
					progtype = "dict";
				else
					stop();
				break;
			}

			newfile = true;
			var blankfile = "";
			if (progtype eq "main" or progtype eq "mainlib") {
				startatlineno = "4,9";
				blankfile ^= "#include <exodus/exodus.h>\n";
				blankfile ^= "\n";
				blankfile ^= "program() {\n";
				blankfile ^= "\tprintl(\"" ^ basefilename ^ " says 'Hello World!'\");\n";
				if (progtype eq "mainlib")
					blankfile ^= "\treturn 0;\n";
				blankfile ^= "}\n";
				if (progtype eq "mainlib")
					blankfile.replacer("program()", "function " ^ basefilename ^ "()");
			} else if (progtype eq "class" or progtype eq "classlib") {
				startatlineno = "6,9";
				blankfile ^= "#include <exodus/program.h>\n";
				//programinit() as 2nd line to avoid ppl in external functions before programinit
				//blankfile^="\n";
				blankfile ^= "programinit()\n";
				blankfile ^= "\n";
				blankfile ^= "function main(";
				//the .h maker not able to parse this yet and is rather clumsy anyway
				//if (progtype eq "classlib")
				//	blankfile^="/*in arg1, out arg2*/";
				blankfile ^= ") {\n";
				blankfile ^= "\tprintl(\"" ^ basefilename ^ " says 'Hello World!'\");\n";
				blankfile ^= "\treturn 0;\n";
				blankfile ^= "}\n";
				blankfile ^= "\nprogramexit()";
				blankfile ^= "\n";

				if (progtype eq "classlib")
					blankfile.replacer("program", "library");
			} else if (progtype eq "dict") {
				startatlineno = "6,9";
				blankfile ^= "#include <exodus/dict.h>\n\n";
				//programinit() as 2nd line to avoid ppl in external functions before programinit
				//blankfile^="\n";
				blankfile ^= "dict(EXAMPLEDICTID1) {\n";
				blankfile ^= "\tANS=RECORD(1)^\"x\";\n";
				blankfile ^= "}\n\n";

				blankfile ^= "dict(EXAMPLEDICTID2) {\n";
				blankfile ^= "\tANS=RECORD(2)^\"x\";\n";
				blankfile ^= "}\n";
			}

			//ensure ends in eol
			if (blankfile.at(1) ne "\n")
				blankfile ^= "\n";

			//convert to DOS format on Windows
			if (OSSLASH eq "\\")
				blankfile.replacer("\n", "\r\n");

			if (not oswrite(blankfile, filename))
				stop("Cannot create " ^ filename ^ ". Invalid file name, or no rights here.");
			//      startatlineno="4,9";
			//startatlineno="";
		}

		var editcmd0 = editcmd;
		var linenopattern0 = linenopattern;

		//keep editing and compiling until no errors
		while (true) {

			editcmd = editcmd0;
			linenopattern = linenopattern0;

			//record the current file update timestamp
			var fileinfo = osfile(filename);

			//build the edit command
			if (editcmd.contains("$LINENO")) {
				if (not startatlineno)
					linenopattern = "";
				else
					linenopattern = "+" ^ startatlineno;
				editcmd.replacer("+$LINENO", linenopattern);
			}

			if (editcmd.contains("$FILENAME"))
				editcmd.replacer("$FILENAME", filename);
			else
				editcmd ^= " " ^ filename;

			//call the editor
			if (verbose)
				printl(editcmd);

			/////////////////
			osshell(editcmd);
			/////////////////

			//if the file hasnt been updated
			var fileinfo2 = osfile(filename);
			if (fileinfo2 ne fileinfo)
				newfile = false;
			else {
				//delete the skeleton
				printl("File unchanged. Not saving.");
				if (newfile)
					osremove(filename);
				//move to the next file
				break;
			}

			//clear the screen (should be cls on win)
			if (OSSLASH eq "/")
				osshell("clear");
			//else
			//	osshell("cls");

			if (not iscompilable)
				break;

			//build the compiler command
			var compiler = "compile";
			var compileoptions = "{C}";
			var compilecmd = compiler ^ " " ^ filename.quote() ^ " " ^ compileoptions;

			//capture the output
			var compileoutputfilename = filename;
			compileoutputfilename ^= ".~";
			//var compileoutputfilename=filename ^ ".2";
			if (OSSLASH eq "/") {
				compilecmd ^= " 2>&1";
				compilecmd ^= " | tee " ^ compileoutputfilename.quote();
				compilecmd ^= " | pager --chop-long-lines --quit-if-one-screen";

			}
			else
			    compilecmd ^= " > " ^ compileoutputfilename.quote() ^ " 2>&1";

			//call the compiler
			if (verbose)
				printl(compilecmd);

			////////////////////
			osshell(compilecmd);
			////////////////////

			//var tt;
			//tt.inputl("Press Enter ...");

			//if any errors then loop back to edit again
			var errors;
			if (osread(errors, compileoutputfilename, "utf8")) {
				osremove(compileoutputfilename);
				if (OSSLASH ne "/")
					print(errors);

				startatlineno = "";
				// gnu style error lines
				// There's a de facto standard format for compiler or linter error messages,
				// which is the same format as grep -n: FILE_NAME:LINE_NUMBER:MESSAGE
				var matches = errors.match("\\w" ^ filename ^ ":(\\d+):(\\d+):");
				if (matches) {

					//printl(startatlineno);
					//startatlineno = startatlineno.field2(":", 2);
					startatlineno = matches.f(1,2);

					// skip column because nano treats tab as one column
					//startatlineno ^= "," ^ matches.f(1,3);

					//printl(startatlineno);
					//msvc style error lines
					//test.cpp(6) : error C2143: syntax error : missing ';' before '}'
//				} else if (charn = index(errors, ") : error ");charn) {
//					startatlineno = errors.b(charn - 10, 10).field2("(", 2);

				}
				if (startatlineno) {

					var reply = "";
					for (;;) {
						reply = "";
						output("Re-edit at line " ^ startatlineno ^ "? (Y/n)");
						osflush();
						reply.inputn(1);
						printl();
						if (reply.convert("YyNn\n","") eq "")
							break;
					}
					if (reply.convert("Yy\n","") eq "")
						continue;

					abort("");
				}
			}
			print(compileoutputfilename);
			break;
		}
	}

	return 0;
}

programexit()
