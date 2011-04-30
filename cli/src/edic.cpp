#include <exodus/exodus.h>

program()
{

        //check command syntax
        //edit filename
        if (dcount(COMMAND,FM)<2)
                abort("Syntax is 'edic osfilename'");

        var verbose=OPTIONS.ucase().index("V");

        var editor=osgetenv("VISUAL");
        var linenopattern="$LINENO ";
        if (not editor)
                editor.osgetenv("EDITOR");

        //TODO simplify this editor finding code

        //enable edit at first error for crimson editor (no quotes around filename for cedt)
        if (editor.lcase().index("cedt") and not editor.index("$") )
                editor^=" /L:$LINENO $FILENAME";

        //look for nano.exe next to edic.exe
        if (not editor and SLASH=="\\") {
                var nanopath=EXECPATH.swap("edic","nano");
                if (nanopath.osfile())
                        editor="nano $LINENO'$FILENAME'";
        }

        //look for nano in parent bin
        if (not editor and SLASH=="\\") {
                var nanopath="..\\bin\\nano.exe";
                if (nanopath.osfile())
                        editor="nano $LINENO'$FILENAME'";
        }

        if (editor.index("nano"))
                linenopattern="+$LINENO ";

        //otherwise on windows try to locate CYGWIN nano or vi
        if (not editor and SLASH=="\\") {
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

                if (cygwinpath and cygwinpath[-1] ne SLASH)
                        cygwinpath^=SLASH;
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
                        nanorcfilename=cygwinpath.field(SLASH,1,dcount(cygwinpath,SLASH)-2) ^ SLASH ^ "etc" ^ SLASH ^ "nanorc";
                } else {
                        nanorcfilename=osgetenv("HOMEDRIVE") ^ osgetenv("HOMEPATH");
                        if (nanorcfilename[-1] ne SLASH)
                                nanorcfilename^=SLASH;
                        nanorcfilename^=".nanorc";
                }
                if (not osfile(nanorcfilename)) {
                        var nanorctemplatefilename=EXECPATH.field(SLASH,1,dcount(EXECPATH,SLASH)-1) ^ SLASH ^ "nanorc";
                        if (oscopy(nanorctemplatefilename,nanorcfilename)) {
                                printl("Copied " ^ nanorctemplatefilename.quote() ^ " to " ^ nanorcfilename.quote());
                                var ().input("Note: nano c++ syntax highlighting has been installed. Press Enter ... ");
                        } else {
                                errputl("Could not copy " ^ nanorctemplatefilename.quote() ^ " to " ^ nanorcfilename.quote());
                                if (not osfile(nanorctemplatefilename))
                                        errputl("nano syntax highlighting file is missing.");
                        }
                }
        }
        if (not editor) {
                if (SLASH=="/")
                        editor="nano ";
                else
                        editor="notepad";
                printl("Environment EDITOR not set. Using " ^ editor);
        }
        //editor="vi";
        editor.swapper("nano ", "nano --const --nowrap --autoindent --suspend ");

        if (editor.index("nano"))
                printl("http://www.nano-editor.org/dist/v2.1/nano.html");

        //configure nano syntax highlighting

        var filenames=field(COMMAND,FM,2,99999);
        var nfiles=dcount(filenames,FM);
        var filen=0;
        while (filen<nfiles) {
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

                var iscompilable=filename.field2(".",-1)[1].lcase() ne "h";

                //make absolute in case EDITOR changes current working directory
                var editcmd=editor;
                if (editcmd.index("$ABSOLUTEFILENAME")) {
                        editcmd.swapper("$ABSOLUTEFILENAME","$FILENAME");

                        filename=oscwd()^SLASH^filename;
                }
                //prepare a skeleton exodus cpp file
                var newfile=false;
                if (iscompilable and !osfile(filename)) {

						var basefilename=field2(filename,SLASH,-1);
						basefilename=field(basefilename,".",dcount(basefilename,".")-1);

						var progtype;
						var question="1=Normal Program, 2=External Subroutine or Function";
						//question^="\n3=main(), 4=simple so/dll\n";
						question^="\n"^basefilename.quote()^" does not exist. Create what? (1-2) ";
						while (true) {
							progtype.input(question);
							if (progtype eq 2)
								progtype="classlib";
							else if (progtype eq 3)
								progtype="main";
							else if (progtype eq 4)
								progtype="mainlib";
							else if (progtype eq 1)
								progtype="class";
							else
								stop();
							break;
						}

                        newfile=true;
                        var blankfile="";
						if (progtype eq "main" or progtype eq "mainlib") {
	                        startatlineno="4,9";
							blankfile^="#include <exodus/exodus.h>\n";
							blankfile^="\n";
							blankfile^="program() {\n";
							blankfile^="\tprintl(\""^basefilename^" says 'Hello World!'\");\n";
							if (progtype eq "mainlib")
								blankfile^="\treturn 0;\n";
							blankfile^="}\n";
							if (progtype eq "mainlib")
								blankfile.swapper("program()","function "^basefilename^"()");
						} else if (progtype eq "class" or progtype eq "classlib") {
	                        startatlineno="6,9";
							blankfile^="#include <exodus/program.h>\n";
							blankfile^="\n";
							blankfile^="programinit()\n";
							blankfile^="\n";
							blankfile^="function main(/*in arg1, out arg2*/) {\n";
							blankfile^="\tprintl(\""^basefilename^" says 'Hello World!'\");\n";
							blankfile^="\treturn 0;\n";
							blankfile^="}\n";
							blankfile^="\nprogramexit()";
							blankfile^="\n";

							if (progtype eq "classlib")
								blankfile.swapper("program","library");
						}

						if (blankfile[1] ne "\n")
							blankfile^="\n";
						if (SLASH ne "/")
                                blankfile.swapper("\n","\r\n");

			if (not oswrite(blankfile,filename))
				stop("Cannot create "^filename^". Invalid file name, or no rights here.");
                  //      startatlineno="4,9";
                        //startatlineno="";
                }

                var editcmd0=editcmd;
                var linenopattern0=linenopattern;

                //keep editing and compiling until no errors
                while (true) {

                        editcmd=editcmd0;
                        linenopattern=linenopattern0;

                        //record the current file update timestamp
                        var fileinfo=osfile(filename);

                        //build the edit command
                        if (editcmd.index("$LINENO")) {
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
                                printl(editcmd);
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
                        if (SLASH eq "/")
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
                        if (SLASH eq "/")
                                compilecmd ^= " 2>&1 | tee " ^ compileoutputfilename.quote();
                        else
                                compilecmd ^= " > " ^ compileoutputfilename.quote() ^ " 2>&1";

                        //call the compiler
                        if (verbose)
                                printl(compilecmd);
                        osshell(compilecmd);

                        //var tt;
                        //tt.inputl("Press Enter ...");

                        //if any errors then loop back to edit again
                        var errors;
                        if (osread(errors,compileoutputfilename)) {
                                osdelete(compileoutputfilename);

                                if (SLASH ne "/")
                                        print(errors);

                                startatlineno="";
                                var charn;
                                //gnu style error lines
                                if (charn=index(errors, ": error:")) {
                                        startatlineno=errors.substr(charn-9,9);

                                        //printl(startatlineno);
                                        startatlineno=startatlineno.field2(":",2);
                                        //printl(startatlineno);
                                        //msvc style error lines
                                        //test.cpp(6) : error C2143: syntax error : missing ';' before '}'
                                } else if (charn=index(errors,") : error ")) {
                                        startatlineno=errors.substr(charn-10,10).field2("(",2);
                                }
                                if (startatlineno) {
                                        print("Press any key to re-edit at line "^startatlineno^" ... ");
                                        var().input("");
                                        continue;
                                }
                        }

                        break;
                }

        }
}
