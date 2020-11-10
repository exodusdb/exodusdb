#include <exodus/library.h>
libraryinit()

#include <gen_common.h>

var xx;
var temp;
var weeksperperiod;//num
var ndec;//num
var companydesc;
var perioddesc;
var currencydesc;
var desc1;
var desc2;
var datax;

function main(in nextcompanycode) {
    //c gen
    #include <common.h>
    #include <general_common.h>
    //global desc1,desc2

	gen.company = "";

    if (not(nextcompanycode.unassigned())) {

        if (nextcompanycode) {
            if (not(xx.read(gen.companies, nextcompanycode))) {
                //TODO return abort code and change all callers to handle failure
                return 0;
            }
        }

        gen.gcurrcompany = nextcompanycode;
    }
	return 1;
}

libraryexit()
