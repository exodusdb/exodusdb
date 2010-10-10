#include <exodus/exodus.h>

program()
{

	//check command syntax
	//edit filename
	if (dcount(_COMMAND,FM)<2) 
		abort("Syntax is 'edit osfilename'");
	
	var verbose=_OPTIONS.ucase().index("V");

	var editor=osgetenv("VISUAL");
	var linenopattern="$LINENO ";
	if (not editor)
		editor.osgetenv("EDITOR");

	//TODO simplify this editor finding code

	//enable edit at first error for crimson editor
	if (editor.lcase().index("cedt") and not editor.index("$") )
		editor^=" /L:$LINENO '$FILENAME'";

	//look for nano.exe next to edic.exe
	if (not editor and _SLASH=="\\") {
		var nanopath=_EXECPATH.swap("edic","nano");
		if (nanopath.osfile())
			editor="nano $LINENO'$FILENAME'";
	}

	//look for nano in parent bin
	if (not editor and _SLASH=="\\") {
		var nanopath="..\\bin\\nano.exe";
		if (nanopath.osfile())
			editor="nano $LINENO'$FILENAME'";
	}

	if (editor.index("nano"))
		linenopattern="+$LINENO ";

	//otherwise on windows try to locate CYGWIN nano or vi
	if (not editor and _SLASH=="\\")
	{
		//from environment variable
		var cygwinpath=osgetenv("CYGWIN_BIN");
		//else from current disk
		if (not cygwinpath)
			cygwinpath="\\cygwin\\bin\\";
		//else from c:
		if (not osdir(cygwinpath))
			cygwinpath="c:\\cygwin\\bin\\";
		//else give up
		if (not osdir(cygwinpath))
			cygwinpath="";

		if (cygwinpath and cygwinpath.substr(-1) ne _SLASH)
			cygwinpath^=_SLASH;
		//editor=cygwinpath^"bash --login -i -c \"/bin/";
		editor=cygwinpath;
		if (osfile(cygwinpath^"nano.exe") or osfile("nano.exe")) {
			editor="nano $LINENO'$FILENAME'";
			if (osfile(cygwinpath^"nano.exe"))
				editor.splicer(1,0,cygwinpath);
			//editor^="\"";
			linenopattern="+$LINENO ";
		} else if (osfile(cygwinpath^"vi.exe") or osfile("vi.exe")) {
			editor="vi -c \":$LINENO\" $FILENAME";
			if (osfile(cygwinpath^"vi.exe"))
				editor.splicer(1,0,cygwinpath);
			//editor^="\"";
		} else
			editor="";

		//configure nanorc (on windows)
		//TODO same for non-windows
		//nano on windows looks for nanorc config file as follows (probably merges all found)
		//C:\cygwin\usr\local\etc\nanorc
		//C:\cygwin\etc\nanorc (only if cygwin exists)
		//C:\Documents and Settings\USERNAME\.nanorc  ($HOMEDRIVE$HOMEPATH)
		var nanorcfilename;
		if (cygwinpath) {
			nanorcfilename=cygwinpath.field(_SLASH,1,dcount(cygwinpath,_SLASH)-2) ^ _SLASH ^ "etc" ^ _SLASH ^ "nanorc";
		} else {
			nanorcfilename=osgetenv("HOMEDRIVE") ^ osgetenv("HOMEPATH");
			if (nanorcfilename.substr(-1) ne _SLASH)
				nanorcfilename^=_SLASH;
			nanorcfilename^=".nanorc";
		}
		if (not osfile(nanorcfilename))
		{
			var nanorctemplatefilename=_EXECPATH.field(_SLASH,1,dcount(_EXECPATH,_SLASH)-1) ^ _SLASH ^ "nanorc";
			if (oscopy(nanorctemplatefilename,nanorcfilename)) {
				println("Copied " ^ nanorctemplatefilename.quote() ^ " to " ^ nanorcfilename.quote());
				var ().inputln("Note: nano c++ syntax highlighting has been installed. Press Enter ... ");
			} else {
				errputln("Could not copy " ^ nanorctemplatefilename.quote() ^ " to " ^ nanorcfilename.quote());
				if (not osfile(nanorctemplatefilename))
					errputln("nano syntax highlighting file is missing.");
			}
		}
	}
	if (not editor)
	{
		if (_SLASH=="/")
			editor="nano ";
		else
			editor="notepad";
		println("Environment EDITOR not set. Using " ^ editor);
	}
	//editor="vi";
	editor.swapper("nano ", "nano --const --nowrap --autoindent --suspend ");

	if (editor.index("nano"))
		println("http://www.nano-editor.org/dist/v2.1/nano.html");

	//configure nano syntax highlighting

	var filenames=field(_COMMAND,FM,2,99999);
	var nfiles=dcount(filenames,FM);
	var filen=0;
	while (filen<nfiles)
	{
		filen+=1;
		var filename=filenames.extract(filen).unquote();
		
		//split out trailing line number after :
		var startatlineno=field(filename,":",2);
		if (startatlineno.isnum())
			filename=field(filename,":",1);
		else
			startatlineno="";
			
		if (not index(filename,"."))
			filename^=".cpp";

		var iscompilable=filename.field2(".",-1).substr(1,1).lcase() ne "h";

		//make absolute in case EDITOR changes current working directory
		var editcmd=editor;
		if (editcmd.index("$ABSOLUTEFILENAME"))
		{
			editcmd.swapper("$ABSOLUTEFILENAME","$FILENAME");

			filename=oscwd()^_SLASH^filename;
		}

		//prepare a skeleton exodus cpp file
		var newfile=false;
		if (iscompilable and !osfile(filename)) {
			newfile=true;
			var blankfile="";
			blankfile^="#include <exodus/exodus.h>\n";
			blankfile^="\n";
			blankfile^="program() {\n";
			blankfile^="\tprint(\"Hello World!\");\n";
			blankfile^="}\n";
			if (_SLASH ne "/")
				blankfile.swapper("\n","\r\n"),
			oswrite(blankfile,filename);
			startatlineno="4,9";
			//startatlineno="";
		}

		var editcmd0=editcmd;
		var linenopattern0=linenopattern;

		//keep editing and compiling until no errors
		while (true)
		{

			editcmd=editcmd0;
			linenopattern=linenopattern0;

			//record the current file update timestamp
			var fileinfo=osfile(filename);

			//build the edit command
			if (editcmd.index("$LINENO"))
			{
				if (not startatlineno)
					linenopattern="";
				else
					linenopattern.swapper("$LINENO",startatlineno.field(",",1));
				editcmd.swapper("$LINENO",linenopattern);
			}
			if (editcmd.index("$FILENAME"))
				editcmd.swapper("$FILENAME",filename);
			else
				editcmd ^= " " ^ filename;

			//call the editor
			if (verbose)
				println(editcmd);
			osshell(editcmd);
	
			//if the file hasnt been updated
			var fileinfo2=osfile(filename);
			if (fileinfo2 ne fileinfo)
				newfile=false;
			else {
				//delete the skeleton
				if (newfile)
					osdelete(filename);
				//move to the next file
				break;
			}

			//clear the screen (should be cls on win)
			if (_SLASH eq "/")
				osshell("clear");
			//else
			//	osshell("cls");

			if (not iscompilable)
				break;

			//build the compiler command
			var compiler="compile";
			var compileoptions="";
			var compilecmd=compiler ^ " " ^ filename.quote() ^ compileoptions;
			//capture the output
			var compileoutputfilename=filename ^ ".2";
			if (_SLASH eq "/")
				compilecmd ^= " 2>&1 | tee " ^ compileoutputfilename.quote();
			else
				compilecmd ^= " > " ^ compileoutputfilename.quote() ^ " 2>&1";

			//call the compiler			
			if (verbose)
				println(compilecmd);
			osshell(compilecmd);

			//var tt;
			//tt.inputln("Press Enter ...");

			//if any errors then loop back to edit again
			var errors;
			if (osread(errors,compileoutputfilename)){
				osdelete(compileoutputfilename);

				if (_SLASH ne "/")
					print(errors);

				startatlineno="";
				var charn;
				//gnu style error lines
				if (charn=index(errors, ": error:")) {
					startatlineno=errors.substr(charn-9,9);

					//println(startatlineno);
					startatlineno=startatlineno.field2(":",2);
					//println(startatlineno);
				//msvc style error lines
				//test.cpp(6) : error C2143: syntax error : missing ';' before '}'
				} else if (charn=index(errors,") : error ")) {
					startatlineno=errors.substr(charn-10,10).field2("(",2);
				}
				if (startatlineno)
				{
					print("Press any key to re-edit at line "^startatlineno^" ... ");
					var().input(1);
					continue;
				}
			}

			break;
		}
			
	}
}
