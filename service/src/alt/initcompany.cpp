#include <exodus/library.h>
libraryinit()

#include <gen_common.h>

var xx;

function main(in nextcompanycode) {
    //c gen
//    #include <common.h>
    #include <general_common.h>
    //global desc1,desc2

    if (gen.companies.unassigned()) {
    	if (!gen.companies.open("COMPANIES")) {
    		createfile("COMPANIES");
	    	if (!gen.companies.open("COMPANIES")) {
	    		call fsmsg();
	    		abort("");
	    	}
    	}
	gen.company = "";
    	gen.gcurrcompany = "";
    }

    if (nextcompanycode.assigned() && gen.companies.assigned()) {

        if (nextcompanycode) {
            if (not(xx.read(gen.companies, nextcompanycode))) {
                //TODO return abort code and change all callers to handle failure
                return 0;
            }
        }

        gen.gcurrcompany = nextcompanycode;
        gen.company = xx;
    }
	return 1;
}

libraryexit()
