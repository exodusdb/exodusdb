#include <exodus/program.h>
programinit()

function main() {
        //hard coded editor at the moment
        //http://www.nano-editor.org/docs.php
        var editor="nano";
//restrict to editing records for now
//#define ALLOW_EDIC
#ifdef ALLOW_EDIC
        if (dcount(COMMAND,FM)<3) {
#else
        if (dcount(COMMAND,FM)<2) {
#endif

                //quit if arguments
                if (dcount(COMMAND,FM)<2)
					abort(
						"Syntax is:"
                        "\nedit databasefilename key ..."
#ifdef ALLOW_EDIC
                        "\nor"
                        "\nedit osfilename"
#endif
					);

                //switch to edic if only one argument
                osshell(COMMAND.pickreplace(1,0,0,"edic").convert(FM," "));
                stop();
        }

        var filename=COMMAND.a(2);
        var key=COMMAND.a(3);
        var fieldno=COMMAND.a(4);

        //connect to the database
        if (not connect())
                stop("Please login");

        //check the file exists
        var file;
        if (not open(filename,file))
                stop("Cannot open file " ^ filename);

        //get the record from the database
        var origrecord;
        if (not read(origrecord,file,key)) {
                //check if exists in upper or lower case
                var key2=key.ucase();
                if (key2 eq key)
                        key2.lcaser();
                if (read(origrecord,file,key2))
                        key=key2;
                else
                        origrecord="";
        }

        var text=origrecord.a(fieldno);
        var sepchar=fieldno ? VM : FM;

        //convert to text format
        text.swapper("\\","\\\\");
        text.swapper("\n","\\n");
        text.swapper(sepchar,"\n");

        //put the text on a temp file in order to edit it
        var temposfilename=filename^ "~" ^ key;
        var invalidfilechars=L"\"\'\u00A3$%^&*(){}[]:;#<>?,./\\|";
        temposfilename.lcaser().converter(invalidfilechars,str("-",len(invalidfilechars)));
        temposfilename^=".tmp";
        oswrite(text,temposfilename);

        //record file update timedate
        var fileinfo=osfile(temposfilename);
        if (not fileinfo)
                abort("Couldnt write local copy for editing "^temposfilename);

        //fire up the editor
        var editcmd=editor ^ " " ^ temposfilename.quote();
        printl(editcmd);
        osshell(editor ^ " " ^ temposfilename);

        //if the file has been updated
        var fileinfo2=osfile(temposfilename);
        if (fileinfo2 eq fileinfo) {
                //file has not been edited
                osdelete(temposfilename);
        } else {
                //file has been edited
                var text2;
                osread(text2,temposfilename);

                //remove trailing lf or cr or crlf
                trimmerb(text2,"\r\n");

                //convert to record format
                text2.swapper("\n",sepchar);
                text2.swapper("\\n","\n");
                text2.swapper("\\\\","\\");

                if (text2 ne text) {

                        //print("Ok to update? ");
                        //var reply=inputl();
                        var reply="Y";

			var newrecord=fieldno ? origrecord.r(fieldno,text2) : text2;

                        //keep trying to update - perhaps futilely
                        //at least temp file will be left in the directory
                        while (ucase(reply)[1] eq "Y" and true) {

                                if (write(newrecord,file,key)) {
                                        printl(filename^" written "^key);
                                        osdelete(temposfilename);
                                        break;
                                }
                                var temp;
                                temp.input();

                        }
                }
        }
        return 0;
}

programexit()
