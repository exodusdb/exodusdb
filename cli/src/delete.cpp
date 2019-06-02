
#include <exodus/program.h>

programinit()

function main() {

        var filename=COMMAND.a(2);

        var itemids=remove(COMMAND,1,0,0);
        remover(itemids,1,0,0);

        var silent=index(ucase(OPTIONS),"S");

        if (not filename or not itemids)
                abort("Syntax is 'delete filename itemid ... (S=Silent)'");

        var file;
        if (not open(filename,file))
                abort(filename^" file does not exist.");

        var sep=0;
        var posn=1;
        var ndeleted=0;
        do {
                var itemid=remove(itemids,posn,sep);

                if (itemid=="*") {
                        clearfile(file);
                        if (not silent)
                                printl("All records deleted");
                        stop();
                }

                if (deleterecord(file,itemid))
                        ++ndeleted;
                else if (not silent)
                        printl(quote(itemid)^" does not exist.");
        } while (sep);

        if (not silent)
                printl(ndeleted^" record(s) deleted.");
	return 0;
}

programexit()
