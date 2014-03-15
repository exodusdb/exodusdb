#include <exodus/exodus.h>

program()
{

        var filename=COMMAND.a(2);
        var fieldname=COMMAND.a(3);
        var silent=index(ucase(OPTIONS),"S");

        if (not filename or not fieldname)
                abort("Syntax is 'createindex filename fieldname (S=Silent)'");

        if (createindex(filename,fieldname)) {
                if (not silent)
                        print("Created ");
        } else {
                if (not silent)
                        print("Cannot create ");
        }
        if (not silent)
                printl(filename ^ " " ^ fieldname ^ " index");

}

