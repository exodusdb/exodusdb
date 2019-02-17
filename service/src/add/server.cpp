#include <exodus/program.h>
programinit()

#include <win.h>
#include <gen.h>
#include <fin.h>
#include <agy.h>

function main() {
        printl("server says 'Hello World!'");
        mv.labelledcommon[win_common_no]=new win_common;
        mv.labelledcommon[gen_common_no]=new gen_common;
        mv.labelledcommon[fin_common_no]=new fin_common;
        mv.labelledcommon[agy_common_no]=new agy_common;

        //batch mode
        SYSTEM.r(33,1,"1");

        //user
        SYSTEM.r(33,2,"NEOSYS");
        SYSTEM.r(33,3,"NEOSYS");

        //module
        APPLICATION="ADAGENCY";

	//user
	USERNAME="exodus";

	//database
	SYSTEM.r(17,"exodus");

	var dbdir="../data/" ^ SYSTEM.a(17);
	if (not osdir(dbdir))
		osmkdir(dbdir);

        var cmd=SENTENCE.field(" ",2,9999);
        if (not cmd)
                cmd=SENTENCE="initgeneral LOGIN";
        perform(cmd);

        return 0;
}

programexit()

