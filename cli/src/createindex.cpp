#include <exodus/exodus.h>

program()
{

        var filename=_COMMAND.extract(2);
        var fieldname=_COMMAND.extract(3);
        var silent=index(ucase(_OPTIONS),"S");

		if (not filename or not fieldname)
			abort("Syntax is 'createindex filename fieldname (S=Silent)'");

        if (createindex(filename,fieldname)) {
                if (not silent)
                        print("Created ");
        }else{
                if (not silent)
                        print("Cannot create ");
        }
        if (not silent)
                println(filename ^ " " ^ fieldname ^ " index");

}

