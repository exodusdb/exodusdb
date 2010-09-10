#include <exodus/exodus.h>

program()
{

        var filename=_COMMAND.extract(2);
        var fieldname=_COMMAND.extract(3);
        var silent=index(ucase(_OPTIONS),"S");

		if (not filename or not fieldname)
			abort("Syntax is 'deleteindex filename fieldname (S=Silent)'");

        if (deleteindex(filename,fieldname)) {
                if (not silent)
                        print("Deleted ");
        }else{
                if (not silent)
                        print("Cannot delete ");
        }
        if (not silent)
                println(filename ^ " " ^ fieldname ^ " index");

}

