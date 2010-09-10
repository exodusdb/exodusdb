#include <exodus/exodus.h>

program()
{

        var filename=_COMMAND.extract(2);
        var silent=index(ucase(_OPTIONS),"S");

        if (deletefile(filename)) {
                if (not silent)
                        print("Deleted ");
        }else{
                if (not silent)
                        print("Cannot delete ");
        }
        if (not silent)
                println(filename);

}





