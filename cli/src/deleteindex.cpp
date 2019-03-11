#include <exodus/program.h>
programinit()

function main(){

        var filename=COMMAND.a(2);
        var fieldname=COMMAND.a(3);
        var silent=index(ucase(OPTIONS),"S");

        if (not filename or not fieldname)
                abort("Syntax is 'deleteindex filename fieldname (S=Silent)'");

        if (deleteindex(filename,fieldname)) {
                if (not silent)
                        print("Deleted ");
        } else {
                if (not silent)
                        print("Cannot delete ");
        }
        if (not silent)
                printl(filename ^ " " ^ fieldname ^ " index");

	return 0;
}

programexit()
