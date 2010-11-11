#include <exodus/exodus.h>

program()
{
        var filename=COMMAND.extract(2);
        if (not filename)
                abort("Syntax is 'listindexes filename'");
        printl(listindexes(filename).convert(FM,"\n"));
}

