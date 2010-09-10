#include <exodus/exodus.h>

program()
{

        var filename=_COMMAND.extract(2);
        var silent=index(ucase(_OPTIONS),"S");

        if (createfile(filename)) {
                if (not silent)
                        print("Created ");
        }else{
                if (not silent)
                        print("Cannot create ");
        }
        if (not silent)
                println(filename);

}





