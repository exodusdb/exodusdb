#include <exodus/program.h>
programinit()

function main()
{

        var filenames=COMMAND.field(FM,2,999999);

		//"classic" global functions
        var silent=index(ucase(OPTIONS),"S");

		//"modern" methods (member functions)
		//var silent=OPTIONS.ucase().index("S"));

        if (not filenames)
                stop("Syntax is clearfile filename filename ...\ndict files are named dict_filename");

        var nfiles=dcount(filenames,FM);
        for (var filen=1; filen<=nfiles; ++filen) {

                var filename=filenames.a(filen);
                if (clearfile(filename)) {
                        if (not silent)
                                print("Cleared ");
                } else {
                        if (not silent)
                                print("Cannot clear ");
                }
                if (not silent)
                        printl(filename);
        }
	return 0;
}


programexit()
